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
#ifndef SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ساكن صحيح isAlterableTable(Parse *pParse, Table *pTab){
  إذا( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_پەیام(pParse, "table %s may not be altered", pTab->zName);
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
  صحيح bTemp,                      /* True if this is the temp db */
  ثابت حرف *zWhen,              /* "when" part of error message */
  صحيح bNoDQS                      /* Do not allow DQS in the schema */
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

  إذا( bTemp==0 ){
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
ساكن فراغ renameFixQuotes(Parse *pParse, ثابت حرف *zDb, صحيح bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  إذا( bTemp==0 ){
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
ساكن فراغ renameReloadSchema(Parse *pParse, صحيح iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  إذا( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    إذا( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
فراغ sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  صحيح iDb;                  /* Database that contains the table */
  حرف *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  حرف *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  صحيح nTabName;             /* Number of UTF-8 characters in zTabName */
  ثابت حرف *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  إذا( NEVER(db->mallocFailed) ) اذهب exit_rename_table;
  أكد( pSrc->nSrc==1 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  إذا( !zName ) اذهب exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  إذا( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_پەیام(pParse,
        "there is already another table or index with this name: %s", zName);
    اذهب exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  إذا( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    اذهب exit_rename_table;
  }
  إذا( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    اذهب exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  إذا( IsView(pTab) ){
    sqlite3_Error_پەیام(pParse, "view %s may not be altered", pTab->zName);
    اذهب exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    اذهب exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  إذا( sqlite3ViewGetColumnNames(pParse, pTab) ){
    اذهب exit_rename_table;
  }
  إذا( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    إذا( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 ){
    اذهب exit_rename_table;
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
  إذا( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

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
#ifndef SQLITE_OMIT_VIRTUALTABLE
  إذا( pVTab ){
    صحيح i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(ثابت حرف*)pVTab, P4_VTAB);
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
ساكن فراغ sqlite3ErrorIfNotEmpty(
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
  Table *pTab, 
  ثابت حرف *zCol, 
  ثابت حرف *zOp
){
  إذا( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_پەیام(pParse, "cannot %s rowid alias: %s", zOp, zCol);
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
فراغ sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  صحيح iDb;                  /* Database number */
  ثابت حرف *zDb;          /* Database name */
  ثابت حرف *zTab;         /* Table name */
  حرف *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  دەربڕین *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  صحيح r1;                   /* Temporary registers */

  db = pParse->db;
  أكد( db->pParse==pParse );
  إذا( pParse->nErr ) أرجع;
  أكد( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  أكد( pNew );

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_دەربڕین(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  أكد( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    أرجع;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  إذا( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_پەیام(pParse, "Cannot add a PRIMARY KEY column");
    أرجع;
  }
  إذا( pNew->pIndex ){
    sqlite3_Error_پەیام(pParse,
         "Cannot add a UNIQUE column");
    أرجع;
  }
  إذا( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) أرجع;
  إذا( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    أكد( pDflt==0 || pDflt->op==TK_SPAN );
    إذا( pDflt && pDflt->p_چەپ->op==TK_NULL ){
      pDflt = 0;
    }
    أكد( IsOrdinaryTable(pNew) );
    إذا( (db->flags&SQLITE_Foreign_کلیلەکان) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    إذا( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    إذا( pDflt ){
      sqlite3_بەها *pVal = 0;
      صحيح rc;
      rc = sqlite3_بەها_From_دەربڕین(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      أكد( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      إذا( rc!=SQLITE_OK ){
        أكد( db->mallocFailed == 1 );
        أرجع;
      }
      إذا( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_بەها_Free(pVal);
    }
  }وإلا إذا( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (حرف*)pColDef->z, pColDef->n);
  إذا( zCol ){
    حرف *zEnd = &zCol[pColDef->n-1];
    طالما( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    أكد( IsOrdinaryTable(pTab) );
    أكد( IsOrdinaryTable(pNew) );
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
  إذا( v ){
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
    إذا( pNew->pCheck!=0
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
فراغ sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  صحيح iDb;
  صحيح i;
  صحيح nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  أكد( pParse->pNewTable==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  إذا( NEVER(db->mallocFailed) ) اذهب exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  إذا( IsVirtual(pTab) ){
    sqlite3_Error_پەیام(pParse, "virtual tables may not be altered");
    اذهب exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  إذا( IsView(pTab) ){
    sqlite3_Error_پەیام(pParse, "Cannot add a column to a view");
    اذهب exit_begin_add_column;
  }
  إذا( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    اذهب exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  أكد( IsOrdinaryTable(pTab) );
  أكد( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, حجم(Table));
  إذا( !pNew ) اذهب exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  أكد( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  أكد( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, حجم(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  إذا( !pNew->aCol || !pNew->zName ){
    أكد( db->mallocFailed );
    اذهب exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, حجم(Column)*(size_t)pNew->nCol);
  لكل(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  أكد( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_دەربڕین_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  أكد( pNew->nTabRef==1 );

exit_begin_add_column:
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
ساكن صحيح isRealTable(Parse *pParse, Table *pTab, صحيح iOp){
  ثابت حرف *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  إذا( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  إذا( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  إذا( zType ){
    ثابت حرف *az_پەیام[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    أكد( iOp>=0 && iOp<ArraySize(az_پەیام) );
    sqlite3_Error_پەیام(pParse, "cannot %s %s \"%s\"",
        az_پەیام[iOp], zType, pTab->zName
    );
    أرجع 1;
  }
  أرجع 0;
}
#وإلا /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
فراغ sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  صحيح iCol;                       /* Index of column being renamed */
  حرف *zOld = 0;                 /* Old column name */
  حرف *zNew = 0;                 /* New column name */
  ثابت حرف *zDb;                /* Name of schema containing the table */
  صحيح iSchema;                    /* Index of the schema */
  صحيح bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_rename_column;

  /* Cannot alter a system table */
  إذا( SQLITE_OK!=isAlterableTable(pParse, pTab) ) اذهب exit_rename_column;
  إذا( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) اذهب exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    اذهب exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  إذا( !zOld ) اذهب exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  إذا( iCol<0 ){
    sqlite3_Error_پەیام(pParse, "no such column: \"%T\"", pOld);
    اذهب exit_rename_column;
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
  إذا( !zNew ) اذهب exit_rename_column;
  إذا( isRowidAlias(pParse, pTab, zOld, "rename from") ) اذهب exit_rename_column;
  إذا( isRowidAlias(pParse, pTab, zNew, "rename to") ) اذهب exit_rename_column;
  أكد( pNew->n>0 );
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
  إذا( iSchema!=1 ){
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
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
تعريفنوع بنية RenameCtx RenameCtx;
بنية RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  صحيح nList;                      /* Number of tokens in pList */
  صحيح iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  ثابت حرف *zOld;               /* Old column name */
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
ساكن فراغ renameTokenCheckAll(Parse *pParse, ثابت فراغ *pPtr){
  أكد( pParse==pParse->db->pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  إذا( pParse->nErr==0 ){
    ثابت RenameToken *p;
    u32 i = 1;
    لكل(p=pParse->pRename; p; p=p->pNext){
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
ثابت فراغ *sqlite3RenameTokenMap(
  Parse *pParse,
  ثابت فراغ *pPtr,
  ثابت Token *pToken
){
  RenameToken *pNew;
  أكد( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  إذا( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, حجم(RenameToken));
    إذا( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
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
فراغ sqlite3RenameTokenRemap(Parse *pParse, ثابت فراغ *pTo, ثابت فراغ *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  لكل(p=pParse->pRename; p; p=p->pNext){
    إذا( p->p==pFrom ){
      p->p = pTo;
      اكسر;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ساكن صحيح rename_Unmap_دەربڕین_Cb(Walker *pWalker, دەربڕین *p_دەربڕین){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ثابت فراغ*)p_دەربڕین);
  إذا( دەربڕین_Use_YTab(p_دەربڕین) ){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت فراغ*)&p_دەربڕین->y.pTab);
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
      pCopy = sqlite3_With_پاڵنان(pParse, pCopy, 1);
    }
    لكل(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, حجم(sNC));
      sNC.pParse = pParse;
      إذا( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      إذا( sNC.pParse->db->mallocFailed ) أرجع;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    إذا( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ساكن فراغ unmapColumnIdlistNames(
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
    دەربڕین_List *pList = p->pEList;
    لكل(i=0; i<pList->n_دەربڕین; i++){
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
        sqlite3_Walk_دەربڕین(pWalker, pSrc->a[i].u3.pOn);
      }وإلا{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  أرجع WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
فراغ sqlite3_Rename_دەربڕین_Unmap(Parse *pParse, دەربڕین *p_دەربڕین){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = pParse;
  sWalker.x_دەربڕین_Callback = rename_Unmap_دەربڕین_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_دەربڕین(&sWalker, p_دەربڕین);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
فراغ sqlite3RenameExprlistUnmap(Parse *pParse, دەربڕین_List *pEList){
  إذا( pEList ){
    صحيح i;
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = pParse;
    sWalker.x_دەربڕین_Callback = rename_Unmap_دەربڕین_Cb;
    sqlite3_Walk_دەربڕین_List(&sWalker, pEList);
    لكل(i=0; i<pEList->n_دەربڕین; i++){
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
  RenameToken *pNext;
  RenameToken *p;
  لكل(p=pToken; p; p=pNext){
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
ساكن RenameToken *renameTokenFind(
  Parse *pParse,
  بنية RenameCtx *pCtx,
  ثابت فراغ *pPtr
){
  RenameToken **pp;
  إذا( NEVER(pPtr==0) ){
    أرجع 0;
  }
  لكل(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    إذا( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      إذا( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
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
ساكن صحيح renameColumnSelectCb(Walker *pWalker, Select *p){
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
ساكن صحيح rename_Column_دەربڕین_Cb(Walker *pWalker, دەربڕین *p_دەربڕین){
  RenameCtx *p = pWalker->u.pRename;
  إذا( p_دەربڕین->op==TK_TRIGGER
   && p_دەربڕین->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (فراغ*)p_دەربڕین);
  }وإلا إذا( p_دەربڕین->op==TK_COLUMN
   && p_دەربڕین->iColumn==p->iCol
   && ALWAYS(دەربڕین_Use_YTab(p_دەربڕین))
   && p->pTab==p_دەربڕین->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (فراغ*)p_دەربڕین);
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
ساكن RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  لكل(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    إذا( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  لكل(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  أرجع pBest;
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
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }وإلا{
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
ساكن فراغ renameColumnParseError(
  sqlite3_context *pCtx,
  ثابت حرف *zWhen,
  sqlite3_بەها *pType,
  sqlite3_بەها *pObject,
  Parse *pParse
){
  ثابت حرف *zT = (ثابت حرف*)sqlite3_بەها_text(pType);
  ثابت حرف *zN = (ثابت حرف*)sqlite3_بەها_text(pObject);
  حرف *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_پەیام
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
ساكن فراغ renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت دەربڕین_List *pEList,
  ثابت حرف *zOld
){
  إذا( pEList ){
    صحيح i;
    لكل(i=0; i<pEList->n_دەربڕین; i++){
      ثابت حرف *zName = pEList->a[i].zEName;
      إذا( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (ثابت فراغ*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ساكن فراغ renameColumnIdlistNames(
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
        renameTokenFind(pParse, pCtx, (ثابت فراغ*)zName);
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
  صحيح bTemp                       /* True if SQL is from temp schema */
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
  إذا( bTemp ){
    db->init.iDb = 1;
  }وإلا{
    صحيح iDb = sqlite3FindDbName(db, zDb);
    أكد( iDb>=0 && iDb<=0xff );
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
  إذا( db->mallocFailed ) rc = SQLITE_NOMEM;
  إذا( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  إذا( rc==SQLITE_OK ){
    صحيح nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    لكل(pToken=p->pRename; pToken; pToken=pToken->pNext){
      أكد( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

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
  حرف *zOut;
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
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }وإلا{
    أكد( nSql>0 );
    zOut = (حرف*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    إذا( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  إذا( zOut ){
    i64 nOut = nSql;
    أكد( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    طالما( pRename->pList ){
      صحيح iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ثابت حرف *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      إذا( zNew ){
        إذا( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }وإلا{
          nReplace = nQuot;
          zReplace = zQuot;
          إذا( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }وإلا{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        أكد( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((صحيح)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (صحيح)(pBest->t.z - zSql);
      إذا( pBest->t.n!=nReplace ){
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
  }وإلا{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  أرجع rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ساكن فراغ renameSetENames(دەربڕین_List *pEList, صحيح val){
  أكد( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  إذا( pEList ){
    صحيح i;
    لكل(i=0; i<pEList->n_دەربڕین; i++){
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
  TriggerStep *pStep;
  NameContext sNC;
  صحيح rc = SQLITE_OK;

  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  أكد( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  إذا( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  إذا( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_دەربڕین_Names(&sNC, pNew->pWhen);
  }

  لكل(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    إذا( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      إذا( pParse->nErr ) rc = pParse->rc;
    }
    إذا( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      إذا( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_دەربڕین_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        إذا( pSel==0 ){
          pStep->p_دەربڕین_List = 0;
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
          renameSetENames(pStep->p_دەربڕین_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_دەربڕین_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          أكد( pStep->p_دەربڕین_List==0 || pStep->p_دەربڕین_List==pSel->pEList );
          أكد( pSrc==pSel->pSrc );
          إذا( pStep->p_دەربڕین_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        إذا( ALWAYS(pStep->pSrc) ){
          صحيح i;
          لكل(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
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
        إذا( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_دەربڕین_Names(&sNC, pStep->pWhere);
        }
        إذا( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_دەربڕین_List_Names(&sNC, pStep->p_دەربڕین_List);
        }
        أكد( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_دەربڕین_List) );
        إذا( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_دەربڕین_List_Names(&sNC, pUpsert->pUpsertTarget);
          إذا( rc==SQLITE_OK ){
            دەربڕین_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_دەربڕین_List_Names(&sNC, pUpsertSet);
          }
          إذا( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_دەربڕین_Names(&sNC, pUpsert->pUpsertWhere);
          }
          إذا( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_دەربڕین_Names(&sNC, pUpsert->pUpsertTargetWhere);
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
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_دەربڕین(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  لكل(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_دەربڕین(pWalker, pStep->pWhere);
    sqlite3_Walk_دەربڕین_List(pWalker, pStep->p_دەربڕین_List);
    إذا( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_دەربڕین_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_دەربڕین_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_دەربڕین(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_دەربڕین(pWalker, pUpsert->pUpsertTargetWhere);
    }
    إذا( pStep->pSrc ){
      صحيح i;
      SrcList *pSrc = pStep->pSrc;
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
  Index *pIdx;
  إذا( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  طالما( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_پەیام);
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
ساكن فراغ renameColumnFunc(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_بەها_text(argv[0]);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_بەها_text(argv[3]);
  ثابت حرف *zTable = (ثابت حرف*)sqlite3_بەها_text(argv[4]);
  صحيح iCol = sqlite3_بەها_int(argv[5]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_بەها_text(argv[6]);
  صحيح bQuote = sqlite3_بەها_int(argv[7]);
  صحيح bTemp = sqlite3_بەها_int(argv[8]);
  ثابت حرف *zOld;
  صحيح rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  صحيح i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  إذا( zSql==0 ) أرجع;
  إذا( zTable==0 ) أرجع;
  إذا( zNew==0 ) أرجع;
  إذا( iCol<0 ) أرجع;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  إذا( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    أرجع;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_دەربڕین_Callback = rename_Column_دەربڕین_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  إذا( rc!=SQLITE_OK ) اذهب renameColumnFunc_done;
  إذا( sParse.pNewTable ){
    إذا( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      إذا( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      إذا( rc!=SQLITE_OK ) اذهب renameColumnFunc_done;
    }وإلا إذا( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      صحيح bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      إذا( bFKOnly==0 ){
        إذا( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (فراغ*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        إذا( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (فراغ*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_دەربڕین_List(&sWalker, sParse.pNewTable->pCheck);
        لكل(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_دەربڕین_List(&sWalker, pIdx->a_Col_دەربڕین);
        }
        لكل(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_دەربڕین_List(&sWalker, pIdx->a_Col_دەربڕین);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        لكل(i=0; i<sParse.pNewTable->nCol; i++){
          دەربڕین *p_دەربڕین = sqlite3_Column_دەربڕین(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_دەربڕین(&sWalker, p_دەربڕین);
        }
#endif
      }

      أكد( IsOrdinaryTable(sParse.pNewTable) );
      لكل(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        لكل(i=0; i<pFKey->nCol; i++){
          إذا( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (فراغ*)&pFKey->aCol[i]);
          }
          إذا( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (فراغ*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }وإلا إذا( sParse.pNewIndex ){
    sqlite3_Walk_دەربڕین_List(&sWalker, sParse.pNewIndex->a_Col_دەربڕین);
    sqlite3_Walk_دەربڕین(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }وإلا{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    إذا( rc!=SQLITE_OK ) اذهب renameColumnFunc_done;

    لكل(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      إذا( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        إذا( pTarget==pTab ){
          إذا( pStep->pUpsert ){
            دەربڕین_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_دەربڕین_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    إذا( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  أكد( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  إذا( rc!=SQLITE_OK ){
    إذا( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_بەها(context, argv[0]);
    }وإلا إذا( sParse.z_Err_پەیام ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }وإلا{
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
ساكن صحيح rename_Table_دەربڕین_Cb(Walker *pWalker, دەربڕین *p_دەربڕین){
  RenameCtx *p = pWalker->u.pRename;
  إذا( p_دەربڕین->op==TK_COLUMN
   && ALWAYS(دەربڕین_Use_YTab(p_دەربڕین))
   && p->pTab==p_دەربڕین->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (فراغ*)&p_دەربڕین->y.pTab);
  }
  أرجع WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ساكن صحيح renameTableSelectCb(Walker *pWalker, Select *pSelect){
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
    SrcItem *pItem = &pSrc->a[i];
    إذا( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
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
ساكن فراغ renameTableFunc(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_بەها_text(argv[0]);
  ثابت حرف *z_تێخستن = (ثابت حرف*)sqlite3_بەها_text(argv[3]);
  ثابت حرف *zOld = (ثابت حرف*)sqlite3_بەها_text(argv[4]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_بەها_text(argv[5]);
  صحيح bTemp = sqlite3_بەها_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  إذا( z_تێخستن && zOld && zNew ){
    Parse sParse;
    صحيح rc;
    صحيح bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, حجم(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_دەربڕین_Callback = rename_Table_دەربڕین_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_تێخستن, bTemp);

    إذا( rc==SQLITE_OK ){
      صحيح isLegacy = (db->flags & SQLITE_LegacyAlter);
      إذا( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

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
#ifndef SQLITE_OMIT_FOREIGN_کلیل
          إذا( (isLegacy==0 || (db->flags & SQLITE_Foreign_کلیلەکان))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            أكد( IsOrdinaryTable(pTab) );
            لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              إذا( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (فراغ*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          إذا( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            إذا( isLegacy==0 ){
              sqlite3_Walk_دەربڕین_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      وإلا إذا( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        إذا( isLegacy==0 ){
          sqlite3_Walk_دەربڕین(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      وإلا{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        إذا( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        إذا( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          إذا( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            لكل(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              إذا( pStep->pSrc ){
                صحيح i;
                لكل(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  إذا( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    إذا( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_تێخستن, zNew, bQuote);
    }
    إذا( rc!=SQLITE_OK ){
      إذا( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_بەها(context, argv[3]);
      }وإلا إذا( sParse.z_Err_پەیام ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }وإلا{
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

  أرجع;
}

ساكن صحيح rename_Quotefix_دەربڕین_Cb(Walker *pWalker, دەربڕین *p_دەربڕین){
  إذا( p_دەربڕین->op==TK_STRING && (p_دەربڕین->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (ثابت فراغ*)p_دەربڕین);
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
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_بەها_text(argv[0]);
  حرف ثابت *z_تێخستن = (ثابت حرف*)sqlite3_بەها_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  إذا( zDb && z_تێخستن ){
    صحيح rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_تێخستن, 0);

    إذا( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, حجم(RenameCtx));
      memset(&sWalker, 0, حجم(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_دەربڕین_Callback = rename_Quotefix_دەربڕین_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      إذا( sParse.pNewTable ){
        إذا( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          إذا( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }وإلا{
          صحيح i;
          sqlite3_Walk_دەربڕین_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          لكل(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_دەربڕین(&sWalker,
               sqlite3_Column_دەربڕین(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }وإلا إذا( sParse.pNewIndex ){
        sqlite3_Walk_دەربڕین_List(&sWalker, sParse.pNewIndex->a_Col_دەربڕین);
        sqlite3_Walk_دەربڕین(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }وإلا{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        إذا( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      إذا( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_تێخستن, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    إذا( rc!=SQLITE_OK ){
      إذا( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_بەها(context, argv[1]);
      }وإلا{
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
ساكن فراغ renameTableTest(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_بەها_text(argv[0]);
  حرف ثابت *z_تێخستن = (ثابت حرف*)sqlite3_بەها_text(argv[1]);
  صحيح bTemp = sqlite3_بەها_int(argv[4]);
  صحيح isLegacy = (db->flags & SQLITE_LegacyAlter);
  حرف ثابت *zWhen = (ثابت حرف*)sqlite3_بەها_text(argv[5]);
  صحيح bNoDQS = sqlite3_بەها_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  إذا( zDb && z_تێخستن ){
    صحيح rc;
    Parse sParse;
    u64 flags = db->flags;
    إذا( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_تێخستن, bTemp);
    db->flags = flags;
    إذا( rc==SQLITE_OK ){
      إذا( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, حجم(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        إذا( sParse.nErr ) rc = sParse.rc;
      }

      وإلا إذا( sParse.pNewTrigger ){
        إذا( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        إذا( rc==SQLITE_OK ){
          صحيح i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          صحيح i2 = sqlite3FindDbName(db, zDb);
          إذا( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    إذا( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
ساكن فراغ dropColumnFunc(
  sqlite3_context *context,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحيح iSchema = sqlite3_بەها_int(argv[0]);
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_بەها_text(argv[1]);
  صحيح iCol = sqlite3_بەها_int(argv[2]);
  ثابت حرف *zDb = db->aDb[iSchema].zDbSName;
  صحيح rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  ثابت حرف *zEnd;
  حرف *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  إذا( rc!=SQLITE_OK ) اذهب drop_column_done;
  pTab = sParse.pNewTable;
  إذا( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    اذهب drop_column_done;
  }

  إذا( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (فراغ*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (فراغ*)pTab->aCol[iCol+1].zCnName);
    zEnd = (ثابت حرف*)pEnd->t.z;
  }وإلا{
    صحيح eTok;
    أكد( IsOrdinaryTable(pTab) );
    أكد( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (فراغ*)pTab->aCol[iCol-1].zCnName);
    افعل {
      pCol->t.z += getConstraintToken((ثابت u8*)pCol->t.z, &eTok);
    }طالما( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (ثابت حرف*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  إذا( rc!=SQLITE_OK ){
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
فراغ sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, ثابت Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  صحيح iDb;                        /* Index of db containing pTab in aDb[] */
  ثابت حرف *zDb;                /* Database containing pTab ("main" etc.) */
  حرف *zCol = 0;                 /* Name of column to drop */
  صحيح iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  أكد( pParse->pNewTable==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  إذا( NEVER(db->mallocFailed) ) اذهب exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  إذا( !pTab ) اذهب exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  إذا( SQLITE_OK!=isAlterableTable(pParse, pTab) ) اذهب exit_drop_column;
  إذا( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) اذهب exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  إذا( zCol==0 ){
    أكد( db->mallocFailed );
    اذهب exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  إذا( iCol<0 ){
    sqlite3_Error_پەیام(pParse, "no such column: \"%T\"", pName);
    اذهب exit_drop_column;
  }
  إذا( isRowidAlias(pParse, pTab, zCol, "drop") ) اذهب exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  إذا( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_پەیام(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    اذهب exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  إذا( pTab->nCol<=1 ){
    sqlite3_Error_پەیام(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    اذهب exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    اذهب exit_drop_column;
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
  إذا( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    صحيح i;
    صحيح addr;
    صحيح reg;
    صحيح regRec;
    Index *pPk = 0;
    صحيح nField = 0;               /* Number of non-virtual columns after drop */
    صحيح iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    إذا( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }وإلا{
      pPk = sqlite3_Primary_کلیل_Index(pTab);
      pParse->nMem += pPk->nColumn;
      لكل(i=0; i<pPk->n_کلیل_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_کلیل_Col;
    }
    regRec = ++pParse->nMem;
    لكل(i=0; i<pTab->nCol; i++){
      إذا( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        صحيح regOut;
        إذا( pPk ){
          صحيح iPos = sqlite3TableColumnToIndex(pPk, i);
          صحيح iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          إذا( iPos<pPk->n_کلیل_Col ) استمر;
          regOut = reg+1+iPos-(iPos>iColPos);
        }وإلا{
          regOut = reg+1+nField;
        }
        إذا( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }وإلا{
          sqlite3_دەربڕین_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          أكد( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    إذا( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    إذا( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_زیادکردن, iCur, regRec, reg+1, pPk->n_کلیل_Col);
    }وإلا{
      sqlite3VdbeAddOp3(v, OP_زیادکردن, iCur, regRec, reg);
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

#ifndef SQLITE_OMIT_FOREIGN_کلیل
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
#endif

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
    sqlite3_result_error_nomem(ctx);
    أرجع SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
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
ساكن صحيح skipCreateTable(sqlite3_context *ctx, ثابت u8 *zSql, صحيح *piOff){
  صحيح iOff = 0;

  إذا( zSql==0 ) أرجع SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  طالما( 1 ){
    صحيح t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    إذا( t==TK_LP ) اكسر;
    إذا( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
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
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  ثابت u8 *zSql = sqlite3_بەها_text(argv[0]);
  ثابت u8 *zCons = 0;
  صحيح iNotNull = -1;
  صحيح ii;
  صحيح iOff = 0;
  صحيح iStart = 0;
  صحيح iEnd = 0;
  حرف *zNew = 0;
  صحيح t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  إذا( zSql==0 ) أرجع;

  /* Jump past the "CREATE TABLE" bit. */
  إذا( skipCreateTable(ctx, zSql, &iOff) ) أرجع;

  إذا( sqlite3_بەها_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_بەها_int(argv[1]);
  }وإلا{
    zCons = sqlite3_بەها_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  لكل(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    طالما( 1 ){
      iStart = iOff;
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
          iEnd = iOff;
          اكسر;
        }

      }وإلا إذا( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        اكسر;
      }وإلا إذا( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        اكسر;
      }وإلا إذا( t==TK_COMMA ){
        اكسر;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  إذا( iEnd<=0 ){
    إذا( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }وإلا{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (ثابت حرف*)zSql, -1, SQLITE_TRANSIENT);
    }
  }وإلا{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ثابت حرف *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    إذا( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      إذا( zSql[iStart-1]==',' ) iStart--;
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
ساكن فراغ addConstraintFunc(
  sqlite3_context *ctx,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  ثابت u8 *zSql = sqlite3_بەها_text(argv[0]);
  ثابت حرف *zCons = (ثابت حرف*)sqlite3_بەها_text(argv[1]);
  صحيح iCol = sqlite3_بەها_int(argv[2]);
  صحيح iOff = 0;
  صحيح ii;
  sqlite3_str *pNew;
  صحيح t = 0;
  UNUSED_PARAMETER(NotUsed);

  إذا( skipCreateTable(ctx, zSql, &iOff) ) أرجع;
  
  لكل(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    طالما( 1 ){
      صحيح nTok = getConstraintToken(&zSql[iOff], &t);
      إذا( t==TK_COMMA || t==TK_RP ) اكسر;
      إذا( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        أرجع;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ثابت حرف*)zSql, iOff);
  إذا( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ساكن صحيح alterFindCol(Parse *pParse, Table *pTab, Token *pCol, صحيح *piCol){
  sqlite3 *db = pParse->db;
  حرف *zName = sqlite3NameFromToken(db, pCol);
  صحيح rc = SQLITE_NOMEM;
  صحيح iCol = -1;

  إذا( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    إذا( iCol<0 ){
      sqlite3_Error_پەیام(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }وإلا{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  إذا( rc==SQLITE_OK ){
    ثابت حرف *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    ثابت حرف *zCol = pTab->aCol[iCol].zCnName;
    إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
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
ساكن Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  صحيح *piDb,            /* OUT: write the iDb here */
  ثابت حرف **pzDb,    /* OUT: write name of schema here */
  صحيح bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  إذا( pTab ){
    صحيح iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    إذا( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  إذا( pTab && bAuth ){
    إذا( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
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
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  صحيح iDb = 0;
  ثابت حرف *zDb = 0;
  حرف *zArg = 0;

  أكد( (pCol==0)!=(pCons==0) );
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  إذا( !pTab ) أرجع;

  إذا( pCons ){
    حرف *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }وإلا{
    صحيح iCol;
    إذا( alterFindCol(pParse, pTab, pCol, &iCol) ) أرجع;
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
ساكن فراغ failConstraintFunc(
  sqlite3_context *ctx,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  ثابت حرف *zText = (ثابت حرف*)sqlite3_بەها_text(argv[0]);
  صحيح err = sqlite3_بەها_int(argv[1]);
  (فراغ)NotUsed;
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
ساكن صحيح alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ثابت حرف *pCons,              /* Buffer containing constraint */
  صحيح nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  صحيح iOff = 0;
  صحيح iEnd = 0;

  إذا( zTmp==0 ) أرجع 0;

  طالما( 1 ){
    صحيح t = 0;
    صحيح nToken = sqlite3GetToken(&zTmp[iOff], &t);
    إذا( t==TK_ILLEGAL ) اكسر;
    إذا( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  أرجع iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
فراغ sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  صحيح iCol = 0;
  صحيح iDb = 0;
  ثابت حرف *zDb = 0;
  ثابت حرف *pCons = 0;
  صحيح nCons = 0;

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  إذا( !pTab ) أرجع;

  /* Find the column being altered. */
  إذا( alterFindCol(pParse, pTab, pCol, &iCol) ){
    أرجع;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (صحيح)pCol->n, pCol->z
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
ساكن فراغ findConstraintFunc(
  sqlite3_context *ctx,
  صحيح NotUsed,
  sqlite3_بەها **argv
){
  ثابت u8 *zSql = 0;
  ثابت u8 *zCons = 0;
  صحيح iOff = 0;
  صحيح t = 0;

  (فراغ)NotUsed;
  zSql = sqlite3_بەها_text(argv[0]);
  zCons = sqlite3_بەها_text(argv[1]);

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
        sqlite3_result_int(ctx, 1);
        أرجع;
      }
    }وإلا إذا( t==TK_ILLEGAL ){
      اكسر;
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
فراغ sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ثابت حرف *z_دەربڕین,       /* Text of CHECK expression */
  صحيح n_دەربڕین,               /* Size of pExpr in bytes */
  دەربڕین *p_دەربڕین              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  صحيح iDb = 0;             /* Which schema does pTab live in */
  ثابت حرف *zDb = 0;     /* Name of the schema in which pTab lives */
  ثابت حرف *pCons = 0;   /* Text of the constraint */
  صحيح nCons;               /* Bytes of text to use from pCons[] */
  صحيح rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  إذا( !pTab ){
    sqlite3_دەربڕین_Delete(pParse->db, p_دەربڕین);
    أرجع;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_دەربڕین, 0);
  sqlite3_دەربڕین_Delete(pParse->db, p_دەربڕین);
  إذا( rc ) أرجع;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  إذا( pName ){
    حرف *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_دەربڕین, z_دەربڕین
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
فراغ sqlite3AlterFunctions(فراغ){
  ساكن FuncDef aAlterTableFuncs[] = {
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
  sqlite3_زیادکردن_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
