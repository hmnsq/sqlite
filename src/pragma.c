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
#شامل "sqliteInt.h"

#اگر !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  اگر defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  وگرنه
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
#شامل "pragma.h"

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
ایستا u8 getSafetyLevel(ثابت نویسه *z, صحیح omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  ایستا ثابت نویسه zText[] = "onoffalseyestruextrafull";
  ایستا ثابت u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ایستا ثابت u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ایستا ثابت u8 i_مقدار[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  صحیح i, n;
  اگر( sqlite3Isdigit(*z) ){
    بازگشت sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  برای(i=0; i<ArraySize(iLength); i++){
    اگر( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_مقدار[i]<=1)
    ){
      بازگشت i_مقدار[i];
    }
  }
  بازگشت dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ثابت نویسه *z, u8 dflt){
  بازگشت getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#اگر !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
ایستا صحیح getLockingMode(ثابت نویسه *z){
  اگر( z ){
    اگر( 0==sqlite3StrICmp(z, "exclusive") ) بازگشت PAGER_LOCKINGMODE_EXCLUSIVE;
    اگر( 0==sqlite3StrICmp(z, "normal") ) بازگشت PAGER_LOCKINGMODE_NORMAL;
  }
  بازگشت PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ایستا صحیح getAutoVacuum(ثابت نویسه *z){
  صحیح i;
  اگر( 0==sqlite3StrICmp(z, "none") ) بازگشت BTREE_AUTOVACUUM_NONE;
  اگر( 0==sqlite3StrICmp(z, "full") ) بازگشت BTREE_AUTOVACUUM_FULL;
  اگر( 0==sqlite3StrICmp(z, "incremental") ) بازگشت BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  بازگشت (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ایستا صحیح getTempStore(ثابت نویسه *z){
  اگر( z[0]>='0' && z[0]<='2' ){
    بازگشت z[0] - '0';
  }وگرنه اگر( sqlite3StrICmp(z, "file")==0 ){
    بازگشت 1;
  }وگرنه اگر( sqlite3StrICmp(z, "memory")==0 ){
    بازگشت 2;
  }وگرنه{
    بازگشت 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ایستا صحیح invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  اگر( db->aDb[1].pBt!=0 ){
    اگر( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_پیام(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      بازگشت SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  بازگشت SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ایستا صحیح changeTempStorage(Parse *pParse, ثابت نویسه *zStorageType){
  صحیح ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  اگر( db->temp_store==ts ) بازگشت SQLITE_OK;
  اگر( invalidateTempStorage( pParse ) != SQLITE_OK ){
    بازگشت SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  بازگشت SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ایستا تهی setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  ثابت PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_رقم_Cols(v, n==0 ? 1 : n);
  اگر( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }وگرنه{
    صحیح i, j;
    برای(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ایستا تهی returnSingleInt(Vdbe *v, i64 مقدار){
  sqlite3VdbeAddInt64(v, 1, مقدار);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ایستا تهی returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  ثابت نویسه *z_مقدار      /* Value to be returned */
){
  اگر( z_مقدار ){
    sqlite3VdbeLoadString(v, 1, (ثابت نویسه*)z_مقدار);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
ایستا تهی setAllPagerFlags(sqlite3 *db){
  اگر( db->autoCommit ){
    Db *pDb = db->aDb;
    صحیح n = db->nDb;
    أكد( SQLITE_FullFSync==PAGER_FULLFSYNC );
    أكد( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    أكد( SQLITE_CacheSpill==PAGER_CACHESPILL );
    أكد( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    أكد( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    وقتی( (n--) > 0 ){
      اگر( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#وگرنه
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_کلید
ایستا ثابت نویسه *actionName(u8 action){
  ثابت نویسه *zName;
  انتخاب( action ){
    حالت OE_SetNull:  zName = "SET NULL";        بشکن;
    حالت OE_SetDflt:  zName = "SET DEFAULT";     بشکن;
    حالت OE_Cascade:  zName = "CASCADE";         بشکن;
    حالت OE_Restrict: zName = "RESTRICT";        بشکن;
    پیشفرض:          zName = "NO ACTION"; 
                      أكد( action==OE_None ); بشکن;
  }
  بازگشت zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ثابت نویسه *sqlite3JournalModename(صحیح eMode){
  ایستا نویسه * ثابت azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  أكد( PAGER_JOURNALMODE_DELETE==0 );
  أكد( PAGER_JOURNALMODE_PERSIST==1 );
  أكد( PAGER_JOURNALMODE_OFF==2 );
  أكد( PAGER_JOURNALMODE_TRUNCATE==3 );
  أكد( PAGER_JOURNALMODE_MEMORY==4 );
  أكد( PAGER_JOURNALMODE_WAL==5 );
  أكد( eMode>=0 && eMode<=ArraySize(azModeName) );

  اگر( eMode==ArraySize(azModeName) ) بازگشت 0;
  بازگشت azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ایستا ثابت PragmaName *pragmaLocate(ثابت نویسه *zName){
  صحیح upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  وقتی( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    اگر( rc==0 ) بشکن;
    اگر( rc<0 ){
      upr = mid - 1;
    }وگرنه{
      lwr = mid + 1;
    }
  }
  بازگشت lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
ایستا تهی pragma_Funclist_سطر(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  صحیح isBuiltin,         /* True if this is a built-in function */
  صحیح showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  اگر( showInternFuncs ) mask = 0xffffffff;
  برای(; p; p=p->pNext){
    ثابت نویسه *zType;
    ایستا ثابت نویسه *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    أكد( SQLITE_FUNC_ENCMASK==0x3 );
    أكد( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    اگر( p->xSFunc==0 ) ادامه;
    اگر( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      ادامه;
    }   
    اگر( p->x_مقدار!=0 ){
      zType = "w";
    }وگرنه اگر( p->xFinalize!=0 ){
      zType = "a";
    }وگرنه{
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
ایستا صحیح integrityCheckResultRow(Vdbe *v){
  صحیح addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  بازگشت addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
ایستا صحیح tableSkipIntegrityCheck(ثابت Table *pTab, ثابت Table *pObjTab){
  اگر( pObjTab ){
    بازگشت pTab!=pObjTab;
  }وگرنه{
    بازگشت (pTab->tabFlags & TF_Imposter)!=0;
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
تهی sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_مقدار,      /* Token for <value>, or NULL */
  صحیح minusFlag       /* True if a '-' sign preceded <value> */
){
  نویسه *z_چپ = 0;       /* Nul-terminated UTF-8 string <id> */
  نویسه *z_راست = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ثابت نویسه *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  نویسه *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  صحیح iDb;               /* Database index for <database> */
  صحیح rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ثابت PragmaName *pPragma;   /* The pragma */

  اگر( v==0 ) بازگشت;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  اگر( iDb<0 ) بازگشت;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  اگر( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    بازگشت;
  }

  z_چپ = sqlite3NameFromToken(db, pId);
  اگر( !z_چپ ) بازگشت;
  اگر( minusFlag ){
    z_راست = sqlite3MPrintf(db, "-%T", p_مقدار);
  }وگرنه{
    z_راست = sqlite3NameFromToken(db, p_مقدار);
  }

  أكد( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  اگر( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_چپ, z_راست, zDb) ){
    برو pragma_out;
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
  aFcntl[1] = z_چپ;
  aFcntl[2] = z_راست;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (تهی*)aFcntl);
  اگر( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_رقم_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    برو pragma_out;
  }
  اگر( rc!=SQLITE_NOTFOUND ){
    اگر( aFcntl[0] ){
      sqlite3_Error_پیام(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    برو pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_چپ);
  اگر( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    برو pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  اگر( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    اگر( sqlite3ReadSchema(pParse) ) برو pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  اگر( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_راست==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  انتخاب( pPragma->ePragTyp ){
 
#اگر !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  حالت PragTyp_DEFAULT_CACHE_SIZE: {
    ایستا ثابت صحیح iLn = VDBE_OFFSET_LINENO(2);
    ایستا ثابت VdbeOpList getCacheSize[] = {
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
    اگر( !z_راست ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      اگر( ONLY_IF_REALLOC_STRESS(aOp==0) ) بشکن;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }وگرنه{
      صحیح size = sqlite3AbsInt32(sqlite3Atoi(z_راست));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    بشکن;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#اگر !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  حالت PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    اگر( !z_راست ){
      صحیح size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }وگرنه{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(z_راست);
      اگر( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    بشکن;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  حالت PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    صحیح b = -1;
    أكد( pBt!=0 );
    اگر( z_راست ){
      اگر( sqlite3_stricmp(z_راست, "fast")==0 ){
        b = 2;
      }وگرنه{
        b = sqlite3GetBoolean(z_راست, 0);
      }
    }
    اگر( pId2->n==0 && b>=0 ){
      صحیح ii;
      برای(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    بشکن;
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
  حالت Prag_Typ_PAGE_شمار: {
    صحیح iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    اگر( sqlite3Tolower(z_چپ[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }وگرنه{
      اگر( z_راست && sqlite3DecOrHexToI64(z_راست,&x)==0 ){
        اگر( x<0 ) x = 0;
        وگرنه اگر( x>0xfffffffe ) x = 0xfffffffe;
      }وگرنه{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (صحیح)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    بشکن;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  حالت PragTyp_LOCKING_MODE: {
    ثابت نویسه *zRet = "normal";
    صحیح eMode = getLockingMode(z_راست);

    اگر( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }وگرنه{
      Pager *pPager;
      اگر( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        صحیح ii;
        أكد(pDb==&db->aDb[0]);
        برای(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    أكد( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    اگر( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    بشکن;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  حالت PragTyp_JOURNAL_MODE: {
    صحیح eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    صحیح ii;           /* Loop counter */

    اگر( z_راست==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }وگرنه{
      ثابت نویسه *zMode;
      صحیح n = sqlite3Strlen30(z_راست);
      برای(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        اگر( sqlite3StrNICmp(z_راست, zMode, n)==0 ) بشکن;
      }
      اگر( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      اگر( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    اگر( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    برای(ii=db->nDb-1; ii>=0; ii--){
      اگر( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    بشکن;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  حالت PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    اگر( z_راست ){
      sqlite3DecOrHexToI64(z_راست, &iLimit);
      اگر( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    بشکن;
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
  حالت PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    اگر( !z_راست ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }وگرنه{
      صحیح eAuto = getAutoVacuum(z_راست);
      أكد( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      اگر( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ایستا ثابت صحیح iLn = VDBE_OFFSET_LINENO(2);
        ایستا ثابت VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_ریشه_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        صحیح iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        اگر( ONLY_IF_REALLOC_STRESS(aOp==0) ) بشکن;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    بشکن;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  حالت PragTyp_INCREMENTAL_VACUUM: {
    صحیح iLimit = 0, addr;
    اگر( z_راست==0 || !sqlite3GetInt32(z_راست, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    بشکن;
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
  حالت PragTyp_CACHE_SIZE: {
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    اگر( !z_راست ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }وگرنه{
      صحیح size = sqlite3Atoi(z_راست);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    بشکن;
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
  حالت PragTyp_CACHE_SPILL: {
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    اگر( !z_راست ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }وگرنه{
      صحیح size = 1;
      اگر( sqlite3GetInt32(z_راست, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      اگر( sqlite3GetBoolean(z_راست, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }وگرنه{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    بشکن;
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
  حالت PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#اگر SQLITE_MAX_MMAP_SIZE>0
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    اگر( z_راست ){
      صحیح ii;
      sqlite3DecOrHexToI64(z_راست, &sz);
      اگر( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      اگر( pId2->n==0 ) db->szMmap = sz;
      برای(ii=db->nDb-1; ii>=0; ii--){
        اگر( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#وگرنه
    sz = 0;
    rc = SQLITE_OK;
#endif
    اگر( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }وگرنه اگر( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    بشکن;
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
  حالت PragTyp_TEMP_STORE: {
    اگر( !z_راست ){
      returnSingleInt(v, db->temp_store);
    }وگرنه{
      changeTempStorage(pParse, z_راست);
    }
    بشکن;
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
  حالت PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    اگر( !z_راست ){
      returnSingleText(v, sqlite3_temp_directory);
    }وگرنه{
#ifndef SQLITE_OMIT_WSD
      اگر( z_راست[0] ){
        صحیح res;
        rc = sqlite3OsAccess(db->pVfs, z_راست, SQLITE_ACCESS_READWRITE, &res);
        اگر( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_پیام(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          برو pragma_out;
        }
      }
      اگر( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      اگر( z_راست[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", z_راست);
      }وگرنه{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    بشکن;
  }

#اگر SQLITE_OS_WIN
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
  حالت PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    اگر( !z_راست ){
      returnSingleText(v, sqlite3_data_directory);
    }وگرنه{
#ifndef SQLITE_OMIT_WSD
      اگر( z_راست[0] ){
        صحیح res;
        rc = sqlite3OsAccess(db->pVfs, z_راست, SQLITE_ACCESS_READWRITE, &res);
        اگر( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_پیام(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          برو pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      اگر( z_راست[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", z_راست);
      }وگرنه{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    بشکن;
  }
#endif

#اگر SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  حالت PragTyp_LOCK_PROXY_FILE: {
    اگر( !z_راست ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      نویسه *proxy_file_path = پوچ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }وگرنه{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      صحیح res;
      اگر( z_راست[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_راست);
      } وگرنه {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     پوچ);
      }
      اگر( res!=SQLITE_OK ){
        sqlite3_Error_پیام(pParse, "failed to set lock proxy file");
        برو pragma_out;
      }
    }
    بشکن;
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
  حالت PragTyp_SYNCHRONOUS: {
    اگر( !z_راست ){
      returnSingleInt(v, pDb->safety_level-1);
    }وگرنه{
      اگر( !db->autoCommit ){
        sqlite3_Error_پیام(pParse,
            "Safety level may not be changed inside a transaction");
      }وگرنه اگر( iDb!=1 ){
        صحیح iLevel;
        اگر( sqlite3Isdigit(z_راست[0]) ){
          iLevel = sqlite3Atoi(z_راست);
        }وگرنه{
          iLevel = getSafetyLevel(z_راست,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        اگر( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    بشکن;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  حالت PragTyp_FLAG: {
    اگر( z_راست==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }وگرنه{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      اگر( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_کلیدها);
      }

      اگر( sqlite3GetBoolean(z_راست, 0) ){
        اگر( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }وگرنه{
        db->flags &= ~mask;
        اگر( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        اگر( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_راست, "reset")==0
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
    بشکن;
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
  حالت PragTyp_TABLE_INFO: اگر( z_راست ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_راست, zDb);
    اگر( pTab ){
      صحیح i, k;
      صحیح nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3_Primary_کلید_Index(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      برای(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        صحیح isHidden = 0;
        ثابت عبارت *p_Col_عبارت;
        اگر( pCol->colFlags & COLFLAG_NOINSERT ){
          اگر( pPragma->iArg==0 ){
            nHidden++;
            ادامه;
          }
          اگر( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }وگرنه اگر( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }وگرنه{ أكد( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        اگر( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }وگرنه اگر( pPk==0 ){
          k = 1;
        }وگرنه{
          برای(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_عبارت = sqlite3_Column_عبارت(pTab,pCol);
        أكد( p_Col_عبارت==0 || p_Col_عبارت->op==TK_SPAN || isHidden>=2 );
        أكد( p_Col_عبارت==0 || !عبارت_Has_Property(p_Col_عبارت, EP_Int_مقدار)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_عبارت==0) ? 0 : p_Col_عبارت->u.zToken,
               k,
               isHidden);
      }
    }
  }
  بشکن;

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
  حالت PragTyp_TABLE_LIST: {
    صحیح ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    برای(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      صحیح initNCol;
      اگر( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) ادامه;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_شمار(pHash);
      وقتی( initNCol-- ){
        برای(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          اگر( k==0 ){ initNCol = 0; بشکن; }
          pTab = sqliteHashData(k);
          اگر( pTab->nCol==0 ){
            نویسه *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            اگر( zSql ){
              sqlite3_stmt *pDummy = 0;
              (تهی)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (تهی)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            اگر( db->mallocFailed ){
              sqlite3_Error_پیام(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            بشکن;
          }
        }
      }

      برای(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        ثابت نویسه *zType;
        اگر( z_راست && sqlite3_stricmp(z_راست, pTab->zName)!=0 ) ادامه;
        اگر( IsView(pTab) ){
          zType = "view";
        }وگرنه اگر( IsVirtual(pTab) ){
          zType = "virtual";
        }وگرنه اگر( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }وگرنه{
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
  بشکن;

#ifdef SQLITE_DEBUG
  حالت PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    برای(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  بشکن;
#endif

  حالت PragTyp_INDEX_INFO: اگر( z_راست ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, z_راست, zDb);
    اگر( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_راست, zDb);
      اگر( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_کلید_Index(pTab);
      }
    }
    اگر( pIdx ){
      صحیح iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      صحیح i;
      صحیح mx;
      اگر( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }وگرنه{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_کلید_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      أكد( pParse->nMem<=pPragma->nPragCName );
      برای(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        اگر( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_کلید_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  بشکن;

  حالت PragTyp_INDEX_LIST: اگر( z_راست ){
    Index *pIdx;
    Table *pTab;
    صحیح i;
    pTab = sqlite3FindTable(db, z_راست, zDb);
    اگر( pTab ){
      صحیح iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      برای(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        ثابت نویسه *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  بشکن;

  حالت PragTyp_DATABASE_LIST: {
    صحیح i;
    pParse->nMem = 3;
    برای(i=0; i<db->nDb; i++){
      اگر( db->aDb[i].pBt==0 ) ادامه;
      أكد( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  بشکن;

  حالت PragTyp_COLLATION_LIST: {
    صحیح i = 0;
    HashElem *p;
    pParse->nMem = 2;
    برای(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  بشکن;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  حالت PragTyp_FUNCTION_LIST: {
    صحیح i;
    HashElem *j;
    FuncDef *p;
    صحیح showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    برای(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      برای(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_سطر(v, p, 1, showInternFunc);
      }
    }
    برای(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      أكد( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_سطر(v, p, 0, showInternFunc);
    }
  }
  بشکن;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  حالت PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    برای(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  بشکن;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  حالت PragTyp_PRAGMA_LIST: {
    صحیح i;
    برای(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  بشکن;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_کلید
  حالت Prag_Typ_FOREIGN_کلید_LIST: اگر( z_راست ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_راست, zDb);
    اگر( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      اگر( pFK ){
        صحیح iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        صحیح i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        وقتی(pFK){
          صحیح j;
          برای(j=0; j<pFK->nCol; j++){
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
  بشکن;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_کلید
#ifndef SQLITE_OMIT_TRIGGER
  حالت Prag_Typ_FOREIGN_کلید_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    صحیح i;                 /* Loop counter:  Foreign key number for pTab */
    صحیح j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    صحیح x;                 /* result variable */
    صحیح regResult;         /* 3 registers to hold a result row */
    صحیح regRow;            /* Registers to hold a row from pTab */
    صحیح addr_بالاترین;           /* Top of a loop checking foreign keys */
    صحیح addrOk;            /* Jump here if the key is OK */
    صحیح *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    وقتی( k ){
      اگر( z_راست ){
        pTab = sqlite3LocateTable(pParse, 0, z_راست, zDb);
        k = 0;
      }وگرنه{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      اگر( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) ادامه;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      أكد( IsOrdinaryTable(pTab) );
      برای(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        اگر( pParent==0 ) ادامه;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        اگر( x==0 ){
          اگر( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }وگرنه{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pIdx);
          }
        }وگرنه{
          k = 0;
          بشکن;
        }
      }
      أكد( pParse->nErr>0 || pFK==0 );
      اگر( pFK ) بشکن;
      اگر( pParse->nTab<i ) pParse->nTab = i;
      addr_بالاترین = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      أكد( IsOrdinaryTable(pTab) );
      برای(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        اگر( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          أكد( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        برای(j=0; j<pFK->nCol; j++){
          صحیح iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        اگر( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }وگرنه اگر( pParent ){
          صحیح jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          أكد( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        اگر( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }وگرنه{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addr_بالاترین+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_بالاترین);
    }
  }
  بشکن;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  حالت PragTyp_CASE_SENSITIVE_LIKE: {
    اگر( z_راست ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_راست, 0));
    }
  }
  بشکن;
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
  حالت PragTyp_INTEGRITY_CHECK: {
    صحیح i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    صحیح isQuick = (sqlite3Tolower(z_چپ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    أكد( iDb>=0 );
    أكد( iDb==0 || pId2->z );
    اگر( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    اگر( z_راست ){
      اگر( sqlite3GetInt32(p_مقدار->z, &mxErr) ){
        اگر( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }وگرنه{
        pObjTab = sqlite3LocateTable(pParse, 0, z_راست,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    برای(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      صحیح *a_ریشه;      /* Array of root page numbers of all btrees */
      صحیح cnt = 0;     /* Number of entries in aRoot[] */

      اگر( OMIT_TEMPDB && i==1 ) ادامه;
      اگر( iDb>=0 && i!=iDb ) ادامه;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      أكد( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      برای(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        اگر( tableSkipIntegrityCheck(pTab,pObjTab) ) ادامه;
        اگر( HasRowid(pTab) ) cnt++;
        برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      اگر( cnt==0 ) ادامه;
      اگر( pObjTab ) cnt++;
      a_ریشه = sqlite3DbMallocRawNN(db, اندازه(صحیح)*(cnt+1));
      اگر( a_ریشه==0 ) بشکن;
      cnt = 0;
      اگر( pObjTab ) a_ریشه[++cnt] = 0;
      برای(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        اگر( tableSkipIntegrityCheck(pTab,pObjTab) ) ادامه;
        اگر( HasRowid(pTab) ) a_ریشه[++cnt] = pTab->tnum;
        برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          a_ریشه[++cnt] = pIdx->tnum;
        }
      }
      a_ریشه[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (نویسه*)a_ریشه,P4_INTARRAY);
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
      برای(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        صحیح iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        اگر( tableSkipIntegrityCheck(pTab,pObjTab) ) ادامه;
        اگر( HasRowid(pTab) ){
          iTab = cnt++;
        }وگرنه{
          iTab = cnt;
          برای(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            اگر( Is_Primary_کلید_Index(pIdx) ) بشکن;
            iTab++;
          }
        }
        برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          اگر( pIdx->pPartIdxWhere==0 ){
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
      برای(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        صحیح loop_بالاترین;
        صحیح iDataCur, iIdxCur;
        صحیح r1 = -1;
        صحیح bStrict;            /* True for a STRICT table */
        صحیح r2;                 /* Previous key for WITHOUT ROWID tables */
        صحیح mxCol;              /* Maximum non-virtual column number */

        اگر( tableSkipIntegrityCheck(pTab,pObjTab) ) ادامه;
        اگر( !IsOrdinaryTable(pTab) ) ادامه;
        اگر( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }وگرنه{
          pPk = sqlite3_Primary_کلید_Index(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->n_کلید_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_کلید_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        برای(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        أكد( pParse->nMem>=8+j );
        أكد( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loop_بالاترین = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        أكد( !IsVirtual(pTab) );
        اگر( HasRowid(pTab) ){
          mxCol = -1;
          برای(j=0; j<pTab->nCol; j++){
            اگر( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          اگر( mxCol==pTab->iPKey ) mxCol--;
        }وگرنه{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_کلید_Index(pTab)->nColumn-1;
        }
        اگر( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        اگر( !isQuick ){
          اگر( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            صحیح a1;
            نویسه *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->n_کلید_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            برای(j=0; j<pPk->n_کلید_Col; j++){
              sqlite3_عبارت_Code_Load_Index_Column(pParse, pPk, iDataCur, j, r2+j);
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
        برای(j=0; j<pTab->nCol; j++){
          نویسه *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          صحیح labelError;               /* Jump here to report an error */
          صحیح labelOk;                  /* Jump here if all looks ok */
          صحیح p1, p3, p4;               /* Operands to the OP_IsType opcode */
          صحیح doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          اگر( j==pTab->iPKey ) ادامه;
          اگر( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }وگرنه{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          اگر( pCol->notNull==0 && !doTypeCheck ) ادامه;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          اگر( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }وگرنه{
            اگر( pCol->iDflt ){
              sqlite3_مقدار *p_Dflt_مقدار = 0;
              sqlite3_مقدار_From_عبارت(db, sqlite3_Column_عبارت_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_مقدار);
              اگر( p_Dflt_مقدار ){
                p4 = sqlite3_مقدار_type(p_Dflt_مقدار);
                sqlite3_مقدار_Free(p_Dflt_مقدار);
              }
            }
            p1 = iDataCur;
            اگر( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3_Primary_کلید_Index(pTab), j);
            }وگرنه{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          اگر( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            صحیح jmp3;
            صحیح jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            اگر( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }وگرنه{
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
            اگر( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }وگرنه{
              /* VDBE byte code will fall thru */
            }
          }
          اگر( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ایستا unsigned نویسه aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            أكد( pCol->eCType>=1 && pCol->eCType<=اندازه(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وگرنه اگر( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وگرنه اگر( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            اگر( p1>=0 ){
              sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
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
        اگر( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          عبارت_List *pCheck = sqlite3_عبارت_List_Dup(db, pTab->pCheck, 0);
          اگر( db->mallocFailed==0 ){
            صحیح addrCkFault = sqlite3VdbeMakeLabel(pParse);
            صحیح addrCkOk = sqlite3VdbeMakeLabel(pParse);
            نویسه *zErr;
            صحیح k;
            pParse->iSelfTab = iDataCur + 1;
            برای(k=pCheck->n_عبارت-1; k>0; k--){
              sqlite3_عبارت_If_False(pParse, pCheck->a[k].p_عبارت, addrCkFault, 0);
            }
            sqlite3_عبارت_If_True(pParse, pCheck->a[0].p_عبارت, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_عبارت_List_Delete(db, pCheck);
        }
        اگر( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          برای(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            صحیح jmp2, jmp3, jmp4, jmp5, label6;
            صحیح kk;
            صحیح ckUniq = sqlite3VdbeMakeLabel(pParse);
            اگر( pPk==pIdx ) ادامه;
            r1 = sqlite3_Generate_Index_کلید(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_کلید, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ثابت نویسه*)pIdx, P4_INDEX);
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
            اگر( HasRowid(pTab) ){
              صحیح jmp7;
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
            برای(kk=0; kk<pIdx->n_کلید_Col; kk++){
              اگر( pIdx->azColl[kk]==sqlite3_Str_دودویی ) ادامه;
              اگر( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            اگر( label6 ){
              صحیح jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            اگر( IsUniqueIndex(pIdx) ){
              صحیح uniqOk = sqlite3VdbeMakeLabel(pParse);
              صحیح jmp6;
              برای(kk=0; kk<pIdx->n_کلید_Col; kk++){
                صحیح iCol = pIdx->aiColumn[kk];
                أكد( iCol!=XN_ROWID && iCol<pTab->nCol );
                اگر( iCol>=0 && pTab->aCol[iCol].notNull ) ادامه;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_کلید_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loop_بالاترین); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_بالاترین-1);
        اگر( pPk ){
          أكد( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->n_کلید_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      برای(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        صحیح a1;
        اگر( tableSkipIntegrityCheck(pTab,pObjTab) ) ادامه;
        اگر( IsOrdinaryTable(pTab) ) ادامه;
        اگر( !IsVirtual(pTab) ) ادامه;
        اگر( pTab->nCol<=0 ){
          ثابت نویسه *zMod = pTab->u.vtab.azArg[0];
          اگر( sqlite3HashFind(&db->aModule, zMod)==0 ) ادامه;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        اگر( pTab->u.vtab.p==0 ) ادامه;
        pVTab = pTab->u.vtab.p->pVtab;
        اگر( NEVER(pVTab==0) ) ادامه;
        اگر( NEVER(pVTab->pModule==0) ) ادامه;
        اگر( pVTab->pModule->iVersion<4 ) ادامه;
        اگر( pVTab->pModule->xIntegrity==0 ) ادامه;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        ادامه;
      }
#endif
    }
    {
      ایستا ثابت صحیح iLn = VDBE_OFFSET_LINENO(2);
      ایستا ثابت VdbeOpList endCode[] = {
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
      اگر( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (نویسه*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  بشکن;
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
  حالت PragTyp_ENCODING: {
    ایستا ثابت ساختار EncName {
      نویسه *zName;
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
    ثابت ساختار EncName *pEnc;
    اگر( !z_راست ){    /* "PRAGMA encoding" */
      اگر( sqlite3ReadSchema(pParse) ) برو pragma_out;
      أكد( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      أكد( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      أكد( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }وگرنه{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      اگر( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        برای(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          اگر( 0==sqlite3StrICmp(z_راست, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            بشکن;
          }
        }
        اگر( !pEnc->zName ){
          sqlite3_Error_پیام(pParse, "unsupported encoding: %s", z_راست);
        }
      }
    }
  }
  بشکن;
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
  حالت Prag_Typ_HEADER_مقدار: {
    صحیح iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    اگر( z_راست && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ایستا ثابت VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      اگر( ONLY_IF_REALLOC_STRESS(aOp==0) ) بشکن;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_راست);
      aOp[1].p5 = 1;
      اگر( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }وگرنه{
      /* Read the specified cookie value */
      ایستا ثابت VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      اگر( ONLY_IF_REALLOC_STRESS(aOp==0) ) بشکن;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  بشکن;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  حالت PragTyp_COMPILE_OPTIONS: {
    صحیح i = 0;
    ثابت نویسه *zOpt;
    pParse->nMem = 1;
    وقتی( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  بشکن;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  حالت PragTyp_WAL_CHECKPOINT: {
    صحیح iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    صحیح eMode = SQLITE_CHECKPOINT_PASSIVE;
    اگر( z_راست ){
      اگر( sqlite3StrICmp(z_راست, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }وگرنه اگر( sqlite3StrICmp(z_راست, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }وگرنه اگر( sqlite3StrICmp(z_راست, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }وگرنه اگر( sqlite3StrICmp(z_راست, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  بشکن;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  حالت PragTyp_WAL_AUTOCHECKPOINT: {
    اگر( z_راست ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_راست));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  بشکن;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  حالت PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    بشکن;
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
  حالت PragTyp_OPTIMIZE: {
    صحیح iDbLast;           /* Loop termination point for the schema loop */
    صحیح iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    نویسه *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    صحیح nLimit;            /* Analysis limit to use */
    صحیح nCheck = 0;        /* Number of tables to be optimized */
    صحیح nBtree = 0;        /* Number of btrees to scan */
    صحیح nIndex;            /* Number of indexes on the current table */

    اگر( z_راست ){
      opMask = (u32)sqlite3Atoi(z_راست);
      اگر( (opMask & 0x02)==0 ) بشکن;
    }وگرنه{
      opMask = 0xfffe;
    }
    اگر( (opMask & 0x10)==0 ){
      nLimit = 0;
    }وگرنه اگر( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }وگرنه{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    برای(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      اگر( iDb==1 ) ادامه;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      برای(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        اگر( !IsOrdinaryTable(pTab) ) ادامه;

        /* Do not scan system tables */
        اگر( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) ادامه;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        برای(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          اگر( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        اگر( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }وگرنه اگر( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }وگرنه اگر( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }وگرنه{
          /* Otherwise, we can skip this table */
          ادامه;
        }

        nCheck++;
        اگر( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        اگر( szThreshold>=0 ){
          ثابت LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }وگرنه{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        اگر( opMask & 0x01 ){
          صحیح r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }وگرنه{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    اگر( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      صحیح iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      اگر( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      برای(iAddr=0; iAddr<iEnd; iAddr++){
        اگر( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    بشکن;
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
  /*case PragTyp_BUSY_TIMEOUT*/ پیشفرض: {
    صحیح n = 0;
    أكد( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    اگر( z_راست && sqlite3GetInt32(z_راست,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    بشکن;
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
  حالت PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    اگر( z_راست && sqlite3DecOrHexToI64(z_راست, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    بشکن;
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
  حالت PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    اگر( z_راست && sqlite3DecOrHexToI64(z_راست, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      اگر( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    بشکن;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  حالت PragTyp_THREADS: {
    sqlite3_int64 N;
    اگر( z_راست
     && sqlite3DecOrHexToI64(z_راست, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (صحیح)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    بشکن;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  حالت PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    اگر( z_راست
     && sqlite3DecOrHexToI64(z_راست, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (صحیح)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    بشکن;
  }

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  حالت PragTyp_LOCK_STATUS: {
    ایستا ثابت نویسه *ثابت azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    صحیح i;
    pParse->nMem = 2;
    برای(i=0; i<db->nDb; i++){
      Btree *pBt;
      ثابت نویسه *zState = "unknown";
      صحیح j;
      اگر( db->aDb[i].zDbSName==0 ) ادامه;
      pBt = db->aDb[i].pBt;
      اگر( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }وگرنه اگر( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    بشکن;
  }
#endif

#اگر defined(SQLITE_ENABLE_CEROD)
  حالت PragTyp_ACTIVATE_EXTENSIONS: اگر( z_راست ){
    اگر( sqlite3StrNICmp(z_راست, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_راست[6]);
    }
  }
  بشکن;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  اگر( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_راست ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, z_چپ);
  sqlite3DbFree(db, z_راست);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
تعریفنوع ساختار PragmaVtab PragmaVtab;
تعریفنوع ساختار PragmaVtabCursor PragmaVtabCursor;
ساختار PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ثابت PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
ساختار PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  نویسه *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
ایستا صحیح pragmaVtabConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  ثابت PragmaName *pPragma = (ثابت PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  صحیح rc;
  صحیح i, j;
  نویسه cSep = '(';
  StrAccum acc;
  نویسه zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, اندازه(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  برای(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  اگر( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  اگر( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  اگر( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  أكد( strlen(zBuf) < اندازه(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  اگر( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(اندازه(PragmaVtab));
    اگر( pTab==0 ){
      rc = SQLITE_NOMEM;
    }وگرنه{
      memset(pTab, 0, اندازه(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }وگرنه{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  بازگشت rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
ایستا صحیح pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  بازگشت SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
ایستا صحیح pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ثابت ساختار sqlite3_index_constraint *pConstraint;
  صحیح i, j;
  صحیح seen[2];

  pIdxInfo->estimatedCost = (دوگانه)1;
  اگر( pTab->nHidden==0 ){ بازگشت SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  برای(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    اگر( pConstraint->iColumn < pTab->iHidden ) ادامه;
    اگر( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ادامه;
    اگر( pConstraint->usable==0 ) بازگشت SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    أكد( j < 2 );
    seen[j] = i+1;
  }
  اگر( seen[0]==0 ){
    pIdxInfo->estimatedCost = (دوگانه)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    بازگشت SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (دوگانه)20;
  pIdxInfo->estimatedRows = 20;
  اگر( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  بازگشت SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
ایستا صحیح pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(اندازه(*pCsr));
  اگر( pCsr==0 ) بازگشت SQLITE_NOMEM;
  memset(pCsr, 0, اندازه(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  بازگشت SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ایستا تهی pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  صحیح i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  برای(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
ایستا صحیح pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  بازگشت SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
ایستا صحیح pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  صحیح rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  أكد( pCsr->pPragma );
  اگر( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  بازگشت rc;
}

/*
** Pragma virtual table module xFilter method.
*/
ایستا صحیح pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  صحیح rc;
  صحیح i, j;
  StrAccum acc;
  نویسه *zSql;

  UNUSED_PARAMETER(idx_رقم);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  برای(i=0; i<argc; i++, j++){
    ثابت نویسه *zText = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
    أكد( j<ArraySize(pCsr->azArg) );
    أكد( pCsr->azArg[j]==0 );
    اگر( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      اگر( pCsr->azArg[j]==0 ){
        بازگشت SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  اگر( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  اگر( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  اگر( zSql==0 ) بازگشت SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  اگر( rc!=SQLITE_OK ){
    pTab->base.z_Err_پیام = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    بازگشت rc;
  }
  بازگشت pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
ایستا صحیح pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  بازگشت (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
ایستا صحیح pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  صحیح i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  اگر( i<pTab->iHidden ){
    sqlite3_result_مقدار(ctx, sqlite3_column_مقدار(pCsr->pPragma, i));
  }وگرنه{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  بازگشت SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
ایستا صحیح pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  بازگشت SQLITE_OK;
}

/* The pragma virtual table object */
ایستا ثابت sqlite3_module pragmaVtabModule = {
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, ثابت نویسه *zName){
  ثابت PragmaName *pName;
  أكد( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  اگر( pName==0 ) بازگشت 0;
  اگر( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) بازگشت 0;
  أكد( sqlite3HashFind(&db->aModule, zName)==0 );
  بازگشت sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (تهی*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
