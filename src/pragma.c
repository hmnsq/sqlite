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
#अन्तर्भाव "sqliteInt.h"

#यदि !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  यदि defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  अन्यथा
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
#अन्तर्भाव "pragma.h"

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
स्थिर u8 getSafetyLevel(अचर वर्ण *z, पूर्णाङ्क omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  स्थिर अचर वर्ण zText[] = "onoffalseyestruextrafull";
  स्थिर अचर u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  स्थिर अचर u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  स्थिर अचर u8 i_मान[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  पूर्णाङ्क i, n;
  यदि( sqlite3Isdigit(*z) ){
    फर्काऊ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  लागि(i=0; i<ArraySize(iLength); i++){
    यदि( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_मान[i]<=1)
    ){
      फर्काऊ i_मान[i];
    }
  }
  फर्काऊ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(अचर वर्ण *z, u8 dflt){
  फर्काऊ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#यदि !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
स्थिर पूर्णाङ्क getLockingMode(अचर वर्ण *z){
  यदि( z ){
    यदि( 0==sqlite3StrICmp(z, "exclusive") ) फर्काऊ PAGER_LOCKINGMODE_EXCLUSIVE;
    यदि( 0==sqlite3StrICmp(z, "normal") ) फर्काऊ PAGER_LOCKINGMODE_NORMAL;
  }
  फर्काऊ PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
स्थिर पूर्णाङ्क getAutoVacuum(अचर वर्ण *z){
  पूर्णाङ्क i;
  यदि( 0==sqlite3StrICmp(z, "none") ) फर्काऊ BTREE_AUTOVACUUM_NONE;
  यदि( 0==sqlite3StrICmp(z, "full") ) फर्काऊ BTREE_AUTOVACUUM_FULL;
  यदि( 0==sqlite3StrICmp(z, "incremental") ) फर्काऊ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  फर्काऊ (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
स्थिर पूर्णाङ्क getTempStore(अचर वर्ण *z){
  यदि( z[0]>='0' && z[0]<='2' ){
    फर्काऊ z[0] - '0';
  }अन्यथा यदि( sqlite3StrICmp(z, "file")==0 ){
    फर्काऊ 1;
  }अन्यथा यदि( sqlite3StrICmp(z, "memory")==0 ){
    फर्काऊ 2;
  }अन्यथा{
    फर्काऊ 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
स्थिर पूर्णाङ्क invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  यदि( db->aDb[1].pBt!=0 ){
    यदि( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_सन्देश(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      फर्काऊ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  फर्काऊ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
स्थिर पूर्णाङ्क changeTempStorage(Parse *pParse, अचर वर्ण *zStorageType){
  पूर्णाङ्क ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  यदि( db->temp_store==ts ) फर्काऊ SQLITE_OK;
  यदि( invalidateTempStorage( pParse ) != SQLITE_OK ){
    फर्काऊ SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  फर्काऊ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
स्थिर शून्य setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  अचर PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_अङ्क_Cols(v, n==0 ? 1 : n);
  यदि( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }अन्यथा{
    पूर्णाङ्क i, j;
    लागि(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
स्थिर शून्य returnSingleInt(Vdbe *v, i64 मान){
  sqlite3VdbeAddInt64(v, 1, मान);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
स्थिर शून्य returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  अचर वर्ण *z_मान      /* Value to be returned */
){
  यदि( z_मान ){
    sqlite3VdbeLoadString(v, 1, (अचर वर्ण*)z_मान);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
स्थिर शून्य setAllPagerFlags(sqlite3 *db){
  यदि( db->autoCommit ){
    Db *pDb = db->aDb;
    पूर्णाङ्क n = db->nDb;
    प्रतिज्ञा( SQLITE_FullFSync==PAGER_FULLFSYNC );
    प्रतिज्ञा( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    प्रतिज्ञा( SQLITE_CacheSpill==PAGER_CACHESPILL );
    प्रतिज्ञा( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    प्रतिज्ञा( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    जबसम्म( (n--) > 0 ){
      यदि( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#अन्यथा
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_साँचो
स्थिर अचर वर्ण *actionName(u8 action){
  अचर वर्ण *zName;
  विकल्प( action ){
    पक्ष OE_SetNull:  zName = "SET NULL";        रोक;
    पक्ष OE_SetDflt:  zName = "SET DEFAULT";     रोक;
    पक्ष OE_Cascade:  zName = "CASCADE";         रोक;
    पक्ष OE_Restrict: zName = "RESTRICT";        रोक;
    स्वभाव:          zName = "NO ACTION"; 
                      प्रतिज्ञा( action==OE_None ); रोक;
  }
  फर्काऊ zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
अचर वर्ण *sqlite3JournalModename(पूर्णाङ्क eMode){
  स्थिर वर्ण * अचर azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  प्रतिज्ञा( PAGER_JOURNALMODE_DELETE==0 );
  प्रतिज्ञा( PAGER_JOURNALMODE_PERSIST==1 );
  प्रतिज्ञा( PAGER_JOURNALMODE_OFF==2 );
  प्रतिज्ञा( PAGER_JOURNALMODE_TRUNCATE==3 );
  प्रतिज्ञा( PAGER_JOURNALMODE_MEMORY==4 );
  प्रतिज्ञा( PAGER_JOURNALMODE_WAL==5 );
  प्रतिज्ञा( eMode>=0 && eMode<=ArraySize(azModeName) );

  यदि( eMode==ArraySize(azModeName) ) फर्काऊ 0;
  फर्काऊ azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
स्थिर अचर PragmaName *pragmaLocate(अचर वर्ण *zName){
  पूर्णाङ्क upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  जबसम्म( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    यदि( rc==0 ) रोक;
    यदि( rc<0 ){
      upr = mid - 1;
    }अन्यथा{
      lwr = mid + 1;
    }
  }
  फर्काऊ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
स्थिर शून्य pragma_Funclist_पङ्क्ति(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  पूर्णाङ्क isBuiltin,         /* True if this is a built-in function */
  पूर्णाङ्क showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  यदि( showInternFuncs ) mask = 0xffffffff;
  लागि(; p; p=p->pNext){
    अचर वर्ण *zType;
    स्थिर अचर वर्ण *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    प्रतिज्ञा( SQLITE_FUNC_ENCMASK==0x3 );
    प्रतिज्ञा( म_भेद(azEnc[SQLITE_UTF8],"utf8")==0 );
    प्रतिज्ञा( म_भेद(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    प्रतिज्ञा( म_भेद(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    यदि( p->xSFunc==0 ) जारी;
    यदि( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      जारी;
    }   
    यदि( p->x_मान!=0 ){
      zType = "w";
    }अन्यथा यदि( p->xFinalize!=0 ){
      zType = "a";
    }अन्यथा{
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
स्थिर पूर्णाङ्क integrityCheckResultRow(Vdbe *v){
  पूर्णाङ्क addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  फर्काऊ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
स्थिर पूर्णाङ्क tableSkipIntegrityCheck(अचर Table *pTab, अचर Table *pObjTab){
  यदि( pObjTab ){
    फर्काऊ pTab!=pObjTab;
  }अन्यथा{
    फर्काऊ (pTab->tabFlags & TF_Imposter)!=0;
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
शून्य sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_मान,      /* Token for <value>, or NULL */
  पूर्णाङ्क minusFlag       /* True if a '-' sign preceded <value> */
){
  वर्ण *z_बायाँ = 0;       /* Nul-terminated UTF-8 string <id> */
  वर्ण *z_दायाँ = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  अचर वर्ण *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  वर्ण *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  पूर्णाङ्क iDb;               /* Database index for <database> */
  पूर्णाङ्क rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  अचर PragmaName *pPragma;   /* The pragma */

  यदि( v==0 ) फर्काऊ;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  यदि( iDb<0 ) फर्काऊ;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  यदि( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    फर्काऊ;
  }

  z_बायाँ = sqlite3NameFromToken(db, pId);
  यदि( !z_बायाँ ) फर्काऊ;
  यदि( minusFlag ){
    z_दायाँ = sqlite3MPrintf(db, "-%T", p_मान);
  }अन्यथा{
    z_दायाँ = sqlite3NameFromToken(db, p_मान);
  }

  प्रतिज्ञा( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  यदि( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_बायाँ, z_दायाँ, zDb) ){
    गच्छ pragma_out;
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
  aFcntl[1] = z_बायाँ;
  aFcntl[2] = z_दायाँ;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (शून्य*)aFcntl);
  यदि( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_अङ्क_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    गच्छ pragma_out;
  }
  यदि( rc!=SQLITE_NOTFOUND ){
    यदि( aFcntl[0] ){
      sqlite3_Error_सन्देश(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    गच्छ pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_बायाँ);
  यदि( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    गच्छ pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  यदि( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    यदि( sqlite3ReadSchema(pParse) ) गच्छ pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  यदि( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_दायाँ==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  विकल्प( pPragma->ePragTyp ){
 
#यदि !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  पक्ष PragTyp_DEFAULT_CACHE_SIZE: {
    स्थिर अचर पूर्णाङ्क iLn = VDBE_OFFSET_LINENO(2);
    स्थिर अचर VdbeOpList getCacheSize[] = {
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
    यदि( !z_दायाँ ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) रोक;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }अन्यथा{
      पूर्णाङ्क size = sqlite3AbsInt32(sqlite3Atoi(z_दायाँ));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    रोक;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#यदि !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  पक्ष PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    प्रतिज्ञा( pBt!=0 );
    यदि( !z_दायाँ ){
      पूर्णाङ्क size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }अन्यथा{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(z_दायाँ);
      यदि( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    रोक;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  पक्ष PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    पूर्णाङ्क b = -1;
    प्रतिज्ञा( pBt!=0 );
    यदि( z_दायाँ ){
      यदि( sqlite3_stricmp(z_दायाँ, "fast")==0 ){
        b = 2;
      }अन्यथा{
        b = sqlite3GetBoolean(z_दायाँ, 0);
      }
    }
    यदि( pId2->n==0 && b>=0 ){
      पूर्णाङ्क ii;
      लागि(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    रोक;
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
  पक्ष Prag_Typ_PAGE_गन्ती: {
    पूर्णाङ्क iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    यदि( sqlite3Tolower(z_बायाँ[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }अन्यथा{
      यदि( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ,&x)==0 ){
        यदि( x<0 ) x = 0;
        अन्यथा यदि( x>0xfffffffe ) x = 0xfffffffe;
      }अन्यथा{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (पूर्णाङ्क)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    रोक;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  पक्ष PragTyp_LOCKING_MODE: {
    अचर वर्ण *zRet = "normal";
    पूर्णाङ्क eMode = getLockingMode(z_दायाँ);

    यदि( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }अन्यथा{
      Pager *pPager;
      यदि( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        पूर्णाङ्क ii;
        प्रतिज्ञा(pDb==&db->aDb[0]);
        लागि(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    प्रतिज्ञा( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    यदि( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    रोक;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  पक्ष PragTyp_JOURNAL_MODE: {
    पूर्णाङ्क eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    पूर्णाङ्क ii;           /* Loop counter */

    यदि( z_दायाँ==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }अन्यथा{
      अचर वर्ण *zMode;
      पूर्णाङ्क n = sqlite3Strlen30(z_दायाँ);
      लागि(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        यदि( sqlite3StrNICmp(z_दायाँ, zMode, n)==0 ) रोक;
      }
      यदि( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      यदि( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    यदि( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    लागि(ii=db->nDb-1; ii>=0; ii--){
      यदि( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    रोक;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  पक्ष PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    यदि( z_दायाँ ){
      sqlite3DecOrHexToI64(z_दायाँ, &iLimit);
      यदि( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    रोक;
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
  पक्ष PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    प्रतिज्ञा( pBt!=0 );
    यदि( !z_दायाँ ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }अन्यथा{
      पूर्णाङ्क eAuto = getAutoVacuum(z_दायाँ);
      प्रतिज्ञा( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      यदि( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        स्थिर अचर पूर्णाङ्क iLn = VDBE_OFFSET_LINENO(2);
        स्थिर अचर VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_जरा_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        पूर्णाङ्क iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) रोक;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    रोक;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  पक्ष PragTyp_INCREMENTAL_VACUUM: {
    पूर्णाङ्क iLimit = 0, addr;
    यदि( z_दायाँ==0 || !sqlite3GetInt32(z_दायाँ, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    रोक;
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
  पक्ष PragTyp_CACHE_SIZE: {
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    यदि( !z_दायाँ ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }अन्यथा{
      पूर्णाङ्क size = sqlite3Atoi(z_दायाँ);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    रोक;
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
  पक्ष PragTyp_CACHE_SPILL: {
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    यदि( !z_दायाँ ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }अन्यथा{
      पूर्णाङ्क size = 1;
      यदि( sqlite3GetInt32(z_दायाँ, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      यदि( sqlite3GetBoolean(z_दायाँ, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }अन्यथा{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    रोक;
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
  पक्ष PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#यदि SQLITE_MAX_MMAP_SIZE>0
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    यदि( z_दायाँ ){
      पूर्णाङ्क ii;
      sqlite3DecOrHexToI64(z_दायाँ, &sz);
      यदि( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      यदि( pId2->n==0 ) db->szMmap = sz;
      लागि(ii=db->nDb-1; ii>=0; ii--){
        यदि( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#अन्यथा
    sz = 0;
    rc = SQLITE_OK;
#endif
    यदि( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }अन्यथा यदि( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    रोक;
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
  पक्ष PragTyp_TEMP_STORE: {
    यदि( !z_दायाँ ){
      returnSingleInt(v, db->temp_store);
    }अन्यथा{
      changeTempStorage(pParse, z_दायाँ);
    }
    रोक;
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
  पक्ष PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    यदि( !z_दायाँ ){
      returnSingleText(v, sqlite3_temp_directory);
    }अन्यथा{
#ifndef SQLITE_OMIT_WSD
      यदि( z_दायाँ[0] ){
        पूर्णाङ्क res;
        rc = sqlite3OsAccess(db->pVfs, z_दायाँ, SQLITE_ACCESS_READWRITE, &res);
        यदि( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_सन्देश(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          गच्छ pragma_out;
        }
      }
      यदि( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      यदि( z_दायाँ[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", z_दायाँ);
      }अन्यथा{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    रोक;
  }

#यदि SQLITE_OS_WIN
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
  पक्ष PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    यदि( !z_दायाँ ){
      returnSingleText(v, sqlite3_data_directory);
    }अन्यथा{
#ifndef SQLITE_OMIT_WSD
      यदि( z_दायाँ[0] ){
        पूर्णाङ्क res;
        rc = sqlite3OsAccess(db->pVfs, z_दायाँ, SQLITE_ACCESS_READWRITE, &res);
        यदि( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_सन्देश(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          गच्छ pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      यदि( z_दायाँ[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", z_दायाँ);
      }अन्यथा{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    रोक;
  }
#endif

#यदि SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  पक्ष PragTyp_LOCK_PROXY_FILE: {
    यदि( !z_दायाँ ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      वर्ण *proxy_file_path = रिक्त;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }अन्यथा{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      पूर्णाङ्क res;
      यदि( z_दायाँ[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_दायाँ);
      } अन्यथा {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     रिक्त);
      }
      यदि( res!=SQLITE_OK ){
        sqlite3_Error_सन्देश(pParse, "failed to set lock proxy file");
        गच्छ pragma_out;
      }
    }
    रोक;
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
  पक्ष PragTyp_SYNCHRONOUS: {
    यदि( !z_दायाँ ){
      returnSingleInt(v, pDb->safety_level-1);
    }अन्यथा{
      यदि( !db->autoCommit ){
        sqlite3_Error_सन्देश(pParse,
            "Safety level may not be changed inside a transaction");
      }अन्यथा यदि( iDb!=1 ){
        पूर्णाङ्क iLevel;
        यदि( sqlite3Isdigit(z_दायाँ[0]) ){
          iLevel = sqlite3Atoi(z_दायाँ);
        }अन्यथा{
          iLevel = getSafetyLevel(z_दायाँ,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        यदि( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    रोक;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  पक्ष PragTyp_FLAG: {
    यदि( z_दायाँ==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }अन्यथा{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      यदि( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_साँचोहरू);
      }

      यदि( sqlite3GetBoolean(z_दायाँ, 0) ){
        यदि( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }अन्यथा{
        db->flags &= ~mask;
        यदि( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        यदि( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_दायाँ, "reset")==0
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
    रोक;
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
  पक्ष PragTyp_TABLE_INFO: यदि( z_दायाँ ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_दायाँ, zDb);
    यदि( pTab ){
      पूर्णाङ्क i, k;
      पूर्णाङ्क nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3_Primary_साँचो_Index(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      लागि(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        पूर्णाङ्क isHidden = 0;
        अचर अभिव्यक्ति *p_Col_अभिव्यक्ति;
        यदि( pCol->colFlags & COLFLAG_NOINSERT ){
          यदि( pPragma->iArg==0 ){
            nHidden++;
            जारी;
          }
          यदि( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }अन्यथा यदि( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }अन्यथा{ प्रतिज्ञा( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        यदि( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }अन्यथा यदि( pPk==0 ){
          k = 1;
        }अन्यथा{
          लागि(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_अभिव्यक्ति = sqlite3_Column_अभिव्यक्ति(pTab,pCol);
        प्रतिज्ञा( p_Col_अभिव्यक्ति==0 || p_Col_अभिव्यक्ति->op==TK_SPAN || isHidden>=2 );
        प्रतिज्ञा( p_Col_अभिव्यक्ति==0 || !अभिव्यक्ति_Has_Property(p_Col_अभिव्यक्ति, EP_Int_मान)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_अभिव्यक्ति==0) ? 0 : p_Col_अभिव्यक्ति->u.zToken,
               k,
               isHidden);
      }
    }
  }
  रोक;

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
  पक्ष PragTyp_TABLE_LIST: {
    पूर्णाङ्क ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    लागि(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      पूर्णाङ्क initNCol;
      यदि( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) जारी;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_गन्ती(pHash);
      जबसम्म( initNCol-- ){
        लागि(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          यदि( k==0 ){ initNCol = 0; रोक; }
          pTab = sqliteHashData(k);
          यदि( pTab->nCol==0 ){
            वर्ण *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            यदि( zSql ){
              sqlite3_stmt *pDummy = 0;
              (शून्य)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (शून्य)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            यदि( db->mallocFailed ){
              sqlite3_Error_सन्देश(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            रोक;
          }
        }
      }

      लागि(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        अचर वर्ण *zType;
        यदि( z_दायाँ && sqlite3_stricmp(z_दायाँ, pTab->zName)!=0 ) जारी;
        यदि( IsView(pTab) ){
          zType = "view";
        }अन्यथा यदि( IsVirtual(pTab) ){
          zType = "virtual";
        }अन्यथा यदि( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }अन्यथा{
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
  रोक;

#ifdef SQLITE_DEBUG
  पक्ष PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    लागि(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      लागि(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  रोक;
#endif

  पक्ष PragTyp_INDEX_INFO: यदि( z_दायाँ ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, z_दायाँ, zDb);
    यदि( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_दायाँ, zDb);
      यदि( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_साँचो_Index(pTab);
      }
    }
    यदि( pIdx ){
      पूर्णाङ्क iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      पूर्णाङ्क i;
      पूर्णाङ्क mx;
      यदि( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }अन्यथा{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_साँचो_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      प्रतिज्ञा( pParse->nMem<=pPragma->nPragCName );
      लागि(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        यदि( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_साँचो_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  रोक;

  पक्ष PragTyp_INDEX_LIST: यदि( z_दायाँ ){
    Index *pIdx;
    Table *pTab;
    पूर्णाङ्क i;
    pTab = sqlite3FindTable(db, z_दायाँ, zDb);
    यदि( pTab ){
      पूर्णाङ्क iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      लागि(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        अचर वर्ण *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  रोक;

  पक्ष PragTyp_DATABASE_LIST: {
    पूर्णाङ्क i;
    pParse->nMem = 3;
    लागि(i=0; i<db->nDb; i++){
      यदि( db->aDb[i].pBt==0 ) जारी;
      प्रतिज्ञा( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  रोक;

  पक्ष PragTyp_COLLATION_LIST: {
    पूर्णाङ्क i = 0;
    HashElem *p;
    pParse->nMem = 2;
    लागि(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  रोक;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  पक्ष PragTyp_FUNCTION_LIST: {
    पूर्णाङ्क i;
    HashElem *j;
    FuncDef *p;
    पूर्णाङ्क showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    लागि(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      लागि(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        प्रतिज्ञा( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_पङ्क्ति(v, p, 1, showInternFunc);
      }
    }
    लागि(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      प्रतिज्ञा( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_पङ्क्ति(v, p, 0, showInternFunc);
    }
  }
  रोक;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  पक्ष PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    लागि(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  रोक;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  पक्ष PragTyp_PRAGMA_LIST: {
    पूर्णाङ्क i;
    लागि(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  रोक;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_साँचो
  पक्ष Prag_Typ_FOREIGN_साँचो_LIST: यदि( z_दायाँ ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_दायाँ, zDb);
    यदि( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      यदि( pFK ){
        पूर्णाङ्क iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        पूर्णाङ्क i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        जबसम्म(pFK){
          पूर्णाङ्क j;
          लागि(j=0; j<pFK->nCol; j++){
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
  रोक;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_साँचो
#ifndef SQLITE_OMIT_TRIGGER
  पक्ष Prag_Typ_FOREIGN_साँचो_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    पूर्णाङ्क i;                 /* Loop counter:  Foreign key number for pTab */
    पूर्णाङ्क j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    पूर्णाङ्क x;                 /* result variable */
    पूर्णाङ्क regResult;         /* 3 registers to hold a result row */
    पूर्णाङ्क regRow;            /* Registers to hold a row from pTab */
    पूर्णाङ्क addr_शीर्ष;           /* Top of a loop checking foreign keys */
    पूर्णाङ्क addrOk;            /* Jump here if the key is OK */
    पूर्णाङ्क *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    जबसम्म( k ){
      यदि( z_दायाँ ){
        pTab = sqlite3LocateTable(pParse, 0, z_दायाँ, zDb);
        k = 0;
      }अन्यथा{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      यदि( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) जारी;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      प्रतिज्ञा( IsOrdinaryTable(pTab) );
      लागि(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        यदि( pParent==0 ) जारी;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        यदि( x==0 ){
          यदि( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }अन्यथा{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_साँचो_Info(pParse, pIdx);
          }
        }अन्यथा{
          k = 0;
          रोक;
        }
      }
      प्रतिज्ञा( pParse->nErr>0 || pFK==0 );
      यदि( pFK ) रोक;
      यदि( pParse->nTab<i ) pParse->nTab = i;
      addr_शीर्ष = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      प्रतिज्ञा( IsOrdinaryTable(pTab) );
      लागि(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        यदि( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          प्रतिज्ञा( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        लागि(j=0; j<pFK->nCol; j++){
          पूर्णाङ्क iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        यदि( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }अन्यथा यदि( pParent ){
          पूर्णाङ्क jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          प्रतिज्ञा( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        यदि( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }अन्यथा{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addr_शीर्ष+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_शीर्ष);
    }
  }
  रोक;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  पक्ष PragTyp_CASE_SENSITIVE_LIKE: {
    यदि( z_दायाँ ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_दायाँ, 0));
    }
  }
  रोक;
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
  पक्ष PragTyp_INTEGRITY_CHECK: {
    पूर्णाङ्क i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    पूर्णाङ्क isQuick = (sqlite3Tolower(z_बायाँ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    प्रतिज्ञा( iDb>=0 );
    प्रतिज्ञा( iDb==0 || pId2->z );
    यदि( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    यदि( z_दायाँ ){
      यदि( sqlite3GetInt32(p_मान->z, &mxErr) ){
        यदि( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }अन्यथा{
        pObjTab = sqlite3LocateTable(pParse, 0, z_दायाँ,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    लागि(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      पूर्णाङ्क *a_जरा;      /* Array of root page numbers of all btrees */
      पूर्णाङ्क cnt = 0;     /* Number of entries in aRoot[] */

      यदि( OMIT_TEMPDB && i==1 ) जारी;
      यदि( iDb>=0 && i!=iDb ) जारी;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      प्रतिज्ञा( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      लागि(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        यदि( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        यदि( HasRowid(pTab) ) cnt++;
        लागि(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      यदि( cnt==0 ) जारी;
      यदि( pObjTab ) cnt++;
      a_जरा = sqlite3DbMallocRawNN(db, परिमाण(पूर्णाङ्क)*(cnt+1));
      यदि( a_जरा==0 ) रोक;
      cnt = 0;
      यदि( pObjTab ) a_जरा[++cnt] = 0;
      लागि(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        यदि( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        यदि( HasRowid(pTab) ) a_जरा[++cnt] = pTab->tnum;
        लागि(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          a_जरा[++cnt] = pIdx->tnum;
        }
      }
      a_जरा[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (वर्ण*)a_जरा,P4_INTARRAY);
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
      लागि(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        पूर्णाङ्क iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        यदि( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        यदि( HasRowid(pTab) ){
          iTab = cnt++;
        }अन्यथा{
          iTab = cnt;
          लागि(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            यदि( Is_Primary_साँचो_Index(pIdx) ) रोक;
            iTab++;
          }
        }
        लागि(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          यदि( pIdx->pPartIdxWhere==0 ){
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
      लागि(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        पूर्णाङ्क loop_शीर्ष;
        पूर्णाङ्क iDataCur, iIdxCur;
        पूर्णाङ्क r1 = -1;
        पूर्णाङ्क bStrict;            /* True for a STRICT table */
        पूर्णाङ्क r2;                 /* Previous key for WITHOUT ROWID tables */
        पूर्णाङ्क mxCol;              /* Maximum non-virtual column number */

        यदि( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        यदि( !IsOrdinaryTable(pTab) ) जारी;
        यदि( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }अन्यथा{
          pPk = sqlite3_Primary_साँचो_Index(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->n_साँचो_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_साँचो_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        लागि(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        प्रतिज्ञा( pParse->nMem>=8+j );
        प्रतिज्ञा( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loop_शीर्ष = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        प्रतिज्ञा( !IsVirtual(pTab) );
        यदि( HasRowid(pTab) ){
          mxCol = -1;
          लागि(j=0; j<pTab->nCol; j++){
            यदि( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          यदि( mxCol==pTab->iPKey ) mxCol--;
        }अन्यथा{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_साँचो_Index(pTab)->nColumn-1;
        }
        यदि( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        यदि( !isQuick ){
          यदि( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            पूर्णाङ्क a1;
            वर्ण *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->n_साँचो_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            लागि(j=0; j<pPk->n_साँचो_Col; j++){
              sqlite3_अभिव्यक्ति_Code_Load_Index_Column(pParse, pPk, iDataCur, j, r2+j);
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
        लागि(j=0; j<pTab->nCol; j++){
          वर्ण *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          पूर्णाङ्क labelError;               /* Jump here to report an error */
          पूर्णाङ्क labelOk;                  /* Jump here if all looks ok */
          पूर्णाङ्क p1, p3, p4;               /* Operands to the OP_IsType opcode */
          पूर्णाङ्क doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          यदि( j==pTab->iPKey ) जारी;
          यदि( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }अन्यथा{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          यदि( pCol->notNull==0 && !doTypeCheck ) जारी;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          यदि( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }अन्यथा{
            यदि( pCol->iDflt ){
              sqlite3_मान *p_Dflt_मान = 0;
              sqlite3_मान_From_अभिव्यक्ति(db, sqlite3_Column_अभिव्यक्ति_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_मान);
              यदि( p_Dflt_मान ){
                p4 = sqlite3_मान_type(p_Dflt_मान);
                sqlite3_मान_Free(p_Dflt_मान);
              }
            }
            p1 = iDataCur;
            यदि( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3_Primary_साँचो_Index(pTab), j);
            }अन्यथा{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          यदि( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            पूर्णाङ्क jmp3;
            पूर्णाङ्क jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            यदि( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }अन्यथा{
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
            यदि( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }अन्यथा{
              /* VDBE byte code will fall thru */
            }
          }
          यदि( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            स्थिर अचिन्हित वर्ण aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            प्रतिज्ञा( pCol->eCType>=1 && pCol->eCType<=परिमाण(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा यदि( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा यदि( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            यदि( p1>=0 ){
              sqlite3_अभिव्यक्ति_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
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
        यदि( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          अभिव्यक्ति_List *pCheck = sqlite3_अभिव्यक्ति_List_Dup(db, pTab->pCheck, 0);
          यदि( db->mallocFailed==0 ){
            पूर्णाङ्क addrCkFault = sqlite3VdbeMakeLabel(pParse);
            पूर्णाङ्क addrCkOk = sqlite3VdbeMakeLabel(pParse);
            वर्ण *zErr;
            पूर्णाङ्क k;
            pParse->iSelfTab = iDataCur + 1;
            लागि(k=pCheck->n_अभिव्यक्ति-1; k>0; k--){
              sqlite3_अभिव्यक्ति_If_False(pParse, pCheck->a[k].p_अभिव्यक्ति, addrCkFault, 0);
            }
            sqlite3_अभिव्यक्ति_If_True(pParse, pCheck->a[0].p_अभिव्यक्ति, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_अभिव्यक्ति_List_Delete(db, pCheck);
        }
        यदि( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          लागि(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            पूर्णाङ्क jmp2, jmp3, jmp4, jmp5, label6;
            पूर्णाङ्क kk;
            पूर्णाङ्क ckUniq = sqlite3VdbeMakeLabel(pParse);
            यदि( pPk==pIdx ) जारी;
            r1 = sqlite3_Generate_Index_साँचो(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_साँचो, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (अचर वर्ण*)pIdx, P4_INDEX);
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
            यदि( HasRowid(pTab) ){
              पूर्णाङ्क jmp7;
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
            लागि(kk=0; kk<pIdx->n_साँचो_Col; kk++){
              यदि( pIdx->azColl[kk]==sqlite3_Str_द्विआधारी ) जारी;
              यदि( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            यदि( label6 ){
              पूर्णाङ्क jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            यदि( IsUniqueIndex(pIdx) ){
              पूर्णाङ्क uniqOk = sqlite3VdbeMakeLabel(pParse);
              पूर्णाङ्क jmp6;
              लागि(kk=0; kk<pIdx->n_साँचो_Col; kk++){
                पूर्णाङ्क iCol = pIdx->aiColumn[kk];
                प्रतिज्ञा( iCol!=XN_ROWID && iCol<pTab->nCol );
                यदि( iCol>=0 && pTab->aCol[iCol].notNull ) जारी;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_साँचो_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loop_शीर्ष); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_शीर्ष-1);
        यदि( pPk ){
          प्रतिज्ञा( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->n_साँचो_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      लागि(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        पूर्णाङ्क a1;
        यदि( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        यदि( IsOrdinaryTable(pTab) ) जारी;
        यदि( !IsVirtual(pTab) ) जारी;
        यदि( pTab->nCol<=0 ){
          अचर वर्ण *zMod = pTab->u.vtab.azArg[0];
          यदि( sqlite3HashFind(&db->aModule, zMod)==0 ) जारी;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        यदि( pTab->u.vtab.p==0 ) जारी;
        pVTab = pTab->u.vtab.p->pVtab;
        यदि( NEVER(pVTab==0) ) जारी;
        यदि( NEVER(pVTab->pModule==0) ) जारी;
        यदि( pVTab->pModule->iVersion<4 ) जारी;
        यदि( pVTab->pModule->xIntegrity==0 ) जारी;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        जारी;
      }
#endif
    }
    {
      स्थिर अचर पूर्णाङ्क iLn = VDBE_OFFSET_LINENO(2);
      स्थिर अचर VdbeOpList endCode[] = {
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
      यदि( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (वर्ण*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  रोक;
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
  पक्ष PragTyp_ENCODING: {
    स्थिर अचर संरचना EncName {
      वर्ण *zName;
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
    अचर संरचना EncName *pEnc;
    यदि( !z_दायाँ ){    /* "PRAGMA encoding" */
      यदि( sqlite3ReadSchema(pParse) ) गच्छ pragma_out;
      प्रतिज्ञा( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      प्रतिज्ञा( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      प्रतिज्ञा( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }अन्यथा{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      यदि( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        लागि(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          यदि( 0==sqlite3StrICmp(z_दायाँ, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            रोक;
          }
        }
        यदि( !pEnc->zName ){
          sqlite3_Error_सन्देश(pParse, "unsupported encoding: %s", z_दायाँ);
        }
      }
    }
  }
  रोक;
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
  पक्ष Prag_Typ_HEADER_मान: {
    पूर्णाङ्क iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    यदि( z_दायाँ && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      स्थिर अचर VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) रोक;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_दायाँ);
      aOp[1].p5 = 1;
      यदि( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }अन्यथा{
      /* Read the specified cookie value */
      स्थिर अचर VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) रोक;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  रोक;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  पक्ष PragTyp_COMPILE_OPTIONS: {
    पूर्णाङ्क i = 0;
    अचर वर्ण *zOpt;
    pParse->nMem = 1;
    जबसम्म( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  रोक;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  पक्ष PragTyp_WAL_CHECKPOINT: {
    पूर्णाङ्क iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    पूर्णाङ्क eMode = SQLITE_CHECKPOINT_PASSIVE;
    यदि( z_दायाँ ){
      यदि( sqlite3StrICmp(z_दायाँ, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }अन्यथा यदि( sqlite3StrICmp(z_दायाँ, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }अन्यथा यदि( sqlite3StrICmp(z_दायाँ, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }अन्यथा यदि( sqlite3StrICmp(z_दायाँ, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  रोक;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  पक्ष PragTyp_WAL_AUTOCHECKPOINT: {
    यदि( z_दायाँ ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_दायाँ));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  रोक;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  पक्ष PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    रोक;
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
  पक्ष PragTyp_OPTIMIZE: {
    पूर्णाङ्क iDbLast;           /* Loop termination point for the schema loop */
    पूर्णाङ्क iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    वर्ण *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    पूर्णाङ्क nLimit;            /* Analysis limit to use */
    पूर्णाङ्क nCheck = 0;        /* Number of tables to be optimized */
    पूर्णाङ्क nBtree = 0;        /* Number of btrees to scan */
    पूर्णाङ्क nIndex;            /* Number of indexes on the current table */

    यदि( z_दायाँ ){
      opMask = (u32)sqlite3Atoi(z_दायाँ);
      यदि( (opMask & 0x02)==0 ) रोक;
    }अन्यथा{
      opMask = 0xfffe;
    }
    यदि( (opMask & 0x10)==0 ){
      nLimit = 0;
    }अन्यथा यदि( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }अन्यथा{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    लागि(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      यदि( iDb==1 ) जारी;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      लागि(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        यदि( !IsOrdinaryTable(pTab) ) जारी;

        /* Do not scan system tables */
        यदि( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) जारी;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        लागि(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          यदि( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        यदि( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }अन्यथा यदि( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }अन्यथा यदि( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }अन्यथा{
          /* Otherwise, we can skip this table */
          जारी;
        }

        nCheck++;
        यदि( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        यदि( szThreshold>=0 ){
          अचर LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }अन्यथा{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        यदि( opMask & 0x01 ){
          पूर्णाङ्क r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }अन्यथा{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    यदि( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      पूर्णाङ्क iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      यदि( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      लागि(iAddr=0; iAddr<iEnd; iAddr++){
        यदि( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    रोक;
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
  /*case PragTyp_BUSY_TIMEOUT*/ स्वभाव: {
    पूर्णाङ्क n = 0;
    प्रतिज्ञा( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    यदि( z_दायाँ && sqlite3GetInt32(z_दायाँ,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    रोक;
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
  पक्ष PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    यदि( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    रोक;
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
  पक्ष PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    यदि( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      यदि( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    रोक;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  पक्ष PragTyp_THREADS: {
    sqlite3_int64 N;
    यदि( z_दायाँ
     && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (पूर्णाङ्क)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    रोक;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  पक्ष PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    यदि( z_दायाँ
     && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (पूर्णाङ्क)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    रोक;
  }

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  पक्ष PragTyp_LOCK_STATUS: {
    स्थिर अचर वर्ण *अचर azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    पूर्णाङ्क i;
    pParse->nMem = 2;
    लागि(i=0; i<db->nDb; i++){
      Btree *pBt;
      अचर वर्ण *zState = "unknown";
      पूर्णाङ्क j;
      यदि( db->aDb[i].zDbSName==0 ) जारी;
      pBt = db->aDb[i].pBt;
      यदि( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }अन्यथा यदि( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    रोक;
  }
#endif

#यदि defined(SQLITE_ENABLE_CEROD)
  पक्ष PragTyp_ACTIVATE_EXTENSIONS: यदि( z_दायाँ ){
    यदि( sqlite3StrNICmp(z_दायाँ, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_दायाँ[6]);
    }
  }
  रोक;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  यदि( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_दायाँ ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, z_बायाँ);
  sqlite3DbFree(db, z_दायाँ);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
प्रकारनिर्देश संरचना PragmaVtab PragmaVtab;
प्रकारनिर्देश संरचना PragmaVtabCursor PragmaVtabCursor;
संरचना PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  अचर PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
संरचना PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  वर्ण *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
स्थिर पूर्णाङ्क pragmaVtabConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, अचर वर्ण *अचर*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  अचर PragmaName *pPragma = (अचर PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  पूर्णाङ्क rc;
  पूर्णाङ्क i, j;
  वर्ण cSep = '(';
  StrAccum acc;
  वर्ण zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  लागि(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  यदि( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  यदि( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  यदि( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  प्रतिज्ञा( म_माप(zBuf) < परिमाण(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  यदि( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(परिमाण(PragmaVtab));
    यदि( pTab==0 ){
      rc = SQLITE_NOMEM;
    }अन्यथा{
      स_रखो(pTab, 0, परिमाण(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }अन्यथा{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  फर्काऊ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
स्थिर पूर्णाङ्क pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  फर्काऊ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
स्थिर पूर्णाङ्क pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  अचर संरचना sqlite3_index_constraint *pConstraint;
  पूर्णाङ्क i, j;
  पूर्णाङ्क seen[2];

  pIdxInfo->estimatedCost = (द्विगुण)1;
  यदि( pTab->nHidden==0 ){ फर्काऊ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  लागि(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    यदि( pConstraint->iColumn < pTab->iHidden ) जारी;
    यदि( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) जारी;
    यदि( pConstraint->usable==0 ) फर्काऊ SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    प्रतिज्ञा( j < 2 );
    seen[j] = i+1;
  }
  यदि( seen[0]==0 ){
    pIdxInfo->estimatedCost = (द्विगुण)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    फर्काऊ SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (द्विगुण)20;
  pIdxInfo->estimatedRows = 20;
  यदि( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  फर्काऊ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
स्थिर पूर्णाङ्क pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(परिमाण(*pCsr));
  यदि( pCsr==0 ) फर्काऊ SQLITE_NOMEM;
  स_रखो(pCsr, 0, परिमाण(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  फर्काऊ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
स्थिर शून्य pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  पूर्णाङ्क i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  लागि(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
स्थिर पूर्णाङ्क pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  फर्काऊ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
स्थिर पूर्णाङ्क pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  पूर्णाङ्क rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  प्रतिज्ञा( pCsr->pPragma );
  यदि( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  फर्काऊ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
स्थिर पूर्णाङ्क pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  पूर्णाङ्क idx_अङ्क, अचर वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मान **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  पूर्णाङ्क rc;
  पूर्णाङ्क i, j;
  StrAccum acc;
  वर्ण *zSql;

  UNUSED_PARAMETER(idx_अङ्क);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  लागि(i=0; i<argc; i++, j++){
    अचर वर्ण *zText = (अचर वर्ण*)sqlite3_मान_text(argv[i]);
    प्रतिज्ञा( j<ArraySize(pCsr->azArg) );
    प्रतिज्ञा( pCsr->azArg[j]==0 );
    यदि( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      यदि( pCsr->azArg[j]==0 ){
        फर्काऊ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  यदि( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  यदि( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  यदि( zSql==0 ) फर्काऊ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  यदि( rc!=SQLITE_OK ){
    pTab->base.z_Err_सन्देश = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    फर्काऊ rc;
  }
  फर्काऊ pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
स्थिर पूर्णाङ्क pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  फर्काऊ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
स्थिर पूर्णाङ्क pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  पूर्णाङ्क i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  यदि( i<pTab->iHidden ){
    sqlite3_result_मान(ctx, sqlite3_column_मान(pCsr->pPragma, i));
  }अन्यथा{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  फर्काऊ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
स्थिर पूर्णाङ्क pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  फर्काऊ SQLITE_OK;
}

/* The pragma virtual table object */
स्थिर अचर sqlite3_module pragmaVtabModule = {
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, अचर वर्ण *zName){
  अचर PragmaName *pName;
  प्रतिज्ञा( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  यदि( pName==0 ) फर्काऊ 0;
  यदि( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) फर्काऊ 0;
  प्रतिज्ञा( sqlite3HashFind(&db->aModule, zName)==0 );
  फर्काऊ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (शून्य*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
