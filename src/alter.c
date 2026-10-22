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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ਸਥਿਰ ਪੂਰਨ is_Alterable_ਸਾਰਣੀ(Parse *pParse, ਸਾਰਣੀ *pTab){
  ਜੇ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#ਜੇ_ਅੰਤ
  ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "table %s may not be altered", pTab->zName);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ਸਥਿਰ ਖਾਲੀ renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ਅਚਲ ਅੱਖਰ *zDb,                /* Name of db to verify schema of */
  ਪੂਰਨ b_ਆਰਜ਼ੀ,                      /* True if this is the temp db */
  ਅਚਲ ਅੱਖਰ *zWhen,              /* "when" part of error message */
  ਪੂਰਨ bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->ਖਾਨਾ_ਨਾਂ_ਸੈੱਟ = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_ਆਰਜ਼ੀ, zWhen, bNoDQS
  );

  ਜੇ( b_ਆਰਜ਼ੀ==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_ਸਾਰਣੀ " "
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
ਸਥਿਰ ਖਾਲੀ renameFixQuotes(Parse *pParse, ਅਚਲ ਅੱਖਰ *zDb, ਪੂਰਨ b_ਆਰਜ਼ੀ){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  ਜੇ( b_ਆਰਜ਼ੀ==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_ਸਾਰਣੀ
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
ਸਥਿਰ ਖਾਲੀ renameReloadSchema(Parse *pParse, ਪੂਰਨ iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  ਜੇ( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ਜੋੜੋ_Parse_Schema_Op(pParse->pVdbe, iDb, 0, p5);
    ਜੇ( iDb!=1 ) sqlite3_Vdbe_ਜੋੜੋ_Parse_Schema_Op(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
ਖਾਲੀ sqlite3_Alter_Rename_ਸਾਰਣੀ(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  ਪੂਰਨ iDb;                  /* Database that contains the table */
  ਅੱਖਰ *zDb;                /* Name of database iDb */
  ਸਾਰਣੀ *pTab;              /* Table being renamed */
  ਅੱਖਰ *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  ਪੂਰਨ nTabName;             /* Number of UTF-8 characters in zTabName */
  ਅਚਲ ਅੱਖਰ *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  ਜੇ( NEVER(db->mallocFailed) ) ਜਾਓ exit_rename_ਸਾਰਣੀ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->nSrc==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, &pSrc->a[0]);
  ਜੇ( !pTab ) ਜਾਓ exit_rename_ਸਾਰਣੀ;
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  ਜੇ( !zName ) ਜਾਓ exit_rename_ਸਾਰਣੀ;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  ਜੇ( sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zName, zDb)
   || sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(db, zName, zDb)
   || sqlite3_Is_Shadow_ਸਾਰਣੀ_ਦਾ(db, pTab, zName)
  ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse,
        "there is already another table or index with this name: %s", zName);
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  ਜੇ( SQLITE_OK!=is_Alterable_ਸਾਰਣੀ(pParse, pTab) ){
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }
  ਜੇ( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
  ਜੇ( IsView(pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "view %s may not be altered", pTab->zName);
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, zDb, pTab->zName, 0) ){
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab) ){
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }
  ਜੇ( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    ਜੇ( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#ਜੇ_ਅੰਤ

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  ਜੇ( v==0 ){
    ਜਾਓ exit_rename_ਸਾਰਣੀ;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  ਜੇ( sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#ਜੇ_ਅੰਤ

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  ਜੇ( iDb!=1 ){
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( pVTab ){
    ਪੂਰਨ i = ++pParse->nMem;
    sqlite3_Vdbe_ਲੱਦੋ_String(v, i, zName);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VRename, i, 0, 0,(ਅਚਲ ਅੱਖਰ*)pVTab, P4_VTAB);
  }
#ਜੇ_ਅੰਤ

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_ਸਾਰਣੀ:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_Error_If_Not_ਖਾਲੀ_ਥਾਵਾਂ(
  Parse *pParse,        /* Parsing context */
  ਅਚਲ ਅੱਖਰ *zDb,      /* Schema holding the table */
  ਅਚਲ ਅੱਖਰ *zTab,     /* Table to check for empty */
  ਅਚਲ ਅੱਖਰ *zErr      /* Error message text */
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
ਸਥਿਰ ਪੂਰਨ isRowidAlias(
  Parse *pParse, 
  ਸਾਰਣੀ *pTab, 
  ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ, 
  ਅਚਲ ਅੱਖਰ *zOp
){
  ਜੇ( HasRowid(pTab) && sqlite3IsRowid(z_ਖਾਨਾ) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot %s rowid alias: %s", zOp, z_ਖਾਨਾ);
    ਵਾਪਸ SQLITE_ERROR;
  }
  ਵਾਪਸ SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
ਖਾਲੀ sqlite3_Alter_ਅੰਤ_ਜੋੜੋ_ਕਾਲਮ(Parse *pParse, Token *p_ਖਾਨਾ_Def){
  ਸਾਰਣੀ *pNew;              /* Copy of pParse->pNewTable */
  ਸਾਰਣੀ *pTab;              /* Table being altered */
  ਪੂਰਨ iDb;                  /* Database number */
  ਅਚਲ ਅੱਖਰ *zDb;          /* Database name */
  ਅਚਲ ਅੱਖਰ *zTab;         /* Table name */
  ਅੱਖਰ *z_ਖਾਨਾ;               /* Null-terminated column definition */
  ਕਾਲਮ *p_ਖਾਨਾ;             /* The new column */
  ਵਿਅੰਜਕ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  ਪੂਰਨ r1;                   /* Temporary registers */

  db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
  pNew = pParse->p_New_ਸਾਰਣੀ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew );

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  p_ਖਾਨਾ = &pNew->a_ਖਾਨਾ[pNew->n_ਖਾਨਾ-1];
  pDflt = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pNew, p_ਖਾਨਾ);
  pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zTab, zDb);
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, zDb, pTab->zName, 0) ){
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_PRIMKEY ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "Cannot add a PRIMARY KEY column");
    ਵਾਪਸ;
  }
  ਜੇ( pNew->p_ਸੂਚਕਾਂਕ ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse,
         "Cannot add a UNIQUE column");
    ਵਾਪਸ;
  }
  ਜੇ( isRowidAlias(pParse, pTab, p_ਖਾਨਾ->zCnName, "add") ) ਵਾਪਸ;
  ਜੇ( (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( pDflt==0 || pDflt->op==TK_ਵਿਸਤਾਰ );
    ਜੇ( pDflt && pDflt->p_ਖੱਬਾ->op==TK_NULL ){
      pDflt = 0;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pNew) );
    ਜੇ( (db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ) && pNew->u.tab.pFKey && pDflt ){
      sqlite3_Error_If_Not_ਖਾਲੀ_ਥਾਵਾਂ(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    ਜੇ( p_ਖਾਨਾ->notNull && !pDflt ){
      sqlite3_Error_If_Not_ਖਾਲੀ_ਥਾਵਾਂ(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    ਜੇ( pDflt ){
      sqlite3_ਮੁੱਲ *pVal = 0;
      ਪੂਰਨ rc;
      rc = sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      ਜੇ( rc!=SQLITE_OK ){
        ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed == 1 );
        ਵਾਪਸ;
      }
      ਜੇ( !pVal ){
        sqlite3_Error_If_Not_ਖਾਲੀ_ਥਾਵਾਂ(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_ਮੁੱਲ_Free(pVal);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_STORED ){
    sqlite3_Error_If_Not_ਖਾਲੀ_ਥਾਵਾਂ(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  z_ਖਾਨਾ = sqlite3DbStrNDup(db, (ਅੱਖਰ*)p_ਖਾਨਾ_Def->z, p_ਖਾਨਾ_Def->n);
  ਜੇ( z_ਖਾਨਾ ){
    ਅੱਖਰ *z_ਸਿਰਾ = &z_ਖਾਨਾ[p_ਖਾਨਾ_Def->n-1];
    ਜਦੋਂ( z_ਸਿਰਾ>z_ਖਾਨਾ && (*z_ਸਿਰਾ==';' || sqlite3Isspace(*z_ਸਿਰਾ)) ){
      *z_ਸਿਰਾ-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ, z_ਖਾਨਾ, pNew->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ,
      zTab
    );
    sqlite3DbFree(db, z_ਖਾਨਾ);
  }

  v = sqlite3GetVdbe(pParse);
  ਜੇ( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, r1, -2);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਸੈੱਟ_Cookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_Alter_ਜੋੜੋ);

    /* Verify that constraints are still satisfied */
    ਜੇ( pNew->pCheck!=0
     || (p_ਖਾਨਾ->notNull && (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->ਖਾਨਾ_ਨਾਂ_ਸੈੱਟ = 1;
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
ਖਾਲੀ sqlite3_Alter_Begin_ਜੋੜੋ_ਕਾਲਮ(Parse *pParse, SrcList *pSrc){
  ਸਾਰਣੀ *pNew;
  ਸਾਰਣੀ *pTab;
  ਪੂਰਨ iDb;
  ਪੂਰਨ i;
  ਪੂਰਨ nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->p_New_ਸਾਰਣੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  ਜੇ( NEVER(db->mallocFailed) ) ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, &pSrc->a[0]);
  ਜੇ( !pTab ) ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( IsVirtual(pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "virtual tables may not be altered");
    ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  }
#ਜੇ_ਅੰਤ

  /* Make sure this is not an attempt to ALTER a view. */
  ਜੇ( IsView(pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "Cannot add a column to a view");
    ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  }
  ਜੇ( SQLITE_OK!=is_Alterable_ਸਾਰਣੀ(pParse, pTab) ){
    ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  }

  sqlite3MayAbort(pParse);
  ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ>0 );
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (ਸਾਰਣੀ*)sqlite3DbMallocZero(db, ਆਕਾਰ(ਸਾਰਣੀ));
  ਜੇ( !pNew ) ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  pParse->p_New_ਸਾਰਣੀ = pNew;
  pNew->nTabRef = 1;
  pNew->n_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->n_ਖਾਨਾ>0 );
  nAlloc = (((pNew->n_ਖਾਨਾ-1)/8)*8)+8;
  ਪ੍ਰਤਿਜ੍ਞਾ( nAlloc>=pNew->n_ਖਾਨਾ && nAlloc%8==0 && nAlloc-pNew->n_ਖਾਨਾ<8 );
  pNew->a_ਖਾਨਾ = (ਕਾਲਮ*)sqlite3DbMallocZero(db, ਆਕਾਰ(ਕਾਲਮ)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  ਜੇ( !pNew->a_ਖਾਨਾ || !pNew->zName ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    ਜਾਓ exit_begin_ਜੋੜੋ_ਕਾਲਮ;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->a_ਖਾਨਾ, pTab->a_ਖਾਨਾ, ਆਕਾਰ(ਕਾਲਮ)*(size_t)pNew->n_ਖਾਨਾ);
  ਲਈ(i=0; i<pNew->n_ਖਾਨਾ; i++){
    ਕਾਲਮ *p_ਖਾਨਾ = &pNew->a_ਖਾਨਾ[i];
    p_ਖਾਨਾ->zCnName = sqlite3DbStrDup(db, p_ਖਾਨਾ->zCnName);
    p_ਖਾਨਾ->hName = sqlite3StrIHash(p_ਖਾਨਾ->zCnName);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pNew) );
  pNew->u.tab.pDfltList = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ = pTab->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->nTabRef==1 );

exit_begin_ਜੋੜੋ_ਕਾਲਮ:
  sqlite3SrcListDelete(db, pSrc);
  ਵਾਪਸ;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ਸਥਿਰ ਪੂਰਨ is_Real_ਸਾਰਣੀ(Parse *pParse, ਸਾਰਣੀ *pTab, ਪੂਰਨ iOp){
  ਅਚਲ ਅੱਖਰ *zType = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
  ਜੇ( IsView(pTab) ){
    zType = "view";
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#ਜੇ_ਅੰਤ
  ਜੇ( zType ){
    ਅਚਲ ਅੱਖਰ *az_ਸੁਨੇਹਾ[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    ਪ੍ਰਤਿਜ੍ਞਾ( iOp>=0 && iOp<Array_ਮਾਪ(az_ਸੁਨੇਹਾ) );
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot %s %s \"%s\"",
        az_ਸੁਨੇਹਾ[iOp], zType, pTab->zName
    );
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define is_Real_ਸਾਰਣੀ(x,y,z) (0)
#ਜੇ_ਅੰਤ

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
ਖਾਲੀ sqlite3_Alter_Rename_ਕਾਲਮ(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  ਸਾਰਣੀ *pTab;                    /* Table being updated */
  ਪੂਰਨ i_ਖਾਨਾ;                       /* Index of column being renamed */
  ਅੱਖਰ *zOld = 0;                 /* Old column name */
  ਅੱਖਰ *zNew = 0;                 /* New column name */
  ਅਚਲ ਅੱਖਰ *zDb;                /* Name of schema containing the table */
  ਪੂਰਨ iSchema;                    /* Index of the schema */
  ਪੂਰਨ bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, &pSrc->a[0]);
  ਜੇ( !pTab ) ਜਾਓ exit_rename_ਕਾਲਮ;

  /* Cannot alter a system table */
  ਜੇ( SQLITE_OK!=is_Alterable_ਸਾਰਣੀ(pParse, pTab) ) ਜਾਓ exit_rename_ਕਾਲਮ;
  ਜੇ( SQLITE_OK!=is_Real_ਸਾਰਣੀ(pParse, pTab, 0) ) ਜਾਓ exit_rename_ਕਾਲਮ;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, zDb, pTab->zName, 0) ){
    ਜਾਓ exit_rename_ਕਾਲਮ;
  }
#ਜੇ_ਅੰਤ

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  ਜੇ( !zOld ) ਜਾਓ exit_rename_ਕਾਲਮ;
  i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, zOld);
  ਜੇ( i_ਖਾਨਾ<0 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such column: \"%T\"", pOld);
    ਜਾਓ exit_rename_ਕਾਲਮ;
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
  ਜੇ( !zNew ) ਜਾਓ exit_rename_ਕਾਲਮ;
  ਜੇ( isRowidAlias(pParse, pTab, zOld, "rename from") ) ਜਾਓ exit_rename_ਕਾਲਮ;
  ਜੇ( isRowidAlias(pParse, pTab, zNew, "rename to") ) ਜਾਓ exit_rename_ਕਾਲਮ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, i_ਖਾਨਾ, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  ਜੇ( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, i_ਖਾਨਾ, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_ਕਾਲਮ:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  ਵਾਪਸ;
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
ਬਣਤਰ RenameToken {
  ਅਚਲ ਖਾਲੀ *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_ਅਗਲਾ;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ RenameCtx RenameCtx;
ਬਣਤਰ RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  ਪੂਰਨ nList;                      /* Number of tokens in pList */
  ਪੂਰਨ i_ਖਾਨਾ;                       /* Index of column being renamed */
  ਸਾਰਣੀ *pTab;                    /* Table being ALTERed */
  ਅਚਲ ਅੱਖਰ *zOld;               /* Old column name */
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਖਾਲੀ renameTokenCheckAll(Parse *pParse, ਅਚਲ ਖਾਲੀ *pPtr){
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse==pParse->db->pParse );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ਜੇ( pParse->nErr==0 ){
    ਅਚਲ RenameToken *p;
    u32 i = 1;
    ਲਈ(p=pParse->pRename; p; p=p->p_ਅਗਲਾ){
      ਜੇ( p->p ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( i>0 );
  }
}
#ਨਹੀਂਤਾਂ
# define renameTokenCheckAll(x,y)
#ਜੇ_ਅੰਤ

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
ਅਚਲ ਖਾਲੀ *sqlite3RenameTokenMap(
  Parse *pParse,
  ਅਚਲ ਖਾਲੀ *pPtr,
  ਅਚਲ Token *pToken
){
  RenameToken *pNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  ਜੇ( ALWAYS(pParse->e_Parse_ਬਹੁਲਕ!=PARSE_ਬਹੁਲਕ_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, ਆਕਾਰ(RenameToken));
    ਜੇ( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_ਅਗਲਾ = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  ਵਾਪਸ pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
ਖਾਲੀ sqlite3RenameTokenRemap(Parse *pParse, ਅਚਲ ਖਾਲੀ *p_ਮੰਜ਼ਿਲ, ਅਚਲ ਖਾਲੀ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, p_ਮੰਜ਼ਿਲ);
  ਲਈ(p=pParse->pRename; p; p=p->p_ਅਗਲਾ){
    ਜੇ( p->p==pFrom ){
      p->p = p_ਮੰਜ਼ਿਲ;
      ਰੋਕੋ;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ਸਥਿਰ ਪੂਰਨ rename_Unmap_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ਅਚਲ ਖਾਲੀ*)p_ਵਿਅੰਜਕ);
  ਜੇ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) ){
    sqlite3RenameTokenRemap(pParse, 0, (ਅਚਲ ਖਾਲੀ*)&p_ਵਿਅੰਜਕ->y.pTab);
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ਸਥਿਰ ਖਾਲੀ renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  ਜੇ( pWith ){
    Parse *pParse = pWalker->pParse;
    ਪੂਰਨ i;
    With *pCopy = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( pWith->nCte>0 );
    ਜੇ( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ਧੱਕੋ(pParse, pCopy, 1);
    }
    ਲਈ(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, ਆਕਾਰ(sNC));
      sNC.pParse = pParse;
      ਜੇ( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      ਜੇ( sNC.pParse->db->mallocFailed ) ਵਾਪਸ;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].p_ਖਾਨੇ_ਗਿਣਤੀ);
    }
    ਜੇ( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ਸਥਿਰ ਖਾਲੀ unmap_ਕਾਲਮ_Idlist_ਨਾਂ(
  Parse *pParse,
  ਅਚਲ IdList *pIdList
){
  ਪੂਰਨ ii;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdList!=0 );
  ਲਈ(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ਅਚਲ ਖਾਲੀ*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ਸਥਿਰ ਪੂਰਨ renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ਪੂਰਨ i;
  ਜੇ( pParse->nErr ) ਵਾਪਸ WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  ਜੇ( p->selFlags & (SF_View|SF_CopyCte) ){
    ਵਾਪਸ WRC_Prune;
  }
  ਜੇ( ALWAYS(p->pEList) ){
    ਵਿਅੰਜਕ_List *pList = p->pEList;
    ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
      ਜੇ( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)pList->a[i].zEName);
      }
    }
  }
  ਜੇ( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    ਲਈ(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)pSrc->a[i].zName);
      ਜੇ( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pSrc->a[i].u3.pOn);
      }ਨਹੀਂਤਾਂ{
        unmap_ਕਾਲਮ_Idlist_ਨਾਂ(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  ਵਾਪਸ WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
ਖਾਲੀ sqlite3_Rename_ਵਿਅੰਜਕ_Unmap(Parse *pParse, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  u8 e_ਬਹੁਲਕ = pParse->e_Parse_ਬਹੁਲਕ;
  Walker sWalker;
  memset(&sWalker, 0, ਆਕਾਰ(Walker));
  sWalker.pParse = pParse;
  sWalker.x_ਵਿਅੰਜਕ_Callback = rename_Unmap_ਵਿਅੰਜਕ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->e_Parse_ਬਹੁਲਕ = PARSE_ਬਹੁਲਕ_UNMAP;
  sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਵਿਅੰਜਕ);
  pParse->e_Parse_ਬਹੁਲਕ = e_ਬਹੁਲਕ;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
ਖਾਲੀ sqlite3RenameExprlistUnmap(Parse *pParse, ਵਿਅੰਜਕ_List *pEList){
  ਜੇ( pEList ){
    ਪੂਰਨ i;
    Walker sWalker;
    memset(&sWalker, 0, ਆਕਾਰ(Walker));
    sWalker.pParse = pParse;
    sWalker.x_ਵਿਅੰਜਕ_Callback = rename_Unmap_ਵਿਅੰਜਕ_Cb;
    sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, pEList);
    ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
      ਜੇ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ਸਥਿਰ ਖਾਲੀ renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_ਅਗਲਾ;
  RenameToken *p;
  ਲਈ(p=pToken; p; p=p_ਅਗਲਾ){
    p_ਅਗਲਾ = p->p_ਅਗਲਾ;
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
ਸਥਿਰ RenameToken *rename_Token_ਲੱਭੋ(
  Parse *pParse,
  ਬਣਤਰ RenameCtx *pCtx,
  ਅਚਲ ਖਾਲੀ *pPtr
){
  RenameToken **pp;
  ਜੇ( NEVER(pPtr==0) ){
    ਵਾਪਸ 0;
  }
  ਲਈ(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_ਅਗਲਾ){
    ਜੇ( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      ਜੇ( pCtx ){
        *pp = pToken->p_ਅਗਲਾ;
        pToken->p_ਅਗਲਾ = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      ਵਾਪਸ pToken;
    }
  }
  ਵਾਪਸ 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ਸਥਿਰ ਪੂਰਨ rename_ਕਾਲਮ_Select_Cb(Walker *pWalker, Select *p){
  ਜੇ( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    ਵਾਪਸ WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ rename_ਕਾਲਮ_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  RenameCtx *p = pWalker->u.pRename;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_TRIGGER
   && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==p->i_ਖਾਨਾ
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    rename_Token_ਲੱਭੋ(pWalker->pParse, p, (ਖਾਲੀ*)p_ਵਿਅੰਜਕ);
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==p->i_ਖਾਨਾ
   && ALWAYS(ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ))
   && p->pTab==p_ਵਿਅੰਜਕ->y.pTab
  ){
    rename_Token_ਲੱਭੋ(pWalker->pParse, p, (ਖਾਲੀ*)p_ਵਿਅੰਜਕ);
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ RenameToken *rename_ਕਾਲਮ_Token_ਅਗਲਾ(RenameCtx *pCtx){
  RenameToken *p_ਉੱਤਮ = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  ਲਈ(pToken=p_ਉੱਤਮ->p_ਅਗਲਾ; pToken; pToken=pToken->p_ਅਗਲਾ){
    ਜੇ( pToken->t.z>p_ਉੱਤਮ->t.z ) p_ਉੱਤਮ = pToken;
  }
  ਲਈ(pp=&pCtx->pList; *pp!=p_ਉੱਤਮ; pp=&(*pp)->p_ਅਗਲਾ);
  *pp = p_ਉੱਤਮ->p_ਅਗਲਾ;

  ਵਾਪਸ p_ਉੱਤਮ;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ਸਥਿਰ ਖਾਲੀ errorMPrintf(sqlite3_context *pCtx, ਅਚਲ ਅੱਖਰ *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ਅੱਖਰ *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  ਜੇ( zErr ){
    sqlite3_ਨਤੀਜਾ_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
ਸਥਿਰ ਖਾਲੀ rename_ਕਾਲਮ_Parse_Error(
  sqlite3_context *pCtx,
  ਅਚਲ ਅੱਖਰ *zWhen,
  sqlite3_ਮੁੱਲ *pType,
  sqlite3_ਮੁੱਲ *pObject,
  Parse *pParse
){
  ਅਚਲ ਅੱਖਰ *zT = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pType);
  ਅਚਲ ਅੱਖਰ *zN = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pObject);
  ਅੱਖਰ *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_ਸੁਨੇਹਾ
  );
  sqlite3_ਨਤੀਜਾ_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
ਸਥਿਰ ਖਾਲੀ rename_ਕਾਲਮ_Elist_ਨਾਂ(
  Parse *pParse,
  RenameCtx *pCtx,
  ਅਚਲ ਵਿਅੰਜਕ_List *pEList,
  ਅਚਲ ਅੱਖਰ *zOld
){
  ਜੇ( pEList ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
      ਅਚਲ ਅੱਖਰ *zName = pEList->a[i].zEName;
      ਜੇ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        rename_Token_ਲੱਭੋ(pParse, pCtx, (ਅਚਲ ਖਾਲੀ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ਸਥਿਰ ਖਾਲੀ rename_ਕਾਲਮ_Idlist_ਨਾਂ(
  Parse *pParse,
  RenameCtx *pCtx,
  ਅਚਲ IdList *pIdList,
  ਅਚਲ ਅੱਖਰ *zOld
){
  ਜੇ( pIdList ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pIdList->nId; i++){
      ਅਚਲ ਅੱਖਰ *zName = pIdList->a[i].zName;
      ਜੇ( 0==sqlite3_stricmp(zName, zOld) ){
        rename_Token_ਲੱਭੋ(pParse, pCtx, (ਅਚਲ ਖਾਲੀ*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ਸਥਿਰ ਪੂਰਨ renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ਅਚਲ ਅੱਖਰ *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ਅਚਲ ਅੱਖਰ *zSql,               /* SQL to parse */
  ਪੂਰਨ b_ਆਰਜ਼ੀ                       /* True if SQL is from temp schema */
){
  ਪੂਰਨ rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  ਜੇ( zSql==0 ){
    ਵਾਪਸ SQLITE_NOMEM;
  }
  ਜੇ( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਜੇ( b_ਆਰਜ਼ੀ ){
    db->init.iDb = 1;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->e_Parse_ਬਹੁਲਕ = PARSE_ਬਹੁਲਕ_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && ਲੜੀ_ਖੋਜ(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3_ਲੜੀਵਾਰ_Parser(p, zSql);
  db->flags = flags;
  ਜੇ( db->mallocFailed ) rc = SQLITE_NOMEM;
  ਜੇ( rc==SQLITE_OK
   && NEVER(p->p_New_ਸਾਰਣੀ==0 && p->p_New_ਸੂਚਕਾਂਕ==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    ਲਈ(pToken=p->pRename; pToken; pToken=pToken->p_ਅਗਲਾ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#ਜੇ_ਅੰਤ

  db->init.iDb = 0;
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ਅਚਲ ਅੱਖਰ *zSql,               /* SQL statement to edit */
  ਅਚਲ ਅੱਖਰ *zNew,               /* New token text */
  ਪੂਰਨ bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ਪੂਰਨ rc = SQLITE_OK;
  ਅੱਖਰ *zQuot = 0;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  i64 nQuot = 0;
  ਅੱਖਰ *zBuf1 = 0;
  ਅੱਖਰ *zBuf2 = 0;

  ਜੇ( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    ਜੇ( zQuot==0 ){
      ਵਾਪਸ SQLITE_NOMEM;
    }ਨਹੀਂਤਾਂ{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( nQuot>=nNew && nSql>=0 && nNew>=0 );
    z_ਬਾਹਰਲਾ = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( nSql>0 );
    z_ਬਾਹਰਲਾ = (ਅੱਖਰ*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    ਜੇ( z_ਬਾਹਰਲਾ ){
      zBuf1 = &z_ਬਾਹਰਲਾ[nSql*2+1];
      zBuf2 = &z_ਬਾਹਰਲਾ[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  ਜੇ( z_ਬਾਹਰਲਾ ){
    i64 n_ਬਾਹਰਲਾ = nSql;
    ਪ੍ਰਤਿਜ੍ਞਾ( nSql>0 );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, zSql, (size_t)nSql);
    ਜਦੋਂ( pRename->pList ){
      ਪੂਰਨ iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ਅਚਲ ਅੱਖਰ *zReplace;
      RenameToken *p_ਉੱਤਮ = rename_ਕਾਲਮ_Token_ਅਗਲਾ(pRename);

      ਜੇ( zNew ){
        ਜੇ( bQuote==0 && sqlite3IsIdChar(*(u8*)p_ਉੱਤਮ->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }ਨਹੀਂਤਾਂ{
          nReplace = nQuot;
          zReplace = zQuot;
          ਜੇ( p_ਉੱਤਮ->t.z[p_ਉੱਤਮ->t.n]=='"' ) nReplace++;
        }
      }ਨਹੀਂਤਾਂ{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf1, p_ਉੱਤਮ->t.z, p_ਉੱਤਮ->t.n);
        zBuf1[p_ਉੱਤਮ->t.n] = 0;
        sqlite3Dequote(zBuf1);
        ਪ੍ਰਤਿਜ੍ਞਾ( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((ਪੂਰਨ)(nSql*2), zBuf2, "%Q%s", zBuf1,
            p_ਉੱਤਮ->t.z[p_ਉੱਤਮ->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (ਪੂਰਨ)(p_ਉੱਤਮ->t.z - zSql);
      ਜੇ( p_ਉੱਤਮ->t.n!=nReplace ){
        memmove(&z_ਬਾਹਰਲਾ[iOff + nReplace], &z_ਬਾਹਰਲਾ[iOff + p_ਉੱਤਮ->t.n],
            n_ਬਾਹਰਲਾ - (iOff + p_ਉੱਤਮ->t.n)
        );
        n_ਬਾਹਰਲਾ += nReplace - p_ਉੱਤਮ->t.n;
        z_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ] = '\0';
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&z_ਬਾਹਰਲਾ[iOff], zReplace, nReplace);
      sqlite3DbFree(db, p_ਉੱਤਮ);
    }

    sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, z_ਬਾਹਰਲਾ, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, z_ਬਾਹਰਲਾ);
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  ਵਾਪਸ rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ਸਥਿਰ ਖਾਲੀ rename_ਸੈੱਟ_ENames(ਵਿਅੰਜਕ_List *pEList, ਪੂਰਨ val){
  ਪ੍ਰਤਿਜ੍ਞਾ( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_ਵਿਸਤਾਰ );
  ਜੇ( pEList ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
ਸਥਿਰ ਪੂਰਨ renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  Trigger_ਕਦਮ *p_ਕਦਮ;
  NameContext sNC;
  ਪੂਰਨ rc = SQLITE_OK;

  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pParse = pParse;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, pNew->ਸਾਰਣੀ_2,
      db->aDb[sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  ਜੇ( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  ਜੇ( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pNew->pWhen);
  }

  ਲਈ(p_ਕਦਮ=pNew->ਕਦਮ_list; rc==SQLITE_OK && p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
    ਜੇ( p_ਕਦਮ->pSelect ){
      sqlite3SelectPrep(pParse, p_ਕਦਮ->pSelect, &sNC);
      ਜੇ( pParse->nErr ) rc = pParse->rc;
    }
    ਜੇ( rc==SQLITE_OK && p_ਕਦਮ->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, p_ਕਦਮ->pSrc, 0);
      ਜੇ( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        ਜੇ( pSel==0 ){
          p_ਕਦਮ->p_ਵਿਅੰਜਕ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }ਨਹੀਂਤਾਂ{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          rename_ਸੈੱਟ_ENames(p_ਕਦਮ->p_ਵਿਅੰਜਕ_List, ENAME_ਵਿਸਤਾਰ);
          sqlite3SelectPrep(pParse, pSel, 0);
          rename_ਸੈੱਟ_ENames(p_ਕਦਮ->p_ਵਿਅੰਜਕ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕਦਮ->p_ਵਿਅੰਜਕ_List==0 || p_ਕਦਮ->p_ਵਿਅੰਜਕ_List==pSel->pEList );
          ਪ੍ਰਤਿਜ੍ਞਾ( pSrc==pSel->pSrc );
          ਜੇ( p_ਕਦਮ->p_ਵਿਅੰਜਕ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        ਜੇ( ALWAYS(p_ਕਦਮ->pSrc) ){
          ਪੂਰਨ i;
          ਲਈ(i=0; i<p_ਕਦਮ->pSrc->nSrc && rc==SQLITE_OK; i++){
            Src_ਵਸਤੂ *p = &p_ਕਦਮ->pSrc->a[i];
            ਜੇ( p->fg.isSubquery ){
              ਪ੍ਰਤਿਜ੍ਞਾ( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        ਜੇ(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        ਜੇ( rc==SQLITE_OK && p_ਕਦਮ->pWhere ){
          rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, p_ਕਦਮ->pWhere);
        }
        ਜੇ( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List);
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( !p_ਕਦਮ->pUpsert || (!p_ਕਦਮ->pWhere && !p_ਕਦਮ->p_ਵਿਅੰਜਕ_List) );
        ਜੇ( p_ਕਦਮ->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = p_ਕਦਮ->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ);
          ਜੇ( rc==SQLITE_OK ){
            ਵਿਅੰਜਕ_List *p_Upsert_ਸੈੱਟ = pUpsert->p_Upsert_ਸੈੱਟ;
            rc = sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, p_Upsert_ਸੈੱਟ);
          }
          ਜੇ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pUpsert->pUpsertWhere);
          }
          ਜੇ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ_Where);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_NOMEM;
      }
    }
  }
  ਵਾਪਸ rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ਸਥਿਰ ਖਾਲੀ renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  Trigger_ਕਦਮ *p_ਕਦਮ;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  ਲਈ(p_ਕਦਮ=pTrigger->ਕਦਮ_list; p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
    sqlite3WalkSelect(pWalker, p_ਕਦਮ->pSelect);
    sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਕਦਮ->pWhere);
    sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List);
    ਜੇ( p_ਕਦਮ->pUpsert ){
      Upsert *pUpsert = p_ਕਦਮ->pUpsert;
      sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ);
      sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pUpsert->p_Upsert_ਸੈੱਟ);
      sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ_Where);
    }
    ਜੇ( p_ਕਦਮ->pSrc ){
      ਪੂਰਨ i;
      SrcList *pSrc = p_ਕਦਮ->pSrc;
      ਲਈ(i=0; i<pSrc->nSrc; i++){
        ਜੇ( pSrc->a[i].fg.isSubquery ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->a[i].u4.pSubq!=0 );
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
ਸਥਿਰ ਖਾਲੀ renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  ਸੂਚਕਾਂਕ *pIdx;
  ਜੇ( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3_Delete_ਸਾਰਣੀ(db, pParse->p_New_ਸਾਰਣੀ);
  ਜਦੋਂ( (pIdx = pParse->p_New_ਸੂਚਕਾਂਕ)!=0 ){
    pParse->p_New_ਸੂਚਕਾਂਕ = pIdx->p_ਅਗਲਾ;
    sqlite3_Free_ਸੂਚਕਾਂਕ(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_ਸੁਨੇਹਾ);
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
ਸਥਿਰ ਖਾਲੀ rename_ਕਾਲਮ_Func(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ਅਚਲ ਅੱਖਰ *zSql = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅਚਲ ਅੱਖਰ *zDb = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[3]);
  ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[4]);
  ਪੂਰਨ i_ਖਾਨਾ = sqlite3_ਮੁੱਲ_int(argv[5]);
  ਅਚਲ ਅੱਖਰ *zNew = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[6]);
  ਪੂਰਨ bQuote = sqlite3_ਮੁੱਲ_int(argv[7]);
  ਪੂਰਨ b_ਆਰਜ਼ੀ = sqlite3_ਮੁੱਲ_int(argv[8]);
  ਅਚਲ ਅੱਖਰ *zOld;
  ਪੂਰਨ rc;
  Parse sParse;
  Walker sWalker;
  ਸੂਚਕਾਂਕ *pIdx;
  ਪੂਰਨ i;
  ਸਾਰਣੀ *pTab;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#ਜੇ_ਅੰਤ

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( zSql==0 ) ਵਾਪਸ;
  ਜੇ( z_ਸਾਰਣੀ==0 ) ਵਾਪਸ;
  ਜੇ( zNew==0 ) ਵਾਪਸ;
  ਜੇ( i_ਖਾਨਾ<0 ) ਵਾਪਸ;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, z_ਸਾਰਣੀ, zDb);
  ਜੇ( pTab==0 || i_ਖਾਨਾ>=pTab->n_ਖਾਨਾ ){
    sqlite3BtreeLeaveAll(db);
    ਵਾਪਸ;
  }
  zOld = pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
  memset(&sCtx, 0, ਆਕਾਰ(sCtx));
  sCtx.i_ਖਾਨਾ = ((i_ਖਾਨਾ==pTab->iPKey) ? -1 : i_ਖਾਨਾ);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#ਜੇ_ਅੰਤ
  rc = renameParseSql(&sParse, zDb, db, zSql, b_ਆਰਜ਼ੀ);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, ਆਕਾਰ(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_ਵਿਅੰਜਕ_Callback = rename_ਕਾਲਮ_ਵਿਅੰਜਕ_Cb;
  sWalker.xSelectCallback = rename_ਕਾਲਮ_Select_Cb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ rename_ਕਾਲਮ_Func_ਪੂਰਾ;
  ਜੇ( sParse.p_New_ਸਾਰਣੀ ){
    ਜੇ( IsView(sParse.p_New_ਸਾਰਣੀ) ){
      Select *pSelect = sParse.p_New_ਸਾਰਣੀ->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      ਜੇ( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      ਜੇ( rc!=SQLITE_OK ) ਜਾਓ rename_ਕਾਲਮ_Func_ਪੂਰਾ;
    }ਨਹੀਂਤਾਂ ਜੇ( Is_Ordinary_ਸਾਰਣੀ(sParse.p_New_ਸਾਰਣੀ) ){
      /* A regular table */
      ਪੂਰਨ bFKOnly = sqlite3_stricmp(z_ਸਾਰਣੀ, sParse.p_New_ਸਾਰਣੀ->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.p_New_ਸਾਰਣੀ;
      ਜੇ( bFKOnly==0 ){
        ਜੇ( i_ਖਾਨਾ<sParse.p_New_ਸਾਰਣੀ->n_ਖਾਨਾ ){
          rename_Token_ਲੱਭੋ(
              &sParse, &sCtx, (ਖਾਲੀ*)sParse.p_New_ਸਾਰਣੀ->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName
          );
        }
        ਜੇ( sCtx.i_ਖਾਨਾ<0 ){
          rename_Token_ਲੱਭੋ(&sParse, &sCtx, (ਖਾਲੀ*)&sParse.p_New_ਸਾਰਣੀ->iPKey);
        }
        sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, sParse.p_New_ਸਾਰਣੀ->pCheck);
        ਲਈ(pIdx=sParse.p_New_ਸਾਰਣੀ->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
          sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ);
        }
        ਲਈ(pIdx=sParse.p_New_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
          sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ);
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
        ਲਈ(i=0; i<sParse.p_New_ਸਾਰਣੀ->n_ਖਾਨਾ; i++){
          ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(sParse.p_New_ਸਾਰਣੀ,
                                                  &sParse.p_New_ਸਾਰਣੀ->a_ਖਾਨਾ[i]);
          sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਵਿਅੰਜਕ);
        }
#ਜੇ_ਅੰਤ
      }

      ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(sParse.p_New_ਸਾਰਣੀ) );
      ਲਈ(pFKey=sParse.p_New_ਸਾਰਣੀ->u.tab.pFKey; pFKey; pFKey=pFKey->p_ਅਗਲਾ_From){
        ਲਈ(i=0; i<pFKey->n_ਖਾਨਾ; i++){
          ਜੇ( bFKOnly==0 && pFKey->a_ਖਾਨਾ[i].iFrom==i_ਖਾਨਾ ){
            rename_Token_ਲੱਭੋ(&sParse, &sCtx, (ਖਾਲੀ*)&pFKey->a_ਖਾਨਾ[i]);
          }
          ਜੇ( 0==sqlite3_stricmp(pFKey->z_ਮੰਜ਼ਿਲ, z_ਸਾਰਣੀ)
           && 0==sqlite3_stricmp(pFKey->a_ਖਾਨਾ[i].z_ਖਾਨਾ, zOld)
          ){
            rename_Token_ਲੱਭੋ(&sParse, &sCtx, (ਖਾਲੀ*)pFKey->a_ਖਾਨਾ[i].z_ਖਾਨਾ);
          }
        }
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( sParse.p_New_ਸੂਚਕਾਂਕ ){
    sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, sParse.p_New_ਸੂਚਕਾਂਕ->a_ਖਾਨਾ_ਵਿਅੰਜਕ);
    sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, sParse.p_New_ਸੂਚਕਾਂਕ->pPartIdxWhere);
  }ਨਹੀਂਤਾਂ{
    /* A trigger */
    Trigger_ਕਦਮ *p_ਕਦਮ;
    rc = renameResolveTrigger(&sParse);
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ rename_ਕਾਲਮ_Func_ਪੂਰਾ;

    ਲਈ(p_ਕਦਮ=sParse.pNewTrigger->ਕਦਮ_list; p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
      ਜੇ( p_ਕਦਮ->pSrc ){
        ਸਾਰਣੀ *p_ਨਿਸ਼ਾਨਾ = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(&sParse, 0, &p_ਕਦਮ->pSrc->a[0]);
        ਜੇ( p_ਨਿਸ਼ਾਨਾ==pTab ){
          ਜੇ( p_ਕਦਮ->pUpsert ){
            ਵਿਅੰਜਕ_List *p_Upsert_ਸੈੱਟ = p_ਕਦਮ->pUpsert->p_Upsert_ਸੈੱਟ;
            rename_ਕਾਲਮ_Elist_ਨਾਂ(&sParse, &sCtx, p_Upsert_ਸੈੱਟ, zOld);
          }
          rename_ਕਾਲਮ_Idlist_ਨਾਂ(&sParse, &sCtx, p_ਕਦਮ->pIdList, zOld);
          rename_ਕਾਲਮ_Elist_ਨਾਂ(&sParse, &sCtx, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    ਜੇ( sParse.pTriggerTab==pTab ){
      rename_ਕਾਲਮ_Idlist_ਨਾਂ(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

rename_ਕਾਲਮ_Func_ਪੂਰਾ:
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[0]);
    }ਨਹੀਂਤਾਂ ਜੇ( sParse.z_Err_ਸੁਨੇਹਾ ){
      rename_ਕਾਲਮ_Parse_Error(context, "", argv[1], argv[2], &sParse);
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ਜੇ_ਅੰਤ
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
ਸਥਿਰ ਪੂਰਨ rename_ਸਾਰਣੀ_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  RenameCtx *p = pWalker->u.pRename;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   && ALWAYS(ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ))
   && p->pTab==p_ਵਿਅੰਜਕ->y.pTab
  ){
    rename_Token_ਲੱਭੋ(pWalker->pParse, p, (ਖਾਲੀ*)&p_ਵਿਅੰਜਕ->y.pTab);
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ਸਥਿਰ ਪੂਰਨ rename_ਸਾਰਣੀ_Select_Cb(Walker *pWalker, Select *pSelect){
  ਪੂਰਨ i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  ਜੇ( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    ਵਾਪਸ WRC_Prune;
  }
  ਜੇ( NEVER(pSrc==0) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pWalker->pParse->db->mallocFailed );
    ਵਾਪਸ WRC_Abort;
  }
  ਲਈ(i=0; i<pSrc->nSrc; i++){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[i];
    ਜੇ( p_ਵਸਤੂ->pSTab==p->pTab ){
      rename_Token_ਲੱਭੋ(pWalker->pParse, p, p_ਵਸਤੂ->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਖਾਲੀ rename_ਸਾਰਣੀ_Func(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅਚਲ ਅੱਖਰ *zDb = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅਚਲ ਅੱਖਰ *z_ਨਿਵੇਸ਼ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[3]);
  ਅਚਲ ਅੱਖਰ *zOld = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[4]);
  ਅਚਲ ਅੱਖਰ *zNew = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[5]);
  ਪੂਰਨ b_ਆਰਜ਼ੀ = sqlite3_ਮੁੱਲ_int(argv[6]);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  ਜੇ( z_ਨਿਵੇਸ਼ && zOld && zNew ){
    Parse sParse;
    ਪੂਰਨ rc;
    ਪੂਰਨ bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#ਜੇ_ਅੰਤ

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, ਆਕਾਰ(RenameCtx));
    sCtx.pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zOld, zDb);
    memset(&sWalker, 0, ਆਕਾਰ(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_ਵਿਅੰਜਕ_Callback = rename_ਸਾਰਣੀ_ਵਿਅੰਜਕ_Cb;
    sWalker.xSelectCallback = rename_ਸਾਰਣੀ_Select_Cb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_ਨਿਵੇਸ਼, b_ਆਰਜ਼ੀ);

    ਜੇ( rc==SQLITE_OK ){
      ਪੂਰਨ isLegacy = (db->flags & SQLITE_LegacyAlter);
      ਜੇ( sParse.p_New_ਸਾਰਣੀ ){
        ਸਾਰਣੀ *pTab = sParse.p_New_ਸਾਰਣੀ;

        ਜੇ( IsView(pTab) ){
          ਜੇ( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, ਆਕਾਰ(sNC));
            sNC.pParse = &sParse;

            ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            ਜੇ( sParse.nErr ){
              rc = sParse.rc;
            }ਨਹੀਂਤਾਂ{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }ਨਹੀਂਤਾਂ{
          /* Modify any FK definitions to point to the new table. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
          ਜੇ( (isLegacy==0 || (db->flags & SQLITE_Foreign_ਕੁੰਜੀਆਂ))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
            ਲਈ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ਅਗਲਾ_From){
              ਜੇ( sqlite3_stricmp(pFKey->z_ਮੰਜ਼ਿਲ, zOld)==0 ){
                rename_Token_ਲੱਭੋ(&sParse, &sCtx, (ਖਾਲੀ*)pFKey->z_ਮੰਜ਼ਿਲ);
              }
            }
          }
#ਜੇ_ਅੰਤ

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          ਜੇ( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            ਜੇ( isLegacy==0 ){
              sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, pTab->pCheck);
            }
            rename_Token_ਲੱਭੋ(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      ਨਹੀਂਤਾਂ ਜੇ( sParse.p_New_ਸੂਚਕਾਂਕ ){
        rename_Token_ਲੱਭੋ(&sParse, &sCtx, sParse.p_New_ਸੂਚਕਾਂਕ->zName);
        ਜੇ( isLegacy==0 ){
          sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, sParse.p_New_ਸੂਚਕਾਂਕ->pPartIdxWhere);
        }
      }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
      ਨਹੀਂਤਾਂ{
        Trigger *pTrigger = sParse.pNewTrigger;
        Trigger_ਕਦਮ *p_ਕਦਮ;
        ਜੇ( 0==sqlite3_stricmp(sParse.pNewTrigger->ਸਾਰਣੀ_2, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          rename_Token_ਲੱਭੋ(&sParse, &sCtx, sParse.pNewTrigger->ਸਾਰਣੀ_2);
        }

        ਜੇ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          ਜੇ( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            ਲਈ(p_ਕਦਮ=pTrigger->ਕਦਮ_list; p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
              ਜੇ( p_ਕਦਮ->pSrc ){
                ਪੂਰਨ i;
                ਲਈ(i=0; i<p_ਕਦਮ->pSrc->nSrc; i++){
                  Src_ਵਸਤੂ *p_ਵਸਤੂ = &p_ਕਦਮ->pSrc->a[i];
                  ਜੇ( 0==sqlite3_stricmp(p_ਵਸਤੂ->zName, zOld) ){
                    rename_Token_ਲੱਭੋ(&sParse, &sCtx, p_ਵਸਤੂ->zName);
                  }
                }
              }
            }
          }
        }
      }
#ਜੇ_ਅੰਤ
    }

    ਜੇ( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_ਨਿਵੇਸ਼, zNew, bQuote);
    }
    ਜੇ( rc!=SQLITE_OK ){
      ਜੇ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[3]);
      }ਨਹੀਂਤਾਂ ਜੇ( sParse.z_Err_ਸੁਨੇਹਾ ){
        rename_ਕਾਲਮ_Parse_Error(context, "", argv[1], argv[2], &sParse);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#ਜੇ_ਅੰਤ
  }

  ਵਾਪਸ;
}

ਸਥਿਰ ਪੂਰਨ rename_Quotefix_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_STRING && (p_ਵਿਅੰਜਕ->flags & EP_DblQuoted) ){
    rename_Token_ਲੱਭੋ(pWalker->pParse, pWalker->u.pRename, (ਅਚਲ ਖਾਲੀ*)p_ਵਿਅੰਜਕ);
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਖਾਲੀ renameQuotefixFunc(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅੱਖਰ ਅਚਲ *zDb = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅੱਖਰ ਅਚਲ *z_ਨਿਵੇਸ਼ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ਜੇ_ਅੰਤ

  sqlite3BtreeEnterAll(db);

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( zDb && z_ਨਿਵੇਸ਼ ){
    ਪੂਰਨ rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_ਨਿਵੇਸ਼, 0);

    ਜੇ( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, ਆਕਾਰ(RenameCtx));
      memset(&sWalker, 0, ਆਕਾਰ(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_ਵਿਅੰਜਕ_Callback = rename_Quotefix_ਵਿਅੰਜਕ_Cb;
      sWalker.xSelectCallback = rename_ਕਾਲਮ_Select_Cb;
      sWalker.u.pRename = &sCtx;

      ਜੇ( sParse.p_New_ਸਾਰਣੀ ){
        ਜੇ( IsView(sParse.p_New_ਸਾਰਣੀ) ){
          Select *pSelect = sParse.p_New_ਸਾਰਣੀ->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          ਜੇ( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ i;
          sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, sParse.p_New_ਸਾਰਣੀ->pCheck);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
          ਲਈ(i=0; i<sParse.p_New_ਸਾਰਣੀ->n_ਖਾਨਾ; i++){
            sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker,
               sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(sParse.p_New_ਸਾਰਣੀ,
                                         &sParse.p_New_ਸਾਰਣੀ->a_ਖਾਨਾ[i]));
          }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }ਨਹੀਂਤਾਂ ਜੇ( sParse.p_New_ਸੂਚਕਾਂਕ ){
        sqlite3_Walk_ਵਿਅੰਜਕ_List(&sWalker, sParse.p_New_ਸੂਚਕਾਂਕ->a_ਖਾਨਾ_ਵਿਅੰਜਕ);
        sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, sParse.p_New_ਸੂਚਕਾਂਕ->pPartIdxWhere);
      }ਨਹੀਂਤਾਂ{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        ਜੇ( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
      }

      ਜੇ( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_ਨਿਵੇਸ਼, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    ਜੇ( rc!=SQLITE_OK ){
      ਜੇ( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[1]);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ rename_ਸਾਰਣੀ_Test(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਅੱਖਰ ਅਚਲ *zDb = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅੱਖਰ ਅਚਲ *z_ਨਿਵੇਸ਼ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਪੂਰਨ b_ਆਰਜ਼ੀ = sqlite3_ਮੁੱਲ_int(argv[4]);
  ਪੂਰਨ isLegacy = (db->flags & SQLITE_LegacyAlter);
  ਅੱਖਰ ਅਚਲ *zWhen = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[5]);
  ਪੂਰਨ bNoDQS = sqlite3_ਮੁੱਲ_int(argv[6]);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ਜੇ_ਅੰਤ

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  ਜੇ( zDb && z_ਨਿਵੇਸ਼ ){
    ਪੂਰਨ rc;
    Parse sParse;
    u64 flags = db->flags;
    ਜੇ( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_ਨਿਵੇਸ਼, b_ਆਰਜ਼ੀ);
    db->flags = flags;
    ਜੇ( rc==SQLITE_OK ){
      ਜੇ( isLegacy==0 && sParse.p_New_ਸਾਰਣੀ && IsView(sParse.p_New_ਸਾਰਣੀ) ){
        NameContext sNC;
        memset(&sNC, 0, ਆਕਾਰ(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.p_New_ਸਾਰਣੀ->u.view.pSelect, &sNC);
        ਜੇ( sParse.nErr ) rc = sParse.rc;
      }

      ਨਹੀਂਤਾਂ ਜੇ( sParse.pNewTrigger ){
        ਜੇ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        ਜੇ( rc==SQLITE_OK ){
          ਪੂਰਨ i1 = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, sParse.pNewTrigger->pTabSchema);
          ਪੂਰਨ i2 = sqlite3_ਲੱਭੋ_Db_Name(db, zDb);
          ਜੇ( i1==i2 ){
            /* Handle output case B */
            sqlite3_ਨਤੀਜਾ_int(context, 1);
          }
        }
      }
    }

    ਜੇ( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      rename_ਕਾਲਮ_Parse_Error(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਪੂਰਨ getConstraintToken(ਅਚਲ u8 *z, ਪੂਰਨ *piToken){
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ t = 0;
  ਕਰੋ {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }ਜਦੋਂ( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  ਜੇ( t==TK_LP ){
    ਪੂਰਨ nNest = 1;
    ਜਦੋਂ( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      ਜੇ( t==TK_LP ){
        nNest++;
      }ਨਹੀਂਤਾਂ ਜੇ( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }ਨਹੀਂਤਾਂ ਜੇ( t==TK_ILLEGAL ){
        ਰੋਕੋ;
      }
    }
  }

  *piToken = t;
  ਵਾਪਸ iOff;
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
ਸਥਿਰ ਖਾਲੀ drop_ਕਾਲਮ_Func(
  sqlite3_context *context,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਪੂਰਨ iSchema = sqlite3_ਮੁੱਲ_int(argv[0]);
  ਅਚਲ ਅੱਖਰ *zSql = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਪੂਰਨ i_ਖਾਨਾ = sqlite3_ਮੁੱਲ_int(argv[2]);
  ਅਚਲ ਅੱਖਰ *zDb = db->aDb[iSchema].zDbSName;
  ਪੂਰਨ rc;
  Parse sParse;
  RenameToken *p_ਖਾਨਾ;
  ਸਾਰਣੀ *pTab;
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ;
  ਅੱਖਰ *zNew = 0;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#ਜੇ_ਅੰਤ

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ drop_ਕਾਲਮ_ਪੂਰਾ;
  pTab = sParse.p_New_ਸਾਰਣੀ;
  ਜੇ( pTab==0 || pTab->n_ਖਾਨਾ==1 || i_ਖਾਨਾ>=pTab->n_ਖਾਨਾ ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    ਜਾਓ drop_ਕਾਲਮ_ਪੂਰਾ;
  }

  ਜੇ( i_ਖਾਨਾ<pTab->n_ਖਾਨਾ-1 ){
    RenameToken *p_ਸਿਰਾ;
    p_ਖਾਨਾ = rename_Token_ਲੱਭੋ(&sParse, 0, (ਖਾਲੀ*)pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName);
    p_ਸਿਰਾ = rename_Token_ਲੱਭੋ(&sParse, 0, (ਖਾਲੀ*)pTab->a_ਖਾਨਾ[i_ਖਾਨਾ+1].zCnName);
    z_ਸਿਰਾ = (ਅਚਲ ਅੱਖਰ*)p_ਸਿਰਾ->t.z;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ eTok;
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
    ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    p_ਖਾਨਾ = rename_Token_ਲੱਭੋ(&sParse, 0, (ਖਾਲੀ*)pTab->a_ਖਾਨਾ[i_ਖਾਨਾ-1].zCnName);
    ਕਰੋ {
      p_ਖਾਨਾ->t.z += getConstraintToken((ਅਚਲ u8*)p_ਖਾਨਾ->t.z, &eTok);
    }ਜਦੋਂ( eTok!=TK_COMMA );
    p_ਖਾਨਾ->t.z--;
    z_ਸਿਰਾ = (ਅਚਲ ਅੱਖਰ*)&zSql[pTab->u.tab.ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", p_ਖਾਨਾ->t.z-zSql, zSql, z_ਸਿਰਾ);
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_ਕਾਲਮ_ਪੂਰਾ:
  renameParseCleanup(&sParse);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#ਜੇ_ਅੰਤ
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
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
ਖਾਲੀ sqlite3_Alter_Drop_ਕਾਲਮ(Parse *pParse, SrcList *pSrc, ਅਚਲ Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  ਸਾਰਣੀ *pTab;                    /* Table to modify */
  ਪੂਰਨ iDb;                        /* Index of db containing pTab in aDb[] */
  ਅਚਲ ਅੱਖਰ *zDb;                /* Database containing pTab ("main" etc.) */
  ਅੱਖਰ *z_ਖਾਨਾ = 0;                 /* Name of column to drop */
  ਪੂਰਨ i_ਖਾਨਾ;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->p_New_ਸਾਰਣੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  ਜੇ( NEVER(db->mallocFailed) ) ਜਾਓ exit_drop_ਕਾਲਮ;
  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, &pSrc->a[0]);
  ਜੇ( !pTab ) ਜਾਓ exit_drop_ਕਾਲਮ;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  ਜੇ( SQLITE_OK!=is_Alterable_ਸਾਰਣੀ(pParse, pTab) ) ਜਾਓ exit_drop_ਕਾਲਮ;
  ਜੇ( SQLITE_OK!=is_Real_ਸਾਰਣੀ(pParse, pTab, 1) ) ਜਾਓ exit_drop_ਕਾਲਮ;

  /* Find the index of the column being dropped. */
  z_ਖਾਨਾ = sqlite3NameFromToken(db, pName);
  ਜੇ( z_ਖਾਨਾ==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    ਜਾਓ exit_drop_ਕਾਲਮ;
  }
  i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, z_ਖਾਨਾ);
  ਜੇ( i_ਖਾਨਾ<0 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such column: \"%T\"", pName);
    ਜਾਓ exit_drop_ਕਾਲਮ;
  }
  ਜੇ( isRowidAlias(pParse, pTab, z_ਖਾਨਾ, "drop") ) ਜਾਓ exit_drop_ਕਾਲਮ;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  ਜੇ( pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot drop %s column: \"%s\"",
        (pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        z_ਖਾਨਾ
    );
    ਜਾਓ exit_drop_ਕਾਲਮ;
  }

  /* Do not allow the number of columns to go to zero */
  ਜੇ( pTab->n_ਖਾਨਾ<=1 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot drop column \"%s\": no other columns exist",z_ਖਾਨਾ);
    ਜਾਓ exit_drop_ਕਾਲਮ;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, zDb, pTab->zName, z_ਖਾਨਾ) ){
    ਜਾਓ exit_drop_ਕਾਲਮ;
  }
#ਜੇ_ਅੰਤ
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, i_ਖਾਨਾ, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  ਜੇ( pParse->nErr==0 && (pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)==0 ){
    ਪੂਰਨ i;
    ਪੂਰਨ addr;
    ਪੂਰਨ reg;
    ਪੂਰਨ regRec;
    ਸੂਚਕਾਂਕ *pPk = 0;
    ਪੂਰਨ n_ਖੇਤਰ = 0;               /* Number of non-virtual columns after drop */
    ਪੂਰਨ iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3_Open_ਸਾਰਣੀ(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    ਜੇ( HasRowid(pTab) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->n_ਖਾਨਾ;
    }ਨਹੀਂਤਾਂ{
      pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      pParse->nMem += pPk->n_ਕਾਲਮ;
      ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iCur, i, reg+i+1);
      }
      n_ਖੇਤਰ = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
    }
    regRec = ++pParse->nMem;
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      ਜੇ( i!=i_ਖਾਨਾ && (pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)==0 ){
        ਪੂਰਨ reg_ਬਾਹਰਲਾ;
        ਜੇ( pPk ){
          ਪੂਰਨ iPos = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pPk, i);
          ਪੂਰਨ i_ਖਾਨਾ_Pos = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pPk, i_ਖਾਨਾ);
          ਜੇ( iPos<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ ) ਜਾਰੀ;
          reg_ਬਾਹਰਲਾ = reg+1+iPos-(iPos>i_ਖਾਨਾ_Pos);
        }ਨਹੀਂਤਾਂ{
          reg_ਬਾਹਰਲਾ = reg+1+n_ਖੇਤਰ;
        }
        ਜੇ( i==pTab->iPKey ){
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਬਾਹਰਲਾ);
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, i, reg_ਬਾਹਰਲਾ);
          ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->a_ਖਾਨਾ[i].affinity==SQLITE_AFF_REAL );
          sqlite3_Vdbe_Delete_ਪੂਰਵ_Opcode(v, OP_RealAffinity);
        }
        n_ਖੇਤਰ++;
      }
    }
    ਜੇ( n_ਖੇਤਰ==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg+1);
      n_ਖੇਤਰ = 1;
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg+1, n_ਖੇਤਰ, regRec);
    ਜੇ( pPk ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iCur, regRec, reg+1, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iCur, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
  }

exit_drop_ਕਾਲਮ:
  sqlite3DbFree(db, z_ਖਾਨਾ);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
ਸਥਿਰ ਪੂਰਨ getWhitespace(ਅਚਲ u8 *z){
  ਪੂਰਨ nRet = 0;
  ਜਦੋਂ( 1 ){
    ਪੂਰਨ t = 0;
    ਪੂਰਨ n = sqlite3GetToken(&z[nRet], &t);
    ਜੇ( t!=TK_SPACE && t!=TK_COMMENT ) ਰੋਕੋ;
    nRet += n;
  }
  ਵਾਪਸ nRet;
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
ਸਥਿਰ ਪੂਰਨ getConstraint(ਅਚਲ u8 *z, ਪੂਰਨ eTok){
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ t = 0;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
  ਜੇ( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  ਜੇ( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#ਜੇ_ਅੰਤ

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  ਜਦੋਂ( 1 ){
    ਪੂਰਨ n = getConstraintToken(&z[iOff], &t);
    ਜੇ( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      ਰੋਕੋ;
    }
    iOff += n;
  }
  
  ਵਾਪਸ iOff;
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
ਸਥਿਰ ਪੂਰਨ quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  ਪੂਰਨ t,                 /* Token type */
  ਅਚਲ u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  ਪੂਰਨ nQuote,            /* Length of zQuote in bytes */
  ਅਚਲ u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  ਪੂਰਨ *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  ਅੱਖਰ *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  ਜੇ( t==TK_ILLEGAL ){
    *pRes = 1;
    ਵਾਪਸ SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  ਜੇ( zCopy==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(ctx);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ਅਚਲ ਅੱਖਰ*)zCopy, (ਅਚਲ ਅੱਖਰ*)zCmp);
  sqlite3_free(zCopy);
  ਵਾਪਸ SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ਸਥਿਰ ਪੂਰਨ skip_Create_ਸਾਰਣੀ(sqlite3_context *ctx, ਅਚਲ u8 *zSql, ਪੂਰਨ *piOff){
  ਪੂਰਨ iOff = 0;

  ਜੇ( zSql==0 ) ਵਾਪਸ SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  ਜਦੋਂ( 1 ){
    ਪੂਰਨ t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    ਜੇ( t==TK_LP ) ਰੋਕੋ;
    ਜੇ( t==TK_ILLEGAL ){
      sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(ctx, SQLITE_CORRUPT_BKPT);
      ਵਾਪਸ SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਖਾਲੀ dropConstraintFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ u8 *zSql = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅਚਲ u8 *zCons = 0;
  ਪੂਰਨ iNotNull = -1;
  ਪੂਰਨ ii;
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ i_ਸ਼ੁਰੂ = 0;
  ਪੂਰਨ i_ਸਿਰਾ = 0;
  ਅੱਖਰ *zNew = 0;
  ਪੂਰਨ t = 0;
  sqlite3 *db;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  ਜੇ( zSql==0 ) ਵਾਪਸ;

  /* Jump past the "CREATE TABLE" bit. */
  ਜੇ( skip_Create_ਸਾਰਣੀ(ctx, zSql, &iOff) ) ਵਾਪਸ;

  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_ਮੁੱਲ_int(argv[1]);
  }ਨਹੀਂਤਾਂ{
    zCons = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  ਲਈ(ii=0; i_ਸਿਰਾ==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    ਜਦੋਂ( 1 ){
      i_ਸ਼ੁਰੂ = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      ਜੇ( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        ਪੂਰਨ nTok = 0;
        ਪੂਰਨ cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        ਜੇ( zCons ){
          ਜੇ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ਵਾਪਸ;
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
        ਜੇ( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }ਨਹੀਂਤਾਂ{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        ਜੇ( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          ਜੇ( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            ਵਾਪਸ;
          }
          i_ਸਿਰਾ = iOff;
          ਰੋਕੋ;
        }

      }ਨਹੀਂਤਾਂ ਜੇ( t==TK_NOT && iNotNull==ii ){
        i_ਸਿਰਾ = iOff + getConstraint(&zSql[iOff], 0);
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ ਜੇ( t==TK_RP || t==TK_ILLEGAL ){
        i_ਸਿਰਾ = -1;
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ ਜੇ( t==TK_COMMA ){
        ਰੋਕੋ;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  ਜੇ( i_ਸਿਰਾ<=0 ){
    ਜੇ( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }ਨਹੀਂਤਾਂ{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, (ਅਚਲ ਅੱਖਰ*)zSql, -1, SQLITE_TRANSIENT);
    }
  }ਨਹੀਂਤਾਂ{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ਅਚਲ ਅੱਖਰ *zSpace = " ";
    i_ਸਿਰਾ += getWhitespace(&zSql[i_ਸਿਰਾ]);
    sqlite3GetToken(&zSql[i_ਸਿਰਾ], &t);
    ਜੇ( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      ਜੇ( zSql[i_ਸ਼ੁਰੂ-1]==',' ) i_ਸ਼ੁਰੂ--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", i_ਸ਼ੁਰੂ, zSql, zSpace, &zSql[i_ਸਿਰਾ]);
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, zNew, -1, SQLITE_DYNAMIC);
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
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_Constraint_Func(
  sqlite3_context *ctx,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ u8 *zSql = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਅਚਲ ਅੱਖਰ *zCons = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
  ਪੂਰਨ i_ਖਾਨਾ = sqlite3_ਮੁੱਲ_int(argv[2]);
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ ii;
  sqlite3_str *pNew;
  ਪੂਰਨ t = 0;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);

  ਜੇ( skip_Create_ਸਾਰਣੀ(ctx, zSql, &iOff) ) ਵਾਪਸ;
  
  ਲਈ(ii=0; ii<=i_ਖਾਨਾ || (i_ਖਾਨਾ<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ਜਦੋਂ( 1 ){
      ਪੂਰਨ nTok = getConstraintToken(&zSql[iOff], &t);
      ਜੇ( t==TK_COMMA || t==TK_RP ) ਰੋਕੋ;
      ਜੇ( t==TK_ILLEGAL ){
        sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(ctx, SQLITE_CORRUPT_BKPT);
        ਵਾਪਸ;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ਅਚਲ ਅੱਖਰ*)zSql, iOff);
  ਜੇ( i_ਖਾਨਾ<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_ਨਤੀਜਾ_str(ctx, pNew, SQLITE_ਅੰਤ);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ਸਥਿਰ ਪੂਰਨ alter_ਲੱਭੋ_ਖਾਨਾ(Parse *pParse, ਸਾਰਣੀ *pTab, Token *p_ਖਾਨਾ, ਪੂਰਨ *pi_ਖਾਨਾ){
  sqlite3 *db = pParse->db;
  ਅੱਖਰ *zName = sqlite3NameFromToken(db, p_ਖਾਨਾ);
  ਪੂਰਨ rc = SQLITE_NOMEM;
  ਪੂਰਨ i_ਖਾਨਾ = -1;

  ਜੇ( zName ){
    i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, zName);
    ਜੇ( i_ਖਾਨਾ<0 ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_OK;
    }
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( rc==SQLITE_OK ){
    ਅਚਲ ਅੱਖਰ *zDb = db->aDb[sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema)].zDbSName;
    ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ = pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
    ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, zDb, pTab->zName, z_ਖਾਨਾ) ){
      pTab = 0;
    }
  }
#ਜੇ_ਅੰਤ

  sqlite3DbFree(db, zName);
  *pi_ਖਾਨਾ = i_ਖਾਨਾ;
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਸਾਰਣੀ *alter_ਲੱਭੋ_ਸਾਰਣੀ(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  ਪੂਰਨ *piDb,            /* OUT: write the iDb here */
  ਅਚਲ ਅੱਖਰ **pzDb,    /* OUT: write name of schema here */
  ਪੂਰਨ bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  ਸਾਰਣੀ *pTab = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, &pSrc->a[0]);
  ਜੇ( pTab ){
    ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    ਜੇ( SQLITE_OK!=is_Real_ਸਾਰਣੀ(pParse, pTab, 2) 
     || SQLITE_OK!=is_Alterable_ਸਾਰਣੀ(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( pTab && bAuth ){
    ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ALTER_ਸਾਰਣੀ, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#ਜੇ_ਅੰਤ
  sqlite3SrcListDelete(db, pSrc);
  ਵਾਪਸ pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
ਖਾਲੀ sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *p_ਖਾਨਾ       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  ਸਾਰਣੀ *pTab = 0;
  ਪੂਰਨ iDb = 0;
  ਅਚਲ ਅੱਖਰ *zDb = 0;
  ਅੱਖਰ *zArg = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਖਾਨਾ==0)!=(pCons==0) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->nSrc==1 );
  pTab = alter_ਲੱਭੋ_ਸਾਰਣੀ(pParse, pSrc, &iDb, &zDb, pCons!=0);
  ਜੇ( !pTab ) ਵਾਪਸ;

  ਜੇ( pCons ){
    ਅੱਖਰ *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i_ਖਾਨਾ;
    ਜੇ( alter_ਲੱਭੋ_ਖਾਨਾ(pParse, pTab, p_ਖਾਨਾ, &i_ਖਾਨਾ) ) ਵਾਪਸ;
    zArg = sqlite3MPrintf(db, "%d", i_ਖਾਨਾ);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
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
ਸਥਿਰ ਖਾਲੀ failConstraintFunc(
  sqlite3_context *ctx,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ ਅੱਖਰ *z_ਪਾਠ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  ਪੂਰਨ err = sqlite3_ਮੁੱਲ_int(argv[1]);
  (ਖਾਲੀ)Not_ਵਰਤਿਆ;
  sqlite3_ਨਤੀਜਾ_error(ctx, z_ਪਾਠ, -1);
  sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(ctx, err);
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
ਸਥਿਰ ਪੂਰਨ alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ਅਚਲ ਅੱਖਰ *pCons,              /* Buffer containing constraint */
  ਪੂਰਨ nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ i_ਸਿਰਾ = 0;

  ਜੇ( zTmp==0 ) ਵਾਪਸ 0;

  ਜਦੋਂ( 1 ){
    ਪੂਰਨ t = 0;
    ਪੂਰਨ nToken = sqlite3GetToken(&zTmp[iOff], &t);
    ਜੇ( t==TK_ILLEGAL ) ਰੋਕੋ;
    ਜੇ( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      i_ਸਿਰਾ = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  ਵਾਪਸ i_ਸਿਰਾ;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
ਖਾਲੀ sqlite3_Alter_ਸੈੱਟ_Not_Null(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *p_ਖਾਨਾ,     /* Name of the column to add a NOT NULL constraint to */
  Token *p_ਪਹਿਲਾ    /* The NOT token of the NOT NULL constraint text */
){
  ਸਾਰਣੀ *pTab = 0;
  ਪੂਰਨ i_ਖਾਨਾ = 0;
  ਪੂਰਨ iDb = 0;
  ਅਚਲ ਅੱਖਰ *zDb = 0;
  ਅਚਲ ਅੱਖਰ *pCons = 0;
  ਪੂਰਨ nCons = 0;

  /* Look up the table being altered. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->nSrc==1 );
  pTab = alter_ਲੱਭੋ_ਸਾਰਣੀ(pParse, pSrc, &iDb, &zDb, 0);
  ਜੇ( !pTab ) ਵਾਪਸ;

  /* Find the column being altered. */
  ਜੇ( alter_ਲੱਭੋ_ਖਾਨਾ(pParse, pTab, p_ਖਾਨਾ, &i_ਖਾਨਾ) ){
    ਵਾਪਸ;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = p_ਪਹਿਲਾ->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_ਆਖਰੀ_Token.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (ਪੂਰਨ)p_ਖਾਨਾ->n, p_ਖਾਨਾ->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, i_ਖਾਨਾ, nCons, pCons, i_ਖਾਨਾ, pTab->zName
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
ਸਥਿਰ ਖਾਲੀ ਲੱਭੋ_Constraint_Func(
  sqlite3_context *ctx,
  ਪੂਰਨ Not_ਵਰਤਿਆ,
  sqlite3_ਮੁੱਲ **argv
){
  ਅਚਲ u8 *zSql = 0;
  ਅਚਲ u8 *zCons = 0;
  ਪੂਰਨ iOff = 0;
  ਪੂਰਨ t = 0;

  (ਖਾਲੀ)Not_ਵਰਤਿਆ;
  zSql = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  zCons = sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);

  ਜੇ( zSql==0 || zCons==0 ) ਵਾਪਸ;
  ਜਦੋਂ( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  ਜਦੋਂ( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ਜੇ( t==TK_CONSTRAINT ){
      ਪੂਰਨ nTok = 0;
      ਪੂਰਨ cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      ਜੇ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ਵਾਪਸ;
      ਜੇ( cmp==0 ){
        sqlite3_ਨਤੀਜਾ_int(ctx, 1);
        ਵਾਪਸ;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( t==TK_ILLEGAL ){
      ਰੋਕੋ;
    }
  }

  sqlite3_ਨਤੀਜਾ_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
ਖਾਲੀ sqlite3_Alter_ਜੋੜੋ_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_ਪਹਿਲਾ,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ਅਚਲ ਅੱਖਰ *z_ਵਿਅੰਜਕ,       /* Text of CHECK expression */
  ਪੂਰਨ n_ਵਿਅੰਜਕ,               /* Size of pExpr in bytes */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ              /* The parsed CHECK expression */
){ 
  ਸਾਰਣੀ *pTab = 0;         /* Table identified by pSrc */
  ਪੂਰਨ iDb = 0;             /* Which schema does pTab live in */
  ਅਚਲ ਅੱਖਰ *zDb = 0;     /* Name of the schema in which pTab lives */
  ਅਚਲ ਅੱਖਰ *pCons = 0;   /* Text of the constraint */
  ਪੂਰਨ nCons;               /* Bytes of text to use from pCons[] */
  ਪੂਰਨ rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->nSrc==1 );
  pTab = alter_ਲੱਭੋ_ਸਾਰਣੀ(pParse, pSrc, &iDb, &zDb, 1);
  ਜੇ( !pTab ){
    sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਵਿਅੰਜਕ);
    ਵਾਪਸ;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_ਵਿਅੰਜਕ, 0);
  sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਵਿਅੰਜਕ);
  ਜੇ( rc ) ਵਾਪਸ;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  ਜੇ( pName ){
    ਅੱਖਰ *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " "
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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_ਵਿਅੰਜਕ, z_ਵਿਅੰਜਕ
  );

  /* Edit the SQL for the named table. */
  pCons = p_ਪਹਿਲਾ->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_ਆਖਰੀ_Token.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_ਸਾਰਣੀ " SET "
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
ਖਾਲੀ sqlite3AlterFunctions(ਖਾਲੀ){
  ਸਥਿਰ FuncDef a_Alter_ਸਾਰਣੀ_Funcs[] = {
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_rename_ਕਾਲਮ,  9, rename_ਕਾਲਮ_Func),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_rename_ਸਾਰਣੀ,   7, rename_ਸਾਰਣੀ_Func),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_rename_test,    7, rename_ਸਾਰਣੀ_Test),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_drop_ਕਾਲਮ,    3, drop_ਕਾਲਮ_Func),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_ਜੋੜੋ_constraint, 3, ਜੋੜੋ_Constraint_Func),
    ਅੰਦਰੂਨੀ_FUNCTION(sqlite_ਲੱਭੋ_constraint,2, ਲੱਭੋ_Constraint_Func),
  };
  sqlite3_ਪਾਓ_Builtin_Funcs(a_Alter_ਸਾਰਣੀ_Funcs, Array_ਮਾਪ(a_Alter_ਸਾਰਣੀ_Funcs));
}
#ਜੇ_ਅੰਤ  /* SQLITE_ALTER_TABLE */
