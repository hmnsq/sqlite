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
#અન્તર્ભાવ "sqliteInt.h"

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
સ્થિર પૂર્ણાંક isAlterableTable(Parse *pParse, Table *pTab){
  જો( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_સંદેશ(pParse, "table %s may not be altered", pTab->zName);
    પાછા 1;
  }
  પાછા 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
સ્થિર શૂન્ય renameTestSchema(
  Parse *pParse,                  /* Parse context */
  અચલ અક્ષર *zDb,                /* Name of db to verify schema of */
  પૂર્ણાંક bTemp,                      /* True if this is the temp db */
  અચલ અક્ષર *zWhen,              /* "when" part of error message */
  પૂર્ણાંક bNoDQS                      /* Do not allow DQS in the schema */
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

  જો( bTemp==0 ){
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
સ્થિર શૂન્ય renameFixQuotes(Parse *pParse, અચલ અક્ષર *zDb, પૂર્ણાંક bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  જો( bTemp==0 ){
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
સ્થિર શૂન્ય renameReloadSchema(Parse *pParse, પૂર્ણાંક iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  જો( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    જો( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
શૂન્ય sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  પૂર્ણાંક iDb;                  /* Database that contains the table */
  અક્ષર *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  અક્ષર *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  પૂર્ણાંક nTabName;             /* Number of UTF-8 characters in zTabName */
  અચલ અક્ષર *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  જો( NEVER(db->mallocFailed) ) જાઓ exit_rename_table;
  પ્રતિજ્ઞા( pSrc->nSrc==1 );
  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  જો( !pTab ) જાઓ exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  જો( !zName ) જાઓ exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  જો( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_સંદેશ(pParse,
        "there is already another table or index with this name: %s", zName);
    જાઓ exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  જો( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    જાઓ exit_rename_table;
  }
  જો( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    જાઓ exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  જો( IsView(pTab) ){
    sqlite3_Error_સંદેશ(pParse, "view %s may not be altered", pTab->zName);
    જાઓ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    જાઓ exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  જો( sqlite3ViewGetColumnNames(pParse, pTab) ){
    જાઓ exit_rename_table;
  }
  જો( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    જો( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  જો( v==0 ){
    જાઓ exit_rename_table;
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
  જો( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  જો( iDb!=1 ){
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
  જો( pVTab ){
    પૂર્ણાંક i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(અચલ અક્ષર*)pVTab, P4_VTAB);
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
સ્થિર શૂન્ય sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  અચલ અક્ષર *zDb,      /* Schema holding the table */
  અચલ અક્ષર *zTab,     /* Table to check for empty */
  અચલ અક્ષર *zErr      /* Error message text */
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
સ્થિર પૂર્ણાંક isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  અચલ અક્ષર *zCol, 
  અચલ અક્ષર *zOp
){
  જો( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_સંદેશ(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    પાછા SQLITE_ERROR;
  }
  પાછા SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
શૂન્ય sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  પૂર્ણાંક iDb;                  /* Database number */
  અચલ અક્ષર *zDb;          /* Database name */
  અચલ અક્ષર *zTab;         /* Table name */
  અક્ષર *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  પદાવલિ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  પૂર્ણાંક r1;                   /* Temporary registers */

  db = pParse->db;
  પ્રતિજ્ઞા( db->pParse==pParse );
  જો( pParse->nErr ) પાછા;
  પ્રતિજ્ઞા( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  પ્રતિજ્ઞા( pNew );

  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_પદાવલિ(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  પ્રતિજ્ઞા( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    પાછા;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  જો( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_સંદેશ(pParse, "Cannot add a PRIMARY KEY column");
    પાછા;
  }
  જો( pNew->pIndex ){
    sqlite3_Error_સંદેશ(pParse,
         "Cannot add a UNIQUE column");
    પાછા;
  }
  જો( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) પાછા;
  જો( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    પ્રતિજ્ઞા( pDflt==0 || pDflt->op==TK_SPAN );
    જો( pDflt && pDflt->p_ડાબું->op==TK_NULL ){
      pDflt = 0;
    }
    પ્રતિજ્ઞા( IsOrdinaryTable(pNew) );
    જો( (db->flags&SQLITE_Foreign_ચાવીઓ) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    જો( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    જો( pDflt ){
      sqlite3_કિંમત *pVal = 0;
      પૂર્ણાંક rc;
      rc = sqlite3_કિંમત_From_પદાવલિ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      પ્રતિજ્ઞા( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      જો( rc!=SQLITE_OK ){
        પ્રતિજ્ઞા( db->mallocFailed == 1 );
        પાછા;
      }
      જો( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_કિંમત_Free(pVal);
    }
  }નહીંતર જો( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (અક્ષર*)pColDef->z, pColDef->n);
  જો( zCol ){
    અક્ષર *zEnd = &zCol[pColDef->n-1];
    જ્યારે( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    પ્રતિજ્ઞા( IsOrdinaryTable(pTab) );
    પ્રતિજ્ઞા( IsOrdinaryTable(pNew) );
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
  જો( v ){
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
    જો( pNew->pCheck!=0
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
શૂન્ય sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  પૂર્ણાંક iDb;
  પૂર્ણાંક i;
  પૂર્ણાંક nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  પ્રતિજ્ઞા( pParse->pNewTable==0 );
  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  જો( NEVER(db->mallocFailed) ) જાઓ exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  જો( !pTab ) જાઓ exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  જો( IsVirtual(pTab) ){
    sqlite3_Error_સંદેશ(pParse, "virtual tables may not be altered");
    જાઓ exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  જો( IsView(pTab) ){
    sqlite3_Error_સંદેશ(pParse, "Cannot add a column to a view");
    જાઓ exit_begin_add_column;
  }
  જો( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    જાઓ exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  પ્રતિજ્ઞા( IsOrdinaryTable(pTab) );
  પ્રતિજ્ઞા( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, કદ(Table));
  જો( !pNew ) જાઓ exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  પ્રતિજ્ઞા( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  પ્રતિજ્ઞા( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, કદ(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  જો( !pNew->aCol || !pNew->zName ){
    પ્રતિજ્ઞા( db->mallocFailed );
    જાઓ exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, કદ(Column)*(size_t)pNew->nCol);
  માટે(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  પ્રતિજ્ઞા( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_પદાવલિ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  પ્રતિજ્ઞા( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  પાછા;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#જો !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
સ્થિર પૂર્ણાંક isRealTable(Parse *pParse, Table *pTab, પૂર્ણાંક iOp){
  અચલ અક્ષર *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  જો( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  જો( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  જો( zType ){
    અચલ અક્ષર *az_સંદેશ[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    પ્રતિજ્ઞા( iOp>=0 && iOp<ArraySize(az_સંદેશ) );
    sqlite3_Error_સંદેશ(pParse, "cannot %s %s \"%s\"",
        az_સંદેશ[iOp], zType, pTab->zName
    );
    પાછા 1;
  }
  પાછા 0;
}
#નહીંતર /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
શૂન્ય sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  પૂર્ણાંક iCol;                       /* Index of column being renamed */
  અક્ષર *zOld = 0;                 /* Old column name */
  અક્ષર *zNew = 0;                 /* New column name */
  અચલ અક્ષર *zDb;                /* Name of schema containing the table */
  પૂર્ણાંક iSchema;                    /* Index of the schema */
  પૂર્ણાંક bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  જો( !pTab ) જાઓ exit_rename_column;

  /* Cannot alter a system table */
  જો( SQLITE_OK!=isAlterableTable(pParse, pTab) ) જાઓ exit_rename_column;
  જો( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) જાઓ exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  પ્રતિજ્ઞા( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    જાઓ exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  જો( !zOld ) જાઓ exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  જો( iCol<0 ){
    sqlite3_Error_સંદેશ(pParse, "no such column: \"%T\"", pOld);
    જાઓ exit_rename_column;
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
  જો( !zNew ) જાઓ exit_rename_column;
  જો( isRowidAlias(pParse, pTab, zOld, "rename from") ) જાઓ exit_rename_column;
  જો( isRowidAlias(pParse, pTab, zNew, "rename to") ) જાઓ exit_rename_column;
  પ્રતિજ્ઞા( pNew->n>0 );
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
  જો( iSchema!=1 ){
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
  પાછા;
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
સંરચના RenameToken {
  અચલ શૂન્ય *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
પ્રકારવ્યાખ્યા સંરચના RenameCtx RenameCtx;
સંરચના RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  પૂર્ણાંક nList;                      /* Number of tokens in pList */
  પૂર્ણાંક iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  અચલ અક્ષર *zOld;               /* Old column name */
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
સ્થિર શૂન્ય renameTokenCheckAll(Parse *pParse, અચલ શૂન્ય *pPtr){
  પ્રતિજ્ઞા( pParse==pParse->db->pParse );
  પ્રતિજ્ઞા( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  જો( pParse->nErr==0 ){
    અચલ RenameToken *p;
    u32 i = 1;
    માટે(p=pParse->pRename; p; p=p->pNext){
      જો( p->p ){
        પ્રતિજ્ઞા( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    પ્રતિજ્ઞા( i>0 );
  }
}
#નહીંતર
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
અચલ શૂન્ય *sqlite3RenameTokenMap(
  Parse *pParse,
  અચલ શૂન્ય *pPtr,
  અચલ Token *pToken
){
  RenameToken *pNew;
  પ્રતિજ્ઞા( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  જો( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, કદ(RenameToken));
    જો( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  પાછા pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
શૂન્ય sqlite3RenameTokenRemap(Parse *pParse, અચલ શૂન્ય *pTo, અચલ શૂન્ય *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  માટે(p=pParse->pRename; p; p=p->pNext){
    જો( p->p==pFrom ){
      p->p = pTo;
      તોડો;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
સ્થિર પૂર્ણાંક rename_Unmap_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (અચલ શૂન્ય*)p_પદાવલિ);
  જો( પદાવલિ_Use_YTab(p_પદાવલિ) ){
    sqlite3RenameTokenRemap(pParse, 0, (અચલ શૂન્ય*)&p_પદાવલિ->y.pTab);
  }
  પાછા WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
સ્થિર શૂન્ય renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  જો( pWith ){
    Parse *pParse = pWalker->pParse;
    પૂર્ણાંક i;
    With *pCopy = 0;
    પ્રતિજ્ઞા( pWith->nCte>0 );
    જો( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_ધકેલો(pParse, pCopy, 1);
    }
    માટે(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, કદ(sNC));
      sNC.pParse = pParse;
      જો( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      જો( sNC.pParse->db->mallocFailed ) પાછા;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    જો( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
સ્થિર શૂન્ય unmapColumnIdlistNames(
  Parse *pParse,
  અચલ IdList *pIdList
){
  પૂર્ણાંક ii;
  પ્રતિજ્ઞા( pIdList!=0 );
  માટે(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (અચલ શૂન્ય*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
સ્થિર પૂર્ણાંક renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  પૂર્ણાંક i;
  જો( pParse->nErr ) પાછા WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  જો( p->selFlags & (SF_View|SF_CopyCte) ){
    પાછા WRC_Prune;
  }
  જો( ALWAYS(p->pEList) ){
    પદાવલિ_List *pList = p->pEList;
    માટે(i=0; i<pList->n_પદાવલિ; i++){
      જો( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)pList->a[i].zEName);
      }
    }
  }
  જો( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    માટે(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)pSrc->a[i].zName);
      જો( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_પદાવલિ(pWalker, pSrc->a[i].u3.pOn);
      }નહીંતર{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  પાછા WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
શૂન્ય sqlite3_Rename_પદાવલિ_Unmap(Parse *pParse, પદાવલિ *p_પદાવલિ){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, કદ(Walker));
  sWalker.pParse = pParse;
  sWalker.x_પદાવલિ_Callback = rename_Unmap_પદાવલિ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_પદાવલિ(&sWalker, p_પદાવલિ);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
શૂન્ય sqlite3RenameExprlistUnmap(Parse *pParse, પદાવલિ_List *pEList){
  જો( pEList ){
    પૂર્ણાંક i;
    Walker sWalker;
    memset(&sWalker, 0, કદ(Walker));
    sWalker.pParse = pParse;
    sWalker.x_પદાવલિ_Callback = rename_Unmap_પદાવલિ_Cb;
    sqlite3_Walk_પદાવલિ_List(&sWalker, pEList);
    માટે(i=0; i<pEList->n_પદાવલિ; i++){
      જો( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
સ્થિર શૂન્ય renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  માટે(p=pToken; p; p=pNext){
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
સ્થિર RenameToken *renameTokenFind(
  Parse *pParse,
  સંરચના RenameCtx *pCtx,
  અચલ શૂન્ય *pPtr
){
  RenameToken **pp;
  જો( NEVER(pPtr==0) ){
    પાછા 0;
  }
  માટે(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    જો( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      જો( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      પાછા pToken;
    }
  }
  પાછા 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
સ્થિર પૂર્ણાંક renameColumnSelectCb(Walker *pWalker, Select *p){
  જો( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    પાછા WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  પાછા WRC_Continue;
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
સ્થિર પૂર્ણાંક rename_Column_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  RenameCtx *p = pWalker->u.pRename;
  જો( p_પદાવલિ->op==TK_TRIGGER
   && p_પદાવલિ->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (શૂન્ય*)p_પદાવલિ);
  }નહીંતર જો( p_પદાવલિ->op==TK_COLUMN
   && p_પદાવલિ->iColumn==p->iCol
   && ALWAYS(પદાવલિ_Use_YTab(p_પદાવલિ))
   && p->pTab==p_પદાવલિ->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (શૂન્ય*)p_પદાવલિ);
  }
  પાછા WRC_Continue;
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
સ્થિર RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  માટે(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    જો( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  માટે(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  પાછા pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
સ્થિર શૂન્ય errorMPrintf(sqlite3_context *pCtx, અચલ અક્ષર *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  અક્ષર *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  જો( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }નહીંતર{
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
સ્થિર શૂન્ય renameColumnParseError(
  sqlite3_context *pCtx,
  અચલ અક્ષર *zWhen,
  sqlite3_કિંમત *pType,
  sqlite3_કિંમત *pObject,
  Parse *pParse
){
  અચલ અક્ષર *zT = (અચલ અક્ષર*)sqlite3_કિંમત_text(pType);
  અચલ અક્ષર *zN = (અચલ અક્ષર*)sqlite3_કિંમત_text(pObject);
  અક્ષર *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_સંદેશ
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
સ્થિર શૂન્ય renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  અચલ પદાવલિ_List *pEList,
  અચલ અક્ષર *zOld
){
  જો( pEList ){
    પૂર્ણાંક i;
    માટે(i=0; i<pEList->n_પદાવલિ; i++){
      અચલ અક્ષર *zName = pEList->a[i].zEName;
      જો( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (અચલ શૂન્ય*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
સ્થિર શૂન્ય renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  અચલ IdList *pIdList,
  અચલ અક્ષર *zOld
){
  જો( pIdList ){
    પૂર્ણાંક i;
    માટે(i=0; i<pIdList->nId; i++){
      અચલ અક્ષર *zName = pIdList->a[i].zName;
      જો( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (અચલ શૂન્ય*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
સ્થિર પૂર્ણાંક renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  અચલ અક્ષર *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  અચલ અક્ષર *zSql,               /* SQL to parse */
  પૂર્ણાંક bTemp                       /* True if SQL is from temp schema */
){
  પૂર્ણાંક rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  જો( zSql==0 ){
    પાછા SQLITE_NOMEM;
  }
  જો( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    પાછા SQLITE_CORRUPT_BKPT;
  }
  જો( bTemp ){
    db->init.iDb = 1;
  }નહીંતર{
    પૂર્ણાંક iDb = sqlite3FindDbName(db, zDb);
    પ્રતિજ્ઞા( iDb>=0 && iDb<=0xff );
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
  જો( db->mallocFailed ) rc = SQLITE_NOMEM;
  જો( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  જો( rc==SQLITE_OK ){
    પૂર્ણાંક nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    માટે(pToken=p->pRename; pToken; pToken=pToken->pNext){
      પ્રતિજ્ઞા( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  પાછા rc;
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
સ્થિર પૂર્ણાંક renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  અચલ અક્ષર *zSql,               /* SQL statement to edit */
  અચલ અક્ષર *zNew,               /* New token text */
  પૂર્ણાંક bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  પૂર્ણાંક rc = SQLITE_OK;
  અક્ષર *zQuot = 0;
  અક્ષર *zOut;
  i64 nQuot = 0;
  અક્ષર *zBuf1 = 0;
  અક્ષર *zBuf2 = 0;

  જો( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    જો( zQuot==0 ){
      પાછા SQLITE_NOMEM;
    }નહીંતર{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    પ્રતિજ્ઞા( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }નહીંતર{
    પ્રતિજ્ઞા( nSql>0 );
    zOut = (અક્ષર*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    જો( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  જો( zOut ){
    i64 nOut = nSql;
    પ્રતિજ્ઞા( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    જ્યારે( pRename->pList ){
      પૂર્ણાંક iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      અચલ અક્ષર *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      જો( zNew ){
        જો( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }નહીંતર{
          nReplace = nQuot;
          zReplace = zQuot;
          જો( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }નહીંતર{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        પ્રતિજ્ઞા( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((પૂર્ણાંક)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (પૂર્ણાંક)(pBest->t.z - zSql);
      જો( pBest->t.n!=nReplace ){
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
  }નહીંતર{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  પાછા rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
સ્થિર શૂન્ય renameSetENames(પદાવલિ_List *pEList, પૂર્ણાંક val){
  પ્રતિજ્ઞા( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  જો( pEList ){
    પૂર્ણાંક i;
    માટે(i=0; i<pEList->n_પદાવલિ; i++){
      પ્રતિજ્ઞા( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
સ્થિર પૂર્ણાંક renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  પૂર્ણાંક rc = SQLITE_OK;

  memset(&sNC, 0, કદ(sNC));
  sNC.pParse = pParse;
  પ્રતિજ્ઞા( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  જો( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  જો( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, pNew->pWhen);
  }

  માટે(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    જો( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      જો( pParse->nErr ) rc = pParse->rc;
    }
    જો( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      જો( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_પદાવલિ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        જો( pSel==0 ){
          pStep->p_પદાવલિ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }નહીંતર{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_પદાવલિ_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_પદાવલિ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          પ્રતિજ્ઞા( pStep->p_પદાવલિ_List==0 || pStep->p_પદાવલિ_List==pSel->pEList );
          પ્રતિજ્ઞા( pSrc==pSel->pSrc );
          જો( pStep->p_પદાવલિ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        જો( ALWAYS(pStep->pSrc) ){
          પૂર્ણાંક i;
          માટે(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            જો( p->fg.isSubquery ){
              પ્રતિજ્ઞા( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        જો(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        જો( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, pStep->pWhere);
        }
        જો( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pStep->p_પદાવલિ_List);
        }
        પ્રતિજ્ઞા( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_પદાવલિ_List) );
        જો( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pUpsert->pUpsertTarget);
          જો( rc==SQLITE_OK ){
            પદાવલિ_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pUpsertSet);
          }
          જો( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, pUpsert->pUpsertWhere);
          }
          જો( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }નહીંતર{
        rc = SQLITE_NOMEM;
      }
    }
  }
  પાછા rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
સ્થિર શૂન્ય renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_પદાવલિ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  માટે(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_પદાવલિ(pWalker, pStep->pWhere);
    sqlite3_Walk_પદાવલિ_List(pWalker, pStep->p_પદાવલિ_List);
    જો( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_પદાવલિ_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_પદાવલિ_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_પદાવલિ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_પદાવલિ(pWalker, pUpsert->pUpsertTargetWhere);
    }
    જો( pStep->pSrc ){
      પૂર્ણાંક i;
      SrcList *pSrc = pStep->pSrc;
      માટે(i=0; i<pSrc->nSrc; i++){
        જો( pSrc->a[i].fg.isSubquery ){
          પ્રતિજ્ઞા( pSrc->a[i].u4.pSubq!=0 );
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
સ્થિર શૂન્ય renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  જો( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  જ્યારે( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_સંદેશ);
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
સ્થિર શૂન્ય renameColumnFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  અચલ અક્ષર *zSql = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  અચલ અક્ષર *zDb = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[3]);
  અચલ અક્ષર *zTable = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[4]);
  પૂર્ણાંક iCol = sqlite3_કિંમત_int(argv[5]);
  અચલ અક્ષર *zNew = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[6]);
  પૂર્ણાંક bQuote = sqlite3_કિંમત_int(argv[7]);
  પૂર્ણાંક bTemp = sqlite3_કિંમત_int(argv[8]);
  અચલ અક્ષર *zOld;
  પૂર્ણાંક rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  પૂર્ણાંક i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  જો( zSql==0 ) પાછા;
  જો( zTable==0 ) પાછા;
  જો( zNew==0 ) પાછા;
  જો( iCol<0 ) પાછા;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  જો( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    પાછા;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, કદ(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, કદ(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_પદાવલિ_Callback = rename_Column_પદાવલિ_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  જો( rc!=SQLITE_OK ) જાઓ renameColumnFunc_done;
  જો( sParse.pNewTable ){
    જો( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      જો( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      જો( rc!=SQLITE_OK ) જાઓ renameColumnFunc_done;
    }નહીંતર જો( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      પૂર્ણાંક bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      જો( bFKOnly==0 ){
        જો( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (શૂન્ય*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        જો( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (શૂન્ય*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_પદાવલિ_List(&sWalker, sParse.pNewTable->pCheck);
        માટે(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_પદાવલિ_List(&sWalker, pIdx->a_Col_પદાવલિ);
        }
        માટે(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_પદાવલિ_List(&sWalker, pIdx->a_Col_પદાવલિ);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        માટે(i=0; i<sParse.pNewTable->nCol; i++){
          પદાવલિ *p_પદાવલિ = sqlite3_Column_પદાવલિ(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_પદાવલિ(&sWalker, p_પદાવલિ);
        }
#endif
      }

      પ્રતિજ્ઞા( IsOrdinaryTable(sParse.pNewTable) );
      માટે(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        માટે(i=0; i<pFKey->nCol; i++){
          જો( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (શૂન્ય*)&pFKey->aCol[i]);
          }
          જો( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (શૂન્ય*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }નહીંતર જો( sParse.pNewIndex ){
    sqlite3_Walk_પદાવલિ_List(&sWalker, sParse.pNewIndex->a_Col_પદાવલિ);
    sqlite3_Walk_પદાવલિ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }નહીંતર{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    જો( rc!=SQLITE_OK ) જાઓ renameColumnFunc_done;

    માટે(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      જો( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        જો( pTarget==pTab ){
          જો( pStep->pUpsert ){
            પદાવલિ_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_પદાવલિ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    જો( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  પ્રતિજ્ઞા( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  જો( rc!=SQLITE_OK ){
    જો( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_કિંમત(context, argv[0]);
    }નહીંતર જો( sParse.z_Err_સંદેશ ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }નહીંતર{
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
સ્થિર પૂર્ણાંક rename_Table_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  RenameCtx *p = pWalker->u.pRename;
  જો( p_પદાવલિ->op==TK_COLUMN
   && ALWAYS(પદાવલિ_Use_YTab(p_પદાવલિ))
   && p->pTab==p_પદાવલિ->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (શૂન્ય*)&p_પદાવલિ->y.pTab);
  }
  પાછા WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
સ્થિર પૂર્ણાંક renameTableSelectCb(Walker *pWalker, Select *pSelect){
  પૂર્ણાંક i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  જો( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    પાછા WRC_Prune;
  }
  જો( NEVER(pSrc==0) ){
    પ્રતિજ્ઞા( pWalker->pParse->db->mallocFailed );
    પાછા WRC_Abort;
  }
  માટે(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    જો( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  પાછા WRC_Continue;
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
સ્થિર શૂન્ય renameTableFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  અચલ અક્ષર *zDb = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  અચલ અક્ષર *z_આગત = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[3]);
  અચલ અક્ષર *zOld = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[4]);
  અચલ અક્ષર *zNew = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[5]);
  પૂર્ણાંક bTemp = sqlite3_કિંમત_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  જો( z_આગત && zOld && zNew ){
    Parse sParse;
    પૂર્ણાંક rc;
    પૂર્ણાંક bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, કદ(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, કદ(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_પદાવલિ_Callback = rename_Table_પદાવલિ_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_આગત, bTemp);

    જો( rc==SQLITE_OK ){
      પૂર્ણાંક isLegacy = (db->flags & SQLITE_LegacyAlter);
      જો( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        જો( IsView(pTab) ){
          જો( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, કદ(sNC));
            sNC.pParse = &sParse;

            પ્રતિજ્ઞા( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            જો( sParse.nErr ){
              rc = sParse.rc;
            }નહીંતર{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }નહીંતર{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_ચાવી
          જો( (isLegacy==0 || (db->flags & SQLITE_Foreign_ચાવીઓ))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            પ્રતિજ્ઞા( IsOrdinaryTable(pTab) );
            માટે(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              જો( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (શૂન્ય*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          જો( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            જો( isLegacy==0 ){
              sqlite3_Walk_પદાવલિ_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      નહીંતર જો( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        જો( isLegacy==0 ){
          sqlite3_Walk_પદાવલિ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      નહીંતર{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        જો( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        જો( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          જો( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            માટે(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              જો( pStep->pSrc ){
                પૂર્ણાંક i;
                માટે(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  જો( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    જો( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_આગત, zNew, bQuote);
    }
    જો( rc!=SQLITE_OK ){
      જો( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_કિંમત(context, argv[3]);
      }નહીંતર જો( sParse.z_Err_સંદેશ ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }નહીંતર{
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

  પાછા;
}

સ્થિર પૂર્ણાંક rename_Quotefix_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_STRING && (p_પદાવલિ->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (અચલ શૂન્ય*)p_પદાવલિ);
  }
  પાછા WRC_Continue;
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
સ્થિર શૂન્ય renameQuotefixFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  અક્ષર અચલ *zDb = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  અક્ષર અચલ *z_આગત = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  જો( zDb && z_આગત ){
    પૂર્ણાંક rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_આગત, 0);

    જો( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, કદ(RenameCtx));
      memset(&sWalker, 0, કદ(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_પદાવલિ_Callback = rename_Quotefix_પદાવલિ_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      જો( sParse.pNewTable ){
        જો( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          જો( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }નહીંતર{
          પૂર્ણાંક i;
          sqlite3_Walk_પદાવલિ_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          માટે(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_પદાવલિ(&sWalker,
               sqlite3_Column_પદાવલિ(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }નહીંતર જો( sParse.pNewIndex ){
        sqlite3_Walk_પદાવલિ_List(&sWalker, sParse.pNewIndex->a_Col_પદાવલિ);
        sqlite3_Walk_પદાવલિ(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }નહીંતર{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        જો( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      જો( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_આગત, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    જો( rc!=SQLITE_OK ){
      જો( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_કિંમત(context, argv[1]);
      }નહીંતર{
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
સ્થિર શૂન્ય renameTableTest(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  અક્ષર અચલ *zDb = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  અક્ષર અચલ *z_આગત = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
  પૂર્ણાંક bTemp = sqlite3_કિંમત_int(argv[4]);
  પૂર્ણાંક isLegacy = (db->flags & SQLITE_LegacyAlter);
  અક્ષર અચલ *zWhen = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[5]);
  પૂર્ણાંક bNoDQS = sqlite3_કિંમત_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  જો( zDb && z_આગત ){
    પૂર્ણાંક rc;
    Parse sParse;
    u64 flags = db->flags;
    જો( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_આગત, bTemp);
    db->flags = flags;
    જો( rc==SQLITE_OK ){
      જો( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, કદ(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        જો( sParse.nErr ) rc = sParse.rc;
      }

      નહીંતર જો( sParse.pNewTrigger ){
        જો( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        જો( rc==SQLITE_OK ){
          પૂર્ણાંક i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          પૂર્ણાંક i2 = sqlite3FindDbName(db, zDb);
          જો( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    જો( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
સ્થિર પૂર્ણાંક getConstraintToken(અચલ u8 *z, પૂર્ણાંક *piToken){
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક t = 0;
  કરો {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }જ્યારે( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  જો( t==TK_LP ){
    પૂર્ણાંક nNest = 1;
    જ્યારે( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      જો( t==TK_LP ){
        nNest++;
      }નહીંતર જો( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }નહીંતર જો( t==TK_ILLEGAL ){
        તોડો;
      }
    }
  }

  *piToken = t;
  પાછા iOff;
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
સ્થિર શૂન્ય dropColumnFunc(
  sqlite3_context *context,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  પૂર્ણાંક iSchema = sqlite3_કિંમત_int(argv[0]);
  અચલ અક્ષર *zSql = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
  પૂર્ણાંક iCol = sqlite3_કિંમત_int(argv[2]);
  અચલ અક્ષર *zDb = db->aDb[iSchema].zDbSName;
  પૂર્ણાંક rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  અચલ અક્ષર *zEnd;
  અક્ષર *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  જો( rc!=SQLITE_OK ) જાઓ drop_column_done;
  pTab = sParse.pNewTable;
  જો( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    જાઓ drop_column_done;
  }

  જો( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (શૂન્ય*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (શૂન્ય*)pTab->aCol[iCol+1].zCnName);
    zEnd = (અચલ અક્ષર*)pEnd->t.z;
  }નહીંતર{
    પૂર્ણાંક eTok;
    પ્રતિજ્ઞા( IsOrdinaryTable(pTab) );
    પ્રતિજ્ઞા( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (શૂન્ય*)pTab->aCol[iCol-1].zCnName);
    કરો {
      pCol->t.z += getConstraintToken((અચલ u8*)pCol->t.z, &eTok);
    }જ્યારે( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (અચલ અક્ષર*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  જો( rc!=SQLITE_OK ){
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
શૂન્ય sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, અચલ Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  પૂર્ણાંક iDb;                        /* Index of db containing pTab in aDb[] */
  અચલ અક્ષર *zDb;                /* Database containing pTab ("main" etc.) */
  અક્ષર *zCol = 0;                 /* Name of column to drop */
  પૂર્ણાંક iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  પ્રતિજ્ઞા( pParse->pNewTable==0 );
  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  જો( NEVER(db->mallocFailed) ) જાઓ exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  જો( !pTab ) જાઓ exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  જો( SQLITE_OK!=isAlterableTable(pParse, pTab) ) જાઓ exit_drop_column;
  જો( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) જાઓ exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  જો( zCol==0 ){
    પ્રતિજ્ઞા( db->mallocFailed );
    જાઓ exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  જો( iCol<0 ){
    sqlite3_Error_સંદેશ(pParse, "no such column: \"%T\"", pName);
    જાઓ exit_drop_column;
  }
  જો( isRowidAlias(pParse, pTab, zCol, "drop") ) જાઓ exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  જો( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_સંદેશ(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    જાઓ exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  જો( pTab->nCol<=1 ){
    sqlite3_Error_સંદેશ(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    જાઓ exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  પ્રતિજ્ઞા( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    જાઓ exit_drop_column;
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
  જો( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    પૂર્ણાંક i;
    પૂર્ણાંક addr;
    પૂર્ણાંક reg;
    પૂર્ણાંક regRec;
    Index *pPk = 0;
    પૂર્ણાંક nField = 0;               /* Number of non-virtual columns after drop */
    પૂર્ણાંક iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    જો( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }નહીંતર{
      pPk = sqlite3_Primary_ચાવી_Index(pTab);
      pParse->nMem += pPk->nColumn;
      માટે(i=0; i<pPk->n_ચાવી_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_ચાવી_Col;
    }
    regRec = ++pParse->nMem;
    માટે(i=0; i<pTab->nCol; i++){
      જો( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        પૂર્ણાંક regOut;
        જો( pPk ){
          પૂર્ણાંક iPos = sqlite3TableColumnToIndex(pPk, i);
          પૂર્ણાંક iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          જો( iPos<pPk->n_ચાવી_Col ) ચાલુ;
          regOut = reg+1+iPos-(iPos>iColPos);
        }નહીંતર{
          regOut = reg+1+nField;
        }
        જો( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }નહીંતર{
          sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          પ્રતિજ્ઞા( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    જો( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    જો( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iCur, regRec, reg+1, pPk->n_ચાવી_Col);
    }નહીંતર{
      sqlite3VdbeAddOp3(v, OP_ઉમેરો, iCur, regRec, reg);
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
સ્થિર પૂર્ણાંક getWhitespace(અચલ u8 *z){
  પૂર્ણાંક nRet = 0;
  જ્યારે( 1 ){
    પૂર્ણાંક t = 0;
    પૂર્ણાંક n = sqlite3GetToken(&z[nRet], &t);
    જો( t!=TK_SPACE && t!=TK_COMMENT ) તોડો;
    nRet += n;
  }
  પાછા nRet;
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
સ્થિર પૂર્ણાંક getConstraint(અચલ u8 *z, પૂર્ણાંક eTok){
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક t = 0;

#ifndef SQLITE_OMIT_FOREIGN_ચાવી
  જો( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  જો( eTok==TK_REFERENCES ){
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
  જ્યારે( 1 ){
    પૂર્ણાંક n = getConstraintToken(&z[iOff], &t);
    જો( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      તોડો;
    }
    iOff += n;
  }
  
  પાછા iOff;
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
સ્થિર પૂર્ણાંક quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  પૂર્ણાંક t,                 /* Token type */
  અચલ u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  પૂર્ણાંક nQuote,            /* Length of zQuote in bytes */
  અચલ u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  પૂર્ણાંક *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  અક્ષર *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  જો( t==TK_ILLEGAL ){
    *pRes = 1;
    પાછા SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  જો( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    પાછા SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((અચલ અક્ષર*)zCopy, (અચલ અક્ષર*)zCmp);
  sqlite3_free(zCopy);
  પાછા SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
સ્થિર પૂર્ણાંક skipCreateTable(sqlite3_context *ctx, અચલ u8 *zSql, પૂર્ણાંક *piOff){
  પૂર્ણાંક iOff = 0;

  જો( zSql==0 ) પાછા SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  જ્યારે( 1 ){
    પૂર્ણાંક t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    જો( t==TK_LP ) તોડો;
    જો( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      પાછા SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  પાછા SQLITE_OK;
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
સ્થિર શૂન્ય dropConstraintFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  અચલ u8 *zSql = sqlite3_કિંમત_text(argv[0]);
  અચલ u8 *zCons = 0;
  પૂર્ણાંક iNotNull = -1;
  પૂર્ણાંક ii;
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક iStart = 0;
  પૂર્ણાંક iEnd = 0;
  અક્ષર *zNew = 0;
  પૂર્ણાંક t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  જો( zSql==0 ) પાછા;

  /* Jump past the "CREATE TABLE" bit. */
  જો( skipCreateTable(ctx, zSql, &iOff) ) પાછા;

  જો( sqlite3_કિંમત_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_કિંમત_int(argv[1]);
  }નહીંતર{
    zCons = sqlite3_કિંમત_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  માટે(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    જ્યારે( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      જો( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        પૂર્ણાંક nTok = 0;
        પૂર્ણાંક cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        જો( zCons ){
          જો( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) પાછા;
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
        જો( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }નહીંતર{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        જો( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          જો( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            પાછા;
          }
          iEnd = iOff;
          તોડો;
        }

      }નહીંતર જો( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        તોડો;
      }નહીંતર જો( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        તોડો;
      }નહીંતર જો( t==TK_COMMA ){
        તોડો;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  જો( iEnd<=0 ){
    જો( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }નહીંતર{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (અચલ અક્ષર*)zSql, -1, SQLITE_TRANSIENT);
    }
  }નહીંતર{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    અચલ અક્ષર *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    જો( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      જો( zSql[iStart-1]==',' ) iStart--;
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
સ્થિર શૂન્ય addConstraintFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  અચલ u8 *zSql = sqlite3_કિંમત_text(argv[0]);
  અચલ અક્ષર *zCons = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
  પૂર્ણાંક iCol = sqlite3_કિંમત_int(argv[2]);
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક ii;
  sqlite3_str *pNew;
  પૂર્ણાંક t = 0;
  UNUSED_PARAMETER(NotUsed);

  જો( skipCreateTable(ctx, zSql, &iOff) ) પાછા;
  
  માટે(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    જ્યારે( 1 ){
      પૂર્ણાંક nTok = getConstraintToken(&zSql[iOff], &t);
      જો( t==TK_COMMA || t==TK_RP ) તોડો;
      જો( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        પાછા;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (અચલ અક્ષર*)zSql, iOff);
  જો( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
સ્થિર પૂર્ણાંક alterFindCol(Parse *pParse, Table *pTab, Token *pCol, પૂર્ણાંક *piCol){
  sqlite3 *db = pParse->db;
  અક્ષર *zName = sqlite3NameFromToken(db, pCol);
  પૂર્ણાંક rc = SQLITE_NOMEM;
  પૂર્ણાંક iCol = -1;

  જો( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    જો( iCol<0 ){
      sqlite3_Error_સંદેશ(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }નહીંતર{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  જો( rc==SQLITE_OK ){
    અચલ અક્ષર *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    અચલ અક્ષર *zCol = pTab->aCol[iCol].zCnName;
    જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  પાછા rc;
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
સ્થિર Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  પૂર્ણાંક *piDb,            /* OUT: write the iDb here */
  અચલ અક્ષર **pzDb,    /* OUT: write name of schema here */
  પૂર્ણાંક bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  પ્રતિજ્ઞા( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  જો( pTab ){
    પૂર્ણાંક iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    જો( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  જો( pTab && bAuth ){
    જો( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  પાછા pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
શૂન્ય sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  પૂર્ણાંક iDb = 0;
  અચલ અક્ષર *zDb = 0;
  અક્ષર *zArg = 0;

  પ્રતિજ્ઞા( (pCol==0)!=(pCons==0) );
  પ્રતિજ્ઞા( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  જો( !pTab ) પાછા;

  જો( pCons ){
    અક્ષર *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }નહીંતર{
    પૂર્ણાંક iCol;
    જો( alterFindCol(pParse, pTab, pCol, &iCol) ) પાછા;
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
સ્થિર શૂન્ય failConstraintFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  અચલ અક્ષર *zText = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  પૂર્ણાંક err = sqlite3_કિંમત_int(argv[1]);
  (શૂન્ય)NotUsed;
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
સ્થિર પૂર્ણાંક alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  અચલ અક્ષર *pCons,              /* Buffer containing constraint */
  પૂર્ણાંક nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક iEnd = 0;

  જો( zTmp==0 ) પાછા 0;

  જ્યારે( 1 ){
    પૂર્ણાંક t = 0;
    પૂર્ણાંક nToken = sqlite3GetToken(&zTmp[iOff], &t);
    જો( t==TK_ILLEGAL ) તોડો;
    જો( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  પાછા iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
શૂન્ય sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  પૂર્ણાંક iCol = 0;
  પૂર્ણાંક iDb = 0;
  અચલ અક્ષર *zDb = 0;
  અચલ અક્ષર *pCons = 0;
  પૂર્ણાંક nCons = 0;

  /* Look up the table being altered. */
  પ્રતિજ્ઞા( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  જો( !pTab ) પાછા;

  /* Find the column being altered. */
  જો( alterFindCol(pParse, pTab, pCol, &iCol) ){
    પાછા;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (પૂર્ણાંક)pCol->n, pCol->z
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
સ્થિર શૂન્ય findConstraintFunc(
  sqlite3_context *ctx,
  પૂર્ણાંક NotUsed,
  sqlite3_કિંમત **argv
){
  અચલ u8 *zSql = 0;
  અચલ u8 *zCons = 0;
  પૂર્ણાંક iOff = 0;
  પૂર્ણાંક t = 0;

  (શૂન્ય)NotUsed;
  zSql = sqlite3_કિંમત_text(argv[0]);
  zCons = sqlite3_કિંમત_text(argv[1]);

  જો( zSql==0 || zCons==0 ) પાછા;
  જ્યારે( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  જ્યારે( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    જો( t==TK_CONSTRAINT ){
      પૂર્ણાંક nTok = 0;
      પૂર્ણાંક cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      જો( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) પાછા;
      જો( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        પાછા;
      }
    }નહીંતર જો( t==TK_ILLEGAL ){
      તોડો;
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
શૂન્ય sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  અચલ અક્ષર *z_પદાવલિ,       /* Text of CHECK expression */
  પૂર્ણાંક n_પદાવલિ,               /* Size of pExpr in bytes */
  પદાવલિ *p_પદાવલિ              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  પૂર્ણાંક iDb = 0;             /* Which schema does pTab live in */
  અચલ અક્ષર *zDb = 0;     /* Name of the schema in which pTab lives */
  અચલ અક્ષર *pCons = 0;   /* Text of the constraint */
  પૂર્ણાંક nCons;               /* Bytes of text to use from pCons[] */
  પૂર્ણાંક rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  પ્રતિજ્ઞા( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  જો( !pTab ){
    sqlite3_પદાવલિ_Delete(pParse->db, p_પદાવલિ);
    પાછા;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_પદાવલિ, 0);
  sqlite3_પદાવલિ_Delete(pParse->db, p_પદાવલિ);
  જો( rc ) પાછા;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  જો( pName ){
    અક્ષર *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_પદાવલિ, z_પદાવલિ
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
શૂન્ય sqlite3AlterFunctions(શૂન્ય){
  સ્થિર FuncDef aAlterTableFuncs[] = {
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
  sqlite3_ઉમેરો_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
