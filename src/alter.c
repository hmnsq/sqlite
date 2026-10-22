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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isAlterableTable(Parse *pParse, Table *pTab){
  ಯದಿ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "table %s may not be altered", pTab->zName);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ಧ್ರುವ ವರ್ಣ *zDb,                /* Name of db to verify schema of */
  ಪೂರ್ಣಾಙ್ಕ bTemp,                      /* True if this is the temp db */
  ಧ್ರುವ ವರ್ಣ *zWhen,              /* "when" part of error message */
  ಪೂರ್ಣಾಙ್ಕ bNoDQS                      /* Do not allow DQS in the schema */
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

  ಯದಿ( bTemp==0 ){
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
ಸ್ಥಿರ ಶೂನ್ಯ renameFixQuotes(Parse *pParse, ಧ್ರುವ ವರ್ಣ *zDb, ಪೂರ್ಣಾಙ್ಕ bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  ಯದಿ( bTemp==0 ){
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
ಸ್ಥಿರ ಶೂನ್ಯ renameReloadSchema(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  ಯದಿ( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    ಯದಿ( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
ಶೂನ್ಯ sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  ಪೂರ್ಣಾಙ್ಕ iDb;                  /* Database that contains the table */
  ವರ್ಣ *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  ವರ್ಣ *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ nTabName;             /* Number of UTF-8 characters in zTabName */
  ಧ್ರುವ ವರ್ಣ *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  ಯದಿ( NEVER(db->mallocFailed) ) ಗಚ್ಛ exit_rename_table;
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ಯದಿ( !pTab ) ಗಚ್ಛ exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  ಯದಿ( !zName ) ಗಚ್ಛ exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  ಯದಿ( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
        "there is already another table or index with this name: %s", zName);
    ಗಚ್ಛ exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  ಯದಿ( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    ಗಚ್ಛ exit_rename_table;
  }
  ಯದಿ( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    ಗಚ್ಛ exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  ಯದಿ( IsView(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "view %s may not be altered", pTab->zName);
    ಗಚ್ಛ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ಗಚ್ಛ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಯದಿ( sqlite3ViewGetColumnNames(pParse, pTab) ){
    ಗಚ್ಛ exit_rename_table;
  }
  ಯದಿ( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    ಯದಿ( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  ಯದಿ( v==0 ){
    ಗಚ್ಛ exit_rename_table;
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
  ಯದಿ( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  ಯದಿ( iDb!=1 ){
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
  ಯದಿ( pVTab ){
    ಪೂರ್ಣಾಙ್ಕ i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(ಧ್ರುವ ವರ್ಣ*)pVTab, P4_VTAB);
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  ಧ್ರುವ ವರ್ಣ *zDb,      /* Schema holding the table */
  ಧ್ರುವ ವರ್ಣ *zTab,     /* Table to check for empty */
  ಧ್ರುವ ವರ್ಣ *zErr      /* Error message text */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  ಧ್ರುವ ವರ್ಣ *zCol, 
  ಧ್ರುವ ವರ್ಣ *zOp
){
  ಯದಿ( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
ಶೂನ್ಯ sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  ಪೂರ್ಣಾಙ್ಕ iDb;                  /* Database number */
  ಧ್ರುವ ವರ್ಣ *zDb;          /* Database name */
  ಧ್ರುವ ವರ್ಣ *zTab;         /* Table name */
  ವರ್ಣ *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  ಅಭಿವ್ಯಕ್ತಿ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  ಪೂರ್ಣಾಙ್ಕ r1;                   /* Temporary registers */

  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  ಪ್ರತಿಜ್ಞಾ( pNew );

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  ಪ್ರತಿಜ್ಞಾ( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  ಯದಿ( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "Cannot add a PRIMARY KEY column");
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( pNew->pIndex ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
         "Cannot add a UNIQUE column");
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    ಪ್ರತಿಜ್ಞಾ( pDflt==0 || pDflt->op==TK_SPAN );
    ಯದಿ( pDflt && pDflt->p_ಎಡ->op==TK_NULL ){
      pDflt = 0;
    }
    ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pNew) );
    ಯದಿ( (db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    ಯದಿ( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    ಯದಿ( pDflt ){
      sqlite3_ಮೌಲ್ಯ *pVal = 0;
      ಪೂರ್ಣಾಙ್ಕ rc;
      rc = sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      ಯದಿ( rc!=SQLITE_OK ){
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed == 1 );
        ಪ್ರತ್ಯಾಗಮ;
      }
      ಯದಿ( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_ಮೌಲ್ಯ_Free(pVal);
    }
  }ಅನ್ಯಥಾ ಯದಿ( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (ವರ್ಣ*)pColDef->z, pColDef->n);
  ಯದಿ( zCol ){
    ವರ್ಣ *zEnd = &zCol[pColDef->n-1];
    ಯಾವತ್( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
    ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pNew) );
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
  ಯದಿ( v ){
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
    ಯದಿ( pNew->pCheck!=0
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
ಶೂನ್ಯ sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  ಪೂರ್ಣಾಙ್ಕ iDb;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTable==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಯದಿ( NEVER(db->mallocFailed) ) ಗಚ್ಛ exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ಯದಿ( !pTab ) ಗಚ್ಛ exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಯದಿ( IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "virtual tables may not be altered");
    ಗಚ್ಛ exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  ಯದಿ( IsView(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "Cannot add a column to a view");
    ಗಚ್ಛ exit_begin_add_column;
  }
  ಯದಿ( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    ಗಚ್ಛ exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
  ಪ್ರತಿಜ್ಞಾ( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Table));
  ಯದಿ( !pNew ) ಗಚ್ಛ exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  ಪ್ರತಿಜ್ಞಾ( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  ಪ್ರತಿಜ್ಞಾ( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  ಯದಿ( !pNew->aCol || !pNew->zName ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಗಚ್ಛ exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, ಪರಿಮಾಣ(Column)*(size_t)pNew->nCol);
  ಪ್ರತಿ(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  ಪ್ರತಿಜ್ಞಾ( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  ಪ್ರತ್ಯಾಗಮ;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#ಯದಿ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isRealTable(Parse *pParse, Table *pTab, ಪೂರ್ಣಾಙ್ಕ iOp){
  ಧ್ರುವ ವರ್ಣ *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  ಯದಿ( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಯದಿ( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  ಯದಿ( zType ){
    ಧ್ರುವ ವರ್ಣ *az_ಸಂದೇಶ[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    ಪ್ರತಿಜ್ಞಾ( iOp>=0 && iOp<ArraySize(az_ಸಂದೇಶ) );
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot %s %s \"%s\"",
        az_ಸಂದೇಶ[iOp], zType, pTab->zName
    );
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
ಶೂನ್ಯ sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  ಪೂರ್ಣಾಙ್ಕ iCol;                       /* Index of column being renamed */
  ವರ್ಣ *zOld = 0;                 /* Old column name */
  ವರ್ಣ *zNew = 0;                 /* New column name */
  ಧ್ರುವ ವರ್ಣ *zDb;                /* Name of schema containing the table */
  ಪೂರ್ಣಾಙ್ಕ iSchema;                    /* Index of the schema */
  ಪೂರ್ಣಾಙ್ಕ bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ಯದಿ( !pTab ) ಗಚ್ಛ exit_rename_column;

  /* Cannot alter a system table */
  ಯದಿ( SQLITE_OK!=isAlterableTable(pParse, pTab) ) ಗಚ್ಛ exit_rename_column;
  ಯದಿ( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) ಗಚ್ಛ exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ಗಚ್ಛ exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  ಯದಿ( !zOld ) ಗಚ್ಛ exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  ಯದಿ( iCol<0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: \"%T\"", pOld);
    ಗಚ್ಛ exit_rename_column;
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
  ಯದಿ( !zNew ) ಗಚ್ಛ exit_rename_column;
  ಯದಿ( isRowidAlias(pParse, pTab, zOld, "rename from") ) ಗಚ್ಛ exit_rename_column;
  ಯದಿ( isRowidAlias(pParse, pTab, zNew, "rename to") ) ಗಚ್ಛ exit_rename_column;
  ಪ್ರತಿಜ್ಞಾ( pNew->n>0 );
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
  ಯದಿ( iSchema!=1 ){
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
  ಪ್ರತ್ಯಾಗಮ;
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
ಸಂರಚನಾ RenameToken {
  ಧ್ರುವ ಶೂನ್ಯ *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ RenameCtx RenameCtx;
ಸಂರಚನಾ RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  ಪೂರ್ಣಾಙ್ಕ nList;                      /* Number of tokens in pList */
  ಪೂರ್ಣಾಙ್ಕ iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  ಧ್ರುವ ವರ್ಣ *zOld;               /* Old column name */
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
ಸ್ಥಿರ ಶೂನ್ಯ renameTokenCheckAll(Parse *pParse, ಧ್ರುವ ಶೂನ್ಯ *pPtr){
  ಪ್ರತಿಜ್ಞಾ( pParse==pParse->db->pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಯದಿ( pParse->nErr==0 ){
    ಧ್ರುವ RenameToken *p;
    u32 i = 1;
    ಪ್ರತಿ(p=pParse->pRename; p; p=p->pNext){
      ಯದಿ( p->p ){
        ಪ್ರತಿಜ್ಞಾ( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( i>0 );
  }
}
#ಅನ್ಯಥಾ
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
ಧ್ರುವ ಶೂನ್ಯ *sqlite3RenameTokenMap(
  Parse *pParse,
  ಧ್ರುವ ಶೂನ್ಯ *pPtr,
  ಧ್ರುವ Token *pToken
){
  RenameToken *pNew;
  ಪ್ರತಿಜ್ಞಾ( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  ಯದಿ( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, ಪರಿಮಾಣ(RenameToken));
    ಯದಿ( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  ಪ್ರತ್ಯಾಗಮ pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
ಶೂನ್ಯ sqlite3RenameTokenRemap(Parse *pParse, ಧ್ರುವ ಶೂನ್ಯ *pTo, ಧ್ರುವ ಶೂನ್ಯ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  ಪ್ರತಿ(p=pParse->pRename; p; p=p->pNext){
    ಯದಿ( p->p==pFrom ){
      p->p = pTo;
      ಭಙ್ಗ;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ಧ್ರುವ ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3RenameTokenRemap(pParse, 0, (ಧ್ರುವ ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  ಯದಿ( pWith ){
    Parse *pParse = pWalker->pParse;
    ಪೂರ್ಣಾಙ್ಕ i;
    With *pCopy = 0;
    ಪ್ರತಿಜ್ಞಾ( pWith->nCte>0 );
    ಯದಿ( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ತಳ್ಳು(pParse, pCopy, 1);
    }
    ಪ್ರತಿ(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
      sNC.pParse = pParse;
      ಯದಿ( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      ಯದಿ( sNC.pParse->db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    ಯದಿ( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unmapColumnIdlistNames(
  Parse *pParse,
  ಧ್ರುವ IdList *pIdList
){
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿಜ್ಞಾ( pIdList!=0 );
  ಪ್ರತಿ(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ಧ್ರುವ ಶೂನ್ಯ*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  ಯದಿ( p->selFlags & (SF_View|SF_CopyCte) ){
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ಯದಿ( ALWAYS(p->pEList) ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = p->pEList;
    ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಯದಿ( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pList->a[i].zEName);
      }
    }
  }
  ಯದಿ( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pSrc->a[i].zName);
      ಯದಿ( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pSrc->a[i].u3.pOn);
      }ಅನ್ಯಥಾ{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
ಶೂನ್ಯ sqlite3_Rename_ಅಭಿವ್ಯಕ್ತಿ_Unmap(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
  sWalker.pParse = pParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
ಶೂನ್ಯ sqlite3RenameExprlistUnmap(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಯದಿ( pEList ){
    ಪೂರ್ಣಾಙ್ಕ i;
    Walker sWalker;
    memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
    sWalker.pParse = pParse;
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pEList);
    ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಯದಿ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  ಪ್ರತಿ(p=pToken; p; p=pNext){
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
ಸ್ಥಿರ RenameToken *renameTokenFind(
  Parse *pParse,
  ಸಂರಚನಾ RenameCtx *pCtx,
  ಧ್ರುವ ಶೂನ್ಯ *pPtr
){
  RenameToken **pp;
  ಯದಿ( NEVER(pPtr==0) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿ(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    ಯದಿ( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      ಯದಿ( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      ಪ್ರತ್ಯಾಗಮ pToken;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameColumnSelectCb(Walker *pWalker, Select *p){
  ಯದಿ( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rename_Column_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  RenameCtx *p = pWalker->u.pRename;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER
   && p_ಅಭಿವ್ಯಕ್ತಿ->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
   && p_ಅಭಿವ್ಯಕ್ತಿ->iColumn==p->iCol
   && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ))
   && p->pTab==p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  ಪ್ರತಿ(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    ಯದಿ( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  ಪ್ರತಿ(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  ಪ್ರತ್ಯಾಗಮ pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ಸ್ಥಿರ ಶೂನ್ಯ errorMPrintf(sqlite3_context *pCtx, ಧ್ರುವ ವರ್ಣ *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ವರ್ಣ *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  ಯದಿ( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಶೂನ್ಯ renameColumnParseError(
  sqlite3_context *pCtx,
  ಧ್ರುವ ವರ್ಣ *zWhen,
  sqlite3_ಮೌಲ್ಯ *pType,
  sqlite3_ಮೌಲ್ಯ *pObject,
  Parse *pParse
){
  ಧ್ರುವ ವರ್ಣ *zT = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pType);
  ಧ್ರುವ ವರ್ಣ *zN = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pObject);
  ವರ್ಣ *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_ಸಂದೇಶ
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
ಸ್ಥಿರ ಶೂನ್ಯ renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pEList,
  ಧ್ರುವ ವರ್ಣ *zOld
){
  ಯದಿ( pEList ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಧ್ರುವ ವರ್ಣ *zName = pEList->a[i].zEName;
      ಯದಿ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (ಧ್ರುವ ಶೂನ್ಯ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ಧ್ರುವ IdList *pIdList,
  ಧ್ರುವ ವರ್ಣ *zOld
){
  ಯದಿ( pIdList ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pIdList->nId; i++){
      ಧ್ರುವ ವರ್ಣ *zName = pIdList->a[i].zName;
      ಯದಿ( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (ಧ್ರುವ ಶೂನ್ಯ*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ಧ್ರುವ ವರ್ಣ *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ಧ್ರುವ ವರ್ಣ *zSql,               /* SQL to parse */
  ಪೂರ್ಣಾಙ್ಕ bTemp                       /* True if SQL is from temp schema */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  ಯದಿ( zSql==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  ಯದಿ( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_CORRUPT_BKPT;
  }
  ಯದಿ( bTemp ){
    db->init.iDb = 1;
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3FindDbName(db, zDb);
    ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<=0xff );
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
  ಯದಿ( db->mallocFailed ) rc = SQLITE_NOMEM;
  ಯದಿ( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  ಯದಿ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಙ್ಕ nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    ಪ್ರತಿ(pToken=p->pRename; pToken; pToken=pToken->pNext){
      ಪ್ರತಿಜ್ಞಾ( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ಧ್ರುವ ವರ್ಣ *zSql,               /* SQL statement to edit */
  ಧ್ರುವ ವರ್ಣ *zNew,               /* New token text */
  ಪೂರ್ಣಾಙ್ಕ bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ವರ್ಣ *zQuot = 0;
  ವರ್ಣ *zOut;
  i64 nQuot = 0;
  ವರ್ಣ *zBuf1 = 0;
  ವರ್ಣ *zBuf2 = 0;

  ಯದಿ( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    ಯದಿ( zQuot==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }ಅನ್ಯಥಾ{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    ಪ್ರತಿಜ್ಞಾ( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( nSql>0 );
    zOut = (ವರ್ಣ*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    ಯದಿ( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  ಯದಿ( zOut ){
    i64 nOut = nSql;
    ಪ್ರತಿಜ್ಞಾ( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    ಯಾವತ್( pRename->pList ){
      ಪೂರ್ಣಾಙ್ಕ iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ಧ್ರುವ ವರ್ಣ *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      ಯದಿ( zNew ){
        ಯದಿ( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }ಅನ್ಯಥಾ{
          nReplace = nQuot;
          zReplace = zQuot;
          ಯದಿ( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }ಅನ್ಯಥಾ{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        ಪ್ರತಿಜ್ಞಾ( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((ಪೂರ್ಣಾಙ್ಕ)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (ಪೂರ್ಣಾಙ್ಕ)(pBest->t.z - zSql);
      ಯದಿ( pBest->t.n!=nReplace ){
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
  }ಅನ್ಯಥಾ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameSetENames(ಅಭಿವ್ಯಕ್ತಿ_List *pEList, ಪೂರ್ಣಾಙ್ಕ val){
  ಪ್ರತಿಜ್ಞಾ( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  ಯದಿ( pEList ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪ್ರತಿಜ್ಞಾ( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
  sNC.pParse = pParse;
  ಪ್ರತಿಜ್ಞಾ( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  ಯದಿ( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  ಯದಿ( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pNew->pWhen);
  }

  ಪ್ರತಿ(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    ಯದಿ( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      ಯದಿ( pParse->nErr ) rc = pParse->rc;
    }
    ಯದಿ( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      ಯದಿ( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        ಯದಿ( pSel==0 ){
          pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }ಅನ್ಯಥಾ{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          ಪ್ರತಿಜ್ಞಾ( pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List==0 || pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List==pSel->pEList );
          ಪ್ರತಿಜ್ಞಾ( pSrc==pSel->pSrc );
          ಯದಿ( pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        ಯದಿ( ALWAYS(pStep->pSrc) ){
          ಪೂರ್ಣಾಙ್ಕ i;
          ಪ್ರತಿ(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            ಯದಿ( p->fg.isSubquery ){
              ಪ್ರತಿಜ್ಞಾ( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        ಯದಿ(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        ಯದಿ( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pStep->pWhere);
        }
        ಯದಿ( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List);
        }
        ಪ್ರತಿಜ್ಞಾ( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List) );
        ಯದಿ( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pUpsert->pUpsertTarget);
          ಯದಿ( rc==SQLITE_OK ){
            ಅಭಿವ್ಯಕ್ತಿ_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pUpsertSet);
          }
          ಯದಿ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pUpsert->pUpsertWhere);
          }
          ಯದಿ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }ಅನ್ಯಥಾ{
        rc = SQLITE_NOMEM;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  ಪ್ರತಿ(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pStep->pWhere);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List);
    ಯದಿ( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pUpsert->pUpsertTargetWhere);
    }
    ಯದಿ( pStep->pSrc ){
      ಪೂರ್ಣಾಙ್ಕ i;
      SrcList *pSrc = pStep->pSrc;
      ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
        ಯದಿ( pSrc->a[i].fg.isSubquery ){
          ಪ್ರತಿಜ್ಞಾ( pSrc->a[i].u4.pSubq!=0 );
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
ಸ್ಥಿರ ಶೂನ್ಯ renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  ಯದಿ( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  ಯಾವತ್( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_ಸಂದೇಶ);
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
ಸ್ಥಿರ ಶೂನ್ಯ renameColumnFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ಧ್ರುವ ವರ್ಣ *zSql = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಧ್ರುವ ವರ್ಣ *zDb = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[3]);
  ಧ್ರುವ ವರ್ಣ *zTable = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[4]);
  ಪೂರ್ಣಾಙ್ಕ iCol = sqlite3_ಮೌಲ್ಯ_int(argv[5]);
  ಧ್ರುವ ವರ್ಣ *zNew = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[6]);
  ಪೂರ್ಣಾಙ್ಕ bQuote = sqlite3_ಮೌಲ್ಯ_int(argv[7]);
  ಪೂರ್ಣಾಙ್ಕ bTemp = sqlite3_ಮೌಲ್ಯ_int(argv[8]);
  ಧ್ರುವ ವರ್ಣ *zOld;
  ಪೂರ್ಣಾಙ್ಕ rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  ಪೂರ್ಣಾಙ್ಕ i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  ಯದಿ( zSql==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( zTable==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( zNew==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( iCol<0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  ಯದಿ( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    ಪ್ರತ್ಯಾಗಮ;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, ಪರಿಮಾಣ(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Column_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ renameColumnFunc_done;
  ಯದಿ( sParse.pNewTable ){
    ಯದಿ( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      ಯದಿ( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ renameColumnFunc_done;
    }ಅನ್ಯಥಾ ಯದಿ( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      ಪೂರ್ಣಾಙ್ಕ bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      ಯದಿ( bFKOnly==0 ){
        ಯದಿ( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (ಶೂನ್ಯ*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        ಯದಿ( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (ಶೂನ್ಯ*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.pNewTable->pCheck);
        ಪ್ರತಿ(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಪ್ರತಿ(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pIdx->a_Col_ಅಭಿವ್ಯಕ್ತಿ);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        ಪ್ರತಿ(i=0; i<sParse.pNewTable->nCol; i++){
          ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
        }
#endif
      }

      ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(sParse.pNewTable) );
      ಪ್ರತಿ(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        ಪ್ರತಿ(i=0; i<pFKey->nCol; i++){
          ಯದಿ( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (ಶೂನ್ಯ*)&pFKey->aCol[i]);
          }
          ಯದಿ( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (ಶೂನ್ಯ*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }ಅನ್ಯಥಾ ಯದಿ( sParse.pNewIndex ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.pNewIndex->a_Col_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }ಅನ್ಯಥಾ{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ renameColumnFunc_done;

    ಪ್ರತಿ(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      ಯದಿ( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        ಯದಿ( pTarget==pTab ){
          ಯದಿ( pStep->pUpsert ){
            ಅಭಿವ್ಯಕ್ತಿ_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    ಯದಿ( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  ಯದಿ( rc!=SQLITE_OK ){
    ಯದಿ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_ಮೌಲ್ಯ(context, argv[0]);
    }ಅನ್ಯಥಾ ಯದಿ( sParse.z_Err_ಸಂದೇಶ ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rename_Table_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  RenameCtx *p = pWalker->u.pRename;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
   && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ))
   && p->pTab==p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renameTableSelectCb(Walker *pWalker, Select *pSelect){
  ಪೂರ್ಣಾಙ್ಕ i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  ಯದಿ( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ಯದಿ( NEVER(pSrc==0) ){
    ಪ್ರತಿಜ್ಞಾ( pWalker->pParse->db->mallocFailed );
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    ಯದಿ( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ renameTableFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಧ್ರುವ ವರ್ಣ *zDb = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಧ್ರುವ ವರ್ಣ *z_ಒಳಹರಿವು = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[3]);
  ಧ್ರುವ ವರ್ಣ *zOld = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[4]);
  ಧ್ರುವ ವರ್ಣ *zNew = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[5]);
  ಪೂರ್ಣಾಙ್ಕ bTemp = sqlite3_ಮೌಲ್ಯ_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  ಯದಿ( z_ಒಳಹರಿವು && zOld && zNew ){
    Parse sParse;
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಪೂರ್ಣಾಙ್ಕ bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, ಪರಿಮಾಣ(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Table_ಅಭಿವ್ಯಕ್ತಿ_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, bTemp);

    ಯದಿ( rc==SQLITE_OK ){
      ಪೂರ್ಣಾಙ್ಕ isLegacy = (db->flags & SQLITE_LegacyAlter);
      ಯದಿ( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        ಯದಿ( IsView(pTab) ){
          ಯದಿ( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
            sNC.pParse = &sParse;

            ಪ್ರತಿಜ್ಞಾ( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            ಯದಿ( sParse.nErr ){
              rc = sParse.rc;
            }ಅನ್ಯಥಾ{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }ಅನ್ಯಥಾ{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_ಕೀಲಿ
          ಯದಿ( (isLegacy==0 || (db->flags & SQLITE_Foreign_ಕೀಲಿಗಳು))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
            ಪ್ರತಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              ಯದಿ( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (ಶೂನ್ಯ*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          ಯದಿ( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            ಯದಿ( isLegacy==0 ){
              sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      ಅನ್ಯಥಾ ಯದಿ( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        ಯದಿ( isLegacy==0 ){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      ಅನ್ಯಥಾ{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        ಯದಿ( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        ಯದಿ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          ಯದಿ( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            ಪ್ರತಿ(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              ಯದಿ( pStep->pSrc ){
                ಪೂರ್ಣಾಙ್ಕ i;
                ಪ್ರತಿ(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  ಯದಿ( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    ಯದಿ( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_ಒಳಹರಿವು, zNew, bQuote);
    }
    ಯದಿ( rc!=SQLITE_OK ){
      ಯದಿ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_ಮೌಲ್ಯ(context, argv[3]);
      }ಅನ್ಯಥಾ ಯದಿ( sParse.z_Err_ಸಂದೇಶ ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }ಅನ್ಯಥಾ{
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

  ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rename_Quotefix_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_STRING && (p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (ಧ್ರುವ ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ renameQuotefixFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ವರ್ಣ ಧ್ರುವ *zDb = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ವರ್ಣ ಧ್ರುವ *z_ಒಳಹರಿವು = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  ಯದಿ( zDb && z_ಒಳಹರಿವು ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, 0);

    ಯದಿ( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, ಪರಿಮಾಣ(RenameCtx));
      memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Quotefix_ಅಭಿವ್ಯಕ್ತಿ_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      ಯದಿ( sParse.pNewTable ){
        ಯದಿ( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          ಯದಿ( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }ಅನ್ಯಥಾ{
          ಪೂರ್ಣಾಙ್ಕ i;
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          ಪ್ರತಿ(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker,
               sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }ಅನ್ಯಥಾ ಯದಿ( sParse.pNewIndex ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.pNewIndex->a_Col_ಅಭಿವ್ಯಕ್ತಿ);
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }ಅನ್ಯಥಾ{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        ಯದಿ( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      ಯದಿ( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_ಒಳಹರಿವು, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    ಯದಿ( rc!=SQLITE_OK ){
      ಯದಿ( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_ಮೌಲ್ಯ(context, argv[1]);
      }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಶೂನ್ಯ renameTableTest(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ವರ್ಣ ಧ್ರುವ *zDb = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ವರ್ಣ ಧ್ರುವ *z_ಒಳಹರಿವು = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  ಪೂರ್ಣಾಙ್ಕ bTemp = sqlite3_ಮೌಲ್ಯ_int(argv[4]);
  ಪೂರ್ಣಾಙ್ಕ isLegacy = (db->flags & SQLITE_LegacyAlter);
  ವರ್ಣ ಧ್ರುವ *zWhen = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[5]);
  ಪೂರ್ಣಾಙ್ಕ bNoDQS = sqlite3_ಮೌಲ್ಯ_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  ಯದಿ( zDb && z_ಒಳಹರಿವು ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    Parse sParse;
    u64 flags = db->flags;
    ಯದಿ( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, bTemp);
    db->flags = flags;
    ಯದಿ( rc==SQLITE_OK ){
      ಯದಿ( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        ಯದಿ( sParse.nErr ) rc = sParse.rc;
      }

      ಅನ್ಯಥಾ ಯದಿ( sParse.pNewTrigger ){
        ಯದಿ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        ಯದಿ( rc==SQLITE_OK ){
          ಪೂರ್ಣಾಙ್ಕ i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          ಪೂರ್ಣಾಙ್ಕ i2 = sqlite3FindDbName(db, zDb);
          ಯದಿ( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    ಯದಿ( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getConstraintToken(ಧ್ರುವ u8 *z, ಪೂರ್ಣಾಙ್ಕ *piToken){
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ t = 0;
  ಕುರು {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }ಯಾವತ್( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  ಯದಿ( t==TK_LP ){
    ಪೂರ್ಣಾಙ್ಕ nNest = 1;
    ಯಾವತ್( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      ಯದಿ( t==TK_LP ){
        nNest++;
      }ಅನ್ಯಥಾ ಯದಿ( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }ಅನ್ಯಥಾ ಯದಿ( t==TK_ILLEGAL ){
        ಭಙ್ಗ;
      }
    }
  }

  *piToken = t;
  ಪ್ರತ್ಯಾಗಮ iOff;
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
ಸ್ಥಿರ ಶೂನ್ಯ dropColumnFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪೂರ್ಣಾಙ್ಕ iSchema = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಧ್ರುವ ವರ್ಣ *zSql = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  ಪೂರ್ಣಾಙ್ಕ iCol = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಧ್ರುವ ವರ್ಣ *zDb = db->aDb[iSchema].zDbSName;
  ಪೂರ್ಣಾಙ್ಕ rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  ಧ್ರುವ ವರ್ಣ *zEnd;
  ವರ್ಣ *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ drop_column_done;
  pTab = sParse.pNewTable;
  ಯದಿ( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    ಗಚ್ಛ drop_column_done;
  }

  ಯದಿ( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (ಶೂನ್ಯ*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (ಶೂನ್ಯ*)pTab->aCol[iCol+1].zCnName);
    zEnd = (ಧ್ರುವ ವರ್ಣ*)pEnd->t.z;
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ eTok;
    ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
    ಪ್ರತಿಜ್ಞಾ( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (ಶೂನ್ಯ*)pTab->aCol[iCol-1].zCnName);
    ಕುರು {
      pCol->t.z += getConstraintToken((ಧ್ರುವ u8*)pCol->t.z, &eTok);
    }ಯಾವತ್( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (ಧ್ರುವ ವರ್ಣ*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  ಯದಿ( rc!=SQLITE_OK ){
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
ಶೂನ್ಯ sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, ಧ್ರುವ Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  ಪೂರ್ಣಾಙ್ಕ iDb;                        /* Index of db containing pTab in aDb[] */
  ಧ್ರುವ ವರ್ಣ *zDb;                /* Database containing pTab ("main" etc.) */
  ವರ್ಣ *zCol = 0;                 /* Name of column to drop */
  ಪೂರ್ಣಾಙ್ಕ iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTable==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಯದಿ( NEVER(db->mallocFailed) ) ಗಚ್ಛ exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ಯದಿ( !pTab ) ಗಚ್ಛ exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  ಯದಿ( SQLITE_OK!=isAlterableTable(pParse, pTab) ) ಗಚ್ಛ exit_drop_column;
  ಯದಿ( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) ಗಚ್ಛ exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  ಯದಿ( zCol==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಗಚ್ಛ exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  ಯದಿ( iCol<0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: \"%T\"", pName);
    ಗಚ್ಛ exit_drop_column;
  }
  ಯದಿ( isRowidAlias(pParse, pTab, zCol, "drop") ) ಗಚ್ಛ exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  ಯದಿ( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    ಗಚ್ಛ exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  ಯದಿ( pTab->nCol<=1 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    ಗಚ್ಛ exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    ಗಚ್ಛ exit_drop_column;
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
  ಯದಿ( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ addr;
    ಪೂರ್ಣಾಙ್ಕ reg;
    ಪೂರ್ಣಾಙ್ಕ regRec;
    Index *pPk = 0;
    ಪೂರ್ಣಾಙ್ಕ nField = 0;               /* Number of non-virtual columns after drop */
    ಪೂರ್ಣಾಙ್ಕ iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    ಯದಿ( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }ಅನ್ಯಥಾ{
      pPk = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
      pParse->nMem += pPk->nColumn;
      ಪ್ರತಿ(i=0; i<pPk->n_ಕೀಲಿ_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_ಕೀಲಿ_Col;
    }
    regRec = ++pParse->nMem;
    ಪ್ರತಿ(i=0; i<pTab->nCol; i++){
      ಯದಿ( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        ಪೂರ್ಣಾಙ್ಕ regOut;
        ಯದಿ( pPk ){
          ಪೂರ್ಣಾಙ್ಕ iPos = sqlite3TableColumnToIndex(pPk, i);
          ಪೂರ್ಣಾಙ್ಕ iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          ಯದಿ( iPos<pPk->n_ಕೀಲಿ_Col ) ಅನುವರ್ತ;
          regOut = reg+1+iPos-(iPos>iColPos);
        }ಅನ್ಯಥಾ{
          regOut = reg+1+nField;
        }
        ಯದಿ( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }ಅನ್ಯಥಾ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          ಪ್ರತಿಜ್ಞಾ( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    ಯದಿ( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    ಯದಿ( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iCur, regRec, reg+1, pPk->n_ಕೀಲಿ_Col);
    }ಅನ್ಯಥಾ{
      sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iCur, regRec, reg);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getWhitespace(ಧ್ರುವ u8 *z){
  ಪೂರ್ಣಾಙ್ಕ nRet = 0;
  ಯಾವತ್( 1 ){
    ಪೂರ್ಣಾಙ್ಕ t = 0;
    ಪೂರ್ಣಾಙ್ಕ n = sqlite3GetToken(&z[nRet], &t);
    ಯದಿ( t!=TK_SPACE && t!=TK_COMMENT ) ಭಙ್ಗ;
    nRet += n;
  }
  ಪ್ರತ್ಯಾಗಮ nRet;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getConstraint(ಧ್ರುವ u8 *z, ಪೂರ್ಣಾಙ್ಕ eTok){
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ t = 0;

#ifndef SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  ಯದಿ( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  ಯದಿ( eTok==TK_REFERENCES ){
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
  ಯಾವತ್( 1 ){
    ಪೂರ್ಣಾಙ್ಕ n = getConstraintToken(&z[iOff], &t);
    ಯದಿ( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      ಭಙ್ಗ;
    }
    iOff += n;
  }
  
  ಪ್ರತ್ಯಾಗಮ iOff;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  ಪೂರ್ಣಾಙ್ಕ t,                 /* Token type */
  ಧ್ರುವ u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  ಪೂರ್ಣಾಙ್ಕ nQuote,            /* Length of zQuote in bytes */
  ಧ್ರುವ u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  ಪೂರ್ಣಾಙ್ಕ *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  ವರ್ಣ *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  ಯದಿ( t==TK_ILLEGAL ){
    *pRes = 1;
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  ಯದಿ( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ಧ್ರುವ ವರ್ಣ*)zCopy, (ಧ್ರುವ ವರ್ಣ*)zCmp);
  sqlite3_free(zCopy);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ skipCreateTable(sqlite3_context *ctx, ಧ್ರುವ u8 *zSql, ಪೂರ್ಣಾಙ್ಕ *piOff){
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;

  ಯದಿ( zSql==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  ಯಾವತ್( 1 ){
    ಪೂರ್ಣಾಙ್ಕ t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    ಯದಿ( t==TK_LP ) ಭಙ್ಗ;
    ಯದಿ( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಶೂನ್ಯ dropConstraintFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ u8 *zSql = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಧ್ರುವ u8 *zCons = 0;
  ಪೂರ್ಣಾಙ್ಕ iNotNull = -1;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ iStart = 0;
  ಪೂರ್ಣಾಙ್ಕ iEnd = 0;
  ವರ್ಣ *zNew = 0;
  ಪೂರ್ಣಾಙ್ಕ t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  ಯದಿ( zSql==0 ) ಪ್ರತ್ಯಾಗಮ;

  /* Jump past the "CREATE TABLE" bit. */
  ಯದಿ( skipCreateTable(ctx, zSql, &iOff) ) ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  }ಅನ್ಯಥಾ{
    zCons = sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  ಪ್ರತಿ(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    ಯಾವತ್( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      ಯದಿ( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        ಪೂರ್ಣಾಙ್ಕ nTok = 0;
        ಪೂರ್ಣಾಙ್ಕ cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        ಯದಿ( zCons ){
          ಯದಿ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ಪ್ರತ್ಯಾಗಮ;
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
        ಯದಿ( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }ಅನ್ಯಥಾ{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        ಯದಿ( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          ಯದಿ( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            ಪ್ರತ್ಯಾಗಮ;
          }
          iEnd = iOff;
          ಭಙ್ಗ;
        }

      }ಅನ್ಯಥಾ ಯದಿ( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ ಯದಿ( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        ಭಙ್ಗ;
      }ಅನ್ಯಥಾ ಯದಿ( t==TK_COMMA ){
        ಭಙ್ಗ;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  ಯದಿ( iEnd<=0 ){
    ಯದಿ( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }ಅನ್ಯಥಾ{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (ಧ್ರುವ ವರ್ಣ*)zSql, -1, SQLITE_TRANSIENT);
    }
  }ಅನ್ಯಥಾ{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ಧ್ರುವ ವರ್ಣ *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    ಯದಿ( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      ಯದಿ( zSql[iStart-1]==',' ) iStart--;
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
ಸ್ಥಿರ ಶೂನ್ಯ addConstraintFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ u8 *zSql = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಧ್ರುವ ವರ್ಣ *zCons = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
  ಪೂರ್ಣಾಙ್ಕ iCol = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ ii;
  sqlite3_str *pNew;
  ಪೂರ್ಣಾಙ್ಕ t = 0;
  UNUSED_PARAMETER(NotUsed);

  ಯದಿ( skipCreateTable(ctx, zSql, &iOff) ) ಪ್ರತ್ಯಾಗಮ;
  
  ಪ್ರತಿ(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ಯಾವತ್( 1 ){
      ಪೂರ್ಣಾಙ್ಕ nTok = getConstraintToken(&zSql[iOff], &t);
      ಯದಿ( t==TK_COMMA || t==TK_RP ) ಭಙ್ಗ;
      ಯದಿ( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        ಪ್ರತ್ಯಾಗಮ;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ಧ್ರುವ ವರ್ಣ*)zSql, iOff);
  ಯದಿ( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ alterFindCol(Parse *pParse, Table *pTab, Token *pCol, ಪೂರ್ಣಾಙ್ಕ *piCol){
  sqlite3 *db = pParse->db;
  ವರ್ಣ *zName = sqlite3NameFromToken(db, pCol);
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOMEM;
  ಪೂರ್ಣಾಙ್ಕ iCol = -1;

  ಯದಿ( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    ಯದಿ( iCol<0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }ಅನ್ಯಥಾ{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  ಯದಿ( rc==SQLITE_OK ){
    ಧ್ರುವ ವರ್ಣ *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    ಧ್ರುವ ವರ್ಣ *zCol = pTab->aCol[iCol].zCnName;
    ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  ಪೂರ್ಣಾಙ್ಕ *piDb,            /* OUT: write the iDb here */
  ಧ್ರುವ ವರ್ಣ **pzDb,    /* OUT: write name of schema here */
  ಪೂರ್ಣಾಙ್ಕ bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  ಯದಿ( pTab ){
    ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    ಯದಿ( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  ಯದಿ( pTab && bAuth ){
    ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  ಪ್ರತ್ಯಾಗಮ pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
ಶೂನ್ಯ sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  ಪೂರ್ಣಾಙ್ಕ iDb = 0;
  ಧ್ರುವ ವರ್ಣ *zDb = 0;
  ವರ್ಣ *zArg = 0;

  ಪ್ರತಿಜ್ಞಾ( (pCol==0)!=(pCons==0) );
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  ಯದಿ( !pTab ) ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ( pCons ){
    ವರ್ಣ *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ iCol;
    ಯದಿ( alterFindCol(pParse, pTab, pCol, &iCol) ) ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಶೂನ್ಯ failConstraintFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ ವರ್ಣ *zText = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  ಪೂರ್ಣಾಙ್ಕ err = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  (ಶೂನ್ಯ)NotUsed;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ಧ್ರುವ ವರ್ಣ *pCons,              /* Buffer containing constraint */
  ಪೂರ್ಣಾಙ್ಕ nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ iEnd = 0;

  ಯದಿ( zTmp==0 ) ಪ್ರತ್ಯಾಗಮ 0;

  ಯಾವತ್( 1 ){
    ಪೂರ್ಣಾಙ್ಕ t = 0;
    ಪೂರ್ಣಾಙ್ಕ nToken = sqlite3GetToken(&zTmp[iOff], &t);
    ಯದಿ( t==TK_ILLEGAL ) ಭಙ್ಗ;
    ಯದಿ( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  ಪ್ರತ್ಯಾಗಮ iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
ಶೂನ್ಯ sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  ಪೂರ್ಣಾಙ್ಕ iCol = 0;
  ಪೂರ್ಣಾಙ್ಕ iDb = 0;
  ಧ್ರುವ ವರ್ಣ *zDb = 0;
  ಧ್ರುವ ವರ್ಣ *pCons = 0;
  ಪೂರ್ಣಾಙ್ಕ nCons = 0;

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  ಯದಿ( !pTab ) ಪ್ರತ್ಯಾಗಮ;

  /* Find the column being altered. */
  ಯದಿ( alterFindCol(pParse, pTab, pCol, &iCol) ){
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (ಪೂರ್ಣಾಙ್ಕ)pCol->n, pCol->z
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
ಸ್ಥಿರ ಶೂನ್ಯ findConstraintFunc(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ NotUsed,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಧ್ರುವ u8 *zSql = 0;
  ಧ್ರುವ u8 *zCons = 0;
  ಪೂರ್ಣಾಙ್ಕ iOff = 0;
  ಪೂರ್ಣಾಙ್ಕ t = 0;

  (ಶೂನ್ಯ)NotUsed;
  zSql = sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  zCons = sqlite3_ಮೌಲ್ಯ_text(argv[1]);

  ಯದಿ( zSql==0 || zCons==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯಾವತ್( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  ಯಾವತ್( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ಯದಿ( t==TK_CONSTRAINT ){
      ಪೂರ್ಣಾಙ್ಕ nTok = 0;
      ಪೂರ್ಣಾಙ್ಕ cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      ಯದಿ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ಪ್ರತ್ಯಾಗಮ;
      ಯದಿ( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        ಪ್ರತ್ಯಾಗಮ;
      }
    }ಅನ್ಯಥಾ ಯದಿ( t==TK_ILLEGAL ){
      ಭಙ್ಗ;
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
ಶೂನ್ಯ sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ಧ್ರುವ ವರ್ಣ *z_ಅಭಿವ್ಯಕ್ತಿ,       /* Text of CHECK expression */
  ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ,               /* Size of pExpr in bytes */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  ಪೂರ್ಣಾಙ್ಕ iDb = 0;             /* Which schema does pTab live in */
  ಧ್ರುವ ವರ್ಣ *zDb = 0;     /* Name of the schema in which pTab lives */
  ಧ್ರುವ ವರ್ಣ *pCons = 0;   /* Text of the constraint */
  ಪೂರ್ಣಾಙ್ಕ nCons;               /* Bytes of text to use from pCons[] */
  ಪೂರ್ಣಾಙ್ಕ rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  ಯದಿ( !pTab ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಪ್ರತ್ಯಾಗಮ;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  ಯದಿ( pName ){
    ವರ್ಣ *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_ಅಭಿವ್ಯಕ್ತಿ, z_ಅಭಿವ್ಯಕ್ತಿ
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
ಶೂನ್ಯ sqlite3AlterFunctions(ಶೂನ್ಯ){
  ಸ್ಥಿರ FuncDef aAlterTableFuncs[] = {
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
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
