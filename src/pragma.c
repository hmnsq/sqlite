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
#অন্তর্ভাব "sqliteInt.h"

#যদি !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  যদি defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  অন্যথা
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
#অন্তর্ভাব "pragma.h"

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
স্থির u8 getSafetyLevel(ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  স্থির ধ্রুবক অক্ষর zText[] = "onoffalseyestruextrafull";
  স্থির ধ্রুবক u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  স্থির ধ্রুবক u8 i_দৈর্ঘ্য[] = {2, 2, 3,  5,    3,   4,   5,    4};
  স্থির ধ্রুবক u8 i_মান[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  পূর্ণসংখ্যা i, n;
  যদি( sqlite3Isdigit(*z) ){
    ফেরত sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  জন্য(i=0; i<Array_আকার(i_দৈর্ঘ্য); i++){
    যদি( i_দৈর্ঘ্য[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_মান[i]<=1)
    ){
      ফেরত i_মান[i];
    }
  }
  ফেরত dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ধ্রুবক অক্ষর *z, u8 dflt){
  ফেরত getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#যদি !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
স্থির পূর্ণসংখ্যা getLockingMode(ধ্রুবক অক্ষর *z){
  যদি( z ){
    যদি( 0==sqlite3StrICmp(z, "exclusive") ) ফেরত PAGER_LOCKINGMODE_EXCLUSIVE;
    যদি( 0==sqlite3StrICmp(z, "normal") ) ফেরত PAGER_LOCKINGMODE_NORMAL;
  }
  ফেরত PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
স্থির পূর্ণসংখ্যা getAutoVacuum(ধ্রুবক অক্ষর *z){
  পূর্ণসংখ্যা i;
  যদি( 0==sqlite3StrICmp(z, "none") ) ফেরত BTREE_AUTOVACUUM_NONE;
  যদি( 0==sqlite3StrICmp(z, "full") ) ফেরত BTREE_AUTOVACUUM_FULL;
  যদি( 0==sqlite3StrICmp(z, "incremental") ) ফেরত BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  ফেরত (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
স্থির পূর্ণসংখ্যা get_অস্থায়ী_Store(ধ্রুবক অক্ষর *z){
  যদি( z[0]>='0' && z[0]<='2' ){
    ফেরত z[0] - '0';
  }অন্যথা যদি( sqlite3StrICmp(z, "file")==0 ){
    ফেরত 1;
  }অন্যথা যদি( sqlite3StrICmp(z, "memory")==0 ){
    ফেরত 2;
  }অন্যথা{
    ফেরত 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
স্থির পূর্ণসংখ্যা invalidate_অস্থায়ী_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  যদি( db->aDb[1].pBt!=0 ){
    যদি( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_বার্তা(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      ফেরত SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
স্থির পূর্ণসংখ্যা change_অস্থায়ী_Storage(Parse *pParse, ধ্রুবক অক্ষর *zStorageType){
  পূর্ণসংখ্যা ts = get_অস্থায়ী_Store(zStorageType);
  sqlite3 *db = pParse->db;
  যদি( db->অস্থায়ী_store==ts ) ফেরত SQLITE_OK;
  যদি( invalidate_অস্থায়ী_Storage( pParse ) != SQLITE_OK ){
    ফেরত SQLITE_ERROR;
  }
  db->অস্থায়ী_store = (u8)ts;
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
স্থির শূন্য set_Pragma_ফলাফল_Column_Names(
  Vdbe *v,                     /* The query under construction */
  ধ্রুবক PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_অঙ্ক_Cols(v, n==0 ? 1 : n);
  যদি( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }অন্যথা{
    পূর্ণসংখ্যা i, j;
    জন্য(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
স্থির শূন্য returnSingleInt(Vdbe *v, i64 মান){
  sqlite3VdbeAddInt64(v, 1, মান);
  sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 1);
}

/*
** Generate code to return a single text value.
*/
স্থির শূন্য returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  ধ্রুবক অক্ষর *z_মান      /* Value to be returned */
){
  যদি( z_মান ){
    sqlite3VdbeLoadString(v, 1, (ধ্রুবক অক্ষর*)z_মান);
    sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
স্থির শূন্য setAllPagerFlags(sqlite3 *db){
  যদি( db->autoCommit ){
    Db *pDb = db->aDb;
    পূর্ণসংখ্যা n = db->nDb;
    প্রতিজ্ঞা( SQLITE_FullFSync==PAGER_FULLFSYNC );
    প্রতিজ্ঞা( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    প্রতিজ্ঞা( SQLITE_CacheSpill==PAGER_CACHESPILL );
    প্রতিজ্ঞা( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    প্রতিজ্ঞা( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    যখন( (n--) > 0 ){
      যদি( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#অন্যথা
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_চাবি
স্থির ধ্রুবক অক্ষর *actionName(u8 action){
  ধ্রুবক অক্ষর *zName;
  নির্বাচন( action ){
    ক্ষেত্র OE_SetNull:  zName = "SET NULL";        বিরতি;
    ক্ষেত্র OE_SetDflt:  zName = "SET DEFAULT";     বিরতি;
    ক্ষেত্র OE_Cascade:  zName = "CASCADE";         বিরতি;
    ক্ষেত্র OE_Restrict: zName = "RESTRICT";        বিরতি;
    পূর্বনির্ধারিত:          zName = "NO ACTION"; 
                      প্রতিজ্ঞা( action==OE_None ); বিরতি;
  }
  ফেরত zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ধ্রুবক অক্ষর *sqlite3JournalModename(পূর্ণসংখ্যা eMode){
  স্থির অক্ষর * ধ্রুবক azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  প্রতিজ্ঞা( PAGER_JOURNALMODE_DELETE==0 );
  প্রতিজ্ঞা( PAGER_JOURNALMODE_PERSIST==1 );
  প্রতিজ্ঞা( PAGER_JOURNALMODE_OFF==2 );
  প্রতিজ্ঞা( PAGER_JOURNALMODE_TRUNCATE==3 );
  প্রতিজ্ঞা( PAGER_JOURNALMODE_MEMORY==4 );
  প্রতিজ্ঞা( PAGER_JOURNALMODE_WAL==5 );
  প্রতিজ্ঞা( eMode>=0 && eMode<=Array_আকার(azModeName) );

  যদি( eMode==Array_আকার(azModeName) ) ফেরত 0;
  ফেরত azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
স্থির ধ্রুবক PragmaName *pragmaLocate(ধ্রুবক অক্ষর *zName){
  পূর্ণসংখ্যা upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_আকার(aPragmaName)-1;
  যখন( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    যদি( rc==0 ) বিরতি;
    যদি( rc<0 ){
      upr = mid - 1;
    }অন্যথা{
      lwr = mid + 1;
    }
  }
  ফেরত lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
স্থির শূন্য pragma_Funclist_সারি(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  পূর্ণসংখ্যা isBuiltin,         /* True if this is a built-in function */
  পূর্ণসংখ্যা showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  যদি( showInternFuncs ) mask = 0xffffffff;
  জন্য(; p; p=p->p_পরবর্তী){
    ধ্রুবক অক্ষর *zType;
    স্থির ধ্রুবক অক্ষর *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    প্রতিজ্ঞা( SQLITE_FUNC_ENCMASK==0x3 );
    প্রতিজ্ঞা( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    প্রতিজ্ঞা( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    প্রতিজ্ঞা( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    যদি( p->xSFunc==0 ) চালু;
    যদি( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      চালু;
    }   
    যদি( p->x_মান!=0 ){
      zType = "w";
    }অন্যথা যদি( p->xFinalize!=0 ){
      zType = "a";
    }অন্যথা{
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
স্থির পূর্ণসংখ্যা integrity_Check_ফলাফল_Row(Vdbe *v){
  পূর্ণসংখ্যা addr;
  sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  ফেরত addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
স্থির পূর্ণসংখ্যা tableSkipIntegrityCheck(ধ্রুবক Table *pTab, ধ্রুবক Table *pObjTab){
  যদি( pObjTab ){
    ফেরত pTab!=pObjTab;
  }অন্যথা{
    ফেরত (pTab->tabFlags & TF_Imposter)!=0;
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
শূন্য sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_মান,      /* Token for <value>, or NULL */
  পূর্ণসংখ্যা minusFlag       /* True if a '-' sign preceded <value> */
){
  অক্ষর *z_বাম = 0;       /* Nul-terminated UTF-8 string <id> */
  অক্ষর *z_ডান = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ধ্রুবক অক্ষর *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  অক্ষর *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  পূর্ণসংখ্যা iDb;               /* Database index for <database> */
  পূর্ণসংখ্যা rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ধ্রুবক PragmaName *pPragma;   /* The pragma */

  যদি( v==0 ) ফেরত;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  যদি( iDb<0 ) ফেরত;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  যদি( iDb==1 && sqlite3_Open_অস্থায়ী_Database(pParse) ){
    ফেরত;
  }

  z_বাম = sqlite3NameFromToken(db, pId);
  যদি( !z_বাম ) ফেরত;
  যদি( minusFlag ){
    z_ডান = sqlite3MPrintf(db, "-%T", p_মান);
  }অন্যথা{
    z_ডান = sqlite3NameFromToken(db, p_মান);
  }

  প্রতিজ্ঞা( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  যদি( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_বাম, z_ডান, zDb) ){
    যাও pragma_out;
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
  aFcntl[1] = z_বাম;
  aFcntl[2] = z_ডান;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (শূন্য*)aFcntl);
  যদি( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_অঙ্ক_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    যাও pragma_out;
  }
  যদি( rc!=SQLITE_NOTFOUND ){
    যদি( aFcntl[0] ){
      sqlite3_Error_বার্তা(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    যাও pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_বাম);
  যদি( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    যাও pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  যদি( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    যদি( sqlite3ReadSchema(pParse) ) যাও pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  যদি( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_ডান==0)
  ){
    set_Pragma_ফলাফল_Column_Names(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  নির্বাচন( pPragma->ePragTyp ){
 
#যদি !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  ক্ষেত্র Prag_Typ_DEFAULT_CACHE_আকার: {
    স্থির ধ্রুবক পূর্ণসংখ্যা iLn = VDBE_OFFSET_LINENO(2);
    স্থির ধ্রুবক VdbeOpList get_Cache_আকার[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_আকার},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_Subtract,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_ফলাফল_Row,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3VdbeUsesBtree(v, iDb);
    যদি( !z_ডান ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_আকার(get_Cache_আকার));
      aOp = sqlite3VdbeAddOpList(v, Array_আকার(get_Cache_আকার), get_Cache_আকার, iLn);
      যদি( ONLY_IF_REALLOC_STRESS(aOp==0) ) বিরতি;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_আকার;
    }অন্যথা{
      পূর্ণসংখ্যা আকার_2 = sqlite3AbsInt32(sqlite3Atoi(z_ডান));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_আকার, আকার_2);
      প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_আকার = আকার_2;
      sqlite3_Btree_Set_Cache_আকার(pDb->pBt, pDb->pSchema->cache_আকার);
    }
    বিরতি;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#যদি !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  ক্ষেত্র Prag_Typ_PAGE_আকার: {
    Btree *pBt = pDb->pBt;
    প্রতিজ্ঞা( pBt!=0 );
    যদি( !z_ডান ){
      পূর্ণসংখ্যা আকার_2 = ALWAYS(pBt) ? sqlite3_Btree_Get_Page_আকার(pBt) : 0;
      returnSingleInt(v, আকার_2);
    }অন্যথা{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->পরবর্তী_Pagesize = sqlite3Atoi(z_ডান);
      যদি( SQLITE_NOMEM==sqlite3_Btree_Set_Page_আকার(pBt, db->পরবর্তী_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    বিরতি;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  ক্ষেত্র PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    পূর্ণসংখ্যা b = -1;
    প্রতিজ্ঞা( pBt!=0 );
    যদি( z_ডান ){
      যদি( sqlite3_stricmp(z_ডান, "fast")==0 ){
        b = 2;
      }অন্যথা{
        b = sqlite3GetBoolean(z_ডান, 0);
      }
    }
    যদি( pId2->n==0 && b>=0 ){
      পূর্ণসংখ্যা ii;
      জন্য(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    বিরতি;
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
  ক্ষেত্র Prag_Typ_PAGE_গুনতি: {
    পূর্ণসংখ্যা iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    যদি( sqlite3Tolower(z_বাম[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }অন্যথা{
      যদি( z_ডান && sqlite3DecOrHexToI64(z_ডান,&x)==0 ){
        যদি( x<0 ) x = 0;
        অন্যথা যদি( x>0xfffffffe ) x = 0xfffffffe;
      }অন্যথা{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (পূর্ণসংখ্যা)x);
    }
    sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, iReg, 1);
    বিরতি;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  ক্ষেত্র PragTyp_LOCKING_MODE: {
    ধ্রুবক অক্ষর *zRet = "normal";
    পূর্ণসংখ্যা eMode = getLockingMode(z_ডান);

    যদি( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }অন্যথা{
      Pager *pPager;
      যদি( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        পূর্ণসংখ্যা ii;
        প্রতিজ্ঞা(pDb==&db->aDb[0]);
        জন্য(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    প্রতিজ্ঞা( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    যদি( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    বিরতি;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  ক্ষেত্র PragTyp_JOURNAL_MODE: {
    পূর্ণসংখ্যা eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    পূর্ণসংখ্যা ii;           /* Loop counter */

    যদি( z_ডান==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }অন্যথা{
      ধ্রুবক অক্ষর *zMode;
      পূর্ণসংখ্যা n = sqlite3Strlen30(z_ডান);
      জন্য(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        যদি( sqlite3StrNICmp(z_ডান, zMode, n)==0 ) বিরতি;
      }
      যদি( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      যদি( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    যদি( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    জন্য(ii=db->nDb-1; ii>=0; ii--){
      যদি( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 1);
    বিরতি;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  ক্ষেত্র Prag_Typ_JOURNAL_আকার_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    যদি( z_ডান ){
      sqlite3DecOrHexToI64(z_ডান, &iLimit);
      যদি( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3_Pager_Journal_আকার_Limit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    বিরতি;
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
  ক্ষেত্র PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    প্রতিজ্ঞা( pBt!=0 );
    যদি( !z_ডান ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }অন্যথা{
      পূর্ণসংখ্যা eAuto = getAutoVacuum(z_ডান);
      প্রতিজ্ঞা( eAuto>=0 && eAuto<=2 );
      db->পরবর্তী_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      যদি( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        স্থির ধ্রুবক পূর্ণসংখ্যা iLn = VDBE_OFFSET_LINENO(2);
        স্থির ধ্রুবক VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_মূল_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        পূর্ণসংখ্যা iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_আকার(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, Array_আকার(setMeta6), setMeta6, iLn);
        যদি( ONLY_IF_REALLOC_STRESS(aOp==0) ) বিরতি;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    বিরতি;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  ক্ষেত্র PragTyp_INCREMENTAL_VACUUM: {
    পূর্ণসংখ্যা iLimit = 0, addr;
    যদি( z_ডান==0 || !sqlite3GetInt32(z_ডান, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ফলাফল_Row, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    বিরতি;
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
  ক্ষেত্র Prag_Typ_CACHE_আকার: {
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
    যদি( !z_ডান ){
      returnSingleInt(v, pDb->pSchema->cache_আকার);
    }অন্যথা{
      পূর্ণসংখ্যা আকার_2 = sqlite3Atoi(z_ডান);
      pDb->pSchema->cache_আকার = আকার_2;
      sqlite3_Btree_Set_Cache_আকার(pDb->pBt, pDb->pSchema->cache_আকার);
    }
    বিরতি;
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
  ক্ষেত্র PragTyp_CACHE_SPILL: {
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
    যদি( !z_ডান ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_Set_Spill_আকার(pDb->pBt,0));
    }অন্যথা{
      পূর্ণসংখ্যা আকার_2 = 1;
      যদি( sqlite3GetInt32(z_ডান, &আকার_2) ){
        sqlite3_Btree_Set_Spill_আকার(pDb->pBt, আকার_2);
      }
      যদি( sqlite3GetBoolean(z_ডান, আকার_2!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }অন্যথা{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    বিরতি;
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
  ক্ষেত্র Prag_Typ_MMAP_আকার: {
    sqlite3_int64 sz;
#যদি SQLITE_MAX_MMAP_আকার>0
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
    যদি( z_ডান ){
      পূর্ণসংখ্যা ii;
      sqlite3DecOrHexToI64(z_ডান, &sz);
      যদি( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      যদি( pId2->n==0 ) db->szMmap = sz;
      জন্য(ii=db->nDb-1; ii>=0; ii--){
        যদি( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_আকার, &sz);
#অন্যথা
    sz = 0;
    rc = SQLITE_OK;
#endif
    যদি( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }অন্যথা যদি( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    বিরতি;
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
  ক্ষেত্র Prag_Typ_অস্থায়ী_STORE: {
    যদি( !z_ডান ){
      returnSingleInt(v, db->অস্থায়ী_store);
    }অন্যথা{
      change_অস্থায়ী_Storage(pParse, z_ডান);
    }
    বিরতি;
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
  ক্ষেত্র Prag_Typ_অস্থায়ী_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    যদি( !z_ডান ){
      returnSingleText(v, sqlite3_অস্থায়ী_directory);
    }অন্যথা{
#ifndef SQLITE_OMIT_WSD
      যদি( z_ডান[0] ){
        পূর্ণসংখ্যা res;
        rc = sqlite3OsAccess(db->pVfs, z_ডান, SQLITE_ACCESS_READWRITE, &res);
        যদি( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_বার্তা(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          যাও pragma_out;
        }
      }
      যদি( SQLITE_অস্থায়ী_STORE==0
       || (SQLITE_অস্থায়ী_STORE==1 && db->অস্থায়ী_store<=1)
       || (SQLITE_অস্থায়ী_STORE==2 && db->অস্থায়ী_store==1)
      ){
        invalidate_অস্থায়ী_Storage(pParse);
      }
      sqlite3_free(sqlite3_অস্থায়ী_directory);
      যদি( z_ডান[0] ){
        sqlite3_অস্থায়ী_directory = sqlite3_mprintf("%s", z_ডান);
      }অন্যথা{
        sqlite3_অস্থায়ী_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    বিরতি;
  }

#যদি SQLITE_OS_WIN
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
  ক্ষেত্র Prag_Typ_তথ্য_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    যদি( !z_ডান ){
      returnSingleText(v, sqlite3_তথ্য_directory);
    }অন্যথা{
#ifndef SQLITE_OMIT_WSD
      যদি( z_ডান[0] ){
        পূর্ণসংখ্যা res;
        rc = sqlite3OsAccess(db->pVfs, z_ডান, SQLITE_ACCESS_READWRITE, &res);
        যদি( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_বার্তা(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          যাও pragma_out;
        }
      }
      sqlite3_free(sqlite3_তথ্য_directory);
      যদি( z_ডান[0] ){
        sqlite3_তথ্য_directory = sqlite3_mprintf("%s", z_ডান);
      }অন্যথা{
        sqlite3_তথ্য_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    বিরতি;
  }
#endif

#যদি SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  ক্ষেত্র PragTyp_LOCK_PROXY_FILE: {
    যদি( !z_ডান ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      অক্ষর *proxy_file_path = ফাঁকা;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }অন্যথা{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      পূর্ণসংখ্যা res;
      যদি( z_ডান[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_ডান);
      } অন্যথা {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     ফাঁকা);
      }
      যদি( res!=SQLITE_OK ){
        sqlite3_Error_বার্তা(pParse, "failed to set lock proxy file");
        যাও pragma_out;
      }
    }
    বিরতি;
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
  ক্ষেত্র PragTyp_SYNCHRONOUS: {
    যদি( !z_ডান ){
      returnSingleInt(v, pDb->safety_level-1);
    }অন্যথা{
      যদি( !db->autoCommit ){
        sqlite3_Error_বার্তা(pParse,
            "Safety level may not be changed inside a transaction");
      }অন্যথা যদি( iDb!=1 ){
        পূর্ণসংখ্যা iLevel;
        যদি( sqlite3Isdigit(z_ডান[0]) ){
          iLevel = sqlite3Atoi(z_ডান);
        }অন্যথা{
          iLevel = getSafetyLevel(z_ডান,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        যদি( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    বিরতি;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  ক্ষেত্র PragTyp_FLAG: {
    যদি( z_ডান==0 ){
      set_Pragma_ফলাফল_Column_Names(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }অন্যথা{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      যদি( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_চাবিগুলি);
      }

      যদি( sqlite3GetBoolean(z_ডান, 0) ){
        যদি( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }অন্যথা{
        db->flags &= ~mask;
        যদি( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        যদি( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_ডান, "reset")==0
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
    বিরতি;
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
  ক্ষেত্র PragTyp_TABLE_INFO: যদি( z_ডান ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_ডান, zDb);
    যদি( pTab ){
      পূর্ণসংখ্যা i, k;
      পূর্ণসংখ্যা nHidden = 0;
      Column *pCol;
      সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      জন্য(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        পূর্ণসংখ্যা isHidden = 0;
        ধ্রুবক রাশিমালা *p_Col_রাশিমালা;
        যদি( pCol->colFlags & COLFLAG_NOINSERT ){
          যদি( pPragma->iArg==0 ){
            nHidden++;
            চালু;
          }
          যদি( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }অন্যথা যদি( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }অন্যথা{ প্রতিজ্ঞা( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        যদি( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }অন্যথা যদি( pPk==0 ){
          k = 1;
        }অন্যথা{
          জন্য(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_রাশিমালা = sqlite3_Column_রাশিমালা(pTab,pCol);
        প্রতিজ্ঞা( p_Col_রাশিমালা==0 || p_Col_রাশিমালা->op==TK_SPAN || isHidden>=2 );
        প্রতিজ্ঞা( p_Col_রাশিমালা==0 || !রাশিমালা_Has_Property(p_Col_রাশিমালা, EP_Int_মান)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_রাশিমালা==0) ? 0 : p_Col_রাশিমালা->u.zToken,
               k,
               isHidden);
      }
    }
  }
  বিরতি;

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
  ক্ষেত্র PragTyp_TABLE_LIST: {
    পূর্ণসংখ্যা ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    জন্য(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      পূর্ণসংখ্যা initNCol;
      যদি( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) চালু;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_গুনতি(pHash);
      যখন( initNCol-- ){
        জন্য(k=sqliteHashFirst(pHash); 1; k=sqlite_Hash_পরবর্তী(k) ){
          Table *pTab;
          যদি( k==0 ){ initNCol = 0; বিরতি; }
          pTab = sqlite_Hash_তথ্য(k);
          যদি( pTab->nCol==0 ){
            অক্ষর *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            যদি( zSql ){
              sqlite3_stmt *pDummy = 0;
              (শূন্য)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (শূন্য)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            যদি( db->mallocFailed ){
              sqlite3_Error_বার্তা(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            বিরতি;
          }
        }
      }

      জন্য(k=sqliteHashFirst(pHash); k; k=sqlite_Hash_পরবর্তী(k) ){
        Table *pTab = sqlite_Hash_তথ্য(k);
        ধ্রুবক অক্ষর *zType;
        যদি( z_ডান && sqlite3_stricmp(z_ডান, pTab->zName)!=0 ) চালু;
        যদি( IsView(pTab) ){
          zType = "view";
        }অন্যথা যদি( IsVirtual(pTab) ){
          zType = "virtual";
        }অন্যথা যদি( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }অন্যথা{
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
  বিরতি;

#ifdef SQLITE_DEBUG
  ক্ষেত্র PragTyp_STATS: {
    সূচক *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    জন্য(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqlite_Hash_পরবর্তী(i)){
      Table *pTab = sqlite_Hash_তথ্য(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 5);
      }
    }
  }
  বিরতি;
#endif

  ক্ষেত্র Prag_Typ_সূচক_INFO: যদি( z_ডান ){
    সূচক *pIdx;
    Table *pTab;
    pIdx = sqlite3_Find_সূচক(db, z_ডান, zDb);
    যদি( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_ডান, zDb);
      যদি( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_চাবি_সূচক(pTab);
      }
    }
    যদি( pIdx ){
      পূর্ণসংখ্যা iIdxDb = sqlite3_Schema_To_সূচক(db, pIdx->pSchema);
      পূর্ণসংখ্যা i;
      পূর্ণসংখ্যা mx;
      যদি( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }অন্যথা{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_চাবি_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      প্রতিজ্ঞা( pParse->nMem<=pPragma->nPragCName );
      জন্য(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        যদি( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_চাবি_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, pParse->nMem);
      }
    }
  }
  বিরতি;

  ক্ষেত্র Prag_Typ_সূচক_LIST: যদি( z_ডান ){
    সূচক *pIdx;
    Table *pTab;
    পূর্ণসংখ্যা i;
    pTab = sqlite3FindTable(db, z_ডান, zDb);
    যদি( pTab ){
      পূর্ণসংখ্যা iTabDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      জন্য(pIdx=pTab->p_সূচক, i=0; pIdx; pIdx=pIdx->p_পরবর্তী, i++){
        ধ্রুবক অক্ষর *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_সূচক(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  বিরতি;

  ক্ষেত্র PragTyp_DATABASE_LIST: {
    পূর্ণসংখ্যা i;
    pParse->nMem = 3;
    জন্য(i=0; i<db->nDb; i++){
      যদি( db->aDb[i].pBt==0 ) চালু;
      প্রতিজ্ঞা( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  বিরতি;

  ক্ষেত্র PragTyp_COLLATION_LIST: {
    পূর্ণসংখ্যা i = 0;
    HashElem *p;
    pParse->nMem = 2;
    জন্য(p=sqliteHashFirst(&db->aCollSeq); p; p=sqlite_Hash_পরবর্তী(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_তথ্য(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  বিরতি;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  ক্ষেত্র PragTyp_FUNCTION_LIST: {
    পূর্ণসংখ্যা i;
    HashElem *j;
    FuncDef *p;
    পূর্ণসংখ্যা showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    জন্য(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      জন্য(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        প্রতিজ্ঞা( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_সারি(v, p, 1, showInternFunc);
      }
    }
    জন্য(j=sqliteHashFirst(&db->aFunc); j; j=sqlite_Hash_পরবর্তী(j)){
      p = (FuncDef*)sqlite_Hash_তথ্য(j);
      প্রতিজ্ঞা( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_সারি(v, p, 0, showInternFunc);
    }
  }
  বিরতি;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ক্ষেত্র PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    জন্য(j=sqliteHashFirst(&db->aModule); j; j=sqlite_Hash_পরবর্তী(j)){
      Module *pMod = (Module*)sqlite_Hash_তথ্য(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  বিরতি;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  ক্ষেত্র PragTyp_PRAGMA_LIST: {
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<Array_আকার(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  বিরতি;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_চাবি
  ক্ষেত্র Prag_Typ_FOREIGN_চাবি_LIST: যদি( z_ডান ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_ডান, zDb);
    যদি( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      যদি( pFK ){
        পূর্ণসংখ্যা iTabDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema);
        পূর্ণসংখ্যা i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        যখন(pFK){
          পূর্ণসংখ্যা j;
          জন্য(j=0; j<pFK->nCol; j++){
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
          pFK = pFK->p_পরবর্তী_From;
        }
      }
    }
  }
  বিরতি;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_চাবি
#ifndef SQLITE_OMIT_TRIGGER
  ক্ষেত্র Prag_Typ_FOREIGN_চাবি_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    সূচক *pIdx;           /* Index in the parent table */
    পূর্ণসংখ্যা i;                 /* Loop counter:  Foreign key number for pTab */
    পূর্ণসংখ্যা j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    পূর্ণসংখ্যা x;                 /* result variable */
    পূর্ণসংখ্যা reg_ফলাফল;         /* 3 registers to hold a result row */
    পূর্ণসংখ্যা regRow;            /* Registers to hold a row from pTab */
    পূর্ণসংখ্যা addr_শীর্ষ;           /* Top of a loop checking foreign keys */
    পূর্ণসংখ্যা addrOk;            /* Jump here if the key is OK */
    পূর্ণসংখ্যা *aiCols;           /* child to parent column mapping */

    reg_ফলাফল = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    যখন( k ){
      যদি( z_ডান ){
        pTab = sqlite3LocateTable(pParse, 0, z_ডান, zDb);
        k = 0;
      }অন্যথা{
        pTab = (Table*)sqlite_Hash_তথ্য(k);
        k = sqlite_Hash_পরবর্তী(k);
      }
      যদি( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) চালু;
      iDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, reg_ফলাফল, pTab->zName);
      প্রতিজ্ঞা( IsOrdinaryTable(pTab) );
      জন্য(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_পরবর্তী_From){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        যদি( pParent==0 ) চালু;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3_Fk_Locate_সূচক(pParse, pParent, pFK, &pIdx, 0);
        যদি( x==0 ){
          যদি( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }অন্যথা{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIdx);
          }
        }অন্যথা{
          k = 0;
          বিরতি;
        }
      }
      প্রতিজ্ঞা( pParse->nErr>0 || pFK==0 );
      যদি( pFK ) বিরতি;
      যদি( pParse->nTab<i ) pParse->nTab = i;
      addr_শীর্ষ = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      প্রতিজ্ঞা( IsOrdinaryTable(pTab) );
      জন্য(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_পরবর্তী_From){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        যদি( pParent ){
          x = sqlite3_Fk_Locate_সূচক(pParse, pParent, pFK, &pIdx, &aiCols);
          প্রতিজ্ঞা( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        জন্য(j=0; j<pFK->nCol; j++){
          পূর্ণসংখ্যা iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        যদি( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3_সূচক_Affinity_Str(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }অন্যথা যদি( pParent ){
          পূর্ণসংখ্যা jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          প্রতিজ্ঞা( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        যদি( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, reg_ফলাফল+1);
        }অন্যথা{
          sqlite3VdbeAddOp2(v, OP_Null, 0, reg_ফলাফল+1);
        }
        sqlite3VdbeMultiLoad(v, reg_ফলাফল+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, reg_ফলাফল, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_পরবর্তী, 0, addr_শীর্ষ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_শীর্ষ);
    }
  }
  বিরতি;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  ক্ষেত্র PragTyp_CASE_SENSITIVE_LIKE: {
    যদি( z_ডান ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_ডান, 0));
    }
  }
  বিরতি;
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
  ক্ষেত্র PragTyp_INTEGRITY_CHECK: {
    পূর্ণসংখ্যা i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    পূর্ণসংখ্যা isQuick = (sqlite3Tolower(z_বাম[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    প্রতিজ্ঞা( iDb>=0 );
    প্রতিজ্ঞা( iDb==0 || pId2->z );
    যদি( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    যদি( z_ডান ){
      যদি( sqlite3GetInt32(p_মান->z, &mxErr) ){
        যদি( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }অন্যথা{
        pObjTab = sqlite3LocateTable(pParse, 0, z_ডান,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    জন্য(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      পূর্ণসংখ্যা *a_মূল;      /* Array of root page numbers of all btrees */
      পূর্ণসংখ্যা cnt = 0;     /* Number of entries in aRoot[] */

      যদি( OMIT_TEMPDB && i==1 ) চালু;
      যদি( iDb>=0 && i!=iDb ) চালু;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      জন্য(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_পরবর্তী(x)){
        Table *pTab = sqlite_Hash_তথ্য(x);  /* Current table */
        সূচক *pIdx;                      /* An index on pTab */
        যদি( tableSkipIntegrityCheck(pTab,pObjTab) ) চালু;
        যদি( HasRowid(pTab) ) cnt++;
        জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){ cnt++; }
      }
      যদি( cnt==0 ) চালু;
      যদি( pObjTab ) cnt++;
      a_মূল = sqlite3DbMallocRawNN(db, আকার(পূর্ণসংখ্যা)*(cnt+1));
      যদি( a_মূল==0 ) বিরতি;
      cnt = 0;
      যদি( pObjTab ) a_মূল[++cnt] = 0;
      জন্য(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_পরবর্তী(x)){
        Table *pTab = sqlite_Hash_তথ্য(x);
        সূচক *pIdx;
        যদি( tableSkipIntegrityCheck(pTab,pObjTab) ) চালু;
        যদি( HasRowid(pTab) ) a_মূল[++cnt] = pTab->tnum;
        জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
          a_মূল[++cnt] = pIdx->tnum;
        }
      }
      a_মূল[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_অস্থায়ী_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (অক্ষর*)a_মূল,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3VdbeAddOp1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3VdbeAddOp3(v, OP_Concat, 2, 3, 3);
      integrity_Check_ফলাফল_Row(v);
      sqlite3VdbeJumpHere(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3VdbeLoadString(v, 2, "wrong # of entries in index ");
      জন্য(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_পরবর্তী(x)){
        পূর্ণসংখ্যা iTab = 0;
        Table *pTab = sqlite_Hash_তথ্য(x);
        সূচক *pIdx;
        যদি( tableSkipIntegrityCheck(pTab,pObjTab) ) চালু;
        যদি( HasRowid(pTab) ){
          iTab = cnt++;
        }অন্যথা{
          iTab = cnt;
          জন্য(pIdx=pTab->p_সূচক; ALWAYS(pIdx); pIdx=pIdx->p_পরবর্তী){
            যদি( Is_Primary_চাবি_সূচক(pIdx) ) বিরতি;
            iTab++;
          }
        }
        জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
          যদি( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3VdbeAddOp3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 2, 3);
            integrity_Check_ফলাফল_Row(v);
            sqlite3VdbeJumpHere(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      জন্য(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_পরবর্তী(x)){
        Table *pTab = sqlite_Hash_তথ্য(x);
        সূচক *pIdx, *pPk;
        সূচক *pPrior = 0;      /* Previous index */
        পূর্ণসংখ্যা loop_শীর্ষ;
        পূর্ণসংখ্যা i_তথ্য_Cur, iIdxCur;
        পূর্ণসংখ্যা r1 = -1;
        পূর্ণসংখ্যা bStrict;            /* True for a STRICT table */
        পূর্ণসংখ্যা r2;                 /* Previous key for WITHOUT ROWID tables */
        পূর্ণসংখ্যা mxCol;              /* Maximum non-virtual column number */

        যদি( tableSkipIntegrityCheck(pTab,pObjTab) ) চালু;
        যদি( !IsOrdinaryTable(pTab) ) চালু;
        যদি( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }অন্যথা{
          pPk = sqlite3_Primary_চাবি_সূচক(pTab);
          r2 = sqlite3_Get_অস্থায়ী_Range(pParse, pPk->n_চাবি_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_চাবি_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_তথ্য_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        জন্য(j=0, pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        প্রতিজ্ঞা( pParse->nMem>=8+j );
        প্রতিজ্ঞা( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, i_তথ্য_Cur, 0); VdbeCoverage(v);
        loop_শীর্ষ = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        প্রতিজ্ঞা( !IsVirtual(pTab) );
        যদি( HasRowid(pTab) ){
          mxCol = -1;
          জন্য(j=0; j<pTab->nCol; j++){
            যদি( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          যদি( mxCol==pTab->iPKey ) mxCol--;
        }অন্যথা{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_চাবি_সূচক(pTab)->nColumn-1;
        }
        যদি( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, i_তথ্য_Cur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        যদি( !isQuick ){
          যদি( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            পূর্ণসংখ্যা a1;
            অক্ষর *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, i_তথ্য_Cur, 0,r2,pPk->n_চাবি_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ফলাফল_Row(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            জন্য(j=0; j<pPk->n_চাবি_Col; j++){
              sqlite3_রাশিমালা_Code_Load_সূচক_Column(pParse, pPk, i_তথ্য_Cur, j, r2+j);
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
        জন্য(j=0; j<pTab->nCol; j++){
          অক্ষর *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          পূর্ণসংখ্যা labelError;               /* Jump here to report an error */
          পূর্ণসংখ্যা labelOk;                  /* Jump here if all looks ok */
          পূর্ণসংখ্যা p1, p3, p4;               /* Operands to the OP_IsType opcode */
          পূর্ণসংখ্যা doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          যদি( j==pTab->iPKey ) চালু;
          যদি( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }অন্যথা{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          যদি( pCol->notNull==0 && !doTypeCheck ) চালু;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          যদি( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }অন্যথা{
            যদি( pCol->iDflt ){
              sqlite3_মান *p_Dflt_মান = 0;
              sqlite3_মান_From_রাশিমালা(db, sqlite3_Column_রাশিমালা_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_মান);
              যদি( p_Dflt_মান ){
                p4 = sqlite3_মান_type(p_Dflt_মান);
                sqlite3_মান_Free(p_Dflt_মান);
              }
            }
            p1 = i_তথ্য_Cur;
            যদি( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3_Table_Column_To_সূচক(sqlite3_Primary_চাবি_সূচক(pTab), j);
            }অন্যথা{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          যদি( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            পূর্ণসংখ্যা jmp3;
            পূর্ণসংখ্যা jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            যদি( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }অন্যথা{
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
            যদি( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }অন্যথা{
              /* VDBE byte code will fall thru */
            }
          }
          যদি( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            স্থির unsigned অক্ষর aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            প্রতিজ্ঞা( pCol->eCType>=1 && pCol->eCType<=আকার(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }অন্যথা যদি( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }অন্যথা যদি( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            যদি( p1>=0 ){
              sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, i_তথ্য_Cur, j, 3);
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
          integrity_Check_ফলাফল_Row(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        যদি( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          রাশিমালা_List *pCheck = sqlite3_রাশিমালা_List_Dup(db, pTab->pCheck, 0);
          যদি( db->mallocFailed==0 ){
            পূর্ণসংখ্যা addrCkFault = sqlite3VdbeMakeLabel(pParse);
            পূর্ণসংখ্যা addrCkOk = sqlite3VdbeMakeLabel(pParse);
            অক্ষর *zErr;
            পূর্ণসংখ্যা k;
            pParse->iSelfTab = i_তথ্য_Cur + 1;
            জন্য(k=pCheck->n_রাশিমালা-1; k>0; k--){
              sqlite3_রাশিমালা_If_False(pParse, pCheck->a[k].p_রাশিমালা, addrCkFault, 0);
            }
            sqlite3_রাশিমালা_If_True(pParse, pCheck->a[0].p_রাশিমালা, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ফলাফল_Row(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_রাশিমালা_List_Delete(db, pCheck);
        }
        যদি( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          জন্য(j=0, pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী, j++){
            পূর্ণসংখ্যা jmp2, jmp3, jmp4, jmp5, label6;
            পূর্ণসংখ্যা kk;
            পূর্ণসংখ্যা ckUniq = sqlite3VdbeMakeLabel(pParse);
            যদি( pPk==pIdx ) চালু;
            r1 = sqlite3_Generate_সূচক_চাবি(pParse, pIdx, i_তথ্য_Cur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_চাবি, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ধ্রুবক অক্ষর*)pIdx, P4_সূচক);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            integrity_Check_ফলাফল_Row(v);
            sqlite3VdbeAddOp2(v, OP_Goto, 0, ckUniq);

            sqlite3VdbeJumpHere(v, jmp2);
            sqlite3VdbeLoadString(v, 3, "row ");
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            sqlite3VdbeLoadString(v, 4, " missing from index ");
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_ফলাফল_Row(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            যদি( HasRowid(pTab) ){
              পূর্ণসংখ্যা jmp7;
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
            জন্য(kk=0; kk<pIdx->n_চাবি_Col; kk++){
              যদি( pIdx->azColl[kk]==sqlite3_Str_দ্বিমিক ) চালু;
              যদি( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            যদি( label6 ){
              পূর্ণসংখ্যা jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            যদি( Is_Unique_সূচক(pIdx) ){
              পূর্ণসংখ্যা uniqOk = sqlite3VdbeMakeLabel(pParse);
              পূর্ণসংখ্যা jmp6;
              জন্য(kk=0; kk<pIdx->n_চাবি_Col; kk++){
                পূর্ণসংখ্যা iCol = pIdx->aiColumn[kk];
                প্রতিজ্ঞা( iCol!=XN_ROWID && iCol<pTab->nCol );
                যদি( iCol>=0 && pTab->aCol[iCol].notNull ) চালু;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_পরবর্তী, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_চাবি_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_পরবর্তী, i_তথ্য_Cur, loop_শীর্ষ); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_শীর্ষ-1);
        যদি( pPk ){
          প্রতিজ্ঞা( !isQuick );
          sqlite3_Release_অস্থায়ী_Range(pParse, r2, pPk->n_চাবি_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      জন্য(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_পরবর্তী(x)){
        Table *pTab = sqlite_Hash_তথ্য(x);
        sqlite3_vtab *pVTab;
        পূর্ণসংখ্যা a1;
        যদি( tableSkipIntegrityCheck(pTab,pObjTab) ) চালু;
        যদি( IsOrdinaryTable(pTab) ) চালু;
        যদি( !IsVirtual(pTab) ) চালু;
        যদি( pTab->nCol<=0 ){
          ধ্রুবক অক্ষর *zMod = pTab->u.vtab.azArg[0];
          যদি( sqlite3HashFind(&db->aModule, zMod)==0 ) চালু;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        যদি( pTab->u.vtab.p==0 ) চালু;
        pVTab = pTab->u.vtab.p->pVtab;
        যদি( NEVER(pVTab==0) ) চালু;
        যদি( NEVER(pVTab->pModule==0) ) চালু;
        যদি( pVTab->pModule->iVersion<4 ) চালু;
        যদি( pVTab->pModule->xIntegrity==0 ) চালু;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_ফলাফল_Row(v);
        sqlite3VdbeJumpHere(v, a1);
        চালু;
      }
#endif
    }
    {
      স্থির ধ্রুবক পূর্ণসংখ্যা iLn = VDBE_OFFSET_LINENO(2);
      স্থির ধ্রুবক VdbeOpList endCode[] = {
        { OP_AddImm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_ফলাফল_Row,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3VdbeAddOpList(v, Array_আকার(endCode), endCode, iLn);
      যদি( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (অক্ষর*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  বিরতি;
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
  ক্ষেত্র PragTyp_ENCODING: {
    স্থির ধ্রুবক কাঠামো EncName {
      অক্ষর *zName;
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
    ধ্রুবক কাঠামো EncName *pEnc;
    যদি( !z_ডান ){    /* "PRAGMA encoding" */
      যদি( sqlite3ReadSchema(pParse) ) যাও pragma_out;
      প্রতিজ্ঞা( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      প্রতিজ্ঞা( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      প্রতিজ্ঞা( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }অন্যথা{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      যদি( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        জন্য(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          যদি( 0==sqlite3StrICmp(z_ডান, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            বিরতি;
          }
        }
        যদি( !pEnc->zName ){
          sqlite3_Error_বার্তা(pParse, "unsupported encoding: %s", z_ডান);
        }
      }
    }
  }
  বিরতি;
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
  ক্ষেত্র Prag_Typ_HEADER_মান: {
    পূর্ণসংখ্যা iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    যদি( z_ডান && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      স্থির ধ্রুবক VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_আকার(setCookie));
      aOp = sqlite3VdbeAddOpList(v, Array_আকার(setCookie), setCookie, 0);
      যদি( ONLY_IF_REALLOC_STRESS(aOp==0) ) বিরতি;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_ডান);
      aOp[1].p5 = 1;
      যদি( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }অন্যথা{
      /* Read the specified cookie value */
      স্থির ধ্রুবক VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ফলাফল_Row,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_আকার(readCookie));
      aOp = sqlite3VdbeAddOpList(v, Array_আকার(readCookie),readCookie,0);
      যদি( ONLY_IF_REALLOC_STRESS(aOp==0) ) বিরতি;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  বিরতি;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  ক্ষেত্র PragTyp_COMPILE_OPTIONS: {
    পূর্ণসংখ্যা i = 0;
    ধ্রুবক অক্ষর *zOpt;
    pParse->nMem = 1;
    যখন( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  বিরতি;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  ক্ষেত্র PragTyp_WAL_CHECKPOINT: {
    পূর্ণসংখ্যা iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    পূর্ণসংখ্যা eMode = SQLITE_CHECKPOINT_PASSIVE;
    যদি( z_ডান ){
      যদি( sqlite3StrICmp(z_ডান, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }অন্যথা যদি( sqlite3StrICmp(z_ডান, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }অন্যথা যদি( sqlite3StrICmp(z_ডান, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }অন্যথা যদি( sqlite3StrICmp(z_ডান, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, 1, 3);
  }
  বিরতি;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  ক্ষেত্র PragTyp_WAL_AUTOCHECKPOINT: {
    যদি( z_ডান ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_ডান));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  বিরতি;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  ক্ষেত্র PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    বিরতি;
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
  ক্ষেত্র PragTyp_OPTIMIZE: {
    পূর্ণসংখ্যা iDbLast;           /* Loop termination point for the schema loop */
    পূর্ণসংখ্যা iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    সূচক *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    অক্ষর *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    পূর্ণসংখ্যা nLimit;            /* Analysis limit to use */
    পূর্ণসংখ্যা nCheck = 0;        /* Number of tables to be optimized */
    পূর্ণসংখ্যা nBtree = 0;        /* Number of btrees to scan */
    পূর্ণসংখ্যা n_সূচক;            /* Number of indexes on the current table */

    যদি( z_ডান ){
      opMask = (u32)sqlite3Atoi(z_ডান);
      যদি( (opMask & 0x02)==0 ) বিরতি;
    }অন্যথা{
      opMask = 0xfffe;
    }
    যদি( (opMask & 0x10)==0 ){
      nLimit = 0;
    }অন্যথা যদি( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }অন্যথা{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    জন্য(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      যদি( iDb==1 ) চালু;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      জন্য(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_পরবর্তী(k)){
        pTab = (Table*)sqlite_Hash_তথ্য(k);

        /* This only works for ordinary tables */
        যদি( !IsOrdinaryTable(pTab) ) চালু;

        /* Do not scan system tables */
        যদি( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) চালু;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        n_সূচক = 0;
        জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
          n_সূচক++;
          যদি( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        যদি( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }অন্যথা যদি( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }অন্যথা যদি( pTab->p_সূচক!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }অন্যথা{
          /* Otherwise, we can skip this table */
          চালু;
        }

        nCheck++;
        যদি( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_সূচক+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        যদি( szThreshold>=0 ){
          ধ্রুবক LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_If_আকার_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }অন্যথা{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        যদি( opMask & 0x01 ){
          পূর্ণসংখ্যা r1 = sqlite3_Get_অস্থায়ী_Reg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ফলাফল_Row, r1, 1);
        }অন্যথা{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    যদি( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      পূর্ণসংখ্যা iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      যদি( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      জন্য(iAddr=0; iAddr<iEnd; iAddr++){
        যদি( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    বিরতি;
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
  /*case PragTyp_BUSY_TIMEOUT*/ পূর্বনির্ধারিত: {
    পূর্ণসংখ্যা n = 0;
    প্রতিজ্ঞা( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    যদি( z_ডান && sqlite3GetInt32(z_ডান,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    বিরতি;
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
  ক্ষেত্র PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    যদি( z_ডান && sqlite3DecOrHexToI64(z_ডান, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    বিরতি;
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
  ক্ষেত্র PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    যদি( z_ডান && sqlite3DecOrHexToI64(z_ডান, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      যদি( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    বিরতি;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  ক্ষেত্র PragTyp_THREADS: {
    sqlite3_int64 N;
    যদি( z_ডান
     && sqlite3DecOrHexToI64(z_ডান, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (পূর্ণসংখ্যা)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    বিরতি;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  ক্ষেত্র PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    যদি( z_ডান
     && sqlite3DecOrHexToI64(z_ডান, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (পূর্ণসংখ্যা)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    বিরতি;
  }

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  ক্ষেত্র PragTyp_LOCK_STATUS: {
    স্থির ধ্রুবক অক্ষর *ধ্রুবক azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    পূর্ণসংখ্যা i;
    pParse->nMem = 2;
    জন্য(i=0; i<db->nDb; i++){
      Btree *pBt;
      ধ্রুবক অক্ষর *zState = "unknown";
      পূর্ণসংখ্যা j;
      যদি( db->aDb[i].zDbSName==0 ) চালু;
      pBt = db->aDb[i].pBt;
      যদি( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }অন্যথা যদি( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    বিরতি;
  }
#endif

#যদি defined(SQLITE_ENABLE_CEROD)
  ক্ষেত্র PragTyp_ACTIVATE_EXTENSIONS: যদি( z_ডান ){
    যদি( sqlite3StrNICmp(z_ডান, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_ডান[6]);
    }
  }
  বিরতি;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  যদি( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_ডান ){
    sqlite3_Vdbe_Verify_No_ফলাফল_Row(v);
  }

pragma_out:
  sqlite3DbFree(db, z_বাম);
  sqlite3DbFree(db, z_ডান);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
প্রকারলেখ কাঠামো PragmaVtab PragmaVtab;
প্রকারলেখ কাঠামো PragmaVtabCursor PragmaVtabCursor;
কাঠামো PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ধ্রুবক PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
কাঠামো PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  অক্ষর *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
স্থির পূর্ণসংখ্যা pragmaVtabConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  ধ্রুবক PragmaName *pPragma = (ধ্রুবক PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i, j;
  অক্ষর cSep = '(';
  StrAccum acc;
  অক্ষর zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, আকার(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  জন্য(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  যদি( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  যদি( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  যদি( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  প্রতিজ্ঞা( strlen(zBuf) < আকার(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  যদি( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(আকার(PragmaVtab));
    যদি( pTab==0 ){
      rc = SQLITE_NOMEM;
    }অন্যথা{
      memset(pTab, 0, আকার(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }অন্যথা{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ফেরত rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
স্থির পূর্ণসংখ্যা pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  ফেরত SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
স্থির পূর্ণসংখ্যা pragma_Vtab_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ধ্রুবক কাঠামো sqlite3_সূচক_constraint *pConstraint;
  পূর্ণসংখ্যা i, j;
  পূর্ণসংখ্যা seen[2];

  pIdxInfo->estimatedCost = (দ্বিগুণ)1;
  যদি( pTab->nHidden==0 ){ ফেরত SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  জন্য(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    যদি( pConstraint->iColumn < pTab->iHidden ) চালু;
    যদি( pConstraint->op!=SQLITE_সূচক_CONSTRAINT_EQ ) চালু;
    যদি( pConstraint->usable==0 ) ফেরত SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    প্রতিজ্ঞা( j < 2 );
    seen[j] = i+1;
  }
  যদি( seen[0]==0 ){
    pIdxInfo->estimatedCost = (দ্বিগুণ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    ফেরত SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_সূচক = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (দ্বিগুণ)20;
  pIdxInfo->estimatedRows = 20;
  যদি( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_সূচক = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  ফেরত SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
স্থির পূর্ণসংখ্যা pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(আকার(*pCsr));
  যদি( pCsr==0 ) ফেরত SQLITE_NOMEM;
  memset(pCsr, 0, আকার(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  ফেরত SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
স্থির শূন্য pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  পূর্ণসংখ্যা i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  জন্য(i=0; i<Array_আকার(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
স্থির পূর্ণসংখ্যা pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  ফেরত SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
স্থির পূর্ণসংখ্যা pragma_Vtab_পরবর্তী(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  প্রতিজ্ঞা( pCsr->pPragma );
  যদি( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  ফেরত rc;
}

/*
** Pragma virtual table module xFilter method.
*/
স্থির পূর্ণসংখ্যা pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা i, j;
  StrAccum acc;
  অক্ষর *zSql;

  UNUSED_PARAMETER(idx_অঙ্ক);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  জন্য(i=0; i<argc; i++, j++){
    ধ্রুবক অক্ষর *zText = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
    প্রতিজ্ঞা( j<Array_আকার(pCsr->azArg) );
    প্রতিজ্ঞা( pCsr->azArg[j]==0 );
    যদি( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      যদি( pCsr->azArg[j]==0 ){
        ফেরত SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_দৈর্ঘ্য]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  যদি( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  যদি( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  যদি( zSql==0 ) ফেরত SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  যদি( rc!=SQLITE_OK ){
    pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    ফেরত rc;
  }
  ফেরত pragma_Vtab_পরবর্তী(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
স্থির পূর্ণসংখ্যা pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ফেরত (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
স্থির পূর্ণসংখ্যা pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  পূর্ণসংখ্যা i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  যদি( i<pTab->iHidden ){
    sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCsr->pPragma, i));
  }অন্যথা{
    sqlite3_ফলাফল_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  ফেরত SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
স্থির পূর্ণসংখ্যা pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  ফেরত SQLITE_OK;
}

/* The pragma virtual table object */
স্থির ধ্রুবক sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_Best_সূচক,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_পরবর্তী,              /* xNext - advance a cursor */
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, ধ্রুবক অক্ষর *zName){
  ধ্রুবক PragmaName *pName;
  প্রতিজ্ঞা( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  যদি( pName==0 ) ফেরত 0;
  যদি( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) ফেরত 0;
  প্রতিজ্ঞা( sqlite3HashFind(&db->aModule, zName)==0 );
  ফেরত sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (শূন্য*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
