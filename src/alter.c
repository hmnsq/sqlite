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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_Alterable_ಕೋಷ್ಟಕ(Parse *pParse, ಕೋಷ್ಟಕ *pTab){
  ಆದರೆ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#ಆದರೆ_ಅಂತ್ಯ
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "table %s may not be altered", pTab->zName);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
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
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,                /* Name of db to verify schema of */
  ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ,                      /* True if this is the temp db */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWhen,              /* "when" part of error message */
  ಪೂರ್ಣಾಂಕ bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->ಲಂಬಸಾಲು_ಹೆಸರುಗಳು_ಗಣ = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_ತಾತ್ಕಾಲಿಕ, zWhen, bNoDQS
  );

  ಆದರೆ( b_ತಾತ್ಕಾಲಿಕ==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " "
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
ಸ್ಥಿರ ಶೂನ್ಯ renameFixQuotes(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb, ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  ಆದರೆ( b_ತಾತ್ಕಾಲಿಕ==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_ಕೋಷ್ಟಕ
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
ಸ್ಥಿರ ಶೂನ್ಯ renameReloadSchema(Parse *pParse, ಪೂರ್ಣಾಂಕ iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  ಆದರೆ( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ಕೂಡಿಸು_Parse_Schema_Op(pParse->pVdbe, iDb, 0, p5);
    ಆದರೆ( iDb!=1 ) sqlite3_Vdbe_ಕೂಡಿಸು_Parse_Schema_Op(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
ಶೂನ್ಯ sqlite3_Alter_Rename_ಕೋಷ್ಟಕ(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  ಪೂರ್ಣಾಂಕ iDb;                  /* Database that contains the table */
  ಅಕ್ಷರ *zDb;                /* Name of database iDb */
  ಕೋಷ್ಟಕ *pTab;              /* Table being renamed */
  ಅಕ್ಷರ *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  ಪೂರ್ಣಾಂಕ nTabName;             /* Number of UTF-8 characters in zTabName */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  ಆದರೆ( NEVER(db->mallocFailed) ) ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, &pSrc->a[0]);
  ಆದರೆ( !pTab ) ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  ಆದರೆ( !zName ) ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  ಆದರೆ( sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, zName, zDb)
   || sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(db, zName, zDb)
   || sqlite3_Is_Shadow_ಕೋಷ್ಟಕ_ದ(db, pTab, zName)
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
        "there is already another table or index with this name: %s", zName);
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  ಆದರೆ( SQLITE_OK!=is_Alterable_ಕೋಷ್ಟಕ(pParse, pTab) ){
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }
  ಆದರೆ( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIEW
  ಆದರೆ( IsView(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "view %s may not be altered", pTab->zName);
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, zDb, pTab->zName, 0) ){
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab) ){
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }
  ಆದರೆ( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    ಆದರೆ( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v==0 ){
    ಹೋಗು exit_rename_ಕೋಷ್ಟಕ;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  ಆದರೆ( sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  ಆದರೆ( iDb!=1 ){
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( pVTab ){
    ಪೂರ್ಣಾಂಕ i = ++pParse->nMem;
    sqlite3_Vdbe_ತುಂಬು_String(v, i, zName);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_VRename, i, 0, 0,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)pVTab, P4_VTAB);
  }
#ಆದರೆ_ಅಂತ್ಯ

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_ಕೋಷ್ಟಕ:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_Error_If_Not_ಖಾಲಿಗಳು(
  Parse *pParse,        /* Parsing context */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,      /* Schema holding the table */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,     /* Table to check for empty */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr      /* Error message text */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isRowidAlias(
  Parse *pParse, 
  ಕೋಷ್ಟಕ *pTab, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp
){
  ಆದರೆ( HasRowid(pTab) && sqlite3IsRowid(z_ಲಂಬಸಾಲು) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot %s rowid alias: %s", zOp, z_ಲಂಬಸಾಲು);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
ಶೂನ್ಯ sqlite3_Alter_ಮುಕ್ತಾಯ_ಕೂಡಿಸು_ಸ್ತಂಭ(Parse *pParse, Token *p_ಲಂಬಸಾಲು_Def){
  ಕೋಷ್ಟಕ *pNew;              /* Copy of pParse->pNewTable */
  ಕೋಷ್ಟಕ *pTab;              /* Table being altered */
  ಪೂರ್ಣಾಂಕ iDb;                  /* Database number */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;          /* Database name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab;         /* Table name */
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;               /* Null-terminated column definition */
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;             /* The new column */
  ಅಭಿವ್ಯಕ್ತಿ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  ಪೂರ್ಣಾಂಕ r1;                   /* Temporary registers */

  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  pNew = pParse->p_New_ಕೋಷ್ಟಕ;
  ಪ್ರತಿಜ್ಞಾ( pNew );

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  p_ಲಂಬಸಾಲು = &pNew->a_ಲಂಬಸಾಲು[pNew->n_ಲಂಬಸಾಲು-1];
  pDflt = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pNew, p_ಲಂಬಸಾಲು);
  pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, zTab, zDb);
  ಪ್ರತಿಜ್ಞಾ( pTab );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, zDb, pTab->zName, 0) ){
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "Cannot add a PRIMARY KEY column");
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( pNew->p_ಸೂಚ್ಯಂಕ ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
         "Cannot add a UNIQUE column");
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( isRowidAlias(pParse, pTab, p_ಲಂಬಸಾಲು->zCnName, "add") ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    ಪ್ರತಿಜ್ಞಾ( pDflt==0 || pDflt->op==TK_ಹರವು );
    ಆದರೆ( pDflt && pDflt->p_ಎಡ->op==TK_NULL ){
      pDflt = 0;
    }
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pNew) );
    ಆದರೆ( (db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು) && pNew->u.tab.pFKey && pDflt ){
      sqlite3_Error_If_Not_ಖಾಲಿಗಳು(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    ಆದರೆ( p_ಲಂಬಸಾಲು->notNull && !pDflt ){
      sqlite3_Error_If_Not_ಖಾಲಿಗಳು(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    ಆದರೆ( pDflt ){
      sqlite3_ಮೌಲ್ಯ *pVal = 0;
      ಪೂರ್ಣಾಂಕ rc;
      rc = sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      ಆದರೆ( rc!=SQLITE_OK ){
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed == 1 );
        ಹಿಂತಿರುಗಿ;
      }
      ಆದರೆ( !pVal ){
        sqlite3_Error_If_Not_ಖಾಲಿಗಳು(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_ಮೌಲ್ಯ_Free(pVal);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_STORED ){
    sqlite3_Error_If_Not_ಖಾಲಿಗಳು(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  z_ಲಂಬಸಾಲು = sqlite3DbStrNDup(db, (ಅಕ್ಷರ*)p_ಲಂಬಸಾಲು_Def->z, p_ಲಂಬಸಾಲು_Def->n);
  ಆದರೆ( z_ಲಂಬಸಾಲು ){
    ಅಕ್ಷರ *z_ಕೊನೆ = &z_ಲಂಬಸಾಲು[p_ಲಂಬಸಾಲು_Def->n-1];
    ಆಗ( z_ಕೊನೆ>z_ಲಂಬಸಾಲು && (*z_ಕೊನೆ==';' || sqlite3Isspace(*z_ಕೊನೆ)) ){
      *z_ಕೊನೆ-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ, z_ಲಂಬಸಾಲು, pNew->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ,
      zTab
    );
    sqlite3DbFree(db, z_ಲಂಬಸಾಲು);
  }

  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, r1, -2);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಗಣ_Cookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_Alter_ಕೂಡಿಸು);

    /* Verify that constraints are still satisfied */
    ಆದರೆ( pNew->pCheck!=0
     || (p_ಲಂಬಸಾಲು->notNull && (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->ಲಂಬಸಾಲು_ಹೆಸರುಗಳು_ಗಣ = 1;
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
ಶೂನ್ಯ sqlite3_Alter_Begin_ಕೂಡಿಸು_ಸ್ತಂಭ(Parse *pParse, SrcList *pSrc){
  ಕೋಷ್ಟಕ *pNew;
  ಕೋಷ್ಟಕ *pTab;
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pParse->p_New_ಕೋಷ್ಟಕ==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಆದರೆ( NEVER(db->mallocFailed) ) ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, &pSrc->a[0]);
  ಆದರೆ( !pTab ) ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "virtual tables may not be altered");
    ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Make sure this is not an attempt to ALTER a view. */
  ಆದರೆ( IsView(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "Cannot add a column to a view");
    ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  }
  ಆದರೆ( SQLITE_OK!=is_Alterable_ಕೋಷ್ಟಕ(pParse, pTab) ){
    ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  }

  sqlite3MayAbort(pParse);
  ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
  ಪ್ರತಿಜ್ಞಾ( pTab->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ>0 );
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (ಕೋಷ್ಟಕ*)sqlite3DbMallocZero(db, ಗಾತ್ರ(ಕೋಷ್ಟಕ));
  ಆದರೆ( !pNew ) ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  pParse->p_New_ಕೋಷ್ಟಕ = pNew;
  pNew->nTabRef = 1;
  pNew->n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
  ಪ್ರತಿಜ್ಞಾ( pNew->n_ಲಂಬಸಾಲು>0 );
  nAlloc = (((pNew->n_ಲಂಬಸಾಲು-1)/8)*8)+8;
  ಪ್ರತಿಜ್ಞಾ( nAlloc>=pNew->n_ಲಂಬಸಾಲು && nAlloc%8==0 && nAlloc-pNew->n_ಲಂಬಸಾಲು<8 );
  pNew->a_ಲಂಬಸಾಲು = (ಸ್ತಂಭ*)sqlite3DbMallocZero(db, ಗಾತ್ರ(ಸ್ತಂಭ)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  ಆದರೆ( !pNew->a_ಲಂಬಸಾಲು || !pNew->zName ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಹೋಗು exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ;
  }
  ಸ್ಮರಣೆ_ನಕಲು(pNew->a_ಲಂಬಸಾಲು, pTab->a_ಲಂಬಸಾಲು, ಗಾತ್ರ(ಸ್ತಂಭ)*(size_t)pNew->n_ಲಂಬಸಾಲು);
  ಗಾಗಿ(i=0; i<pNew->n_ಲಂಬಸಾಲು; i++){
    ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = &pNew->a_ಲಂಬಸಾಲು[i];
    p_ಲಂಬಸಾಲು->zCnName = sqlite3DbStrDup(db, p_ಲಂಬಸಾಲು->zCnName);
    p_ಲಂಬಸಾಲು->hName = sqlite3StrIHash(p_ಲಂಬಸಾಲು->zCnName);
  }
  ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pNew) );
  pNew->u.tab.pDfltList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ = pTab->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ;
  ಪ್ರತಿಜ್ಞಾ( pNew->nTabRef==1 );

exit_begin_ಕೂಡಿಸು_ಸ್ತಂಭ:
  sqlite3SrcListDelete(db, pSrc);
  ಹಿಂತಿರುಗಿ;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_Real_ಕೋಷ್ಟಕ(Parse *pParse, ಕೋಷ್ಟಕ *pTab, ಪೂರ್ಣಾಂಕ iOp){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIEW
  ಆದರೆ( IsView(pTab) ){
    zType = "view";
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( zType ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *az_ಸಂದೇಶ[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    ಪ್ರತಿಜ್ಞಾ( iOp>=0 && iOp<Array_ಗಾತ್ರ(az_ಸಂದೇಶ) );
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot %s %s \"%s\"",
        az_ಸಂದೇಶ[iOp], zType, pTab->zName
    );
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define is_Real_ಕೋಷ್ಟಕ(x,y,z) (0)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
ಶೂನ್ಯ sqlite3_Alter_Rename_ಸ್ತಂಭ(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  ಕೋಷ್ಟಕ *pTab;                    /* Table being updated */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                       /* Index of column being renamed */
  ಅಕ್ಷರ *zOld = 0;                 /* Old column name */
  ಅಕ್ಷರ *zNew = 0;                 /* New column name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;                /* Name of schema containing the table */
  ಪೂರ್ಣಾಂಕ iSchema;                    /* Index of the schema */
  ಪೂರ್ಣಾಂಕ bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, &pSrc->a[0]);
  ಆದರೆ( !pTab ) ಹೋಗು exit_rename_ಸ್ತಂಭ;

  /* Cannot alter a system table */
  ಆದರೆ( SQLITE_OK!=is_Alterable_ಕೋಷ್ಟಕ(pParse, pTab) ) ಹೋಗು exit_rename_ಸ್ತಂಭ;
  ಆದರೆ( SQLITE_OK!=is_Real_ಕೋಷ್ಟಕ(pParse, pTab, 0) ) ಹೋಗು exit_rename_ಸ್ತಂಭ;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, zDb, pTab->zName, 0) ){
    ಹೋಗು exit_rename_ಸ್ತಂಭ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  ಆದರೆ( !zOld ) ಹೋಗು exit_rename_ಸ್ತಂಭ;
  i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, zOld);
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: \"%T\"", pOld);
    ಹೋಗು exit_rename_ಸ್ತಂಭ;
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
  ಆದರೆ( !zNew ) ಹೋಗು exit_rename_ಸ್ತಂಭ;
  ಆದರೆ( isRowidAlias(pParse, pTab, zOld, "rename from") ) ಹೋಗು exit_rename_ಸ್ತಂಭ;
  ಆದರೆ( isRowidAlias(pParse, pTab, zNew, "rename to") ) ಹೋಗು exit_rename_ಸ್ತಂಭ;
  ಪ್ರತಿಜ್ಞಾ( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, i_ಲಂಬಸಾಲು, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  ಆದರೆ( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, i_ಲಂಬಸಾಲು, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_ಸ್ತಂಭ:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  ಹಿಂತಿರುಗಿ;
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
ರಚನೆ RenameToken {
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_ಮುಂದಿನ;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ RenameCtx RenameCtx;
ರಚನೆ RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  ಪೂರ್ಣಾಂಕ nList;                      /* Number of tokens in pList */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                       /* Index of column being renamed */
  ಕೋಷ್ಟಕ *pTab;                    /* Table being ALTERed */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOld;               /* Old column name */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಸ್ಥಿರ ಶೂನ್ಯ renameTokenCheckAll(Parse *pParse, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pPtr){
  ಪ್ರತಿಜ್ಞಾ( pParse==pParse->db->pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಆದರೆ( pParse->nErr==0 ){
    ಸ್ಥಿರಾಂಕ RenameToken *p;
    u32 i = 1;
    ಗಾಗಿ(p=pParse->pRename; p; p=p->p_ಮುಂದಿನ){
      ಆದರೆ( p->p ){
        ಪ್ರತಿಜ್ಞಾ( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( i>0 );
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define renameTokenCheckAll(x,y)
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3RenameTokenMap(
  Parse *pParse,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pPtr,
  ಸ್ಥಿರಾಂಕ Token *pToken
){
  RenameToken *pNew;
  ಪ್ರತಿಜ್ಞಾ( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  ಆದರೆ( ALWAYS(pParse->e_Parse_ಬಹುಲಕ!=PARSE_ಬಹುಲಕ_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, ಗಾತ್ರ(RenameToken));
    ಆದರೆ( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_ಮುಂದಿನ = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  ಹಿಂತಿರುಗಿ pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
ಶೂನ್ಯ sqlite3RenameTokenRemap(Parse *pParse, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಗಮ್ಯ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, p_ಗಮ್ಯ);
  ಗಾಗಿ(p=pParse->pRename; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( p->p==pFrom ){
      p->p = p_ಗಮ್ಯ;
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3RenameTokenRemap(pParse, 0, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  ಆದರೆ( pWith ){
    Parse *pParse = pWalker->pParse;
    ಪೂರ್ಣಾಂಕ i;
    With *pCopy = 0;
    ಪ್ರತಿಜ್ಞಾ( pWith->nCte>0 );
    ಆದರೆ( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ತಳ್ಳು(pParse, pCopy, 1);
    }
    ಗಾಗಿ(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, ಗಾತ್ರ(sNC));
      sNC.pParse = pParse;
      ಆದರೆ( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      ಆದರೆ( sNC.pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
    }
    ಆದರೆ( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unmap_ಸ್ತಂಭ_Idlist_ಹೆಸರುಗಳು(
  Parse *pParse,
  ಸ್ಥಿರಾಂಕ IdList *pIdList
){
  ಪೂರ್ಣಾಂಕ ii;
  ಪ್ರತಿಜ್ಞಾ( pIdList!=0 );
  ಗಾಗಿ(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  ಆದರೆ( p->selFlags & (SF_View|SF_CopyCte) ){
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಆದರೆ( ALWAYS(p->pEList) ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = p->pEList;
    ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಆದರೆ( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pList->a[i].zEName);
      }
    }
  }
  ಆದರೆ( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pSrc->a[i].zName);
      ಆದರೆ( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pSrc->a[i].u3.pOn);
      }ಇಲ್ಲದಿದ್ದರೆ{
        unmap_ಸ್ತಂಭ_Idlist_ಹೆಸರುಗಳು(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
ಶೂನ್ಯ sqlite3_Rename_ಅಭಿವ್ಯಕ್ತಿ_Unmap(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u8 e_ಬಹುಲಕ = pParse->e_Parse_ಬಹುಲಕ;
  Walker sWalker;
  memset(&sWalker, 0, ಗಾತ್ರ(Walker));
  sWalker.pParse = pParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->e_Parse_ಬಹುಲಕ = PARSE_ಬಹುಲಕ_UNMAP;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
  pParse->e_Parse_ಬಹುಲಕ = e_ಬಹುಲಕ;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
ಶೂನ್ಯ sqlite3RenameExprlistUnmap(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಆದರೆ( pEList ){
    ಪೂರ್ಣಾಂಕ i;
    Walker sWalker;
    memset(&sWalker, 0, ಗಾತ್ರ(Walker));
    sWalker.pParse = pParse;
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Unmap_ಅಭಿವ್ಯಕ್ತಿ_Cb;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pEList);
    ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಆದರೆ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_ಮುಂದಿನ;
  RenameToken *p;
  ಗಾಗಿ(p=pToken; p; p=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = p->p_ಮುಂದಿನ;
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
ಸ್ಥಿರ RenameToken *rename_Token_ಕಂಡುಹಿಡಿ(
  Parse *pParse,
  ರಚನೆ RenameCtx *pCtx,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pPtr
){
  RenameToken **pp;
  ಆದರೆ( NEVER(pPtr==0) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_ಮುಂದಿನ){
    ಆದರೆ( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      ಆದರೆ( pCtx ){
        *pp = pToken->p_ಮುಂದಿನ;
        pToken->p_ಮುಂದಿನ = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      ಹಿಂತಿರುಗಿ pToken;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_ಸ್ತಂಭ_Select_Cb(Walker *pWalker, Select *p){
  ಆದರೆ( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  RenameCtx *p = pWalker->u.pRename;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER
   && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ==p->i_ಲಂಬಸಾಲು
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    rename_Token_ಕಂಡುಹಿಡಿ(pWalker->pParse, p, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ==p->i_ಲಂಬಸಾಲು
   && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ))
   && p->pTab==p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab
  ){
    rename_Token_ಕಂಡುಹಿಡಿ(pWalker->pParse, p, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಸ್ಥಿರ RenameToken *rename_ಸ್ತಂಭ_Token_ಮುಂದಿನ(RenameCtx *pCtx){
  RenameToken *p_ಉತ್ತಮ = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  ಗಾಗಿ(pToken=p_ಉತ್ತಮ->p_ಮುಂದಿನ; pToken; pToken=pToken->p_ಮುಂದಿನ){
    ಆದರೆ( pToken->t.z>p_ಉತ್ತಮ->t.z ) p_ಉತ್ತಮ = pToken;
  }
  ಗಾಗಿ(pp=&pCtx->pList; *pp!=p_ಉತ್ತಮ; pp=&(*pp)->p_ಮುಂದಿನ);
  *pp = p_ಉತ್ತಮ->p_ಮುಂದಿನ;

  ಹಿಂತಿರುಗಿ p_ಉತ್ತಮ;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ಸ್ಥಿರ ಶೂನ್ಯ errorMPrintf(sqlite3_context *pCtx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ಅಕ್ಷರ *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  ಆದರೆ( zErr ){
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಸ್ತಂಭ_Parse_Error(
  sqlite3_context *pCtx,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWhen,
  sqlite3_ಮೌಲ್ಯ *pType,
  sqlite3_ಮೌಲ್ಯ *pObject,
  Parse *pParse
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zT = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pType);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zN = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pObject);
  ಅಕ್ಷರ *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_ಸಂದೇಶ
  );
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಸ್ತಂಭ_Elist_ಹೆಸರುಗಳು(
  Parse *pParse,
  RenameCtx *pCtx,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pEList,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOld
){
  ಆದರೆ( pEList ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = pEList->a[i].zEName;
      ಆದರೆ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        rename_Token_ಕಂಡುಹಿಡಿ(pParse, pCtx, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಸ್ತಂಭ_Idlist_ಹೆಸರುಗಳು(
  Parse *pParse,
  RenameCtx *pCtx,
  ಸ್ಥಿರಾಂಕ IdList *pIdList,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOld
){
  ಆದರೆ( pIdList ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pIdList->nId; i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = pIdList->a[i].zName;
      ಆದರೆ( 0==sqlite3_stricmp(zName, zOld) ){
        rename_Token_ಕಂಡುಹಿಡಿ(pParse, pCtx, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,               /* SQL to parse */
  ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ                       /* True if SQL is from temp schema */
){
  ಪೂರ್ಣಾಂಕ rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  ಆದರೆ( zSql==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  ಆದರೆ( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಆದರೆ( b_ತಾತ್ಕಾಲಿಕ ){
    db->init.iDb = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
    ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->e_Parse_ಬಹುಲಕ = PARSE_ಬಹುಲಕ_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3_ಸತತ_Parser(p, zSql);
  db->flags = flags;
  ಆದರೆ( db->mallocFailed ) rc = SQLITE_NOMEM;
  ಆದರೆ( rc==SQLITE_OK
   && NEVER(p->p_New_ಕೋಷ್ಟಕ==0 && p->p_New_ಸೂಚ್ಯಂಕ==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    ಗಾಗಿ(pToken=p->pRename; pToken; pToken=pToken->p_ಮುಂದಿನ){
      ಪ್ರತಿಜ್ಞಾ( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  db->init.iDb = 0;
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,               /* SQL statement to edit */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNew,               /* New token text */
  ಪೂರ್ಣಾಂಕ bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಅಕ್ಷರ *zQuot = 0;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  i64 nQuot = 0;
  ಅಕ್ಷರ *zBuf1 = 0;
  ಅಕ್ಷರ *zBuf2 = 0;

  ಆದರೆ( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    ಆದರೆ( zQuot==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }ಇಲ್ಲದಿದ್ದರೆ{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    ಪ್ರತಿಜ್ಞಾ( nQuot>=nNew && nSql>=0 && nNew>=0 );
    z_ಹೊರಹೋಗು = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( nSql>0 );
    z_ಹೊರಹೋಗು = (ಅಕ್ಷರ*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    ಆದರೆ( z_ಹೊರಹೋಗು ){
      zBuf1 = &z_ಹೊರಹೋಗು[nSql*2+1];
      zBuf2 = &z_ಹೊರಹೋಗು[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  ಆದರೆ( z_ಹೊರಹೋಗು ){
    i64 n_ಹೊರಹೋಗು = nSql;
    ಪ್ರತಿಜ್ಞಾ( nSql>0 );
    ಸ್ಮರಣೆ_ನಕಲು(z_ಹೊರಹೋಗು, zSql, (size_t)nSql);
    ಆಗ( pRename->pList ){
      ಪೂರ್ಣಾಂಕ iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zReplace;
      RenameToken *p_ಉತ್ತಮ = rename_ಸ್ತಂಭ_Token_ಮುಂದಿನ(pRename);

      ಆದರೆ( zNew ){
        ಆದರೆ( bQuote==0 && sqlite3IsIdChar(*(u8*)p_ಉತ್ತಮ->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }ಇಲ್ಲದಿದ್ದರೆ{
          nReplace = nQuot;
          zReplace = zQuot;
          ಆದರೆ( p_ಉತ್ತಮ->t.z[p_ಉತ್ತಮ->t.n]=='"' ) nReplace++;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        ಸ್ಮರಣೆ_ನಕಲು(zBuf1, p_ಉತ್ತಮ->t.z, p_ಉತ್ತಮ->t.n);
        zBuf1[p_ಉತ್ತಮ->t.n] = 0;
        sqlite3Dequote(zBuf1);
        ಪ್ರತಿಜ್ಞಾ( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((ಪೂರ್ಣಾಂಕ)(nSql*2), zBuf2, "%Q%s", zBuf1,
            p_ಉತ್ತಮ->t.z[p_ಉತ್ತಮ->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (ಪೂರ್ಣಾಂಕ)(p_ಉತ್ತಮ->t.z - zSql);
      ಆದರೆ( p_ಉತ್ತಮ->t.n!=nReplace ){
        memmove(&z_ಹೊರಹೋಗು[iOff + nReplace], &z_ಹೊರಹೋಗು[iOff + p_ಉತ್ತಮ->t.n],
            n_ಹೊರಹೋಗು - (iOff + p_ಉತ್ತಮ->t.n)
        );
        n_ಹೊರಹೋಗು += nReplace - p_ಉತ್ತಮ->t.n;
        z_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು] = '\0';
      }
      ಸ್ಮರಣೆ_ನಕಲು(&z_ಹೊರಹೋಗು[iOff], zReplace, nReplace);
      sqlite3DbFree(db, p_ಉತ್ತಮ);
    }

    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, z_ಹೊರಹೋಗು, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, z_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಗಣ_ENames(ಅಭಿವ್ಯಕ್ತಿ_List *pEList, ಪೂರ್ಣಾಂಕ val){
  ಪ್ರತಿಜ್ಞಾ( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_ಹರವು );
  ಆದರೆ( pEList ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;
  NameContext sNC;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  sNC.pParse = pParse;
  ಪ್ರತಿಜ್ಞಾ( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, pNew->ಕೋಷ್ಟಕ_2,
      db->aDb[sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  ಆದರೆ( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  ಆದರೆ( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pNew->pWhen);
  }

  ಗಾಗಿ(p_ಹೆಜ್ಜೆ=pNew->ಹೆಜ್ಜೆ_list; rc==SQLITE_OK && p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
    ಆದರೆ( p_ಹೆಜ್ಜೆ->pSelect ){
      sqlite3SelectPrep(pParse, p_ಹೆಜ್ಜೆ->pSelect, &sNC);
      ಆದರೆ( pParse->nErr ) rc = pParse->rc;
    }
    ಆದರೆ( rc==SQLITE_OK && p_ಹೆಜ್ಜೆ->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, p_ಹೆಜ್ಜೆ->pSrc, 0);
      ಆದರೆ( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        ಆದರೆ( pSel==0 ){
          p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          rename_ಗಣ_ENames(p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List, ENAME_ಹರವು);
          sqlite3SelectPrep(pParse, pSel, 0);
          rename_ಗಣ_ENames(p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          ಪ್ರತಿಜ್ಞಾ( p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List==0 || p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List==pSel->pEList );
          ಪ್ರತಿಜ್ಞಾ( pSrc==pSel->pSrc );
          ಆದರೆ( p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        ಆದರೆ( ALWAYS(p_ಹೆಜ್ಜೆ->pSrc) ){
          ಪೂರ್ಣಾಂಕ i;
          ಗಾಗಿ(i=0; i<p_ಹೆಜ್ಜೆ->pSrc->nSrc && rc==SQLITE_OK; i++){
            Src_ಅಂಶ *p = &p_ಹೆಜ್ಜೆ->pSrc->a[i];
            ಆದರೆ( p->fg.isSubquery ){
              ಪ್ರತಿಜ್ಞಾ( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        ಆದರೆ(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        ಆದರೆ( rc==SQLITE_OK && p_ಹೆಜ್ಜೆ->pWhere ){
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, p_ಹೆಜ್ಜೆ->pWhere);
        }
        ಆದರೆ( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List);
        }
        ಪ್ರತಿಜ್ಞಾ( !p_ಹೆಜ್ಜೆ->pUpsert || (!p_ಹೆಜ್ಜೆ->pWhere && !p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List) );
        ಆದರೆ( p_ಹೆಜ್ಜೆ->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = p_ಹೆಜ್ಜೆ->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pUpsert->p_Upsert_ಗುರಿ);
          ಆದರೆ( rc==SQLITE_OK ){
            ಅಭಿವ್ಯಕ್ತಿ_List *p_Upsert_ಗಣ = pUpsert->p_Upsert_ಗಣ;
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, p_Upsert_ಗಣ);
          }
          ಆದರೆ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pUpsert->pUpsertWhere);
          }
          ಆದರೆ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pUpsert->p_Upsert_ಗುರಿ_Where);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = SQLITE_NOMEM;
      }
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  ಗಾಗಿ(p_ಹೆಜ್ಜೆ=pTrigger->ಹೆಜ್ಜೆ_list; p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
    sqlite3WalkSelect(pWalker, p_ಹೆಜ್ಜೆ->pSelect);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಹೆಜ್ಜೆ->pWhere);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List);
    ಆದರೆ( p_ಹೆಜ್ಜೆ->pUpsert ){
      Upsert *pUpsert = p_ಹೆಜ್ಜೆ->pUpsert;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pUpsert->p_Upsert_ಗುರಿ);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pUpsert->p_Upsert_ಗಣ);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pUpsert->p_Upsert_ಗುರಿ_Where);
    }
    ಆದರೆ( p_ಹೆಜ್ಜೆ->pSrc ){
      ಪೂರ್ಣಾಂಕ i;
      SrcList *pSrc = p_ಹೆಜ್ಜೆ->pSrc;
      ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
        ಆದರೆ( pSrc->a[i].fg.isSubquery ){
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
  ಸೂಚ್ಯಂಕ *pIdx;
  ಆದರೆ( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3_Delete_ಕೋಷ್ಟಕ(db, pParse->p_New_ಕೋಷ್ಟಕ);
  ಆಗ( (pIdx = pParse->p_New_ಸೂಚ್ಯಂಕ)!=0 ){
    pParse->p_New_ಸೂಚ್ಯಂಕ = pIdx->p_ಮುಂದಿನ;
    sqlite3_Free_ಸೂಚ್ಯಂಕ(db, pIdx);
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
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಸ್ತಂಭ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[3]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[4]);
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = sqlite3_ಮೌಲ್ಯ_int(argv[5]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNew = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[6]);
  ಪೂರ್ಣಾಂಕ bQuote = sqlite3_ಮೌಲ್ಯ_int(argv[7]);
  ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ = sqlite3_ಮೌಲ್ಯ_int(argv[8]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOld;
  ಪೂರ್ಣಾಂಕ rc;
  Parse sParse;
  Walker sWalker;
  ಸೂಚ್ಯಂಕ *pIdx;
  ಪೂರ್ಣಾಂಕ i;
  ಕೋಷ್ಟಕ *pTab;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#ಆದರೆ_ಅಂತ್ಯ

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( zSql==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( z_ಕೋಷ್ಟಕ==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( zNew==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) ಹಿಂತಿರುಗಿ;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, z_ಕೋಷ್ಟಕ, zDb);
  ಆದರೆ( pTab==0 || i_ಲಂಬಸಾಲು>=pTab->n_ಲಂಬಸಾಲು ){
    sqlite3BtreeLeaveAll(db);
    ಹಿಂತಿರುಗಿ;
  }
  zOld = pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
  memset(&sCtx, 0, ಗಾತ್ರ(sCtx));
  sCtx.i_ಲಂಬಸಾಲು = ((i_ಲಂಬಸಾಲು==pTab->iPKey) ? -1 : i_ಲಂಬಸಾಲು);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ
  rc = renameParseSql(&sParse, zDb, db, zSql, b_ತಾತ್ಕಾಲಿಕ);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, ಗಾತ್ರ(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.xSelectCallback = rename_ಸ್ತಂಭ_Select_Cb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು rename_ಸ್ತಂಭ_Func_ಮುಗಿದ;
  ಆದರೆ( sParse.p_New_ಕೋಷ್ಟಕ ){
    ಆದರೆ( IsView(sParse.p_New_ಕೋಷ್ಟಕ) ){
      Select *pSelect = sParse.p_New_ಕೋಷ್ಟಕ->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      ಆದರೆ( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು rename_ಸ್ತಂಭ_Func_ಮುಗಿದ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( Is_Ordinary_ಕೋಷ್ಟಕ(sParse.p_New_ಕೋಷ್ಟಕ) ){
      /* A regular table */
      ಪೂರ್ಣಾಂಕ bFKOnly = sqlite3_stricmp(z_ಕೋಷ್ಟಕ, sParse.p_New_ಕೋಷ್ಟಕ->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.p_New_ಕೋಷ್ಟಕ;
      ಆದರೆ( bFKOnly==0 ){
        ಆದರೆ( i_ಲಂಬಸಾಲು<sParse.p_New_ಕೋಷ್ಟಕ->n_ಲಂಬಸಾಲು ){
          rename_Token_ಕಂಡುಹಿಡಿ(
              &sParse, &sCtx, (ಶೂನ್ಯ*)sParse.p_New_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName
          );
        }
        ಆದರೆ( sCtx.i_ಲಂಬಸಾಲು<0 ){
          rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, (ಶೂನ್ಯ*)&sParse.p_New_ಕೋಷ್ಟಕ->iPKey);
        }
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.p_New_ಕೋಷ್ಟಕ->pCheck);
        ಗಾಗಿ(pIdx=sParse.p_New_ಕೋಷ್ಟಕ->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಗಾಗಿ(pIdx=sParse.p_New_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ);
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
        ಗಾಗಿ(i=0; i<sParse.p_New_ಕೋಷ್ಟಕ->n_ಲಂಬಸಾಲು; i++){
          ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(sParse.p_New_ಕೋಷ್ಟಕ,
                                                  &sParse.p_New_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i]);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }

      ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(sParse.p_New_ಕೋಷ್ಟಕ) );
      ಗಾಗಿ(pFKey=sParse.p_New_ಕೋಷ್ಟಕ->u.tab.pFKey; pFKey; pFKey=pFKey->p_ಮುಂದಿನ_From){
        ಗಾಗಿ(i=0; i<pFKey->n_ಲಂಬಸಾಲು; i++){
          ಆದರೆ( bFKOnly==0 && pFKey->a_ಲಂಬಸಾಲು[i].iFrom==i_ಲಂಬಸಾಲು ){
            rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, (ಶೂನ್ಯ*)&pFKey->a_ಲಂಬಸಾಲು[i]);
          }
          ಆದರೆ( 0==sqlite3_stricmp(pFKey->z_ಗಮ್ಯ, z_ಕೋಷ್ಟಕ)
           && 0==sqlite3_stricmp(pFKey->a_ಲಂಬಸಾಲು[i].z_ಲಂಬಸಾಲು, zOld)
          ){
            rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, (ಶೂನ್ಯ*)pFKey->a_ಲಂಬಸಾಲು[i].z_ಲಂಬಸಾಲು);
          }
        }
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.p_New_ಸೂಚ್ಯಂಕ ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.p_New_ಸೂಚ್ಯಂಕ->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.p_New_ಸೂಚ್ಯಂಕ->pPartIdxWhere);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* A trigger */
    Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;
    rc = renameResolveTrigger(&sParse);
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು rename_ಸ್ತಂಭ_Func_ಮುಗಿದ;

    ಗಾಗಿ(p_ಹೆಜ್ಜೆ=sParse.pNewTrigger->ಹೆಜ್ಜೆ_list; p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
      ಆದರೆ( p_ಹೆಜ್ಜೆ->pSrc ){
        ಕೋಷ್ಟಕ *p_ಗುರಿ = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(&sParse, 0, &p_ಹೆಜ್ಜೆ->pSrc->a[0]);
        ಆದರೆ( p_ಗುರಿ==pTab ){
          ಆದರೆ( p_ಹೆಜ್ಜೆ->pUpsert ){
            ಅಭಿವ್ಯಕ್ತಿ_List *p_Upsert_ಗಣ = p_ಹೆಜ್ಜೆ->pUpsert->p_Upsert_ಗಣ;
            rename_ಸ್ತಂಭ_Elist_ಹೆಸರುಗಳು(&sParse, &sCtx, p_Upsert_ಗಣ, zOld);
          }
          rename_ಸ್ತಂಭ_Idlist_ಹೆಸರುಗಳು(&sParse, &sCtx, p_ಹೆಜ್ಜೆ->pIdList, zOld);
          rename_ಸ್ತಂಭ_Elist_ಹೆಸರುಗಳು(&sParse, &sCtx, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    ಆದರೆ( sParse.pTriggerTab==pTab ){
      rename_ಸ್ತಂಭ_Idlist_ಹೆಸರುಗಳು(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

rename_ಸ್ತಂಭ_Func_ಮುಗಿದ:
  ಆದರೆ( rc!=SQLITE_OK ){
    ಆದರೆ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[0]);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.z_Err_ಸಂದೇಶ ){
      rename_ಸ್ತಂಭ_Parse_Error(context, "", argv[1], argv[2], &sParse);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_ಕೋಷ್ಟಕ_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  RenameCtx *p = pWalker->u.pRename;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ))
   && p->pTab==p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab
  ){
    rename_Token_ಕಂಡುಹಿಡಿ(pWalker->pParse, p, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_ಕೋಷ್ಟಕ_Select_Cb(Walker *pWalker, Select *pSelect){
  ಪೂರ್ಣಾಂಕ i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  ಆದರೆ( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಆದರೆ( NEVER(pSrc==0) ){
    ಪ್ರತಿಜ್ಞಾ( pWalker->pParse->db->mallocFailed );
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
    Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[i];
    ಆದರೆ( p_ಅಂಶ->pSTab==p->pTab ){
      rename_Token_ಕಂಡುಹಿಡಿ(pWalker->pParse, p, p_ಅಂಶ->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಕೋಷ್ಟಕ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳಹರಿವು = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[3]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOld = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[4]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNew = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[5]);
  ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ = sqlite3_ಮೌಲ್ಯ_int(argv[6]);
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  ಆದರೆ( z_ಒಳಹರಿವು && zOld && zNew ){
    Parse sParse;
    ಪೂರ್ಣಾಂಕ rc;
    ಪೂರ್ಣಾಂಕ bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, ಗಾತ್ರ(RenameCtx));
    sCtx.pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, zOld, zDb);
    memset(&sWalker, 0, ಗಾತ್ರ(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_ಕೋಷ್ಟಕ_ಅಭಿವ್ಯಕ್ತಿ_Cb;
    sWalker.xSelectCallback = rename_ಕೋಷ್ಟಕ_Select_Cb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, b_ತಾತ್ಕಾಲಿಕ);

    ಆದರೆ( rc==SQLITE_OK ){
      ಪೂರ್ಣಾಂಕ isLegacy = (db->flags & SQLITE_LegacyAlter);
      ಆದರೆ( sParse.p_New_ಕೋಷ್ಟಕ ){
        ಕೋಷ್ಟಕ *pTab = sParse.p_New_ಕೋಷ್ಟಕ;

        ಆದರೆ( IsView(pTab) ){
          ಆದರೆ( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, ಗಾತ್ರ(sNC));
            sNC.pParse = &sParse;

            ಪ್ರತಿಜ್ಞಾ( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            ಆದರೆ( sParse.nErr ){
              rc = sParse.rc;
            }ಇಲ್ಲದಿದ್ದರೆ{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Modify any FK definitions to point to the new table. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
          ಆದರೆ( (isLegacy==0 || (db->flags & SQLITE_Foreign_ಕೀಲಿಗಳು))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
            ಗಾಗಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ಮುಂದಿನ_From){
              ಆದರೆ( sqlite3_stricmp(pFKey->z_ಗಮ್ಯ, zOld)==0 ){
                rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, (ಶೂನ್ಯ*)pFKey->z_ಗಮ್ಯ);
              }
            }
          }
#ಆದರೆ_ಅಂತ್ಯ

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          ಆದರೆ( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            ಆದರೆ( isLegacy==0 ){
              sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, pTab->pCheck);
            }
            rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.p_New_ಸೂಚ್ಯಂಕ ){
        rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, sParse.p_New_ಸೂಚ್ಯಂಕ->zName);
        ಆದರೆ( isLegacy==0 ){
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.p_New_ಸೂಚ್ಯಂಕ->pPartIdxWhere);
        }
      }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
      ಇಲ್ಲದಿದ್ದರೆ{
        Trigger *pTrigger = sParse.pNewTrigger;
        Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;
        ಆದರೆ( 0==sqlite3_stricmp(sParse.pNewTrigger->ಕೋಷ್ಟಕ_2, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, sParse.pNewTrigger->ಕೋಷ್ಟಕ_2);
        }

        ಆದರೆ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          ಆದರೆ( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            ಗಾಗಿ(p_ಹೆಜ್ಜೆ=pTrigger->ಹೆಜ್ಜೆ_list; p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
              ಆದರೆ( p_ಹೆಜ್ಜೆ->pSrc ){
                ಪೂರ್ಣಾಂಕ i;
                ಗಾಗಿ(i=0; i<p_ಹೆಜ್ಜೆ->pSrc->nSrc; i++){
                  Src_ಅಂಶ *p_ಅಂಶ = &p_ಹೆಜ್ಜೆ->pSrc->a[i];
                  ಆದರೆ( 0==sqlite3_stricmp(p_ಅಂಶ->zName, zOld) ){
                    rename_Token_ಕಂಡುಹಿಡಿ(&sParse, &sCtx, p_ಅಂಶ->zName);
                  }
                }
              }
            }
          }
        }
      }
#ಆದರೆ_ಅಂತ್ಯ
    }

    ಆದರೆ( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_ಒಳಹರಿವು, zNew, bQuote);
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      ಆದರೆ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[3]);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.z_Err_ಸಂದೇಶ ){
        rename_ಸ್ತಂಭ_Parse_Error(context, "", argv[1], argv[2], &sParse);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  }

  ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rename_Quotefix_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_STRING && (p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_DblQuoted) ){
    rename_Token_ಕಂಡುಹಿಡಿ(pWalker->pParse, pWalker->u.pRename, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zDb = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_ಒಳಹರಿವು = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3BtreeEnterAll(db);

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( zDb && z_ಒಳಹರಿವು ){
    ಪೂರ್ಣಾಂಕ rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, 0);

    ಆದರೆ( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, ಗಾತ್ರ(RenameCtx));
      memset(&sWalker, 0, ಗಾತ್ರ(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = rename_Quotefix_ಅಭಿವ್ಯಕ್ತಿ_Cb;
      sWalker.xSelectCallback = rename_ಸ್ತಂಭ_Select_Cb;
      sWalker.u.pRename = &sCtx;

      ಆದರೆ( sParse.p_New_ಕೋಷ್ಟಕ ){
        ಆದರೆ( IsView(sParse.p_New_ಕೋಷ್ಟಕ) ){
          Select *pSelect = sParse.p_New_ಕೋಷ್ಟಕ->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          ಆದರೆ( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ i;
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.p_New_ಕೋಷ್ಟಕ->pCheck);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
          ಗಾಗಿ(i=0; i<sParse.p_New_ಕೋಷ್ಟಕ->n_ಲಂಬಸಾಲು; i++){
            sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker,
               sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(sParse.p_New_ಕೋಷ್ಟಕ,
                                         &sParse.p_New_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i]));
          }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.p_New_ಸೂಚ್ಯಂಕ ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&sWalker, sParse.p_New_ಸೂಚ್ಯಂಕ->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ);
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, sParse.p_New_ಸೂಚ್ಯಂಕ->pPartIdxWhere);
      }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        ಆದರೆ( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
      }

      ಆದರೆ( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_ಒಳಹರಿವು, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      ಆದರೆ( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[1]);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಶೂನ್ಯ rename_ಕೋಷ್ಟಕ_Test(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zDb = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_ಒಳಹರಿವು = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಪೂರ್ಣಾಂಕ b_ತಾತ್ಕಾಲಿಕ = sqlite3_ಮೌಲ್ಯ_int(argv[4]);
  ಪೂರ್ಣಾಂಕ isLegacy = (db->flags & SQLITE_LegacyAlter);
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zWhen = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[5]);
  ಪೂರ್ಣಾಂಕ bNoDQS = sqlite3_ಮೌಲ್ಯ_int(argv[6]);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  ಆದರೆ( zDb && z_ಒಳಹರಿವು ){
    ಪೂರ್ಣಾಂಕ rc;
    Parse sParse;
    u64 flags = db->flags;
    ಆದರೆ( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_ಒಳಹರಿವು, b_ತಾತ್ಕಾಲಿಕ);
    db->flags = flags;
    ಆದರೆ( rc==SQLITE_OK ){
      ಆದರೆ( isLegacy==0 && sParse.p_New_ಕೋಷ್ಟಕ && IsView(sParse.p_New_ಕೋಷ್ಟಕ) ){
        NameContext sNC;
        memset(&sNC, 0, ಗಾತ್ರ(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.p_New_ಕೋಷ್ಟಕ->u.view.pSelect, &sNC);
        ಆದರೆ( sParse.nErr ) rc = sParse.rc;
      }

      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sParse.pNewTrigger ){
        ಆದರೆ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        ಆದರೆ( rc==SQLITE_OK ){
          ಪೂರ್ಣಾಂಕ i1 = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, sParse.pNewTrigger->pTabSchema);
          ಪೂರ್ಣಾಂಕ i2 = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zDb);
          ಆದರೆ( i1==i2 ){
            /* Handle output case B */
            sqlite3_ಫಲಿತಾಂಶ_int(context, 1);
          }
        }
      }
    }

    ಆದರೆ( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      rename_ಸ್ತಂಭ_Parse_Error(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getConstraintToken(ಸ್ಥಿರಾಂಕ u8 *z, ಪೂರ್ಣಾಂಕ *piToken){
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ t = 0;
  ಮಾಡು {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }ಆಗ( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  ಆದರೆ( t==TK_LP ){
    ಪೂರ್ಣಾಂಕ nNest = 1;
    ಆಗ( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      ಆದರೆ( t==TK_LP ){
        nNest++;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_ILLEGAL ){
        ನಿಲ್ಲಿಸು;
      }
    }
  }

  *piToken = t;
  ಹಿಂತಿರುಗಿ iOff;
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
ಸ್ಥಿರ ಶೂನ್ಯ drop_ಸ್ತಂಭ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪೂರ್ಣಾಂಕ iSchema = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = db->aDb[iSchema].zDbSName;
  ಪೂರ್ಣಾಂಕ rc;
  Parse sParse;
  RenameToken *p_ಲಂಬಸಾಲು;
  ಕೋಷ್ಟಕ *pTab;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ;
  ಅಕ್ಷರ *zNew = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು drop_ಸ್ತಂಭ_ಮುಗಿದ;
  pTab = sParse.p_New_ಕೋಷ್ಟಕ;
  ಆದರೆ( pTab==0 || pTab->n_ಲಂಬಸಾಲು==1 || i_ಲಂಬಸಾಲು>=pTab->n_ಲಂಬಸಾಲು ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    ಹೋಗು drop_ಸ್ತಂಭ_ಮುಗಿದ;
  }

  ಆದರೆ( i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು-1 ){
    RenameToken *p_ಕೊನೆ;
    p_ಲಂಬಸಾಲು = rename_Token_ಕಂಡುಹಿಡಿ(&sParse, 0, (ಶೂನ್ಯ*)pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName);
    p_ಕೊನೆ = rename_Token_ಕಂಡುಹಿಡಿ(&sParse, 0, (ಶೂನ್ಯ*)pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು+1].zCnName);
    z_ಕೊನೆ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)p_ಕೊನೆ->t.z;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ eTok;
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
    ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    p_ಲಂಬಸಾಲು = rename_Token_ಕಂಡುಹಿಡಿ(&sParse, 0, (ಶೂನ್ಯ*)pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು-1].zCnName);
    ಮಾಡು {
      p_ಲಂಬಸಾಲು->t.z += getConstraintToken((ಸ್ಥಿರಾಂಕ u8*)p_ಲಂಬಸಾಲು->t.z, &eTok);
    }ಆಗ( eTok!=TK_COMMA );
    p_ಲಂಬಸಾಲು->t.z--;
    z_ಕೊನೆ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&zSql[pTab->u.tab.ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", p_ಲಂಬಸಾಲು->t.z-zSql, zSql, z_ಕೊನೆ);
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_ಸ್ತಂಭ_ಮುಗಿದ:
  renameParseCleanup(&sParse);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
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
ಶೂನ್ಯ sqlite3_Alter_Drop_ಸ್ತಂಭ(Parse *pParse, SrcList *pSrc, ಸ್ಥಿರಾಂಕ Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  ಕೋಷ್ಟಕ *pTab;                    /* Table to modify */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Index of db containing pTab in aDb[] */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;                /* Database containing pTab ("main" etc.) */
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು = 0;                 /* Name of column to drop */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pParse->p_New_ಕೋಷ್ಟಕ==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಆದರೆ( NEVER(db->mallocFailed) ) ಹೋಗು exit_drop_ಸ್ತಂಭ;
  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, &pSrc->a[0]);
  ಆದರೆ( !pTab ) ಹೋಗು exit_drop_ಸ್ತಂಭ;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  ಆದರೆ( SQLITE_OK!=is_Alterable_ಕೋಷ್ಟಕ(pParse, pTab) ) ಹೋಗು exit_drop_ಸ್ತಂಭ;
  ಆದರೆ( SQLITE_OK!=is_Real_ಕೋಷ್ಟಕ(pParse, pTab, 1) ) ಹೋಗು exit_drop_ಸ್ತಂಭ;

  /* Find the index of the column being dropped. */
  z_ಲಂಬಸಾಲು = sqlite3NameFromToken(db, pName);
  ಆದರೆ( z_ಲಂಬಸಾಲು==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಹೋಗು exit_drop_ಸ್ತಂಭ;
  }
  i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, z_ಲಂಬಸಾಲು);
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: \"%T\"", pName);
    ಹೋಗು exit_drop_ಸ್ತಂಭ;
  }
  ಆದರೆ( isRowidAlias(pParse, pTab, z_ಲಂಬಸಾಲು, "drop") ) ಹೋಗು exit_drop_ಸ್ತಂಭ;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot drop %s column: \"%s\"",
        (pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        z_ಲಂಬಸಾಲು
    );
    ಹೋಗು exit_drop_ಸ್ತಂಭ;
  }

  /* Do not allow the number of columns to go to zero */
  ಆದರೆ( pTab->n_ಲಂಬಸಾಲು<=1 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot drop column \"%s\": no other columns exist",z_ಲಂಬಸಾಲು);
    ಹೋಗು exit_drop_ಸ್ತಂಭ;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, zDb, pTab->zName, z_ಲಂಬಸಾಲು) ){
    ಹೋಗು exit_drop_ಸ್ತಂಭ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, i_ಲಂಬಸಾಲು, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  ಆದರೆ( pParse->nErr==0 && (pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)==0 ){
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ addr;
    ಪೂರ್ಣಾಂಕ reg;
    ಪೂರ್ಣಾಂಕ regRec;
    ಸೂಚ್ಯಂಕ *pPk = 0;
    ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ = 0;               /* Number of non-virtual columns after drop */
    ಪೂರ್ಣಾಂಕ iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    ಆದರೆ( HasRowid(pTab) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->n_ಲಂಬಸಾಲು;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      pParse->nMem += pPk->n_ಸ್ತಂಭ;
      ಗಾಗಿ(i=0; i<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iCur, i, reg+i+1);
      }
      n_ಕ್ಷೇತ್ರ = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
    }
    regRec = ++pParse->nMem;
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( i!=i_ಲಂಬಸಾಲು && (pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)==0 ){
        ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು;
        ಆದರೆ( pPk ){
          ಪೂರ್ಣಾಂಕ iPos = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pPk, i);
          ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು_Pos = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pPk, i_ಲಂಬಸಾಲು);
          ಆದರೆ( iPos<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು ) ಮುಂದುವರಿ;
          reg_ಹೊರಹೋಗು = reg+1+iPos-(iPos>i_ಲಂಬಸಾಲು_Pos);
        }ಇಲ್ಲದಿದ್ದರೆ{
          reg_ಹೊರಹೋಗು = reg+1+n_ಕ್ಷೇತ್ರ;
        }
        ಆದರೆ( i==pTab->iPKey ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ಹೊರಹೋಗು);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, i, reg_ಹೊರಹೋಗು);
          ಪ್ರತಿಜ್ಞಾ( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->a_ಲಂಬಸಾಲು[i].affinity==SQLITE_AFF_REAL );
          sqlite3_Vdbe_Delete_ಪೂರ್ವ_Opcode(v, OP_RealAffinity);
        }
        n_ಕ್ಷೇತ್ರ++;
      }
    }
    ಆದರೆ( n_ಕ್ಷೇತ್ರ==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg+1);
      n_ಕ್ಷೇತ್ರ = 1;
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg+1, n_ಕ್ಷೇತ್ರ, regRec);
    ಆದರೆ( pPk ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iCur, regRec, reg+1, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iCur, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
  }

exit_drop_ಸ್ತಂಭ:
  sqlite3DbFree(db, z_ಲಂಬಸಾಲು);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getWhitespace(ಸ್ಥಿರಾಂಕ u8 *z){
  ಪೂರ್ಣಾಂಕ nRet = 0;
  ಆಗ( 1 ){
    ಪೂರ್ಣಾಂಕ t = 0;
    ಪೂರ್ಣಾಂಕ n = sqlite3GetToken(&z[nRet], &t);
    ಆದರೆ( t!=TK_SPACE && t!=TK_COMMENT ) ನಿಲ್ಲಿಸು;
    nRet += n;
  }
  ಹಿಂತಿರುಗಿ nRet;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getConstraint(ಸ್ಥಿರಾಂಕ u8 *z, ಪೂರ್ಣಾಂಕ eTok){
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ t = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  ಆದರೆ( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  ಆದರೆ( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  ಆಗ( 1 ){
    ಪೂರ್ಣಾಂಕ n = getConstraintToken(&z[iOff], &t);
    ಆದರೆ( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      ನಿಲ್ಲಿಸು;
    }
    iOff += n;
  }
  
  ಹಿಂತಿರುಗಿ iOff;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  ಪೂರ್ಣಾಂಕ t,                 /* Token type */
  ಸ್ಥಿರಾಂಕ u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  ಪೂರ್ಣಾಂಕ nQuote,            /* Length of zQuote in bytes */
  ಸ್ಥಿರಾಂಕ u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  ಪೂರ್ಣಾಂಕ *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  ಅಕ್ಷರ *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  ಆದರೆ( t==TK_ILLEGAL ){
    *pRes = 1;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  ಆದರೆ( zCopy==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(ctx);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಸ್ಮರಣೆ_ನಕಲು(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zCopy, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zCmp);
  sqlite3_free(zCopy);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ skip_Create_ಕೋಷ್ಟಕ(sqlite3_context *ctx, ಸ್ಥಿರಾಂಕ u8 *zSql, ಪೂರ್ಣಾಂಕ *piOff){
  ಪೂರ್ಣಾಂಕ iOff = 0;

  ಆದರೆ( zSql==0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  ಆಗ( 1 ){
    ಪೂರ್ಣಾಂಕ t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    ಆದರೆ( t==TK_LP ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( t==TK_ILLEGAL ){
      sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(ctx, SQLITE_CORRUPT_BKPT);
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ u8 *zSql = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಸ್ಥಿರಾಂಕ u8 *zCons = 0;
  ಪೂರ್ಣಾಂಕ iNotNull = -1;
  ಪೂರ್ಣಾಂಕ ii;
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ i_ಆರಂಭ = 0;
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = 0;
  ಅಕ್ಷರ *zNew = 0;
  ಪೂರ್ಣಾಂಕ t = 0;
  sqlite3 *db;
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  ಆದರೆ( zSql==0 ) ಹಿಂತಿರುಗಿ;

  /* Jump past the "CREATE TABLE" bit. */
  ಆದರೆ( skip_Create_ಕೋಷ್ಟಕ(ctx, zSql, &iOff) ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    zCons = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  ಗಾಗಿ(ii=0; i_ಕೊನೆ==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    ಆಗ( 1 ){
      i_ಆರಂಭ = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      ಆದರೆ( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        ಪೂರ್ಣಾಂಕ nTok = 0;
        ಪೂರ್ಣಾಂಕ cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        ಆದರೆ( zCons ){
          ಆದರೆ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ಹಿಂತಿರುಗಿ;
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
        ಆದರೆ( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }ಇಲ್ಲದಿದ್ದರೆ{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        ಆದರೆ( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          ಆದರೆ( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            ಹಿಂತಿರುಗಿ;
          }
          i_ಕೊನೆ = iOff;
          ನಿಲ್ಲಿಸು;
        }

      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_NOT && iNotNull==ii ){
        i_ಕೊನೆ = iOff + getConstraint(&zSql[iOff], 0);
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_RP || t==TK_ILLEGAL ){
        i_ಕೊನೆ = -1;
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_COMMA ){
        ನಿಲ್ಲಿಸು;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  ಆದರೆ( i_ಕೊನೆ<=0 ){
    ಆದರೆ( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zSql, -1, SQLITE_TRANSIENT);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSpace = " ";
    i_ಕೊನೆ += getWhitespace(&zSql[i_ಕೊನೆ]);
    sqlite3GetToken(&zSql[i_ಕೊನೆ], &t);
    ಆದರೆ( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      ಆದರೆ( zSql[i_ಆರಂಭ-1]==',' ) i_ಆರಂಭ--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", i_ಆರಂಭ, zSql, zSpace, &zSql[i_ಕೊನೆ]);
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, zNew, -1, SQLITE_DYNAMIC);
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
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_Constraint_Func(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ u8 *zSql = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCons = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ ii;
  sqlite3_str *pNew;
  ಪೂರ್ಣಾಂಕ t = 0;
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);

  ಆದರೆ( skip_Create_ಕೋಷ್ಟಕ(ctx, zSql, &iOff) ) ಹಿಂತಿರುಗಿ;
  
  ಗಾಗಿ(ii=0; ii<=i_ಲಂಬಸಾಲು || (i_ಲಂಬಸಾಲು<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ಆಗ( 1 ){
      ಪೂರ್ಣಾಂಕ nTok = getConstraintToken(&zSql[iOff], &t);
      ಆದರೆ( t==TK_COMMA || t==TK_RP ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( t==TK_ILLEGAL ){
        sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(ctx, SQLITE_CORRUPT_BKPT);
        ಹಿಂತಿರುಗಿ;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zSql, iOff);
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_ಫಲಿತಾಂಶ_str(ctx, pNew, SQLITE_ಮುಕ್ತಾಯ);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ alter_ಕಂಡುಹಿಡಿ_ಲಂಬಸಾಲು(Parse *pParse, ಕೋಷ್ಟಕ *pTab, Token *p_ಲಂಬಸಾಲು, ಪೂರ್ಣಾಂಕ *pi_ಲಂಬಸಾಲು){
  sqlite3 *db = pParse->db;
  ಅಕ್ಷರ *zName = sqlite3NameFromToken(db, p_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOMEM;
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = -1;

  ಆದರೆ( zName ){
    i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, zName);
    ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_OK;
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( rc==SQLITE_OK ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = db->aDb[sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema)].zDbSName;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
    ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, zDb, pTab->zName, z_ಲಂಬಸಾಲು) ){
      pTab = 0;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3DbFree(db, zName);
  *pi_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು;
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಕೋಷ್ಟಕ *alter_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  ಪೂರ್ಣಾಂಕ *piDb,            /* OUT: write the iDb here */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pzDb,    /* OUT: write name of schema here */
  ಪೂರ್ಣಾಂಕ bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  ಕೋಷ್ಟಕ *pTab = 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, &pSrc->a[0]);
  ಆದರೆ( pTab ){
    ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    ಆದರೆ( SQLITE_OK!=is_Real_ಕೋಷ್ಟಕ(pParse, pTab, 2) 
     || SQLITE_OK!=is_Alterable_ಕೋಷ್ಟಕ(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( pTab && bAuth ){
    ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ಕೋಷ್ಟಕ, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3SrcListDelete(db, pSrc);
  ಹಿಂತಿರುಗಿ pTab;
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
  Token *p_ಲಂಬಸಾಲು       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  ಕೋಷ್ಟಕ *pTab = 0;
  ಪೂರ್ಣಾಂಕ iDb = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = 0;
  ಅಕ್ಷರ *zArg = 0;

  ಪ್ರತಿಜ್ಞಾ( (p_ಲಂಬಸಾಲು==0)!=(pCons==0) );
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alter_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(pParse, pSrc, &iDb, &zDb, pCons!=0);
  ಆದರೆ( !pTab ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( pCons ){
    ಅಕ್ಷರ *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
    ಆದರೆ( alter_ಕಂಡುಹಿಡಿ_ಲಂಬಸಾಲು(pParse, pTab, p_ಲಂಬಸಾಲು, &i_ಲಂಬಸಾಲು) ) ಹಿಂತಿರುಗಿ;
    zArg = sqlite3MPrintf(db, "%d", i_ಲಂಬಸಾಲು);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
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
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  ಪೂರ್ಣಾಂಕ err = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  (ಶೂನ್ಯ)Not_ಬಳಸಿದ;
  sqlite3_ಫಲಿತಾಂಶ_error(ctx, z_ಪಠ್ಯ, -1);
  sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(ctx, err);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pCons,              /* Buffer containing constraint */
  ಪೂರ್ಣಾಂಕ nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = 0;

  ಆದರೆ( zTmp==0 ) ಹಿಂತಿರುಗಿ 0;

  ಆಗ( 1 ){
    ಪೂರ್ಣಾಂಕ t = 0;
    ಪೂರ್ಣಾಂಕ nToken = sqlite3GetToken(&zTmp[iOff], &t);
    ಆದರೆ( t==TK_ILLEGAL ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      i_ಕೊನೆ = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  ಹಿಂತಿರುಗಿ i_ಕೊನೆ;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
ಶೂನ್ಯ sqlite3_Alter_ಗಣ_Not_Null(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *p_ಲಂಬಸಾಲು,     /* Name of the column to add a NOT NULL constraint to */
  Token *p_ಮೊದಲ    /* The NOT token of the NOT NULL constraint text */
){
  ಕೋಷ್ಟಕ *pTab = 0;
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = 0;
  ಪೂರ್ಣಾಂಕ iDb = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pCons = 0;
  ಪೂರ್ಣಾಂಕ nCons = 0;

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alter_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(pParse, pSrc, &iDb, &zDb, 0);
  ಆದರೆ( !pTab ) ಹಿಂತಿರುಗಿ;

  /* Find the column being altered. */
  ಆದರೆ( alter_ಕಂಡುಹಿಡಿ_ಲಂಬಸಾಲು(pParse, pTab, p_ಲಂಬಸಾಲು, &i_ಲಂಬಸಾಲು) ){
    ಹಿಂತಿರುಗಿ;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = p_ಮೊದಲ->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_ಕೊನೆಯ_Token.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (ಪೂರ್ಣಾಂಕ)p_ಲಂಬಸಾಲು->n, p_ಲಂಬಸಾಲು->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, i_ಲಂಬಸಾಲು, nCons, pCons, i_ಲಂಬಸಾಲು, pTab->zName
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
ಸ್ಥಿರ ಶೂನ್ಯ ಕಂಡುಹಿಡಿ_Constraint_Func(
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಂಕ Not_ಬಳಸಿದ,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸ್ಥಿರಾಂಕ u8 *zSql = 0;
  ಸ್ಥಿರಾಂಕ u8 *zCons = 0;
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಪೂರ್ಣಾಂಕ t = 0;

  (ಶೂನ್ಯ)Not_ಬಳಸಿದ;
  zSql = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  zCons = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);

  ಆದರೆ( zSql==0 || zCons==0 ) ಹಿಂತಿರುಗಿ;
  ಆಗ( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  ಆಗ( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ಆದರೆ( t==TK_CONSTRAINT ){
      ಪೂರ್ಣಾಂಕ nTok = 0;
      ಪೂರ್ಣಾಂಕ cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      ಆದರೆ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ಹಿಂತಿರುಗಿ;
      ಆದರೆ( cmp==0 ){
        sqlite3_ಫಲಿತಾಂಶ_int(ctx, 1);
        ಹಿಂತಿರುಗಿ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t==TK_ILLEGAL ){
      ನಿಲ್ಲಿಸು;
    }
  }

  sqlite3_ಫಲಿತಾಂಶ_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
ಶೂನ್ಯ sqlite3_Alter_ಕೂಡಿಸು_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_ಮೊದಲ,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಅಭಿವ್ಯಕ್ತಿ,       /* Text of CHECK expression */
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ,               /* Size of pExpr in bytes */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ              /* The parsed CHECK expression */
){ 
  ಕೋಷ್ಟಕ *pTab = 0;         /* Table identified by pSrc */
  ಪೂರ್ಣಾಂಕ iDb = 0;             /* Which schema does pTab live in */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = 0;     /* Name of the schema in which pTab lives */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pCons = 0;   /* Text of the constraint */
  ಪೂರ್ಣಾಂಕ nCons;               /* Bytes of text to use from pCons[] */
  ಪೂರ್ಣಾಂಕ rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  ಪ್ರತಿಜ್ಞಾ( pSrc->nSrc==1 );
  pTab = alter_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(pParse, pSrc, &iDb, &zDb, 1);
  ಆದರೆ( !pTab ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಹಿಂತಿರುಗಿ;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  ಆದರೆ( pName ){
    ಅಕ್ಷರ *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " "
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
  pCons = p_ಮೊದಲ->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_ಕೊನೆಯ_Token.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " SET "
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
  ಸ್ಥಿರ FuncDef a_Alter_ಕೋಷ್ಟಕ_Funcs[] = {
    ಆಂತರಿಕ_FUNCTION(sqlite_rename_ಸ್ತಂಭ,  9, rename_ಸ್ತಂಭ_Func),
    ಆಂತರಿಕ_FUNCTION(sqlite_rename_ಕೋಷ್ಟಕ,   7, rename_ಕೋಷ್ಟಕ_Func),
    ಆಂತರಿಕ_FUNCTION(sqlite_rename_test,    7, rename_ಕೋಷ್ಟಕ_Test),
    ಆಂತರಿಕ_FUNCTION(sqlite_drop_ಸ್ತಂಭ,    3, drop_ಸ್ತಂಭ_Func),
    ಆಂತರಿಕ_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    ಆಂತರಿಕ_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    ಆಂತರಿಕ_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    ಆಂತರಿಕ_FUNCTION(sqlite_ಕೂಡಿಸು_constraint, 3, ಕೂಡಿಸು_Constraint_Func),
    ಆಂತರಿಕ_FUNCTION(sqlite_ಕಂಡುಹಿಡಿ_constraint,2, ಕಂಡುಹಿಡಿ_Constraint_Func),
  };
  sqlite3_ಸೇರಿಸು_Builtin_Funcs(a_Alter_ಕೋಷ್ಟಕ_Funcs, Array_ಗಾತ್ರ(a_Alter_ಕೋಷ್ಟಕ_Funcs));
}
#ಆದರೆ_ಅಂತ್ಯ  /* SQLITE_ALTER_TABLE */
