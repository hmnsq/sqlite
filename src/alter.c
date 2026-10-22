/*
** 2005 February 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that used to generate VDBE code
** that implements the ALTER TABLE command.
*/
#ضمن "sqliteInt.h"

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#جے_بنا_تعریف SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ساكن صحيح is_Alterable_جدول(Parse *pParse, جدول *pTab){
  إذا( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#جے_انت
  ){
    sqlite3_Error_پیغام(pParse, "table %s may not be altered", pTab->zName);
    أرجع 1;
  }
  أرجع 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ساكن فراغ renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ثابت حرف *zDb,                /* Name of db to verify schema of */
  صحيح b_عارضی,                      /* True if this is the temp db */
  ثابت حرف *zWhen,              /* "when" part of error message */
  صحيح bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->خانہ_ناں_سیٹ = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_جدول " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_عارضی, zWhen, bNoDQS
  );

  إذا( b_عارضی==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_جدول " "
        "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
        " AND sql NOT LIKE 'create virtual%%'"
        " AND sqlite_rename_test(%Q, sql, type, name, 1, %Q, %d)=NULL ",
        zDb, zWhen, bNoDQS
    );
  }
}

/*
** Generate VM code to replace any double-quoted strings (but not double-quoted
** identifiers) within the "sql" column of the sqlite_schema table in
** database zDb with their single-quoted equivalents. If argument bTemp is
** not true, similarly update all SQL statements in the sqlite_schema table
** of the temp db.
*/
ساكن فراغ renameFixQuotes(Parse *pParse, ثابت حرف *zDb, صحيح b_عارضی){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  إذا( b_عارضی==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_جدول
      " SET sql = sqlite_rename_quotefix('temp', sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
    );
  }
}

/*
** Generate code to reload the schema for database iDb. And, if iDb!=1, for
** the temp database as well.
*/
ساكن فراغ renameReloadSchema(Parse *pParse, صحيح iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  إذا( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_جوڑو_Parse_Schema_Op(pParse->pVdbe, iDb, 0, p5);
    إذا( iDb!=1 ) sqlite3_Vdbe_جوڑو_Parse_Schema_Op(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
فراغ sqlite3_Alter_Rename_جدول(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  صحيح iDb;                  /* Database that contains the table */
  حرف *zDb;                /* Name of database iDb */
  جدول *pTab;              /* Table being renamed */
  حرف *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  صحيح nTabName;             /* Number of UTF-8 characters in zTabName */
  ثابت حرف *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  إذا( NEVER(db->mallocFailed) ) اذهب exit_rename_جدول;
  أكد( pSrc->nSrc==1 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_rename_جدول;
  iDb = sqlite3_Schema_منزل_اشاریہ(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  إذا( !zName ) اذهب exit_rename_جدول;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  إذا( sqlite3_لبھو_جدول(db, zName, zDb)
   || sqlite3_لبھو_اشاریہ(db, zName, zDb)
   || sqlite3_Is_Shadow_جدول_دا(db, pTab, zName)
  ){
    sqlite3_Error_پیغام(pParse,
        "there is already another table or index with this name: %s", zName);
    اذهب exit_rename_جدول;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  إذا( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ){
    اذهب exit_rename_جدول;
  }
  إذا( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    اذهب exit_rename_جدول;
  }

#جے_بنا_تعریف SQLITE_OMIT_VIEW
  إذا( IsView(pTab) ){
    sqlite3_Error_پیغام(pParse, "view %s may not be altered", pTab->zName);
    اذهب exit_rename_جدول;
  }
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    اذهب exit_rename_جدول;
  }
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  إذا( sqlite3_View_Get_کالم_ناں(pParse, pTab) ){
    اذهب exit_rename_جدول;
  }
  إذا( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    إذا( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#جے_انت

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 ){
    اذهب exit_rename_جدول;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_جدول " SET "
          "tbl_name = %Q, "
          "name = CASE "
            "WHEN type='table' THEN %Q "
            "WHEN name LIKE 'sqliteX_autoindex%%' ESCAPE 'X' "
            "     AND type='index' THEN "
             "'sqlite_autoindex_' || %Q || substr(name,%d+18) "
            "ELSE name END "
      "WHERE tbl_name=%Q COLLATE nocase AND "
          "(type='table' OR type='index' OR type='trigger');",
      zDb,
      zName, zName, zName,
      nTabName, zTabName
  );

#جے_بنا_تعریف SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  إذا( sqlite3_لبھو_جدول(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#جے_انت

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  إذا( iDb!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE sqlite_temp_schema SET "
            "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, 1), "
            "tbl_name = "
              "CASE WHEN tbl_name=%Q COLLATE nocase AND "
              "  sqlite_rename_test(%Q, sql, type, name, 1, 'after rename', 0) "
              "THEN %Q ELSE tbl_name END "
            "WHERE type IN ('view', 'trigger')"
        , zDb, zTabName, zName, zTabName, zDb, zName);
  }

  /* If this is a virtual table, invoke the xRename() function if
  ** one is defined. The xRename() callback will modify the names
  ** of any resources used by the v-table implementation (including other
  ** SQLite tables) that are identified by the name of the virtual table.
  */
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  إذا( pVTab ){
    صحيح i = ++pParse->nMem;
    sqlite3_Vdbe_لدو_String(v, i, zName);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_VRename, i, 0, 0,(ثابت حرف*)pVTab, P4_VTAB);
  }
#جے_انت

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_جدول:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
ساكن فراغ sqlite3_Error_If_Not_خالی_تھاواں(
  Parse *pParse,        /* Parsing context */
  ثابت حرف *zDb,      /* Schema holding the table */
  ثابت حرف *zTab,     /* Table to check for empty */
  ثابت حرف *zErr      /* Error message text */
){
  sqlite3NestedParse(pParse,
     "SELECT raise(ABORT,%Q) FROM \"%w\".\"%w\"",
     zErr, zDb, zTab
  );
}

/*
** zCol is a column name used in an ALTER TABLE DROP, ADD or RENAME COLUMN
** operation. zOp identifies the specific operation - "drop", "add", "rename
** to" or "rename from". pTab is the table being altered.
**
** If pTab has a rowid and zCol is a rowid alias, then SQLITE_ERROR is 
** returned and an error message left in pParse. Or, if zCol is not an alias 
** for "rowid" or pTab is not an intkey table, then SQLITE_OK is returned.
*/
ساكن صحيح isRowidAlias(
  Parse *pParse, 
  جدول *pTab, 
  ثابت حرف *z_خانہ, 
  ثابت حرف *zOp
){
  إذا( HasRowid(pTab) && sqlite3IsRowid(z_خانہ) ){
    sqlite3_Error_پیغام(pParse, "cannot %s rowid alias: %s", zOp, z_خانہ);
    أرجع SQLITE_ERROR;
  }
  أرجع SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
فراغ sqlite3_Alter_انت_جوڑو_کالم(Parse *pParse, Token *p_خانہ_Def){
  جدول *pNew;              /* Copy of pParse->pNewTable */
  جدول *pTab;              /* Table being altered */
  صحيح iDb;                  /* Database number */
  ثابت حرف *zDb;          /* Database name */
  ثابت حرف *zTab;         /* Table name */
  حرف *z_خانہ;               /* Null-terminated column definition */
  کالم *p_خانہ;             /* The new column */
  اظہاریہ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  صحيح r1;                   /* Temporary registers */

  db = pParse->db;
  أكد( db->pParse==pParse );
  إذا( pParse->nErr ) أرجع;
  أكد( db->mallocFailed==0 );
  pNew = pParse->p_New_جدول;
  أكد( pNew );

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_منزل_اشاریہ(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  p_خانہ = &pNew->a_خانہ[pNew->n_خانہ-1];
  pDflt = sqlite3_کالم_اظہاریہ(pNew, p_خانہ);
  pTab = sqlite3_لبھو_جدول(db, zTab, zDb);
  أكد( pTab );

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    أرجع;
  }
#جے_انت


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  إذا( p_خانہ->خانہ_Flags & COLFLAG_PRIMKEY ){
    sqlite3_Error_پیغام(pParse, "Cannot add a PRIMARY KEY column");
    أرجع;
  }
  إذا( pNew->p_اشاریہ ){
    sqlite3_Error_پیغام(pParse,
         "Cannot add a UNIQUE column");
    أرجع;
  }
  إذا( isRowidAlias(pParse, pTab, p_خانہ->zCnName, "add") ) أرجع;
  إذا( (p_خانہ->خانہ_Flags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    أكد( pDflt==0 || pDflt->op==TK_وسعت );
    إذا( pDflt && pDflt->p_بایاں->op==TK_NULL ){
      pDflt = 0;
    }
    أكد( Is_Ordinary_جدول(pNew) );
    إذا( (db->flags&SQLITE_Foreign_کلیدیں) && pNew->u.tab.pFKey && pDflt ){
      sqlite3_Error_If_Not_خالی_تھاواں(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    إذا( p_خانہ->notNull && !pDflt ){
      sqlite3_Error_If_Not_خالی_تھاواں(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    إذا( pDflt ){
      sqlite3_قدر *pVal = 0;
      صحيح rc;
      rc = sqlite3_قدر_From_اظہاریہ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      أكد( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      إذا( rc!=SQLITE_OK ){
        أكد( db->mallocFailed == 1 );
        أرجع;
      }
      إذا( !pVal ){
        sqlite3_Error_If_Not_خالی_تھاواں(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_قدر_Free(pVal);
    }
  }وإلا إذا( p_خانہ->خانہ_Flags & COLFLAG_STORED ){
    sqlite3_Error_If_Not_خالی_تھاواں(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  z_خانہ = sqlite3DbStrNDup(db, (حرف*)p_خانہ_Def->z, p_خانہ_Def->n);
  إذا( z_خانہ ){
    حرف *z_سرا = &z_خانہ[p_خانہ_Def->n-1];
    طالما( z_سرا>z_خانہ && (*z_سرا==';' || sqlite3Isspace(*z_سرا)) ){
      *z_سرا-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    أكد( Is_Ordinary_جدول(pTab) );
    أكد( Is_Ordinary_جدول(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.جوڑو_خانہ_وتھ, z_خانہ, pNew->u.tab.جوڑو_خانہ_وتھ,
      zTab
    );
    sqlite3DbFree(db, z_خانہ);
  }

  v = sqlite3GetVdbe(pParse);
  إذا( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_عارضی_Reg(pParse);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3_Vdbe_ورتدا_Btree(v, iDb);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, r1, -2);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_سیٹ_Cookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_عارضی_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_Alter_جوڑو);

    /* Verify that constraints are still satisfied */
    إذا( pNew->pCheck!=0
     || (p_خانہ->notNull && (p_خانہ->خانہ_Flags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->خانہ_ناں_سیٹ = 1;
      sqlite3NestedParse(pParse,
        "SELECT CASE WHEN quick_check GLOB 'CHECK*'"
        " THEN raise(ABORT,'CHECK constraint failed')"
        " WHEN quick_check GLOB 'non-* value in*'"
        " THEN raise(ABORT,'type mismatch on DEFAULT')"
        " ELSE raise(ABORT,'NOT NULL constraint failed')"
        " END"
        "  FROM pragma_quick_check(%Q,%Q)"
        " WHERE quick_check GLOB 'CHECK*'"
        " OR quick_check GLOB 'NULL*'"
        " OR quick_check GLOB 'non-* value in*'",
        zTab, zDb
      );
    }
  }
}

/*
** This function is called by the parser after the table-name in
** an "ALTER TABLE <table-name> ADD" statement is parsed. Argument
** pSrc is the full-name of the table being altered.
**
** This routine makes a (partial) copy of the Table structure
** for the table being altered and sets Parse.pNewTable to point
** to it. Routines called by the parser as the column definition
** is parsed (i.e. sqlite3AddColumn()) add the new Column data to
** the copy. The copy of the Table structure is deleted by tokenize.c
** after parsing is finished.
**
** Routine sqlite3AlterFinishAddColumn() will be called to complete
** coding the "ALTER TABLE ... ADD" statement.
*/
فراغ sqlite3_Alter_Begin_جوڑو_کالم(Parse *pParse, SrcList *pSrc){
  جدول *pNew;
  جدول *pTab;
  صحيح iDb;
  صحيح i;
  صحيح nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  أكد( pParse->p_New_جدول==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  إذا( NEVER(db->mallocFailed) ) اذهب exit_begin_جوڑو_کالم;
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_begin_جوڑو_کالم;

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  إذا( IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "virtual tables may not be altered");
    اذهب exit_begin_جوڑو_کالم;
  }
#جے_انت

  /* Make sure this is not an attempt to ALTER a view. */
  إذا( IsView(pTab) ){
    sqlite3_Error_پیغام(pParse, "Cannot add a column to a view");
    اذهب exit_begin_جوڑو_کالم;
  }
  إذا( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ){
    اذهب exit_begin_جوڑو_کالم;
  }

  sqlite3MayAbort(pParse);
  أكد( Is_Ordinary_جدول(pTab) );
  أكد( pTab->u.tab.جوڑو_خانہ_وتھ>0 );
  iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (جدول*)sqlite3DbMallocZero(db, حجم(جدول));
  إذا( !pNew ) اذهب exit_begin_جوڑو_کالم;
  pParse->p_New_جدول = pNew;
  pNew->nTabRef = 1;
  pNew->n_خانہ = pTab->n_خانہ;
  أكد( pNew->n_خانہ>0 );
  nAlloc = (((pNew->n_خانہ-1)/8)*8)+8;
  أكد( nAlloc>=pNew->n_خانہ && nAlloc%8==0 && nAlloc-pNew->n_خانہ<8 );
  pNew->a_خانہ = (کالم*)sqlite3DbMallocZero(db, حجم(کالم)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  إذا( !pNew->a_خانہ || !pNew->zName ){
    أكد( db->mallocFailed );
    اذهب exit_begin_جوڑو_کالم;
  }
  حافظہ_نقل(pNew->a_خانہ, pTab->a_خانہ, حجم(کالم)*(size_t)pNew->n_خانہ);
  لكل(i=0; i<pNew->n_خانہ; i++){
    کالم *p_خانہ = &pNew->a_خانہ[i];
    p_خانہ->zCnName = sqlite3DbStrDup(db, p_خانہ->zCnName);
    p_خانہ->hName = sqlite3StrIHash(p_خانہ->zCnName);
  }
  أكد( Is_Ordinary_جدول(pNew) );
  pNew->u.tab.pDfltList = sqlite3_اظہاریہ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.جوڑو_خانہ_وتھ = pTab->u.tab.جوڑو_خانہ_وتھ;
  أكد( pNew->nTabRef==1 );

exit_begin_جوڑو_کالم:
  sqlite3SrcListDelete(db, pSrc);
  أرجع;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ساكن صحيح is_Real_جدول(Parse *pParse, جدول *pTab, صحيح iOp){
  ثابت حرف *zType = 0;
#جے_بنا_تعریف SQLITE_OMIT_VIEW
  إذا( IsView(pTab) ){
    zType = "view";
  }
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  إذا( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#جے_انت
  إذا( zType ){
    ثابت حرف *az_پیغام[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    أكد( iOp>=0 && iOp<Array_ناپ(az_پیغام) );
    sqlite3_Error_پیغام(pParse, "cannot %s %s \"%s\"",
        az_پیغام[iOp], zType, pTab->zName
    );
    أرجع 1;
  }
  أرجع 0;
}
#وإلا /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define is_Real_جدول(x,y,z) (0)
#جے_انت

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
فراغ sqlite3_Alter_Rename_کالم(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  جدول *pTab;                    /* Table being updated */
  صحيح i_خانہ;                       /* Index of column being renamed */
  حرف *zOld = 0;                 /* Old column name */
  حرف *zNew = 0;                 /* New column name */
  ثابت حرف *zDb;                /* Name of schema containing the table */
  صحيح iSchema;                    /* Index of the schema */
  صحيح bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_rename_کالم;

  /* Cannot alter a system table */
  إذا( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ) اذهب exit_rename_کالم;
  إذا( SQLITE_OK!=is_Real_جدول(pParse, pTab, 0) ) اذهب exit_rename_کالم;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
  أكد( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    اذهب exit_rename_کالم;
  }
#جے_انت

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  إذا( !zOld ) اذهب exit_rename_کالم;
  i_خانہ = sqlite3_کالم_اشاریہ(pTab, zOld);
  إذا( i_خانہ<0 ){
    sqlite3_Error_پیغام(pParse, "no such column: \"%T\"", pOld);
    اذهب exit_rename_کالم;
  }

  /* Ensure the schema contains no double-quoted strings */
  renameTestSchema(pParse, zDb, iSchema==1, "", 0);
  renameFixQuotes(pParse, zDb, iSchema==1);

  /* Do the rename operation using a recursive UPDATE statement that
  ** uses the sqlite_rename_column() SQL function to compute the new
  ** CREATE statement text for the sqlite_schema table.
  */
  sqlite3MayAbort(pParse);
  zNew = sqlite3NameFromToken(db, pNew);
  إذا( !zNew ) اذهب exit_rename_کالم;
  إذا( isRowidAlias(pParse, pTab, zOld, "rename from") ) اذهب exit_rename_کالم;
  إذا( isRowidAlias(pParse, pTab, zNew, "rename to") ) اذهب exit_rename_کالم;
  أكد( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, i_خانہ, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  إذا( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_جدول " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, i_خانہ, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_کالم:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  أرجع;
}

/*
** Each RenameToken object maps an element of the parse tree into
** the token that generated that element.  The parse tree element
** might be one of:
**
**     *  A pointer to an Expr that represents an ID
**     *  The name of a table column in Column.zName
**
** A list of RenameToken objects can be constructed during parsing.
** Each new object is created by sqlite3RenameTokenMap().
** As the parse tree is transformed, the sqlite3RenameTokenRemap()
** routine is used to keep the mapping current.
**
** After the parse finishes, renameTokenFind() routine can be used
** to look up the actual token value that created some element in
** the parse tree.
*/
بنية RenameToken {
  ثابت فراغ *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_اگلا;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
تعريفنوع بنية RenameCtx RenameCtx;
بنية RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  صحيح nList;                      /* Number of tokens in pList */
  صحيح i_خانہ;                       /* Index of column being renamed */
  جدول *pTab;                    /* Table being ALTERed */
  ثابت حرف *zOld;               /* Old column name */
};

#جے_تعریف_شدہ SQLITE_DEBUG
/*
** This function is only for debugging. It performs two tasks:
**
**   1. Checks that pointer pPtr does not already appear in the
**      rename-token list.
**
**   2. Dereferences each pointer in the rename-token list.
**
** The second is most effective when debugging under valgrind or
** address-sanitizer or similar. If any of these pointers no longer
** point to valid objects, an exception is raised by the memory-checking
** tool.
**
** The point of this is to prevent comparisons of invalid pointer values.
** Even though this always seems to work, it is undefined according to the
** C standard. Example of undefined comparison:
**
**     sqlite3_free(x);
**     if( x==y ) ...
**
** Technically, as x no longer points into a valid object or to the byte
** following a valid object, it may not be used in comparison operations.
*/
ساكن فراغ renameTokenCheckAll(Parse *pParse, ثابت فراغ *pPtr){
  أكد( pParse==pParse->db->pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  إذا( pParse->nErr==0 ){
    ثابت RenameToken *p;
    u32 i = 1;
    لكل(p=pParse->pRename; p; p=p->p_اگلا){
      إذا( p->p ){
        أكد( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    أكد( i>0 );
  }
}
#وإلا
# define renameTokenCheckAll(x,y)
#جے_انت

/*
** Remember that the parser tree element pPtr was created using
** the token pToken.
**
** In other words, construct a new RenameToken object and add it
** to the list of RenameToken objects currently being built up
** in pParse->pRename.
**
** The pPtr argument is returned so that this routine can be used
** with tail recursion in tokenExpr() routine, for a small performance
** improvement.
*/
ثابت فراغ *sqlite3RenameTokenMap(
  Parse *pParse,
  ثابت فراغ *pPtr,
  ثابت Token *pToken
){
  RenameToken *pNew;
  أكد( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  إذا( ALWAYS(pParse->e_Parse_منوال!=PARSE_منوال_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, حجم(RenameToken));
    إذا( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_اگلا = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  أرجع pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
فراغ sqlite3RenameTokenRemap(Parse *pParse, ثابت فراغ *p_منزل, ثابت فراغ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, p_منزل);
  لكل(p=pParse->pRename; p; p=p->p_اگلا){
    إذا( p->p==pFrom ){
      p->p = p_منزل;
      اكسر;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ساكن صحيح rename_Unmap_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ثابت فراغ*)p_اظہاریہ);
  إذا( اظہاریہ_Use_YTab(p_اظہاریہ) ){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت فراغ*)&p_اظہاریہ->y.pTab);
  }
  أرجع WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ساكن فراغ renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  إذا( pWith ){
    Parse *pParse = pWalker->pParse;
    صحيح i;
    With *pCopy = 0;
    أكد( pWith->nCte>0 );
    إذا( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ڈالو(pParse, pCopy, 1);
    }
    لكل(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, حجم(sNC));
      sNC.pParse = pParse;
      إذا( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      إذا( sNC.pParse->db->mallocFailed ) أرجع;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].p_خانے_گنتی);
    }
    إذا( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ساكن فراغ unmap_کالم_Idlist_ناں(
  Parse *pParse,
  ثابت IdList *pIdList
){
  صحيح ii;
  أكد( pIdList!=0 );
  لكل(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت فراغ*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ساكن صحيح renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  صحيح i;
  إذا( pParse->nErr ) أرجع WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  إذا( p->selFlags & (SF_View|SF_CopyCte) ){
    أرجع WRC_Prune;
  }
  إذا( ALWAYS(p->pEList) ){
    اظہاریہ_List *pList = p->pEList;
    لكل(i=0; i<pList->n_اظہاریہ; i++){
      إذا( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (فراغ*)pList->a[i].zEName);
      }
    }
  }
  إذا( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    لكل(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (فراغ*)pSrc->a[i].zName);
      إذا( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_اظہاریہ(pWalker, pSrc->a[i].u3.pOn);
      }وإلا{
        unmap_کالم_Idlist_ناں(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  أرجع WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
فراغ sqlite3_Rename_اظہاریہ_Unmap(Parse *pParse, اظہاریہ *p_اظہاریہ){
  u8 e_منوال = pParse->e_Parse_منوال;
  Walker sWalker;
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = pParse;
  sWalker.x_اظہاریہ_Callback = rename_Unmap_اظہاریہ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->e_Parse_منوال = PARSE_منوال_UNMAP;
  sqlite3_Walk_اظہاریہ(&sWalker, p_اظہاریہ);
  pParse->e_Parse_منوال = e_منوال;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
فراغ sqlite3RenameExprlistUnmap(Parse *pParse, اظہاریہ_List *pEList){
  إذا( pEList ){
    صحيح i;
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = pParse;
    sWalker.x_اظہاریہ_Callback = rename_Unmap_اظہاریہ_Cb;
    sqlite3_Walk_اظہاریہ_List(&sWalker, pEList);
    لكل(i=0; i<pEList->n_اظہاریہ; i++){
      إذا( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (فراغ*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ساكن فراغ renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_اگلا;
  RenameToken *p;
  لكل(p=pToken; p; p=p_اگلا){
    p_اگلا = p->p_اگلا;
    sqlite3DbFree(db, p);
  }
}

/*
** Search the Parse object passed as the first argument for a RenameToken
** object associated with parse tree element pPtr. If found, return a pointer
** to it. Otherwise, return NULL.
**
** If the second argument passed to this function is not NULL and a matching
** RenameToken object is found, remove it from the Parse object and add it to
** the list maintained by the RenameCtx object.
*/
ساكن RenameToken *rename_Token_لبھو(
  Parse *pParse,
  بنية RenameCtx *pCtx,
  ثابت فراغ *pPtr
){
  RenameToken **pp;
  إذا( NEVER(pPtr==0) ){
    أرجع 0;
  }
  لكل(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_اگلا){
    إذا( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      إذا( pCtx ){
        *pp = pToken->p_اگلا;
        pToken->p_اگلا = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      أرجع pToken;
    }
  }
  أرجع 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ساكن صحيح rename_کالم_Select_Cb(Walker *pWalker, Select *p){
  إذا( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    أرجع WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  أرجع WRC_Continue;
}

/*
** This is a Walker expression callback.
**
** For every TK_COLUMN node in the expression tree, search to see
** if the column being references is the column being renamed by an
** ALTER TABLE statement.  If it is, then attach its associated
** RenameToken object to the list of RenameToken objects being
** constructed in RenameCtx object at pWalker->u.pRename.
*/
ساكن صحيح rename_کالم_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  RenameCtx *p = pWalker->u.pRename;
  إذا( p_اظہاریہ->op==TK_TRIGGER
   && p_اظہاریہ->i_کالم==p->i_خانہ
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    rename_Token_لبھو(pWalker->pParse, p, (فراغ*)p_اظہاریہ);
  }وإلا إذا( p_اظہاریہ->op==TK_کالم
   && p_اظہاریہ->i_کالم==p->i_خانہ
   && ALWAYS(اظہاریہ_Use_YTab(p_اظہاریہ))
   && p->pTab==p_اظہاریہ->y.pTab
  ){
    rename_Token_لبھو(pWalker->pParse, p, (فراغ*)p_اظہاریہ);
  }
  أرجع WRC_Continue;
}

/*
** The RenameCtx contains a list of tokens that reference a column that
** is being renamed by an ALTER TABLE statement.  Return the "last"
** RenameToken in the RenameCtx and remove that RenameToken from the
** RenameContext.  "Last" means the last RenameToken encountered when
** the input SQL is parsed from left to right.  Repeated calls to this routine
** return all column name tokens in the order that they are encountered
** in the SQL statement.
*/
ساكن RenameToken *rename_کالم_Token_اگلا(RenameCtx *pCtx){
  RenameToken *p_بہترین = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  لكل(pToken=p_بہترین->p_اگلا; pToken; pToken=pToken->p_اگلا){
    إذا( pToken->t.z>p_بہترین->t.z ) p_بہترین = pToken;
  }
  لكل(pp=&pCtx->pList; *pp!=p_بہترین; pp=&(*pp)->p_اگلا);
  *pp = p_بہترین->p_اگلا;

  أرجع p_بہترین;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ساكن فراغ errorMPrintf(sqlite3_context *pCtx, ثابت حرف *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  حرف *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  إذا( zErr ){
    sqlite3_نتیجہ_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }وإلا{
    sqlite3_نتیجہ_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
ساكن فراغ rename_کالم_Parse_Error(
  sqlite3_context *pCtx,
  ثابت حرف *zWhen,
  sqlite3_قدر *pType,
  sqlite3_قدر *pObject,
  Parse *pParse
){
  ثابت حرف *zT = (ثابت حرف*)sqlite3_قدر_متن(pType);
  ثابت حرف *zN = (ثابت حرف*)sqlite3_قدر_متن(pObject);
  حرف *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_پیغام
  );
  sqlite3_نتیجہ_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
ساكن فراغ rename_کالم_Elist_ناں(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت اظہاریہ_List *pEList,
  ثابت حرف *zOld
){
  إذا( pEList ){
    صحيح i;
    لكل(i=0; i<pEList->n_اظہاریہ; i++){
      ثابت حرف *zName = pEList->a[i].zEName;
      إذا( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        rename_Token_لبھو(pParse, pCtx, (ثابت فراغ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ساكن فراغ rename_کالم_Idlist_ناں(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت IdList *pIdList,
  ثابت حرف *zOld
){
  إذا( pIdList ){
    صحيح i;
    لكل(i=0; i<pIdList->nId; i++){
      ثابت حرف *zName = pIdList->a[i].zName;
      إذا( 0==sqlite3_stricmp(zName, zOld) ){
        rename_Token_لبھو(pParse, pCtx, (ثابت فراغ*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ساكن صحيح renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ثابت حرف *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ثابت حرف *zSql,               /* SQL to parse */
  صحيح b_عارضی                       /* True if SQL is from temp schema */
){
  صحيح rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  إذا( zSql==0 ){
    أرجع SQLITE_NOMEM;
  }
  إذا( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( b_عارضی ){
    db->init.iDb = 1;
  }وإلا{
    صحيح iDb = sqlite3_لبھو_Db_Name(db, zDb);
    أكد( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->e_Parse_منوال = PARSE_منوال_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && لڑی_کھوج(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3_لگاتار_Parser(p, zSql);
  db->flags = flags;
  إذا( db->mallocFailed ) rc = SQLITE_NOMEM;
  إذا( rc==SQLITE_OK
   && NEVER(p->p_New_جدول==0 && p->p_New_اشاریہ==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#جے_تعریف_شدہ SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  إذا( rc==SQLITE_OK ){
    صحيح nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    لكل(pToken=p->pRename; pToken; pToken=pToken->p_اگلا){
      أكد( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#جے_انت

  db->init.iDb = 0;
  أرجع rc;
}

/*
** This function edits SQL statement zSql, replacing each token identified
** by the linked list pRename with the text of zNew. If argument bQuote is
** true, then zNew is always quoted first. If no error occurs, the result
** is loaded into context object pCtx as the result.
**
** Or, if an error occurs (i.e. an OOM condition), an error is left in
** pCtx and an SQLite error code returned.
*/
ساكن صحيح renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ثابت حرف *zSql,               /* SQL statement to edit */
  ثابت حرف *zNew,               /* New token text */
  صحيح bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  صحيح rc = SQLITE_OK;
  حرف *zQuot = 0;
  حرف *z_باہرلا;
  i64 nQuot = 0;
  حرف *zBuf1 = 0;
  حرف *zBuf2 = 0;

  إذا( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    إذا( zQuot==0 ){
      أرجع SQLITE_NOMEM;
    }وإلا{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    أكد( nQuot>=nNew && nSql>=0 && nNew>=0 );
    z_باہرلا = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }وإلا{
    أكد( nSql>0 );
    z_باہرلا = (حرف*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    إذا( z_باہرلا ){
      zBuf1 = &z_باہرلا[nSql*2+1];
      zBuf2 = &z_باہرلا[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  إذا( z_باہرلا ){
    i64 n_باہرلا = nSql;
    أكد( nSql>0 );
    حافظہ_نقل(z_باہرلا, zSql, (size_t)nSql);
    طالما( pRename->pList ){
      صحيح iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ثابت حرف *zReplace;
      RenameToken *p_بہترین = rename_کالم_Token_اگلا(pRename);

      إذا( zNew ){
        إذا( bQuote==0 && sqlite3IsIdChar(*(u8*)p_بہترین->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }وإلا{
          nReplace = nQuot;
          zReplace = zQuot;
          إذا( p_بہترین->t.z[p_بہترین->t.n]=='"' ) nReplace++;
        }
      }وإلا{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        حافظہ_نقل(zBuf1, p_بہترین->t.z, p_بہترین->t.n);
        zBuf1[p_بہترین->t.n] = 0;
        sqlite3Dequote(zBuf1);
        أكد( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((صحيح)(nSql*2), zBuf2, "%Q%s", zBuf1,
            p_بہترین->t.z[p_بہترین->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (صحيح)(p_بہترین->t.z - zSql);
      إذا( p_بہترین->t.n!=nReplace ){
        memmove(&z_باہرلا[iOff + nReplace], &z_باہرلا[iOff + p_بہترین->t.n],
            n_باہرلا - (iOff + p_بہترین->t.n)
        );
        n_باہرلا += nReplace - p_بہترین->t.n;
        z_باہرلا[n_باہرلا] = '\0';
      }
      حافظہ_نقل(&z_باہرلا[iOff], zReplace, nReplace);
      sqlite3DbFree(db, p_بہترین);
    }

    sqlite3_نتیجہ_متن(pCtx, z_باہرلا, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, z_باہرلا);
  }وإلا{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  أرجع rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ساكن فراغ rename_سیٹ_ENames(اظہاریہ_List *pEList, صحيح val){
  أكد( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_وسعت );
  إذا( pEList ){
    صحيح i;
    لكل(i=0; i<pEList->n_اظہاریہ; i++){
      أكد( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
      pEList->a[i].fg.eEName = val&0x3;
    }
  }
}

/*
** Resolve all symbols in the trigger at pParse->pNewTrigger, assuming
** it was read from the schema of database zDb. Return SQLITE_OK if
** successful. Otherwise, return an SQLite error code and leave an error
** message in the Parse object.
*/
ساكن صحيح renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  Trigger_قدم *p_قدم;
  NameContext sNC;
  صحيح rc = SQLITE_OK;

  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  أكد( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3_لبھو_جدول(db, pNew->جدول_2,
      db->aDb[sqlite3_Schema_منزل_اشاریہ(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  إذا( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3_View_Get_کالم_ناں(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  إذا( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_اظہاریہ_ناں(&sNC, pNew->pWhen);
  }

  لكل(p_قدم=pNew->قدم_list; rc==SQLITE_OK && p_قدم; p_قدم=p_قدم->p_اگلا){
    إذا( p_قدم->pSelect ){
      sqlite3SelectPrep(pParse, p_قدم->pSelect, &sNC);
      إذا( pParse->nErr ) rc = pParse->rc;
    }
    إذا( rc==SQLITE_OK && p_قدم->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, p_قدم->pSrc, 0);
      إذا( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, p_قدم->p_اظہاریہ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        إذا( pSel==0 ){
          p_قدم->p_اظہاریہ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }وإلا{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          rename_سیٹ_ENames(p_قدم->p_اظہاریہ_List, ENAME_وسعت);
          sqlite3SelectPrep(pParse, pSel, 0);
          rename_سیٹ_ENames(p_قدم->p_اظہاریہ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          أكد( p_قدم->p_اظہاریہ_List==0 || p_قدم->p_اظہاریہ_List==pSel->pEList );
          أكد( pSrc==pSel->pSrc );
          إذا( p_قدم->p_اظہاریہ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        إذا( ALWAYS(p_قدم->pSrc) ){
          صحيح i;
          لكل(i=0; i<p_قدم->pSrc->nSrc && rc==SQLITE_OK; i++){
            Src_شے *p = &p_قدم->pSrc->a[i];
            إذا( p->fg.isSubquery ){
              أكد( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        إذا(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        إذا( rc==SQLITE_OK && p_قدم->pWhere ){
          rc = sqlite3_Resolve_اظہاریہ_ناں(&sNC, p_قدم->pWhere);
        }
        إذا( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_اظہاریہ_List_ناں(&sNC, p_قدم->p_اظہاریہ_List);
        }
        أكد( !p_قدم->pUpsert || (!p_قدم->pWhere && !p_قدم->p_اظہاریہ_List) );
        إذا( p_قدم->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = p_قدم->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_اظہاریہ_List_ناں(&sNC, pUpsert->p_Upsert_نشانہ);
          إذا( rc==SQLITE_OK ){
            اظہاریہ_List *p_Upsert_سیٹ = pUpsert->p_Upsert_سیٹ;
            rc = sqlite3_Resolve_اظہاریہ_List_ناں(&sNC, p_Upsert_سیٹ);
          }
          إذا( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_اظہاریہ_ناں(&sNC, pUpsert->pUpsertWhere);
          }
          إذا( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_اظہاریہ_ناں(&sNC, pUpsert->p_Upsert_نشانہ_Where);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }وإلا{
        rc = SQLITE_NOMEM;
      }
    }
  }
  أرجع rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ساكن فراغ renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  Trigger_قدم *p_قدم;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_اظہاریہ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  لكل(p_قدم=pTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
    sqlite3WalkSelect(pWalker, p_قدم->pSelect);
    sqlite3_Walk_اظہاریہ(pWalker, p_قدم->pWhere);
    sqlite3_Walk_اظہاریہ_List(pWalker, p_قدم->p_اظہاریہ_List);
    إذا( p_قدم->pUpsert ){
      Upsert *pUpsert = p_قدم->pUpsert;
      sqlite3_Walk_اظہاریہ_List(pWalker, pUpsert->p_Upsert_نشانہ);
      sqlite3_Walk_اظہاریہ_List(pWalker, pUpsert->p_Upsert_سیٹ);
      sqlite3_Walk_اظہاریہ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_اظہاریہ(pWalker, pUpsert->p_Upsert_نشانہ_Where);
    }
    إذا( p_قدم->pSrc ){
      صحيح i;
      SrcList *pSrc = p_قدم->pSrc;
      لكل(i=0; i<pSrc->nSrc; i++){
        إذا( pSrc->a[i].fg.isSubquery ){
          أكد( pSrc->a[i].u4.pSubq!=0 );
          sqlite3WalkSelect(pWalker, pSrc->a[i].u4.pSubq->pSelect);
        }
      }
    }
  }
}

/*
** Free the contents of Parse object (*pParse). Do not free the memory
** occupied by the Parse object itself.
*/
ساكن فراغ renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  اشاریہ *pIdx;
  إذا( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3_Delete_جدول(db, pParse->p_New_جدول);
  طالما( (pIdx = pParse->p_New_اشاریہ)!=0 ){
    pParse->p_New_اشاریہ = pIdx->p_اگلا;
    sqlite3_Free_اشاریہ(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_پیغام);
  renameTokenFree(db, pParse->pRename);
  sqlite3ParseObjectReset(pParse);
}

/*
** SQL function:
**
**     sqlite_rename_column(SQL,TYPE,OBJ,DB,TABLE,COL,NEWNAME,QUOTE,TEMP)
**
**   0. zSql:     SQL statement to rewrite
**   1. type:     Type of object ("table", "view" etc.)
**   2. object:   Name of object
**   3. Database: Database name (e.g. "main")
**   4. Table:    Table name
**   5. iCol:     Index of column to rename
**   6. zNew:     New column name
**   7. bQuote:   Non-zero if the new column name should be quoted.
**   8. bTemp:    True if zSql comes from temp schema
**
** Do a column rename operation on the CREATE statement given in zSql.
** The iCol-th column (left-most is 0) of table zTable is renamed from zCol
** into zNew.  The name should be quoted if bQuote is true.
**
** This function is used internally by the ALTER TABLE RENAME COLUMN command.
** It is only accessible to SQL created using sqlite3NestedParse().  It is
** not reachable from ordinary SQL passed into sqlite3_prepare() unless the
** SQLITE_TESTCTRL_INTERNAL_FUNCTIONS test setting is enabled.
*/
ساكن فراغ rename_کالم_Func(
  sqlite3_context *context,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[3]);
  ثابت حرف *z_جدول = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
  صحيح i_خانہ = sqlite3_قدر_int(argv[5]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_قدر_متن(argv[6]);
  صحيح bQuote = sqlite3_قدر_int(argv[7]);
  صحيح b_عارضی = sqlite3_قدر_int(argv[8]);
  ثابت حرف *zOld;
  صحيح rc;
  Parse sParse;
  Walker sWalker;
  اشاریہ *pIdx;
  صحيح i;
  جدول *pTab;
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#جے_انت

  بن_ورتیا_PARAMETER(Not_ورتیا);
  إذا( zSql==0 ) أرجع;
  إذا( z_جدول==0 ) أرجع;
  إذا( zNew==0 ) أرجع;
  إذا( i_خانہ<0 ) أرجع;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3_لبھو_جدول(db, z_جدول, zDb);
  إذا( pTab==0 || i_خانہ>=pTab->n_خانہ ){
    sqlite3BtreeLeaveAll(db);
    أرجع;
  }
  zOld = pTab->a_خانہ[i_خانہ].zCnName;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.i_خانہ = ((i_خانہ==pTab->iPKey) ? -1 : i_خانہ);

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#جے_انت
  rc = renameParseSql(&sParse, zDb, db, zSql, b_عارضی);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_اظہاریہ_Callback = rename_کالم_اظہاریہ_Cb;
  sWalker.xSelectCallback = rename_کالم_Select_Cb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  إذا( rc!=SQLITE_OK ) اذهب rename_کالم_Func_پورا;
  إذا( sParse.p_New_جدول ){
    إذا( IsView(sParse.p_New_جدول) ){
      Select *pSelect = sParse.p_New_جدول->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      إذا( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      إذا( rc!=SQLITE_OK ) اذهب rename_کالم_Func_پورا;
    }وإلا إذا( Is_Ordinary_جدول(sParse.p_New_جدول) ){
      /* A regular table */
      صحيح bFKOnly = sqlite3_stricmp(z_جدول, sParse.p_New_جدول->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.p_New_جدول;
      إذا( bFKOnly==0 ){
        إذا( i_خانہ<sParse.p_New_جدول->n_خانہ ){
          rename_Token_لبھو(
              &sParse, &sCtx, (فراغ*)sParse.p_New_جدول->a_خانہ[i_خانہ].zCnName
          );
        }
        إذا( sCtx.i_خانہ<0 ){
          rename_Token_لبھو(&sParse, &sCtx, (فراغ*)&sParse.p_New_جدول->iPKey);
        }
        sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_جدول->pCheck);
        لكل(pIdx=sParse.p_New_جدول->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          sqlite3_Walk_اظہاریہ_List(&sWalker, pIdx->a_خانہ_اظہاریہ);
        }
        لكل(pIdx=sParse.p_New_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          sqlite3_Walk_اظہاریہ_List(&sWalker, pIdx->a_خانہ_اظہاریہ);
        }
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
        لكل(i=0; i<sParse.p_New_جدول->n_خانہ; i++){
          اظہاریہ *p_اظہاریہ = sqlite3_کالم_اظہاریہ(sParse.p_New_جدول,
                                                  &sParse.p_New_جدول->a_خانہ[i]);
          sqlite3_Walk_اظہاریہ(&sWalker, p_اظہاریہ);
        }
#جے_انت
      }

      أكد( Is_Ordinary_جدول(sParse.p_New_جدول) );
      لكل(pFKey=sParse.p_New_جدول->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
        لكل(i=0; i<pFKey->n_خانہ; i++){
          إذا( bFKOnly==0 && pFKey->a_خانہ[i].iFrom==i_خانہ ){
            rename_Token_لبھو(&sParse, &sCtx, (فراغ*)&pFKey->a_خانہ[i]);
          }
          إذا( 0==sqlite3_stricmp(pFKey->z_منزل, z_جدول)
           && 0==sqlite3_stricmp(pFKey->a_خانہ[i].z_خانہ, zOld)
          ){
            rename_Token_لبھو(&sParse, &sCtx, (فراغ*)pFKey->a_خانہ[i].z_خانہ);
          }
        }
      }
    }
  }وإلا إذا( sParse.p_New_اشاریہ ){
    sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_اشاریہ->a_خانہ_اظہاریہ);
    sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
  }وإلا{
    /* A trigger */
    Trigger_قدم *p_قدم;
    rc = renameResolveTrigger(&sParse);
    إذا( rc!=SQLITE_OK ) اذهب rename_کالم_Func_پورا;

    لكل(p_قدم=sParse.pNewTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
      إذا( p_قدم->pSrc ){
        جدول *p_نشانہ = sqlite3_Locate_جدول_شے(&sParse, 0, &p_قدم->pSrc->a[0]);
        إذا( p_نشانہ==pTab ){
          إذا( p_قدم->pUpsert ){
            اظہاریہ_List *p_Upsert_سیٹ = p_قدم->pUpsert->p_Upsert_سیٹ;
            rename_کالم_Elist_ناں(&sParse, &sCtx, p_Upsert_سیٹ, zOld);
          }
          rename_کالم_Idlist_ناں(&sParse, &sCtx, p_قدم->pIdList, zOld);
          rename_کالم_Elist_ناں(&sParse, &sCtx, p_قدم->p_اظہاریہ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    إذا( sParse.pTriggerTab==pTab ){
      rename_کالم_Idlist_ناں(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  أكد( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

rename_کالم_Func_پورا:
  إذا( rc!=SQLITE_OK ){
    إذا( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_نتیجہ_قدر(context, argv[0]);
    }وإلا إذا( sParse.z_Err_پیغام ){
      rename_کالم_Parse_Error(context, "", argv[1], argv[2], &sParse);
    }وإلا{
      sqlite3_نتیجہ_error_کوڈ(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#جے_انت
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
ساكن صحيح rename_جدول_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  RenameCtx *p = pWalker->u.pRename;
  إذا( p_اظہاریہ->op==TK_کالم
   && ALWAYS(اظہاریہ_Use_YTab(p_اظہاریہ))
   && p->pTab==p_اظہاریہ->y.pTab
  ){
    rename_Token_لبھو(pWalker->pParse, p, (فراغ*)&p_اظہاریہ->y.pTab);
  }
  أرجع WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ساكن صحيح rename_جدول_Select_Cb(Walker *pWalker, Select *pSelect){
  صحيح i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  إذا( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    أرجع WRC_Prune;
  }
  إذا( NEVER(pSrc==0) ){
    أكد( pWalker->pParse->db->mallocFailed );
    أرجع WRC_Abort;
  }
  لكل(i=0; i<pSrc->nSrc; i++){
    Src_شے *p_شے = &pSrc->a[i];
    إذا( p_شے->pSTab==p->pTab ){
      rename_Token_لبھو(pWalker->pParse, p, p_شے->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  أرجع WRC_Continue;
}


/*
** This C function implements an SQL user function that is used by SQL code
** generated by the ALTER TABLE ... RENAME command to modify the definition
** of any foreign key constraints that use the table being renamed as the
** parent table. It is passed three arguments:
**
**   0: The database containing the table being renamed.
**   1. type:     Type of object ("table", "view" etc.)
**   2. object:   Name of object
**   3: The complete text of the schema statement being modified,
**   4: The old name of the table being renamed, and
**   5: The new name of the table being renamed.
**   6: True if the schema statement comes from the temp db.
**
** It returns the new schema statement. For example:
**
** sqlite_rename_table('main', 'CREATE TABLE t1(a REFERENCES t2)','t2','t3',0)
**       -> 'CREATE TABLE t1(a REFERENCES t3)'
*/
ساكن فراغ rename_جدول_Func(
  sqlite3_context *context,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  ثابت حرف *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[3]);
  ثابت حرف *zOld = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_قدر_متن(argv[5]);
  صحيح b_عارضی = sqlite3_قدر_int(argv[6]);
  بن_ورتیا_PARAMETER(Not_ورتیا);

  إذا( z_داخلہ && zOld && zNew ){
    Parse sParse;
    صحيح rc;
    صحيح bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#جے_انت

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, حجم(RenameCtx));
    sCtx.pTab = sqlite3_لبھو_جدول(db, zOld, zDb);
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_اظہاریہ_Callback = rename_جدول_اظہاریہ_Cb;
    sWalker.xSelectCallback = rename_جدول_Select_Cb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, b_عارضی);

    إذا( rc==SQLITE_OK ){
      صحيح isLegacy = (db->flags & SQLITE_LegacyAlter);
      إذا( sParse.p_New_جدول ){
        جدول *pTab = sParse.p_New_جدول;

        إذا( IsView(pTab) ){
          إذا( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, حجم(sNC));
            sNC.pParse = &sParse;

            أكد( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            إذا( sParse.nErr ){
              rc = sParse.rc;
            }وإلا{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }وإلا{
          /* Modify any FK definitions to point to the new table. */
#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
          إذا( (isLegacy==0 || (db->flags & SQLITE_Foreign_کلیدیں))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            أكد( Is_Ordinary_جدول(pTab) );
            لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
              إذا( sqlite3_stricmp(pFKey->z_منزل, zOld)==0 ){
                rename_Token_لبھو(&sParse, &sCtx, (فراغ*)pFKey->z_منزل);
              }
            }
          }
#جے_انت

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          إذا( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            إذا( isLegacy==0 ){
              sqlite3_Walk_اظہاریہ_List(&sWalker, pTab->pCheck);
            }
            rename_Token_لبھو(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      وإلا إذا( sParse.p_New_اشاریہ ){
        rename_Token_لبھو(&sParse, &sCtx, sParse.p_New_اشاریہ->zName);
        إذا( isLegacy==0 ){
          sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
        }
      }

#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
      وإلا{
        Trigger *pTrigger = sParse.pNewTrigger;
        Trigger_قدم *p_قدم;
        إذا( 0==sqlite3_stricmp(sParse.pNewTrigger->جدول_2, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          rename_Token_لبھو(&sParse, &sCtx, sParse.pNewTrigger->جدول_2);
        }

        إذا( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          إذا( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            لكل(p_قدم=pTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
              إذا( p_قدم->pSrc ){
                صحيح i;
                لكل(i=0; i<p_قدم->pSrc->nSrc; i++){
                  Src_شے *p_شے = &p_قدم->pSrc->a[i];
                  إذا( 0==sqlite3_stricmp(p_شے->zName, zOld) ){
                    rename_Token_لبھو(&sParse, &sCtx, p_شے->zName);
                  }
                }
              }
            }
          }
        }
      }
#جے_انت
    }

    إذا( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_داخلہ, zNew, bQuote);
    }
    إذا( rc!=SQLITE_OK ){
      إذا( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_نتیجہ_قدر(context, argv[3]);
      }وإلا إذا( sParse.z_Err_پیغام ){
        rename_کالم_Parse_Error(context, "", argv[1], argv[2], &sParse);
      }وإلا{
        sqlite3_نتیجہ_error_کوڈ(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#جے_انت
  }

  أرجع;
}

ساكن صحيح rename_Quotefix_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_STRING && (p_اظہاریہ->flags & EP_DblQuoted) ){
    rename_Token_لبھو(pWalker->pParse, pWalker->u.pRename, (ثابت فراغ*)p_اظہاریہ);
  }
  أرجع WRC_Continue;
}

/* SQL function: sqlite_rename_quotefix(DB,SQL)
**
** Rewrite the DDL statement "SQL" so that any string literals that use
** double-quotes use single quotes instead.
**
** Two arguments must be passed:
**
**   0: Database name ("main", "temp" etc.).
**   1: SQL statement to edit.
**
** The returned value is the modified SQL statement. For example, given
** the database schema:
**
**   CREATE TABLE t1(a, b, c);
**
**   SELECT sqlite_rename_quotefix('main',
**       'CREATE VIEW v1 AS SELECT "a", "string" FROM t1'
**   );
**
** returns the string:
**
**   CREATE VIEW v1 AS SELECT "a", 'string' FROM t1
**
** If there is a error in the input SQL, then raise an error, except
** if PRAGMA writable_schema=ON, then just return the input string
** unmodified following an error.
*/
ساكن فراغ renameQuotefixFunc(
  sqlite3_context *context,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  حرف ثابت *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#جے_انت

  sqlite3BtreeEnterAll(db);

  بن_ورتیا_PARAMETER(Not_ورتیا);
  إذا( zDb && z_داخلہ ){
    صحيح rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, 0);

    إذا( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, حجم(RenameCtx));
      memset(&sWalker, 0, حجم(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_اظہاریہ_Callback = rename_Quotefix_اظہاریہ_Cb;
      sWalker.xSelectCallback = rename_کالم_Select_Cb;
      sWalker.u.pRename = &sCtx;

      إذا( sParse.p_New_جدول ){
        إذا( IsView(sParse.p_New_جدول) ){
          Select *pSelect = sParse.p_New_جدول->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          إذا( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }وإلا{
          صحيح i;
          sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_جدول->pCheck);
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
          لكل(i=0; i<sParse.p_New_جدول->n_خانہ; i++){
            sqlite3_Walk_اظہاریہ(&sWalker,
               sqlite3_کالم_اظہاریہ(sParse.p_New_جدول,
                                         &sParse.p_New_جدول->a_خانہ[i]));
          }
#جے_انت /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }وإلا إذا( sParse.p_New_اشاریہ ){
        sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_اشاریہ->a_خانہ_اظہاریہ);
        sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
      }وإلا{
#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        إذا( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#جے_انت /* SQLITE_OMIT_TRIGGER */
      }

      إذا( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_داخلہ, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    إذا( rc!=SQLITE_OK ){
      إذا( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_نتیجہ_قدر(context, argv[1]);
      }وإلا{
        sqlite3_نتیجہ_error_کوڈ(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#جے_انت

  sqlite3BtreeLeaveAll(db);
}

/* Function:  sqlite_rename_test(DB,SQL,TYPE,NAME,ISTEMP,WHEN,DQS)
**
** An SQL user function that checks that there are no parse or symbol
** resolution problems in a CREATE TRIGGER|TABLE|VIEW|INDEX statement.
** After an ALTER TABLE .. RENAME operation is performed and the schema
** reloaded, this function is called on each SQL statement in the schema
** to ensure that it is still usable.
**
**   0: Database name ("main", "temp" etc.).
**   1: SQL statement.
**   2: Object type ("view", "table", "trigger" or "index").
**   3: Object name.
**   4: True if object is from temp schema.
**   5: "when" part of error message.
**   6: True to disable the DQS quirk when parsing SQL.
**
** The return value is computed as follows:
**
**   A. If an error is seen and not in PRAGMA writable_schema=ON mode,
**      then raise the error.
**   B. Else if a trigger is created and the the table that the trigger is
**      attached to is in database zDb, then return 1.
**   C. Otherwise return NULL.
*/
ساكن فراغ rename_جدول_Test(
  sqlite3_context *context,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  حرف ثابت *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحيح b_عارضی = sqlite3_قدر_int(argv[4]);
  صحيح isLegacy = (db->flags & SQLITE_LegacyAlter);
  حرف ثابت *zWhen = (ثابت حرف*)sqlite3_قدر_متن(argv[5]);
  صحيح bNoDQS = sqlite3_قدر_int(argv[6]);

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#جے_انت

  بن_ورتیا_PARAMETER(Not_ورتیا);

  إذا( zDb && z_داخلہ ){
    صحيح rc;
    Parse sParse;
    u64 flags = db->flags;
    إذا( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, b_عارضی);
    db->flags = flags;
    إذا( rc==SQLITE_OK ){
      إذا( isLegacy==0 && sParse.p_New_جدول && IsView(sParse.p_New_جدول) ){
        NameContext sNC;
        memset(&sNC, 0, حجم(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.p_New_جدول->u.view.pSelect, &sNC);
        إذا( sParse.nErr ) rc = sParse.rc;
      }

      وإلا إذا( sParse.pNewTrigger ){
        إذا( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        إذا( rc==SQLITE_OK ){
          صحيح i1 = sqlite3_Schema_منزل_اشاریہ(db, sParse.pNewTrigger->pTabSchema);
          صحيح i2 = sqlite3_لبھو_Db_Name(db, zDb);
          إذا( i1==i2 ){
            /* Handle output case B */
            sqlite3_نتیجہ_int(context, 1);
          }
        }
      }
    }

    إذا( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      rename_کالم_Parse_Error(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#جے_انت
}


/*
** Return the number of bytes until the end of the next non-whitespace and
** non-comment token.  For the purpose of this function, a "(" token includes
** all of the bytes through and including the matching ")", or until the
** first illegal token, whichever comes first.
**
** Write the token type into *piToken.
**
** The value returned is the number of bytes in the token itself plus
** the number of bytes of leading whitespace and comments skipped plus
** all bytes through the next matching ")" if the token is TK_LP.
**
** Example:    (Note: '.' used in place of '*' in the example z[] text)
**
**                                    ,--------- *piToken := TK_RP
**                                    v
**    z[] = " /.comment./ --comment\n (two three four) five"
**          |                                        |
**          |<-------------------------------------->|
**                              |
**                              `--- return value
*/
ساكن صحيح getConstraintToken(ثابت u8 *z, صحيح *piToken){
  صحيح iOff = 0;
  صحيح t = 0;
  افعل {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }طالما( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  إذا( t==TK_LP ){
    صحيح nNest = 1;
    طالما( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      إذا( t==TK_LP ){
        nNest++;
      }وإلا إذا( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }وإلا إذا( t==TK_ILLEGAL ){
        اكسر;
      }
    }
  }

  *piToken = t;
  أرجع iOff;
}

/*
** The implementation of internal UDF sqlite_drop_column().
**
** Arguments:
**
**  argv[0]: An integer - the index of the schema containing the table
**  argv[1]: CREATE TABLE statement to modify.
**  argv[2]: An integer - the index of the column to remove.
**
** The value returned is a string containing the CREATE TABLE statement
** with column argv[2] removed.
*/
ساكن فراغ drop_کالم_Func(
  sqlite3_context *context,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحيح iSchema = sqlite3_قدر_int(argv[0]);
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحيح i_خانہ = sqlite3_قدر_int(argv[2]);
  ثابت حرف *zDb = db->aDb[iSchema].zDbSName;
  صحيح rc;
  Parse sParse;
  RenameToken *p_خانہ;
  جدول *pTab;
  ثابت حرف *z_سرا;
  حرف *zNew = 0;

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#جے_انت

  بن_ورتیا_PARAMETER(Not_ورتیا);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  إذا( rc!=SQLITE_OK ) اذهب drop_کالم_پورا;
  pTab = sParse.p_New_جدول;
  إذا( pTab==0 || pTab->n_خانہ==1 || i_خانہ>=pTab->n_خانہ ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    اذهب drop_کالم_پورا;
  }

  إذا( i_خانہ<pTab->n_خانہ-1 ){
    RenameToken *p_سرا;
    p_خانہ = rename_Token_لبھو(&sParse, 0, (فراغ*)pTab->a_خانہ[i_خانہ].zCnName);
    p_سرا = rename_Token_لبھو(&sParse, 0, (فراغ*)pTab->a_خانہ[i_خانہ+1].zCnName);
    z_سرا = (ثابت حرف*)p_سرا->t.z;
  }وإلا{
    صحيح eTok;
    أكد( Is_Ordinary_جدول(pTab) );
    أكد( i_خانہ!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    p_خانہ = rename_Token_لبھو(&sParse, 0, (فراغ*)pTab->a_خانہ[i_خانہ-1].zCnName);
    افعل {
      p_خانہ->t.z += getConstraintToken((ثابت u8*)p_خانہ->t.z, &eTok);
    }طالما( eTok!=TK_COMMA );
    p_خانہ->t.z--;
    z_سرا = (ثابت حرف*)&zSql[pTab->u.tab.جوڑو_خانہ_وتھ];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", p_خانہ->t.z-zSql, zSql, z_سرا);
  sqlite3_نتیجہ_متن(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_کالم_پورا:
  renameParseCleanup(&sParse);
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#جے_انت
  إذا( rc!=SQLITE_OK ){
    sqlite3_نتیجہ_error_کوڈ(context, rc);
  }
}

/*
** This function is called by the parser upon parsing an
**
**     ALTER TABLE pSrc DROP COLUMN pName
**
** statement. Argument pSrc contains the possibly qualified name of the
** table being edited, and token pName the name of the column to drop.
*/
فراغ sqlite3_Alter_Drop_کالم(Parse *pParse, SrcList *pSrc, ثابت Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  جدول *pTab;                    /* Table to modify */
  صحيح iDb;                        /* Index of db containing pTab in aDb[] */
  ثابت حرف *zDb;                /* Database containing pTab ("main" etc.) */
  حرف *z_خانہ = 0;                 /* Name of column to drop */
  صحيح i_خانہ;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  أكد( pParse->p_New_جدول==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  إذا( NEVER(db->mallocFailed) ) اذهب exit_drop_کالم;
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_drop_کالم;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  إذا( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ) اذهب exit_drop_کالم;
  إذا( SQLITE_OK!=is_Real_جدول(pParse, pTab, 1) ) اذهب exit_drop_کالم;

  /* Find the index of the column being dropped. */
  z_خانہ = sqlite3NameFromToken(db, pName);
  إذا( z_خانہ==0 ){
    أكد( db->mallocFailed );
    اذهب exit_drop_کالم;
  }
  i_خانہ = sqlite3_کالم_اشاریہ(pTab, z_خانہ);
  إذا( i_خانہ<0 ){
    sqlite3_Error_پیغام(pParse, "no such column: \"%T\"", pName);
    اذهب exit_drop_کالم;
  }
  إذا( isRowidAlias(pParse, pTab, z_خانہ, "drop") ) اذهب exit_drop_کالم;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  إذا( pTab->a_خانہ[i_خانہ].خانہ_Flags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_پیغام(pParse, "cannot drop %s column: \"%s\"",
        (pTab->a_خانہ[i_خانہ].خانہ_Flags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        z_خانہ
    );
    اذهب exit_drop_کالم;
  }

  /* Do not allow the number of columns to go to zero */
  إذا( pTab->n_خانہ<=1 ){
    sqlite3_Error_پیغام(pParse, "cannot drop column \"%s\": no other columns exist",z_خانہ);
    اذهب exit_drop_کالم;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
  أكد( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, z_خانہ) ){
    اذهب exit_drop_کالم;
  }
#جے_انت
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, i_خانہ, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  إذا( pParse->nErr==0 && (pTab->a_خانہ[i_خانہ].خانہ_Flags & COLFLAG_VIRTUAL)==0 ){
    صحيح i;
    صحيح addr;
    صحيح reg;
    صحيح regRec;
    اشاریہ *pPk = 0;
    صحيح n_میدان = 0;               /* Number of non-virtual columns after drop */
    صحيح iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3_Open_جدول(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    إذا( HasRowid(pTab) ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->n_خانہ;
    }وإلا{
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      pParse->nMem += pPk->n_کالم;
      لكل(i=0; i<pPk->n_کلید_خانہ; i++){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iCur, i, reg+i+1);
      }
      n_میدان = pPk->n_کلید_خانہ;
    }
    regRec = ++pParse->nMem;
    لكل(i=0; i<pTab->n_خانہ; i++){
      إذا( i!=i_خانہ && (pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL)==0 ){
        صحيح reg_باہرلا;
        إذا( pPk ){
          صحيح iPos = sqlite3_جدول_کالم_منزل_اشاریہ(pPk, i);
          صحيح i_خانہ_Pos = sqlite3_جدول_کالم_منزل_اشاریہ(pPk, i_خانہ);
          إذا( iPos<pPk->n_کلید_خانہ ) استمر;
          reg_باہرلا = reg+1+iPos-(iPos>i_خانہ_Pos);
        }وإلا{
          reg_باہرلا = reg+1+n_میدان;
        }
        إذا( i==pTab->iPKey ){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg_باہرلا);
        }وإلا{
          sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(v, pTab, iCur, i, reg_باہرلا);
          أكد( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->a_خانہ[i].affinity==SQLITE_AFF_REAL );
          sqlite3_Vdbe_Delete_پہلوں_Opcode(v, OP_RealAffinity);
        }
        n_میدان++;
      }
    }
    إذا( n_میدان==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg+1);
      n_میدان = 1;
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg+1, n_میدان, regRec);
    إذا( pPk ){
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iCur, regRec, reg+1, pPk->n_کلید_خانہ);
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iCur, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ایتھے(v, addr);
  }

exit_drop_کالم:
  sqlite3DbFree(db, z_خانہ);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
ساكن صحيح getWhitespace(ثابت u8 *z){
  صحيح nRet = 0;
  طالما( 1 ){
    صحيح t = 0;
    صحيح n = sqlite3GetToken(&z[nRet], &t);
    إذا( t!=TK_SPACE && t!=TK_COMMENT ) اكسر;
    nRet += n;
  }
  أرجع nRet;
}


/*
** Argument z points into the body of a constraint - specifically the 
** second token of the constraint definition.  For a named constraint,
** z points to the second token of the constraint definition. For an 
** unnamed NOT NULL constraint, z points to the first byte past the NOT 
** keyword.
**
** Argument eTok may be the token value of the first token of the constraint
** (e.g. TK_CHECK or TK_REFERENCES) or zero. If it is either TK_REFERENCES
** or TK_FOREIGN, special parsing is enabled to find the end of the foreign-key
** constraint definition.
**
** Return the number of bytes until the end of the constraint. 
*/
ساكن صحيح getConstraint(ثابت u8 *z, صحيح eTok){
  صحيح iOff = 0;
  صحيح t = 0;

#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
  إذا( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  إذا( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#جے_انت

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  طالما( 1 ){
    صحيح n = getConstraintToken(&z[iOff], &t);
    إذا( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      اكسر;
    }
    iOff += n;
  }
  
  أرجع iOff;
}

/*
** Compare two constraint names.
**
** Summary:   *pRes := zQuote != zCmp
**
** Details:
** Compare the (possibly quoted) constraint name zQuote[0..nQuote-1]
** against zCmp[].  Write zero into *pRes if they are the same and
** non-zero if they differ.  Normally return SQLITE_OK, except if there
** is an OOM, set the OOM error condition on ctx and return SQLITE_NOMEM.
*/
ساكن صحيح quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  صحيح t,                 /* Token type */
  ثابت u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  صحيح nQuote,            /* Length of zQuote in bytes */
  ثابت u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  صحيح *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  حرف *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  إذا( t==TK_ILLEGAL ){
    *pRes = 1;
    أرجع SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  إذا( zCopy==0 ){
    sqlite3_نتیجہ_error_nomem(ctx);
    أرجع SQLITE_NOMEM_BKPT;
  }
  حافظہ_نقل(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ثابت حرف*)zCopy, (ثابت حرف*)zCmp);
  sqlite3_free(zCopy);
  أرجع SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ساكن صحيح skip_Create_جدول(sqlite3_context *ctx, ثابت u8 *zSql, صحيح *piOff){
  صحيح iOff = 0;

  إذا( zSql==0 ) أرجع SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  طالما( 1 ){
    صحيح t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    إذا( t==TK_LP ) اكسر;
    إذا( t==TK_ILLEGAL ){
      sqlite3_نتیجہ_error_کوڈ(ctx, SQLITE_CORRUPT_BKPT);
      أرجع SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  أرجع SQLITE_OK;
}

/*
** Internal SQL function sqlite3_drop_constraint():  Given an input
** CREATE TABLE statement, return a revised CREATE TABLE statement
** with a constraint removed.  Two forms, depending on the datatype
** of argv[2]:
**
**   sqlite_drop_constraint(SQL, INT)  -- Omit NOT NULL from the INT-th column
**   sqlite_drop_constraint(SQL, TEXT) -- OMIT constraint with name TEXT
**
** In the first case, the left-most column is 0.
*/
ساكن فراغ dropConstraintFunc(
  sqlite3_context *ctx,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = sqlite3_قدر_متن(argv[0]);
  ثابت u8 *zCons = 0;
  صحيح iNotNull = -1;
  صحيح ii;
  صحيح iOff = 0;
  صحيح i_شروع = 0;
  صحيح i_سرا = 0;
  حرف *zNew = 0;
  صحيح t = 0;
  sqlite3 *db;
  بن_ورتیا_PARAMETER(Not_ورتیا);

  إذا( zSql==0 ) أرجع;

  /* Jump past the "CREATE TABLE" bit. */
  إذا( skip_Create_جدول(ctx, zSql, &iOff) ) أرجع;

  إذا( sqlite3_قدر_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_قدر_int(argv[1]);
  }وإلا{
    zCons = sqlite3_قدر_متن(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  لكل(ii=0; i_سرا==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    طالما( 1 ){
      i_شروع = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      إذا( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        صحيح nTok = 0;
        صحيح cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        إذا( zCons ){
          إذا( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) أرجع;
        }
        iOff += nTok;

        /* The next token is usually the first token of the constraint
        ** definition. This is enough to tell the type of the constraint - 
        ** TK_NOT means it is a NOT NULL, TK_CHECK a CHECK constraint etc.
        **
        ** There is also the chance that the next token is TK_CONSTRAINT
        ** (or TK_DEFAULT or TK_COLLATE), for example if a table has been
        ** created as follows:
        **
        **    CREATE TABLE t1(cols, CONSTRAINT one CONSTRAINT two NOT NULL);
        **
        ** In this case, allow the "CONSTRAINT one" bit to be dropped by
        ** this command if that is what is requested, or to advance to
        ** the next iteration of the loop with &zSql[iOff] still pointing
        ** to the CONSTRAINT keyword.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        إذا( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }وإلا{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        إذا( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          إذا( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            أرجع;
          }
          i_سرا = iOff;
          اكسر;
        }

      }وإلا إذا( t==TK_NOT && iNotNull==ii ){
        i_سرا = iOff + getConstraint(&zSql[iOff], 0);
        اكسر;
      }وإلا إذا( t==TK_RP || t==TK_ILLEGAL ){
        i_سرا = -1;
        اكسر;
      }وإلا إذا( t==TK_COMMA ){
        اكسر;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  إذا( i_سرا<=0 ){
    إذا( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }وإلا{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_نتیجہ_متن(ctx, (ثابت حرف*)zSql, -1, SQLITE_TRANSIENT);
    }
  }وإلا{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ثابت حرف *zSpace = " ";
    i_سرا += getWhitespace(&zSql[i_سرا]);
    sqlite3GetToken(&zSql[i_سرا], &t);
    إذا( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      إذا( zSql[i_شروع-1]==',' ) i_شروع--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", i_شروع, zSql, zSpace, &zSql[i_سرا]);
    sqlite3_نتیجہ_متن(ctx, zNew, -1, SQLITE_DYNAMIC);
  }
}

/*
** Internal SQL function:
**
**     sqlite_add_constraint(SQL, CONSTRAINT-TEXT, ICOL)
**
** SQL is a CREATE TABLE statement.  Return a modified version of
** SQL that adds CONSTRAINT-TEXT at the end of the ICOL-th column
** definition.  (The left-most column defintion is 0.)
*/
ساكن فراغ جوڑو_Constraint_Func(
  sqlite3_context *ctx,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = sqlite3_قدر_متن(argv[0]);
  ثابت حرف *zCons = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحيح i_خانہ = sqlite3_قدر_int(argv[2]);
  صحيح iOff = 0;
  صحيح ii;
  sqlite3_str *pNew;
  صحيح t = 0;
  بن_ورتیا_PARAMETER(Not_ورتیا);

  إذا( skip_Create_جدول(ctx, zSql, &iOff) ) أرجع;
  
  لكل(ii=0; ii<=i_خانہ || (i_خانہ<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    طالما( 1 ){
      صحيح nTok = getConstraintToken(&zSql[iOff], &t);
      إذا( t==TK_COMMA || t==TK_RP ) اكسر;
      إذا( t==TK_ILLEGAL ){
        sqlite3_نتیجہ_error_کوڈ(ctx, SQLITE_CORRUPT_BKPT);
        أرجع;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ثابت حرف*)zSql, iOff);
  إذا( i_خانہ<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_نتیجہ_str(ctx, pNew, SQLITE_انت);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ساكن صحيح alter_لبھو_خانہ(Parse *pParse, جدول *pTab, Token *p_خانہ, صحيح *pi_خانہ){
  sqlite3 *db = pParse->db;
  حرف *zName = sqlite3NameFromToken(db, p_خانہ);
  صحيح rc = SQLITE_NOMEM;
  صحيح i_خانہ = -1;

  إذا( zName ){
    i_خانہ = sqlite3_کالم_اشاریہ(pTab, zName);
    إذا( i_خانہ<0 ){
      sqlite3_Error_پیغام(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }وإلا{
      rc = SQLITE_OK;
    }
  }

#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  إذا( rc==SQLITE_OK ){
    ثابت حرف *zDb = db->aDb[sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema)].zDbSName;
    ثابت حرف *z_خانہ = pTab->a_خانہ[i_خانہ].zCnName;
    إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, z_خانہ) ){
      pTab = 0;
    }
  }
#جے_انت

  sqlite3DbFree(db, zName);
  *pi_خانہ = i_خانہ;
  أرجع rc;
}


/*
** Find the table named by the first entry in source list pSrc. If successful,
** return a pointer to the Table structure and set output variable (*pzDb)
** to point to the name of the database containin the table (i.e. "main",
** "temp" or the name of an attached database). 
**
** If the table cannot be located, return NULL. The value of the two output
** parameters is undefined in this case.
*/
ساكن جدول *alter_لبھو_جدول(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  صحيح *piDb,            /* OUT: write the iDb here */
  ثابت حرف **pzDb,    /* OUT: write name of schema here */
  صحيح bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  جدول *pTab = 0;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  إذا( pTab ){
    صحيح iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    إذا( SQLITE_OK!=is_Real_جدول(pParse, pTab, 2) 
     || SQLITE_OK!=is_Alterable_جدول(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  إذا( pTab && bAuth ){
    إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#جے_انت
  sqlite3SrcListDelete(db, pSrc);
  أرجع pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
فراغ sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *p_خانہ       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  جدول *pTab = 0;
  صحيح iDb = 0;
  ثابت حرف *zDb = 0;
  حرف *zArg = 0;

  أكد( (p_خانہ==0)!=(pCons==0) );
  أكد( pSrc->nSrc==1 );
  pTab = alter_لبھو_جدول(pParse, pSrc, &iDb, &zDb, pCons!=0);
  إذا( !pTab ) أرجع;

  إذا( pCons ){
    حرف *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }وإلا{
    صحيح i_خانہ;
    إذا( alter_لبھو_خانہ(pParse, pTab, p_خانہ, &i_خانہ) ) أرجع;
    zArg = sqlite3MPrintf(db, "%d", i_خانہ);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_drop_constraint(sql, %s) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, zArg, pTab->zName
  );
  sqlite3DbFree(db, zArg);

  /* Finally, reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDropCons);
}

/*
** The implementation of SQL function sqlite_fail(MSG). This takes a single
** argument, and returns it as an error message with the error code set to
** SQLITE_CONSTRAINT.
*/
ساكن فراغ failConstraintFunc(
  sqlite3_context *ctx,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  ثابت حرف *z_متن = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  صحيح err = sqlite3_قدر_int(argv[1]);
  (فراغ)Not_ورتیا;
  sqlite3_نتیجہ_error(ctx, z_متن, -1);
  sqlite3_نتیجہ_error_کوڈ(ctx, err);
}

/*
** Buffer pCons, which is nCons bytes in size, contains the text of a 
** NOT NULL or CHECK constraint that will be inserted into a CREATE TABLE
** statement. If successful, this function returns the size of the buffer in
** bytes not including any trailing whitespace or "--" style comments. Or,
** if an OOM occurs, it returns 0 and sets db->mallocFailed to true.
**
** C-style comments at the end are preserved.  "--" style comments are
** removed because the comment terminator might be \000, and we are about
** to insert the pCons[] text into the middle of a larger string, and that
** will have the effect of removing the comment terminator and messing up
** the syntax.
*/
ساكن صحيح alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ثابت حرف *pCons,              /* Buffer containing constraint */
  صحيح nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  صحيح iOff = 0;
  صحيح i_سرا = 0;

  إذا( zTmp==0 ) أرجع 0;

  طالما( 1 ){
    صحيح t = 0;
    صحيح nToken = sqlite3GetToken(&zTmp[iOff], &t);
    إذا( t==TK_ILLEGAL ) اكسر;
    إذا( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      i_سرا = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  أرجع i_سرا;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
فراغ sqlite3_Alter_سیٹ_Not_Null(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *p_خانہ,     /* Name of the column to add a NOT NULL constraint to */
  Token *p_پہلا    /* The NOT token of the NOT NULL constraint text */
){
  جدول *pTab = 0;
  صحيح i_خانہ = 0;
  صحيح iDb = 0;
  ثابت حرف *zDb = 0;
  ثابت حرف *pCons = 0;
  صحيح nCons = 0;

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alter_لبھو_جدول(pParse, pSrc, &iDb, &zDb, 0);
  إذا( !pTab ) أرجع;

  /* Find the column being altered. */
  إذا( alter_لبھو_خانہ(pParse, pTab, p_خانہ, &i_خانہ) ){
    أرجع;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = p_پہلا->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_آخری_Token.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (صحيح)p_خانہ->n, p_خانہ->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, i_خانہ, nCons, pCons, i_خانہ, pTab->zName
  );

  /* Finally, reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDropCons);
}

/*
** Implementation of internal SQL function:
**
**     sqlite_find_constraint(SQL, CONSTRAINT-NAME)
**
** This function returns true if the SQL passed as the first argument is a
** CREATE TABLE that contains a constraint with the name CONSTRAINT-NAME,
** or false otherwise.
*/
ساكن فراغ لبھو_Constraint_Func(
  sqlite3_context *ctx,
  صحيح Not_ورتیا,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = 0;
  ثابت u8 *zCons = 0;
  صحيح iOff = 0;
  صحيح t = 0;

  (فراغ)Not_ورتیا;
  zSql = sqlite3_قدر_متن(argv[0]);
  zCons = sqlite3_قدر_متن(argv[1]);

  إذا( zSql==0 || zCons==0 ) أرجع;
  طالما( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  طالما( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    إذا( t==TK_CONSTRAINT ){
      صحيح nTok = 0;
      صحيح cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      إذا( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) أرجع;
      إذا( cmp==0 ){
        sqlite3_نتیجہ_int(ctx, 1);
        أرجع;
      }
    }وإلا إذا( t==TK_ILLEGAL ){
      اكسر;
    }
  }

  sqlite3_نتیجہ_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
فراغ sqlite3_Alter_جوڑو_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_پہلا,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ثابت حرف *z_اظہاریہ,       /* Text of CHECK expression */
  صحيح n_اظہاریہ,               /* Size of pExpr in bytes */
  اظہاریہ *p_اظہاریہ              /* The parsed CHECK expression */
){ 
  جدول *pTab = 0;         /* Table identified by pSrc */
  صحيح iDb = 0;             /* Which schema does pTab live in */
  ثابت حرف *zDb = 0;     /* Name of the schema in which pTab lives */
  ثابت حرف *pCons = 0;   /* Text of the constraint */
  صحيح nCons;               /* Bytes of text to use from pCons[] */
  صحيح rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alter_لبھو_جدول(pParse, pSrc, &iDb, &zDb, 1);
  إذا( !pTab ){
    sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
    أرجع;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_اظہاریہ, 0);
  sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
  إذا( rc ) أرجع;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  إذا( pName ){
    حرف *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_جدول " "
        "WHERE type='table' AND tbl_name=%Q COLLATE nocase "
        "AND sqlite_find_constraint(sql, %Q)",
        zName, SQLITE_ERROR, zDb, pTab->zName, zName
    );
    sqlite3DbFree(pParse->db, zName);
  }

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q WHERE (%.*s) IS FALSE", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_اظہاریہ, z_اظہاریہ
  );

  /* Edit the SQL for the named table. */
  pCons = p_پہلا->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_آخری_Token.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_add_constraint(sql, %.*Q, -1) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, nCons, pCons, pTab->zName
  );

  /* Finally, reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDropCons);
}

/*
** Register built-in functions used to help implement ALTER TABLE
*/
فراغ sqlite3AlterFunctions(فراغ){
  ساكن FuncDef a_Alter_جدول_Funcs[] = {
    اندرونی_FUNCTION(sqlite_rename_کالم,  9, rename_کالم_Func),
    اندرونی_FUNCTION(sqlite_rename_جدول,   7, rename_جدول_Func),
    اندرونی_FUNCTION(sqlite_rename_test,    7, rename_جدول_Test),
    اندرونی_FUNCTION(sqlite_drop_کالم,    3, drop_کالم_Func),
    اندرونی_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    اندرونی_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    اندرونی_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    اندرونی_FUNCTION(sqlite_جوڑو_constraint, 3, جوڑو_Constraint_Func),
    اندرونی_FUNCTION(sqlite_لبھو_constraint,2, لبھو_Constraint_Func),
  };
  sqlite3_داخل_کرو_Builtin_Funcs(a_Alter_جدول_Funcs, Array_ناپ(a_Alter_جدول_Funcs));
}
#جے_انت  /* SQLITE_ALTER_TABLE */
