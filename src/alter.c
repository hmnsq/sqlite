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
#అన్తర్భావ "sqliteInt.h"

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
స్థిర పూర్ణాంకం is_Alterable_పట్టిక(Parse *pParse, పట్టిక *pTab){
  ఒకవేళ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#అయితే_ముగింపు
  ){
    sqlite3_Error_సందేశం(pParse, "table %s may not be altered", pTab->zName);
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
స్థిర శూన్య renameTestSchema(
  Parse *pParse,                  /* Parse context */
  స్థిరరాశి అక్షరం *zDb,                /* Name of db to verify schema of */
  పూర్ణాంకం b_తాత్కాలిక,                      /* True if this is the temp db */
  స్థిరరాశి అక్షరం *zWhen,              /* "when" part of error message */
  పూర్ణాంకం bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->నిలువు_పేర్లు_సమితి = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_పట్టిక " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_తాత్కాలిక, zWhen, bNoDQS
  );

  ఒకవేళ( b_తాత్కాలిక==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_పట్టిక " "
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
స్థిర శూన్య renameFixQuotes(Parse *pParse, స్థిరరాశి అక్షరం *zDb, పూర్ణాంకం b_తాత్కాలిక){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  ఒకవేళ( b_తాత్కాలిక==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_పట్టిక
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
స్థిర శూన్య renameReloadSchema(Parse *pParse, పూర్ణాంకం iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  ఒకవేళ( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_కూడు_Parse_Schema_Op(pParse->pVdbe, iDb, 0, p5);
    ఒకవేళ( iDb!=1 ) sqlite3_Vdbe_కూడు_Parse_Schema_Op(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
శూన్య sqlite3_Alter_Rename_పట్టిక(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  పూర్ణాంకం iDb;                  /* Database that contains the table */
  అక్షరం *zDb;                /* Name of database iDb */
  పట్టిక *pTab;              /* Table being renamed */
  అక్షరం *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  పూర్ణాంకం nTabName;             /* Number of UTF-8 characters in zTabName */
  స్థిరరాశి అక్షరం *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  ఒకవేళ( NEVER(db->mallocFailed) ) వెళ్ళు exit_rename_పట్టిక;
  ప్రతిజ్ఞా( pSrc->nSrc==1 );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, &pSrc->a[0]);
  ఒకవేళ( !pTab ) వెళ్ళు exit_rename_పట్టిక;
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  ఒకవేళ( !zName ) వెళ్ళు exit_rename_పట్టిక;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  ఒకవేళ( sqlite3_కనుగొను_పట్టిక(db, zName, zDb)
   || sqlite3_కనుగొను_సూచీ(db, zName, zDb)
   || sqlite3_Is_Shadow_పట్టిక_యొక్క(db, pTab, zName)
  ){
    sqlite3_Error_సందేశం(pParse,
        "there is already another table or index with this name: %s", zName);
    వెళ్ళు exit_rename_పట్టిక;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  ఒకవేళ( SQLITE_OK!=is_Alterable_పట్టిక(pParse, pTab) ){
    వెళ్ళు exit_rename_పట్టిక;
  }
  ఒకవేళ( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    వెళ్ళు exit_rename_పట్టిక;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIEW
  ఒకవేళ( IsView(pTab) ){
    sqlite3_Error_సందేశం(pParse, "view %s may not be altered", pTab->zName);
    వెళ్ళు exit_rename_పట్టిక;
  }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, zDb, pTab->zName, 0) ){
    వెళ్ళు exit_rename_పట్టిక;
  }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab) ){
    వెళ్ళు exit_rename_పట్టిక;
  }
  ఒకవేళ( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    ఒకవేళ( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#అయితే_ముగింపు

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v==0 ){
    వెళ్ళు exit_rename_పట్టిక;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_పట్టిక " SET "
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

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  ఒకవేళ( sqlite3_కనుగొను_పట్టిక(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#అయితే_ముగింపు

  /* If the table being renamed is not itself part of the temp database,
  ** edit view and trigger definitions within the temp database
  ** as required.  */
  ఒకవేళ( iDb!=1 ){
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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( pVTab ){
    పూర్ణాంకం i = ++pParse->nMem;
    sqlite3_Vdbe_ఎక్కించు_String(v, i, zName);
    sqlite3_Vdbe_కూడు_Op4(v, OP_VRename, i, 0, 0,(స్థిరరాశి అక్షరం*)pVTab, P4_VTAB);
  }
#అయితే_ముగింపు

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_పట్టిక:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
స్థిర శూన్య sqlite3_Error_If_Not_ఖాళీలు(
  Parse *pParse,        /* Parsing context */
  స్థిరరాశి అక్షరం *zDb,      /* Schema holding the table */
  స్థిరరాశి అక్షరం *zTab,     /* Table to check for empty */
  స్థిరరాశి అక్షరం *zErr      /* Error message text */
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
స్థిర పూర్ణాంకం isRowidAlias(
  Parse *pParse, 
  పట్టిక *pTab, 
  స్థిరరాశి అక్షరం *z_నిలువు, 
  స్థిరరాశి అక్షరం *zOp
){
  ఒకవేళ( HasRowid(pTab) && sqlite3IsRowid(z_నిలువు) ){
    sqlite3_Error_సందేశం(pParse, "cannot %s rowid alias: %s", zOp, z_నిలువు);
    తిరిగి SQLITE_ERROR;
  }
  తిరిగి SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
శూన్య sqlite3_Alter_ముగింపు_కూడు_స్తంభం(Parse *pParse, Token *p_నిలువు_Def){
  పట్టిక *pNew;              /* Copy of pParse->pNewTable */
  పట్టిక *pTab;              /* Table being altered */
  పూర్ణాంకం iDb;                  /* Database number */
  స్థిరరాశి అక్షరం *zDb;          /* Database name */
  స్థిరరాశి అక్షరం *zTab;         /* Table name */
  అక్షరం *z_నిలువు;               /* Null-terminated column definition */
  స్తంభం *p_నిలువు;             /* The new column */
  సమాసం *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  పూర్ణాంకం r1;                   /* Temporary registers */

  db = pParse->db;
  ప్రతిజ్ఞా( db->pParse==pParse );
  ఒకవేళ( pParse->nErr ) తిరిగి;
  ప్రతిజ్ఞా( db->mallocFailed==0 );
  pNew = pParse->p_New_పట్టిక;
  ప్రతిజ్ఞా( pNew );

  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  p_నిలువు = &pNew->a_నిలువు[pNew->n_నిలువు-1];
  pDflt = sqlite3_స్తంభం_సమాసం(pNew, p_నిలువు);
  pTab = sqlite3_కనుగొను_పట్టిక(db, zTab, zDb);
  ప్రతిజ్ఞా( pTab );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, zDb, pTab->zName, 0) ){
    తిరిగి;
  }
#అయితే_ముగింపు


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_PRIMKEY ){
    sqlite3_Error_సందేశం(pParse, "Cannot add a PRIMARY KEY column");
    తిరిగి;
  }
  ఒకవేళ( pNew->p_సూచీ ){
    sqlite3_Error_సందేశం(pParse,
         "Cannot add a UNIQUE column");
    తిరిగి;
  }
  ఒకవేళ( isRowidAlias(pParse, pTab, p_నిలువు->zCnName, "add") ) తిరిగి;
  ఒకవేళ( (p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    ప్రతిజ్ఞా( pDflt==0 || pDflt->op==TK_పరిధి );
    ఒకవేళ( pDflt && pDflt->p_ఎడమ->op==TK_NULL ){
      pDflt = 0;
    }
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pNew) );
    ఒకవేళ( (db->flags&SQLITE_Foreign_కీలకాలు) && pNew->u.tab.pFKey && pDflt ){
      sqlite3_Error_If_Not_ఖాళీలు(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    ఒకవేళ( p_నిలువు->notNull && !pDflt ){
      sqlite3_Error_If_Not_ఖాళీలు(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    ఒకవేళ( pDflt ){
      sqlite3_విలువ *pVal = 0;
      పూర్ణాంకం rc;
      rc = sqlite3_విలువ_From_సమాసం(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      ప్రతిజ్ఞా( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      ఒకవేళ( rc!=SQLITE_OK ){
        ప్రతిజ్ఞా( db->mallocFailed == 1 );
        తిరిగి;
      }
      ఒకవేళ( !pVal ){
        sqlite3_Error_If_Not_ఖాళీలు(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_విలువ_Free(pVal);
    }
  }లేకపోతే ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_STORED ){
    sqlite3_Error_If_Not_ఖాళీలు(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  z_నిలువు = sqlite3DbStrNDup(db, (అక్షరం*)p_నిలువు_Def->z, p_నిలువు_Def->n);
  ఒకవేళ( z_నిలువు ){
    అక్షరం *z_చివర = &z_నిలువు[p_నిలువు_Def->n-1];
    అయితే( z_చివర>z_నిలువు && (*z_చివర==';' || sqlite3Isspace(*z_చివర)) ){
      *z_చివర-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.కూడు_నిలువు_స్థానభ్రంశం, z_నిలువు, pNew->u.tab.కూడు_నిలువు_స్థానభ్రంశం,
      zTab
    );
    sqlite3DbFree(db, z_నిలువు);
  }

  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    sqlite3_Vdbe_కూడు_Op3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb);
    sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, r1, -2);
    sqlite3_Vdbe_కూడు_Op2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op3(v, OP_సమితి_Cookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_Alter_కూడు);

    /* Verify that constraints are still satisfied */
    ఒకవేళ( pNew->pCheck!=0
     || (p_నిలువు->notNull && (p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->నిలువు_పేర్లు_సమితి = 1;
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
శూన్య sqlite3_Alter_Begin_కూడు_స్తంభం(Parse *pParse, SrcList *pSrc){
  పట్టిక *pNew;
  పట్టిక *pTab;
  పూర్ణాంకం iDb;
  పూర్ణాంకం i;
  పూర్ణాంకం nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  ప్రతిజ్ఞా( pParse->p_New_పట్టిక==0 );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  ఒకవేళ( NEVER(db->mallocFailed) ) వెళ్ళు exit_begin_కూడు_స్తంభం;
  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, &pSrc->a[0]);
  ఒకవేళ( !pTab ) వెళ్ళు exit_begin_కూడు_స్తంభం;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( IsVirtual(pTab) ){
    sqlite3_Error_సందేశం(pParse, "virtual tables may not be altered");
    వెళ్ళు exit_begin_కూడు_స్తంభం;
  }
#అయితే_ముగింపు

  /* Make sure this is not an attempt to ALTER a view. */
  ఒకవేళ( IsView(pTab) ){
    sqlite3_Error_సందేశం(pParse, "Cannot add a column to a view");
    వెళ్ళు exit_begin_కూడు_స్తంభం;
  }
  ఒకవేళ( SQLITE_OK!=is_Alterable_పట్టిక(pParse, pTab) ){
    వెళ్ళు exit_begin_కూడు_స్తంభం;
  }

  sqlite3MayAbort(pParse);
  ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
  ప్రతిజ్ఞా( pTab->u.tab.కూడు_నిలువు_స్థానభ్రంశం>0 );
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (పట్టిక*)sqlite3DbMallocZero(db, పరిమాణం(పట్టిక));
  ఒకవేళ( !pNew ) వెళ్ళు exit_begin_కూడు_స్తంభం;
  pParse->p_New_పట్టిక = pNew;
  pNew->nTabRef = 1;
  pNew->n_నిలువు = pTab->n_నిలువు;
  ప్రతిజ్ఞా( pNew->n_నిలువు>0 );
  nAlloc = (((pNew->n_నిలువు-1)/8)*8)+8;
  ప్రతిజ్ఞా( nAlloc>=pNew->n_నిలువు && nAlloc%8==0 && nAlloc-pNew->n_నిలువు<8 );
  pNew->a_నిలువు = (స్తంభం*)sqlite3DbMallocZero(db, పరిమాణం(స్తంభం)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  ఒకవేళ( !pNew->a_నిలువు || !pNew->zName ){
    ప్రతిజ్ఞా( db->mallocFailed );
    వెళ్ళు exit_begin_కూడు_స్తంభం;
  }
  జ్ఞాపకం_నకలు(pNew->a_నిలువు, pTab->a_నిలువు, పరిమాణం(స్తంభం)*(size_t)pNew->n_నిలువు);
  కొరకు(i=0; i<pNew->n_నిలువు; i++){
    స్తంభం *p_నిలువు = &pNew->a_నిలువు[i];
    p_నిలువు->zCnName = sqlite3DbStrDup(db, p_నిలువు->zCnName);
    p_నిలువు->hName = sqlite3StrIHash(p_నిలువు->zCnName);
  }
  ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pNew) );
  pNew->u.tab.pDfltList = sqlite3_సమాసం_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.కూడు_నిలువు_స్థానభ్రంశం = pTab->u.tab.కూడు_నిలువు_స్థానభ్రంశం;
  ప్రతిజ్ఞా( pNew->nTabRef==1 );

exit_begin_కూడు_స్తంభం:
  sqlite3SrcListDelete(db, pSrc);
  తిరిగి;
}

/*
** Parameter pTab is the subject of an ALTER TABLE ... RENAME COLUMN
** command. This function checks if the table is a view or virtual
** table (columns of views or virtual tables may not be renamed). If so,
** it loads an error message into pParse and returns non-zero.
**
** Or, if pTab is not a view or virtual table, zero is returned.
*/
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
స్థిర పూర్ణాంకం is_Real_పట్టిక(Parse *pParse, పట్టిక *pTab, పూర్ణాంకం iOp){
  స్థిరరాశి అక్షరం *zType = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIEW
  ఒకవేళ( IsView(pTab) ){
    zType = "view";
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#అయితే_ముగింపు
  ఒకవేళ( zType ){
    స్థిరరాశి అక్షరం *az_సందేశం[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    ప్రతిజ్ఞా( iOp>=0 && iOp<Array_కొలత(az_సందేశం) );
    sqlite3_Error_సందేశం(pParse, "cannot %s %s \"%s\"",
        az_సందేశం[iOp], zType, pTab->zName
    );
    తిరిగి 1;
  }
  తిరిగి 0;
}
#లేకపోతే /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define is_Real_పట్టిక(x,y,z) (0)
#అయితే_ముగింపు

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
శూన్య sqlite3_Alter_Rename_స్తంభం(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  పట్టిక *pTab;                    /* Table being updated */
  పూర్ణాంకం i_నిలువు;                       /* Index of column being renamed */
  అక్షరం *zOld = 0;                 /* Old column name */
  అక్షరం *zNew = 0;                 /* New column name */
  స్థిరరాశి అక్షరం *zDb;                /* Name of schema containing the table */
  పూర్ణాంకం iSchema;                    /* Index of the schema */
  పూర్ణాంకం bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, &pSrc->a[0]);
  ఒకవేళ( !pTab ) వెళ్ళు exit_rename_స్తంభం;

  /* Cannot alter a system table */
  ఒకవేళ( SQLITE_OK!=is_Alterable_పట్టిక(pParse, pTab) ) వెళ్ళు exit_rename_స్తంభం;
  ఒకవేళ( SQLITE_OK!=is_Real_పట్టిక(pParse, pTab, 0) ) వెళ్ళు exit_rename_స్తంభం;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, zDb, pTab->zName, 0) ){
    వెళ్ళు exit_rename_స్తంభం;
  }
#అయితే_ముగింపు

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  ఒకవేళ( !zOld ) వెళ్ళు exit_rename_స్తంభం;
  i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, zOld);
  ఒకవేళ( i_నిలువు<0 ){
    sqlite3_Error_సందేశం(pParse, "no such column: \"%T\"", pOld);
    వెళ్ళు exit_rename_స్తంభం;
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
  ఒకవేళ( !zNew ) వెళ్ళు exit_rename_స్తంభం;
  ఒకవేళ( isRowidAlias(pParse, pTab, zOld, "rename from") ) వెళ్ళు exit_rename_స్తంభం;
  ఒకవేళ( isRowidAlias(pParse, pTab, zNew, "rename to") ) వెళ్ళు exit_rename_స్తంభం;
  ప్రతిజ్ఞా( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, i_నిలువు, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  ఒకవేళ( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_పట్టిక " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, i_నిలువు, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_స్తంభం:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  తిరిగి;
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
నిర్మాణం RenameToken {
  స్థిరరాశి శూన్య *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_తదుపరి;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
రకనిర్వచనం నిర్మాణం RenameCtx RenameCtx;
నిర్మాణం RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  పూర్ణాంకం nList;                      /* Number of tokens in pList */
  పూర్ణాంకం i_నిలువు;                       /* Index of column being renamed */
  పట్టిక *pTab;                    /* Table being ALTERed */
  స్థిరరాశి అక్షరం *zOld;               /* Old column name */
};

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర శూన్య renameTokenCheckAll(Parse *pParse, స్థిరరాశి శూన్య *pPtr){
  ప్రతిజ్ఞా( pParse==pParse->db->pParse );
  ప్రతిజ్ఞా( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ఒకవేళ( pParse->nErr==0 ){
    స్థిరరాశి RenameToken *p;
    u32 i = 1;
    కొరకు(p=pParse->pRename; p; p=p->p_తదుపరి){
      ఒకవేళ( p->p ){
        ప్రతిజ్ఞా( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    ప్రతిజ్ఞా( i>0 );
  }
}
#లేకపోతే
# define renameTokenCheckAll(x,y)
#అయితే_ముగింపు

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
స్థిరరాశి శూన్య *sqlite3RenameTokenMap(
  Parse *pParse,
  స్థిరరాశి శూన్య *pPtr,
  స్థిరరాశి Token *pToken
){
  RenameToken *pNew;
  ప్రతిజ్ఞా( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  ఒకవేళ( ALWAYS(pParse->e_Parse_బహుళకం!=PARSE_బహుళకం_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, పరిమాణం(RenameToken));
    ఒకవేళ( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_తదుపరి = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  తిరిగి pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
శూన్య sqlite3RenameTokenRemap(Parse *pParse, స్థిరరాశి శూన్య *p_గమ్యం, స్థిరరాశి శూన్య *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, p_గమ్యం);
  కొరకు(p=pParse->pRename; p; p=p->p_తదుపరి){
    ఒకవేళ( p->p==pFrom ){
      p->p = p_గమ్యం;
      ఆపు;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
స్థిర పూర్ణాంకం rename_Unmap_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (స్థిరరాశి శూన్య*)p_సమాసం);
  ఒకవేళ( సమాసం_Use_YTab(p_సమాసం) ){
    sqlite3RenameTokenRemap(pParse, 0, (స్థిరరాశి శూన్య*)&p_సమాసం->y.pTab);
  }
  తిరిగి WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
స్థిర శూన్య renameWalkWith(Walker *pWalker, Select *pSelect){
  With *pWith = pSelect->pWith;
  ఒకవేళ( pWith ){
    Parse *pParse = pWalker->pParse;
    పూర్ణాంకం i;
    With *pCopy = 0;
    ప్రతిజ్ఞా( pWith->nCte>0 );
    ఒకవేళ( (pWith->a[0].pSelect->selFlags & SF_Expanded)==0 ){
      /* Push a copy of the With object onto the with-stack. We use a copy
      ** here as the original will be expanded and resolved (flags SF_Expanded
      ** and SF_Resolved) below. And the parser code that uses the with-stack
      ** fails if the Select objects on it have already been expanded and
      ** resolved.  */
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3_With_నెట్టు(pParse, pCopy, 1);
    }
    కొరకు(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, పరిమాణం(sNC));
      sNC.pParse = pParse;
      ఒకవేళ( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      ఒకవేళ( sNC.pParse->db->mallocFailed ) తిరిగి;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].p_నిలువుల_సంఖ్య);
    }
    ఒకవేళ( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
స్థిర శూన్య unmap_స్తంభం_Idlist_పేర్లు(
  Parse *pParse,
  స్థిరరాశి IdList *pIdList
){
  పూర్ణాంకం ii;
  ప్రతిజ్ఞా( pIdList!=0 );
  కొరకు(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (స్థిరరాశి శూన్య*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
స్థిర పూర్ణాంకం renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  పూర్ణాంకం i;
  ఒకవేళ( pParse->nErr ) తిరిగి WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  ఒకవేళ( p->selFlags & (SF_View|SF_CopyCte) ){
    తిరిగి WRC_Prune;
  }
  ఒకవేళ( ALWAYS(p->pEList) ){
    సమాసం_List *pList = p->pEList;
    కొరకు(i=0; i<pList->n_సమాసం; i++){
      ఒకవేళ( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)pList->a[i].zEName);
      }
    }
  }
  ఒకవేళ( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    కొరకు(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)pSrc->a[i].zName);
      ఒకవేళ( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_సమాసం(pWalker, pSrc->a[i].u3.pOn);
      }లేకపోతే{
        unmap_స్తంభం_Idlist_పేర్లు(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  తిరిగి WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
శూన్య sqlite3_Rename_సమాసం_Unmap(Parse *pParse, సమాసం *p_సమాసం){
  u8 e_బహుళకం = pParse->e_Parse_బహుళకం;
  Walker sWalker;
  memset(&sWalker, 0, పరిమాణం(Walker));
  sWalker.pParse = pParse;
  sWalker.x_సమాసం_Callback = rename_Unmap_సమాసం_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->e_Parse_బహుళకం = PARSE_బహుళకం_UNMAP;
  sqlite3_Walk_సమాసం(&sWalker, p_సమాసం);
  pParse->e_Parse_బహుళకం = e_బహుళకం;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
శూన్య sqlite3RenameExprlistUnmap(Parse *pParse, సమాసం_List *pEList){
  ఒకవేళ( pEList ){
    పూర్ణాంకం i;
    Walker sWalker;
    memset(&sWalker, 0, పరిమాణం(Walker));
    sWalker.pParse = pParse;
    sWalker.x_సమాసం_Callback = rename_Unmap_సమాసం_Cb;
    sqlite3_Walk_సమాసం_List(&sWalker, pEList);
    కొరకు(i=0; i<pEList->n_సమాసం; i++){
      ఒకవేళ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
స్థిర శూన్య renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_తదుపరి;
  RenameToken *p;
  కొరకు(p=pToken; p; p=p_తదుపరి){
    p_తదుపరి = p->p_తదుపరి;
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
స్థిర RenameToken *rename_Token_కనుగొను(
  Parse *pParse,
  నిర్మాణం RenameCtx *pCtx,
  స్థిరరాశి శూన్య *pPtr
){
  RenameToken **pp;
  ఒకవేళ( NEVER(pPtr==0) ){
    తిరిగి 0;
  }
  కొరకు(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_తదుపరి){
    ఒకవేళ( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      ఒకవేళ( pCtx ){
        *pp = pToken->p_తదుపరి;
        pToken->p_తదుపరి = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      తిరిగి pToken;
    }
  }
  తిరిగి 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
స్థిర పూర్ణాంకం rename_స్తంభం_Select_Cb(Walker *pWalker, Select *p){
  ఒకవేళ( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    తిరిగి WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  తిరిగి WRC_Continue;
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
స్థిర పూర్ణాంకం rename_స్తంభం_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  RenameCtx *p = pWalker->u.pRename;
  ఒకవేళ( p_సమాసం->op==TK_TRIGGER
   && p_సమాసం->i_స్తంభం==p->i_నిలువు
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    rename_Token_కనుగొను(pWalker->pParse, p, (శూన్య*)p_సమాసం);
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   && p_సమాసం->i_స్తంభం==p->i_నిలువు
   && ALWAYS(సమాసం_Use_YTab(p_సమాసం))
   && p->pTab==p_సమాసం->y.pTab
  ){
    rename_Token_కనుగొను(pWalker->pParse, p, (శూన్య*)p_సమాసం);
  }
  తిరిగి WRC_Continue;
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
స్థిర RenameToken *rename_స్తంభం_Token_తదుపరి(RenameCtx *pCtx){
  RenameToken *p_ఉత్తమ = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  కొరకు(pToken=p_ఉత్తమ->p_తదుపరి; pToken; pToken=pToken->p_తదుపరి){
    ఒకవేళ( pToken->t.z>p_ఉత్తమ->t.z ) p_ఉత్తమ = pToken;
  }
  కొరకు(pp=&pCtx->pList; *pp!=p_ఉత్తమ; pp=&(*pp)->p_తదుపరి);
  *pp = p_ఉత్తమ->p_తదుపరి;

  తిరిగి p_ఉత్తమ;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
స్థిర శూన్య errorMPrintf(sqlite3_context *pCtx, స్థిరరాశి అక్షరం *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  అక్షరం *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  ఒకవేళ( zErr ){
    sqlite3_ఫలితం_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }లేకపోతే{
    sqlite3_ఫలితం_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
స్థిర శూన్య rename_స్తంభం_Parse_Error(
  sqlite3_context *pCtx,
  స్థిరరాశి అక్షరం *zWhen,
  sqlite3_విలువ *pType,
  sqlite3_విలువ *pObject,
  Parse *pParse
){
  స్థిరరాశి అక్షరం *zT = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pType);
  స్థిరరాశి అక్షరం *zN = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pObject);
  అక్షరం *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_సందేశం
  );
  sqlite3_ఫలితం_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
స్థిర శూన్య rename_స్తంభం_Elist_పేర్లు(
  Parse *pParse,
  RenameCtx *pCtx,
  స్థిరరాశి సమాసం_List *pEList,
  స్థిరరాశి అక్షరం *zOld
){
  ఒకవేళ( pEList ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pEList->n_సమాసం; i++){
      స్థిరరాశి అక్షరం *zName = pEList->a[i].zEName;
      ఒకవేళ( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        rename_Token_కనుగొను(pParse, pCtx, (స్థిరరాశి శూన్య*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
స్థిర శూన్య rename_స్తంభం_Idlist_పేర్లు(
  Parse *pParse,
  RenameCtx *pCtx,
  స్థిరరాశి IdList *pIdList,
  స్థిరరాశి అక్షరం *zOld
){
  ఒకవేళ( pIdList ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pIdList->nId; i++){
      స్థిరరాశి అక్షరం *zName = pIdList->a[i].zName;
      ఒకవేళ( 0==sqlite3_stricmp(zName, zOld) ){
        rename_Token_కనుగొను(pParse, pCtx, (స్థిరరాశి శూన్య*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
స్థిర పూర్ణాంకం renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  స్థిరరాశి అక్షరం *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  స్థిరరాశి అక్షరం *zSql,               /* SQL to parse */
  పూర్ణాంకం b_తాత్కాలిక                       /* True if SQL is from temp schema */
){
  పూర్ణాంకం rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  ఒకవేళ( zSql==0 ){
    తిరిగి SQLITE_NOMEM;
  }
  ఒకవేళ( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ఒకవేళ( b_తాత్కాలిక ){
    db->init.iDb = 1;
  }లేకపోతే{
    పూర్ణాంకం iDb = sqlite3_కనుగొను_Db_Name(db, zDb);
    ప్రతిజ్ఞా( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->e_Parse_బహుళకం = PARSE_బహుళకం_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && అక్షరశ్రేణి_శోధన(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3_నిరంతరం_Parser(p, zSql);
  db->flags = flags;
  ఒకవేళ( db->mallocFailed ) rc = SQLITE_NOMEM;
  ఒకవేళ( rc==SQLITE_OK
   && NEVER(p->p_New_పట్టిక==0 && p->p_New_సూచీ==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#నిర్వచితమైతే SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    కొరకు(pToken=p->pRename; pToken; pToken=pToken->p_తదుపరి){
      ప్రతిజ్ఞా( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#అయితే_ముగింపు

  db->init.iDb = 0;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  స్థిరరాశి అక్షరం *zSql,               /* SQL statement to edit */
  స్థిరరాశి అక్షరం *zNew,               /* New token text */
  పూర్ణాంకం bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  పూర్ణాంకం rc = SQLITE_OK;
  అక్షరం *zQuot = 0;
  అక్షరం *z_బహిర్గతం;
  i64 nQuot = 0;
  అక్షరం *zBuf1 = 0;
  అక్షరం *zBuf2 = 0;

  ఒకవేళ( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    ఒకవేళ( zQuot==0 ){
      తిరిగి SQLITE_NOMEM;
    }లేకపోతే{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    ప్రతిజ్ఞా( nQuot>=nNew && nSql>=0 && nNew>=0 );
    z_బహిర్గతం = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }లేకపోతే{
    ప్రతిజ్ఞా( nSql>0 );
    z_బహిర్గతం = (అక్షరం*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    ఒకవేళ( z_బహిర్గతం ){
      zBuf1 = &z_బహిర్గతం[nSql*2+1];
      zBuf2 = &z_బహిర్గతం[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  ఒకవేళ( z_బహిర్గతం ){
    i64 n_బహిర్గతం = nSql;
    ప్రతిజ్ఞా( nSql>0 );
    జ్ఞాపకం_నకలు(z_బహిర్గతం, zSql, (size_t)nSql);
    అయితే( pRename->pList ){
      పూర్ణాంకం iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      స్థిరరాశి అక్షరం *zReplace;
      RenameToken *p_ఉత్తమ = rename_స్తంభం_Token_తదుపరి(pRename);

      ఒకవేళ( zNew ){
        ఒకవేళ( bQuote==0 && sqlite3IsIdChar(*(u8*)p_ఉత్తమ->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }లేకపోతే{
          nReplace = nQuot;
          zReplace = zQuot;
          ఒకవేళ( p_ఉత్తమ->t.z[p_ఉత్తమ->t.n]=='"' ) nReplace++;
        }
      }లేకపోతే{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        జ్ఞాపకం_నకలు(zBuf1, p_ఉత్తమ->t.z, p_ఉత్తమ->t.n);
        zBuf1[p_ఉత్తమ->t.n] = 0;
        sqlite3Dequote(zBuf1);
        ప్రతిజ్ఞా( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((పూర్ణాంకం)(nSql*2), zBuf2, "%Q%s", zBuf1,
            p_ఉత్తమ->t.z[p_ఉత్తమ->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (పూర్ణాంకం)(p_ఉత్తమ->t.z - zSql);
      ఒకవేళ( p_ఉత్తమ->t.n!=nReplace ){
        memmove(&z_బహిర్గతం[iOff + nReplace], &z_బహిర్గతం[iOff + p_ఉత్తమ->t.n],
            n_బహిర్గతం - (iOff + p_ఉత్తమ->t.n)
        );
        n_బహిర్గతం += nReplace - p_ఉత్తమ->t.n;
        z_బహిర్గతం[n_బహిర్గతం] = '\0';
      }
      జ్ఞాపకం_నకలు(&z_బహిర్గతం[iOff], zReplace, nReplace);
      sqlite3DbFree(db, p_ఉత్తమ);
    }

    sqlite3_ఫలితం_పాఠ్యం(pCtx, z_బహిర్గతం, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, z_బహిర్గతం);
  }లేకపోతే{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  తిరిగి rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
స్థిర శూన్య rename_సమితి_ENames(సమాసం_List *pEList, పూర్ణాంకం val){
  ప్రతిజ్ఞా( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_పరిధి );
  ఒకవేళ( pEList ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pEList->n_సమాసం; i++){
      ప్రతిజ్ఞా( val==ENAME_NAME || pEList->a[i].fg.eEName==ENAME_NAME );
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
స్థిర పూర్ణాంకం renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  Trigger_అడుగు *p_అడుగు;
  NameContext sNC;
  పూర్ణాంకం rc = SQLITE_OK;

  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pParse = pParse;
  ప్రతిజ్ఞా( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3_కనుగొను_పట్టిక(db, pNew->పట్టిక_2,
      db->aDb[sqlite3_Schema_గమ్యం_సూచీ(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  ఒకవేళ( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  ఒకవేళ( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pNew->pWhen);
  }

  కొరకు(p_అడుగు=pNew->అడుగు_list; rc==SQLITE_OK && p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
    ఒకవేళ( p_అడుగు->pSelect ){
      sqlite3SelectPrep(pParse, p_అడుగు->pSelect, &sNC);
      ఒకవేళ( pParse->nErr ) rc = pParse->rc;
    }
    ఒకవేళ( rc==SQLITE_OK && p_అడుగు->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, p_అడుగు->pSrc, 0);
      ఒకవేళ( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, p_అడుగు->p_సమాసం_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        ఒకవేళ( pSel==0 ){
          p_అడుగు->p_సమాసం_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }లేకపోతే{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          rename_సమితి_ENames(p_అడుగు->p_సమాసం_List, ENAME_పరిధి);
          sqlite3SelectPrep(pParse, pSel, 0);
          rename_సమితి_ENames(p_అడుగు->p_సమాసం_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          ప్రతిజ్ఞా( p_అడుగు->p_సమాసం_List==0 || p_అడుగు->p_సమాసం_List==pSel->pEList );
          ప్రతిజ్ఞా( pSrc==pSel->pSrc );
          ఒకవేళ( p_అడుగు->p_సమాసం_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        ఒకవేళ( ALWAYS(p_అడుగు->pSrc) ){
          పూర్ణాంకం i;
          కొరకు(i=0; i<p_అడుగు->pSrc->nSrc && rc==SQLITE_OK; i++){
            Src_అంశం *p = &p_అడుగు->pSrc->a[i];
            ఒకవేళ( p->fg.isSubquery ){
              ప్రతిజ్ఞా( p->u4.pSubq!=0 );
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        ఒకవేళ(  db->mallocFailed ){
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        ఒకవేళ( rc==SQLITE_OK && p_అడుగు->pWhere ){
          rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, p_అడుగు->pWhere);
        }
        ఒకవేళ( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, p_అడుగు->p_సమాసం_List);
        }
        ప్రతిజ్ఞా( !p_అడుగు->pUpsert || (!p_అడుగు->pWhere && !p_అడుగు->p_సమాసం_List) );
        ఒకవేళ( p_అడుగు->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = p_అడుగు->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pUpsert->p_Upsert_లక్ష్యం);
          ఒకవేళ( rc==SQLITE_OK ){
            సమాసం_List *p_Upsert_సమితి = pUpsert->p_Upsert_సమితి;
            rc = sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, p_Upsert_సమితి);
          }
          ఒకవేళ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pUpsert->pUpsertWhere);
          }
          ఒకవేళ( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pUpsert->p_Upsert_లక్ష్యం_Where);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }లేకపోతే{
        rc = SQLITE_NOMEM;
      }
    }
  }
  తిరిగి rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
స్థిర శూన్య renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  Trigger_అడుగు *p_అడుగు;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_సమాసం(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  కొరకు(p_అడుగు=pTrigger->అడుగు_list; p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
    sqlite3WalkSelect(pWalker, p_అడుగు->pSelect);
    sqlite3_Walk_సమాసం(pWalker, p_అడుగు->pWhere);
    sqlite3_Walk_సమాసం_List(pWalker, p_అడుగు->p_సమాసం_List);
    ఒకవేళ( p_అడుగు->pUpsert ){
      Upsert *pUpsert = p_అడుగు->pUpsert;
      sqlite3_Walk_సమాసం_List(pWalker, pUpsert->p_Upsert_లక్ష్యం);
      sqlite3_Walk_సమాసం_List(pWalker, pUpsert->p_Upsert_సమితి);
      sqlite3_Walk_సమాసం(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_సమాసం(pWalker, pUpsert->p_Upsert_లక్ష్యం_Where);
    }
    ఒకవేళ( p_అడుగు->pSrc ){
      పూర్ణాంకం i;
      SrcList *pSrc = p_అడుగు->pSrc;
      కొరకు(i=0; i<pSrc->nSrc; i++){
        ఒకవేళ( pSrc->a[i].fg.isSubquery ){
          ప్రతిజ్ఞా( pSrc->a[i].u4.pSubq!=0 );
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
స్థిర శూన్య renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  సూచీ *pIdx;
  ఒకవేళ( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3_Delete_పట్టిక(db, pParse->p_New_పట్టిక);
  అయితే( (pIdx = pParse->p_New_సూచీ)!=0 ){
    pParse->p_New_సూచీ = pIdx->p_తదుపరి;
    sqlite3_Free_సూచీ(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_సందేశం);
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
స్థిర శూన్య rename_స్తంభం_Func(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  స్థిరరాశి అక్షరం *zSql = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  స్థిరరాశి అక్షరం *zDb = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[3]);
  స్థిరరాశి అక్షరం *z_పట్టిక = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[4]);
  పూర్ణాంకం i_నిలువు = sqlite3_విలువ_int(argv[5]);
  స్థిరరాశి అక్షరం *zNew = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[6]);
  పూర్ణాంకం bQuote = sqlite3_విలువ_int(argv[7]);
  పూర్ణాంకం b_తాత్కాలిక = sqlite3_విలువ_int(argv[8]);
  స్థిరరాశి అక్షరం *zOld;
  పూర్ణాంకం rc;
  Parse sParse;
  Walker sWalker;
  సూచీ *pIdx;
  పూర్ణాంకం i;
  పట్టిక *pTab;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#అయితే_ముగింపు

  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( zSql==0 ) తిరిగి;
  ఒకవేళ( z_పట్టిక==0 ) తిరిగి;
  ఒకవేళ( zNew==0 ) తిరిగి;
  ఒకవేళ( i_నిలువు<0 ) తిరిగి;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3_కనుగొను_పట్టిక(db, z_పట్టిక, zDb);
  ఒకవేళ( pTab==0 || i_నిలువు>=pTab->n_నిలువు ){
    sqlite3BtreeLeaveAll(db);
    తిరిగి;
  }
  zOld = pTab->a_నిలువు[i_నిలువు].zCnName;
  memset(&sCtx, 0, పరిమాణం(sCtx));
  sCtx.i_నిలువు = ((i_నిలువు==pTab->iPKey) ? -1 : i_నిలువు);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#అయితే_ముగింపు
  rc = renameParseSql(&sParse, zDb, db, zSql, b_తాత్కాలిక);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, పరిమాణం(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_సమాసం_Callback = rename_స్తంభం_సమాసం_Cb;
  sWalker.xSelectCallback = rename_స్తంభం_Select_Cb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు rename_స్తంభం_Func_పూర్తి;
  ఒకవేళ( sParse.p_New_పట్టిక ){
    ఒకవేళ( IsView(sParse.p_New_పట్టిక) ){
      Select *pSelect = sParse.p_New_పట్టిక->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      ఒకవేళ( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు rename_స్తంభం_Func_పూర్తి;
    }లేకపోతే ఒకవేళ( Is_Ordinary_పట్టిక(sParse.p_New_పట్టిక) ){
      /* A regular table */
      పూర్ణాంకం bFKOnly = sqlite3_stricmp(z_పట్టిక, sParse.p_New_పట్టిక->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.p_New_పట్టిక;
      ఒకవేళ( bFKOnly==0 ){
        ఒకవేళ( i_నిలువు<sParse.p_New_పట్టిక->n_నిలువు ){
          rename_Token_కనుగొను(
              &sParse, &sCtx, (శూన్య*)sParse.p_New_పట్టిక->a_నిలువు[i_నిలువు].zCnName
          );
        }
        ఒకవేళ( sCtx.i_నిలువు<0 ){
          rename_Token_కనుగొను(&sParse, &sCtx, (శూన్య*)&sParse.p_New_పట్టిక->iPKey);
        }
        sqlite3_Walk_సమాసం_List(&sWalker, sParse.p_New_పట్టిక->pCheck);
        కొరకు(pIdx=sParse.p_New_పట్టిక->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
          sqlite3_Walk_సమాసం_List(&sWalker, pIdx->a_నిలువు_సమాసం);
        }
        కొరకు(pIdx=sParse.p_New_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
          sqlite3_Walk_సమాసం_List(&sWalker, pIdx->a_నిలువు_సమాసం);
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
        కొరకు(i=0; i<sParse.p_New_పట్టిక->n_నిలువు; i++){
          సమాసం *p_సమాసం = sqlite3_స్తంభం_సమాసం(sParse.p_New_పట్టిక,
                                                  &sParse.p_New_పట్టిక->a_నిలువు[i]);
          sqlite3_Walk_సమాసం(&sWalker, p_సమాసం);
        }
#అయితే_ముగింపు
      }

      ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(sParse.p_New_పట్టిక) );
      కొరకు(pFKey=sParse.p_New_పట్టిక->u.tab.pFKey; pFKey; pFKey=pFKey->p_తదుపరి_From){
        కొరకు(i=0; i<pFKey->n_నిలువు; i++){
          ఒకవేళ( bFKOnly==0 && pFKey->a_నిలువు[i].iFrom==i_నిలువు ){
            rename_Token_కనుగొను(&sParse, &sCtx, (శూన్య*)&pFKey->a_నిలువు[i]);
          }
          ఒకవేళ( 0==sqlite3_stricmp(pFKey->z_గమ్యం, z_పట్టిక)
           && 0==sqlite3_stricmp(pFKey->a_నిలువు[i].z_నిలువు, zOld)
          ){
            rename_Token_కనుగొను(&sParse, &sCtx, (శూన్య*)pFKey->a_నిలువు[i].z_నిలువు);
          }
        }
      }
    }
  }లేకపోతే ఒకవేళ( sParse.p_New_సూచీ ){
    sqlite3_Walk_సమాసం_List(&sWalker, sParse.p_New_సూచీ->a_నిలువు_సమాసం);
    sqlite3_Walk_సమాసం(&sWalker, sParse.p_New_సూచీ->pPartIdxWhere);
  }లేకపోతే{
    /* A trigger */
    Trigger_అడుగు *p_అడుగు;
    rc = renameResolveTrigger(&sParse);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు rename_స్తంభం_Func_పూర్తి;

    కొరకు(p_అడుగు=sParse.pNewTrigger->అడుగు_list; p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
      ఒకవేళ( p_అడుగు->pSrc ){
        పట్టిక *p_లక్ష్యం = sqlite3_Locate_పట్టిక_అంశం(&sParse, 0, &p_అడుగు->pSrc->a[0]);
        ఒకవేళ( p_లక్ష్యం==pTab ){
          ఒకవేళ( p_అడుగు->pUpsert ){
            సమాసం_List *p_Upsert_సమితి = p_అడుగు->pUpsert->p_Upsert_సమితి;
            rename_స్తంభం_Elist_పేర్లు(&sParse, &sCtx, p_Upsert_సమితి, zOld);
          }
          rename_స్తంభం_Idlist_పేర్లు(&sParse, &sCtx, p_అడుగు->pIdList, zOld);
          rename_స్తంభం_Elist_పేర్లు(&sParse, &sCtx, p_అడుగు->p_సమాసం_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    ఒకవేళ( sParse.pTriggerTab==pTab ){
      rename_స్తంభం_Idlist_పేర్లు(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  ప్రతిజ్ఞా( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

rename_స్తంభం_Func_పూర్తి:
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_ఫలితం_విలువ(context, argv[0]);
    }లేకపోతే ఒకవేళ( sParse.z_Err_సందేశం ){
      rename_స్తంభం_Parse_Error(context, "", argv[1], argv[2], &sParse);
    }లేకపోతే{
      sqlite3_ఫలితం_error_సంకేతం(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#అయితే_ముగింపు
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
స్థిర పూర్ణాంకం rename_పట్టిక_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  RenameCtx *p = pWalker->u.pRename;
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   && ALWAYS(సమాసం_Use_YTab(p_సమాసం))
   && p->pTab==p_సమాసం->y.pTab
  ){
    rename_Token_కనుగొను(pWalker->pParse, p, (శూన్య*)&p_సమాసం->y.pTab);
  }
  తిరిగి WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
స్థిర పూర్ణాంకం rename_పట్టిక_Select_Cb(Walker *pWalker, Select *pSelect){
  పూర్ణాంకం i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  ఒకవేళ( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    తిరిగి WRC_Prune;
  }
  ఒకవేళ( NEVER(pSrc==0) ){
    ప్రతిజ్ఞా( pWalker->pParse->db->mallocFailed );
    తిరిగి WRC_Abort;
  }
  కొరకు(i=0; i<pSrc->nSrc; i++){
    Src_అంశం *p_అంశం = &pSrc->a[i];
    ఒకవేళ( p_అంశం->pSTab==p->pTab ){
      rename_Token_కనుగొను(pWalker->pParse, p, p_అంశం->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  తిరిగి WRC_Continue;
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
స్థిర శూన్య rename_పట్టిక_Func(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  స్థిరరాశి అక్షరం *zDb = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  స్థిరరాశి అక్షరం *z_ఆదానం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[3]);
  స్థిరరాశి అక్షరం *zOld = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[4]);
  స్థిరరాశి అక్షరం *zNew = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[5]);
  పూర్ణాంకం b_తాత్కాలిక = sqlite3_విలువ_int(argv[6]);
  వాడని_PARAMETER(Not_వాడిన);

  ఒకవేళ( z_ఆదానం && zOld && zNew ){
    Parse sParse;
    పూర్ణాంకం rc;
    పూర్ణాంకం bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#అయితే_ముగింపు

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, పరిమాణం(RenameCtx));
    sCtx.pTab = sqlite3_కనుగొను_పట్టిక(db, zOld, zDb);
    memset(&sWalker, 0, పరిమాణం(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_సమాసం_Callback = rename_పట్టిక_సమాసం_Cb;
    sWalker.xSelectCallback = rename_పట్టిక_Select_Cb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_ఆదానం, b_తాత్కాలిక);

    ఒకవేళ( rc==SQLITE_OK ){
      పూర్ణాంకం isLegacy = (db->flags & SQLITE_LegacyAlter);
      ఒకవేళ( sParse.p_New_పట్టిక ){
        పట్టిక *pTab = sParse.p_New_పట్టిక;

        ఒకవేళ( IsView(pTab) ){
          ఒకవేళ( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, పరిమాణం(sNC));
            sNC.pParse = &sParse;

            ప్రతిజ్ఞా( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            ఒకవేళ( sParse.nErr ){
              rc = sParse.rc;
            }లేకపోతే{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }లేకపోతే{
          /* Modify any FK definitions to point to the new table. */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
          ఒకవేళ( (isLegacy==0 || (db->flags & SQLITE_Foreign_కీలకాలు))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
            కొరకు(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_తదుపరి_From){
              ఒకవేళ( sqlite3_stricmp(pFKey->z_గమ్యం, zOld)==0 ){
                rename_Token_కనుగొను(&sParse, &sCtx, (శూన్య*)pFKey->z_గమ్యం);
              }
            }
          }
#అయితే_ముగింపు

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          ఒకవేళ( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            ఒకవేళ( isLegacy==0 ){
              sqlite3_Walk_సమాసం_List(&sWalker, pTab->pCheck);
            }
            rename_Token_కనుగొను(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      లేకపోతే ఒకవేళ( sParse.p_New_సూచీ ){
        rename_Token_కనుగొను(&sParse, &sCtx, sParse.p_New_సూచీ->zName);
        ఒకవేళ( isLegacy==0 ){
          sqlite3_Walk_సమాసం(&sWalker, sParse.p_New_సూచీ->pPartIdxWhere);
        }
      }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
      లేకపోతే{
        Trigger *pTrigger = sParse.pNewTrigger;
        Trigger_అడుగు *p_అడుగు;
        ఒకవేళ( 0==sqlite3_stricmp(sParse.pNewTrigger->పట్టిక_2, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          rename_Token_కనుగొను(&sParse, &sCtx, sParse.pNewTrigger->పట్టిక_2);
        }

        ఒకవేళ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          ఒకవేళ( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            కొరకు(p_అడుగు=pTrigger->అడుగు_list; p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
              ఒకవేళ( p_అడుగు->pSrc ){
                పూర్ణాంకం i;
                కొరకు(i=0; i<p_అడుగు->pSrc->nSrc; i++){
                  Src_అంశం *p_అంశం = &p_అడుగు->pSrc->a[i];
                  ఒకవేళ( 0==sqlite3_stricmp(p_అంశం->zName, zOld) ){
                    rename_Token_కనుగొను(&sParse, &sCtx, p_అంశం->zName);
                  }
                }
              }
            }
          }
        }
      }
#అయితే_ముగింపు
    }

    ఒకవేళ( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_ఆదానం, zNew, bQuote);
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      ఒకవేళ( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_ఫలితం_విలువ(context, argv[3]);
      }లేకపోతే ఒకవేళ( sParse.z_Err_సందేశం ){
        rename_స్తంభం_Parse_Error(context, "", argv[1], argv[2], &sParse);
      }లేకపోతే{
        sqlite3_ఫలితం_error_సంకేతం(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#అయితే_ముగింపు
  }

  తిరిగి;
}

స్థిర పూర్ణాంకం rename_Quotefix_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_STRING && (p_సమాసం->flags & EP_DblQuoted) ){
    rename_Token_కనుగొను(pWalker->pParse, pWalker->u.pRename, (స్థిరరాశి శూన్య*)p_సమాసం);
  }
  తిరిగి WRC_Continue;
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
స్థిర శూన్య renameQuotefixFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  అక్షరం స్థిరరాశి *zDb = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  అక్షరం స్థిరరాశి *z_ఆదానం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#అయితే_ముగింపు

  sqlite3BtreeEnterAll(db);

  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( zDb && z_ఆదానం ){
    పూర్ణాంకం rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_ఆదానం, 0);

    ఒకవేళ( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, పరిమాణం(RenameCtx));
      memset(&sWalker, 0, పరిమాణం(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_సమాసం_Callback = rename_Quotefix_సమాసం_Cb;
      sWalker.xSelectCallback = rename_స్తంభం_Select_Cb;
      sWalker.u.pRename = &sCtx;

      ఒకవేళ( sParse.p_New_పట్టిక ){
        ఒకవేళ( IsView(sParse.p_New_పట్టిక) ){
          Select *pSelect = sParse.p_New_పట్టిక->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          ఒకవేళ( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }లేకపోతే{
          పూర్ణాంకం i;
          sqlite3_Walk_సమాసం_List(&sWalker, sParse.p_New_పట్టిక->pCheck);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
          కొరకు(i=0; i<sParse.p_New_పట్టిక->n_నిలువు; i++){
            sqlite3_Walk_సమాసం(&sWalker,
               sqlite3_స్తంభం_సమాసం(sParse.p_New_పట్టిక,
                                         &sParse.p_New_పట్టిక->a_నిలువు[i]));
          }
#అయితే_ముగింపు /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }లేకపోతే ఒకవేళ( sParse.p_New_సూచీ ){
        sqlite3_Walk_సమాసం_List(&sWalker, sParse.p_New_సూచీ->a_నిలువు_సమాసం);
        sqlite3_Walk_సమాసం(&sWalker, sParse.p_New_సూచీ->pPartIdxWhere);
      }లేకపోతే{
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        ఒకవేళ( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#అయితే_ముగింపు /* SQLITE_OMIT_TRIGGER */
      }

      ఒకవేళ( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_ఆదానం, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      ఒకవేళ( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_ఫలితం_విలువ(context, argv[1]);
      }లేకపోతే{
        sqlite3_ఫలితం_error_సంకేతం(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#అయితే_ముగింపు

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
స్థిర శూన్య rename_పట్టిక_Test(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  అక్షరం స్థిరరాశి *zDb = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  అక్షరం స్థిరరాశి *z_ఆదానం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
  పూర్ణాంకం b_తాత్కాలిక = sqlite3_విలువ_int(argv[4]);
  పూర్ణాంకం isLegacy = (db->flags & SQLITE_LegacyAlter);
  అక్షరం స్థిరరాశి *zWhen = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[5]);
  పూర్ణాంకం bNoDQS = sqlite3_విలువ_int(argv[6]);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#అయితే_ముగింపు

  వాడని_PARAMETER(Not_వాడిన);

  ఒకవేళ( zDb && z_ఆదానం ){
    పూర్ణాంకం rc;
    Parse sParse;
    u64 flags = db->flags;
    ఒకవేళ( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_ఆదానం, b_తాత్కాలిక);
    db->flags = flags;
    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( isLegacy==0 && sParse.p_New_పట్టిక && IsView(sParse.p_New_పట్టిక) ){
        NameContext sNC;
        memset(&sNC, 0, పరిమాణం(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.p_New_పట్టిక->u.view.pSelect, &sNC);
        ఒకవేళ( sParse.nErr ) rc = sParse.rc;
      }

      లేకపోతే ఒకవేళ( sParse.pNewTrigger ){
        ఒకవేళ( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        ఒకవేళ( rc==SQLITE_OK ){
          పూర్ణాంకం i1 = sqlite3_Schema_గమ్యం_సూచీ(db, sParse.pNewTrigger->pTabSchema);
          పూర్ణాంకం i2 = sqlite3_కనుగొను_Db_Name(db, zDb);
          ఒకవేళ( i1==i2 ){
            /* Handle output case B */
            sqlite3_ఫలితం_int(context, 1);
          }
        }
      }
    }

    ఒకవేళ( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      rename_స్తంభం_Parse_Error(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#అయితే_ముగింపు
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
స్థిర పూర్ణాంకం getConstraintToken(స్థిరరాశి u8 *z, పూర్ణాంకం *piToken){
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం t = 0;
  చేయి {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }అయితే( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  ఒకవేళ( t==TK_LP ){
    పూర్ణాంకం nNest = 1;
    అయితే( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      ఒకవేళ( t==TK_LP ){
        nNest++;
      }లేకపోతే ఒకవేళ( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }లేకపోతే ఒకవేళ( t==TK_ILLEGAL ){
        ఆపు;
      }
    }
  }

  *piToken = t;
  తిరిగి iOff;
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
స్థిర శూన్య drop_స్తంభం_Func(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  పూర్ణాంకం iSchema = sqlite3_విలువ_int(argv[0]);
  స్థిరరాశి అక్షరం *zSql = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
  పూర్ణాంకం i_నిలువు = sqlite3_విలువ_int(argv[2]);
  స్థిరరాశి అక్షరం *zDb = db->aDb[iSchema].zDbSName;
  పూర్ణాంకం rc;
  Parse sParse;
  RenameToken *p_నిలువు;
  పట్టిక *pTab;
  స్థిరరాశి అక్షరం *z_చివర;
  అక్షరం *zNew = 0;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#అయితే_ముగింపు

  వాడని_PARAMETER(Not_వాడిన);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు drop_స్తంభం_పూర్తి;
  pTab = sParse.p_New_పట్టిక;
  ఒకవేళ( pTab==0 || pTab->n_నిలువు==1 || i_నిలువు>=pTab->n_నిలువు ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    వెళ్ళు drop_స్తంభం_పూర్తి;
  }

  ఒకవేళ( i_నిలువు<pTab->n_నిలువు-1 ){
    RenameToken *p_చివర;
    p_నిలువు = rename_Token_కనుగొను(&sParse, 0, (శూన్య*)pTab->a_నిలువు[i_నిలువు].zCnName);
    p_చివర = rename_Token_కనుగొను(&sParse, 0, (శూన్య*)pTab->a_నిలువు[i_నిలువు+1].zCnName);
    z_చివర = (స్థిరరాశి అక్షరం*)p_చివర->t.z;
  }లేకపోతే{
    పూర్ణాంకం eTok;
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
    ప్రతిజ్ఞా( i_నిలువు!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    p_నిలువు = rename_Token_కనుగొను(&sParse, 0, (శూన్య*)pTab->a_నిలువు[i_నిలువు-1].zCnName);
    చేయి {
      p_నిలువు->t.z += getConstraintToken((స్థిరరాశి u8*)p_నిలువు->t.z, &eTok);
    }అయితే( eTok!=TK_COMMA );
    p_నిలువు->t.z--;
    z_చివర = (స్థిరరాశి అక్షరం*)&zSql[pTab->u.tab.కూడు_నిలువు_స్థానభ్రంశం];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", p_నిలువు->t.z-zSql, zSql, z_చివర);
  sqlite3_ఫలితం_పాఠ్యం(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_స్తంభం_పూర్తి:
  renameParseCleanup(&sParse);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#అయితే_ముగింపు
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_ఫలితం_error_సంకేతం(context, rc);
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
శూన్య sqlite3_Alter_Drop_స్తంభం(Parse *pParse, SrcList *pSrc, స్థిరరాశి Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  పట్టిక *pTab;                    /* Table to modify */
  పూర్ణాంకం iDb;                        /* Index of db containing pTab in aDb[] */
  స్థిరరాశి అక్షరం *zDb;                /* Database containing pTab ("main" etc.) */
  అక్షరం *z_నిలువు = 0;                 /* Name of column to drop */
  పూర్ణాంకం i_నిలువు;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  ప్రతిజ్ఞా( pParse->p_New_పట్టిక==0 );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  ఒకవేళ( NEVER(db->mallocFailed) ) వెళ్ళు exit_drop_స్తంభం;
  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, &pSrc->a[0]);
  ఒకవేళ( !pTab ) వెళ్ళు exit_drop_స్తంభం;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  ఒకవేళ( SQLITE_OK!=is_Alterable_పట్టిక(pParse, pTab) ) వెళ్ళు exit_drop_స్తంభం;
  ఒకవేళ( SQLITE_OK!=is_Real_పట్టిక(pParse, pTab, 1) ) వెళ్ళు exit_drop_స్తంభం;

  /* Find the index of the column being dropped. */
  z_నిలువు = sqlite3NameFromToken(db, pName);
  ఒకవేళ( z_నిలువు==0 ){
    ప్రతిజ్ఞా( db->mallocFailed );
    వెళ్ళు exit_drop_స్తంభం;
  }
  i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, z_నిలువు);
  ఒకవేళ( i_నిలువు<0 ){
    sqlite3_Error_సందేశం(pParse, "no such column: \"%T\"", pName);
    వెళ్ళు exit_drop_స్తంభం;
  }
  ఒకవేళ( isRowidAlias(pParse, pTab, z_నిలువు, "drop") ) వెళ్ళు exit_drop_స్తంభం;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  ఒకవేళ( pTab->a_నిలువు[i_నిలువు].నిలువు_Flags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_సందేశం(pParse, "cannot drop %s column: \"%s\"",
        (pTab->a_నిలువు[i_నిలువు].నిలువు_Flags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        z_నిలువు
    );
    వెళ్ళు exit_drop_స్తంభం;
  }

  /* Do not allow the number of columns to go to zero */
  ఒకవేళ( pTab->n_నిలువు<=1 ){
    sqlite3_Error_సందేశం(pParse, "cannot drop column \"%s\": no other columns exist",z_నిలువు);
    వెళ్ళు exit_drop_స్తంభం;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, zDb, pTab->zName, z_నిలువు) ){
    వెళ్ళు exit_drop_స్తంభం;
  }
#అయితే_ముగింపు
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, i_నిలువు, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  ఒకవేళ( pParse->nErr==0 && (pTab->a_నిలువు[i_నిలువు].నిలువు_Flags & COLFLAG_VIRTUAL)==0 ){
    పూర్ణాంకం i;
    పూర్ణాంకం addr;
    పూర్ణాంకం reg;
    పూర్ణాంకం regRec;
    సూచీ *pPk = 0;
    పూర్ణాంకం n_క్షేత్రం = 0;               /* Number of non-virtual columns after drop */
    పూర్ణాంకం iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3_Open_పట్టిక(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    ఒకవేళ( HasRowid(pTab) ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->n_నిలువు;
    }లేకపోతే{
      pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      pParse->nMem += pPk->n_స్తంభం;
      కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iCur, i, reg+i+1);
      }
      n_క్షేత్రం = pPk->n_కీలకం_నిలువు;
    }
    regRec = ++pParse->nMem;
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      ఒకవేళ( i!=i_నిలువు && (pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL)==0 ){
        పూర్ణాంకం reg_బహిర్గతం;
        ఒకవేళ( pPk ){
          పూర్ణాంకం iPos = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pPk, i);
          పూర్ణాంకం i_నిలువు_Pos = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pPk, i_నిలువు);
          ఒకవేళ( iPos<pPk->n_కీలకం_నిలువు ) కొనసాగు;
          reg_బహిర్గతం = reg+1+iPos-(iPos>i_నిలువు_Pos);
        }లేకపోతే{
          reg_బహిర్గతం = reg+1+n_క్షేత్రం;
        }
        ఒకవేళ( i==pTab->iPKey ){
          sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_బహిర్గతం);
        }లేకపోతే{
          sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, i, reg_బహిర్గతం);
          ప్రతిజ్ఞా( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->a_నిలువు[i].affinity==SQLITE_AFF_REAL );
          sqlite3_Vdbe_Delete_పూర్వ_Opcode(v, OP_RealAffinity);
        }
        n_క్షేత్రం++;
      }
    }
    ఒకవేళ( n_క్షేత్రం==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg+1);
      n_క్షేత్రం = 1;
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg+1, n_క్షేత్రం, regRec);
    ఒకవేళ( pPk ){
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iCur, regRec, reg+1, pPk->n_కీలకం_నిలువు);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iCur, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
  }

exit_drop_స్తంభం:
  sqlite3DbFree(db, z_నిలువు);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
స్థిర పూర్ణాంకం getWhitespace(స్థిరరాశి u8 *z){
  పూర్ణాంకం nRet = 0;
  అయితే( 1 ){
    పూర్ణాంకం t = 0;
    పూర్ణాంకం n = sqlite3GetToken(&z[nRet], &t);
    ఒకవేళ( t!=TK_SPACE && t!=TK_COMMENT ) ఆపు;
    nRet += n;
  }
  తిరిగి nRet;
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
స్థిర పూర్ణాంకం getConstraint(స్థిరరాశి u8 *z, పూర్ణాంకం eTok){
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం t = 0;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
  ఒకవేళ( eTok==TK_FOREIGN ){
    /* For a FOREIGN KEY constraint, use getConstraint() to parse everything
    ** up to the REFERENCES keyword. Then getConstraintToken() to consume
    ** the TK_REFERENCES token itself. Then fall through to the special
    ** handling for TK_REFERENCES below.  */
    iOff = getConstraint(z, 0);
    iOff += getConstraintToken(&z[iOff], &eTok);
  }

  ఒకవేళ( eTok==TK_REFERENCES ){
    /* REFERENCES is followed by a table name. Gobble this up here in
    ** case the table name is a fallback token like TK_GENERATED. */
    iOff += getConstraintToken(&z[iOff], &t);
  }
#అయితే_ముగింపు

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  అయితే( 1 ){
    పూర్ణాంకం n = getConstraintToken(&z[iOff], &t);
    ఒకవేళ( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      ఆపు;
    }
    iOff += n;
  }
  
  తిరిగి iOff;
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
స్థిర పూర్ణాంకం quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  పూర్ణాంకం t,                 /* Token type */
  స్థిరరాశి u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  పూర్ణాంకం nQuote,            /* Length of zQuote in bytes */
  స్థిరరాశి u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  పూర్ణాంకం *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  అక్షరం *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  ఒకవేళ( t==TK_ILLEGAL ){
    *pRes = 1;
    తిరిగి SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  ఒకవేళ( zCopy==0 ){
    sqlite3_ఫలితం_error_nomem(ctx);
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  జ్ఞాపకం_నకలు(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((స్థిరరాశి అక్షరం*)zCopy, (స్థిరరాశి అక్షరం*)zCmp);
  sqlite3_free(zCopy);
  తిరిగి SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
స్థిర పూర్ణాంకం skip_Create_పట్టిక(sqlite3_context *ctx, స్థిరరాశి u8 *zSql, పూర్ణాంకం *piOff){
  పూర్ణాంకం iOff = 0;

  ఒకవేళ( zSql==0 ) తిరిగి SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  అయితే( 1 ){
    పూర్ణాంకం t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    ఒకవేళ( t==TK_LP ) ఆపు;
    ఒకవేళ( t==TK_ILLEGAL ){
      sqlite3_ఫలితం_error_సంకేతం(ctx, SQLITE_CORRUPT_BKPT);
      తిరిగి SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  తిరిగి SQLITE_OK;
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
స్థిర శూన్య dropConstraintFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిరరాశి u8 *zSql = sqlite3_విలువ_పాఠ్యం(argv[0]);
  స్థిరరాశి u8 *zCons = 0;
  పూర్ణాంకం iNotNull = -1;
  పూర్ణాంకం ii;
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం i_ప్రారంభం = 0;
  పూర్ణాంకం i_చివర = 0;
  అక్షరం *zNew = 0;
  పూర్ణాంకం t = 0;
  sqlite3 *db;
  వాడని_PARAMETER(Not_వాడిన);

  ఒకవేళ( zSql==0 ) తిరిగి;

  /* Jump past the "CREATE TABLE" bit. */
  ఒకవేళ( skip_Create_పట్టిక(ctx, zSql, &iOff) ) తిరిగి;

  ఒకవేళ( sqlite3_విలువ_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_విలువ_int(argv[1]);
  }లేకపోతే{
    zCons = sqlite3_విలువ_పాఠ్యం(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  కొరకు(ii=0; i_చివర==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    అయితే( 1 ){
      i_ప్రారంభం = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      ఒకవేళ( t==TK_CONSTRAINT && (zCons || iNotNull==ii) ){
        /* Check if this is the constraint we are searching for. */
        పూర్ణాంకం nTok = 0;
        పూర్ణాంకం cmp = 1;

        /* Skip past any whitespace. */
        iOff += getWhitespace(&zSql[iOff]);

        /* Compare the next token - which may be quoted - with the name of
        ** the constraint being dropped.  */
        nTok = getConstraintToken(&zSql[iOff], &t);
        ఒకవేళ( zCons ){
          ఒకవేళ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) తిరిగి;
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
        ఒకవేళ( t==TK_CONSTRAINT || t==TK_DEFAULT || t==TK_COLLATE 
         || t==TK_COMMA || t==TK_RP || t==TK_GENERATED || t==TK_AS 
        ){
          t = TK_CHECK;
        }లేకపోతే{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        ఒకవేళ( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          ఒకవేళ( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            తిరిగి;
          }
          i_చివర = iOff;
          ఆపు;
        }

      }లేకపోతే ఒకవేళ( t==TK_NOT && iNotNull==ii ){
        i_చివర = iOff + getConstraint(&zSql[iOff], 0);
        ఆపు;
      }లేకపోతే ఒకవేళ( t==TK_RP || t==TK_ILLEGAL ){
        i_చివర = -1;
        ఆపు;
      }లేకపోతే ఒకవేళ( t==TK_COMMA ){
        ఆపు;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  ఒకవేళ( i_చివర<=0 ){
    ఒకవేళ( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }లేకపోతే{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_ఫలితం_పాఠ్యం(ctx, (స్థిరరాశి అక్షరం*)zSql, -1, SQLITE_TRANSIENT);
    }
  }లేకపోతే{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    స్థిరరాశి అక్షరం *zSpace = " ";
    i_చివర += getWhitespace(&zSql[i_చివర]);
    sqlite3GetToken(&zSql[i_చివర], &t);
    ఒకవేళ( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      ఒకవేళ( zSql[i_ప్రారంభం-1]==',' ) i_ప్రారంభం--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", i_ప్రారంభం, zSql, zSpace, &zSql[i_చివర]);
    sqlite3_ఫలితం_పాఠ్యం(ctx, zNew, -1, SQLITE_DYNAMIC);
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
స్థిర శూన్య కూడు_Constraint_Func(
  sqlite3_context *ctx,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిరరాశి u8 *zSql = sqlite3_విలువ_పాఠ్యం(argv[0]);
  స్థిరరాశి అక్షరం *zCons = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
  పూర్ణాంకం i_నిలువు = sqlite3_విలువ_int(argv[2]);
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం ii;
  sqlite3_str *pNew;
  పూర్ణాంకం t = 0;
  వాడని_PARAMETER(Not_వాడిన);

  ఒకవేళ( skip_Create_పట్టిక(ctx, zSql, &iOff) ) తిరిగి;
  
  కొరకు(ii=0; ii<=i_నిలువు || (i_నిలువు<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    అయితే( 1 ){
      పూర్ణాంకం nTok = getConstraintToken(&zSql[iOff], &t);
      ఒకవేళ( t==TK_COMMA || t==TK_RP ) ఆపు;
      ఒకవేళ( t==TK_ILLEGAL ){
        sqlite3_ఫలితం_error_సంకేతం(ctx, SQLITE_CORRUPT_BKPT);
        తిరిగి;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (స్థిరరాశి అక్షరం*)zSql, iOff);
  ఒకవేళ( i_నిలువు<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_ఫలితం_str(ctx, pNew, SQLITE_ముగింపు);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
స్థిర పూర్ణాంకం alter_కనుగొను_నిలువు(Parse *pParse, పట్టిక *pTab, Token *p_నిలువు, పూర్ణాంకం *pi_నిలువు){
  sqlite3 *db = pParse->db;
  అక్షరం *zName = sqlite3NameFromToken(db, p_నిలువు);
  పూర్ణాంకం rc = SQLITE_NOMEM;
  పూర్ణాంకం i_నిలువు = -1;

  ఒకవేళ( zName ){
    i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, zName);
    ఒకవేళ( i_నిలువు<0 ){
      sqlite3_Error_సందేశం(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }లేకపోతే{
      rc = SQLITE_OK;
    }
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( rc==SQLITE_OK ){
    స్థిరరాశి అక్షరం *zDb = db->aDb[sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema)].zDbSName;
    స్థిరరాశి అక్షరం *z_నిలువు = pTab->a_నిలువు[i_నిలువు].zCnName;
    ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, zDb, pTab->zName, z_నిలువు) ){
      pTab = 0;
    }
  }
#అయితే_ముగింపు

  sqlite3DbFree(db, zName);
  *pi_నిలువు = i_నిలువు;
  తిరిగి rc;
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
స్థిర పట్టిక *alter_కనుగొను_పట్టిక(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  పూర్ణాంకం *piDb,            /* OUT: write the iDb here */
  స్థిరరాశి అక్షరం **pzDb,    /* OUT: write name of schema here */
  పూర్ణాంకం bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  పట్టిక *pTab = 0;
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, &pSrc->a[0]);
  ఒకవేళ( pTab ){
    పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    ఒకవేళ( SQLITE_OK!=is_Real_పట్టిక(pParse, pTab, 2) 
     || SQLITE_OK!=is_Alterable_పట్టిక(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( pTab && bAuth ){
    ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ALTER_పట్టిక, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#అయితే_ముగింపు
  sqlite3SrcListDelete(db, pSrc);
  తిరిగి pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
శూన్య sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *p_నిలువు       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  పట్టిక *pTab = 0;
  పూర్ణాంకం iDb = 0;
  స్థిరరాశి అక్షరం *zDb = 0;
  అక్షరం *zArg = 0;

  ప్రతిజ్ఞా( (p_నిలువు==0)!=(pCons==0) );
  ప్రతిజ్ఞా( pSrc->nSrc==1 );
  pTab = alter_కనుగొను_పట్టిక(pParse, pSrc, &iDb, &zDb, pCons!=0);
  ఒకవేళ( !pTab ) తిరిగి;

  ఒకవేళ( pCons ){
    అక్షరం *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }లేకపోతే{
    పూర్ణాంకం i_నిలువు;
    ఒకవేళ( alter_కనుగొను_నిలువు(pParse, pTab, p_నిలువు, &i_నిలువు) ) తిరిగి;
    zArg = sqlite3MPrintf(db, "%d", i_నిలువు);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
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
స్థిర శూన్య failConstraintFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిరరాశి అక్షరం *z_పాఠ్యం = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  పూర్ణాంకం err = sqlite3_విలువ_int(argv[1]);
  (శూన్య)Not_వాడిన;
  sqlite3_ఫలితం_error(ctx, z_పాఠ్యం, -1);
  sqlite3_ఫలితం_error_సంకేతం(ctx, err);
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
స్థిర పూర్ణాంకం alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  స్థిరరాశి అక్షరం *pCons,              /* Buffer containing constraint */
  పూర్ణాంకం nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం i_చివర = 0;

  ఒకవేళ( zTmp==0 ) తిరిగి 0;

  అయితే( 1 ){
    పూర్ణాంకం t = 0;
    పూర్ణాంకం nToken = sqlite3GetToken(&zTmp[iOff], &t);
    ఒకవేళ( t==TK_ILLEGAL ) ఆపు;
    ఒకవేళ( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      i_చివర = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  తిరిగి i_చివర;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
శూన్య sqlite3_Alter_సమితి_Not_Null(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *p_నిలువు,     /* Name of the column to add a NOT NULL constraint to */
  Token *p_మొదటి    /* The NOT token of the NOT NULL constraint text */
){
  పట్టిక *pTab = 0;
  పూర్ణాంకం i_నిలువు = 0;
  పూర్ణాంకం iDb = 0;
  స్థిరరాశి అక్షరం *zDb = 0;
  స్థిరరాశి అక్షరం *pCons = 0;
  పూర్ణాంకం nCons = 0;

  /* Look up the table being altered. */
  ప్రతిజ్ఞా( pSrc->nSrc==1 );
  pTab = alter_కనుగొను_పట్టిక(pParse, pSrc, &iDb, &zDb, 0);
  ఒకవేళ( !pTab ) తిరిగి;

  /* Find the column being altered. */
  ఒకవేళ( alter_కనుగొను_నిలువు(pParse, pTab, p_నిలువు, &i_నిలువు) ){
    తిరిగి;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = p_మొదటి->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_చివరి_Token.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (పూర్ణాంకం)p_నిలువు->n, p_నిలువు->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, i_నిలువు, nCons, pCons, i_నిలువు, pTab->zName
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
స్థిర శూన్య కనుగొను_Constraint_Func(
  sqlite3_context *ctx,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిరరాశి u8 *zSql = 0;
  స్థిరరాశి u8 *zCons = 0;
  పూర్ణాంకం iOff = 0;
  పూర్ణాంకం t = 0;

  (శూన్య)Not_వాడిన;
  zSql = sqlite3_విలువ_పాఠ్యం(argv[0]);
  zCons = sqlite3_విలువ_పాఠ్యం(argv[1]);

  ఒకవేళ( zSql==0 || zCons==0 ) తిరిగి;
  అయితే( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  అయితే( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    ఒకవేళ( t==TK_CONSTRAINT ){
      పూర్ణాంకం nTok = 0;
      పూర్ణాంకం cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      ఒకవేళ( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) తిరిగి;
      ఒకవేళ( cmp==0 ){
        sqlite3_ఫలితం_int(ctx, 1);
        తిరిగి;
      }
    }లేకపోతే ఒకవేళ( t==TK_ILLEGAL ){
      ఆపు;
    }
  }

  sqlite3_ఫలితం_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
శూన్య sqlite3_Alter_కూడు_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_మొదటి,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  స్థిరరాశి అక్షరం *z_సమాసం,       /* Text of CHECK expression */
  పూర్ణాంకం n_సమాసం,               /* Size of pExpr in bytes */
  సమాసం *p_సమాసం              /* The parsed CHECK expression */
){ 
  పట్టిక *pTab = 0;         /* Table identified by pSrc */
  పూర్ణాంకం iDb = 0;             /* Which schema does pTab live in */
  స్థిరరాశి అక్షరం *zDb = 0;     /* Name of the schema in which pTab lives */
  స్థిరరాశి అక్షరం *pCons = 0;   /* Text of the constraint */
  పూర్ణాంకం nCons;               /* Bytes of text to use from pCons[] */
  పూర్ణాంకం rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  ప్రతిజ్ఞా( pSrc->nSrc==1 );
  pTab = alter_కనుగొను_పట్టిక(pParse, pSrc, &iDb, &zDb, 1);
  ఒకవేళ( !pTab ){
    sqlite3_సమాసం_Delete(pParse->db, p_సమాసం);
    తిరిగి;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_సమాసం, 0);
  sqlite3_సమాసం_Delete(pParse->db, p_సమాసం);
  ఒకవేళ( rc ) తిరిగి;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  ఒకవేళ( pName ){
    అక్షరం *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_పట్టిక " "
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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_సమాసం, z_సమాసం
  );

  /* Edit the SQL for the named table. */
  pCons = p_మొదటి->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_చివరి_Token.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_పట్టిక " SET "
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
శూన్య sqlite3AlterFunctions(శూన్య){
  స్థిర FuncDef a_Alter_పట్టిక_Funcs[] = {
    అంతర్గత_FUNCTION(sqlite_rename_స్తంభం,  9, rename_స్తంభం_Func),
    అంతర్గత_FUNCTION(sqlite_rename_పట్టిక,   7, rename_పట్టిక_Func),
    అంతర్గత_FUNCTION(sqlite_rename_test,    7, rename_పట్టిక_Test),
    అంతర్గత_FUNCTION(sqlite_drop_స్తంభం,    3, drop_స్తంభం_Func),
    అంతర్గత_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    అంతర్గత_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    అంతర్గత_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    అంతర్గత_FUNCTION(sqlite_కూడు_constraint, 3, కూడు_Constraint_Func),
    అంతర్గత_FUNCTION(sqlite_కనుగొను_constraint,2, కనుగొను_Constraint_Func),
  };
  sqlite3_చొప్పించు_Builtin_Funcs(a_Alter_పట్టిక_Funcs, Array_కొలత(a_Alter_పట్టిక_Funcs));
}
#అయితే_ముగింపు  /* SQLITE_ALTER_TABLE */
