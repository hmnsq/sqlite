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
#समावेश "sqliteInt.h"

#अगर !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  अगर defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  अन्यथा
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#पूर्ण_अगर

/***************************************************************************
** The "pragma.h" include file is an automatically generated file that
** that includes the PragType_XXXX macro definitions and the aPragmaName[]
** object.  This ensures that the aPragmaName[] table is arranged in
** lexicographical order to facility a binary search of the pragma name.
** Do not edit pragma.h directly.  Edit and rerun the script in at
** ../tool/mkpragmatab.tcl. */
#समावेश "pragma.h"

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
#अगर_अघोषित SQLITE_DEFAULT_OPTIMIZE_LIMIT
# define SQLITE_DEFAULT_OPTIMIZE_LIMIT 2000
#पूर्ण_अगर

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
अटल u8 getSafetyLevel(स्थिर अक्षर *z, पूर्णांक omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  अटल स्थिर अक्षर zText[] = "onoffalseyestruextrafull";
  अटल स्थिर u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  अटल स्थिर u8 i_लंबाई[] = {2, 2, 3,  5,    3,   4,   5,    4};
  अटल स्थिर u8 i_मूल्य[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  पूर्णांक i, n;
  अगर( sqlite3Isdigit(*z) ){
    वापस sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  क्रम(i=0; i<Array_आकार(i_लंबाई); i++){
    अगर( i_लंबाई[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_मूल्य[i]<=1)
    ){
      वापस i_मूल्य[i];
    }
  }
  वापस dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(स्थिर अक्षर *z, u8 dflt){
  वापस getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#अगर !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
अटल पूर्णांक getLockingMode(स्थिर अक्षर *z){
  अगर( z ){
    अगर( 0==sqlite3StrICmp(z, "exclusive") ) वापस PAGER_LOCKINGMODE_EXCLUSIVE;
    अगर( 0==sqlite3StrICmp(z, "normal") ) वापस PAGER_LOCKINGMODE_NORMAL;
  }
  वापस PAGER_LOCKINGMODE_QUERY;
}

#अगर_अघोषित SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
अटल पूर्णांक getAutoVacuum(स्थिर अक्षर *z){
  पूर्णांक i;
  अगर( 0==sqlite3StrICmp(z, "none") ) वापस BTREE_AUTOVACUUM_NONE;
  अगर( 0==sqlite3StrICmp(z, "full") ) वापस BTREE_AUTOVACUUM_FULL;
  अगर( 0==sqlite3StrICmp(z, "incremental") ) वापस BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  वापस (u8)((i>=0&&i<=2)?i:0);
}
#पूर्ण_अगर /* ifndef SQLITE_OMIT_AUTOVACUUM */

#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
अटल पूर्णांक get_अस्थायी_Store(स्थिर अक्षर *z){
  अगर( z[0]>='0' && z[0]<='2' ){
    वापस z[0] - '0';
  }अन्यथा अगर( sqlite3StrICmp(z, "file")==0 ){
    वापस 1;
  }अन्यथा अगर( sqlite3StrICmp(z, "memory")==0 ){
    वापस 2;
  }अन्यथा{
    वापस 0;
  }
}
#पूर्ण_अगर /* SQLITE_PAGER_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
अटल पूर्णांक invalidate_अस्थायी_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  अगर( db->aDb[1].pBt!=0 ){
    अगर( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_संदेश(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      वापस SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_PAGER_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
अटल पूर्णांक change_अस्थायी_Storage(Parse *pParse, स्थिर अक्षर *zStorageType){
  पूर्णांक ts = get_अस्थायी_Store(zStorageType);
  sqlite3 *db = pParse->db;
  अगर( db->अस्थायी_store==ts ) वापस SQLITE_OK;
  अगर( invalidate_अस्थायी_Storage( pParse ) != SQLITE_OK ){
    वापस SQLITE_ERROR;
  }
  db->अस्थायी_store = (u8)ts;
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
अटल व्योम set_Pragma_परिणाम_Column_Names(
  Vdbe *v,                     /* The query under construction */
  स्थिर PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_अंक_Cols(v, n==0 ? 1 : n);
  अगर( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }अन्यथा{
    पूर्णांक i, j;
    क्रम(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
अटल व्योम returnSingleInt(Vdbe *v, i64 मूल्य){
  sqlite3VdbeAddInt64(v, 1, मूल्य);
  sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 1);
}

/*
** Generate code to return a single text value.
*/
अटल व्योम returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  स्थिर अक्षर *z_मूल्य      /* Value to be returned */
){
  अगर( z_मूल्य ){
    sqlite3VdbeLoadString(v, 1, (स्थिर अक्षर*)z_मूल्य);
    sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
अटल व्योम setAllPagerFlags(sqlite3 *db){
  अगर( db->autoCommit ){
    Db *pDb = db->aDb;
    पूर्णांक n = db->nDb;
    निश्चित( SQLITE_FullFSync==PAGER_FULLFSYNC );
    निश्चित( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    निश्चित( SQLITE_CacheSpill==PAGER_CACHESPILL );
    निश्चित( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    निश्चित( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    जबतक( (n--) > 0 ){
      अगर( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#अन्यथा
# define setAllPagerFlags(X)  /* no-op */
#पूर्ण_अगर


/*
** Return a human-readable name for a constraint resolution action.
*/
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
अटल स्थिर अक्षर *actionName(u8 action){
  स्थिर अक्षर *zName;
  चयन( action ){
    हाल OE_SetNull:  zName = "SET NULL";        अवरोध;
    हाल OE_SetDflt:  zName = "SET DEFAULT";     अवरोध;
    हाल OE_Cascade:  zName = "CASCADE";         अवरोध;
    हाल OE_Restrict: zName = "RESTRICT";        अवरोध;
    शेष:          zName = "NO ACTION"; 
                      निश्चित( action==OE_None ); अवरोध;
  }
  वापस zName;
}
#पूर्ण_अगर


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
स्थिर अक्षर *sqlite3JournalModename(पूर्णांक eMode){
  अटल अक्षर * स्थिर azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#अगर_अघोषित SQLITE_OMIT_WAL
     , "wal"
#पूर्ण_अगर
  };
  निश्चित( PAGER_JOURNALMODE_DELETE==0 );
  निश्चित( PAGER_JOURNALMODE_PERSIST==1 );
  निश्चित( PAGER_JOURNALMODE_OFF==2 );
  निश्चित( PAGER_JOURNALMODE_TRUNCATE==3 );
  निश्चित( PAGER_JOURNALMODE_MEMORY==4 );
  निश्चित( PAGER_JOURNALMODE_WAL==5 );
  निश्चित( eMode>=0 && eMode<=Array_आकार(azModeName) );

  अगर( eMode==Array_आकार(azModeName) ) वापस 0;
  वापस azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
अटल स्थिर PragmaName *pragmaLocate(स्थिर अक्षर *zName){
  पूर्णांक upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_आकार(aPragmaName)-1;
  जबतक( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    अगर( rc==0 ) अवरोध;
    अगर( rc<0 ){
      upr = mid - 1;
    }अन्यथा{
      lwr = mid + 1;
    }
  }
  वापस lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
अटल व्योम pragma_Funclist_पंक्ति(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  पूर्णांक isBuiltin,         /* True if this is a built-in function */
  पूर्णांक showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  अगर( showInternFuncs ) mask = 0xffffffff;
  क्रम(; p; p=p->p_अग्र){
    स्थिर अक्षर *zType;
    अटल स्थिर अक्षर *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    निश्चित( SQLITE_FUNC_ENCMASK==0x3 );
    निश्चित( म_भेद(azEnc[SQLITE_UTF8],"utf8")==0 );
    निश्चित( म_भेद(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    निश्चित( म_भेद(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    अगर( p->xSFunc==0 ) जारी;
    अगर( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      जारी;
    }   
    अगर( p->x_मूल्य!=0 ){
      zType = "w";
    }अन्यथा अगर( p->xFinalize!=0 ){
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
अटल पूर्णांक integrity_Check_परिणाम_Row(Vdbe *v){
  पूर्णांक addr;
  sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  वापस addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
अटल पूर्णांक tableSkipIntegrityCheck(स्थिर Table *pTab, स्थिर Table *pObjTab){
  अगर( pObjTab ){
    वापस pTab!=pObjTab;
  }अन्यथा{
    वापस (pTab->tabFlags & TF_Imposter)!=0;
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
व्योम sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_मूल्य,      /* Token for <value>, or NULL */
  पूर्णांक minusFlag       /* True if a '-' sign preceded <value> */
){
  अक्षर *z_बायाँ = 0;       /* Nul-terminated UTF-8 string <id> */
  अक्षर *z_दायाँ = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  स्थिर अक्षर *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  अक्षर *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  पूर्णांक iDb;               /* Database index for <database> */
  पूर्णांक rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  स्थिर PragmaName *pPragma;   /* The pragma */

  अगर( v==0 ) वापस;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  अगर( iDb<0 ) वापस;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  अगर( iDb==1 && sqlite3_Open_अस्थायी_Database(pParse) ){
    वापस;
  }

  z_बायाँ = sqlite3NameFromToken(db, pId);
  अगर( !z_बायाँ ) वापस;
  अगर( minusFlag ){
    z_दायाँ = sqlite3MPrintf(db, "-%T", p_मूल्य);
  }अन्यथा{
    z_दायाँ = sqlite3NameFromToken(db, p_मूल्य);
  }

  निश्चित( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  अगर( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_बायाँ, z_दायाँ, zDb) ){
    जाओ pragma_out;
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
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (व्योम*)aFcntl);
  अगर( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_अंक_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    जाओ pragma_out;
  }
  अगर( rc!=SQLITE_NOTFOUND ){
    अगर( aFcntl[0] ){
      sqlite3_Error_संदेश(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    जाओ pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_बायाँ);
  अगर( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    जाओ pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  अगर( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    अगर( sqlite3ReadSchema(pParse) ) जाओ pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  अगर( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_दायाँ==0)
  ){
    set_Pragma_परिणाम_Column_Names(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  चयन( pPragma->ePragTyp ){
 
#अगर !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  हाल Prag_Typ_DEFAULT_CACHE_आकार: {
    अटल स्थिर पूर्णांक iLn = VDBE_OFFSET_LINENO(2);
    अटल स्थिर VdbeOpList get_Cache_आकार[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_आकार},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_Subtract,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_परिणाम_Row,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3VdbeUsesBtree(v, iDb);
    अगर( !z_दायाँ ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(get_Cache_आकार));
      aOp = sqlite3VdbeAddOpList(v, Array_आकार(get_Cache_आकार), get_Cache_आकार, iLn);
      अगर( ONLY_IF_REALLOC_STRESS(aOp==0) ) अवरोध;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_आकार;
    }अन्यथा{
      पूर्णांक आकार = sqlite3AbsInt32(sqlite3Atoi(z_दायाँ));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_आकार, आकार);
      निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_आकार = आकार;
      sqlite3_Btree_Set_Cache_आकार(pDb->pBt, pDb->pSchema->cache_आकार);
    }
    अवरोध;
  }
#पूर्ण_अगर /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#अगर !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  हाल Prag_Typ_PAGE_आकार: {
    Btree *pBt = pDb->pBt;
    निश्चित( pBt!=0 );
    अगर( !z_दायाँ ){
      पूर्णांक आकार = ALWAYS(pBt) ? sqlite3_Btree_Get_Page_आकार(pBt) : 0;
      returnSingleInt(v, आकार);
    }अन्यथा{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->अग्र_Pagesize = sqlite3Atoi(z_दायाँ);
      अगर( SQLITE_NOMEM==sqlite3_Btree_Set_Page_आकार(pBt, db->अग्र_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    अवरोध;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  हाल PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    पूर्णांक b = -1;
    निश्चित( pBt!=0 );
    अगर( z_दायाँ ){
      अगर( sqlite3_stricmp(z_दायाँ, "fast")==0 ){
        b = 2;
      }अन्यथा{
        b = sqlite3GetBoolean(z_दायाँ, 0);
      }
    }
    अगर( pId2->n==0 && b>=0 ){
      पूर्णांक ii;
      क्रम(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    अवरोध;
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
  हाल Prag_Typ_PAGE_गिनती: {
    पूर्णांक iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    अगर( sqlite3Tolower(z_बायाँ[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }अन्यथा{
      अगर( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ,&x)==0 ){
        अगर( x<0 ) x = 0;
        अन्यथा अगर( x>0xfffffffe ) x = 0xfffffffe;
      }अन्यथा{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (पूर्णांक)x);
    }
    sqlite3VdbeAddOp2(v, OP_परिणाम_Row, iReg, 1);
    अवरोध;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  हाल PragTyp_LOCKING_MODE: {
    स्थिर अक्षर *zRet = "normal";
    पूर्णांक eMode = getLockingMode(z_दायाँ);

    अगर( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }अन्यथा{
      Pager *pPager;
      अगर( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        पूर्णांक ii;
        निश्चित(pDb==&db->aDb[0]);
        क्रम(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    निश्चित( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    अगर( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    अवरोध;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  हाल PragTyp_JOURNAL_MODE: {
    पूर्णांक eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    पूर्णांक ii;           /* Loop counter */

    अगर( z_दायाँ==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }अन्यथा{
      स्थिर अक्षर *zMode;
      पूर्णांक n = sqlite3Strlen30(z_दायाँ);
      क्रम(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        अगर( sqlite3StrNICmp(z_दायाँ, zMode, n)==0 ) अवरोध;
      }
      अगर( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      अगर( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    अगर( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    क्रम(ii=db->nDb-1; ii>=0; ii--){
      अगर( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 1);
    अवरोध;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  हाल Prag_Typ_JOURNAL_आकार_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    अगर( z_दायाँ ){
      sqlite3DecOrHexToI64(z_दायाँ, &iLimit);
      अगर( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3_Pager_Journal_आकार_Limit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    अवरोध;
  }

#पूर्ण_अगर /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#अगर_अघोषित SQLITE_OMIT_AUTOVACUUM
  हाल PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    निश्चित( pBt!=0 );
    अगर( !z_दायाँ ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }अन्यथा{
      पूर्णांक eAuto = getAutoVacuum(z_दायाँ);
      निश्चित( eAuto>=0 && eAuto<=2 );
      db->अग्र_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      अगर( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        अटल स्थिर पूर्णांक iLn = VDBE_OFFSET_LINENO(2);
        अटल स्थिर VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_मूल_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        पूर्णांक iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, Array_आकार(setMeta6), setMeta6, iLn);
        अगर( ONLY_IF_REALLOC_STRESS(aOp==0) ) अवरोध;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    अवरोध;
  }
#पूर्ण_अगर

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#अगर_अघोषित SQLITE_OMIT_AUTOVACUUM
  हाल PragTyp_INCREMENTAL_VACUUM: {
    पूर्णांक iLimit = 0, addr;
    अगर( z_दायाँ==0 || !sqlite3GetInt32(z_दायाँ, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_परिणाम_Row, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    अवरोध;
  }
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_PAGER_PRAGMAS
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
  हाल Prag_Typ_CACHE_आकार: {
    निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
    अगर( !z_दायाँ ){
      returnSingleInt(v, pDb->pSchema->cache_आकार);
    }अन्यथा{
      पूर्णांक आकार = sqlite3Atoi(z_दायाँ);
      pDb->pSchema->cache_आकार = आकार;
      sqlite3_Btree_Set_Cache_आकार(pDb->pBt, pDb->pSchema->cache_आकार);
    }
    अवरोध;
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
  हाल PragTyp_CACHE_SPILL: {
    निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
    अगर( !z_दायाँ ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_Set_Spill_आकार(pDb->pBt,0));
    }अन्यथा{
      पूर्णांक आकार = 1;
      अगर( sqlite3GetInt32(z_दायाँ, &आकार) ){
        sqlite3_Btree_Set_Spill_आकार(pDb->pBt, आकार);
      }
      अगर( sqlite3GetBoolean(z_दायाँ, आकार!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }अन्यथा{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    अवरोध;
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
  हाल Prag_Typ_MMAP_आकार: {
    sqlite3_int64 sz;
#अगर SQLITE_MAX_MMAP_आकार>0
    निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
    अगर( z_दायाँ ){
      पूर्णांक ii;
      sqlite3DecOrHexToI64(z_दायाँ, &sz);
      अगर( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      अगर( pId2->n==0 ) db->szMmap = sz;
      क्रम(ii=db->nDb-1; ii>=0; ii--){
        अगर( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_आकार, &sz);
#अन्यथा
    sz = 0;
    rc = SQLITE_OK;
#पूर्ण_अगर
    अगर( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }अन्यथा अगर( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    अवरोध;
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
  हाल Prag_Typ_अस्थायी_STORE: {
    अगर( !z_दायाँ ){
      returnSingleInt(v, db->अस्थायी_store);
    }अन्यथा{
      change_अस्थायी_Storage(pParse, z_दायाँ);
    }
    अवरोध;
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
  हाल Prag_Typ_अस्थायी_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    अगर( !z_दायाँ ){
      returnSingleText(v, sqlite3_अस्थायी_directory);
    }अन्यथा{
#अगर_अघोषित SQLITE_OMIT_WSD
      अगर( z_दायाँ[0] ){
        पूर्णांक res;
        rc = sqlite3OsAccess(db->pVfs, z_दायाँ, SQLITE_ACCESS_READWRITE, &res);
        अगर( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_संदेश(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          जाओ pragma_out;
        }
      }
      अगर( SQLITE_अस्थायी_STORE==0
       || (SQLITE_अस्थायी_STORE==1 && db->अस्थायी_store<=1)
       || (SQLITE_अस्थायी_STORE==2 && db->अस्थायी_store==1)
      ){
        invalidate_अस्थायी_Storage(pParse);
      }
      sqlite3_free(sqlite3_अस्थायी_directory);
      अगर( z_दायाँ[0] ){
        sqlite3_अस्थायी_directory = sqlite3_mprintf("%s", z_दायाँ);
      }अन्यथा{
        sqlite3_अस्थायी_directory = 0;
      }
#पूर्ण_अगर /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    अवरोध;
  }

#अगर SQLITE_OS_WIN
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
  हाल Prag_Typ_तथ्य_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    अगर( !z_दायाँ ){
      returnSingleText(v, sqlite3_तथ्य_directory);
    }अन्यथा{
#अगर_अघोषित SQLITE_OMIT_WSD
      अगर( z_दायाँ[0] ){
        पूर्णांक res;
        rc = sqlite3OsAccess(db->pVfs, z_दायाँ, SQLITE_ACCESS_READWRITE, &res);
        अगर( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_संदेश(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          जाओ pragma_out;
        }
      }
      sqlite3_free(sqlite3_तथ्य_directory);
      अगर( z_दायाँ[0] ){
        sqlite3_तथ्य_directory = sqlite3_mprintf("%s", z_दायाँ);
      }अन्यथा{
        sqlite3_तथ्य_directory = 0;
      }
#पूर्ण_अगर /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    अवरोध;
  }
#पूर्ण_अगर

#अगर SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  हाल PragTyp_LOCK_PROXY_FILE: {
    अगर( !z_दायाँ ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      अक्षर *proxy_file_path = शून्य;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }अन्यथा{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      पूर्णांक res;
      अगर( z_दायाँ[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_दायाँ);
      } अन्यथा {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     शून्य);
      }
      अगर( res!=SQLITE_OK ){
        sqlite3_Error_संदेश(pParse, "failed to set lock proxy file");
        जाओ pragma_out;
      }
    }
    अवरोध;
  }
#पूर्ण_अगर /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  हाल PragTyp_SYNCHRONOUS: {
    अगर( !z_दायाँ ){
      returnSingleInt(v, pDb->safety_level-1);
    }अन्यथा{
      अगर( !db->autoCommit ){
        sqlite3_Error_संदेश(pParse,
            "Safety level may not be changed inside a transaction");
      }अन्यथा अगर( iDb!=1 ){
        पूर्णांक iLevel;
        अगर( sqlite3Isdigit(z_दायाँ[0]) ){
          iLevel = sqlite3Atoi(z_दायाँ);
        }अन्यथा{
          iLevel = getSafetyLevel(z_दायाँ,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        अगर( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    अवरोध;
  }
#पूर्ण_अगर /* SQLITE_OMIT_PAGER_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_FLAG_PRAGMAS
  हाल PragTyp_FLAG: {
    अगर( z_दायाँ==0 ){
      set_Pragma_परिणाम_Column_Names(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }अन्यथा{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      अगर( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_कुंजियाँ);
      }

      अगर( sqlite3GetBoolean(z_दायाँ, 0) ){
        अगर( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }अन्यथा{
        db->flags &= ~mask;
        अगर( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        अगर( (mask & SQLITE_WriteSchema)!=0
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
    अवरोध;
  }
#पूर्ण_अगर /* SQLITE_OMIT_FLAG_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_SCHEMA_PRAGMAS
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
  हाल PragTyp_TABLE_INFO: अगर( z_दायाँ ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_दायाँ, zDb);
    अगर( pTab ){
      पूर्णांक i, k;
      पूर्णांक nHidden = 0;
      Column *pCol;
      सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      क्रम(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        पूर्णांक isHidden = 0;
        स्थिर व्यंजक *p_Col_व्यंजक;
        अगर( pCol->colFlags & COLFLAG_NOINSERT ){
          अगर( pPragma->iArg==0 ){
            nHidden++;
            जारी;
          }
          अगर( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }अन्यथा अगर( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }अन्यथा{ निश्चित( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        अगर( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }अन्यथा अगर( pPk==0 ){
          k = 1;
        }अन्यथा{
          क्रम(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_व्यंजक = sqlite3_Column_व्यंजक(pTab,pCol);
        निश्चित( p_Col_व्यंजक==0 || p_Col_व्यंजक->op==TK_SPAN || isHidden>=2 );
        निश्चित( p_Col_व्यंजक==0 || !व्यंजक_Has_Property(p_Col_व्यंजक, EP_Int_मूल्य)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_व्यंजक==0) ? 0 : p_Col_व्यंजक->u.zToken,
               k,
               isHidden);
      }
    }
  }
  अवरोध;

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
  हाल PragTyp_TABLE_LIST: {
    पूर्णांक ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    क्रम(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      पूर्णांक initNCol;
      अगर( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) जारी;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_गिनती(pHash);
      जबतक( initNCol-- ){
        क्रम(k=sqliteHashFirst(pHash); 1; k=sqlite_Hash_अग्र(k) ){
          Table *pTab;
          अगर( k==0 ){ initNCol = 0; अवरोध; }
          pTab = sqlite_Hash_तथ्य(k);
          अगर( pTab->nCol==0 ){
            अक्षर *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            अगर( zSql ){
              sqlite3_stmt *pDummy = 0;
              (व्योम)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (व्योम)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            अगर( db->mallocFailed ){
              sqlite3_Error_संदेश(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            अवरोध;
          }
        }
      }

      क्रम(k=sqliteHashFirst(pHash); k; k=sqlite_Hash_अग्र(k) ){
        Table *pTab = sqlite_Hash_तथ्य(k);
        स्थिर अक्षर *zType;
        अगर( z_दायाँ && sqlite3_stricmp(z_दायाँ, pTab->zName)!=0 ) जारी;
        अगर( IsView(pTab) ){
          zType = "view";
        }अन्यथा अगर( IsVirtual(pTab) ){
          zType = "virtual";
        }अन्यथा अगर( pTab->tabFlags & TF_Shadow ){
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
  अवरोध;

#अगर_घोषित SQLITE_DEBUG
  हाल PragTyp_STATS: {
    सूचकांक *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    क्रम(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqlite_Hash_अग्र(i)){
      Table *pTab = sqlite_Hash_तथ्य(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 5);
      }
    }
  }
  अवरोध;
#पूर्ण_अगर

  हाल Prag_Typ_सूचकांक_INFO: अगर( z_दायाँ ){
    सूचकांक *pIdx;
    Table *pTab;
    pIdx = sqlite3_Find_सूचकांक(db, z_दायाँ, zDb);
    अगर( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_दायाँ, zDb);
      अगर( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      }
    }
    अगर( pIdx ){
      पूर्णांक iIdxDb = sqlite3_Schema_To_सूचकांक(db, pIdx->pSchema);
      पूर्णांक i;
      पूर्णांक mx;
      अगर( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }अन्यथा{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_कुंजी_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      निश्चित( pParse->nMem<=pPragma->nPragCName );
      क्रम(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        अगर( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_कुंजी_Col);
        }
        sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, pParse->nMem);
      }
    }
  }
  अवरोध;

  हाल Prag_Typ_सूचकांक_LIST: अगर( z_दायाँ ){
    सूचकांक *pIdx;
    Table *pTab;
    पूर्णांक i;
    pTab = sqlite3FindTable(db, z_दायाँ, zDb);
    अगर( pTab ){
      पूर्णांक iTabDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      क्रम(pIdx=pTab->p_सूचकांक, i=0; pIdx; pIdx=pIdx->p_अग्र, i++){
        स्थिर अक्षर *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_सूचकांक(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  अवरोध;

  हाल PragTyp_DATABASE_LIST: {
    पूर्णांक i;
    pParse->nMem = 3;
    क्रम(i=0; i<db->nDb; i++){
      अगर( db->aDb[i].pBt==0 ) जारी;
      निश्चित( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  अवरोध;

  हाल PragTyp_COLLATION_LIST: {
    पूर्णांक i = 0;
    HashElem *p;
    pParse->nMem = 2;
    क्रम(p=sqliteHashFirst(&db->aCollSeq); p; p=sqlite_Hash_अग्र(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_तथ्य(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  अवरोध;

#अगर_अघोषित SQLITE_OMIT_INTROSPECTION_PRAGMAS
  हाल PragTyp_FUNCTION_LIST: {
    पूर्णांक i;
    HashElem *j;
    FuncDef *p;
    पूर्णांक showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    क्रम(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      क्रम(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        निश्चित( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_पंक्ति(v, p, 1, showInternFunc);
      }
    }
    क्रम(j=sqliteHashFirst(&db->aFunc); j; j=sqlite_Hash_अग्र(j)){
      p = (FuncDef*)sqlite_Hash_तथ्य(j);
      निश्चित( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_पंक्ति(v, p, 0, showInternFunc);
    }
  }
  अवरोध;

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  हाल PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    क्रम(j=sqliteHashFirst(&db->aModule); j; j=sqlite_Hash_अग्र(j)){
      Module *pMod = (Module*)sqlite_Hash_तथ्य(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

  हाल PragTyp_PRAGMA_LIST: {
    पूर्णांक i;
    क्रम(i=0; i<Array_आकार(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_INTROSPECTION_PRAGMAS */

#पूर्ण_अगर /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
  हाल Prag_Typ_FOREIGN_कुंजी_LIST: अगर( z_दायाँ ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_दायाँ, zDb);
    अगर( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      अगर( pFK ){
        पूर्णांक iTabDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
        पूर्णांक i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        जबतक(pFK){
          पूर्णांक j;
          क्रम(j=0; j<pFK->nCol; j++){
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
          pFK = pFK->p_अग्र_From;
        }
      }
    }
  }
  अवरोध;
#पूर्ण_अगर /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  हाल Prag_Typ_FOREIGN_कुंजी_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    सूचकांक *pIdx;           /* Index in the parent table */
    पूर्णांक i;                 /* Loop counter:  Foreign key number for pTab */
    पूर्णांक j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    पूर्णांक x;                 /* result variable */
    पूर्णांक reg_परिणाम;         /* 3 registers to hold a result row */
    पूर्णांक regRow;            /* Registers to hold a row from pTab */
    पूर्णांक addr_शीर्ष;           /* Top of a loop checking foreign keys */
    पूर्णांक addrOk;            /* Jump here if the key is OK */
    पूर्णांक *aiCols;           /* child to parent column mapping */

    reg_परिणाम = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    जबतक( k ){
      अगर( z_दायाँ ){
        pTab = sqlite3LocateTable(pParse, 0, z_दायाँ, zDb);
        k = 0;
      }अन्यथा{
        pTab = (Table*)sqlite_Hash_तथ्य(k);
        k = sqlite_Hash_अग्र(k);
      }
      अगर( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) जारी;
      iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, reg_परिणाम, pTab->zName);
      निश्चित( IsOrdinaryTable(pTab) );
      क्रम(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_अग्र_From){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        अगर( pParent==0 ) जारी;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3_Fk_Locate_सूचकांक(pParse, pParent, pFK, &pIdx, 0);
        अगर( x==0 ){
          अगर( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }अन्यथा{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIdx);
          }
        }अन्यथा{
          k = 0;
          अवरोध;
        }
      }
      निश्चित( pParse->nErr>0 || pFK==0 );
      अगर( pFK ) अवरोध;
      अगर( pParse->nTab<i ) pParse->nTab = i;
      addr_शीर्ष = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      निश्चित( IsOrdinaryTable(pTab) );
      क्रम(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_अग्र_From){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        अगर( pParent ){
          x = sqlite3_Fk_Locate_सूचकांक(pParse, pParent, pFK, &pIdx, &aiCols);
          निश्चित( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        क्रम(j=0; j<pFK->nCol; j++){
          पूर्णांक iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        अगर( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3_सूचकांक_Affinity_Str(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }अन्यथा अगर( pParent ){
          पूर्णांक jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          निश्चित( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        अगर( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, reg_परिणाम+1);
        }अन्यथा{
          sqlite3VdbeAddOp2(v, OP_Null, 0, reg_परिणाम+1);
        }
        sqlite3VdbeMultiLoad(v, reg_परिणाम+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_परिणाम_Row, reg_परिणाम, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_अग्र, 0, addr_शीर्ष+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_शीर्ष);
    }
  }
  अवरोध;
#पूर्ण_अगर /* !defined(SQLITE_OMIT_TRIGGER) */
#पूर्ण_अगर /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#अगर_अघोषित SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  हाल PragTyp_CASE_SENSITIVE_LIKE: {
    अगर( z_दायाँ ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_दायाँ, 0));
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#अगर_अघोषित SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_INTEGRITY_CHECK
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
  हाल PragTyp_INTEGRITY_CHECK: {
    पूर्णांक i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    पूर्णांक isQuick = (sqlite3Tolower(z_बायाँ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    निश्चित( iDb>=0 );
    निश्चित( iDb==0 || pId2->z );
    अगर( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    अगर( z_दायाँ ){
      अगर( sqlite3GetInt32(p_मूल्य->z, &mxErr) ){
        अगर( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }अन्यथा{
        pObjTab = sqlite3LocateTable(pParse, 0, z_दायाँ,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    क्रम(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      पूर्णांक *a_मूल;      /* Array of root page numbers of all btrees */
      पूर्णांक cnt = 0;     /* Number of entries in aRoot[] */

      अगर( OMIT_TEMPDB && i==1 ) जारी;
      अगर( iDb>=0 && i!=iDb ) जारी;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      निश्चित( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      क्रम(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        Table *pTab = sqlite_Hash_तथ्य(x);  /* Current table */
        सूचकांक *pIdx;                      /* An index on pTab */
        अगर( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        अगर( HasRowid(pTab) ) cnt++;
        क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){ cnt++; }
      }
      अगर( cnt==0 ) जारी;
      अगर( pObjTab ) cnt++;
      a_मूल = sqlite3DbMallocRawNN(db, माप(पूर्णांक)*(cnt+1));
      अगर( a_मूल==0 ) अवरोध;
      cnt = 0;
      अगर( pObjTab ) a_मूल[++cnt] = 0;
      क्रम(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        Table *pTab = sqlite_Hash_तथ्य(x);
        सूचकांक *pIdx;
        अगर( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        अगर( HasRowid(pTab) ) a_मूल[++cnt] = pTab->tnum;
        क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          a_मूल[++cnt] = pIdx->tnum;
        }
      }
      a_मूल[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_अस्थायी_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (अक्षर*)a_मूल,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3VdbeAddOp1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3VdbeAddOp3(v, OP_Concat, 2, 3, 3);
      integrity_Check_परिणाम_Row(v);
      sqlite3VdbeJumpHere(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3VdbeLoadString(v, 2, "wrong # of entries in index ");
      क्रम(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        पूर्णांक iTab = 0;
        Table *pTab = sqlite_Hash_तथ्य(x);
        सूचकांक *pIdx;
        अगर( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        अगर( HasRowid(pTab) ){
          iTab = cnt++;
        }अन्यथा{
          iTab = cnt;
          क्रम(pIdx=pTab->p_सूचकांक; ALWAYS(pIdx); pIdx=pIdx->p_अग्र){
            अगर( Is_Primary_कुंजी_सूचकांक(pIdx) ) अवरोध;
            iTab++;
          }
        }
        क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          अगर( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3VdbeAddOp3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 2, 3);
            integrity_Check_परिणाम_Row(v);
            sqlite3VdbeJumpHere(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      क्रम(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        Table *pTab = sqlite_Hash_तथ्य(x);
        सूचकांक *pIdx, *pPk;
        सूचकांक *pPrior = 0;      /* Previous index */
        पूर्णांक loop_शीर्ष;
        पूर्णांक i_तथ्य_Cur, iIdxCur;
        पूर्णांक r1 = -1;
        पूर्णांक bStrict;            /* True for a STRICT table */
        पूर्णांक r2;                 /* Previous key for WITHOUT ROWID tables */
        पूर्णांक mxCol;              /* Maximum non-virtual column number */

        अगर( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        अगर( !IsOrdinaryTable(pTab) ) जारी;
        अगर( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }अन्यथा{
          pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
          r2 = sqlite3_Get_अस्थायी_Range(pParse, pPk->n_कुंजी_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_कुंजी_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_तथ्य_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        क्रम(j=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        निश्चित( pParse->nMem>=8+j );
        निश्चित( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, i_तथ्य_Cur, 0); VdbeCoverage(v);
        loop_शीर्ष = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        निश्चित( !IsVirtual(pTab) );
        अगर( HasRowid(pTab) ){
          mxCol = -1;
          क्रम(j=0; j<pTab->nCol; j++){
            अगर( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          अगर( mxCol==pTab->iPKey ) mxCol--;
        }अन्यथा{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_कुंजी_सूचकांक(pTab)->nColumn-1;
        }
        अगर( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, i_तथ्य_Cur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        अगर( !isQuick ){
          अगर( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            पूर्णांक a1;
            अक्षर *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, i_तथ्य_Cur, 0,r2,pPk->n_कुंजी_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_परिणाम_Row(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            क्रम(j=0; j<pPk->n_कुंजी_Col; j++){
              sqlite3_व्यंजक_Code_Load_सूचकांक_Column(pParse, pPk, i_तथ्य_Cur, j, r2+j);
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
        क्रम(j=0; j<pTab->nCol; j++){
          अक्षर *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          पूर्णांक labelError;               /* Jump here to report an error */
          पूर्णांक labelOk;                  /* Jump here if all looks ok */
          पूर्णांक p1, p3, p4;               /* Operands to the OP_IsType opcode */
          पूर्णांक doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          अगर( j==pTab->iPKey ) जारी;
          अगर( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }अन्यथा{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          अगर( pCol->notNull==0 && !doTypeCheck ) जारी;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          अगर( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }अन्यथा{
            अगर( pCol->iDflt ){
              sqlite3_मूल्य *p_Dflt_मूल्य = 0;
              sqlite3_मूल्य_From_व्यंजक(db, sqlite3_Column_व्यंजक_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_मूल्य);
              अगर( p_Dflt_मूल्य ){
                p4 = sqlite3_मूल्य_type(p_Dflt_मूल्य);
                sqlite3_मूल्य_Free(p_Dflt_मूल्य);
              }
            }
            p1 = i_तथ्य_Cur;
            अगर( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3_Table_Column_To_सूचकांक(sqlite3_Primary_कुंजी_सूचकांक(pTab), j);
            }अन्यथा{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          अगर( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            पूर्णांक jmp3;
            पूर्णांक jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            अगर( p1<0 ){
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
            अगर( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }अन्यथा{
              /* VDBE byte code will fall thru */
            }
          }
          अगर( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            अटल अचिन्हित अक्षर aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            निश्चित( pCol->eCType>=1 && pCol->eCType<=माप(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा अगर( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा अगर( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            अगर( p1>=0 ){
              sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, i_तथ्य_Cur, j, 3);
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
          integrity_Check_परिणाम_Row(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        अगर( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          व्यंजक_List *pCheck = sqlite3_व्यंजक_List_Dup(db, pTab->pCheck, 0);
          अगर( db->mallocFailed==0 ){
            पूर्णांक addrCkFault = sqlite3VdbeMakeLabel(pParse);
            पूर्णांक addrCkOk = sqlite3VdbeMakeLabel(pParse);
            अक्षर *zErr;
            पूर्णांक k;
            pParse->iSelfTab = i_तथ्य_Cur + 1;
            क्रम(k=pCheck->n_व्यंजक-1; k>0; k--){
              sqlite3_व्यंजक_If_False(pParse, pCheck->a[k].p_व्यंजक, addrCkFault, 0);
            }
            sqlite3_व्यंजक_If_True(pParse, pCheck->a[0].p_व्यंजक, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_परिणाम_Row(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_व्यंजक_List_Delete(db, pCheck);
        }
        अगर( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          क्रम(j=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, j++){
            पूर्णांक jmp2, jmp3, jmp4, jmp5, label6;
            पूर्णांक kk;
            पूर्णांक ckUniq = sqlite3VdbeMakeLabel(pParse);
            अगर( pPk==pIdx ) जारी;
            r1 = sqlite3_Generate_सूचकांक_कुंजी(pParse, pIdx, i_तथ्य_Cur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_कुंजी, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (स्थिर अक्षर*)pIdx, P4_सूचकांक);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            integrity_Check_परिणाम_Row(v);
            sqlite3VdbeAddOp2(v, OP_Goto, 0, ckUniq);

            sqlite3VdbeJumpHere(v, jmp2);
            sqlite3VdbeLoadString(v, 3, "row ");
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            sqlite3VdbeLoadString(v, 4, " missing from index ");
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_परिणाम_Row(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            अगर( HasRowid(pTab) ){
              पूर्णांक jmp7;
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
            क्रम(kk=0; kk<pIdx->n_कुंजी_Col; kk++){
              अगर( pIdx->azColl[kk]==sqlite3_Str_द्विआधारी ) जारी;
              अगर( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            अगर( label6 ){
              पूर्णांक jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            अगर( Is_Unique_सूचकांक(pIdx) ){
              पूर्णांक uniqOk = sqlite3VdbeMakeLabel(pParse);
              पूर्णांक jmp6;
              क्रम(kk=0; kk<pIdx->n_कुंजी_Col; kk++){
                पूर्णांक iCol = pIdx->aiColumn[kk];
                निश्चित( iCol!=XN_ROWID && iCol<pTab->nCol );
                अगर( iCol>=0 && pTab->aCol[iCol].notNull ) जारी;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_अग्र, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_कुंजी_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_अग्र, i_तथ्य_Cur, loop_शीर्ष); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_शीर्ष-1);
        अगर( pPk ){
          निश्चित( !isQuick );
          sqlite3_Release_अस्थायी_Range(pParse, r2, pPk->n_कुंजी_Col);
        }
      }

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      क्रम(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        Table *pTab = sqlite_Hash_तथ्य(x);
        sqlite3_vtab *pVTab;
        पूर्णांक a1;
        अगर( tableSkipIntegrityCheck(pTab,pObjTab) ) जारी;
        अगर( IsOrdinaryTable(pTab) ) जारी;
        अगर( !IsVirtual(pTab) ) जारी;
        अगर( pTab->nCol<=0 ){
          स्थिर अक्षर *zMod = pTab->u.vtab.azArg[0];
          अगर( sqlite3HashFind(&db->aModule, zMod)==0 ) जारी;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        अगर( pTab->u.vtab.p==0 ) जारी;
        pVTab = pTab->u.vtab.p->pVtab;
        अगर( NEVER(pVTab==0) ) जारी;
        अगर( NEVER(pVTab->pModule==0) ) जारी;
        अगर( pVTab->pModule->iVersion<4 ) जारी;
        अगर( pVTab->pModule->xIntegrity==0 ) जारी;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_परिणाम_Row(v);
        sqlite3VdbeJumpHere(v, a1);
        जारी;
      }
#पूर्ण_अगर
    }
    {
      अटल स्थिर पूर्णांक iLn = VDBE_OFFSET_LINENO(2);
      अटल स्थिर VdbeOpList endCode[] = {
        { OP_AddImm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_परिणाम_Row,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3VdbeAddOpList(v, Array_आकार(endCode), endCode, iLn);
      अगर( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (अक्षर*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_INTEGRITY_CHECK */

#अगर_अघोषित SQLITE_OMIT_UTF16
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
  हाल PragTyp_ENCODING: {
    अटल स्थिर काष्ठा EncName {
      अक्षर *zName;
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
    स्थिर काष्ठा EncName *pEnc;
    अगर( !z_दायाँ ){    /* "PRAGMA encoding" */
      अगर( sqlite3ReadSchema(pParse) ) जाओ pragma_out;
      निश्चित( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      निश्चित( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      निश्चित( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }अन्यथा{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      अगर( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        क्रम(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          अगर( 0==sqlite3StrICmp(z_दायाँ, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            अवरोध;
          }
        }
        अगर( !pEnc->zName ){
          sqlite3_Error_संदेश(pParse, "unsupported encoding: %s", z_दायाँ);
        }
      }
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_UTF16 */

#अगर_अघोषित SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS
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
  हाल Prag_Typ_HEADER_मूल्य: {
    पूर्णांक iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    अगर( z_दायाँ && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      अटल स्थिर VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(setCookie));
      aOp = sqlite3VdbeAddOpList(v, Array_आकार(setCookie), setCookie, 0);
      अगर( ONLY_IF_REALLOC_STRESS(aOp==0) ) अवरोध;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_दायाँ);
      aOp[1].p5 = 1;
      अगर( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }अन्यथा{
      /* Read the specified cookie value */
      अटल स्थिर VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_परिणाम_Row,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(readCookie));
      aOp = sqlite3VdbeAddOpList(v, Array_आकार(readCookie),readCookie,0);
      अगर( ONLY_IF_REALLOC_STRESS(aOp==0) ) अवरोध;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  हाल PragTyp_COMPILE_OPTIONS: {
    पूर्णांक i = 0;
    स्थिर अक्षर *zOpt;
    pParse->nMem = 1;
    जबतक( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  अवरोध;
#पूर्ण_अगर /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#अगर_अघोषित SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  हाल PragTyp_WAL_CHECKPOINT: {
    पूर्णांक iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    पूर्णांक eMode = SQLITE_CHECKPOINT_PASSIVE;
    अगर( z_दायाँ ){
      अगर( sqlite3StrICmp(z_दायाँ, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }अन्यथा अगर( sqlite3StrICmp(z_दायाँ, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }अन्यथा अगर( sqlite3StrICmp(z_दायाँ, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }अन्यथा अगर( sqlite3StrICmp(z_दायाँ, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_परिणाम_Row, 1, 3);
  }
  अवरोध;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  हाल PragTyp_WAL_AUTOCHECKPOINT: {
    अगर( z_दायाँ ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_दायाँ));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  अवरोध;
#पूर्ण_अगर

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  हाल PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    अवरोध;
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
  हाल PragTyp_OPTIMIZE: {
    पूर्णांक iDbLast;           /* Loop termination point for the schema loop */
    पूर्णांक iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    सूचकांक *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    अक्षर *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    पूर्णांक nLimit;            /* Analysis limit to use */
    पूर्णांक nCheck = 0;        /* Number of tables to be optimized */
    पूर्णांक nBtree = 0;        /* Number of btrees to scan */
    पूर्णांक n_सूचकांक;            /* Number of indexes on the current table */

    अगर( z_दायाँ ){
      opMask = (u32)sqlite3Atoi(z_दायाँ);
      अगर( (opMask & 0x02)==0 ) अवरोध;
    }अन्यथा{
      opMask = 0xfffe;
    }
    अगर( (opMask & 0x10)==0 ){
      nLimit = 0;
    }अन्यथा अगर( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }अन्यथा{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    क्रम(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      अगर( iDb==1 ) जारी;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      क्रम(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_अग्र(k)){
        pTab = (Table*)sqlite_Hash_तथ्य(k);

        /* This only works for ordinary tables */
        अगर( !IsOrdinaryTable(pTab) ) जारी;

        /* Do not scan system tables */
        अगर( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) जारी;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        n_सूचकांक = 0;
        क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          n_सूचकांक++;
          अगर( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        अगर( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }अन्यथा अगर( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }अन्यथा अगर( pTab->p_सूचकांक!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }अन्यथा{
          /* Otherwise, we can skip this table */
          जारी;
        }

        nCheck++;
        अगर( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_सूचकांक+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        अगर( szThreshold>=0 ){
          स्थिर LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_If_आकार_Between, iTabCur,
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
        अगर( opMask & 0x01 ){
          पूर्णांक r1 = sqlite3_Get_अस्थायी_Reg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_परिणाम_Row, r1, 1);
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
    अगर( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      पूर्णांक iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      अगर( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      क्रम(iAddr=0; iAddr<iEnd; iAddr++){
        अगर( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    अवरोध;
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
  /*case PragTyp_BUSY_TIMEOUT*/ शेष: {
    पूर्णांक n = 0;
    निश्चित( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    अगर( z_दायाँ && sqlite3GetInt32(z_दायाँ,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    अवरोध;
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
  हाल PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    अगर( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    अवरोध;
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
  हाल PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    अगर( z_दायाँ && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      अगर( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    अवरोध;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  हाल PragTyp_THREADS: {
    sqlite3_int64 N;
    अगर( z_दायाँ
     && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (पूर्णांक)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    अवरोध;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  हाल PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    अगर( z_दायाँ
     && sqlite3DecOrHexToI64(z_दायाँ, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (पूर्णांक)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    अवरोध;
  }

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  हाल PragTyp_LOCK_STATUS: {
    अटल स्थिर अक्षर *स्थिर azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    पूर्णांक i;
    pParse->nMem = 2;
    क्रम(i=0; i<db->nDb; i++){
      Btree *pBt;
      स्थिर अक्षर *zState = "unknown";
      पूर्णांक j;
      अगर( db->aDb[i].zDbSName==0 ) जारी;
      pBt = db->aDb[i].pBt;
      अगर( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }अन्यथा अगर( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    अवरोध;
  }
#पूर्ण_अगर

#अगर defined(SQLITE_ENABLE_CEROD)
  हाल PragTyp_ACTIVATE_EXTENSIONS: अगर( z_दायाँ ){
    अगर( sqlite3StrNICmp(z_दायाँ, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_दायाँ[6]);
    }
  }
  अवरोध;
#पूर्ण_अगर

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  अगर( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_दायाँ ){
    sqlite3_Vdbe_Verify_No_परिणाम_Row(v);
  }

pragma_out:
  sqlite3DbFree(db, z_बायाँ);
  sqlite3DbFree(db, z_दायाँ);
}
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
प्रकार काष्ठा PragmaVtab PragmaVtab;
प्रकार काष्ठा PragmaVtabCursor PragmaVtabCursor;
काष्ठा PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  स्थिर PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
काष्ठा PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  अक्षर *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
अटल पूर्णांक pragmaVtabConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  स्थिर PragmaName *pPragma = (स्थिर PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  पूर्णांक rc;
  पूर्णांक i, j;
  अक्षर cSep = '(';
  StrAccum acc;
  अक्षर zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, माप(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  क्रम(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  अगर( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  अगर( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  अगर( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  निश्चित( म_माप(zBuf) < माप(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  अगर( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(माप(PragmaVtab));
    अगर( pTab==0 ){
      rc = SQLITE_NOMEM;
    }अन्यथा{
      स_रखो(pTab, 0, माप(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }अन्यथा{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  वापस rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
अटल पूर्णांक pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  वापस SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
अटल पूर्णांक pragma_Vtab_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  स्थिर काष्ठा sqlite3_सूचकांक_constraint *pConstraint;
  पूर्णांक i, j;
  पूर्णांक seen[2];

  pIdxInfo->estimatedCost = (द्विगुन)1;
  अगर( pTab->nHidden==0 ){ वापस SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  क्रम(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    अगर( pConstraint->iColumn < pTab->iHidden ) जारी;
    अगर( pConstraint->op!=SQLITE_सूचकांक_CONSTRAINT_EQ ) जारी;
    अगर( pConstraint->usable==0 ) वापस SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    निश्चित( j < 2 );
    seen[j] = i+1;
  }
  अगर( seen[0]==0 ){
    pIdxInfo->estimatedCost = (द्विगुन)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    वापस SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_सूचकांक = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (द्विगुन)20;
  pIdxInfo->estimatedRows = 20;
  अगर( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_सूचकांक = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  वापस SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
अटल पूर्णांक pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(माप(*pCsr));
  अगर( pCsr==0 ) वापस SQLITE_NOMEM;
  स_रखो(pCsr, 0, माप(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  वापस SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
अटल व्योम pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  पूर्णांक i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  क्रम(i=0; i<Array_आकार(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
अटल पूर्णांक pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  वापस SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
अटल पूर्णांक pragma_Vtab_अग्र(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  पूर्णांक rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  निश्चित( pCsr->pPragma );
  अगर( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  वापस rc;
}

/*
** Pragma virtual table module xFilter method.
*/
अटल पूर्णांक pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  पूर्णांक rc;
  पूर्णांक i, j;
  StrAccum acc;
  अक्षर *zSql;

  UNUSED_PARAMETER(idx_अंक);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  क्रम(i=0; i<argc; i++, j++){
    स्थिर अक्षर *zText = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
    निश्चित( j<Array_आकार(pCsr->azArg) );
    निश्चित( pCsr->azArg[j]==0 );
    अगर( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      अगर( pCsr->azArg[j]==0 ){
        वापस SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_लंबाई]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  अगर( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  अगर( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  अगर( zSql==0 ) वापस SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  अगर( rc!=SQLITE_OK ){
    pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    वापस rc;
  }
  वापस pragma_Vtab_अग्र(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
अटल पूर्णांक pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  वापस (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
अटल पूर्णांक pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  पूर्णांक i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  अगर( i<pTab->iHidden ){
    sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCsr->pPragma, i));
  }अन्यथा{
    sqlite3_परिणाम_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  वापस SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
अटल पूर्णांक pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  वापस SQLITE_OK;
}

/* The pragma virtual table object */
अटल स्थिर sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_Best_सूचकांक,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_अग्र,              /* xNext - advance a cursor */
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, स्थिर अक्षर *zName){
  स्थिर PragmaName *pName;
  निश्चित( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  अगर( pName==0 ) वापस 0;
  अगर( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) वापस 0;
  निश्चित( sqlite3HashFind(&db->aModule, zName)==0 );
  वापस sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (व्योम*)pName, 0);
}

#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

#पूर्ण_अगर /* SQLITE_OMIT_PRAGMA */
