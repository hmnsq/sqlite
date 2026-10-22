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
#ضمن "sqliteInt.h"

#إذا !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  إذا defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  وإلا
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#جيڪڏهن_پورو

/***************************************************************************
** The "pragma.h" include file is an automatically generated file that
** that includes the PragType_XXXX macro definitions and the aPragmaName[]
** object.  This ensures that the aPragmaName[] table is arranged in
** lexicographical order to facility a binary search of the pragma name.
** Do not edit pragma.h directly.  Edit and rerun the script in at
** ../tool/mkpragmatab.tcl. */
#ضمن "pragma.h"

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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DEFAULT_OPTIMIZE_حد_بندي
# define SQLITE_DEFAULT_OPTIMIZE_حد_بندي 2000
#جيڪڏهن_پورو

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
ساكن u8 getSafetyLevel(ثابت حرف *z, صحيح omit_ڀريل, u8 dflt){
                             /* 123456789 123456789 123 */
  ساكن ثابت حرف z_متن[] = "onoffalseyestruextrafull";
  ساكن ثابت u8 i_وٿي[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ساكن ثابت u8 i_ڊيگهائي[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ساكن ثابت u8 i_قدر[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  صحيح i, n;
  إذا( sqlite3Isdigit(*z) ){
    أرجع sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  لكل(i=0; i<Array_سائيز(i_ڊيگهائي); i++){
    إذا( i_ڊيگهائي[i]==n && sqlite3StrNICmp(&z_متن[i_وٿي[i]],z,n)==0
     && (!omit_ڀريل || i_قدر[i]<=1)
    ){
      أرجع i_قدر[i];
    }
  }
  أرجع dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ثابت حرف *z, u8 dflt){
  أرجع getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#إذا !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
ساكن صحيح get_Locking_گهڻائي(ثابت حرف *z){
  إذا( z ){
    إذا( 0==sqlite3StrICmp(z, "exclusive") ) أرجع PAGER_LOCKINGMODE_EXCLUSIVE;
    إذا( 0==sqlite3StrICmp(z, "normal") ) أرجع PAGER_LOCKINGMODE_عام;
  }
  أرجع PAGER_LOCKINGMODE_QUERY;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ساكن صحيح getAutoVacuum(ثابت حرف *z){
  صحيح i;
  إذا( 0==sqlite3StrICmp(z, "none") ) أرجع BTREE_AUTOVACUUM_NONE;
  إذا( 0==sqlite3StrICmp(z, "full") ) أرجع BTREE_AUTOVACUUM_ڀريل;
  إذا( 0==sqlite3StrICmp(z, "incremental") ) أرجع BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  أرجع (u8)((i>=0&&i<=2)?i:0);
}
#جيڪڏهن_پورو /* ifndef SQLITE_OMIT_AUTOVACUUM */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ساكن صحيح get_عارضي_Store(ثابت حرف *z){
  إذا( z[0]>='0' && z[0]<='2' ){
    أرجع z[0] - '0';
  }وإلا إذا( sqlite3StrICmp(z, "file")==0 ){
    أرجع 1;
  }وإلا إذا( sqlite3StrICmp(z, "memory")==0 ){
    أرجع 2;
  }وإلا{
    أرجع 0;
  }
}
#جيڪڏهن_پورو /* SQLITE_PAGER_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ساكن صحيح invalidate_عارضي_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  إذا( db->aDb[1].pBt!=0 ){
    إذا( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_نياپو(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      أرجع SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_جو_Connection(db);
  }
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_PAGER_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ساكن صحيح change_عارضي_Storage(Parse *pParse, ثابت حرف *zStorageType){
  صحيح ts = get_عارضي_Store(zStorageType);
  sqlite3 *db = pParse->db;
  إذا( db->عارضي_store==ts ) أرجع SQLITE_OK;
  إذا( invalidate_عارضي_Storage( pParse ) != SQLITE_OK ){
    أرجع SQLITE_ERROR;
  }
  db->عارضي_store = (u8)ts;
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ساكن فراغ سيٽ_Pragma_نتيجو_ڪالم_نالا(
  Vdbe *v,                     /* The query under construction */
  ثابت PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_سيٽ_انگ_کانن_ڳڻپ(v, n==0 ? 1 : n);
  إذا( n==0 ){
    sqlite3_Vdbe_سيٽ_کانو_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }وإلا{
    صحيح i, j;
    لكل(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ساكن فراغ returnSingleInt(Vdbe *v, i64 قدر){
  sqlite3_Vdbe_جوڙيو_Int64(v, 1, قدر);
  sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ساكن فراغ return_Single_متن(
  Vdbe *v,                /* Prepared statement under construction */
  ثابت حرف *z_قدر      /* Value to be returned */
){
  إذا( z_قدر ){
    sqlite3_Vdbe_ڀريو_String(v, 1, (ثابت حرف*)z_قدر);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_PAGER_PRAGMAS
ساكن فراغ سيٽ_All_Pager_Flags(sqlite3 *db){
  إذا( db->autoCommit ){
    Db *pDb = db->aDb;
    صحيح n = db->nDb;
    أكد( SQLITE_ڀريل_FSync==PAGER_FULLFSYNC );
    أكد( SQLITE_Ckpt_ڀريل_FSync==PAGER_CKPT_FULLFSYNC );
    أكد( SQLITE_CacheSpill==PAGER_CACHESPILL );
    أكد( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_نقاب );
    أكد( (pDb->safety_level & PAGER_SYNCHRONOUS_نقاب)==pDb->safety_level );
    طالما( (n--) > 0 ){
      إذا( pDb->pBt ){
        sqlite3_Btree_سيٽ_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_نقاب) );
      }
      pDb++;
    }
  }
}
#وإلا
# define سيٽ_All_Pager_Flags(X)  /* no-op */
#جيڪڏهن_پورو


/*
** Return a human-readable name for a constraint resolution action.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FOREIGN_ڪنجي
ساكن ثابت حرف *actionName(u8 action){
  ثابت حرف *zName;
  بدل( action ){
    حالة OE_سيٽ_Null:  zName = "SET NULL";        اكسر;
    حالة OE_سيٽ_Dflt:  zName = "SET DEFAULT";     اكسر;
    حالة OE_Cascade:  zName = "CASCADE";         اكسر;
    حالة OE_Restrict: zName = "RESTRICT";        اكسر;
    افتراضي:          zName = "NO ACTION"; 
                      أكد( action==OE_None ); اكسر;
  }
  أرجع zName;
}
#جيڪڏهن_پورو


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ثابت حرف *sqlite3JournalModename(صحيح e_گهڻائي){
  ساكن حرف * ثابت az_گهڻائي_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
     , "wal"
#جيڪڏهن_پورو
  };
  أكد( PAGER_JOURNALMODE_DELETE==0 );
  أكد( PAGER_JOURNALMODE_PERSIST==1 );
  أكد( PAGER_JOURNALMODE_OFF==2 );
  أكد( PAGER_JOURNALMODE_TRUNCATE==3 );
  أكد( PAGER_JOURNALMODE_يادگيري==4 );
  أكد( PAGER_JOURNALMODE_WAL==5 );
  أكد( e_گهڻائي>=0 && e_گهڻائي<=Array_سائيز(az_گهڻائي_Name) );

  إذا( e_گهڻائي==Array_سائيز(az_گهڻائي_Name) ) أرجع 0;
  أرجع az_گهڻائي_Name[e_گهڻائي];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ساكن ثابت PragmaName *pragmaLocate(ثابت حرف *zName){
  صحيح upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_سائيز(aPragmaName)-1;
  طالما( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    إذا( rc==0 ) اكسر;
    إذا( rc<0 ){
      upr = mid - 1;
    }وإلا{
      lwr = mid + 1;
    }
  }
  أرجع lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
ساكن فراغ pragma_Funclist_سٽ(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  صحيح isBuiltin,         /* True if this is a built-in function */
  صحيح showInternFuncs    /* True if showing internal functions */
){
  u32 نقاب =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_اندروني
  ;
  إذا( showInternFuncs ) نقاب = 0xffffffff;
  لكل(; p; p=p->p_ايندڙ){
    ثابت حرف *zType;
    ساكن ثابت حرف *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    أكد( SQLITE_FUNC_ENCMASK==0x3 );
    أكد( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    إذا( p->xSFunc==0 ) استمر;
    إذا( (p->funcFlags & SQLITE_FUNC_اندروني)!=0
     && showInternFuncs==0
    ){
      استمر;
    }   
    إذا( p->x_قدر!=0 ){
      zType = "w";
    }وإلا إذا( p->xFinalize!=0 ){
      zType = "a";
    }وإلا{
      zType = "s";
    }
    sqlite3_Vdbe_Multi_ڀريو(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & نقاب) ^ SQLITE_INNOCUOUS
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
ساكن صحيح integrity_Check_نتيجو_صف(Vdbe *v){
  صحيح addr;
  sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 3, 1);
  addr = sqlite3_Vdbe_جوڙيو_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_جوڙيو_Op0(v, OP_Halt);
  أرجع addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
ساكن صحيح جدول_Skip_Integrity_Check(ثابت جدول *pTab, ثابت جدول *pObjTab){
  إذا( pObjTab ){
    أرجع pTab!=pObjTab;
  }وإلا{
    أرجع (pTab->tabFlags & TF_Imposter)!=0;
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
فراغ sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_قدر,      /* Token for <value>, or NULL */
  صحيح minusFlag       /* True if a '-' sign preceded <value> */
){
  حرف *z_کاٻو = 0;       /* Nul-terminated UTF-8 string <id> */
  حرف *z_ساڄو = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ثابت حرف *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  حرف *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  صحيح iDb;               /* Database index for <database> */
  صحيح rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ثابت PragmaName *pPragma;   /* The pragma */

  إذا( v==0 ) أرجع;
  sqlite3_Vdbe_لاڳيتو_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  إذا( iDb<0 ) أرجع;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  إذا( iDb==1 && sqlite3_Open_عارضي_Database(pParse) ){
    أرجع;
  }

  z_کاٻو = sqlite3NameFromToken(db, pId);
  إذا( !z_کاٻو ) أرجع;
  إذا( minusFlag ){
    z_ساڄو = sqlite3MPrintf(db, "-%T", p_قدر);
  }وإلا{
    z_ساڄو = sqlite3NameFromToken(db, p_قدر);
  }

  أكد( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  إذا( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_کاٻو, z_ساڄو, zDb) ){
    اذهب pragma_ٻاهريون;
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
  aFcntl[1] = z_کاٻو;
  aFcntl[2] = z_ساڄو;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (فراغ*)aFcntl);
  إذا( rc==SQLITE_OK ){
    sqlite3_Vdbe_سيٽ_انگ_کانن_ڳڻپ(v, 1);
    sqlite3_Vdbe_سيٽ_کانو_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    return_Single_متن(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    اذهب pragma_ٻاهريون;
  }
  إذا( rc!=SQLITE_NOTFOUND ){
    إذا( aFcntl[0] ){
      sqlite3_Error_نياپو(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    اذهب pragma_ٻاهريون;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_کاٻو);
  إذا( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    اذهب pragma_ٻاهريون;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  إذا( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    إذا( sqlite3ReadSchema(pParse) ) اذهب pragma_ٻاهريون;
  }

  /* Register the result column names for pragmas that return results */
  إذا( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_ساڄو==0)
  ){
    سيٽ_Pragma_نتيجو_ڪالم_نالا(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  بدل( pPragma->ePragTyp ){
 
#إذا !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  حالة Prag_Typ_DEFAULT_CACHE_سائيز: {
    ساكن ثابت صحيح iLn = VDBE_وٿي_LINENO(2);
    ساكن ثابت VdbeOpList get_Cache_سائيز[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_سائيز},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_ڪٽيو,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_نتيجو_صف,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_استعمال_ڪري_Btree(v, iDb);
    إذا( !z_ساڄو ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_سائيز(get_Cache_سائيز));
      aOp = sqlite3_Vdbe_جوڙيو_Op_List(v, Array_سائيز(get_Cache_سائيز), get_Cache_سائيز, iLn);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_سائيز;
    }وإلا{
      صحيح سائيز = sqlite3AbsInt32(sqlite3Atoi(z_ساڄو));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_سيٽ_Cookie, iDb, BTREE_DEFAULT_CACHE_سائيز, سائيز);
      أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_سائيز = سائيز;
      sqlite3_Btree_سيٽ_Cache_سائيز(pDb->pBt, pDb->pSchema->cache_سائيز);
    }
    اكسر;
  }
#جيڪڏهن_پورو /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#إذا !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  حالة Prag_Typ_صفحو_سائيز: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    إذا( !z_ساڄو ){
      صحيح سائيز = ALWAYS(pBt) ? sqlite3_Btree_Get_صفحو_سائيز(pBt) : 0;
      returnSingleInt(v, سائيز);
    }وإلا{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->ايندڙ_Pagesize = sqlite3Atoi(z_ساڄو);
      إذا( SQLITE_NOMEM==sqlite3_Btree_سيٽ_صفحو_سائيز(pBt, db->ايندڙ_Pagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    اكسر;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  حالة PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    صحيح b = -1;
    أكد( pBt!=0 );
    إذا( z_ساڄو ){
      إذا( sqlite3_stricmp(z_ساڄو, "fast")==0 ){
        b = 2;
      }وإلا{
        b = sqlite3GetBoolean(z_ساڄو, 0);
      }
    }
    إذا( pId2->n==0 && b>=0 ){
      صحيح ii;
      لكل(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    اكسر;
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
  حالة Prag_Typ_صفحو_ڳڻپ: {
    صحيح iReg;
    i64 x = 0;
    sqlite3_ڪوڊ_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    إذا( sqlite3Tolower(z_کاٻو[0])=='p' ){
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Pagecount, iDb, iReg);
    }وإلا{
      إذا( z_ساڄو && sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو,&x)==0 ){
        إذا( x<0 ) x = 0;
        وإلا إذا( x>0xfffffffe ) x = 0xfffffffe;
      }وإلا{
        x = 0;
      }
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_MaxPgcnt, iDb, iReg, (صحيح)x);
    }
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, iReg, 1);
    اكسر;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  حالة Prag_Typ_LOCKING_گهڻائي: {
    ثابت حرف *zRet = "normal";
    صحيح e_گهڻائي = get_Locking_گهڻائي(z_ساڄو);

    إذا( pId2->n==0 && e_گهڻائي==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_گهڻائي = db->dflt_تالو_گهڻائي;
    }وإلا{
      Pager *pPager;
      إذا( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        صحيح ii;
        أكد(pDb==&db->aDb[0]);
        لكل(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3_Pager_Locking_گهڻائي(pPager, e_گهڻائي);
        }
        db->dflt_تالو_گهڻائي = (u8)e_گهڻائي;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_گهڻائي = sqlite3_Pager_Locking_گهڻائي(pPager, e_گهڻائي);
    }

    أكد( e_گهڻائي==PAGER_LOCKINGMODE_عام
            || e_گهڻائي==PAGER_LOCKINGMODE_EXCLUSIVE );
    إذا( e_گهڻائي==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    return_Single_متن(v, zRet);
    اكسر;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  حالة Prag_Typ_JOURNAL_گهڻائي: {
    صحيح e_گهڻائي;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    صحيح ii;           /* Loop counter */

    إذا( z_ساڄو==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_گهڻائي = PAGER_JOURNALMODE_QUERY;
    }وإلا{
      ثابت حرف *z_گهڻائي;
      صحيح n = sqlite3Strlen30(z_ساڄو);
      لكل(e_گهڻائي=0; (z_گهڻائي = sqlite3JournalModename(e_گهڻائي))!=0; e_گهڻائي++){
        إذا( sqlite3StrNICmp(z_ساڄو, z_گهڻائي, n)==0 ) اكسر;
      }
      إذا( !z_گهڻائي ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_گهڻائي = PAGER_JOURNALMODE_QUERY;
      }
      إذا( e_گهڻائي==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_گهڻائي = PAGER_JOURNALMODE_QUERY;
      }
    }
    إذا( e_گهڻائي==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    لكل(ii=db->nDb-1; ii>=0; ii--){
      إذا( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_استعمال_ڪري_Btree(v, ii);
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_Journal_گهڻائي, ii, 1, e_گهڻائي);
      }
    }
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 1);
    اكسر;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  حالة Prag_Typ_JOURNAL_سائيز_حد_بندي: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_حد_بندي = -2;
    إذا( z_ساڄو ){
      sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &i_حد_بندي);
      إذا( i_حد_بندي<-1 ) i_حد_بندي = -1;
    }
    i_حد_بندي = sqlite3_Pager_Journal_سائيز_حد_بندي(pPager, i_حد_بندي);
    returnSingleInt(v, i_حد_بندي);
    اكسر;
  }

#جيڪڏهن_پورو /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOVACUUM
  حالة PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    إذا( !z_ساڄو ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }وإلا{
      صحيح eAuto = getAutoVacuum(z_ساڄو);
      أكد( eAuto>=0 && eAuto<=2 );
      db->ايندڙ_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_سيٽ_Auto_Vacuum(pBt, eAuto);
      إذا( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ساكن ثابت صحيح iLn = VDBE_وٿي_LINENO(2);
        ساكن ثابت VdbeOpList سيٽ_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_سڀ_کان_وڏو_پاڙ_صفحو},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_سيٽ_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        صحيح iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_سائيز(سيٽ_Meta6));
        aOp = sqlite3_Vdbe_جوڙيو_Op_List(v, Array_سائيز(سيٽ_Meta6), سيٽ_Meta6, iLn);
        إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_استعمال_ڪري_Btree(v, iDb);
      }
    }
    اكسر;
  }
#جيڪڏهن_پورو

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOVACUUM
  حالة PragTyp_INCREMENTAL_VACUUM: {
    صحيح i_حد_بندي = 0, addr;
    إذا( z_ساڄو==0 || !sqlite3GetInt32(z_ساڄو, &i_حد_بندي) || i_حد_بندي<=0 ){
      i_حد_بندي = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, i_حد_بندي, 1);
    addr = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_جوڙيو_Op1(v, OP_نتيجو_صف, 1);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_جوڙيو_Imm, 1, -1);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_هتي(v, addr);
    اكسر;
  }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_PAGER_PRAGMAS
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
  حالة Prag_Typ_CACHE_سائيز: {
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( !z_ساڄو ){
      returnSingleInt(v, pDb->pSchema->cache_سائيز);
    }وإلا{
      صحيح سائيز = sqlite3Atoi(z_ساڄو);
      pDb->pSchema->cache_سائيز = سائيز;
      sqlite3_Btree_سيٽ_Cache_سائيز(pDb->pBt, pDb->pSchema->cache_سائيز);
    }
    اكسر;
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
  حالة PragTyp_CACHE_SPILL: {
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( !z_ساڄو ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_سيٽ_Spill_سائيز(pDb->pBt,0));
    }وإلا{
      صحيح سائيز = 1;
      إذا( sqlite3GetInt32(z_ساڄو, &سائيز) ){
        sqlite3_Btree_سيٽ_Spill_سائيز(pDb->pBt, سائيز);
      }
      إذا( sqlite3GetBoolean(z_ساڄو, سائيز!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }وإلا{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      سيٽ_All_Pager_Flags(db);
    }
    اكسر;
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
  حالة Prag_Typ_MMAP_سائيز: {
    sqlite3_int64 sz;
#إذا SQLITE_MAX_MMAP_سائيز>0
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( z_ساڄو ){
      صحيح ii;
      sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &sz);
      إذا( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      إذا( pId2->n==0 ) db->szMmap = sz;
      لكل(ii=db->nDb-1; ii>=0; ii--){
        إذا( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_سيٽ_Mmap_حد_بندي(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_سائيز, &sz);
#وإلا
    sz = 0;
    rc = SQLITE_OK;
#جيڪڏهن_پورو
    إذا( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }وإلا إذا( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    اكسر;
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
  حالة Prag_Typ_عارضي_STORE: {
    إذا( !z_ساڄو ){
      returnSingleInt(v, db->عارضي_store);
    }وإلا{
      change_عارضي_Storage(pParse, z_ساڄو);
    }
    اكسر;
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
  حالة Prag_Typ_عارضي_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    إذا( !z_ساڄو ){
      return_Single_متن(v, sqlite3_عارضي_directory);
    }وإلا{
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WSD
      إذا( z_ساڄو[0] ){
        صحيح res;
        rc = sqlite3OsAccess(db->pVfs, z_ساڄو, SQLITE_ACCESS_READWRITE, &res);
        إذا( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_نياپو(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          اذهب pragma_ٻاهريون;
        }
      }
      إذا( SQLITE_عارضي_STORE==0
       || (SQLITE_عارضي_STORE==1 && db->عارضي_store<=1)
       || (SQLITE_عارضي_STORE==2 && db->عارضي_store==1)
      ){
        invalidate_عارضي_Storage(pParse);
      }
      sqlite3_free(sqlite3_عارضي_directory);
      إذا( z_ساڄو[0] ){
        sqlite3_عارضي_directory = sqlite3_mprintf("%s", z_ساڄو);
      }وإلا{
        sqlite3_عارضي_directory = 0;
      }
#جيڪڏهن_پورو /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    اكسر;
  }

#إذا SQLITE_OS_WIN
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
  حالة Prag_Typ_معلومات_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    إذا( !z_ساڄو ){
      return_Single_متن(v, sqlite3_معلومات_directory);
    }وإلا{
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WSD
      إذا( z_ساڄو[0] ){
        صحيح res;
        rc = sqlite3OsAccess(db->pVfs, z_ساڄو, SQLITE_ACCESS_READWRITE, &res);
        إذا( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_نياپو(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          اذهب pragma_ٻاهريون;
        }
      }
      sqlite3_free(sqlite3_معلومات_directory);
      إذا( z_ساڄو[0] ){
        sqlite3_معلومات_directory = sqlite3_mprintf("%s", z_ساڄو);
      }وإلا{
        sqlite3_معلومات_directory = 0;
      }
#جيڪڏهن_پورو /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    اكسر;
  }
#جيڪڏهن_پورو

#إذا SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  حالة Prag_Typ_تالو_PROXY_FILE: {
    إذا( !z_ساڄو ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      حرف *proxy_file_رستو = عدم;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_رستو);
      return_Single_متن(v, proxy_file_رستو);
    }وإلا{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      صحيح res;
      إذا( z_ساڄو[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_سيٽ_LOCKPROXYFILE,
                                     z_ساڄو);
      } وإلا {
        res=sqlite3OsFileControl(pFile, SQLITE_سيٽ_LOCKPROXYFILE,
                                     عدم);
      }
      إذا( res!=SQLITE_OK ){
        sqlite3_Error_نياپو(pParse, "failed to set lock proxy file");
        اذهب pragma_ٻاهريون;
      }
    }
    اكسر;
  }
#جيڪڏهن_پورو /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  حالة PragTyp_SYNCHRONOUS: {
    إذا( !z_ساڄو ){
      returnSingleInt(v, pDb->safety_level-1);
    }وإلا{
      إذا( !db->autoCommit ){
        sqlite3_Error_نياپو(pParse,
            "Safety level may not be changed inside a transaction");
      }وإلا إذا( iDb!=1 ){
        صحيح iLevel;
        إذا( sqlite3Isdigit(z_ساڄو[0]) ){
          iLevel = sqlite3Atoi(z_ساڄو);
        }وإلا{
          iLevel = getSafetyLevel(z_ساڄو,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_نقاب;
        إذا( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_سيٽ = 1;
        سيٽ_All_Pager_Flags(db);
      }
    }
    اكسر;
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_PAGER_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FLAG_PRAGMAS
  حالة PragTyp_FLAG: {
    إذا( z_ساڄو==0 ){
      سيٽ_Pragma_نتيجو_ڪالم_نالا(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }وإلا{
      u64 نقاب = pPragma->iArg;    /* Mask of bits to set or clear. */
      إذا( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        نقاب &= ~(SQLITE_Foreign_ڪنجيون);
      }

      إذا( sqlite3GetBoolean(z_ساڄو, 0) ){
        إذا( (نقاب & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= نقاب;
        }
      }وإلا{
        db->flags &= ~نقاب;
        إذا( نقاب==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        إذا( (نقاب & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_ساڄو, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_جو_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_جوڙيو_Op0(v, OP_Expire);
      سيٽ_All_Pager_Flags(db);
    }
    اكسر;
  }
#جيڪڏهن_پورو /* SQLITE_OMIT_FLAG_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SCHEMA_PRAGMAS
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
  حالة Prag_Typ_جدول_INFO: إذا( z_ساڄو ){
    جدول *pTab;
    sqlite3_ڪوڊ_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_جدول(pParse, LOCATE_NOERR, z_ساڄو, zDb);
    إذا( pTab ){
      صحيح i, k;
      صحيح nHidden = 0;
      ڪالم *p_کانو;
      اشاريو *pPk = sqlite3_Primary_ڪنجي_اشاريو(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_ڪالم_نالا(pParse, pTab);
      لكل(i=0, p_کانو=pTab->a_کانو; i<pTab->n_کانو; i++, p_کانو++){
        صحيح isHidden = 0;
        ثابت اظهار *p_کانو_اظهار;
        إذا( p_کانو->کانو_Flags & COLFLAG_NOINSERT ){
          إذا( pPragma->iArg==0 ){
            nHidden++;
            استمر;
          }
          إذا( p_کانو->کانو_Flags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }وإلا إذا( p_کانو->کانو_Flags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }وإلا{ أكد( p_کانو->کانو_Flags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        إذا( (p_کانو->کانو_Flags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }وإلا إذا( pPk==0 ){
          k = 1;
        }وإلا{
          لكل(k=1; k<=pTab->n_کانو && pPk->ai_ڪالم[k-1]!=i; k++){}
        }
        p_کانو_اظهار = sqlite3_ڪالم_اظهار(pTab,p_کانو);
        أكد( p_کانو_اظهار==0 || p_کانو_اظهار->op==TK_وسعت || isHidden>=2 );
        أكد( p_کانو_اظهار==0 || !اظهار_Has_Property(p_کانو_اظهار, EP_Int_قدر)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_ڀريو(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               p_کانو->zCnName,
               sqlite3_ڪالم_Type(p_کانو,""),
               p_کانو->notNull ? 1 : 0,
               (isHidden>=2 || p_کانو_اظهار==0) ? 0 : p_کانو_اظهار->u.zToken,
               k,
               isHidden);
      }
    }
  }
  اكسر;

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
  حالة Prag_Typ_جدول_LIST: {
    صحيح ii;
    pParse->nMem = 6;
    sqlite3_ڪوڊ_Verify_Named_Schema(pParse, zDb);
    لكل(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      صحيح initNCol;
      إذا( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) استمر;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_ڳڻپ(pHash);
      طالما( initNCol-- ){
        لكل(k=sqlite_Hash_پهريون(pHash); 1; k=sqlite_Hash_ايندڙ(k) ){
          جدول *pTab;
          إذا( k==0 ){ initNCol = 0; اكسر; }
          pTab = sqlite_Hash_معلومات(k);
          إذا( pTab->n_کانو==0 ){
            حرف *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            إذا( zSql ){
              sqlite3_stmt *pDummy = 0;
              (فراغ)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (فراغ)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            إذا( db->mallocFailed ){
              sqlite3_Error_نياپو(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            اكسر;
          }
        }
      }

      لكل(k=sqlite_Hash_پهريون(pHash); k; k=sqlite_Hash_ايندڙ(k) ){
        جدول *pTab = sqlite_Hash_معلومات(k);
        ثابت حرف *zType;
        إذا( z_ساڄو && sqlite3_stricmp(z_ساڄو, pTab->zName)!=0 ) استمر;
        إذا( IsView(pTab) ){
          zType = "view";
        }وإلا إذا( IsVirtual(pTab) ){
          zType = "virtual";
        }وإلا إذا( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }وإلا{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_ڀريو(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_جدول_Name(pTab->zName),
           zType,
           pTab->n_کانو,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  اكسر;

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  حالة PragTyp_STATS: {
    اشاريو *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_ڪوڊ_Verify_Schema(pParse, iDb);
    لكل(i=sqlite_Hash_پهريون(&pDb->pSchema->tblHash); i; i=sqlite_Hash_ايندڙ(i)){
      جدول *pTab = sqlite_Hash_معلومات(i);
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "ssiii",
           sqlite3_Preferred_جدول_Name(pTab->zName),
           0,
           pTab->sz_Tab_صف,
           pTab->n_صف_Log_Est,
           pTab->tabFlags);
      لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
        sqlite3_Vdbe_Multi_ڀريو(v, 2, "siiiX",
           pIdx->zName,
           pIdx->sz_Idx_صف,
           pIdx->ai_صف_Log_Est[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 5);
      }
    }
  }
  اكسر;
#جيڪڏهن_پورو

  حالة Prag_Typ_اشاريو_INFO: إذا( z_ساڄو ){
    اشاريو *pIdx;
    جدول *pTab;
    pIdx = sqlite3_ڳوليو_اشاريو(db, z_ساڄو, zDb);
    إذا( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_جدول(pParse, LOCATE_NOERR, z_ساڄو, zDb);
      إذا( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_ڪنجي_اشاريو(pTab);
      }
    }
    إذا( pIdx ){
      صحيح iIdxDb = sqlite3_Schema_منزل_اشاريو(db, pIdx->pSchema);
      صحيح i;
      صحيح mx;
      إذا( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->n_ڪالم;
        pParse->nMem = 6;
      }وإلا{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_ڪنجي_کانو;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_جدول;
      sqlite3_ڪوڊ_Verify_Schema(pParse, iIdxDb);
      أكد( pParse->nMem<=pPragma->nPragCName );
      لكل(i=0; i<mx; i++){
        i16 cnum = pIdx->ai_ڪالم[i];
        sqlite3_Vdbe_Multi_ڀريو(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->a_کانو[cnum].zCnName);
        إذا( pPragma->iArg ){
          sqlite3_Vdbe_Multi_ڀريو(v, 4, "isiX",
            pIdx->a_ترتيب_ڏيو_ترتيب[i],
            pIdx->azColl[i],
            i<pIdx->n_ڪنجي_کانو);
        }
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, pParse->nMem);
      }
    }
  }
  اكسر;

  حالة Prag_Typ_اشاريو_LIST: إذا( z_ساڄو ){
    اشاريو *pIdx;
    جدول *pTab;
    صحيح i;
    pTab = sqlite3_ڳوليو_جدول(db, z_ساڄو, zDb);
    إذا( pTab ){
      صحيح iTabDb = sqlite3_Schema_منزل_اشاريو(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_ڪوڊ_Verify_Schema(pParse, iTabDb);
      لكل(pIdx=pTab->p_اشاريو, i=0; pIdx; pIdx=pIdx->p_ايندڙ, i++){
        ثابت حرف *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_ڀريو(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_اشاريو(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  اكسر;

  حالة PragTyp_DATABASE_LIST: {
    صحيح i;
    pParse->nMem = 3;
    لكل(i=0; i<db->nDb; i++){
      إذا( db->aDb[i].pBt==0 ) استمر;
      أكد( db->aDb[i].zDbSName!=0 );
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  اكسر;

  حالة PragTyp_COLLATION_LIST: {
    صحيح i = 0;
    HashElem *p;
    pParse->nMem = 2;
    لكل(p=sqlite_Hash_پهريون(&db->aCollSeq); p; p=sqlite_Hash_ايندڙ(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_معلومات(p);
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "is", i++, pColl->zName);
    }
  }
  اكسر;

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_INTROSPECTION_PRAGMAS
  حالة PragTyp_FUNCTION_LIST: {
    صحيح i;
    HashElem *j;
    FuncDef *p;
    صحيح showInternFunc = (db->mDbFlags & DBFLAG_اندروني_Func)!=0;
    pParse->nMem = 6;
    لكل(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      لكل(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_سٽ(v, p, 1, showInternFunc);
      }
    }
    لكل(j=sqlite_Hash_پهريون(&db->aFunc); j; j=sqlite_Hash_ايندڙ(j)){
      p = (FuncDef*)sqlite_Hash_معلومات(j);
      أكد( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_سٽ(v, p, 0, showInternFunc);
    }
  }
  اكسر;

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
  حالة PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    لكل(j=sqlite_Hash_پهريون(&db->aModule); j; j=sqlite_Hash_ايندڙ(j)){
      Module *pMod = (Module*)sqlite_Hash_معلومات(j);
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "s", pMod->zName);
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_VIRTUALTABLE */

  حالة PragTyp_PRAGMA_LIST: {
    صحيح i;
    لكل(i=0; i<Array_سائيز(aPragmaName); i++){
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "s", aPragmaName[i].zName);
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_INTROSPECTION_PRAGMAS */

#جيڪڏهن_پورو /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FOREIGN_ڪنجي
  حالة Prag_Typ_FOREIGN_ڪنجي_LIST: إذا( z_ساڄو ){
    FKey *pFK;
    جدول *pTab;
    pTab = sqlite3_ڳوليو_جدول(db, z_ساڄو, zDb);
    إذا( pTab && Is_Ordinary_جدول(pTab) ){
      pFK = pTab->u.tab.pFKey;
      إذا( pFK ){
        صحيح iTabDb = sqlite3_Schema_منزل_اشاريو(db, pTab->pSchema);
        صحيح i = 0;
        pParse->nMem = 8;
        sqlite3_ڪوڊ_Verify_Schema(pParse, iTabDb);
        طالما(pFK){
          صحيح j;
          لكل(j=0; j<pFK->n_کانو; j++){
            sqlite3_Vdbe_Multi_ڀريو(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_منزل,
                   pTab->a_کانو[pFK->a_کانو[j].iFrom].zCnName,
                   pFK->a_کانو[j].z_کانو,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->p_ايندڙ_From;
        }
      }
    }
  }
  اكسر;
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_FOREIGN_ڪنجي
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TRIGGER
  حالة Prag_Typ_FOREIGN_ڪنجي_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    جدول *pTab;           /* Child table contain "REFERENCES" keyword */
    جدول *p_والد;        /* Parent table that child points to */
    اشاريو *pIdx;           /* Index in the parent table */
    صحيح i;                 /* Loop counter:  Foreign key number for pTab */
    صحيح j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    صحيح x;                 /* result variable */
    صحيح reg_نتيجو;         /* 3 registers to hold a result row */
    صحيح reg_صف;            /* Registers to hold a row from pTab */
    صحيح addr_چوٽي;           /* Top of a loop checking foreign keys */
    صحيح addrOk;            /* Jump here if the key is OK */
    صحيح *ai_کانن_ڳڻپ;           /* child to parent column mapping */

    reg_نتيجو = pParse->nMem+1;
    pParse->nMem += 4;
    reg_صف = ++pParse->nMem;
    k = sqlite_Hash_پهريون(&db->aDb[iDb].pSchema->tblHash);
    طالما( k ){
      إذا( z_ساڄو ){
        pTab = sqlite3_Locate_جدول(pParse, 0, z_ساڄو, zDb);
        k = 0;
      }وإلا{
        pTab = (جدول*)sqlite_Hash_معلومات(k);
        k = sqlite_Hash_ايندڙ(k);
      }
      إذا( pTab==0 || !Is_Ordinary_جدول(pTab) || pTab->u.tab.pFKey==0 ) استمر;
      iDb = sqlite3_Schema_منزل_اشاريو(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_ڪوڊ_Verify_Schema(pParse, iDb);
      sqlite3_جدول_تالو(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_رجسٽر(pParse, pTab->n_کانو+reg_صف);
      sqlite3_Open_جدول(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_ڀريو_String(v, reg_نتيجو, pTab->zName);
      أكد( Is_Ordinary_جدول(pTab) );
      لكل(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ايندڙ_From){
        p_والد = sqlite3_ڳوليو_جدول(db, pFK->z_منزل, zDb);
        إذا( p_والد==0 ) استمر;
        pIdx = 0;
        sqlite3_جدول_تالو(pParse, iDb, p_والد->tnum, 0, p_والد->zName);
        x = sqlite3_Fk_Locate_اشاريو(pParse, p_والد, pFK, &pIdx, 0);
        إذا( x==0 ){
          إذا( pIdx==0 ){
            sqlite3_Open_جدول(pParse, i, iDb, p_والد, OP_OpenRead);
          }وإلا{
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_سيٽ_P4_ڪنجي_Info(pParse, pIdx);
          }
        }وإلا{
          k = 0;
          اكسر;
        }
      }
      أكد( pParse->nErr>0 || pFK==0 );
      إذا( pFK ) اكسر;
      إذا( pParse->nTab<i ) pParse->nTab = i;
      addr_چوٽي = sqlite3_Vdbe_جوڙيو_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      أكد( Is_Ordinary_جدول(pTab) );
      لكل(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_ايندڙ_From){
        p_والد = sqlite3_ڳوليو_جدول(db, pFK->z_منزل, zDb);
        pIdx = 0;
        ai_کانن_ڳڻپ = 0;
        إذا( p_والد ){
          x = sqlite3_Fk_Locate_اشاريو(pParse, p_والد, pFK, &pIdx, &ai_کانن_ڳڻپ);
          أكد( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_رجسٽر(pParse, reg_صف + pFK->n_کانو);
        لكل(j=0; j<pFK->n_کانو; j++){
          صحيح i_کانو = ai_کانن_ڳڻپ ? ai_کانن_ڳڻپ[j] : pFK->a_کانو[j].iFrom;
          sqlite3_اظهار_ڪوڊ_Get_ڪالم_جو_جدول(v, pTab, 0, i_کانو, reg_صف+j);
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_IsNull, reg_صف+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        إذا( pIdx ){
          sqlite3_Vdbe_جوڙيو_Op4(v, OP_Affinity, reg_صف, pFK->n_کانو, 0,
              sqlite3_اشاريو_Affinity_Str(db,pIdx), pFK->n_کانو);
          sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, i, addrOk, reg_صف, pFK->n_کانو);
          VdbeCoverage(v);
        }وإلا إذا( p_والد ){
          صحيح jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_SeekRowid, i, jmp, reg_صف); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          أكد( pFK->n_کانو==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        إذا( HasRowid(pTab) ){
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_Rowid, 0, reg_نتيجو+1);
        }وإلا{
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_Null, 0, reg_نتيجو+1);
        }
        sqlite3_Vdbe_Multi_ڀريو(v, reg_نتيجو+2, "siX", pFK->z_منزل, i-1);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, reg_نتيجو, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_کانن_ڳڻپ);
      }
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_ايندڙ, 0, addr_چوٽي+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_هتي(v, addr_چوٽي);
    }
  }
  اكسر;
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_TRIGGER) */
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  حالة PragTyp_CASE_SENSITIVE_LIKE: {
    إذا( z_ساڄو ){
      sqlite3_رجسٽر_Like_Functions(db, sqlite3GetBoolean(z_ساڄو, 0));
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_INTEGRITY_CHECK
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
  حالة PragTyp_INTEGRITY_CHECK: {
    صحيح i, j, addr, mxErr;
    جدول *pObjTab = 0;     /* Check only this one table, if not NULL */

    صحيح is_تيز = (sqlite3Tolower(z_کاٻو[0])=='q');

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
    إذا( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    إذا( z_ساڄو ){
      إذا( sqlite3GetInt32(p_قدر->z, &mxErr) ){
        إذا( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }وإلا{
        pObjTab = sqlite3_Locate_جدول(pParse, 0, z_ساڄو,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    لكل(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      صحيح *a_پاڙ;      /* Array of root page numbers of all btrees */
      صحيح cnt = 0;     /* Number of entries in aRoot[] */

      إذا( OMIT_TEMPDB && i==1 ) استمر;
      إذا( iDb>=0 && i!=iDb ) استمر;

      sqlite3_ڪوڊ_Verify_Schema(pParse, i);
      pParse->ok_Const_ضرب_ڪندڙ = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      أكد( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      لكل(cnt=0, x=sqlite_Hash_پهريون(pTbls); x; x=sqlite_Hash_ايندڙ(x)){
        جدول *pTab = sqlite_Hash_معلومات(x);  /* Current table */
        اشاريو *pIdx;                      /* An index on pTab */
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ) cnt++;
        لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){ cnt++; }
      }
      إذا( cnt==0 ) استمر;
      إذا( pObjTab ) cnt++;
      a_پاڙ = sqlite3DbMallocRawNN(db, حجم(صحيح)*(cnt+1));
      إذا( a_پاڙ==0 ) اكسر;
      cnt = 0;
      إذا( pObjTab ) a_پاڙ[++cnt] = 0;
      لكل(x=sqlite_Hash_پهريون(pTbls); x; x=sqlite_Hash_ايندڙ(x)){
        جدول *pTab = sqlite_Hash_معلومات(x);
        اشاريو *pIdx;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ) a_پاڙ[++cnt] = pTab->tnum;
        لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
          a_پاڙ[++cnt] = pIdx->tnum;
        }
      }
      a_پاڙ[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_رجسٽر(pParse, 8+cnt);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_عارضي_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_IntegrityCk, 1, cnt, 8, (حرف*)a_پاڙ,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_نتيجو_صف(v);
      sqlite3_Vdbe_Jump_هتي(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_ڀريو_String(v, 2, "wrong # of entries in index ");
      لكل(x=sqlite_Hash_پهريون(pTbls); x; x=sqlite_Hash_ايندڙ(x)){
        صحيح iTab = 0;
        جدول *pTab = sqlite_Hash_معلومات(x);
        اشاريو *pIdx;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ){
          iTab = cnt++;
        }وإلا{
          iTab = cnt;
          لكل(pIdx=pTab->p_اشاريو; ALWAYS(pIdx); pIdx=pIdx->p_ايندڙ){
            إذا( Is_Primary_ڪنجي_اشاريو(pIdx) ) اكسر;
            iTab++;
          }
        }
        لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
          إذا( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_جوڙيو_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_ڀريو_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_نتيجو_صف(v);
            sqlite3_Vdbe_Jump_هتي(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      لكل(x=sqlite_Hash_پهريون(pTbls); x; x=sqlite_Hash_ايندڙ(x)){
        جدول *pTab = sqlite_Hash_معلومات(x);
        اشاريو *pIdx, *pPk;
        اشاريو *p_اڳواٽ = 0;      /* Previous index */
        صحيح loop_چوٽي;
        صحيح i_معلومات_Cur, iIdxCur;
        صحيح r1 = -1;
        صحيح bStrict;            /* True for a STRICT table */
        صحيح r2;                 /* Previous key for WITHOUT ROWID tables */
        صحيح mx_کانو;              /* Maximum non-virtual column number */

        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( !Is_Ordinary_جدول(pTab) ) استمر;
        إذا( is_تيز || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }وإلا{
          pPk = sqlite3_Primary_ڪنجي_اشاريو(pTab);
          r2 = sqlite3_Get_عارضي_Range(pParse, pPk->n_ڪنجي_کانو);
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_Null, 1, r2, r2+pPk->n_ڪنجي_کانو-1);
        }
        sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_معلومات_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, 7);
        لكل(j=0, pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ, j++){
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        أكد( pParse->nMem>=8+j );
        أكد( sqlite3_No_Temps_اندريون_Range(pParse,1,7+j) );
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Rewind, i_معلومات_Cur, 0); VdbeCoverage(v);
        loop_چوٽي = sqlite3_Vdbe_جوڙيو_Op2(v, OP_جوڙيو_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        أكد( !IsVirtual(pTab) );
        إذا( HasRowid(pTab) ){
          mx_کانو = -1;
          لكل(j=0; j<pTab->n_کانو; j++){
            إذا( (pTab->a_کانو[j].کانو_Flags & COLFLAG_VIRTUAL)==0 ) mx_کانو++;
          }
          إذا( mx_کانو==pTab->iPKey ) mx_کانو--;
        }وإلا{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mx_کانو = sqlite3_Primary_ڪنجي_اشاريو(pTab)->n_ڪالم-1;
        }
        إذا( mx_کانو>=0 ){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_معلومات_Cur, mx_کانو, 3);
          sqlite3_Vdbe_Typeof_ڪالم(v, 3);
        }

        إذا( !is_تيز ){
          إذا( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            صحيح a1;
            حرف *zErr;
            a1 = sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IdxGT, i_معلومات_Cur, 0,r2,pPk->n_ڪنجي_کانو);
            VdbeCoverage(v);
            sqlite3_Vdbe_جوڙيو_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_نتيجو_صف(v);
            sqlite3_Vdbe_Jump_هتي(v, a1);
            sqlite3_Vdbe_Jump_هتي(v, a1+1);
            لكل(j=0; j<pPk->n_ڪنجي_کانو; j++){
              sqlite3_اظهار_ڪوڊ_ڀريو_اشاريو_ڪالم(pParse, pPk, i_معلومات_Cur, j, r2+j);
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
        لكل(j=0; j<pTab->n_کانو; j++){
          حرف *zErr;
          ڪالم *p_کانو = pTab->a_کانو + j;  /* The column to be checked */
          صحيح labelError;               /* Jump here to report an error */
          صحيح labelOk;                  /* Jump here if all looks ok */
          صحيح p1, p3, p4;               /* Operands to the OP_IsType opcode */
          صحيح doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          إذا( j==pTab->iPKey ) استمر;
          إذا( bStrict ){
            doTypeCheck = p_کانو->eCType>COLTYPE_ANY;
          }وإلا{
            doTypeCheck = p_کانو->affinity>SQLITE_AFF_BLOB;
          }
          إذا( p_کانو->notNull==0 && !doTypeCheck ) استمر;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          إذا( p_کانو->کانو_Flags & COLFLAG_VIRTUAL ){
            sqlite3_اظهار_ڪوڊ_Get_ڪالم_جو_جدول(v, pTab, i_معلومات_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }وإلا{
            إذا( p_کانو->iDflt ){
              sqlite3_قدر *p_Dflt_قدر = 0;
              sqlite3_قدر_From_اظهار(db, sqlite3_ڪالم_اظهار_Auth(pTab,p_کانو,pParse),
                                   ENC(db), p_کانو->affinity, &p_Dflt_قدر);
              إذا( p_Dflt_قدر ){
                p4 = sqlite3_قدر_type(p_Dflt_قدر);
                sqlite3_قدر_Free(p_Dflt_قدر);
              }
            }
            p1 = i_معلومات_Cur;
            إذا( !HasRowid(pTab) ){
              testcase( j!=sqlite3_جدول_ڪالم_منزل_Storage(pTab, j) );
              p3 = sqlite3_جدول_ڪالم_منزل_اشاريو(sqlite3_Primary_ڪنجي_اشاريو(pTab), j);
            }وإلا{
              p3 = sqlite3_جدول_ڪالم_منزل_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          إذا( p_کانو->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            صحيح jmp3;
            صحيح jmp2 = sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            إذا( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }وإلا{
              sqlite3VdbeChangeP5(v, 0x0d); /* INT, TEXT, or BLOB */
              /* OP_IsType does not detect NaN values in the database file
              ** which should be treated as a NULL.  So if the header type
              ** is REAL, we have to load the actual data using OP_Column
              ** to reliably determine if the value is a NULL. */
              sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, p1, p3, 3);
              sqlite3_ڪالم_Default(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_جوڙيو_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                p_کانو->zCnName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            إذا( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_هتي(v, jmp2);
              sqlite3_Vdbe_Jump_هتي(v, jmp3);
            }وإلا{
              /* VDBE byte code will fall thru */
            }
          }
          إذا( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ساكن بنانشان حرف a_Std_Type_نقاب[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /*متن*/
            };
            sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            أكد( p_کانو->eCType>=1 && p_کانو->eCType<=حجم(a_Std_Type_نقاب) );
            sqlite3VdbeChangeP5(v, a_Std_Type_نقاب[p_کانو->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[p_کانو->eCType-1],
                                  pTab->zName, pTab->a_کانو[j].zCnName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وإلا إذا( !bStrict && p_کانو->affinity==SQLITE_AFF_متن ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->a_کانو[j].zCnName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وإلا إذا( !bStrict && p_کانو->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            إذا( p1>=0 ){
              sqlite3_اظهار_ڪوڊ_Get_ڪالم_جو_جدول(v, pTab, i_معلومات_Cur, j, 3);
            }
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->a_کانو[j].zCnName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_نتيجو_صف(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        إذا( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          اظهار_List *pCheck = sqlite3_اظهار_List_Dup(db, pTab->pCheck, 0);
          إذا( db->mallocFailed==0 ){
            صحيح addrCkFault = sqlite3VdbeMakeLabel(pParse);
            صحيح addrCkOk = sqlite3VdbeMakeLabel(pParse);
            حرف *zErr;
            صحيح k;
            pParse->iSelfTab = i_معلومات_Cur + 1;
            لكل(k=pCheck->n_اظهار-1; k>0; k--){
              sqlite3_اظهار_If_False(pParse, pCheck->a[k].p_اظهار, addrCkFault, 0);
            }
            sqlite3_اظهار_If_True(pParse, pCheck->a[0].p_اظهار, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_نتيجو_صف(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_اظهار_List_Delete(db, pCheck);
        }
        إذا( !is_تيز ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          لكل(j=0, pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ, j++){
            صحيح jmp2, jmp3, jmp4, jmp5, label6;
            صحيح kk;
            صحيح ckUniq = sqlite3VdbeMakeLabel(pParse);
            إذا( pPk==pIdx ) استمر;
            r1 = sqlite3_Generate_اشاريو_ڪنجي(pParse, pIdx, i_معلومات_Cur, 0, 0, &jmp3,
                                         p_اڳواٽ, r1);
            p_اڳواٽ = pIdx;
            sqlite3_Vdbe_جوڙيو_Op2(v, OP_جوڙيو_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, iIdxCur+j, ckUniq, r1,
                                        pIdx->n_ڪالم); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_جوڙيو_Op3(v, OP_IFind_ڪنجي, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pIdx, P4_اشاريو);
            sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_نتيجو_صف(v);
            sqlite3_Vdbe_جوڙيو_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_هتي(v, jmp2);
            sqlite3_Vdbe_ڀريو_String(v, 3, "row ");
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_ڀريو_String(v, 4, " missing from index ");
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_ڀريو_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_نتيجو_صف(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            إذا( HasRowid(pTab) ){
              صحيح jmp7;
              sqlite3_Vdbe_جوڙيو_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_جوڙيو_Op3(v, OP_Eq, 3, 0, r1+pIdx->n_ڪالم-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_ڀريو_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ڀريو_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_هتي(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            لكل(kk=0; kk<pIdx->n_ڪنجي_کانو; kk++){
              إذا( pIdx->azColl[kk]==sqlite3_Str_ٻٽو ) استمر;
              إذا( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_جوڙيو_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            إذا( label6 ){
              صحيح jmp6 = sqlite3_Vdbe_جوڙيو_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_ڀريو_String(v, 3, "row ");
              sqlite3_Vdbe_جوڙيو_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_ڀريو_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_هتي(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            إذا( Is_Unique_اشاريو(pIdx) ){
              صحيح uniqOk = sqlite3VdbeMakeLabel(pParse);
              صحيح jmp6;
              لكل(kk=0; kk<pIdx->n_ڪنجي_کانو; kk++){
                صحيح i_کانو = pIdx->ai_ڪالم[kk];
                أكد( i_کانو!=XN_ROWID && i_کانو<pTab->n_کانو );
                إذا( i_کانو>=0 && pTab->a_کانو[i_کانو].notNull ) استمر;
                sqlite3_Vdbe_جوڙيو_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_جوڙيو_Op1(v, OP_ايندڙ, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_هتي(v, jmp6);
              sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_ڪنجي_کانو); VdbeCoverage(v);
              sqlite3_Vdbe_ڀريو_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_هتي(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_ايندڙ, i_معلومات_Cur, loop_چوٽي); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_هتي(v, loop_چوٽي-1);
        إذا( pPk ){
          أكد( !is_تيز );
          sqlite3_Release_عارضي_Range(pParse, r2, pPk->n_ڪنجي_کانو);
        }
      }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      لكل(x=sqlite_Hash_پهريون(pTbls); x; x=sqlite_Hash_ايندڙ(x)){
        جدول *pTab = sqlite_Hash_معلومات(x);
        sqlite3_vtab *pVTab;
        صحيح a1;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( Is_Ordinary_جدول(pTab) ) استمر;
        إذا( !IsVirtual(pTab) ) استمر;
        إذا( pTab->n_کانو<=0 ){
          ثابت حرف *zMod = pTab->u.vtab.azArg[0];
          إذا( sqlite3_Hash_ڳوليو(&db->aModule, zMod)==0 ) استمر;
        }
        sqlite3_View_Get_ڪالم_نالا(pParse, pTab);
        إذا( pTab->u.vtab.p==0 ) استمر;
        pVTab = pTab->u.vtab.p->pVtab;
        إذا( NEVER(pVTab==0) ) استمر;
        إذا( NEVER(pVTab->pModule==0) ) استمر;
        إذا( pVTab->pModule->i_نسخو<4 ) استمر;
        إذا( pVTab->pModule->xIntegrity==0 ) استمر;
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_VCheck, i, 3, is_تيز);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_نتيجو_صف(v);
        sqlite3_Vdbe_Jump_هتي(v, a1);
        استمر;
      }
#جيڪڏهن_پورو
    }
    {
      ساكن ثابت صحيح iLn = VDBE_وٿي_LINENO(2);
      ساكن ثابت VdbeOpList پڇاڙي_ڪوڊ[] = {
        { OP_جوڙيو_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_نتيجو_صف,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_جوڙيو_Op_List(v, Array_سائيز(پڇاڙي_ڪوڊ), پڇاڙي_ڪوڊ, iLn);
      إذا( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (حرف*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_INTEGRITY_CHECK */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
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
  حالة PragTyp_ENCODING: {
    ساكن ثابت بنية EncName {
      حرف *zName;
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
    ثابت بنية EncName *pEnc;
    إذا( !z_ساڄو ){    /* "PRAGMA encoding" */
      إذا( sqlite3ReadSchema(pParse) ) اذهب pragma_ٻاهريون;
      أكد( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      أكد( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      أكد( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      return_Single_متن(v, encnames[ENC(pParse->db)].zName);
    }وإلا{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      إذا( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        لكل(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          إذا( 0==sqlite3StrICmp(z_ساڄو, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_سيٽ_متن_Encoding(db, enc);
            اكسر;
          }
        }
        إذا( !pEnc->zName ){
          sqlite3_Error_نياپو(pParse, "unsupported encoding: %s", z_ساڄو);
        }
      }
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_UTF16 */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SCHEMA_نسخو_PRAGMAS
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
  حالة Prag_Typ_سرو_قدر: {
    صحيح iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_استعمال_ڪري_Btree(v, iDb);
    إذا( z_ساڄو && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ساكن ثابت VdbeOpList سيٽ_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_سيٽ_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_سائيز(سيٽ_Cookie));
      aOp = sqlite3_Vdbe_جوڙيو_Op_List(v, Array_سائيز(سيٽ_Cookie), سيٽ_Cookie, 0);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_ساڄو);
      aOp[1].p5 = 1;
      إذا( iCookie==BTREE_SCHEMA_نسخو && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }وإلا{
      /* Read the specified cookie value */
      ساكن ثابت VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_نتيجو_صف,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_سائيز(readCookie));
      aOp = sqlite3_Vdbe_جوڙيو_Op_List(v, Array_سائيز(readCookie),readCookie,0);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  حالة PragTyp_COMPILE_OPTIONS: {
    صحيح i = 0;
    ثابت حرف *zOpt;
    pParse->nMem = 1;
    طالما( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3_Vdbe_ڀريو_String(v, 1, zOpt);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  اكسر;
#جيڪڏهن_پورو /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  حالة PragTyp_WAL_CHECKPOINT: {
    صحيح iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    صحيح e_گهڻائي = SQLITE_CHECKPOINT_PASSIVE;
    إذا( z_ساڄو ){
      إذا( sqlite3StrICmp(z_ساڄو, "full")==0 ){
        e_گهڻائي = SQLITE_CHECKPOINT_ڀريل;
      }وإلا إذا( sqlite3StrICmp(z_ساڄو, "restart")==0 ){
        e_گهڻائي = SQLITE_CHECKPOINT_RESTART;
      }وإلا إذا( sqlite3StrICmp(z_ساڄو, "truncate")==0 ){
        e_گهڻائي = SQLITE_CHECKPOINT_TRUNCATE;
      }وإلا إذا( sqlite3StrICmp(z_ساڄو, "noop")==0 ){
        e_گهڻائي = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Checkpoint, iBt, e_گهڻائي, 1);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, 1, 3);
  }
  اكسر;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  حالة PragTyp_WAL_AUTOCHECKPOINT: {
    إذا( z_ساڄو ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_ساڄو));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_منزل_INT(db->pWalArg) : 0);
  }
  اكسر;
#جيڪڏهن_پورو

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  حالة Prag_Typ_SHRINK_يادگيري: {
    sqlite3_db_release_يادگيري(db);
    اكسر;
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
  حالة PragTyp_OPTIMIZE: {
    صحيح i_Db_آخري;           /* Loop termination point for the schema loop */
    صحيح iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    جدول *pTab;           /* A table in the schema */
    اشاريو *pIdx;           /* An index of the table */
    LogEst sz_حد;    /* Size threshold above which reanalysis needed */
    حرف *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_نقاب;            /* Mask of operations to perform */
    صحيح n_حد_بندي;            /* Analysis limit to use */
    صحيح nCheck = 0;        /* Number of tables to be optimized */
    صحيح nBtree = 0;        /* Number of btrees to scan */
    صحيح n_اشاريو;            /* Number of indexes on the current table */

    إذا( z_ساڄو ){
      op_نقاب = (u32)sqlite3Atoi(z_ساڄو);
      إذا( (op_نقاب & 0x02)==0 ) اكسر;
    }وإلا{
      op_نقاب = 0xfffe;
    }
    إذا( (op_نقاب & 0x10)==0 ){
      n_حد_بندي = 0;
    }وإلا إذا( db->n_Analysis_حد_بندي>0
           && db->n_Analysis_حد_بندي<SQLITE_DEFAULT_OPTIMIZE_حد_بندي ){
      n_حد_بندي = 0;
    }وإلا{
      n_حد_بندي = SQLITE_DEFAULT_OPTIMIZE_حد_بندي;
    }
    iTabCur = pParse->nTab++;
    لكل(i_Db_آخري = zDb?iDb:db->nDb-1; iDb<=i_Db_آخري; iDb++){
      إذا( iDb==1 ) استمر;
      sqlite3_ڪوڊ_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      لكل(k=sqlite_Hash_پهريون(&pSchema->tblHash); k; k=sqlite_Hash_ايندڙ(k)){
        pTab = (جدول*)sqlite_Hash_معلومات(k);

        /* This only works for ordinary tables */
        إذا( !Is_Ordinary_جدول(pTab) ) استمر;

        /* Do not scan system tables */
        إذا( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) استمر;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_حد = pTab->n_صف_Log_Est;
        n_اشاريو = 0;
        لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
          n_اشاريو++;
          إذا( !pIdx->hasStat1 ){
            sz_حد = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        إذا( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }وإلا إذا( op_نقاب & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }وإلا إذا( pTab->p_اشاريو!=0 && sz_حد<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }وإلا{
          /* Otherwise, we can skip this table */
          استمر;
        }

        nCheck++;
        إذا( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += n_اشاريو+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_جدول(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        إذا( sz_حد>=0 ){
          ثابت LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_If_سائيز_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_نقاب&1),
                         sz_حد>=iRange ? sz_حد-iRange : -1,
                         sz_حد+iRange);
          VdbeCoverage(v);
        }وإلا{
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_نقاب&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        إذا( op_نقاب & 0x01 ){
          صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
          sqlite3_Vdbe_جوڙيو_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, r1, 1);
        }وإلا{
          sqlite3_Vdbe_جوڙيو_Op4(v, OP_SqlExec, n_حد_بندي ? 0x02 : 00, n_حد_بندي, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_جوڙيو_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    إذا( !db->mallocFailed && n_حد_بندي>0 && nBtree>100 ){
      صحيح iAddr, i_پڇاڙي;
      VdbeOp *aOp;
      n_حد_بندي = 100*n_حد_بندي/nBtree;
      إذا( n_حد_بندي<100 ) n_حد_بندي = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_پڇاڙي = sqlite3VdbeCurrentAddr(v);
      لكل(iAddr=0; iAddr<i_پڇاڙي; iAddr++){
        إذا( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_حد_بندي;
      }
    }
    اكسر;
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
  /*case PragTyp_BUSY_TIMEOUT*/ افتراضي: {
    صحيح n = 0;
    أكد( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    إذا( z_ساڄو && sqlite3GetInt32(z_ساڄو,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    اكسر;
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
  حالة Prag_Typ_SOFT_ڍير_حد_بندي: {
    sqlite3_int64 N;
    إذا( z_ساڄو && sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &N)==SQLITE_OK ){
      sqlite3_soft_ڍير_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_ڍير_limit64(-1));
    اكسر;
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
  حالة Prag_Typ_HARD_ڍير_حد_بندي: {
    sqlite3_int64 N;
    إذا( z_ساڄو && sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &N)==SQLITE_OK ){
      sqlite3_int64 i_اڳواٽ = sqlite3_hard_ڍير_limit64(-1);
      إذا( N>0 && (i_اڳواٽ==0 || i_اڳواٽ>N) ) sqlite3_hard_ڍير_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_ڍير_limit64(-1));
    اكسر;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  حالة Prag_Typ_ڌاڳا: {
    sqlite3_int64 N;
    إذا( z_ساڄو
     && sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_حد_بندي(db, SQLITE_حد_بندي_ڪم_ڪندڙ_ڌاڳا, (صحيح)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_حد_بندي(db, SQLITE_حد_بندي_ڪم_ڪندڙ_ڌاڳا, -1));
    اكسر;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  حالة Prag_Typ_ANALYSIS_حد_بندي: {
    sqlite3_int64 N;
    إذا( z_ساڄو
     && sqlite3_Dec_Or_Hex_منزل_I64(z_ساڄو, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_حد_بندي = (صحيح)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_حد_بندي); /* IMP: R-57594-65522 */
    اكسر;
  }

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  حالة Prag_Typ_تالو_حالت: {
    ساكن ثابت حرف *ثابت az_تالو_Name[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    صحيح i;
    pParse->nMem = 2;
    لكل(i=0; i<db->nDb; i++){
      Btree *pBt;
      ثابت حرف *zState = "unknown";
      صحيح j;
      إذا( db->aDb[i].zDbSName==0 ) استمر;
      pBt = db->aDb[i].pBt;
      إذا( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }وإلا إذا( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = az_تالو_Name[j];
      }
      sqlite3_Vdbe_Multi_ڀريو(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    اكسر;
  }
#جيڪڏهن_پورو

#إذا defined(SQLITE_ENABLE_CEROD)
  حالة PragTyp_ACTIVATE_EXTENSIONS: إذا( z_ساڄو ){
    إذا( sqlite3StrNICmp(z_ساڄو, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_ساڄو[6]);
    }
  }
  اكسر;
#جيڪڏهن_پورو

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  إذا( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_ساڄو ){
    sqlite3_Vdbe_Verify_No_نتيجو_صف(v);
  }

pragma_ٻاهريون:
  sqlite3DbFree(db, z_کاٻو);
  sqlite3DbFree(db, z_ساڄو);
}
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
تعريفنوع بنية PragmaVtab PragmaVtab;
تعريفنوع بنية PragmaVtabCursor PragmaVtabCursor;
بنية PragmaVtab {
  sqlite3_vtab بنياد;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ثابت PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
بنية PragmaVtabCursor {
  sqlite3_vtab_cursor بنياد; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  حرف *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
ساكن صحيح pragmaVtabConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  ثابت PragmaName *pPragma = (ثابت PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  صحيح rc;
  صحيح i, j;
  حرف cSep = '(';
  StrAccum acc;
  حرف zBuf[200];

  اڻ_استعمال_ٿيل_PARAMETER(argc);
  اڻ_استعمال_ٿيل_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  لكل(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  إذا( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  إذا( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  إذا( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3_Str_Accum_پڄاڻي(&acc);
  أكد( لڙي_ڊيگهه(zBuf) < حجم(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  إذا( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(حجم(PragmaVtab));
    إذا( pTab==0 ){
      rc = SQLITE_NOMEM;
    }وإلا{
      memset(pTab, 0, حجم(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }وإلا{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  أرجع rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
ساكن صحيح pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  أرجع SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
ساكن صحيح pragma_Vtab_بهترين_اشاريو(sqlite3_vtab *tab, sqlite3_اشاريو_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ثابت بنية sqlite3_اشاريو_constraint *pConstraint;
  صحيح i, j;
  صحيح ڏٺل[2];

  pIdxInfo->estimated_لاڳت = (مزدوج)1;
  إذا( pTab->nHidden==0 ){ أرجع SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  ڏٺل[0] = 0;
  ڏٺل[1] = 0;
  لكل(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    إذا( pConstraint->i_ڪالم < pTab->iHidden ) استمر;
    إذا( pConstraint->op!=SQLITE_اشاريو_CONSTRAINT_EQ ) استمر;
    إذا( pConstraint->usable==0 ) أرجع SQLITE_CONSTRAINT;
    j = pConstraint->i_ڪالم - pTab->iHidden;
    أكد( j < 2 );
    ڏٺل[j] = i+1;
  }
  إذا( ڏٺل[0]==0 ){
    pIdxInfo->estimated_لاڳت = (مزدوج)2147483647;
    pIdxInfo->estimated_صفون = 2147483647;
    أرجع SQLITE_OK;
  }
  j = ڏٺل[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_اشاريو = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimated_لاڳت = (مزدوج)20;
  pIdxInfo->estimated_صفون = 20;
  إذا( ڏٺل[1] ){
    j = ڏٺل[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_اشاريو = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  أرجع SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
ساكن صحيح pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(حجم(*pCsr));
  إذا( pCsr==0 ) أرجع SQLITE_NOMEM;
  memset(pCsr, 0, حجم(PragmaVtabCursor));
  pCsr->بنياد.pVtab = pVtab;
  *ppCursor = &pCsr->بنياد;
  أرجع SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ساكن فراغ pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  صحيح i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  لكل(i=0; i<Array_سائيز(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
ساكن صحيح pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  أرجع SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
ساكن صحيح pragma_Vtab_ايندڙ(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  صحيح rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  أكد( pCsr->pPragma );
  إذا( SQLITE_صف!=sqlite3_قدم(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  أرجع rc;
}

/*
** Pragma virtual table module xFilter method.
*/
ساكن صحيح pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  صحيح idx_انگ, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  صحيح rc;
  صحيح i, j;
  StrAccum acc;
  حرف *zSql;

  اڻ_استعمال_ٿيل_PARAMETER(idx_انگ);
  اڻ_استعمال_ٿيل_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  لكل(i=0; i<argc; i++, j++){
    ثابت حرف *z_متن = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    أكد( j<Array_سائيز(pCsr->azArg) );
    أكد( pCsr->azArg[j]==0 );
    إذا( z_متن ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", z_متن);
      إذا( pCsr->azArg[j]==0 ){
        أرجع SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_حد_بندي[SQLITE_حد_بندي_SQL_ڊيگهائي]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  إذا( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  إذا( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3_Str_Accum_پڄاڻي(&acc);
  إذا( zSql==0 ) أرجع SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  إذا( rc!=SQLITE_OK ){
    pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    أرجع rc;
  }
  أرجع pragma_Vtab_ايندڙ(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
ساكن صحيح pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  أرجع (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
ساكن صحيح pragma_Vtab_ڪالم(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  صحيح i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  إذا( i<pTab->iHidden ){
    sqlite3_نتيجو_قدر(ctx, sqlite3_ڪالم_قدر(pCsr->pPragma, i));
  }وإلا{
    sqlite3_نتيجو_متن(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  أرجع SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
ساكن صحيح pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  أرجع SQLITE_OK;
}

/* The pragma virtual table object */
ساكن ثابت sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragma_Vtab_بهترين_اشاريو,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_ايندڙ,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragma_Vtab_ڪالم,            /* xColumn - read data */
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
Module *sqlite3_Pragma_Vtab_رجسٽر(sqlite3 *db, ثابت حرف *zName){
  ثابت PragmaName *pName;
  أكد( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  إذا( pName==0 ) أرجع 0;
  إذا( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) أرجع 0;
  أكد( sqlite3_Hash_ڳوليو(&db->aModule, zName)==0 );
  أرجع sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (فراغ*)pName, 0);
}

#جيڪڏهن_پورو /* SQLITE_OMIT_VIRTUALTABLE */

#جيڪڏهن_پورو /* SQLITE_OMIT_PRAGMA */
