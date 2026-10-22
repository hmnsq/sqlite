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
#অন্তর্ভাৱ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
শূন্য sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  যেতিয়ালৈ( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_ৰাশিমালা_Delete(db, pTmp->pWhere);
    sqlite3_ৰাশিমালা_List_Delete(db, pTmp->p_ৰাশিমালা_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->zSpan);

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
Trigger *sqlite3TriggerList(Parse *pParse, Table *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  প্রতিজ্ঞা( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  যেতিয়ালৈ( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    যদি( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }অন্যথা যদি( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      প্রতিজ্ঞা( pParse->db->pVtabCtx==0 );
#endif
      প্রতিজ্ঞা( pParse->bReturning );
      প্রতিজ্ঞা( !pParse->isCreate );
      প্রতিজ্ঞা( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#যদি 0
  যদি( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    বাবে(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  ঘূৰাই pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
শূন্য sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  পূর্ণাঙ্ক tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  পূর্ণাঙ্ক op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  ৰাশিমালা *pWhen,        /* WHEN clause */
  পূর্ণাঙ্ক isTemp,         /* True if the TEMPORARY keyword is present */
  পূর্ণাঙ্ক noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  ৱর্ণ *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  পূর্ণাঙ্ক iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  প্রতিজ্ঞা( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  প্রতিজ্ঞা( pName2!=0 );
  প্রতিজ্ঞা( op==TK_ঢোকাও || op==TK_UPDATE || op==TK_DELETE );
  প্রতিজ্ঞা( op>0 && op<0xff );
  যদি( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    যদি( pName2->n>0 ){
      sqlite3_Error_বাৰ্তা(pParse, "temporary trigger may not have qualified name");
      গচ্ছ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }অন্যথা{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    যদি( iDb<0 ){
      গচ্ছ trigger_cleanup;
    }
  }
  যদি( !pTableName || db->mallocFailed ){
    গচ্ছ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  যদি( db->init.busy && iDb!=1 ){
    প্রতিজ্ঞা( pTableName->a[0].fg.fixedSchema==0 );
    প্রতিজ্ঞা( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  যদি( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  যদি( db->mallocFailed ) গচ্ছ trigger_cleanup;
  প্রতিজ্ঞা( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  যদি( sqlite3FixSrcList(&sFix, pTableName) ){
    গচ্ছ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  যদি( !pTab ){
    /* The table does not exist. */
    গচ্ছ trigger_orphan_error;
  }
  যদি( IsVirtual(pTab) ){
    sqlite3_Error_বাৰ্তা(pParse, "cannot create triggers on virtual tables");
    গচ্ছ trigger_orphan_error;
  }
  যদি( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_বাৰ্তা(pParse, "cannot create triggers on shadow tables");
    গচ্ছ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  যদি( zName==0 ){
    প্রতিজ্ঞা( db->mallocFailed );
    গচ্ছ trigger_cleanup;
  }
  যদি( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    গচ্ছ trigger_cleanup;
  }
  প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
  যদি( !IN_RENAME_OBJECT ){
    যদি( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      যদি( !noErr ){
        sqlite3_Error_বাৰ্তা(pParse, "trigger %T already exists", pName);
      }অন্যথা যদি( db->init.busy ){
        sqlite3_Error_বাৰ্তা(pParse,""); /* Err msg generated by corruptSchema() */
      }অন্যথা{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      গচ্ছ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#যদি !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  যদি( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_বাৰ্তা(pParse, "cannot create trigger on system table");
    গচ্ছ trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  যদি( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_বাৰ্তা(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    গচ্ছ trigger_orphan_error;
  }
  যদি( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_বাৰ্তা(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    গচ্ছ trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  যদি( !IN_RENAME_OBJECT ){
    পূর্ণাঙ্ক iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    পূর্ণাঙ্ক code = SQLITE_CREATE_TRIGGER;
    ধ্রুৱ ৱর্ণ *zDb = db->aDb[iTabDb].zDbSName;
    ধ্রুৱ ৱর্ণ *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    যদি( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    যদি( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      গচ্ছ trigger_cleanup;
    }
    যদি( sqlite3AuthCheck(pParse, SQLITE_ঢোকাও, SCHEMA_TABLE(iTabDb),0,zDb)){
      গচ্ছ trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  যদি (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, পরিমাণ(Trigger));
  যদি( pTrigger==0 ) গচ্ছ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  যদি( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }অন্যথা{
    pTrigger->pWhen = sqlite3_ৰাশিমালা_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  প্রতিজ্ঞা( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_ৰাশিমালা_Delete(db, pWhen);
  যদি( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }অন্যথা{
    প্রতিজ্ঞা( pParse->pNewTrigger==pTrigger );
  }
  ঘূৰাই;

trigger_orphan_error:
  যদি( db->init.iDb==1 ){
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
  গচ্ছ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
শূন্য sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  ৱর্ণ *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  পূর্ণাঙ্ক iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  যদি( NEVER(pParse->nErr) || !pTrig ) গচ্ছ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  প্রতিজ্ঞা( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  বাবে(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  যদি( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_বাৰ্তা(pParse, "trigger \"%w\" contains too many steps", zName);
    গচ্ছ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  যদি( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_ৰাশিমালা(&sFix, pTrig->pWhen) 
  ){
    গচ্ছ triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  যদি( IN_RENAME_OBJECT ){
    প্রতিজ্ঞা( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }অন্যথা
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  যদি( !db->init.busy ){
    Vdbe *v;
    ৱর্ণ *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    যদি( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      বাবে(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        যদি( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_বাৰ্তা(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          গচ্ছ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    যদি( v==0 ) গচ্ছ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (ৱর্ণ*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_TABLE
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->table, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  যদি( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
    প্রতিজ্ঞা( pLink!=0 );
    pTrig = sqlite3_Hash_ঢোকাও(pHash, zName, pTrig);
    যদি( pTrig ){
      sqlite3OomFault(db);
    }অন্যথা যদি( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      প্রতিজ্ঞা( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  প্রতিজ্ঞা( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
স্থির ৱর্ণ *triggerSpanDup(sqlite3 *db, ধ্রুৱ ৱর্ণ *zStart, ধ্রুৱ ৱর্ণ *zEnd){
  ৱর্ণ *z = sqlite3DbSpanDup(db, zStart, zEnd);
  পূর্ণাঙ্ক i;
  যদি( z ) বাবে(i=0; z[i]; i++) যদি( sqlite3Isspace(z[i]) ) z[i] = ' ';
  ঘূৰাই z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
TriggerStep *sqlite3TriggerSelectStep(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ধ্রুৱ ৱর্ণ *zStart,         /* Start of SQL text */
  ধ্রুৱ ৱর্ণ *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, পরিমাণ(TriggerStep));
  যদি( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    ঘূৰাই 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  ঘূৰাই pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
স্থির TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ধ্রুৱ ৱর্ণ *zStart,         /* Start of SQL text */
  ধ্রুৱ ৱর্ণ *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  যদি( pParse->nErr==0 ){
    যদি( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_বাৰ্তা(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }অন্যথা{
      pTriggerStep = sqlite3DbMallocZero(db, পরিমাণ(TriggerStep));
      যদি( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        যদি( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  ঘূৰাই pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_ঢোকাও_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ধ্রুৱ ৱর্ণ *zStart, /* Start of SQL text */
  ধ্রুৱ ৱর্ণ *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  প্রতিজ্ঞা(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_ঢোকাও, pTabList, zStart, zEnd);
  যদি( pTriggerStep ){
    যদি( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }অন্যথা{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    যদি( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }অন্যথা{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  ঘূৰাই pTriggerStep;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerUpdateStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  ৰাশিমালা_List *pEList,    /* The SET clause: list of column and new values */
  ৰাশিমালা *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ধ্রুৱ ৱর্ণ *zStart,  /* Start of SQL text */
  ধ্রুৱ ৱর্ণ *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  যদি( pTriggerStep ){
    SrcList *pFromDup = 0;
    যদি( IN_RENAME_OBJECT ){
      pTriggerStep->p_ৰাশিমালা_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }অন্যথা{
      pTriggerStep->p_ৰাশিমালা_List = sqlite3_ৰাশিমালা_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_ৰাশিমালা_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    যদি( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    যদি( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }অন্যথা{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_ৰাশিমালা_List_Delete(db, pEList);
  sqlite3_ৰাশিমালা_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  ঘূৰাই pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  ৰাশিমালা *pWhere,           /* The WHERE clause */
  ধ্রুৱ ৱর্ণ *zStart,     /* Start of SQL text */
  ধ্রুৱ ৱর্ণ *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  যদি( pTriggerStep ){
    যদি( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }অন্যথা{
      pTriggerStep->pWhere = sqlite3_ৰাশিমালা_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_ৰাশিমালা_Delete(db, pWhere);
  ঘূৰাই pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
শূন্য sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  যদি( pTrigger==0 || pTrigger->bReturning ) ঘূৰাই;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_ৰাশিমালা_Delete(db, pTrigger->pWhen);
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
শূন্য sqlite3DropTrigger(Parse *pParse, SrcList *pName, পূর্ণাঙ্ক noErr){
  Trigger *pTrigger = 0;
  পূর্ণাঙ্ক i;
  ধ্রুৱ ৱর্ণ *zDb;
  ধ্রুৱ ৱর্ণ *zName;
  sqlite3 *db = pParse->db;

  যদি( db->mallocFailed ) গচ্ছ drop_trigger_cleanup;
  যদি( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    গচ্ছ drop_trigger_cleanup;
  }

  প্রতিজ্ঞা( pName->nSrc==1 );
  প্রতিজ্ঞা( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  প্রতিজ্ঞা( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  বাবে(i=OMIT_TEMPDB; i<db->nDb; i++){
    পূর্ণাঙ্ক j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    যদি( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) অনুৱর্ত;
    প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    যদি( pTrigger ) ভঙ্গ;
  }
  যদি( !pTrigger ){
    যদি( !noErr ){
      sqlite3_Error_বাৰ্তা(pParse, "no such trigger: %S", pName->a);
    }অন্যথা{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    গচ্ছ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
স্থির Table *tableOfTrigger(Trigger *pTrigger){
  ঘূৰাই sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
শূন্য sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  পূর্ণাঙ্ক iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  প্রতিজ্ঞা( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  প্রতিজ্ঞা( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  যদি( pTable ){
    পূর্ণাঙ্ক code = SQLITE_DROP_TRIGGER;
    ধ্রুৱ ৱর্ণ *zDb = db->aDb[iDb].zDbSName;
    ধ্রুৱ ৱর্ণ *zTab = SCHEMA_TABLE(iDb);
    যদি( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    যদি( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      ঘূৰাই;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  যদি( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_TABLE " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddOp4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
শূন্য sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, পূর্ণাঙ্ক iDb, ধ্রুৱ ৱর্ণ *zName){
  Trigger *pTrigger;
  Hash *pHash;

  প্রতিজ্ঞা( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ঢোকাও(pHash, zName, 0);
  যদি( ALWAYS(pTrigger) ){
    যদি( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      যদি( pTab ){
        Trigger **pp;
        বাবে(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          যদি( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            ভঙ্গ;
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
স্থির পূর্ণাঙ্ক checkColumnOverlap(IdList *pIdList, ৰাশিমালা_List *pEList){
  পূর্ণাঙ্ক e;
  যদি( pIdList==0 || NEVER(pEList==0) ) ঘূৰাই 1;
  বাবে(e=0; e<pEList->n_ৰাশিমালা; e++){
    যদি( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) ঘূৰাই 1;
  }
  ঘূৰাই 0; 
}

/*
** Return true if any TEMP triggers exist
*/
স্থির পূর্ণাঙ্ক tempTriggersExist(sqlite3 *db){
  যদি( NEVER(db->aDb[1].pSchema==0) ) ঘূৰাই 0;
  যদি( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) ঘূৰাই 0;
  ঘূৰাই 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
স্থির SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  পূর্ণাঙ্ক op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ৰাশিমালা_List *pChanges,     /* Columns that change in an UPDATE statement */
  পূর্ণাঙ্ক *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  পূর্ণাঙ্ক mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  প্রতিজ্ঞা( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  যদি( pList!=0 ){
    p = pList;
    যদি( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      যদি( pList==pTab->pTrigger ){
        pList = 0;
        গচ্ছ exit_triggers_exist;
      }
      যেতিয়ালৈ( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    কৰা{
      যদি( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }অন্যথা যদি( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        প্রতিজ্ঞা( sqlite3IsToplevel(pParse) );
        p->op = op;
        যদি( IsVirtual(pTab) ){
          যদি( op!=TK_ঢোকাও ){
            sqlite3_Error_বাৰ্তা(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }অন্যথা{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }অন্যথা যদি( p->bReturning && p->op==TK_ঢোকাও && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }যেতিয়ালৈ( p );
  }
exit_triggers_exist:
  যদি( pMask ){
    *pMask = mask;
  }
  ঘূৰাই (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  পূর্ণাঙ্ক op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ৰাশিমালা_List *pChanges,     /* Columns that change in an UPDATE statement */
  পূর্ণাঙ্ক *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  প্রতিজ্ঞা( pTab!=0 );
  যদি( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    যদি( pMask ) *pMask = 0;
    ঘূৰাই 0;
  }
  ঘূৰাই triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
স্থির পূর্ণাঙ্ক isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  ৰাশিমালা *pTerm         /* A term in the RETURNING clause */
){
  প্রতিজ্ঞা( pTerm!=0 );
  যদি( pTerm->op==TK_ASTERISK ) ঘূৰাই 1;
  যদি( pTerm->op!=TK_DOT ) ঘূৰাই 0;
  প্রতিজ্ঞা( pTerm->p_ডান!=0 );
  প্রতিজ্ঞা( pTerm->p_বাম!=0 );
  যদি( pTerm->p_ডান->op!=TK_ASTERISK ) ঘূৰাই 0;
  sqlite3_Error_বাৰ্তা(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  ঘূৰাই 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
স্থির ৰাশিমালা_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  ৰাশিমালা_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  ৰাশিমালা_List *pNew = 0;
  sqlite3 *db = pParse->db;
  পূর্ণাঙ্ক i;

  বাবে(i=0; i<pList->n_ৰাশিমালা; i++){
    ৰাশিমালা *p_Old_ৰাশিমালা = pList->a[i].p_ৰাশিমালা;
    যদি( NEVER(p_Old_ৰাশিমালা==0) ) অনুৱর্ত;
    যদি( isAsteriskTerm(pParse, p_Old_ৰাশিমালা) ){
      পূর্ণাঙ্ক jj;
      বাবে(jj=0; jj<pTab->nCol; jj++){
        ৰাশিমালা *p_New_ৰাশিমালা;
        যদি( IsHiddenColumn(pTab->aCol+jj) ) অনুৱর্ত;
        p_New_ৰাশিমালা = sqlite3_ৰাশিমালা(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_ৰাশিমালা_List_Append(pParse, pNew, p_New_ৰাশিমালা);
        যদি( !db->mallocFailed ){
          সংরচনা ৰাশিমালা_List_item *pItem = &pNew->a[pNew->n_ৰাশিমালা-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }অন্যথা{
      ৰাশিমালা *p_New_ৰাশিমালা = sqlite3_ৰাশিমালা_Dup(db, p_Old_ৰাশিমালা, 0);
      pNew = sqlite3_ৰাশিমালা_List_Append(pParse, pNew, p_New_ৰাশিমালা);
      যদি( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        সংরচনা ৰাশিমালা_List_item *pItem = &pNew->a[pNew->n_ৰাশিমালা-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  ঘূৰাই pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
স্থির পূর্ণাঙ্ক sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, ৰাশিমালা *p_ৰাশিমালা){
  UNUSED_PARAMETER(NotUsed);
  যদি( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা)
   && (p_ৰাশিমালা->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_VarSelect) );
    ৰাশিমালা_Set_Property(p_ৰাশিমালা, EP_VarSelect);
  }
  ঘূৰাই WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
স্থির পূর্ণাঙ্ক sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  পূর্ণাঙ্ক i;
  SrcList *pSrc;
  প্রতিজ্ঞা( pSelect!=0 );
  pSrc = pSelect->pSrc;
  প্রতিজ্ঞা( pSrc!=0 );
  বাবে(i=0; i<pSrc->nSrc; i++){
    যদি( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      ভঙ্গ;
    }
  }
  ঘূৰাই WRC_Continue;
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
স্থির শূন্য sqlite3ProcessReturningSubqueries(
  ৰাশিমালা_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, পরিমাণ(w));
  w.x_ৰাশিমালা_Callback = sqlite3_ৰাশিমালা_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_ৰাশিমালা_List(&w, pEList);
  যদি( w.eCode ){
    w.x_ৰাশিমালা_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_ৰাশিমালা_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
স্থির শূন্য codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  পূর্ণাঙ্ক regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ৰাশিমালা_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  সঙ্ঘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  প্রতিজ্ঞা( v!=0 );
  যদি( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    ঘূৰাই;
  }
  প্রতিজ্ঞা( db->pParse==pParse );
  প্রতিজ্ঞা( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  যদি( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    ঘূৰাই;
  }
  memset(&sSelect, 0, পরিমাণ(sSelect));
  memset(&uSrc, 0, পরিমাণ(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_ৰাশিমালা_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  যদি( pParse->nErr==0 ){
    প্রতিজ্ঞা( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_ৰাশিমালা_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  যদি( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, পরিমাণ(sNC));
    যদি( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_ৰাশিমালা;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    যদি( sqlite3_Resolve_ৰাশিমালা_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      পূর্ণাঙ্ক i;
      পূর্ণাঙ্ক nCol = pNew->n_ৰাশিমালা;
      পূর্ণাঙ্ক reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      বাবে(i=0; i<nCol; i++){
        ৰাশিমালা *pCol = pNew->a[i].p_ৰাশিমালা;
        প্রতিজ্ঞা( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_ৰাশিমালা_Code_Factorable(pParse, pCol, reg+i);
        যদি( sqlite3_ৰাশিমালা_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_ঢোকাও, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_ৰাশিমালা_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
স্থির পূর্ণাঙ্ক codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  পূর্ণাঙ্ক orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  প্রতিজ্ঞা( pParse->pTriggerTab && pParse->pToplevel );
  প্রতিজ্ঞা( pStepList );
  প্রতিজ্ঞা( v!=0 );
  বাবে(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    pParse->eOrconf = (orconf==OE_Default)?pStep->orconf:(u8)orconf;
    প্রতিজ্ঞা( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    যদি( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    ৱিকল্প( pStep->op ){
      পক্ষ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ৰাশিমালা_List_Dup(db, pStep->p_ৰাশিমালা_List, 0), 
          sqlite3_ৰাশিমালা_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_গুনতি);
        ভঙ্গ;
      }
      পক্ষ TK_ঢোকাও: {
        sqlite3_ঢোকাও(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_গুনতি);
        ভঙ্গ;
      }
      পক্ষ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ৰাশিমালা_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_গুনতি);
        ভঙ্গ;
      }
      স্ৱভাৱ: প্রতিজ্ঞা( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ভঙ্গ;
      }
    } 
  }

  ঘূৰাই 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
স্থির ধ্রুৱ ৱর্ণ *onErrorText(পূর্ণাঙ্ক onError){
  ৱিকল্প( onError ){
    পক্ষ OE_Abort:    ঘূৰাই "abort";
    পক্ষ OE_Rollback: ঘূৰাই "rollback";
    পক্ষ OE_Fail:     ঘূৰাই "fail";
    পক্ষ OE_Replace:  ঘূৰাই "replace";
    পক্ষ OE_Ignore:   ঘূৰাই "ignore";
    পক্ষ OE_Default:  ঘূৰাই "default";
  }
  ঘূৰাই "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
স্থির শূন্য transferParseError(Parse *pTo, Parse *pFrom){
  প্রতিজ্ঞা( pFrom->z_Err_বাৰ্তা==0 || pFrom->nErr );
  প্রতিজ্ঞা( pTo->z_Err_বাৰ্তা==0 || pTo->nErr );
  যদি( pTo->nErr==0 ){
    pTo->z_Err_বাৰ্তা = pFrom->z_Err_বাৰ্তা;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }অন্যথা{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_বাৰ্তা);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
স্থির TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  পূর্ণাঙ্ক orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_শীৰ্ষ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  ৰাশিমালা *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  পূর্ণাঙ্ক iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  পূর্ণাঙ্ক nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_শীৰ্ষ = pParse;
  বাবে(nDepth=0; p_শীৰ্ষ->pOuterParse; p_শীৰ্ষ = p_শীৰ্ষ->pOuterParse, nDepth++){}
  যদি( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_বাৰ্তা(pParse, "triggers nested too deep");
    ঘূৰাই 0;
  }

  p_শীৰ্ষ = sqlite3ParseToplevel(pParse);
  প্রতিজ্ঞা( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  প্রতিজ্ঞা( p_শীৰ্ষ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, পরিমাণ(TriggerPrg));
  যদি( !pPrg ) ঘূৰাই 0;
  pPrg->pNext = p_শীৰ্ষ->pTriggerPrg;
  p_শীৰ্ষ->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, পরিমাণ(SubProgram));
  যদি( !pProgram ) ঘূৰাই 0;
  sqlite3VdbeLinkSubProgram(p_শীৰ্ষ->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, পরিমাণ(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_শীৰ্ষ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  যদি( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ঢোকাও ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    যদি( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    যদি( pTrigger->pWhen ){
      pWhen = sqlite3_ৰাশিমালা_Dup(db, pTrigger->pWhen, 0);
      যদি( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_ৰাশিমালা_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_ৰাশিমালা_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_ৰাশিমালা_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    যদি( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    যদি( pParse->nErr==0 ){
      প্রতিজ্ঞা( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_শীৰ্ষ->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (শূন্য *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }অন্যথা{
    transferParseError(pParse, &sSubParse);
  }

  প্রতিজ্ঞা( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  ঘূৰাই pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
স্থির TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  পূর্ণাঙ্ক orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_মূল = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  প্রতিজ্ঞা( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  বাবে(pPrg=p_মূল->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  যদি( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  ঘূৰাই pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
শূন্য sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  পূর্ণাঙ্ক reg,             /* Reg array containing OLD.* and NEW.* values */
  পূর্ণাঙ্ক orconf,          /* ON CONFLICT policy */
  পূর্ণাঙ্ক ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  প্রতিজ্ঞা( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  যদি( pPrg ){
    পূর্ণাঙ্ক bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (ধ্রুৱ ৱর্ণ *)pPrg->pProgram, P4_SUBPROGRAM);
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
শূন্য sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  পূর্ণাঙ্ক op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ৰাশিমালা_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  পূর্ণাঙ্ক tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  পূর্ণাঙ্ক reg,             /* The first in an array of registers (see above) */
  পূর্ণাঙ্ক orconf,          /* ON CONFLICT policy */
  পূর্ণাঙ্ক ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  প্রতিজ্ঞা( op==TK_UPDATE || op==TK_ঢোকাও || op==TK_DELETE );
  প্রতিজ্ঞা( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  প্রতিজ্ঞা( (op==TK_UPDATE)==(pChanges!=0) );

  বাবে(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    প্রতিজ্ঞা( p->pSchema!=0 );
    প্রতিজ্ঞা( p->pTabSchema!=0 );
    প্রতিজ্ঞা( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    যদি( (p->op==op || (p->bReturning && p->op==TK_ঢোকাও && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      যদি( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }অন্যথা যদি( sqlite3IsToplevel(pParse) ){
        codeReturningTrigger(pParse, p, pTab, reg);
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
  ৰাশিমালা_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  পূর্ণাঙ্ক isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  পূর্ণাঙ্ক tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  পূর্ণাঙ্ক orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ধ্রুৱ পূর্ণাঙ্ক op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  প্রতিজ্ঞা( isNew==1 || isNew==0 );
  যদি( IsView(pTab) ){
    ঘূৰাই 0xffffffff;
  }
  বাবে(p=pTrigger; p; p=p->pNext){
    যদি( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      যদি( p->bReturning ){
        mask = 0xffffffff;
      }অন্যথা{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        যদি( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  ঘূৰাই mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
