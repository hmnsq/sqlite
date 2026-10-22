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
#جے_انت

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
#جے_بنا_تعریف SQLITE_DEFAULT_OPTIMIZE_حد_بندی
# define SQLITE_DEFAULT_OPTIMIZE_حد_بندی 2000
#جے_انت

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
ساكن u8 getSafetyLevel(ثابت حرف *z, صحيح omit_بھریا, u8 dflt){
                             /* 123456789 123456789 123 */
  ساكن ثابت حرف z_متن[] = "onoffalseyestruextrafull";
  ساكن ثابت u8 i_وتھ[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ساكن ثابت u8 i_لمائی[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ساكن ثابت u8 i_قدر[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  صحيح i, n;
  إذا( sqlite3Isdigit(*z) ){
    أرجع sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  لكل(i=0; i<Array_ناپ(i_لمائی); i++){
    إذا( i_لمائی[i]==n && sqlite3StrNICmp(&z_متن[i_وتھ[i]],z,n)==0
     && (!omit_بھریا || i_قدر[i]<=1)
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
ساكن صحيح get_Locking_منوال(ثابت حرف *z){
  إذا( z ){
    إذا( 0==sqlite3StrICmp(z, "exclusive") ) أرجع PAGER_LOCKINGMODE_EXCLUSIVE;
    إذا( 0==sqlite3StrICmp(z, "normal") ) أرجع PAGER_LOCKINGMODE_عام;
  }
  أرجع PAGER_LOCKINGMODE_QUERY;
}

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ساكن صحيح getAutoVacuum(ثابت حرف *z){
  صحيح i;
  إذا( 0==sqlite3StrICmp(z, "none") ) أرجع BTREE_AUTOVACUUM_NONE;
  إذا( 0==sqlite3StrICmp(z, "full") ) أرجع BTREE_AUTOVACUUM_بھریا;
  إذا( 0==sqlite3StrICmp(z, "incremental") ) أرجع BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  أرجع (u8)((i>=0&&i<=2)?i:0);
}
#جے_انت /* ifndef SQLITE_OMIT_AUTOVACUUM */

#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ساكن صحيح get_عارضی_Store(ثابت حرف *z){
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
#جے_انت /* SQLITE_PAGER_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ساكن صحيح invalidate_عارضی_Storage(Parse *pParse){
  sqlite3 *db = pParse->db;
  إذا( db->aDb[1].pBt!=0 ){
    إذا( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_پیغام(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      أرجع SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3_Reset_All_Schemas_دا_Connection(db);
  }
  أرجع SQLITE_OK;
}
#جے_انت /* SQLITE_PAGER_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ساكن صحيح change_عارضی_Storage(Parse *pParse, ثابت حرف *zStorageType){
  صحيح ts = get_عارضی_Store(zStorageType);
  sqlite3 *db = pParse->db;
  إذا( db->عارضی_store==ts ) أرجع SQLITE_OK;
  إذا( invalidate_عارضی_Storage( pParse ) != SQLITE_OK ){
    أرجع SQLITE_ERROR;
  }
  db->عارضی_store = (u8)ts;
  أرجع SQLITE_OK;
}
#جے_انت /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ساكن فراغ سیٹ_Pragma_نتیجہ_کالم_ناں(
  Vdbe *v,                     /* The query under construction */
  ثابت PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_سیٹ_عدد_خانے_گنتی(v, n==0 ? 1 : n);
  إذا( n==0 ){
    sqlite3_Vdbe_سیٹ_خانہ_Name(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }وإلا{
    صحيح i, j;
    لكل(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3_Vdbe_سیٹ_خانہ_Name(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ساكن فراغ returnSingleInt(Vdbe *v, i64 قدر){
  sqlite3_Vdbe_جوڑو_Int64(v, 1, قدر);
  sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ساكن فراغ return_Single_متن(
  Vdbe *v,                /* Prepared statement under construction */
  ثابت حرف *z_قدر      /* Value to be returned */
){
  إذا( z_قدر ){
    sqlite3_Vdbe_لدو_String(v, 1, (ثابت حرف*)z_قدر);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
ساكن فراغ سیٹ_All_Pager_Flags(sqlite3 *db){
  إذا( db->autoCommit ){
    Db *pDb = db->aDb;
    صحيح n = db->nDb;
    أكد( SQLITE_بھریا_FSync==PAGER_FULLFSYNC );
    أكد( SQLITE_Ckpt_بھریا_FSync==PAGER_CKPT_FULLFSYNC );
    أكد( SQLITE_CacheSpill==PAGER_CACHESPILL );
    أكد( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_نقاب );
    أكد( (pDb->safety_level & PAGER_SYNCHRONOUS_نقاب)==pDb->safety_level );
    طالما( (n--) > 0 ){
      إذا( pDb->pBt ){
        sqlite3_Btree_سیٹ_Pager_Flags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_نقاب) );
      }
      pDb++;
    }
  }
}
#وإلا
# define سیٹ_All_Pager_Flags(X)  /* no-op */
#جے_انت


/*
** Return a human-readable name for a constraint resolution action.
*/
#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
ساكن ثابت حرف *actionName(u8 action){
  ثابت حرف *zName;
  بدل( action ){
    حالة OE_سیٹ_Null:  zName = "SET NULL";        اكسر;
    حالة OE_سیٹ_Dflt:  zName = "SET DEFAULT";     اكسر;
    حالة OE_Cascade:  zName = "CASCADE";         اكسر;
    حالة OE_Restrict: zName = "RESTRICT";        اكسر;
    افتراضي:          zName = "NO ACTION"; 
                      أكد( action==OE_None ); اكسر;
  }
  أرجع zName;
}
#جے_انت


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ثابت حرف *sqlite3JournalModename(صحيح e_منوال){
  ساكن حرف * ثابت az_منوال_Name[] = {
    "delete", "persist", "off", "truncate", "memory"
#جے_بنا_تعریف SQLITE_OMIT_WAL
     , "wal"
#جے_انت
  };
  أكد( PAGER_JOURNALMODE_DELETE==0 );
  أكد( PAGER_JOURNALMODE_PERSIST==1 );
  أكد( PAGER_JOURNALMODE_OFF==2 );
  أكد( PAGER_JOURNALMODE_TRUNCATE==3 );
  أكد( PAGER_JOURNALMODE_حافظہ==4 );
  أكد( PAGER_JOURNALMODE_WAL==5 );
  أكد( e_منوال>=0 && e_منوال<=Array_ناپ(az_منوال_Name) );

  إذا( e_منوال==Array_ناپ(az_منوال_Name) ) أرجع 0;
  أرجع az_منوال_Name[e_منوال];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ساكن ثابت PragmaName *pragmaLocate(ثابت حرف *zName){
  صحيح upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = Array_ناپ(aPragmaName)-1;
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
ساكن فراغ pragma_Funclist_سطر(
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
      SQLITE_FUNC_اندرونی
  ;
  إذا( showInternFuncs ) نقاب = 0xffffffff;
  لكل(; p; p=p->p_اگلا){
    ثابت حرف *zType;
    ساكن ثابت حرف *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    أكد( SQLITE_FUNC_ENCMASK==0x3 );
    أكد( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    أكد( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    إذا( p->xSFunc==0 ) استمر;
    إذا( (p->funcFlags & SQLITE_FUNC_اندرونی)!=0
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
    sqlite3_Vdbe_Multi_لدو(v, 1, "sissii",
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
ساكن صحيح integrity_Check_نتیجہ_صف(Vdbe *v){
  صحيح addr;
  sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 3, 1);
  addr = sqlite3_Vdbe_جوڑو_Op3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3_Vdbe_جوڑو_Op0(v, OP_Halt);
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
  حرف *z_بایاں = 0;       /* Nul-terminated UTF-8 string <id> */
  حرف *z_دایاں = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
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
  sqlite3_Vdbe_لگاتار_Only_Once(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  إذا( iDb<0 ) أرجع;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  إذا( iDb==1 && sqlite3_Open_عارضی_Database(pParse) ){
    أرجع;
  }

  z_بایاں = sqlite3NameFromToken(db, pId);
  إذا( !z_بایاں ) أرجع;
  إذا( minusFlag ){
    z_دایاں = sqlite3MPrintf(db, "-%T", p_قدر);
  }وإلا{
    z_دایاں = sqlite3NameFromToken(db, p_قدر);
  }

  أكد( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  إذا( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_بایاں, z_دایاں, zDb) ){
    اذهب pragma_باہرلا;
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
  aFcntl[1] = z_بایاں;
  aFcntl[2] = z_دایاں;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (فراغ*)aFcntl);
  إذا( rc==SQLITE_OK ){
    sqlite3_Vdbe_سیٹ_عدد_خانے_گنتی(v, 1);
    sqlite3_Vdbe_سیٹ_خانہ_Name(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    return_Single_متن(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    اذهب pragma_باہرلا;
  }
  إذا( rc!=SQLITE_NOTFOUND ){
    إذا( aFcntl[0] ){
      sqlite3_Error_پیغام(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    اذهب pragma_باہرلا;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_بایاں);
  إذا( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    اذهب pragma_باہرلا;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  إذا( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    إذا( sqlite3ReadSchema(pParse) ) اذهب pragma_باہرلا;
  }

  /* Register the result column names for pragmas that return results */
  إذا( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_دایاں==0)
  ){
    سیٹ_Pragma_نتیجہ_کالم_ناں(v, pPragma);
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
  حالة Prag_Typ_DEFAULT_CACHE_ناپ: {
    ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
    ساكن ثابت VdbeOpList get_Cache_ناپ[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_ناپ},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_گھٹاؤ,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_نتیجہ_صف,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3_Vdbe_ورتدا_Btree(v, iDb);
    إذا( !z_دایاں ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ناپ(get_Cache_ناپ));
      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(get_Cache_ناپ), get_Cache_ناپ, iLn);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_ناپ;
    }وإلا{
      صحيح ناپ = sqlite3AbsInt32(sqlite3Atoi(z_دایاں));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_سیٹ_Cookie, iDb, BTREE_DEFAULT_CACHE_ناپ, ناپ);
      أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_ناپ = ناپ;
      sqlite3_Btree_سیٹ_Cache_ناپ(pDb->pBt, pDb->pSchema->cache_ناپ);
    }
    اكسر;
  }
#جے_انت /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

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
  حالة Prag_Typ_ورقہ_ناپ: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    إذا( !z_دایاں ){
      صحيح ناپ = ALWAYS(pBt) ? sqlite3_Btree_Get_ورقہ_ناپ(pBt) : 0;
      returnSingleInt(v, ناپ);
    }وإلا{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->اگلا_Pagesize = sqlite3Atoi(z_دایاں);
      إذا( SQLITE_NOMEM==sqlite3_Btree_سیٹ_ورقہ_ناپ(pBt, db->اگلا_Pagesize,0,0) ){
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
    إذا( z_دایاں ){
      إذا( sqlite3_stricmp(z_دایاں, "fast")==0 ){
        b = 2;
      }وإلا{
        b = sqlite3GetBoolean(z_دایاں, 0);
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
  حالة Prag_Typ_ورقہ_گنتی: {
    صحيح iReg;
    i64 x = 0;
    sqlite3_کوڈ_Verify_Schema(pParse, iDb);
    iReg = ++pParse->nMem;
    إذا( sqlite3Tolower(z_بایاں[0])=='p' ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Pagecount, iDb, iReg);
    }وإلا{
      إذا( z_دایاں && sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں,&x)==0 ){
        إذا( x<0 ) x = 0;
        وإلا إذا( x>0xfffffffe ) x = 0xfffffffe;
      }وإلا{
        x = 0;
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_MaxPgcnt, iDb, iReg, (صحيح)x);
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, iReg, 1);
    اكسر;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  حالة Prag_Typ_LOCKING_منوال: {
    ثابت حرف *zRet = "normal";
    صحيح e_منوال = get_Locking_منوال(z_دایاں);

    إذا( pId2->n==0 && e_منوال==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      e_منوال = db->dflt_جندرا_منوال;
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
          sqlite3_Pager_Locking_منوال(pPager, e_منوال);
        }
        db->dflt_جندرا_منوال = (u8)e_منوال;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      e_منوال = sqlite3_Pager_Locking_منوال(pPager, e_منوال);
    }

    أكد( e_منوال==PAGER_LOCKINGMODE_عام
            || e_منوال==PAGER_LOCKINGMODE_EXCLUSIVE );
    إذا( e_منوال==PAGER_LOCKINGMODE_EXCLUSIVE ){
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
  حالة Prag_Typ_JOURNAL_منوال: {
    صحيح e_منوال;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    صحيح ii;           /* Loop counter */

    إذا( z_دایاں==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      e_منوال = PAGER_JOURNALMODE_QUERY;
    }وإلا{
      ثابت حرف *z_منوال;
      صحيح n = sqlite3Strlen30(z_دایاں);
      لكل(e_منوال=0; (z_منوال = sqlite3JournalModename(e_منوال))!=0; e_منوال++){
        إذا( sqlite3StrNICmp(z_دایاں, z_منوال, n)==0 ) اكسر;
      }
      إذا( !z_منوال ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        e_منوال = PAGER_JOURNALMODE_QUERY;
      }
      إذا( e_منوال==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        e_منوال = PAGER_JOURNALMODE_QUERY;
      }
    }
    إذا( e_منوال==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    لكل(ii=db->nDb-1; ii>=0; ii--){
      إذا( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3_Vdbe_ورتدا_Btree(v, ii);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Journal_منوال, ii, 1, e_منوال);
      }
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 1);
    اكسر;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  حالة Prag_Typ_JOURNAL_ناپ_حد_بندی: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 i_حد_بندی = -2;
    إذا( z_دایاں ){
      sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &i_حد_بندی);
      إذا( i_حد_بندی<-1 ) i_حد_بندی = -1;
    }
    i_حد_بندی = sqlite3_Pager_Journal_ناپ_حد_بندی(pPager, i_حد_بندی);
    returnSingleInt(v, i_حد_بندی);
    اكسر;
  }

#جے_انت /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
  حالة PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    أكد( pBt!=0 );
    إذا( !z_دایاں ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }وإلا{
      صحيح eAuto = getAutoVacuum(z_دایاں);
      أكد( eAuto>=0 && eAuto<=2 );
      db->اگلا_Autovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3_Btree_سیٹ_Auto_Vacuum(pBt, eAuto);
      إذا( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
        ساكن ثابت VdbeOpList سیٹ_Meta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_سبھ_توں_وڈا_جڑ_ورقہ},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_سیٹ_Cookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        صحيح iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, Array_ناپ(سیٹ_Meta6));
        aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(سیٹ_Meta6), سیٹ_Meta6, iLn);
        إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3_Vdbe_ورتدا_Btree(v, iDb);
      }
    }
    اكسر;
  }
#جے_انت

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
  حالة PragTyp_INCREMENTAL_VACUUM: {
    صحيح i_حد_بندی = 0, addr;
    إذا( z_دایاں==0 || !sqlite3GetInt32(z_دایاں, &i_حد_بندی) || i_حد_بندی<=0 ){
      i_حد_بندی = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, i_حد_بندی, 1);
    addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op1(v, OP_نتیجہ_صف, 1);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, 1, -1);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ایتھے(v, addr);
    اكسر;
  }
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
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
  حالة Prag_Typ_CACHE_ناپ: {
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( !z_دایاں ){
      returnSingleInt(v, pDb->pSchema->cache_ناپ);
    }وإلا{
      صحيح ناپ = sqlite3Atoi(z_دایاں);
      pDb->pSchema->cache_ناپ = ناپ;
      sqlite3_Btree_سیٹ_Cache_ناپ(pDb->pBt, pDb->pSchema->cache_ناپ);
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
    إذا( !z_دایاں ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3_Btree_سیٹ_Spill_ناپ(pDb->pBt,0));
    }وإلا{
      صحيح ناپ = 1;
      إذا( sqlite3GetInt32(z_دایاں, &ناپ) ){
        sqlite3_Btree_سیٹ_Spill_ناپ(pDb->pBt, ناپ);
      }
      إذا( sqlite3GetBoolean(z_دایاں, ناپ!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }وإلا{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      سیٹ_All_Pager_Flags(db);
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
  حالة Prag_Typ_MMAP_ناپ: {
    sqlite3_int64 sz;
#إذا SQLITE_MAX_MMAP_ناپ>0
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    إذا( z_دایاں ){
      صحيح ii;
      sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &sz);
      إذا( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      إذا( pId2->n==0 ) db->szMmap = sz;
      لكل(ii=db->nDb-1; ii>=0; ii--){
        إذا( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3_Btree_سیٹ_Mmap_حد_بندی(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_ناپ, &sz);
#وإلا
    sz = 0;
    rc = SQLITE_OK;
#جے_انت
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
  حالة Prag_Typ_عارضی_STORE: {
    إذا( !z_دایاں ){
      returnSingleInt(v, db->عارضی_store);
    }وإلا{
      change_عارضی_Storage(pParse, z_دایاں);
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
  حالة Prag_Typ_عارضی_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    إذا( !z_دایاں ){
      return_Single_متن(v, sqlite3_عارضی_directory);
    }وإلا{
#جے_بنا_تعریف SQLITE_OMIT_WSD
      إذا( z_دایاں[0] ){
        صحيح res;
        rc = sqlite3OsAccess(db->pVfs, z_دایاں, SQLITE_ACCESS_READWRITE, &res);
        إذا( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_پیغام(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          اذهب pragma_باہرلا;
        }
      }
      إذا( SQLITE_عارضی_STORE==0
       || (SQLITE_عارضی_STORE==1 && db->عارضی_store<=1)
       || (SQLITE_عارضی_STORE==2 && db->عارضی_store==1)
      ){
        invalidate_عارضی_Storage(pParse);
      }
      sqlite3_free(sqlite3_عارضی_directory);
      إذا( z_دایاں[0] ){
        sqlite3_عارضی_directory = sqlite3_mprintf("%s", z_دایاں);
      }وإلا{
        sqlite3_عارضی_directory = 0;
      }
#جے_انت /* SQLITE_OMIT_WSD */
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
  حالة Prag_Typ_انکڑے_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    إذا( !z_دایاں ){
      return_Single_متن(v, sqlite3_انکڑے_directory);
    }وإلا{
#جے_بنا_تعریف SQLITE_OMIT_WSD
      إذا( z_دایاں[0] ){
        صحيح res;
        rc = sqlite3OsAccess(db->pVfs, z_دایاں, SQLITE_ACCESS_READWRITE, &res);
        إذا( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_پیغام(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          اذهب pragma_باہرلا;
        }
      }
      sqlite3_free(sqlite3_انکڑے_directory);
      إذا( z_دایاں[0] ){
        sqlite3_انکڑے_directory = sqlite3_mprintf("%s", z_دایاں);
      }وإلا{
        sqlite3_انکڑے_directory = 0;
      }
#جے_انت /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    اكسر;
  }
#جے_انت

#إذا SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  حالة Prag_Typ_جندرا_PROXY_FILE: {
    إذا( !z_دایاں ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      حرف *proxy_file_رستہ = عدم;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_رستہ);
      return_Single_متن(v, proxy_file_رستہ);
    }وإلا{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      صحيح res;
      إذا( z_دایاں[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_سیٹ_LOCKPROXYFILE,
                                     z_دایاں);
      } وإلا {
        res=sqlite3OsFileControl(pFile, SQLITE_سیٹ_LOCKPROXYFILE,
                                     عدم);
      }
      إذا( res!=SQLITE_OK ){
        sqlite3_Error_پیغام(pParse, "failed to set lock proxy file");
        اذهب pragma_باہرلا;
      }
    }
    اكسر;
  }
#جے_انت /* SQLITE_ENABLE_LOCKING_STYLE */     
   
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
    إذا( !z_دایاں ){
      returnSingleInt(v, pDb->safety_level-1);
    }وإلا{
      إذا( !db->autoCommit ){
        sqlite3_Error_پیغام(pParse,
            "Safety level may not be changed inside a transaction");
      }وإلا إذا( iDb!=1 ){
        صحيح iLevel;
        إذا( sqlite3Isdigit(z_دایاں[0]) ){
          iLevel = sqlite3Atoi(z_دایاں);
        }وإلا{
          iLevel = getSafetyLevel(z_دایاں,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_نقاب;
        إذا( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->b_Sync_سیٹ = 1;
        سیٹ_All_Pager_Flags(db);
      }
    }
    اكسر;
  }
#جے_انت /* SQLITE_OMIT_PAGER_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_FLAG_PRAGMAS
  حالة PragTyp_FLAG: {
    إذا( z_دایاں==0 ){
      سیٹ_Pragma_نتیجہ_کالم_ناں(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }وإلا{
      u64 نقاب = pPragma->iArg;    /* Mask of bits to set or clear. */
      إذا( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        نقاب &= ~(SQLITE_Foreign_کلیدیں);
      }

      إذا( sqlite3GetBoolean(z_دایاں, 0) ){
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
         && sqlite3_stricmp(z_دایاں, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3_Reset_All_Schemas_دا_Connection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3_Vdbe_جوڑو_Op0(v, OP_Expire);
      سیٹ_All_Pager_Flags(db);
    }
    اكسر;
  }
#جے_انت /* SQLITE_OMIT_FLAG_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_SCHEMA_PRAGMAS
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
  حالة Prag_Typ_جدول_INFO: إذا( z_دایاں ){
    جدول *pTab;
    sqlite3_کوڈ_Verify_Named_Schema(pParse, zDb);
    pTab = sqlite3_Locate_جدول(pParse, LOCATE_NOERR, z_دایاں, zDb);
    إذا( pTab ){
      صحيح i, k;
      صحيح nHidden = 0;
      کالم *p_خانہ;
      اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      pParse->nMem = 7;
      sqlite3_View_Get_کالم_ناں(pParse, pTab);
      لكل(i=0, p_خانہ=pTab->a_خانہ; i<pTab->n_خانہ; i++, p_خانہ++){
        صحيح isHidden = 0;
        ثابت اظہاریہ *p_خانہ_اظہاریہ;
        إذا( p_خانہ->خانہ_Flags & COLFLAG_NOINSERT ){
          إذا( pPragma->iArg==0 ){
            nHidden++;
            استمر;
          }
          إذا( p_خانہ->خانہ_Flags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }وإلا إذا( p_خانہ->خانہ_Flags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }وإلا{ أكد( p_خانہ->خانہ_Flags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        إذا( (p_خانہ->خانہ_Flags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }وإلا إذا( pPk==0 ){
          k = 1;
        }وإلا{
          لكل(k=1; k<=pTab->n_خانہ && pPk->ai_کالم[k-1]!=i; k++){}
        }
        p_خانہ_اظہاریہ = sqlite3_کالم_اظہاریہ(pTab,p_خانہ);
        أكد( p_خانہ_اظہاریہ==0 || p_خانہ_اظہاریہ->op==TK_وسعت || isHidden>=2 );
        أكد( p_خانہ_اظہاریہ==0 || !اظہاریہ_Has_Property(p_خانہ_اظہاریہ, EP_Int_قدر)
                  || isHidden>=2 );
        sqlite3_Vdbe_Multi_لدو(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               p_خانہ->zCnName,
               sqlite3_کالم_Type(p_خانہ,""),
               p_خانہ->notNull ? 1 : 0,
               (isHidden>=2 || p_خانہ_اظہاریہ==0) ? 0 : p_خانہ_اظہاریہ->u.zToken,
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
    sqlite3_کوڈ_Verify_Named_Schema(pParse, zDb);
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
      initNCol = sqlite_Hash_گنتی(pHash);
      طالما( initNCol-- ){
        لكل(k=sqlite_Hash_پہلا(pHash); 1; k=sqlite_Hash_اگلا(k) ){
          جدول *pTab;
          إذا( k==0 ){ initNCol = 0; اكسر; }
          pTab = sqlite_Hash_انکڑے(k);
          إذا( pTab->n_خانہ==0 ){
            حرف *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            إذا( zSql ){
              sqlite3_stmt *pDummy = 0;
              (فراغ)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (فراغ)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            إذا( db->mallocFailed ){
              sqlite3_Error_پیغام(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            اكسر;
          }
        }
      }

      لكل(k=sqlite_Hash_پہلا(pHash); k; k=sqlite_Hash_اگلا(k) ){
        جدول *pTab = sqlite_Hash_انکڑے(k);
        ثابت حرف *zType;
        إذا( z_دایاں && sqlite3_stricmp(z_دایاں, pTab->zName)!=0 ) استمر;
        إذا( IsView(pTab) ){
          zType = "view";
        }وإلا إذا( IsVirtual(pTab) ){
          zType = "virtual";
        }وإلا إذا( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }وإلا{
          zType = "table";
        }
        sqlite3_Vdbe_Multi_لدو(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3_Preferred_جدول_Name(pTab->zName),
           zType,
           pTab->n_خانہ,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  اكسر;

#جے_تعریف_شدہ SQLITE_DEBUG
  حالة PragTyp_STATS: {
    اشاریہ *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3_کوڈ_Verify_Schema(pParse, iDb);
    لكل(i=sqlite_Hash_پہلا(&pDb->pSchema->tblHash); i; i=sqlite_Hash_اگلا(i)){
      جدول *pTab = sqlite_Hash_انکڑے(i);
      sqlite3_Vdbe_Multi_لدو(v, 1, "ssiii",
           sqlite3_Preferred_جدول_Name(pTab->zName),
           0,
           pTab->sz_Tab_صف,
           pTab->n_صف_Log_Est,
           pTab->tabFlags);
      لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
        sqlite3_Vdbe_Multi_لدو(v, 2, "siiiX",
           pIdx->zName,
           pIdx->sz_Idx_صف,
           pIdx->ai_صف_Log_Est[0],
           pIdx->hasStat1);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 5);
      }
    }
  }
  اكسر;
#جے_انت

  حالة Prag_Typ_اشاریہ_INFO: إذا( z_دایاں ){
    اشاریہ *pIdx;
    جدول *pTab;
    pIdx = sqlite3_لبھو_اشاریہ(db, z_دایاں, zDb);
    إذا( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3_Locate_جدول(pParse, LOCATE_NOERR, z_دایاں, zDb);
      إذا( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_کلید_اشاریہ(pTab);
      }
    }
    إذا( pIdx ){
      صحيح iIdxDb = sqlite3_Schema_منزل_اشاریہ(db, pIdx->pSchema);
      صحيح i;
      صحيح mx;
      إذا( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->n_کالم;
        pParse->nMem = 6;
      }وإلا{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_کلید_خانہ;
        pParse->nMem = 3;
      }
      pTab = pIdx->p_جدول;
      sqlite3_کوڈ_Verify_Schema(pParse, iIdxDb);
      أكد( pParse->nMem<=pPragma->nPragCName );
      لكل(i=0; i<mx; i++){
        i16 cnum = pIdx->ai_کالم[i];
        sqlite3_Vdbe_Multi_لدو(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->a_خانہ[cnum].zCnName);
        إذا( pPragma->iArg ){
          sqlite3_Vdbe_Multi_لدو(v, 4, "isiX",
            pIdx->a_ترتیب_دیو_ترتیب[i],
            pIdx->azColl[i],
            i<pIdx->n_کلید_خانہ);
        }
        sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, pParse->nMem);
      }
    }
  }
  اكسر;

  حالة Prag_Typ_اشاریہ_LIST: إذا( z_دایاں ){
    اشاریہ *pIdx;
    جدول *pTab;
    صحيح i;
    pTab = sqlite3_لبھو_جدول(db, z_دایاں, zDb);
    إذا( pTab ){
      صحيح iTabDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3_کوڈ_Verify_Schema(pParse, iTabDb);
      لكل(pIdx=pTab->p_اشاریہ, i=0; pIdx; pIdx=pIdx->p_اگلا, i++){
        ثابت حرف *azOrigin[] = { "c", "u", "pk" };
        sqlite3_Vdbe_Multi_لدو(v, 1, "isisi",
           i,
           pIdx->zName,
           Is_Unique_اشاریہ(pIdx),
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
      sqlite3_Vdbe_Multi_لدو(v, 1, "iss",
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
    لكل(p=sqlite_Hash_پہلا(&db->aCollSeq); p; p=sqlite_Hash_اگلا(p)){
      CollSeq *pColl = (CollSeq *)sqlite_Hash_انکڑے(p);
      sqlite3_Vdbe_Multi_لدو(v, 1, "is", i++, pColl->zName);
    }
  }
  اكسر;

#جے_بنا_تعریف SQLITE_OMIT_INTROSPECTION_PRAGMAS
  حالة PragTyp_FUNCTION_LIST: {
    صحيح i;
    HashElem *j;
    FuncDef *p;
    صحيح showInternFunc = (db->mDbFlags & DBFLAG_اندرونی_Func)!=0;
    pParse->nMem = 6;
    لكل(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      لكل(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        أكد( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_سطر(v, p, 1, showInternFunc);
      }
    }
    لكل(j=sqlite_Hash_پہلا(&db->aFunc); j; j=sqlite_Hash_اگلا(j)){
      p = (FuncDef*)sqlite_Hash_انکڑے(j);
      أكد( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_سطر(v, p, 0, showInternFunc);
    }
  }
  اكسر;

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  حالة PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    لكل(j=sqlite_Hash_پہلا(&db->aModule); j; j=sqlite_Hash_اگلا(j)){
      Module *pMod = (Module*)sqlite_Hash_انکڑے(j);
      sqlite3_Vdbe_Multi_لدو(v, 1, "s", pMod->zName);
    }
  }
  اكسر;
#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */

  حالة PragTyp_PRAGMA_LIST: {
    صحيح i;
    لكل(i=0; i<Array_ناپ(aPragmaName); i++){
      sqlite3_Vdbe_Multi_لدو(v, 1, "s", aPragmaName[i].zName);
    }
  }
  اكسر;
#جے_انت /* SQLITE_INTROSPECTION_PRAGMAS */

#جے_انت /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
  حالة Prag_Typ_FOREIGN_کلید_LIST: إذا( z_دایاں ){
    FKey *pFK;
    جدول *pTab;
    pTab = sqlite3_لبھو_جدول(db, z_دایاں, zDb);
    إذا( pTab && Is_Ordinary_جدول(pTab) ){
      pFK = pTab->u.tab.pFKey;
      إذا( pFK ){
        صحيح iTabDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
        صحيح i = 0;
        pParse->nMem = 8;
        sqlite3_کوڈ_Verify_Schema(pParse, iTabDb);
        طالما(pFK){
          صحيح j;
          لكل(j=0; j<pFK->n_خانہ; j++){
            sqlite3_Vdbe_Multi_لدو(v, 1, "iissssss",
                   i,
                   j,
                   pFK->z_منزل,
                   pTab->a_خانہ[pFK->a_خانہ[j].iFrom].zCnName,
                   pFK->a_خانہ[j].z_خانہ,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->p_اگلا_From;
        }
      }
    }
  }
  اكسر;
#جے_انت /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
  حالة Prag_Typ_FOREIGN_کلید_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    جدول *pTab;           /* Child table contain "REFERENCES" keyword */
    جدول *p_ماپا;        /* Parent table that child points to */
    اشاریہ *pIdx;           /* Index in the parent table */
    صحيح i;                 /* Loop counter:  Foreign key number for pTab */
    صحيح j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    صحيح x;                 /* result variable */
    صحيح reg_نتیجہ;         /* 3 registers to hold a result row */
    صحيح reg_صف;            /* Registers to hold a row from pTab */
    صحيح addr_چوٹی;           /* Top of a loop checking foreign keys */
    صحيح addrOk;            /* Jump here if the key is OK */
    صحيح *ai_خانے_گنتی;           /* child to parent column mapping */

    reg_نتیجہ = pParse->nMem+1;
    pParse->nMem += 4;
    reg_صف = ++pParse->nMem;
    k = sqlite_Hash_پہلا(&db->aDb[iDb].pSchema->tblHash);
    طالما( k ){
      إذا( z_دایاں ){
        pTab = sqlite3_Locate_جدول(pParse, 0, z_دایاں, zDb);
        k = 0;
      }وإلا{
        pTab = (جدول*)sqlite_Hash_انکڑے(k);
        k = sqlite_Hash_اگلا(k);
      }
      إذا( pTab==0 || !Is_Ordinary_جدول(pTab) || pTab->u.tab.pFKey==0 ) استمر;
      iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3_کوڈ_Verify_Schema(pParse, iDb);
      sqlite3_جدول_جندرا(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3_Touch_رجسٹر(pParse, pTab->n_خانہ+reg_صف);
      sqlite3_Open_جدول(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3_Vdbe_لدو_String(v, reg_نتیجہ, pTab->zName);
      أكد( Is_Ordinary_جدول(pTab) );
      لكل(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_اگلا_From){
        p_ماپا = sqlite3_لبھو_جدول(db, pFK->z_منزل, zDb);
        إذا( p_ماپا==0 ) استمر;
        pIdx = 0;
        sqlite3_جدول_جندرا(pParse, iDb, p_ماپا->tnum, 0, p_ماپا->zName);
        x = sqlite3_Fk_Locate_اشاریہ(pParse, p_ماپا, pFK, &pIdx, 0);
        إذا( x==0 ){
          إذا( pIdx==0 ){
            sqlite3_Open_جدول(pParse, i, iDb, p_ماپا, OP_OpenRead);
          }وإلا{
            sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
          }
        }وإلا{
          k = 0;
          اكسر;
        }
      }
      أكد( pParse->nErr>0 || pFK==0 );
      إذا( pFK ) اكسر;
      إذا( pParse->nTab<i ) pParse->nTab = i;
      addr_چوٹی = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, 0); VdbeCoverage(v);
      أكد( Is_Ordinary_جدول(pTab) );
      لكل(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->p_اگلا_From){
        p_ماپا = sqlite3_لبھو_جدول(db, pFK->z_منزل, zDb);
        pIdx = 0;
        ai_خانے_گنتی = 0;
        إذا( p_ماپا ){
          x = sqlite3_Fk_Locate_اشاریہ(pParse, p_ماپا, pFK, &pIdx, &ai_خانے_گنتی);
          أكد( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3_Touch_رجسٹر(pParse, reg_صف + pFK->n_خانہ);
        لكل(j=0; j<pFK->n_خانہ; j++){
          صحيح i_خانہ = ai_خانے_گنتی ? ai_خانے_گنتی[j] : pFK->a_خانہ[j].iFrom;
          sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, 0, i_خانہ, reg_صف+j);
          sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, reg_صف+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        إذا( pIdx ){
          sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, reg_صف, pFK->n_خانہ, 0,
              sqlite3_اشاریہ_Affinity_Str(db,pIdx), pFK->n_خانہ);
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_لبھیا, i, addrOk, reg_صف, pFK->n_خانہ);
          VdbeCoverage(v);
        }وإلا إذا( p_ماپا ){
          صحيح jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, i, jmp, reg_صف); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          أكد( pFK->n_خانہ==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        إذا( HasRowid(pTab) ){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, 0, reg_نتیجہ+1);
        }وإلا{
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg_نتیجہ+1);
        }
        sqlite3_Vdbe_Multi_لدو(v, reg_نتیجہ+2, "siX", pFK->z_منزل, i-1);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, reg_نتیجہ, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, ai_خانے_گنتی);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, 0, addr_چوٹی+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ایتھے(v, addr_چوٹی);
    }
  }
  اكسر;
#جے_انت /* !defined(SQLITE_OMIT_TRIGGER) */
#جے_انت /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#جے_بنا_تعریف SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  حالة PragTyp_CASE_SENSITIVE_LIKE: {
    إذا( z_دایاں ){
      sqlite3_رجسٹر_Like_Functions(db, sqlite3GetBoolean(z_دایاں, 0));
    }
  }
  اكسر;
#جے_انت /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#جے_بنا_تعریف SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_INTEGRITY_CHECK
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

    صحيح is_تیز = (sqlite3Tolower(z_بایاں[0])=='q');

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
    إذا( z_دایاں ){
      إذا( sqlite3GetInt32(p_قدر->z, &mxErr) ){
        إذا( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }وإلا{
        pObjTab = sqlite3_Locate_جدول(pParse, 0, z_دایاں,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    لكل(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      صحيح *a_جڑ;      /* Array of root page numbers of all btrees */
      صحيح cnt = 0;     /* Number of entries in aRoot[] */

      إذا( OMIT_TEMPDB && i==1 ) استمر;
      إذا( iDb>=0 && i!=iDb ) استمر;

      sqlite3_کوڈ_Verify_Schema(pParse, i);
      pParse->ok_Const_ضارب = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      أكد( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      لكل(cnt=0, x=sqlite_Hash_پہلا(pTbls); x; x=sqlite_Hash_اگلا(x)){
        جدول *pTab = sqlite_Hash_انکڑے(x);  /* Current table */
        اشاریہ *pIdx;                      /* An index on pTab */
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ) cnt++;
        لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){ cnt++; }
      }
      إذا( cnt==0 ) استمر;
      إذا( pObjTab ) cnt++;
      a_جڑ = sqlite3DbMallocRawNN(db, حجم(صحيح)*(cnt+1));
      إذا( a_جڑ==0 ) اكسر;
      cnt = 0;
      إذا( pObjTab ) a_جڑ[++cnt] = 0;
      لكل(x=sqlite_Hash_پہلا(pTbls); x; x=sqlite_Hash_اگلا(x)){
        جدول *pTab = sqlite_Hash_انکڑے(x);
        اشاریہ *pIdx;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ) a_جڑ[++cnt] = pTab->tnum;
        لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          a_جڑ[++cnt] = pIdx->tnum;
        }
      }
      a_جڑ[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3_Touch_رجسٹر(pParse, 8+cnt);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3_Clear_عارضی_Reg_Cache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3_Vdbe_جوڑو_Op4(v, OP_IntegrityCk, 1, cnt, 8, (حرف*)a_جڑ,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 2, 3, 3);
      integrity_Check_نتیجہ_صف(v);
      sqlite3_Vdbe_Jump_ایتھے(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3_Vdbe_لدو_String(v, 2, "wrong # of entries in index ");
      لكل(x=sqlite_Hash_پہلا(pTbls); x; x=sqlite_Hash_اگلا(x)){
        صحيح iTab = 0;
        جدول *pTab = sqlite_Hash_انکڑے(x);
        اشاریہ *pIdx;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( HasRowid(pTab) ){
          iTab = cnt++;
        }وإلا{
          iTab = cnt;
          لكل(pIdx=pTab->p_اشاریہ; ALWAYS(pIdx); pIdx=pIdx->p_اگلا){
            إذا( Is_Primary_کلید_اشاریہ(pIdx) ) اكسر;
            iTab++;
          }
        }
        لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          إذا( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3_Vdbe_لدو_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 4, 2, 3);
            integrity_Check_نتیجہ_صف(v);
            sqlite3_Vdbe_Jump_ایتھے(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      لكل(x=sqlite_Hash_پہلا(pTbls); x; x=sqlite_Hash_اگلا(x)){
        جدول *pTab = sqlite_Hash_انکڑے(x);
        اشاریہ *pIdx, *pPk;
        اشاریہ *p_پہلوں = 0;      /* Previous index */
        صحيح loop_چوٹی;
        صحيح i_انکڑے_Cur, iIdxCur;
        صحيح r1 = -1;
        صحيح bStrict;            /* True for a STRICT table */
        صحيح r2;                 /* Previous key for WITHOUT ROWID tables */
        صحيح mx_خانہ;              /* Maximum non-virtual column number */

        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( !Is_Ordinary_جدول(pTab) ) استمر;
        إذا( is_تیز || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }وإلا{
          pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
          r2 = sqlite3_Get_عارضی_Range(pParse, pPk->n_کلید_خانہ);
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 1, r2, r2+pPk->n_کلید_خانہ-1);
        }
        sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &i_انکڑے_Cur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, 7);
        لكل(j=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, j++){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        أكد( pParse->nMem>=8+j );
        أكد( sqlite3_No_Temps_وچ_Range(pParse,1,7+j) );
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, i_انکڑے_Cur, 0); VdbeCoverage(v);
        loop_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        أكد( !IsVirtual(pTab) );
        إذا( HasRowid(pTab) ){
          mx_خانہ = -1;
          لكل(j=0; j<pTab->n_خانہ; j++){
            إذا( (pTab->a_خانہ[j].خانہ_Flags & COLFLAG_VIRTUAL)==0 ) mx_خانہ++;
          }
          إذا( mx_خانہ==pTab->iPKey ) mx_خانہ--;
        }وإلا{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mx_خانہ = sqlite3_Primary_کلید_اشاریہ(pTab)->n_کالم-1;
        }
        إذا( mx_خانہ>=0 ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, i_انکڑے_Cur, mx_خانہ, 3);
          sqlite3_Vdbe_Typeof_کالم(v, 3);
        }

        إذا( !is_تیز ){
          إذا( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            صحيح a1;
            حرف *zErr;
            a1 = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IdxGT, i_انکڑے_Cur, 0,r2,pPk->n_کلید_خانہ);
            VdbeCoverage(v);
            sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_نتیجہ_صف(v);
            sqlite3_Vdbe_Jump_ایتھے(v, a1);
            sqlite3_Vdbe_Jump_ایتھے(v, a1+1);
            لكل(j=0; j<pPk->n_کلید_خانہ; j++){
              sqlite3_اظہاریہ_کوڈ_لدو_اشاریہ_کالم(pParse, pPk, i_انکڑے_Cur, j, r2+j);
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
        لكل(j=0; j<pTab->n_خانہ; j++){
          حرف *zErr;
          کالم *p_خانہ = pTab->a_خانہ + j;  /* The column to be checked */
          صحيح labelError;               /* Jump here to report an error */
          صحيح labelOk;                  /* Jump here if all looks ok */
          صحيح p1, p3, p4;               /* Operands to the OP_IsType opcode */
          صحيح doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          إذا( j==pTab->iPKey ) استمر;
          إذا( bStrict ){
            doTypeCheck = p_خانہ->eCType>COLTYPE_ANY;
          }وإلا{
            doTypeCheck = p_خانہ->affinity>SQLITE_AFF_BLOB;
          }
          إذا( p_خانہ->notNull==0 && !doTypeCheck ) استمر;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          إذا( p_خانہ->خانہ_Flags & COLFLAG_VIRTUAL ){
            sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur, j, 3);
            p1 = -1;
            p3 = 3;
          }وإلا{
            إذا( p_خانہ->iDflt ){
              sqlite3_قدر *p_Dflt_قدر = 0;
              sqlite3_قدر_From_اظہاریہ(db, sqlite3_کالم_اظہاریہ_Auth(pTab,p_خانہ,pParse),
                                   ENC(db), p_خانہ->affinity, &p_Dflt_قدر);
              إذا( p_Dflt_قدر ){
                p4 = sqlite3_قدر_type(p_Dflt_قدر);
                sqlite3_قدر_Free(p_Dflt_قدر);
              }
            }
            p1 = i_انکڑے_Cur;
            إذا( !HasRowid(pTab) ){
              testcase( j!=sqlite3_جدول_کالم_منزل_Storage(pTab, j) );
              p3 = sqlite3_جدول_کالم_منزل_اشاریہ(sqlite3_Primary_کلید_اشاریہ(pTab), j);
            }وإلا{
              p3 = sqlite3_جدول_کالم_منزل_Storage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          إذا( p_خانہ->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            صحيح jmp3;
            صحيح jmp2 = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
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
              sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, p1, p3, 3);
              sqlite3_کالم_Default(v, pTab, j, 3);
              jmp3 = sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                p_خانہ->zCnName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            إذا( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3_Vdbe_Jump_ایتھے(v, jmp2);
              sqlite3_Vdbe_Jump_ایتھے(v, jmp3);
            }وإلا{
              /* VDBE byte code will fall thru */
            }
          }
          إذا( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ساكن بنانشانی حرف a_Std_Type_نقاب[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /*متن*/
            };
            sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            أكد( p_خانہ->eCType>=1 && p_خانہ->eCType<=حجم(a_Std_Type_نقاب) );
            sqlite3VdbeChangeP5(v, a_Std_Type_نقاب[p_خانہ->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[p_خانہ->eCType-1],
                                  pTab->zName, pTab->a_خانہ[j].zCnName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وإلا إذا( !bStrict && p_خانہ->affinity==SQLITE_AFF_متن ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->a_خانہ[j].zCnName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }وإلا إذا( !bStrict && p_خانہ->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            إذا( p1>=0 ){
              sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, i_انکڑے_Cur, j, 3);
            }
            sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->a_خانہ[j].zCnName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrity_Check_نتیجہ_صف(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        إذا( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          اظہاریہ_List *pCheck = sqlite3_اظہاریہ_List_Dup(db, pTab->pCheck, 0);
          إذا( db->mallocFailed==0 ){
            صحيح addrCkFault = sqlite3VdbeMakeLabel(pParse);
            صحيح addrCkOk = sqlite3VdbeMakeLabel(pParse);
            حرف *zErr;
            صحيح k;
            pParse->iSelfTab = i_انکڑے_Cur + 1;
            لكل(k=pCheck->n_اظہاریہ-1; k>0; k--){
              sqlite3_اظہاریہ_If_False(pParse, pCheck->a[k].p_اظہاریہ, addrCkFault, 0);
            }
            sqlite3_اظہاریہ_If_True(pParse, pCheck->a[0].p_اظہاریہ, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrity_Check_نتیجہ_صف(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_اظہاریہ_List_Delete(db, pCheck);
        }
        إذا( !is_تیز ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          لكل(j=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, j++){
            صحيح jmp2, jmp3, jmp4, jmp5, label6;
            صحيح kk;
            صحيح ckUniq = sqlite3VdbeMakeLabel(pParse);
            إذا( pPk==pIdx ) استمر;
            r1 = sqlite3_Generate_اشاریہ_کلید(pParse, pIdx, i_انکڑے_Cur, 0, 0, &jmp3,
                                         p_پہلوں, r1);
            p_پہلوں = pIdx;
            sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_لبھیا, iIdxCur+j, ckUniq, r1,
                                        pIdx->n_کالم); VdbeCoverage(v);
            jmp2 = sqlite3_Vdbe_جوڑو_Op3(v, OP_IFind_کلید, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pIdx, P4_اشاریہ);
            sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 7, 3, 3);
            integrity_Check_نتیجہ_صف(v);
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, ckUniq);

            sqlite3_Vdbe_Jump_ایتھے(v, jmp2);
            sqlite3_Vdbe_لدو_String(v, 3, "row ");
            sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 7, 3, 3);
            sqlite3_Vdbe_لدو_String(v, 4, " missing from index ");
            sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3_Vdbe_لدو_String(v, 4, pIdx->zName);
            sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrity_Check_نتیجہ_صف(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            إذا( HasRowid(pTab) ){
              صحيح jmp7;
              sqlite3_Vdbe_جوڑو_Op2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, 3, 0, r1+pIdx->n_کالم-1);
              VdbeCoverageNeverNull(v);
              sqlite3_Vdbe_لدو_String(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_لدو_String(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ایتھے(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            لكل(kk=0; kk<pIdx->n_کلید_خانہ; kk++){
              إذا( pIdx->azColl[kk]==sqlite3_Str_ثنائی ) استمر;
              إذا( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iIdxCur+j, kk, 3);
              sqlite3_Vdbe_جوڑو_Op3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            إذا( label6 ){
              صحيح jmp6 = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3_Vdbe_لدو_String(v, 3, "row ");
              sqlite3_Vdbe_جوڑو_Op3(v, OP_Concat, 7, 3, 3);
              sqlite3_Vdbe_لدو_String(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3_Vdbe_Jump_ایتھے(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            إذا( Is_Unique_اشاریہ(pIdx) ){
              صحيح uniqOk = sqlite3VdbeMakeLabel(pParse);
              صحيح jmp6;
              لكل(kk=0; kk<pIdx->n_کلید_خانہ; kk++){
                صحيح i_خانہ = pIdx->ai_کالم[kk];
                أكد( i_خانہ!=XN_ROWID && i_خانہ<pTab->n_خانہ );
                إذا( i_خانہ>=0 && pTab->a_خانہ[i_خانہ].notNull ) استمر;
                sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3_Vdbe_جوڑو_Op1(v, OP_اگلا, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3_Vdbe_Jump_ایتھے(v, jmp6);
              sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_کلید_خانہ); VdbeCoverage(v);
              sqlite3_Vdbe_لدو_String(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3_Vdbe_Jump_ایتھے(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, i_انکڑے_Cur, loop_چوٹی); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ایتھے(v, loop_چوٹی-1);
        إذا( pPk ){
          أكد( !is_تیز );
          sqlite3_Release_عارضی_Range(pParse, r2, pPk->n_کلید_خانہ);
        }
      }

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      لكل(x=sqlite_Hash_پہلا(pTbls); x; x=sqlite_Hash_اگلا(x)){
        جدول *pTab = sqlite_Hash_انکڑے(x);
        sqlite3_vtab *pVTab;
        صحيح a1;
        إذا( جدول_Skip_Integrity_Check(pTab,pObjTab) ) استمر;
        إذا( Is_Ordinary_جدول(pTab) ) استمر;
        إذا( !IsVirtual(pTab) ) استمر;
        إذا( pTab->n_خانہ<=0 ){
          ثابت حرف *zMod = pTab->u.vtab.azArg[0];
          إذا( sqlite3_Hash_لبھو(&db->aModule, zMod)==0 ) استمر;
        }
        sqlite3_View_Get_کالم_ناں(pParse, pTab);
        إذا( pTab->u.vtab.p==0 ) استمر;
        pVTab = pTab->u.vtab.p->pVtab;
        إذا( NEVER(pVTab==0) ) استمر;
        إذا( NEVER(pVTab->pModule==0) ) استمر;
        إذا( pVTab->pModule->i_نسخہ<4 ) استمر;
        إذا( pVTab->pModule->xIntegrity==0 ) استمر;
        sqlite3_Vdbe_جوڑو_Op3(v, OP_VCheck, i, 3, is_تیز);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrity_Check_نتیجہ_صف(v);
        sqlite3_Vdbe_Jump_ایتھے(v, a1);
        استمر;
      }
#جے_انت
    }
    {
      ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
      ساكن ثابت VdbeOpList سرا_کوڈ[] = {
        { OP_جوڑو_Imm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_نتیجہ_صف,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(سرا_کوڈ), سرا_کوڈ, iLn);
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
#جے_انت /* SQLITE_OMIT_INTEGRITY_CHECK */

#جے_بنا_تعریف SQLITE_OMIT_UTF16
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
    إذا( !z_دایاں ){    /* "PRAGMA encoding" */
      إذا( sqlite3ReadSchema(pParse) ) اذهب pragma_باہرلا;
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
          إذا( 0==sqlite3StrICmp(z_دایاں, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3_سیٹ_متن_Encoding(db, enc);
            اكسر;
          }
        }
        إذا( !pEnc->zName ){
          sqlite3_Error_پیغام(pParse, "unsupported encoding: %s", z_دایاں);
        }
      }
    }
  }
  اكسر;
#جے_انت /* SQLITE_OMIT_UTF16 */

#جے_بنا_تعریف SQLITE_OMIT_SCHEMA_نسخہ_PRAGMAS
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
  حالة Prag_Typ_سرخی_قدر: {
    صحيح iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3_Vdbe_ورتدا_Btree(v, iDb);
    إذا( z_دایاں && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ساكن ثابت VdbeOpList سیٹ_Cookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_سیٹ_Cookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ناپ(سیٹ_Cookie));
      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(سیٹ_Cookie), سیٹ_Cookie, 0);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_دایاں);
      aOp[1].p5 = 1;
      إذا( iCookie==BTREE_SCHEMA_نسخہ && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }وإلا{
      /* Read the specified cookie value */
      ساكن ثابت VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_نتیجہ_صف,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, Array_ناپ(readCookie));
      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(readCookie),readCookie,0);
      إذا( ONLY_IF_REALLOC_STRESS(aOp==0) ) اكسر;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  اكسر;
#جے_انت /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#جے_بنا_تعریف SQLITE_OMIT_COMPILEOPTION_DIAGS
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
      sqlite3_Vdbe_لدو_String(v, 1, zOpt);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  اكسر;
#جے_انت /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#جے_بنا_تعریف SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  حالة PragTyp_WAL_CHECKPOINT: {
    صحيح iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    صحيح e_منوال = SQLITE_CHECKPOINT_PASSIVE;
    إذا( z_دایاں ){
      إذا( sqlite3StrICmp(z_دایاں, "full")==0 ){
        e_منوال = SQLITE_CHECKPOINT_بھریا;
      }وإلا إذا( sqlite3StrICmp(z_دایاں, "restart")==0 ){
        e_منوال = SQLITE_CHECKPOINT_RESTART;
      }وإلا إذا( sqlite3StrICmp(z_دایاں, "truncate")==0 ){
        e_منوال = SQLITE_CHECKPOINT_TRUNCATE;
      }وإلا إذا( sqlite3StrICmp(z_دایاں, "noop")==0 ){
        e_منوال = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Checkpoint, iBt, e_منوال, 1);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, 1, 3);
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
    إذا( z_دایاں ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_دایاں));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_منزل_INT(db->pWalArg) : 0);
  }
  اكسر;
#جے_انت

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  حالة Prag_Typ_SHRINK_حافظہ: {
    sqlite3_db_release_حافظہ(db);
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
    صحيح i_Db_آخری;           /* Loop termination point for the schema loop */
    صحيح iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    جدول *pTab;           /* A table in the schema */
    اشاریہ *pIdx;           /* An index of the table */
    LogEst sz_حد;    /* Size threshold above which reanalysis needed */
    حرف *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 op_نقاب;            /* Mask of operations to perform */
    صحيح n_حد_بندی;            /* Analysis limit to use */
    صحيح nCheck = 0;        /* Number of tables to be optimized */
    صحيح nBtree = 0;        /* Number of btrees to scan */
    صحيح n_اشاریہ;            /* Number of indexes on the current table */

    إذا( z_دایاں ){
      op_نقاب = (u32)sqlite3Atoi(z_دایاں);
      إذا( (op_نقاب & 0x02)==0 ) اكسر;
    }وإلا{
      op_نقاب = 0xfffe;
    }
    إذا( (op_نقاب & 0x10)==0 ){
      n_حد_بندی = 0;
    }وإلا إذا( db->n_Analysis_حد_بندی>0
           && db->n_Analysis_حد_بندی<SQLITE_DEFAULT_OPTIMIZE_حد_بندی ){
      n_حد_بندی = 0;
    }وإلا{
      n_حد_بندی = SQLITE_DEFAULT_OPTIMIZE_حد_بندی;
    }
    iTabCur = pParse->nTab++;
    لكل(i_Db_آخری = zDb?iDb:db->nDb-1; iDb<=i_Db_آخری; iDb++){
      إذا( iDb==1 ) استمر;
      sqlite3_کوڈ_Verify_Schema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      لكل(k=sqlite_Hash_پہلا(&pSchema->tblHash); k; k=sqlite_Hash_اگلا(k)){
        pTab = (جدول*)sqlite_Hash_انکڑے(k);

        /* This only works for ordinary tables */
        إذا( !Is_Ordinary_جدول(pTab) ) استمر;

        /* Do not scan system tables */
        إذا( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) استمر;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        sz_حد = pTab->n_صف_Log_Est;
        n_اشاریہ = 0;
        لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          n_اشاریہ++;
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
        }وإلا إذا( pTab->p_اشاریہ!=0 && sz_حد<0 ){
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
        nBtree += n_اشاریہ+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3_Open_جدول(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        إذا( sz_حد>=0 ){
          ثابت LogEst iRange = 33;   /* 10x size change */
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_If_ناپ_Between, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_نقاب&1),
                         sz_حد>=iRange ? sz_حد-iRange : -1,
                         sz_حد+iRange);
          VdbeCoverage(v);
        }وإلا{
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(op_نقاب&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        إذا( op_نقاب & 0x01 ){
          صحيح r1 = sqlite3_Get_عارضی_Reg(pParse);
          sqlite3_Vdbe_جوڑو_Op4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, r1, 1);
        }وإلا{
          sqlite3_Vdbe_جوڑو_Op4(v, OP_SqlExec, n_حد_بندی ? 0x02 : 00, n_حد_بندی, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3_Vdbe_جوڑو_Op0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    إذا( !db->mallocFailed && n_حد_بندی>0 && nBtree>100 ){
      صحيح iAddr, i_سرا;
      VdbeOp *aOp;
      n_حد_بندی = 100*n_حد_بندی/nBtree;
      إذا( n_حد_بندی<100 ) n_حد_بندی = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      i_سرا = sqlite3VdbeCurrentAddr(v);
      لكل(iAddr=0; iAddr<i_سرا; iAddr++){
        إذا( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = n_حد_بندی;
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
    إذا( z_دایاں && sqlite3GetInt32(z_دایاں,&n) ){
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
  حالة Prag_Typ_SOFT_ڈھیر_حد_بندی: {
    sqlite3_int64 N;
    إذا( z_دایاں && sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &N)==SQLITE_OK ){
      sqlite3_soft_ڈھیر_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_ڈھیر_limit64(-1));
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
  حالة Prag_Typ_HARD_ڈھیر_حد_بندی: {
    sqlite3_int64 N;
    إذا( z_دایاں && sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &N)==SQLITE_OK ){
      sqlite3_int64 i_پہلوں = sqlite3_hard_ڈھیر_limit64(-1);
      إذا( N>0 && (i_پہلوں==0 || i_پہلوں>N) ) sqlite3_hard_ڈھیر_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_ڈھیر_limit64(-1));
    اكسر;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  حالة Prag_Typ_دھاگے: {
    sqlite3_int64 N;
    إذا( z_دایاں
     && sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_حد_بندی(db, SQLITE_حد_بندی_کامہ_دھاگے, (صحيح)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_حد_بندی(db, SQLITE_حد_بندی_کامہ_دھاگے, -1));
    اكسر;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  حالة Prag_Typ_ANALYSIS_حد_بندی: {
    sqlite3_int64 N;
    إذا( z_دایاں
     && sqlite3_Dec_Or_Hex_منزل_I64(z_دایاں, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->n_Analysis_حد_بندی = (صحيح)(N&0x7fffffff);
    }
    returnSingleInt(v, db->n_Analysis_حد_بندی); /* IMP: R-57594-65522 */
    اكسر;
  }

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  حالة Prag_Typ_جندرا_حالت: {
    ساكن ثابت حرف *ثابت az_جندرا_Name[] = {
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
         zState = az_جندرا_Name[j];
      }
      sqlite3_Vdbe_Multi_لدو(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    اكسر;
  }
#جے_انت

#إذا defined(SQLITE_ENABLE_CEROD)
  حالة PragTyp_ACTIVATE_EXTENSIONS: إذا( z_دایاں ){
    إذا( sqlite3StrNICmp(z_دایاں, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_دایاں[6]);
    }
  }
  اكسر;
#جے_انت

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  إذا( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_دایاں ){
    sqlite3_Vdbe_Verify_No_نتیجہ_صف(v);
  }

pragma_باہرلا:
  sqlite3DbFree(db, z_بایاں);
  sqlite3DbFree(db, z_دایاں);
}
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
تعريفنوع بنية PragmaVtab PragmaVtab;
تعريفنوع بنية PragmaVtabCursor PragmaVtabCursor;
بنية PragmaVtab {
  sqlite3_vtab ادھار;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ثابت PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
بنية PragmaVtabCursor {
  sqlite3_vtab_cursor ادھار; /* Base class.  Must be first */
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

  بن_ورتیا_PARAMETER(argc);
  بن_ورتیا_PARAMETER(argv);
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
  sqlite3_Str_Accum_انت(&acc);
  أكد( لڑی_لمبائی(zBuf) < حجم(zBuf)-1 );
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
ساكن صحيح pragma_Vtab_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ثابت بنية sqlite3_اشاریہ_constraint *pConstraint;
  صحيح i, j;
  صحيح تکیا[2];

  pIdxInfo->estimated_لاگت = (مزدوج)1;
  إذا( pTab->nHidden==0 ){ أرجع SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  تکیا[0] = 0;
  تکیا[1] = 0;
  لكل(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    إذا( pConstraint->i_کالم < pTab->iHidden ) استمر;
    إذا( pConstraint->op!=SQLITE_اشاریہ_CONSTRAINT_EQ ) استمر;
    إذا( pConstraint->usable==0 ) أرجع SQLITE_CONSTRAINT;
    j = pConstraint->i_کالم - pTab->iHidden;
    أكد( j < 2 );
    تکیا[j] = i+1;
  }
  إذا( تکیا[0]==0 ){
    pIdxInfo->estimated_لاگت = (مزدوج)2147483647;
    pIdxInfo->estimated_صفاں = 2147483647;
    أرجع SQLITE_OK;
  }
  j = تکیا[0]-1;
  pIdxInfo->aConstraintUsage[j].argv_اشاریہ = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimated_لاگت = (مزدوج)20;
  pIdxInfo->estimated_صفاں = 20;
  إذا( تکیا[1] ){
    j = تکیا[1]-1;
    pIdxInfo->aConstraintUsage[j].argv_اشاریہ = 2;
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
  pCsr->ادھار.pVtab = pVtab;
  *ppCursor = &pCsr->ادھار;
  أرجع SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ساكن فراغ pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  صحيح i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  لكل(i=0; i<Array_ناپ(pCsr->azArg); i++){
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
ساكن صحيح pragma_Vtab_اگلا(sqlite3_vtab_cursor *pVtabCursor){
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
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  صحيح rc;
  صحيح i, j;
  StrAccum acc;
  حرف *zSql;

  بن_ورتیا_PARAMETER(idx_عدد);
  بن_ورتیا_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  لكل(i=0; i<argc; i++, j++){
    ثابت حرف *z_متن = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
    أكد( j<Array_ناپ(pCsr->azArg) );
    أكد( pCsr->azArg[j]==0 );
    إذا( z_متن ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", z_متن);
      إذا( pCsr->azArg[j]==0 ){
        أرجع SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->a_حد_بندی[SQLITE_حد_بندی_SQL_لمائی]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  إذا( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  إذا( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3_Str_Accum_انت(&acc);
  إذا( zSql==0 ) أرجع SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  إذا( rc!=SQLITE_OK ){
    pTab->ادھار.z_Err_پیغام = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    أرجع rc;
  }
  أرجع pragma_Vtab_اگلا(pVtabCursor);
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
ساكن صحيح pragma_Vtab_کالم(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  صحيح i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  إذا( i<pTab->iHidden ){
    sqlite3_نتیجہ_قدر(ctx, sqlite3_کالم_قدر(pCsr->pPragma, i));
  }وإلا{
    sqlite3_نتیجہ_متن(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
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
  pragma_Vtab_بہترین_اشاریہ,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragma_Vtab_اگلا,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragma_Vtab_کالم,            /* xColumn - read data */
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
Module *sqlite3_Pragma_Vtab_رجسٹر(sqlite3 *db, ثابت حرف *zName){
  ثابت PragmaName *pName;
  أكد( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  إذا( pName==0 ) أرجع 0;
  إذا( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) أرجع 0;
  أكد( sqlite3_Hash_لبھو(&db->aModule, zName)==0 );
  أرجع sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (فراغ*)pName, 0);
}

#جے_انت /* SQLITE_OMIT_VIRTUALTABLE */

#جے_انت /* SQLITE_OMIT_PRAGMA */
