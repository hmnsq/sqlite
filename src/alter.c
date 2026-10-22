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
#अन्तर्भाव "sqliteInt.h"

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
स्थिर पूर्णाङ्क isAlterableTable(Parse *pParse, Table *pTab){
  सचे( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_सन्देशः(pParse, "table %s may not be altered", pTab->zName);
    पटिनिवत्त 1;
  }
  पटिनिवत्त 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
स्थिर शून्य renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ध्रुव वर्ण *zDb,                /* Name of db to verify schema of */
  पूर्णाङ्क bTemp,                      /* True if this is the temp db */
  ध्रुव वर्ण *zWhen,              /* "when" part of error message */
  पूर्णाङ्क bNoDQS                      /* Do not allow DQS in the schema */
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

  सचे( bTemp==0 ){
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
स्थिर शून्य renameFixQuotes(Parse *pParse, ध्रुव वर्ण *zDb, पूर्णाङ्क bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  सचे( bTemp==0 ){
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
स्थिर शून्य renameReloadSchema(Parse *pParse, पूर्णाङ्क iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  सचे( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    सचे( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
शून्य sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  पूर्णाङ्क iDb;                  /* Database that contains the table */
  वर्ण *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  वर्ण *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  पूर्णाङ्क nTabName;             /* Number of UTF-8 characters in zTabName */
  ध्रुव वर्ण *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  सचे( NEVER(db->mallocFailed) ) गच्छ exit_rename_table;
  प्रतिज्ञा( pSrc->nSrc==1 );
  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  सचे( !pTab ) गच्छ exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  सचे( !zName ) गच्छ exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  सचे( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_सन्देशः(pParse,
        "there is already another table or index with this name: %s", zName);
    गच्छ exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  सचे( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    गच्छ exit_rename_table;
  }
  सचे( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    गच्छ exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  सचे( IsView(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "view %s may not be altered", pTab->zName);
    गच्छ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    गच्छ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  सचे( sqlite3ViewGetColumnNames(pParse, pTab) ){
    गच्छ exit_rename_table;
  }
  सचे( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    सचे( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  सचे( v==0 ){
    गच्छ exit_rename_table;
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
  सचे( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  सचे( iDb!=1 ){
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
  सचे( pVTab ){
    पूर्णाङ्क i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(ध्रुव वर्ण*)pVTab, P4_VTAB);
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
स्थिर शून्य sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  ध्रुव वर्ण *zDb,      /* Schema holding the table */
  ध्रुव वर्ण *zTab,     /* Table to check for empty */
  ध्रुव वर्ण *zErr      /* Error message text */
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
स्थिर पूर्णाङ्क isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  ध्रुव वर्ण *zCol, 
  ध्रुव वर्ण *zOp
){
  सचे( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_सन्देशः(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    पटिनिवत्त SQLITE_ERROR;
  }
  पटिनिवत्त SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
शून्य sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  पूर्णाङ्क iDb;                  /* Database number */
  ध्रुव वर्ण *zDb;          /* Database name */
  ध्रुव वर्ण *zTab;         /* Table name */
  वर्ण *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  व्यञ्जकम् *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  पूर्णाङ्क r1;                   /* Temporary registers */

  db = pParse->db;
  प्रतिज्ञा( db->pParse==pParse );
  सचे( pParse->nErr ) पटिनिवत्त;
  प्रतिज्ञा( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  प्रतिज्ञा( pNew );

  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_व्यञ्जकम्(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  प्रतिज्ञा( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    पटिनिवत्त;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  सचे( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_सन्देशः(pParse, "Cannot add a PRIMARY KEY column");
    पटिनिवत्त;
  }
  सचे( pNew->pIndex ){
    sqlite3_Error_सन्देशः(pParse,
         "Cannot add a UNIQUE column");
    पटिनिवत्त;
  }
  सचे( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) पटिनिवत्त;
  सचे( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    प्रतिज्ञा( pDflt==0 || pDflt->op==TK_SPAN );
    सचे( pDflt && pDflt->p_वामः->op==TK_NULL ){
      pDflt = 0;
    }
    प्रतिज्ञा( IsOrdinaryTable(pNew) );
    सचे( (db->flags&SQLITE_Foreign_कुञ्चिकाः) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    सचे( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    सचे( pDflt ){
      sqlite3_मानम् *pVal = 0;
      पूर्णाङ्क rc;
      rc = sqlite3_मानम्_From_व्यञ्जकम्(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      प्रतिज्ञा( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      सचे( rc!=SQLITE_OK ){
        प्रतिज्ञा( db->mallocFailed == 1 );
        पटिनिवत्त;
      }
      सचे( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_मानम्_Free(pVal);
    }
  }अञ्ञथा सचे( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (वर्ण*)pColDef->z, pColDef->n);
  सचे( zCol ){
    वर्ण *zEnd = &zCol[pColDef->n-1];
    याव( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    प्रतिज्ञा( IsOrdinaryTable(pTab) );
    प्रतिज्ञा( IsOrdinaryTable(pNew) );
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
  सचे( v ){
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
    सचे( pNew->pCheck!=0
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
शून्य sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  पूर्णाङ्क iDb;
  पूर्णाङ्क i;
  पूर्णाङ्क nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  प्रतिज्ञा( pParse->pNewTable==0 );
  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  सचे( NEVER(db->mallocFailed) ) गच्छ exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  सचे( !pTab ) गच्छ exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  सचे( IsVirtual(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "virtual tables may not be altered");
    गच्छ exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  सचे( IsView(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "Cannot add a column to a view");
    गच्छ exit_begin_add_column;
  }
  सचे( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    गच्छ exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  प्रतिज्ञा( IsOrdinaryTable(pTab) );
  प्रतिज्ञा( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, परिमाण(Table));
  सचे( !pNew ) गच्छ exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  प्रतिज्ञा( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  प्रतिज्ञा( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, परिमाण(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  सचे( !pNew->aCol || !pNew->zName ){
    प्रतिज्ञा( db->mallocFailed );
    गच्छ exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, परिमाण(Column)*(size_t)pNew->nCol);
  अत्थाय(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  प्रतिज्ञा( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_व्यञ्जकम्_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  प्रतिज्ञा( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  पटिनिवत्त;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#सचे !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
स्थिर पूर्णाङ्क isRealTable(Parse *pParse, Table *pTab, पूर्णाङ्क iOp){
  ध्रुव वर्ण *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  सचे( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  सचे( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  सचे( zType ){
    ध्रुव वर्ण *az_सन्देशः[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    प्रतिज्ञा( iOp>=0 && iOp<ArraySize(az_सन्देशः) );
    sqlite3_Error_सन्देशः(pParse, "cannot %s %s \"%s\"",
        az_सन्देशः[iOp], zType, pTab->zName
    );
    पटिनिवत्त 1;
  }
  पटिनिवत्त 0;
}
#अञ्ञथा /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
शून्य sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  पूर्णाङ्क iCol;                       /* Index of column being renamed */
  वर्ण *zOld = 0;                 /* Old column name */
  वर्ण *zNew = 0;                 /* New column name */
  ध्रुव वर्ण *zDb;                /* Name of schema containing the table */
  पूर्णाङ्क iSchema;                    /* Index of the schema */
  पूर्णाङ्क bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  सचे( !pTab ) गच्छ exit_rename_column;

  /* Cannot alter a system table */
  सचे( SQLITE_OK!=isAlterableTable(pParse, pTab) ) गच्छ exit_rename_column;
  सचे( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) गच्छ exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  प्रतिज्ञा( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    गच्छ exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  सचे( !zOld ) गच्छ exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  सचे( iCol<0 ){
    sqlite3_Error_सन्देशः(pParse, "no such column: \"%T\"", pOld);
    गच्छ exit_rename_column;
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
  सचे( !zNew ) गच्छ exit_rename_column;
  सचे( isRowidAlias(pParse, pTab, zOld, "rename from") ) गच्छ exit_rename_column;
  सचे( isRowidAlias(pParse, pTab, zNew, "rename to") ) गच्छ exit_rename_column;
  प्रतिज्ञा( pNew->n>0 );
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
  सचे( iSchema!=1 ){
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
  पटिनिवत्त;
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
संरचना RenameToken {
  ध्रुव शून्य *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
प्रकारनिर्देश संरचना RenameCtx RenameCtx;
संरचना RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  पूर्णाङ्क nList;                      /* Number of tokens in pList */
  पूर्णाङ्क iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  ध्रुव वर्ण *zOld;               /* Old column name */
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
स्थिर शून्य renameTokenCheckAll(Parse *pParse, ध्रुव शून्य *pPtr){
  प्रतिज्ञा( pParse==pParse->db->pParse );
  प्रतिज्ञा( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  सचे( pParse->nErr==0 ){
    ध्रुव RenameToken *p;
    u32 i = 1;
    अत्थाय(p=pParse->pRename; p; p=p->pNext){
      सचे( p->p ){
        प्रतिज्ञा( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    प्रतिज्ञा( i>0 );
  }
}
#अञ्ञथा
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
ध्रुव शून्य *sqlite3RenameTokenMap(
  Parse *pParse,
  ध्रुव शून्य *pPtr,
  ध्रुव Token *pToken
){
  RenameToken *pNew;
  प्रतिज्ञा( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  सचे( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, परिमाण(RenameToken));
    सचे( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  पटिनिवत्त pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
शून्य sqlite3RenameTokenRemap(Parse *pParse, ध्रुव शून्य *pTo, ध्रुव शून्य *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  अत्थाय(p=pParse->pRename; p; p=p->pNext){
    सचे( p->p==pFrom ){
      p->p = pTo;
      भङ्ग;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
स्थिर पूर्णाङ्क rename_Unmap_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ध्रुव शून्य*)p_व्यञ्जकम्);
  सचे( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) ){
    sqlite3RenameTokenRemap(pParse, 0, (ध्रुव शून्य*)&p_व्यञ्जकम्->y.pTab);
  }
  पटिनिवत्त WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
स्थिर शून्य renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  सचे( pWith ){
    Parse *pParse = pWalker->pParse;
    पूर्णाङ्क i;
    With *pCopy = 0;
    प्रतिज्ञा( pWith->nCte>0 );
    सचे( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_प्रक्षिप(pParse, pCopy, 1);
    }
    अत्थाय(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, परिमाण(sNC));
      sNC.pParse = pParse;
      सचे( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      सचे( sNC.pParse->db->mallocFailed ) पटिनिवत्त;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    सचे( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
स्थिर शून्य unmapColumnIdlistNames(
  Parse *pParse,
  ध्रुव IdList *pIdList
){
  पूर्णाङ्क ii;
  प्रतिज्ञा( pIdList!=0 );
  अत्थाय(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ध्रुव शून्य*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
स्थिर पूर्णाङ्क renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  पूर्णाङ्क i;
  सचे( pParse->nErr ) पटिनिवत्त WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  सचे( p->selFlags & (SF_View|SF_CopyCte) ){
    पटिनिवत्त WRC_Prune;
  }
  सचे( ALWAYS(p->pEList) ){
    व्यञ्जकम्_List *pList = p->pEList;
    अत्थाय(i=0; i<pList->n_व्यञ्जकम्; i++){
      सचे( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (शून्य*)pList->a[i].zEName);
      }
    }
  }
  सचे( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    अत्थाय(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (शून्य*)pSrc->a[i].zName);
      सचे( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_व्यञ्जकम्(pWalker, pSrc->a[i].u3.pOn);
      }अञ्ञथा{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  पटिनिवत्त WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
शून्य sqlite3_Rename_व्यञ्जकम्_Unmap(Parse *pParse, व्यञ्जकम् *p_व्यञ्जकम्){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, परिमाण(Walker));
  sWalker.pParse = pParse;
  sWalker.x_व्यञ्जकम्_Callback = rename_Unmap_व्यञ्जकम्_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_व्यञ्जकम्(&sWalker, p_व्यञ्जकम्);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
शून्य sqlite3RenameExprlistUnmap(Parse *pParse, व्यञ्जकम्_List *pEList){
  सचे( pEList ){
    पूर्णाङ्क i;
    Walker sWalker;
    memset(&sWalker, 0, परिमाण(Walker));
    sWalker.pParse = pParse;
    sWalker.x_व्यञ्जकम्_Callback = rename_Unmap_व्यञ्जकम्_Cb;
    sqlite3_Walk_व्यञ्जकम्_List(&sWalker, pEList);
    अत्थाय(i=0; i<pEList->n_व्यञ्जकम्; i++){
      सचे( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (शून्य*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
स्थिर शून्य renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  अत्थाय(p=pToken; p; p=pNext){
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
स्थिर RenameToken *renameTokenFind(
  Parse *pParse,
  संरचना RenameCtx *pCtx,
  ध्रुव शून्य *pPtr
){
  RenameToken **pp;
  सचे( NEVER(pPtr==0) ){
    पटिनिवत्त 0;
  }
  अत्थाय(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    सचे( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      सचे( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      पटिनिवत्त pToken;
    }
  }
  पटिनिवत्त 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
स्थिर पूर्णाङ्क renameColumnSelectCb(Walker *pWalker, Select *p){
  सचे( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    पटिनिवत्त WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  पटिनिवत्त WRC_Continue;
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
स्थिर पूर्णाङ्क rename_Column_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  RenameCtx *p = pWalker->u.pRename;
  सचे( p_व्यञ्जकम्->op==TK_TRIGGER
   && p_व्यञ्जकम्->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (शून्य*)p_व्यञ्जकम्);
  }अञ्ञथा सचे( p_व्यञ्जकम्->op==TK_COLUMN
   && p_व्यञ्जकम्->iColumn==p->iCol
   && ALWAYS(व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्))
   && p->pTab==p_व्यञ्जकम्->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (शून्य*)p_व्यञ्जकम्);
  }
  पटिनिवत्त WRC_Continue;
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
स्थिर RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  अत्थाय(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    सचे( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  अत्थाय(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  पटिनिवत्त pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
स्थिर शून्य errorMPrintf(sqlite3_context *pCtx, ध्रुव वर्ण *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  वर्ण *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  सचे( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }अञ्ञथा{
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
स्थिर शून्य renameColumnParseError(
  sqlite3_context *pCtx,
  ध्रुव वर्ण *zWhen,
  sqlite3_मानम् *pType,
  sqlite3_मानम् *pObject,
  Parse *pParse
){
  ध्रुव वर्ण *zT = (ध्रुव वर्ण*)sqlite3_मानम्_text(pType);
  ध्रुव वर्ण *zN = (ध्रुव वर्ण*)sqlite3_मानम्_text(pObject);
  वर्ण *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_सन्देशः
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
स्थिर शून्य renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ध्रुव व्यञ्जकम्_List *pEList,
  ध्रुव वर्ण *zOld
){
  सचे( pEList ){
    पूर्णाङ्क i;
    अत्थाय(i=0; i<pEList->n_व्यञ्जकम्; i++){
      ध्रुव वर्ण *zName = pEList->a[i].zEName;
      सचे( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (ध्रुव शून्य*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
स्थिर शून्य renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ध्रुव IdList *pIdList,
  ध्रुव वर्ण *zOld
){
  सचे( pIdList ){
    पूर्णाङ्क i;
    अत्थाय(i=0; i<pIdList->nId; i++){
      ध्रुव वर्ण *zName = pIdList->a[i].zName;
      सचे( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (ध्रुव शून्य*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
स्थिर पूर्णाङ्क renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ध्रुव वर्ण *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ध्रुव वर्ण *zSql,               /* SQL to parse */
  पूर्णाङ्क bTemp                       /* True if SQL is from temp schema */
){
  पूर्णाङ्क rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  सचे( zSql==0 ){
    पटिनिवत्त SQLITE_NOMEM;
  }
  सचे( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    पटिनिवत्त SQLITE_CORRUPT_BKPT;
  }
  सचे( bTemp ){
    db->init.iDb = 1;
  }अञ्ञथा{
    पूर्णाङ्क iDb = sqlite3FindDbName(db, zDb);
    प्रतिज्ञा( iDb>=0 && iDb<=0xff );
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
  सचे( db->mallocFailed ) rc = SQLITE_NOMEM;
  सचे( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  सचे( rc==SQLITE_OK ){
    पूर्णाङ्क nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    अत्थाय(pToken=p->pRename; pToken; pToken=pToken->pNext){
      प्रतिज्ञा( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  पटिनिवत्त rc;
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
स्थिर पूर्णाङ्क renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ध्रुव वर्ण *zSql,               /* SQL statement to edit */
  ध्रुव वर्ण *zNew,               /* New token text */
  पूर्णाङ्क bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  पूर्णाङ्क rc = SQLITE_OK;
  वर्ण *zQuot = 0;
  वर्ण *zOut;
  i64 nQuot = 0;
  वर्ण *zBuf1 = 0;
  वर्ण *zBuf2 = 0;

  सचे( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    सचे( zQuot==0 ){
      पटिनिवत्त SQLITE_NOMEM;
    }अञ्ञथा{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    प्रतिज्ञा( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }अञ्ञथा{
    प्रतिज्ञा( nSql>0 );
    zOut = (वर्ण*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    सचे( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  सचे( zOut ){
    i64 nOut = nSql;
    प्रतिज्ञा( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    याव( pRename->pList ){
      पूर्णाङ्क iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ध्रुव वर्ण *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      सचे( zNew ){
        सचे( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }अञ्ञथा{
          nReplace = nQuot;
          zReplace = zQuot;
          सचे( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }अञ्ञथा{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        प्रतिज्ञा( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((पूर्णाङ्क)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (पूर्णाङ्क)(pBest->t.z - zSql);
      सचे( pBest->t.n!=nReplace ){
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
  }अञ्ञथा{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  पटिनिवत्त rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
स्थिर शून्य renameSetENames(व्यञ्जकम्_List *pEList, पूर्णाङ्क val){
  प्रतिज्ञा( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  सचे( pEList ){
    पूर्णाङ्क i;
    अत्थाय(i=0; i<pEList->n_व्यञ्जकम्; i++){
      प्रतिज्ञा( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
स्थिर पूर्णाङ्क renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  पूर्णाङ्क rc = SQLITE_OK;

  memset(&sNC, 0, परिमाण(sNC));
  sNC.pParse = pParse;
  प्रतिज्ञा( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  सचे( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  सचे( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_व्यञ्जकम्_Names(&sNC, pNew->pWhen);
  }

  अत्थाय(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    सचे( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      सचे( pParse->nErr ) rc = pParse->rc;
    }
    सचे( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      सचे( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_व्यञ्जकम्_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        सचे( pSel==0 ){
          pStep->p_व्यञ्जकम्_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }अञ्ञथा{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_व्यञ्जकम्_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_व्यञ्जकम्_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          प्रतिज्ञा( pStep->p_व्यञ्जकम्_List==0 || pStep->p_व्यञ्जकम्_List==pSel->pEList );
          प्रतिज्ञा( pSrc==pSel->pSrc );
          सचे( pStep->p_व्यञ्जकम्_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        सचे( ALWAYS(pStep->pSrc) ){
          पूर्णाङ्क i;
          अत्थाय(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            सचे( p->fg.isSubquery ){
              प्रतिज्ञा( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        सचे(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        सचे( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_व्यञ्जकम्_Names(&sNC, pStep->pWhere);
        }
        सचे( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_व्यञ्जकम्_List_Names(&sNC, pStep->p_व्यञ्जकम्_List);
        }
        प्रतिज्ञा( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_व्यञ्जकम्_List) );
        सचे( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_व्यञ्जकम्_List_Names(&sNC, pUpsert->pUpsertTarget);
          सचे( rc==SQLITE_OK ){
            व्यञ्जकम्_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_व्यञ्जकम्_List_Names(&sNC, pUpsertSet);
          }
          सचे( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_व्यञ्जकम्_Names(&sNC, pUpsert->pUpsertWhere);
          }
          सचे( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_व्यञ्जकम्_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }अञ्ञथा{
        rc = SQLITE_NOMEM;
      }
    }
  }
  पटिनिवत्त rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
स्थिर शून्य renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_व्यञ्जकम्(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  अत्थाय(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_व्यञ्जकम्(pWalker, pStep->pWhere);
    sqlite3_Walk_व्यञ्जकम्_List(pWalker, pStep->p_व्यञ्जकम्_List);
    सचे( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_व्यञ्जकम्_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_व्यञ्जकम्_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_व्यञ्जकम्(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_व्यञ्जकम्(pWalker, pUpsert->pUpsertTargetWhere);
    }
    सचे( pStep->pSrc ){
      पूर्णाङ्क i;
      SrcList *pSrc = pStep->pSrc;
      अत्थाय(i=0; i<pSrc->nSrc; i++){
        सचे( pSrc->a[i].fg.isSubquery ){
          प्रतिज्ञा( pSrc->a[i].u4.pSubq!=0 );
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
स्थिर शून्य renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  सचे( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  याव( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_सन्देशः);
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
स्थिर शून्य renameColumnFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ध्रुव वर्ण *zSql = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  ध्रुव वर्ण *zDb = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[3]);
  ध्रुव वर्ण *zTable = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[4]);
  पूर्णाङ्क iCol = sqlite3_मानम्_int(argv[5]);
  ध्रुव वर्ण *zNew = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[6]);
  पूर्णाङ्क bQuote = sqlite3_मानम्_int(argv[7]);
  पूर्णाङ्क bTemp = sqlite3_मानम्_int(argv[8]);
  ध्रुव वर्ण *zOld;
  पूर्णाङ्क rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  पूर्णाङ्क i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  सचे( zSql==0 ) पटिनिवत्त;
  सचे( zTable==0 ) पटिनिवत्त;
  सचे( zNew==0 ) पटिनिवत्त;
  सचे( iCol<0 ) पटिनिवत्त;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  सचे( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    पटिनिवत्त;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, परिमाण(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, परिमाण(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_व्यञ्जकम्_Callback = rename_Column_व्यञ्जकम्_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  सचे( rc!=SQLITE_OK ) गच्छ renameColumnFunc_done;
  सचे( sParse.pNewTable ){
    सचे( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      सचे( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      सचे( rc!=SQLITE_OK ) गच्छ renameColumnFunc_done;
    }अञ्ञथा सचे( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      पूर्णाङ्क bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      सचे( bFKOnly==0 ){
        सचे( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (शून्य*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        सचे( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (शून्य*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_व्यञ्जकम्_List(&sWalker, sParse.pNewTable->pCheck);
        अत्थाय(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_व्यञ्जकम्_List(&sWalker, pIdx->a_Col_व्यञ्जकम्);
        }
        अत्थाय(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_व्यञ्जकम्_List(&sWalker, pIdx->a_Col_व्यञ्जकम्);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        अत्थाय(i=0; i<sParse.pNewTable->nCol; i++){
          व्यञ्जकम् *p_व्यञ्जकम् = sqlite3_Column_व्यञ्जकम्(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_व्यञ्जकम्(&sWalker, p_व्यञ्जकम्);
        }
#endif
      }

      प्रतिज्ञा( IsOrdinaryTable(sParse.pNewTable) );
      अत्थाय(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        अत्थाय(i=0; i<pFKey->nCol; i++){
          सचे( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (शून्य*)&pFKey->aCol[i]);
          }
          सचे( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (शून्य*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }अञ्ञथा सचे( sParse.pNewIndex ){
    sqlite3_Walk_व्यञ्जकम्_List(&sWalker, sParse.pNewIndex->a_Col_व्यञ्जकम्);
    sqlite3_Walk_व्यञ्जकम्(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }अञ्ञथा{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    सचे( rc!=SQLITE_OK ) गच्छ renameColumnFunc_done;

    अत्थाय(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      सचे( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        सचे( pTarget==pTab ){
          सचे( pStep->pUpsert ){
            व्यञ्जकम्_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_व्यञ्जकम्_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    सचे( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  प्रतिज्ञा( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  सचे( rc!=SQLITE_OK ){
    सचे( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_मानम्(context, argv[0]);
    }अञ्ञथा सचे( sParse.z_Err_सन्देशः ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }अञ्ञथा{
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
स्थिर पूर्णाङ्क rename_Table_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  RenameCtx *p = pWalker->u.pRename;
  सचे( p_व्यञ्जकम्->op==TK_COLUMN
   && ALWAYS(व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्))
   && p->pTab==p_व्यञ्जकम्->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (शून्य*)&p_व्यञ्जकम्->y.pTab);
  }
  पटिनिवत्त WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
स्थिर पूर्णाङ्क renameTableSelectCb(Walker *pWalker, Select *pSelect){
  पूर्णाङ्क i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  सचे( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    पटिनिवत्त WRC_Prune;
  }
  सचे( NEVER(pSrc==0) ){
    प्रतिज्ञा( pWalker->pParse->db->mallocFailed );
    पटिनिवत्त WRC_Abort;
  }
  अत्थाय(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    सचे( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  पटिनिवत्त WRC_Continue;
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
स्थिर शून्य renameTableFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ध्रुव वर्ण *zDb = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  ध्रुव वर्ण *z_निवेशः = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[3]);
  ध्रुव वर्ण *zOld = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[4]);
  ध्रुव वर्ण *zNew = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[5]);
  पूर्णाङ्क bTemp = sqlite3_मानम्_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  सचे( z_निवेशः && zOld && zNew ){
    Parse sParse;
    पूर्णाङ्क rc;
    पूर्णाङ्क bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, परिमाण(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, परिमाण(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_व्यञ्जकम्_Callback = rename_Table_व्यञ्जकम्_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_निवेशः, bTemp);

    सचे( rc==SQLITE_OK ){
      पूर्णाङ्क isLegacy = (db->flags & SQLITE_LegacyAlter);
      सचे( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        सचे( IsView(pTab) ){
          सचे( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, परिमाण(sNC));
            sNC.pParse = &sParse;

            प्रतिज्ञा( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            सचे( sParse.nErr ){
              rc = sParse.rc;
            }अञ्ञथा{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }अञ्ञथा{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_कुञ्चिका
          सचे( (isLegacy==0 || (db->flags & SQLITE_Foreign_कुञ्चिकाः))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            प्रतिज्ञा( IsOrdinaryTable(pTab) );
            अत्थाय(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              सचे( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (शून्य*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          सचे( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            सचे( isLegacy==0 ){
              sqlite3_Walk_व्यञ्जकम्_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      अञ्ञथा सचे( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        सचे( isLegacy==0 ){
          sqlite3_Walk_व्यञ्जकम्(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      अञ्ञथा{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        सचे( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        सचे( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          सचे( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            अत्थाय(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              सचे( pStep->pSrc ){
                पूर्णाङ्क i;
                अत्थाय(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  सचे( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    सचे( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_निवेशः, zNew, bQuote);
    }
    सचे( rc!=SQLITE_OK ){
      सचे( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_मानम्(context, argv[3]);
      }अञ्ञथा सचे( sParse.z_Err_सन्देशः ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }अञ्ञथा{
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

  पटिनिवत्त;
}

स्थिर पूर्णाङ्क rename_Quotefix_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  सचे( p_व्यञ्जकम्->op==TK_STRING && (p_व्यञ्जकम्->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (ध्रुव शून्य*)p_व्यञ्जकम्);
  }
  पटिनिवत्त WRC_Continue;
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
स्थिर शून्य renameQuotefixFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  वर्ण ध्रुव *zDb = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  वर्ण ध्रुव *z_निवेशः = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  सचे( zDb && z_निवेशः ){
    पूर्णाङ्क rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_निवेशः, 0);

    सचे( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, परिमाण(RenameCtx));
      memset(&sWalker, 0, परिमाण(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_व्यञ्जकम्_Callback = rename_Quotefix_व्यञ्जकम्_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      सचे( sParse.pNewTable ){
        सचे( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          सचे( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }अञ्ञथा{
          पूर्णाङ्क i;
          sqlite3_Walk_व्यञ्जकम्_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          अत्थाय(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_व्यञ्जकम्(&sWalker,
               sqlite3_Column_व्यञ्जकम्(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }अञ्ञथा सचे( sParse.pNewIndex ){
        sqlite3_Walk_व्यञ्जकम्_List(&sWalker, sParse.pNewIndex->a_Col_व्यञ्जकम्);
        sqlite3_Walk_व्यञ्जकम्(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }अञ्ञथा{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        सचे( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      सचे( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_निवेशः, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    सचे( rc!=SQLITE_OK ){
      सचे( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_मानम्(context, argv[1]);
      }अञ्ञथा{
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
स्थिर शून्य renameTableTest(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  वर्ण ध्रुव *zDb = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  वर्ण ध्रुव *z_निवेशः = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[1]);
  पूर्णाङ्क bTemp = sqlite3_मानम्_int(argv[4]);
  पूर्णाङ्क isLegacy = (db->flags & SQLITE_LegacyAlter);
  वर्ण ध्रुव *zWhen = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[5]);
  पूर्णाङ्क bNoDQS = sqlite3_मानम्_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  सचे( zDb && z_निवेशः ){
    पूर्णाङ्क rc;
    Parse sParse;
    u64 flags = db->flags;
    सचे( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_निवेशः, bTemp);
    db->flags = flags;
    सचे( rc==SQLITE_OK ){
      सचे( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, परिमाण(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        सचे( sParse.nErr ) rc = sParse.rc;
      }

      अञ्ञथा सचे( sParse.pNewTrigger ){
        सचे( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        सचे( rc==SQLITE_OK ){
          पूर्णाङ्क i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          पूर्णाङ्क i2 = sqlite3FindDbName(db, zDb);
          सचे( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    सचे( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
स्थिर पूर्णाङ्क getConstraintToken(ध्रुव u8 *z, पूर्णाङ्क *piToken){
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क t = 0;
  करोहि {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }याव( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  सचे( t==TK_LP ){
    पूर्णाङ्क nNest = 1;
    याव( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      सचे( t==TK_LP ){
        nNest++;
      }अञ्ञथा सचे( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }अञ्ञथा सचे( t==TK_ILLEGAL ){
        भङ्ग;
      }
    }
  }

  *piToken = t;
  पटिनिवत्त iOff;
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
स्थिर शून्य dropColumnFunc(
  sqlite3_context *context,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णाङ्क iSchema = sqlite3_मानम्_int(argv[0]);
  ध्रुव वर्ण *zSql = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[1]);
  पूर्णाङ्क iCol = sqlite3_मानम्_int(argv[2]);
  ध्रुव वर्ण *zDb = db->aDb[iSchema].zDbSName;
  पूर्णाङ्क rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  ध्रुव वर्ण *zEnd;
  वर्ण *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  सचे( rc!=SQLITE_OK ) गच्छ drop_column_done;
  pTab = sParse.pNewTable;
  सचे( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    गच्छ drop_column_done;
  }

  सचे( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (शून्य*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (शून्य*)pTab->aCol[iCol+1].zCnName);
    zEnd = (ध्रुव वर्ण*)pEnd->t.z;
  }अञ्ञथा{
    पूर्णाङ्क eTok;
    प्रतिज्ञा( IsOrdinaryTable(pTab) );
    प्रतिज्ञा( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (शून्य*)pTab->aCol[iCol-1].zCnName);
    करोहि {
      pCol->t.z += getConstraintToken((ध्रुव u8*)pCol->t.z, &eTok);
    }याव( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (ध्रुव वर्ण*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  सचे( rc!=SQLITE_OK ){
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
शून्य sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, ध्रुव Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  पूर्णाङ्क iDb;                        /* Index of db containing pTab in aDb[] */
  ध्रुव वर्ण *zDb;                /* Database containing pTab ("main" etc.) */
  वर्ण *zCol = 0;                 /* Name of column to drop */
  पूर्णाङ्क iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  प्रतिज्ञा( pParse->pNewTable==0 );
  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  सचे( NEVER(db->mallocFailed) ) गच्छ exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  सचे( !pTab ) गच्छ exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  सचे( SQLITE_OK!=isAlterableTable(pParse, pTab) ) गच्छ exit_drop_column;
  सचे( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) गच्छ exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  सचे( zCol==0 ){
    प्रतिज्ञा( db->mallocFailed );
    गच्छ exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  सचे( iCol<0 ){
    sqlite3_Error_सन्देशः(pParse, "no such column: \"%T\"", pName);
    गच्छ exit_drop_column;
  }
  सचे( isRowidAlias(pParse, pTab, zCol, "drop") ) गच्छ exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  सचे( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_सन्देशः(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    गच्छ exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  सचे( pTab->nCol<=1 ){
    sqlite3_Error_सन्देशः(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    गच्छ exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  प्रतिज्ञा( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    गच्छ exit_drop_column;
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
  सचे( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    पूर्णाङ्क i;
    पूर्णाङ्क addr;
    पूर्णाङ्क reg;
    पूर्णाङ्क regRec;
    Index *pPk = 0;
    पूर्णाङ्क nField = 0;               /* Number of non-virtual columns after drop */
    पूर्णाङ्क iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    सचे( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }अञ्ञथा{
      pPk = sqlite3_Primary_कुञ्चिका_Index(pTab);
      pParse->nMem += pPk->nColumn;
      अत्थाय(i=0; i<pPk->n_कुञ्चिका_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_कुञ्चिका_Col;
    }
    regRec = ++pParse->nMem;
    अत्थाय(i=0; i<pTab->nCol; i++){
      सचे( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        पूर्णाङ्क regOut;
        सचे( pPk ){
          पूर्णाङ्क iPos = sqlite3TableColumnToIndex(pPk, i);
          पूर्णाङ्क iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          सचे( iPos<pPk->n_कुञ्चिका_Col ) अनुवर्त;
          regOut = reg+1+iPos-(iPos>iColPos);
        }अञ्ञथा{
          regOut = reg+1+nField;
        }
        सचे( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }अञ्ञथा{
          sqlite3_व्यञ्जकम्_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          प्रतिज्ञा( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    सचे( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    सचे( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_निवेशय, iCur, regRec, reg+1, pPk->n_कुञ्चिका_Col);
    }अञ्ञथा{
      sqlite3VdbeAddOp3(v, OP_निवेशय, iCur, regRec, reg);
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
स्थिर पूर्णाङ्क getWhitespace(ध्रुव u8 *z){
  पूर्णाङ्क nRet = 0;
  याव( 1 ){
    पूर्णाङ्क t = 0;
    पूर्णाङ्क n = sqlite3GetToken(&z[nRet], &t);
    सचे( t!=TK_SPACE && t!=TK_COMMENT ) भङ्ग;
    nRet += n;
  }
  पटिनिवत्त nRet;
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
स्थिर पूर्णाङ्क getConstraint(ध्रुव u8 *z, पूर्णाङ्क eTok){
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क t = 0;

#ifndef SQLITE_OMIT_FOREIGN_कुञ्चिका
  सचे( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  सचे( eTok==TK_REFERENCES ){
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
  याव( 1 ){
    पूर्णाङ्क n = getConstraintToken(&z[iOff], &t);
    सचे( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      भङ्ग;
    }
    iOff += n;
  }
  
  पटिनिवत्त iOff;
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
स्थिर पूर्णाङ्क quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  पूर्णाङ्क t,                 /* Token type */
  ध्रुव u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  पूर्णाङ्क nQuote,            /* Length of zQuote in bytes */
  ध्रुव u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  पूर्णाङ्क *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  वर्ण *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  सचे( t==TK_ILLEGAL ){
    *pRes = 1;
    पटिनिवत्त SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  सचे( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    पटिनिवत्त SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ध्रुव वर्ण*)zCopy, (ध्रुव वर्ण*)zCmp);
  sqlite3_free(zCopy);
  पटिनिवत्त SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
स्थिर पूर्णाङ्क skipCreateTable(sqlite3_context *ctx, ध्रुव u8 *zSql, पूर्णाङ्क *piOff){
  पूर्णाङ्क iOff = 0;

  सचे( zSql==0 ) पटिनिवत्त SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  याव( 1 ){
    पूर्णाङ्क t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    सचे( t==TK_LP ) भङ्ग;
    सचे( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      पटिनिवत्त SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  पटिनिवत्त SQLITE_OK;
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
स्थिर शून्य dropConstraintFunc(
  sqlite3_context *ctx,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  ध्रुव u8 *zSql = sqlite3_मानम्_text(argv[0]);
  ध्रुव u8 *zCons = 0;
  पूर्णाङ्क iNotNull = -1;
  पूर्णाङ्क ii;
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क iStart = 0;
  पूर्णाङ्क iEnd = 0;
  वर्ण *zNew = 0;
  पूर्णाङ्क t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  सचे( zSql==0 ) पटिनिवत्त;

  /* Jump past the "CREATE TABLE" bit. */
  सचे( skipCreateTable(ctx, zSql, &iOff) ) पटिनिवत्त;

  सचे( sqlite3_मानम्_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_मानम्_int(argv[1]);
  }अञ्ञथा{
    zCons = sqlite3_मानम्_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  अत्थाय(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    याव( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      सचे( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        पूर्णाङ्क nTok = 0;
        पूर्णाङ्क cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        सचे( zCons ){
          सचे( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) पटिनिवत्त;
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
        सचे( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }अञ्ञथा{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        सचे( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          सचे( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            पटिनिवत्त;
          }
          iEnd = iOff;
          भङ्ग;
        }

      }अञ्ञथा सचे( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        भङ्ग;
      }अञ्ञथा सचे( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        भङ्ग;
      }अञ्ञथा सचे( t==TK_COMMA ){
        भङ्ग;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  सचे( iEnd<=0 ){
    सचे( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }अञ्ञथा{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (ध्रुव वर्ण*)zSql, -1, SQLITE_TRANSIENT);
    }
  }अञ्ञथा{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ध्रुव वर्ण *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    सचे( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      सचे( zSql[iStart-1]==',' ) iStart--;
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
स्थिर शून्य addConstraintFunc(
  sqlite3_context *ctx,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  ध्रुव u8 *zSql = sqlite3_मानम्_text(argv[0]);
  ध्रुव वर्ण *zCons = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[1]);
  पूर्णाङ्क iCol = sqlite3_मानम्_int(argv[2]);
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क ii;
  sqlite3_str *pNew;
  पूर्णाङ्क t = 0;
  UNUSED_PARAMETER(NotUsed);

  सचे( skipCreateTable(ctx, zSql, &iOff) ) पटिनिवत्त;
  
  अत्थाय(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    याव( 1 ){
      पूर्णाङ्क nTok = getConstraintToken(&zSql[iOff], &t);
      सचे( t==TK_COMMA || t==TK_RP ) भङ्ग;
      सचे( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        पटिनिवत्त;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ध्रुव वर्ण*)zSql, iOff);
  सचे( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
स्थिर पूर्णाङ्क alterFindCol(Parse *pParse, Table *pTab, Token *pCol, पूर्णाङ्क *piCol){
  sqlite3 *db = pParse->db;
  वर्ण *zName = sqlite3NameFromToken(db, pCol);
  पूर्णाङ्क rc = SQLITE_NOMEM;
  पूर्णाङ्क iCol = -1;

  सचे( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    सचे( iCol<0 ){
      sqlite3_Error_सन्देशः(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }अञ्ञथा{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  सचे( rc==SQLITE_OK ){
    ध्रुव वर्ण *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    ध्रुव वर्ण *zCol = pTab->aCol[iCol].zCnName;
    सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  पटिनिवत्त rc;
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
स्थिर Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  पूर्णाङ्क *piDb,            /* OUT: write the iDb here */
  ध्रुव वर्ण **pzDb,    /* OUT: write name of schema here */
  पूर्णाङ्क bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  प्रतिज्ञा( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  सचे( pTab ){
    पूर्णाङ्क iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    सचे( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  सचे( pTab && bAuth ){
    सचे( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  पटिनिवत्त pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
शून्य sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  पूर्णाङ्क iDb = 0;
  ध्रुव वर्ण *zDb = 0;
  वर्ण *zArg = 0;

  प्रतिज्ञा( (pCol==0)!=(pCons==0) );
  प्रतिज्ञा( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  सचे( !pTab ) पटिनिवत्त;

  सचे( pCons ){
    वर्ण *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }अञ्ञथा{
    पूर्णाङ्क iCol;
    सचे( alterFindCol(pParse, pTab, pCol, &iCol) ) पटिनिवत्त;
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
स्थिर शून्य failConstraintFunc(
  sqlite3_context *ctx,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  ध्रुव वर्ण *zText = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  पूर्णाङ्क err = sqlite3_मानम्_int(argv[1]);
  (शून्य)NotUsed;
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
स्थिर पूर्णाङ्क alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ध्रुव वर्ण *pCons,              /* Buffer containing constraint */
  पूर्णाङ्क nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क iEnd = 0;

  सचे( zTmp==0 ) पटिनिवत्त 0;

  याव( 1 ){
    पूर्णाङ्क t = 0;
    पूर्णाङ्क nToken = sqlite3GetToken(&zTmp[iOff], &t);
    सचे( t==TK_ILLEGAL ) भङ्ग;
    सचे( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  पटिनिवत्त iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
शून्य sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  पूर्णाङ्क iCol = 0;
  पूर्णाङ्क iDb = 0;
  ध्रुव वर्ण *zDb = 0;
  ध्रुव वर्ण *pCons = 0;
  पूर्णाङ्क nCons = 0;

  /* Look up the table being altered. */
  प्रतिज्ञा( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  सचे( !pTab ) पटिनिवत्त;

  /* Find the column being altered. */
  सचे( alterFindCol(pParse, pTab, pCol, &iCol) ){
    पटिनिवत्त;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (पूर्णाङ्क)pCol->n, pCol->z
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
स्थिर शून्य findConstraintFunc(
  sqlite3_context *ctx,
  पूर्णाङ्क NotUsed,
  sqlite3_मानम् **argv
){
  ध्रुव u8 *zSql = 0;
  ध्रुव u8 *zCons = 0;
  पूर्णाङ्क iOff = 0;
  पूर्णाङ्क t = 0;

  (शून्य)NotUsed;
  zSql = sqlite3_मानम्_text(argv[0]);
  zCons = sqlite3_मानम्_text(argv[1]);

  सचे( zSql==0 || zCons==0 ) पटिनिवत्त;
  याव( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  याव( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    सचे( t==TK_CONSTRAINT ){
      पूर्णाङ्क nTok = 0;
      पूर्णाङ्क cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      सचे( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) पटिनिवत्त;
      सचे( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        पटिनिवत्त;
      }
    }अञ्ञथा सचे( t==TK_ILLEGAL ){
      भङ्ग;
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
शून्य sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ध्रुव वर्ण *z_व्यञ्जकम्,       /* Text of CHECK expression */
  पूर्णाङ्क n_व्यञ्जकम्,               /* Size of pExpr in bytes */
  व्यञ्जकम् *p_व्यञ्जकम्              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  पूर्णाङ्क iDb = 0;             /* Which schema does pTab live in */
  ध्रुव वर्ण *zDb = 0;     /* Name of the schema in which pTab lives */
  ध्रुव वर्ण *pCons = 0;   /* Text of the constraint */
  पूर्णाङ्क nCons;               /* Bytes of text to use from pCons[] */
  पूर्णाङ्क rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  प्रतिज्ञा( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  सचे( !pTab ){
    sqlite3_व्यञ्जकम्_Delete(pParse->db, p_व्यञ्जकम्);
    पटिनिवत्त;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_व्यञ्जकम्, 0);
  sqlite3_व्यञ्जकम्_Delete(pParse->db, p_व्यञ्जकम्);
  सचे( rc ) पटिनिवत्त;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  सचे( pName ){
    वर्ण *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_व्यञ्जकम्, z_व्यञ्जकम्
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
शून्य sqlite3AlterFunctions(शून्य){
  स्थिर FuncDef aAlterTableFuncs[] = {
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
  sqlite3_निवेशय_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
