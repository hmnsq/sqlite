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
#समावेश "sqliteInt.h"

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#अगर_अघोषित SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
अटल पूर्णांक isAlterableTable(Parse *pParse, Table *pTab){
  अगर( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#पूर्ण_अगर
  ){
    sqlite3_Error_संदेश(pParse, "table %s may not be altered", pTab->zName);
    वापस 1;
  }
  वापस 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
अटल व्योम renameTestSchema(
  Parse *pParse,                  /* Parse context */
  स्थिर अक्षर *zDb,                /* Name of db to verify schema of */
  पूर्णांक b_अस्थायी,                      /* True if this is the temp db */
  स्थिर अक्षर *zWhen,              /* "when" part of error message */
  पूर्णांक bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->colNamesSet = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_TABLE " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_अस्थायी, zWhen, bNoDQS
  );

  अगर( b_अस्थायी==0 ){
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
अटल व्योम renameFixQuotes(Parse *pParse, स्थिर अक्षर *zDb, पूर्णांक b_अस्थायी){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  अगर( b_अस्थायी==0 ){
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
अटल व्योम renameReloadSchema(Parse *pParse, पूर्णांक iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  अगर( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    अगर( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
व्योम sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  पूर्णांक iDb;                  /* Database that contains the table */
  अक्षर *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  अक्षर *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  पूर्णांक nTabName;             /* Number of UTF-8 characters in zTabName */
  स्थिर अक्षर *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  अगर( NEVER(db->mallocFailed) ) जाओ exit_rename_table;
  निश्चित( pSrc->nSrc==1 );
  निश्चित( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  अगर( !pTab ) जाओ exit_rename_table;
  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  अगर( !zName ) जाओ exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  अगर( sqlite3FindTable(db, zName, zDb)
   || sqlite3_Find_सूचकांक(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_संदेश(pParse,
        "there is already another table or index with this name: %s", zName);
    जाओ exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  अगर( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    जाओ exit_rename_table;
  }
  अगर( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    जाओ exit_rename_table;
  }

#अगर_अघोषित SQLITE_OMIT_VIEW
  अगर( IsView(pTab) ){
    sqlite3_Error_संदेश(pParse, "view %s may not be altered", pTab->zName);
    जाओ exit_rename_table;
  }
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    जाओ exit_rename_table;
  }
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( sqlite3ViewGetColumnNames(pParse, pTab) ){
    जाओ exit_rename_table;
  }
  अगर( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    अगर( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#पूर्ण_अगर

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  अगर( v==0 ){
    जाओ exit_rename_table;
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

#अगर_अघोषित SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  अगर( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#पूर्ण_अगर

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  अगर( iDb!=1 ){
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
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( pVTab ){
    पूर्णांक i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(स्थिर अक्षर*)pVTab, P4_VTAB);
  }
#पूर्ण_अगर

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
अटल व्योम sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  स्थिर अक्षर *zDb,      /* Schema holding the table */
  स्थिर अक्षर *zTab,     /* Table to check for empty */
  स्थिर अक्षर *zErr      /* Error message text */
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
अटल पूर्णांक isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  स्थिर अक्षर *zCol, 
  स्थिर अक्षर *zOp
){
  अगर( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_संदेश(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    वापस SQLITE_ERROR;
  }
  वापस SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
व्योम sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  पूर्णांक iDb;                  /* Database number */
  स्थिर अक्षर *zDb;          /* Database name */
  स्थिर अक्षर *zTab;         /* Table name */
  अक्षर *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  व्यंजक *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  पूर्णांक r1;                   /* Temporary registers */

  db = pParse->db;
  निश्चित( db->pParse==pParse );
  अगर( pParse->nErr ) वापस;
  निश्चित( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  निश्चित( pNew );

  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_To_सूचकांक(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_व्यंजक(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  निश्चित( pTab );

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    वापस;
  }
#पूर्ण_अगर


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  अगर( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_संदेश(pParse, "Cannot add a PRIMARY KEY column");
    वापस;
  }
  अगर( pNew->p_सूचकांक ){
    sqlite3_Error_संदेश(pParse,
         "Cannot add a UNIQUE column");
    वापस;
  }
  अगर( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) वापस;
  अगर( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    निश्चित( pDflt==0 || pDflt->op==TK_SPAN );
    अगर( pDflt && pDflt->p_बायाँ->op==TK_NULL ){
      pDflt = 0;
    }
    निश्चित( IsOrdinaryTable(pNew) );
    अगर( (db->flags&SQLITE_Foreign_कुंजियाँ) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    अगर( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    अगर( pDflt ){
      sqlite3_मूल्य *pVal = 0;
      पूर्णांक rc;
      rc = sqlite3_मूल्य_From_व्यंजक(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      निश्चित( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      अगर( rc!=SQLITE_OK ){
        निश्चित( db->mallocFailed == 1 );
        वापस;
      }
      अगर( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_मूल्य_Free(pVal);
    }
  }अन्यथा अगर( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (अक्षर*)pColDef->z, pColDef->n);
  अगर( zCol ){
    अक्षर *zEnd = &zCol[pColDef->n-1];
    जबतक( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    निश्चित( IsOrdinaryTable(pTab) );
    निश्चित( IsOrdinaryTable(pNew) );
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
  अगर( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_अस्थायी_Reg(pParse);
    sqlite3VdbeAddOp3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3VdbeUsesBtree(v, iDb);
    sqlite3VdbeAddOp2(v, OP_AddImm, r1, -2);
    sqlite3VdbeAddOp2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_अस्थायी_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_AlterAdd);

    /* Verify that constraints are still satisfied */
    अगर( pNew->pCheck!=0
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
व्योम sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  पूर्णांक iDb;
  पूर्णांक i;
  पूर्णांक nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  निश्चित( pParse->pNewTable==0 );
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  अगर( NEVER(db->mallocFailed) ) जाओ exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  अगर( !pTab ) जाओ exit_begin_add_column;

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( IsVirtual(pTab) ){
    sqlite3_Error_संदेश(pParse, "virtual tables may not be altered");
    जाओ exit_begin_add_column;
  }
#पूर्ण_अगर

  /* Make sure this is not an attempt to ALTER a view. */
  अगर( IsView(pTab) ){
    sqlite3_Error_संदेश(pParse, "Cannot add a column to a view");
    जाओ exit_begin_add_column;
  }
  अगर( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    जाओ exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  निश्चित( IsOrdinaryTable(pTab) );
  निश्चित( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, माप(Table));
  अगर( !pNew ) जाओ exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  निश्चित( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  निश्चित( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, माप(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  अगर( !pNew->aCol || !pNew->zName ){
    निश्चित( db->mallocFailed );
    जाओ exit_begin_add_column;
  }
  स_नकल(pNew->aCol, pTab->aCol, माप(Column)*(माप_प्रकार)pNew->nCol);
  क्रम(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  निश्चित( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_व्यंजक_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  निश्चित( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  वापस;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#अगर !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
अटल पूर्णांक isRealTable(Parse *pParse, Table *pTab, पूर्णांक iOp){
  स्थिर अक्षर *zType = 0;
#अगर_अघोषित SQLITE_OMIT_VIEW
  अगर( IsView(pTab) ){
    zType = "view";
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#पूर्ण_अगर
  अगर( zType ){
    स्थिर अक्षर *az_संदेश[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    निश्चित( iOp>=0 && iOp<Array_आकार(az_संदेश) );
    sqlite3_Error_संदेश(pParse, "cannot %s %s \"%s\"",
        az_संदेश[iOp], zType, pTab->zName
    );
    वापस 1;
  }
  वापस 0;
}
#अन्यथा /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#पूर्ण_अगर

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
व्योम sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  पूर्णांक iCol;                       /* Index of column being renamed */
  अक्षर *zOld = 0;                 /* Old column name */
  अक्षर *zNew = 0;                 /* New column name */
  स्थिर अक्षर *zDb;                /* Name of schema containing the table */
  पूर्णांक iSchema;                    /* Index of the schema */
  पूर्णांक bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  अगर( !pTab ) जाओ exit_rename_column;

  /* Cannot alter a system table */
  अगर( SQLITE_OK!=isAlterableTable(pParse, pTab) ) जाओ exit_rename_column;
  अगर( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) जाओ exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  निश्चित( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    जाओ exit_rename_column;
  }
#पूर्ण_अगर

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  अगर( !zOld ) जाओ exit_rename_column;
  iCol = sqlite3_Column_सूचकांक(pTab, zOld);
  अगर( iCol<0 ){
    sqlite3_Error_संदेश(pParse, "no such column: \"%T\"", pOld);
    जाओ exit_rename_column;
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
  अगर( !zNew ) जाओ exit_rename_column;
  अगर( isRowidAlias(pParse, pTab, zOld, "rename from") ) जाओ exit_rename_column;
  अगर( isRowidAlias(pParse, pTab, zNew, "rename to") ) जाओ exit_rename_column;
  निश्चित( pNew->n>0 );
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
  अगर( iSchema!=1 ){
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
  वापस;
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
काष्ठा RenameToken {
  स्थिर व्योम *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_अग्र;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
प्रकार काष्ठा RenameCtx RenameCtx;
काष्ठा RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  पूर्णांक nList;                      /* Number of tokens in pList */
  पूर्णांक iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  स्थिर अक्षर *zOld;               /* Old column name */
};

#अगर_घोषित SQLITE_DEBUG
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
अटल व्योम renameTokenCheckAll(Parse *pParse, स्थिर व्योम *pPtr){
  निश्चित( pParse==pParse->db->pParse );
  निश्चित( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  अगर( pParse->nErr==0 ){
    स्थिर RenameToken *p;
    u32 i = 1;
    क्रम(p=pParse->pRename; p; p=p->p_अग्र){
      अगर( p->p ){
        निश्चित( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    निश्चित( i>0 );
  }
}
#अन्यथा
# define renameTokenCheckAll(x,y)
#पूर्ण_अगर

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
स्थिर व्योम *sqlite3RenameTokenMap(
  Parse *pParse,
  स्थिर व्योम *pPtr,
  स्थिर Token *pToken
){
  RenameToken *pNew;
  निश्चित( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  अगर( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, माप(RenameToken));
    अगर( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_अग्र = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  वापस pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
व्योम sqlite3RenameTokenRemap(Parse *pParse, स्थिर व्योम *pTo, स्थिर व्योम *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  क्रम(p=pParse->pRename; p; p=p->p_अग्र){
    अगर( p->p==pFrom ){
      p->p = pTo;
      अवरोध;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
अटल पूर्णांक rename_Unmap_व्यंजक_Cb(Walker *pWalker, व्यंजक *p_व्यंजक){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (स्थिर व्योम*)p_व्यंजक);
  अगर( व्यंजक_Use_YTab(p_व्यंजक) ){
    sqlite3RenameTokenRemap(pParse, 0, (स्थिर व्योम*)&p_व्यंजक->y.pTab);
  }
  वापस WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
अटल व्योम renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  अगर( pWith ){
    Parse *pParse = pWalker->pParse;
    पूर्णांक i;
    With *pCopy = 0;
    निश्चित( pWith->nCte>0 );
    अगर( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_धकेलो(pParse, pCopy, 1);
    }
    क्रम(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      स_रखो(&sNC, 0, माप(sNC));
      sNC.pParse = pParse;
      अगर( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      अगर( sNC.pParse->db->mallocFailed ) वापस;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    अगर( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
अटल व्योम unmapColumnIdlistNames(
  Parse *pParse,
  स्थिर IdList *pIdList
){
  पूर्णांक ii;
  निश्चित( pIdList!=0 );
  क्रम(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (स्थिर व्योम*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
अटल पूर्णांक renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  पूर्णांक i;
  अगर( pParse->nErr ) वापस WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  अगर( p->selFlags & (SF_View|SF_CopyCte) ){
    वापस WRC_Prune;
  }
  अगर( ALWAYS(p->pEList) ){
    व्यंजक_List *pList = p->pEList;
    क्रम(i=0; i<pList->n_व्यंजक; i++){
      अगर( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (व्योम*)pList->a[i].zEName);
      }
    }
  }
  अगर( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    क्रम(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (व्योम*)pSrc->a[i].zName);
      अगर( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_व्यंजक(pWalker, pSrc->a[i].u3.pOn);
      }अन्यथा{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  वापस WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
व्योम sqlite3_Rename_व्यंजक_Unmap(Parse *pParse, व्यंजक *p_व्यंजक){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  स_रखो(&sWalker, 0, माप(Walker));
  sWalker.pParse = pParse;
  sWalker.x_व्यंजक_Callback = rename_Unmap_व्यंजक_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_व्यंजक(&sWalker, p_व्यंजक);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
व्योम sqlite3RenameExprlistUnmap(Parse *pParse, व्यंजक_List *pEList){
  अगर( pEList ){
    पूर्णांक i;
    Walker sWalker;
    स_रखो(&sWalker, 0, माप(Walker));
    sWalker.pParse = pParse;
    sWalker.x_व्यंजक_Callback = rename_Unmap_व्यंजक_Cb;
    sqlite3_Walk_व्यंजक_List(&sWalker, pEList);
    क्रम(i=0; i<pEList->n_व्यंजक; i++){
      अगर( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (व्योम*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
अटल व्योम renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_अग्र;
  RenameToken *p;
  क्रम(p=pToken; p; p=p_अग्र){
    p_अग्र = p->p_अग्र;
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
अटल RenameToken *renameTokenFind(
  Parse *pParse,
  काष्ठा RenameCtx *pCtx,
  स्थिर व्योम *pPtr
){
  RenameToken **pp;
  अगर( NEVER(pPtr==0) ){
    वापस 0;
  }
  क्रम(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_अग्र){
    अगर( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      अगर( pCtx ){
        *pp = pToken->p_अग्र;
        pToken->p_अग्र = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      वापस pToken;
    }
  }
  वापस 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
अटल पूर्णांक renameColumnSelectCb(Walker *pWalker, Select *p){
  अगर( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    वापस WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  वापस WRC_Continue;
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
अटल पूर्णांक rename_Column_व्यंजक_Cb(Walker *pWalker, व्यंजक *p_व्यंजक){
  RenameCtx *p = pWalker->u.pRename;
  अगर( p_व्यंजक->op==TK_TRIGGER
   && p_व्यंजक->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (व्योम*)p_व्यंजक);
  }अन्यथा अगर( p_व्यंजक->op==TK_COLUMN
   && p_व्यंजक->iColumn==p->iCol
   && ALWAYS(व्यंजक_Use_YTab(p_व्यंजक))
   && p->pTab==p_व्यंजक->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (व्योम*)p_व्यंजक);
  }
  वापस WRC_Continue;
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
अटल RenameToken *rename_Column_Token_अग्र(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  क्रम(pToken=pBest->p_अग्र; pToken; pToken=pToken->p_अग्र){
    अगर( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  क्रम(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->p_अग्र);
  *pp = pBest->p_अग्र;

  वापस pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
अटल व्योम errorMPrintf(sqlite3_context *pCtx, स्थिर अक्षर *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  अक्षर *zErr = 0;
  बहु_सूची ap;
  बहु_शुरू(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  बहु_पूर्ण(ap);
  अगर( zErr ){
    sqlite3_परिणाम_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }अन्यथा{
    sqlite3_परिणाम_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
अटल व्योम renameColumnParseError(
  sqlite3_context *pCtx,
  स्थिर अक्षर *zWhen,
  sqlite3_मूल्य *pType,
  sqlite3_मूल्य *pObject,
  Parse *pParse
){
  स्थिर अक्षर *zT = (स्थिर अक्षर*)sqlite3_मूल्य_text(pType);
  स्थिर अक्षर *zN = (स्थिर अक्षर*)sqlite3_मूल्य_text(pObject);
  अक्षर *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_संदेश
  );
  sqlite3_परिणाम_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
अटल व्योम renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  स्थिर व्यंजक_List *pEList,
  स्थिर अक्षर *zOld
){
  अगर( pEList ){
    पूर्णांक i;
    क्रम(i=0; i<pEList->n_व्यंजक; i++){
      स्थिर अक्षर *zName = pEList->a[i].zEName;
      अगर( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (स्थिर व्योम*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
अटल व्योम renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  स्थिर IdList *pIdList,
  स्थिर अक्षर *zOld
){
  अगर( pIdList ){
    पूर्णांक i;
    क्रम(i=0; i<pIdList->nId; i++){
      स्थिर अक्षर *zName = pIdList->a[i].zName;
      अगर( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (स्थिर व्योम*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
अटल पूर्णांक renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  स्थिर अक्षर *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  स्थिर अक्षर *zSql,               /* SQL to parse */
  पूर्णांक b_अस्थायी                       /* True if SQL is from temp schema */
){
  पूर्णांक rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  अगर( zSql==0 ){
    वापस SQLITE_NOMEM;
  }
  अगर( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    वापस SQLITE_CORRUPT_BKPT;
  }
  अगर( b_अस्थायी ){
    db->init.iDb = 1;
  }अन्यथा{
    पूर्णांक iDb = sqlite3FindDbName(db, zDb);
    निश्चित( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->eParseMode = PARSE_MODE_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && म_माला(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3RunParser(p, zSql);
  db->flags = flags;
  अगर( db->mallocFailed ) rc = SQLITE_NOMEM;
  अगर( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->p_New_सूचकांक==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#अगर_घोषित SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  अगर( rc==SQLITE_OK ){
    पूर्णांक nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    क्रम(pToken=p->pRename; pToken; pToken=pToken->p_अग्र){
      निश्चित( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#पूर्ण_अगर

  db->init.iDb = 0;
  वापस rc;
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
अटल पूर्णांक renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  स्थिर अक्षर *zSql,               /* SQL statement to edit */
  स्थिर अक्षर *zNew,               /* New token text */
  पूर्णांक bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  पूर्णांक rc = SQLITE_OK;
  अक्षर *zQuot = 0;
  अक्षर *zOut;
  i64 nQuot = 0;
  अक्षर *zBuf1 = 0;
  अक्षर *zBuf2 = 0;

  अगर( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    अगर( zQuot==0 ){
      वापस SQLITE_NOMEM;
    }अन्यथा{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    निश्चित( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }अन्यथा{
    निश्चित( nSql>0 );
    zOut = (अक्षर*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    अगर( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  अगर( zOut ){
    i64 nOut = nSql;
    निश्चित( nSql>0 );
    स_नकल(zOut, zSql, (माप_प्रकार)nSql);
    जबतक( pRename->pList ){
      पूर्णांक iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      स्थिर अक्षर *zReplace;
      RenameToken *pBest = rename_Column_Token_अग्र(pRename);

      अगर( zNew ){
        अगर( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }अन्यथा{
          nReplace = nQuot;
          zReplace = zQuot;
          अगर( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }अन्यथा{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        स_नकल(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        निश्चित( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((पूर्णांक)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (पूर्णांक)(pBest->t.z - zSql);
      अगर( pBest->t.n!=nReplace ){
        स_हटाओ(&zOut[iOff + nReplace], &zOut[iOff + pBest->t.n],
            nOut - (iOff + pBest->t.n)
        );
        nOut += nReplace - pBest->t.n;
        zOut[nOut] = '\0';
      }
      स_नकल(&zOut[iOff], zReplace, nReplace);
      sqlite3DbFree(db, pBest);
    }

    sqlite3_परिणाम_text(pCtx, zOut, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, zOut);
  }अन्यथा{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  वापस rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
अटल व्योम renameSetENames(व्यंजक_List *pEList, पूर्णांक val){
  निश्चित( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  अगर( pEList ){
    पूर्णांक i;
    क्रम(i=0; i<pEList->n_व्यंजक; i++){
      निश्चित( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
अटल पूर्णांक renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  पूर्णांक rc = SQLITE_OK;

  स_रखो(&sNC, 0, माप(sNC));
  sNC.pParse = pParse;
  निश्चित( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3_Schema_To_सूचकांक(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  अगर( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  अगर( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_व्यंजक_Names(&sNC, pNew->pWhen);
  }

  क्रम(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->p_अग्र){
    अगर( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      अगर( pParse->nErr ) rc = pParse->rc;
    }
    अगर( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      अगर( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_व्यंजक_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        अगर( pSel==0 ){
          pStep->p_व्यंजक_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }अन्यथा{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_व्यंजक_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_व्यंजक_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          निश्चित( pStep->p_व्यंजक_List==0 || pStep->p_व्यंजक_List==pSel->pEList );
          निश्चित( pSrc==pSel->pSrc );
          अगर( pStep->p_व्यंजक_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        अगर( ALWAYS(pStep->pSrc) ){
          पूर्णांक i;
          क्रम(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            अगर( p->fg.isSubquery ){
              निश्चित( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        अगर(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        अगर( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_व्यंजक_Names(&sNC, pStep->pWhere);
        }
        अगर( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_व्यंजक_List_Names(&sNC, pStep->p_व्यंजक_List);
        }
        निश्चित( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_व्यंजक_List) );
        अगर( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_व्यंजक_List_Names(&sNC, pUpsert->pUpsertTarget);
          अगर( rc==SQLITE_OK ){
            व्यंजक_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_व्यंजक_List_Names(&sNC, pUpsertSet);
          }
          अगर( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_व्यंजक_Names(&sNC, pUpsert->pUpsertWhere);
          }
          अगर( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_व्यंजक_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }अन्यथा{
        rc = SQLITE_NOMEM;
      }
    }
  }
  वापस rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
अटल व्योम renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_व्यंजक(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  क्रम(pStep=pTrigger->step_list; pStep; pStep=pStep->p_अग्र){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_व्यंजक(pWalker, pStep->pWhere);
    sqlite3_Walk_व्यंजक_List(pWalker, pStep->p_व्यंजक_List);
    अगर( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_व्यंजक_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_व्यंजक_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_व्यंजक(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_व्यंजक(pWalker, pUpsert->pUpsertTargetWhere);
    }
    अगर( pStep->pSrc ){
      पूर्णांक i;
      SrcList *pSrc = pStep->pSrc;
      क्रम(i=0; i<pSrc->nSrc; i++){
        अगर( pSrc->a[i].fg.isSubquery ){
          निश्चित( pSrc->a[i].u4.pSubq!=0 );
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
अटल व्योम renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  सूचकांक *pIdx;
  अगर( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  जबतक( (pIdx = pParse->p_New_सूचकांक)!=0 ){
    pParse->p_New_सूचकांक = pIdx->p_अग्र;
    sqlite3_Free_सूचकांक(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_संदेश);
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
अटल व्योम renameColumnFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  स्थिर अक्षर *zSql = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  स्थिर अक्षर *zDb = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[3]);
  स्थिर अक्षर *zTable = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[4]);
  पूर्णांक iCol = sqlite3_मूल्य_int(argv[5]);
  स्थिर अक्षर *zNew = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[6]);
  पूर्णांक bQuote = sqlite3_मूल्य_int(argv[7]);
  पूर्णांक b_अस्थायी = sqlite3_मूल्य_int(argv[8]);
  स्थिर अक्षर *zOld;
  पूर्णांक rc;
  Parse sParse;
  Walker sWalker;
  सूचकांक *pIdx;
  पूर्णांक i;
  Table *pTab;
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#पूर्ण_अगर

  UNUSED_PARAMETER(NotUsed);
  अगर( zSql==0 ) वापस;
  अगर( zTable==0 ) वापस;
  अगर( zNew==0 ) वापस;
  अगर( iCol<0 ) वापस;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  अगर( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    वापस;
  }
  zOld = pTab->aCol[iCol].zCnName;
  स_रखो(&sCtx, 0, माप(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#पूर्ण_अगर
  rc = renameParseSql(&sParse, zDb, db, zSql, b_अस्थायी);

  /* Find tokens that need to be replaced. */
  स_रखो(&sWalker, 0, माप(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_व्यंजक_Callback = rename_Column_व्यंजक_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  अगर( rc!=SQLITE_OK ) जाओ renameColumnFunc_done;
  अगर( sParse.pNewTable ){
    अगर( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      अगर( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      अगर( rc!=SQLITE_OK ) जाओ renameColumnFunc_done;
    }अन्यथा अगर( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      पूर्णांक bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      अगर( bFKOnly==0 ){
        अगर( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (व्योम*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        अगर( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (व्योम*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_व्यंजक_List(&sWalker, sParse.pNewTable->pCheck);
        क्रम(pIdx=sParse.pNewTable->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          sqlite3_Walk_व्यंजक_List(&sWalker, pIdx->a_Col_व्यंजक);
        }
        क्रम(pIdx=sParse.p_New_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
          sqlite3_Walk_व्यंजक_List(&sWalker, pIdx->a_Col_व्यंजक);
        }
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
        क्रम(i=0; i<sParse.pNewTable->nCol; i++){
          व्यंजक *p_व्यंजक = sqlite3_Column_व्यंजक(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_व्यंजक(&sWalker, p_व्यंजक);
        }
#पूर्ण_अगर
      }

      निश्चित( IsOrdinaryTable(sParse.pNewTable) );
      क्रम(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->p_अग्र_From){
        क्रम(i=0; i<pFKey->nCol; i++){
          अगर( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (व्योम*)&pFKey->aCol[i]);
          }
          अगर( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (व्योम*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }अन्यथा अगर( sParse.p_New_सूचकांक ){
    sqlite3_Walk_व्यंजक_List(&sWalker, sParse.p_New_सूचकांक->a_Col_व्यंजक);
    sqlite3_Walk_व्यंजक(&sWalker, sParse.p_New_सूचकांक->pPartIdxWhere);
  }अन्यथा{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    अगर( rc!=SQLITE_OK ) जाओ renameColumnFunc_done;

    क्रम(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->p_अग्र){
      अगर( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        अगर( pTarget==pTab ){
          अगर( pStep->pUpsert ){
            व्यंजक_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_व्यंजक_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    अगर( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  निश्चित( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  अगर( rc!=SQLITE_OK ){
    अगर( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_परिणाम_मूल्य(context, argv[0]);
    }अन्यथा अगर( sParse.z_Err_संदेश ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }अन्यथा{
      sqlite3_परिणाम_error_code(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#पूर्ण_अगर
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
अटल पूर्णांक rename_Table_व्यंजक_Cb(Walker *pWalker, व्यंजक *p_व्यंजक){
  RenameCtx *p = pWalker->u.pRename;
  अगर( p_व्यंजक->op==TK_COLUMN
   && ALWAYS(व्यंजक_Use_YTab(p_व्यंजक))
   && p->pTab==p_व्यंजक->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (व्योम*)&p_व्यंजक->y.pTab);
  }
  वापस WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
अटल पूर्णांक renameTableSelectCb(Walker *pWalker, Select *pSelect){
  पूर्णांक i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  अगर( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    वापस WRC_Prune;
  }
  अगर( NEVER(pSrc==0) ){
    निश्चित( pWalker->pParse->db->mallocFailed );
    वापस WRC_Abort;
  }
  क्रम(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    अगर( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  वापस WRC_Continue;
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
अटल व्योम renameTableFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  स्थिर अक्षर *zDb = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  स्थिर अक्षर *z_निवेश = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[3]);
  स्थिर अक्षर *zOld = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[4]);
  स्थिर अक्षर *zNew = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[5]);
  पूर्णांक b_अस्थायी = sqlite3_मूल्य_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  अगर( z_निवेश && zOld && zNew ){
    Parse sParse;
    पूर्णांक rc;
    पूर्णांक bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#पूर्ण_अगर

    sqlite3BtreeEnterAll(db);

    स_रखो(&sCtx, 0, माप(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    स_रखो(&sWalker, 0, माप(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_व्यंजक_Callback = rename_Table_व्यंजक_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_निवेश, b_अस्थायी);

    अगर( rc==SQLITE_OK ){
      पूर्णांक isLegacy = (db->flags & SQLITE_LegacyAlter);
      अगर( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        अगर( IsView(pTab) ){
          अगर( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            स_रखो(&sNC, 0, माप(sNC));
            sNC.pParse = &sParse;

            निश्चित( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            अगर( sParse.nErr ){
              rc = sParse.rc;
            }अन्यथा{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }अन्यथा{
          /* Modify any FK definitions to point to the new table. */
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
          अगर( (isLegacy==0 || (db->flags & SQLITE_Foreign_कुंजियाँ))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            निश्चित( IsOrdinaryTable(pTab) );
            क्रम(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_अग्र_From){
              अगर( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (व्योम*)pFKey->zTo);
              }
            }
          }
#पूर्ण_अगर

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          अगर( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            अगर( isLegacy==0 ){
              sqlite3_Walk_व्यंजक_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      अन्यथा अगर( sParse.p_New_सूचकांक ){
        renameTokenFind(&sParse, &sCtx, sParse.p_New_सूचकांक->zName);
        अगर( isLegacy==0 ){
          sqlite3_Walk_व्यंजक(&sWalker, sParse.p_New_सूचकांक->pPartIdxWhere);
        }
      }

#अगर_अघोषित SQLITE_OMIT_TRIGGER
      अन्यथा{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        अगर( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        अगर( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          अगर( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            क्रम(pStep=pTrigger->step_list; pStep; pStep=pStep->p_अग्र){
              अगर( pStep->pSrc ){
                पूर्णांक i;
                क्रम(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  अगर( 0==sqlite3_stricmp(pItem->zName, zOld) ){
                    renameTokenFind(&sParse, &sCtx, pItem->zName);
                  }
                }
              }
            }
          }
        }
      }
#पूर्ण_अगर
    }

    अगर( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_निवेश, zNew, bQuote);
    }
    अगर( rc!=SQLITE_OK ){
      अगर( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_परिणाम_मूल्य(context, argv[3]);
      }अन्यथा अगर( sParse.z_Err_संदेश ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }अन्यथा{
        sqlite3_परिणाम_error_code(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#पूर्ण_अगर
  }

  वापस;
}

अटल पूर्णांक rename_Quotefix_व्यंजक_Cb(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_STRING && (p_व्यंजक->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (स्थिर व्योम*)p_व्यंजक);
  }
  वापस WRC_Continue;
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
अटल व्योम renameQuotefixFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  अक्षर स्थिर *zDb = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अक्षर स्थिर *z_निवेश = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#पूर्ण_अगर

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  अगर( zDb && z_निवेश ){
    पूर्णांक rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_निवेश, 0);

    अगर( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      स_रखो(&sCtx, 0, माप(RenameCtx));
      स_रखो(&sWalker, 0, माप(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_व्यंजक_Callback = rename_Quotefix_व्यंजक_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      अगर( sParse.pNewTable ){
        अगर( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          अगर( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }अन्यथा{
          पूर्णांक i;
          sqlite3_Walk_व्यंजक_List(&sWalker, sParse.pNewTable->pCheck);
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
          क्रम(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_व्यंजक(&sWalker,
               sqlite3_Column_व्यंजक(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#पूर्ण_अगर /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }अन्यथा अगर( sParse.p_New_सूचकांक ){
        sqlite3_Walk_व्यंजक_List(&sWalker, sParse.p_New_सूचकांक->a_Col_व्यंजक);
        sqlite3_Walk_व्यंजक(&sWalker, sParse.p_New_सूचकांक->pPartIdxWhere);
      }अन्यथा{
#अगर_अघोषित SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        अगर( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#पूर्ण_अगर /* SQLITE_OMIT_TRIGGER */
      }

      अगर( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_निवेश, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    अगर( rc!=SQLITE_OK ){
      अगर( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_परिणाम_मूल्य(context, argv[1]);
      }अन्यथा{
        sqlite3_परिणाम_error_code(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#पूर्ण_अगर

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
अटल व्योम renameTableTest(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  अक्षर स्थिर *zDb = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अक्षर स्थिर *z_निवेश = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
  पूर्णांक b_अस्थायी = sqlite3_मूल्य_int(argv[4]);
  पूर्णांक isLegacy = (db->flags & SQLITE_LegacyAlter);
  अक्षर स्थिर *zWhen = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[5]);
  पूर्णांक bNoDQS = sqlite3_मूल्य_int(argv[6]);

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#पूर्ण_अगर

  UNUSED_PARAMETER(NotUsed);

  अगर( zDb && z_निवेश ){
    पूर्णांक rc;
    Parse sParse;
    u64 flags = db->flags;
    अगर( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_निवेश, b_अस्थायी);
    db->flags = flags;
    अगर( rc==SQLITE_OK ){
      अगर( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        स_रखो(&sNC, 0, माप(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        अगर( sParse.nErr ) rc = sParse.rc;
      }

      अन्यथा अगर( sParse.pNewTrigger ){
        अगर( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        अगर( rc==SQLITE_OK ){
          पूर्णांक i1 = sqlite3_Schema_To_सूचकांक(db, sParse.pNewTrigger->pTabSchema);
          पूर्णांक i2 = sqlite3FindDbName(db, zDb);
          अगर( i1==i2 ){
            /* Handle output case B */
            sqlite3_परिणाम_int(context, 1);
          }
        }
      }
    }

    अगर( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      renameColumnParseError(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#पूर्ण_अगर
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
अटल पूर्णांक getConstraintToken(स्थिर u8 *z, पूर्णांक *piToken){
  पूर्णांक iOff = 0;
  पूर्णांक t = 0;
  करो {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }जबतक( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  अगर( t==TK_LP ){
    पूर्णांक nNest = 1;
    जबतक( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      अगर( t==TK_LP ){
        nNest++;
      }अन्यथा अगर( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }अन्यथा अगर( t==TK_ILLEGAL ){
        अवरोध;
      }
    }
  }

  *piToken = t;
  वापस iOff;
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
अटल व्योम dropColumnFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णांक iSchema = sqlite3_मूल्य_int(argv[0]);
  स्थिर अक्षर *zSql = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
  पूर्णांक iCol = sqlite3_मूल्य_int(argv[2]);
  स्थिर अक्षर *zDb = db->aDb[iSchema].zDbSName;
  पूर्णांक rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  स्थिर अक्षर *zEnd;
  अक्षर *zNew = 0;

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#पूर्ण_अगर

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  अगर( rc!=SQLITE_OK ) जाओ drop_column_done;
  pTab = sParse.pNewTable;
  अगर( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    जाओ drop_column_done;
  }

  अगर( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (व्योम*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (व्योम*)pTab->aCol[iCol+1].zCnName);
    zEnd = (स्थिर अक्षर*)pEnd->t.z;
  }अन्यथा{
    पूर्णांक eTok;
    निश्चित( IsOrdinaryTable(pTab) );
    निश्चित( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (व्योम*)pTab->aCol[iCol-1].zCnName);
    करो {
      pCol->t.z += getConstraintToken((स्थिर u8*)pCol->t.z, &eTok);
    }जबतक( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (स्थिर अक्षर*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_परिणाम_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#पूर्ण_अगर
  अगर( rc!=SQLITE_OK ){
    sqlite3_परिणाम_error_code(context, rc);
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
व्योम sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, स्थिर Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  पूर्णांक iDb;                        /* Index of db containing pTab in aDb[] */
  स्थिर अक्षर *zDb;                /* Database containing pTab ("main" etc.) */
  अक्षर *zCol = 0;                 /* Name of column to drop */
  पूर्णांक iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  निश्चित( pParse->pNewTable==0 );
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  अगर( NEVER(db->mallocFailed) ) जाओ exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  अगर( !pTab ) जाओ exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  अगर( SQLITE_OK!=isAlterableTable(pParse, pTab) ) जाओ exit_drop_column;
  अगर( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) जाओ exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  अगर( zCol==0 ){
    निश्चित( db->mallocFailed );
    जाओ exit_drop_column;
  }
  iCol = sqlite3_Column_सूचकांक(pTab, zCol);
  अगर( iCol<0 ){
    sqlite3_Error_संदेश(pParse, "no such column: \"%T\"", pName);
    जाओ exit_drop_column;
  }
  अगर( isRowidAlias(pParse, pTab, zCol, "drop") ) जाओ exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  अगर( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_संदेश(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    जाओ exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  अगर( pTab->nCol<=1 ){
    sqlite3_Error_संदेश(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    जाओ exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  निश्चित( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    जाओ exit_drop_column;
  }
#पूर्ण_अगर
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
  अगर( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    पूर्णांक i;
    पूर्णांक addr;
    पूर्णांक reg;
    पूर्णांक regRec;
    सूचकांक *pPk = 0;
    पूर्णांक nField = 0;               /* Number of non-virtual columns after drop */
    पूर्णांक iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    अगर( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }अन्यथा{
      pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
      pParse->nMem += pPk->nColumn;
      क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_कुंजी_Col;
    }
    regRec = ++pParse->nMem;
    क्रम(i=0; i<pTab->nCol; i++){
      अगर( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        पूर्णांक regOut;
        अगर( pPk ){
          पूर्णांक iPos = sqlite3_Table_Column_To_सूचकांक(pPk, i);
          पूर्णांक iColPos = sqlite3_Table_Column_To_सूचकांक(pPk, iCol);
          अगर( iPos<pPk->n_कुंजी_Col ) जारी;
          regOut = reg+1+iPos-(iPos>iColPos);
        }अन्यथा{
          regOut = reg+1+nField;
        }
        अगर( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }अन्यथा{
          sqlite3_व्यंजक_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          निश्चित( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    अगर( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    अगर( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_जोड़ो, iCur, regRec, reg+1, pPk->n_कुंजी_Col);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_जोड़ो, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3VdbeAddOp2(v, OP_अग्र, iCur, addr+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
  }

exit_drop_column:
  sqlite3DbFree(db, zCol);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
अटल पूर्णांक getWhitespace(स्थिर u8 *z){
  पूर्णांक nRet = 0;
  जबतक( 1 ){
    पूर्णांक t = 0;
    पूर्णांक n = sqlite3GetToken(&z[nRet], &t);
    अगर( t!=TK_SPACE && t!=TK_COMMENT ) अवरोध;
    nRet += n;
  }
  वापस nRet;
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
अटल पूर्णांक getConstraint(स्थिर u8 *z, पूर्णांक eTok){
  पूर्णांक iOff = 0;
  पूर्णांक t = 0;

#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
  अगर( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  अगर( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#पूर्ण_अगर

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  जबतक( 1 ){
    पूर्णांक n = getConstraintToken(&z[iOff], &t);
    अगर( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      अवरोध;
    }
    iOff += n;
  }
  
  वापस iOff;
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
अटल पूर्णांक quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  पूर्णांक t,                 /* Token type */
  स्थिर u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  पूर्णांक nQuote,            /* Length of zQuote in bytes */
  स्थिर u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  पूर्णांक *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  अक्षर *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  अगर( t==TK_ILLEGAL ){
    *pRes = 1;
    वापस SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  अगर( zCopy==0 ){
    sqlite3_परिणाम_error_nomem(ctx);
    वापस SQLITE_NOMEM_BKPT;
  }
  स_नकल(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((स्थिर अक्षर*)zCopy, (स्थिर अक्षर*)zCmp);
  sqlite3_free(zCopy);
  वापस SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
अटल पूर्णांक skipCreateTable(sqlite3_context *ctx, स्थिर u8 *zSql, पूर्णांक *piOff){
  पूर्णांक iOff = 0;

  अगर( zSql==0 ) वापस SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  जबतक( 1 ){
    पूर्णांक t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    अगर( t==TK_LP ) अवरोध;
    अगर( t==TK_ILLEGAL ){
      sqlite3_परिणाम_error_code(ctx, SQLITE_CORRUPT_BKPT);
      वापस SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  वापस SQLITE_OK;
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
अटल व्योम dropConstraintFunc(
  sqlite3_context *ctx,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  स्थिर u8 *zSql = sqlite3_मूल्य_text(argv[0]);
  स्थिर u8 *zCons = 0;
  पूर्णांक iNotNull = -1;
  पूर्णांक ii;
  पूर्णांक iOff = 0;
  पूर्णांक iStart = 0;
  पूर्णांक iEnd = 0;
  अक्षर *zNew = 0;
  पूर्णांक t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  अगर( zSql==0 ) वापस;

  /* Jump past the "CREATE TABLE" bit. */
  अगर( skipCreateTable(ctx, zSql, &iOff) ) वापस;

  अगर( sqlite3_मूल्य_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_मूल्य_int(argv[1]);
  }अन्यथा{
    zCons = sqlite3_मूल्य_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  क्रम(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    जबतक( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      अगर( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        पूर्णांक nTok = 0;
        पूर्णांक cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        अगर( zCons ){
          अगर( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) वापस;
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
        अगर( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }अन्यथा{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        अगर( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          अगर( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            वापस;
          }
          iEnd = iOff;
          अवरोध;
        }

      }अन्यथा अगर( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        अवरोध;
      }अन्यथा अगर( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        अवरोध;
      }अन्यथा अगर( t==TK_COMMA ){
        अवरोध;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  अगर( iEnd<=0 ){
    अगर( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }अन्यथा{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_परिणाम_text(ctx, (स्थिर अक्षर*)zSql, -1, SQLITE_TRANSIENT);
    }
  }अन्यथा{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    स्थिर अक्षर *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    अगर( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      अगर( zSql[iStart-1]==',' ) iStart--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", iStart, zSql, zSpace, &zSql[iEnd]);
    sqlite3_परिणाम_text(ctx, zNew, -1, SQLITE_DYNAMIC);
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
अटल व्योम addConstraintFunc(
  sqlite3_context *ctx,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  स्थिर u8 *zSql = sqlite3_मूल्य_text(argv[0]);
  स्थिर अक्षर *zCons = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
  पूर्णांक iCol = sqlite3_मूल्य_int(argv[2]);
  पूर्णांक iOff = 0;
  पूर्णांक ii;
  sqlite3_str *pNew;
  पूर्णांक t = 0;
  UNUSED_PARAMETER(NotUsed);

  अगर( skipCreateTable(ctx, zSql, &iOff) ) वापस;
  
  क्रम(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    जबतक( 1 ){
      पूर्णांक nTok = getConstraintToken(&zSql[iOff], &t);
      अगर( t==TK_COMMA || t==TK_RP ) अवरोध;
      अगर( t==TK_ILLEGAL ){
        sqlite3_परिणाम_error_code(ctx, SQLITE_CORRUPT_BKPT);
        वापस;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (स्थिर अक्षर*)zSql, iOff);
  अगर( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_परिणाम_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
अटल पूर्णांक alterFindCol(Parse *pParse, Table *pTab, Token *pCol, पूर्णांक *piCol){
  sqlite3 *db = pParse->db;
  अक्षर *zName = sqlite3NameFromToken(db, pCol);
  पूर्णांक rc = SQLITE_NOMEM;
  पूर्णांक iCol = -1;

  अगर( zName ){
    iCol = sqlite3_Column_सूचकांक(pTab, zName);
    अगर( iCol<0 ){
      sqlite3_Error_संदेश(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }अन्यथा{
      rc = SQLITE_OK;
    }
  }

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( rc==SQLITE_OK ){
    स्थिर अक्षर *zDb = db->aDb[sqlite3_Schema_To_सूचकांक(db, pTab->pSchema)].zDbSName;
    स्थिर अक्षर *zCol = pTab->aCol[iCol].zCnName;
    अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#पूर्ण_अगर

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  वापस rc;
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
अटल Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  पूर्णांक *piDb,            /* OUT: write the iDb here */
  स्थिर अक्षर **pzDb,    /* OUT: write name of schema here */
  पूर्णांक bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  अगर( pTab ){
    पूर्णांक iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    अगर( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( pTab && bAuth ){
    अगर( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#पूर्ण_अगर
  sqlite3SrcListDelete(db, pSrc);
  वापस pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
व्योम sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  पूर्णांक iDb = 0;
  स्थिर अक्षर *zDb = 0;
  अक्षर *zArg = 0;

  निश्चित( (pCol==0)!=(pCons==0) );
  निश्चित( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  अगर( !pTab ) वापस;

  अगर( pCons ){
    अक्षर *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }अन्यथा{
    पूर्णांक iCol;
    अगर( alterFindCol(pParse, pTab, pCol, &iCol) ) वापस;
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
अटल व्योम failConstraintFunc(
  sqlite3_context *ctx,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  स्थिर अक्षर *zText = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  पूर्णांक err = sqlite3_मूल्य_int(argv[1]);
  (व्योम)NotUsed;
  sqlite3_परिणाम_error(ctx, zText, -1);
  sqlite3_परिणाम_error_code(ctx, err);
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
अटल पूर्णांक alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  स्थिर अक्षर *pCons,              /* Buffer containing constraint */
  पूर्णांक nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  पूर्णांक iOff = 0;
  पूर्णांक iEnd = 0;

  अगर( zTmp==0 ) वापस 0;

  जबतक( 1 ){
    पूर्णांक t = 0;
    पूर्णांक nToken = sqlite3GetToken(&zTmp[iOff], &t);
    अगर( t==TK_ILLEGAL ) अवरोध;
    अगर( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  वापस iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
व्योम sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  पूर्णांक iCol = 0;
  पूर्णांक iDb = 0;
  स्थिर अक्षर *zDb = 0;
  स्थिर अक्षर *pCons = 0;
  पूर्णांक nCons = 0;

  /* Look up the table being altered. */
  निश्चित( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  अगर( !pTab ) वापस;

  /* Find the column being altered. */
  अगर( alterFindCol(pParse, pTab, pCol, &iCol) ){
    वापस;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (पूर्णांक)pCol->n, pCol->z
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
अटल व्योम findConstraintFunc(
  sqlite3_context *ctx,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  स्थिर u8 *zSql = 0;
  स्थिर u8 *zCons = 0;
  पूर्णांक iOff = 0;
  पूर्णांक t = 0;

  (व्योम)NotUsed;
  zSql = sqlite3_मूल्य_text(argv[0]);
  zCons = sqlite3_मूल्य_text(argv[1]);

  अगर( zSql==0 || zCons==0 ) वापस;
  जबतक( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  जबतक( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    अगर( t==TK_CONSTRAINT ){
      पूर्णांक nTok = 0;
      पूर्णांक cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      अगर( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) वापस;
      अगर( cmp==0 ){
        sqlite3_परिणाम_int(ctx, 1);
        वापस;
      }
    }अन्यथा अगर( t==TK_ILLEGAL ){
      अवरोध;
    }
  }

  sqlite3_परिणाम_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
व्योम sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  स्थिर अक्षर *z_व्यंजक,       /* Text of CHECK expression */
  पूर्णांक n_व्यंजक,               /* Size of pExpr in bytes */
  व्यंजक *p_व्यंजक              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  पूर्णांक iDb = 0;             /* Which schema does pTab live in */
  स्थिर अक्षर *zDb = 0;     /* Name of the schema in which pTab lives */
  स्थिर अक्षर *pCons = 0;   /* Text of the constraint */
  पूर्णांक nCons;               /* Bytes of text to use from pCons[] */
  पूर्णांक rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  निश्चित( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  अगर( !pTab ){
    sqlite3_व्यंजक_Delete(pParse->db, p_व्यंजक);
    वापस;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_व्यंजक, 0);
  sqlite3_व्यंजक_Delete(pParse->db, p_व्यंजक);
  अगर( rc ) वापस;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  अगर( pName ){
    अक्षर *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_व्यंजक, z_व्यंजक
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
व्योम sqlite3AlterFunctions(व्योम){
  अटल FuncDef aAlterTableFuncs[] = {
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
  sqlite3_जोड़ो_Builtin_Funcs(aAlterTableFuncs, Array_आकार(aAlterTableFuncs));
}
#पूर्ण_अगर  /* SQLITE_ALTER_TABLE */
