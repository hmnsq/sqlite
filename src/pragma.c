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
#include "sqliteInt.h"

#ប្រសិនបើ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ប្រសិនបើ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  បើមិនដូច្នេះ
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
#include "pragma.h"

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
static u8 getSafetyLevel(const var_na *z, ចំនួនគត់ omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  static const var_na zText[] = "onoffalseyestruextrafull";
  static const u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  static const u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  static const u8 iValue[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  ចំនួនគត់ i, n;
  ប្រសិនបើ( sqlite3Isdigit(*z) ){
    ត្រឡប់ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  សម្រាប់(i=0; i<ArraySize(iLength); i++){
    ប្រសិនបើ( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || iValue[i]<=1)
    ){
      ត្រឡប់ iValue[i];
    }
  }
  ត្រឡប់ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(const var_na *z, u8 dflt){
  ត្រឡប់ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#ប្រសិនបើ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
static ចំនួនគត់ getLockingMode(const var_na *z){
  ប្រសិនបើ( z ){
    ប្រសិនបើ( 0==sqlite3StrICmp(z, "exclusive") ) ត្រឡប់ PAGER_LOCKINGMODE_EXCLUSIVE;
    ប្រសិនបើ( 0==sqlite3StrICmp(z, "normal") ) ត្រឡប់ PAGER_LOCKINGMODE_NORMAL;
  }
  ត្រឡប់ PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
static ចំនួនគត់ getAutoVacuum(const var_na *z){
  ចំនួនគត់ i;
  ប្រសិនបើ( 0==sqlite3StrICmp(z, "none") ) ត្រឡប់ BTREE_AUTOVACUUM_NONE;
  ប្រសិនបើ( 0==sqlite3StrICmp(z, "full") ) ត្រឡប់ BTREE_AUTOVACUUM_FULL;
  ប្រសិនបើ( 0==sqlite3StrICmp(z, "incremental") ) ត្រឡប់ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  ត្រឡប់ (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
static ចំនួនគត់ getTempStore(const var_na *z){
  ប្រសិនបើ( z[0]>='0' && z[0]<='2' ){
    ត្រឡប់ z[0] - '0';
  }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3StrICmp(z, "file")==0 ){
    ត្រឡប់ 1;
  }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3StrICmp(z, "memory")==0 ){
    ត្រឡប់ 2;
  }បើមិនដូច្នេះ{
    ត្រឡប់ 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
static ចំនួនគត់ invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  ប្រសិនបើ( db->aDb[1].pBt!=0 ){
    ប្រសិនបើ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3ErrorMsg(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      ត្រឡប់ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  ត្រឡប់ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
static ចំនួនគត់ changeTempStorage(Parse *pParse, const var_na *zStorageType){
  ចំនួនគត់ ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  ប្រសិនបើ( db->temp_store==ts ) ត្រឡប់ SQLITE_OK;
  ប្រសិនបើ( invalidateTempStorage( pParse ) != SQLITE_OK ){
    ត្រឡប់ SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  ត្រឡប់ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
static void setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  const PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3VdbeSetNumCols(v, n==0 ? 1 : n);
  ប្រសិនបើ( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }បើមិនដូច្នេះ{
    ចំនួនគត់ i, j;
    សម្រាប់(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
static void returnSingleInt(Vdbe *v, i64 value){
  sqlite3VdbeAddInt64(v, 1, value);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
static void returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  const var_na *zValue      /* Value to be returned */
){
  ប្រសិនបើ( zValue ){
    sqlite3VdbeLoadString(v, 1, (const var_na*)zValue);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
static void setAllPagerFlags(sqlite3 *db){
  ប្រសិនបើ( db->autoCommit ){
    Db *pDb = db->aDb;
    ចំនួនគត់ n = db->nDb;
    assert( SQLITE_FullFSync==PAGER_FULLFSYNC );
    assert( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    assert( SQLITE_CacheSpill==PAGER_CACHESPILL );
    assert( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    assert( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    ខណៈពេល( (n--) > 0 ){
      ប្រសិនបើ( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#បើមិនដូច្នេះ
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_KEY
static const var_na *actionName(u8 action){
  const var_na *zName;
  switch( action ){
    pak_sa OE_SetNull:  zName = "SET NULL";        bha_nga;
    pak_sa OE_SetDflt:  zName = "SET DEFAULT";     bha_nga;
    pak_sa OE_Cascade:  zName = "CASCADE";         bha_nga;
    pak_sa OE_Restrict: zName = "RESTRICT";        bha_nga;
    default:          zName = "NO ACTION"; 
                      assert( action==OE_None ); bha_nga;
  }
  ត្រឡប់ zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
const var_na *sqlite3JournalModename(ចំនួនគត់ eMode){
  static var_na * const azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  assert( PAGER_JOURNALMODE_DELETE==0 );
  assert( PAGER_JOURNALMODE_PERSIST==1 );
  assert( PAGER_JOURNALMODE_OFF==2 );
  assert( PAGER_JOURNALMODE_TRUNCATE==3 );
  assert( PAGER_JOURNALMODE_MEMORY==4 );
  assert( PAGER_JOURNALMODE_WAL==5 );
  assert( eMode>=0 && eMode<=ArraySize(azModeName) );

  ប្រសិនបើ( eMode==ArraySize(azModeName) ) ត្រឡប់ 0;
  ត្រឡប់ azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
static const PragmaName *pragmaLocate(const var_na *zName){
  ចំនួនគត់ upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  ខណៈពេល( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    ប្រសិនបើ( rc==0 ) bha_nga;
    ប្រសិនបើ( rc<0 ){
      upr = mid - 1;
    }បើមិនដូច្នេះ{
      lwr = mid + 1;
    }
  }
  ត្រឡប់ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
static void pragmaFunclistLine(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  ចំនួនគត់ isBuiltin,         /* True if this is a built-in function */
  ចំនួនគត់ showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  ប្រសិនបើ( showInternFuncs ) mask = 0xffffffff;
  សម្រាប់(; p; p=p->pNext){
    const var_na *zType;
    static const var_na *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    assert( SQLITE_FUNC_ENCMASK==0x3 );
    assert( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    assert( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    assert( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    ប្រសិនបើ( p->xSFunc==0 ) continue;
    ប្រសិនបើ( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      continue;
    }   
    ប្រសិនបើ( p->xValue!=0 ){
      zType = "w";
    }បើមិនដូច្នេះ ប្រសិនបើ( p->xFinalize!=0 ){
      zType = "a";
    }បើមិនដូច្នេះ{
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
static ចំនួនគត់ integrityCheckResultRow(Vdbe *v){
  ចំនួនគត់ addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  ត្រឡប់ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
static ចំនួនគត់ tableSkipIntegrityCheck(const Table *pTab, const Table *pObjTab){
  ប្រសិនបើ( pObjTab ){
    ត្រឡប់ pTab!=pObjTab;
  }បើមិនដូច្នេះ{
    ត្រឡប់ (pTab->tabFlags & TF_Imposter)!=0;
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
void sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *pValue,      /* Token for <value>, or NULL */
  ចំនួនគត់ minusFlag       /* True if a '-' sign preceded <value> */
){
  var_na *zLeft = 0;       /* Nul-terminated UTF-8 string <id> */
  var_na *zRight = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  const var_na *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  var_na *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  ចំនួនគត់ iDb;               /* Database index for <database> */
  ចំនួនគត់ rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  const PragmaName *pPragma;   /* The pragma */

  ប្រសិនបើ( v==0 ) ត្រឡប់;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  ប្រសិនបើ( iDb<0 ) ត្រឡប់;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  ប្រសិនបើ( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    ត្រឡប់;
  }

  zLeft = sqlite3NameFromToken(db, pId);
  ប្រសិនបើ( !zLeft ) ត្រឡប់;
  ប្រសិនបើ( minusFlag ){
    zRight = sqlite3MPrintf(db, "-%T", pValue);
  }បើមិនដូច្នេះ{
    zRight = sqlite3NameFromToken(db, pValue);
  }

  assert( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  ប្រសិនបើ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, zLeft, zRight, zDb) ){
    goto pragma_out;
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
  aFcntl[1] = zLeft;
  aFcntl[2] = zRight;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (void*)aFcntl);
  ប្រសិនបើ( rc==SQLITE_OK ){
    sqlite3VdbeSetNumCols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    goto pragma_out;
  }
  ប្រសិនបើ( rc!=SQLITE_NOTFOUND ){
    ប្រសិនបើ( aFcntl[0] ){
      sqlite3ErrorMsg(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    goto pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(zLeft);
  ប្រសិនបើ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    goto pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  ប្រសិនបើ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    ប្រសិនបើ( sqlite3ReadSchema(pParse) ) goto pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  ប្រសិនបើ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || zRight==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  switch( pPragma->ePragTyp ){
 
#ប្រសិនបើ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  pak_sa PragTyp_DEFAULT_CACHE_SIZE: {
    static const ចំនួនគត់ iLn = VDBE_OFFSET_LINENO(2);
    static const VdbeOpList getCacheSize[] = {
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
    ប្រសិនបើ( !zRight ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      ប្រសិនបើ( ONLY_IF_REALLOC_STRESS(aOp==0) ) bha_nga;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }បើមិនដូច្នេះ{
      ចំនួនគត់ size = sqlite3AbsInt32(sqlite3Atoi(zRight));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    bha_nga;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#ប្រសិនបើ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  pak_sa PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    ប្រសិនបើ( !zRight ){
      ចំនួនគត់ size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }បើមិនដូច្នេះ{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(zRight);
      ប្រសិនបើ( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    bha_nga;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  pak_sa PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    ចំនួនគត់ b = -1;
    assert( pBt!=0 );
    ប្រសិនបើ( zRight ){
      ប្រសិនបើ( sqlite3_stricmp(zRight, "fast")==0 ){
        b = 2;
      }បើមិនដូច្នេះ{
        b = sqlite3GetBoolean(zRight, 0);
      }
    }
    ប្រសិនបើ( pId2->n==0 && b>=0 ){
      ចំនួនគត់ ii;
      សម្រាប់(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    bha_nga;
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
  pak_sa PragTyp_PAGE_COUNT: {
    ចំនួនគត់ iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    ប្រសិនបើ( sqlite3Tolower(zLeft[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }បើមិនដូច្នេះ{
      ប្រសិនបើ( zRight && sqlite3DecOrHexToI64(zRight,&x)==0 ){
        ប្រសិនបើ( x<0 ) x = 0;
        បើមិនដូច្នេះ ប្រសិនបើ( x>0xfffffffe ) x = 0xfffffffe;
      }បើមិនដូច្នេះ{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (ចំនួនគត់)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    bha_nga;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  pak_sa PragTyp_LOCKING_MODE: {
    const var_na *zRet = "normal";
    ចំនួនគត់ eMode = getLockingMode(zRight);

    ប្រសិនបើ( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }បើមិនដូច្នេះ{
      Pager *pPager;
      ប្រសិនបើ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        ចំនួនគត់ ii;
        assert(pDb==&db->aDb[0]);
        សម្រាប់(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    assert( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    ប្រសិនបើ( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    bha_nga;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  pak_sa PragTyp_JOURNAL_MODE: {
    ចំនួនគត់ eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    ចំនួនគត់ ii;           /* Loop counter */

    ប្រសិនបើ( zRight==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }បើមិនដូច្នេះ{
      const var_na *zMode;
      ចំនួនគត់ n = sqlite3Strlen30(zRight);
      សម្រាប់(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        ប្រសិនបើ( sqlite3StrNICmp(zRight, zMode, n)==0 ) bha_nga;
      }
      ប្រសិនបើ( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      ប្រសិនបើ( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    ប្រសិនបើ( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    សម្រាប់(ii=db->nDb-1; ii>=0; ii--){
      ប្រសិនបើ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    bha_nga;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  pak_sa PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    ប្រសិនបើ( zRight ){
      sqlite3DecOrHexToI64(zRight, &iLimit);
      ប្រសិនបើ( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    bha_nga;
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
  pak_sa PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    ប្រសិនបើ( !zRight ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }បើមិនដូច្នេះ{
      ចំនួនគត់ eAuto = getAutoVacuum(zRight);
      assert( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      ប្រសិនបើ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        static const ចំនួនគត់ iLn = VDBE_OFFSET_LINENO(2);
        static const VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_ROOT_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        ចំនួនគត់ iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        ប្រសិនបើ( ONLY_IF_REALLOC_STRESS(aOp==0) ) bha_nga;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    bha_nga;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  pak_sa PragTyp_INCREMENTAL_VACUUM: {
    ចំនួនគត់ iLimit = 0, addr;
    ប្រសិនបើ( zRight==0 || !sqlite3GetInt32(zRight, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    bha_nga;
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
  pak_sa PragTyp_CACHE_SIZE: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ប្រសិនបើ( !zRight ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }បើមិនដូច្នេះ{
      ចំនួនគត់ size = sqlite3Atoi(zRight);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    bha_nga;
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
  pak_sa PragTyp_CACHE_SPILL: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ប្រសិនបើ( !zRight ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }បើមិនដូច្នេះ{
      ចំនួនគត់ size = 1;
      ប្រសិនបើ( sqlite3GetInt32(zRight, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      ប្រសិនបើ( sqlite3GetBoolean(zRight, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }បើមិនដូច្នេះ{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    bha_nga;
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
  pak_sa PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#ប្រសិនបើ SQLITE_MAX_MMAP_SIZE>0
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ប្រសិនបើ( zRight ){
      ចំនួនគត់ ii;
      sqlite3DecOrHexToI64(zRight, &sz);
      ប្រសិនបើ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      ប្រសិនបើ( pId2->n==0 ) db->szMmap = sz;
      សម្រាប់(ii=db->nDb-1; ii>=0; ii--){
        ប្រសិនបើ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#បើមិនដូច្នេះ
    sz = 0;
    rc = SQLITE_OK;
#endif
    ប្រសិនបើ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }បើមិនដូច្នេះ ប្រសិនបើ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    bha_nga;
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
  pak_sa PragTyp_TEMP_STORE: {
    ប្រសិនបើ( !zRight ){
      returnSingleInt(v, db->temp_store);
    }បើមិនដូច្នេះ{
      changeTempStorage(pParse, zRight);
    }
    bha_nga;
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
  pak_sa PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ប្រសិនបើ( !zRight ){
      returnSingleText(v, sqlite3_temp_directory);
    }បើមិនដូច្នេះ{
#ifndef SQLITE_OMIT_WSD
      ប្រសិនបើ( zRight[0] ){
        ចំនួនគត់ res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        ប្រសិនបើ( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          goto pragma_out;
        }
      }
      ប្រសិនបើ( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      ប្រសិនបើ( zRight[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", zRight);
      }បើមិនដូច្នេះ{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    bha_nga;
  }

#ប្រសិនបើ SQLITE_OS_WIN
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
  pak_sa PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ប្រសិនបើ( !zRight ){
      returnSingleText(v, sqlite3_data_directory);
    }បើមិនដូច្នេះ{
#ifndef SQLITE_OMIT_WSD
      ប្រសិនបើ( zRight[0] ){
        ចំនួនគត់ res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        ប្រសិនបើ( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          goto pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      ប្រសិនបើ( zRight[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", zRight);
      }បើមិនដូច្នេះ{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    bha_nga;
  }
#endif

#ប្រសិនបើ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  pak_sa PragTyp_LOCK_PROXY_FILE: {
    ប្រសិនបើ( !zRight ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      var_na *proxy_file_path = NULL;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }បើមិនដូច្នេះ{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      ចំនួនគត់ res;
      ប្រសិនបើ( zRight[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     zRight);
      } បើមិនដូច្នេះ {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     NULL);
      }
      ប្រសិនបើ( res!=SQLITE_OK ){
        sqlite3ErrorMsg(pParse, "failed to set lock proxy file");
        goto pragma_out;
      }
    }
    bha_nga;
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
  pak_sa PragTyp_SYNCHRONOUS: {
    ប្រសិនបើ( !zRight ){
      returnSingleInt(v, pDb->safety_level-1);
    }បើមិនដូច្នេះ{
      ប្រសិនបើ( !db->autoCommit ){
        sqlite3ErrorMsg(pParse,
            "Safety level may not be changed inside a transaction");
      }បើមិនដូច្នេះ ប្រសិនបើ( iDb!=1 ){
        ចំនួនគត់ iLevel;
        ប្រសិនបើ( sqlite3Isdigit(zRight[0]) ){
          iLevel = sqlite3Atoi(zRight);
        }បើមិនដូច្នេះ{
          iLevel = getSafetyLevel(zRight,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        ប្រសិនបើ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    bha_nga;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  pak_sa PragTyp_FLAG: {
    ប្រសិនបើ( zRight==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }បើមិនដូច្នេះ{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      ប្រសិនបើ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_ForeignKeys);
      }

      ប្រសិនបើ( sqlite3GetBoolean(zRight, 0) ){
        ប្រសិនបើ( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }បើមិនដូច្នេះ{
        db->flags &= ~mask;
        ប្រសិនបើ( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        ប្រសិនបើ( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(zRight, "reset")==0
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
    bha_nga;
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
  pak_sa PragTyp_TABLE_INFO: ប្រសិនបើ( zRight ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
    ប្រសិនបើ( pTab ){
      ចំនួនគត់ i, k;
      ចំនួនគត់ nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      សម្រាប់(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        ចំនួនគត់ isHidden = 0;
        const Expr *pColExpr;
        ប្រសិនបើ( pCol->colFlags & COLFLAG_NOINSERT ){
          ប្រសិនបើ( pPragma->iArg==0 ){
            nHidden++;
            continue;
          }
          ប្រសិនបើ( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }បើមិនដូច្នេះ ប្រសិនបើ( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }បើមិនដូច្នេះ{ assert( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        ប្រសិនបើ( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }បើមិនដូច្នេះ ប្រសិនបើ( pPk==0 ){
          k = 1;
        }បើមិនដូច្នេះ{
          សម្រាប់(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        pColExpr = sqlite3ColumnExpr(pTab,pCol);
        assert( pColExpr==0 || pColExpr->op==TK_SPAN || isHidden>=2 );
        assert( pColExpr==0 || !ExprHasProperty(pColExpr, EP_IntValue)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || pColExpr==0) ? 0 : pColExpr->u.zToken,
               k,
               isHidden);
      }
    }
  }
  bha_nga;

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
  pak_sa PragTyp_TABLE_LIST: {
    ចំនួនគត់ ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    សម្រាប់(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      ចំនួនគត់ initNCol;
      ប្រសិនបើ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) continue;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqliteHashCount(pHash);
      ខណៈពេល( initNCol-- ){
        សម្រាប់(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          ប្រសិនបើ( k==0 ){ initNCol = 0; bha_nga; }
          pTab = sqliteHashData(k);
          ប្រសិនបើ( pTab->nCol==0 ){
            var_na *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            ប្រសិនបើ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (void)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (void)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            ប្រសិនបើ( db->mallocFailed ){
              sqlite3ErrorMsg(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            bha_nga;
          }
        }
      }

      សម្រាប់(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        const var_na *zType;
        ប្រសិនបើ( zRight && sqlite3_stricmp(zRight, pTab->zName)!=0 ) continue;
        ប្រសិនបើ( IsView(pTab) ){
          zType = "view";
        }បើមិនដូច្នេះ ប្រសិនបើ( IsVirtual(pTab) ){
          zType = "virtual";
        }បើមិនដូច្នេះ ប្រសិនបើ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }បើមិនដូច្នេះ{
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
  bha_nga;

#ifdef SQLITE_DEBUG
  pak_sa PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    សម្រាប់(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      សម្រាប់(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  bha_nga;
#endif

  pak_sa PragTyp_INDEX_INFO: ប្រសិនបើ( zRight ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, zRight, zDb);
    ប្រសិនបើ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
      ប្រសិនបើ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3PrimaryKeyIndex(pTab);
      }
    }
    ប្រសិនបើ( pIdx ){
      ចំនួនគត់ iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      ចំនួនគត់ i;
      ចំនួនគត់ mx;
      ប្រសិនបើ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }បើមិនដូច្នេះ{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->nKeyCol;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      assert( pParse->nMem<=pPragma->nPragCName );
      សម្រាប់(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        ប្រសិនបើ( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->nKeyCol);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  bha_nga;

  pak_sa PragTyp_INDEX_LIST: ប្រសិនបើ( zRight ){
    Index *pIdx;
    Table *pTab;
    ចំនួនគត់ i;
    pTab = sqlite3FindTable(db, zRight, zDb);
    ប្រសិនបើ( pTab ){
      ចំនួនគត់ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      សម្រាប់(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        const var_na *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  bha_nga;

  pak_sa PragTyp_DATABASE_LIST: {
    ចំនួនគត់ i;
    pParse->nMem = 3;
    សម្រាប់(i=0; i<db->nDb; i++){
      ប្រសិនបើ( db->aDb[i].pBt==0 ) continue;
      assert( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  bha_nga;

  pak_sa PragTyp_COLLATION_LIST: {
    ចំនួនគត់ i = 0;
    HashElem *p;
    pParse->nMem = 2;
    សម្រាប់(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  bha_nga;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  pak_sa PragTyp_FUNCTION_LIST: {
    ចំនួនគត់ i;
    HashElem *j;
    FuncDef *p;
    ចំនួនគត់ showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    សម្រាប់(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      សម្រាប់(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        assert( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragmaFunclistLine(v, p, 1, showInternFunc);
      }
    }
    សម្រាប់(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      assert( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragmaFunclistLine(v, p, 0, showInternFunc);
    }
  }
  bha_nga;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  pak_sa PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    សម្រាប់(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  bha_nga;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  pak_sa PragTyp_PRAGMA_LIST: {
    ចំនួនគត់ i;
    សម្រាប់(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  bha_nga;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_KEY
  pak_sa PragTyp_FOREIGN_KEY_LIST: ប្រសិនបើ( zRight ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, zRight, zDb);
    ប្រសិនបើ( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      ប្រសិនបើ( pFK ){
        ចំនួនគត់ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        ចំនួនគត់ i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        ខណៈពេល(pFK){
          ចំនួនគត់ j;
          សម្រាប់(j=0; j<pFK->nCol; j++){
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
  bha_nga;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_KEY
#ifndef SQLITE_OMIT_TRIGGER
  pak_sa PragTyp_FOREIGN_KEY_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    ចំនួនគត់ i;                 /* Loop counter:  Foreign key number for pTab */
    ចំនួនគត់ j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    ចំនួនគត់ x;                 /* result variable */
    ចំនួនគត់ regResult;         /* 3 registers to hold a result row */
    ចំនួនគត់ regRow;            /* Registers to hold a row from pTab */
    ចំនួនគត់ addrTop;           /* Top of a loop checking foreign keys */
    ចំនួនគត់ addrOk;            /* Jump here if the key is OK */
    ចំនួនគត់ *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    ខណៈពេល( k ){
      ប្រសិនបើ( zRight ){
        pTab = sqlite3LocateTable(pParse, 0, zRight, zDb);
        k = 0;
      }បើមិនដូច្នេះ{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      ប្រសិនបើ( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) continue;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      assert( IsOrdinaryTable(pTab) );
      សម្រាប់(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        ប្រសិនបើ( pParent==0 ) continue;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        ប្រសិនបើ( x==0 ){
          ប្រសិនបើ( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }បើមិនដូច្នេះ{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
          }
        }បើមិនដូច្នេះ{
          k = 0;
          bha_nga;
        }
      }
      assert( pParse->nErr>0 || pFK==0 );
      ប្រសិនបើ( pFK ) bha_nga;
      ប្រសិនបើ( pParse->nTab<i ) pParse->nTab = i;
      addrTop = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      assert( IsOrdinaryTable(pTab) );
      សម្រាប់(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        ប្រសិនបើ( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          assert( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        សម្រាប់(j=0; j<pFK->nCol; j++){
          ចំនួនគត់ iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3ExprCodeGetColumnOfTable(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        ប្រសិនបើ( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }បើមិនដូច្នេះ ប្រសិនបើ( pParent ){
          ចំនួនគត់ jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          assert( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        ប្រសិនបើ( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }បើមិនដូច្នេះ{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addrTop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrTop);
    }
  }
  bha_nga;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  pak_sa PragTyp_CASE_SENSITIVE_LIKE: {
    ប្រសិនបើ( zRight ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(zRight, 0));
    }
  }
  bha_nga;
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
  pak_sa PragTyp_INTEGRITY_CHECK: {
    ចំនួនគត់ i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    ចំនួនគត់ isQuick = (sqlite3Tolower(zLeft[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    assert( iDb>=0 );
    assert( iDb==0 || pId2->z );
    ប្រសិនបើ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    ប្រសិនបើ( zRight ){
      ប្រសិនបើ( sqlite3GetInt32(pValue->z, &mxErr) ){
        ប្រសិនបើ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }បើមិនដូច្នេះ{
        pObjTab = sqlite3LocateTable(pParse, 0, zRight,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    សម្រាប់(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      ចំនួនគត់ *aRoot;      /* Array of root page numbers of all btrees */
      ចំនួនគត់ cnt = 0;     /* Number of entries in aRoot[] */

      ប្រសិនបើ( OMIT_TEMPDB && i==1 ) continue;
      ប្រសិនបើ( iDb>=0 && i!=iDb ) continue;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      assert( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      សម្រាប់(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        ប្រសិនបើ( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        ប្រសិនបើ( HasRowid(pTab) ) cnt++;
        សម្រាប់(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      ប្រសិនបើ( cnt==0 ) continue;
      ប្រសិនបើ( pObjTab ) cnt++;
      aRoot = sqlite3DbMallocRawNN(db, parimaa_na(ចំនួនគត់)*(cnt+1));
      ប្រសិនបើ( aRoot==0 ) bha_nga;
      cnt = 0;
      ប្រសិនបើ( pObjTab ) aRoot[++cnt] = 0;
      សម្រាប់(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        ប្រសិនបើ( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        ប្រសិនបើ( HasRowid(pTab) ) aRoot[++cnt] = pTab->tnum;
        សម្រាប់(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          aRoot[++cnt] = pIdx->tnum;
        }
      }
      aRoot[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (var_na*)aRoot,P4_INTARRAY);
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
      សម្រាប់(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        ចំនួនគត់ iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        ប្រសិនបើ( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        ប្រសិនបើ( HasRowid(pTab) ){
          iTab = cnt++;
        }បើមិនដូច្នេះ{
          iTab = cnt;
          សម្រាប់(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            ប្រសិនបើ( IsPrimaryKeyIndex(pIdx) ) bha_nga;
            iTab++;
          }
        }
        សម្រាប់(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          ប្រសិនបើ( pIdx->pPartIdxWhere==0 ){
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
      សម្រាប់(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        ចំនួនគត់ loopTop;
        ចំនួនគត់ iDataCur, iIdxCur;
        ចំនួនគត់ r1 = -1;
        ចំនួនគត់ bStrict;            /* True for a STRICT table */
        ចំនួនគត់ r2;                 /* Previous key for WITHOUT ROWID tables */
        ចំនួនគត់ mxCol;              /* Maximum non-virtual column number */

        ប្រសិនបើ( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        ប្រសិនបើ( !IsOrdinaryTable(pTab) ) continue;
        ប្រសិនបើ( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }បើមិនដូច្នេះ{
          pPk = sqlite3PrimaryKeyIndex(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->nKeyCol);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->nKeyCol-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        សម្រាប់(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        assert( pParse->nMem>=8+j );
        assert( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loopTop = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        assert( !IsVirtual(pTab) );
        ប្រសិនបើ( HasRowid(pTab) ){
          mxCol = -1;
          សម្រាប់(j=0; j<pTab->nCol; j++){
            ប្រសិនបើ( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          ប្រសិនបើ( mxCol==pTab->iPKey ) mxCol--;
        }បើមិនដូច្នេះ{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3PrimaryKeyIndex(pTab)->nColumn-1;
        }
        ប្រសិនបើ( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        ប្រសិនបើ( !isQuick ){
          ប្រសិនបើ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            ចំនួនគត់ a1;
            var_na *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->nKeyCol);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            សម្រាប់(j=0; j<pPk->nKeyCol; j++){
              sqlite3ExprCodeLoadIndexColumn(pParse, pPk, iDataCur, j, r2+j);
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
        សម្រាប់(j=0; j<pTab->nCol; j++){
          var_na *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          ចំនួនគត់ labelError;               /* Jump here to report an error */
          ចំនួនគត់ labelOk;                  /* Jump here if all looks ok */
          ចំនួនគត់ p1, p3, p4;               /* Operands to the OP_IsType opcode */
          ចំនួនគត់ doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          ប្រសិនបើ( j==pTab->iPKey ) continue;
          ប្រសិនបើ( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }បើមិនដូច្នេះ{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          ប្រសិនបើ( pCol->notNull==0 && !doTypeCheck ) continue;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          ប្រសិនបើ( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }បើមិនដូច្នេះ{
            ប្រសិនបើ( pCol->iDflt ){
              sqlite3_value *pDfltValue = 0;
              sqlite3ValueFromExpr(db, sqlite3ColumnExprAuth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &pDfltValue);
              ប្រសិនបើ( pDfltValue ){
                p4 = sqlite3_value_type(pDfltValue);
                sqlite3ValueFree(pDfltValue);
              }
            }
            p1 = iDataCur;
            ប្រសិនបើ( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), j);
            }បើមិនដូច្នេះ{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          ប្រសិនបើ( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            ចំនួនគត់ jmp3;
            ចំនួនគត់ jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            ប្រសិនបើ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }បើមិនដូច្នេះ{
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
            ប្រសិនបើ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }បើមិនដូច្នេះ{
              /* VDBE byte code will fall thru */
            }
          }
          ប្រសិនបើ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            static unsigned var_na aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            assert( pCol->eCType>=1 && pCol->eCType<=parimaa_na(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }បើមិនដូច្នេះ ប្រសិនបើ( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }បើមិនដូច្នេះ ប្រសិនបើ( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            ប្រសិនបើ( p1>=0 ){
              sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
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
        ប្រសិនបើ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ExprList *pCheck = sqlite3ExprListDup(db, pTab->pCheck, 0);
          ប្រសិនបើ( db->mallocFailed==0 ){
            ចំនួនគត់ addrCkFault = sqlite3VdbeMakeLabel(pParse);
            ចំនួនគត់ addrCkOk = sqlite3VdbeMakeLabel(pParse);
            var_na *zErr;
            ចំនួនគត់ k;
            pParse->iSelfTab = iDataCur + 1;
            សម្រាប់(k=pCheck->nExpr-1; k>0; k--){
              sqlite3ExprIfFalse(pParse, pCheck->a[k].pExpr, addrCkFault, 0);
            }
            sqlite3ExprIfTrue(pParse, pCheck->a[0].pExpr, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3ExprListDelete(db, pCheck);
        }
        ប្រសិនបើ( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          សម្រាប់(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            ចំនួនគត់ jmp2, jmp3, jmp4, jmp5, label6;
            ចំនួនគត់ kk;
            ចំនួនគត់ ckUniq = sqlite3VdbeMakeLabel(pParse);
            ប្រសិនបើ( pPk==pIdx ) continue;
            r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFindKey, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (const var_na*)pIdx, P4_INDEX);
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
            ប្រសិនបើ( HasRowid(pTab) ){
              ចំនួនគត់ jmp7;
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
            សម្រាប់(kk=0; kk<pIdx->nKeyCol; kk++){
              ប្រសិនបើ( pIdx->azColl[kk]==sqlite3StrBINARY ) continue;
              ប្រសិនបើ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            ប្រសិនបើ( label6 ){
              ចំនួនគត់ jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            ប្រសិនបើ( IsUniqueIndex(pIdx) ){
              ចំនួនគត់ uniqOk = sqlite3VdbeMakeLabel(pParse);
              ចំនួនគត់ jmp6;
              សម្រាប់(kk=0; kk<pIdx->nKeyCol; kk++){
                ចំនួនគត់ iCol = pIdx->aiColumn[kk];
                assert( iCol!=XN_ROWID && iCol<pTab->nCol );
                ប្រសិនបើ( iCol>=0 && pTab->aCol[iCol].notNull ) continue;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->nKeyCol); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loopTop); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loopTop-1);
        ប្រសិនបើ( pPk ){
          assert( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->nKeyCol);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      សម្រាប់(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        ចំនួនគត់ a1;
        ប្រសិនបើ( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        ប្រសិនបើ( IsOrdinaryTable(pTab) ) continue;
        ប្រសិនបើ( !IsVirtual(pTab) ) continue;
        ប្រសិនបើ( pTab->nCol<=0 ){
          const var_na *zMod = pTab->u.vtab.azArg[0];
          ប្រសិនបើ( sqlite3HashFind(&db->aModule, zMod)==0 ) continue;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        ប្រសិនបើ( pTab->u.vtab.p==0 ) continue;
        pVTab = pTab->u.vtab.p->pVtab;
        ប្រសិនបើ( NEVER(pVTab==0) ) continue;
        ប្រសិនបើ( NEVER(pVTab->pModule==0) ) continue;
        ប្រសិនបើ( pVTab->pModule->iVersion<4 ) continue;
        ប្រសិនបើ( pVTab->pModule->xIntegrity==0 ) continue;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        continue;
      }
#endif
    }
    {
      static const ចំនួនគត់ iLn = VDBE_OFFSET_LINENO(2);
      static const VdbeOpList endCode[] = {
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
      ប្រសិនបើ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (var_na*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  bha_nga;
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
  pak_sa PragTyp_ENCODING: {
    static const sa_nracanaa EncName {
      var_na *zName;
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
    const sa_nracanaa EncName *pEnc;
    ប្រសិនបើ( !zRight ){    /* "PRAGMA encoding" */
      ប្រសិនបើ( sqlite3ReadSchema(pParse) ) goto pragma_out;
      assert( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      assert( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      assert( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }បើមិនដូច្នេះ{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      ប្រសិនបើ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        សម្រាប់(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          ប្រសិនបើ( 0==sqlite3StrICmp(zRight, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            bha_nga;
          }
        }
        ប្រសិនបើ( !pEnc->zName ){
          sqlite3ErrorMsg(pParse, "unsupported encoding: %s", zRight);
        }
      }
    }
  }
  bha_nga;
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
  pak_sa PragTyp_HEADER_VALUE: {
    ចំនួនគត់ iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    ប្រសិនបើ( zRight && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      static const VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      ប្រសិនបើ( ONLY_IF_REALLOC_STRESS(aOp==0) ) bha_nga;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(zRight);
      aOp[1].p5 = 1;
      ប្រសិនបើ( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }បើមិនដូច្នេះ{
      /* Read the specified cookie value */
      static const VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      ប្រសិនបើ( ONLY_IF_REALLOC_STRESS(aOp==0) ) bha_nga;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  bha_nga;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  pak_sa PragTyp_COMPILE_OPTIONS: {
    ចំនួនគត់ i = 0;
    const var_na *zOpt;
    pParse->nMem = 1;
    ខណៈពេល( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  bha_nga;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  pak_sa PragTyp_WAL_CHECKPOINT: {
    ចំនួនគត់ iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    ចំនួនគត់ eMode = SQLITE_CHECKPOINT_PASSIVE;
    ប្រសិនបើ( zRight ){
      ប្រសិនបើ( sqlite3StrICmp(zRight, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3StrICmp(zRight, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3StrICmp(zRight, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3StrICmp(zRight, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  bha_nga;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  pak_sa PragTyp_WAL_AUTOCHECKPOINT: {
    ប្រសិនបើ( zRight ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(zRight));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  bha_nga;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  pak_sa PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    bha_nga;
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
  pak_sa PragTyp_OPTIMIZE: {
    ចំនួនគត់ iDbLast;           /* Loop termination point for the schema loop */
    ចំនួនគត់ iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    var_na *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    ចំនួនគត់ nLimit;            /* Analysis limit to use */
    ចំនួនគត់ nCheck = 0;        /* Number of tables to be optimized */
    ចំនួនគត់ nBtree = 0;        /* Number of btrees to scan */
    ចំនួនគត់ nIndex;            /* Number of indexes on the current table */

    ប្រសិនបើ( zRight ){
      opMask = (u32)sqlite3Atoi(zRight);
      ប្រសិនបើ( (opMask & 0x02)==0 ) bha_nga;
    }បើមិនដូច្នេះ{
      opMask = 0xfffe;
    }
    ប្រសិនបើ( (opMask & 0x10)==0 ){
      nLimit = 0;
    }បើមិនដូច្នេះ ប្រសិនបើ( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }បើមិនដូច្នេះ{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    សម្រាប់(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      ប្រសិនបើ( iDb==1 ) continue;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      សម្រាប់(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        ប្រសិនបើ( !IsOrdinaryTable(pTab) ) continue;

        /* Do not scan system tables */
        ប្រសិនបើ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) continue;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        សម្រាប់(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          ប្រសិនបើ( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        ប្រសិនបើ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }បើមិនដូច្នេះ ប្រសិនបើ( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }បើមិនដូច្នេះ ប្រសិនបើ( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }បើមិនដូច្នេះ{
          /* Otherwise, we can skip this table */
          continue;
        }

        nCheck++;
        ប្រសិនបើ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        ប្រសិនបើ( szThreshold>=0 ){
          const LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }បើមិនដូច្នេះ{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        ប្រសិនបើ( opMask & 0x01 ){
          ចំនួនគត់ r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }បើមិនដូច្នេះ{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    ប្រសិនបើ( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      ចំនួនគត់ iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      ប្រសិនបើ( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      សម្រាប់(iAddr=0; iAddr<iEnd; iAddr++){
        ប្រសិនបើ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    bha_nga;
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
  /*case PragTyp_BUSY_TIMEOUT*/ default: {
    ចំនួនគត់ n = 0;
    assert( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    ប្រសិនបើ( zRight && sqlite3GetInt32(zRight,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    bha_nga;
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
  pak_sa PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    ប្រសិនបើ( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    bha_nga;
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
  pak_sa PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    ប្រសិនបើ( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      ប្រសិនបើ( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    bha_nga;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  pak_sa PragTyp_THREADS: {
    sqlite3_int64 N;
    ប្រសិនបើ( zRight
     && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (ចំនួនគត់)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    bha_nga;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  pak_sa PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    ប្រសិនបើ( zRight
     && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (ចំនួនគត់)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    bha_nga;
  }

#ប្រសិនបើ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  pak_sa PragTyp_LOCK_STATUS: {
    static const var_na *const azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    ចំនួនគត់ i;
    pParse->nMem = 2;
    សម្រាប់(i=0; i<db->nDb; i++){
      Btree *pBt;
      const var_na *zState = "unknown";
      ចំនួនគត់ j;
      ប្រសិនបើ( db->aDb[i].zDbSName==0 ) continue;
      pBt = db->aDb[i].pBt;
      ប្រសិនបើ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    bha_nga;
  }
#endif

#ប្រសិនបើ defined(SQLITE_ENABLE_CEROD)
  pak_sa PragTyp_ACTIVATE_EXTENSIONS: ប្រសិនបើ( zRight ){
    ប្រសិនបើ( sqlite3StrNICmp(zRight, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&zRight[6]);
    }
  }
  bha_nga;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  ប្រសិនបើ( (pPragma->mPragFlg & PragFlg_NoColumns1) && zRight ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, zLeft);
  sqlite3DbFree(db, zRight);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
typedef sa_nracanaa PragmaVtab PragmaVtab;
typedef sa_nracanaa PragmaVtabCursor PragmaVtabCursor;
sa_nracanaa PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  const PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
sa_nracanaa PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  var_na *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
static ចំនួនគត់ pragmaVtabConnect(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  const PragmaName *pPragma = (const PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  ចំនួនគត់ rc;
  ចំនួនគត់ i, j;
  var_na cSep = '(';
  StrAccum acc;
  var_na zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, parimaa_na(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  សម្រាប់(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  ប្រសិនបើ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  ប្រសិនបើ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  ប្រសិនបើ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  assert( strlen(zBuf) < parimaa_na(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  ប្រសិនបើ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(parimaa_na(PragmaVtab));
    ប្រសិនបើ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }បើមិនដូច្នេះ{
      memset(pTab, 0, parimaa_na(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }បើមិនដូច្នេះ{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ត្រឡប់ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
static ចំនួនគត់ pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  ត្រឡប់ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
static ចំនួនគត់ pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  const sa_nracanaa sqlite3_index_constraint *pConstraint;
  ចំនួនគត់ i, j;
  ចំនួនគត់ seen[2];

  pIdxInfo->estimatedCost = (dviguu_na)1;
  ប្រសិនបើ( pTab->nHidden==0 ){ ត្រឡប់ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  សម្រាប់(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ប្រសិនបើ( pConstraint->iColumn < pTab->iHidden ) continue;
    ប្រសិនបើ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ប្រសិនបើ( pConstraint->usable==0 ) ត្រឡប់ SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    assert( j < 2 );
    seen[j] = i+1;
  }
  ប្រសិនបើ( seen[0]==0 ){
    pIdxInfo->estimatedCost = (dviguu_na)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    ត្រឡប់ SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (dviguu_na)20;
  pIdxInfo->estimatedRows = 20;
  ប្រសិនបើ( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  ត្រឡប់ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
static ចំនួនគត់ pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(parimaa_na(*pCsr));
  ប្រសិនបើ( pCsr==0 ) ត្រឡប់ SQLITE_NOMEM;
  memset(pCsr, 0, parimaa_na(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  ត្រឡប់ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
static void pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  ចំនួនគត់ i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  សម្រាប់(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
static ចំនួនគត់ pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  ត្រឡប់ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
static ចំនួនគត់ pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ចំនួនគត់ rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  assert( pCsr->pPragma );
  ប្រសិនបើ( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  ត្រឡប់ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
static ចំនួនគត់ pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  ចំនួនគត់ idxNum, const var_na *idxStr,
  ចំនួនគត់ argc, sqlite3_value **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ចំនួនគត់ rc;
  ចំនួនគត់ i, j;
  StrAccum acc;
  var_na *zSql;

  UNUSED_PARAMETER(idxNum);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  សម្រាប់(i=0; i<argc; i++, j++){
    const var_na *zText = (const var_na*)sqlite3_value_text(argv[i]);
    assert( j<ArraySize(pCsr->azArg) );
    assert( pCsr->azArg[j]==0 );
    ប្រសិនបើ( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      ប្រសិនបើ( pCsr->azArg[j]==0 ){
        ត្រឡប់ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  ប្រសិនបើ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  ប្រសិនបើ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  ប្រសិនបើ( zSql==0 ) ត្រឡប់ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    pTab->base.zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    ត្រឡប់ rc;
  }
  ត្រឡប់ pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
static ចំនួនគត់ pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ត្រឡប់ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
static ចំនួនគត់ pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  ចំនួនគត់ i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ប្រសិនបើ( i<pTab->iHidden ){
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pPragma, i));
  }បើមិនដូច្នេះ{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  ត្រឡប់ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
static ចំនួនគត់ pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  ត្រឡប់ SQLITE_OK;
}

/* The pragma virtual table object */
static const sqlite3_module pragmaVtabModule = {
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, const var_na *zName){
  const PragmaName *pName;
  assert( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  ប្រសិនបើ( pName==0 ) ត្រឡប់ 0;
  ប្រសិនបើ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) ត្រឡប់ 0;
  assert( sqlite3HashFind(&db->aModule, zName)==0 );
  ត្រឡប់ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (void*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
