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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#ifndef SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isAlterableTable(Parse *pParse, Table *pTab){
  ଯଦି( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "table %s may not be altered", pTab->zName);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ଅଚଳ ଅକ୍ଷର *zDb,                /* Name of db to verify schema of */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp,                      /* True if this is the temp db */
  ଅଚଳ ଅକ୍ଷର *zWhen,              /* "when" part of error message */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->colNamesSet = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_TABLE " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, bTemp, zWhen, bNoDQS
  );

  ଯଦି( bTemp==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_TABLE " "
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
ସ୍ଥିର ଶୂନ୍ୟ renameFixQuotes(Parse *pParse, ଅଚଳ ଅକ୍ଷର *zDb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  ଯଦି( bTemp==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_TABLE
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
ସ୍ଥିର ଶୂନ୍ୟ renameReloadSchema(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  ଯଦି( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    ଯଦି( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
ଶୂନ୍ୟ sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                  /* Database that contains the table */
  ଅକ୍ଷର *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  ଅକ୍ଷର *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTabName;             /* Number of UTF-8 characters in zTabName */
  ଅଚଳ ଅକ୍ଷର *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  ଯଦି( NEVER(db->mallocFailed) ) ଯାଅ exit_rename_table;
  ପ୍ରତିଜ୍ଞା( pSrc->nSrc==1 );
  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ଯଦି( !pTab ) ଯାଅ exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  ଯଦି( !zName ) ଯାଅ exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  ଯଦି( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse,
        "there is already another table or index with this name: %s", zName);
    ଯାଅ exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  ଯଦି( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    ଯାଅ exit_rename_table;
  }
  ଯଦି( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    ଯାଅ exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  ଯଦି( IsView(pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "view %s may not be altered", pTab->zName);
    ଯାଅ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ଯାଅ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ଯଦି( sqlite3ViewGetColumnNames(pParse, pTab) ){
    ଯାଅ exit_rename_table;
  }
  ଯଦି( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    ଯଦି( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  ଯଦି( v==0 ){
    ଯାଅ exit_rename_table;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_TABLE " SET "
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

#ifndef SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  ଯଦି( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  ଯଦି( iDb!=1 ){
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
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ଯଦି( pVTab ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(ଅଚଳ ଅକ୍ଷର*)pVTab, P4_VTAB);
  }
#endif

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_table:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  ଅଚଳ ଅକ୍ଷର *zDb,      /* Schema holding the table */
  ଅଚଳ ଅକ୍ଷର *zTab,     /* Table to check for empty */
  ଅଚଳ ଅକ୍ଷର *zErr      /* Error message text */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  ଅଚଳ ଅକ୍ଷର *zCol, 
  ଅଚଳ ଅକ୍ଷର *zOp
){
  ଯଦି( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
ଶୂନ୍ୟ sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                  /* Database number */
  ଅଚଳ ଅକ୍ଷର *zDb;          /* Database name */
  ଅଚଳ ଅକ୍ଷର *zTab;         /* Table name */
  ଅକ୍ଷର *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  ପଦସମଷ୍ଟି *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1;                   /* Temporary registers */

  db = pParse->db;
  ପ୍ରତିଜ୍ଞା( db->pParse==pParse );
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  ପ୍ରତିଜ୍ଞା( pNew );

  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_ପଦସମଷ୍ଟି(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  ପ୍ରତିଜ୍ଞା( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ଫେରନ୍ତୁ;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  ଯଦି( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "Cannot add a PRIMARY KEY column");
    ଫେରନ୍ତୁ;
  }
  ଯଦି( pNew->pIndex ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse,
         "Cannot add a UNIQUE column");
    ଫେରନ୍ତୁ;
  }
  ଯଦି( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) ଫେରନ୍ତୁ;
  ଯଦି( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    ପ୍ରତିଜ୍ଞା( pDflt==0 || pDflt->op==TK_SPAN );
    ଯଦି( pDflt && pDflt->p_ବାମ->op==TK_NULL ){
      pDflt = 0;
    }
    ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pNew) );
    ଯଦି( (db->flags&SQLITE_Foreign_ଚାବିଗୁଡ଼ିକ) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    ଯଦି( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    ଯଦି( pDflt ){
      sqlite3_ମୂଲ୍ୟ *pVal = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      rc = sqlite3_ମୂଲ୍ୟ_From_ପଦସମଷ୍ଟି(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      ଯଦି( rc!=SQLITE_OK ){
        ପ୍ରତିଜ୍ଞା( db->mallocFailed == 1 );
        ଫେରନ୍ତୁ;
      }
      ଯଦି( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_ମୂଲ୍ୟ_Free(pVal);
    }
  }ଅନ୍ୟଥା ଯଦି( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (ଅକ୍ଷର*)pColDef->z, pColDef->n);
  ଯଦି( zCol ){
    ଅକ୍ଷର *zEnd = &zCol[pColDef->n-1];
    ଯେତେବେଳେ( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pTab) );
    ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.addColOffset, zCol, pNew->u.tab.addColOffset,
      zTab
    );
    sqlite3DbFree(db, zCol);
  }

  v = sqlite3GetVdbe(pParse);
  ଯଦି( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3GetTempReg(pParse);
    sqlite3VdbeAddOp3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3VdbeUsesBtree(v, iDb);
    sqlite3VdbeAddOp2(v, OP_AddImm, r1, -2);
    sqlite3VdbeAddOp2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3ReleaseTempReg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_AlterAdd);

    /* Verify that constraints are still satisfied */
    ଯଦି( pNew->pCheck!=0
     || (pCol->notNull && (pCol->colFlags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->colNamesSet = 1;
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
ଶୂନ୍ୟ sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  ପ୍ରତିଜ୍ଞା( pParse->pNewTable==0 );
  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  ଯଦି( NEVER(db->mallocFailed) ) ଯାଅ exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ଯଦି( !pTab ) ଯାଅ exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ଯଦି( IsVirtual(pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "virtual tables may not be altered");
    ଯାଅ exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  ଯଦି( IsView(pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "Cannot add a column to a view");
    ଯାଅ exit_begin_add_column;
  }
  ଯଦି( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    ଯାଅ exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pTab) );
  ପ୍ରତିଜ୍ଞା( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, ଆକାର(Table));
  ଯଦି( !pNew ) ଯାଅ exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  ପ୍ରତିଜ୍ଞା( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  ପ୍ରତିଜ୍ଞା( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, ଆକାର(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  ଯଦି( !pNew->aCol || !pNew->zName ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    ଯାଅ exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, ଆକାର(Column)*(size_t)pNew->nCol);
  ପାଇଁ(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  ପ୍ରତିଜ୍ଞା( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  ଫେରନ୍ତୁ;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#ଯଦି !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isRealTable(Parse *pParse, Table *pTab, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOp){
  ଅଚଳ ଅକ୍ଷର *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  ଯଦି( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ଯଦି( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  ଯଦି( zType ){
    ଅଚଳ ଅକ୍ଷର *az_ସନ୍ଦେଶ[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    ପ୍ରତିଜ୍ଞା( iOp>=0 && iOp<ArraySize(az_ସନ୍ଦେଶ) );
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot %s %s \"%s\"",
        az_ସନ୍ଦେଶ[iOp], zType, pTab->zName
    );
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
ଶୂନ୍ୟ sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                       /* Index of column being renamed */
  ଅକ୍ଷର *zOld = 0;                 /* Old column name */
  ଅକ୍ଷର *zNew = 0;                 /* New column name */
  ଅଚଳ ଅକ୍ଷର *zDb;                /* Name of schema containing the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSchema;                    /* Index of the schema */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ଯଦି( !pTab ) ଯାଅ exit_rename_column;

  /* Cannot alter a system table */
  ଯଦି( SQLITE_OK!=isAlterableTable(pParse, pTab) ) ଯାଅ exit_rename_column;
  ଯଦି( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) ଯାଅ exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  ପ୍ରତିଜ୍ଞା( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ଯାଅ exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  ଯଦି( !zOld ) ଯାଅ exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  ଯଦି( iCol<0 ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such column: \"%T\"", pOld);
    ଯାଅ exit_rename_column;
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
  ଯଦି( !zNew ) ଯାଅ exit_rename_column;
  ଯଦି( isRowidAlias(pParse, pTab, zOld, "rename from") ) ଯାଅ exit_rename_column;
  ଯଦି( isRowidAlias(pParse, pTab, zNew, "rename to") ) ଯାଅ exit_rename_column;
  ପ୍ରତିଜ୍ଞା( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, iCol, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  ଯଦି( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_TABLE " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, iCol, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_column:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  ଫେରନ୍ତୁ;
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
ସଂରଚନା RenameToken {
  ଅଚଳ ଶୂନ୍ୟ *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା RenameCtx RenameCtx;
ସଂରଚନା RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nList;                      /* Number of tokens in pList */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  ଅଚଳ ଅକ୍ଷର *zOld;               /* Old column name */
};

#ifdef SQLITE_DEBUG
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
ସ୍ଥିର ଶୂନ୍ୟ renameTokenCheckAll(Parse *pParse, ଅଚଳ ଶୂନ୍ୟ *pPtr){
  ପ୍ରତିଜ୍ଞା( pParse==pParse->db->pParse );
  ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ଯଦି( pParse->nErr==0 ){
    ଅଚଳ RenameToken *p;
    u32 i = 1;
    ପାଇଁ(p=pParse->pRename; p; p=p->pNext){
      ଯଦି( p->p ){
        ପ୍ରତିଜ୍ଞା( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    ପ୍ରତିଜ୍ଞା( i>0 );
  }
}
#ଅନ୍ୟଥା
# define renameTokenCheckAll(x,y)
#endif

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
ଅଚଳ ଶୂନ୍ୟ *sqlite3RenameTokenMap(
  Parse *pParse,
  ଅଚଳ ଶୂନ୍ୟ *pPtr,
  ଅଚଳ Token *pToken
){
  RenameToken *pNew;
  ପ୍ରତିଜ୍ଞା( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  ଯଦି( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, ଆକାର(RenameToken));
    ଯଦି( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  ଫେରନ୍ତୁ pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
ଶୂନ୍ୟ sqlite3RenameTokenRemap(Parse *pParse, ଅଚଳ ଶୂନ୍ୟ *pTo, ଅଚଳ ଶୂନ୍ୟ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  ପାଇଁ(p=pParse->pRename; p; p=p->pNext){
    ଯଦି( p->p==pFrom ){
      p->p = pTo;
      ଭାଙ୍ଗ;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rename_Unmap_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ଅଚଳ ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି);
  ଯଦି( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) ){
    sqlite3RenameTokenRemap(pParse, 0, (ଅଚଳ ଶୂନ୍ୟ*)&p_ପଦସମଷ୍ଟି->y.pTab);
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  ଯଦି( pWith ){
    Parse *pParse = pWalker->pParse;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    With *pCopy = 0;
    ପ୍ରତିଜ୍ଞା( pWith->nCte>0 );
    ଯଦି( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ଠେଲ(pParse, pCopy, 1);
    }
    ପାଇଁ(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, ଆକାର(sNC));
      sNC.pParse = pParse;
      ଯଦି( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      ଯଦି( sNC.pParse->db->mallocFailed ) ଫେରନ୍ତୁ;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    ଯଦି( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ସ୍ଥିର ଶୂନ୍ୟ unmapColumnIdlistNames(
  Parse *pParse,
  ଅଚଳ IdList *pIdList
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପ୍ରତିଜ୍ଞା( pIdList!=0 );
  ପାଇଁ(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ଅଚଳ ଶୂନ୍ୟ*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  ଯଦି( p->selFlags & (SF_View|SF_CopyCte) ){
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ଯଦି( ALWAYS(p->pEList) ){
    ପଦସମଷ୍ଟି_List *pList = p->pEList;
    ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
      ଯଦି( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)pList->a[i].zEName);
      }
    }
  }
  ଯଦି( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    ପାଇଁ(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)pSrc->a[i].zName);
      ଯଦି( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pSrc->a[i].u3.pOn);
      }ଅନ୍ୟଥା{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
ଶୂନ୍ୟ sqlite3_Rename_ପଦସମଷ୍ଟି_Unmap(Parse *pParse, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, ଆକାର(Walker));
  sWalker.pParse = pParse;
  sWalker.x_ପଦସମଷ୍ଟି_Callback = rename_Unmap_ପଦସମଷ୍ଟି_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, p_ପଦସମଷ୍ଟି);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
ଶୂନ୍ୟ sqlite3RenameExprlistUnmap(Parse *pParse, ପଦସମଷ୍ଟି_List *pEList){
  ଯଦି( pEList ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    Walker sWalker;
    memset(&sWalker, 0, ଆକାର(Walker));
    sWalker.pParse = pParse;
    sWalker.x_ପଦସମଷ୍ଟି_Callback = rename_Unmap_ପଦସମଷ୍ଟି_Cb;
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, pEList);
    ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
      ଯଦି( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ସ୍ଥିର ଶୂନ୍ୟ renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  ପାଇଁ(p=pToken; p; p=pNext){
    pNext = p->pNext;
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
ସ୍ଥିର RenameToken *renameTokenFind(
  Parse *pParse,
  ସଂରଚନା RenameCtx *pCtx,
  ଅଚଳ ଶୂନ୍ୟ *pPtr
){
  RenameToken **pp;
  ଯଦି( NEVER(pPtr==0) ){
    ଫେରନ୍ତୁ 0;
  }
  ପାଇଁ(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    ଯଦି( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      ଯଦି( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      ଫେରନ୍ତୁ pToken;
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameColumnSelectCb(Walker *pWalker, Select *p){
  ଯଦି( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    ଫେରନ୍ତୁ WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rename_Column_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  RenameCtx *p = pWalker->u.pRename;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_TRIGGER
   && p_ପଦସମଷ୍ଟି->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି);
  }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
   && p_ପଦସମଷ୍ଟି->iColumn==p->iCol
   && ALWAYS(ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି))
   && p->pTab==p_ପଦସମଷ୍ଟି->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି);
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  ପାଇଁ(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    ଯଦି( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  ପାଇଁ(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  ଫେରନ୍ତୁ pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ସ୍ଥିର ଶୂନ୍ୟ errorMPrintf(sqlite3_context *pCtx, ଅଚଳ ଅକ୍ଷର *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ଅକ୍ଷର *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  ଯଦି( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }ଅନ୍ୟଥା{
    sqlite3_result_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameColumnParseError(
  sqlite3_context *pCtx,
  ଅଚଳ ଅକ୍ଷର *zWhen,
  sqlite3_ମୂଲ୍ୟ *pType,
  sqlite3_ମୂଲ୍ୟ *pObject,
  Parse *pParse
){
  ଅଚଳ ଅକ୍ଷର *zT = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pType);
  ଅଚଳ ଅକ୍ଷର *zN = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pObject);
  ଅକ୍ଷର *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_ସନ୍ଦେଶ
  );
  sqlite3_result_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pEList,
  ଅଚଳ ଅକ୍ଷର *zOld
){
  ଯଦି( pEList ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
      ଅଚଳ ଅକ୍ଷର *zName = pEList->a[i].zEName;
      ଯଦି( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (ଅଚଳ ଶୂନ୍ୟ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ଅଚଳ IdList *pIdList,
  ଅଚଳ ଅକ୍ଷର *zOld
){
  ଯଦି( pIdList ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pIdList->nId; i++){
      ଅଚଳ ଅକ୍ଷର *zName = pIdList->a[i].zName;
      ଯଦି( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (ଅଚଳ ଶୂନ୍ୟ*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ଅଚଳ ଅକ୍ଷର *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ଅଚଳ ଅକ୍ଷର *zSql,               /* SQL to parse */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp                       /* True if SQL is from temp schema */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  ଯଦି( zSql==0 ){
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  ଯଦି( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    ଫେରନ୍ତୁ SQLITE_CORRUPT_BKPT;
  }
  ଯଦି( bTemp ){
    db->init.iDb = 1;
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3FindDbName(db, zDb);
    ପ୍ରତିଜ୍ଞା( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->eParseMode = PARSE_MODE_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && strstr(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3RunParser(p, zSql);
  db->flags = flags;
  ଯଦି( db->mallocFailed ) rc = SQLITE_NOMEM;
  ଯଦି( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  ଯଦି( rc==SQLITE_OK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    ପାଇଁ(pToken=p->pRename; pToken; pToken=pToken->pNext){
      ପ୍ରତିଜ୍ଞା( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ଅଚଳ ଅକ୍ଷର *zSql,               /* SQL statement to edit */
  ଅଚଳ ଅକ୍ଷର *zNew,               /* New token text */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଅକ୍ଷର *zQuot = 0;
  ଅକ୍ଷର *zOut;
  i64 nQuot = 0;
  ଅକ୍ଷର *zBuf1 = 0;
  ଅକ୍ଷର *zBuf2 = 0;

  ଯଦି( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    ଯଦି( zQuot==0 ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }ଅନ୍ୟଥା{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    ପ୍ରତିଜ୍ଞା( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( nSql>0 );
    zOut = (ଅକ୍ଷର*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    ଯଦି( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  ଯଦି( zOut ){
    i64 nOut = nSql;
    ପ୍ରତିଜ୍ଞା( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    ଯେତେବେଳେ( pRename->pList ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ଅଚଳ ଅକ୍ଷର *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      ଯଦି( zNew ){
        ଯଦି( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }ଅନ୍ୟଥା{
          nReplace = nQuot;
          zReplace = zQuot;
          ଯଦି( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }ଅନ୍ୟଥା{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        ପ୍ରତିଜ୍ଞା( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pBest->t.z - zSql);
      ଯଦି( pBest->t.n!=nReplace ){
        memmove(&zOut[iOff + nReplace], &zOut[iOff + pBest->t.n],
            nOut - (iOff + pBest->t.n)
        );
        nOut += nReplace - pBest->t.n;
        zOut[nOut] = '\0';
      }
      memcpy(&zOut[iOff], zReplace, nReplace);
      sqlite3DbFree(db, pBest);
    }

    sqlite3_result_text(pCtx, zOut, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, zOut);
  }ଅନ୍ୟଥା{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  ଫେରନ୍ତୁ rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameSetENames(ପଦସମଷ୍ଟି_List *pEList, ପୂର୍ଣ୍ଣସଂଖ୍ୟା val){
  ପ୍ରତିଜ୍ଞା( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  ଯଦି( pEList ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
      ପ୍ରତିଜ୍ଞା( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  memset(&sNC, 0, ଆକାର(sNC));
  sNC.pParse = pParse;
  ପ୍ରତିଜ୍ଞା( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  ଯଦି( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  ଯଦି( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pNew->pWhen);
  }

  ପାଇଁ(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    ଯଦି( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      ଯଦି( pParse->nErr ) rc = pParse->rc;
    }
    ଯଦି( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      ଯଦି( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_ପଦସମଷ୍ଟି_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        ଯଦି( pSel==0 ){
          pStep->p_ପଦସମଷ୍ଟି_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }ଅନ୍ୟଥା{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_ପଦସମଷ୍ଟି_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_ପଦସମଷ୍ଟି_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          ପ୍ରତିଜ୍ଞା( pStep->p_ପଦସମଷ୍ଟି_List==0 || pStep->p_ପଦସମଷ୍ଟି_List==pSel->pEList );
          ପ୍ରତିଜ୍ଞା( pSrc==pSel->pSrc );
          ଯଦି( pStep->p_ପଦସମଷ୍ଟି_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        ଯଦି( ALWAYS(pStep->pSrc) ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
          ପାଇଁ(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            ଯଦି( p->fg.isSubquery ){
              ପ୍ରତିଜ୍ଞା( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        ଯଦି(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        ଯଦି( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pStep->pWhere);
        }
        ଯଦି( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pStep->p_ପଦସମଷ୍ଟି_List);
        }
        ପ୍ରତିଜ୍ଞା( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_ପଦସମଷ୍ଟି_List) );
        ଯଦି( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pUpsert->pUpsertTarget);
          ଯଦି( rc==SQLITE_OK ){
            ପଦସମଷ୍ଟି_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pUpsertSet);
          }
          ଯଦି( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pUpsert->pUpsertWhere);
          }
          ଯଦି( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }ଅନ୍ୟଥା{
        rc = SQLITE_NOMEM;
      }
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ସ୍ଥିର ଶୂନ୍ୟ renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  ପାଇଁ(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pStep->pWhere);
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pStep->p_ପଦସମଷ୍ଟି_List);
    ଯଦି( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pUpsert->pUpsertTargetWhere);
    }
    ଯଦି( pStep->pSrc ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      SrcList *pSrc = pStep->pSrc;
      ପାଇଁ(i=0; i<pSrc->nSrc; i++){
        ଯଦି( pSrc->a[i].fg.isSubquery ){
          ପ୍ରତିଜ୍ଞା( pSrc->a[i].u4.pSubq!=0 );
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
ସ୍ଥିର ଶୂନ୍ୟ renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  ଯଦି( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  ଯେତେବେଳେ( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_ସନ୍ଦେଶ);
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
ସ୍ଥିର ଶୂନ୍ୟ renameColumnFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ଅଚଳ ଅକ୍ଷର *zSql = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅଚଳ ଅକ୍ଷର *zDb = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[3]);
  ଅଚଳ ଅକ୍ଷର *zTable = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[4]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = sqlite3_ମୂଲ୍ୟ_int(argv[5]);
  ଅଚଳ ଅକ୍ଷର *zNew = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[6]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bQuote = sqlite3_ମୂଲ୍ୟ_int(argv[7]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp = sqlite3_ମୂଲ୍ୟ_int(argv[8]);
  ଅଚଳ ଅକ୍ଷର *zOld;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  ଯଦି( zSql==0 ) ଫେରନ୍ତୁ;
  ଯଦି( zTable==0 ) ଫେରନ୍ତୁ;
  ଯଦି( zNew==0 ) ଫେରନ୍ତୁ;
  ଯଦି( iCol<0 ) ଫେରନ୍ତୁ;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  ଯଦି( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    ଫେରନ୍ତୁ;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, ଆକାର(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, ଆକାର(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_ପଦସମଷ୍ଟି_Callback = rename_Column_ପଦସମଷ୍ଟି_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  ଯଦି( rc!=SQLITE_OK ) ଯାଅ renameColumnFunc_done;
  ଯଦି( sParse.pNewTable ){
    ଯଦି( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      ଯଦି( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      ଯଦି( rc!=SQLITE_OK ) ଯାଅ renameColumnFunc_done;
    }ଅନ୍ୟଥା ଯଦି( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      ଯଦି( bFKOnly==0 ){
        ଯଦି( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (ଶୂନ୍ୟ*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        ଯଦି( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (ଶୂନ୍ୟ*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, sParse.pNewTable->pCheck);
        ପାଇଁ(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, pIdx->a_Col_ପଦସମଷ୍ଟି);
        }
        ପାଇଁ(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, pIdx->a_Col_ପଦସମଷ୍ଟି);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        ପାଇଁ(i=0; i<sParse.pNewTable->nCol; i++){
          ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = sqlite3_Column_ପଦସମଷ୍ଟି(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, p_ପଦସମଷ୍ଟି);
        }
#endif
      }

      ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(sParse.pNewTable) );
      ପାଇଁ(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        ପାଇଁ(i=0; i<pFKey->nCol; i++){
          ଯଦି( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (ଶୂନ୍ୟ*)&pFKey->aCol[i]);
          }
          ଯଦି( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (ଶୂନ୍ୟ*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }ଅନ୍ୟଥା ଯଦି( sParse.pNewIndex ){
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, sParse.pNewIndex->a_Col_ପଦସମଷ୍ଟି);
    sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }ଅନ୍ୟଥା{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    ଯଦି( rc!=SQLITE_OK ) ଯାଅ renameColumnFunc_done;

    ପାଇଁ(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      ଯଦି( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        ଯଦି( pTarget==pTab ){
          ଯଦି( pStep->pUpsert ){
            ପଦସମଷ୍ଟି_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_ପଦସମଷ୍ଟି_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    ଯଦି( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  ଯଦି( rc!=SQLITE_OK ){
    ଯଦି( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_ମୂଲ୍ୟ(context, argv[0]);
    }ଅନ୍ୟଥା ଯଦି( sParse.z_Err_ସନ୍ଦେଶ ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }ଅନ୍ୟଥା{
      sqlite3_result_error_code(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rename_Table_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  RenameCtx *p = pWalker->u.pRename;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
   && ALWAYS(ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି))
   && p->pTab==p_ପଦସମଷ୍ଟି->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ଶୂନ୍ୟ*)&p_ପଦସମଷ୍ଟି->y.pTab);
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renameTableSelectCb(Walker *pWalker, Select *pSelect){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  ଯଦି( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ଯଦି( NEVER(pSrc==0) ){
    ପ୍ରତିଜ୍ଞା( pWalker->pParse->db->mallocFailed );
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ପାଇଁ(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    ଯଦି( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ଶୂନ୍ୟ renameTableFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ଅଚଳ ଅକ୍ଷର *zDb = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅଚଳ ଅକ୍ଷର *z_ନିବେଶ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[3]);
  ଅଚଳ ଅକ୍ଷର *zOld = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[4]);
  ଅଚଳ ଅକ୍ଷର *zNew = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[5]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp = sqlite3_ମୂଲ୍ୟ_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  ଯଦି( z_ନିବେଶ && zOld && zNew ){
    Parse sParse;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, ଆକାର(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, ଆକାର(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_ପଦସମଷ୍ଟି_Callback = rename_Table_ପଦସମଷ୍ଟି_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_ନିବେଶ, bTemp);

    ଯଦି( rc==SQLITE_OK ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLegacy = (db->flags & SQLITE_LegacyAlter);
      ଯଦି( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        ଯଦି( IsView(pTab) ){
          ଯଦି( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, ଆକାର(sNC));
            sNC.pParse = &sParse;

            ପ୍ରତିଜ୍ଞା( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            ଯଦି( sParse.nErr ){
              rc = sParse.rc;
            }ଅନ୍ୟଥା{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }ଅନ୍ୟଥା{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_ଚାବି
          ଯଦି( (isLegacy==0 || (db->flags & SQLITE_Foreign_ଚାବିଗୁଡ଼ିକ))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pTab) );
            ପାଇଁ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              ଯଦି( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (ଶୂନ୍ୟ*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          ଯଦି( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            ଯଦି( isLegacy==0 ){
              sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      ଅନ୍ୟଥା ଯଦି( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        ଯଦି( isLegacy==0 ){
          sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      ଅନ୍ୟଥା{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        ଯଦି( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        ଯଦି( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          ଯଦି( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            ପାଇଁ(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              ଯଦି( pStep->pSrc ){
                ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
                ପାଇଁ(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  ଯଦି( 0==sqlite3_stricmp(pItem->zName, zOld) ){
                    renameTokenFind(&sParse, &sCtx, pItem->zName);
                  }
                }
              }
            }
          }
        }
      }
#endif
    }

    ଯଦି( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_ନିବେଶ, zNew, bQuote);
    }
    ଯଦି( rc!=SQLITE_OK ){
      ଯଦି( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_ମୂଲ୍ୟ(context, argv[3]);
      }ଅନ୍ୟଥା ଯଦି( sParse.z_Err_ସନ୍ଦେଶ ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }ଅନ୍ୟଥା{
        sqlite3_result_error_code(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#ifndef SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#endif
  }

  ଫେରନ୍ତୁ;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rename_Quotefix_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_STRING && (p_ପଦସମଷ୍ଟି->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (ଅଚଳ ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି);
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ଶୂନ୍ୟ renameQuotefixFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ଅକ୍ଷର ଅଚଳ *zDb = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅକ୍ଷର ଅଚଳ *z_ନିବେଶ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  ଯଦି( zDb && z_ନିବେଶ ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_ନିବେଶ, 0);

    ଯଦି( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, ଆକାର(RenameCtx));
      memset(&sWalker, 0, ଆକାର(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_ପଦସମଷ୍ଟି_Callback = rename_Quotefix_ପଦସମଷ୍ଟି_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      ଯଦି( sParse.pNewTable ){
        ଯଦି( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          ଯଦି( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }ଅନ୍ୟଥା{
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          ପାଇଁ(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker,
               sqlite3_Column_ପଦସମଷ୍ଟି(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }ଅନ୍ୟଥା ଯଦି( sParse.pNewIndex ){
        sqlite3_Walk_ପଦସମଷ୍ଟି_List(&sWalker, sParse.pNewIndex->a_Col_ପଦସମଷ୍ଟି);
        sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }ଅନ୍ୟଥା{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        ଯଦି( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      ଯଦି( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_ନିବେଶ, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    ଯଦି( rc!=SQLITE_OK ){
      ଯଦି( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_ମୂଲ୍ୟ(context, argv[1]);
      }ଅନ୍ୟଥା{
        sqlite3_result_error_code(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif

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
ସ୍ଥିର ଶୂନ୍ୟ renameTableTest(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ଅକ୍ଷର ଅଚଳ *zDb = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅକ୍ଷର ଅଚଳ *z_ନିବେଶ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTemp = sqlite3_ମୂଲ୍ୟ_int(argv[4]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLegacy = (db->flags & SQLITE_LegacyAlter);
  ଅକ୍ଷର ଅଚଳ *zWhen = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[5]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bNoDQS = sqlite3_ମୂଲ୍ୟ_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  ଯଦି( zDb && z_ନିବେଶ ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    Parse sParse;
    u64 flags = db->flags;
    ଯଦି( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_ନିବେଶ, bTemp);
    db->flags = flags;
    ଯଦି( rc==SQLITE_OK ){
      ଯଦି( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, ଆକାର(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        ଯଦି( sParse.nErr ) rc = sParse.rc;
      }

      ଅନ୍ୟଥା ଯଦି( sParse.pNewTrigger ){
        ଯଦି( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        ଯଦି( rc==SQLITE_OK ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା i2 = sqlite3FindDbName(db, zDb);
          ଯଦି( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    ଯଦି( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      renameColumnParseError(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getConstraintToken(ଅଚଳ u8 *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piToken){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
  କର {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }ଯେତେବେଳେ( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  ଯଦି( t==TK_LP ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNest = 1;
    ଯେତେବେଳେ( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      ଯଦି( t==TK_LP ){
        nNest++;
      }ଅନ୍ୟଥା ଯଦି( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }ଅନ୍ୟଥା ଯଦି( t==TK_ILLEGAL ){
        ଭାଙ୍ଗ;
      }
    }
  }

  *piToken = t;
  ଫେରନ୍ତୁ iOff;
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
ସ୍ଥିର ଶୂନ୍ୟ dropColumnFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSchema = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
  ଅଚଳ ଅକ୍ଷର *zSql = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = sqlite3_ମୂଲ୍ୟ_int(argv[2]);
  ଅଚଳ ଅକ୍ଷର *zDb = db->aDb[iSchema].zDbSName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  ଅଚଳ ଅକ୍ଷର *zEnd;
  ଅକ୍ଷର *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  ଯଦି( rc!=SQLITE_OK ) ଯାଅ drop_column_done;
  pTab = sParse.pNewTable;
  ଯଦି( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    ଯାଅ drop_column_done;
  }

  ଯଦି( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (ଶୂନ୍ୟ*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (ଶୂନ୍ୟ*)pTab->aCol[iCol+1].zCnName);
    zEnd = (ଅଚଳ ଅକ୍ଷର*)pEnd->t.z;
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTok;
    ପ୍ରତିଜ୍ଞା( IsOrdinaryTable(pTab) );
    ପ୍ରତିଜ୍ଞା( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (ଶୂନ୍ୟ*)pTab->aCol[iCol-1].zCnName);
    କର {
      pCol->t.z += getConstraintToken((ଅଚଳ u8*)pCol->t.z, &eTok);
    }ଯେତେବେଳେ( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (ଅଚଳ ଅକ୍ଷର*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  ଯଦି( rc!=SQLITE_OK ){
    sqlite3_result_error_code(context, rc);
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
ଶୂନ୍ୟ sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, ଅଚଳ Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                        /* Index of db containing pTab in aDb[] */
  ଅଚଳ ଅକ୍ଷର *zDb;                /* Database containing pTab ("main" etc.) */
  ଅକ୍ଷର *zCol = 0;                 /* Name of column to drop */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  ପ୍ରତିଜ୍ଞା( pParse->pNewTable==0 );
  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  ଯଦି( NEVER(db->mallocFailed) ) ଯାଅ exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ଯଦି( !pTab ) ଯାଅ exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  ଯଦି( SQLITE_OK!=isAlterableTable(pParse, pTab) ) ଯାଅ exit_drop_column;
  ଯଦି( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) ଯାଅ exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  ଯଦି( zCol==0 ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    ଯାଅ exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  ଯଦି( iCol<0 ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such column: \"%T\"", pName);
    ଯାଅ exit_drop_column;
  }
  ଯଦି( isRowidAlias(pParse, pTab, zCol, "drop") ) ଯାଅ exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  ଯଦି( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    ଯାଅ exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  ଯଦି( pTab->nCol<=1 ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    ଯାଅ exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  ପ୍ରତିଜ୍ଞା( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    ଯାଅ exit_drop_column;
  }
#endif
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, iCol, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  ଯଦି( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRec;
    Index *pPk = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nField = 0;               /* Number of non-virtual columns after drop */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    ଯଦି( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }ଅନ୍ୟଥା{
      pPk = sqlite3_Primary_ଚାବି_Index(pTab);
      pParse->nMem += pPk->nColumn;
      ପାଇଁ(i=0; i<pPk->n_ଚାବି_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_ଚାବି_Col;
    }
    regRec = ++pParse->nMem;
    ପାଇଁ(i=0; i<pTab->nCol; i++){
      ଯଦି( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut;
        ଯଦି( pPk ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPos = sqlite3TableColumnToIndex(pPk, i);
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          ଯଦି( iPos<pPk->n_ଚାବି_Col ) ଜାରି;
          regOut = reg+1+iPos-(iPos>iColPos);
        }ଅନ୍ୟଥା{
          regOut = reg+1+nField;
        }
        ଯଦି( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }ଅନ୍ୟଥା{
          sqlite3_ପଦସମଷ୍ଟି_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          ପ୍ରତିଜ୍ଞା( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    ଯଦି( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    ଯଦି( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iCur, regRec, reg+1, pPk->n_ଚାବି_Col);
    }ଅନ୍ୟଥା{
      sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3VdbeAddOp2(v, OP_Next, iCur, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
  }

exit_drop_column:
  sqlite3DbFree(db, zCol);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getWhitespace(ଅଚଳ u8 *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRet = 0;
  ଯେତେବେଳେ( 1 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sqlite3GetToken(&z[nRet], &t);
    ଯଦି( t!=TK_SPACE && t!=TK_COMMENT ) ଭାଙ୍ଗ;
    nRet += n;
  }
  ଫେରନ୍ତୁ nRet;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getConstraint(ଅଚଳ u8 *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTok){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;

#ifndef SQLITE_OMIT_FOREIGN_ଚାବି
  ଯଦି( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  ଯଦି( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#endif

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  ଯେତେବେଳେ( 1 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = getConstraintToken(&z[iOff], &t);
    ଯଦି( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      ଭାଙ୍ଗ;
    }
    iOff += n;
  }
  
  ଫେରନ୍ତୁ iOff;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t,                 /* Token type */
  ଅଚଳ u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nQuote,            /* Length of zQuote in bytes */
  ଅଚଳ u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  ଅକ୍ଷର *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  ଯଦି( t==TK_ILLEGAL ){
    *pRes = 1;
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  ଯଦି( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ଅଚଳ ଅକ୍ଷର*)zCopy, (ଅଚଳ ଅକ୍ଷର*)zCmp);
  sqlite3_free(zCopy);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା skipCreateTable(sqlite3_context *ctx, ଅଚଳ u8 *zSql, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piOff){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;

  ଯଦି( zSql==0 ) ଫେରନ୍ତୁ SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  ଯେତେବେଳେ( 1 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    ଯଦି( t==TK_LP ) ଭାଙ୍ଗ;
    ଯଦି( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ଶୂନ୍ୟ dropConstraintFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅଚଳ u8 *zSql = sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅଚଳ u8 *zCons = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNotNull = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStart = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd = 0;
  ଅକ୍ଷର *zNew = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  ଯଦି( zSql==0 ) ଫେରନ୍ତୁ;

  /* Jump past the "CREATE TABLE" bit. */
  ଯଦି( skipCreateTable(ctx, zSql, &iOff) ) ଫେରନ୍ତୁ;

  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_ମୂଲ୍ୟ_int(argv[1]);
  }ଅନ୍ୟଥା{
    zCons = sqlite3_ମୂଲ୍ୟ_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  ପାଇଁ(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    ଯେତେବେଳେ( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      ଯଦି( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTok = 0;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        ଯଦି( zCons ){
          ଯଦି( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ଫେରନ୍ତୁ;
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
        ଯଦି( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }ଅନ୍ୟଥା{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        ଯଦି( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          ଯଦି( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            ଫେରନ୍ତୁ;
          }
          iEnd = iOff;
          ଭାଙ୍ଗ;
        }

      }ଅନ୍ୟଥା ଯଦି( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା ଯଦି( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        ଭାଙ୍ଗ;
      }ଅନ୍ୟଥା ଯଦି( t==TK_COMMA ){
        ଭାଙ୍ଗ;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  ଯଦି( iEnd<=0 ){
    ଯଦି( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }ଅନ୍ୟଥା{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (ଅଚଳ ଅକ୍ଷର*)zSql, -1, SQLITE_TRANSIENT);
    }
  }ଅନ୍ୟଥା{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ଅଚଳ ଅକ୍ଷର *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    ଯଦି( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      ଯଦି( zSql[iStart-1]==',' ) iStart--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", iStart, zSql, zSpace, &zSql[iEnd]);
    sqlite3_result_text(ctx, zNew, -1, SQLITE_DYNAMIC);
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
ସ୍ଥିର ଶୂନ୍ୟ addConstraintFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅଚଳ u8 *zSql = sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ଅଚଳ ଅକ୍ଷର *zCons = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = sqlite3_ମୂଲ୍ୟ_int(argv[2]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  sqlite3_str *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
  UNUSED_PARAMETER(NotUsed);

  ଯଦି( skipCreateTable(ctx, zSql, &iOff) ) ଫେରନ୍ତୁ;
  
  ପାଇଁ(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ଯେତେବେଳେ( 1 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTok = getConstraintToken(&zSql[iOff], &t);
      ଯଦି( t==TK_COMMA || t==TK_RP ) ଭାଙ୍ଗ;
      ଯଦି( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        ଫେରନ୍ତୁ;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ଅଚଳ ଅକ୍ଷର*)zSql, iOff);
  ଯଦି( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା alterFindCol(Parse *pParse, Table *pTab, Token *pCol, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piCol){
  sqlite3 *db = pParse->db;
  ଅକ୍ଷର *zName = sqlite3NameFromToken(db, pCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_NOMEM;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = -1;

  ଯଦି( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    ଯଦି( iCol<0 ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }ଅନ୍ୟଥା{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  ଯଦି( rc==SQLITE_OK ){
    ଅଚଳ ଅକ୍ଷର *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    ଅଚଳ ଅକ୍ଷର *zCol = pTab->aCol[iCol].zCnName;
    ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piDb,            /* OUT: write the iDb here */
  ଅଚଳ ଅକ୍ଷର **pzDb,    /* OUT: write name of schema here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  ପ୍ରତିଜ୍ଞା( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ଯଦି( pTab ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    ଯଦି( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  ଯଦି( pTab && bAuth ){
    ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  ଫେରନ୍ତୁ pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
ଶୂନ୍ୟ sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = 0;
  ଅଚଳ ଅକ୍ଷର *zDb = 0;
  ଅକ୍ଷର *zArg = 0;

  ପ୍ରତିଜ୍ଞା( (pCol==0)!=(pCons==0) );
  ପ୍ରତିଜ୍ଞା( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  ଯଦି( !pTab ) ଫେରନ୍ତୁ;

  ଯଦି( pCons ){
    ଅକ୍ଷର *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
    ଯଦି( alterFindCol(pParse, pTab, pCol, &iCol) ) ଫେରନ୍ତୁ;
    zArg = sqlite3MPrintf(db, "%d", iCol);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
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
ସ୍ଥିର ଶୂନ୍ୟ failConstraintFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅଚଳ ଅକ୍ଷର *zText = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = sqlite3_ମୂଲ୍ୟ_int(argv[1]);
  (ଶୂନ୍ୟ)NotUsed;
  sqlite3_result_error(ctx, zText, -1);
  sqlite3_result_error_code(ctx, err);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ଅଚଳ ଅକ୍ଷର *pCons,              /* Buffer containing constraint */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd = 0;

  ଯଦି( zTmp==0 ) ଫେରନ୍ତୁ 0;

  ଯେତେବେଳେ( 1 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nToken = sqlite3GetToken(&zTmp[iOff], &t);
    ଯଦି( t==TK_ILLEGAL ) ଭାଙ୍ଗ;
    ଯଦି( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  ଫେରନ୍ତୁ iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
ଶୂନ୍ୟ sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = 0;
  ଅଚଳ ଅକ୍ଷର *zDb = 0;
  ଅଚଳ ଅକ୍ଷର *pCons = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCons = 0;

  /* Look up the table being altered. */
  ପ୍ରତିଜ୍ଞା( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  ଯଦି( !pTab ) ଫେରନ୍ତୁ;

  /* Find the column being altered. */
  ଯଦି( alterFindCol(pParse, pTab, pCol, &iCol) ){
    ଫେରନ୍ତୁ;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pCol->n, pCol->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, iCol, nCons, pCons, iCol, pTab->zName
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
ସ୍ଥିର ଶୂନ୍ୟ findConstraintFunc(
  sqlite3_context *ctx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NotUsed,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅଚଳ u8 *zSql = 0;
  ଅଚଳ u8 *zCons = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = 0;

  (ଶୂନ୍ୟ)NotUsed;
  zSql = sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  zCons = sqlite3_ମୂଲ୍ୟ_text(argv[1]);

  ଯଦି( zSql==0 || zCons==0 ) ଫେରନ୍ତୁ;
  ଯେତେବେଳେ( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  ଯେତେବେଳେ( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ଯଦି( t==TK_CONSTRAINT ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTok = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      ଯଦି( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ଫେରନ୍ତୁ;
      ଯଦି( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        ଫେରନ୍ତୁ;
      }
    }ଅନ୍ୟଥା ଯଦି( t==TK_ILLEGAL ){
      ଭାଙ୍ଗ;
    }
  }

  sqlite3_result_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
ଶୂନ୍ୟ sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ଅଚଳ ଅକ୍ଷର *z_ପଦସମଷ୍ଟି,       /* Text of CHECK expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି,               /* Size of pExpr in bytes */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = 0;             /* Which schema does pTab live in */
  ଅଚଳ ଅକ୍ଷର *zDb = 0;     /* Name of the schema in which pTab lives */
  ଅଚଳ ଅକ୍ଷର *pCons = 0;   /* Text of the constraint */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCons;               /* Bytes of text to use from pCons[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  ପ୍ରତିଜ୍ଞା( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  ଯଦି( !pTab ){
    sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ପଦସମଷ୍ଟି);
    ଫେରନ୍ତୁ;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_ପଦସମଷ୍ଟି, 0);
  sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ପଦସମଷ୍ଟି);
  ଯଦି( rc ) ଫେରନ୍ତୁ;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  ଯଦି( pName ){
    ଅକ୍ଷର *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_TABLE " "
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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_ପଦସମଷ୍ଟି, z_ପଦସମଷ୍ଟି
  );

  /* Edit the SQL for the named table. */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE " SET "
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
ଶୂନ୍ୟ sqlite3AlterFunctions(ଶୂନ୍ୟ){
  ସ୍ଥିର FuncDef aAlterTableFuncs[] = {
    INTERNAL_FUNCTION(sqlite_rename_column,  9, renameColumnFunc),
    INTERNAL_FUNCTION(sqlite_rename_table,   7, renameTableFunc),
    INTERNAL_FUNCTION(sqlite_rename_test,    7, renameTableTest),
    INTERNAL_FUNCTION(sqlite_drop_column,    3, dropColumnFunc),
    INTERNAL_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    INTERNAL_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    INTERNAL_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    INTERNAL_FUNCTION(sqlite_add_constraint, 3, addConstraintFunc),
    INTERNAL_FUNCTION(sqlite_find_constraint,2, findConstraintFunc),
  };
  sqlite3_ଭର୍ତ୍ତି_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
