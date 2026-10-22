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
#అన్తర్భావ "sqliteInt.h"

#ఒకవేళ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ఒకవేళ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  లేకపోతే
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#అయితే_ముగింపు

/***************************************************************************
** The "pragma.h" include file is an automatically generated file that
** that includes the PragType_XXXX macro definitions and the aPragmaName[]
** object.  This ensures that the aPragmaName[] table is arranged in
** lexicographical order to facility a binary search of the pragma name.
** Do not edit pragma.h directly.  Edit and rerun the script in at
** ../tool/mkpragmatab.tcl. */
#అన్తర్భావ "pragma.h"

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
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_OPTIMIZE_హద్దు
# define SQLITE_DEFAULT_OPTIMIZE_హద్దు 2000
#అయితే_ముగింపు

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
స్థిర u8 getSafetyLevel(స్థిరరాశి అక్షరం *z, పూర్ణాంకం omit_నిండిన, u8 dflt){
                             /* 123456789 123456789 123 */
  స్థిర స్థిరరాశి అక్షరం z_పాఠ్యం[] = "onoffalseyestruextrafull";
  స్థిర స్థిరరాశి u8 i_స్థానభ్రంశం[] = {0, 1, 2,  4,    9,  12,  15,   20};
  స్థిర స్థిరరాశి u8 i_నిడివి[] = {2, 2, 3,  5,    3,   4,   5,    4};
  స్థిర స్థిరరాశి u8 i_విలువ[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  పూర్ణాంకం i, n;
  ఒకవేళ( sqlite3Isdigit(*z) ){
    తిరిగి sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  కొరకు(i=0; i<Array_కొలత(i_నిడివి); i++){
    ఒకవేళ( i_నిడివి[i]==n && sqlite3StrNICmp(&z_పాఠ్యం[i_స్థానభ్రంశం[i]],z,n)==0
     && (!omit_నిండిన || i_విలువ[i]<=1)
    ){
      తిరిగి i_విలువ[i];
    }
  }
  తిరిగి dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(స్థిరరాశి అక్షరం *z, u8 dflt){
  తిరిగి getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#ఒకవేళ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
స్థిర పూర్ణాంకం get_Locking_బహుళకం(స్థిరరాశి అక్షరం *z){
  ఒకవేళ( z ){
    ఒకవేళ( 0==sqlite3StrICmp(z, "exclusive") ) తిరిగి PAGER_LOCKINGMODE_EXCLUSIVE;
    ఒకవేళ( 0==sqlite3StrICmp(z, "normal") ) తిరిగి PAGER_LOCKINGMODE_సాధారణ;
  }
  తిరిగి PAGER_LOCKINGMODE_QUERY;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
స్థిర పూర్ణాంకం getAutoVacuum(స్థిరరాశి అక్షరం *z){
  పూర్ణాంకం i;
  ఒకవేళ( 0==sqlite3StrICmp(z, "none") ) తిరిగి BTREE_AUTOVACUUM_NONE;
  ఒకవేళ( 0==sqlite3StrICmp(z, "full") ) తిరిగి BTREE_AUTOVACUUM_నిండిన;
  ఒకవేళ( 0==sqlite3StrICmp(z, "incremental") ) తిరిగి BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  తిరిగి (u8)((i>=0&&i<=2)?i:0);
}
#అయితే_ముగింపు /* ifndef SQLITE_OMIT_AUTOVACUUM */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
స్థిర పూర్ణాంకం get_తాత్కాలిక_Store(స్థిరరాశి అక్షరం *z){
  ఒకవేళ( z[0]>='0' && z[0]<='2' ){
    తిరిగి z[0] - '0';
  }లేకపోతే ఒకవేళ( sqlite3StrICmp(z, "file")==0 ){
    తిరిగి 1;
  }లేకపోతే ఒకవేళ( sqlite3StrICmp(z, "memory")==0 ){
    తిరిగి 2;
  }లేకపోతే{
    తిరిగి 0;
  }
}
#అయితే_ముగింపు /* SQLITE_PAGER_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
స్థిర పూర్ణాంకం invalidate_తాత్కాలిక_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  ఒకవేళ( db->aDb[1].pBt!=0 ){
    ఒకవేళ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_సందేశం(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      తిరిగి SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
  }
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_PAGER_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
స్థిర పూర్ణాంకం change_తాత్కాలిక_Storage(Parse *pParse, స్థిరరాశి అక్షరం *zStorageType){
  పూర్ణాంకం ts = get_తాత్కాలిక_Store(zStorageType);
  sqlite3 *db = pParse->db;
  ఒకవేళ( db->తాత్కాలిక_store==ts ) తిరిగి SQLITE_OK;
  ఒకవేళ( invalidate_తాత్కాలిక_Storage( pParse ) != SQLITE_OK ){
    తిరిగి SQLITE_ERROR;
  }
  db->తాత్కాలిక_store = (u8)ts;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
స్థిర శూన్య సమితి_Pragma_ఫలితం_స్తంభం_పేర్లు(
  Vdbe *v,                     /* The query under construction */
  స్థిరరాశి PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_సమితి_అంకె_నిలువుల_సంఖ్య(v, n==0 ? 1 : n);
  ఒకవేళ( n==0 ){
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }లేకపోతే{
    పూర్ణాంకం i, j;
    కొరకు(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
స్థిర శూన్య returnSingleInt(Vdbe *v, i64 విలువ){
  sqlite3_Vdbe_కూడు_Int64(v, 1, విలువ);
  sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 1);
}

/*
** Generate code to return a single text value.
*/
స్థిర శూన్య return_Single_పాఠ్యం(
  Vdbe *v,                /* Prepared statement under construction */
  స్థిరరాశి అక్షరం *z_విలువ      /* Value to be returned */
){
  ఒకవేళ( z_విలువ ){
    sqlite3_Vdbe_ఎక్కించు_String(v, 1, (స్థిరరాశి అక్షరం*)z_విలువ);
    sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
స్థిర శూన్య సమితి_All_Pager_Flags(sqlite3 *db){
  ఒకవేళ( db->autoCommit ){
    Db *pDb = db->aDb;
    పూర్ణాంకం n = db->nDb;
    ప్రతిజ్ఞా( SQLITE_నిండిన_FSync==PAGER_FULLFSYNC );
    ప్రతిజ్ఞా( SQLITE_Ckpt_నిండిన_FSync==PAGER_CKPT_FULLFSYNC );
    ప్రతిజ్ఞా( SQLITE_CacheSpill==PAGER_CACHESPILL );
    ప్రతిజ్ఞా( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_ముసుగు );
    ప్రతిజ్ఞా( (pDb->safety_level & PAGER_SYNCHRONOUS_ముసుగు)==pDb->safety_level );
    అయితే( (n--) > 0 ){
      ఒకవేళ( pDb->pBt ){
        sqlite3_Btree_సమితి_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_ముసుగు) );
      }
      pDb++;
    }
  }
}
#లేకపోతే
# define సమితి_All_Pager_Flags(X)  /* no-op */
#అయితే_ముగింపు


/*
** Return a human-readable name for a constraint resolution action.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
స్థిర స్థిరరాశి అక్షరం *actionName(u8 action){
  స్థిరరాశి అక్షరం *zName;
  ఎంపిక( action ){
    స్థితి OE_సమితి_Null:  zName = "SET NULL";        ఆపు;
    స్థితి OE_సమితి_Dflt:  zName = "SET DEFAULT";     ఆపు;
    స్థితి OE_Cascade:  zName = "CASCADE";         ఆపు;
    స్థితి OE_Restrict: zName = "RESTRICT";        ఆపు;
    అప్రమేయ:          zName = "NO ACTION"; 
                      ప్రతిజ్ఞా( action==OE_None ); ఆపు;
  }
  తిరిగి zName;
}
#అయితే_ముగింపు


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
స్థిరరాశి అక్షరం *sqlite3JournalModename(పూర్ణాంకం e_బహుళకం){
  స్థిర అక్షరం * స్థిరరాశి az_బహుళకం_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
     , "wal"
#అయితే_ముగింపు
  };
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_DELETE==0 );
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_PERSIST==1 );
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_OFF==2 );
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_TRUNCATE==3 );
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_జ్ఞాపకం==4 );
  ప్రతిజ్ఞా( PAGER_JOURNALMODE_WAL==5 );
  ప్రతిజ్ఞా( e_బహుళకం>=0 && e_బహుళకం<=Array_కొలత(az_బహుళకం_Name) );

  ఒకవేళ( e_బహుళకం==Array_కొలత(az_బహుళకం_Name) ) తిరిగి 0;
  తిరిగి az_బహుళకం_Name[e_బహుళకం];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
స్థిర స్థిరరాశి PragmaName *pragmaLocate(స్థిరరాశి అక్షరం *zName){
  పూర్ణాంకం upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_కొలత(aPragmaName)-1;
  అయితే( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    ఒకవేళ( rc==0 ) ఆపు;
    ఒకవేళ( rc<0 ){
      upr = mid - 1;
    }లేకపోతే{
      lwr = mid + 1;
    }
  }
  తిరిగి lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
స్థిర శూన్య pragma_Funclist_పంక్తి(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  పూర్ణాంకం isBuiltin,         /* True if this is a built-in function */
  పూర్ణాంకం showInternFuncs    /* True if showing internal functions */
){
  u32 ముసుగు =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_అంతర్గత
  ;
  ఒకవేళ( showInternFuncs ) ముసుగు = 0xffffffff;
  కొరకు(; p; p=p->p_తదుపరి){
    స్థిరరాశి అక్షరం *zType;
    స్థిర స్థిరరాశి అక్షరం *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    ప్రతిజ్ఞా( SQLITE_FUNC_ENCMASK==0x3 );
    ప్రతిజ్ఞా( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    ప్రతిజ్ఞా( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    ప్రతిజ్ఞా( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    ఒకవేళ( p->xSFunc==0 ) కొనసాగు;
    ఒకవేళ( (p->funcFlags & SQLITE_FUNC_అంతర్గత)!=0
     && showInternFuncs==0
    ){
      కొనసాగు;
    }   
    ఒకవేళ( p->x_విలువ!=0 ){
      zType = "w";
    }లేకపోతే ఒకవేళ( p->xFinalize!=0 ){
      zType = "a";
    }లేకపోతే{
      zType = "s";
    }
    sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & ముసుగు) ^ SQLITE_INNOCUOUS
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
స్థిర పూర్ణాంకం integrity_Check_ఫలితం_అడ్డువరుస(Vdbe *v){
  పూర్ణాంకం addr;
  sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 3, 1);
  addr = sqlite3_Vdbe_కూడు_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_కూడు_Op0(v, OP_Halt);
  తిరిగి addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
స్థిర పూర్ణాంకం పట్టిక_Skip_Integrity_Check(స్థిరరాశి పట్టిక *pTab, స్థిరరాశి పట్టిక *pObjTab){
  ఒకవేళ( pObjTab ){
    తిరిగి pTab!=pObjTab;
  }లేకపోతే{
    తిరిగి (pTab->tabFlags & TF_Imposter)!=0;
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
శూన్య sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_విలువ,      /* Token for <value>, or NULL */
  పూర్ణాంకం minusFlag       /* True if a '-' sign preceded <value> */
){
  అక్షరం *z_ఎడమ = 0;       /* Nul-terminated UTF-8 string <id> */
  అక్షరం *z_కుడి = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  స్థిరరాశి అక్షరం *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  అక్షరం *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  పూర్ణాంకం iDb;               /* Database index for <database> */
  పూర్ణాంకం rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  స్థిరరాశి PragmaName *pPragma;   /* The pragma */

  ఒకవేళ( v==0 ) తిరిగి;
  sqlite3_Vdbe_నిరంతరం_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  ఒకవేళ( iDb<0 ) తిరిగి;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  ఒకవేళ( iDb==1 && sqlite3_Open_తాత్కాలిక_Database(pParse) ){
    తిరిగి;
  }

  z_ఎడమ = sqlite3NameFromToken(db, pId);
  ఒకవేళ( !z_ఎడమ ) తిరిగి;
  ఒకవేళ( minusFlag ){
    z_కుడి = sqlite3MPrintf(db, "-%T", p_విలువ);
  }లేకపోతే{
    z_కుడి = sqlite3NameFromToken(db, p_విలువ);
  }

  ప్రతిజ్ఞా( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_ఎడమ, z_కుడి, zDb) ){
    వెళ్ళు pragma_బహిర్గతం;
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
  aFcntl[1] = z_ఎడమ;
  aFcntl[2] = z_కుడి;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (శూన్య*)aFcntl);
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3_Vdbe_సమితి_అంకె_నిలువుల_సంఖ్య(v, 1);
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    return_Single_పాఠ్యం(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    వెళ్ళు pragma_బహిర్గతం;
  }
  ఒకవేళ( rc!=SQLITE_NOTFOUND ){
    ఒకవేళ( aFcntl[0] ){
      sqlite3_Error_సందేశం(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    వెళ్ళు pragma_బహిర్గతం;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_ఎడమ);
  ఒకవేళ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    వెళ్ళు pragma_బహిర్గతం;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  ఒకవేళ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    ఒకవేళ( sqlite3ReadSchema(pParse) ) వెళ్ళు pragma_బహిర్గతం;
  }

  /* Register the result column names for pragmas that return results */
  ఒకవేళ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_కుడి==0)
  ){
    సమితి_Pragma_ఫలితం_స్తంభం_పేర్లు(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  ఎంపిక( pPragma->ePragTyp ){
 
#ఒకవేళ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  స్థితి Prag_Typ_DEFAULT_CACHE_కొలత: {
    స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
    స్థిర స్థిరరాశి VdbeOpList get_Cache_కొలత[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_కొలత},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_వ్యవకలనం,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_ఫలితం_అడ్డువరుస,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb);
    ఒకవేళ( !z_కుడి ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_కొలత(get_Cache_కొలత));
      aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(get_Cache_కొలత), get_Cache_కొలత, iLn);
      ఒకవేళ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ఆపు;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_కొలత;
    }లేకపోతే{
      పూర్ణాంకం కొలత = sqlite3AbsInt32(sqlite3Atoi(z_కుడి));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_కూడు_Op3(v, OP_సమితి_Cookie, iDb, BTREE_DEFAULT_CACHE_కొలత, కొలత);
      ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_కొలత = కొలత;
      sqlite3_Btree_సమితి_Cache_కొలత(pDb->pBt, pDb->pSchema->cache_కొలత);
    }
    ఆపు;
  }
#అయితే_ముగింపు /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#ఒకవేళ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  స్థితి Prag_Typ_పుట_కొలత: {
    Btree *pBt = pDb->pBt;
    ప్రతిజ్ఞా( pBt!=0 );
    ఒకవేళ( !z_కుడి ){
      పూర్ణాంకం కొలత = ALWAYS(pBt) ? sqlite3_Btree_Get_పుట_కొలత(pBt) : 0;
      returnSingleInt(v, కొలత);
    }లేకపోతే{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->తదుపరి_Pagesize = sqlite3Atoi(z_కుడి);
      ఒకవేళ( SQLITE_NOMEM==sqlite3_Btree_సమితి_పుట_కొలత(pBt, db->తదుపరి_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    ఆపు;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  స్థితి PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    పూర్ణాంకం b = -1;
    ప్రతిజ్ఞా( pBt!=0 );
    ఒకవేళ( z_కుడి ){
      ఒకవేళ( sqlite3_stricmp(z_కుడి, "fast")==0 ){
        b = 2;
      }లేకపోతే{
        b = sqlite3GetBoolean(z_కుడి, 0);
      }
    }
    ఒకవేళ( pId2->n==0 && b>=0 ){
      పూర్ణాంకం ii;
      కొరకు(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    ఆపు;
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
  స్థితి Prag_Typ_పుట_లెక్క: {
    పూర్ణాంకం iReg;
    i64 x = 0;
    sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    ఒకవేళ( sqlite3Tolower(z_ఎడమ[0])=='p' ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Pagecount, iDb, iReg);
    }లేకపోతే{
      ఒకవేళ( z_కుడి && sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి,&x)==0 ){
        ఒకవేళ( x<0 ) x = 0;
        లేకపోతే ఒకవేళ( x>0xfffffffe ) x = 0xfffffffe;
      }లేకపోతే{
        x = 0;
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_MaxPgcnt, iDb, iReg, (పూర్ణాంకం)x);
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, iReg, 1);
    ఆపు;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  స్థితి Prag_Typ_LOCKING_బహుళకం: {
    స్థిరరాశి అక్షరం *zRet = "normal";
    పూర్ణాంకం e_బహుళకం = get_Locking_బహుళకం(z_కుడి);

    ఒకవేళ( pId2->n==0 && e_బహుళకం==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_బహుళకం = db->dflt_తాళం_బహుళకం;
    }లేకపోతే{
      Pager *pPager;
      ఒకవేళ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        పూర్ణాంకం ii;
        ప్రతిజ్ఞా(pDb==&db->aDb[0]);
        కొరకు(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3_Pager_Locking_బహుళకం(pPager, e_బహుళకం);
        }
        db->dflt_తాళం_బహుళకం = (u8)e_బహుళకం;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_బహుళకం = sqlite3_Pager_Locking_బహుళకం(pPager, e_బహుళకం);
    }

    ప్రతిజ్ఞా( e_బహుళకం==PAGER_LOCKINGMODE_సాధారణ
            || e_బహుళకం==PAGER_LOCKINGMODE_EXCLUSIVE );
    ఒకవేళ( e_బహుళకం==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    return_Single_పాఠ్యం(v, zRet);
    ఆపు;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  స్థితి Prag_Typ_JOURNAL_బహుళకం: {
    పూర్ణాంకం e_బహుళకం;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    పూర్ణాంకం ii;           /* Loop counter */

    ఒకవేళ( z_కుడి==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_బహుళకం = PAGER_JOURNALMODE_QUERY;
    }లేకపోతే{
      స్థిరరాశి అక్షరం *z_బహుళకం;
      పూర్ణాంకం n = sqlite3Strlen30(z_కుడి);
      కొరకు(e_బహుళకం=0; (z_బహుళకం = sqlite3JournalModename(e_బహుళకం))!=0; e_బహుళకం++){
        ఒకవేళ( sqlite3StrNICmp(z_కుడి, z_బహుళకం, n)==0 ) ఆపు;
      }
      ఒకవేళ( !z_బహుళకం ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_బహుళకం = PAGER_JOURNALMODE_QUERY;
      }
      ఒకవేళ( e_బహుళకం==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_బహుళకం = PAGER_JOURNALMODE_QUERY;
      }
    }
    ఒకవేళ( e_బహుళకం==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    కొరకు(ii=db->nDb-1; ii>=0; ii--){
      ఒకవేళ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, ii);
        sqlite3_Vdbe_కూడు_Op3(v, OP_Journal_బహుళకం, ii, 1, e_బహుళకం);
      }
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 1);
    ఆపు;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  స్థితి Prag_Typ_JOURNAL_కొలత_హద్దు: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_హద్దు = -2;
    ఒకవేళ( z_కుడి ){
      sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &i_హద్దు);
      ఒకవేళ( i_హద్దు<-1 ) i_హద్దు = -1;
    }
    i_హద్దు = sqlite3_Pager_Journal_కొలత_హద్దు(pPager, i_హద్దు);
    returnSingleInt(v, i_హద్దు);
    ఆపు;
  }

#అయితే_ముగింపు /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  స్థితి PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    ప్రతిజ్ఞా( pBt!=0 );
    ఒకవేళ( !z_కుడి ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }లేకపోతే{
      పూర్ణాంకం eAuto = getAutoVacuum(z_కుడి);
      ప్రతిజ్ఞా( eAuto>=0 && eAuto<=2 );
      db->తదుపరి_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_సమితి_Auto_Vacuum(pBt, eAuto);
      ఒకవేళ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
        స్థిర స్థిరరాశి VdbeOpList సమితి_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_అతిపెద్ద_మూలం_పుట},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_సమితి_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        పూర్ణాంకం iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_కొలత(సమితి_Meta6));
        aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(సమితి_Meta6), సమితి_Meta6, iLn);
        ఒకవేళ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ఆపు;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb);
      }
    }
    ఆపు;
  }
#అయితే_ముగింపు

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  స్థితి PragTyp_INCREMENTAL_VACUUM: {
    పూర్ణాంకం i_హద్దు = 0, addr;
    ఒకవేళ( z_కుడి==0 || !sqlite3GetInt32(z_కుడి, &i_హద్దు) || i_హద్దు<=0 ){
      i_హద్దు = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, i_హద్దు, 1);
    addr = sqlite3_Vdbe_కూడు_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op1(v, OP_ఫలితం_అడ్డువరుస, 1);
    sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, 1, -1);
    sqlite3_Vdbe_కూడు_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
    ఆపు;
  }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
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
  స్థితి Prag_Typ_CACHE_కొలత: {
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ఒకవేళ( !z_కుడి ){
      returnSingleInt(v, pDb->pSchema->cache_కొలత);
    }లేకపోతే{
      పూర్ణాంకం కొలత = sqlite3Atoi(z_కుడి);
      pDb->pSchema->cache_కొలత = కొలత;
      sqlite3_Btree_సమితి_Cache_కొలత(pDb->pBt, pDb->pSchema->cache_కొలత);
    }
    ఆపు;
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
  స్థితి PragTyp_CACHE_SPILL: {
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ఒకవేళ( !z_కుడి ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_సమితి_Spill_కొలత(pDb->pBt,0));
    }లేకపోతే{
      పూర్ణాంకం కొలత = 1;
      ఒకవేళ( sqlite3GetInt32(z_కుడి, &కొలత) ){
        sqlite3_Btree_సమితి_Spill_కొలత(pDb->pBt, కొలత);
      }
      ఒకవేళ( sqlite3GetBoolean(z_కుడి, కొలత!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }లేకపోతే{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      సమితి_All_Pager_Flags(db);
    }
    ఆపు;
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
  స్థితి Prag_Typ_MMAP_కొలత: {
    sqlite3_int64 sz;
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ఒకవేళ( z_కుడి ){
      పూర్ణాంకం ii;
      sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &sz);
      ఒకవేళ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      ఒకవేళ( pId2->n==0 ) db->szMmap = sz;
      కొరకు(ii=db->nDb-1; ii>=0; ii--){
        ఒకవేళ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_సమితి_Mmap_హద్దు(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_కొలత, &sz);
#లేకపోతే
    sz = 0;
    rc = SQLITE_OK;
#అయితే_ముగింపు
    ఒకవేళ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }లేకపోతే ఒకవేళ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    ఆపు;
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
  స్థితి Prag_Typ_తాత్కాలిక_STORE: {
    ఒకవేళ( !z_కుడి ){
      returnSingleInt(v, db->తాత్కాలిక_store);
    }లేకపోతే{
      change_తాత్కాలిక_Storage(pParse, z_కుడి);
    }
    ఆపు;
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
  స్థితి Prag_Typ_తాత్కాలిక_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ఒకవేళ( !z_కుడి ){
      return_Single_పాఠ్యం(v, sqlite3_తాత్కాలిక_directory);
    }లేకపోతే{
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
      ఒకవేళ( z_కుడి[0] ){
        పూర్ణాంకం res;
        rc = sqlite3OsAccess(db->pVfs, z_కుడి, SQLITE_ACCESS_READWRITE, &res);
        ఒకవేళ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_సందేశం(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          వెళ్ళు pragma_బహిర్గతం;
        }
      }
      ఒకవేళ( SQLITE_తాత్కాలిక_STORE==0
       || (SQLITE_తాత్కాలిక_STORE==1 && db->తాత్కాలిక_store<=1)
       || (SQLITE_తాత్కాలిక_STORE==2 && db->తాత్కాలిక_store==1)
      ){
        invalidate_తాత్కాలిక_Storage(pParse);
      }
      sqlite3_free(sqlite3_తాత్కాలిక_directory);
      ఒకవేళ( z_కుడి[0] ){
        sqlite3_తాత్కాలిక_directory = sqlite3_mprintf("%s", z_కుడి);
      }లేకపోతే{
        sqlite3_తాత్కాలిక_directory = 0;
      }
#అయితే_ముగింపు /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ఆపు;
  }

#ఒకవేళ SQLITE_OS_WIN
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
  స్థితి Prag_Typ_దత్తాంశం_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ఒకవేళ( !z_కుడి ){
      return_Single_పాఠ్యం(v, sqlite3_దత్తాంశం_directory);
    }లేకపోతే{
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
      ఒకవేళ( z_కుడి[0] ){
        పూర్ణాంకం res;
        rc = sqlite3OsAccess(db->pVfs, z_కుడి, SQLITE_ACCESS_READWRITE, &res);
        ఒకవేళ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_సందేశం(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          వెళ్ళు pragma_బహిర్గతం;
        }
      }
      sqlite3_free(sqlite3_దత్తాంశం_directory);
      ఒకవేళ( z_కుడి[0] ){
        sqlite3_దత్తాంశం_directory = sqlite3_mprintf("%s", z_కుడి);
      }లేకపోతే{
        sqlite3_దత్తాంశం_directory = 0;
      }
#అయితే_ముగింపు /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ఆపు;
  }
#అయితే_ముగింపు

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  స్థితి Prag_Typ_తాళం_PROXY_FILE: {
    ఒకవేళ( !z_కుడి ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      అక్షరం *proxy_file_మార్గం = ఖాళీ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_మార్గం);
      return_Single_పాఠ్యం(v, proxy_file_మార్గం);
    }లేకపోతే{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      పూర్ణాంకం res;
      ఒకవేళ( z_కుడి[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_సమితి_LOCKPROXYFILE,
                                     z_కుడి);
      } లేకపోతే {
        res=sqlite3OsFileControl(pFile, SQLITE_సమితి_LOCKPROXYFILE,
                                     ఖాళీ);
      }
      ఒకవేళ( res!=SQLITE_OK ){
        sqlite3_Error_సందేశం(pParse, "failed to set lock proxy file");
        వెళ్ళు pragma_బహిర్గతం;
      }
    }
    ఆపు;
  }
#అయితే_ముగింపు /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  స్థితి PragTyp_SYNCHRONOUS: {
    ఒకవేళ( !z_కుడి ){
      returnSingleInt(v, pDb->safety_level-1);
    }లేకపోతే{
      ఒకవేళ( !db->autoCommit ){
        sqlite3_Error_సందేశం(pParse,
            "Safety level may not be changed inside a transaction");
      }లేకపోతే ఒకవేళ( iDb!=1 ){
        పూర్ణాంకం iLevel;
        ఒకవేళ( sqlite3Isdigit(z_కుడి[0]) ){
          iLevel = sqlite3Atoi(z_కుడి);
        }లేకపోతే{
          iLevel = getSafetyLevel(z_కుడి,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_ముసుగు;
        ఒకవేళ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_సమితి = 1;
        సమితి_All_Pager_Flags(db);
      }
    }
    ఆపు;
  }
#అయితే_ముగింపు /* SQLITE_OMIT_PAGER_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLAG_PRAGMAS
  స్థితి PragTyp_FLAG: {
    ఒకవేళ( z_కుడి==0 ){
      సమితి_Pragma_ఫలితం_స్తంభం_పేర్లు(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }లేకపోతే{
      u64 ముసుగు = pPragma->iArg;    /* Mask of bits to set or clear. */
      ఒకవేళ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        ముసుగు &= ~(SQLITE_Foreign_కీలకాలు);
      }

      ఒకవేళ( sqlite3GetBoolean(z_కుడి, 0) ){
        ఒకవేళ( (ముసుగు & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= ముసుగు;
        }
      }లేకపోతే{
        db->flags &= ~ముసుగు;
        ఒకవేళ( ముసుగు==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        ఒకవేళ( (ముసుగు & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_కుడి, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_యొక్క_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_కూడు_Op0(v, OP_Expire);
      సమితి_All_Pager_Flags(db);
    }
    ఆపు;
  }
#అయితే_ముగింపు /* SQLITE_OMIT_FLAG_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SCHEMA_PRAGMAS
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
  స్థితి Prag_Typ_పట్టిక_INFO: ఒకవేళ( z_కుడి ){
    పట్టిక *pTab;
    sqlite3_సంకేతం_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_పట్టిక(pParse, LOCATE_NOERR, z_కుడి, zDb);
    ఒకవేళ( pTab ){
      పూర్ణాంకం i, k;
      పూర్ణాంకం nHidden = 0;
      స్తంభం *p_నిలువు;
      సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab);
      కొరకు(i=0, p_నిలువు=pTab->a_నిలువు; i<pTab->n_నిలువు; i++, p_నిలువు++){
        పూర్ణాంకం isHidden = 0;
        స్థిరరాశి సమాసం *p_నిలువు_సమాసం;
        ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_NOINSERT ){
          ఒకవేళ( pPragma->iArg==0 ){
            nHidden++;
            కొనసాగు;
          }
          ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }లేకపోతే ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }లేకపోతే{ ప్రతిజ్ఞా( p_నిలువు->నిలువు_Flags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        ఒకవేళ( (p_నిలువు->నిలువు_Flags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }లేకపోతే ఒకవేళ( pPk==0 ){
          k = 1;
        }లేకపోతే{
          కొరకు(k=1; k<=pTab->n_నిలువు && pPk->ai_స్తంభం[k-1]!=i; k++){}
        }
        p_నిలువు_సమాసం = sqlite3_స్తంభం_సమాసం(pTab,p_నిలువు);
        ప్రతిజ్ఞా( p_నిలువు_సమాసం==0 || p_నిలువు_సమాసం->op==TK_పరిధి || isHidden>=2 );
        ప్రతిజ్ఞా( p_నిలువు_సమాసం==0 || !సమాసం_Has_Property(p_నిలువు_సమాసం, EP_Int_విలువ)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               p_నిలువు->zCnName,
               sqlite3_స్తంభం_Type(p_నిలువు,""),
               p_నిలువు->notNull ? 1 : 0,
               (isHidden>=2 || p_నిలువు_సమాసం==0) ? 0 : p_నిలువు_సమాసం->u.zToken,
               k,
               isHidden);
      }
    }
  }
  ఆపు;

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
  స్థితి Prag_Typ_పట్టిక_LIST: {
    పూర్ణాంకం ii;
    pParse->nMem = 6;
    sqlite3_సంకేతం_Verify_Named_Schema(pParse, zDb);
    కొరకు(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      పూర్ణాంకం initNCol;
      ఒకవేళ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) కొనసాగు;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_లెక్క(pHash);
      అయితే( initNCol-- ){
        కొరకు(k=sqlite_Hash_మొదటి(pHash); 1; k=sqlite_Hash_తదుపరి(k) ){
          పట్టిక *pTab;
          ఒకవేళ( k==0 ){ initNCol = 0; ఆపు; }
          pTab = sqlite_Hash_దత్తాంశం(k);
          ఒకవేళ( pTab->n_నిలువు==0 ){
            అక్షరం *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            ఒకవేళ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (శూన్య)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (శూన్య)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            ఒకవేళ( db->mallocFailed ){
              sqlite3_Error_సందేశం(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            ఆపు;
          }
        }
      }

      కొరకు(k=sqlite_Hash_మొదటి(pHash); k; k=sqlite_Hash_తదుపరి(k) ){
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(k);
        స్థిరరాశి అక్షరం *zType;
        ఒకవేళ( z_కుడి && sqlite3_stricmp(z_కుడి, pTab->zName)!=0 ) కొనసాగు;
        ఒకవేళ( IsView(pTab) ){
          zType = "view";
        }లేకపోతే ఒకవేళ( IsVirtual(pTab) ){
          zType = "virtual";
        }లేకపోతే ఒకవేళ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }లేకపోతే{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_పట్టిక_Name(pTab->zName),
           zType,
           pTab->n_నిలువు,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  ఆపు;

#నిర్వచితమైతే SQLITE_DEBUG
  స్థితి PragTyp_STATS: {
    సూచీ *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
    కొరకు(i=sqlite_Hash_మొదటి(&pDb->pSchema->tblHash); i; i=sqlite_Hash_తదుపరి(i)){
      పట్టిక *pTab = sqlite_Hash_దత్తాంశం(i);
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "ssiii",
           sqlite3_Preferred_పట్టిక_Name(pTab->zName),
           0,
           pTab->sz_Tab_అడ్డువరుస,
           pTab->n_అడ్డువరుస_Log_Est,
           pTab->tabFlags);
      కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
        sqlite3_Vdbe_Multi_ఎక్కించు(v, 2, "siiiX",
           pIdx->zName,
           pIdx->sz_Idx_అడ్డువరుస,
           pIdx->ai_అడ్డువరుస_Log_Est[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 5);
      }
    }
  }
  ఆపు;
#అయితే_ముగింపు

  స్థితి Prag_Typ_సూచీ_INFO: ఒకవేళ( z_కుడి ){
    సూచీ *pIdx;
    పట్టిక *pTab;
    pIdx = sqlite3_కనుగొను_సూచీ(db, z_కుడి, zDb);
    ఒకవేళ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_పట్టిక(pParse, LOCATE_NOERR, z_కుడి, zDb);
      ఒకవేళ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_కీలకం_సూచీ(pTab);
      }
    }
    ఒకవేళ( pIdx ){
      పూర్ణాంకం iIdxDb = sqlite3_Schema_గమ్యం_సూచీ(db, pIdx->pSchema);
      పూర్ణాంకం i;
      పూర్ణాంకం mx;
      ఒకవేళ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->n_స్తంభం;
        pParse->nMem = 6;
      }లేకపోతే{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_కీలకం_నిలువు;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_పట్టిక;
      sqlite3_సంకేతం_Verify_Schema(pParse, iIdxDb);
      ప్రతిజ్ఞా( pParse->nMem<=pPragma->nPragCName );
      కొరకు(i=0; i<mx; i++){
        i16 cnum = pIdx->ai_స్తంభం[i];
        sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->a_నిలువు[cnum].zCnName);
        ఒకవేళ( pPragma->iArg ){
          sqlite3_Vdbe_Multi_ఎక్కించు(v, 4, "isiX",
            pIdx->a_క్రమీకరించు_క్రమం[i],
            pIdx->azColl[i],
            i<pIdx->n_కీలకం_నిలువు);
        }
        sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, pParse->nMem);
      }
    }
  }
  ఆపు;

  స్థితి Prag_Typ_సూచీ_LIST: ఒకవేళ( z_కుడి ){
    సూచీ *pIdx;
    పట్టిక *pTab;
    పూర్ణాంకం i;
    pTab = sqlite3_కనుగొను_పట్టిక(db, z_కుడి, zDb);
    ఒకవేళ( pTab ){
      పూర్ణాంకం iTabDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_సంకేతం_Verify_Schema(pParse, iTabDb);
      కొరకు(pIdx=pTab->p_సూచీ, i=0; pIdx; pIdx=pIdx->p_తదుపరి, i++){
        స్థిరరాశి అక్షరం *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_సూచీ(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  ఆపు;

  స్థితి PragTyp_DATABASE_LIST: {
    పూర్ణాంకం i;
    pParse->nMem = 3;
    కొరకు(i=0; i<db->nDb; i++){
      ఒకవేళ( db->aDb[i].pBt==0 ) కొనసాగు;
      ప్రతిజ్ఞా( db->aDb[i].zDbSName!=0 );
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  ఆపు;

  స్థితి PragTyp_COLLATION_LIST: {
    పూర్ణాంకం i = 0;
    HashElem *p;
    pParse->nMem = 2;
    కొరకు(p=sqlite_Hash_మొదటి(&db->aCollSeq); p; p=sqlite_Hash_తదుపరి(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_దత్తాంశం(p);
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "is", i++, pColl->zName);
    }
  }
  ఆపు;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTROSPECTION_PRAGMAS
  స్థితి PragTyp_FUNCTION_LIST: {
    పూర్ణాంకం i;
    HashElem *j;
    FuncDef *p;
    పూర్ణాంకం showInternFunc = (db->mDbFlags & DBFLAG_అంతర్గత_Func)!=0;
    pParse->nMem = 6;
    కొరకు(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      కొరకు(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        ప్రతిజ్ఞా( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_పంక్తి(v, p, 1, showInternFunc);
      }
    }
    కొరకు(j=sqlite_Hash_మొదటి(&db->aFunc); j; j=sqlite_Hash_తదుపరి(j)){
      p = (FuncDef*)sqlite_Hash_దత్తాంశం(j);
      ప్రతిజ్ఞా( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_పంక్తి(v, p, 0, showInternFunc);
    }
  }
  ఆపు;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  స్థితి PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    కొరకు(j=sqlite_Hash_మొదటి(&db->aModule); j; j=sqlite_Hash_తదుపరి(j)){
      Module *pMod = (Module*)sqlite_Hash_దత్తాంశం(j);
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "s", pMod->zName);
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

  స్థితి PragTyp_PRAGMA_LIST: {
    పూర్ణాంకం i;
    కొరకు(i=0; i<Array_కొలత(aPragmaName); i++){
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "s", aPragmaName[i].zName);
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_INTROSPECTION_PRAGMAS */

#అయితే_ముగింపు /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
  స్థితి Prag_Typ_FOREIGN_కీలకం_LIST: ఒకవేళ( z_కుడి ){
    FKey *pFK;
    పట్టిక *pTab;
    pTab = sqlite3_కనుగొను_పట్టిక(db, z_కుడి, zDb);
    ఒకవేళ( pTab && Is_Ordinary_పట్టిక(pTab) ){
      pFK = pTab->u.tab.pFKey;
      ఒకవేళ( pFK ){
        పూర్ణాంకం iTabDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
        పూర్ణాంకం i = 0;
        pParse->nMem = 8;
        sqlite3_సంకేతం_Verify_Schema(pParse, iTabDb);
        అయితే(pFK){
          పూర్ణాంకం j;
          కొరకు(j=0; j<pFK->n_నిలువు; j++){
            sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_గమ్యం,
                   pTab->a_నిలువు[pFK->a_నిలువు[j].iFrom].zCnName,
                   pFK->a_నిలువు[j].z_నిలువు,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->p_తదుపరి_From;
        }
      }
    }
  }
  ఆపు;
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  స్థితి Prag_Typ_FOREIGN_కీలకం_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    పట్టిక *pTab;           /* Child table contain "REFERENCES" keyword */
    పట్టిక *p_మాతృ;        /* Parent table that child points to */
    సూచీ *pIdx;           /* Index in the parent table */
    పూర్ణాంకం i;                 /* Loop counter:  Foreign key number for pTab */
    పూర్ణాంకం j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    పూర్ణాంకం x;                 /* result variable */
    పూర్ణాంకం reg_ఫలితం;         /* 3 registers to hold a result row */
    పూర్ణాంకం reg_అడ్డువరుస;            /* Registers to hold a row from pTab */
    పూర్ణాంకం addr_శిఖరం;           /* Top of a loop checking foreign keys */
    పూర్ణాంకం addrOk;            /* Jump here if the key is OK */
    పూర్ణాంకం *ai_నిలువుల_సంఖ్య;           /* child to parent column mapping */

    reg_ఫలితం = pParse->nMem+1;
    pParse->nMem += 4;
    reg_అడ్డువరుస = ++pParse->nMem;
    k = sqlite_Hash_మొదటి(&db->aDb[iDb].pSchema->tblHash);
    అయితే( k ){
      ఒకవేళ( z_కుడి ){
        pTab = sqlite3_Locate_పట్టిక(pParse, 0, z_కుడి, zDb);
        k = 0;
      }లేకపోతే{
        pTab = (పట్టిక*)sqlite_Hash_దత్తాంశం(k);
        k = sqlite_Hash_తదుపరి(k);
      }
      ఒకవేళ( pTab==0 || !Is_Ordinary_పట్టిక(pTab) || pTab->u.tab.pFKey==0 ) కొనసాగు;
      iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
      sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_రిజిస్టరు(pParse, pTab->n_నిలువు+reg_అడ్డువరుస);
      sqlite3_Open_పట్టిక(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ఎక్కించు_String(v, reg_ఫలితం, pTab->zName);
      ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
      కొరకు(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_తదుపరి_From){
        p_మాతృ = sqlite3_కనుగొను_పట్టిక(db, pFK->z_గమ్యం, zDb);
        ఒకవేళ( p_మాతృ==0 ) కొనసాగు;
        pIdx = 0;
        sqlite3_పట్టిక_తాళం(pParse, iDb, p_మాతృ->tnum, 0, p_మాతృ->zName);
        x = sqlite3_Fk_Locate_సూచీ(pParse, p_మాతృ, pFK, &pIdx, 0);
        ఒకవేళ( x==0 ){
          ఒకవేళ( pIdx==0 ){
            sqlite3_Open_పట్టిక(pParse, i, iDb, p_మాతృ, OP_OpenRead);
          }లేకపోతే{
            sqlite3_Vdbe_కూడు_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
          }
        }లేకపోతే{
          k = 0;
          ఆపు;
        }
      }
      ప్రతిజ్ఞా( pParse->nErr>0 || pFK==0 );
      ఒకవేళ( pFK ) ఆపు;
      ఒకవేళ( pParse->nTab<i ) pParse->nTab = i;
      addr_శిఖరం = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
      కొరకు(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_తదుపరి_From){
        p_మాతృ = sqlite3_కనుగొను_పట్టిక(db, pFK->z_గమ్యం, zDb);
        pIdx = 0;
        ai_నిలువుల_సంఖ్య = 0;
        ఒకవేళ( p_మాతృ ){
          x = sqlite3_Fk_Locate_సూచీ(pParse, p_మాతృ, pFK, &pIdx, &ai_నిలువుల_సంఖ్య);
          ప్రతిజ్ఞా( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_రిజిస్టరు(pParse, reg_అడ్డువరుస + pFK->n_నిలువు);
        కొరకు(j=0; j<pFK->n_నిలువు; j++){
          పూర్ణాంకం i_నిలువు = ai_నిలువుల_సంఖ్య ? ai_నిలువుల_సంఖ్య[j] : pFK->a_నిలువు[j].iFrom;
          sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, 0, i_నిలువు, reg_అడ్డువరుస+j);
          sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, reg_అడ్డువరుస+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        ఒకవేళ( pIdx ){
          sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, reg_అడ్డువరుస, pFK->n_నిలువు, 0,
              sqlite3_సూచీ_Affinity_Str(db,pIdx), pFK->n_నిలువు);
          sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, i, addrOk, reg_అడ్డువరుస, pFK->n_నిలువు);
          VdbeCoverage(v);
        }లేకపోతే ఒకవేళ( p_మాతృ ){
          పూర్ణాంకం jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_కూడు_Op3(v, OP_SeekRowid, i, jmp, reg_అడ్డువరుస); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          ప్రతిజ్ఞా( pFK->n_నిలువు==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        ఒకవేళ( HasRowid(pTab) ){
          sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, 0, reg_ఫలితం+1);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_ఫలితం+1);
        }
        sqlite3_Vdbe_Multi_ఎక్కించు(v, reg_ఫలితం+2, "siX", pFK->z_గమ్యం, i-1);
        sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, reg_ఫలితం, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_నిలువుల_సంఖ్య);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, 0, addr_శిఖరం+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం);
    }
  }
  ఆపు;
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_TRIGGER) */
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  స్థితి PragTyp_CASE_SENSITIVE_LIKE: {
    ఒకవేళ( z_కుడి ){
      sqlite3_రిజిస్టరు_Like_Functions(db, sqlite3GetBoolean(z_కుడి, 0));
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#నిర్వచితం_కాకపోతే SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK
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
  స్థితి PragTyp_INTEGRITY_CHECK: {
    పూర్ణాంకం i, j, addr, mxErr;
    పట్టిక *pObjTab = 0;     /* Check only this one table, if not NULL */

    పూర్ణాంకం is_శీఘ్ర = (sqlite3Tolower(z_ఎడమ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    ప్రతిజ్ఞా( iDb>=0 );
    ప్రతిజ్ఞా( iDb==0 || pId2->z );
    ఒకవేళ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    ఒకవేళ( z_కుడి ){
      ఒకవేళ( sqlite3GetInt32(p_విలువ->z, &mxErr) ){
        ఒకవేళ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }లేకపోతే{
        pObjTab = sqlite3_Locate_పట్టిక(pParse, 0, z_కుడి,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    కొరకు(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      పూర్ణాంకం *a_మూలం;      /* Array of root page numbers of all btrees */
      పూర్ణాంకం cnt = 0;     /* Number of entries in aRoot[] */

      ఒకవేళ( OMIT_TEMPDB && i==1 ) కొనసాగు;
      ఒకవేళ( iDb>=0 && i!=iDb ) కొనసాగు;

      sqlite3_సంకేతం_Verify_Schema(pParse, i);
      pParse->ok_Const_గుణకం = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      కొరకు(cnt=0, x=sqlite_Hash_మొదటి(pTbls); x; x=sqlite_Hash_తదుపరి(x)){
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(x);  /* Current table */
        సూచీ *pIdx;                      /* An index on pTab */
        ఒకవేళ( పట్టిక_Skip_Integrity_Check(pTab,pObjTab) ) కొనసాగు;
        ఒకవేళ( HasRowid(pTab) ) cnt++;
        కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){ cnt++; }
      }
      ఒకవేళ( cnt==0 ) కొనసాగు;
      ఒకవేళ( pObjTab ) cnt++;
      a_మూలం = sqlite3DbMallocRawNN(db, పరిమాణం(పూర్ణాంకం)*(cnt+1));
      ఒకవేళ( a_మూలం==0 ) ఆపు;
      cnt = 0;
      ఒకవేళ( pObjTab ) a_మూలం[++cnt] = 0;
      కొరకు(x=sqlite_Hash_మొదటి(pTbls); x; x=sqlite_Hash_తదుపరి(x)){
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(x);
        సూచీ *pIdx;
        ఒకవేళ( పట్టిక_Skip_Integrity_Check(pTab,pObjTab) ) కొనసాగు;
        ఒకవేళ( HasRowid(pTab) ) a_మూలం[++cnt] = pTab->tnum;
        కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
          a_మూలం[++cnt] = pIdx->tnum;
        }
      }
      a_మూలం[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_రిజిస్టరు(pParse, 8+cnt);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_కూడు_Op4(v, OP_IntegrityCk, 1, cnt, 8, (అక్షరం*)a_మూలం,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_ఫలితం_అడ్డువరుస(v);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_ఎక్కించు_String(v, 2, "wrong # of entries in index ");
      కొరకు(x=sqlite_Hash_మొదటి(pTbls); x; x=sqlite_Hash_తదుపరి(x)){
        పూర్ణాంకం iTab = 0;
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(x);
        సూచీ *pIdx;
        ఒకవేళ( పట్టిక_Skip_Integrity_Check(pTab,pObjTab) ) కొనసాగు;
        ఒకవేళ( HasRowid(pTab) ){
          iTab = cnt++;
        }లేకపోతే{
          iTab = cnt;
          కొరకు(pIdx=pTab->p_సూచీ; ALWAYS(pIdx); pIdx=pIdx->p_తదుపరి){
            ఒకవేళ( Is_Primary_కీలకం_సూచీ(pIdx) ) ఆపు;
            iTab++;
          }
        }
        కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
          ఒకవేళ( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_ఎక్కించు_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_ఫలితం_అడ్డువరుస(v);
            sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      కొరకు(x=sqlite_Hash_మొదటి(pTbls); x; x=sqlite_Hash_తదుపరి(x)){
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(x);
        సూచీ *pIdx, *pPk;
        సూచీ *p_పూర్వ = 0;      /* Previous index */
        పూర్ణాంకం loop_శిఖరం;
        పూర్ణాంకం i_దత్తాంశం_Cur, iIdxCur;
        పూర్ణాంకం r1 = -1;
        పూర్ణాంకం bStrict;            /* True for a STRICT table */
        పూర్ణాంకం r2;                 /* Previous key for WITHOUT ROWID tables */
        పూర్ణాంకం mx_నిలువు;              /* Maximum non-virtual column number */

        ఒకవేళ( పట్టిక_Skip_Integrity_Check(pTab,pObjTab) ) కొనసాగు;
        ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ) కొనసాగు;
        ఒకవేళ( is_శీఘ్ర || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }లేకపోతే{
          pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
          r2 = sqlite3_Get_తాత్కాలిక_Range(pParse, pPk->n_కీలకం_నిలువు);
          sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 1, r2, r2+pPk->n_కీలకం_నిలువు-1);
        }
        sqlite3_Open_పట్టిక_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_దత్తాంశం_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, 7);
        కొరకు(j=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, j++){
          sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        ప్రతిజ్ఞా( pParse->nMem>=8+j );
        ప్రతిజ్ఞా( sqlite3_No_Temps_లోపల_Range(pParse,1,7+j) );
        sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, i_దత్తాంశం_Cur, 0); VdbeCoverage(v);
        loop_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        ప్రతిజ్ఞా( !IsVirtual(pTab) );
        ఒకవేళ( HasRowid(pTab) ){
          mx_నిలువు = -1;
          కొరకు(j=0; j<pTab->n_నిలువు; j++){
            ఒకవేళ( (pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_VIRTUAL)==0 ) mx_నిలువు++;
          }
          ఒకవేళ( mx_నిలువు==pTab->iPKey ) mx_నిలువు--;
        }లేకపోతే{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mx_నిలువు = sqlite3_Primary_కీలకం_సూచీ(pTab)->n_స్తంభం-1;
        }
        ఒకవేళ( mx_నిలువు>=0 ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_దత్తాంశం_Cur, mx_నిలువు, 3);
          sqlite3_Vdbe_Typeof_స్తంభం(v, 3);
        }

        ఒకవేళ( !is_శీఘ్ర ){
          ఒకవేళ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            పూర్ణాంకం a1;
            అక్షరం *zErr;
            a1 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IdxGT, i_దత్తాంశం_Cur, 0,r2,pPk->n_కీలకం_నిలువు);
            VdbeCoverage(v);
            sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ఫలితం_అడ్డువరుస(v);
            sqlite3_Vdbe_Jump_ఇక్కడ(v, a1);
            sqlite3_Vdbe_Jump_ఇక్కడ(v, a1+1);
            కొరకు(j=0; j<pPk->n_కీలకం_నిలువు; j++){
              sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(pParse, pPk, i_దత్తాంశం_Cur, j, r2+j);
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
        కొరకు(j=0; j<pTab->n_నిలువు; j++){
          అక్షరం *zErr;
          స్తంభం *p_నిలువు = pTab->a_నిలువు + j;  /* The column to be checked */
          పూర్ణాంకం labelError;               /* Jump here to report an error */
          పూర్ణాంకం labelOk;                  /* Jump here if all looks ok */
          పూర్ణాంకం p1, p3, p4;               /* Operands to the OP_IsType opcode */
          పూర్ణాంకం doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          ఒకవేళ( j==pTab->iPKey ) కొనసాగు;
          ఒకవేళ( bStrict ){
            doTypeCheck = p_నిలువు->eCType>COLTYPE_ANY;
          }లేకపోతే{
            doTypeCheck = p_నిలువు->affinity>SQLITE_AFF_BLOB;
          }
          ఒకవేళ( p_నిలువు->notNull==0 && !doTypeCheck ) కొనసాగు;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL ){
            sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }లేకపోతే{
            ఒకవేళ( p_నిలువు->iDflt ){
              sqlite3_విలువ *p_Dflt_విలువ = 0;
              sqlite3_విలువ_From_సమాసం(db, sqlite3_స్తంభం_సమాసం_Auth(pTab,p_నిలువు,pParse),
                                   ENC(db), p_నిలువు->affinity, &p_Dflt_విలువ);
              ఒకవేళ( p_Dflt_విలువ ){
                p4 = sqlite3_విలువ_type(p_Dflt_విలువ);
                sqlite3_విలువ_Free(p_Dflt_విలువ);
              }
            }
            p1 = i_దత్తాంశం_Cur;
            ఒకవేళ( !HasRowid(pTab) ){
              testcase( j!=sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, j) );
              p3 = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(sqlite3_Primary_కీలకం_సూచీ(pTab), j);
            }లేకపోతే{
              p3 = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          ఒకవేళ( p_నిలువు->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            పూర్ణాంకం jmp3;
            పూర్ణాంకం jmp2 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            ఒకవేళ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }లేకపోతే{
              sqlite3VdbeChangeP5(v, 0x0d); /* INT, TEXT, or BLOB */
              /* OP_IsType does not detect NaN values in the database file
              ** which should be treated as a NULL.  So if the header type
              ** is REAL, we have to load the actual data using OP_Column
              ** to reliably determine if the value is a NULL. */
              sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, p1, p3, 3);
              sqlite3_స్తంభం_Default(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_కూడు_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                p_నిలువు->zCnName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            ఒకవేళ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp2);
              sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp3);
            }లేకపోతే{
              /* VDBE byte code will fall thru */
            }
          }
          ఒకవేళ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            స్థిర గుర్తులేని అక్షరం a_Std_Type_ముసుగు[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /*పాఠ్యం*/
            };
            sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            ప్రతిజ్ఞా( p_నిలువు->eCType>=1 && p_నిలువు->eCType<=పరిమాణం(a_Std_Type_ముసుగు) );
            sqlite3VdbeChangeP5(v, a_Std_Type_ముసుగు[p_నిలువు->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[p_నిలువు->eCType-1],
                                  pTab->zName, pTab->a_నిలువు[j].zCnName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }లేకపోతే ఒకవేళ( !bStrict && p_నిలువు->affinity==SQLITE_AFF_పాఠ్యం ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->a_నిలువు[j].zCnName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }లేకపోతే ఒకవేళ( !bStrict && p_నిలువు->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            ఒకవేళ( p1>=0 ){
              sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, i_దత్తాంశం_Cur, j, 3);
            }
            sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->a_నిలువు[j].zCnName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_ఫలితం_అడ్డువరుస(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        ఒకవేళ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          సమాసం_List *pCheck = sqlite3_సమాసం_List_Dup(db, pTab->pCheck, 0);
          ఒకవేళ( db->mallocFailed==0 ){
            పూర్ణాంకం addrCkFault = sqlite3VdbeMakeLabel(pParse);
            పూర్ణాంకం addrCkOk = sqlite3VdbeMakeLabel(pParse);
            అక్షరం *zErr;
            పూర్ణాంకం k;
            pParse->iSelfTab = i_దత్తాంశం_Cur + 1;
            కొరకు(k=pCheck->n_సమాసం-1; k>0; k--){
              sqlite3_సమాసం_If_False(pParse, pCheck->a[k].p_సమాసం, addrCkFault, 0);
            }
            sqlite3_సమాసం_If_True(pParse, pCheck->a[0].p_సమాసం, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ఫలితం_అడ్డువరుస(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_సమాసం_List_Delete(db, pCheck);
        }
        ఒకవేళ( !is_శీఘ్ర ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          కొరకు(j=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, j++){
            పూర్ణాంకం jmp2, jmp3, jmp4, jmp5, label6;
            పూర్ణాంకం kk;
            పూర్ణాంకం ckUniq = sqlite3VdbeMakeLabel(pParse);
            ఒకవేళ( pPk==pIdx ) కొనసాగు;
            r1 = sqlite3_Generate_సూచీ_కీలకం(pParse, pIdx, i_దత్తాంశం_Cur, 0, 0, &jmp3,
                                         p_పూర్వ, r1);
            p_పూర్వ = pIdx;
            sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iIdxCur+j, ckUniq, r1,
                                        pIdx->n_స్తంభం); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_కూడు_Op3(v, OP_IFind_కీలకం, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (స్థిరరాశి అక్షరం*)pIdx, P4_సూచీ);
            sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_ఫలితం_అడ్డువరుస(v);
            sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp2);
            sqlite3_Vdbe_ఎక్కించు_String(v, 3, "row ");
            sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_ఎక్కించు_String(v, 4, " missing from index ");
            sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_ఎక్కించు_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_ఫలితం_అడ్డువరుస(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            ఒకవేళ( HasRowid(pTab) ){
              పూర్ణాంకం jmp7;
              sqlite3_Vdbe_కూడు_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, 3, 0, r1+pIdx->n_స్తంభం-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_ఎక్కించు_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ఎక్కించు_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            కొరకు(kk=0; kk<pIdx->n_కీలకం_నిలువు; kk++){
              ఒకవేళ( pIdx->azColl[kk]==sqlite3_Str_ద్విమాన ) కొనసాగు;
              ఒకవేళ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_కూడు_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            ఒకవేళ( label6 ){
              పూర్ణాంకం jmp6 = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_ఎక్కించు_String(v, 3, "row ");
              sqlite3_Vdbe_కూడు_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ఎక్కించు_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            ఒకవేళ( Is_Unique_సూచీ(pIdx) ){
              పూర్ణాంకం uniqOk = sqlite3VdbeMakeLabel(pParse);
              పూర్ణాంకం jmp6;
              కొరకు(kk=0; kk<pIdx->n_కీలకం_నిలువు; kk++){
                పూర్ణాంకం i_నిలువు = pIdx->ai_స్తంభం[kk];
                ప్రతిజ్ఞా( i_నిలువు!=XN_ROWID && i_నిలువు<pTab->n_నిలువు );
                ఒకవేళ( i_నిలువు>=0 && pTab->a_నిలువు[i_నిలువు].notNull ) కొనసాగు;
                sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_కూడు_Op1(v, OP_తదుపరి, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp6);
              sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_కీలకం_నిలువు); VdbeCoverage(v);
              sqlite3_Vdbe_ఎక్కించు_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, i_దత్తాంశం_Cur, loop_శిఖరం); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, loop_శిఖరం-1);
        ఒకవేళ( pPk ){
          ప్రతిజ్ఞా( !is_శీఘ్ర );
          sqlite3_Release_తాత్కాలిక_Range(pParse, r2, pPk->n_కీలకం_నిలువు);
        }
      }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      కొరకు(x=sqlite_Hash_మొదటి(pTbls); x; x=sqlite_Hash_తదుపరి(x)){
        పట్టిక *pTab = sqlite_Hash_దత్తాంశం(x);
        sqlite3_vtab *pVTab;
        పూర్ణాంకం a1;
        ఒకవేళ( పట్టిక_Skip_Integrity_Check(pTab,pObjTab) ) కొనసాగు;
        ఒకవేళ( Is_Ordinary_పట్టిక(pTab) ) కొనసాగు;
        ఒకవేళ( !IsVirtual(pTab) ) కొనసాగు;
        ఒకవేళ( pTab->n_నిలువు<=0 ){
          స్థిరరాశి అక్షరం *zMod = pTab->u.vtab.azArg[0];
          ఒకవేళ( sqlite3_Hash_కనుగొను(&db->aModule, zMod)==0 ) కొనసాగు;
        }
        sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab);
        ఒకవేళ( pTab->u.vtab.p==0 ) కొనసాగు;
        pVTab = pTab->u.vtab.p->pVtab;
        ఒకవేళ( NEVER(pVTab==0) ) కొనసాగు;
        ఒకవేళ( NEVER(pVTab->pModule==0) ) కొనసాగు;
        ఒకవేళ( pVTab->pModule->i_సంస్కరణ<4 ) కొనసాగు;
        ఒకవేళ( pVTab->pModule->xIntegrity==0 ) కొనసాగు;
        sqlite3_Vdbe_కూడు_Op3(v, OP_VCheck, i, 3, is_శీఘ్ర);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_ఫలితం_అడ్డువరుస(v);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, a1);
        కొనసాగు;
      }
#అయితే_ముగింపు
    }
    {
      స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
      స్థిర స్థిరరాశి VdbeOpList చివర_సంకేతం[] = {
        { OP_కూడు_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_ఫలితం_అడ్డువరుస,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(చివర_సంకేతం), చివర_సంకేతం, iLn);
      ఒకవేళ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (అక్షరం*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
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
  స్థితి PragTyp_ENCODING: {
    స్థిర స్థిరరాశి నిర్మాణం EncName {
      అక్షరం *zName;
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
    స్థిరరాశి నిర్మాణం EncName *pEnc;
    ఒకవేళ( !z_కుడి ){    /* "PRAGMA encoding" */
      ఒకవేళ( sqlite3ReadSchema(pParse) ) వెళ్ళు pragma_బహిర్గతం;
      ప్రతిజ్ఞా( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      ప్రతిజ్ఞా( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      ప్రతిజ్ఞా( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      return_Single_పాఠ్యం(v, encnames[ENC(pParse->db)].zName);
    }లేకపోతే{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      ఒకవేళ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        కొరకు(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          ఒకవేళ( 0==sqlite3StrICmp(z_కుడి, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_సమితి_పాఠ్యం_Encoding(db, enc);
            ఆపు;
          }
        }
        ఒకవేళ( !pEnc->zName ){
          sqlite3_Error_సందేశం(pParse, "unsupported encoding: %s", z_కుడి);
        }
      }
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SCHEMA_సంస్కరణ_PRAGMAS
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
  స్థితి Prag_Typ_శీర్షిక_విలువ: {
    పూర్ణాంకం iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb);
    ఒకవేళ( z_కుడి && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      స్థిర స్థిరరాశి VdbeOpList సమితి_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_సమితి_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_కొలత(సమితి_Cookie));
      aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(సమితి_Cookie), సమితి_Cookie, 0);
      ఒకవేళ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ఆపు;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_కుడి);
      aOp[1].p5 = 1;
      ఒకవేళ( iCookie==BTREE_SCHEMA_సంస్కరణ && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }లేకపోతే{
      /* Read the specified cookie value */
      స్థిర స్థిరరాశి VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ఫలితం_అడ్డువరుస,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_కొలత(readCookie));
      aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(readCookie),readCookie,0);
      ఒకవేళ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ఆపు;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  స్థితి PragTyp_COMPILE_OPTIONS: {
    పూర్ణాంకం i = 0;
    స్థిరరాశి అక్షరం *zOpt;
    pParse->nMem = 1;
    అయితే( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3_Vdbe_ఎక్కించు_String(v, 1, zOpt);
      sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  ఆపు;
#అయితే_ముగింపు /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  స్థితి PragTyp_WAL_CHECKPOINT: {
    పూర్ణాంకం iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    పూర్ణాంకం e_బహుళకం = SQLITE_CHECKPOINT_PASSIVE;
    ఒకవేళ( z_కుడి ){
      ఒకవేళ( sqlite3StrICmp(z_కుడి, "full")==0 ){
        e_బహుళకం = SQLITE_CHECKPOINT_నిండిన;
      }లేకపోతే ఒకవేళ( sqlite3StrICmp(z_కుడి, "restart")==0 ){
        e_బహుళకం = SQLITE_CHECKPOINT_RESTART;
      }లేకపోతే ఒకవేళ( sqlite3StrICmp(z_కుడి, "truncate")==0 ){
        e_బహుళకం = SQLITE_CHECKPOINT_TRUNCATE;
      }లేకపోతే ఒకవేళ( sqlite3StrICmp(z_కుడి, "noop")==0 ){
        e_బహుళకం = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_కూడు_Op3(v, OP_Checkpoint, iBt, e_బహుళకం, 1);
    sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, 1, 3);
  }
  ఆపు;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  స్థితి PragTyp_WAL_AUTOCHECKPOINT: {
    ఒకవేళ( z_కుడి ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_కుడి));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_గమ్యం_INT(db->pWalArg) : 0);
  }
  ఆపు;
#అయితే_ముగింపు

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  స్థితి Prag_Typ_SHRINK_జ్ఞాపకం: {
    sqlite3_db_release_జ్ఞాపకం(db);
    ఆపు;
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
  స్థితి PragTyp_OPTIMIZE: {
    పూర్ణాంకం i_Db_చివరి;           /* Loop termination point for the schema loop */
    పూర్ణాంకం iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    పట్టిక *pTab;           /* A table in the schema */
    సూచీ *pIdx;           /* An index of the table */
    LogEst sz_పరిమితి;    /* Size threshold above which reanalysis needed */
    అక్షరం *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_ముసుగు;            /* Mask of operations to perform */
    పూర్ణాంకం n_హద్దు;            /* Analysis limit to use */
    పూర్ణాంకం nCheck = 0;        /* Number of tables to be optimized */
    పూర్ణాంకం nBtree = 0;        /* Number of btrees to scan */
    పూర్ణాంకం n_సూచీ;            /* Number of indexes on the current table */

    ఒకవేళ( z_కుడి ){
      op_ముసుగు = (u32)sqlite3Atoi(z_కుడి);
      ఒకవేళ( (op_ముసుగు & 0x02)==0 ) ఆపు;
    }లేకపోతే{
      op_ముసుగు = 0xfffe;
    }
    ఒకవేళ( (op_ముసుగు & 0x10)==0 ){
      n_హద్దు = 0;
    }లేకపోతే ఒకవేళ( db->n_Analysis_హద్దు>0
           && db->n_Analysis_హద్దు<SQLITE_DEFAULT_OPTIMIZE_హద్దు ){
      n_హద్దు = 0;
    }లేకపోతే{
      n_హద్దు = SQLITE_DEFAULT_OPTIMIZE_హద్దు;
    }
    iTabCur = pParse->nTab++;
    కొరకు(i_Db_చివరి = zDb?iDb:db->nDb-1; iDb<=i_Db_చివరి; iDb++){
      ఒకవేళ( iDb==1 ) కొనసాగు;
      sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      కొరకు(k=sqlite_Hash_మొదటి(&pSchema->tblHash); k; k=sqlite_Hash_తదుపరి(k)){
        pTab = (పట్టిక*)sqlite_Hash_దత్తాంశం(k);

        /* This only works for ordinary tables */
        ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ) కొనసాగు;

        /* Do not scan system tables */
        ఒకవేళ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) కొనసాగు;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_పరిమితి = pTab->n_అడ్డువరుస_Log_Est;
        n_సూచీ = 0;
        కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
          n_సూచీ++;
          ఒకవేళ( !pIdx->hasStat1 ){
            sz_పరిమితి = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        ఒకవేళ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }లేకపోతే ఒకవేళ( op_ముసుగు & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }లేకపోతే ఒకవేళ( pTab->p_సూచీ!=0 && sz_పరిమితి<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }లేకపోతే{
          /* Otherwise, we can skip this table */
          కొనసాగు;
        }

        nCheck++;
        ఒకవేళ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_సూచీ+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_పట్టిక(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        ఒకవేళ( sz_పరిమితి>=0 ){
          స్థిరరాశి LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_కూడు_Op4_Int(v, OP_If_కొలత_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ముసుగు&1),
                         sz_పరిమితి>=iRange ? sz_పరిమితి-iRange : -1,
                         sz_పరిమితి+iRange);
          VdbeCoverage(v);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ముసుగు&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        ఒకవేళ( op_ముసుగు & 0x01 ){
          పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
          sqlite3_Vdbe_కూడు_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, r1, 1);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op4(v, OP_SqlExec, n_హద్దు ? 0x02 : 00, n_హద్దు, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_కూడు_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    ఒకవేళ( !db->mallocFailed && n_హద్దు>0 && nBtree>100 ){
      పూర్ణాంకం iAddr, i_చివర;
      VdbeOp *aOp;
      n_హద్దు = 100*n_హద్దు/nBtree;
      ఒకవేళ( n_హద్దు<100 ) n_హద్దు = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_చివర = sqlite3VdbeCurrentAddr(v);
      కొరకు(iAddr=0; iAddr<i_చివర; iAddr++){
        ఒకవేళ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_హద్దు;
      }
    }
    ఆపు;
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
  /*case PragTyp_BUSY_TIMEOUT*/ అప్రమేయ: {
    పూర్ణాంకం n = 0;
    ప్రతిజ్ఞా( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    ఒకవేళ( z_కుడి && sqlite3GetInt32(z_కుడి,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    ఆపు;
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
  స్థితి Prag_Typ_SOFT_కుప్ప_హద్దు: {
    sqlite3_int64 N;
    ఒకవేళ( z_కుడి && sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &N)==SQLITE_OK ){
      sqlite3_soft_కుప్ప_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_కుప్ప_limit64(-1));
    ఆపు;
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
  స్థితి Prag_Typ_HARD_కుప్ప_హద్దు: {
    sqlite3_int64 N;
    ఒకవేళ( z_కుడి && sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &N)==SQLITE_OK ){
      sqlite3_int64 i_పూర్వ = sqlite3_hard_కుప్ప_limit64(-1);
      ఒకవేళ( N>0 && (i_పూర్వ==0 || i_పూర్వ>N) ) sqlite3_hard_కుప్ప_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_కుప్ప_limit64(-1));
    ఆపు;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  స్థితి Prag_Typ_దారాలు: {
    sqlite3_int64 N;
    ఒకవేళ( z_కుడి
     && sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_హద్దు(db, SQLITE_హద్దు_కార్మికుడు_దారాలు, (పూర్ణాంకం)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_హద్దు(db, SQLITE_హద్దు_కార్మికుడు_దారాలు, -1));
    ఆపు;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  స్థితి Prag_Typ_ANALYSIS_హద్దు: {
    sqlite3_int64 N;
    ఒకవేళ( z_కుడి
     && sqlite3_Dec_Or_Hex_గమ్యం_I64(z_కుడి, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_హద్దు = (పూర్ణాంకం)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_హద్దు); /* IMP: R-57594-65522 */
    ఆపు;
  }

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  స్థితి Prag_Typ_తాళం_స్థితి: {
    స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి az_తాళం_Name[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    పూర్ణాంకం i;
    pParse->nMem = 2;
    కొరకు(i=0; i<db->nDb; i++){
      Btree *pBt;
      స్థిరరాశి అక్షరం *zState = "unknown";
      పూర్ణాంకం j;
      ఒకవేళ( db->aDb[i].zDbSName==0 ) కొనసాగు;
      pBt = db->aDb[i].pBt;
      ఒకవేళ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }లేకపోతే ఒకవేళ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = az_తాళం_Name[j];
      }
      sqlite3_Vdbe_Multi_ఎక్కించు(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    ఆపు;
  }
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_ENABLE_CEROD)
  స్థితి PragTyp_ACTIVATE_EXTENSIONS: ఒకవేళ( z_కుడి ){
    ఒకవేళ( sqlite3StrNICmp(z_కుడి, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_కుడి[6]);
    }
  }
  ఆపు;
#అయితే_ముగింపు

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  ఒకవేళ( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_కుడి ){
    sqlite3_Vdbe_Verify_No_ఫలితం_అడ్డువరుస(v);
  }

pragma_బహిర్గతం:
  sqlite3DbFree(db, z_ఎడమ);
  sqlite3DbFree(db, z_కుడి);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
రకనిర్వచనం నిర్మాణం PragmaVtab PragmaVtab;
రకనిర్వచనం నిర్మాణం PragmaVtabCursor PragmaVtabCursor;
నిర్మాణం PragmaVtab {
  sqlite3_vtab పునాది;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  స్థిరరాశి PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
నిర్మాణం PragmaVtabCursor {
  sqlite3_vtab_cursor పునాది; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  అక్షరం *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
స్థిర పూర్ణాంకం pragmaVtabConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  స్థిరరాశి PragmaName *pPragma = (స్థిరరాశి PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  పూర్ణాంకం rc;
  పూర్ణాంకం i, j;
  అక్షరం cSep = '(';
  StrAccum acc;
  అక్షరం zBuf[200];

  వాడని_PARAMETER(argc);
  వాడని_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  కొరకు(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  ఒకవేళ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  ఒకవేళ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  ఒకవేళ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3_Str_Accum_ముగింపు(&acc);
  ప్రతిజ్ఞా( అక్షరశ్రేణి_పొడవు(zBuf) < పరిమాణం(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  ఒకవేళ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(పరిమాణం(PragmaVtab));
    ఒకవేళ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }లేకపోతే{
      memset(pTab, 0, పరిమాణం(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }లేకపోతే{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  తిరిగి rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
స్థిర పూర్ణాంకం pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  తిరిగి SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
స్థిర పూర్ణాంకం pragma_Vtab_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  స్థిరరాశి నిర్మాణం sqlite3_సూచీ_constraint *pConstraint;
  పూర్ణాంకం i, j;
  పూర్ణాంకం చూసిన[2];

  pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)1;
  ఒకవేళ( pTab->nHidden==0 ){ తిరిగి SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  చూసిన[0] = 0;
  చూసిన[1] = 0;
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ఒకవేళ( pConstraint->i_స్తంభం < pTab->iHidden ) కొనసాగు;
    ఒకవేళ( pConstraint->op!=SQLITE_సూచీ_CONSTRAINT_EQ ) కొనసాగు;
    ఒకవేళ( pConstraint->usable==0 ) తిరిగి SQLITE_CONSTRAINT;
    j = pConstraint->i_స్తంభం - pTab->iHidden;
    ప్రతిజ్ఞా( j < 2 );
    చూసిన[j] = i+1;
  }
  ఒకవేళ( చూసిన[0]==0 ){
    pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)2147483647;
    pIdxInfo->estimated_అడ్డువరుసలు = 2147483647;
    తిరిగి SQLITE_OK;
  }
  j = చూసిన[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_సూచీ = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)20;
  pIdxInfo->estimated_అడ్డువరుసలు = 20;
  ఒకవేళ( చూసిన[1] ){
    j = చూసిన[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_సూచీ = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  తిరిగి SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
స్థిర పూర్ణాంకం pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(పరిమాణం(*pCsr));
  ఒకవేళ( pCsr==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCsr, 0, పరిమాణం(PragmaVtabCursor));
  pCsr->పునాది.pVtab = pVtab;
  *ppCursor = &pCsr->పునాది;
  తిరిగి SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
స్థిర శూన్య pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  పూర్ణాంకం i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  కొరకు(i=0; i<Array_కొలత(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
స్థిర పూర్ణాంకం pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  తిరిగి SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
స్థిర పూర్ణాంకం pragma_Vtab_తదుపరి(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  పూర్ణాంకం rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  ప్రతిజ్ఞా( pCsr->pPragma );
  ఒకవేళ( SQLITE_అడ్డువరుస!=sqlite3_అడుగు(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  తిరిగి rc;
}

/*
** Pragma virtual table module xFilter method.
*/
స్థిర పూర్ణాంకం pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  పూర్ణాంకం rc;
  పూర్ణాంకం i, j;
  StrAccum acc;
  అక్షరం *zSql;

  వాడని_PARAMETER(idx_అంకె);
  వాడని_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  కొరకు(i=0; i<argc; i++, j++){
    స్థిరరాశి అక్షరం *z_పాఠ్యం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    ప్రతిజ్ఞా( j<Array_కొలత(pCsr->azArg) );
    ప్రతిజ్ఞా( pCsr->azArg[j]==0 );
    ఒకవేళ( z_పాఠ్యం ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", z_పాఠ్యం);
      ఒకవేళ( pCsr->azArg[j]==0 ){
        తిరిగి SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_హద్దు[SQLITE_హద్దు_SQL_నిడివి]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  ఒకవేళ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  ఒకవేళ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3_Str_Accum_ముగింపు(&acc);
  ఒకవేళ( zSql==0 ) తిరిగి SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  ఒకవేళ( rc!=SQLITE_OK ){
    pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    తిరిగి rc;
  }
  తిరిగి pragma_Vtab_తదుపరి(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
స్థిర పూర్ణాంకం pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  తిరిగి (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
స్థిర పూర్ణాంకం pragma_Vtab_స్తంభం(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  పూర్ణాంకం i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ఒకవేళ( i<pTab->iHidden ){
    sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCsr->pPragma, i));
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  తిరిగి SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
స్థిర పూర్ణాంకం pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  తిరిగి SQLITE_OK;
}

/* The pragma virtual table object */
స్థిర స్థిరరాశి sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_ఉత్తమ_సూచీ,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_తదుపరి,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragma_Vtab_స్తంభం,            /* xColumn - read data */
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
Module *sqlite3_Pragma_Vtab_రిజిస్టరు(sqlite3 *db, స్థిరరాశి అక్షరం *zName){
  స్థిరరాశి PragmaName *pName;
  ప్రతిజ్ఞా( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  ఒకవేళ( pName==0 ) తిరిగి 0;
  ఒకవేళ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) తిరిగి 0;
  ప్రతిజ్ఞా( sqlite3_Hash_కనుగొను(&db->aModule, zName)==0 );
  తిరిగి sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (శూన్య*)pName, 0);
}

#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#అయితే_ముగింపు /* SQLITE_OMIT_PRAGMA */
