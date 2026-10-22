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

#ಆದರೆ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ಆದರೆ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ಇಲ್ಲದಿದ್ದರೆ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_OPTIMIZE_ಎಲ್ಲೆ
# define SQLITE_DEFAULT_OPTIMIZE_ಎಲ್ಲೆ 2000
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ u8 getSafetyLevel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ omit_ತುಂಬಿದ, u8 dflt){
                             /* 123456789 123456789 123 */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ z_ಪಠ್ಯ[] = "onoffalseyestruextrafull";
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 i_ಸ್ಥಾನಾಂತರ[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 i_ಉದ್ದಳತೆ[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 i_ಮೌಲ್ಯ[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  ಪೂರ್ಣಾಂಕ i, n;
  ಆದರೆ( sqlite3Isdigit(*z) ){
    ಹಿಂತಿರುಗಿ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(i_ಉದ್ದಳತೆ); i++){
    ಆದರೆ( i_ಉದ್ದಳತೆ[i]==n && sqlite3StrNICmp(&z_ಪಠ್ಯ[i_ಸ್ಥಾನಾಂತರ[i]],z,n)==0
     && (!omit_ತುಂಬಿದ || i_ಮೌಲ್ಯ[i]<=1)
    ){
      ಹಿಂತಿರುಗಿ i_ಮೌಲ್ಯ[i];
    }
  }
  ಹಿಂತಿರುಗಿ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, u8 dflt){
  ಹಿಂತಿರುಗಿ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#ಆದರೆ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_Locking_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಆದರೆ( z ){
    ಆದರೆ( 0==sqlite3StrICmp(z, "exclusive") ) ಹಿಂತಿರುಗಿ PAGER_LOCKINGMODE_EXCLUSIVE;
    ಆದರೆ( 0==sqlite3StrICmp(z, "normal") ) ಹಿಂತಿರುಗಿ PAGER_LOCKINGMODE_ಸಾಮಾನ್ಯ;
  }
  ಹಿಂತಿರುಗಿ PAGER_LOCKINGMODE_QUERY;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getAutoVacuum(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( 0==sqlite3StrICmp(z, "none") ) ಹಿಂತಿರುಗಿ BTREE_AUTOVACUUM_NONE;
  ಆದರೆ( 0==sqlite3StrICmp(z, "full") ) ಹಿಂತಿರುಗಿ BTREE_AUTOVACUUM_ತುಂಬಿದ;
  ಆದರೆ( 0==sqlite3StrICmp(z, "incremental") ) ಹಿಂತಿರುಗಿ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  ಹಿಂತಿರುಗಿ (u8)((i>=0&&i<=2)?i:0);
}
#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ತಾತ್ಕಾಲಿಕ_Store(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಆದರೆ( z[0]>='0' && z[0]<='2' ){
    ಹಿಂತಿರುಗಿ z[0] - '0';
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(z, "file")==0 ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(z, "memory")==0 ){
    ಹಿಂತಿರುಗಿ 2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_PAGER_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ invalidate_ತಾತ್ಕಾಲಿಕ_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಆದರೆ( db->aDb[1].pBt!=0 ){
    ಆದರೆ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_ದ_Connection(db);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_PAGER_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ change_ತಾತ್ಕಾಲಿಕ_Storage(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zStorageType){
  ಪೂರ್ಣಾಂಕ ts = get_ತಾತ್ಕಾಲಿಕ_Store(zStorageType);
  sqlite3 *db = pParse->db;
  ಆದರೆ( db->ತಾತ್ಕಾಲಿಕ_store==ts ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಆದರೆ( invalidate_ತಾತ್ಕಾಲಿಕ_Storage( pParse ) != SQLITE_OK ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  db->ತಾತ್ಕಾಲಿಕ_store = (u8)ts;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Pragma_ಫಲಿತಾಂಶ_ಸ್ತಂಭ_ಹೆಸರುಗಳು(
  Vdbe *v,                     /* The query under construction */
  ಸ್ಥಿರಾಂಕ PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_ಗಣ_ಅಂಕೆ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(v, n==0 ? 1 : n);
  ಆದರೆ( n==0 ){
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i, j;
    ಗಾಗಿ(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ returnSingleInt(Vdbe *v, i64 ಮೌಲ್ಯ){
  sqlite3_Vdbe_ಕೂಡಿಸು_Int64(v, 1, ಮೌಲ್ಯ);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ return_Single_ಪಠ್ಯ(
  Vdbe *v,                /* Prepared statement under construction */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮೌಲ್ಯ      /* Value to be returned */
){
  ಆದರೆ( z_ಮೌಲ್ಯ ){
    sqlite3_Vdbe_ತುಂಬು_String(v, 1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)z_ಮೌಲ್ಯ);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_All_Pager_Flags(sqlite3 *db){
  ಆದರೆ( db->autoCommit ){
    Db *pDb = db->aDb;
    ಪೂರ್ಣಾಂಕ n = db->nDb;
    ಪ್ರತಿಜ್ಞಾ( SQLITE_ತುಂಬಿದ_FSync==PAGER_FULLFSYNC );
    ಪ್ರತಿಜ್ಞಾ( SQLITE_Ckpt_ತುಂಬಿದ_FSync==PAGER_CKPT_FULLFSYNC );
    ಪ್ರತಿಜ್ಞಾ( SQLITE_CacheSpill==PAGER_CACHESPILL );
    ಪ್ರತಿಜ್ಞಾ( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_ಮುಖವಾಡ );
    ಪ್ರತಿಜ್ಞಾ( (pDb->safety_level & PAGER_SYNCHRONOUS_ಮುಖವಾಡ)==pDb->safety_level );
    ಆಗ( (n--) > 0 ){
      ಆದರೆ( pDb->pBt ){
        sqlite3_Btree_ಗಣ_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_ಮುಖವಾಡ) );
      }
      pDb++;
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಗಣ_All_Pager_Flags(X)  /* no-op */
#ಆದರೆ_ಅಂತ್ಯ


/*
** Return a human-readable name for a constraint resolution action.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *actionName(u8 action){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  ಆಯ್ಕೆ( action ){
    ಸ್ಥಿತಿ OE_ಗಣ_Null:  zName = "SET NULL";        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_ಗಣ_Dflt:  zName = "SET DEFAULT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Cascade:  zName = "CASCADE";         ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Restrict: zName = "RESTRICT";        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:          zName = "NO ACTION"; 
                      ಪ್ರತಿಜ್ಞಾ( action==OE_None ); ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ zName;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3JournalModename(ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ){
  ಸ್ಥಿರ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ az_ಬಹುಲಕ_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
     , "wal"
#ಆದರೆ_ಅಂತ್ಯ
  };
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_DELETE==0 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_PERSIST==1 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_OFF==2 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_TRUNCATE==3 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_ಸ್ಮರಣೆ==4 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_WAL==5 );
  ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ>=0 && e_ಬಹುಲಕ<=Array_ಗಾತ್ರ(az_ಬಹುಲಕ_Name) );

  ಆದರೆ( e_ಬಹುಲಕ==Array_ಗಾತ್ರ(az_ಬಹುಲಕ_Name) ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ az_ಬಹುಲಕ_Name[e_ಬಹುಲಕ];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ PragmaName *pragmaLocate(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಪೂರ್ಣಾಂಕ upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_ಗಾತ್ರ(aPragmaName)-1;
  ಆಗ( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    ಆದರೆ( rc==0 ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( rc<0 ){
      upr = mid - 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      lwr = mid + 1;
    }
  }
  ಹಿಂತಿರುಗಿ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pragma_Funclist_ಸಾಲು(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  ಪೂರ್ಣಾಂಕ isBuiltin,         /* True if this is a built-in function */
  ಪೂರ್ಣಾಂಕ showInternFuncs    /* True if showing internal functions */
){
  u32 ಮುಖವಾಡ =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_ಆಂತರಿಕ
  ;
  ಆದರೆ( showInternFuncs ) ಮುಖವಾಡ = 0xffffffff;
  ಗಾಗಿ(; p; p=p->p_ಮುಂದಿನ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ENCMASK==0x3 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    ಆದರೆ( p->xSFunc==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( (p->funcFlags & SQLITE_FUNC_ಆಂತರಿಕ)!=0
     && showInternFuncs==0
    ){
      ಮುಂದುವರಿ;
    }   
    ಆದರೆ( p->x_ಮೌಲ್ಯ!=0 ){
      zType = "w";
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->xFinalize!=0 ){
      zType = "a";
    }ಇಲ್ಲದಿದ್ದರೆ{
      zType = "s";
    }
    sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & ಮುಖವಾಡ) ^ SQLITE_INNOCUOUS
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(Vdbe *v){
  ಪೂರ್ಣಾಂಕ addr;
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 3, 1);
  addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Halt);
  ಹಿಂತಿರುಗಿ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_Skip_Integrity_Check(ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pTab, ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pObjTab){
  ಆದರೆ( pObjTab ){
    ಹಿಂತಿರುಗಿ pTab!=pObjTab;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ (pTab->tabFlags & TF_Imposter)!=0;
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
  ಪೂರ್ಣಾಂಕ minusFlag       /* True if a '-' sign preceded <value> */
){
  ಅಕ್ಷರ *z_ಎಡ = 0;       /* Nul-terminated UTF-8 string <id> */
  ಅಕ್ಷರ *z_ಬಲ = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  ಅಕ್ಷರ *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  ಪೂರ್ಣಾಂಕ iDb;               /* Database index for <database> */
  ಪೂರ್ಣಾಂಕ rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ಸ್ಥಿರಾಂಕ PragmaName *pPragma;   /* The pragma */

  ಆದರೆ( v==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_Vdbe_ಸತತ_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  ಆದರೆ( iDb<0 ) ಹಿಂತಿರುಗಿ;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  ಆದರೆ( iDb==1 && sqlite3_Open_ತಾತ್ಕಾಲಿಕ_Database(pParse) ){
    ಹಿಂತಿರುಗಿ;
  }

  z_ಎಡ = sqlite3NameFromToken(db, pId);
  ಆದರೆ( !z_ಎಡ ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( minusFlag ){
    z_ಬಲ = sqlite3MPrintf(db, "-%T", p_ಮೌಲ್ಯ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ಬಲ = sqlite3NameFromToken(db, p_ಮೌಲ್ಯ);
  }

  ಪ್ರತಿಜ್ಞಾ( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_ಎಡ, z_ಬಲ, zDb) ){
    ಹೋಗು pragma_ಹೊರಹೋಗು;
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
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3_Vdbe_ಗಣ_ಅಂಕೆ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(v, 1);
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    return_Single_ಪಠ್ಯ(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    ಹೋಗು pragma_ಹೊರಹೋಗು;
  }
  ಆದರೆ( rc!=SQLITE_NOTFOUND ){
    ಆದರೆ( aFcntl[0] ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    ಹೋಗು pragma_ಹೊರಹೋಗು;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_ಎಡ);
  ಆದರೆ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    ಹೋಗು pragma_ಹೊರಹೋಗು;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  ಆದರೆ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    ಆದರೆ( sqlite3ReadSchema(pParse) ) ಹೋಗು pragma_ಹೊರಹೋಗು;
  }

  /* Register the result column names for pragmas that return results */
  ಆದರೆ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_ಬಲ==0)
  ){
    ಗಣ_Pragma_ಫಲಿತಾಂಶ_ಸ್ತಂಭ_ಹೆಸರುಗಳು(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  ಆಯ್ಕೆ( pPragma->ePragTyp ){
 
#ಆದರೆ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  ಸ್ಥಿತಿ Prag_Typ_DEFAULT_CACHE_ಗಾತ್ರ: {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList get_Cache_ಗಾತ್ರ[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_ಗಾತ್ರ},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_ಕಳೆ,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb);
    ಆದರೆ( !z_ಬಲ ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ಗಾತ್ರ(get_Cache_ಗಾತ್ರ));
      aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(get_Cache_ಗಾತ್ರ), get_Cache_ಗಾತ್ರ, iLn);
      ಆದರೆ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ನಿಲ್ಲಿಸು;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_ಗಾತ್ರ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = sqlite3AbsInt32(sqlite3Atoi(z_ಬಲ));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಗಣ_Cookie, iDb, BTREE_DEFAULT_CACHE_ಗಾತ್ರ, ಗಾತ್ರ_2);
      ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_ಗಾತ್ರ = ಗಾತ್ರ_2;
      sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(pDb->pBt, pDb->pSchema->cache_ಗಾತ್ರ);
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#ಆದರೆ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  ಸ್ಥಿತಿ Prag_Typ_ಪುಟ_ಗಾತ್ರ: {
    Btree *pBt = pDb->pBt;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಆದರೆ( !z_ಬಲ ){
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ALWAYS(pBt) ? sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt) : 0;
      returnSingleInt(v, ಗಾತ್ರ_2);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->ಮುಂದಿನ_Pagesize = sqlite3Atoi(z_ಬಲ);
      ಆದರೆ( SQLITE_NOMEM==sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(pBt, db->ಮುಂದಿನ_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  ಸ್ಥಿತಿ PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    ಪೂರ್ಣಾಂಕ b = -1;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಆದರೆ( z_ಬಲ ){
      ಆದರೆ( sqlite3_stricmp(z_ಬಲ, "fast")==0 ){
        b = 2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        b = sqlite3GetBoolean(z_ಬಲ, 0);
      }
    }
    ಆದರೆ( pId2->n==0 && b>=0 ){
      ಪೂರ್ಣಾಂಕ ii;
      ಗಾಗಿ(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_ಪುಟ_ಎಣಿಕೆ: {
    ಪೂರ್ಣಾಂಕ iReg;
    i64 x = 0;
    sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    ಆದರೆ( sqlite3Tolower(z_ಎಡ[0])=='p' ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Pagecount, iDb, iReg);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( z_ಬಲ && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ,&x)==0 ){
        ಆದರೆ( x<0 ) x = 0;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x>0xfffffffe ) x = 0xfffffffe;
      }ಇಲ್ಲದಿದ್ದರೆ{
        x = 0;
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_MaxPgcnt, iDb, iReg, (ಪೂರ್ಣಾಂಕ)x);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, iReg, 1);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  ಸ್ಥಿತಿ Prag_Typ_LOCKING_ಬಹುಲಕ: {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRet = "normal";
    ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ = get_Locking_ಬಹುಲಕ(z_ಬಲ);

    ಆದರೆ( pId2->n==0 && e_ಬಹುಲಕ==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_ಬಹುಲಕ = db->dflt_ಬೀಗ_ಬಹುಲಕ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Pager *pPager;
      ಆದರೆ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        ಪೂರ್ಣಾಂಕ ii;
        ಪ್ರತಿಜ್ಞಾ(pDb==&db->aDb[0]);
        ಗಾಗಿ(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3_Pager_Locking_ಬಹುಲಕ(pPager, e_ಬಹುಲಕ);
        }
        db->dflt_ಬೀಗ_ಬಹುಲಕ = (u8)e_ಬಹುಲಕ;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_ಬಹುಲಕ = sqlite3_Pager_Locking_ಬಹುಲಕ(pPager, e_ಬಹುಲಕ);
    }

    ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ==PAGER_LOCKINGMODE_ಸಾಮಾನ್ಯ
            || e_ಬಹುಲಕ==PAGER_LOCKINGMODE_EXCLUSIVE );
    ಆದರೆ( e_ಬಹುಲಕ==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    return_Single_ಪಠ್ಯ(v, zRet);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  ಸ್ಥಿತಿ Prag_Typ_JOURNAL_ಬಹುಲಕ: {
    ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    ಪೂರ್ಣಾಂಕ ii;           /* Loop counter */

    ಆದರೆ( z_ಬಲ==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_ಬಹುಲಕ = PAGER_JOURNALMODE_QUERY;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಹುಲಕ;
      ಪೂರ್ಣಾಂಕ n = sqlite3Strlen30(z_ಬಲ);
      ಗಾಗಿ(e_ಬಹುಲಕ=0; (z_ಬಹುಲಕ = sqlite3JournalModename(e_ಬಹುಲಕ))!=0; e_ಬಹುಲಕ++){
        ಆದರೆ( sqlite3StrNICmp(z_ಬಲ, z_ಬಹುಲಕ, n)==0 ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( !z_ಬಹುಲಕ ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_ಬಹುಲಕ = PAGER_JOURNALMODE_QUERY;
      }
      ಆದರೆ( e_ಬಹುಲಕ==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_ಬಹುಲಕ = PAGER_JOURNALMODE_QUERY;
      }
    }
    ಆದರೆ( e_ಬಹುಲಕ==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    ಗಾಗಿ(ii=db->nDb-1; ii>=0; ii--){
      ಆದರೆ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, ii);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Journal_ಬಹುಲಕ, ii, 1, e_ಬಹುಲಕ);
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 1);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  ಸ್ಥಿತಿ Prag_Typ_JOURNAL_ಗಾತ್ರ_ಎಲ್ಲೆ: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_ಎಲ್ಲೆ = -2;
    ಆದರೆ( z_ಬಲ ){
      sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &i_ಎಲ್ಲೆ);
      ಆದರೆ( i_ಎಲ್ಲೆ<-1 ) i_ಎಲ್ಲೆ = -1;
    }
    i_ಎಲ್ಲೆ = sqlite3_Pager_Journal_ಗಾತ್ರ_ಎಲ್ಲೆ(pPager, i_ಎಲ್ಲೆ);
    returnSingleInt(v, i_ಎಲ್ಲೆ);
    ನಿಲ್ಲಿಸು;
  }

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  ಸ್ಥಿತಿ PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಆದರೆ( !z_ಬಲ ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ eAuto = getAutoVacuum(z_ಬಲ);
      ಪ್ರತಿಜ್ಞಾ( eAuto>=0 && eAuto<=2 );
      db->ಮುಂದಿನ_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_ಗಣ_Auto_Vacuum(pBt, eAuto);
      ಆದರೆ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList ಗಣ_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_ಅತಿದೊಡ್ಡ_ಬೇರು_ಪುಟ},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_ಗಣ_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        ಪೂರ್ಣಾಂಕ iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_ಗಾತ್ರ(ಗಣ_Meta6));
        aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(ಗಣ_Meta6), ಗಣ_Meta6, iLn);
        ಆದರೆ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ನಿಲ್ಲಿಸು;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb);
      }
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  ಸ್ಥಿತಿ PragTyp_INCREMENTAL_VACUUM: {
    ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ = 0, addr;
    ಆದರೆ( z_ಬಲ==0 || !sqlite3GetInt32(z_ಬಲ, &i_ಎಲ್ಲೆ) || i_ಎಲ್ಲೆ<=0 ){
      i_ಎಲ್ಲೆ = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, i_ಎಲ್ಲೆ, 1);
    addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, 1, -1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
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
  ಸ್ಥಿತಿ Prag_Typ_CACHE_ಗಾತ್ರ: {
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಆದರೆ( !z_ಬಲ ){
      returnSingleInt(v, pDb->pSchema->cache_ಗಾತ್ರ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = sqlite3Atoi(z_ಬಲ);
      pDb->pSchema->cache_ಗಾತ್ರ = ಗಾತ್ರ_2;
      sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(pDb->pBt, pDb->pSchema->cache_ಗಾತ್ರ);
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ PragTyp_CACHE_SPILL: {
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಆದರೆ( !z_ಬಲ ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(pDb->pBt,0));
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = 1;
      ಆದರೆ( sqlite3GetInt32(z_ಬಲ, &ಗಾತ್ರ_2) ){
        sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(pDb->pBt, ಗಾತ್ರ_2);
      }
      ಆದರೆ( sqlite3GetBoolean(z_ಬಲ, ಗಾತ್ರ_2!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }ಇಲ್ಲದಿದ್ದರೆ{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      ಗಣ_All_Pager_Flags(db);
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_MMAP_ಗಾತ್ರ: {
    sqlite3_int64 sz;
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಆದರೆ( z_ಬಲ ){
      ಪೂರ್ಣಾಂಕ ii;
      sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &sz);
      ಆದರೆ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      ಆದರೆ( pId2->n==0 ) db->szMmap = sz;
      ಗಾಗಿ(ii=db->nDb-1; ii>=0; ii--){
        ಆದರೆ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_ಗಣ_Mmap_ಎಲ್ಲೆ(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_ಗಾತ್ರ, &sz);
#ಇಲ್ಲದಿದ್ದರೆ
    sz = 0;
    rc = SQLITE_OK;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_ತಾತ್ಕಾಲಿಕ_STORE: {
    ಆದರೆ( !z_ಬಲ ){
      returnSingleInt(v, db->ತಾತ್ಕಾಲಿಕ_store);
    }ಇಲ್ಲದಿದ್ದರೆ{
      change_ತಾತ್ಕಾಲಿಕ_Storage(pParse, z_ಬಲ);
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_ತಾತ್ಕಾಲಿಕ_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಆದರೆ( !z_ಬಲ ){
      return_Single_ಪಠ್ಯ(v, sqlite3_ತಾತ್ಕಾಲಿಕ_directory);
    }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
      ಆದರೆ( z_ಬಲ[0] ){
        ಪೂರ್ಣಾಂಕ res;
        rc = sqlite3OsAccess(db->pVfs, z_ಬಲ, SQLITE_ACCESS_READWRITE, &res);
        ಆದರೆ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ಹೋಗು pragma_ಹೊರಹೋಗು;
        }
      }
      ಆದರೆ( SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==0
       || (SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==1 && db->ತಾತ್ಕಾಲಿಕ_store<=1)
       || (SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==2 && db->ತಾತ್ಕಾಲಿಕ_store==1)
      ){
        invalidate_ತಾತ್ಕಾಲಿಕ_Storage(pParse);
      }
      sqlite3_free(sqlite3_ತಾತ್ಕಾಲಿಕ_directory);
      ಆದರೆ( z_ಬಲ[0] ){
        sqlite3_ತಾತ್ಕಾಲಿಕ_directory = sqlite3_mprintf("%s", z_ಬಲ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ತಾತ್ಕಾಲಿಕ_directory = 0;
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ನಿಲ್ಲಿಸು;
  }

#ಆದರೆ SQLITE_OS_WIN
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
  ಸ್ಥಿತಿ Prag_Typ_ದತ್ತಾಂಶ_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಆದರೆ( !z_ಬಲ ){
      return_Single_ಪಠ್ಯ(v, sqlite3_ದತ್ತಾಂಶ_directory);
    }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
      ಆದರೆ( z_ಬಲ[0] ){
        ಪೂರ್ಣಾಂಕ res;
        rc = sqlite3OsAccess(db->pVfs, z_ಬಲ, SQLITE_ACCESS_READWRITE, &res);
        ಆದರೆ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ಹೋಗು pragma_ಹೊರಹೋಗು;
        }
      }
      sqlite3_free(sqlite3_ದತ್ತಾಂಶ_directory);
      ಆದರೆ( z_ಬಲ[0] ){
        sqlite3_ದತ್ತಾಂಶ_directory = sqlite3_mprintf("%s", z_ಬಲ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ದತ್ತಾಂಶ_directory = 0;
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  ಸ್ಥಿತಿ Prag_Typ_ಬೀಗ_PROXY_FILE: {
    ಆದರೆ( !z_ಬಲ ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      ಅಕ್ಷರ *proxy_file_ಮಾರ್ಗ = ಖಾಲಿ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_ಮಾರ್ಗ);
      return_Single_ಪಠ್ಯ(v, proxy_file_ಮಾರ್ಗ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      ಪೂರ್ಣಾಂಕ res;
      ಆದರೆ( z_ಬಲ[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_ಗಣ_LOCKPROXYFILE,
                                     z_ಬಲ);
      } ಇಲ್ಲದಿದ್ದರೆ {
        res=sqlite3OsFileControl(pFile, SQLITE_ಗಣ_LOCKPROXYFILE,
                                     ಖಾಲಿ);
      }
      ಆದರೆ( res!=SQLITE_OK ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "failed to set lock proxy file");
        ಹೋಗು pragma_ಹೊರಹೋಗು;
      }
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  ಸ್ಥಿತಿ PragTyp_SYNCHRONOUS: {
    ಆದರೆ( !z_ಬಲ ){
      returnSingleInt(v, pDb->safety_level-1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( !db->autoCommit ){
        sqlite3_Error_ಸಂದೇಶ(pParse,
            "Safety level may not be changed inside a transaction");
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iDb!=1 ){
        ಪೂರ್ಣಾಂಕ iLevel;
        ಆದರೆ( sqlite3Isdigit(z_ಬಲ[0]) ){
          iLevel = sqlite3Atoi(z_ಬಲ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          iLevel = getSafetyLevel(z_ಬಲ,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_ಮುಖವಾಡ;
        ಆದರೆ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_ಗಣ = 1;
        ಗಣ_All_Pager_Flags(db);
      }
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_PAGER_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLAG_PRAGMAS
  ಸ್ಥಿತಿ PragTyp_FLAG: {
    ಆದರೆ( z_ಬಲ==0 ){
      ಗಣ_Pragma_ಫಲಿತಾಂಶ_ಸ್ತಂಭ_ಹೆಸರುಗಳು(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      u64 ಮುಖವಾಡ = pPragma->iArg;    /* Mask of bits to set or clear. */
      ಆದರೆ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        ಮುಖವಾಡ &= ~(SQLITE_Foreign_ಕೀಲಿಗಳು);
      }

      ಆದರೆ( sqlite3GetBoolean(z_ಬಲ, 0) ){
        ಆದರೆ( (ಮುಖವಾಡ & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= ಮುಖವಾಡ;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        db->flags &= ~ಮುಖವಾಡ;
        ಆದರೆ( ಮುಖವಾಡ==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        ಆದರೆ( (ಮುಖವಾಡ & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_ಬಲ, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_ದ_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Expire);
      ಗಣ_All_Pager_Flags(db);
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_FLAG_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SCHEMA_PRAGMAS
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
  ಸ್ಥಿತಿ Prag_Typ_ಕೋಷ್ಟಕ_INFO: ಆದರೆ( z_ಬಲ ){
    ಕೋಷ್ಟಕ *pTab;
    sqlite3_ಸಂಕೇತ_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, LOCATE_NOERR, z_ಬಲ, zDb);
    ಆದರೆ( pTab ){
      ಪೂರ್ಣಾಂಕ i, k;
      ಪೂರ್ಣಾಂಕ nHidden = 0;
      ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
      ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab);
      ಗಾಗಿ(i=0, p_ಲಂಬಸಾಲು=pTab->a_ಲಂಬಸಾಲು; i<pTab->n_ಲಂಬಸಾಲು; i++, p_ಲಂಬಸಾಲು++){
        ಪೂರ್ಣಾಂಕ isHidden = 0;
        ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_NOINSERT ){
          ಆದರೆ( pPragma->iArg==0 ){
            nHidden++;
            ಮುಂದುವರಿ;
          }
          ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }ಇಲ್ಲದಿದ್ದರೆ{ ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        ಆದರೆ( (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPk==0 ){
          k = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಗಾಗಿ(k=1; k<=pTab->n_ಲಂಬಸಾಲು && pPk->ai_ಸ್ತಂಭ[k-1]!=i; k++){}
        }
        p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pTab,p_ಲಂಬಸಾಲು);
        ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ==0 || p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಹರವು || isHidden>=2 );
        ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               p_ಲಂಬಸಾಲು->zCnName,
               sqlite3_ಸ್ತಂಭ_Type(p_ಲಂಬಸಾಲು,""),
               p_ಲಂಬಸಾಲು->notNull ? 1 : 0,
               (isHidden>=2 || p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ==0) ? 0 : p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,
               k,
               isHidden);
      }
    }
  }
  ನಿಲ್ಲಿಸು;

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
  ಸ್ಥಿತಿ Prag_Typ_ಕೋಷ್ಟಕ_LIST: {
    ಪೂರ್ಣಾಂಕ ii;
    pParse->nMem = 6;
    sqlite3_ಸಂಕೇತ_Verify_Named_Schema(pParse, zDb);
    ಗಾಗಿ(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      ಪೂರ್ಣಾಂಕ initNCol;
      ಆದರೆ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) ಮುಂದುವರಿ;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_ಎಣಿಕೆ(pHash);
      ಆಗ( initNCol-- ){
        ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(pHash); 1; k=sqlite_Hash_ಮುಂದಿನ(k) ){
          ಕೋಷ್ಟಕ *pTab;
          ಆದರೆ( k==0 ){ initNCol = 0; ನಿಲ್ಲಿಸು; }
          pTab = sqlite_Hash_ದತ್ತಾಂಶ(k);
          ಆದರೆ( pTab->n_ಲಂಬಸಾಲು==0 ){
            ಅಕ್ಷರ *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            ಆದರೆ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (ಶೂನ್ಯ)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (ಶೂನ್ಯ)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            ಆದರೆ( db->mallocFailed ){
              sqlite3_Error_ಸಂದೇಶ(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            ನಿಲ್ಲಿಸು;
          }
        }
      }

      ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(pHash); k; k=sqlite_Hash_ಮುಂದಿನ(k) ){
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(k);
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;
        ಆದರೆ( z_ಬಲ && sqlite3_stricmp(z_ಬಲ, pTab->zName)!=0 ) ಮುಂದುವರಿ;
        ಆದರೆ( IsView(pTab) ){
          zType = "view";
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( IsVirtual(pTab) ){
          zType = "virtual";
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }ಇಲ್ಲದಿದ್ದರೆ{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_ಕೋಷ್ಟಕ_Name(pTab->zName),
           zType,
           pTab->n_ಲಂಬಸಾಲು,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಸ್ಥಿತಿ PragTyp_STATS: {
    ಸೂಚ್ಯಂಕ *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
    ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&pDb->pSchema->tblHash); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
      ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(i);
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "ssiii",
           sqlite3_Preferred_ಕೋಷ್ಟಕ_Name(pTab->zName),
           0,
           pTab->sz_Tab_ಅಡ್ಡಸಾಲು,
           pTab->n_ಅಡ್ಡಸಾಲು_Log_Est,
           pTab->tabFlags);
      ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
        sqlite3_Vdbe_Multi_ತುಂಬು(v, 2, "siiiX",
           pIdx->zName,
           pIdx->sz_Idx_ಅಡ್ಡಸಾಲು,
           pIdx->ai_ಅಡ್ಡಸಾಲು_Log_Est[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 5);
      }
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ಥಿತಿ Prag_Typ_ಸೂಚ್ಯಂಕ_INFO: ಆದರೆ( z_ಬಲ ){
    ಸೂಚ್ಯಂಕ *pIdx;
    ಕೋಷ್ಟಕ *pTab;
    pIdx = sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(db, z_ಬಲ, zDb);
    ಆದರೆ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, LOCATE_NOERR, z_ಬಲ, zDb);
      ಆದರೆ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      }
    }
    ಆದರೆ( pIdx ){
      ಪೂರ್ಣಾಂಕ iIdxDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pIdx->pSchema);
      ಪೂರ್ಣಾಂಕ i;
      ಪೂರ್ಣಾಂಕ mx;
      ಆದರೆ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->n_ಸ್ತಂಭ;
        pParse->nMem = 6;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_ಕೋಷ್ಟಕ;
      sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iIdxDb);
      ಪ್ರತಿಜ್ಞಾ( pParse->nMem<=pPragma->nPragCName );
      ಗಾಗಿ(i=0; i<mx; i++){
        i16 cnum = pIdx->ai_ಸ್ತಂಭ[i];
        sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->a_ಲಂಬಸಾಲು[cnum].zCnName);
        ಆದರೆ( pPragma->iArg ){
          sqlite3_Vdbe_Multi_ತುಂಬು(v, 4, "isiX",
            pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[i],
            pIdx->azColl[i],
            i<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, pParse->nMem);
      }
    }
  }
  ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ Prag_Typ_ಸೂಚ್ಯಂಕ_LIST: ಆದರೆ( z_ಬಲ ){
    ಸೂಚ್ಯಂಕ *pIdx;
    ಕೋಷ್ಟಕ *pTab;
    ಪೂರ್ಣಾಂಕ i;
    pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, z_ಬಲ, zDb);
    ಆದರೆ( pTab ){
      ಪೂರ್ಣಾಂಕ iTabDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iTabDb);
      ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ, i=0; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, i++){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_ಸೂಚ್ಯಂಕ(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ PragTyp_DATABASE_LIST: {
    ಪೂರ್ಣಾಂಕ i;
    pParse->nMem = 3;
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      ಆದರೆ( db->aDb[i].pBt==0 ) ಮುಂದುವರಿ;
      ಪ್ರತಿಜ್ಞಾ( db->aDb[i].zDbSName!=0 );
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ PragTyp_COLLATION_LIST: {
    ಪೂರ್ಣಾಂಕ i = 0;
    HashElem *p;
    pParse->nMem = 2;
    ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&db->aCollSeq); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_ದತ್ತಾಂಶ(p);
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "is", i++, pColl->zName);
    }
  }
  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTROSPECTION_PRAGMAS
  ಸ್ಥಿತಿ PragTyp_FUNCTION_LIST: {
    ಪೂರ್ಣಾಂಕ i;
    HashElem *j;
    FuncDef *p;
    ಪೂರ್ಣಾಂಕ showInternFunc = (db->mDbFlags & DBFLAG_ಆಂತರಿಕ_Func)!=0;
    pParse->nMem = 6;
    ಗಾಗಿ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      ಗಾಗಿ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        ಪ್ರತಿಜ್ಞಾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_ಸಾಲು(v, p, 1, showInternFunc);
      }
    }
    ಗಾಗಿ(j=sqlite_Hash_ಮೊದಲ(&db->aFunc); j; j=sqlite_Hash_ಮುಂದಿನ(j)){
      p = (FuncDef*)sqlite_Hash_ದತ್ತಾಂಶ(j);
      ಪ್ರತಿಜ್ಞಾ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_ಸಾಲು(v, p, 0, showInternFunc);
    }
  }
  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿತಿ PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    ಗಾಗಿ(j=sqlite_Hash_ಮೊದಲ(&db->aModule); j; j=sqlite_Hash_ಮುಂದಿನ(j)){
      Module *pMod = (Module*)sqlite_Hash_ದತ್ತಾಂಶ(j);
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "s", pMod->zName);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

  ಸ್ಥಿತಿ PragTyp_PRAGMA_LIST: {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aPragmaName); i++){
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "s", aPragmaName[i].zName);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_INTROSPECTION_PRAGMAS */

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  ಸ್ಥಿತಿ Prag_Typ_FOREIGN_ಕೀಲಿ_LIST: ಆದರೆ( z_ಬಲ ){
    FKey *pFK;
    ಕೋಷ್ಟಕ *pTab;
    pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, z_ಬಲ, zDb);
    ಆದರೆ( pTab && Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
      pFK = pTab->u.tab.pFKey;
      ಆದರೆ( pFK ){
        ಪೂರ್ಣಾಂಕ iTabDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
        ಪೂರ್ಣಾಂಕ i = 0;
        pParse->nMem = 8;
        sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iTabDb);
        ಆಗ(pFK){
          ಪೂರ್ಣಾಂಕ j;
          ಗಾಗಿ(j=0; j<pFK->n_ಲಂಬಸಾಲು; j++){
            sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_ಗಮ್ಯ,
                   pTab->a_ಲಂಬಸಾಲು[pFK->a_ಲಂಬಸಾಲು[j].iFrom].zCnName,
                   pFK->a_ಲಂಬಸಾಲು[j].z_ಲಂಬಸಾಲು,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->p_ಮುಂದಿನ_From;
        }
      }
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಸ್ಥಿತಿ Prag_Typ_FOREIGN_ಕೀಲಿ_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    ಕೋಷ್ಟಕ *pTab;           /* Child table contain "REFERENCES" keyword */
    ಕೋಷ್ಟಕ *p_ಪೋಷಕ;        /* Parent table that child points to */
    ಸೂಚ್ಯಂಕ *pIdx;           /* Index in the parent table */
    ಪೂರ್ಣಾಂಕ i;                 /* Loop counter:  Foreign key number for pTab */
    ಪೂರ್ಣಾಂಕ j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    ಪೂರ್ಣಾಂಕ x;                 /* result variable */
    ಪೂರ್ಣಾಂಕ reg_ಫಲಿತಾಂಶ;         /* 3 registers to hold a result row */
    ಪೂರ್ಣಾಂಕ reg_ಅಡ್ಡಸಾಲು;            /* Registers to hold a row from pTab */
    ಪೂರ್ಣಾಂಕ addr_ತುದಿ;           /* Top of a loop checking foreign keys */
    ಪೂರ್ಣಾಂಕ addrOk;            /* Jump here if the key is OK */
    ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;           /* child to parent column mapping */

    reg_ಫಲಿತಾಂಶ = pParse->nMem+1;
    pParse->nMem += 4;
    reg_ಅಡ್ಡಸಾಲು = ++pParse->nMem;
    k = sqlite_Hash_ಮೊದಲ(&db->aDb[iDb].pSchema->tblHash);
    ಆಗ( k ){
      ಆದರೆ( z_ಬಲ ){
        pTab = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, 0, z_ಬಲ, zDb);
        k = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pTab = (ಕೋಷ್ಟಕ*)sqlite_Hash_ದತ್ತಾಂಶ(k);
        k = sqlite_Hash_ಮುಂದಿನ(k);
      }
      ಆದರೆ( pTab==0 || !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) || pTab->u.tab.pFKey==0 ) ಮುಂದುವರಿ;
      iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
      sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, pTab->n_ಲಂಬಸಾಲು+reg_ಅಡ್ಡಸಾಲು);
      sqlite3_Open_ಕೋಷ್ಟಕ(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ತುಂಬು_String(v, reg_ಫಲಿತಾಂಶ, pTab->zName);
      ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
      ಗಾಗಿ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ಮುಂದಿನ_From){
        p_ಪೋಷಕ = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, pFK->z_ಗಮ್ಯ, zDb);
        ಆದರೆ( p_ಪೋಷಕ==0 ) ಮುಂದುವರಿ;
        pIdx = 0;
        sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, p_ಪೋಷಕ->tnum, 0, p_ಪೋಷಕ->zName);
        x = sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, p_ಪೋಷಕ, pFK, &pIdx, 0);
        ಆದರೆ( x==0 ){
          ಆದರೆ( pIdx==0 ){
            sqlite3_Open_ಕೋಷ್ಟಕ(pParse, i, iDb, p_ಪೋಷಕ, OP_OpenRead);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pIdx);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          k = 0;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಪ್ರತಿಜ್ಞಾ( pParse->nErr>0 || pFK==0 );
      ಆದರೆ( pFK ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( pParse->nTab<i ) pParse->nTab = i;
      addr_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
      ಗಾಗಿ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ಮುಂದಿನ_From){
        p_ಪೋಷಕ = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, pFK->z_ಗಮ್ಯ, zDb);
        pIdx = 0;
        ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 0;
        ಆದರೆ( p_ಪೋಷಕ ){
          x = sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(pParse, p_ಪೋಷಕ, pFK, &pIdx, &ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
          ಪ್ರತಿಜ್ಞಾ( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, reg_ಅಡ್ಡಸಾಲು + pFK->n_ಲಂಬಸಾಲು);
        ಗಾಗಿ(j=0; j<pFK->n_ಲಂಬಸಾಲು; j++){
          ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ ? ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ[j] : pFK->a_ಲಂಬಸಾಲು[j].iFrom;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, 0, i_ಲಂಬಸಾಲು, reg_ಅಡ್ಡಸಾಲು+j);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, reg_ಅಡ್ಡಸಾಲು+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        ಆದರೆ( pIdx ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, reg_ಅಡ್ಡಸಾಲು, pFK->n_ಲಂಬಸಾಲು, 0,
              sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(db,pIdx), pFK->n_ಲಂಬಸಾಲು);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, i, addrOk, reg_ಅಡ್ಡಸಾಲು, pFK->n_ಲಂಬಸಾಲು);
          VdbeCoverage(v);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಪೋಷಕ ){
          ಪೂರ್ಣಾಂಕ jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_SeekRowid, i, jmp, reg_ಅಡ್ಡಸಾಲು); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          ಪ್ರತಿಜ್ಞಾ( pFK->n_ಲಂಬಸಾಲು==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        ಆದರೆ( HasRowid(pTab) ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, 0, reg_ಫಲಿತಾಂಶ+1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ಫಲಿತಾಂಶ+1);
        }
        sqlite3_Vdbe_Multi_ತುಂಬು(v, reg_ಫಲಿತಾಂಶ+2, "siX", pFK->z_ಗಮ್ಯ, i-1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, reg_ಫಲಿತಾಂಶ, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, 0, addr_ತುದಿ+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_ತುದಿ);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_TRIGGER) */
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  ಸ್ಥಿತಿ PragTyp_CASE_SENSITIVE_LIKE: {
    ಆದರೆ( z_ಬಲ ){
      sqlite3_ರಿಜಿಸ್ಟರ್_Like_Functions(db, sqlite3GetBoolean(z_ಬಲ, 0));
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK
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
  ಸ್ಥಿತಿ PragTyp_INTEGRITY_CHECK: {
    ಪೂರ್ಣಾಂಕ i, j, addr, mxErr;
    ಕೋಷ್ಟಕ *pObjTab = 0;     /* Check only this one table, if not NULL */

    ಪೂರ್ಣಾಂಕ is_ಶೀಘ್ರ = (sqlite3Tolower(z_ಎಡ[0])=='q');

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
    ಆದರೆ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    ಆದರೆ( z_ಬಲ ){
      ಆದರೆ( sqlite3GetInt32(p_ಮೌಲ್ಯ->z, &mxErr) ){
        ಆದರೆ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        pObjTab = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, 0, z_ಬಲ,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      ಪೂರ್ಣಾಂಕ *a_ಬೇರು;      /* Array of root page numbers of all btrees */
      ಪೂರ್ಣಾಂಕ cnt = 0;     /* Number of entries in aRoot[] */

      ಆದರೆ( OMIT_TEMPDB && i==1 ) ಮುಂದುವರಿ;
      ಆದರೆ( iDb>=0 && i!=iDb ) ಮುಂದುವರಿ;

      sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, i);
      pParse->ok_Const_ಗುಣಕ = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      ಗಾಗಿ(cnt=0, x=sqlite_Hash_ಮೊದಲ(pTbls); x; x=sqlite_Hash_ಮುಂದಿನ(x)){
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(x);  /* Current table */
        ಸೂಚ್ಯಂಕ *pIdx;                      /* An index on pTab */
        ಆದರೆ( ಕೋಷ್ಟಕ_Skip_Integrity_Check(pTab,pObjTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( HasRowid(pTab) ) cnt++;
        ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){ cnt++; }
      }
      ಆದರೆ( cnt==0 ) ಮುಂದುವರಿ;
      ಆದರೆ( pObjTab ) cnt++;
      a_ಬೇರು = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*(cnt+1));
      ಆದರೆ( a_ಬೇರು==0 ) ನಿಲ್ಲಿಸು;
      cnt = 0;
      ಆದರೆ( pObjTab ) a_ಬೇರು[++cnt] = 0;
      ಗಾಗಿ(x=sqlite_Hash_ಮೊದಲ(pTbls); x; x=sqlite_Hash_ಮುಂದಿನ(x)){
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(x);
        ಸೂಚ್ಯಂಕ *pIdx;
        ಆದರೆ( ಕೋಷ್ಟಕ_Skip_Integrity_Check(pTab,pObjTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( HasRowid(pTab) ) a_ಬೇರು[++cnt] = pTab->tnum;
        ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
          a_ಬೇರು[++cnt] = pIdx->tnum;
        }
      }
      a_ಬೇರು[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, 8+cnt);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_IntegrityCk, 1, cnt, 8, (ಅಕ್ಷರ*)a_ಬೇರು,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_ತುಂಬು_String(v, 2, "wrong # of entries in index ");
      ಗಾಗಿ(x=sqlite_Hash_ಮೊದಲ(pTbls); x; x=sqlite_Hash_ಮುಂದಿನ(x)){
        ಪೂರ್ಣಾಂಕ iTab = 0;
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(x);
        ಸೂಚ್ಯಂಕ *pIdx;
        ಆದರೆ( ಕೋಷ್ಟಕ_Skip_Integrity_Check(pTab,pObjTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( HasRowid(pTab) ){
          iTab = cnt++;
        }ಇಲ್ಲದಿದ್ದರೆ{
          iTab = cnt;
          ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; ALWAYS(pIdx); pIdx=pIdx->p_ಮುಂದಿನ){
            ಆದರೆ( Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ) ನಿಲ್ಲಿಸು;
            iTab++;
          }
        }
        ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
          ಆದರೆ( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_ತುಂಬು_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      ಗಾಗಿ(x=sqlite_Hash_ಮೊದಲ(pTbls); x; x=sqlite_Hash_ಮುಂದಿನ(x)){
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(x);
        ಸೂಚ್ಯಂಕ *pIdx, *pPk;
        ಸೂಚ್ಯಂಕ *p_ಪೂರ್ವ = 0;      /* Previous index */
        ಪೂರ್ಣಾಂಕ loop_ತುದಿ;
        ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur, iIdxCur;
        ಪೂರ್ಣಾಂಕ r1 = -1;
        ಪೂರ್ಣಾಂಕ bStrict;            /* True for a STRICT table */
        ಪೂರ್ಣಾಂಕ r2;                 /* Previous key for WITHOUT ROWID tables */
        ಪೂರ್ಣಾಂಕ mx_ಲಂಬಸಾಲು;              /* Maximum non-virtual column number */

        ಆದರೆ( ಕೋಷ್ಟಕ_Skip_Integrity_Check(pTab,pObjTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( is_ಶೀಘ್ರ || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
          r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 1, r2, r2+pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು-1);
        }
        sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_ದತ್ತಾಂಶ_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, 7);
        ಗಾಗಿ(j=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, j++){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        ಪ್ರತಿಜ್ಞಾ( pParse->nMem>=8+j );
        ಪ್ರತಿಜ್ಞಾ( sqlite3_No_Temps_ಒಳ_Range(pParse,1,7+j) );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, i_ದತ್ತಾಂಶ_Cur, 0); VdbeCoverage(v);
        loop_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        ಪ್ರತಿಜ್ಞಾ( !IsVirtual(pTab) );
        ಆದರೆ( HasRowid(pTab) ){
          mx_ಲಂಬಸಾಲು = -1;
          ಗಾಗಿ(j=0; j<pTab->n_ಲಂಬಸಾಲು; j++){
            ಆದರೆ( (pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)==0 ) mx_ಲಂಬಸಾಲು++;
          }
          ಆದರೆ( mx_ಲಂಬಸಾಲು==pTab->iPKey ) mx_ಲಂಬಸಾಲು--;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mx_ಲಂಬಸಾಲು = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab)->n_ಸ್ತಂಭ-1;
        }
        ಆದರೆ( mx_ಲಂಬಸಾಲು>=0 ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ದತ್ತಾಂಶ_Cur, mx_ಲಂಬಸಾಲು, 3);
          sqlite3_Vdbe_Typeof_ಸ್ತಂಭ(v, 3);
        }

        ಆದರೆ( !is_ಶೀಘ್ರ ){
          ಆದರೆ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            ಪೂರ್ಣಾಂಕ a1;
            ಅಕ್ಷರ *zErr;
            a1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IdxGT, i_ದತ್ತಾಂಶ_Cur, 0,r2,pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
            VdbeCoverage(v);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, a1);
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, a1+1);
            ಗಾಗಿ(j=0; j<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; j++){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತುಂಬು_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ(pParse, pPk, i_ದತ್ತಾಂಶ_Cur, j, r2+j);
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
        ಗಾಗಿ(j=0; j<pTab->n_ಲಂಬಸಾಲು; j++){
          ಅಕ್ಷರ *zErr;
          ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು + j;  /* The column to be checked */
          ಪೂರ್ಣಾಂಕ labelError;               /* Jump here to report an error */
          ಪೂರ್ಣಾಂಕ labelOk;                  /* Jump here if all looks ok */
          ಪೂರ್ಣಾಂಕ p1, p3, p4;               /* Operands to the OP_IsType opcode */
          ಪೂರ್ಣಾಂಕ doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          ಆದರೆ( j==pTab->iPKey ) ಮುಂದುವರಿ;
          ಆದರೆ( bStrict ){
            doTypeCheck = p_ಲಂಬಸಾಲು->eCType>COLTYPE_ANY;
          }ಇಲ್ಲದಿದ್ದರೆ{
            doTypeCheck = p_ಲಂಬಸಾಲು->affinity>SQLITE_AFF_BLOB;
          }
          ಆದರೆ( p_ಲಂಬಸಾಲು->notNull==0 && !doTypeCheck ) ಮುಂದುವರಿ;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಆದರೆ( p_ಲಂಬಸಾಲು->iDflt ){
              sqlite3_ಮೌಲ್ಯ *p_Dflt_ಮೌಲ್ಯ = 0;
              sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab,p_ಲಂಬಸಾಲು,pParse),
                                   ENC(db), p_ಲಂಬಸಾಲು->affinity, &p_Dflt_ಮೌಲ್ಯ);
              ಆದರೆ( p_Dflt_ಮೌಲ್ಯ ){
                p4 = sqlite3_ಮೌಲ್ಯ_type(p_Dflt_ಮೌಲ್ಯ);
                sqlite3_ಮೌಲ್ಯ_Free(p_Dflt_ಮೌಲ್ಯ);
              }
            }
            p1 = i_ದತ್ತಾಂಶ_Cur;
            ಆದರೆ( !HasRowid(pTab) ){
              testcase( j!=sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, j) );
              p3 = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab), j);
            }ಇಲ್ಲದಿದ್ದರೆ{
              p3 = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          ಆದರೆ( p_ಲಂಬಸಾಲು->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            ಪೂರ್ಣಾಂಕ jmp3;
            ಪೂರ್ಣಾಂಕ jmp2 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            ಆದರೆ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }ಇಲ್ಲದಿದ್ದರೆ{
              sqlite3VdbeChangeP5(v, 0x0d); /* INT, TEXT, or BLOB */
              /* OP_IsType does not detect NaN values in the database file
              ** which should be treated as a NULL.  So if the header type
              ** is REAL, we have to load the actual data using OP_Column
              ** to reliably determine if the value is a NULL. */
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, p1, p3, 3);
              sqlite3_ಸ್ತಂಭ_Default(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                p_ಲಂಬಸಾಲು->zCnName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            ಆದರೆ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp2);
              sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp3);
            }ಇಲ್ಲದಿದ್ದರೆ{
              /* VDBE byte code will fall thru */
            }
          }
          ಆದರೆ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_Std_Type_ಮುಖವಾಡ[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /*ಪಠ್ಯ*/
            };
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->eCType>=1 && p_ಲಂಬಸಾಲು->eCType<=ಗಾತ್ರ(a_Std_Type_ಮುಖವಾಡ) );
            sqlite3VdbeChangeP5(v, a_Std_Type_ಮುಖವಾಡ[p_ಲಂಬಸಾಲು->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[p_ಲಂಬಸಾಲು->eCType-1],
                                  pTab->zName, pTab->a_ಲಂಬಸಾಲು[j].zCnName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !bStrict && p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_ಪಠ್ಯ ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->a_ಲಂಬಸಾಲು[j].zCnName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !bStrict && p_ಲಂಬಸಾಲು->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            ಆದರೆ( p1>=0 ){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, i_ದತ್ತಾಂಶ_Cur, j, 3);
            }
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->a_ಲಂಬಸಾಲು[j].zCnName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        ಆದರೆ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ಅಭಿವ್ಯಕ್ತಿ_List *pCheck = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pTab->pCheck, 0);
          ಆದರೆ( db->mallocFailed==0 ){
            ಪೂರ್ಣಾಂಕ addrCkFault = sqlite3VdbeMakeLabel(pParse);
            ಪೂರ್ಣಾಂಕ addrCkOk = sqlite3VdbeMakeLabel(pParse);
            ಅಕ್ಷರ *zErr;
            ಪೂರ್ಣಾಂಕ k;
            pParse->iSelfTab = i_ದತ್ತಾಂಶ_Cur + 1;
            ಗಾಗಿ(k=pCheck->n_ಅಭಿವ್ಯಕ್ತಿ-1; k>0; k--){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pCheck->a[k].p_ಅಭಿವ್ಯಕ್ತಿ, addrCkFault, 0);
            }
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pCheck->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pCheck);
        }
        ಆದರೆ( !is_ಶೀಘ್ರ ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          ಗಾಗಿ(j=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, j++){
            ಪೂರ್ಣಾಂಕ jmp2, jmp3, jmp4, jmp5, label6;
            ಪೂರ್ಣಾಂಕ kk;
            ಪೂರ್ಣಾಂಕ ckUniq = sqlite3VdbeMakeLabel(pParse);
            ಆದರೆ( pPk==pIdx ) ಮುಂದುವರಿ;
            r1 = sqlite3_Generate_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(pParse, pIdx, i_ದತ್ತಾಂಶ_Cur, 0, 0, &jmp3,
                                         p_ಪೂರ್ವ, r1);
            p_ಪೂರ್ವ = pIdx;
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iIdxCur+j, ckUniq, r1,
                                        pIdx->n_ಸ್ತಂಭ); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_IFind_ಕೀಲಿ, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)pIdx, P4_ಸೂಚ್ಯಂಕ);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp2);
            sqlite3_Vdbe_ತುಂಬು_String(v, 3, "row ");
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_ತುಂಬು_String(v, 4, " missing from index ");
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_ತುಂಬು_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            ಆದರೆ( HasRowid(pTab) ){
              ಪೂರ್ಣಾಂಕ jmp7;
              sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, 3, 0, r1+pIdx->n_ಸ್ತಂಭ-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_ತುಂಬು_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ತುಂಬು_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            ಗಾಗಿ(kk=0; kk<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; kk++){
              ಆದರೆ( pIdx->azColl[kk]==sqlite3_Str_ದ್ವಿಮಾನ ) ಮುಂದುವರಿ;
              ಆದರೆ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            ಆದರೆ( label6 ){
              ಪೂರ್ಣಾಂಕ jmp6 = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_ತುಂಬು_String(v, 3, "row ");
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ತುಂಬು_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            ಆದರೆ( Is_Unique_ಸೂಚ್ಯಂಕ(pIdx) ){
              ಪೂರ್ಣಾಂಕ uniqOk = sqlite3VdbeMakeLabel(pParse);
              ಪೂರ್ಣಾಂಕ jmp6;
              ಗಾಗಿ(kk=0; kk<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; kk++){
                ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[kk];
                ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು!=XN_ROWID && i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು );
                ಆದರೆ( i_ಲಂಬಸಾಲು>=0 && pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].notNull ) ಮುಂದುವರಿ;
                sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ಮುಂದಿನ, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp6);
              sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು); VdbeCoverage(v);
              sqlite3_Vdbe_ತುಂಬು_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, i_ದತ್ತಾಂಶ_Cur, loop_ತುದಿ); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, loop_ತುದಿ-1);
        ಆದರೆ( pPk ){
          ಪ್ರತಿಜ್ಞಾ( !is_ಶೀಘ್ರ );
          sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r2, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
        }
      }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      ಗಾಗಿ(x=sqlite_Hash_ಮೊದಲ(pTbls); x; x=sqlite_Hash_ಮುಂದಿನ(x)){
        ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(x);
        sqlite3_vtab *pVTab;
        ಪೂರ್ಣಾಂಕ a1;
        ಆದರೆ( ಕೋಷ್ಟಕ_Skip_Integrity_Check(pTab,pObjTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( !IsVirtual(pTab) ) ಮುಂದುವರಿ;
        ಆದರೆ( pTab->n_ಲಂಬಸಾಲು<=0 ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zMod = pTab->u.vtab.azArg[0];
          ಆದರೆ( sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aModule, zMod)==0 ) ಮುಂದುವರಿ;
        }
        sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab);
        ಆದರೆ( pTab->u.vtab.p==0 ) ಮುಂದುವರಿ;
        pVTab = pTab->u.vtab.p->pVtab;
        ಆದರೆ( NEVER(pVTab==0) ) ಮುಂದುವರಿ;
        ಆದರೆ( NEVER(pVTab->pModule==0) ) ಮುಂದುವರಿ;
        ಆದರೆ( pVTab->pModule->i_ಆವೃತ್ತಿ<4 ) ಮುಂದುವರಿ;
        ಆದರೆ( pVTab->pModule->xIntegrity==0 ) ಮುಂದುವರಿ;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_VCheck, i, 3, is_ಶೀಘ್ರ);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, a1);
        ಮುಂದುವರಿ;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    {
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList ಕೊನೆ_ಸಂಕೇತ[] = {
        { OP_ಕೂಡಿಸು_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(ಕೊನೆ_ಸಂಕೇತ), ಕೊನೆ_ಸಂಕೇತ, iLn);
      ಆದರೆ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (ಅಕ್ಷರ*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
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
  ಸ್ಥಿತಿ PragTyp_ENCODING: {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ EncName {
      ಅಕ್ಷರ *zName;
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
    ಸ್ಥಿರಾಂಕ ರಚನೆ EncName *pEnc;
    ಆದರೆ( !z_ಬಲ ){    /* "PRAGMA encoding" */
      ಆದರೆ( sqlite3ReadSchema(pParse) ) ಹೋಗು pragma_ಹೊರಹೋಗು;
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      return_Single_ಪಠ್ಯ(v, encnames[ENC(pParse->db)].zName);
    }ಇಲ್ಲದಿದ್ದರೆ{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      ಆದರೆ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        ಗಾಗಿ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          ಆದರೆ( 0==sqlite3StrICmp(z_ಬಲ, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_ಗಣ_ಪಠ್ಯ_Encoding(db, enc);
            ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( !pEnc->zName ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "unsupported encoding: %s", z_ಬಲ);
        }
      }
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SCHEMA_ಆವೃತ್ತಿ_PRAGMAS
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
  ಸ್ಥಿತಿ Prag_Typ_ಶೀರ್ಷಿಕೆ_ಮೌಲ್ಯ: {
    ಪೂರ್ಣಾಂಕ iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb);
    ಆದರೆ( z_ಬಲ && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList ಗಣ_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_ಗಣ_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ಗಾತ್ರ(ಗಣ_Cookie));
      aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(ಗಣ_Cookie), ಗಣ_Cookie, 0);
      ಆದರೆ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ನಿಲ್ಲಿಸು;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_ಬಲ);
      aOp[1].p5 = 1;
      ಆದರೆ( iCookie==BTREE_SCHEMA_ಆವೃತ್ತಿ && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Read the specified cookie value */
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ಗಾತ್ರ(readCookie));
      aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(readCookie),readCookie,0);
      ಆದರೆ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ನಿಲ್ಲಿಸು;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  ಸ್ಥಿತಿ PragTyp_COMPILE_OPTIONS: {
    ಪೂರ್ಣಾಂಕ i = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpt;
    pParse->nMem = 1;
    ಆಗ( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3_Vdbe_ತುಂಬು_String(v, 1, zOpt);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  ಸ್ಥಿತಿ PragTyp_WAL_CHECKPOINT: {
    ಪೂರ್ಣಾಂಕ iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ = SQLITE_CHECKPOINT_PASSIVE;
    ಆದರೆ( z_ಬಲ ){
      ಆದರೆ( sqlite3StrICmp(z_ಬಲ, "full")==0 ){
        e_ಬಹುಲಕ = SQLITE_CHECKPOINT_ತುಂಬಿದ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(z_ಬಲ, "restart")==0 ){
        e_ಬಹುಲಕ = SQLITE_CHECKPOINT_RESTART;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(z_ಬಲ, "truncate")==0 ){
        e_ಬಹುಲಕ = SQLITE_CHECKPOINT_TRUNCATE;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(z_ಬಲ, "noop")==0 ){
        e_ಬಹುಲಕ = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Checkpoint, iBt, e_ಬಹುಲಕ, 1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, 1, 3);
  }
  ನಿಲ್ಲಿಸು;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  ಸ್ಥಿತಿ PragTyp_WAL_AUTOCHECKPOINT: {
    ಆದರೆ( z_ಬಲ ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_ಬಲ));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_ಗಮ್ಯ_INT(db->pWalArg) : 0);
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  ಸ್ಥಿತಿ Prag_Typ_SHRINK_ಸ್ಮರಣೆ: {
    sqlite3_db_release_ಸ್ಮರಣೆ(db);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ PragTyp_OPTIMIZE: {
    ಪೂರ್ಣಾಂಕ i_Db_ಕೊನೆಯ;           /* Loop termination point for the schema loop */
    ಪೂರ್ಣಾಂಕ iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    ಕೋಷ್ಟಕ *pTab;           /* A table in the schema */
    ಸೂಚ್ಯಂಕ *pIdx;           /* An index of the table */
    LogEst sz_ಮಿತಿ;    /* Size threshold above which reanalysis needed */
    ಅಕ್ಷರ *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_ಮುಖವಾಡ;            /* Mask of operations to perform */
    ಪೂರ್ಣಾಂಕ n_ಎಲ್ಲೆ;            /* Analysis limit to use */
    ಪೂರ್ಣಾಂಕ nCheck = 0;        /* Number of tables to be optimized */
    ಪೂರ್ಣಾಂಕ nBtree = 0;        /* Number of btrees to scan */
    ಪೂರ್ಣಾಂಕ n_ಸೂಚ್ಯಂಕ;            /* Number of indexes on the current table */

    ಆದರೆ( z_ಬಲ ){
      op_ಮುಖವಾಡ = (u32)sqlite3Atoi(z_ಬಲ);
      ಆದರೆ( (op_ಮುಖವಾಡ & 0x02)==0 ) ನಿಲ್ಲಿಸು;
    }ಇಲ್ಲದಿದ್ದರೆ{
      op_ಮುಖವಾಡ = 0xfffe;
    }
    ಆದರೆ( (op_ಮುಖವಾಡ & 0x10)==0 ){
      n_ಎಲ್ಲೆ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->n_Analysis_ಎಲ್ಲೆ>0
           && db->n_Analysis_ಎಲ್ಲೆ<SQLITE_DEFAULT_OPTIMIZE_ಎಲ್ಲೆ ){
      n_ಎಲ್ಲೆ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಎಲ್ಲೆ = SQLITE_DEFAULT_OPTIMIZE_ಎಲ್ಲೆ;
    }
    iTabCur = pParse->nTab++;
    ಗಾಗಿ(i_Db_ಕೊನೆಯ = zDb?iDb:db->nDb-1; iDb<=i_Db_ಕೊನೆಯ; iDb++){
      ಆದರೆ( iDb==1 ) ಮುಂದುವರಿ;
      sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); k; k=sqlite_Hash_ಮುಂದಿನ(k)){
        pTab = (ಕೋಷ್ಟಕ*)sqlite_Hash_ದತ್ತಾಂಶ(k);

        /* This only works for ordinary tables */
        ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಮುಂದುವರಿ;

        /* Do not scan system tables */
        ಆದರೆ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) ಮುಂದುವರಿ;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_ಮಿತಿ = pTab->n_ಅಡ್ಡಸಾಲು_Log_Est;
        n_ಸೂಚ್ಯಂಕ = 0;
        ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
          n_ಸೂಚ್ಯಂಕ++;
          ಆದರೆ( !pIdx->hasStat1 ){
            sz_ಮಿತಿ = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        ಆದರೆ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op_ಮುಖವಾಡ & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->p_ಸೂಚ್ಯಂಕ!=0 && sz_ಮಿತಿ<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Otherwise, we can skip this table */
          ಮುಂದುವರಿ;
        }

        nCheck++;
        ಆದರೆ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_ಸೂಚ್ಯಂಕ+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        ಆದರೆ( sz_ಮಿತಿ>=0 ){
          ಸ್ಥಿರಾಂಕ LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_If_ಗಾತ್ರ_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ಮುಖವಾಡ&1),
                         sz_ಮಿತಿ>=iRange ? sz_ಮಿತಿ-iRange : -1,
                         sz_ಮಿತಿ+iRange);
          VdbeCoverage(v);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_ಮುಖವಾಡ&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        ಆದರೆ( op_ಮುಖವಾಡ & 0x01 ){
          ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, r1, 1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_SqlExec, n_ಎಲ್ಲೆ ? 0x02 : 00, n_ಎಲ್ಲೆ, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    ಆದರೆ( !db->mallocFailed && n_ಎಲ್ಲೆ>0 && nBtree>100 ){
      ಪೂರ್ಣಾಂಕ iAddr, i_ಕೊನೆ;
      VdbeOp *aOp;
      n_ಎಲ್ಲೆ = 100*n_ಎಲ್ಲೆ/nBtree;
      ಆದರೆ( n_ಎಲ್ಲೆ<100 ) n_ಎಲ್ಲೆ = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_ಕೊನೆ = sqlite3VdbeCurrentAddr(v);
      ಗಾಗಿ(iAddr=0; iAddr<i_ಕೊನೆ; iAddr++){
        ಆದರೆ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_ಎಲ್ಲೆ;
      }
    }
    ನಿಲ್ಲಿಸು;
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
  /*case PragTyp_BUSY_TIMEOUT*/ ಪೂರ್ವನಿಯೋಜಿತ: {
    ಪೂರ್ಣಾಂಕ n = 0;
    ಪ್ರತಿಜ್ಞಾ( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    ಆದರೆ( z_ಬಲ && sqlite3GetInt32(z_ಬಲ,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_SOFT_ರಾಶಿ_ಎಲ್ಲೆ: {
    sqlite3_int64 N;
    ಆದರೆ( z_ಬಲ && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &N)==SQLITE_OK ){
      sqlite3_soft_ರಾಶಿ_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_ರಾಶಿ_limit64(-1));
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ Prag_Typ_HARD_ರಾಶಿ_ಎಲ್ಲೆ: {
    sqlite3_int64 N;
    ಆದರೆ( z_ಬಲ && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &N)==SQLITE_OK ){
      sqlite3_int64 i_ಪೂರ್ವ = sqlite3_hard_ರಾಶಿ_limit64(-1);
      ಆದರೆ( N>0 && (i_ಪೂರ್ವ==0 || i_ಪೂರ್ವ>N) ) sqlite3_hard_ರಾಶಿ_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_ರಾಶಿ_limit64(-1));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  ಸ್ಥಿತಿ Prag_Typ_ಎಳೆಗಳು: {
    sqlite3_int64 N;
    ಆದರೆ( z_ಬಲ
     && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_ಎಲ್ಲೆ(db, SQLITE_ಎಲ್ಲೆ_ಕೆಲಸಗಾರ_ಎಳೆಗಳು, (ಪೂರ್ಣಾಂಕ)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_ಎಲ್ಲೆ(db, SQLITE_ಎಲ್ಲೆ_ಕೆಲಸಗಾರ_ಎಳೆಗಳು, -1));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  ಸ್ಥಿತಿ Prag_Typ_ANALYSIS_ಎಲ್ಲೆ: {
    sqlite3_int64 N;
    ಆದರೆ( z_ಬಲ
     && sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z_ಬಲ, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_ಎಲ್ಲೆ = (ಪೂರ್ಣಾಂಕ)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_ಎಲ್ಲೆ); /* IMP: R-57594-65522 */
    ನಿಲ್ಲಿಸು;
  }

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  ಸ್ಥಿತಿ Prag_Typ_ಬೀಗ_ಸ್ಥಿತಿ: {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ az_ಬೀಗ_Name[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    ಪೂರ್ಣಾಂಕ i;
    pParse->nMem = 2;
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      Btree *pBt;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zState = "unknown";
      ಪೂರ್ಣಾಂಕ j;
      ಆದರೆ( db->aDb[i].zDbSName==0 ) ಮುಂದುವರಿ;
      pBt = db->aDb[i].pBt;
      ಆದರೆ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = az_ಬೀಗ_Name[j];
      }
      sqlite3_Vdbe_Multi_ತುಂಬು(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    ನಿಲ್ಲಿಸು;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_ENABLE_CEROD)
  ಸ್ಥಿತಿ PragTyp_ACTIVATE_EXTENSIONS: ಆದರೆ( z_ಬಲ ){
    ಆದರೆ( sqlite3StrNICmp(z_ಬಲ, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_ಬಲ[6]);
    }
  }
  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  ಆದರೆ( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_ಬಲ ){
    sqlite3_Vdbe_Verify_No_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(v);
  }

pragma_ಹೊರಹೋಗು:
  sqlite3DbFree(db, z_ಎಡ);
  sqlite3DbFree(db, z_ಬಲ);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PragmaVtab PragmaVtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PragmaVtabCursor PragmaVtabCursor;
ರಚನೆ PragmaVtab {
  sqlite3_vtab ತಳ;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ಸ್ಥಿರಾಂಕ PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
ರಚನೆ PragmaVtabCursor {
  sqlite3_vtab_cursor ತಳ; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  ಅಕ್ಷರ *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  ಸ್ಥಿರಾಂಕ PragmaName *pPragma = (ಸ್ಥಿರಾಂಕ PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i, j;
  ಅಕ್ಷರ cSep = '(';
  StrAccum acc;
  ಅಕ್ಷರ zBuf[200];

  ಬಳಸದ_PARAMETER(argc);
  ಬಳಸದ_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  ಗಾಗಿ(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  ಆದರೆ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  ಆದರೆ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  ಆದರೆ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
  ಪ್ರತಿಜ್ಞಾ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zBuf) < ಗಾತ್ರ(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  ಆದರೆ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(ಗಾತ್ರ(PragmaVtab));
    ಆದರೆ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }ಇಲ್ಲದಿದ್ದರೆ{
      memset(pTab, 0, ಗಾತ್ರ(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragma_Vtab_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ಸ್ಥಿರಾಂಕ ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *pConstraint;
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ ನೋಡಿದ[2];

  pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)1;
  ಆದರೆ( pTab->nHidden==0 ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  ನೋಡಿದ[0] = 0;
  ನೋಡಿದ[1] = 0;
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಆದರೆ( pConstraint->i_ಸ್ತಂಭ < pTab->iHidden ) ಮುಂದುವರಿ;
    ಆದರೆ( pConstraint->op!=SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ) ಮುಂದುವರಿ;
    ಆದರೆ( pConstraint->usable==0 ) ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
    j = pConstraint->i_ಸ್ತಂಭ - pTab->iHidden;
    ಪ್ರತಿಜ್ಞಾ( j < 2 );
    ನೋಡಿದ[j] = i+1;
  }
  ಆದರೆ( ನೋಡಿದ[0]==0 ){
    pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)2147483647;
    pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 2147483647;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  j = ನೋಡಿದ[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_ಸೂಚ್ಯಂಕ = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)20;
  pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 20;
  ಆದರೆ( ನೋಡಿದ[1] ){
    j = ನೋಡಿದ[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_ಸೂಚ್ಯಂಕ = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(ಗಾತ್ರ(*pCsr));
  ಆದರೆ( pCsr==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCsr, 0, ಗಾತ್ರ(PragmaVtabCursor));
  pCsr->ತಳ.pVtab = pVtab;
  *ppCursor = &pCsr->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ಸ್ಥಿರ ಶೂನ್ಯ pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  ಪೂರ್ಣಾಂಕ i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragma_Vtab_ಮುಂದಿನ(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  ಪ್ರತಿಜ್ಞಾ( pCsr->pPragma );
  ಆದರೆ( SQLITE_ಅಡ್ಡಸಾಲು!=sqlite3_ಹೆಜ್ಜೆ(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ i, j;
  StrAccum acc;
  ಅಕ್ಷರ *zSql;

  ಬಳಸದ_PARAMETER(idx_ಅಂಕೆ);
  ಬಳಸದ_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  ಗಾಗಿ(i=0; i<argc; i++, j++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಪ್ರತಿಜ್ಞಾ( j<Array_ಗಾತ್ರ(pCsr->azArg) );
    ಪ್ರತಿಜ್ಞಾ( pCsr->azArg[j]==0 );
    ಆದರೆ( z_ಪಠ್ಯ ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", z_ಪಠ್ಯ);
      ಆದರೆ( pCsr->azArg[j]==0 ){
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SQL_ಉದ್ದಳತೆ]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  ಆದರೆ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  ಆದರೆ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
  ಆದರೆ( zSql==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  ಆದರೆ( rc!=SQLITE_OK ){
    pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    ಹಿಂತಿರುಗಿ rc;
  }
  ಹಿಂತಿರುಗಿ pragma_Vtab_ಮುಂದಿನ(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ಹಿಂತಿರುಗಿ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragma_Vtab_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ಆದರೆ( i<pTab->iHidden ){
    sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCsr->pPragma, i));
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* The pragma virtual table object */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_ಮುಂದಿನ,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragma_Vtab_ಸ್ತಂಭ,            /* xColumn - read data */
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
Module *sqlite3_Pragma_Vtab_ರಿಜಿಸ್ಟರ್(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಸ್ಥಿರಾಂಕ PragmaName *pName;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  ಆದರೆ( pName==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aModule, zName)==0 );
  ಹಿಂತಿರುಗಿ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (ಶೂನ್ಯ*)pName, 0);
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_PRAGMA */
