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
#شامل "sqliteInt.h"

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
ایستا صحیح isAlterableTable(Parse *pParse, Table *pTab){
  اگر( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#ifndef SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#endif
  ){
    sqlite3_Error_پیام(pParse, "table %s may not be altered", pTab->zName);
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ایستا تهی renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ثابت نویسه *zDb,                /* Name of db to verify schema of */
  صحیح bTemp,                      /* True if this is the temp db */
  ثابت نویسه *zWhen,              /* "when" part of error message */
  صحیح bNoDQS                      /* Do not allow DQS in the schema */
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

  اگر( bTemp==0 ){
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
ایستا تهی renameFixQuotes(Parse *pParse, ثابت نویسه *zDb, صحیح bTemp){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_TABLE
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  اگر( bTemp==0 ){
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
ایستا تهی renameReloadSchema(Parse *pParse, صحیح iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  اگر( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    اگر( iDb!=1 ) sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
تهی sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  صحیح iDb;                  /* Database that contains the table */
  نویسه *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  نویسه *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  صحیح nTabName;             /* Number of UTF-8 characters in zTabName */
  ثابت نویسه *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  اگر( NEVER(db->mallocFailed) ) برو exit_rename_table;
  أكد( pSrc->nSrc==1 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) برو exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  اگر( !zName ) برو exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  اگر( sqlite3FindTable(db, zName, zDb)
   || sqlite3FindIndex(db, zName, zDb)
   || sqlite3IsShadowTableOf(db, pTab, zName)
  ){
    sqlite3_Error_پیام(pParse,
        "there is already another table or index with this name: %s", zName);
    برو exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  اگر( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    برو exit_rename_table;
  }
  اگر( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    برو exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  اگر( IsView(pTab) ){
    sqlite3_Error_پیام(pParse, "view %s may not be altered", pTab->zName);
    برو exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    برو exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  اگر( sqlite3ViewGetColumnNames(pParse, pTab) ){
    برو exit_rename_table;
  }
  اگر( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    اگر( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  اگر( v==0 ){
    برو exit_rename_table;
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
  اگر( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  اگر( iDb!=1 ){
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
  اگر( pVTab ){
    صحیح i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(ثابت نویسه*)pVTab, P4_VTAB);
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
ایستا تهی sqlite3ErrorIfNotEmpty(
  Parse *pParse,        /* Parsing context */
  ثابت نویسه *zDb,      /* Schema holding the table */
  ثابت نویسه *zTab,     /* Table to check for empty */
  ثابت نویسه *zErr      /* Error message text */
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
ایستا صحیح isRowidAlias(
  Parse *pParse, 
  Table *pTab, 
  ثابت نویسه *zCol, 
  ثابت نویسه *zOp
){
  اگر( HasRowid(pTab) && sqlite3IsRowid(zCol) ){
    sqlite3_Error_پیام(pParse, "cannot %s rowid alias: %s", zOp, zCol);
    بازگشت SQLITE_ERROR;
  }
  بازگشت SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
تهی sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  صحیح iDb;                  /* Database number */
  ثابت نویسه *zDb;          /* Database name */
  ثابت نویسه *zTab;         /* Table name */
  نویسه *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  عبارت *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  صحیح r1;                   /* Temporary registers */

  db = pParse->db;
  أكد( db->pParse==pParse );
  اگر( pParse->nErr ) بازگشت;
  أكد( db->mallocFailed==0 );
  pNew = pParse->pNewTable;
  أكد( pNew );

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = sqlite3_Column_عبارت(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);
  أكد( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    بازگشت;
  }
#endif


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  اگر( pCol->colFlags & COLFLAG_PRIMKEY ){
    sqlite3_Error_پیام(pParse, "Cannot add a PRIMARY KEY column");
    بازگشت;
  }
  اگر( pNew->pIndex ){
    sqlite3_Error_پیام(pParse,
         "Cannot add a UNIQUE column");
    بازگشت;
  }
  اگر( isRowidAlias(pParse, pTab, pCol->zCnName, "add") ) بازگشت;
  اگر( (pCol->colFlags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    أكد( pDflt==0 || pDflt->op==TK_SPAN );
    اگر( pDflt && pDflt->p_چپ->op==TK_NULL ){
      pDflt = 0;
    }
    أكد( IsOrdinaryTable(pNew) );
    اگر( (db->flags&SQLITE_Foreign_کلیدها) && pNew->u.tab.pFKey && pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    اگر( pCol->notNull && !pDflt ){
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    اگر( pDflt ){
      sqlite3_مقدار *pVal = 0;
      صحیح rc;
      rc = sqlite3_مقدار_From_عبارت(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      أكد( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      اگر( rc!=SQLITE_OK ){
        أكد( db->mallocFailed == 1 );
        بازگشت;
      }
      اگر( !pVal ){
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_مقدار_Free(pVal);
    }
  }وگرنه اگر( pCol->colFlags & COLFLAG_STORED ){
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (نویسه*)pColDef->z, pColDef->n);
  اگر( zCol ){
    نویسه *zEnd = &zCol[pColDef->n-1];
    وقتی( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
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
  اگر( v ){
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
    اگر( pNew->pCheck!=0
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
تهی sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  صحیح iDb;
  صحیح i;
  صحیح nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  أكد( pParse->pNewTable==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  اگر( NEVER(db->mallocFailed) ) برو exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) برو exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  اگر( IsVirtual(pTab) ){
    sqlite3_Error_پیام(pParse, "virtual tables may not be altered");
    برو exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  اگر( IsView(pTab) ){
    sqlite3_Error_پیام(pParse, "Cannot add a column to a view");
    برو exit_begin_add_column;
  }
  اگر( SQLITE_OK!=isAlterableTable(pParse, pTab) ){
    برو exit_begin_add_column;
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
  pNew = (Table*)sqlite3DbMallocZero(db, اندازه(Table));
  اگر( !pNew ) برو exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;
  أكد( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  أكد( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, اندازه(Column)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  اگر( !pNew->aCol || !pNew->zName ){
    أكد( db->mallocFailed );
    برو exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, اندازه(Column)*(size_t)pNew->nCol);
  برای(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }
  أكد( IsOrdinaryTable(pNew) );
  pNew->u.tab.pDfltList = sqlite3_عبارت_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;
  أكد( pNew->nTabRef==1 );

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  بازگشت;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#اگر !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
ایستا صحیح isRealTable(Parse *pParse, Table *pTab, صحیح iOp){
  ثابت نویسه *zType = 0;
#ifndef SQLITE_OMIT_VIEW
  اگر( IsView(pTab) ){
    zType = "view";
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  اگر( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#endif
  اگر( zType ){
    ثابت نویسه *az_پیام[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    أكد( iOp>=0 && iOp<ArraySize(az_پیام) );
    sqlite3_Error_پیام(pParse, "cannot %s %s \"%s\"",
        az_پیام[iOp], zType, pTab->zName
    );
    بازگشت 1;
  }
  بازگشت 0;
}
#وگرنه /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define isRealTable(x,y,z) (0)
#endif

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
تهی sqlite3AlterRenameColumn(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  Table *pTab;                    /* Table being updated */
  صحیح iCol;                       /* Index of column being renamed */
  نویسه *zOld = 0;                 /* Old column name */
  نویسه *zNew = 0;                 /* New column name */
  ثابت نویسه *zDb;                /* Name of schema containing the table */
  صحیح iSchema;                    /* Index of the schema */
  صحیح bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) برو exit_rename_column;

  /* Cannot alter a system table */
  اگر( SQLITE_OK!=isAlterableTable(pParse, pTab) ) برو exit_rename_column;
  اگر( SQLITE_OK!=isRealTable(pParse, pTab, 0) ) برو exit_rename_column;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    برو exit_rename_column;
  }
#endif

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  اگر( !zOld ) برو exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  اگر( iCol<0 ){
    sqlite3_Error_پیام(pParse, "no such column: \"%T\"", pOld);
    برو exit_rename_column;
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
  اگر( !zNew ) برو exit_rename_column;
  اگر( isRowidAlias(pParse, pTab, zOld, "rename from") ) برو exit_rename_column;
  اگر( isRowidAlias(pParse, pTab, zNew, "rename to") ) برو exit_rename_column;
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
  اگر( iSchema!=1 ){
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
  بازگشت;
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
ساختار RenameToken {
  ثابت تهی *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *pNext;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
تعریفنوع ساختار RenameCtx RenameCtx;
ساختار RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  صحیح nList;                      /* Number of tokens in pList */
  صحیح iCol;                       /* Index of column being renamed */
  Table *pTab;                    /* Table being ALTERed */
  ثابت نویسه *zOld;               /* Old column name */
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
ایستا تهی renameTokenCheckAll(Parse *pParse, ثابت تهی *pPtr){
  أكد( pParse==pParse->db->pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  اگر( pParse->nErr==0 ){
    ثابت RenameToken *p;
    u32 i = 1;
    برای(p=pParse->pRename; p; p=p->pNext){
      اگر( p->p ){
        أكد( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    أكد( i>0 );
  }
}
#وگرنه
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
ثابت تهی *sqlite3RenameTokenMap(
  Parse *pParse,
  ثابت تهی *pPtr,
  ثابت Token *pToken
){
  RenameToken *pNew;
  أكد( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  اگر( ALWAYS(pParse->eParseMode!=PARSE_MODE_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, اندازه(RenameToken));
    اگر( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  بازگشت pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
تهی sqlite3RenameTokenRemap(Parse *pParse, ثابت تهی *pTo, ثابت تهی *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, pTo);
  برای(p=pParse->pRename; p; p=p->pNext){
    اگر( p->p==pFrom ){
      p->p = pTo;
      بشکن;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ایستا صحیح rename_Unmap_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ثابت تهی*)p_عبارت);
  اگر( عبارت_Use_YTab(p_عبارت) ){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت تهی*)&p_عبارت->y.pTab);
  }
  بازگشت WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ایستا تهی renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  اگر( pWith ){
    Parse *pParse = pWalker->pParse;
    صحیح i;
    With *pCopy = 0;
    أكد( pWith->nCte>0 );
    اگر( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_افزودن(pParse, pCopy, 1);
    }
    برای(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, اندازه(sNC));
      sNC.pParse = pParse;
      اگر( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      اگر( sNC.pParse->db->mallocFailed ) بازگشت;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    اگر( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ایستا تهی unmapColumnIdlistNames(
  Parse *pParse,
  ثابت IdList *pIdList
){
  صحیح ii;
  أكد( pIdList!=0 );
  برای(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت تهی*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ایستا صحیح renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  صحیح i;
  اگر( pParse->nErr ) بازگشت WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  اگر( p->selFlags & (SF_View|SF_CopyCte) ){
    بازگشت WRC_Prune;
  }
  اگر( ALWAYS(p->pEList) ){
    عبارت_List *pList = p->pEList;
    برای(i=0; i<pList->n_عبارت; i++){
      اگر( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (تهی*)pList->a[i].zEName);
      }
    }
  }
  اگر( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    برای(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (تهی*)pSrc->a[i].zName);
      اگر( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_عبارت(pWalker, pSrc->a[i].u3.pOn);
      }وگرنه{
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  بازگشت WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
تهی sqlite3_Rename_عبارت_Unmap(Parse *pParse, عبارت *p_عبارت){
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, اندازه(Walker));
  sWalker.pParse = pParse;
  sWalker.x_عبارت_Callback = rename_Unmap_عبارت_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = PARSE_MODE_UNMAP;
  sqlite3_Walk_عبارت(&sWalker, p_عبارت);
  pParse->eParseMode = eMode;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
تهی sqlite3RenameExprlistUnmap(Parse *pParse, عبارت_List *pEList){
  اگر( pEList ){
    صحیح i;
    Walker sWalker;
    memset(&sWalker, 0, اندازه(Walker));
    sWalker.pParse = pParse;
    sWalker.x_عبارت_Callback = rename_Unmap_عبارت_Cb;
    sqlite3_Walk_عبارت_List(&sWalker, pEList);
    برای(i=0; i<pEList->n_عبارت; i++){
      اگر( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (تهی*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ایستا تهی renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *pNext;
  RenameToken *p;
  برای(p=pToken; p; p=pNext){
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
ایستا RenameToken *renameTokenFind(
  Parse *pParse,
  ساختار RenameCtx *pCtx,
  ثابت تهی *pPtr
){
  RenameToken **pp;
  اگر( NEVER(pPtr==0) ){
    بازگشت 0;
  }
  برای(pp=&pParse->pRename; (*pp); pp=&(*pp)->pNext){
    اگر( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      اگر( pCtx ){
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      بازگشت pToken;
    }
  }
  بازگشت 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ایستا صحیح renameColumnSelectCb(Walker *pWalker, Select *p){
  اگر( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    بازگشت WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  بازگشت WRC_Continue;
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
ایستا صحیح rename_Column_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  RenameCtx *p = pWalker->u.pRename;
  اگر( p_عبارت->op==TK_TRIGGER
   && p_عبارت->iColumn==p->iCol
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    renameTokenFind(pWalker->pParse, p, (تهی*)p_عبارت);
  }وگرنه اگر( p_عبارت->op==TK_COLUMN
   && p_عبارت->iColumn==p->iCol
   && ALWAYS(عبارت_Use_YTab(p_عبارت))
   && p->pTab==p_عبارت->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (تهی*)p_عبارت);
  }
  بازگشت WRC_Continue;
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
ایستا RenameToken *renameColumnTokenNext(RenameCtx *pCtx){
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  برای(pToken=pBest->pNext; pToken; pToken=pToken->pNext){
    اگر( pToken->t.z>pBest->t.z ) pBest = pToken;
  }
  برای(pp=&pCtx->pList; *pp!=pBest; pp=&(*pp)->pNext);
  *pp = pBest->pNext;

  بازگشت pBest;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ایستا تهی errorMPrintf(sqlite3_context *pCtx, ثابت نویسه *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  نویسه *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  اگر( zErr ){
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }وگرنه{
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
ایستا تهی renameColumnParseError(
  sqlite3_context *pCtx,
  ثابت نویسه *zWhen,
  sqlite3_مقدار *pType,
  sqlite3_مقدار *pObject,
  Parse *pParse
){
  ثابت نویسه *zT = (ثابت نویسه*)sqlite3_مقدار_text(pType);
  ثابت نویسه *zN = (ثابت نویسه*)sqlite3_مقدار_text(pObject);
  نویسه *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_پیام
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
ایستا تهی renameColumnElistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت عبارت_List *pEList,
  ثابت نویسه *zOld
){
  اگر( pEList ){
    صحیح i;
    برای(i=0; i<pEList->n_عبارت; i++){
      ثابت نویسه *zName = pEList->a[i].zEName;
      اگر( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        renameTokenFind(pParse, pCtx, (ثابت تهی*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ایستا تهی renameColumnIdlistNames(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت IdList *pIdList,
  ثابت نویسه *zOld
){
  اگر( pIdList ){
    صحیح i;
    برای(i=0; i<pIdList->nId; i++){
      ثابت نویسه *zName = pIdList->a[i].zName;
      اگر( 0==sqlite3_stricmp(zName, zOld) ){
        renameTokenFind(pParse, pCtx, (ثابت تهی*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ایستا صحیح renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ثابت نویسه *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ثابت نویسه *zSql,               /* SQL to parse */
  صحیح bTemp                       /* True if SQL is from temp schema */
){
  صحیح rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  اگر( zSql==0 ){
    بازگشت SQLITE_NOMEM;
  }
  اگر( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    بازگشت SQLITE_CORRUPT_BKPT;
  }
  اگر( bTemp ){
    db->init.iDb = 1;
  }وگرنه{
    صحیح iDb = sqlite3FindDbName(db, zDb);
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
  اگر( db->mallocFailed ) rc = SQLITE_NOMEM;
  اگر( rc==SQLITE_OK
   && NEVER(p->pNewTable==0 && p->pNewIndex==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#ifdef SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  اگر( rc==SQLITE_OK ){
    صحیح nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    برای(pToken=p->pRename; pToken; pToken=pToken->pNext){
      أكد( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#endif

  db->init.iDb = 0;
  بازگشت rc;
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
ایستا صحیح renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ثابت نویسه *zSql,               /* SQL statement to edit */
  ثابت نویسه *zNew,               /* New token text */
  صحیح bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  صحیح rc = SQLITE_OK;
  نویسه *zQuot = 0;
  نویسه *zOut;
  i64 nQuot = 0;
  نویسه *zBuf1 = 0;
  نویسه *zBuf2 = 0;

  اگر( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    اگر( zQuot==0 ){
      بازگشت SQLITE_NOMEM;
    }وگرنه{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    أكد( nQuot>=nNew && nSql>=0 && nNew>=0 );
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }وگرنه{
    أكد( nSql>0 );
    zOut = (نویسه*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    اگر( zOut ){
      zBuf1 = &zOut[nSql*2+1];
      zBuf2 = &zOut[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  اگر( zOut ){
    i64 nOut = nSql;
    أكد( nSql>0 );
    memcpy(zOut, zSql, (size_t)nSql);
    وقتی( pRename->pList ){
      صحیح iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ثابت نویسه *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      اگر( zNew ){
        اگر( bQuote==0 && sqlite3IsIdChar(*(u8*)pBest->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }وگرنه{
          nReplace = nQuot;
          zReplace = zQuot;
          اگر( pBest->t.z[pBest->t.n]=='"' ) nReplace++;
        }
      }وگرنه{
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
        sqlite3_snprintf((صحیح)(nSql*2), zBuf2, "%Q%s", zBuf1,
            pBest->t.z[pBest->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (صحیح)(pBest->t.z - zSql);
      اگر( pBest->t.n!=nReplace ){
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
  }وگرنه{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  بازگشت rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ایستا تهی renameSetENames(عبارت_List *pEList, صحیح val){
  أكد( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_SPAN );
  اگر( pEList ){
    صحیح i;
    برای(i=0; i<pEList->n_عبارت; i++){
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
ایستا صحیح renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  صحیح rc = SQLITE_OK;

  memset(&sNC, 0, اندازه(sNC));
  sNC.pParse = pParse;
  أكد( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table,
      db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  اگر( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  اگر( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_عبارت_Names(&sNC, pNew->pWhen);
  }

  برای(pStep=pNew->step_list; rc==SQLITE_OK && pStep; pStep=pStep->pNext){
    اگر( pStep->pSelect ){
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      اگر( pParse->nErr ) rc = pParse->rc;
    }
    اگر( rc==SQLITE_OK && pStep->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      اگر( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, pStep->p_عبارت_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        اگر( pSel==0 ){
          pStep->p_عبارت_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }وگرنه{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          renameSetENames(pStep->p_عبارت_List, ENAME_SPAN);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->p_عبارت_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          أكد( pStep->p_عبارت_List==0 || pStep->p_عبارت_List==pSel->pEList );
          أكد( pSrc==pSel->pSrc );
          اگر( pStep->p_عبارت_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        اگر( ALWAYS(pStep->pSrc) ){
          صحیح i;
          برای(i=0; i<pStep->pSrc->nSrc && rc==SQLITE_OK; i++){
            SrcItem *p = &pStep->pSrc->a[i];
            اگر( p->fg.isSubquery ){
              أكد( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        اگر(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        اگر( rc==SQLITE_OK && pStep->pWhere ){
          rc = sqlite3_Resolve_عبارت_Names(&sNC, pStep->pWhere);
        }
        اگر( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_عبارت_List_Names(&sNC, pStep->p_عبارت_List);
        }
        أكد( !pStep->pUpsert || (!pStep->pWhere && !pStep->p_عبارت_List) );
        اگر( pStep->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_عبارت_List_Names(&sNC, pUpsert->pUpsertTarget);
          اگر( rc==SQLITE_OK ){
            عبارت_List *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3_Resolve_عبارت_List_Names(&sNC, pUpsertSet);
          }
          اگر( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_عبارت_Names(&sNC, pUpsert->pUpsertWhere);
          }
          اگر( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_عبارت_Names(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }وگرنه{
        rc = SQLITE_NOMEM;
      }
    }
  }
  بازگشت rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ایستا تهی renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  TriggerStep *pStep;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_عبارت(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  برای(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3_Walk_عبارت(pWalker, pStep->pWhere);
    sqlite3_Walk_عبارت_List(pWalker, pStep->p_عبارت_List);
    اگر( pStep->pUpsert ){
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3_Walk_عبارت_List(pWalker, pUpsert->pUpsertTarget);
      sqlite3_Walk_عبارت_List(pWalker, pUpsert->pUpsertSet);
      sqlite3_Walk_عبارت(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_عبارت(pWalker, pUpsert->pUpsertTargetWhere);
    }
    اگر( pStep->pSrc ){
      صحیح i;
      SrcList *pSrc = pStep->pSrc;
      برای(i=0; i<pSrc->nSrc; i++){
        اگر( pSrc->a[i].fg.isSubquery ){
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
ایستا تهی renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  Index *pIdx;
  اگر( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  وقتی( (pIdx = pParse->pNewIndex)!=0 ){
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_پیام);
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
ایستا تهی renameColumnFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ثابت نویسه *zSql = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  ثابت نویسه *zDb = (ثابت نویسه*)sqlite3_مقدار_text(argv[3]);
  ثابت نویسه *zTable = (ثابت نویسه*)sqlite3_مقدار_text(argv[4]);
  صحیح iCol = sqlite3_مقدار_int(argv[5]);
  ثابت نویسه *zNew = (ثابت نویسه*)sqlite3_مقدار_text(argv[6]);
  صحیح bQuote = sqlite3_مقدار_int(argv[7]);
  صحیح bTemp = sqlite3_مقدار_int(argv[8]);
  ثابت نویسه *zOld;
  صحیح rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  صحیح i;
  Table *pTab;
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#endif

  UNUSED_PARAMETER(NotUsed);
  اگر( zSql==0 ) بازگشت;
  اگر( zTable==0 ) بازگشت;
  اگر( zNew==0 ) بازگشت;
  اگر( iCol<0 ) بازگشت;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  اگر( pTab==0 || iCol>=pTab->nCol ){
    sqlite3BtreeLeaveAll(db);
    بازگشت;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, اندازه(sCtx));
  sCtx.iCol = ((iCol==pTab->iPKey) ? -1 : iCol);

#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#endif
  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, اندازه(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_عبارت_Callback = rename_Column_عبارت_Cb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  اگر( rc!=SQLITE_OK ) برو renameColumnFunc_done;
  اگر( sParse.pNewTable ){
    اگر( IsView(sParse.pNewTable) ){
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      اگر( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      اگر( rc!=SQLITE_OK ) برو renameColumnFunc_done;
    }وگرنه اگر( IsOrdinaryTable(sParse.pNewTable) ){
      /* A regular table */
      صحیح bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      اگر( bFKOnly==0 ){
        اگر( iCol<sParse.pNewTable->nCol ){
          renameTokenFind(
              &sParse, &sCtx, (تهی*)sParse.pNewTable->aCol[iCol].zCnName
          );
        }
        اگر( sCtx.iCol<0 ){
          renameTokenFind(&sParse, &sCtx, (تهی*)&sParse.pNewTable->iPKey);
        }
        sqlite3_Walk_عبارت_List(&sWalker, sParse.pNewTable->pCheck);
        برای(pIdx=sParse.pNewTable->pIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_عبارت_List(&sWalker, pIdx->a_Col_عبارت);
        }
        برای(pIdx=sParse.pNewIndex; pIdx; pIdx=pIdx->pNext){
          sqlite3_Walk_عبارت_List(&sWalker, pIdx->a_Col_عبارت);
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        برای(i=0; i<sParse.pNewTable->nCol; i++){
          عبارت *p_عبارت = sqlite3_Column_عبارت(sParse.pNewTable,
                                                  &sParse.pNewTable->aCol[i]);
          sqlite3_Walk_عبارت(&sWalker, p_عبارت);
        }
#endif
      }

      أكد( IsOrdinaryTable(sParse.pNewTable) );
      برای(pFKey=sParse.pNewTable->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
        برای(i=0; i<pFKey->nCol; i++){
          اگر( bFKOnly==0 && pFKey->aCol[i].iFrom==iCol ){
            renameTokenFind(&sParse, &sCtx, (تهی*)&pFKey->aCol[i]);
          }
          اگر( 0==sqlite3_stricmp(pFKey->zTo, zTable)
           && 0==sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)
          ){
            renameTokenFind(&sParse, &sCtx, (تهی*)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  }وگرنه اگر( sParse.pNewIndex ){
    sqlite3_Walk_عبارت_List(&sWalker, sParse.pNewIndex->a_Col_عبارت);
    sqlite3_Walk_عبارت(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  }وگرنه{
    /* A trigger */
    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    اگر( rc!=SQLITE_OK ) برو renameColumnFunc_done;

    برای(pStep=sParse.pNewTrigger->step_list; pStep; pStep=pStep->pNext){
      اگر( pStep->pSrc ){
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        اگر( pTarget==pTab ){
          اگر( pStep->pUpsert ){
            عبارت_List *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->p_عبارت_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    اگر( sParse.pTriggerTab==pTab ){
      renameColumnIdlistNames(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  أكد( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  اگر( rc!=SQLITE_OK ){
    اگر( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_result_مقدار(context, argv[0]);
    }وگرنه اگر( sParse.z_Err_پیام ){
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    }وگرنه{
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
ایستا صحیح rename_Table_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  RenameCtx *p = pWalker->u.pRename;
  اگر( p_عبارت->op==TK_COLUMN
   && ALWAYS(عبارت_Use_YTab(p_عبارت))
   && p->pTab==p_عبارت->y.pTab
  ){
    renameTokenFind(pWalker->pParse, p, (تهی*)&p_عبارت->y.pTab);
  }
  بازگشت WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ایستا صحیح renameTableSelectCb(Walker *pWalker, Select *pSelect){
  صحیح i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  اگر( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    بازگشت WRC_Prune;
  }
  اگر( NEVER(pSrc==0) ){
    أكد( pWalker->pParse->db->mallocFailed );
    بازگشت WRC_Abort;
  }
  برای(i=0; i<pSrc->nSrc; i++){
    SrcItem *pItem = &pSrc->a[i];
    اگر( pItem->pSTab==p->pTab ){
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  بازگشت WRC_Continue;
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
ایستا تهی renameTableFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت نویسه *zDb = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  ثابت نویسه *z_درون_داد = (ثابت نویسه*)sqlite3_مقدار_text(argv[3]);
  ثابت نویسه *zOld = (ثابت نویسه*)sqlite3_مقدار_text(argv[4]);
  ثابت نویسه *zNew = (ثابت نویسه*)sqlite3_مقدار_text(argv[5]);
  صحیح bTemp = sqlite3_مقدار_int(argv[6]);
  UNUSED_PARAMETER(NotUsed);

  اگر( z_درون_داد && zOld && zNew ){
    Parse sParse;
    صحیح rc;
    صحیح bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#ifndef SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#endif

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, اندازه(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, اندازه(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_عبارت_Callback = rename_Table_عبارت_Cb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_درون_داد, bTemp);

    اگر( rc==SQLITE_OK ){
      صحیح isLegacy = (db->flags & SQLITE_LegacyAlter);
      اگر( sParse.pNewTable ){
        Table *pTab = sParse.pNewTable;

        اگر( IsView(pTab) ){
          اگر( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, اندازه(sNC));
            sNC.pParse = &sParse;

            أكد( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            اگر( sParse.nErr ){
              rc = sParse.rc;
            }وگرنه{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }وگرنه{
          /* Modify any FK definitions to point to the new table. */
#ifndef SQLITE_OMIT_FOREIGN_کلید
          اگر( (isLegacy==0 || (db->flags & SQLITE_Foreign_کلیدها))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            أكد( IsOrdinaryTable(pTab) );
            برای(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
              اگر( sqlite3_stricmp(pFKey->zTo, zOld)==0 ){
                renameTokenFind(&sParse, &sCtx, (تهی*)pFKey->zTo);
              }
            }
          }
#endif

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          اگر( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            اگر( isLegacy==0 ){
              sqlite3_Walk_عبارت_List(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      وگرنه اگر( sParse.pNewIndex ){
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        اگر( isLegacy==0 ){
          sqlite3_Walk_عبارت(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

#ifndef SQLITE_OMIT_TRIGGER
      وگرنه{
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        اگر( 0==sqlite3_stricmp(sParse.pNewTrigger->table, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        اگر( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          اگر( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            برای(pStep=pTrigger->step_list; pStep; pStep=pStep->pNext){
              اگر( pStep->pSrc ){
                صحیح i;
                برای(i=0; i<pStep->pSrc->nSrc; i++){
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  اگر( 0==sqlite3_stricmp(pItem->zName, zOld) ){
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

    اگر( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_درون_داد, zNew, bQuote);
    }
    اگر( rc!=SQLITE_OK ){
      اگر( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_result_مقدار(context, argv[3]);
      }وگرنه اگر( sParse.z_Err_پیام ){
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      }وگرنه{
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

  بازگشت;
}

ایستا صحیح rename_Quotefix_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_STRING && (p_عبارت->flags & EP_DblQuoted) ){
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (ثابت تهی*)p_عبارت);
  }
  بازگشت WRC_Continue;
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
ایستا تهی renameQuotefixFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  نویسه ثابت *zDb = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  نویسه ثابت *z_درون_داد = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  sqlite3BtreeEnterAll(db);

  UNUSED_PARAMETER(NotUsed);
  اگر( zDb && z_درون_داد ){
    صحیح rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_درون_داد, 0);

    اگر( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, اندازه(RenameCtx));
      memset(&sWalker, 0, اندازه(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_عبارت_Callback = rename_Quotefix_عبارت_Cb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      اگر( sParse.pNewTable ){
        اگر( IsView(sParse.pNewTable) ){
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          اگر( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }وگرنه{
          صحیح i;
          sqlite3_Walk_عبارت_List(&sWalker, sParse.pNewTable->pCheck);
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          برای(i=0; i<sParse.pNewTable->nCol; i++){
            sqlite3_Walk_عبارت(&sWalker,
               sqlite3_Column_عبارت(sParse.pNewTable,
                                         &sParse.pNewTable->aCol[i]));
          }
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }وگرنه اگر( sParse.pNewIndex ){
        sqlite3_Walk_عبارت_List(&sWalker, sParse.pNewIndex->a_Col_عبارت);
        sqlite3_Walk_عبارت(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      }وگرنه{
#ifndef SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        اگر( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#endif /* SQLITE_OMIT_TRIGGER */
      }

      اگر( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_درون_داد, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    اگر( rc!=SQLITE_OK ){
      اگر( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_result_مقدار(context, argv[1]);
      }وگرنه{
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
ایستا تهی renameTableTest(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  نویسه ثابت *zDb = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  نویسه ثابت *z_درون_داد = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
  صحیح bTemp = sqlite3_مقدار_int(argv[4]);
  صحیح isLegacy = (db->flags & SQLITE_LegacyAlter);
  نویسه ثابت *zWhen = (ثابت نویسه*)sqlite3_مقدار_text(argv[5]);
  صحیح bNoDQS = sqlite3_مقدار_int(argv[6]);

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);

  اگر( zDb && z_درون_داد ){
    صحیح rc;
    Parse sParse;
    u64 flags = db->flags;
    اگر( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_درون_داد, bTemp);
    db->flags = flags;
    اگر( rc==SQLITE_OK ){
      اگر( isLegacy==0 && sParse.pNewTable && IsView(sParse.pNewTable) ){
        NameContext sNC;
        memset(&sNC, 0, اندازه(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        اگر( sParse.nErr ) rc = sParse.rc;
      }

      وگرنه اگر( sParse.pNewTrigger ){
        اگر( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        اگر( rc==SQLITE_OK ){
          صحیح i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          صحیح i2 = sqlite3FindDbName(db, zDb);
          اگر( i1==i2 ){
            /* Handle output case B */
            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    اگر( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
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
ایستا صحیح getConstraintToken(ثابت u8 *z, صحیح *piToken){
  صحیح iOff = 0;
  صحیح t = 0;
  انجامبده {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }وقتی( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  اگر( t==TK_LP ){
    صحیح nNest = 1;
    وقتی( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      اگر( t==TK_LP ){
        nNest++;
      }وگرنه اگر( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }وگرنه اگر( t==TK_ILLEGAL ){
        بشکن;
      }
    }
  }

  *piToken = t;
  بازگشت iOff;
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
ایستا تهی dropColumnFunc(
  sqlite3_context *context,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحیح iSchema = sqlite3_مقدار_int(argv[0]);
  ثابت نویسه *zSql = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
  صحیح iCol = sqlite3_مقدار_int(argv[2]);
  ثابت نویسه *zDb = db->aDb[iSchema].zDbSName;
  صحیح rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  ثابت نویسه *zEnd;
  نویسه *zNew = 0;

#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#endif

  UNUSED_PARAMETER(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  اگر( rc!=SQLITE_OK ) برو drop_column_done;
  pTab = sParse.pNewTable;
  اگر( pTab==0 || pTab->nCol==1 || iCol>=pTab->nCol ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    برو drop_column_done;
  }

  اگر( iCol<pTab->nCol-1 ){
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (تهی*)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (تهی*)pTab->aCol[iCol+1].zCnName);
    zEnd = (ثابت نویسه*)pEnd->t.z;
  }وگرنه{
    صحیح eTok;
    أكد( IsOrdinaryTable(pTab) );
    أكد( iCol!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    pCol = renameTokenFind(&sParse, 0, (تهی*)pTab->aCol[iCol-1].zCnName);
    انجامبده {
      pCol->t.z += getConstraintToken((ثابت u8*)pCol->t.z, &eTok);
    }وقتی( eTok!=TK_COMMA );
    pCol->t.z--;
    zEnd = (ثابت نویسه*)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z-zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#endif
  اگر( rc!=SQLITE_OK ){
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
تهی sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, ثابت Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  Table *pTab;                    /* Table to modify */
  صحیح iDb;                        /* Index of db containing pTab in aDb[] */
  ثابت نویسه *zDb;                /* Database containing pTab ("main" etc.) */
  نویسه *zCol = 0;                 /* Name of column to drop */
  صحیح iCol;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  أكد( pParse->pNewTable==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  اگر( NEVER(db->mallocFailed) ) برو exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) برو exit_drop_column;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  اگر( SQLITE_OK!=isAlterableTable(pParse, pTab) ) برو exit_drop_column;
  اگر( SQLITE_OK!=isRealTable(pParse, pTab, 1) ) برو exit_drop_column;

  /* Find the index of the column being dropped. */
  zCol = sqlite3NameFromToken(db, pName);
  اگر( zCol==0 ){
    أكد( db->mallocFailed );
    برو exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  اگر( iCol<0 ){
    sqlite3_Error_پیام(pParse, "no such column: \"%T\"", pName);
    برو exit_drop_column;
  }
  اگر( isRowidAlias(pParse, pTab, zCol, "drop") ) برو exit_drop_column;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  اگر( pTab->aCol[iCol].colFlags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_پیام(pParse, "cannot drop %s column: \"%s\"",
        (pTab->aCol[iCol].colFlags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        zCol
    );
    برو exit_drop_column;
  }

  /* Do not allow the number of columns to go to zero */
  اگر( pTab->nCol<=1 ){
    sqlite3_Error_پیام(pParse, "cannot drop column \"%s\": no other columns exist",zCol);
    برو exit_drop_column;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  أكد( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
    برو exit_drop_column;
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
  اگر( pParse->nErr==0 && (pTab->aCol[iCol].colFlags & COLFLAG_VIRTUAL)==0 ){
    صحیح i;
    صحیح addr;
    صحیح reg;
    صحیح regRec;
    Index *pPk = 0;
    صحیح nField = 0;               /* Number of non-virtual columns after drop */
    صحیح iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3VdbeAddOp1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    اگر( HasRowid(pTab) ){
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->nCol;
    }وگرنه{
      pPk = sqlite3_Primary_کلید_Index(pTab);
      pParse->nMem += pPk->nColumn;
      برای(i=0; i<pPk->n_کلید_Col; i++){
        sqlite3VdbeAddOp3(v, OP_Column, iCur, i, reg+i+1);
      }
      nField = pPk->n_کلید_Col;
    }
    regRec = ++pParse->nMem;
    برای(i=0; i<pTab->nCol; i++){
      اگر( i!=iCol && (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        صحیح regOut;
        اگر( pPk ){
          صحیح iPos = sqlite3TableColumnToIndex(pPk, i);
          صحیح iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          اگر( iPos<pPk->n_کلید_Col ) ادامه;
          regOut = reg+1+iPos-(iPos>iColPos);
        }وگرنه{
          regOut = reg+1+nField;
        }
        اگر( i==pTab->iPKey ){
          sqlite3VdbeAddOp2(v, OP_Null, 0, regOut);
        }وگرنه{
          sqlite3_عبارت_Code_Get_Column_Of_Table(v, pTab, iCur, i, regOut);
          أكد( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->aCol[i].affinity==SQLITE_AFF_REAL );
          sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
        }
        nField++;
      }
    }
    اگر( nField==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, OP_Null, 0, reg+1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg+1, nField, regRec);
    اگر( pPk ){
      sqlite3VdbeAddOp4Int(v, OP_Idx_درج, iCur, regRec, reg+1, pPk->n_کلید_Col);
    }وگرنه{
      sqlite3VdbeAddOp3(v, OP_درج, iCur, regRec, reg);
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
ایستا صحیح getWhitespace(ثابت u8 *z){
  صحیح nRet = 0;
  وقتی( 1 ){
    صحیح t = 0;
    صحیح n = sqlite3GetToken(&z[nRet], &t);
    اگر( t!=TK_SPACE && t!=TK_COMMENT ) بشکن;
    nRet += n;
  }
  بازگشت nRet;
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
ایستا صحیح getConstraint(ثابت u8 *z, صحیح eTok){
  صحیح iOff = 0;
  صحیح t = 0;

#ifndef SQLITE_OMIT_FOREIGN_کلید
  اگر( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  اگر( eTok==TK_REFERENCES ){
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
  وقتی( 1 ){
    صحیح n = getConstraintToken(&z[iOff], &t);
    اگر( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      بشکن;
    }
    iOff += n;
  }
  
  بازگشت iOff;
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
ایستا صحیح quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  صحیح t,                 /* Token type */
  ثابت u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  صحیح nQuote,            /* Length of zQuote in bytes */
  ثابت u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  صحیح *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  نویسه *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  اگر( t==TK_ILLEGAL ){
    *pRes = 1;
    بازگشت SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  اگر( zCopy==0 ){
    sqlite3_result_error_nomem(ctx);
    بازگشت SQLITE_NOMEM_BKPT;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ثابت نویسه*)zCopy, (ثابت نویسه*)zCmp);
  sqlite3_free(zCopy);
  بازگشت SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ایستا صحیح skipCreateTable(sqlite3_context *ctx, ثابت u8 *zSql, صحیح *piOff){
  صحیح iOff = 0;

  اگر( zSql==0 ) بازگشت SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  وقتی( 1 ){
    صحیح t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    اگر( t==TK_LP ) بشکن;
    اگر( t==TK_ILLEGAL ){
      sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
      بازگشت SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  بازگشت SQLITE_OK;
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
ایستا تهی dropConstraintFunc(
  sqlite3_context *ctx,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ثابت u8 *zSql = sqlite3_مقدار_text(argv[0]);
  ثابت u8 *zCons = 0;
  صحیح iNotNull = -1;
  صحیح ii;
  صحیح iOff = 0;
  صحیح iStart = 0;
  صحیح iEnd = 0;
  نویسه *zNew = 0;
  صحیح t = 0;
  sqlite3 *db;
  UNUSED_PARAMETER(NotUsed);

  اگر( zSql==0 ) بازگشت;

  /* Jump past the "CREATE TABLE" bit. */
  اگر( skipCreateTable(ctx, zSql, &iOff) ) بازگشت;

  اگر( sqlite3_مقدار_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_مقدار_int(argv[1]);
  }وگرنه{
    zCons = sqlite3_مقدار_text(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  برای(ii=0; iEnd==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    وقتی( 1 ){
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      اگر( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        صحیح nTok = 0;
        صحیح cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        اگر( zCons ){
          اگر( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) بازگشت;
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
        اگر( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }وگرنه{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        اگر( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          اگر( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            بازگشت;
          }
          iEnd = iOff;
          بشکن;
        }

      }وگرنه اگر( t==TK_NOT && iNotNull==ii ){
        iEnd = iOff + getConstraint(&zSql[iOff], 0);
        بشکن;
      }وگرنه اگر( t==TK_RP || t==TK_ILLEGAL ){
        iEnd = -1;
        بشکن;
      }وگرنه اگر( t==TK_COMMA ){
        بشکن;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  اگر( iEnd<=0 ){
    اگر( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }وگرنه{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_result_text(ctx, (ثابت نویسه*)zSql, -1, SQLITE_TRANSIENT);
    }
  }وگرنه{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ثابت نویسه *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    اگر( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      اگر( zSql[iStart-1]==',' ) iStart--;
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
ایستا تهی addConstraintFunc(
  sqlite3_context *ctx,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ثابت u8 *zSql = sqlite3_مقدار_text(argv[0]);
  ثابت نویسه *zCons = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
  صحیح iCol = sqlite3_مقدار_int(argv[2]);
  صحیح iOff = 0;
  صحیح ii;
  sqlite3_str *pNew;
  صحیح t = 0;
  UNUSED_PARAMETER(NotUsed);

  اگر( skipCreateTable(ctx, zSql, &iOff) ) بازگشت;
  
  برای(ii=0; ii<=iCol || (iCol<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    وقتی( 1 ){
      صحیح nTok = getConstraintToken(&zSql[iOff], &t);
      اگر( t==TK_COMMA || t==TK_RP ) بشکن;
      اگر( t==TK_ILLEGAL ){
        sqlite3_result_error_code(ctx, SQLITE_CORRUPT_BKPT);
        بازگشت;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ثابت نویسه*)zSql, iOff);
  اگر( iCol<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_result_str(ctx, pNew, SQLITE_FINISH);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ایستا صحیح alterFindCol(Parse *pParse, Table *pTab, Token *pCol, صحیح *piCol){
  sqlite3 *db = pParse->db;
  نویسه *zName = sqlite3NameFromToken(db, pCol);
  صحیح rc = SQLITE_NOMEM;
  صحیح iCol = -1;

  اگر( zName ){
    iCol = sqlite3ColumnIndex(pTab, zName);
    اگر( iCol<0 ){
      sqlite3_Error_پیام(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }وگرنه{
      rc = SQLITE_OK;
    }
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  اگر( rc==SQLITE_OK ){
    ثابت نویسه *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    ثابت نویسه *zCol = pTab->aCol[iCol].zCnName;
    اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol) ){
      pTab = 0;
    }
  }
#endif

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  بازگشت rc;
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
ایستا Table *alterFindTable(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  صحیح *piDb,            /* OUT: write the iDb here */
  ثابت نویسه **pzDb,    /* OUT: write name of schema here */
  صحیح bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  اگر( pTab ){
    صحیح iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    اگر( SQLITE_OK!=isRealTable(pParse, pTab, 2) 
     || SQLITE_OK!=isAlterableTable(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  اگر( pTab && bAuth ){
    اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#endif
  sqlite3SrcListDelete(db, pSrc);
  بازگشت pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
تهی sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *pCol       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  صحیح iDb = 0;
  ثابت نویسه *zDb = 0;
  نویسه *zArg = 0;

  أكد( (pCol==0)!=(pCons==0) );
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons!=0);
  اگر( !pTab ) بازگشت;

  اگر( pCons ){
    نویسه *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }وگرنه{
    صحیح iCol;
    اگر( alterFindCol(pParse, pTab, pCol, &iCol) ) بازگشت;
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
ایستا تهی failConstraintFunc(
  sqlite3_context *ctx,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ثابت نویسه *zText = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  صحیح err = sqlite3_مقدار_int(argv[1]);
  (تهی)NotUsed;
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
ایستا صحیح alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ثابت نویسه *pCons,              /* Buffer containing constraint */
  صحیح nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  صحیح iOff = 0;
  صحیح iEnd = 0;

  اگر( zTmp==0 ) بازگشت 0;

  وقتی( 1 ){
    صحیح t = 0;
    صحیح nToken = sqlite3GetToken(&zTmp[iOff], &t);
    اگر( t==TK_ILLEGAL ) بشکن;
    اگر( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      iEnd = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  بازگشت iEnd;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
تهی sqlite3AlterSetNotNull(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *pCol,     /* Name of the column to add a NOT NULL constraint to */
  Token *pFirst    /* The NOT token of the NOT NULL constraint text */
){
  Table *pTab = 0;
  صحیح iCol = 0;
  صحیح iDb = 0;
  ثابت نویسه *zDb = 0;
  ثابت نویسه *pCons = 0;
  صحیح nCons = 0;

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  اگر( !pTab ) بازگشت;

  /* Find the column being altered. */
  اگر( alterFindCol(pParse, pTab, pCol, &iCol) ){
    بازگشت;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (صحیح)pCol->n, pCol->z
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
ایستا تهی findConstraintFunc(
  sqlite3_context *ctx,
  صحیح NotUsed,
  sqlite3_مقدار **argv
){
  ثابت u8 *zSql = 0;
  ثابت u8 *zCons = 0;
  صحیح iOff = 0;
  صحیح t = 0;

  (تهی)NotUsed;
  zSql = sqlite3_مقدار_text(argv[0]);
  zCons = sqlite3_مقدار_text(argv[1]);

  اگر( zSql==0 || zCons==0 ) بازگشت;
  وقتی( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  وقتی( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    اگر( t==TK_CONSTRAINT ){
      صحیح nTok = 0;
      صحیح cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      اگر( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) بازگشت;
      اگر( cmp==0 ){
        sqlite3_result_int(ctx, 1);
        بازگشت;
      }
    }وگرنه اگر( t==TK_ILLEGAL ){
      بشکن;
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
تهی sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ثابت نویسه *z_عبارت,       /* Text of CHECK expression */
  صحیح n_عبارت,               /* Size of pExpr in bytes */
  عبارت *p_عبارت              /* The parsed CHECK expression */
){ 
  Table *pTab = 0;         /* Table identified by pSrc */
  صحیح iDb = 0;             /* Which schema does pTab live in */
  ثابت نویسه *zDb = 0;     /* Name of the schema in which pTab lives */
  ثابت نویسه *pCons = 0;   /* Text of the constraint */
  صحیح nCons;               /* Bytes of text to use from pCons[] */
  صحیح rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  اگر( !pTab ){
    sqlite3_عبارت_Delete(pParse->db, p_عبارت);
    بازگشت;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_عبارت, 0);
  sqlite3_عبارت_Delete(pParse->db, p_عبارت);
  اگر( rc ) بازگشت;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  اگر( pName ){
    نویسه *zName = sqlite3NameFromToken(pParse->db, pName);

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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_عبارت, z_عبارت
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
تهی sqlite3AlterFunctions(تهی){
  ایستا FuncDef aAlterTableFuncs[] = {
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
  sqlite3_درج_Builtin_Funcs(aAlterTableFuncs, ArraySize(aAlterTableFuncs));
}
#endif  /* SQLITE_ALTER_TABLE */
