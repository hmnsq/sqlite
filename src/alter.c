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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_ALTERTABLE

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
ساکن صحیح is_Alterable_جدول(Parse *pParse, جدول *pTab){
  اگر( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7)
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
   || (pTab->tabFlags & TF_Eponymous)!=0
   || ( (pTab->tabFlags & TF_Shadow)!=0
        && sqlite3ReadOnlyShadowTables(pParse->db)
   )
#اگر_ختم
  ){
    sqlite3_Error_پیغام(pParse, "table %s may not be altered", pTab->zName);
    واپس 1;
  }
  واپس 0;
}

/*
** Generate code to verify that the schemas of database zDb and, if
** bTemp is not true, database "temp", can still be parsed. This is
** called at the end of the generation of an ALTER TABLE ... RENAME ...
** statement to ensure that the operation has not rendered any schema
** objects unusable.
*/
ساکن خالی renameTestSchema(
  Parse *pParse,                  /* Parse context */
  ثابت حرف *zDb,                /* Name of db to verify schema of */
  صحیح b_عارضی,                      /* True if this is the temp db */
  ثابت حرف *zWhen,              /* "when" part of error message */
  صحیح bNoDQS                      /* Do not allow DQS in the schema */
){
  pParse->ستون_نام_سیٹ = 1;
  sqlite3NestedParse(pParse,
      "SELECT 1 "
      "FROM \"%w\"." LEGACY_SCHEMA_جدول " "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'"
      " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
      zDb,
      zDb, b_عارضی, zWhen, bNoDQS
  );

  اگر( b_عارضی==0 ){
    sqlite3NestedParse(pParse,
        "SELECT 1 "
        "FROM temp." LEGACY_SCHEMA_جدول " "
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
ساکن خالی renameFixQuotes(Parse *pParse, ثابت حرف *zDb, صحیح b_عارضی){
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول
      " SET sql = sqlite_rename_quotefix(%Q, sql)"
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      " AND sql NOT LIKE 'create virtual%%'" , zDb, zDb
  );
  اگر( b_عارضی==0 ){
    sqlite3NestedParse(pParse,
      "UPDATE temp." LEGACY_SCHEMA_جدول
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
ساکن خالی renameReloadSchema(Parse *pParse, صحیح iDb, u16 p5){
  Vdbe *v = pParse->pVdbe;
  اگر( v ){
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_جوڑو_Parse_Schema_Op(pParse->pVdbe, iDb, 0, p5);
    اگر( iDb!=1 ) sqlite3_Vdbe_جوڑو_Parse_Schema_Op(pParse->pVdbe, 1, 0, p5);
  }
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy"
** command.
*/
خالی sqlite3_Alter_Rename_جدول(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  صحیح iDb;                  /* Database that contains the table */
  حرف *zDb;                /* Name of database iDb */
  جدول *pTab;              /* Table being renamed */
  حرف *zName = 0;          /* NULL-terminated version of pName */
  sqlite3 *db = pParse->db; /* Database connection */
  صحیح nTabName;             /* Number of UTF-8 characters in zTabName */
  ثابت حرف *zTabName;     /* Original name of the table */
  Vdbe *v;
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */

  اگر( NEVER(db->mallocFailed) ) جاؤ exit_rename_جدول;
  أكد( pSrc->nSrc==1 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) جاؤ exit_rename_جدول;
  iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  اگر( !zName ) جاؤ exit_rename_جدول;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  اگر( sqlite3_ڈھونڈو_جدول(db, zName, zDb)
   || sqlite3_ڈھونڈو_اشاریہ(db, zName, zDb)
   || sqlite3_Is_Shadow_جدول_کا(db, pTab, zName)
  ){
    sqlite3_Error_پیغام(pParse,
        "there is already another table or index with this name: %s", zName);
    جاؤ exit_rename_جدول;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  اگر( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ){
    جاؤ exit_rename_جدول;
  }
  اگر( SQLITE_OK!=sqlite3CheckObjectName(pParse,zName,"table",zName) ){
    جاؤ exit_rename_جدول;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIEW
  اگر( IsView(pTab) ){
    sqlite3_Error_پیغام(pParse, "view %s may not be altered", pTab->zName);
    جاؤ exit_rename_جدول;
  }
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    جاؤ exit_rename_جدول;
  }
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  اگر( sqlite3_View_Get_عمود_نام(pParse, pTab) ){
    جاؤ exit_rename_جدول;
  }
  اگر( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    اگر( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#اگر_ختم

  /* Begin a transaction for database iDb. Then modify the schema cookie
  ** (since the ALTER TABLE modifies the schema). Call sqlite3MayAbort(),
  ** as the scalar functions (e.g. sqlite_rename_table()) invoked by the
  ** nested SQL may raise an exception.  */
  v = sqlite3GetVdbe(pParse);
  اگر( v==0 ){
    جاؤ exit_rename_جدول;
  }
  sqlite3MayAbort(pParse);

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  /* Rewrite all CREATE TABLE, INDEX, TRIGGER or VIEW statements in
  ** the schema to use the new table name.  */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
      "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
      "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
      , zDb, zDb, zTabName, zName, (iDb==1), zTabName
  );

  /* Update the tbl_name and name columns of the sqlite_schema table
  ** as required.  */
  sqlite3NestedParse(pParse,
      "UPDATE %Q." LEGACY_SCHEMA_جدول " SET "
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

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update
  ** it with the new table name.
  */
  اگر( sqlite3_ڈھونڈو_جدول(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#اگر_ختم

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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  اگر( pVTab ){
    صحیح i = ++pParse->nMem;
    sqlite3_Vdbe_بھرو_String(v, i, zName);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_VRename, i, 0, 0,(ثابت حرف*)pVTab, P4_VTAB);
  }
#اگر_ختم

  renameReloadSchema(pParse, iDb, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iDb==1, "after rename", 0);

exit_rename_جدول:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

/*
** Write code that will raise an error if the table described by
** zDb and zTab is not empty.
*/
ساکن خالی sqlite3_Error_If_Not_خالی_خانے(
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
ساکن صحیح isRowidAlias(
  Parse *pParse, 
  جدول *pTab, 
  ثابت حرف *z_ستون, 
  ثابت حرف *zOp
){
  اگر( HasRowid(pTab) && sqlite3IsRowid(z_ستون) ){
    sqlite3_Error_پیغام(pParse, "cannot %s rowid alias: %s", zOp, z_ستون);
    واپس SQLITE_ERROR;
  }
  واپس SQLITE_OK;
}



/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
خالی sqlite3_Alter_اختتام_جوڑو_عمود(Parse *pParse, Token *p_ستون_Def){
  جدول *pNew;              /* Copy of pParse->pNewTable */
  جدول *pTab;              /* Table being altered */
  صحیح iDb;                  /* Database number */
  ثابت حرف *zDb;          /* Database name */
  ثابت حرف *zTab;         /* Table name */
  حرف *z_ستون;               /* Null-terminated column definition */
  عمود *p_ستون;             /* The new column */
  اظہاریہ *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */
  Vdbe *v;                  /* The prepared statement under construction */
  صحیح r1;                   /* Temporary registers */

  db = pParse->db;
  أكد( db->pParse==pParse );
  اگر( pParse->nErr ) واپس;
  أكد( db->mallocFailed==0 );
  pNew = pParse->p_New_جدول;
  أكد( pNew );

  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_مقصد_اشاریہ(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  p_ستون = &pNew->a_ستون[pNew->n_ستون-1];
  pDflt = sqlite3_عمود_اظہاریہ(pNew, p_ستون);
  pTab = sqlite3_ڈھونڈو_جدول(db, zTab, zDb);
  أكد( pTab );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    واپس;
  }
#اگر_ختم


  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE,
  ** or a rowid alias. If there is a NOT NULL constraint, then the default
  ** value for the column must not be NULL.
  */
  اگر( p_ستون->ستون_Flags & COLFLAG_PRIMKEY ){
    sqlite3_Error_پیغام(pParse, "Cannot add a PRIMARY KEY column");
    واپس;
  }
  اگر( pNew->p_اشاریہ ){
    sqlite3_Error_پیغام(pParse,
         "Cannot add a UNIQUE column");
    واپس;
  }
  اگر( isRowidAlias(pParse, pTab, p_ستون->zCnName, "add") ) واپس;
  اگر( (p_ستون->ستون_Flags & COLFLAG_GENERATED)==0 ){
    /* If the default value for the new column was specified with a
    ** literal NULL, then set pDflt to 0. This simplifies checking
    ** for an SQL NULL default below.
    */
    أكد( pDflt==0 || pDflt->op==TK_وسعت );
    اگر( pDflt && pDflt->p_بایاں->op==TK_NULL ){
      pDflt = 0;
    }
    أكد( Is_Ordinary_جدول(pNew) );
    اگر( (db->flags&SQLITE_Foreign_کلیدیں) && pNew->u.tab.pFKey && pDflt ){
      sqlite3_Error_If_Not_خالی_خانے(pParse, zDb, zTab,
          "Cannot add a REFERENCES column with non-NULL default value");
    }
    اگر( p_ستون->notNull && !pDflt ){
      sqlite3_Error_If_Not_خالی_خانے(pParse, zDb, zTab,
          "Cannot add a NOT NULL column with default value NULL");
    }


    /* Ensure the default expression is something that sqlite3ValueFromExpr()
    ** can handle (i.e. not CURRENT_TIME etc.)
    */
    اگر( pDflt ){
      sqlite3_قدر *pVal = 0;
      صحیح rc;
      rc = sqlite3_قدر_From_اظہاریہ(db, pDflt, SQLITE_UTF8, SQLITE_AFF_BLOB, &pVal);
      أكد( rc==SQLITE_OK || rc==SQLITE_NOMEM );
      اگر( rc!=SQLITE_OK ){
        أكد( db->mallocFailed == 1 );
        واپس;
      }
      اگر( !pVal ){
        sqlite3_Error_If_Not_خالی_خانے(pParse, zDb, zTab,
           "Cannot add a column with non-constant default");
      }
      sqlite3_قدر_Free(pVal);
    }
  }ورنہ اگر( p_ستون->ستون_Flags & COLFLAG_STORED ){
    sqlite3_Error_If_Not_خالی_خانے(pParse, zDb, zTab, "cannot add a STORED column");
  }


  /* Modify the CREATE TABLE statement. */
  z_ستون = sqlite3DbStrNDup(db, (حرف*)p_ستون_Def->z, p_ستون_Def->n);
  اگر( z_ستون ){
    حرف *z_انتہا = &z_ستون[p_ستون_Def->n-1];
    جب( z_انتہا>z_ستون && (*z_انتہا==';' || sqlite3Isspace(*z_انتہا)) ){
      *z_انتہا-- = '\0';
    }
    /* substr() operations on characters, but addColOffset is in bytes. So we
    ** have to use printf() to translate between these units: */
    أكد( Is_Ordinary_جدول(pTab) );
    أكد( Is_Ordinary_جدول(pNew) );
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
          "sql = printf('%%.%ds, ',sql) || %Q"
          " || substr(sql,1+length(printf('%%.%ds',sql))) "
        "WHERE type = 'table' AND name = %Q",
      zDb, pNew->u.tab.جوڑو_ستون_انحراف, z_ستون, pNew->u.tab.جوڑو_ستون_انحراف,
      zTab
    );
    sqlite3DbFree(db, z_ستون);
  }

  v = sqlite3GetVdbe(pParse);
  اگر( v ){
    /* Make sure the schema version is at least 3.  But do not upgrade
    ** from less than 3 to 4, as that will corrupt any preexisting DESC
    ** index.
    */
    r1 = sqlite3_Get_عارضی_Reg(pParse);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3_Vdbe_استعمال_کرتا_Btree(v, iDb);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, r1, -2);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_IfPos, r1, sqlite3VdbeCurrentAddr(v)+2);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_سیٹ_Cookie, iDb, BTREE_FILE_FORMAT, 3);
    sqlite3_Release_عارضی_Reg(pParse, r1);

    /* Reload the table definition */
    renameReloadSchema(pParse, iDb, INITFLAG_Alter_جوڑو);

    /* Verify that constraints are still satisfied */
    اگر( pNew->pCheck!=0
     || (p_ستون->notNull && (p_ستون->ستون_Flags & COLFLAG_GENERATED)!=0)
     || (pTab->tabFlags & TF_Strict)!=0
    ){
      pParse->ستون_نام_سیٹ = 1;
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
خالی sqlite3_Alter_Begin_جوڑو_عمود(Parse *pParse, SrcList *pSrc){
  جدول *pNew;
  جدول *pTab;
  صحیح iDb;
  صحیح i;
  صحیح nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  أكد( pParse->p_New_جدول==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  اگر( NEVER(db->mallocFailed) ) جاؤ exit_begin_جوڑو_عمود;
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) جاؤ exit_begin_جوڑو_عمود;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  اگر( IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "virtual tables may not be altered");
    جاؤ exit_begin_جوڑو_عمود;
  }
#اگر_ختم

  /* Make sure this is not an attempt to ALTER a view. */
  اگر( IsView(pTab) ){
    sqlite3_Error_پیغام(pParse, "Cannot add a column to a view");
    جاؤ exit_begin_جوڑو_عمود;
  }
  اگر( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ){
    جاؤ exit_begin_جوڑو_عمود;
  }

  sqlite3MayAbort(pParse);
  أكد( Is_Ordinary_جدول(pTab) );
  أكد( pTab->u.tab.جوڑو_ستون_انحراف>0 );
  iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (جدول*)sqlite3DbMallocZero(db, حجم(جدول));
  اگر( !pNew ) جاؤ exit_begin_جوڑو_عمود;
  pParse->p_New_جدول = pNew;
  pNew->nTabRef = 1;
  pNew->n_ستون = pTab->n_ستون;
  أكد( pNew->n_ستون>0 );
  nAlloc = (((pNew->n_ستون-1)/8)*8)+8;
  أكد( nAlloc>=pNew->n_ستون && nAlloc%8==0 && nAlloc-pNew->n_ستون<8 );
  pNew->a_ستون = (عمود*)sqlite3DbMallocZero(db, حجم(عمود)*(u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  اگر( !pNew->a_ستون || !pNew->zName ){
    أكد( db->mallocFailed );
    جاؤ exit_begin_جوڑو_عمود;
  }
  حافظہ_نقل(pNew->a_ستون, pTab->a_ستون, حجم(عمود)*(size_t)pNew->n_ستون);
  برائے(i=0; i<pNew->n_ستون; i++){
    عمود *p_ستون = &pNew->a_ستون[i];
    p_ستون->zCnName = sqlite3DbStrDup(db, p_ستون->zCnName);
    p_ستون->hName = sqlite3StrIHash(p_ستون->zCnName);
  }
  أكد( Is_Ordinary_جدول(pNew) );
  pNew->u.tab.pDfltList = sqlite3_اظہاریہ_List_Dup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.جوڑو_ستون_انحراف = pTab->u.tab.جوڑو_ستون_انحراف;
  أكد( pNew->nTabRef==1 );

exit_begin_جوڑو_عمود:
  sqlite3SrcListDelete(db, pSrc);
  واپس;
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
ساکن صحیح is_Real_جدول(Parse *pParse, جدول *pTab, صحیح iOp){
  ثابت حرف *zType = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIEW
  اگر( IsView(pTab) ){
    zType = "view";
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  اگر( IsVirtual(pTab) ){
    zType = "virtual table";
  }
#اگر_ختم
  اگر( zType ){
    ثابت حرف *az_پیغام[] = {
      "rename columns of", "drop column from", "edit constraints of"
    };
    أكد( iOp>=0 && iOp<Array_سائز(az_پیغام) );
    sqlite3_Error_پیغام(pParse, "cannot %s %s \"%s\"",
        az_پیغام[iOp], zType, pTab->zName
    );
    واپس 1;
  }
  واپس 0;
}
#ورنہ /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */
# define is_Real_جدول(x,y,z) (0)
#اگر_ختم

/*
** Handles the following parser reduction:
**
**  cmd ::= ALTER TABLE pSrc RENAME COLUMN pOld TO pNew
*/
خالی sqlite3_Alter_Rename_عمود(
  Parse *pParse,                  /* Parsing context */
  SrcList *pSrc,                  /* Table being altered.  pSrc->nSrc==1 */
  Token *pOld,                    /* Name of column being changed */
  Token *pNew                     /* New column name */
){
  sqlite3 *db = pParse->db;       /* Database connection */
  جدول *pTab;                    /* Table being updated */
  صحیح i_ستون;                       /* Index of column being renamed */
  حرف *zOld = 0;                 /* Old column name */
  حرف *zNew = 0;                 /* New column name */
  ثابت حرف *zDb;                /* Name of schema containing the table */
  صحیح iSchema;                    /* Index of the schema */
  صحیح bQuote;                     /* True to quote the new name */

  /* Locate the table to be altered */
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) جاؤ exit_rename_عمود;

  /* Cannot alter a system table */
  اگر( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ) جاؤ exit_rename_عمود;
  اگر( SQLITE_OK!=is_Real_جدول(pParse, pTab, 0) ) جاؤ exit_rename_عمود;

  /* Which schema holds the table to be altered */ 
  iSchema = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
  أكد( iSchema>=0 );
  zDb = db->aDb[iSchema].zDbSName;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, 0) ){
    جاؤ exit_rename_عمود;
  }
#اگر_ختم

  /* Make sure the old name really is a column name in the table to be
  ** altered.  Set iCol to be the index of the column being renamed */
  zOld = sqlite3NameFromToken(db, pOld);
  اگر( !zOld ) جاؤ exit_rename_عمود;
  i_ستون = sqlite3_عمود_اشاریہ(pTab, zOld);
  اگر( i_ستون<0 ){
    sqlite3_Error_پیغام(pParse, "no such column: \"%T\"", pOld);
    جاؤ exit_rename_عمود;
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
  اگر( !zNew ) جاؤ exit_rename_عمود;
  اگر( isRowidAlias(pParse, pTab, zOld, "rename from") ) جاؤ exit_rename_عمود;
  اگر( isRowidAlias(pParse, pTab, zNew, "rename to") ) جاؤ exit_rename_عمود;
  أكد( pNew->n>0 );
  bQuote = sqlite3Isquote(pNew->z[0]);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
      "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
      " AND (type != 'index' OR tbl_name = %Q)",
      zDb,
      zDb, pTab->zName, i_ستون, zNew, bQuote, iSchema==1,
      pTab->zName
  );
  اگر( iSchema!=1 ){
    sqlite3NestedParse(pParse,
        "UPDATE temp." LEGACY_SCHEMA_جدول " SET "
        "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
        "WHERE type IN ('trigger', 'view')",
        zDb, pTab->zName, i_ستون, zNew, bQuote
    );
  }

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iSchema, INITFLAG_AlterRename);
  renameTestSchema(pParse, zDb, iSchema==1, "after rename", 1);

 exit_rename_عمود:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  واپس;
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
ساخت RenameToken {
  ثابت خالی *p;         /* Parse tree element created by token t */
  Token t;               /* The token that created parse tree element p */
  RenameToken *p_اگلا;    /* Next is a list of all RenameToken objects */
};

/*
** The context of an ALTER TABLE RENAME COLUMN operation that gets passed
** down into the Walker.
*/
قسمتعریف ساخت RenameCtx RenameCtx;
ساخت RenameCtx {
  RenameToken *pList;             /* List of tokens to overwrite */
  صحیح nList;                      /* Number of tokens in pList */
  صحیح i_ستون;                       /* Index of column being renamed */
  جدول *pTab;                    /* Table being ALTERed */
  ثابت حرف *zOld;               /* Old column name */
};

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن خالی renameTokenCheckAll(Parse *pParse, ثابت خالی *pPtr){
  أكد( pParse==pParse->db->pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  اگر( pParse->nErr==0 ){
    ثابت RenameToken *p;
    u32 i = 1;
    برائے(p=pParse->pRename; p; p=p->p_اگلا){
      اگر( p->p ){
        أكد( p->p!=pPtr );
        i += *(u8*)(p->p) | 1;
      }
    }
    أكد( i>0 );
  }
}
#ورنہ
# define renameTokenCheckAll(x,y)
#اگر_ختم

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
ثابت خالی *sqlite3RenameTokenMap(
  Parse *pParse,
  ثابت خالی *pPtr,
  ثابت Token *pToken
){
  RenameToken *pNew;
  أكد( pPtr || pParse->db->mallocFailed );
  renameTokenCheckAll(pParse, pPtr);
  اگر( ALWAYS(pParse->e_Parse_منوال!=PARSE_منوال_UNMAP) ){
    pNew = sqlite3DbMallocZero(pParse->db, حجم(RenameToken));
    اگر( pNew ){
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->p_اگلا = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  واپس pPtr;
}

/*
** It is assumed that there is already a RenameToken object associated
** with parse tree element pFrom. This function remaps the associated token
** to parse tree element pTo.
*/
خالی sqlite3RenameTokenRemap(Parse *pParse, ثابت خالی *p_مقصد, ثابت خالی *pFrom){
  RenameToken *p;
  renameTokenCheckAll(pParse, p_مقصد);
  برائے(p=pParse->pRename; p; p=p->p_اگلا){
    اگر( p->p==pFrom ){
      p->p = p_مقصد;
      توڑو;
    }
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ساکن صحیح rename_Unmap_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (ثابت خالی*)p_اظہاریہ);
  اگر( اظہاریہ_Use_YTab(p_اظہاریہ) ){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت خالی*)&p_اظہاریہ->y.pTab);
  }
  واپس WRC_Continue;
}

/*
** Iterate through the Select objects that are part of WITH clauses attached
** to select statement pSelect.
*/
ساکن خالی renameWalkWith(Walker *pWalker, Select *pSelect){
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
      pCopy = sqlite3_With_ڈالو(pParse, pCopy, 1);
    }
    برائے(i=0; i<pWith->nCte; i++){
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, حجم(sNC));
      sNC.pParse = pParse;
      اگر( pCopy ) sqlite3SelectPrep(sNC.pParse, p, &sNC);
      اگر( sNC.pParse->db->mallocFailed ) واپس;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].p_ستون_تعداد);
    }
    اگر( pCopy && pParse->pWith==pCopy ){
      pParse->pWith = pCopy->pOuter;
    }
  }
}

/*
** Unmap all tokens in the IdList object passed as the second argument.
*/
ساکن خالی unmap_عمود_Idlist_نام(
  Parse *pParse,
  ثابت IdList *pIdList
){
  صحیح ii;
  أكد( pIdList!=0 );
  برائے(ii=0; ii<pIdList->nId; ii++){
    sqlite3RenameTokenRemap(pParse, 0, (ثابت خالی*)pIdList->a[ii].zName);
  }
}

/*
** Walker callback used by sqlite3RenameExprUnmap().
*/
ساکن صحیح renameUnmapSelectCb(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  صحیح i;
  اگر( pParse->nErr ) واپس WRC_Abort;
  testcase( p->selFlags & SF_View );
  testcase( p->selFlags & SF_CopyCte );
  اگر( p->selFlags & (SF_View|SF_CopyCte) ){
    واپس WRC_Prune;
  }
  اگر( ALWAYS(p->pEList) ){
    اظہاریہ_List *pList = p->pEList;
    برائے(i=0; i<pList->n_اظہاریہ; i++){
      اگر( pList->a[i].zEName && pList->a[i].fg.eEName==ENAME_NAME ){
        sqlite3RenameTokenRemap(pParse, 0, (خالی*)pList->a[i].zEName);
      }
    }
  }
  اگر( ALWAYS(p->pSrc) ){  /* Every Select as a SrcList, even if it is empty */
    SrcList *pSrc = p->pSrc;
    برائے(i=0; i<pSrc->nSrc; i++){
      sqlite3RenameTokenRemap(pParse, 0, (خالی*)pSrc->a[i].zName);
      اگر( pSrc->a[i].fg.isUsing==0 ){
        sqlite3_Walk_اظہاریہ(pWalker, pSrc->a[i].u3.pOn);
      }ورنہ{
        unmap_عمود_Idlist_نام(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  واپس WRC_Continue;
}

/*
** Remove all nodes that are part of expression pExpr from the rename list.
*/
خالی sqlite3_Rename_اظہاریہ_Unmap(Parse *pParse, اظہاریہ *p_اظہاریہ){
  u8 e_منوال = pParse->e_Parse_منوال;
  Walker sWalker;
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = pParse;
  sWalker.x_اظہاریہ_Callback = rename_Unmap_اظہاریہ_Cb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->e_Parse_منوال = PARSE_منوال_UNMAP;
  sqlite3_Walk_اظہاریہ(&sWalker, p_اظہاریہ);
  pParse->e_Parse_منوال = e_منوال;
}

/*
** Remove all nodes that are part of expression-list pEList from the
** rename list.
*/
خالی sqlite3RenameExprlistUnmap(Parse *pParse, اظہاریہ_List *pEList){
  اگر( pEList ){
    صحیح i;
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = pParse;
    sWalker.x_اظہاریہ_Callback = rename_Unmap_اظہاریہ_Cb;
    sqlite3_Walk_اظہاریہ_List(&sWalker, pEList);
    برائے(i=0; i<pEList->n_اظہاریہ; i++){
      اگر( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME) ){
        sqlite3RenameTokenRemap(pParse, 0, (خالی*)pEList->a[i].zEName);
      }
    }
  }
}

/*
** Free the list of RenameToken objects given in the second argument
*/
ساکن خالی renameTokenFree(sqlite3 *db, RenameToken *pToken){
  RenameToken *p_اگلا;
  RenameToken *p;
  برائے(p=pToken; p; p=p_اگلا){
    p_اگلا = p->p_اگلا;
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
ساکن RenameToken *rename_Token_ڈھونڈو(
  Parse *pParse,
  ساخت RenameCtx *pCtx,
  ثابت خالی *pPtr
){
  RenameToken **pp;
  اگر( NEVER(pPtr==0) ){
    واپس 0;
  }
  برائے(pp=&pParse->pRename; (*pp); pp=&(*pp)->p_اگلا){
    اگر( (*pp)->p==pPtr ){
      RenameToken *pToken = *pp;
      اگر( pCtx ){
        *pp = pToken->p_اگلا;
        pToken->p_اگلا = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      واپس pToken;
    }
  }
  واپس 0;
}

/*
** This is a Walker select callback. It does nothing. It is only required
** because without a dummy callback, sqlite3WalkExpr() and similar do not
** descend into sub-select statements.
*/
ساکن صحیح rename_عمود_Select_Cb(Walker *pWalker, Select *p){
  اگر( p->selFlags & (SF_View|SF_CopyCte) ){
    testcase( p->selFlags & SF_View );
    testcase( p->selFlags & SF_CopyCte );
    واپس WRC_Prune;
  }
  renameWalkWith(pWalker, p);
  واپس WRC_Continue;
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
ساکن صحیح rename_عمود_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  RenameCtx *p = pWalker->u.pRename;
  اگر( p_اظہاریہ->op==TK_TRIGGER
   && p_اظہاریہ->i_عمود==p->i_ستون
   && pWalker->pParse->pTriggerTab==p->pTab
  ){
    rename_Token_ڈھونڈو(pWalker->pParse, p, (خالی*)p_اظہاریہ);
  }ورنہ اگر( p_اظہاریہ->op==TK_عمود
   && p_اظہاریہ->i_عمود==p->i_ستون
   && ALWAYS(اظہاریہ_Use_YTab(p_اظہاریہ))
   && p->pTab==p_اظہاریہ->y.pTab
  ){
    rename_Token_ڈھونڈو(pWalker->pParse, p, (خالی*)p_اظہاریہ);
  }
  واپس WRC_Continue;
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
ساکن RenameToken *rename_عمود_Token_اگلا(RenameCtx *pCtx){
  RenameToken *p_بہترین = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  برائے(pToken=p_بہترین->p_اگلا; pToken; pToken=pToken->p_اگلا){
    اگر( pToken->t.z>p_بہترین->t.z ) p_بہترین = pToken;
  }
  برائے(pp=&pCtx->pList; *pp!=p_بہترین; pp=&(*pp)->p_اگلا);
  *pp = p_بہترین->p_اگلا;

  واپس p_بہترین;
}

/*
** Set the error message of the context passed as the first argument to
** the result of formatting zFmt using printf() style formatting.
*/
ساکن خالی errorMPrintf(sqlite3_context *pCtx, ثابت حرف *zFmt, ...){
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  حرف *zErr = 0;
  va_list ap;
  va_start(ap, zFmt);
  zErr = sqlite3VMPrintf(db, zFmt, ap);
  va_end(ap);
  اگر( zErr ){
    sqlite3_نتیجہ_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  }ورنہ{
    sqlite3_نتیجہ_error_nomem(pCtx);
  }
}

/*
** An error occurred while parsing or otherwise processing a database
** object (either pParse->pNewTable, pNewIndex or pNewTrigger) as part of an
** ALTER TABLE RENAME COLUMN program. The error message emitted by the
** sub-routine is currently stored in pParse->zErrMsg. This function
** adds context to the error message and then stores it in pCtx.
*/
ساکن خالی rename_عمود_Parse_Error(
  sqlite3_context *pCtx,
  ثابت حرف *zWhen,
  sqlite3_قدر *pType,
  sqlite3_قدر *pObject,
  Parse *pParse
){
  ثابت حرف *zT = (ثابت حرف*)sqlite3_قدر_متن(pType);
  ثابت حرف *zN = (ثابت حرف*)sqlite3_قدر_متن(pObject);
  حرف *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s",
      zT, zN, (zWhen[0] ? " " : ""), zWhen,
      pParse->z_Err_پیغام
  );
  sqlite3_نتیجہ_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

/*
** For each name in the the expression-list pEList (i.e. each
** pEList->a[i].zName) that matches the string in zOld, extract the
** corresponding rename-token from Parse object pParse and add it
** to the RenameCtx pCtx.
*/
ساکن خالی rename_عمود_Elist_نام(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت اظہاریہ_List *pEList,
  ثابت حرف *zOld
){
  اگر( pEList ){
    صحیح i;
    برائے(i=0; i<pEList->n_اظہاریہ; i++){
      ثابت حرف *zName = pEList->a[i].zEName;
      اگر( ALWAYS(pEList->a[i].fg.eEName==ENAME_NAME)
       && ALWAYS(zName!=0)
       && 0==sqlite3_stricmp(zName, zOld)
      ){
        rename_Token_ڈھونڈو(pParse, pCtx, (ثابت خالی*)zName);
      }
    }
  }
}

/*
** For each name in the the id-list pIdList (i.e. each pIdList->a[i].zName)
** that matches the string in zOld, extract the corresponding rename-token
** from Parse object pParse and add it to the RenameCtx pCtx.
*/
ساکن خالی rename_عمود_Idlist_نام(
  Parse *pParse,
  RenameCtx *pCtx,
  ثابت IdList *pIdList,
  ثابت حرف *zOld
){
  اگر( pIdList ){
    صحیح i;
    برائے(i=0; i<pIdList->nId; i++){
      ثابت حرف *zName = pIdList->a[i].zName;
      اگر( 0==sqlite3_stricmp(zName, zOld) ){
        rename_Token_ڈھونڈو(pParse, pCtx, (ثابت خالی*)zName);
      }
    }
  }
}


/*
** Parse the SQL statement zSql using Parse object (*p). The Parse object
** is initialized by this function before it is used.
*/
ساکن صحیح renameParseSql(
  Parse *p,                       /* Memory to use for Parse object */
  ثابت حرف *zDb,                /* Name of schema SQL belongs to */
  sqlite3 *db,                    /* Database handle */
  ثابت حرف *zSql,               /* SQL to parse */
  صحیح b_عارضی                       /* True if SQL is from temp schema */
){
  صحیح rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  اگر( zSql==0 ){
    واپس SQLITE_NOMEM;
  }
  اگر( sqlite3StrNICmp(zSql,"CREATE ",7)!=0 ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  اگر( b_عارضی ){
    db->init.iDb = 1;
  }ورنہ{
    صحیح iDb = sqlite3_ڈھونڈو_Db_Name(db, zDb);
    أكد( iDb>=0 && iDb<=0xff );
    db->init.iDb = (u8)iDb;
  }
  p->e_Parse_منوال = PARSE_منوال_RENAME;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  testcase( (db->flags & SQLITE_Comments)==0 && لڑی_تلاش(zSql," /* ")!=0 );
  db->flags |= SQLITE_Comments;
  rc = sqlite3_تسلسل_Parser(p, zSql);
  db->flags = flags;
  اگر( db->mallocFailed ) rc = SQLITE_NOMEM;
  اگر( rc==SQLITE_OK
   && NEVER(p->p_New_جدول==0 && p->p_New_اشاریہ==0 && p->pNewTrigger==0)
  ){
    rc = SQLITE_CORRUPT_BKPT;
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Ensure that all mappings in the Parse.pRename list really do map to
  ** a part of the input string.  */
  اگر( rc==SQLITE_OK ){
    صحیح nSql = sqlite3Strlen30(zSql);
    RenameToken *pToken;
    برائے(pToken=p->pRename; pToken; pToken=pToken->p_اگلا){
      أكد( pToken->t.z>=zSql && &pToken->t.z[pToken->t.n]<=&zSql[nSql] );
    }
  }
#اگر_ختم

  db->init.iDb = 0;
  واپس rc;
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
ساکن صحیح renameEditSql(
  sqlite3_context *pCtx,          /* Return result here */
  RenameCtx *pRename,             /* Rename context */
  ثابت حرف *zSql,               /* SQL statement to edit */
  ثابت حرف *zNew,               /* New token text */
  صحیح bQuote                      /* True to always quote token */
){
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  صحیح rc = SQLITE_OK;
  حرف *zQuot = 0;
  حرف *z_برآمد;
  i64 nQuot = 0;
  حرف *zBuf1 = 0;
  حرف *zBuf2 = 0;

  اگر( zNew ){
    /* Set zQuot to point to a buffer containing a quoted copy of the
    ** identifier zNew. If the corresponding identifier in the original
    ** ALTER TABLE statement was quoted (bQuote==1), then set zNew to
    ** point to zQuot so that all substitutions are made using the
    ** quoted version of the new column name.  */
    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    اگر( zQuot==0 ){
      واپس SQLITE_NOMEM;
    }ورنہ{
      nQuot = sqlite3Strlen30(zQuot)-1;
    }

    أكد( nQuot>=nNew && nSql>=0 && nNew>=0 );
    z_برآمد = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList*(u64)nQuot + 1);
  }ورنہ{
    أكد( nSql>0 );
    z_برآمد = (حرف*)sqlite3DbMallocZero(db, (2*(u64)nSql + 1) * 3);
    اگر( z_برآمد ){
      zBuf1 = &z_برآمد[nSql*2+1];
      zBuf2 = &z_برآمد[nSql*4+2];
    }
  }

  /* At this point pRename->pList contains a list of RenameToken objects
  ** corresponding to all tokens in the input SQL that must be replaced
  ** with the new column name, or with single-quoted versions of themselves.
  ** All that remains is to construct and return the edited SQL string. */
  اگر( z_برآمد ){
    i64 n_برآمد = nSql;
    أكد( nSql>0 );
    حافظہ_نقل(z_برآمد, zSql, (size_t)nSql);
    جب( pRename->pList ){
      صحیح iOff;                   /* Offset of token to replace in zOut */
      i64 nReplace;
      ثابت حرف *zReplace;
      RenameToken *p_بہترین = rename_عمود_Token_اگلا(pRename);

      اگر( zNew ){
        اگر( bQuote==0 && sqlite3IsIdChar(*(u8*)p_بہترین->t.z) ){
          nReplace = nNew;
          zReplace = zNew;
        }ورنہ{
          nReplace = nQuot;
          zReplace = zQuot;
          اگر( p_بہترین->t.z[p_بہترین->t.n]=='"' ) nReplace++;
        }
      }ورنہ{
        /* Dequote the double-quoted token. Then requote it again, this time
        ** using single quotes. If the character immediately following the
        ** original token within the input SQL was a single quote ('), then
        ** add another space after the new, single-quoted version of the
        ** token. This is so that (SELECT "string"'alias') maps to
        ** (SELECT 'string' 'alias'), and not (SELECT 'string''alias').  */
        حافظہ_نقل(zBuf1, p_بہترین->t.z, p_بہترین->t.n);
        zBuf1[p_بہترین->t.n] = 0;
        sqlite3Dequote(zBuf1);
        أكد( nSql < 0x15555554 /* otherwise malloc would have failed */ );
        sqlite3_snprintf((صحیح)(nSql*2), zBuf2, "%Q%s", zBuf1,
            p_بہترین->t.z[p_بہترین->t.n]=='\'' ? " " : ""
        );
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (صحیح)(p_بہترین->t.z - zSql);
      اگر( p_بہترین->t.n!=nReplace ){
        memmove(&z_برآمد[iOff + nReplace], &z_برآمد[iOff + p_بہترین->t.n],
            n_برآمد - (iOff + p_بہترین->t.n)
        );
        n_برآمد += nReplace - p_بہترین->t.n;
        z_برآمد[n_برآمد] = '\0';
      }
      حافظہ_نقل(&z_برآمد[iOff], zReplace, nReplace);
      sqlite3DbFree(db, p_بہترین);
    }

    sqlite3_نتیجہ_متن(pCtx, z_برآمد, -1, SQLITE_TRANSIENT);
    sqlite3DbFree(db, z_برآمد);
  }ورنہ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zQuot);
  واپس rc;
}

/*
** Set all pEList->a[].fg.eEName fields in the expression-list to val.
*/
ساکن خالی rename_سیٹ_ENames(اظہاریہ_List *pEList, صحیح val){
  أكد( val==ENAME_NAME || val==ENAME_TAB || val==ENAME_وسعت );
  اگر( pEList ){
    صحیح i;
    برائے(i=0; i<pEList->n_اظہاریہ; i++){
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
ساکن صحیح renameResolveTrigger(Parse *pParse){
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  Trigger_قدم *p_قدم;
  NameContext sNC;
  صحیح rc = SQLITE_OK;

  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = pParse;
  أكد( pNew->pTabSchema );
  pParse->pTriggerTab = sqlite3_ڈھونڈو_جدول(db, pNew->جدول_2,
      db->aDb[sqlite3_Schema_مقصد_اشاریہ(db, pNew->pTabSchema)].zDbSName
  );
  pParse->eTriggerOp = pNew->op;
  /* ALWAYS() because if the table of the trigger does not exist, the
  ** error would have been hit before this point */
  اگر( ALWAYS(pParse->pTriggerTab) ){
    rc = sqlite3_View_Get_عمود_نام(pParse, pParse->pTriggerTab)!=0;
  }

  /* Resolve symbols in WHEN clause */
  اگر( rc==SQLITE_OK && pNew->pWhen ){
    rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, pNew->pWhen);
  }

  برائے(p_قدم=pNew->قدم_list; rc==SQLITE_OK && p_قدم; p_قدم=p_قدم->p_اگلا){
    اگر( p_قدم->pSelect ){
      sqlite3SelectPrep(pParse, p_قدم->pSelect, &sNC);
      اگر( pParse->nErr ) rc = pParse->rc;
    }
    اگر( rc==SQLITE_OK && p_قدم->pSrc ){
      SrcList *pSrc = sqlite3SrcListDup(db, p_قدم->pSrc, 0);
      اگر( pSrc ){
        Select *pSel = sqlite3SelectNew(
            pParse, p_قدم->p_اظہاریہ_List, pSrc, 0, 0, 0, 0, 0, 0
        );
        اگر( pSel==0 ){
          p_قدم->p_اظہاریہ_List = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        }ورنہ{
          /* pStep->pExprList contains an expression-list used for an UPDATE
          ** statement. So the a[].zEName values are the RHS of the
          ** "<col> = <expr>" clauses of the UPDATE statement. So, before
          ** running SelectPrep(), change all the eEName values in
          ** pStep->pExprList to ENAME_SPAN (from their current value of
          ** ENAME_NAME). This is to prevent any ids in ON() clauses that are
          ** part of pSrc from being incorrectly resolved against the
          ** a[].zEName values as if they were column aliases.  */
          rename_سیٹ_ENames(p_قدم->p_اظہاریہ_List, ENAME_وسعت);
          sqlite3SelectPrep(pParse, pSel, 0);
          rename_سیٹ_ENames(p_قدم->p_اظہاریہ_List, ENAME_NAME);
          rc = pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
          أكد( p_قدم->p_اظہاریہ_List==0 || p_قدم->p_اظہاریہ_List==pSel->pEList );
          أكد( pSrc==pSel->pSrc );
          اگر( p_قدم->p_اظہاریہ_List ) pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        اگر( ALWAYS(p_قدم->pSrc) ){
          صحیح i;
          برائے(i=0; i<p_قدم->pSrc->nSrc && rc==SQLITE_OK; i++){
            Src_شے *p = &p_قدم->pSrc->a[i];
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
        اگر( rc==SQLITE_OK && p_قدم->pWhere ){
          rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, p_قدم->pWhere);
        }
        اگر( rc==SQLITE_OK ){
          rc = sqlite3_Resolve_اظہاریہ_List_نام(&sNC, p_قدم->p_اظہاریہ_List);
        }
        أكد( !p_قدم->pUpsert || (!p_قدم->pWhere && !p_قدم->p_اظہاریہ_List) );
        اگر( p_قدم->pUpsert && rc==SQLITE_OK ){
          Upsert *pUpsert = p_قدم->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = NC_UUpsert;
          rc = sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pUpsert->p_Upsert_ہدف);
          اگر( rc==SQLITE_OK ){
            اظہاریہ_List *p_Upsert_سیٹ = pUpsert->p_Upsert_سیٹ;
            rc = sqlite3_Resolve_اظہاریہ_List_نام(&sNC, p_Upsert_سیٹ);
          }
          اگر( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, pUpsert->pUpsertWhere);
          }
          اگر( rc==SQLITE_OK ){
            rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, pUpsert->p_Upsert_ہدف_Where);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      }ورنہ{
        rc = SQLITE_NOMEM;
      }
    }
  }
  واپس rc;
}

/*
** Invoke sqlite3WalkExpr() or sqlite3WalkSelect() on all Select or Expr
** objects that are part of the trigger passed as the second argument.
*/
ساکن خالی renameWalkTrigger(Walker *pWalker, Trigger *pTrigger){
  Trigger_قدم *p_قدم;

  /* Find tokens to edit in WHEN clause */
  sqlite3_Walk_اظہاریہ(pWalker, pTrigger->pWhen);

  /* Find tokens to edit in trigger steps */
  برائے(p_قدم=pTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
    sqlite3WalkSelect(pWalker, p_قدم->pSelect);
    sqlite3_Walk_اظہاریہ(pWalker, p_قدم->pWhere);
    sqlite3_Walk_اظہاریہ_List(pWalker, p_قدم->p_اظہاریہ_List);
    اگر( p_قدم->pUpsert ){
      Upsert *pUpsert = p_قدم->pUpsert;
      sqlite3_Walk_اظہاریہ_List(pWalker, pUpsert->p_Upsert_ہدف);
      sqlite3_Walk_اظہاریہ_List(pWalker, pUpsert->p_Upsert_سیٹ);
      sqlite3_Walk_اظہاریہ(pWalker, pUpsert->pUpsertWhere);
      sqlite3_Walk_اظہاریہ(pWalker, pUpsert->p_Upsert_ہدف_Where);
    }
    اگر( p_قدم->pSrc ){
      صحیح i;
      SrcList *pSrc = p_قدم->pSrc;
      برائے(i=0; i<pSrc->nSrc; i++){
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
ساکن خالی renameParseCleanup(Parse *pParse){
  sqlite3 *db = pParse->db;
  اشاریہ *pIdx;
  اگر( pParse->pVdbe ){
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3_Delete_جدول(db, pParse->p_New_جدول);
  جب( (pIdx = pParse->p_New_اشاریہ)!=0 ){
    pParse->p_New_اشاریہ = pIdx->p_اگلا;
    sqlite3_Free_اشاریہ(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->z_Err_پیغام);
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
ساکن خالی rename_عمود_Func(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[3]);
  ثابت حرف *z_جدول = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
  صحیح i_ستون = sqlite3_قدر_int(argv[5]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_قدر_متن(argv[6]);
  صحیح bQuote = sqlite3_قدر_int(argv[7]);
  صحیح b_عارضی = sqlite3_قدر_int(argv[8]);
  ثابت حرف *zOld;
  صحیح rc;
  Parse sParse;
  Walker sWalker;
  اشاریہ *pIdx;
  صحیح i;
  جدول *pTab;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
#اگر_ختم

  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( zSql==0 ) واپس;
  اگر( z_جدول==0 ) واپس;
  اگر( zNew==0 ) واپس;
  اگر( i_ستون<0 ) واپس;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3_ڈھونڈو_جدول(db, z_جدول, zDb);
  اگر( pTab==0 || i_ستون>=pTab->n_ستون ){
    sqlite3BtreeLeaveAll(db);
    واپس;
  }
  zOld = pTab->a_ستون[i_ستون].zCnName;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.i_ستون = ((i_ستون==pTab->iPKey) ? -1 : i_ستون);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = 0;
#اگر_ختم
  rc = renameParseSql(&sParse, zDb, db, zSql, b_عارضی);

  /* Find tokens that need to be replaced. */
  memset(&sWalker, 0, حجم(Walker));
  sWalker.pParse = &sParse;
  sWalker.x_اظہاریہ_Callback = rename_عمود_اظہاریہ_Cb;
  sWalker.xSelectCallback = rename_عمود_Select_Cb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  اگر( rc!=SQLITE_OK ) جاؤ rename_عمود_Func_مکمل;
  اگر( sParse.p_New_جدول ){
    اگر( IsView(sParse.p_New_جدول) ){
      Select *pSelect = sParse.p_New_جدول->u.view.pSelect;
      pSelect->selFlags &= ~(u32)SF_View;
      sParse.rc = SQLITE_OK;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
      اگر( rc==SQLITE_OK ){
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      اگر( rc!=SQLITE_OK ) جاؤ rename_عمود_Func_مکمل;
    }ورنہ اگر( Is_Ordinary_جدول(sParse.p_New_جدول) ){
      /* A regular table */
      صحیح bFKOnly = sqlite3_stricmp(z_جدول, sParse.p_New_جدول->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.p_New_جدول;
      اگر( bFKOnly==0 ){
        اگر( i_ستون<sParse.p_New_جدول->n_ستون ){
          rename_Token_ڈھونڈو(
              &sParse, &sCtx, (خالی*)sParse.p_New_جدول->a_ستون[i_ستون].zCnName
          );
        }
        اگر( sCtx.i_ستون<0 ){
          rename_Token_ڈھونڈو(&sParse, &sCtx, (خالی*)&sParse.p_New_جدول->iPKey);
        }
        sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_جدول->pCheck);
        برائے(pIdx=sParse.p_New_جدول->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          sqlite3_Walk_اظہاریہ_List(&sWalker, pIdx->a_ستون_اظہاریہ);
        }
        برائے(pIdx=sParse.p_New_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
          sqlite3_Walk_اظہاریہ_List(&sWalker, pIdx->a_ستون_اظہاریہ);
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
        برائے(i=0; i<sParse.p_New_جدول->n_ستون; i++){
          اظہاریہ *p_اظہاریہ = sqlite3_عمود_اظہاریہ(sParse.p_New_جدول,
                                                  &sParse.p_New_جدول->a_ستون[i]);
          sqlite3_Walk_اظہاریہ(&sWalker, p_اظہاریہ);
        }
#اگر_ختم
      }

      أكد( Is_Ordinary_جدول(sParse.p_New_جدول) );
      برائے(pFKey=sParse.p_New_جدول->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
        برائے(i=0; i<pFKey->n_ستون; i++){
          اگر( bFKOnly==0 && pFKey->a_ستون[i].iFrom==i_ستون ){
            rename_Token_ڈھونڈو(&sParse, &sCtx, (خالی*)&pFKey->a_ستون[i]);
          }
          اگر( 0==sqlite3_stricmp(pFKey->z_مقصد, z_جدول)
           && 0==sqlite3_stricmp(pFKey->a_ستون[i].z_ستون, zOld)
          ){
            rename_Token_ڈھونڈو(&sParse, &sCtx, (خالی*)pFKey->a_ستون[i].z_ستون);
          }
        }
      }
    }
  }ورنہ اگر( sParse.p_New_اشاریہ ){
    sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_اشاریہ->a_ستون_اظہاریہ);
    sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
  }ورنہ{
    /* A trigger */
    Trigger_قدم *p_قدم;
    rc = renameResolveTrigger(&sParse);
    اگر( rc!=SQLITE_OK ) جاؤ rename_عمود_Func_مکمل;

    برائے(p_قدم=sParse.pNewTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
      اگر( p_قدم->pSrc ){
        جدول *p_ہدف = sqlite3_Locate_جدول_شے(&sParse, 0, &p_قدم->pSrc->a[0]);
        اگر( p_ہدف==pTab ){
          اگر( p_قدم->pUpsert ){
            اظہاریہ_List *p_Upsert_سیٹ = p_قدم->pUpsert->p_Upsert_سیٹ;
            rename_عمود_Elist_نام(&sParse, &sCtx, p_Upsert_سیٹ, zOld);
          }
          rename_عمود_Idlist_نام(&sParse, &sCtx, p_قدم->pIdList, zOld);
          rename_عمود_Elist_نام(&sParse, &sCtx, p_قدم->p_اظہاریہ_List, zOld);
        }
      }
    }

    /* Find tokens to edit in UPDATE OF clause */
    اگر( sParse.pTriggerTab==pTab ){
      rename_عمود_Idlist_نام(&sParse, &sCtx,sParse.pNewTrigger->pColumns,zOld);
    }

    /* Find tokens to edit in various expressions and selects */
    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  أكد( rc==SQLITE_OK );
  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

rename_عمود_Func_مکمل:
  اگر( rc!=SQLITE_OK ){
    اگر( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
      sqlite3_نتیجہ_قدر(context, argv[0]);
    }ورنہ اگر( sParse.z_Err_پیغام ){
      rename_عمود_Parse_Error(context, "", argv[1], argv[2], &sParse);
    }ورنہ{
      sqlite3_نتیجہ_error_رمز(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#اگر_ختم
  sqlite3BtreeLeaveAll(db);
}

/*
** Walker expression callback used by "RENAME TABLE".
*/
ساکن صحیح rename_جدول_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  RenameCtx *p = pWalker->u.pRename;
  اگر( p_اظہاریہ->op==TK_عمود
   && ALWAYS(اظہاریہ_Use_YTab(p_اظہاریہ))
   && p->pTab==p_اظہاریہ->y.pTab
  ){
    rename_Token_ڈھونڈو(pWalker->pParse, p, (خالی*)&p_اظہاریہ->y.pTab);
  }
  واپس WRC_Continue;
}

/*
** Walker select callback used by "RENAME TABLE".
*/
ساکن صحیح rename_جدول_Select_Cb(Walker *pWalker, Select *pSelect){
  صحیح i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  اگر( pSelect->selFlags & (SF_View|SF_CopyCte) ){
    testcase( pSelect->selFlags & SF_View );
    testcase( pSelect->selFlags & SF_CopyCte );
    واپس WRC_Prune;
  }
  اگر( NEVER(pSrc==0) ){
    أكد( pWalker->pParse->db->mallocFailed );
    واپس WRC_Abort;
  }
  برائے(i=0; i<pSrc->nSrc; i++){
    Src_شے *p_شے = &pSrc->a[i];
    اگر( p_شے->pSTab==p->pTab ){
      rename_Token_ڈھونڈو(pWalker->pParse, p, p_شے->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  واپس WRC_Continue;
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
ساکن خالی rename_جدول_Func(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ثابت حرف *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  ثابت حرف *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[3]);
  ثابت حرف *zOld = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
  ثابت حرف *zNew = (ثابت حرف*)sqlite3_قدر_متن(argv[5]);
  صحیح b_عارضی = sqlite3_قدر_int(argv[6]);
  غیر_مستعمل_PARAMETER(Not_مستعمل);

  اگر( z_داخلہ && zOld && zNew ){
    Parse sParse;
    صحیح rc;
    صحیح bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;
#اگر_ختم

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, حجم(RenameCtx));
    sCtx.pTab = sqlite3_ڈھونڈو_جدول(db, zOld, zDb);
    memset(&sWalker, 0, حجم(Walker));
    sWalker.pParse = &sParse;
    sWalker.x_اظہاریہ_Callback = rename_جدول_اظہاریہ_Cb;
    sWalker.xSelectCallback = rename_جدول_Select_Cb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, b_عارضی);

    اگر( rc==SQLITE_OK ){
      صحیح isLegacy = (db->flags & SQLITE_LegacyAlter);
      اگر( sParse.p_New_جدول ){
        جدول *pTab = sParse.p_New_جدول;

        اگر( IsView(pTab) ){
          اگر( isLegacy==0 ){
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, حجم(sNC));
            sNC.pParse = &sParse;

            أكد( pSelect->selFlags & SF_View );
            pSelect->selFlags &= ~(u32)SF_View;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            اگر( sParse.nErr ){
              rc = sParse.rc;
            }ورنہ{
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        }ورنہ{
          /* Modify any FK definitions to point to the new table. */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
          اگر( (isLegacy==0 || (db->flags & SQLITE_Foreign_کلیدیں))
           && !IsVirtual(pTab)
          ){
            FKey *pFKey;
            أكد( Is_Ordinary_جدول(pTab) );
            برائے(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
              اگر( sqlite3_stricmp(pFKey->z_مقصد, zOld)==0 ){
                rename_Token_ڈھونڈو(&sParse, &sCtx, (خالی*)pFKey->z_مقصد);
              }
            }
          }
#اگر_ختم

          /* If this is the table being altered, fix any table refs in CHECK
          ** expressions. Also update the name that appears right after the
          ** "CREATE [VIRTUAL] TABLE" bit. */
          اگر( sqlite3_stricmp(zOld, pTab->zName)==0 ){
            sCtx.pTab = pTab;
            اگر( isLegacy==0 ){
              sqlite3_Walk_اظہاریہ_List(&sWalker, pTab->pCheck);
            }
            rename_Token_ڈھونڈو(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      ورنہ اگر( sParse.p_New_اشاریہ ){
        rename_Token_ڈھونڈو(&sParse, &sCtx, sParse.p_New_اشاریہ->zName);
        اگر( isLegacy==0 ){
          sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
        }
      }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
      ورنہ{
        Trigger *pTrigger = sParse.pNewTrigger;
        Trigger_قدم *p_قدم;
        اگر( 0==sqlite3_stricmp(sParse.pNewTrigger->جدول_2, zOld)
            && sCtx.pTab->pSchema==pTrigger->pTabSchema
          ){
          rename_Token_ڈھونڈو(&sParse, &sCtx, sParse.pNewTrigger->جدول_2);
        }

        اگر( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
          اگر( rc==SQLITE_OK ){
            renameWalkTrigger(&sWalker, pTrigger);
            برائے(p_قدم=pTrigger->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
              اگر( p_قدم->pSrc ){
                صحیح i;
                برائے(i=0; i<p_قدم->pSrc->nSrc; i++){
                  Src_شے *p_شے = &p_قدم->pSrc->a[i];
                  اگر( 0==sqlite3_stricmp(p_شے->zName, zOld) ){
                    rename_Token_ڈھونڈو(&sParse, &sCtx, p_شے->zName);
                  }
                }
              }
            }
          }
        }
      }
#اگر_ختم
    }

    اگر( rc==SQLITE_OK ){
      rc = renameEditSql(context, &sCtx, z_داخلہ, zNew, bQuote);
    }
    اگر( rc!=SQLITE_OK ){
      اگر( rc==SQLITE_ERROR && sqlite3WritableSchema(db) ){
        sqlite3_نتیجہ_قدر(context, argv[3]);
      }ورنہ اگر( sParse.z_Err_پیغام ){
        rename_عمود_Parse_Error(context, "", argv[1], argv[2], &sParse);
      }ورنہ{
        sqlite3_نتیجہ_error_رمز(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
    db->xAuth = xAuth;
#اگر_ختم
  }

  واپس;
}

ساکن صحیح rename_Quotefix_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_STRING && (p_اظہاریہ->flags & EP_DblQuoted) ){
    rename_Token_ڈھونڈو(pWalker->pParse, pWalker->u.pRename, (ثابت خالی*)p_اظہاریہ);
  }
  واپس WRC_Continue;
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
ساکن خالی renameQuotefixFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  حرف ثابت *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#اگر_ختم

  sqlite3BtreeEnterAll(db);

  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( zDb && z_داخلہ ){
    صحیح rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, 0);

    اگر( rc==SQLITE_OK ){
      RenameCtx sCtx;
      Walker sWalker;

      /* Walker to find tokens that need to be replaced. */
      memset(&sCtx, 0, حجم(RenameCtx));
      memset(&sWalker, 0, حجم(Walker));
      sWalker.pParse = &sParse;
      sWalker.x_اظہاریہ_Callback = rename_Quotefix_اظہاریہ_Cb;
      sWalker.xSelectCallback = rename_عمود_Select_Cb;
      sWalker.u.pRename = &sCtx;

      اگر( sParse.p_New_جدول ){
        اگر( IsView(sParse.p_New_جدول) ){
          Select *pSelect = sParse.p_New_جدول->u.view.pSelect;
          pSelect->selFlags &= ~(u32)SF_View;
          sParse.rc = SQLITE_OK;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? SQLITE_NOMEM : sParse.rc);
          اگر( rc==SQLITE_OK ){
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        }ورنہ{
          صحیح i;
          sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_جدول->pCheck);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
          برائے(i=0; i<sParse.p_New_جدول->n_ستون; i++){
            sqlite3_Walk_اظہاریہ(&sWalker,
               sqlite3_عمود_اظہاریہ(sParse.p_New_جدول,
                                         &sParse.p_New_جدول->a_ستون[i]));
          }
#اگر_ختم /* SQLITE_OMIT_GENERATED_COLUMNS */
        }
      }ورنہ اگر( sParse.p_New_اشاریہ ){
        sqlite3_Walk_اظہاریہ_List(&sWalker, sParse.p_New_اشاریہ->a_ستون_اظہاریہ);
        sqlite3_Walk_اظہاریہ(&sWalker, sParse.p_New_اشاریہ->pPartIdxWhere);
      }ورنہ{
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
        rc = renameResolveTrigger(&sParse);
        اگر( rc==SQLITE_OK ){
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
#اگر_ختم /* SQLITE_OMIT_TRIGGER */
      }

      اگر( rc==SQLITE_OK ){
        rc = renameEditSql(context, &sCtx, z_داخلہ, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    اگر( rc!=SQLITE_OK ){
      اگر( sqlite3WritableSchema(db) && rc==SQLITE_ERROR ){
        sqlite3_نتیجہ_قدر(context, argv[1]);
      }ورنہ{
        sqlite3_نتیجہ_error_رمز(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#اگر_ختم

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
ساکن خالی rename_جدول_Test(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  حرف ثابت *zDb = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  حرف ثابت *z_داخلہ = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحیح b_عارضی = sqlite3_قدر_int(argv[4]);
  صحیح isLegacy = (db->flags & SQLITE_LegacyAlter);
  حرف ثابت *zWhen = (ثابت حرف*)sqlite3_قدر_متن(argv[5]);
  صحیح bNoDQS = sqlite3_قدر_int(argv[6]);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#اگر_ختم

  غیر_مستعمل_PARAMETER(Not_مستعمل);

  اگر( zDb && z_داخلہ ){
    صحیح rc;
    Parse sParse;
    u64 flags = db->flags;
    اگر( bNoDQS ) db->flags &= ~(SQLITE_DqsDML|SQLITE_DqsDDL);
    rc = renameParseSql(&sParse, zDb, db, z_داخلہ, b_عارضی);
    db->flags = flags;
    اگر( rc==SQLITE_OK ){
      اگر( isLegacy==0 && sParse.p_New_جدول && IsView(sParse.p_New_جدول) ){
        NameContext sNC;
        memset(&sNC, 0, حجم(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.p_New_جدول->u.view.pSelect, &sNC);
        اگر( sParse.nErr ) rc = sParse.rc;
      }

      ورنہ اگر( sParse.pNewTrigger ){
        اگر( isLegacy==0 ){
          rc = renameResolveTrigger(&sParse);
        }
        اگر( rc==SQLITE_OK ){
          صحیح i1 = sqlite3_Schema_مقصد_اشاریہ(db, sParse.pNewTrigger->pTabSchema);
          صحیح i2 = sqlite3_ڈھونڈو_Db_Name(db, zDb);
          اگر( i1==i2 ){
            /* Handle output case B */
            sqlite3_نتیجہ_int(context, 1);
          }
        }
      }
    }

    اگر( rc!=SQLITE_OK && zWhen && !sqlite3WritableSchema(db) ){
      /* Output case A */
      rename_عمود_Parse_Error(context, zWhen, argv[2], argv[3],&sParse);
    }
    renameParseCleanup(&sParse);
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#اگر_ختم
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
ساکن صحیح getConstraintToken(ثابت u8 *z, صحیح *piToken){
  صحیح iOff = 0;
  صحیح t = 0;
  کرو {
    iOff += sqlite3GetToken(&z[iOff], &t);
  }جب( t==TK_SPACE || t==TK_COMMENT );

  *piToken = t;

  اگر( t==TK_LP ){
    صحیح nNest = 1;
    جب( nNest>0 ){
      iOff += sqlite3GetToken(&z[iOff], &t);
      اگر( t==TK_LP ){
        nNest++;
      }ورنہ اگر( t==TK_RP ){
        t = TK_LP;
        nNest--;
      }ورنہ اگر( t==TK_ILLEGAL ){
        توڑو;
      }
    }
  }

  *piToken = t;
  واپس iOff;
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
ساکن خالی drop_عمود_Func(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحیح iSchema = sqlite3_قدر_int(argv[0]);
  ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحیح i_ستون = sqlite3_قدر_int(argv[2]);
  ثابت حرف *zDb = db->aDb[iSchema].zDbSName;
  صحیح rc;
  Parse sParse;
  RenameToken *p_ستون;
  جدول *pTab;
  ثابت حرف *z_انتہا;
  حرف *zNew = 0;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;
#اگر_ختم

  غیر_مستعمل_PARAMETER(Not_مستعمل);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema==1);
  اگر( rc!=SQLITE_OK ) جاؤ drop_عمود_مکمل;
  pTab = sParse.p_New_جدول;
  اگر( pTab==0 || pTab->n_ستون==1 || i_ستون>=pTab->n_ستون ){
    /* This can happen if the sqlite_schema table is corrupt */
    rc = SQLITE_CORRUPT_BKPT;
    جاؤ drop_عمود_مکمل;
  }

  اگر( i_ستون<pTab->n_ستون-1 ){
    RenameToken *p_انتہا;
    p_ستون = rename_Token_ڈھونڈو(&sParse, 0, (خالی*)pTab->a_ستون[i_ستون].zCnName);
    p_انتہا = rename_Token_ڈھونڈو(&sParse, 0, (خالی*)pTab->a_ستون[i_ستون+1].zCnName);
    z_انتہا = (ثابت حرف*)p_انتہا->t.z;
  }ورنہ{
    صحیح eTok;
    أكد( Is_Ordinary_جدول(pTab) );
    أكد( i_ستون!=0 );
    /* Point pCol->t.z at the "," immediately preceding the definition of
    ** the column being dropped. To do this, start at the name of the 
    ** previous column, and tokenize until the next ",".  */
    p_ستون = rename_Token_ڈھونڈو(&sParse, 0, (خالی*)pTab->a_ستون[i_ستون-1].zCnName);
    کرو {
      p_ستون->t.z += getConstraintToken((ثابت u8*)p_ستون->t.z, &eTok);
    }جب( eTok!=TK_COMMA );
    p_ستون->t.z--;
    z_انتہا = (ثابت حرف*)&zSql[pTab->u.tab.جوڑو_ستون_انحراف];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", p_ستون->t.z-zSql, zSql, z_انتہا);
  sqlite3_نتیجہ_متن(context, zNew, -1, SQLITE_TRANSIENT);
  sqlite3_free(zNew);

drop_عمود_مکمل:
  renameParseCleanup(&sParse);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  db->xAuth = xAuth;
#اگر_ختم
  اگر( rc!=SQLITE_OK ){
    sqlite3_نتیجہ_error_رمز(context, rc);
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
خالی sqlite3_Alter_Drop_عمود(Parse *pParse, SrcList *pSrc, ثابت Token *pName){
  sqlite3 *db = pParse->db;       /* Database handle */
  جدول *pTab;                    /* Table to modify */
  صحیح iDb;                        /* Index of db containing pTab in aDb[] */
  ثابت حرف *zDb;                /* Database containing pTab ("main" etc.) */
  حرف *z_ستون = 0;                 /* Name of column to drop */
  صحیح i_ستون;                       /* Index of column zCol in pTab->aCol[] */

  /* Look up the table being altered. */
  أكد( pParse->p_New_جدول==0 );
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  اگر( NEVER(db->mallocFailed) ) جاؤ exit_drop_عمود;
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  اگر( !pTab ) جاؤ exit_drop_عمود;

  /* Make sure this is not an attempt to ALTER a view, virtual table or
  ** system table. */
  اگر( SQLITE_OK!=is_Alterable_جدول(pParse, pTab) ) جاؤ exit_drop_عمود;
  اگر( SQLITE_OK!=is_Real_جدول(pParse, pTab, 1) ) جاؤ exit_drop_عمود;

  /* Find the index of the column being dropped. */
  z_ستون = sqlite3NameFromToken(db, pName);
  اگر( z_ستون==0 ){
    أكد( db->mallocFailed );
    جاؤ exit_drop_عمود;
  }
  i_ستون = sqlite3_عمود_اشاریہ(pTab, z_ستون);
  اگر( i_ستون<0 ){
    sqlite3_Error_پیغام(pParse, "no such column: \"%T\"", pName);
    جاؤ exit_drop_عمود;
  }
  اگر( isRowidAlias(pParse, pTab, z_ستون, "drop") ) جاؤ exit_drop_عمود;

  /* Do not allow the user to drop a PRIMARY KEY column or a column
  ** constrained by a UNIQUE constraint.  */
  اگر( pTab->a_ستون[i_ستون].ستون_Flags & (COLFLAG_PRIMKEY|COLFLAG_UNIQUE) ){
    sqlite3_Error_پیغام(pParse, "cannot drop %s column: \"%s\"",
        (pTab->a_ستون[i_ستون].ستون_Flags&COLFLAG_PRIMKEY) ? "PRIMARY KEY" : "UNIQUE",
        z_ستون
    );
    جاؤ exit_drop_عمود;
  }

  /* Do not allow the number of columns to go to zero */
  اگر( pTab->n_ستون<=1 ){
    sqlite3_Error_پیغام(pParse, "cannot drop column \"%s\": no other columns exist",z_ستون);
    جاؤ exit_drop_عمود;
  }

  /* Edit the sqlite_schema table */
  iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
  أكد( iDb>=0 );
  zDb = db->aDb[iDb].zDbSName;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, z_ستون) ){
    جاؤ exit_drop_عمود;
  }
#اگر_ختم
  renameTestSchema(pParse, zDb, iDb==1, "", 0);
  renameFixQuotes(pParse, zDb, iDb==1);
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_drop_column(%d, sql, %d) "
      "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)"
      , zDb, iDb, i_ستون, pTab->zName
  );

  /* Drop and reload the database schema. */
  renameReloadSchema(pParse, iDb, INITFLAG_AlterDrop);
  renameTestSchema(pParse, zDb, iDb==1, "after drop column", 1);

  /* Edit rows of table on disk */
  اگر( pParse->nErr==0 && (pTab->a_ستون[i_ستون].ستون_Flags & COLFLAG_VIRTUAL)==0 ){
    صحیح i;
    صحیح addr;
    صحیح reg;
    صحیح regRec;
    اشاریہ *pPk = 0;
    صحیح n_میدان = 0;               /* Number of non-virtual columns after drop */
    صحیح iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3_Open_جدول(pParse, iCur, iDb, pTab, OP_OpenWrite);
    addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, iCur); VdbeCoverage(v);
    reg = ++pParse->nMem;
    اگر( HasRowid(pTab) ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iCur, reg);
      pParse->nMem += pTab->n_ستون;
    }ورنہ{
      pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
      pParse->nMem += pPk->n_عمود;
      برائے(i=0; i<pPk->n_کلید_ستون; i++){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iCur, i, reg+i+1);
      }
      n_میدان = pPk->n_کلید_ستون;
    }
    regRec = ++pParse->nMem;
    برائے(i=0; i<pTab->n_ستون; i++){
      اگر( i!=i_ستون && (pTab->a_ستون[i].ستون_Flags & COLFLAG_VIRTUAL)==0 ){
        صحیح reg_برآمد;
        اگر( pPk ){
          صحیح iPos = sqlite3_جدول_عمود_مقصد_اشاریہ(pPk, i);
          صحیح i_ستون_Pos = sqlite3_جدول_عمود_مقصد_اشاریہ(pPk, i_ستون);
          اگر( iPos<pPk->n_کلید_ستون ) جاری;
          reg_برآمد = reg+1+iPos-(iPos>i_ستون_Pos);
        }ورنہ{
          reg_برآمد = reg+1+n_میدان;
        }
        اگر( i==pTab->iPKey ){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg_برآمد);
        }ورنہ{
          sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(v, pTab, iCur, i, reg_برآمد);
          أكد( pParse->nErr
               || sqlite3VdbeGetOp(v,sqlite3VdbeCurrentAddr(v)-1)->opcode
                    != OP_RealAffinity
               || pTab->a_ستون[i].affinity==SQLITE_AFF_REAL );
          sqlite3_Vdbe_Delete_پیشگی_Opcode(v, OP_RealAffinity);
        }
        n_میدان++;
      }
    }
    اگر( n_میدان==0 ){
      /* dbsqlfuzz 5f09e7bcc78b4954d06bf9f2400d7715f48d1fef */
      pParse->nMem++;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg+1);
      n_میدان = 1;
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg+1, n_میدان, regRec);
    اگر( pPk ){
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iCur, regRec, reg+1, pPk->n_کلید_ستون);
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, OPFLAG_SAVEPOSITION);

    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iCur, addr+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_یہاں(v, addr);
  }

exit_drop_عمود:
  sqlite3DbFree(db, z_ستون);
  sqlite3SrcListDelete(db, pSrc);
}

/*
** Return the number of bytes of leading whitespace/comments in string z[].
*/
ساکن صحیح getWhitespace(ثابت u8 *z){
  صحیح nRet = 0;
  جب( 1 ){
    صحیح t = 0;
    صحیح n = sqlite3GetToken(&z[nRet], &t);
    اگر( t!=TK_SPACE && t!=TK_COMMENT ) توڑو;
    nRet += n;
  }
  واپس nRet;
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
ساکن صحیح getConstraint(ثابت u8 *z, صحیح eTok){
  صحیح iOff = 0;
  صحیح t = 0;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
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
#اگر_ختم

  /* Now, the current constraint proceeds until the next occurence of one 
  ** of the following tokens: 
  **
  **   CONSTRAINT, PRIMARY, NOT, UNIQUE, CHECK, DEFAULT, 
  **   COLLATE, REFERENCES, FOREIGN, GENERATED, AS, RP, or COMMA
  **
  ** Also exit the loop if ILLEGAL turns up.
  */
  جب( 1 ){
    صحیح n = getConstraintToken(&z[iOff], &t);
    اگر( t==TK_CONSTRAINT || t==TK_PRIMARY || t==TK_NOT || t==TK_UNIQUE
     || t==TK_CHECK || t==TK_DEFAULT || t==TK_COLLATE || t==TK_REFERENCES
     || t==TK_FOREIGN || t==TK_RP || t==TK_COMMA || t==TK_ILLEGAL
     || t==TK_AS || t==TK_GENERATED
    ){
      توڑو;
    }
    iOff += n;
  }
  
  واپس iOff;
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
ساکن صحیح quotedCompare(
  sqlite3_context *ctx,  /* Function context on which to report errors */
  صحیح t,                 /* Token type */
  ثابت u8 *zQuote,      /* Possibly quoted text.  Not zero-terminated. */
  صحیح nQuote,            /* Length of zQuote in bytes */
  ثابت u8 *zCmp,        /* Zero-terminated, unquoted name to compare against */
  صحیح *pRes              /* OUT: Set to 0 if equal, non-zero if unequal */
){
  حرف *zCopy = 0;       /* De-quoted, zero-terminated copy of zQuote[] */

  اگر( t==TK_ILLEGAL ){
    *pRes = 1;
    واپس SQLITE_OK;
  }
  zCopy = sqlite3MallocZero(nQuote+1);
  اگر( zCopy==0 ){
    sqlite3_نتیجہ_error_nomem(ctx);
    واپس SQLITE_NOMEM_BKPT;
  }
  حافظہ_نقل(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((ثابت حرف*)zCopy, (ثابت حرف*)zCmp);
  sqlite3_free(zCopy);
  واپس SQLITE_OK;
}

/*
** zSql[] is a CREATE TABLE statement, supposedly.  Find the offset
** into zSql[] of the first character past the first "(" and write
** that offset into *piOff and return SQLITE_OK.  Or, if not found,
** set the SQLITE_CORRUPT error code and return SQLITE_ERROR.
*/
ساکن صحیح skip_Create_جدول(sqlite3_context *ctx, ثابت u8 *zSql, صحیح *piOff){
  صحیح iOff = 0;

  اگر( zSql==0 ) واپس SQLITE_ERROR;

  /* Jump past the "CREATE TABLE" bit. */
  جب( 1 ){
    صحیح t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    اگر( t==TK_LP ) توڑو;
    اگر( t==TK_ILLEGAL ){
      sqlite3_نتیجہ_error_رمز(ctx, SQLITE_CORRUPT_BKPT);
      واپس SQLITE_ERROR;
    }
  }

  *piOff = iOff;
  واپس SQLITE_OK;
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
ساکن خالی dropConstraintFunc(
  sqlite3_context *ctx,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = sqlite3_قدر_متن(argv[0]);
  ثابت u8 *zCons = 0;
  صحیح iNotNull = -1;
  صحیح ii;
  صحیح iOff = 0;
  صحیح i_آغاز = 0;
  صحیح i_انتہا = 0;
  حرف *zNew = 0;
  صحیح t = 0;
  sqlite3 *db;
  غیر_مستعمل_PARAMETER(Not_مستعمل);

  اگر( zSql==0 ) واپس;

  /* Jump past the "CREATE TABLE" bit. */
  اگر( skip_Create_جدول(ctx, zSql, &iOff) ) واپس;

  اگر( sqlite3_قدر_type(argv[1])==SQLITE_INTEGER ){
    iNotNull = sqlite3_قدر_int(argv[1]);
  }ورنہ{
    zCons = sqlite3_قدر_متن(argv[1]);
  }

  /* Search for the named constraint within column definitions. */
  برائے(ii=0; i_انتہا==0; ii++){
  
    /* Now parse the column or table constraint definition. Search
    ** for the token CONSTRAINT if this is a DROP CONSTRAINT command, or
    ** NOT in the right column if this is a DROP NOT NULL. */
    جب( 1 ){
      i_آغاز = iOff;
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
          اگر( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) واپس;
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
        }ورنہ{
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff], t);
        }

        اگر( cmp==0 || (iNotNull>=0 && t==TK_NOT) ){
          اگر( t!=TK_NOT && t!=TK_CHECK && t!=TK_REFERENCES && t!=TK_FOREIGN ){
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            واپس;
          }
          i_انتہا = iOff;
          توڑو;
        }

      }ورنہ اگر( t==TK_NOT && iNotNull==ii ){
        i_انتہا = iOff + getConstraint(&zSql[iOff], 0);
        توڑو;
      }ورنہ اگر( t==TK_RP || t==TK_ILLEGAL ){
        i_انتہا = -1;
        توڑو;
      }ورنہ اگر( t==TK_COMMA ){
        توڑو;
      }
    }
  }

  /* If the constraint has not been found it is an error. */
  اگر( i_انتہا<=0 ){
    اگر( zCons ){
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    }ورنہ{
      /* SQLite follows postgres in that a DROP NOT NULL on a column that is
      ** not NOT NULL is not an error. So just return the original SQL here. */
      sqlite3_نتیجہ_متن(ctx, (ثابت حرف*)zSql, -1, SQLITE_TRANSIENT);
    }
  }ورنہ{

    /* Figure out if an extra space should be inserted after the constraint
    ** is removed. And if an additional comma preceding the constraint 
    ** should be removed. */
    ثابت حرف *zSpace = " ";
    i_انتہا += getWhitespace(&zSql[i_انتہا]);
    sqlite3GetToken(&zSql[i_انتہا], &t);
    اگر( t==TK_RP || t==TK_COMMA ){
      zSpace = "";
      اگر( zSql[i_آغاز-1]==',' ) i_آغاز--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", i_آغاز, zSql, zSpace, &zSql[i_انتہا]);
    sqlite3_نتیجہ_متن(ctx, zNew, -1, SQLITE_DYNAMIC);
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
ساکن خالی جوڑو_Constraint_Func(
  sqlite3_context *ctx,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = sqlite3_قدر_متن(argv[0]);
  ثابت حرف *zCons = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
  صحیح i_ستون = sqlite3_قدر_int(argv[2]);
  صحیح iOff = 0;
  صحیح ii;
  sqlite3_str *pNew;
  صحیح t = 0;
  غیر_مستعمل_PARAMETER(Not_مستعمل);

  اگر( skip_Create_جدول(ctx, zSql, &iOff) ) واپس;
  
  برائے(ii=0; ii<=i_ستون || (i_ستون<0 && t!=TK_RP); ii++){
    iOff += getConstraintToken(&zSql[iOff], &t);
    جب( 1 ){
      صحیح nTok = getConstraintToken(&zSql[iOff], &t);
      اگر( t==TK_COMMA || t==TK_RP ) توڑو;
      اگر( t==TK_ILLEGAL ){
        sqlite3_نتیجہ_error_رمز(ctx, SQLITE_CORRUPT_BKPT);
        واپس;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  pNew = sqlite3_str_new(sqlite3_context_db_handle(ctx));
  sqlite3_str_append(pNew, (ثابت حرف*)zSql, iOff);
  اگر( i_ستون<0 ) sqlite3_str_append(pNew, ",", 1);
  sqlite3_str_appendf(pNew, " %s%s", zCons, &zSql[iOff]);
  sqlite3_نتیجہ_str(ctx, pNew, SQLITE_اختتام);
}

/*
** Find a column named pCol in table pTab. If successful, set output 
** parameter *piCol to the index of the column in the table and return
** SQLITE_OK. Otherwise, set *piCol to -1 and return an SQLite error
** code.
*/
ساکن صحیح alter_ڈھونڈو_ستون(Parse *pParse, جدول *pTab, Token *p_ستون, صحیح *pi_ستون){
  sqlite3 *db = pParse->db;
  حرف *zName = sqlite3NameFromToken(db, p_ستون);
  صحیح rc = SQLITE_NOMEM;
  صحیح i_ستون = -1;

  اگر( zName ){
    i_ستون = sqlite3_عمود_اشاریہ(pTab, zName);
    اگر( i_ستون<0 ){
      sqlite3_Error_پیغام(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    }ورنہ{
      rc = SQLITE_OK;
    }
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  اگر( rc==SQLITE_OK ){
    ثابت حرف *zDb = db->aDb[sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema)].zDbSName;
    ثابت حرف *z_ستون = pTab->a_ستون[i_ستون].zCnName;
    اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, zDb, pTab->zName, z_ستون) ){
      pTab = 0;
    }
  }
#اگر_ختم

  sqlite3DbFree(db, zName);
  *pi_ستون = i_ستون;
  واپس rc;
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
ساکن جدول *alter_ڈھونڈو_جدول(
  Parse *pParse,        /* Parsing context */
  SrcList *pSrc,        /* Name of the table to look for */
  صحیح *piDb,            /* OUT: write the iDb here */
  ثابت حرف **pzDb,    /* OUT: write name of schema here */
  صحیح bAuth             /* Do ALTER TABLE authorization checks if true */
){
  sqlite3 *db = pParse->db;
  جدول *pTab = 0;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  pTab = sqlite3_Locate_جدول_شے(pParse, 0, &pSrc->a[0]);
  اگر( pTab ){
    صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    اگر( SQLITE_OK!=is_Real_جدول(pParse, pTab, 2) 
     || SQLITE_OK!=is_Alterable_جدول(pParse, pTab) 
    ){
      pTab = 0;
    }
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  اگر( pTab && bAuth ){
    اگر( sqlite3AuthCheck(pParse, SQLITE_ALTER_جدول, *pzDb, pTab->zName, 0) ){
      pTab = 0;
    }
  }
#اگر_ختم
  sqlite3SrcListDelete(db, pSrc);
  واپس pTab;
}

/*
** Generate bytecode for one of:
**
**  (1)   ALTER TABLE pSrc DROP CONSTRAINT pCons
**  (2)   ALTER TABLE pSrc ALTER pCol DROP NOT NULL
**
** One of pCons and pCol must be NULL and the other non-null.
*/
خالی sqlite3AlterDropConstraint(
  Parse *pParse,    /* Parsing context */
  SrcList *pSrc,    /* The table being altered */
  Token *pCons,     /* Name of the constraint to drop */
  Token *p_ستون       /* Name of the column from which to remove the NOT NULL */
){
  sqlite3 *db = pParse->db;
  جدول *pTab = 0;
  صحیح iDb = 0;
  ثابت حرف *zDb = 0;
  حرف *zArg = 0;

  أكد( (p_ستون==0)!=(pCons==0) );
  أكد( pSrc->nSrc==1 );
  pTab = alter_ڈھونڈو_جدول(pParse, pSrc, &iDb, &zDb, pCons!=0);
  اگر( !pTab ) واپس;

  اگر( pCons ){
    حرف *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  }ورنہ{
    صحیح i_ستون;
    اگر( alter_ڈھونڈو_ستون(pParse, pTab, p_ستون, &i_ستون) ) واپس;
    zArg = sqlite3MPrintf(db, "%d", i_ستون);
  }

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
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
ساکن خالی failConstraintFunc(
  sqlite3_context *ctx,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  ثابت حرف *z_متن = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  صحیح err = sqlite3_قدر_int(argv[1]);
  (خالی)Not_مستعمل;
  sqlite3_نتیجہ_error(ctx, z_متن, -1);
  sqlite3_نتیجہ_error_رمز(ctx, err);
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
ساکن صحیح alterRtrimConstraint(
  sqlite3 *db,                    /* used to record OOM error */
  ثابت حرف *pCons,              /* Buffer containing constraint */
  صحیح nCons                       /* Size of pCons in bytes */
){
  u8 *zTmp = (u8*)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  صحیح iOff = 0;
  صحیح i_انتہا = 0;

  اگر( zTmp==0 ) واپس 0;

  جب( 1 ){
    صحیح t = 0;
    صحیح nToken = sqlite3GetToken(&zTmp[iOff], &t);
    اگر( t==TK_ILLEGAL ) توڑو;
    اگر( t!=TK_SPACE && (t!=TK_COMMENT || zTmp[iOff]!='-') ){
      i_انتہا = iOff+nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  واپس i_انتہا;
}

/*
** Prepare a statement of the form:
**
**   ALTER TABLE pSrc ALTER pCol SET NOT NULL
*/
خالی sqlite3_Alter_سیٹ_Not_Null(
  Parse *pParse,   /* Parsing context */
  SrcList *pSrc,   /* Name of the table being altered */
  Token *p_ستون,     /* Name of the column to add a NOT NULL constraint to */
  Token *p_پہلا    /* The NOT token of the NOT NULL constraint text */
){
  جدول *pTab = 0;
  صحیح i_ستون = 0;
  صحیح iDb = 0;
  ثابت حرف *zDb = 0;
  ثابت حرف *pCons = 0;
  صحیح nCons = 0;

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alter_ڈھونڈو_جدول(pParse, pSrc, &iDb, &zDb, 0);
  اگر( !pTab ) واپس;

  /* Find the column being altered. */
  اگر( alter_ڈھونڈو_ستون(pParse, pTab, p_ستون, &i_ستون) ){
    واپس;
  }

  /* Find the length in bytes of the constraint definition */
  pCons = p_پہلا->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_آخری_Token.z - pCons);

  /* Search for a constraint violation. Throw an exception if one is found. */
  sqlite3NestedParse(pParse,
      "SELECT sqlite_fail('constraint failed', %d) "
      "FROM %Q.%Q AS x WHERE x.%.*s IS NULL", 
      SQLITE_CONSTRAINT, zDb, pTab->zName, (صحیح)p_ستون->n, p_ستون->z
  );

  /* Edit the SQL for the named table. */
  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
      "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
      "WHERE type='table' AND tbl_name=%Q COLLATE nocase"
      , zDb, i_ستون, nCons, pCons, i_ستون, pTab->zName
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
ساکن خالی ڈھونڈو_Constraint_Func(
  sqlite3_context *ctx,
  صحیح Not_مستعمل,
  sqlite3_قدر **argv
){
  ثابت u8 *zSql = 0;
  ثابت u8 *zCons = 0;
  صحیح iOff = 0;
  صحیح t = 0;

  (خالی)Not_مستعمل;
  zSql = sqlite3_قدر_متن(argv[0]);
  zCons = sqlite3_قدر_متن(argv[1]);

  اگر( zSql==0 || zCons==0 ) واپس;
  جب( t!=TK_LP && t!=TK_ILLEGAL ){
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  جب( 1 ){
    iOff += getConstraintToken(&zSql[iOff], &t);
    اگر( t==TK_CONSTRAINT ){
      صحیح nTok = 0;
      صحیح cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      اگر( quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp) ) واپس;
      اگر( cmp==0 ){
        sqlite3_نتیجہ_int(ctx, 1);
        واپس;
      }
    }ورنہ اگر( t==TK_ILLEGAL ){
      توڑو;
    }
  }

  sqlite3_نتیجہ_int(ctx, 0);
}

/*
** Generate bytecode to implement:
**
**    ALTER TABLE pSrc ADD [CONSTRAINT pName] CHECK(pExpr)
**
** Any "ON CONFLICT" text that occurs after the "CHECK(...)", up
** until pParse->sLastToken, is included as part of the new constraint.
*/
خالی sqlite3_Alter_جوڑو_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_پہلا,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ثابت حرف *z_اظہاریہ,       /* Text of CHECK expression */
  صحیح n_اظہاریہ,               /* Size of pExpr in bytes */
  اظہاریہ *p_اظہاریہ              /* The parsed CHECK expression */
){ 
  جدول *pTab = 0;         /* Table identified by pSrc */
  صحیح iDb = 0;             /* Which schema does pTab live in */
  ثابت حرف *zDb = 0;     /* Name of the schema in which pTab lives */
  ثابت حرف *pCons = 0;   /* Text of the constraint */
  صحیح nCons;               /* Bytes of text to use from pCons[] */
  صحیح rc;                  /* Result from error checking pExpr */

  /* Look up the table being altered. */
  أكد( pSrc->nSrc==1 );
  pTab = alter_ڈھونڈو_جدول(pParse, pSrc, &iDb, &zDb, 1);
  اگر( !pTab ){
    sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
    واپس;
  }

  /* Verify that the new CHECK constraint does not contain any
  ** internal-use-only function.  Forum post 2026-05-10T01:11:28Z
  */
  rc = sqlite3ResolveSelfReference(pParse, pTab, NC_IsCheck, p_اظہاریہ, 0);
  sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
  اگر( rc ) واپس;

  /* If this new constraint has a name, check that it is not a duplicate of
  ** an existing constraint. It is an error if it is.  */
  اگر( pName ){
    حرف *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
        "SELECT sqlite_fail('constraint %q already exists', %d) "
        "FROM \"%w\"." LEGACY_SCHEMA_جدول " "
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
      SQLITE_CONSTRAINT, zDb, pTab->zName, n_اظہاریہ, z_اظہاریہ
  );

  /* Edit the SQL for the named table. */
  pCons = p_پہلا->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->s_آخری_Token.z - pCons);

  sqlite3NestedParse(pParse,
      "UPDATE \"%w\"." LEGACY_SCHEMA_جدول " SET "
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
خالی sqlite3AlterFunctions(خالی){
  ساکن FuncDef a_Alter_جدول_Funcs[] = {
    اندرونی_FUNCTION(sqlite_rename_عمود,  9, rename_عمود_Func),
    اندرونی_FUNCTION(sqlite_rename_جدول,   7, rename_جدول_Func),
    اندرونی_FUNCTION(sqlite_rename_test,    7, rename_جدول_Test),
    اندرونی_FUNCTION(sqlite_drop_عمود,    3, drop_عمود_Func),
    اندرونی_FUNCTION(sqlite_rename_quotefix,2, renameQuotefixFunc),
    اندرونی_FUNCTION(sqlite_drop_constraint,2, dropConstraintFunc),
    اندرونی_FUNCTION(sqlite_fail,           2, failConstraintFunc),
    اندرونی_FUNCTION(sqlite_جوڑو_constraint, 3, جوڑو_Constraint_Func),
    اندرونی_FUNCTION(sqlite_ڈھونڈو_constraint,2, ڈھونڈو_Constraint_Func),
  };
  sqlite3_داخل_کرو_Builtin_Funcs(a_Alter_جدول_Funcs, Array_سائز(a_Alter_جدول_Funcs));
}
#اگر_ختم  /* SQLITE_ALTER_TABLE */
