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
#यदि_अन्तः

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
#यदि_अपरिभाषितम् SQLITE_DEFAULT_OPTIMIZE_सीमा
# define SQLITE_DEFAULT_OPTIMIZE_सीमा 2000
#यदि_अन्तः

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
स्थिर u8 getSafetyLevel(ध्रुव वर्ण *z, पूर्णाङ्क omit_पूर्णम्, u8 dflt){
                             /* 123456789 123456789 123 */
  स्थिर ध्रुव वर्ण zText[] = "onoffalseyestruextrafull";
  स्थिर ध्रुव u8 i_विस्थापनम्[] = {0, 1, 2,  4,    9,  12,  15,   20};
  स्थिर ध्रुव u8 i_दीर्घता[] = {2, 2, 3,  5,    3,   4,   5,    4};
  स्थिर ध्रुव u8 i_मानम्[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  पूर्णाङ्क i, n;
  यदि( sqlite3Isdigit(*z) ){
    प्रत्यागम sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  प्रति(i=0; i<Array_आकार(i_दीर्घता); i++){
    यदि( i_दीर्घता[i]==n && sqlite3StrNICmp(&zText[i_विस्थापनम्[i]],z,n)==0
     && (!omit_पूर्णम् || i_मानम्[i]<=1)
    ){
      प्रत्यागम i_मानम्[i];
    }
  }
  प्रत्यागम dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ध्रुव वर्ण *z, u8 dflt){
  प्रत्यागम getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#यदि !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
स्थिर पूर्णाङ्क get_Locking_बहुलकम्(ध्रुव वर्ण *z){
  यदि( z ){
    यदि( 0==sqlite3StrICmp(z, "exclusive") ) प्रत्यागम PAGER_LOCKINGMODE_EXCLUSIVE;
    यदि( 0==sqlite3StrICmp(z, "normal") ) प्रत्यागम PAGER_LOCKINGMODE_प्रसामान्यम्;
  }
  प्रत्यागम PAGER_LOCKINGMODE_QUERY;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
स्थिर पूर्णाङ्क getAutoVacuum(ध्रुव वर्ण *z){
  पूर्णाङ्क i;
  यदि( 0==sqlite3StrICmp(z, "none") ) प्रत्यागम BTREE_AUTOVACUUM_NONE;
  यदि( 0==sqlite3StrICmp(z, "full") ) प्रत्यागम BTREE_AUTOVACUUM_पूर्णम्;
  यदि( 0==sqlite3StrICmp(z, "incremental") ) प्रत्यागम BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  प्रत्यागम (u8)((i>=0&&i<=2)?i:0);
}
#यदि_अन्तः /* ifndef SQLITE_OMIT_AUTOVACUUM */

#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
स्थिर पूर्णाङ्क get_अस्थायी_Store(ध्रुव वर्ण *z){
  यदि( z[0]>='0' && z[0]<='2' ){
    प्रत्यागम z[0] - '0';
  }अन्यथा यदि( sqlite3StrICmp(z, "file")==0 ){
    प्रत्यागम 1;
  }अन्यथा यदि( sqlite3StrICmp(z, "memory")==0 ){
    प्रत्यागम 2;
  }अन्यथा{
    प्रत्यागम 0;
  }
}
#यदि_अन्तः /* SQLITE_PAGER_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
स्थिर पूर्णाङ्क invalidate_अस्थायी_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  यदि( db->aDb[1].pBt!=0 ){
    यदि( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_सन्देशः(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      प्रत्यागम SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_सम्बन्धि_Connection(db);
  }
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* SQLITE_PAGER_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
स्थिर पूर्णाङ्क change_अस्थायी_Storage(Parse *pParse, ध्रुव वर्ण *zStorageType){
  पूर्णाङ्क ts = get_अस्थायी_Store(zStorageType);
  sqlite3 *db = pParse->db;
  यदि( db->अस्थायी_store==ts ) प्रत्यागम SQLITE_OK;
  यदि( invalidate_अस्थायी_Storage( pParse ) != SQLITE_OK ){
    प्रत्यागम SQLITE_ERROR;
  }
  db->अस्थायी_store = (u8)ts;
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
स्थिर शून्य समुच्चयः_Pragma_परिणाम_Column_नामानि(
  Vdbe *v,                     /* The query under construction */
  ध्रुव PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_समुच्चयः_अङ्कः_स्तम्भसङ्ख्या(v, n==0 ? 1 : n);
  यदि( n==0 ){
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }अन्यथा{
    पूर्णाङ्क i, j;
    प्रति(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
स्थिर शून्य returnSingleInt(Vdbe *v, i64 मानम्){
  sqlite3_Vdbe_सङ्कलय_Int64(v, 1, मानम्);
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 1);
}

/*
** Generate code to return a single text value.
*/
स्थिर शून्य returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  ध्रुव वर्ण *z_मानम्      /* Value to be returned */
){
  यदि( z_मानम् ){
    sqlite3_Vdbe_आरोपय_String(v, 1, (ध्रुव वर्ण*)z_मानम्);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
स्थिर शून्य समुच्चयः_All_Pager_Flags(sqlite3 *db){
  यदि( db->autoCommit ){
    Db *pDb = db->aDb;
    पूर्णाङ्क n = db->nDb;
    प्रतिज्ञा( SQLITE_पूर्णम्_FSync==PAGER_FULLFSYNC );
    प्रतिज्ञा( SQLITE_Ckpt_पूर्णम्_FSync==PAGER_CKPT_FULLFSYNC );
    प्रतिज्ञा( SQLITE_CacheSpill==PAGER_CACHESPILL );
    प्रतिज्ञा( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_आवरणम् );
    प्रतिज्ञा( (pDb->safety_level & PAGER_SYNCHRONOUS_आवरणम्)==pDb->safety_level );
    यावत्( (n--) > 0 ){
      यदि( pDb->pBt ){
        sqlite3_Btree_समुच्चयः_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_आवरणम्) );
      }
      pDb++;
    }
  }
}
#अन्यथा
# define समुच्चयः_All_Pager_Flags(X)  /* no-op */
#यदि_अन्तः


/*
** Return a human-readable name for a constraint resolution action.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_FOREIGN_कुञ्चिका
स्थिर ध्रुव वर्ण *actionName(u8 action){
  ध्रुव वर्ण *zName;
  विकल्प( action ){
    पक्ष OE_समुच्चयः_Null:  zName = "SET NULL";        भङ्ग;
    पक्ष OE_समुच्चयः_Dflt:  zName = "SET DEFAULT";     भङ्ग;
    पक्ष OE_Cascade:  zName = "CASCADE";         भङ्ग;
    पक्ष OE_Restrict: zName = "RESTRICT";        भङ्ग;
    स्वभाव:          zName = "NO ACTION"; 
                      प्रतिज्ञा( action==OE_None ); भङ्ग;
  }
  प्रत्यागम zName;
}
#यदि_अन्तः


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ध्रुव वर्ण *sqlite3JournalModename(पूर्णाङ्क e_बहुलकम्){
  स्थिर वर्ण * ध्रुव az_बहुलकम्_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
     , "wal"
#यदि_अन्तः
  };
  प्रतिज्ञा( PAGER_JOURNALMODE_DELETE==0 );
  प्रतिज्ञा( PAGER_JOURNALMODE_PERSIST==1 );
  प्रतिज्ञा( PAGER_JOURNALMODE_OFF==2 );
  प्रतिज्ञा( PAGER_JOURNALMODE_TRUNCATE==3 );
  प्रतिज्ञा( PAGER_JOURNALMODE_स्मृतिः==4 );
  प्रतिज्ञा( PAGER_JOURNALMODE_WAL==5 );
  प्रतिज्ञा( e_बहुलकम्>=0 && e_बहुलकम्<=Array_आकार(az_बहुलकम्_Name) );

  यदि( e_बहुलकम्==Array_आकार(az_बहुलकम्_Name) ) प्रत्यागम 0;
  प्रत्यागम az_बहुलकम्_Name[e_बहुलकम्];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
स्थिर ध्रुव PragmaName *pragmaLocate(ध्रुव वर्ण *zName){
  पूर्णाङ्क upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_आकार(aPragmaName)-1;
  यावत्( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    यदि( rc==0 ) भङ्ग;
    यदि( rc<0 ){
      upr = mid - 1;
    }अन्यथा{
      lwr = mid + 1;
    }
  }
  प्रत्यागम lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
स्थिर शून्य pragma_Funclist_पङ्क्तिः(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  पूर्णाङ्क isBuiltin,         /* True if this is a built-in function */
  पूर्णाङ्क showInternFuncs    /* True if showing internal functions */
){
  u32 आवरणम् =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_आन्तरिकम्
  ;
  यदि( showInternFuncs ) आवरणम् = 0xffffffff;
  प्रति(; p; p=p->p_अग्र){
    ध्रुव वर्ण *zType;
    स्थिर ध्रुव वर्ण *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    प्रतिज्ञा( SQLITE_FUNC_ENCMASK==0x3 );
    प्रतिज्ञा( वाक्यं_तुलय(azEnc[SQLITE_UTF8],"utf8")==0 );
    प्रतिज्ञा( वाक्यं_तुलय(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    प्रतिज्ञा( वाक्यं_तुलय(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    यदि( p->xSFunc==0 ) अनुवर्त;
    यदि( (p->funcFlags & SQLITE_FUNC_आन्तरिकम्)!=0
     && showInternFuncs==0
    ){
      अनुवर्त;
    }   
    यदि( p->x_मानम्!=0 ){
      zType = "w";
    }अन्यथा यदि( p->xFinalize!=0 ){
      zType = "a";
    }अन्यथा{
      zType = "s";
    }
    sqlite3_Vdbe_Multi_आरोपय(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & आवरणम्) ^ SQLITE_INNOCUOUS
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
स्थिर पूर्णाङ्क integrity_Check_परिणाम_Row(Vdbe *v){
  पूर्णाङ्क addr;
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 3, 1);
  addr = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Halt);
  प्रत्यागम addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
स्थिर पूर्णाङ्क सारणी_Skip_Integrity_Check(ध्रुव सारणी *pTab, ध्रुव सारणी *pObjTab){
  यदि( pObjTab ){
    प्रत्यागम pTab!=pObjTab;
  }अन्यथा{
    प्रत्यागम (pTab->tabFlags & TF_Imposter)!=0;
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
  Token *p_मानम्,      /* Token for <value>, or NULL */
  पूर्णाङ्क minusFlag       /* True if a '-' sign preceded <value> */
){
  वर्ण *z_वामः = 0;       /* Nul-terminated UTF-8 string <id> */
  वर्ण *z_दक्षिणः = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ध्रुव वर्ण *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  वर्ण *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  पूर्णाङ्क iDb;               /* Database index for <database> */
  पूर्णाङ्क rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ध्रुव PragmaName *pPragma;   /* The pragma */

  यदि( v==0 ) प्रत्यागम;
  sqlite3_Vdbe_सततम्_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  यदि( iDb<0 ) प्रत्यागम;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  यदि( iDb==1 && sqlite3_Open_अस्थायी_Database(pParse) ){
    प्रत्यागम;
  }

  z_वामः = sqlite3NameFromToken(db, pId);
  यदि( !z_वामः ) प्रत्यागम;
  यदि( minusFlag ){
    z_दक्षिणः = sqlite3MPrintf(db, "-%T", p_मानम्);
  }अन्यथा{
    z_दक्षिणः = sqlite3NameFromToken(db, p_मानम्);
  }

  प्रतिज्ञा( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  यदि( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_वामः, z_दक्षिणः, zDb) ){
    गच्छ pragma_बहिर्गमनम्;
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
  aFcntl[1] = z_वामः;
  aFcntl[2] = z_दक्षिणः;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (शून्य*)aFcntl);
  यदि( rc==SQLITE_OK ){
    sqlite3_Vdbe_समुच्चयः_अङ्कः_स्तम्भसङ्ख्या(v, 1);
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    गच्छ pragma_बहिर्गमनम्;
  }
  यदि( rc!=SQLITE_NOTFOUND ){
    यदि( aFcntl[0] ){
      sqlite3_Error_सन्देशः(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    गच्छ pragma_बहिर्गमनम्;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_वामः);
  यदि( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    गच्छ pragma_बहिर्गमनम्;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  यदि( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    यदि( sqlite3ReadSchema(pParse) ) गच्छ pragma_बहिर्गमनम्;
  }

  /* Register the result column names for pragmas that return results */
  यदि( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_दक्षिणः==0)
  ){
    समुच्चयः_Pragma_परिणाम_Column_नामानि(v, pPragma);
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
  पक्ष Prag_Typ_DEFAULT_CACHE_आकार: {
    स्थिर ध्रुव पूर्णाङ्क iLn = VDBE_विस्थापनम्_LINENO(2);
    स्थिर ध्रुव VdbeOpList get_Cache_आकार[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_आकार},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_व्यवकलय,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_परिणाम_Row,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_उपयुङ्क्ते_Btree(v, iDb);
    यदि( !z_दक्षिणः ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(get_Cache_आकार));
      aOp = sqlite3_Vdbe_सङ्कलय_Op_List(v, Array_आकार(get_Cache_आकार), get_Cache_आकार, iLn);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) भङ्ग;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_आकार;
    }अन्यथा{
      पूर्णाङ्क आकार = sqlite3AbsInt32(sqlite3Atoi(z_दक्षिणः));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_समुच्चयः_Cookie, iDb, BTREE_DEFAULT_CACHE_आकार, आकार);
      प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_आकार = आकार;
      sqlite3_Btree_समुच्चयः_Cache_आकार(pDb->pBt, pDb->pSchema->cache_आकार);
    }
    भङ्ग;
  }
#यदि_अन्तः /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

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
  पक्ष Prag_Typ_PAGE_आकार: {
    Btree *pBt = pDb->pBt;
    प्रतिज्ञा( pBt!=0 );
    यदि( !z_दक्षिणः ){
      पूर्णाङ्क आकार = ALWAYS(pBt) ? sqlite3_Btree_Get_Page_आकार(pBt) : 0;
      returnSingleInt(v, आकार);
    }अन्यथा{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->अग्र_Pagesize = sqlite3Atoi(z_दक्षिणः);
      यदि( SQLITE_NOMEM==sqlite3_Btree_समुच्चयः_Page_आकार(pBt, db->अग्र_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    भङ्ग;
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
    यदि( z_दक्षिणः ){
      यदि( sqlite3_stricmp(z_दक्षिणः, "fast")==0 ){
        b = 2;
      }अन्यथा{
        b = sqlite3GetBoolean(z_दक्षिणः, 0);
      }
    }
    यदि( pId2->n==0 && b>=0 ){
      पूर्णाङ्क ii;
      प्रति(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    भङ्ग;
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
  पक्ष Prag_Typ_PAGE_गणनम्: {
    पूर्णाङ्क iReg;
    i64 x = 0;
    sqlite3_संहिता_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    यदि( sqlite3Tolower(z_वामः[0])=='p' ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Pagecount, iDb, iReg);
    }अन्यथा{
      यदि( z_दक्षिणः && sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः,&x)==0 ){
        यदि( x<0 ) x = 0;
        अन्यथा यदि( x>0xfffffffe ) x = 0xfffffffe;
      }अन्यथा{
        x = 0;
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_MaxPgcnt, iDb, iReg, (पूर्णाङ्क)x);
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, iReg, 1);
    भङ्ग;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  पक्ष Prag_Typ_LOCKING_बहुलकम्: {
    ध्रुव वर्ण *zRet = "normal";
    पूर्णाङ्क e_बहुलकम् = get_Locking_बहुलकम्(z_दक्षिणः);

    यदि( pId2->n==0 && e_बहुलकम्==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_बहुलकम् = db->dflt_तालकम्_बहुलकम्;
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
        प्रति(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3_Pager_Locking_बहुलकम्(pPager, e_बहुलकम्);
        }
        db->dflt_तालकम्_बहुलकम् = (u8)e_बहुलकम्;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_बहुलकम् = sqlite3_Pager_Locking_बहुलकम्(pPager, e_बहुलकम्);
    }

    प्रतिज्ञा( e_बहुलकम्==PAGER_LOCKINGMODE_प्रसामान्यम्
            || e_बहुलकम्==PAGER_LOCKINGMODE_EXCLUSIVE );
    यदि( e_बहुलकम्==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    भङ्ग;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  पक्ष Prag_Typ_JOURNAL_बहुलकम्: {
    पूर्णाङ्क e_बहुलकम्;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    पूर्णाङ्क ii;           /* Loop counter */

    यदि( z_दक्षिणः==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_बहुलकम् = PAGER_JOURNALMODE_QUERY;
    }अन्यथा{
      ध्रुव वर्ण *z_बहुलकम्;
      पूर्णाङ्क n = sqlite3Strlen30(z_दक्षिणः);
      प्रति(e_बहुलकम्=0; (z_बहुलकम् = sqlite3JournalModename(e_बहुलकम्))!=0; e_बहुलकम्++){
        यदि( sqlite3StrNICmp(z_दक्षिणः, z_बहुलकम्, n)==0 ) भङ्ग;
      }
      यदि( !z_बहुलकम् ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_बहुलकम् = PAGER_JOURNALMODE_QUERY;
      }
      यदि( e_बहुलकम्==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_बहुलकम् = PAGER_JOURNALMODE_QUERY;
      }
    }
    यदि( e_बहुलकम्==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    प्रति(ii=db->nDb-1; ii>=0; ii--){
      यदि( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_उपयुङ्क्ते_Btree(v, ii);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Journal_बहुलकम्, ii, 1, e_बहुलकम्);
      }
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 1);
    भङ्ग;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  पक्ष Prag_Typ_JOURNAL_आकार_सीमा: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_सीमा = -2;
    यदि( z_दक्षिणः ){
      sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &i_सीमा);
      यदि( i_सीमा<-1 ) i_सीमा = -1;
    }
    i_सीमा = sqlite3_Pager_Journal_आकार_सीमा(pPager, i_सीमा);
    returnSingleInt(v, i_सीमा);
    भङ्ग;
  }

#यदि_अन्तः /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOVACUUM
  पक्ष PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    प्रतिज्ञा( pBt!=0 );
    यदि( !z_दक्षिणः ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }अन्यथा{
      पूर्णाङ्क eAuto = getAutoVacuum(z_दक्षिणः);
      प्रतिज्ञा( eAuto>=0 && eAuto<=2 );
      db->अग्र_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_समुच्चयः_Auto_Vacuum(pBt, eAuto);
      यदि( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        स्थिर ध्रुव पूर्णाङ्क iLn = VDBE_विस्थापनम्_LINENO(2);
        स्थिर ध्रुव VdbeOpList समुच्चयः_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_मूलम्_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_समुच्चयः_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        पूर्णाङ्क iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(समुच्चयः_Meta6));
        aOp = sqlite3_Vdbe_सङ्कलय_Op_List(v, Array_आकार(समुच्चयः_Meta6), समुच्चयः_Meta6, iLn);
        यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) भङ्ग;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_उपयुङ्क्ते_Btree(v, iDb);
      }
    }
    भङ्ग;
  }
#यदि_अन्तः

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOVACUUM
  पक्ष PragTyp_INCREMENTAL_VACUUM: {
    पूर्णाङ्क i_सीमा = 0, addr;
    यदि( z_दक्षिणः==0 || !sqlite3GetInt32(z_दक्षिणः, &i_सीमा) || i_सीमा<=0 ){
      i_सीमा = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, i_सीमा, 1);
    addr = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_परिणाम_Row, 1);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, 1, -1);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_अत्र(v, addr);
    भङ्ग;
  }
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_PAGER_PRAGMAS
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
  पक्ष Prag_Typ_CACHE_आकार: {
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    यदि( !z_दक्षिणः ){
      returnSingleInt(v, pDb->pSchema->cache_आकार);
    }अन्यथा{
      पूर्णाङ्क आकार = sqlite3Atoi(z_दक्षिणः);
      pDb->pSchema->cache_आकार = आकार;
      sqlite3_Btree_समुच्चयः_Cache_आकार(pDb->pBt, pDb->pSchema->cache_आकार);
    }
    भङ्ग;
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
    यदि( !z_दक्षिणः ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_समुच्चयः_Spill_आकार(pDb->pBt,0));
    }अन्यथा{
      पूर्णाङ्क आकार = 1;
      यदि( sqlite3GetInt32(z_दक्षिणः, &आकार) ){
        sqlite3_Btree_समुच्चयः_Spill_आकार(pDb->pBt, आकार);
      }
      यदि( sqlite3GetBoolean(z_दक्षिणः, आकार!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }अन्यथा{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      समुच्चयः_All_Pager_Flags(db);
    }
    भङ्ग;
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
  पक्ष Prag_Typ_MMAP_आकार: {
    sqlite3_int64 sz;
#यदि SQLITE_MAX_MMAP_आकार>0
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    यदि( z_दक्षिणः ){
      पूर्णाङ्क ii;
      sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &sz);
      यदि( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      यदि( pId2->n==0 ) db->szMmap = sz;
      प्रति(ii=db->nDb-1; ii>=0; ii--){
        यदि( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_समुच्चयः_Mmap_सीमा(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_आकार, &sz);
#अन्यथा
    sz = 0;
    rc = SQLITE_OK;
#यदि_अन्तः
    यदि( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }अन्यथा यदि( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    भङ्ग;
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
  पक्ष Prag_Typ_अस्थायी_STORE: {
    यदि( !z_दक्षिणः ){
      returnSingleInt(v, db->अस्थायी_store);
    }अन्यथा{
      change_अस्थायी_Storage(pParse, z_दक्षिणः);
    }
    भङ्ग;
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
  पक्ष Prag_Typ_अस्थायी_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    यदि( !z_दक्षिणः ){
      returnSingleText(v, sqlite3_अस्थायी_directory);
    }अन्यथा{
#यदि_अपरिभाषितम् SQLITE_OMIT_WSD
      यदि( z_दक्षिणः[0] ){
        पूर्णाङ्क res;
        rc = sqlite3OsAccess(db->pVfs, z_दक्षिणः, SQLITE_ACCESS_READWRITE, &res);
        यदि( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_सन्देशः(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          गच्छ pragma_बहिर्गमनम्;
        }
      }
      यदि( SQLITE_अस्थायी_STORE==0
       || (SQLITE_अस्थायी_STORE==1 && db->अस्थायी_store<=1)
       || (SQLITE_अस्थायी_STORE==2 && db->अस्थायी_store==1)
      ){
        invalidate_अस्थायी_Storage(pParse);
      }
      sqlite3_free(sqlite3_अस्थायी_directory);
      यदि( z_दक्षिणः[0] ){
        sqlite3_अस्थायी_directory = sqlite3_mprintf("%s", z_दक्षिणः);
      }अन्यथा{
        sqlite3_अस्थायी_directory = 0;
      }
#यदि_अन्तः /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    भङ्ग;
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
  पक्ष Prag_Typ_दत्त_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    यदि( !z_दक्षिणः ){
      returnSingleText(v, sqlite3_दत्त_directory);
    }अन्यथा{
#यदि_अपरिभाषितम् SQLITE_OMIT_WSD
      यदि( z_दक्षिणः[0] ){
        पूर्णाङ्क res;
        rc = sqlite3OsAccess(db->pVfs, z_दक्षिणः, SQLITE_ACCESS_READWRITE, &res);
        यदि( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_सन्देशः(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          गच्छ pragma_बहिर्गमनम्;
        }
      }
      sqlite3_free(sqlite3_दत्त_directory);
      यदि( z_दक्षिणः[0] ){
        sqlite3_दत्त_directory = sqlite3_mprintf("%s", z_दक्षिणः);
      }अन्यथा{
        sqlite3_दत्त_directory = 0;
      }
#यदि_अन्तः /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    भङ्ग;
  }
#यदि_अन्तः

#यदि SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  पक्ष Prag_Typ_तालकम्_PROXY_FILE: {
    यदि( !z_दक्षिणः ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      वर्ण *proxy_file_मार्गः = रिक्त;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_मार्गः);
      returnSingleText(v, proxy_file_मार्गः);
    }अन्यथा{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      पूर्णाङ्क res;
      यदि( z_दक्षिणः[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_समुच्चयः_LOCKPROXYFILE,
                                     z_दक्षिणः);
      } अन्यथा {
        res=sqlite3OsFileControl(pFile, SQLITE_समुच्चयः_LOCKPROXYFILE,
                                     रिक्त);
      }
      यदि( res!=SQLITE_OK ){
        sqlite3_Error_सन्देशः(pParse, "failed to set lock proxy file");
        गच्छ pragma_बहिर्गमनम्;
      }
    }
    भङ्ग;
  }
#यदि_अन्तः /* SQLITE_ENABLE_LOCKING_STYLE */     
   
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
    यदि( !z_दक्षिणः ){
      returnSingleInt(v, pDb->safety_level-1);
    }अन्यथा{
      यदि( !db->autoCommit ){
        sqlite3_Error_सन्देशः(pParse,
            "Safety level may not be changed inside a transaction");
      }अन्यथा यदि( iDb!=1 ){
        पूर्णाङ्क iLevel;
        यदि( sqlite3Isdigit(z_दक्षिणः[0]) ){
          iLevel = sqlite3Atoi(z_दक्षिणः);
        }अन्यथा{
          iLevel = getSafetyLevel(z_दक्षिणः,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_आवरणम्;
        यदि( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_समुच्चयः = 1;
        समुच्चयः_All_Pager_Flags(db);
      }
    }
    भङ्ग;
  }
#यदि_अन्तः /* SQLITE_OMIT_PAGER_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_FLAG_PRAGMAS
  पक्ष PragTyp_FLAG: {
    यदि( z_दक्षिणः==0 ){
      समुच्चयः_Pragma_परिणाम_Column_नामानि(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }अन्यथा{
      u64 आवरणम् = pPragma->iArg;    /* Mask of bits to set or clear. */
      यदि( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        आवरणम् &= ~(SQLITE_Foreign_कुञ्चिकाः);
      }

      यदि( sqlite3GetBoolean(z_दक्षिणः, 0) ){
        यदि( (आवरणम् & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= आवरणम्;
        }
      }अन्यथा{
        db->flags &= ~आवरणम्;
        यदि( आवरणम्==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        यदि( (आवरणम् & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_दक्षिणः, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_सम्बन्धि_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Expire);
      समुच्चयः_All_Pager_Flags(db);
    }
    भङ्ग;
  }
#यदि_अन्तः /* SQLITE_OMIT_FLAG_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_SCHEMA_PRAGMAS
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
  पक्ष Prag_Typ_सारणी_INFO: यदि( z_दक्षिणः ){
    सारणी *pTab;
    sqlite3_संहिता_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_सारणी(pParse, LOCATE_NOERR, z_दक्षिणः, zDb);
    यदि( pTab ){
      पूर्णाङ्क i, k;
      पूर्णाङ्क nHidden = 0;
      Column *pCol;
      सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_Column_नामानि(pParse, pTab);
      प्रति(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        पूर्णाङ्क isHidden = 0;
        ध्रुव व्यञ्जकम् *p_Col_व्यञ्जकम्;
        यदि( pCol->colFlags & COLFLAG_NOINSERT ){
          यदि( pPragma->iArg==0 ){
            nHidden++;
            अनुवर्त;
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
          प्रति(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_व्यञ्जकम् = sqlite3_Column_व्यञ्जकम्(pTab,pCol);
        प्रतिज्ञा( p_Col_व्यञ्जकम्==0 || p_Col_व्यञ्जकम्->op==TK_विस्तृतिः || isHidden>=2 );
        प्रतिज्ञा( p_Col_व्यञ्जकम्==0 || !व्यञ्जकम्_Has_Property(p_Col_व्यञ्जकम्, EP_Int_मानम्)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_आरोपय(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_व्यञ्जकम्==0) ? 0 : p_Col_व्यञ्जकम्->u.zToken,
               k,
               isHidden);
      }
    }
  }
  भङ्ग;

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
  पक्ष Prag_Typ_सारणी_LIST: {
    पूर्णाङ्क ii;
    pParse->nMem = 6;
    sqlite3_संहिता_Verify_Named_Schema(pParse, zDb);
    प्रति(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      पूर्णाङ्क initNCol;
      यदि( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) अनुवर्त;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_गणनम्(pHash);
      यावत्( initNCol-- ){
        प्रति(k=sqliteHashFirst(pHash); 1; k=sqlite_Hash_अग्र(k) ){
          सारणी *pTab;
          यदि( k==0 ){ initNCol = 0; भङ्ग; }
          pTab = sqlite_Hash_दत्त(k);
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
              sqlite3_Error_सन्देशः(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            भङ्ग;
          }
        }
      }

      प्रति(k=sqliteHashFirst(pHash); k; k=sqlite_Hash_अग्र(k) ){
        सारणी *pTab = sqlite_Hash_दत्त(k);
        ध्रुव वर्ण *zType;
        यदि( z_दक्षिणः && sqlite3_stricmp(z_दक्षिणः, pTab->zName)!=0 ) अनुवर्त;
        यदि( IsView(pTab) ){
          zType = "view";
        }अन्यथा यदि( IsVirtual(pTab) ){
          zType = "virtual";
        }अन्यथा यदि( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }अन्यथा{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_आरोपय(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_सारणी_Name(pTab->zName),
           zType,
           pTab->nCol,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  भङ्ग;

#यदि_परिभाषितम् SQLITE_DEBUG
  पक्ष PragTyp_STATS: {
    सूचकांक *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_संहिता_Verify_Schema(pParse, iDb);
    प्रति(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqlite_Hash_अग्र(i)){
      सारणी *pTab = sqlite_Hash_दत्त(i);
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "ssiii",
           sqlite3_Preferred_सारणी_Name(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
        sqlite3_Vdbe_Multi_आरोपय(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 5);
      }
    }
  }
  भङ्ग;
#यदि_अन्तः

  पक्ष Prag_Typ_सूचकांक_INFO: यदि( z_दक्षिणः ){
    सूचकांक *pIdx;
    सारणी *pTab;
    pIdx = sqlite3_Find_सूचकांक(db, z_दक्षिणः, zDb);
    यदि( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_सारणी(pParse, LOCATE_NOERR, z_दक्षिणः, zDb);
      यदि( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      }
    }
    यदि( pIdx ){
      पूर्णाङ्क iIdxDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pIdx->pSchema);
      पूर्णाङ्क i;
      पूर्णाङ्क mx;
      यदि( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }अन्यथा{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_कुञ्चिका_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_सारणी;
      sqlite3_संहिता_Verify_Schema(pParse, iIdxDb);
      प्रतिज्ञा( pParse->nMem<=pPragma->nPragCName );
      प्रति(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3_Vdbe_Multi_आरोपय(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        यदि( pPragma->iArg ){
          sqlite3_Vdbe_Multi_आरोपय(v, 4, "isiX",
            pIdx->a_क्रमय_क्रमः[i],
            pIdx->azColl[i],
            i<pIdx->n_कुञ्चिका_Col);
        }
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, pParse->nMem);
      }
    }
  }
  भङ्ग;

  पक्ष Prag_Typ_सूचकांक_LIST: यदि( z_दक्षिणः ){
    सूचकांक *pIdx;
    सारणी *pTab;
    पूर्णाङ्क i;
    pTab = sqlite3_Find_सारणी(db, z_दक्षिणः, zDb);
    यदि( pTab ){
      पूर्णाङ्क iTabDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_संहिता_Verify_Schema(pParse, iTabDb);
      प्रति(pIdx=pTab->p_सूचकांक, i=0; pIdx; pIdx=pIdx->p_अग्र, i++){
        ध्रुव वर्ण *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_आरोपय(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_सूचकांक(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  भङ्ग;

  पक्ष PragTyp_DATABASE_LIST: {
    पूर्णाङ्क i;
    pParse->nMem = 3;
    प्रति(i=0; i<db->nDb; i++){
      यदि( db->aDb[i].pBt==0 ) अनुवर्त;
      प्रतिज्ञा( db->aDb[i].zDbSName!=0 );
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  भङ्ग;

  पक्ष PragTyp_COLLATION_LIST: {
    पूर्णाङ्क i = 0;
    HashElem *p;
    pParse->nMem = 2;
    प्रति(p=sqliteHashFirst(&db->aCollSeq); p; p=sqlite_Hash_अग्र(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_दत्त(p);
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "is", i++, pColl->zName);
    }
  }
  भङ्ग;

#यदि_अपरिभाषितम् SQLITE_OMIT_INTROSPECTION_PRAGMAS
  पक्ष PragTyp_FUNCTION_LIST: {
    पूर्णाङ्क i;
    HashElem *j;
    FuncDef *p;
    पूर्णाङ्क showInternFunc = (db->mDbFlags & DBFLAG_आन्तरिकम्_Func)!=0;
    pParse->nMem = 6;
    प्रति(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      प्रति(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        प्रतिज्ञा( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_पङ्क्तिः(v, p, 1, showInternFunc);
      }
    }
    प्रति(j=sqliteHashFirst(&db->aFunc); j; j=sqlite_Hash_अग्र(j)){
      p = (FuncDef*)sqlite_Hash_दत्त(j);
      प्रतिज्ञा( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_पङ्क्तिः(v, p, 0, showInternFunc);
    }
  }
  भङ्ग;

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  पक्ष PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    प्रति(j=sqliteHashFirst(&db->aModule); j; j=sqlite_Hash_अग्र(j)){
      Module *pMod = (Module*)sqlite_Hash_दत्त(j);
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "s", pMod->zName);
    }
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

  पक्ष PragTyp_PRAGMA_LIST: {
    पूर्णाङ्क i;
    प्रति(i=0; i<Array_आकार(aPragmaName); i++){
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "s", aPragmaName[i].zName);
    }
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_INTROSPECTION_PRAGMAS */

#यदि_अन्तः /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_FOREIGN_कुञ्चिका
  पक्ष Prag_Typ_FOREIGN_कुञ्चिका_LIST: यदि( z_दक्षिणः ){
    FKey *pFK;
    सारणी *pTab;
    pTab = sqlite3_Find_सारणी(db, z_दक्षिणः, zDb);
    यदि( pTab && Is_Ordinary_सारणी(pTab) ){
      pFK = pTab->u.tab.pFKey;
      यदि( pFK ){
        पूर्णाङ्क iTabDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
        पूर्णाङ्क i = 0;
        pParse->nMem = 8;
        sqlite3_संहिता_Verify_Schema(pParse, iTabDb);
        यावत्(pFK){
          पूर्णाङ्क j;
          प्रति(j=0; j<pFK->nCol; j++){
            sqlite3_Vdbe_Multi_आरोपय(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_गन्तव्यम्,
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
  भङ्ग;
#यदि_अन्तः /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#यदि_अपरिभाषितम् SQLITE_OMIT_FOREIGN_कुञ्चिका
#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  पक्ष Prag_Typ_FOREIGN_कुञ्चिका_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    सारणी *pTab;           /* Child table contain "REFERENCES" keyword */
    सारणी *pParent;        /* Parent table that child points to */
    सूचकांक *pIdx;           /* Index in the parent table */
    पूर्णाङ्क i;                 /* Loop counter:  Foreign key number for pTab */
    पूर्णाङ्क j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    पूर्णाङ्क x;                 /* result variable */
    पूर्णाङ्क reg_परिणाम;         /* 3 registers to hold a result row */
    पूर्णाङ्क regRow;            /* Registers to hold a row from pTab */
    पूर्णाङ्क addr_शिखरम्;           /* Top of a loop checking foreign keys */
    पूर्णाङ्क addrOk;            /* Jump here if the key is OK */
    पूर्णाङ्क *ai_स्तम्भसङ्ख्या;           /* child to parent column mapping */

    reg_परिणाम = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    यावत्( k ){
      यदि( z_दक्षिणः ){
        pTab = sqlite3_Locate_सारणी(pParse, 0, z_दक्षिणः, zDb);
        k = 0;
      }अन्यथा{
        pTab = (सारणी*)sqlite_Hash_दत्त(k);
        k = sqlite_Hash_अग्र(k);
      }
      यदि( pTab==0 || !Is_Ordinary_सारणी(pTab) || pTab->u.tab.pFKey==0 ) अनुवर्त;
      iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_संहिता_Verify_Schema(pParse, iDb);
      sqlite3_सारणी_तालकम्(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_पञ्जिकाकोशः(pParse, pTab->nCol+regRow);
      sqlite3_Open_सारणी(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_आरोपय_String(v, reg_परिणाम, pTab->zName);
      प्रतिज्ञा( Is_Ordinary_सारणी(pTab) );
      प्रति(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_अग्र_From){
        pParent = sqlite3_Find_सारणी(db, pFK->z_गन्तव्यम्, zDb);
        यदि( pParent==0 ) अनुवर्त;
        pIdx = 0;
        sqlite3_सारणी_तालकम्(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3_Fk_Locate_सूचकांक(pParse, pParent, pFK, &pIdx, 0);
        यदि( x==0 ){
          यदि( pIdx==0 ){
            sqlite3_Open_सारणी(pParse, i, iDb, pParent, OP_OpenRead);
          }अन्यथा{
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pIdx);
          }
        }अन्यथा{
          k = 0;
          भङ्ग;
        }
      }
      प्रतिज्ञा( pParse->nErr>0 || pFK==0 );
      यदि( pFK ) भङ्ग;
      यदि( pParse->nTab<i ) pParse->nTab = i;
      addr_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      प्रतिज्ञा( Is_Ordinary_सारणी(pTab) );
      प्रति(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_अग्र_From){
        pParent = sqlite3_Find_सारणी(db, pFK->z_गन्तव्यम्, zDb);
        pIdx = 0;
        ai_स्तम्भसङ्ख्या = 0;
        यदि( pParent ){
          x = sqlite3_Fk_Locate_सूचकांक(pParse, pParent, pFK, &pIdx, &ai_स्तम्भसङ्ख्या);
          प्रतिज्ञा( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_पञ्जिकाकोशः(pParse, regRow + pFK->nCol);
        प्रति(j=0; j<pFK->nCol; j++){
          पूर्णाङ्क iCol = ai_स्तम्भसङ्ख्या ? ai_स्तम्भसङ्ख्या[j] : pFK->aCol[j].iFrom;
          sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, 0, iCol, regRow+j);
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        यदि( pIdx ){
          sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3_सूचकांक_Affinity_Str(db,pIdx), pFK->nCol);
          sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }अन्यथा यदि( pParent ){
          पूर्णाङ्क jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          प्रतिज्ञा( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        यदि( HasRowid(pTab) ){
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, 0, reg_परिणाम+1);
        }अन्यथा{
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, reg_परिणाम+1);
        }
        sqlite3_Vdbe_Multi_आरोपय(v, reg_परिणाम+2, "siX", pFK->z_गन्तव्यम्, i-1);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, reg_परिणाम, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_स्तम्भसङ्ख्या);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, 0, addr_शिखरम्+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_अत्र(v, addr_शिखरम्);
    }
  }
  भङ्ग;
#यदि_अन्तः /* !defined(SQLITE_OMIT_TRIGGER) */
#यदि_अन्तः /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#यदि_अपरिभाषितम् SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  पक्ष PragTyp_CASE_SENSITIVE_LIKE: {
    यदि( z_दक्षिणः ){
      sqlite3_पञ्जिकाकोशः_Like_Functions(db, sqlite3GetBoolean(z_दक्षिणः, 0));
    }
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#यदि_अपरिभाषितम् SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_INTEGRITY_CHECK
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
    सारणी *pObjTab = 0;     /* Check only this one table, if not NULL */

    पूर्णाङ्क is_शीघ्र = (sqlite3Tolower(z_वामः[0])=='q');

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
    यदि( z_दक्षिणः ){
      यदि( sqlite3GetInt32(p_मानम्->z, &mxErr) ){
        यदि( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }अन्यथा{
        pObjTab = sqlite3_Locate_सारणी(pParse, 0, z_दक्षिणः,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    प्रति(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      पूर्णाङ्क *a_मूलम्;      /* Array of root page numbers of all btrees */
      पूर्णाङ्क cnt = 0;     /* Number of entries in aRoot[] */

      यदि( OMIT_TEMPDB && i==1 ) अनुवर्त;
      यदि( iDb>=0 && i!=iDb ) अनुवर्त;

      sqlite3_संहिता_Verify_Schema(pParse, i);
      pParse->ok_Const_गुणकः = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      प्रतिज्ञा( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      प्रति(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        सारणी *pTab = sqlite_Hash_दत्त(x);  /* Current table */
        सूचकांक *pIdx;                      /* An index on pTab */
        यदि( सारणी_Skip_Integrity_Check(pTab,pObjTab) ) अनुवर्त;
        यदि( HasRowid(pTab) ) cnt++;
        प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){ cnt++; }
      }
      यदि( cnt==0 ) अनुवर्त;
      यदि( pObjTab ) cnt++;
      a_मूलम् = sqlite3DbMallocRawNN(db, परिमाण(पूर्णाङ्क)*(cnt+1));
      यदि( a_मूलम्==0 ) भङ्ग;
      cnt = 0;
      यदि( pObjTab ) a_मूलम्[++cnt] = 0;
      प्रति(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        सारणी *pTab = sqlite_Hash_दत्त(x);
        सूचकांक *pIdx;
        यदि( सारणी_Skip_Integrity_Check(pTab,pObjTab) ) अनुवर्त;
        यदि( HasRowid(pTab) ) a_मूलम्[++cnt] = pTab->tnum;
        प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          a_मूलम्[++cnt] = pIdx->tnum;
        }
      }
      a_मूलम्[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_पञ्जिकाकोशः(pParse, 8+cnt);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_अस्थायी_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_IntegrityCk, 1, cnt, 8, (वर्ण*)a_मूलम्,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_परिणाम_Row(v);
      sqlite3_Vdbe_Jump_अत्र(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_आरोपय_String(v, 2, "wrong # of entries in index ");
      प्रति(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        पूर्णाङ्क iTab = 0;
        सारणी *pTab = sqlite_Hash_दत्त(x);
        सूचकांक *pIdx;
        यदि( सारणी_Skip_Integrity_Check(pTab,pObjTab) ) अनुवर्त;
        यदि( HasRowid(pTab) ){
          iTab = cnt++;
        }अन्यथा{
          iTab = cnt;
          प्रति(pIdx=pTab->p_सूचकांक; ALWAYS(pIdx); pIdx=pIdx->p_अग्र){
            यदि( Is_Primary_कुञ्चिका_सूचकांक(pIdx) ) भङ्ग;
            iTab++;
          }
        }
        प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          यदि( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_आरोपय_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_परिणाम_Row(v);
            sqlite3_Vdbe_Jump_अत्र(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      प्रति(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        सारणी *pTab = sqlite_Hash_दत्त(x);
        सूचकांक *pIdx, *pPk;
        सूचकांक *p_प्राक् = 0;      /* Previous index */
        पूर्णाङ्क loop_शिखरम्;
        पूर्णाङ्क i_दत्त_Cur, iIdxCur;
        पूर्णाङ्क r1 = -1;
        पूर्णाङ्क bStrict;            /* True for a STRICT table */
        पूर्णाङ्क r2;                 /* Previous key for WITHOUT ROWID tables */
        पूर्णाङ्क mxCol;              /* Maximum non-virtual column number */

        यदि( सारणी_Skip_Integrity_Check(pTab,pObjTab) ) अनुवर्त;
        यदि( !Is_Ordinary_सारणी(pTab) ) अनुवर्त;
        यदि( is_शीघ्र || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }अन्यथा{
          pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
          r2 = sqlite3_Get_अस्थायी_Range(pParse, pPk->n_कुञ्चिका_Col);
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 1, r2, r2+pPk->n_कुञ्चिका_Col-1);
        }
        sqlite3_Open_सारणी_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_दत्त_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, 7);
        प्रति(j=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, j++){
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        प्रतिज्ञा( pParse->nMem>=8+j );
        प्रतिज्ञा( sqlite3_No_Temps_अन्तर्_Range(pParse,1,7+j) );
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, i_दत्त_Cur, 0); VdbeCoverage(v);
        loop_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        प्रतिज्ञा( !IsVirtual(pTab) );
        यदि( HasRowid(pTab) ){
          mxCol = -1;
          प्रति(j=0; j<pTab->nCol; j++){
            यदि( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          यदि( mxCol==pTab->iPKey ) mxCol--;
        }अन्यथा{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab)->nColumn-1;
        }
        यदि( mxCol>=0 ){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, i_दत्त_Cur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        यदि( !is_शीघ्र ){
          यदि( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            पूर्णाङ्क a1;
            वर्ण *zErr;
            a1 = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IdxGT, i_दत्त_Cur, 0,r2,pPk->n_कुञ्चिका_Col);
            VdbeCoverage(v);
            sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_परिणाम_Row(v);
            sqlite3_Vdbe_Jump_अत्र(v, a1);
            sqlite3_Vdbe_Jump_अत्र(v, a1+1);
            प्रति(j=0; j<pPk->n_कुञ्चिका_Col; j++){
              sqlite3_व्यञ्जकम्_संहिता_आरोपय_सूचकांक_Column(pParse, pPk, i_दत्त_Cur, j, r2+j);
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
        प्रति(j=0; j<pTab->nCol; j++){
          वर्ण *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          पूर्णाङ्क labelError;               /* Jump here to report an error */
          पूर्णाङ्क labelOk;                  /* Jump here if all looks ok */
          पूर्णाङ्क p1, p3, p4;               /* Operands to the OP_IsType opcode */
          पूर्णाङ्क doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          यदि( j==pTab->iPKey ) अनुवर्त;
          यदि( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }अन्यथा{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          यदि( pCol->notNull==0 && !doTypeCheck ) अनुवर्त;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          यदि( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }अन्यथा{
            यदि( pCol->iDflt ){
              sqlite3_मानम् *p_Dflt_मानम् = 0;
              sqlite3_मानम्_From_व्यञ्जकम्(db, sqlite3_Column_व्यञ्जकम्_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_मानम्);
              यदि( p_Dflt_मानम् ){
                p4 = sqlite3_मानम्_type(p_Dflt_मानम्);
                sqlite3_मानम्_Free(p_Dflt_मानम्);
              }
            }
            p1 = i_दत्त_Cur;
            यदि( !HasRowid(pTab) ){
              testcase( j!=sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab, j) );
              p3 = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(sqlite3_Primary_कुञ्चिका_सूचकांक(pTab), j);
            }अन्यथा{
              p3 = sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          यदि( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            पूर्णाङ्क jmp3;
            पूर्णाङ्क jmp2 = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
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
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, p1, p3, 3);
              sqlite3ColumnDefault(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                pCol->zCnName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            यदि( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_अत्र(v, jmp2);
              sqlite3_Vdbe_Jump_अत्र(v, jmp3);
            }अन्यथा{
              /* VDBE byte code will fall thru */
            }
          }
          यदि( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            स्थिर अचिह्नित वर्ण a_Std_Type_आवरणम्[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            प्रतिज्ञा( pCol->eCType>=1 && pCol->eCType<=परिमाण(a_Std_Type_आवरणम्) );
            sqlite3VdbeChangeP5(v, a_Std_Type_आवरणम्[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा यदि( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }अन्यथा यदि( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            यदि( p1>=0 ){
              sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, i_दत्त_Cur, j, 3);
            }
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_परिणाम_Row(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        यदि( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          व्यञ्जकम्_List *pCheck = sqlite3_व्यञ्जकम्_List_Dup(db, pTab->pCheck, 0);
          यदि( db->mallocFailed==0 ){
            पूर्णाङ्क addrCkFault = sqlite3VdbeMakeLabel(pParse);
            पूर्णाङ्क addrCkOk = sqlite3VdbeMakeLabel(pParse);
            वर्ण *zErr;
            पूर्णाङ्क k;
            pParse->iSelfTab = i_दत्त_Cur + 1;
            प्रति(k=pCheck->n_व्यञ्जकम्-1; k>0; k--){
              sqlite3_व्यञ्जकम्_If_False(pParse, pCheck->a[k].p_व्यञ्जकम्, addrCkFault, 0);
            }
            sqlite3_व्यञ्जकम्_If_True(pParse, pCheck->a[0].p_व्यञ्जकम्, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_परिणाम_Row(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_व्यञ्जकम्_List_Delete(db, pCheck);
        }
        यदि( !is_शीघ्र ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          प्रति(j=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, j++){
            पूर्णाङ्क jmp2, jmp3, jmp4, jmp5, label6;
            पूर्णाङ्क kk;
            पूर्णाङ्क ckUniq = sqlite3VdbeMakeLabel(pParse);
            यदि( pPk==pIdx ) अनुवर्त;
            r1 = sqlite3_Generate_सूचकांक_कुञ्चिका(pParse, pIdx, i_दत्त_Cur, 0, 0, &jmp3,
                                         p_प्राक्, r1);
            p_प्राक् = pIdx;
            sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_IFind_कुञ्चिका, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ध्रुव वर्ण*)pIdx, P4_सूचकांक);
            sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_परिणाम_Row(v);
            sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_अत्र(v, jmp2);
            sqlite3_Vdbe_आरोपय_String(v, 3, "row ");
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_आरोपय_String(v, 4, " missing from index ");
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_आरोपय_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_परिणाम_Row(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            यदि( HasRowid(pTab) ){
              पूर्णाङ्क jmp7;
              sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Eq, 3, 0, r1+pIdx->nColumn-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_आरोपय_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_आरोपय_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_अत्र(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            प्रति(kk=0; kk<pIdx->n_कुञ्चिका_Col; kk++){
              यदि( pIdx->azColl[kk]==sqlite3_Str_द्विमान ) अनुवर्त;
              यदि( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            यदि( label6 ){
              पूर्णाङ्क jmp6 = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_आरोपय_String(v, 3, "row ");
              sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_आरोपय_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_अत्र(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            यदि( Is_Unique_सूचकांक(pIdx) ){
              पूर्णाङ्क uniqOk = sqlite3VdbeMakeLabel(pParse);
              पूर्णाङ्क jmp6;
              प्रति(kk=0; kk<pIdx->n_कुञ्चिका_Col; kk++){
                पूर्णाङ्क iCol = pIdx->aiColumn[kk];
                प्रतिज्ञा( iCol!=XN_ROWID && iCol<pTab->nCol );
                यदि( iCol>=0 && pTab->aCol[iCol].notNull ) अनुवर्त;
                sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_अग्र, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_अत्र(v, jmp6);
              sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_कुञ्चिका_Col); VdbeCoverage(v);
              sqlite3_Vdbe_आरोपय_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_अत्र(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, i_दत्त_Cur, loop_शिखरम्); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_अत्र(v, loop_शिखरम्-1);
        यदि( pPk ){
          प्रतिज्ञा( !is_शीघ्र );
          sqlite3_Release_अस्थायी_Range(pParse, r2, pPk->n_कुञ्चिका_Col);
        }
      }

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      प्रति(x=sqliteHashFirst(pTbls); x; x=sqlite_Hash_अग्र(x)){
        सारणी *pTab = sqlite_Hash_दत्त(x);
        sqlite3_vtab *pVTab;
        पूर्णाङ्क a1;
        यदि( सारणी_Skip_Integrity_Check(pTab,pObjTab) ) अनुवर्त;
        यदि( Is_Ordinary_सारणी(pTab) ) अनुवर्त;
        यदि( !IsVirtual(pTab) ) अनुवर्त;
        यदि( pTab->nCol<=0 ){
          ध्रुव वर्ण *zMod = pTab->u.vtab.azArg[0];
          यदि( sqlite3HashFind(&db->aModule, zMod)==0 ) अनुवर्त;
        }
        sqlite3_View_Get_Column_नामानि(pParse, pTab);
        यदि( pTab->u.vtab.p==0 ) अनुवर्त;
        pVTab = pTab->u.vtab.p->pVtab;
        यदि( NEVER(pVTab==0) ) अनुवर्त;
        यदि( NEVER(pVTab->pModule==0) ) अनुवर्त;
        यदि( pVTab->pModule->i_संस्करणम्<4 ) अनुवर्त;
        यदि( pVTab->pModule->xIntegrity==0 ) अनुवर्त;
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_VCheck, i, 3, is_शीघ्र);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_परिणाम_Row(v);
        sqlite3_Vdbe_Jump_अत्र(v, a1);
        अनुवर्त;
      }
#यदि_अन्तः
    }
    {
      स्थिर ध्रुव पूर्णाङ्क iLn = VDBE_विस्थापनम्_LINENO(2);
      स्थिर ध्रुव VdbeOpList अन्तः_संहिता[] = {
        { OP_सङ्कलय_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_परिणाम_Row,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_सङ्कलय_Op_List(v, Array_आकार(अन्तः_संहिता), अन्तः_संहिता, iLn);
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
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_INTEGRITY_CHECK */

#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
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
    स्थिर ध्रुव संरचना EncName {
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
    ध्रुव संरचना EncName *pEnc;
    यदि( !z_दक्षिणः ){    /* "PRAGMA encoding" */
      यदि( sqlite3ReadSchema(pParse) ) गच्छ pragma_बहिर्गमनम्;
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
        प्रति(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          यदि( 0==sqlite3StrICmp(z_दक्षिणः, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_समुच्चयः_Text_Encoding(db, enc);
            भङ्ग;
          }
        }
        यदि( !pEnc->zName ){
          sqlite3_Error_सन्देशः(pParse, "unsupported encoding: %s", z_दक्षिणः);
        }
      }
    }
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */

#यदि_अपरिभाषितम् SQLITE_OMIT_SCHEMA_संस्करणम्_PRAGMAS
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
  पक्ष Prag_Typ_शीर्षकम्_मानम्: {
    पूर्णाङ्क iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_उपयुङ्क्ते_Btree(v, iDb);
    यदि( z_दक्षिणः && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      स्थिर ध्रुव VdbeOpList समुच्चयः_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_समुच्चयः_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(समुच्चयः_Cookie));
      aOp = sqlite3_Vdbe_सङ्कलय_Op_List(v, Array_आकार(समुच्चयः_Cookie), समुच्चयः_Cookie, 0);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) भङ्ग;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_दक्षिणः);
      aOp[1].p5 = 1;
      यदि( iCookie==BTREE_SCHEMA_संस्करणम् && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }अन्यथा{
      /* Read the specified cookie value */
      स्थिर ध्रुव VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_परिणाम_Row,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_आकार(readCookie));
      aOp = sqlite3_Vdbe_सङ्कलय_Op_List(v, Array_आकार(readCookie),readCookie,0);
      यदि( ONLY_IF_REALLOC_STRESS(aOp==0) ) भङ्ग;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  पक्ष PragTyp_COMPILE_OPTIONS: {
    पूर्णाङ्क i = 0;
    ध्रुव वर्ण *zOpt;
    pParse->nMem = 1;
    यावत्( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3_Vdbe_आरोपय_String(v, 1, zOpt);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  भङ्ग;
#यदि_अन्तः /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  पक्ष PragTyp_WAL_CHECKPOINT: {
    पूर्णाङ्क iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    पूर्णाङ्क e_बहुलकम् = SQLITE_CHECKPOINT_PASSIVE;
    यदि( z_दक्षिणः ){
      यदि( sqlite3StrICmp(z_दक्षिणः, "full")==0 ){
        e_बहुलकम् = SQLITE_CHECKPOINT_पूर्णम्;
      }अन्यथा यदि( sqlite3StrICmp(z_दक्षिणः, "restart")==0 ){
        e_बहुलकम् = SQLITE_CHECKPOINT_RESTART;
      }अन्यथा यदि( sqlite3StrICmp(z_दक्षिणः, "truncate")==0 ){
        e_बहुलकम् = SQLITE_CHECKPOINT_TRUNCATE;
      }अन्यथा यदि( sqlite3StrICmp(z_दक्षिणः, "noop")==0 ){
        e_बहुलकम् = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Checkpoint, iBt, e_बहुलकम्, 1);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, 1, 3);
  }
  भङ्ग;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  पक्ष PragTyp_WAL_AUTOCHECKPOINT: {
    यदि( z_दक्षिणः ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_दक्षिणः));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_गन्तव्यम्_INT(db->pWalArg) : 0);
  }
  भङ्ग;
#यदि_अन्तः

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  पक्ष Prag_Typ_SHRINK_स्मृतिः: {
    sqlite3_db_release_स्मृतिः(db);
    भङ्ग;
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
    पूर्णाङ्क i_Db_चरमः;           /* Loop termination point for the schema loop */
    पूर्णाङ्क iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    सारणी *pTab;           /* A table in the schema */
    सूचकांक *pIdx;           /* An index of the table */
    LogEst sz_देहली;    /* Size threshold above which reanalysis needed */
    वर्ण *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_आवरणम्;            /* Mask of operations to perform */
    पूर्णाङ्क n_सीमा;            /* Analysis limit to use */
    पूर्णाङ्क nCheck = 0;        /* Number of tables to be optimized */
    पूर्णाङ्क nBtree = 0;        /* Number of btrees to scan */
    पूर्णाङ्क n_सूचकांक;            /* Number of indexes on the current table */

    यदि( z_दक्षिणः ){
      op_आवरणम् = (u32)sqlite3Atoi(z_दक्षिणः);
      यदि( (op_आवरणम् & 0x02)==0 ) भङ्ग;
    }अन्यथा{
      op_आवरणम् = 0xfffe;
    }
    यदि( (op_आवरणम् & 0x10)==0 ){
      n_सीमा = 0;
    }अन्यथा यदि( db->n_Analysis_सीमा>0
           && db->n_Analysis_सीमा<SQLITE_DEFAULT_OPTIMIZE_सीमा ){
      n_सीमा = 0;
    }अन्यथा{
      n_सीमा = SQLITE_DEFAULT_OPTIMIZE_सीमा;
    }
    iTabCur = pParse->nTab++;
    प्रति(i_Db_चरमः = zDb?iDb:db->nDb-1; iDb<=i_Db_चरमः; iDb++){
      यदि( iDb==1 ) अनुवर्त;
      sqlite3_संहिता_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      प्रति(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_अग्र(k)){
        pTab = (सारणी*)sqlite_Hash_दत्त(k);

        /* This only works for ordinary tables */
        यदि( !Is_Ordinary_सारणी(pTab) ) अनुवर्त;

        /* Do not scan system tables */
        यदि( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) अनुवर्त;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_देहली = pTab->nRowLogEst;
        n_सूचकांक = 0;
        प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          n_सूचकांक++;
          यदि( !pIdx->hasStat1 ){
            sz_देहली = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        यदि( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }अन्यथा यदि( op_आवरणम् & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }अन्यथा यदि( pTab->p_सूचकांक!=0 && sz_देहली<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }अन्यथा{
          /* Otherwise, we can skip this table */
          अनुवर्त;
        }

        nCheck++;
        यदि( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_सूचकांक+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_सारणी(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        यदि( sz_देहली>=0 ){
          ध्रुव LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_If_आकार_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_आवरणम्&1),
                         sz_देहली>=iRange ? sz_देहली-iRange : -1,
                         sz_देहली+iRange);
          VdbeCoverage(v);
        }अन्यथा{
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_आवरणम्&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        यदि( op_आवरणम् & 0x01 ){
          पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
          sqlite3_Vdbe_सङ्कलय_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, r1, 1);
        }अन्यथा{
          sqlite3_Vdbe_सङ्कलय_Op4(v, OP_SqlExec, n_सीमा ? 0x02 : 00, n_सीमा, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    यदि( !db->mallocFailed && n_सीमा>0 && nBtree>100 ){
      पूर्णाङ्क iAddr, i_अन्तः;
      VdbeOp *aOp;
      n_सीमा = 100*n_सीमा/nBtree;
      यदि( n_सीमा<100 ) n_सीमा = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_अन्तः = sqlite3VdbeCurrentAddr(v);
      प्रति(iAddr=0; iAddr<i_अन्तः; iAddr++){
        यदि( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_सीमा;
      }
    }
    भङ्ग;
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
    यदि( z_दक्षिणः && sqlite3GetInt32(z_दक्षिणः,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    भङ्ग;
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
  पक्ष Prag_Typ_SOFT_HEAP_सीमा: {
    sqlite3_int64 N;
    यदि( z_दक्षिणः && sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    भङ्ग;
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
  पक्ष Prag_Typ_HARD_HEAP_सीमा: {
    sqlite3_int64 N;
    यदि( z_दक्षिणः && sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &N)==SQLITE_OK ){
      sqlite3_int64 i_प्राक् = sqlite3_hard_heap_limit64(-1);
      यदि( N>0 && (i_प्राक्==0 || i_प्राक्>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    भङ्ग;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  पक्ष Prag_Typ_तन्तवः: {
    sqlite3_int64 N;
    यदि( z_दक्षिणः
     && sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_सीमा(db, SQLITE_सीमा_कार्यकर्ता_तन्तवः, (पूर्णाङ्क)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_सीमा(db, SQLITE_सीमा_कार्यकर्ता_तन्तवः, -1));
    भङ्ग;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  पक्ष Prag_Typ_ANALYSIS_सीमा: {
    sqlite3_int64 N;
    यदि( z_दक्षिणः
     && sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(z_दक्षिणः, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_सीमा = (पूर्णाङ्क)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_सीमा); /* IMP: R-57594-65522 */
    भङ्ग;
  }

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  पक्ष Prag_Typ_तालकम्_स्थितिः: {
    स्थिर ध्रुव वर्ण *ध्रुव az_तालकम्_Name[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    पूर्णाङ्क i;
    pParse->nMem = 2;
    प्रति(i=0; i<db->nDb; i++){
      Btree *pBt;
      ध्रुव वर्ण *zState = "unknown";
      पूर्णाङ्क j;
      यदि( db->aDb[i].zDbSName==0 ) अनुवर्त;
      pBt = db->aDb[i].pBt;
      यदि( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }अन्यथा यदि( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = az_तालकम्_Name[j];
      }
      sqlite3_Vdbe_Multi_आरोपय(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    भङ्ग;
  }
#यदि_अन्तः

#यदि defined(SQLITE_ENABLE_CEROD)
  पक्ष PragTyp_ACTIVATE_EXTENSIONS: यदि( z_दक्षिणः ){
    यदि( sqlite3StrNICmp(z_दक्षिणः, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_दक्षिणः[6]);
    }
  }
  भङ्ग;
#यदि_अन्तः

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  यदि( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_दक्षिणः ){
    sqlite3_Vdbe_Verify_No_परिणाम_Row(v);
  }

pragma_बहिर्गमनम्:
  sqlite3DbFree(db, z_वामः);
  sqlite3DbFree(db, z_दक्षिणः);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
प्रकारनिर्देश संरचना PragmaVtab PragmaVtab;
प्रकारनिर्देश संरचना PragmaVtabCursor PragmaVtabCursor;
संरचना PragmaVtab {
  sqlite3_vtab आधारः;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ध्रुव PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
संरचना PragmaVtabCursor {
  sqlite3_vtab_cursor आधारः; /* Base class.  Must be first */
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
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  ध्रुव PragmaName *pPragma = (ध्रुव PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  पूर्णाङ्क rc;
  पूर्णाङ्क i, j;
  वर्ण cSep = '(';
  StrAccum acc;
  वर्ण zBuf[200];

  अप्रयुक्तम्_PARAMETER(argc);
  अप्रयुक्तम्_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  प्रति(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
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
  प्रतिज्ञा( वाक्यदैर्घ्यम्(zBuf) < परिमाण(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  यदि( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(परिमाण(PragmaVtab));
    यदि( pTab==0 ){
      rc = SQLITE_NOMEM;
    }अन्यथा{
      स्मृतिं_पूरय(pTab, 0, परिमाण(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }अन्यथा{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  प्रत्यागम rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
स्थिर पूर्णाङ्क pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  प्रत्यागम SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
स्थिर पूर्णाङ्क pragma_Vtab_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ध्रुव संरचना sqlite3_सूचकांक_constraint *pConstraint;
  पूर्णाङ्क i, j;
  पूर्णाङ्क आलोकितम्[2];

  pIdxInfo->estimatedCost = (द्विगुण)1;
  यदि( pTab->nHidden==0 ){ प्रत्यागम SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  आलोकितम्[0] = 0;
  आलोकितम्[1] = 0;
  प्रति(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    यदि( pConstraint->iColumn < pTab->iHidden ) अनुवर्त;
    यदि( pConstraint->op!=SQLITE_सूचकांक_CONSTRAINT_EQ ) अनुवर्त;
    यदि( pConstraint->usable==0 ) प्रत्यागम SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    प्रतिज्ञा( j < 2 );
    आलोकितम्[j] = i+1;
  }
  यदि( आलोकितम्[0]==0 ){
    pIdxInfo->estimatedCost = (द्विगुण)2147483647;
    pIdxInfo->estimated_पङ्क्तिकाः = 2147483647;
    प्रत्यागम SQLITE_OK;
  }
  j = आलोकितम्[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_सूचकांक = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (द्विगुण)20;
  pIdxInfo->estimated_पङ्क्तिकाः = 20;
  यदि( आलोकितम्[1] ){
    j = आलोकितम्[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_सूचकांक = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  प्रत्यागम SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
स्थिर पूर्णाङ्क pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(परिमाण(*pCsr));
  यदि( pCsr==0 ) प्रत्यागम SQLITE_NOMEM;
  स्मृतिं_पूरय(pCsr, 0, परिमाण(PragmaVtabCursor));
  pCsr->आधारः.pVtab = pVtab;
  *ppCursor = &pCsr->आधारः;
  प्रत्यागम SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
स्थिर शून्य pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  पूर्णाङ्क i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  प्रति(i=0; i<Array_आकार(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
स्थिर पूर्णाङ्क pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  प्रत्यागम SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
स्थिर पूर्णाङ्क pragma_Vtab_अग्र(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  पूर्णाङ्क rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  प्रतिज्ञा( pCsr->pPragma );
  यदि( SQLITE_ROW!=sqlite3_चरणः(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  प्रत्यागम rc;
}

/*
** Pragma virtual table module xFilter method.
*/
स्थिर पूर्णाङ्क pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  पूर्णाङ्क idx_अङ्कः, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मानम् **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  पूर्णाङ्क rc;
  पूर्णाङ्क i, j;
  StrAccum acc;
  वर्ण *zSql;

  अप्रयुक्तम्_PARAMETER(idx_अङ्कः);
  अप्रयुक्तम्_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  प्रति(i=0; i<argc; i++, j++){
    ध्रुव वर्ण *zText = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[i]);
    प्रतिज्ञा( j<Array_आकार(pCsr->azArg) );
    प्रतिज्ञा( pCsr->azArg[j]==0 );
    यदि( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      यदि( pCsr->azArg[j]==0 ){
        प्रत्यागम SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_सीमा[SQLITE_सीमा_SQL_दीर्घता]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  यदि( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  यदि( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  यदि( zSql==0 ) प्रत्यागम SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  यदि( rc!=SQLITE_OK ){
    pTab->आधारः.z_Err_सन्देशः = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    प्रत्यागम rc;
  }
  प्रत्यागम pragma_Vtab_अग्र(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
स्थिर पूर्णाङ्क pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  प्रत्यागम (pCsr->pPragma==0);
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
    sqlite3_परिणाम_मानम्(ctx, sqlite3_column_मानम्(pCsr->pPragma, i));
  }अन्यथा{
    sqlite3_परिणाम_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
स्थिर पूर्णाङ्क pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  प्रत्यागम SQLITE_OK;
}

/* The pragma virtual table object */
स्थिर ध्रुव sqlite3_module pragmaVtabModule = {
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
Module *sqlite3_Pragma_Vtab_पञ्जिकाकोशः(sqlite3 *db, ध्रुव वर्ण *zName){
  ध्रुव PragmaName *pName;
  प्रतिज्ञा( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  यदि( pName==0 ) प्रत्यागम 0;
  यदि( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) प्रत्यागम 0;
  प्रतिज्ञा( sqlite3HashFind(&db->aModule, zName)==0 );
  प्रत्यागम sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (शून्य*)pName, 0);
}

#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

#यदि_अन्तः /* SQLITE_OMIT_PRAGMA */
