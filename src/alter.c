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
#include "sqliteInt.h"

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
static පූර්ණ_සංඛ්‍යා isAlterableTable(Parse *pParse, Table *pTab){
  නම්( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3ErrorMsg(pParse, "table %s may not be altered", pTab->zName);
    ආපසු 1;
  }
  ආපසු 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
static void renameTestSchema(
  Parse *pParse,                  /* Parse context */
  const var_na *zDb,                /* Name of db to verify schema of */
  පූර්ණ_සංඛ්‍යා bTemp,                      /* True if this is the temp db */
  const var_na *zWhen,              /* "when" part of error message */
  පූර්ණ_සංඛ්‍යා bNoDQS                      /* Do not allow DQS in the schema */
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

  නම්( bTemp==0 ){
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
static void renameFixQuotes(Parse *pParse, const var_na *zDb, පූර්ණ_සංඛ්‍යා bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  නම්( bTemp==0 ){
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
static void renameReloadSchema(Parse *pParse, පූර්ණ_සංඛ්‍යා iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  නම්( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    නම්( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
void sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  පූර්ණ_සංඛ්‍යා iDb;                  /* Database that contains the table */
  var_na *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  var_na *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  පූර්ණ_සංඛ්‍යා nTabName;             /* Number of UTF-8 characters in zTabName */
  const var_na *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  නම්( NEVER(db->mallocFailed) ) goto exit_rename_table;
  assert( pSrc->nSrc==1 );
  assert( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  නම්( !pTab ) goto exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  නම්( !zName ) goto exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  නම්( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3ErrorMsg(pParse,
        "there is already another table or index with this name: %s", zName);
    goto exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  නම්( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    goto exit_rename_table;
  }
  නම්( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    goto exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  නම්( IsView(pTab) ){
    sqlite3ErrorMsg(pParse, "view %s may not be altered", pTab->zName);
    goto exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    goto exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  නම්( sqlite3ViewGetColumnNames(pParse, pTab) ){
    goto exit_rename_table;
  }
  නම්( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    නම්( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  නම්( v==0 ){
    goto exit_rename_table;
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
  නම්( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  නම්( iDb!=1 ){
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
  නම්( pVTab ){
    පූර්ණ_සංඛ්‍යා i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(const var_na*)pVTab, P4_VTAB);
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
static void sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  const var_na *zDb,      /* Schema holding the table */
  const var_na *zTab,     /* Table to check for empty */
  const var_na *zErr      /* Error message text */
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
static පූර්ණ_සංඛ්‍යා isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  const var_na *zCol, 
  const var_na *zOp
){
  නම්( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3ErrorMsg(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    ආපසු SQLITE_ERROR;
  }
  ආපසු SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
void sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  පූර්ණ_සංඛ්‍යා iDb;                  /* Database number */
  const var_na *zDb;          /* Database name */
  const var_na *zTab;         /* Table name */
  var_na *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  Expr *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  පූර්ණ_සංඛ්‍යා r1;                   /* Temporary registers */

  db = pParse->db;
  assert( db->pParse==pParse );
  නම්( pParse->nErr ) ආපසු;
  assert( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  assert( pNew );

  assert( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3ColumnExpr(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  assert( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    ආපසු;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  නම්( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3ErrorMsg(pParse, "Cannot add a PRIMARY KEY column");
    ආපසු;
  }
  නම්( pNew->pIndex ){
    sqlite3ErrorMsg(pParse,
         "Cannot add a UNIQUE column");
    ආපසු;
  }
  නම්( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) ආපසු;
  නම්( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    assert( pDflt==0 || pDflt->op==TK_SPAN );
    නම්( pDflt && pDflt->pLeft->op==TK_NULL ){
      pDflt = 0;
    }
    assert( IsOrdinaryTable(pNew) );
    නම්( (db->flags&SQLITE_ForeignKeys) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    නම්( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    නම්( pDflt ){
      sqlite3_value *pVal = 0;
      පූර්ණ_සංඛ්‍යා rc;
      rc = sqlite3ValueFromExpr(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      assert( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      නම්( rc!=SQLITE_OK ){
        assert( db->mallocFailed == 1 );
        ආපසු;
      }
      නම්( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3ValueFree(pVal);
    }
  }නැතහොත් නම්( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (var_na*)pColDef->z, pColDef->n);
  නම්( zCol ){
    var_na *zEnd = &zCol[pColDef->n-1];
    අතරතුර( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    assert( IsOrdinaryTable(pTab) );
    assert( IsOrdinaryTable(pNew) );
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
  නම්( v ){
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
    නම්( pNew->pCheck!=0
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
void sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  පූර්ණ_සංඛ්‍යා iDb;
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  assert( pParse->pNewTable==0 );
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  නම්( NEVER(db->mallocFailed) ) goto exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  නම්( !pTab ) goto exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  නම්( IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "virtual tables may not be altered");
    goto exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  නම්( IsView(pTab) ){
    sqlite3ErrorMsg(pParse, "Cannot add a column to a view");
    goto exit_begin_add_column;
  }
  නම්( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    goto exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);
  assert( IsOrdinaryTable(pTab) );
  assert( pTab->u.tab.addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, parimaa_na(Table));
  නම්( !pNew ) goto exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  assert( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  assert( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, parimaa_na(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  නම්( !pNew->aCol || !pNew->zName ){
    assert( db->mallocFailed );
    goto exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, parimaa_na(Column)*(size_t)pNew->nCol);
  සඳහා(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  assert( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3ExprListDup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  assert( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  ආපසු;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#නම් !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
static පූර්ණ_සංඛ්‍යා isRealTable(Parse *pParse, Table *pTab, පූර්ණ_සංඛ්‍යා iOp){
  const var_na *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  නම්( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  නම්( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  නම්( zType ){
    const var_na *azMsg[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    assert( iOp>=0 && iOp<ArraySize(azMsg) );
    sqlite3ErrorMsg(pParse, "cannot %s %s \"%s\"",
        azMsg[iOp], zType, pTab->zName
    );
    ආපසු 1;
  }
  ආපසු 0;
}
#නැතහොත් /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
void sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  පූර්ණ_සංඛ්‍යා iCol;                       /* Index of column being renamed */
  var_na *zOld = 0;                 /* Old column name */
  var_na *zNew = 0;                 /* New column name */
  const var_na *zDb;                /* Name of schema containing the table */
  පූර්ණ_සංඛ්‍යා iSchema;                    /* Index of the schema */
  පූර්ණ_සංඛ්‍යා bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  නම්( !pTab ) goto exit_rename_column;

  /* Cannot alter a system table */
  නම්( SQLITE_OK!=isAlterableTable(pParse, pTab) ) goto exit_rename_column;
  නම්( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) goto exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    goto exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  නම්( !zOld ) goto exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  නම්( iCol<0 ){
    sqlite3ErrorMsg(pParse, "no such column: \"%T\"", pOld);
    goto exit_rename_column;
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
  නම්( !zNew ) goto exit_rename_column;
  නම්( isRowidAlias(pParse, pTab, zOld, "rename from") ) goto exit_rename_column;
  නම්( isRowidAlias(pParse, pTab, zNew, "rename to") ) goto exit_rename_column;
  assert( pNew->n>0 );
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
  නම්( iSchema!=1 ){
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
  ආපසු;
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
sa_nracanaa RenameToken {
  const void *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
typedef sa_nracanaa RenameCtx RenameCtx;
sa_nracanaa RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  පූර්ණ_සංඛ්‍යා nList;                      /* Number of tokens in pList */
  පූර්ණ_සංඛ්‍යා iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  const var_na *zOld;               /* Old column name */
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
static void renameTokenCheckAll(Parse *pParse, const void *pPtr){
  assert( pParse==pParse->db->pParse );
  assert( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  නම්( pParse->nErr==0 ){
    const RenameToken *p;
    u32 i = 1;
    සඳහා(p=pParse->pRename; p; p=p->pNext){
      නම්( p->p ){
        assert( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    assert( i>0 );
  }
}
#නැතහොත්
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
const void *sqlite3RenameTokenMap(
  Parse *pParse,
  const void *pPtr,
  const Token *pToken
){
  RenameToken *pNew;
  assert( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  නම්( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, parimaa_na(RenameToken));
    නම්( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  ආපසු pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
void sqlite3RenameTokenRemap(Parse *pParse, const void *pTo, const void *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  සඳහා(p=pParse->pRename; p; p=p->pNext){
    නම්( p->p==pFrom ){
      p->p = pTo;
      bha_nga;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
static පූර්ණ_සංඛ්‍යා renameUnmapExprCb(Walker *pWalker, Expr *pExpr){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (const void*)pExpr);
  නම්( ExprUseYTab(pExpr) ){
    sqlite3RenameTokenRemap(pParse, 0, (const void*)&pExpr->y.pTab);
  }
  ආපසු WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
static void renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  නම්( pWith ){
    Parse *pParse = pWalker->pParse;
    පූර්ණ_සංඛ්‍යා i;
    With *pCopy = 0;
    assert( pWith->nCte>0 );
    නම්( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3WithPush(pParse, pCopy, 1);
    }
    සඳහා(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, parimaa_na(sNC));
      sNC.pParse = pParse;
      නම්( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      නම්( sNC.pParse->db->mallocFailed ) ආපසු;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    නම්( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
static void unmapColumnIdlistNames(
  Parse *pParse,
  const IdList *pIdList
){
  පූර්ණ_සංඛ්‍යා ii;
  assert( pIdList!=0 );
  සඳහා(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (const void*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
static පූර්ණ_සංඛ්‍යා renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  පූර්ණ_සංඛ්‍යා i;
  නම්( pParse->nErr ) ආපසු WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  නම්( p->selFlags & (SF_View|SF_CopyCte) ){
    ආපසු WRC_Prune;
  }
  නම්( ALWAYS(p->pEList) ){
    ExprList *pList = p->pEList;
    සඳහා(i=0; i<pList->nExpr; i++){
      නම්( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (void*)pList->a[i].zEName);
      }
    }
  }
  නම්( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    සඳහා(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (void*)pSrc->a[i].zName);
      නම්( pSrc->a[i].fg.isUsing==0 ){
        sqlite3WalkExpr(pWalker, pSrc->a[i].u3.pOn);
      }නැතහොත්{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  ආපසු WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
void sqlite3RenameExprUnmap(Parse *pParse, Expr *pExpr){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, parimaa_na(Walker));
  sWalker.pParse = pParse;
  sWalker.xExprCallback = renameUnmapExprCb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3WalkExpr(&sWalker, pExpr);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
void sqlite3RenameExprlistUnmap(Parse *pParse, ExprList *pEList){
  නම්( pEList ){
    පූර්ණ_සංඛ්‍යා i;
    Walker sWalker;
    memset(&sWalker, 0, parimaa_na(Walker));
    sWalker.pParse = pParse;
    sWalker.xExprCallback = renameUnmapExprCb;
    sqlite3WalkExprList(&sWalker, pEList);
    සඳහා(i=0; i<pEList->nExpr; i++){
      නම්( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (void*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
static void renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  සඳහා(p=pToken; p; p=pNext){
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
static RenameToken *renameTokenFind(
  Parse *pParse,
  sa_nracanaa RenameCtx *pCtx,
  const void *pPtr
){
  RenameToken **pp;
  නම්( NEVER(pPtr==0) ){
    ආපසු 0;
  }
  සඳහා(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    නම්( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      නම්( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      ආපසු pToken;
    }
  }
  ආපසු 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
static පූර්ණ_සංඛ්‍යා renameColumnSelectCb(Walker *pWalker, Select *p){
  නම්( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    ආපසු WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  ආපසු WRC_Continue;
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
static පූර්ණ_සංඛ්‍යා renameColumnExprCb(Walker *pWalker, Expr *pExpr){
  RenameCtx *p = pWalker->u.pRename;
  නම්( pExpr->op==TK_TRIGGER
   && pExpr->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (void*)pExpr);
  }නැතහොත් නම්( pExpr->op==TK_COLUMN
   && pExpr->iColumn==p->iCol
   && ALWAYS(ExprUseYTab(pExpr))
   && p->pTab==pExpr->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (void*)pExpr);
  }
  ආපසු WRC_Continue;
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
static RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  සඳහා(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    නම්( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  සඳහා(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  ආපසු pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
static void errorMPrintf(sqlite3_context *pCtx, const var_na *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  var_na *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  නම්( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }නැතහොත්{
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
static void renameColumnParseError(
  sqlite3_context *pCtx,
  const var_na *zWhen,
  sqlite3_value *pType,
  sqlite3_value *pObject,
  Parse *pParse
){
  const var_na *zT = (const var_na*)sqlite3_value_text(pType);
  const var_na *zN = (const var_na*)sqlite3_value_text(pObject);
  var_na *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->zErrMsg
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
static void renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  const ExprList *pEList,
  const var_na *zOld
){
  නම්( pEList ){
    පූර්ණ_සංඛ්‍යා i;
    සඳහා(i=0; i<pEList->nExpr; i++){
      const var_na *zName = pEList->a[i].zEName;
      නම්( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (const void*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
static void renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  const IdList *pIdList,
  const var_na *zOld
){
  නම්( pIdList ){
    පූර්ණ_සංඛ්‍යා i;
    සඳහා(i=0; i<pIdList->nId; i++){
      const var_na *zName = pIdList->a[i].zName;
      නම්( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (const void*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
static පූර්ණ_සංඛ්‍යා renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  const var_na *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  const var_na *zSql,               /* SQL to parse */
  පූර්ණ_සංඛ්‍යා bTemp                       /* True if SQL is from temp schema */
){
  පූර්ණ_සංඛ්‍යා rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  නම්( zSql==0 ){
    ආපසු SQLITE_NOMEM;
  }
  නම්( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    ආපසු SQLITE_CORRUPT_BKPT;
  }
  නම්( bTemp ){
    db->init.iDb = 1;
  }නැතහොත්{
    පූර්ණ_සංඛ්‍යා iDb = sqlite3FindDbName(db, zDb);
    assert( iDb>=0 && iDb<=0xff );
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
  නම්( db->mallocFailed ) rc = SQLITE_NOMEM;
  නම්( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  නම්( rc==SQLITE_OK ){
    පූර්ණ_සංඛ්‍යා nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    සඳහා(pToken=p->pRename; pToken; pToken=pToken->pNext){
      assert( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  ආපසු rc;
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
static පූර්ණ_සංඛ්‍යා renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  const var_na *zSql,               /* SQL statement to edit */
  const var_na *zNew,               /* New token text */
  පූර්ණ_සංඛ්‍යා bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;
  var_na *zQuot = 0;
  var_na *zOut;
  i64 nQuot = 0;
  var_na *zBuf1 = 0;
  var_na *zBuf2 = 0;

  නම්( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    නම්( zQuot==0 ){
      ආපසු SQLITE_NOMEM;
    }නැතහොත්{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    assert( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }නැතහොත්{
    assert( nSql>0 );
    zOut = (var_na*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    නම්( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  නම්( zOut ){
    i64 nOut = nSql;
    assert( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    අතරතුර( pRename->pList ){
      පූර්ණ_සංඛ්‍යා iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      const var_na *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      නම්( zNew ){
        නම්( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }නැතහොත්{
          nReplace = nQuot;
          zReplace = zQuot;
          නම්( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }නැතහොත්{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);
        assert( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((පූර්ණ_සංඛ්‍යා)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (පූර්ණ_සංඛ්‍යා)(pBest->t.z - zSql);
      නම්( pBest->t.n!=nReplace ){
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
  }නැතහොත්{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  ආපසු rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
static void renameSetENames(ExprList *pEList, පූර්ණ_සංඛ්‍යා val){
  assert( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  නම්( pEList ){
    පූර්ණ_සංඛ්‍යා i;
    සඳහා(i=0; i<pEList->nExpr; i++){
      assert( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
static පූර්ණ_සංඛ්‍යා renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;

  memset(&sNC, 0, parimaa_na(sNC));
  sNC.pParse = pParse;
  assert( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  නම්( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  නම්( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3ResolveExprNames(&sNC, pNew->pWhen);
  }

  සඳහා(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    නම්( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      නම්( pParse->nErr ) rc = pParse->rc;
    }
    නම්( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      නම්( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->pExprList, pSrc, 0, 0, 0, 0, 0, 0
        );
        නම්( pSel==0 ){
          pStep->pExprList = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }නැතහොත්{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->pExprList, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->pExprList, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          assert( pStep->pExprList==0 || pStep->pExprList==pSel->pEList );
          assert( pSrc==pSel->pSrc );
          නම්( pStep->pExprList ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        නම්( ALWAYS(pStep->pSrc) ){
          පූර්ණ_සංඛ්‍යා i;
          සඳහා(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            නම්( p->fg.isSubquery ){
              assert( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        නම්(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        නම්( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3ResolveExprNames(&sNC, pStep->pWhere);
        }
        නම්( rc==SQLITE_OK ){
          rc = sqlite3ResolveExprListNames(&sNC, pStep->pExprList);
        }
        assert( !pStep->pUpsert || (!pStep->pWhere && !pStep->pExprList) );
        නම්( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3ResolveExprListNames(&sNC, pUpsert->pUpsertTarget);
          නම්( rc==SQLITE_OK ){
            ExprList *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3ResolveExprListNames(&sNC, pUpsertSet);
          }
          නම්( rc==SQLITE_OK ){
            rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertWhere);
          }
          නම්( rc==SQLITE_OK ){
            rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }නැතහොත්{
        rc = SQLITE_NOMEM;
      }
    }
  }
  ආපසු rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
static void renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3WalkExpr(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  සඳහා(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3WalkExpr(pWalker, pStep->pWhere);
    sqlite3WalkExprList(pWalker, pStep->pExprList);
    නම්( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3WalkExprList(pWalker, pUpsert->pUpsertTarget);
      sqlite3WalkExprList(pWalker, pUpsert->pUpsertSet);
      sqlite3WalkExpr(pWalker, pUpsert->pUpsertWhere);
      sqlite3WalkExpr(pWalker, pUpsert->pUpsertTargetWhere);
    }
    නම්( pStep->pSrc ){
      පූර්ණ_සංඛ්‍යා i;
      SrcList *pSrc = pStep->pSrc;
      සඳහා(i=0; i<pSrc->nSrc; i++){
        නම්( pSrc->a[i].fg.isSubquery ){
          assert( pSrc->a[i].u4.pSubq!=0 );
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
static void renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  නම්( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  අතරතුර( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->zErrMsg);
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
static void renameColumnFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  const var_na *zSql = (const var_na*)sqlite3_value_text(argv[0]);
  const var_na *zDb = (const var_na*)sqlite3_value_text(argv[3]);
  const var_na *zTable = (const var_na*)sqlite3_value_text(argv[4]);
  පූර්ණ_සංඛ්‍යා iCol = sqlite3_value_int(argv[5]);
  const var_na *zNew = (const var_na*)sqlite3_value_text(argv[6]);
  පූර්ණ_සංඛ්‍යා bQuote = sqlite3_value_int(argv[7]);
  පූර්ණ_සංඛ්‍යා bTemp = sqlite3_value_int(argv[8]);
  const var_na *zOld;
  පූර්ණ_සංඛ්‍යා rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  පූර්ණ_සංඛ්‍යා i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  නම්( zSql==0 ) ආපසු;
  නම්( zTable==0 ) ආපසු;
  නම්( zNew==0 ) ආපසු;
  නම්( iCol<0 ) ආපසු;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  නම්( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    ආපසු;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, parimaa_na(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, parimaa_na(Walker));
  sWalker.pParse = &sParse;
  sWalker.xExprCallback = renameColumnExprCb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  නම්( rc!=SQLITE_OK ) goto renameColumnFunc_done;
  නම්( sParse.pNewTable ){
    නම්( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      නම්( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      නම්( rc!=SQLITE_OK ) goto renameColumnFunc_done;
    }නැතහොත් නම්( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      පූර්ණ_සංඛ්‍යා bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      නම්( bFKOnly==0 ){
        නම්( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (void*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        නම්( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (void*)&sParse.pNewTable->iPKey);
        }
        sqlite3WalkExprList(&sWalker, sParse.pNewTable->pCheck);
        සඳහා(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3WalkExprList(&sWalker, pIdx->aColExpr);
        }
        සඳහා(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3WalkExprList(&sWalker, pIdx->aColExpr);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        සඳහා(i=0; i<sParse.pNewTable->nCol; i++){
          Expr *pExpr = sqlite3ColumnExpr(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3WalkExpr(&sWalker, pExpr);
        }
#endif
      }

      assert( IsOrdinaryTable(sParse.pNewTable) );
      සඳහා(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        සඳහා(i=0; i<pFKey->nCol; i++){
          නම්( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (void*)&pFKey->aCol[i]);
          }
          නම්( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (void*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }නැතහොත් නම්( sParse.pNewIndex ){
    sqlite3WalkExprList(&sWalker, sParse.pNewIndex->aColExpr);
    sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }නැතහොත්{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    නම්( rc!=SQLITE_OK ) goto renameColumnFunc_done;

    සඳහා(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      නම්( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        නම්( pTarget==pTab ){
          නම්( pStep->pUpsert ){
            ExprList *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->pExprList, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    නම්( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  assert( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  නම්( rc!=SQLITE_OK ){
    නම්( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_value(context, argv[0]);
    }නැතහොත් නම්( sParse.zErrMsg ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }නැතහොත්{
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
static පූර්ණ_සංඛ්‍යා renameTableExprCb(Walker *pWalker, Expr *pExpr){
  RenameCtx *p = pWalker->u.pRename;
  නම්( pExpr->op==TK_COLUMN
   && ALWAYS(ExprUseYTab(pExpr))
   && p->pTab==pExpr->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (void*)&pExpr->y.pTab);
  }
  ආපසු WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
static පූර්ණ_සංඛ්‍යා renameTableSelectCb(Walker *pWalker, Select *pSelect){
  පූර්ණ_සංඛ්‍යා i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  නම්( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    ආපසු WRC_Prune;
  }
  නම්( NEVER(pSrc==0) ){
    assert( pWalker->pParse->db->mallocFailed );
    ආපසු WRC_Abort;
  }
  සඳහා(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    නම්( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  ආපසු WRC_Continue;
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
static void renameTableFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  const var_na *zDb = (const var_na*)sqlite3_value_text(argv[0]);
  const var_na *zInput = (const var_na*)sqlite3_value_text(argv[3]);
  const var_na *zOld = (const var_na*)sqlite3_value_text(argv[4]);
  const var_na *zNew = (const var_na*)sqlite3_value_text(argv[5]);
  පූර්ණ_සංඛ්‍යා bTemp = sqlite3_value_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  නම්( zInput && zOld && zNew ){
    Parse sParse;
    පූර්ණ_සංඛ්‍යා rc;
    පූර්ණ_සංඛ්‍යා bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, parimaa_na(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, parimaa_na(Walker));
    sWalker.pParse = &sParse;
    sWalker.xExprCallback = renameTableExprCb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, zInput, bTemp);

    නම්( rc==SQLITE_OK ){
      පූර්ණ_සංඛ්‍යා isLegacy = (db->flags & SQLITE_LegacyAlter);
      නම්( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        නම්( IsView(pTab) ){
          නම්( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, parimaa_na(sNC));
            sNC.pParse = &sParse;

            assert( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            නම්( sParse.nErr ){
              rc = sParse.rc;
            }නැතහොත්{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }නැතහොත්{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_KEY
          නම්( (isLegacy==0 || (db->flags & SQLITE_ForeignKeys))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            assert( IsOrdinaryTable(pTab) );
            සඳහා(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              නම්( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (void*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          නම්( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            නම්( isLegacy==0 ){
              sqlite3WalkExprList(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      නැතහොත් නම්( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        නම්( isLegacy==0 ){
          sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      නැතහොත්{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        නම්( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        නම්( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          නම්( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            සඳහා(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              නම්( pStep->pSrc ){
                පූර්ණ_සංඛ්‍යා i;
                සඳහා(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  නම්( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    නම්( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, zInput, zNew, bQuote);
    }
    නම්( rc!=SQLITE_OK ){
      නම්( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_value(context, argv[3]);
      }නැතහොත් නම්( sParse.zErrMsg ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }නැතහොත්{
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

  ආපසු;
}

static පූර්ණ_සංඛ්‍යා renameQuotefixExprCb(Walker *pWalker, Expr *pExpr){
  නම්( pExpr->op==TK_STRING && (pExpr->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (const void*)pExpr);
  }
  ආපසු WRC_Continue;
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
static void renameQuotefixFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  var_na const *zDb = (const var_na*)sqlite3_value_text(argv[0]);
  var_na const *zInput = (const var_na*)sqlite3_value_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  නම්( zDb && zInput ){
    පූර්ණ_සංඛ්‍යා rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, zInput, 0);

    නම්( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, parimaa_na(RenameCtx));
      memset(&sWalker, 0, parimaa_na(Walker));
      sWalker.pParse = &sParse;
      sWalker.xExprCallback = renameQuotefixExprCb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      නම්( sParse.pNewTable ){
        නම්( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          නම්( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }නැතහොත්{
          පූර්ණ_සංඛ්‍යා i;
          sqlite3WalkExprList(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          සඳහා(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3WalkExpr(&sWalker,
               sqlite3ColumnExpr(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }නැතහොත් නම්( sParse.pNewIndex ){
        sqlite3WalkExprList(&sWalker, sParse.pNewIndex->aColExpr);
        sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }නැතහොත්{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        නම්( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      නම්( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, zInput, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    නම්( rc!=SQLITE_OK ){
      නම්( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_value(context, argv[1]);
      }නැතහොත්{
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
static void renameTableTest(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  var_na const *zDb = (const var_na*)sqlite3_value_text(argv[0]);
  var_na const *zInput = (const var_na*)sqlite3_value_text(argv[1]);
  පූර්ණ_සංඛ්‍යා bTemp = sqlite3_value_int(argv[4]);
  පූර්ණ_සංඛ්‍යා isLegacy = (db->flags & SQLITE_LegacyAlter);
  var_na const *zWhen = (const var_na*)sqlite3_value_text(argv[5]);
  පූර්ණ_සංඛ්‍යා bNoDQS = sqlite3_value_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  නම්( zDb && zInput ){
    පූර්ණ_සංඛ්‍යා rc;
    Parse sParse;
    u64 flags = db->flags;
    නම්( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, zInput, bTemp);
    db->flags = flags;
    නම්( rc==SQLITE_OK ){
      නම්( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, parimaa_na(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        නම්( sParse.nErr ) rc = sParse.rc;
      }

      නැතහොත් නම්( sParse.pNewTrigger ){
        නම්( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        නම්( rc==SQLITE_OK ){
          පූර්ණ_සංඛ්‍යා i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          පූර්ණ_සංඛ්‍යා i2 = sqlite3FindDbName(db, zDb);
          නම්( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    නම්( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
static පූර්ණ_සංඛ්‍යා getConstraintToken(const u8 *z, පූර්ණ_සංඛ්‍යා *piToken){
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා t = 0;
  do {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }අතරතුර( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  නම්( t==TK_LP ){
    පූර්ණ_සංඛ්‍යා nNest = 1;
    අතරතුර( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      නම්( t==TK_LP ){
        nNest++;
      }නැතහොත් නම්( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }නැතහොත් නම්( t==TK_ILLEGAL ){
        bha_nga;
      }
    }
  }

  *piToken = t;
  ආපසු iOff;
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
static void dropColumnFunc(
  sqlite3_context *context,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  පූර්ණ_සංඛ්‍යා iSchema = sqlite3_value_int(argv[0]);
  const var_na *zSql = (const var_na*)sqlite3_value_text(argv[1]);
  පූර්ණ_සංඛ්‍යා iCol = sqlite3_value_int(argv[2]);
  const var_na *zDb = db->aDb[iSchema].zDbSName;
  පූර්ණ_සංඛ්‍යා rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  const var_na *zEnd;
  var_na *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  නම්( rc!=SQLITE_OK ) goto drop_column_done;
  pTab = sParse.pNewTable;
  නම්( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    goto drop_column_done;
  }

  නම්( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (void*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (void*)pTab->aCol[iCol+1].zCnName);
    zEnd = (const var_na*)pEnd->t.z;
  }නැතහොත්{
    පූර්ණ_සංඛ්‍යා eTok;
    assert( IsOrdinaryTable(pTab) );
    assert( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (void*)pTab->aCol[iCol-1].zCnName);
    do {
      pCol->t.z += getConstraintToken((const u8*)pCol->t.z, &eTok);
    }අතරතුර( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (const var_na*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  නම්( rc!=SQLITE_OK ){
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
void sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, const Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  පූර්ණ_සංඛ්‍යා iDb;                        /* Index of db containing pTab in aDb[] */
  const var_na *zDb;                /* Database containing pTab ("main" etc.) */
  var_na *zCol = 0;                 /* Name of column to drop */
  පූර්ණ_සංඛ්‍යා iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  assert( pParse->pNewTable==0 );
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  නම්( NEVER(db->mallocFailed) ) goto exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  නම්( !pTab ) goto exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  නම්( SQLITE_OK!=isAlterableTable(pParse, pTab) ) goto exit_drop_column;
  නම්( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) goto exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  නම්( zCol==0 ){
    assert( db->mallocFailed );
    goto exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  නම්( iCol<0 ){
    sqlite3ErrorMsg(pParse, "no such column: \"%T\"", pName);
    goto exit_drop_column;
  }
  නම්( isRowidAlias(pParse, pTab, zCol, "drop") ) goto exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  නම්( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3ErrorMsg(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    goto exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  නම්( pTab->nCol<=1 ){
    sqlite3ErrorMsg(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    goto exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    goto exit_drop_column;
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
  නම්( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා addr;
    පූර්ණ_සංඛ්‍යා reg;
    පූර්ණ_සංඛ්‍යා regRec;
    Index *pPk = 0;
    පූර්ණ_සංඛ්‍යා nField = 0;               /* Number of non-virtual columns after drop */
    පූර්ණ_සංඛ්‍යා iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    නම්( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }නැතහොත්{
      pPk = sqlite3PrimaryKeyIndex(pTab);
      pParse->nMem += pPk->nColumn;
      සඳහා(i=0; i<pPk->nKeyCol; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->nKeyCol;
    }
    regRec = ++pParse->nMem;
    සඳහා(i=0; i<pTab->nCol; i++){
      නම්( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        පූර්ණ_සංඛ්‍යා regOut;
        නම්( pPk ){
          පූර්ණ_සංඛ්‍යා iPos = sqlite3TableColumnToIndex(pPk, i);
          පූර්ණ_සංඛ්‍යා iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          නම්( iPos<pPk->nKeyCol ) continue;
          regOut = reg+1+iPos-(iPos>iColPos);
        }නැතහොත්{
          regOut = reg+1+nField;
        }
        නම්( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }නැතහොත්{
          sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, i, regOut);
          assert( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    නම්( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    නම්( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_IdxInsert, iCur, regRec, reg+1, pPk->nKeyCol);
    }නැතහොත්{
      sqlite3VdbeAddOp3(v, OP_Insert, iCur, regRec, reg);
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
static පූර්ණ_සංඛ්‍යා getWhitespace(const u8 *z){
  පූර්ණ_සංඛ්‍යා nRet = 0;
  අතරතුර( 1 ){
    පූර්ණ_සංඛ්‍යා t = 0;
    පූර්ණ_සංඛ්‍යා n = sqlite3GetToken(&z[nRet], &t);
    නම්( t!=TK_SPACE && t!=TK_COMMENT ) bha_nga;
    nRet += n;
  }
  ආපසු nRet;
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
static පූර්ණ_සංඛ්‍යා getConstraint(const u8 *z, පූර්ණ_සංඛ්‍යා eTok){
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා t = 0;

#ifndef SQLITE_OMIT_FOREIGN_KEY
  නම්( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  නම්( eTok==TK_REFERENCES ){
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
  අතරතුර( 1 ){
    පූර්ණ_සංඛ්‍යා n = getConstraintToken(&z[iOff], &t);
    නම්( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      bha_nga;
    }
    iOff += n;
  }
  
  ආපසු iOff;
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
static පූර්ණ_සංඛ්‍යා quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  පූර්ණ_සංඛ්‍යා t,                 /* Token type */
  const u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  පූර්ණ_සංඛ්‍යා nQuote,            /* Length of zQuote in bytes */
  const u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  පූර්ණ_සංඛ්‍යා *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  var_na *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  නම්( t==TK_ILLEGAL ){
    *pRes = 1;
    ආපසු SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  නම්( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    ආපසු SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((const var_na*)zCopy, (const var_na*)zCmp);
  sqlite3_free(zCopy);
  ආපසු SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
static පූර්ණ_සංඛ්‍යා skipCreateTable(sqlite3_context *ctx, const u8 *zSql, පූර්ණ_සංඛ්‍යා *piOff){
  පූර්ණ_සංඛ්‍යා iOff = 0;

  නම්( zSql==0 ) ආපසු SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  අතරතුර( 1 ){
    පූර්ණ_සංඛ්‍යා t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    නම්( t==TK_LP ) bha_nga;
    නම්( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      ආපසු SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  ආපසු SQLITE_OK;
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
static void dropConstraintFunc(
  sqlite3_context *ctx,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  const u8 *zSql = sqlite3_value_text(argv[0]);
  const u8 *zCons = 0;
  පූර්ණ_සංඛ්‍යා iNotNull = -1;
  පූර්ණ_සංඛ්‍යා ii;
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා iStart = 0;
  පූර්ණ_සංඛ්‍යා iEnd = 0;
  var_na *zNew = 0;
  පූර්ණ_සංඛ්‍යා t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  නම්( zSql==0 ) ආපසු;

  /* Jump past the "CREATE TABLE" bit. */
  නම්( skipCreateTable(ctx, zSql, &iOff) ) ආපසු;

  නම්( sqlite3_value_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_value_int(argv[1]);
  }නැතහොත්{
    zCons = sqlite3_value_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  සඳහා(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    අතරතුර( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      නම්( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        පූර්ණ_සංඛ්‍යා nTok = 0;
        පූර්ණ_සංඛ්‍යා cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        නම්( zCons ){
          නම්( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ආපසු;
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
        නම්( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }නැතහොත්{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        නම්( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          නම්( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            ආපසු;
          }
          iEnd = iOff;
          bha_nga;
        }

      }නැතහොත් නම්( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        bha_nga;
      }නැතහොත් නම්( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        bha_nga;
      }නැතහොත් නම්( t==TK_COMMA ){
        bha_nga;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  නම්( iEnd<=0 ){
    නම්( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }නැතහොත්{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (const var_na*)zSql, -1, SQLITE_TRANSIENT);
    }
  }නැතහොත්{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    const var_na *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    නම්( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      නම්( zSql[iStart-1]==',' ) iStart--;
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
static void addConstraintFunc(
  sqlite3_context *ctx,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  const u8 *zSql = sqlite3_value_text(argv[0]);
  const var_na *zCons = (const var_na*)sqlite3_value_text(argv[1]);
  පූර්ණ_සංඛ්‍යා iCol = sqlite3_value_int(argv[2]);
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා ii;
  sqlite3_str *pNew;
  පූර්ණ_සංඛ්‍යා t = 0;
  UNUSED_PARAMETER(NotUsed);

  නම්( skipCreateTable(ctx, zSql, &iOff) ) ආපසු;
  
  සඳහා(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    අතරතුර( 1 ){
      පූර්ණ_සංඛ්‍යා nTok = getConstraintToken(&zSql[iOff], &t);
      නම්( t==TK_COMMA || t==TK_RP ) bha_nga;
      නම්( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        ආපසු;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (const var_na*)zSql, iOff);
  නම්( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
static පූර්ණ_සංඛ්‍යා alterFindCol(Parse *pParse, Table *pTab, Token *pCol, පූර්ණ_සංඛ්‍යා *piCol){
  sqlite3 *db = pParse->db;
  var_na *zName = sqlite3NameFromToken(db, pCol);
  පූර්ණ_සංඛ්‍යා rc = SQLITE_NOMEM;
  පූර්ණ_සංඛ්‍යා iCol = -1;

  නම්( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    නම්( iCol<0 ){
      sqlite3ErrorMsg(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }නැතහොත්{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  නම්( rc==SQLITE_OK ){
    const var_na *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    const var_na *zCol = pTab->aCol[iCol].zCnName;
    නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  ආපසු rc;
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
static Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  පූර්ණ_සංඛ්‍යා *piDb,            /* OUT: write the iDb here */
  const var_na **pzDb,    /* OUT: write name of schema here */
  පූර්ණ_සංඛ්‍යා bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  නම්( pTab ){
    පූර්ණ_සංඛ්‍යා iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    නම්( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  නම්( pTab && bAuth ){
    නම්( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  ආපසු pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
void sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  පූර්ණ_සංඛ්‍යා iDb = 0;
  const var_na *zDb = 0;
  var_na *zArg = 0;

  assert( (pCol==0)!=(pCons==0) );
  assert( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  නම්( !pTab ) ආපසු;

  නම්( pCons ){
    var_na *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }නැතහොත්{
    පූර්ණ_සංඛ්‍යා iCol;
    නම්( alterFindCol(pParse, pTab, pCol, &iCol) ) ආපසු;
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
static void failConstraintFunc(
  sqlite3_context *ctx,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  const var_na *zText = (const var_na*)sqlite3_value_text(argv[0]);
  පූර්ණ_සංඛ්‍යා err = sqlite3_value_int(argv[1]);
  (void)NotUsed;
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
static පූර්ණ_සංඛ්‍යා alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  const var_na *pCons,              /* Buffer containing constraint */
  පූර්ණ_සංඛ්‍යා nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා iEnd = 0;

  නම්( zTmp==0 ) ආපසු 0;

  අතරතුර( 1 ){
    පූර්ණ_සංඛ්‍යා t = 0;
    පූර්ණ_සංඛ්‍යා nToken = sqlite3GetToken(&zTmp[iOff], &t);
    නම්( t==TK_ILLEGAL ) bha_nga;
    නම්( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  ආපසු iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
void sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  පූර්ණ_සංඛ්‍යා iCol = 0;
  පූර්ණ_සංඛ්‍යා iDb = 0;
  const var_na *zDb = 0;
  const var_na *pCons = 0;
  පූර්ණ_සංඛ්‍යා nCons = 0;

  /* Look up the table being altered. */
  assert( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  නම්( !pTab ) ආපසු;

  /* Find the column being altered. */
  නම්( alterFindCol(pParse, pTab, pCol, &iCol) ){
    ආපසු;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (පූර්ණ_සංඛ්‍යා)pCol->n, pCol->z
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
static void findConstraintFunc(
  sqlite3_context *ctx,
  පූර්ණ_සංඛ්‍යා NotUsed,
  sqlite3_value **argv
){
  const u8 *zSql = 0;
  const u8 *zCons = 0;
  පූර්ණ_සංඛ්‍යා iOff = 0;
  පූර්ණ_සංඛ්‍යා t = 0;

  (void)NotUsed;
  zSql = sqlite3_value_text(argv[0]);
  zCons = sqlite3_value_text(argv[1]);

  නම්( zSql==0 || zCons==0 ) ආපසු;
  අතරතුර( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  අතරතුර( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    නම්( t==TK_CONSTRAINT ){
      පූර්ණ_සංඛ්‍යා nTok = 0;
      පූර්ණ_සංඛ්‍යා cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      නම්( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) ආපසු;
      නම්( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        ආපසු;
      }
    }නැතහොත් නම්( t==TK_ILLEGAL ){
      bha_nga;
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
void sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  const var_na *zExpr,       /* Text of CHECK expression */
  පූර්ණ_සංඛ්‍යා nExpr,               /* Size of pExpr in bytes */
  Expr *pExpr              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  පූර්ණ_සංඛ්‍යා iDb = 0;             /* Which schema does pTab live in */
  const var_na *zDb = 0;     /* Name of the schema in which pTab lives */
  const var_na *pCons = 0;   /* Text of the constraint */
  පූර්ණ_සංඛ්‍යා nCons;               /* Bytes of text to use from pCons[] */
  පූර්ණ_සංඛ්‍යා rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  assert( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  නම්( !pTab ){
    sqlite3ExprDelete(pParse->db, pExpr);
    ආපසු;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, pExpr, 0);
  sqlite3ExprDelete(pParse->db, pExpr);
  නම්( rc ) ආපසු;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  නම්( pName ){
    var_na *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, nExpr, zExpr
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
void sqlite3AlterFunctions(void){
  static FuncDef aAlterTableFuncs[] = {
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
  sqlite3InsertBuiltinFuncs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
