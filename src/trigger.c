/*
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the implementation for TRIGGERs
*/
#अन्तर्भाव "sqliteInt.h"

#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
शून्य sqlite3_Delete_Trigger_चरणः(sqlite3 *db, Trigger_चरणः *p_Trigger_चरणः){
  यावत्( p_Trigger_चरणः ){
    Trigger_चरणः * pTmp = p_Trigger_चरणः;
    p_Trigger_चरणः = p_Trigger_चरणः->p_अग्र;

    sqlite3_व्यञ्जकम्_Delete(db, pTmp->pWhere);
    sqlite3_व्यञ्जकम्_List_Delete(db, pTmp->p_व्यञ्जकम्_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_विस्तृतिः);

    sqlite3DbFree(db, pTmp);
  }
}

/*
** Given table pTab, return a list of all the triggers attached to 
** the table. The list is connected by Trigger.pNext pointers.
**
** All of the triggers on pTab that are in the same database as pTab
** are already attached to pTab->pTrigger.  But there might be additional
** triggers on pTab in the TEMP schema.  This routine prepends all
** TEMP triggers on pTab to the beginning of the pTab->pTrigger list
** and returns the combined list.
**
** To state it another way:  This routine returns a list of all triggers
** that fire off of pTab.  The list will include any TEMP triggers on
** pTab as well as the triggers lised in pTab->pTrigger.
*/
Trigger *sqlite3TriggerList(Parse *pParse, सारणी *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  प्रतिज्ञा( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  यावत्( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_दत्त(p);
    यदि( pTrig->pTabSchema==pTab->pSchema
     && pTrig->सारणी_2
     && 0==sqlite3StrICmp(pTrig->सारणी_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_अग्र = pList;
      pList = pTrig;
    }अन्यथा यदि( pTrig->op==TK_RETURNING ){
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
      प्रतिज्ञा( pParse->db->pVtabCtx==0 );
#यदि_अन्तः
      प्रतिज्ञा( pParse->bReturning );
      प्रतिज्ञा( !pParse->isCreate );
      प्रतिज्ञा( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->सारणी_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_अग्र = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_अग्र(p);    
  }
#यदि 0
  यदि( pList ){
    Trigger *pX;
    प्रारूप_लिख("Triggers for %s:", pTab->zName);
    प्रति(pX=pList; pX; pX=pX->p_अग्र){
      प्रारूप_लिख(" %s", pX->zName);
    }
    प्रारूप_लिख("\n");
    fflush(stdout);
  }
#यदि_अन्तः
  प्रत्यागम pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
शून्य sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  पूर्णाङ्क tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  पूर्णाङ्क op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *p_सारणी_Name,/* The name of the table/view the trigger applies to */
  व्यञ्जकम् *pWhen,        /* WHEN clause */
  पूर्णाङ्क is_अस्थायी,         /* True if the TEMPORARY keyword is present */
  पूर्णाङ्क noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  सारणी *pTab;            /* Table that the trigger fires off of */
  वर्ण *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  पूर्णाङ्क iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  प्रतिज्ञा( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  प्रतिज्ञा( pName2!=0 );
  प्रतिज्ञा( op==TK_निवेशय || op==TK_UPDATE || op==TK_DELETE );
  प्रतिज्ञा( op>0 && op<0xff );
  यदि( is_अस्थायी ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    यदि( pName2->n>0 ){
      sqlite3_Error_सन्देशः(pParse, "temporary trigger may not have qualified name");
      गच्छ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }अन्यथा{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    यदि( iDb<0 ){
      गच्छ trigger_cleanup;
    }
  }
  यदि( !p_सारणी_Name || db->mallocFailed ){
    गच्छ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  यदि( db->init.busy && iDb!=1 ){
    प्रतिज्ञा( p_सारणी_Name->a[0].fg.fixedSchema==0 );
    प्रतिज्ञा( p_सारणी_Name->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, p_सारणी_Name->a[0].u4.zDatabase);
    p_सारणी_Name->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, p_सारणी_Name);
  यदि( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  यदि( db->mallocFailed ) गच्छ trigger_cleanup;
  प्रतिज्ञा( p_सारणी_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  यदि( sqlite3FixSrcList(&sFix, p_सारणी_Name) ){
    गच्छ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_सारणी_Name);
  यदि( !pTab ){
    /* The table does not exist. */
    गच्छ trigger_orphan_error;
  }
  यदि( IsVirtual(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "cannot create triggers on virtual tables");
    गच्छ trigger_orphan_error;
  }
  यदि( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_सन्देशः(pParse, "cannot create triggers on shadow tables");
    गच्छ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  यदि( zName==0 ){
    प्रतिज्ञा( db->mallocFailed );
    गच्छ trigger_cleanup;
  }
  यदि( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    गच्छ trigger_cleanup;
  }
  प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
  यदि( !अन्तर्_RENAME_OBJECT ){
    यदि( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      यदि( !noErr ){
        sqlite3_Error_सन्देशः(pParse, "trigger %T already exists", pName);
      }अन्यथा यदि( db->init.busy ){
        sqlite3_Error_सन्देशः(pParse,""); /* Err msg generated by corruptSchema() */
      }अन्यथा{
        sqlite3_संहिता_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      गच्छ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#यदि !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  यदि( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_सन्देशः(pParse, "cannot create trigger on system table");
    गच्छ trigger_cleanup;
  }
#यदि_अन्तः

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  यदि( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_सन्देशः(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_सारणी_Name->a);
    गच्छ trigger_orphan_error;
  }
  यदि( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_सन्देशः(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_सारणी_Name->a);
    गच्छ trigger_orphan_error;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
  यदि( !अन्तर्_RENAME_OBJECT ){
    पूर्णाङ्क iTabDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
    पूर्णाङ्क संहिता = SQLITE_CREATE_TRIGGER;
    ध्रुव वर्ण *zDb = db->aDb[iTabDb].zDbSName;
    ध्रुव वर्ण *zDbTrig = is_अस्थायी ? db->aDb[1].zDbSName : zDb;
    यदि( iTabDb==1 || is_अस्थायी ) संहिता = SQLITE_CREATE_अस्थायी_TRIGGER;
    यदि( sqlite3AuthCheck(pParse, संहिता, zName, pTab->zName, zDbTrig) ){
      गच्छ trigger_cleanup;
    }
    यदि( sqlite3AuthCheck(pParse, SQLITE_निवेशय, SCHEMA_सारणी(iTabDb),0,zDb)){
      गच्छ trigger_cleanup;
    }
  }
#यदि_अन्तः

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  यदि (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, परिमाण(Trigger));
  यदि( pTrigger==0 ) गच्छ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->सारणी_2 = sqlite3DbStrDup(db, p_सारणी_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  यदि( अन्तर्_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->सारणी_2, p_सारणी_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }अन्यथा{
    pTrigger->pWhen = sqlite3_व्यञ्जकम्_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  प्रतिज्ञा( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_सारणी_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_व्यञ्जकम्_Delete(db, pWhen);
  यदि( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }अन्यथा{
    प्रतिज्ञा( pParse->pNewTrigger==pTrigger );
  }
  प्रत्यागम;

trigger_orphan_error:
  यदि( db->init.iDb==1 ){
    /* Ticket #3810.
    ** Normally, whenever a table is dropped, all associated triggers are
    ** dropped too.  But if a TEMP trigger is created on a non-TEMP table
    ** and the table is dropped by a different database connection, the
    ** trigger is not visible to the database connection that does the
    ** drop so the trigger cannot be dropped.  This results in an
    ** "orphaned trigger" - a trigger whose associated table is missing.
    **
    ** 2020-11-05 see also https://sqlite.org/forum/forumpost/157dc791df
    */
    db->init.orphanTrigger = 1;
  }
  गच्छ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
शून्य sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  Trigger_चरणः *p_चरणः_List, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  वर्ण *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  पूर्णाङ्क iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  यदि( NEVER(pParse->nErr) || !pTrig ) गच्छ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTrig->pSchema);
  प्रतिज्ञा( iDb>=00 && iDb<db->nDb );
  pTrig->चरणः_list = p_चरणः_List;
  प्रति(n=0; p_चरणः_List; n++){
    p_चरणः_List->pTrig = pTrig;
    p_चरणः_List = p_चरणः_List->p_अग्र;
  }
  यदि( n>pParse->db->a_सीमा[SQLITE_सीमा_TRIGGER_चरणाः] ){
    sqlite3_Error_सन्देशः(pParse, "trigger \"%w\" contains too many steps", zName);
    गच्छ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  यदि( sqlite3_Fix_Trigger_चरणः(&sFix, pTrig->चरणः_list) 
   || sqlite3_Fix_व्यञ्जकम्(&sFix, pTrig->pWhen) 
  ){
    गच्छ triggerfinish_cleanup;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_ALTERTABLE
  यदि( अन्तर्_RENAME_OBJECT ){
    प्रतिज्ञा( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }अन्यथा
#यदि_अन्तः

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  यदि( !db->init.busy ){
    Vdbe *v;
    वर्ण *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    यदि( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_चरणः *p_चरणः;
      प्रति(p_चरणः=pTrig->चरणः_list; p_चरणः; p_चरणः=p_चरणः->p_अग्र){
        यदि( p_चरणः->pSrc!=0
         && sqlite3_Shadow_सारणी_Name(db, p_चरणः->pSrc->a[0].zName)
        ){
          sqlite3_Error_सन्देशः(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_चरणः->pSrc->a[0].zName);
          गच्छ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    यदि( v==0 ) गच्छ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (वर्ण*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_सारणी
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->सारणी_2, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_सङ्कलय_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  यदि( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    प्रतिज्ञा( pLink!=0 );
    pTrig = sqlite3_Hash_निवेशय(pHash, zName, pTrig);
    यदि( pTrig ){
      sqlite3OomFault(db);
    }अन्यथा यदि( pLink->pSchema==pLink->pTabSchema ){
      सारणी *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->सारणी_2);
      प्रतिज्ञा( pTab!=0 );
      pLink->p_अग्र = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  प्रतिज्ञा( अन्तर्_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_चरणः(db, p_चरणः_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
स्थिर वर्ण *trigger_विस्तृतिः_Dup(sqlite3 *db, ध्रुव वर्ण *zStart, ध्रुव वर्ण *z_अन्तः){
  वर्ण *z = sqlite3_Db_विस्तृतिः_Dup(db, zStart, z_अन्तः);
  पूर्णाङ्क i;
  यदि( z ) प्रति(i=0; z[i]; i++) यदि( sqlite3Isspace(z[i]) ) z[i] = ' ';
  प्रत्यागम z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_चरणः *sqlite3_Trigger_Select_चरणः(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ध्रुव वर्ण *zStart,         /* Start of SQL text */
  ध्रुव वर्ण *z_अन्तः            /* End of SQL text */
){
  Trigger_चरणः *p_Trigger_चरणः = sqlite3DbMallocZero(db, परिमाण(Trigger_चरणः));
  यदि( p_Trigger_चरणः==0 ) {
    sqlite3SelectDelete(db, pSelect);
    प्रत्यागम 0;
  }
  p_Trigger_चरणः->op = TK_SELECT;
  p_Trigger_चरणः->pSelect = pSelect;
  p_Trigger_चरणः->orconf = OE_Default;
  p_Trigger_चरणः->z_विस्तृतिः = trigger_विस्तृतिः_Dup(db, zStart, z_अन्तः);
  प्रत्यागम p_Trigger_चरणः;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
स्थिर Trigger_चरणः *trigger_चरणः_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ध्रुव वर्ण *zStart,         /* Start of SQL text */
  ध्रुव वर्ण *z_अन्तः            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_चरणः *p_Trigger_चरणः = 0;

  यदि( pParse->nErr==0 ){
    यदि( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_सन्देशः(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }अन्यथा{
      p_Trigger_चरणः = sqlite3DbMallocZero(db, परिमाण(Trigger_चरणः));
      यदि( p_Trigger_चरणः ){
        p_Trigger_चरणः->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_चरणः->op = op;
        p_Trigger_चरणः->z_विस्तृतिः = trigger_विस्तृतिः_Dup(db, zStart, z_अन्तः);
        यदि( p_Trigger_चरणः->pSrc && अन्तर्_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_चरणः->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  प्रत्यागम p_Trigger_चरणः;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_चरणः *sqlite3_Trigger_निवेशय_चरणः(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ध्रुव वर्ण *zStart, /* Start of SQL text */
  ध्रुव वर्ण *z_अन्तः    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_चरणः *p_Trigger_चरणः;

  प्रतिज्ञा(pSelect != 0 || db->mallocFailed);

  p_Trigger_चरणः = trigger_चरणः_Allocate(pParse, TK_निवेशय, pTabList, zStart, z_अन्तः);
  यदि( p_Trigger_चरणः ){
    यदि( अन्तर्_RENAME_OBJECT ){
      p_Trigger_चरणः->pSelect = pSelect;
      pSelect = 0;
    }अन्यथा{
      p_Trigger_चरणः->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_चरणः->pIdList = pColumn;
    p_Trigger_चरणः->pUpsert = pUpsert;
    p_Trigger_चरणः->orconf = orconf;
    यदि( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }अन्यथा{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  प्रत्यागम p_Trigger_चरणः;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_चरणः *sqlite3_Trigger_Update_चरणः(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  व्यञ्जकम्_List *pEList,    /* The SET clause: list of column and new values */
  व्यञ्जकम् *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ध्रुव वर्ण *zStart,  /* Start of SQL text */
  ध्रुव वर्ण *z_अन्तः     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_चरणः *p_Trigger_चरणः;

  p_Trigger_चरणः = trigger_चरणः_Allocate(pParse, TK_UPDATE, pTabList, zStart, z_अन्तः);
  यदि( p_Trigger_चरणः ){
    SrcList *pFromDup = 0;
    यदि( अन्तर्_RENAME_OBJECT ){
      p_Trigger_चरणः->p_व्यञ्जकम्_List = pEList;
      p_Trigger_चरणः->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }अन्यथा{
      p_Trigger_चरणः->p_व्यञ्जकम्_List = sqlite3_व्यञ्जकम्_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_चरणः->pWhere = sqlite3_व्यञ्जकम्_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_चरणः->orconf = orconf;

    यदि( pFromDup && !अन्तर्_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_पदांशः(pParse, 0, 0, 0, &as, pSub ,0);
    }
    यदि( pFromDup && p_Trigger_चरणः->pSrc ){
      p_Trigger_चरणः->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_चरणः->pSrc, pFromDup
      );
    }अन्यथा{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_व्यञ्जकम्_List_Delete(db, pEList);
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  प्रत्यागम p_Trigger_चरणः;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_चरणः *sqlite3_Trigger_Delete_चरणः(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  व्यञ्जकम् *pWhere,           /* The WHERE clause */
  ध्रुव वर्ण *zStart,     /* Start of SQL text */
  ध्रुव वर्ण *z_अन्तः        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_चरणः *p_Trigger_चरणः;

  p_Trigger_चरणः = trigger_चरणः_Allocate(pParse, TK_DELETE, pTabList, zStart, z_अन्तः);
  यदि( p_Trigger_चरणः ){
    यदि( अन्तर्_RENAME_OBJECT ){
      p_Trigger_चरणः->pWhere = pWhere;
      pWhere = 0;
    }अन्यथा{
      p_Trigger_चरणः->pWhere = sqlite3_व्यञ्जकम्_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_चरणः->orconf = OE_Default;
  }
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
  प्रत्यागम p_Trigger_चरणः;
}

/* 
** Recursively delete a Trigger structure
*/
शून्य sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  यदि( pTrigger==0 || pTrigger->bReturning ) प्रत्यागम;
  sqlite3_Delete_Trigger_चरणः(db, pTrigger->चरणः_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->सारणी_2);
  sqlite3_व्यञ्जकम्_Delete(db, pTrigger->pWhen);
  sqlite3IdListDelete(db, pTrigger->pColumns);
  sqlite3DbFree(db, pTrigger);
}

/*
** This function is called to drop a trigger from the database schema. 
**
** This may be called directly from the parser and therefore identifies
** the trigger by name.  The sqlite3DropTriggerPtr() routine does the
** same job as this routine except it takes a pointer to the trigger
** instead of the trigger name.
**/
शून्य sqlite3DropTrigger(Parse *pParse, SrcList *pName, पूर्णाङ्क noErr){
  Trigger *pTrigger = 0;
  पूर्णाङ्क i;
  ध्रुव वर्ण *zDb;
  ध्रुव वर्ण *zName;
  sqlite3 *db = pParse->db;

  यदि( db->mallocFailed ) गच्छ drop_trigger_cleanup;
  यदि( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    गच्छ drop_trigger_cleanup;
  }

  प्रतिज्ञा( pName->nSrc==1 );
  प्रतिज्ञा( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  प्रतिज्ञा( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  प्रति(i=OMIT_TEMPDB; i<db->nDb; i++){
    पूर्णाङ्क j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    यदि( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) अनुवर्त;
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    यदि( pTrigger ) भङ्ग;
  }
  यदि( !pTrigger ){
    यदि( !noErr ){
      sqlite3_Error_सन्देशः(pParse, "no such trigger: %S", pName->a);
    }अन्यथा{
      sqlite3_संहिता_Verify_Named_Schema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    गच्छ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
स्थिर सारणी *सारणी_सम्बन्धि_Trigger(Trigger *pTrigger){
  प्रत्यागम sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->सारणी_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
शून्य sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  सारणी   *p_सारणी;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क iDb;

  iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTrigger->pSchema);
  प्रतिज्ञा( iDb>=0 && iDb<db->nDb );
  p_सारणी = सारणी_सम्बन्धि_Trigger(pTrigger);
  प्रतिज्ञा( (p_सारणी && p_सारणी->pSchema==pTrigger->pSchema) || iDb==1 );
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
  यदि( p_सारणी ){
    पूर्णाङ्क संहिता = SQLITE_DROP_TRIGGER;
    ध्रुव वर्ण *zDb = db->aDb[iDb].zDbSName;
    ध्रुव वर्ण *zTab = SCHEMA_सारणी(iDb);
    यदि( iDb==1 ) संहिता = SQLITE_DROP_अस्थायी_TRIGGER;
    यदि( sqlite3AuthCheck(pParse, संहिता, pTrigger->zName, p_सारणी->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      प्रत्यागम;
    }
  }
#यदि_अन्तः

  /* Generate code to destroy the database record of the trigger.
  */
  यदि( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_सारणी " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
शून्य sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zName){
  Trigger *pTrigger;
  Hash *pHash;

  प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_निवेशय(pHash, zName, 0);
  यदि( ALWAYS(pTrigger) ){
    यदि( pTrigger->pSchema==pTrigger->pTabSchema ){
      सारणी *pTab = सारणी_सम्बन्धि_Trigger(pTrigger);
      यदि( pTab ){
        Trigger **pp;
        प्रति(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_अग्र)){
          यदि( *pp==pTrigger ){
            *pp = (*pp)->p_अग्र;
            भङ्ग;
          }
        }
      }
    }
    sqlite3DeleteTrigger(db, pTrigger);
    db->mDbFlags |= DBFLAG_SchemaChange;
  }
}

/*
** pEList is the SET clause of an UPDATE statement.  Each entry
** in pEList is of the format <id>=<expr>.  If any of the entries
** in pEList have an <id> which matches an identifier in pIdList,
** then return TRUE.  If pIdList==NULL, then it is considered a
** wildcard that matches anything.  Likewise if pEList==NULL then
** it matches anything so always return true.  Return false only
** if there is no match.
*/
स्थिर पूर्णाङ्क checkColumnOverlap(IdList *pIdList, व्यञ्जकम्_List *pEList){
  पूर्णाङ्क e;
  यदि( pIdList==0 || NEVER(pEList==0) ) प्रत्यागम 1;
  प्रति(e=0; e<pEList->n_व्यञ्जकम्; e++){
    यदि( sqlite3_Id_List_सूचकांक(pIdList, pEList->a[e].zEName)>=0 ) प्रत्यागम 1;
  }
  प्रत्यागम 0; 
}

/*
** Return true if any TEMP triggers exist
*/
स्थिर पूर्णाङ्क अस्थायी_Triggers_Exist(sqlite3 *db){
  यदि( NEVER(db->aDb[1].pSchema==0) ) प्रत्यागम 0;
  यदि( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) प्रत्यागम 0;
  प्रत्यागम 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
स्थिर SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  सारणी *pTab,            /* The table the contains the triggers */
  पूर्णाङ्क op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यञ्जकम्_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णाङ्क *p_आवरणम्              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  पूर्णाङ्क आवरणम् = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  प्रतिज्ञा( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_अग्र==0) );
  यदि( pList!=0 ){
    p = pList;
    यदि( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      यदि( pList==pTab->pTrigger ){
        pList = 0;
        गच्छ exit_triggers_exist;
      }
      यावत्( ALWAYS(p->p_अग्र) && p->p_अग्र!=pTab->pTrigger ) p = p->p_अग्र;
      p->p_अग्र = 0;
      p = pList;
    }
    कुरु{
      यदि( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        आवरणम् |= p->tr_tm;
      }अन्यथा यदि( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        प्रतिज्ञा( sqlite3IsToplevel(pParse) );
        p->op = op;
        यदि( IsVirtual(pTab) ){
          यदि( op!=TK_निवेशय ){
            sqlite3_Error_सन्देशः(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }अन्यथा{
          p->tr_tm = TRIGGER_AFTER;
        }
        आवरणम् |= p->tr_tm;
      }अन्यथा यदि( p->bReturning && p->op==TK_निवेशय && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        आवरणम् |= p->tr_tm;
      }
      p = p->p_अग्र;
    }यावत्( p );
  }
exit_triggers_exist:
  यदि( p_आवरणम् ){
    *p_आवरणम् = आवरणम्;
  }
  प्रत्यागम (आवरणम् ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  सारणी *pTab,            /* The table the contains the triggers */
  पूर्णाङ्क op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यञ्जकम्_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णाङ्क *p_आवरणम्              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  प्रतिज्ञा( pTab!=0 );
  यदि( (pTab->pTrigger==0 && !अस्थायी_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    यदि( p_आवरणम् ) *p_आवरणम् = 0;
    प्रत्यागम 0;
  }
  प्रत्यागम triggersReallyExist(pParse,pTab,op,pChanges,p_आवरणम्);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
स्थिर पूर्णाङ्क is_Asterisk_पदांशः(
  Parse *pParse,      /* Parsing context */
  व्यञ्जकम् *p_पदांशः         /* A term in the RETURNING clause */
){
  प्रतिज्ञा( p_पदांशः!=0 );
  यदि( p_पदांशः->op==TK_ASTERISK ) प्रत्यागम 1;
  यदि( p_पदांशः->op!=TK_DOT ) प्रत्यागम 0;
  प्रतिज्ञा( p_पदांशः->p_दक्षिणः!=0 );
  प्रतिज्ञा( p_पदांशः->p_वामः!=0 );
  यदि( p_पदांशः->p_दक्षिणः->op!=TK_ASTERISK ) प्रत्यागम 0;
  sqlite3_Error_सन्देशः(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  प्रत्यागम 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
स्थिर व्यञ्जकम्_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  व्यञ्जकम्_List *pList,      /* The arguments to RETURNING */
  सारणी *pTab           /* The table being updated */
){
  व्यञ्जकम्_List *pNew = 0;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क i;

  प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p_Old_व्यञ्जकम् = pList->a[i].p_व्यञ्जकम्;
    यदि( NEVER(p_Old_व्यञ्जकम्==0) ) अनुवर्त;
    यदि( is_Asterisk_पदांशः(pParse, p_Old_व्यञ्जकम्) ){
      पूर्णाङ्क jj;
      प्रति(jj=0; jj<pTab->nCol; jj++){
        व्यञ्जकम् *p_New_व्यञ्जकम्;
        यदि( IsHiddenColumn(pTab->aCol+jj) ) अनुवर्त;
        p_New_व्यञ्जकम् = sqlite3_व्यञ्जकम्(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_New_व्यञ्जकम्);
        यदि( !db->mallocFailed ){
          संरचना व्यञ्जकम्_List_item *pItem = &pNew->a[pNew->n_व्यञ्जकम्-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }अन्यथा{
      व्यञ्जकम् *p_New_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_Old_व्यञ्जकम्, 0);
      pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_New_व्यञ्जकम्);
      यदि( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        संरचना व्यञ्जकम्_List_item *pItem = &pNew->a[pNew->n_व्यञ्जकम्-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  प्रत्यागम pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
स्थिर पूर्णाङ्क sqlite3ReturningSubqueryVarSelect(Walker *Not_प्रयुक्तम्, व्यञ्जकम् *p_व्यञ्जकम्){
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  यदि( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्)
   && (p_व्यञ्जकम्->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_VarSelect) );
    व्यञ्जकम्_समुच्चयः_Property(p_व्यञ्जकम्, EP_VarSelect);
  }
  प्रत्यागम WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
स्थिर पूर्णाङ्क sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  पूर्णाङ्क i;
  SrcList *pSrc;
  प्रतिज्ञा( pSelect!=0 );
  pSrc = pSelect->pSrc;
  प्रतिज्ञा( pSrc!=0 );
  प्रति(i=0; i<pSrc->nSrc; i++){
    यदि( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_संहिता = 1;
      भङ्ग;
    }
  }
  प्रत्यागम WRC_Continue;
}

/*
** Scan the expression list that is the argument to RETURNING looking
** for subqueries that depend on the table which is being modified in the
** statement that is hosting the RETURNING clause (pTab).  Mark all such
** subqueries as SF_Correlated.  If the subqueries are part of an
** expression, mark the expression as EP_VarSelect.
**
** https://sqlite.org/forum/forumpost/2c83569ce8945d39
*/
स्थिर शून्य sqlite3ProcessReturningSubqueries(
  व्यञ्जकम्_List *pEList,
  सारणी *pTab
){
  Walker w;
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.x_व्यञ्जकम्_Callback = sqlite3_व्यञ्जकम्_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_व्यञ्जकम्_List(&w, pEList);
  यदि( w.e_संहिता ){
    w.x_व्यञ्जकम्_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_व्यञ्जकम्_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
स्थिर शून्य संहिता_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  सारणी *pTab,         /* The table to code triggers from */
  पूर्णाङ्क reg_अन्तर्            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  व्यञ्जकम्_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  सङ्घ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  प्रतिज्ञा( v!=0 );
  यदि( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    प्रत्यागम;
  }
  प्रतिज्ञा( db->pParse==pParse );
  प्रतिज्ञा( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  यदि( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    प्रत्यागम;
  }
  स्मृतिं_पूरय(&sSelect, 0, परिमाण(sSelect));
  स्मृतिं_पूरय(&uSrc, 0, परिमाण(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_व्यञ्जकम्_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  यदि( pParse->nErr==0 ){
    प्रतिज्ञा( db->mallocFailed==0 );
    sqlite3_Generate_Column_नामानि(pParse, &sSelect);
  }
  sqlite3_व्यञ्जकम्_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  यदि( pParse->nErr==0 ){
    NameContext sNC;
    स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
    यदि( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_व्यञ्जकम्;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_आधारः_Reg = reg_अन्तर्;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    यदि( sqlite3_Resolve_व्यञ्जकम्_List_नामानि(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      पूर्णाङ्क i;
      पूर्णाङ्क nCol = pNew->n_व्यञ्जकम्;
      पूर्णाङ्क reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      प्रति(i=0; i<nCol; i++){
        व्यञ्जकम् *pCol = pNew->a[i].p_व्यञ्जकम्;
        प्रतिज्ञा( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_व्यञ्जकम्_संहिता_Factorable(pParse, pCol, reg+i);
        यदि( sqlite3_व्यञ्जकम्_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_सङ्कलय_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg, i, reg+i);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_व्यञ्जकम्_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
स्थिर पूर्णाङ्क संहिता_Trigger_क्रमादेशः(
  Parse *pParse,            /* The parser context */
  Trigger_चरणः *p_चरणः_List,   /* List of statements inside the trigger body */
  पूर्णाङ्क orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_चरणः *p_चरणः;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  प्रतिज्ञा( pParse->pTriggerTab && pParse->pToplevel );
  प्रतिज्ञा( p_चरणः_List );
  प्रतिज्ञा( v!=0 );
  प्रति(p_चरणः=p_चरणः_List; p_चरणः; p_चरणः=p_चरणः->p_अग्र){
    /* Figure out the ON CONFLICT policy that will be used for this step
    ** of the trigger program. If the statement that caused this trigger
    ** to fire had an explicit ON CONFLICT, then use it. Otherwise, use
    ** the ON CONFLICT policy that was specified as part of the trigger
    ** step statement. Example:
    **
    **   CREATE TRIGGER AFTER INSERT ON t1 BEGIN;
    **     INSERT OR REPLACE INTO t2 VALUES(new.a, new.b);
    **   END;
    **
    **   INSERT INTO t1 ... ;            -- insert into t2 uses REPLACE policy
    **   INSERT OR IGNORE INTO t1 ... ;  -- insert into t2 uses IGNORE policy
    */
    pParse->eOrconf = (orconf==OE_Default)?p_चरणः->orconf:(u8)orconf;
    प्रतिज्ञा( pParse->ok_Const_गुणकः==0 );

#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
    यदि( p_चरणः->z_विस्तृतिः ){
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_चरणः->z_विस्तृतिः),
                        P4_DYNAMIC);
    }
#यदि_अन्तः

    विकल्प( p_चरणः->op ){
      पक्ष TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_चरणः->pSrc, 0),
          sqlite3_व्यञ्जकम्_List_Dup(db, p_चरणः->p_व्यञ्जकम्_List, 0), 
          sqlite3_व्यञ्जकम्_Dup(db, p_चरणः->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      पक्ष TK_निवेशय: {
        sqlite3_निवेशय(pParse, 
          sqlite3SrcListDup(db, p_चरणः->pSrc, 0),
          sqlite3SelectDup(db, p_चरणः->pSelect, 0), 
          sqlite3IdListDup(db, p_चरणः->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_चरणः->pUpsert)
        );
        sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      पक्ष TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_चरणः->pSrc, 0),
          sqlite3_व्यञ्जकम्_Dup(db, p_चरणः->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      स्वभाव: प्रतिज्ञा( p_चरणः->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_चरणः->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        भङ्ग;
      }
    } 
  }

  प्रत्यागम 0;
}

#यदि_परिभाषितम् SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
स्थिर ध्रुव वर्ण *onErrorText(पूर्णाङ्क onError){
  विकल्प( onError ){
    पक्ष OE_Abort:    प्रत्यागम "abort";
    पक्ष OE_Rollback: प्रत्यागम "rollback";
    पक्ष OE_Fail:     प्रत्यागम "fail";
    पक्ष OE_Replace:  प्रत्यागम "replace";
    पक्ष OE_Ignore:   प्रत्यागम "ignore";
    पक्ष OE_Default:  प्रत्यागम "default";
  }
  प्रत्यागम "n/a";
}
#यदि_अन्तः

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
स्थिर शून्य transferParseError(Parse *p_गन्तव्यम्, Parse *pFrom){
  प्रतिज्ञा( pFrom->z_Err_सन्देशः==0 || pFrom->nErr );
  प्रतिज्ञा( p_गन्तव्यम्->z_Err_सन्देशः==0 || p_गन्तव्यम्->nErr );
  यदि( p_गन्तव्यम्->nErr==0 ){
    p_गन्तव्यम्->z_Err_सन्देशः = pFrom->z_Err_सन्देशः;
    p_गन्तव्यम्->nErr = pFrom->nErr;
    p_गन्तव्यम्->rc = pFrom->rc;
  }अन्यथा{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_सन्देशः);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
स्थिर TriggerPrg *संहिता_Row_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  सारणी *pTab,         /* The table pTrigger is attached to */
  पूर्णाङ्क orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_शिखरम्;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  व्यञ्जकम् *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_क्रमादेशः *p_क्रमादेशः = 0;   /* Sub-vdbe for trigger program */
  पूर्णाङ्क i_अन्तः_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  पूर्णाङ्क n_गभीरता;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_शिखरम् = pParse;
  प्रति(n_गभीरता=0; p_शिखरम्->pOuterParse; p_शिखरम् = p_शिखरम्->pOuterParse, n_गभीरता++){}
  यदि( n_गभीरता>=db->a_सीमा[SQLITE_सीमा_TRIGGER_गभीरता] ){
    sqlite3_Error_सन्देशः(pParse, "triggers nested too deep");
    प्रत्यागम 0;
  }

  p_शिखरम् = sqlite3ParseToplevel(pParse);
  प्रतिज्ञा( pTrigger->zName==0 || pTab==सारणी_सम्बन्धि_Trigger(pTrigger) );
  प्रतिज्ञा( p_शिखरम्->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, परिमाण(TriggerPrg));
  यदि( !pPrg ) प्रत्यागम 0;
  pPrg->p_अग्र = p_शिखरम्->pTriggerPrg;
  p_शिखरम्->pTriggerPrg = pPrg;
  pPrg->p_क्रमादेशः = p_क्रमादेशः = sqlite3DbMallocZero(db, परिमाण(Sub_क्रमादेशः));
  यदि( !p_क्रमादेशः ) प्रत्यागम 0;
  sqlite3_Vdbe_Link_Sub_क्रमादेशः(p_शिखरम्->pVdbe, p_क्रमादेशः);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_शिखरम्;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  यदि( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_निवेशय ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
    यदि( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#यदि_अन्तः

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    यदि( pTrigger->pWhen ){
      pWhen = sqlite3_व्यञ्जकम्_Dup(db, pTrigger->pWhen, 0);
      यदि( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_व्यञ्जकम्_नामानि(&sNC, pWhen) 
      ){
        i_अन्तः_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_व्यञ्जकम्_If_False(&sSubParse, pWhen, i_अन्तः_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_व्यञ्जकम्_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    संहिता_Trigger_क्रमादेशः(&sSubParse, pTrigger->चरणः_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    यदि( i_अन्तः_Trigger ){
      sqlite3VdbeResolveLabel(v, i_अन्तः_Trigger);
    }
    sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    यदि( pParse->nErr==0 ){
      प्रतिज्ञा( db->mallocFailed==0 );
      p_क्रमादेशः->aOp = sqlite3VdbeTakeOpArray(v, &p_क्रमादेशः->nOp, &p_शिखरम्->nMaxArg);
    }
    p_क्रमादेशः->nMem = sSubParse.nMem;
    p_क्रमादेशः->nCsr = sSubParse.nTab;
    p_क्रमादेशः->token = (शून्य *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }अन्यथा{
    transferParseError(pParse, &sSubParse);
  }

  प्रतिज्ञा( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  प्रत्यागम pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
स्थिर TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  सारणी *pTab,         /* The table trigger pTrigger is attached to */
  पूर्णाङ्क orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_मूलम् = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  प्रतिज्ञा( pTrigger->zName==0 || pTab==सारणी_सम्बन्धि_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  प्रति(pPrg=p_मूलम्->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_अग्र
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  यदि( !pPrg ){
    pPrg = संहिता_Row_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_विस्थापनम् = -1;
  }

  प्रत्यागम pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
शून्य sqlite3_संहिता_Row_Trigger_प्रत्यक्षम्(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  सारणी *pTab,         /* The table to code triggers from */
  पूर्णाङ्क reg,             /* Reg array containing OLD.* and NEW.* values */
  पूर्णाङ्क orconf,          /* ON CONFLICT policy */
  पूर्णाङ्क ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  प्रतिज्ञा( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  यदि( pPrg ){
    पूर्णाङ्क bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_क्रमादेशः, reg, ignoreJump, ++pParse->nMem,
                      (ध्रुव वर्ण *)pPrg->p_क्रमादेशः, P4_SUBPROGRAM);
    VdbeComment(
        (v, "Call: %s.%s", (p->zName?p->zName:"fkey"), onErrorText(orconf)));

    /* Set the P5 operand of the OP_Program instruction to non-zero if
    ** recursive invocation of this trigger program is disallowed. Recursive
    ** invocation is disallowed if (a) the sub-program is really a trigger,
    ** not a foreign key action, and (b) the flag to enable recursive triggers
    ** is clear.  */
    sqlite3VdbeChangeP5(v, (u16)bRecursive);
  }
}

/*
** This is called to code the required FOR EACH ROW triggers for an operation
** on table pTab. The operation to code triggers for (INSERT, UPDATE or DELETE)
** is given by the op parameter. The tr_tm parameter determines whether the
** BEFORE or AFTER triggers are coded. If the operation is an UPDATE, then
** parameter pChanges is passed the list of columns being modified.
**
** If there are no triggers that fire at the specified time for the specified
** operation on pTab, this function is a no-op.
**
** The reg argument is the address of the first in an array of registers 
** that contain the values substituted for the new.* and old.* references
** in the trigger program. If N is the number of columns in table pTab
** (a copy of pTab->nCol), then registers are populated as follows:
**
**   Register       Contains
**   ------------------------------------------------------
**   reg+0          OLD.rowid
**   reg+1          OLD.* value of left-most column of pTab
**   ...            ...
**   reg+N          OLD.* value of right-most column of pTab
**   reg+N+1        NEW.rowid
**   reg+N+2        NEW.* value of left-most column of pTab
**   ...            ...
**   reg+N+N+1      NEW.* value of right-most column of pTab
**
** For ON DELETE triggers, the registers containing the NEW.* values will
** never be accessed by the trigger program, so they are not allocated or 
** populated by the caller (there is no data to populate them with anyway). 
** Similarly, for ON INSERT triggers the values stored in the OLD.* registers
** are never accessed, and so are not allocated by the caller. So, for an
** ON INSERT trigger, the value passed to this function as parameter reg
** is not a readable register, although registers (reg+N) through 
** (reg+N+N+1) are.
**
** Parameter orconf is the default conflict resolution algorithm for the
** trigger program to use (REPLACE, IGNORE etc.). Parameter ignoreJump
** is the instruction that control should jump to if a trigger program
** raises an IGNORE exception.
*/
शून्य sqlite3_संहिता_Row_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  पूर्णाङ्क op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  व्यञ्जकम्_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णाङ्क tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  सारणी *pTab,         /* The table to code triggers from */
  पूर्णाङ्क reg,             /* The first in an array of registers (see above) */
  पूर्णाङ्क orconf,          /* ON CONFLICT policy */
  पूर्णाङ्क ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  प्रतिज्ञा( op==TK_UPDATE || op==TK_निवेशय || op==TK_DELETE );
  प्रतिज्ञा( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  प्रतिज्ञा( (op==TK_UPDATE)==(pChanges!=0) );

  प्रति(p=pTrigger; p; p=p->p_अग्र){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    प्रतिज्ञा( p->pSchema!=0 );
    प्रतिज्ञा( p->pTabSchema!=0 );
    प्रतिज्ञा( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    यदि( (p->op==op || (p->bReturning && p->op==TK_निवेशय && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      यदि( !p->bReturning ){
        sqlite3_संहिता_Row_Trigger_प्रत्यक्षम्(pParse, p, pTab, reg, orconf, ignoreJump);
      }अन्यथा यदि( sqlite3IsToplevel(pParse) ){
        संहिता_Returning_Trigger(pParse, p, pTab, reg);
      }
    }
  }
}

/*
** Triggers may access values stored in the old.* or new.* pseudo-table. 
** This function returns a 32-bit bitmask indicating which columns of the 
** old.* or new.* tables actually are used by triggers. This information 
** may be used by the caller, for example, to avoid having to load the entire
** old.* record into memory when executing an UPDATE or DELETE command.
**
** Bit 0 of the returned mask is set if the left-most column of the
** table may be accessed using an [old|new].<col> reference. Bit 1 is set if
** the second leftmost column value is required, and so on. If there
** are more than 32 columns in the table, and at least one of the columns
** with an index greater than 32 may be accessed, 0xffffffff is returned.
**
** It is not possible to determine if the old.rowid or new.rowid column is 
** accessed by triggers. The caller must always assume that it is.
**
** Parameter isNew must be either 1 or 0. If it is 0, then the mask returned
** applies to the old.* table. If 1, the new.* table.
**
** Parameter tr_tm must be a mask with one or both of the TRIGGER_BEFORE
** and TRIGGER_AFTER bits set. Values accessed by BEFORE triggers are only
** included in the returned mask if the TRIGGER_BEFORE bit is set in the
** tr_tm parameter. Similarly, values accessed by AFTER triggers are only
** included in the returned mask if the TRIGGER_AFTER bit is set in tr_tm.
*/
u32 sqlite3TriggerColmask(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  व्यञ्जकम्_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णाङ्क isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  पूर्णाङ्क tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  सारणी *pTab,         /* The table to code triggers from */
  पूर्णाङ्क orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ध्रुव पूर्णाङ्क op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 आवरणम् = 0;
  Trigger *p;

  प्रतिज्ञा( isNew==1 || isNew==0 );
  यदि( IsView(pTab) ){
    प्रत्यागम 0xffffffff;
  }
  प्रति(p=pTrigger; p; p=p->p_अग्र){
    यदि( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      यदि( p->bReturning ){
        आवरणम् = 0xffffffff;
      }अन्यथा{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        यदि( pPrg ){
          आवरणम् |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  प्रत्यागम आवरणम्;
}

#यदि_अन्तः /* !defined(SQLITE_OMIT_TRIGGER) */
