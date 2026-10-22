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

#如果 !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  如果 defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  否则
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
static u8 getSafetyLevel(const char *z, 整数 omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  static const char zText[] = "onoffalseyestruextrafull";
  static const u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  static const u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  static const u8 iValue[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  整数 i, n;
  如果( sqlite3Isdigit(*z) ){
    返回 sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  为(i=0; i<ArraySize(iLength); i++){
    如果( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || iValue[i]<=1)
    ){
      返回 iValue[i];
    }
  }
  返回 dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(const char *z, u8 dflt){
  返回 getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#如果 !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
static 整数 getLockingMode(const char *z){
  如果( z ){
    如果( 0==sqlite3StrICmp(z, "exclusive") ) 返回 PAGER_LOCKINGMODE_EXCLUSIVE;
    如果( 0==sqlite3StrICmp(z, "normal") ) 返回 PAGER_LOCKINGMODE_NORMAL;
  }
  返回 PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
static 整数 getAutoVacuum(const char *z){
  整数 i;
  如果( 0==sqlite3StrICmp(z, "none") ) 返回 BTREE_AUTOVACUUM_NONE;
  如果( 0==sqlite3StrICmp(z, "full") ) 返回 BTREE_AUTOVACUUM_FULL;
  如果( 0==sqlite3StrICmp(z, "incremental") ) 返回 BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  返回 (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
static 整数 getTempStore(const char *z){
  如果( z[0]>='0' && z[0]<='2' ){
    返回 z[0] - '0';
  }否则 如果( sqlite3StrICmp(z, "file")==0 ){
    返回 1;
  }否则 如果( sqlite3StrICmp(z, "memory")==0 ){
    返回 2;
  }否则{
    返回 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
static 整数 invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  如果( db->aDb[1].pBt!=0 ){
    如果( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3ErrorMsg(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      返回 SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  返回 SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
static 整数 changeTempStorage(Parse *pParse, const char *zStorageType){
  整数 ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  如果( db->temp_store==ts ) 返回 SQLITE_OK;
  如果( invalidateTempStorage( pParse ) != SQLITE_OK ){
    返回 SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  返回 SQLITE_OK;
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
  如果( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }否则{
    整数 i, j;
    为(i=0, j=pPragma->iPragCName; i<n; i++, j++){
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
  const char *zValue      /* Value to be returned */
){
  如果( zValue ){
    sqlite3VdbeLoadString(v, 1, (const char*)zValue);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
static void setAllPagerFlags(sqlite3 *db){
  如果( db->autoCommit ){
    Db *pDb = db->aDb;
    整数 n = db->nDb;
    assert( SQLITE_FullFSync==PAGER_FULLFSYNC );
    assert( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    assert( SQLITE_CacheSpill==PAGER_CACHESPILL );
    assert( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    assert( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    当( (n--) > 0 ){
      如果( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#否则
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_KEY
static const char *actionName(u8 action){
  const char *zName;
  switch( action ){
    case OE_SetNull:  zName = "SET NULL";        break;
    case OE_SetDflt:  zName = "SET DEFAULT";     break;
    case OE_Cascade:  zName = "CASCADE";         break;
    case OE_Restrict: zName = "RESTRICT";        break;
    default:          zName = "NO ACTION"; 
                      assert( action==OE_None ); break;
  }
  返回 zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
const char *sqlite3JournalModename(整数 eMode){
  static char * const azModeName[] = {
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

  如果( eMode==ArraySize(azModeName) ) 返回 0;
  返回 azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
static const PragmaName *pragmaLocate(const char *zName){
  整数 upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  当( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    如果( rc==0 ) break;
    如果( rc<0 ){
      upr = mid - 1;
    }否则{
      lwr = mid + 1;
    }
  }
  返回 lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
static void pragmaFunclistLine(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  整数 isBuiltin,         /* True if this is a built-in function */
  整数 showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  如果( showInternFuncs ) mask = 0xffffffff;
  为(; p; p=p->pNext){
    const char *zType;
    static const char *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    assert( SQLITE_FUNC_ENCMASK==0x3 );
    assert( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    assert( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    assert( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    如果( p->xSFunc==0 ) continue;
    如果( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      continue;
    }   
    如果( p->xValue!=0 ){
      zType = "w";
    }否则 如果( p->xFinalize!=0 ){
      zType = "a";
    }否则{
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
static 整数 integrityCheckResultRow(Vdbe *v){
  整数 addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  返回 addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
static 整数 tableSkipIntegrityCheck(const Table *pTab, const Table *pObjTab){
  如果( pObjTab ){
    返回 pTab!=pObjTab;
  }否则{
    返回 (pTab->tabFlags & TF_Imposter)!=0;
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
  整数 minusFlag       /* True if a '-' sign preceded <value> */
){
  char *zLeft = 0;       /* Nul-terminated UTF-8 string <id> */
  char *zRight = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  const char *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  char *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  整数 iDb;               /* Database index for <database> */
  整数 rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  const PragmaName *pPragma;   /* The pragma */

  如果( v==0 ) 返回;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  如果( iDb<0 ) 返回;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  如果( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    返回;
  }

  zLeft = sqlite3NameFromToken(db, pId);
  如果( !zLeft ) 返回;
  如果( minusFlag ){
    zRight = sqlite3MPrintf(db, "-%T", pValue);
  }否则{
    zRight = sqlite3NameFromToken(db, pValue);
  }

  assert( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  如果( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, zLeft, zRight, zDb) ){
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
  如果( rc==SQLITE_OK ){
    sqlite3VdbeSetNumCols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    goto pragma_out;
  }
  如果( rc!=SQLITE_NOTFOUND ){
    如果( aFcntl[0] ){
      sqlite3ErrorMsg(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    goto pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(zLeft);
  如果( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    goto pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  如果( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    如果( sqlite3ReadSchema(pParse) ) goto pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  如果( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || zRight==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  switch( pPragma->ePragTyp ){
 
#如果 !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  case PragTyp_DEFAULT_CACHE_SIZE: {
    static const 整数 iLn = VDBE_OFFSET_LINENO(2);
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
    如果( !zRight ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      如果( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }否则{
      整数 size = sqlite3AbsInt32(sqlite3Atoi(zRight));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    break;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#如果 !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  case PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    如果( !zRight ){
      整数 size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }否则{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(zRight);
      如果( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    break;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  case PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    整数 b = -1;
    assert( pBt!=0 );
    如果( zRight ){
      如果( sqlite3_stricmp(zRight, "fast")==0 ){
        b = 2;
      }否则{
        b = sqlite3GetBoolean(zRight, 0);
      }
    }
    如果( pId2->n==0 && b>=0 ){
      整数 ii;
      为(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    break;
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
  case PragTyp_PAGE_COUNT: {
    整数 iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    如果( sqlite3Tolower(zLeft[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }否则{
      如果( zRight && sqlite3DecOrHexToI64(zRight,&x)==0 ){
        如果( x<0 ) x = 0;
        否则 如果( x>0xfffffffe ) x = 0xfffffffe;
      }否则{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (整数)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    break;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  case PragTyp_LOCKING_MODE: {
    const char *zRet = "normal";
    整数 eMode = getLockingMode(zRight);

    如果( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }否则{
      Pager *pPager;
      如果( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        整数 ii;
        assert(pDb==&db->aDb[0]);
        为(ii=2; ii<db->nDb; ii++){
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
    如果( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    break;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  case PragTyp_JOURNAL_MODE: {
    整数 eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    整数 ii;           /* Loop counter */

    如果( zRight==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }否则{
      const char *zMode;
      整数 n = sqlite3Strlen30(zRight);
      为(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        如果( sqlite3StrNICmp(zRight, zMode, n)==0 ) break;
      }
      如果( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      如果( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    如果( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    为(ii=db->nDb-1; ii>=0; ii--){
      如果( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    break;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  case PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    如果( zRight ){
      sqlite3DecOrHexToI64(zRight, &iLimit);
      如果( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    break;
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
  case PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    如果( !zRight ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }否则{
      整数 eAuto = getAutoVacuum(zRight);
      assert( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      如果( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        static const 整数 iLn = VDBE_OFFSET_LINENO(2);
        static const VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_ROOT_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        整数 iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        如果( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    break;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  case PragTyp_INCREMENTAL_VACUUM: {
    整数 iLimit = 0, addr;
    如果( zRight==0 || !sqlite3GetInt32(zRight, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    break;
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
  case PragTyp_CACHE_SIZE: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    如果( !zRight ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }否则{
      整数 size = sqlite3Atoi(zRight);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    break;
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
  case PragTyp_CACHE_SPILL: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    如果( !zRight ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }否则{
      整数 size = 1;
      如果( sqlite3GetInt32(zRight, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      如果( sqlite3GetBoolean(zRight, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }否则{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    break;
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
  case PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#如果 SQLITE_MAX_MMAP_SIZE>0
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    如果( zRight ){
      整数 ii;
      sqlite3DecOrHexToI64(zRight, &sz);
      如果( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      如果( pId2->n==0 ) db->szMmap = sz;
      为(ii=db->nDb-1; ii>=0; ii--){
        如果( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#否则
    sz = 0;
    rc = SQLITE_OK;
#endif
    如果( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }否则 如果( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    break;
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
  case PragTyp_TEMP_STORE: {
    如果( !zRight ){
      returnSingleInt(v, db->temp_store);
    }否则{
      changeTempStorage(pParse, zRight);
    }
    break;
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
  case PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    如果( !zRight ){
      returnSingleText(v, sqlite3_temp_directory);
    }否则{
#ifndef SQLITE_OMIT_WSD
      如果( zRight[0] ){
        整数 res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        如果( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          goto pragma_out;
        }
      }
      如果( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      如果( zRight[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", zRight);
      }否则{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    break;
  }

#如果 SQLITE_OS_WIN
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
  case PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    如果( !zRight ){
      returnSingleText(v, sqlite3_data_directory);
    }否则{
#ifndef SQLITE_OMIT_WSD
      如果( zRight[0] ){
        整数 res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        如果( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          goto pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      如果( zRight[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", zRight);
      }否则{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    break;
  }
#endif

#如果 SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  case PragTyp_LOCK_PROXY_FILE: {
    如果( !zRight ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      char *proxy_file_path = NULL;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }否则{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      整数 res;
      如果( zRight[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     zRight);
      } 否则 {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     NULL);
      }
      如果( res!=SQLITE_OK ){
        sqlite3ErrorMsg(pParse, "failed to set lock proxy file");
        goto pragma_out;
      }
    }
    break;
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
  case PragTyp_SYNCHRONOUS: {
    如果( !zRight ){
      returnSingleInt(v, pDb->safety_level-1);
    }否则{
      如果( !db->autoCommit ){
        sqlite3ErrorMsg(pParse,
            "Safety level may not be changed inside a transaction");
      }否则 如果( iDb!=1 ){
        整数 iLevel;
        如果( sqlite3Isdigit(zRight[0]) ){
          iLevel = sqlite3Atoi(zRight);
        }否则{
          iLevel = getSafetyLevel(zRight,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        如果( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    break;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  case PragTyp_FLAG: {
    如果( zRight==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }否则{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      如果( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_ForeignKeys);
      }

      如果( sqlite3GetBoolean(zRight, 0) ){
        如果( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }否则{
        db->flags &= ~mask;
        如果( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        如果( (mask & SQLITE_WriteSchema)!=0
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
    break;
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
  case PragTyp_TABLE_INFO: 如果( zRight ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
    如果( pTab ){
      整数 i, k;
      整数 nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      为(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        整数 isHidden = 0;
        const Expr *pColExpr;
        如果( pCol->colFlags & COLFLAG_NOINSERT ){
          如果( pPragma->iArg==0 ){
            nHidden++;
            continue;
          }
          如果( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }否则 如果( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }否则{ assert( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        如果( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }否则 如果( pPk==0 ){
          k = 1;
        }否则{
          为(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
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
  break;

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
  case PragTyp_TABLE_LIST: {
    整数 ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    为(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      整数 initNCol;
      如果( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) continue;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqliteHashCount(pHash);
      当( initNCol-- ){
        为(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          如果( k==0 ){ initNCol = 0; break; }
          pTab = sqliteHashData(k);
          如果( pTab->nCol==0 ){
            char *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            如果( zSql ){
              sqlite3_stmt *pDummy = 0;
              (void)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (void)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            如果( db->mallocFailed ){
              sqlite3ErrorMsg(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            break;
          }
        }
      }

      为(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        const char *zType;
        如果( zRight && sqlite3_stricmp(zRight, pTab->zName)!=0 ) continue;
        如果( IsView(pTab) ){
          zType = "view";
        }否则 如果( IsVirtual(pTab) ){
          zType = "virtual";
        }否则 如果( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }否则{
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
  break;

#ifdef SQLITE_DEBUG
  case PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    为(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      为(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  break;
#endif

  case PragTyp_INDEX_INFO: 如果( zRight ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, zRight, zDb);
    如果( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
      如果( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3PrimaryKeyIndex(pTab);
      }
    }
    如果( pIdx ){
      整数 iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      整数 i;
      整数 mx;
      如果( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }否则{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->nKeyCol;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      assert( pParse->nMem<=pPragma->nPragCName );
      为(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        如果( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->nKeyCol);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  break;

  case PragTyp_INDEX_LIST: 如果( zRight ){
    Index *pIdx;
    Table *pTab;
    整数 i;
    pTab = sqlite3FindTable(db, zRight, zDb);
    如果( pTab ){
      整数 iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      为(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        const char *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  break;

  case PragTyp_DATABASE_LIST: {
    整数 i;
    pParse->nMem = 3;
    为(i=0; i<db->nDb; i++){
      如果( db->aDb[i].pBt==0 ) continue;
      assert( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  break;

  case PragTyp_COLLATION_LIST: {
    整数 i = 0;
    HashElem *p;
    pParse->nMem = 2;
    为(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  break;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  case PragTyp_FUNCTION_LIST: {
    整数 i;
    HashElem *j;
    FuncDef *p;
    整数 showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    为(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      为(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        assert( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragmaFunclistLine(v, p, 1, showInternFunc);
      }
    }
    为(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      assert( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragmaFunclistLine(v, p, 0, showInternFunc);
    }
  }
  break;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  case PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    为(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  break;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  case PragTyp_PRAGMA_LIST: {
    整数 i;
    为(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  break;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_KEY
  case PragTyp_FOREIGN_KEY_LIST: 如果( zRight ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, zRight, zDb);
    如果( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      如果( pFK ){
        整数 iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        整数 i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        当(pFK){
          整数 j;
          为(j=0; j<pFK->nCol; j++){
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
  break;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_KEY
#ifndef SQLITE_OMIT_TRIGGER
  case PragTyp_FOREIGN_KEY_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    整数 i;                 /* Loop counter:  Foreign key number for pTab */
    整数 j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    整数 x;                 /* result variable */
    整数 regResult;         /* 3 registers to hold a result row */
    整数 regRow;            /* Registers to hold a row from pTab */
    整数 addrTop;           /* Top of a loop checking foreign keys */
    整数 addrOk;            /* Jump here if the key is OK */
    整数 *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    当( k ){
      如果( zRight ){
        pTab = sqlite3LocateTable(pParse, 0, zRight, zDb);
        k = 0;
      }否则{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      如果( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) continue;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      assert( IsOrdinaryTable(pTab) );
      为(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        如果( pParent==0 ) continue;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        如果( x==0 ){
          如果( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }否则{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
          }
        }否则{
          k = 0;
          break;
        }
      }
      assert( pParse->nErr>0 || pFK==0 );
      如果( pFK ) break;
      如果( pParse->nTab<i ) pParse->nTab = i;
      addrTop = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      assert( IsOrdinaryTable(pTab) );
      为(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        如果( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          assert( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        为(j=0; j<pFK->nCol; j++){
          整数 iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3ExprCodeGetColumnOfTable(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        如果( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }否则 如果( pParent ){
          整数 jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          assert( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        如果( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }否则{
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
  break;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  case PragTyp_CASE_SENSITIVE_LIKE: {
    如果( zRight ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(zRight, 0));
    }
  }
  break;
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
  case PragTyp_INTEGRITY_CHECK: {
    整数 i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    整数 isQuick = (sqlite3Tolower(zLeft[0])=='q');

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
    如果( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    如果( zRight ){
      如果( sqlite3GetInt32(pValue->z, &mxErr) ){
        如果( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }否则{
        pObjTab = sqlite3LocateTable(pParse, 0, zRight,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    为(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      整数 *aRoot;      /* Array of root page numbers of all btrees */
      整数 cnt = 0;     /* Number of entries in aRoot[] */

      如果( OMIT_TEMPDB && i==1 ) continue;
      如果( iDb>=0 && i!=iDb ) continue;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      assert( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      为(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        如果( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        如果( HasRowid(pTab) ) cnt++;
        为(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      如果( cnt==0 ) continue;
      如果( pObjTab ) cnt++;
      aRoot = sqlite3DbMallocRawNN(db, sizeof(整数)*(cnt+1));
      如果( aRoot==0 ) break;
      cnt = 0;
      如果( pObjTab ) aRoot[++cnt] = 0;
      为(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        如果( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        如果( HasRowid(pTab) ) aRoot[++cnt] = pTab->tnum;
        为(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          aRoot[++cnt] = pIdx->tnum;
        }
      }
      aRoot[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (char*)aRoot,P4_INTARRAY);
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
      为(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        整数 iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        如果( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        如果( HasRowid(pTab) ){
          iTab = cnt++;
        }否则{
          iTab = cnt;
          为(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            如果( IsPrimaryKeyIndex(pIdx) ) break;
            iTab++;
          }
        }
        为(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          如果( pIdx->pPartIdxWhere==0 ){
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
      为(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        整数 loopTop;
        整数 iDataCur, iIdxCur;
        整数 r1 = -1;
        整数 bStrict;            /* True for a STRICT table */
        整数 r2;                 /* Previous key for WITHOUT ROWID tables */
        整数 mxCol;              /* Maximum non-virtual column number */

        如果( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        如果( !IsOrdinaryTable(pTab) ) continue;
        如果( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }否则{
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
        为(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
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
        如果( HasRowid(pTab) ){
          mxCol = -1;
          为(j=0; j<pTab->nCol; j++){
            如果( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          如果( mxCol==pTab->iPKey ) mxCol--;
        }否则{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3PrimaryKeyIndex(pTab)->nColumn-1;
        }
        如果( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        如果( !isQuick ){
          如果( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            整数 a1;
            char *zErr;
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
            为(j=0; j<pPk->nKeyCol; j++){
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
        为(j=0; j<pTab->nCol; j++){
          char *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          整数 labelError;               /* Jump here to report an error */
          整数 labelOk;                  /* Jump here if all looks ok */
          整数 p1, p3, p4;               /* Operands to the OP_IsType opcode */
          整数 doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          如果( j==pTab->iPKey ) continue;
          如果( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }否则{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          如果( pCol->notNull==0 && !doTypeCheck ) continue;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          如果( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }否则{
            如果( pCol->iDflt ){
              sqlite3_value *pDfltValue = 0;
              sqlite3ValueFromExpr(db, sqlite3ColumnExprAuth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &pDfltValue);
              如果( pDfltValue ){
                p4 = sqlite3_value_type(pDfltValue);
                sqlite3ValueFree(pDfltValue);
              }
            }
            p1 = iDataCur;
            如果( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), j);
            }否则{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          如果( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            整数 jmp3;
            整数 jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            如果( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }否则{
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
            如果( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }否则{
              /* VDBE byte code will fall thru */
            }
          }
          如果( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            static unsigned char aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            assert( pCol->eCType>=1 && pCol->eCType<=sizeof(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }否则 如果( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }否则 如果( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            如果( p1>=0 ){
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
        如果( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ExprList *pCheck = sqlite3ExprListDup(db, pTab->pCheck, 0);
          如果( db->mallocFailed==0 ){
            整数 addrCkFault = sqlite3VdbeMakeLabel(pParse);
            整数 addrCkOk = sqlite3VdbeMakeLabel(pParse);
            char *zErr;
            整数 k;
            pParse->iSelfTab = iDataCur + 1;
            为(k=pCheck->nExpr-1; k>0; k--){
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
        如果( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          为(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            整数 jmp2, jmp3, jmp4, jmp5, label6;
            整数 kk;
            整数 ckUniq = sqlite3VdbeMakeLabel(pParse);
            如果( pPk==pIdx ) continue;
            r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFindKey, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (const char*)pIdx, P4_INDEX);
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
            如果( HasRowid(pTab) ){
              整数 jmp7;
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
            为(kk=0; kk<pIdx->nKeyCol; kk++){
              如果( pIdx->azColl[kk]==sqlite3StrBINARY ) continue;
              如果( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            如果( label6 ){
              整数 jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            如果( IsUniqueIndex(pIdx) ){
              整数 uniqOk = sqlite3VdbeMakeLabel(pParse);
              整数 jmp6;
              为(kk=0; kk<pIdx->nKeyCol; kk++){
                整数 iCol = pIdx->aiColumn[kk];
                assert( iCol!=XN_ROWID && iCol<pTab->nCol );
                如果( iCol>=0 && pTab->aCol[iCol].notNull ) continue;
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
        如果( pPk ){
          assert( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->nKeyCol);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      为(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        整数 a1;
        如果( tableSkipIntegrityCheck(pTab,pObjTab) ) continue;
        如果( IsOrdinaryTable(pTab) ) continue;
        如果( !IsVirtual(pTab) ) continue;
        如果( pTab->nCol<=0 ){
          const char *zMod = pTab->u.vtab.azArg[0];
          如果( sqlite3HashFind(&db->aModule, zMod)==0 ) continue;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        如果( pTab->u.vtab.p==0 ) continue;
        pVTab = pTab->u.vtab.p->pVtab;
        如果( NEVER(pVTab==0) ) continue;
        如果( NEVER(pVTab->pModule==0) ) continue;
        如果( pVTab->pModule->iVersion<4 ) continue;
        如果( pVTab->pModule->xIntegrity==0 ) continue;
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
      static const 整数 iLn = VDBE_OFFSET_LINENO(2);
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
      如果( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (char*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  break;
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
  case PragTyp_ENCODING: {
    static const struct EncName {
      char *zName;
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
    const struct EncName *pEnc;
    如果( !zRight ){    /* "PRAGMA encoding" */
      如果( sqlite3ReadSchema(pParse) ) goto pragma_out;
      assert( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      assert( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      assert( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }否则{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      如果( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        为(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          如果( 0==sqlite3StrICmp(zRight, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            break;
          }
        }
        如果( !pEnc->zName ){
          sqlite3ErrorMsg(pParse, "unsupported encoding: %s", zRight);
        }
      }
    }
  }
  break;
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
  case PragTyp_HEADER_VALUE: {
    整数 iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    如果( zRight && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      static const VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      如果( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(zRight);
      aOp[1].p5 = 1;
      如果( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }否则{
      /* Read the specified cookie value */
      static const VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      如果( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  break;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  case PragTyp_COMPILE_OPTIONS: {
    整数 i = 0;
    const char *zOpt;
    pParse->nMem = 1;
    当( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  break;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  case PragTyp_WAL_CHECKPOINT: {
    整数 iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    整数 eMode = SQLITE_CHECKPOINT_PASSIVE;
    如果( zRight ){
      如果( sqlite3StrICmp(zRight, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }否则 如果( sqlite3StrICmp(zRight, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }否则 如果( sqlite3StrICmp(zRight, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }否则 如果( sqlite3StrICmp(zRight, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  break;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  case PragTyp_WAL_AUTOCHECKPOINT: {
    如果( zRight ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(zRight));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  break;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  case PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    break;
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
  case PragTyp_OPTIMIZE: {
    整数 iDbLast;           /* Loop termination point for the schema loop */
    整数 iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    char *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    整数 nLimit;            /* Analysis limit to use */
    整数 nCheck = 0;        /* Number of tables to be optimized */
    整数 nBtree = 0;        /* Number of btrees to scan */
    整数 nIndex;            /* Number of indexes on the current table */

    如果( zRight ){
      opMask = (u32)sqlite3Atoi(zRight);
      如果( (opMask & 0x02)==0 ) break;
    }否则{
      opMask = 0xfffe;
    }
    如果( (opMask & 0x10)==0 ){
      nLimit = 0;
    }否则 如果( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }否则{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    为(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      如果( iDb==1 ) continue;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      为(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        如果( !IsOrdinaryTable(pTab) ) continue;

        /* Do not scan system tables */
        如果( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) continue;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        为(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          如果( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        如果( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }否则 如果( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }否则 如果( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }否则{
          /* Otherwise, we can skip this table */
          continue;
        }

        nCheck++;
        如果( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        如果( szThreshold>=0 ){
          const LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }否则{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        如果( opMask & 0x01 ){
          整数 r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }否则{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    如果( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      整数 iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      如果( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      为(iAddr=0; iAddr<iEnd; iAddr++){
        如果( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    break;
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
    整数 n = 0;
    assert( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    如果( zRight && sqlite3GetInt32(zRight,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    break;
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
  case PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    如果( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    break;
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
  case PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    如果( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      如果( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    break;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  case PragTyp_THREADS: {
    sqlite3_int64 N;
    如果( zRight
     && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (整数)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    break;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  case PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    如果( zRight
     && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (整数)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    break;
  }

#如果 defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  case PragTyp_LOCK_STATUS: {
    static const char *const azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    整数 i;
    pParse->nMem = 2;
    为(i=0; i<db->nDb; i++){
      Btree *pBt;
      const char *zState = "unknown";
      整数 j;
      如果( db->aDb[i].zDbSName==0 ) continue;
      pBt = db->aDb[i].pBt;
      如果( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }否则 如果( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    break;
  }
#endif

#如果 defined(SQLITE_ENABLE_CEROD)
  case PragTyp_ACTIVATE_EXTENSIONS: 如果( zRight ){
    如果( sqlite3StrNICmp(zRight, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&zRight[6]);
    }
  }
  break;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  如果( (pPragma->mPragFlg & PragFlg_NoColumns1) && zRight ){
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
typedef struct PragmaVtab PragmaVtab;
typedef struct PragmaVtabCursor PragmaVtabCursor;
struct PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  const PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
struct PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  char *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
static 整数 pragmaVtabConnect(
  sqlite3 *db,
  void *pAux,
  整数 argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  const PragmaName *pPragma = (const PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  整数 rc;
  整数 i, j;
  char cSep = '(';
  StrAccum acc;
  char zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, sizeof(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  为(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  如果( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  如果( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  如果( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  assert( strlen(zBuf) < sizeof(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  如果( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(sizeof(PragmaVtab));
    如果( pTab==0 ){
      rc = SQLITE_NOMEM;
    }否则{
      memset(pTab, 0, sizeof(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }否则{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  返回 rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
static 整数 pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  返回 SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
static 整数 pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  const struct sqlite3_index_constraint *pConstraint;
  整数 i, j;
  整数 seen[2];

  pIdxInfo->estimatedCost = (double)1;
  如果( pTab->nHidden==0 ){ 返回 SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  为(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    如果( pConstraint->iColumn < pTab->iHidden ) continue;
    如果( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    如果( pConstraint->usable==0 ) 返回 SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    assert( j < 2 );
    seen[j] = i+1;
  }
  如果( seen[0]==0 ){
    pIdxInfo->estimatedCost = (double)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    返回 SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (double)20;
  pIdxInfo->estimatedRows = 20;
  如果( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  返回 SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
static 整数 pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(sizeof(*pCsr));
  如果( pCsr==0 ) 返回 SQLITE_NOMEM;
  memset(pCsr, 0, sizeof(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  返回 SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
static void pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  整数 i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  为(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
static 整数 pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  返回 SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
static 整数 pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  整数 rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  assert( pCsr->pPragma );
  如果( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  返回 rc;
}

/*
** Pragma virtual table module xFilter method.
*/
static 整数 pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  整数 idxNum, const char *idxStr,
  整数 argc, sqlite3_value **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  整数 rc;
  整数 i, j;
  StrAccum acc;
  char *zSql;

  UNUSED_PARAMETER(idxNum);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  为(i=0; i<argc; i++, j++){
    const char *zText = (const char*)sqlite3_value_text(argv[i]);
    assert( j<ArraySize(pCsr->azArg) );
    assert( pCsr->azArg[j]==0 );
    如果( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      如果( pCsr->azArg[j]==0 ){
        返回 SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  如果( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  如果( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  如果( zSql==0 ) 返回 SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  如果( rc!=SQLITE_OK ){
    pTab->base.zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    返回 rc;
  }
  返回 pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
static 整数 pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  返回 (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
static 整数 pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  整数 i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  如果( i<pTab->iHidden ){
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pPragma, i));
  }否则{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  返回 SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
static 整数 pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  返回 SQLITE_OK;
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, const char *zName){
  const PragmaName *pName;
  assert( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  如果( pName==0 ) 返回 0;
  如果( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) 返回 0;
  assert( sqlite3HashFind(&db->aModule, zName)==0 );
  返回 sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (void*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
