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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ਜੇ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ਨਹੀਂਤਾਂ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#ਜੇ_ਅੰਤ

/***************************************************************************
** The "pragma.h" include file is an automatically generated file that
** that includes the PragType_XXXX macro definitions and the aPragmaName[]
** object.  This ensures that the aPragmaName[] table is arranged in
** lexicographical order to facility a binary search of the pragma name.
** Do not edit pragma.h directly.  Edit and rerun the script in at
** ../tool/mkpragmatab.tcl. */
#ਅਨ੍ਤਰ੍ਭਾਵ "pragma.h"

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_OPTIMIZE_ਹੱਦਬੰਦੀ
# define SQLITE_DEFAULT_OPTIMIZE_ਹੱਦਬੰਦੀ 2000
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ u8 getSafetyLevel(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ omit_ਭਰਿਆ, u8 dflt){
                             /* 123456789 123456789 123 */
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ z_ਪਾਠ[] = "onoffalseyestruextrafull";
  ਸਥਿਰ ਅਚਲ u8 i_ਵਿੱਥ[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ਸਥਿਰ ਅਚਲ u8 i_ਲੰਮਾਈ[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ਸਥਿਰ ਅਚਲ u8 i_ਮੁੱਲ[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  ਪੂਰਨ i, n;
  ਜੇ( sqlite3Isdigit(*z) ){
    ਵਾਪਸ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  ਲਈ(i=0; i<Array_ਮਾਪ(i_ਲੰਮਾਈ); i++){
    ਜੇ( i_ਲੰਮਾਈ[i]==n && sqlite3StrNICmp(&z_ਪਾਠ[i_ਵਿੱਥ[i]],z,n)==0
     && (!omit_ਭਰਿਆ || i_ਮੁੱਲ[i]<=1)
    ){
      ਵਾਪਸ i_ਮੁੱਲ[i];
    }
  }
  ਵਾਪਸ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ਅਚਲ ਅੱਖਰ *z, u8 dflt){
  ਵਾਪਸ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#ਜੇ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
ਸਥਿਰ ਪੂਰਨ get_Locking_ਬਹੁਲਕ(ਅਚਲ ਅੱਖਰ *z){
  ਜੇ( z ){
    ਜੇ( 0==sqlite3StrICmp(z, "exclusive") ) ਵਾਪਸ PAGER_LOCKINGMODE_EXCLUSIVE;
    ਜੇ( 0==sqlite3StrICmp(z, "normal") ) ਵਾਪਸ PAGER_LOCKINGMODE_ਸਾਧਾਰਨ;
  }
  ਵਾਪਸ PAGER_LOCKINGMODE_QUERY;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ਸਥਿਰ ਪੂਰਨ getAutoVacuum(ਅਚਲ ਅੱਖਰ *z){
  ਪੂਰਨ i;
  ਜੇ( 0==sqlite3StrICmp(z, "none") ) ਵਾਪਸ BTREE_AUTOVACUUM_NONE;
  ਜੇ( 0==sqlite3StrICmp(z, "full") ) ਵਾਪਸ BTREE_AUTOVACUUM_ਭਰਿਆ;
  ਜੇ( 0==sqlite3StrICmp(z, "incremental") ) ਵਾਪਸ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  ਵਾਪਸ (u8)((i>=0&&i<=2)?i:0);
}
#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ਸਥਿਰ ਪੂਰਨ get_ਆਰਜ਼ੀ_Store(ਅਚਲ ਅੱਖਰ *z){
  ਜੇ( z[0]>='0' && z[0]<='2' ){
    ਵਾਪਸ z[0] - '0';
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(z, "file")==0 ){
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(z, "memory")==0 ){
    ਵਾਪਸ 2;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_PAGER_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ਸਥਿਰ ਪੂਰਨ invalidate_ਆਰਜ਼ੀ_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  ਜੇ( db->aDb[1].pBt!=0 ){
    ਜੇ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      ਵਾਪਸ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
  }
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_PAGER_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ਸਥਿਰ ਪੂਰਨ change_ਆਰਜ਼ੀ_Storage(Parse *pParse, ਅਚਲ ਅੱਖਰ *zStorageType){
  ਪੂਰਨ ts = get_ਆਰਜ਼ੀ_Store(zStorageType);
  sqlite3 *db = pParse->db;
  ਜੇ( db->ਆਰਜ਼ੀ_store==ts ) ਵਾਪਸ SQLITE_OK;
  ਜੇ( invalidate_ਆਰਜ਼ੀ_Storage( pParse ) != SQLITE_OK ){
    ਵਾਪਸ SQLITE_ERROR;
  }
  db->ਆਰਜ਼ੀ_store = (u8)ts;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Pragma_ਨਤੀਜਾ_ਕਾਲਮ_ਨਾਂ(
  Vdbe *v,                     /* The query under construction */
  ਅਚਲ PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_ਸੈੱਟ_ਅੰਕ_ਖਾਨੇ_ਗਿਣਤੀ(v, n==0 ? 1 : n);
  ਜੇ( n==0 ){
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i, j;
    ਲਈ(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ਸਥਿਰ ਖਾਲੀ returnSingleInt(Vdbe *v, i64 ਮੁੱਲ){
  sqlite3_Vdbe_ਜੋੜੋ_Int64(v, 1, ਮੁੱਲ);
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ਸਥਿਰ ਖਾਲੀ return_Single_ਪਾਠ(
  Vdbe *v,                /* Prepared statement under construction */
  ਅਚਲ ਅੱਖਰ *z_ਮੁੱਲ      /* Value to be returned */
){
  ਜੇ( z_ਮੁੱਲ ){
    sqlite3_Vdbe_ਲੱਦੋ_String(v, 1, (ਅਚਲ ਅੱਖਰ*)z_ਮੁੱਲ);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_All_Pager_Flags(sqlite3 *db){
  ਜੇ( db->autoCommit ){
    Db *pDb = db->aDb;
    ਪੂਰਨ n = db->nDb;
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_ਭਰਿਆ_FSync==PAGER_FULLFSYNC );
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_Ckpt_ਭਰਿਆ_FSync==PAGER_CKPT_FULLFSYNC );
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_CacheSpill==PAGER_CACHESPILL );
    ਪ੍ਰਤਿਜ੍ਞਾ( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_ਨਕਾਬ );
    ਪ੍ਰਤਿਜ੍ਞਾ( (pDb->safety_level & PAGER_SYNCHRONOUS_ਨਕਾਬ)==pDb->safety_level );
    ਜਦੋਂ( (n--) > 0 ){
      ਜੇ( pDb->pBt ){
        sqlite3_Btree_ਸੈੱਟ_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_ਨਕਾਬ) );
      }
      pDb++;
    }
  }
}
#ਨਹੀਂਤਾਂ
# define ਸੈੱਟ_All_Pager_Flags(X)  /* no-op */
#ਜੇ_ਅੰਤ


/*
** Return a human-readable name for a constraint resolution action.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *actionName(u8 action){
  ਅਚਲ ਅੱਖਰ *zName;
  ਚੋਣ( action ){
    ਹਾਲਤ OE_ਸੈੱਟ_Null:  zName = "SET NULL";        ਰੋਕੋ;
    ਹਾਲਤ OE_ਸੈੱਟ_Dflt:  zName = "SET DEFAULT";     ਰੋਕੋ;
    ਹਾਲਤ OE_Cascade:  zName = "CASCADE";         ਰੋਕੋ;
    ਹਾਲਤ OE_Restrict: zName = "RESTRICT";        ਰੋਕੋ;
    ਮੂਲ:          zName = "NO ACTION"; 
                      ਪ੍ਰਤਿਜ੍ਞਾ( action==OE_None ); ਰੋਕੋ;
  }
  ਵਾਪਸ zName;
}
#ਜੇ_ਅੰਤ


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ਅਚਲ ਅੱਖਰ *sqlite3JournalModename(ਪੂਰਨ e_ਬਹੁਲਕ){
  ਸਥਿਰ ਅੱਖਰ * ਅਚਲ az_ਬਹੁਲਕ_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
     , "wal"
#ਜੇ_ਅੰਤ
  };
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_DELETE==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_PERSIST==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_OFF==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_TRUNCATE==3 );
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_ਸਮ੍ਰਿਤੀ==4 );
  ਪ੍ਰਤਿਜ੍ਞਾ( PAGER_JOURNALMODE_WAL==5 );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ>=0 && e_ਬਹੁਲਕ<=Array_ਮਾਪ(az_ਬਹੁਲਕ_Name) );

  ਜੇ( e_ਬਹੁਲਕ==Array_ਮਾਪ(az_ਬਹੁਲਕ_Name) ) ਵਾਪਸ 0;
  ਵਾਪਸ az_ਬਹੁਲਕ_Name[e_ਬਹੁਲਕ];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ਸਥਿਰ ਅਚਲ PragmaName *pragmaLocate(ਅਚਲ ਅੱਖਰ *zName){
  ਪੂਰਨ upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_ਮਾਪ(aPragmaName)-1;
  ਜਦੋਂ( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    ਜੇ( rc==0 ) ਰੋਕੋ;
    ਜੇ( rc<0 ){
      upr = mid - 1;
    }ਨਹੀਂਤਾਂ{
      lwr = mid + 1;
    }
  }
  ਵਾਪਸ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
ਸਥਿਰ ਖਾਲੀ pragma_Funclist_ਸਤਰ(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  ਪੂਰਨ isBuiltin,         /* True if this is a built-in function */
  ਪੂਰਨ showInternFuncs    /* True if showing internal functions */
){
  u32 ਨਕਾਬ =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_ਅੰਦਰੂਨੀ
  ;
  ਜੇ( showInternFuncs ) ਨਕਾਬ = 0xffffffff;
  ਲਈ(; p; p=p->p_ਅਗਲਾ){
    ਅਚਲ ਅੱਖਰ *zType;
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_ENCMASK==0x3 );
    ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    ਜੇ( p->xSFunc==0 ) ਜਾਰੀ;
    ਜੇ( (p->funcFlags & SQLITE_FUNC_ਅੰਦਰੂਨੀ)!=0
     && showInternFuncs==0
    ){
      ਜਾਰੀ;
    }   
    ਜੇ( p->x_ਮੁੱਲ!=0 ){
      zType = "w";
    }ਨਹੀਂਤਾਂ ਜੇ( p->xFinalize!=0 ){
      zType = "a";
    }ਨਹੀਂਤਾਂ{
      zType = "s";
    }
    sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & ਨਕਾਬ) ^ SQLITE_INNOCUOUS
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
ਸਥਿਰ ਪੂਰਨ integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(Vdbe *v){
  ਪੂਰਨ addr;
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 3, 1);
  addr = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Halt);
  ਵਾਪਸ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
ਸਥਿਰ ਪੂਰਨ ਸਾਰਣੀ_Skip_Integrity_Check(ਅਚਲ ਸਾਰਣੀ *pTab, ਅਚਲ ਸਾਰਣੀ *pObjTab){
  ਜੇ( pObjTab ){
    ਵਾਪਸ pTab!=pObjTab;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ (pTab->tabFlags & TF_Imposter)!=0;
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
ਖਾਲੀ sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_ਮੁੱਲ,      /* Token for <value>, or NULL */
  ਪੂਰਨ minusFlag       /* True if a '-' sign preceded <value> */
){
  ਅੱਖਰ *z_ਖੱਬਾ = 0;       /* Nul-terminated UTF-8 string <id> */
  ਅੱਖਰ *z_ਸੱਜਾ = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ਅਚਲ ਅੱਖਰ *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  ਅੱਖਰ *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  ਪੂਰਨ iDb;               /* Database index for <database> */
  ਪੂਰਨ rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ਅਚਲ PragmaName *pPragma;   /* The pragma */

  ਜੇ( v==0 ) ਵਾਪਸ;
  sqlite3_Vdbe_ਲੜੀਵਾਰ_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  ਜੇ( iDb<0 ) ਵਾਪਸ;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  ਜੇ( iDb==1 && sqlite3_Open_ਆਰਜ਼ੀ_Database(pParse) ){
    ਵਾਪਸ;
  }

  z_ਖੱਬਾ = sqlite3NameFromToken(db, pId);
  ਜੇ( !z_ਖੱਬਾ ) ਵਾਪਸ;
  ਜੇ( minusFlag ){
    z_ਸੱਜਾ = sqlite3MPrintf(db, "-%T", p_ਮੁੱਲ);
  }ਨਹੀਂਤਾਂ{
    z_ਸੱਜਾ = sqlite3NameFromToken(db, p_ਮੁੱਲ);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_ਖੱਬਾ, z_ਸੱਜਾ, zDb) ){
    ਜਾਓ pragma_ਬਾਹਰਲਾ;
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
  aFcntl[1] = z_ਖੱਬਾ;
  aFcntl[2] = z_ਸੱਜਾ;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (ਖਾਲੀ*)aFcntl);
  ਜੇ( rc==SQLITE_OK ){
    sqlite3_Vdbe_ਸੈੱਟ_ਅੰਕ_ਖਾਨੇ_ਗਿਣਤੀ(v, 1);
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    return_Single_ਪਾਠ(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    ਜਾਓ pragma_ਬਾਹਰਲਾ;
  }
  ਜੇ( rc!=SQLITE_NOTFOUND ){
    ਜੇ( aFcntl[0] ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    ਜਾਓ pragma_ਬਾਹਰਲਾ;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_ਖੱਬਾ);
  ਜੇ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    ਜਾਓ pragma_ਬਾਹਰਲਾ;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  ਜੇ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    ਜੇ( sqlite3ReadSchema(pParse) ) ਜਾਓ pragma_ਬਾਹਰਲਾ;
  }

  /* Register the result column names for pragmas that return results */
  ਜੇ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_ਸੱਜਾ==0)
  ){
    ਸੈੱਟ_Pragma_ਨਤੀਜਾ_ਕਾਲਮ_ਨਾਂ(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  ਚੋਣ( pPragma->ePragTyp ){
 
#ਜੇ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  ਹਾਲਤ Prag_Typ_DEFAULT_CACHE_ਮਾਪ: {
    ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
    ਸਥਿਰ ਅਚਲ VdbeOpList get_Cache_ਮਾਪ[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_ਮਾਪ},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_ਘਟਾਓ,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_ਨਤੀਜਾ_ਪੰਕਤੀ,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb);
    ਜੇ( !z_ਸੱਜਾ ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ਮਾਪ(get_Cache_ਮਾਪ));
      aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(get_Cache_ਮਾਪ), get_Cache_ਮਾਪ, iLn);
      ਜੇ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ਰੋਕੋ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_ਮਾਪ;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ ਮਾਪ = sqlite3AbsInt32(sqlite3Atoi(z_ਸੱਜਾ));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਸੈੱਟ_Cookie, iDb, BTREE_DEFAULT_CACHE_ਮਾਪ, ਮਾਪ);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_ਮਾਪ = ਮਾਪ;
      sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(pDb->pBt, pDb->pSchema->cache_ਮਾਪ);
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#ਜੇ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  ਹਾਲਤ Prag_Typ_ਪੰਨਾ_ਮਾਪ: {
    Btree *pBt = pDb->pBt;
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt!=0 );
    ਜੇ( !z_ਸੱਜਾ ){
      ਪੂਰਨ ਮਾਪ = ALWAYS(pBt) ? sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt) : 0;
      returnSingleInt(v, ਮਾਪ);
    }ਨਹੀਂਤਾਂ{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->ਅਗਲਾ_Pagesize = sqlite3Atoi(z_ਸੱਜਾ);
      ਜੇ( SQLITE_NOMEM==sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(pBt, db->ਅਗਲਾ_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    ਰੋਕੋ;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  ਹਾਲਤ PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    ਪੂਰਨ b = -1;
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt!=0 );
    ਜੇ( z_ਸੱਜਾ ){
      ਜੇ( sqlite3_stricmp(z_ਸੱਜਾ, "fast")==0 ){
        b = 2;
      }ਨਹੀਂਤਾਂ{
        b = sqlite3GetBoolean(z_ਸੱਜਾ, 0);
      }
    }
    ਜੇ( pId2->n==0 && b>=0 ){
      ਪੂਰਨ ii;
      ਲਈ(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_ਪੰਨਾ_ਗਿਣਤੀ: {
    ਪੂਰਨ iReg;
    i64 x = 0;
    sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    ਜੇ( sqlite3Tolower(z_ਖੱਬਾ[0])=='p' ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Pagecount, iDb, iReg);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( z_ਸੱਜਾ && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ,&x)==0 ){
        ਜੇ( x<0 ) x = 0;
        ਨਹੀਂਤਾਂ ਜੇ( x>0xfffffffe ) x = 0xfffffffe;
      }ਨਹੀਂਤਾਂ{
        x = 0;
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_MaxPgcnt, iDb, iReg, (ਪੂਰਨ)x);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, iReg, 1);
    ਰੋਕੋ;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  ਹਾਲਤ Prag_Typ_LOCKING_ਬਹੁਲਕ: {
    ਅਚਲ ਅੱਖਰ *zRet = "normal";
    ਪੂਰਨ e_ਬਹੁਲਕ = get_Locking_ਬਹੁਲਕ(z_ਸੱਜਾ);

    ਜੇ( pId2->n==0 && e_ਬਹੁਲਕ==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_ਬਹੁਲਕ = db->dflt_ਤਾਲਾ_ਬਹੁਲਕ;
    }ਨਹੀਂਤਾਂ{
      Pager *pPager;
      ਜੇ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        ਪੂਰਨ ii;
        ਪ੍ਰਤਿਜ੍ਞਾ(pDb==&db->aDb[0]);
        ਲਈ(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3_Pager_Locking_ਬਹੁਲਕ(pPager, e_ਬਹੁਲਕ);
        }
        db->dflt_ਤਾਲਾ_ਬਹੁਲਕ = (u8)e_ਬਹੁਲਕ;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_ਬਹੁਲਕ = sqlite3_Pager_Locking_ਬਹੁਲਕ(pPager, e_ਬਹੁਲਕ);
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ==PAGER_LOCKINGMODE_ਸਾਧਾਰਨ
            || e_ਬਹੁਲਕ==PAGER_LOCKINGMODE_EXCLUSIVE );
    ਜੇ( e_ਬਹੁਲਕ==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    return_Single_ਪਾਠ(v, zRet);
    ਰੋਕੋ;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  ਹਾਲਤ Prag_Typ_JOURNAL_ਬਹੁਲਕ: {
    ਪੂਰਨ e_ਬਹੁਲਕ;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    ਪੂਰਨ ii;           /* Loop counter */

    ਜੇ( z_ਸੱਜਾ==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_ਬਹੁਲਕ = PAGER_JOURNALMODE_QUERY;
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *z_ਬਹੁਲਕ;
      ਪੂਰਨ n = sqlite3Strlen30(z_ਸੱਜਾ);
      ਲਈ(e_ਬਹੁਲਕ=0; (z_ਬਹੁਲਕ = sqlite3JournalModename(e_ਬਹੁਲਕ))!=0; e_ਬਹੁਲਕ++){
        ਜੇ( sqlite3StrNICmp(z_ਸੱਜਾ, z_ਬਹੁਲਕ, n)==0 ) ਰੋਕੋ;
      }
      ਜੇ( !z_ਬਹੁਲਕ ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_ਬਹੁਲਕ = PAGER_JOURNALMODE_QUERY;
      }
      ਜੇ( e_ਬਹੁਲਕ==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_ਬਹੁਲਕ = PAGER_JOURNALMODE_QUERY;
      }
    }
    ਜੇ( e_ਬਹੁਲਕ==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    ਲਈ(ii=db->nDb-1; ii>=0; ii--){
      ਜੇ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, ii);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Journal_ਬਹੁਲਕ, ii, 1, e_ਬਹੁਲਕ);
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 1);
    ਰੋਕੋ;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  ਹਾਲਤ Prag_Typ_JOURNAL_ਮਾਪ_ਹੱਦਬੰਦੀ: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_ਹੱਦਬੰਦੀ = -2;
    ਜੇ( z_ਸੱਜਾ ){
      sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &i_ਹੱਦਬੰਦੀ);
      ਜੇ( i_ਹੱਦਬੰਦੀ<-1 ) i_ਹੱਦਬੰਦੀ = -1;
    }
    i_ਹੱਦਬੰਦੀ = sqlite3_Pager_Journal_ਮਾਪ_ਹੱਦਬੰਦੀ(pPager, i_ਹੱਦਬੰਦੀ);
    returnSingleInt(v, i_ਹੱਦਬੰਦੀ);
    ਰੋਕੋ;
  }

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਹਾਲਤ PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt!=0 );
    ਜੇ( !z_ਸੱਜਾ ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ eAuto = getAutoVacuum(z_ਸੱਜਾ);
      ਪ੍ਰਤਿਜ੍ਞਾ( eAuto>=0 && eAuto<=2 );
      db->ਅਗਲਾ_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_ਸੈੱਟ_Auto_Vacuum(pBt, eAuto);
      ਜੇ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
        ਸਥਿਰ ਅਚਲ VdbeOpList ਸੈੱਟ_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_ਸਭ_ਤੋਂ_ਵੱਡਾ_ਜੜ੍ਹ_ਪੰਨਾ},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_ਸੈੱਟ_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        ਪੂਰਨ iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_ਮਾਪ(ਸੈੱਟ_Meta6));
        aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(ਸੈੱਟ_Meta6), ਸੈੱਟ_Meta6, iLn);
        ਜੇ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ਰੋਕੋ;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb);
      }
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਹਾਲਤ PragTyp_INCREMENTAL_VACUUM: {
    ਪੂਰਨ i_ਹੱਦਬੰਦੀ = 0, addr;
    ਜੇ( z_ਸੱਜਾ==0 || !sqlite3GetInt32(z_ਸੱਜਾ, &i_ਹੱਦਬੰਦੀ) || i_ਹੱਦਬੰਦੀ<=0 ){
      i_ਹੱਦਬੰਦੀ = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, i_ਹੱਦਬੰਦੀ, 1);
    addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, 1, -1);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
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
  ਹਾਲਤ Prag_Typ_CACHE_ਮਾਪ: {
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ਜੇ( !z_ਸੱਜਾ ){
      returnSingleInt(v, pDb->pSchema->cache_ਮਾਪ);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ ਮਾਪ = sqlite3Atoi(z_ਸੱਜਾ);
      pDb->pSchema->cache_ਮਾਪ = ਮਾਪ;
      sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(pDb->pBt, pDb->pSchema->cache_ਮਾਪ);
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ PragTyp_CACHE_SPILL: {
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ਜੇ( !z_ਸੱਜਾ ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(pDb->pBt,0));
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ ਮਾਪ = 1;
      ਜੇ( sqlite3GetInt32(z_ਸੱਜਾ, &ਮਾਪ) ){
        sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(pDb->pBt, ਮਾਪ);
      }
      ਜੇ( sqlite3GetBoolean(z_ਸੱਜਾ, ਮਾਪ!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }ਨਹੀਂਤਾਂ{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      ਸੈੱਟ_All_Pager_Flags(db);
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_MMAP_ਮਾਪ: {
    sqlite3_int64 sz;
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ਜੇ( z_ਸੱਜਾ ){
      ਪੂਰਨ ii;
      sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &sz);
      ਜੇ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      ਜੇ( pId2->n==0 ) db->szMmap = sz;
      ਲਈ(ii=db->nDb-1; ii>=0; ii--){
        ਜੇ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_ਮਾਪ, &sz);
#ਨਹੀਂਤਾਂ
    sz = 0;
    rc = SQLITE_OK;
#ਜੇ_ਅੰਤ
    ਜੇ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_ਆਰਜ਼ੀ_STORE: {
    ਜੇ( !z_ਸੱਜਾ ){
      returnSingleInt(v, db->ਆਰਜ਼ੀ_store);
    }ਨਹੀਂਤਾਂ{
      change_ਆਰਜ਼ੀ_Storage(pParse, z_ਸੱਜਾ);
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_ਆਰਜ਼ੀ_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ਜੇ( !z_ਸੱਜਾ ){
      return_Single_ਪਾਠ(v, sqlite3_ਆਰਜ਼ੀ_directory);
    }ਨਹੀਂਤਾਂ{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
      ਜੇ( z_ਸੱਜਾ[0] ){
        ਪੂਰਨ res;
        rc = sqlite3OsAccess(db->pVfs, z_ਸੱਜਾ, SQLITE_ACCESS_READWRITE, &res);
        ਜੇ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ਜਾਓ pragma_ਬਾਹਰਲਾ;
        }
      }
      ਜੇ( SQLITE_ਆਰਜ਼ੀ_STORE==0
       || (SQLITE_ਆਰਜ਼ੀ_STORE==1 && db->ਆਰਜ਼ੀ_store<=1)
       || (SQLITE_ਆਰਜ਼ੀ_STORE==2 && db->ਆਰਜ਼ੀ_store==1)
      ){
        invalidate_ਆਰਜ਼ੀ_Storage(pParse);
      }
      sqlite3_free(sqlite3_ਆਰਜ਼ੀ_directory);
      ਜੇ( z_ਸੱਜਾ[0] ){
        sqlite3_ਆਰਜ਼ੀ_directory = sqlite3_mprintf("%s", z_ਸੱਜਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਆਰਜ਼ੀ_directory = 0;
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ਰੋਕੋ;
  }

#ਜੇ SQLITE_OS_WIN
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
  ਹਾਲਤ Prag_Typ_ਅੰਕੜੇ_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ਜੇ( !z_ਸੱਜਾ ){
      return_Single_ਪਾਠ(v, sqlite3_ਅੰਕੜੇ_directory);
    }ਨਹੀਂਤਾਂ{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
      ਜੇ( z_ਸੱਜਾ[0] ){
        ਪੂਰਨ res;
        rc = sqlite3OsAccess(db->pVfs, z_ਸੱਜਾ, SQLITE_ACCESS_READWRITE, &res);
        ਜੇ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ਜਾਓ pragma_ਬਾਹਰਲਾ;
        }
      }
      sqlite3_free(sqlite3_ਅੰਕੜੇ_directory);
      ਜੇ( z_ਸੱਜਾ[0] ){
        sqlite3_ਅੰਕੜੇ_directory = sqlite3_mprintf("%s", z_ਸੱਜਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਅੰਕੜੇ_directory = 0;
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  ਹਾਲਤ Prag_Typ_ਤਾਲਾ_PROXY_FILE: {
    ਜੇ( !z_ਸੱਜਾ ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      ਅੱਖਰ *proxy_file_ਰਸਤਾ = ਨਿਰਾ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_ਰਸਤਾ);
      return_Single_ਪਾਠ(v, proxy_file_ਰਸਤਾ);
    }ਨਹੀਂਤਾਂ{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      ਪੂਰਨ res;
      ਜੇ( z_ਸੱਜਾ[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_ਸੈੱਟ_LOCKPROXYFILE,
                                     z_ਸੱਜਾ);
      } ਨਹੀਂਤਾਂ {
        res=sqlite3OsFileControl(pFile, SQLITE_ਸੈੱਟ_LOCKPROXYFILE,
                                     ਨਿਰਾ);
      }
      ਜੇ( res!=SQLITE_OK ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "failed to set lock proxy file");
        ਜਾਓ pragma_ਬਾਹਰਲਾ;
      }
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  ਹਾਲਤ PragTyp_SYNCHRONOUS: {
    ਜੇ( !z_ਸੱਜਾ ){
      returnSingleInt(v, pDb->safety_level-1);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( !db->autoCommit ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse,
            "Safety level may not be changed inside a transaction");
      }ਨਹੀਂਤਾਂ ਜੇ( iDb!=1 ){
        ਪੂਰਨ iLevel;
        ਜੇ( sqlite3Isdigit(z_ਸੱਜਾ[0]) ){
          iLevel = sqlite3Atoi(z_ਸੱਜਾ);
        }ਨਹੀਂਤਾਂ{
          iLevel = getSafetyLevel(z_ਸੱਜਾ,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_ਨਕਾਬ;
        ਜੇ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_ਸੈੱਟ = 1;
        ਸੈੱਟ_All_Pager_Flags(db);
      }
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_PAGER_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLAG_PRAGMAS
  ਹਾਲਤ PragTyp_FLAG: {
    ਜੇ( z_ਸੱਜਾ==0 ){
      ਸੈੱਟ_Pragma_ਨਤੀਜਾ_ਕਾਲਮ_ਨਾਂ(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }ਨਹੀਂਤਾਂ{
      u64 ਨਕਾਬ = pPragma->iArg;    /* Mask of bits to set or clear. */
      ਜੇ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        ਨਕਾਬ &= ~(SQLITE_Foreign_ਕੁੰਜੀਆਂ);
      }

      ਜੇ( sqlite3GetBoolean(z_ਸੱਜਾ, 0) ){
        ਜੇ( (ਨਕਾਬ & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= ਨਕਾਬ;
        }
      }ਨਹੀਂਤਾਂ{
        db->flags &= ~ਨਕਾਬ;
        ਜੇ( ਨਕਾਬ==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        ਜੇ( (ਨਕਾਬ & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_ਸੱਜਾ, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_ਦਾ_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Expire);
      ਸੈੱਟ_All_Pager_Flags(db);
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_FLAG_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SCHEMA_PRAGMAS
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
  ਹਾਲਤ Prag_Typ_ਸਾਰਣੀ_INFO: ਜੇ( z_ਸੱਜਾ ){
    ਸਾਰਣੀ *pTab;
    sqlite3_ਕੋਡ_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_ਸਾਰਣੀ(pParse, LOCATE_NOERR, z_ਸੱਜਾ, zDb);
    ਜੇ( pTab ){
      ਪੂਰਨ i, k;
      ਪੂਰਨ nHidden = 0;
      ਕਾਲਮ *p_ਖਾਨਾ;
      ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab);
      ਲਈ(i=0, p_ਖਾਨਾ=pTab->a_ਖਾਨਾ; i<pTab->n_ਖਾਨਾ; i++, p_ਖਾਨਾ++){
        ਪੂਰਨ isHidden = 0;
        ਅਚਲ ਵਿਅੰਜਕ *p_ਖਾਨਾ_ਵਿਅੰਜਕ;
        ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_NOINSERT ){
          ਜੇ( pPragma->iArg==0 ){
            nHidden++;
            ਜਾਰੀ;
          }
          ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }ਨਹੀਂਤਾਂ{ ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        ਜੇ( (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }ਨਹੀਂਤਾਂ ਜੇ( pPk==0 ){
          k = 1;
        }ਨਹੀਂਤਾਂ{
          ਲਈ(k=1; k<=pTab->n_ਖਾਨਾ && pPk->ai_ਕਾਲਮ[k-1]!=i; k++){}
        }
        p_ਖਾਨਾ_ਵਿਅੰਜਕ = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pTab,p_ਖਾਨਾ);
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ_ਵਿਅੰਜਕ==0 || p_ਖਾਨਾ_ਵਿਅੰਜਕ->op==TK_ਵਿਸਤਾਰ || isHidden>=2 );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ_ਵਿਅੰਜਕ==0 || !ਵਿਅੰਜਕ_Has_Property(p_ਖਾਨਾ_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               p_ਖਾਨਾ->zCnName,
               sqlite3_ਕਾਲਮ_Type(p_ਖਾਨਾ,""),
               p_ਖਾਨਾ->notNull ? 1 : 0,
               (isHidden>=2 || p_ਖਾਨਾ_ਵਿਅੰਜਕ==0) ? 0 : p_ਖਾਨਾ_ਵਿਅੰਜਕ->u.zToken,
               k,
               isHidden);
      }
    }
  }
  ਰੋਕੋ;

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
  ਹਾਲਤ Prag_Typ_ਸਾਰਣੀ_LIST: {
    ਪੂਰਨ ii;
    pParse->nMem = 6;
    sqlite3_ਕੋਡ_Verify_Named_Schema(pParse, zDb);
    ਲਈ(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      ਪੂਰਨ initNCol;
      ਜੇ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) ਜਾਰੀ;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_ਗਿਣਤੀ(pHash);
      ਜਦੋਂ( initNCol-- ){
        ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(pHash); 1; k=sqlite_Hash_ਅਗਲਾ(k) ){
          ਸਾਰਣੀ *pTab;
          ਜੇ( k==0 ){ initNCol = 0; ਰੋਕੋ; }
          pTab = sqlite_Hash_ਅੰਕੜੇ(k);
          ਜੇ( pTab->n_ਖਾਨਾ==0 ){
            ਅੱਖਰ *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            ਜੇ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (ਖਾਲੀ)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (ਖਾਲੀ)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            ਜੇ( db->mallocFailed ){
              sqlite3_Error_ਸੁਨੇਹਾ(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            ਰੋਕੋ;
          }
        }
      }

      ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(pHash); k; k=sqlite_Hash_ਅਗਲਾ(k) ){
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(k);
        ਅਚਲ ਅੱਖਰ *zType;
        ਜੇ( z_ਸੱਜਾ && sqlite3_stricmp(z_ਸੱਜਾ, pTab->zName)!=0 ) ਜਾਰੀ;
        ਜੇ( IsView(pTab) ){
          zType = "view";
        }ਨਹੀਂਤਾਂ ਜੇ( IsVirtual(pTab) ){
          zType = "virtual";
        }ਨਹੀਂਤਾਂ ਜੇ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }ਨਹੀਂਤਾਂ{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_ਸਾਰਣੀ_Name(pTab->zName),
           zType,
           pTab->n_ਖਾਨਾ,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਹਾਲਤ PragTyp_STATS: {
    ਸੂਚਕਾਂਕ *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
    ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&pDb->pSchema->tblHash); i; i=sqlite_Hash_ਅਗਲਾ(i)){
      ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(i);
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "ssiii",
           sqlite3_Preferred_ਸਾਰਣੀ_Name(pTab->zName),
           0,
           pTab->sz_Tab_ਪੰਕਤੀ,
           pTab->n_ਪੰਕਤੀ_Log_Est,
           pTab->tabFlags);
      ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 2, "siiiX",
           pIdx->zName,
           pIdx->sz_Idx_ਪੰਕਤੀ,
           pIdx->ai_ਪੰਕਤੀ_Log_Est[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 5);
      }
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

  ਹਾਲਤ Prag_Typ_ਸੂਚਕਾਂਕ_INFO: ਜੇ( z_ਸੱਜਾ ){
    ਸੂਚਕਾਂਕ *pIdx;
    ਸਾਰਣੀ *pTab;
    pIdx = sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(db, z_ਸੱਜਾ, zDb);
    ਜੇ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_ਸਾਰਣੀ(pParse, LOCATE_NOERR, z_ਸੱਜਾ, zDb);
      ਜੇ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      }
    }
    ਜੇ( pIdx ){
      ਪੂਰਨ iIdxDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pIdx->pSchema);
      ਪੂਰਨ i;
      ਪੂਰਨ mx;
      ਜੇ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->n_ਕਾਲਮ;
        pParse->nMem = 6;
      }ਨਹੀਂਤਾਂ{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_ਸਾਰਣੀ;
      sqlite3_ਕੋਡ_Verify_Schema(pParse, iIdxDb);
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nMem<=pPragma->nPragCName );
      ਲਈ(i=0; i<mx; i++){
        i16 cnum = pIdx->ai_ਕਾਲਮ[i];
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->a_ਖਾਨਾ[cnum].zCnName);
        ਜੇ( pPragma->iArg ){
          sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 4, "isiX",
            pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[i],
            pIdx->azColl[i],
            i<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, pParse->nMem);
      }
    }
  }
  ਰੋਕੋ;

  ਹਾਲਤ Prag_Typ_ਸੂਚਕਾਂਕ_LIST: ਜੇ( z_ਸੱਜਾ ){
    ਸੂਚਕਾਂਕ *pIdx;
    ਸਾਰਣੀ *pTab;
    ਪੂਰਨ i;
    pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, z_ਸੱਜਾ, zDb);
    ਜੇ( pTab ){
      ਪੂਰਨ iTabDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_ਕੋਡ_Verify_Schema(pParse, iTabDb);
      ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ, i=0; pIdx; pIdx=pIdx->p_ਅਗਲਾ, i++){
        ਅਚਲ ਅੱਖਰ *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_ਸੂਚਕਾਂਕ(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  ਰੋਕੋ;

  ਹਾਲਤ PragTyp_DATABASE_LIST: {
    ਪੂਰਨ i;
    pParse->nMem = 3;
    ਲਈ(i=0; i<db->nDb; i++){
      ਜੇ( db->aDb[i].pBt==0 ) ਜਾਰੀ;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[i].zDbSName!=0 );
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  ਰੋਕੋ;

  ਹਾਲਤ PragTyp_COLLATION_LIST: {
    ਪੂਰਨ i = 0;
    HashElem *p;
    pParse->nMem = 2;
    ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&db->aCollSeq); p; p=sqlite_Hash_ਅਗਲਾ(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_ਅੰਕੜੇ(p);
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "is", i++, pColl->zName);
    }
  }
  ਰੋਕੋ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTROSPECTION_PRAGMAS
  ਹਾਲਤ PragTyp_FUNCTION_LIST: {
    ਪੂਰਨ i;
    HashElem *j;
    FuncDef *p;
    ਪੂਰਨ showInternFunc = (db->mDbFlags & DBFLAG_ਅੰਦਰੂਨੀ_Func)!=0;
    pParse->nMem = 6;
    ਲਈ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      ਲਈ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_ਸਤਰ(v, p, 1, showInternFunc);
      }
    }
    ਲਈ(j=sqlite_Hash_ਪਹਿਲਾ(&db->aFunc); j; j=sqlite_Hash_ਅਗਲਾ(j)){
      p = (FuncDef*)sqlite_Hash_ਅੰਕੜੇ(j);
      ਪ੍ਰਤਿਜ੍ਞਾ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_ਸਤਰ(v, p, 0, showInternFunc);
    }
  }
  ਰੋਕੋ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਹਾਲਤ PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    ਲਈ(j=sqlite_Hash_ਪਹਿਲਾ(&db->aModule); j; j=sqlite_Hash_ਅਗਲਾ(j)){
      Module *pMod = (Module*)sqlite_Hash_ਅੰਕੜੇ(j);
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "s", pMod->zName);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

  ਹਾਲਤ PragTyp_PRAGMA_LIST: {
    ਪੂਰਨ i;
    ਲਈ(i=0; i<Array_ਮਾਪ(aPragmaName); i++){
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "s", aPragmaName[i].zName);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_INTROSPECTION_PRAGMAS */

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
  ਹਾਲਤ Prag_Typ_FOREIGN_ਕੁੰਜੀ_LIST: ਜੇ( z_ਸੱਜਾ ){
    FKey *pFK;
    ਸਾਰਣੀ *pTab;
    pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, z_ਸੱਜਾ, zDb);
    ਜੇ( pTab && Is_Ordinary_ਸਾਰਣੀ(pTab) ){
      pFK = pTab->u.tab.pFKey;
      ਜੇ( pFK ){
        ਪੂਰਨ iTabDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
        ਪੂਰਨ i = 0;
        pParse->nMem = 8;
        sqlite3_ਕੋਡ_Verify_Schema(pParse, iTabDb);
        ਜਦੋਂ(pFK){
          ਪੂਰਨ j;
          ਲਈ(j=0; j<pFK->n_ਖਾਨਾ; j++){
            sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_ਮੰਜ਼ਿਲ,
                   pTab->a_ਖਾਨਾ[pFK->a_ਖਾਨਾ[j].iFrom].zCnName,
                   pFK->a_ਖਾਨਾ[j].z_ਖਾਨਾ,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->p_ਅਗਲਾ_From;
        }
      }
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਹਾਲਤ Prag_Typ_FOREIGN_ਕੁੰਜੀ_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    ਸਾਰਣੀ *pTab;           /* Child table contain "REFERENCES" keyword */
    ਸਾਰਣੀ *p_ਮਾਪਾ;        /* Parent table that child points to */
    ਸੂਚਕਾਂਕ *pIdx;           /* Index in the parent table */
    ਪੂਰਨ i;                 /* Loop counter:  Foreign key number for pTab */
    ਪੂਰਨ j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    ਪੂਰਨ x;                 /* result variable */
    ਪੂਰਨ reg_ਨਤੀਜਾ;         /* 3 registers to hold a result row */
    ਪੂਰਨ reg_ਪੰਕਤੀ;            /* Registers to hold a row from pTab */
    ਪੂਰਨ addr_ਸਿਖਰ;           /* Top of a loop checking foreign keys */
    ਪੂਰਨ addrOk;            /* Jump here if the key is OK */
    ਪੂਰਨ *ai_ਖਾਨੇ_ਗਿਣਤੀ;           /* child to parent column mapping */

    reg_ਨਤੀਜਾ = pParse->nMem+1;
    pParse->nMem += 4;
    reg_ਪੰਕਤੀ = ++pParse->nMem;
    k = sqlite_Hash_ਪਹਿਲਾ(&db->aDb[iDb].pSchema->tblHash);
    ਜਦੋਂ( k ){
      ਜੇ( z_ਸੱਜਾ ){
        pTab = sqlite3_Locate_ਸਾਰਣੀ(pParse, 0, z_ਸੱਜਾ, zDb);
        k = 0;
      }ਨਹੀਂਤਾਂ{
        pTab = (ਸਾਰਣੀ*)sqlite_Hash_ਅੰਕੜੇ(k);
        k = sqlite_Hash_ਅਗਲਾ(k);
      }
      ਜੇ( pTab==0 || !Is_Ordinary_ਸਾਰਣੀ(pTab) || pTab->u.tab.pFKey==0 ) ਜਾਰੀ;
      iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
      sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_ਰਜਿਸਟਰ(pParse, pTab->n_ਖਾਨਾ+reg_ਪੰਕਤੀ);
      sqlite3_Open_ਸਾਰਣੀ(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ਲੱਦੋ_String(v, reg_ਨਤੀਜਾ, pTab->zName);
      ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
      ਲਈ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ਅਗਲਾ_From){
        p_ਮਾਪਾ = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, pFK->z_ਮੰਜ਼ਿਲ, zDb);
        ਜੇ( p_ਮਾਪਾ==0 ) ਜਾਰੀ;
        pIdx = 0;
        sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, p_ਮਾਪਾ->tnum, 0, p_ਮਾਪਾ->zName);
        x = sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, p_ਮਾਪਾ, pFK, &pIdx, 0);
        ਜੇ( x==0 ){
          ਜੇ( pIdx==0 ){
            sqlite3_Open_ਸਾਰਣੀ(pParse, i, iDb, p_ਮਾਪਾ, OP_OpenRead);
          }ਨਹੀਂਤਾਂ{
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
          }
        }ਨਹੀਂਤਾਂ{
          k = 0;
          ਰੋਕੋ;
        }
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr>0 || pFK==0 );
      ਜੇ( pFK ) ਰੋਕੋ;
      ਜੇ( pParse->nTab<i ) pParse->nTab = i;
      addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
      ਲਈ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ਅਗਲਾ_From){
        p_ਮਾਪਾ = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, pFK->z_ਮੰਜ਼ਿਲ, zDb);
        pIdx = 0;
        ai_ਖਾਨੇ_ਗਿਣਤੀ = 0;
        ਜੇ( p_ਮਾਪਾ ){
          x = sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(pParse, p_ਮਾਪਾ, pFK, &pIdx, &ai_ਖਾਨੇ_ਗਿਣਤੀ);
          ਪ੍ਰਤਿਜ੍ਞਾ( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_ਰਜਿਸਟਰ(pParse, reg_ਪੰਕਤੀ + pFK->n_ਖਾਨਾ);
        ਲਈ(j=0; j<pFK->n_ਖਾਨਾ; j++){
          ਪੂਰਨ i_ਖਾਨਾ = ai_ਖਾਨੇ_ਗਿਣਤੀ ? ai_ਖਾਨੇ_ਗਿਣਤੀ[j] : pFK->a_ਖਾਨਾ[j].iFrom;
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, 0, i_ਖਾਨਾ, reg_ਪੰਕਤੀ+j);
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, reg_ਪੰਕਤੀ+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        ਜੇ( pIdx ){
          sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, reg_ਪੰਕਤੀ, pFK->n_ਖਾਨਾ, 0,
              sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(db,pIdx), pFK->n_ਖਾਨਾ);
          sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, i, addrOk, reg_ਪੰਕਤੀ, pFK->n_ਖਾਨਾ);
          VdbeCoverage(v);
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਮਾਪਾ ){
          ਪੂਰਨ jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_SeekRowid, i, jmp, reg_ਪੰਕਤੀ); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          ਪ੍ਰਤਿਜ੍ਞਾ( pFK->n_ਖਾਨਾ==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        ਜੇ( HasRowid(pTab) ){
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, 0, reg_ਨਤੀਜਾ+1);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਨਤੀਜਾ+1);
        }
        sqlite3_Vdbe_Multi_ਲੱਦੋ(v, reg_ਨਤੀਜਾ+2, "siX", pFK->z_ਮੰਜ਼ਿਲ, i-1);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, reg_ਨਤੀਜਾ, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_ਖਾਨੇ_ਗਿਣਤੀ);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, 0, addr_ਸਿਖਰ+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_TRIGGER) */
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  ਹਾਲਤ PragTyp_CASE_SENSITIVE_LIKE: {
    ਜੇ( z_ਸੱਜਾ ){
      sqlite3_ਰਜਿਸਟਰ_Like_Functions(db, sqlite3GetBoolean(z_ਸੱਜਾ, 0));
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTEGRITY_CHECK
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
  ਹਾਲਤ PragTyp_INTEGRITY_CHECK: {
    ਪੂਰਨ i, j, addr, mxErr;
    ਸਾਰਣੀ *pObjTab = 0;     /* Check only this one table, if not NULL */

    ਪੂਰਨ is_ਤੇਜ਼ = (sqlite3Tolower(z_ਖੱਬਾ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb==0 || pId2->z );
    ਜੇ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    ਜੇ( z_ਸੱਜਾ ){
      ਜੇ( sqlite3GetInt32(p_ਮੁੱਲ->z, &mxErr) ){
        ਜੇ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }ਨਹੀਂਤਾਂ{
        pObjTab = sqlite3_Locate_ਸਾਰਣੀ(pParse, 0, z_ਸੱਜਾ,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    ਲਈ(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      ਪੂਰਨ *a_ਜੜ੍ਹ;      /* Array of root page numbers of all btrees */
      ਪੂਰਨ cnt = 0;     /* Number of entries in aRoot[] */

      ਜੇ( OMIT_TEMPDB && i==1 ) ਜਾਰੀ;
      ਜੇ( iDb>=0 && i!=iDb ) ਜਾਰੀ;

      sqlite3_ਕੋਡ_Verify_Schema(pParse, i);
      pParse->ok_Const_ਗੁਣਕ = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      ਲਈ(cnt=0, x=sqlite_Hash_ਪਹਿਲਾ(pTbls); x; x=sqlite_Hash_ਅਗਲਾ(x)){
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(x);  /* Current table */
        ਸੂਚਕਾਂਕ *pIdx;                      /* An index on pTab */
        ਜੇ( ਸਾਰਣੀ_Skip_Integrity_Check(pTab,pObjTab) ) ਜਾਰੀ;
        ਜੇ( HasRowid(pTab) ) cnt++;
        ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){ cnt++; }
      }
      ਜੇ( cnt==0 ) ਜਾਰੀ;
      ਜੇ( pObjTab ) cnt++;
      a_ਜੜ੍ਹ = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਪੂਰਨ)*(cnt+1));
      ਜੇ( a_ਜੜ੍ਹ==0 ) ਰੋਕੋ;
      cnt = 0;
      ਜੇ( pObjTab ) a_ਜੜ੍ਹ[++cnt] = 0;
      ਲਈ(x=sqlite_Hash_ਪਹਿਲਾ(pTbls); x; x=sqlite_Hash_ਅਗਲਾ(x)){
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(x);
        ਸੂਚਕਾਂਕ *pIdx;
        ਜੇ( ਸਾਰਣੀ_Skip_Integrity_Check(pTab,pObjTab) ) ਜਾਰੀ;
        ਜੇ( HasRowid(pTab) ) a_ਜੜ੍ਹ[++cnt] = pTab->tnum;
        ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
          a_ਜੜ੍ਹ[++cnt] = pIdx->tnum;
        }
      }
      a_ਜੜ੍ਹ[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_ਰਜਿਸਟਰ(pParse, 8+cnt);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_IntegrityCk, 1, cnt, 8, (ਅੱਖਰ*)a_ਜੜ੍ਹ,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_ਲੱਦੋ_String(v, 2, "wrong # of entries in index ");
      ਲਈ(x=sqlite_Hash_ਪਹਿਲਾ(pTbls); x; x=sqlite_Hash_ਅਗਲਾ(x)){
        ਪੂਰਨ iTab = 0;
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(x);
        ਸੂਚਕਾਂਕ *pIdx;
        ਜੇ( ਸਾਰਣੀ_Skip_Integrity_Check(pTab,pObjTab) ) ਜਾਰੀ;
        ਜੇ( HasRowid(pTab) ){
          iTab = cnt++;
        }ਨਹੀਂਤਾਂ{
          iTab = cnt;
          ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; ALWAYS(pIdx); pIdx=pIdx->p_ਅਗਲਾ){
            ਜੇ( Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ) ਰੋਕੋ;
            iTab++;
          }
        }
        ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
          ਜੇ( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_ਲੱਦੋ_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      ਲਈ(x=sqlite_Hash_ਪਹਿਲਾ(pTbls); x; x=sqlite_Hash_ਅਗਲਾ(x)){
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(x);
        ਸੂਚਕਾਂਕ *pIdx, *pPk;
        ਸੂਚਕਾਂਕ *p_ਪੂਰਵ = 0;      /* Previous index */
        ਪੂਰਨ loop_ਸਿਖਰ;
        ਪੂਰਨ i_ਅੰਕੜੇ_Cur, iIdxCur;
        ਪੂਰਨ r1 = -1;
        ਪੂਰਨ bStrict;            /* True for a STRICT table */
        ਪੂਰਨ r2;                 /* Previous key for WITHOUT ROWID tables */
        ਪੂਰਨ mx_ਖਾਨਾ;              /* Maximum non-virtual column number */

        ਜੇ( ਸਾਰਣੀ_Skip_Integrity_Check(pTab,pObjTab) ) ਜਾਰੀ;
        ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਜਾਰੀ;
        ਜੇ( is_ਤੇਜ਼ || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }ਨਹੀਂਤਾਂ{
          pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
          r2 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 1, r2, r2+pPk->n_ਕੁੰਜੀ_ਖਾਨਾ-1);
        }
        sqlite3_Open_ਸਾਰਣੀ_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_ਅੰਕੜੇ_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, 7);
        ਲਈ(j=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, j++){
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nMem>=8+j );
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_No_Temps_ਵਿੱਚ_Range(pParse,1,7+j) );
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, i_ਅੰਕੜੇ_Cur, 0); VdbeCoverage(v);
        loop_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( !IsVirtual(pTab) );
        ਜੇ( HasRowid(pTab) ){
          mx_ਖਾਨਾ = -1;
          ਲਈ(j=0; j<pTab->n_ਖਾਨਾ; j++){
            ਜੇ( (pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)==0 ) mx_ਖਾਨਾ++;
          }
          ਜੇ( mx_ਖਾਨਾ==pTab->iPKey ) mx_ਖਾਨਾ--;
        }ਨਹੀਂਤਾਂ{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mx_ਖਾਨਾ = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab)->n_ਕਾਲਮ-1;
        }
        ਜੇ( mx_ਖਾਨਾ>=0 ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਅੰਕੜੇ_Cur, mx_ਖਾਨਾ, 3);
          sqlite3_Vdbe_Typeof_ਕਾਲਮ(v, 3);
        }

        ਜੇ( !is_ਤੇਜ਼ ){
          ਜੇ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            ਪੂਰਨ a1;
            ਅੱਖਰ *zErr;
            a1 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IdxGT, i_ਅੰਕੜੇ_Cur, 0,r2,pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
            VdbeCoverage(v);
            sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, a1);
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, a1+1);
            ਲਈ(j=0; j<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
              sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(pParse, pPk, i_ਅੰਕੜੇ_Cur, j, r2+j);
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
        ਲਈ(j=0; j<pTab->n_ਖਾਨਾ; j++){
          ਅੱਖਰ *zErr;
          ਕਾਲਮ *p_ਖਾਨਾ = pTab->a_ਖਾਨਾ + j;  /* The column to be checked */
          ਪੂਰਨ labelError;               /* Jump here to report an error */
          ਪੂਰਨ labelOk;                  /* Jump here if all looks ok */
          ਪੂਰਨ p1, p3, p4;               /* Operands to the OP_IsType opcode */
          ਪੂਰਨ doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          ਜੇ( j==pTab->iPKey ) ਜਾਰੀ;
          ਜੇ( bStrict ){
            doTypeCheck = p_ਖਾਨਾ->eCType>COLTYPE_ANY;
          }ਨਹੀਂਤਾਂ{
            doTypeCheck = p_ਖਾਨਾ->affinity>SQLITE_AFF_BLOB;
          }
          ਜੇ( p_ਖਾਨਾ->notNull==0 && !doTypeCheck ) ਜਾਰੀ;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ){
            sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }ਨਹੀਂਤਾਂ{
            ਜੇ( p_ਖਾਨਾ->iDflt ){
              sqlite3_ਮੁੱਲ *p_Dflt_ਮੁੱਲ = 0;
              sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab,p_ਖਾਨਾ,pParse),
                                   ENC(db), p_ਖਾਨਾ->affinity, &p_Dflt_ਮੁੱਲ);
              ਜੇ( p_Dflt_ਮੁੱਲ ){
                p4 = sqlite3_ਮੁੱਲ_type(p_Dflt_ਮੁੱਲ);
                sqlite3_ਮੁੱਲ_Free(p_Dflt_ਮੁੱਲ);
              }
            }
            p1 = i_ਅੰਕੜੇ_Cur;
            ਜੇ( !HasRowid(pTab) ){
              testcase( j!=sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, j) );
              p3 = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab), j);
            }ਨਹੀਂਤਾਂ{
              p3 = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          ਜੇ( p_ਖਾਨਾ->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            ਪੂਰਨ jmp3;
            ਪੂਰਨ jmp2 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            ਜੇ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }ਨਹੀਂਤਾਂ{
              sqlite3VdbeChangeP5(v, 0x0d); /* INT, TEXT, or BLOB */
              /* OP_IsType does not detect NaN values in the database file
              ** which should be treated as a NULL.  So if the header type
              ** is REAL, we have to load the actual data using OP_Column
              ** to reliably determine if the value is a NULL. */
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, p1, p3, 3);
              sqlite3_ਕਾਲਮ_Default(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                p_ਖਾਨਾ->zCnName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            ਜੇ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp2);
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp3);
            }ਨਹੀਂਤਾਂ{
              /* VDBE byte code will fall thru */
            }
          }
          ਜੇ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ a_Std_Type_ਨਕਾਬ[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /*ਪਾਠ*/
            };
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->eCType>=1 && p_ਖਾਨਾ->eCType<=ਆਕਾਰ(a_Std_Type_ਨਕਾਬ) );
            sqlite3VdbeChangeP5(v, a_Std_Type_ਨਕਾਬ[p_ਖਾਨਾ->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[p_ਖਾਨਾ->eCType-1],
                                  pTab->zName, pTab->a_ਖਾਨਾ[j].zCnName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ਨਹੀਂਤਾਂ ਜੇ( !bStrict && p_ਖਾਨਾ->affinity==SQLITE_AFF_ਪਾਠ ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->a_ਖਾਨਾ[j].zCnName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ਨਹੀਂਤਾਂ ਜੇ( !bStrict && p_ਖਾਨਾ->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            ਜੇ( p1>=0 ){
              sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, i_ਅੰਕੜੇ_Cur, j, 3);
            }
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->a_ਖਾਨਾ[j].zCnName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        ਜੇ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ਵਿਅੰਜਕ_List *pCheck = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pTab->pCheck, 0);
          ਜੇ( db->mallocFailed==0 ){
            ਪੂਰਨ addrCkFault = sqlite3VdbeMakeLabel(pParse);
            ਪੂਰਨ addrCkOk = sqlite3VdbeMakeLabel(pParse);
            ਅੱਖਰ *zErr;
            ਪੂਰਨ k;
            pParse->iSelfTab = i_ਅੰਕੜੇ_Cur + 1;
            ਲਈ(k=pCheck->n_ਵਿਅੰਜਕ-1; k>0; k--){
              sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pCheck->a[k].p_ਵਿਅੰਜਕ, addrCkFault, 0);
            }
            sqlite3_ਵਿਅੰਜਕ_If_True(pParse, pCheck->a[0].p_ਵਿਅੰਜਕ, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pCheck);
        }
        ਜੇ( !is_ਤੇਜ਼ ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          ਲਈ(j=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, j++){
            ਪੂਰਨ jmp2, jmp3, jmp4, jmp5, label6;
            ਪੂਰਨ kk;
            ਪੂਰਨ ckUniq = sqlite3VdbeMakeLabel(pParse);
            ਜੇ( pPk==pIdx ) ਜਾਰੀ;
            r1 = sqlite3_Generate_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(pParse, pIdx, i_ਅੰਕੜੇ_Cur, 0, 0, &jmp3,
                                         p_ਪੂਰਵ, r1);
            p_ਪੂਰਵ = pIdx;
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iIdxCur+j, ckUniq, r1,
                                        pIdx->n_ਕਾਲਮ); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_IFind_ਕੁੰਜੀ, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ਅਚਲ ਅੱਖਰ*)pIdx, P4_ਸੂਚਕਾਂਕ);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp2);
            sqlite3_Vdbe_ਲੱਦੋ_String(v, 3, "row ");
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_ਲੱਦੋ_String(v, 4, " missing from index ");
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_ਲੱਦੋ_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            ਜੇ( HasRowid(pTab) ){
              ਪੂਰਨ jmp7;
              sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, 3, 0, r1+pIdx->n_ਕਾਲਮ-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_ਲੱਦੋ_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ਲੱਦੋ_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            ਲਈ(kk=0; kk<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; kk++){
              ਜੇ( pIdx->azColl[kk]==sqlite3_Str_ਦੁਆਧਾਰੀ ) ਜਾਰੀ;
              ਜੇ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            ਜੇ( label6 ){
              ਪੂਰਨ jmp6 = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_ਲੱਦੋ_String(v, 3, "row ");
              sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ਲੱਦੋ_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            ਜੇ( Is_Unique_ਸੂਚਕਾਂਕ(pIdx) ){
              ਪੂਰਨ uniqOk = sqlite3VdbeMakeLabel(pParse);
              ਪੂਰਨ jmp6;
              ਲਈ(kk=0; kk<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; kk++){
                ਪੂਰਨ i_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[kk];
                ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ!=XN_ROWID && i_ਖਾਨਾ<pTab->n_ਖਾਨਾ );
                ਜੇ( i_ਖਾਨਾ>=0 && pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].notNull ) ਜਾਰੀ;
                sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਅਗਲਾ, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp6);
              sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ); VdbeCoverage(v);
              sqlite3_Vdbe_ਲੱਦੋ_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, i_ਅੰਕੜੇ_Cur, loop_ਸਿਖਰ); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, loop_ਸਿਖਰ-1);
        ਜੇ( pPk ){
          ਪ੍ਰਤਿਜ੍ਞਾ( !is_ਤੇਜ਼ );
          sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r2, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
        }
      }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      ਲਈ(x=sqlite_Hash_ਪਹਿਲਾ(pTbls); x; x=sqlite_Hash_ਅਗਲਾ(x)){
        ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(x);
        sqlite3_vtab *pVTab;
        ਪੂਰਨ a1;
        ਜੇ( ਸਾਰਣੀ_Skip_Integrity_Check(pTab,pObjTab) ) ਜਾਰੀ;
        ਜੇ( Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਜਾਰੀ;
        ਜੇ( !IsVirtual(pTab) ) ਜਾਰੀ;
        ਜੇ( pTab->n_ਖਾਨਾ<=0 ){
          ਅਚਲ ਅੱਖਰ *zMod = pTab->u.vtab.azArg[0];
          ਜੇ( sqlite3_Hash_ਲੱਭੋ(&db->aModule, zMod)==0 ) ਜਾਰੀ;
        }
        sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab);
        ਜੇ( pTab->u.vtab.p==0 ) ਜਾਰੀ;
        pVTab = pTab->u.vtab.p->pVtab;
        ਜੇ( NEVER(pVTab==0) ) ਜਾਰੀ;
        ਜੇ( NEVER(pVTab->pModule==0) ) ਜਾਰੀ;
        ਜੇ( pVTab->pModule->i_ਸੰਸਕਰਣ<4 ) ਜਾਰੀ;
        ਜੇ( pVTab->pModule->xIntegrity==0 ) ਜਾਰੀ;
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_VCheck, i, 3, is_ਤੇਜ਼);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, a1);
        ਜਾਰੀ;
      }
#ਜੇ_ਅੰਤ
    }
    {
      ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
      ਸਥਿਰ ਅਚਲ VdbeOpList ਸਿਰਾ_ਕੋਡ[] = {
        { OP_ਜੋੜੋ_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_ਨਤੀਜਾ_ਪੰਕਤੀ,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(ਸਿਰਾ_ਕੋਡ), ਸਿਰਾ_ਕੋਡ, iLn);
      ਜੇ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (ਅੱਖਰ*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
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
  ਹਾਲਤ PragTyp_ENCODING: {
    ਸਥਿਰ ਅਚਲ ਬਣਤਰ EncName {
      ਅੱਖਰ *zName;
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
    ਅਚਲ ਬਣਤਰ EncName *pEnc;
    ਜੇ( !z_ਸੱਜਾ ){    /* "PRAGMA encoding" */
      ਜੇ( sqlite3ReadSchema(pParse) ) ਜਾਓ pragma_ਬਾਹਰਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      ਪ੍ਰਤਿਜ੍ਞਾ( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      ਪ੍ਰਤਿਜ੍ਞਾ( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      return_Single_ਪਾਠ(v, encnames[ENC(pParse->db)].zName);
    }ਨਹੀਂਤਾਂ{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      ਜੇ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        ਲਈ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          ਜੇ( 0==sqlite3StrICmp(z_ਸੱਜਾ, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_ਸੈੱਟ_ਪਾਠ_Encoding(db, enc);
            ਰੋਕੋ;
          }
        }
        ਜੇ( !pEnc->zName ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unsupported encoding: %s", z_ਸੱਜਾ);
        }
      }
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SCHEMA_ਸੰਸਕਰਣ_PRAGMAS
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
  ਹਾਲਤ Prag_Typ_ਸਿਰਲੇਖ_ਮੁੱਲ: {
    ਪੂਰਨ iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb);
    ਜੇ( z_ਸੱਜਾ && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ਸਥਿਰ ਅਚਲ VdbeOpList ਸੈੱਟ_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_ਸੈੱਟ_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ਮਾਪ(ਸੈੱਟ_Cookie));
      aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(ਸੈੱਟ_Cookie), ਸੈੱਟ_Cookie, 0);
      ਜੇ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ਰੋਕੋ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_ਸੱਜਾ);
      aOp[1].p5 = 1;
      ਜੇ( iCookie==BTREE_SCHEMA_ਸੰਸਕਰਣ && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }ਨਹੀਂਤਾਂ{
      /* Read the specified cookie value */
      ਸਥਿਰ ਅਚਲ VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ਨਤੀਜਾ_ਪੰਕਤੀ,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ਮਾਪ(readCookie));
      aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(readCookie),readCookie,0);
      ਜੇ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ਰੋਕੋ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  ਹਾਲਤ PragTyp_COMPILE_OPTIONS: {
    ਪੂਰਨ i = 0;
    ਅਚਲ ਅੱਖਰ *zOpt;
    pParse->nMem = 1;
    ਜਦੋਂ( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3_Vdbe_ਲੱਦੋ_String(v, 1, zOpt);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  ਹਾਲਤ PragTyp_WAL_CHECKPOINT: {
    ਪੂਰਨ iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    ਪੂਰਨ e_ਬਹੁਲਕ = SQLITE_CHECKPOINT_PASSIVE;
    ਜੇ( z_ਸੱਜਾ ){
      ਜੇ( sqlite3StrICmp(z_ਸੱਜਾ, "full")==0 ){
        e_ਬਹੁਲਕ = SQLITE_CHECKPOINT_ਭਰਿਆ;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(z_ਸੱਜਾ, "restart")==0 ){
        e_ਬਹੁਲਕ = SQLITE_CHECKPOINT_RESTART;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(z_ਸੱਜਾ, "truncate")==0 ){
        e_ਬਹੁਲਕ = SQLITE_CHECKPOINT_TRUNCATE;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(z_ਸੱਜਾ, "noop")==0 ){
        e_ਬਹੁਲਕ = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Checkpoint, iBt, e_ਬਹੁਲਕ, 1);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, 1, 3);
  }
  ਰੋਕੋ;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  ਹਾਲਤ PragTyp_WAL_AUTOCHECKPOINT: {
    ਜੇ( z_ਸੱਜਾ ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_ਸੱਜਾ));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(db->pWalArg) : 0);
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  ਹਾਲਤ Prag_Typ_SHRINK_ਸਮ੍ਰਿਤੀ: {
    sqlite3_db_release_ਸਮ੍ਰਿਤੀ(db);
    ਰੋਕੋ;
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
  ਹਾਲਤ PragTyp_OPTIMIZE: {
    ਪੂਰਨ i_Db_ਆਖਰੀ;           /* Loop termination point for the schema loop */
    ਪੂਰਨ iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    ਸਾਰਣੀ *pTab;           /* A table in the schema */
    ਸੂਚਕਾਂਕ *pIdx;           /* An index of the table */
    LogEst sz_ਹੱਦ;    /* Size threshold above which reanalysis needed */
    ਅੱਖਰ *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_ਨਕਾਬ;            /* Mask of operations to perform */
    ਪੂਰਨ n_ਹੱਦਬੰਦੀ;            /* Analysis limit to use */
    ਪੂਰਨ nCheck = 0;        /* Number of tables to be optimized */
    ਪੂਰਨ nBtree = 0;        /* Number of btrees to scan */
    ਪੂਰਨ n_ਸੂਚਕਾਂਕ;            /* Number of indexes on the current table */

    ਜੇ( z_ਸੱਜਾ ){
      op_ਨਕਾਬ = (u32)sqlite3Atoi(z_ਸੱਜਾ);
      ਜੇ( (op_ਨਕਾਬ & 0x02)==0 ) ਰੋਕੋ;
    }ਨਹੀਂਤਾਂ{
      op_ਨਕਾਬ = 0xfffe;
    }
    ਜੇ( (op_ਨਕਾਬ & 0x10)==0 ){
      n_ਹੱਦਬੰਦੀ = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( db->n_Analysis_ਹੱਦਬੰਦੀ>0
           && db->n_Analysis_ਹੱਦਬੰਦੀ<SQLITE_DEFAULT_OPTIMIZE_ਹੱਦਬੰਦੀ ){
      n_ਹੱਦਬੰਦੀ = 0;
    }ਨਹੀਂਤਾਂ{
      n_ਹੱਦਬੰਦੀ = SQLITE_DEFAULT_OPTIMIZE_ਹੱਦਬੰਦੀ;
    }
    iTabCur = pParse->nTab++;
    ਲਈ(i_Db_ਆਖਰੀ = zDb?iDb:db->nDb-1; iDb<=i_Db_ਆਖਰੀ; iDb++){
      ਜੇ( iDb==1 ) ਜਾਰੀ;
      sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); k; k=sqlite_Hash_ਅਗਲਾ(k)){
        pTab = (ਸਾਰਣੀ*)sqlite_Hash_ਅੰਕੜੇ(k);

        /* This only works for ordinary tables */
        ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਜਾਰੀ;

        /* Do not scan system tables */
        ਜੇ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) ਜਾਰੀ;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_ਹੱਦ = pTab->n_ਪੰਕਤੀ_Log_Est;
        n_ਸੂਚਕਾਂਕ = 0;
        ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
          n_ਸੂਚਕਾਂਕ++;
          ਜੇ( !pIdx->hasStat1 ){
            sz_ਹੱਦ = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        ਜੇ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }ਨਹੀਂਤਾਂ ਜੇ( op_ਨਕਾਬ & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }ਨਹੀਂਤਾਂ ਜੇ( pTab->p_ਸੂਚਕਾਂਕ!=0 && sz_ਹੱਦ<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }ਨਹੀਂਤਾਂ{
          /* Otherwise, we can skip this table */
          ਜਾਰੀ;
        }

        nCheck++;
        ਜੇ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_ਸੂਚਕਾਂਕ+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_ਸਾਰਣੀ(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        ਜੇ( sz_ਹੱਦ>=0 ){
          ਅਚਲ LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_If_ਮਾਪ_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ਨਕਾਬ&1),
                         sz_ਹੱਦ>=iRange ? sz_ਹੱਦ-iRange : -1,
                         sz_ਹੱਦ+iRange);
          VdbeCoverage(v);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ਨਕਾਬ&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        ਜੇ( op_ਨਕਾਬ & 0x01 ){
          ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
          sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, r1, 1);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_SqlExec, n_ਹੱਦਬੰਦੀ ? 0x02 : 00, n_ਹੱਦਬੰਦੀ, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    ਜੇ( !db->mallocFailed && n_ਹੱਦਬੰਦੀ>0 && nBtree>100 ){
      ਪੂਰਨ iAddr, i_ਸਿਰਾ;
      VdbeOp *aOp;
      n_ਹੱਦਬੰਦੀ = 100*n_ਹੱਦਬੰਦੀ/nBtree;
      ਜੇ( n_ਹੱਦਬੰਦੀ<100 ) n_ਹੱਦਬੰਦੀ = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_ਸਿਰਾ = sqlite3VdbeCurrentAddr(v);
      ਲਈ(iAddr=0; iAddr<i_ਸਿਰਾ; iAddr++){
        ਜੇ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_ਹੱਦਬੰਦੀ;
      }
    }
    ਰੋਕੋ;
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
  /*case PragTyp_BUSY_TIMEOUT*/ ਮੂਲ: {
    ਪੂਰਨ n = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    ਜੇ( z_ਸੱਜਾ && sqlite3GetInt32(z_ਸੱਜਾ,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_SOFT_ਢੇਰ_ਹੱਦਬੰਦੀ: {
    sqlite3_int64 N;
    ਜੇ( z_ਸੱਜਾ && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &N)==SQLITE_OK ){
      sqlite3_soft_ਢੇਰ_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_ਢੇਰ_limit64(-1));
    ਰੋਕੋ;
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
  ਹਾਲਤ Prag_Typ_HARD_ਢੇਰ_ਹੱਦਬੰਦੀ: {
    sqlite3_int64 N;
    ਜੇ( z_ਸੱਜਾ && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &N)==SQLITE_OK ){
      sqlite3_int64 i_ਪੂਰਵ = sqlite3_hard_ਢੇਰ_limit64(-1);
      ਜੇ( N>0 && (i_ਪੂਰਵ==0 || i_ਪੂਰਵ>N) ) sqlite3_hard_ਢੇਰ_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_ਢੇਰ_limit64(-1));
    ਰੋਕੋ;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  ਹਾਲਤ Prag_Typ_ਧਾਗੇ: {
    sqlite3_int64 N;
    ਜੇ( z_ਸੱਜਾ
     && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_ਹੱਦਬੰਦੀ(db, SQLITE_ਹੱਦਬੰਦੀ_ਕਾਮਾ_ਧਾਗੇ, (ਪੂਰਨ)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_ਹੱਦਬੰਦੀ(db, SQLITE_ਹੱਦਬੰਦੀ_ਕਾਮਾ_ਧਾਗੇ, -1));
    ਰੋਕੋ;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  ਹਾਲਤ Prag_Typ_ANALYSIS_ਹੱਦਬੰਦੀ: {
    sqlite3_int64 N;
    ਜੇ( z_ਸੱਜਾ
     && sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(z_ਸੱਜਾ, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_ਹੱਦਬੰਦੀ = (ਪੂਰਨ)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_ਹੱਦਬੰਦੀ); /* IMP: R-57594-65522 */
    ਰੋਕੋ;
  }

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  ਹਾਲਤ Prag_Typ_ਤਾਲਾ_ਸਥਿਤੀ: {
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ az_ਤਾਲਾ_Name[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    ਪੂਰਨ i;
    pParse->nMem = 2;
    ਲਈ(i=0; i<db->nDb; i++){
      Btree *pBt;
      ਅਚਲ ਅੱਖਰ *zState = "unknown";
      ਪੂਰਨ j;
      ਜੇ( db->aDb[i].zDbSName==0 ) ਜਾਰੀ;
      pBt = db->aDb[i].pBt;
      ਜੇ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = az_ਤਾਲਾ_Name[j];
      }
      sqlite3_Vdbe_Multi_ਲੱਦੋ(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    ਰੋਕੋ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_ENABLE_CEROD)
  ਹਾਲਤ PragTyp_ACTIVATE_EXTENSIONS: ਜੇ( z_ਸੱਜਾ ){
    ਜੇ( sqlite3StrNICmp(z_ਸੱਜਾ, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_ਸੱਜਾ[6]);
    }
  }
  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  ਜੇ( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_ਸੱਜਾ ){
    sqlite3_Vdbe_Verify_No_ਨਤੀਜਾ_ਪੰਕਤੀ(v);
  }

pragma_ਬਾਹਰਲਾ:
  sqlite3DbFree(db, z_ਖੱਬਾ);
  sqlite3DbFree(db, z_ਸੱਜਾ);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PragmaVtab PragmaVtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PragmaVtabCursor PragmaVtabCursor;
ਬਣਤਰ PragmaVtab {
  sqlite3_vtab ਅਧਾਰ;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ਅਚਲ PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
ਬਣਤਰ PragmaVtabCursor {
  sqlite3_vtab_cursor ਅਧਾਰ; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  ਅੱਖਰ *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
ਸਥਿਰ ਪੂਰਨ pragmaVtabConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  ਅਚਲ PragmaName *pPragma = (ਅਚਲ PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  ਪੂਰਨ rc;
  ਪੂਰਨ i, j;
  ਅੱਖਰ cSep = '(';
  StrAccum acc;
  ਅੱਖਰ zBuf[200];

  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  ਅਣਵਰਤਿਆ_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, ਆਕਾਰ(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  ਲਈ(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  ਜੇ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  ਜੇ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  ਜੇ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3_Str_Accum_ਅੰਤ(&acc);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਲੜੀ_ਲੰਬਾਈ(zBuf) < ਆਕਾਰ(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  ਜੇ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(ਆਕਾਰ(PragmaVtab));
    ਜੇ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }ਨਹੀਂਤਾਂ{
      memset(pTab, 0, ਆਕਾਰ(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }ਨਹੀਂਤਾਂ{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ਵਾਪਸ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
ਸਥਿਰ ਪੂਰਨ pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  ਵਾਪਸ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
ਸਥਿਰ ਪੂਰਨ pragma_Vtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ਅਚਲ ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pConstraint;
  ਪੂਰਨ i, j;
  ਪੂਰਨ ਵੇਖਿਆ[2];

  pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)1;
  ਜੇ( pTab->nHidden==0 ){ ਵਾਪਸ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  ਵੇਖਿਆ[0] = 0;
  ਵੇਖਿਆ[1] = 0;
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ਜੇ( pConstraint->i_ਕਾਲਮ < pTab->iHidden ) ਜਾਰੀ;
    ਜੇ( pConstraint->op!=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ) ਜਾਰੀ;
    ਜੇ( pConstraint->usable==0 ) ਵਾਪਸ SQLITE_CONSTRAINT;
    j = pConstraint->i_ਕਾਲਮ - pTab->iHidden;
    ਪ੍ਰਤਿਜ੍ਞਾ( j < 2 );
    ਵੇਖਿਆ[j] = i+1;
  }
  ਜੇ( ਵੇਖਿਆ[0]==0 ){
    pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)2147483647;
    pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 2147483647;
    ਵਾਪਸ SQLITE_OK;
  }
  j = ਵੇਖਿਆ[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_ਸੂਚਕਾਂਕ = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)20;
  pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 20;
  ਜੇ( ਵੇਖਿਆ[1] ){
    j = ਵੇਖਿਆ[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_ਸੂਚਕਾਂਕ = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  ਵਾਪਸ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
ਸਥਿਰ ਪੂਰਨ pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(ਆਕਾਰ(*pCsr));
  ਜੇ( pCsr==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCsr, 0, ਆਕਾਰ(PragmaVtabCursor));
  pCsr->ਅਧਾਰ.pVtab = pVtab;
  *ppCursor = &pCsr->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ਸਥਿਰ ਖਾਲੀ pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  ਪੂਰਨ i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  ਲਈ(i=0; i<Array_ਮਾਪ(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
ਸਥਿਰ ਪੂਰਨ pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  ਵਾਪਸ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
ਸਥਿਰ ਪੂਰਨ pragma_Vtab_ਅਗਲਾ(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ਪੂਰਨ rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->pPragma );
  ਜੇ( SQLITE_ਪੰਕਤੀ!=sqlite3_ਕਦਮ(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  ਵਾਪਸ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
ਸਥਿਰ ਪੂਰਨ pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ਪੂਰਨ rc;
  ਪੂਰਨ i, j;
  StrAccum acc;
  ਅੱਖਰ *zSql;

  ਅਣਵਰਤਿਆ_PARAMETER(idx_ਅੰਕ);
  ਅਣਵਰਤਿਆ_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  ਲਈ(i=0; i<argc; i++, j++){
    ਅਚਲ ਅੱਖਰ *z_ਪਾਠ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਪ੍ਰਤਿਜ੍ਞਾ( j<Array_ਮਾਪ(pCsr->azArg) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->azArg[j]==0 );
    ਜੇ( z_ਪਾਠ ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", z_ਪਾਠ);
      ਜੇ( pCsr->azArg[j]==0 ){
        ਵਾਪਸ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SQL_ਲੰਮਾਈ]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  ਜੇ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  ਜੇ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3_Str_Accum_ਅੰਤ(&acc);
  ਜੇ( zSql==0 ) ਵਾਪਸ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  ਜੇ( rc!=SQLITE_OK ){
    pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    ਵਾਪਸ rc;
  }
  ਵਾਪਸ pragma_Vtab_ਅਗਲਾ(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
ਸਥਿਰ ਪੂਰਨ pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ਵਾਪਸ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
ਸਥਿਰ ਪੂਰਨ pragma_Vtab_ਕਾਲਮ(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  ਪੂਰਨ i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ਜੇ( i<pTab->iHidden ){
    sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCsr->pPragma, i));
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
ਸਥਿਰ ਪੂਰਨ pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  ਵਾਪਸ SQLITE_OK;
}

/* The pragma virtual table object */
ਸਥਿਰ ਅਚਲ sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_ਅਗਲਾ,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragma_Vtab_ਕਾਲਮ,            /* xColumn - read data */
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
Module *sqlite3_Pragma_Vtab_ਰਜਿਸਟਰ(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zName){
  ਅਚਲ PragmaName *pName;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  ਜੇ( pName==0 ) ਵਾਪਸ 0;
  ਜੇ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Hash_ਲੱਭੋ(&db->aModule, zName)==0 );
  ਵਾਪਸ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (ਖਾਲੀ*)pName, 0);
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_PRAGMA */
