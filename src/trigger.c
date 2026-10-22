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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
ଶୂନ୍ୟ sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  ଯେତେବେଳେ( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pTmp->pWhere);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pTmp->p_ପଦସମଷ୍ଟି_List);
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

  ପ୍ରତିଜ୍ଞା( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  ଯେତେବେଳେ( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    ଯଦି( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }ଅନ୍ୟଥା ଯଦି( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      ପ୍ରତିଜ୍ଞା( pParse->db->pVtabCtx==0 );
#endif
      ପ୍ରତିଜ୍ଞା( pParse->bReturning );
      ପ୍ରତିଜ୍ଞା( !pParse->isCreate );
      ପ୍ରତିଜ୍ଞା( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#ଯଦି 0
  ଯଦି( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    ପାଇଁ(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  ଫେରନ୍ତୁ pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
ଶୂନ୍ୟ sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  ପଦସମଷ୍ଟି *pWhen,        /* WHEN clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isTemp,         /* True if the TEMPORARY keyword is present */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  ଅକ୍ଷର *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ପ୍ରତିଜ୍ଞା( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ପ୍ରତିଜ୍ଞା( pName2!=0 );
  ପ୍ରତିଜ୍ଞା( op==TK_ଭର୍ତ୍ତି || op==TK_UPDATE || op==TK_DELETE );
  ପ୍ରତିଜ୍ଞା( op>0 && op<0xff );
  ଯଦି( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    ଯଦି( pName2->n>0 ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "temporary trigger may not have qualified name");
      ଯାଅ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }ଅନ୍ୟଥା{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    ଯଦି( iDb<0 ){
      ଯାଅ trigger_cleanup;
    }
  }
  ଯଦି( !pTableName || db->mallocFailed ){
    ଯାଅ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  ଯଦି( db->init.busy && iDb!=1 ){
    ପ୍ରତିଜ୍ଞା( pTableName->a[0].fg.fixedSchema==0 );
    ପ୍ରତିଜ୍ଞା( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  ଯଦି( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  ଯଦି( db->mallocFailed ) ଯାଅ trigger_cleanup;
  ପ୍ରତିଜ୍ଞା( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  ଯଦି( sqlite3FixSrcList(&sFix, pTableName) ){
    ଯାଅ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  ଯଦି( !pTab ){
    /* The table does not exist. */
    ଯାଅ trigger_orphan_error;
  }
  ଯଦି( IsVirtual(pTab) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot create triggers on virtual tables");
    ଯାଅ trigger_orphan_error;
  }
  ଯଦି( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot create triggers on shadow tables");
    ଯାଅ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  ଯଦି( zName==0 ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed );
    ଯାଅ trigger_cleanup;
  }
  ଯଦି( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    ଯାଅ trigger_cleanup;
  }
  ପ୍ରତିଜ୍ଞା( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ଯଦି( !IN_RENAME_OBJECT ){
    ଯଦି( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      ଯଦି( !noErr ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "trigger %T already exists", pName);
      }ଅନ୍ୟଥା ଯଦି( db->init.busy ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse,""); /* Err msg generated by corruptSchema() */
      }ଅନ୍ୟଥା{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      ଯାଅ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#ଯଦି !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  ଯଦି( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot create trigger on system table");
    ଯାଅ trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  ଯଦି( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    ଯାଅ trigger_orphan_error;
  }
  ଯଦି( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    ଯାଅ trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  ଯଦି( !IN_RENAME_OBJECT ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = SQLITE_CREATE_TRIGGER;
    ଅଚଳ ଅକ୍ଷର *zDb = db->aDb[iTabDb].zDbSName;
    ଅଚଳ ଅକ୍ଷର *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    ଯଦି( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    ଯଦି( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      ଯାଅ trigger_cleanup;
    }
    ଯଦି( sqlite3AuthCheck(pParse, SQLITE_ଭର୍ତ୍ତି, SCHEMA_TABLE(iTabDb),0,zDb)){
      ଯାଅ trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  ଯଦି (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, ଆକାର(Trigger));
  ଯଦି( pTrigger==0 ) ଯାଅ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  ଯଦି( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }ଅନ୍ୟଥା{
    pTrigger->pWhen = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ପ୍ରତିଜ୍ଞା( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhen);
  ଯଦି( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( pParse->pNewTrigger==pTrigger );
  }
  ଫେରନ୍ତୁ;

trigger_orphan_error:
  ଯଦି( db->init.iDb==1 ){
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
  ଯାଅ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
ଶୂନ୍ୟ sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  ଅକ୍ଷର *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  ଯଦି( NEVER(pParse->nErr) || !pTrig ) ଯାଅ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  ପ୍ରତିଜ୍ଞା( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  ପାଇଁ(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  ଯଦି( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "trigger \"%w\" contains too many steps", zName);
    ଯାଅ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  ଯଦି( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_ପଦସମଷ୍ଟି(&sFix, pTrig->pWhen) 
  ){
    ଯାଅ triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  ଯଦି( IN_RENAME_OBJECT ){
    ପ୍ରତିଜ୍ଞା( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }ଅନ୍ୟଥା
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  ଯଦି( !db->init.busy ){
    Vdbe *v;
    ଅକ୍ଷର *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    ଯଦି( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      ପାଇଁ(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        ଯଦି( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          ଯାଅ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    ଯଦି( v==0 ) ଯାଅ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (ଅକ୍ଷର*)pAll->z, pAll->n);
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

  ଯଦି( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ପ୍ରତିଜ୍ଞା( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ପ୍ରତିଜ୍ଞା( pLink!=0 );
    pTrig = sqlite3_Hash_ଭର୍ତ୍ତି(pHash, zName, pTrig);
    ଯଦି( pTrig ){
      sqlite3OomFault(db);
    }ଅନ୍ୟଥା ଯଦି( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      ପ୍ରତିଜ୍ଞା( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ପ୍ରତିଜ୍ଞା( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ସ୍ଥିର ଅକ୍ଷର *triggerSpanDup(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zStart, ଅଚଳ ଅକ୍ଷର *zEnd){
  ଅକ୍ଷର *z = sqlite3DbSpanDup(db, zStart, zEnd);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( z ) ପାଇଁ(i=0; z[i]; i++) ଯଦି( sqlite3Isspace(z[i]) ) z[i] = ' ';
  ଫେରନ୍ତୁ z;
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
  ଅଚଳ ଅକ୍ଷର *zStart,         /* Start of SQL text */
  ଅଚଳ ଅକ୍ଷର *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, ଆକାର(TriggerStep));
  ଯଦି( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    ଫେରନ୍ତୁ 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  ଫେରନ୍ତୁ pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ସ୍ଥିର TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ଅଚଳ ଅକ୍ଷର *zStart,         /* Start of SQL text */
  ଅଚଳ ଅକ୍ଷର *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  ଯଦି( pParse->nErr==0 ){
    ଯଦି( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }ଅନ୍ୟଥା{
      pTriggerStep = sqlite3DbMallocZero(db, ଆକାର(TriggerStep));
      ଯଦି( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        ଯଦି( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  ଫେରନ୍ତୁ pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_ଭର୍ତ୍ତି_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ଅଚଳ ଅକ୍ଷର *zStart, /* Start of SQL text */
  ଅଚଳ ଅକ୍ଷର *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  ପ୍ରତିଜ୍ଞା(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_ଭର୍ତ୍ତି, pTabList, zStart, zEnd);
  ଯଦି( pTriggerStep ){
    ଯଦି( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }ଅନ୍ୟଥା{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    ଯଦି( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }ଅନ୍ୟଥା{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  ଫେରନ୍ତୁ pTriggerStep;
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
  ପଦସମଷ୍ଟି_List *pEList,    /* The SET clause: list of column and new values */
  ପଦସମଷ୍ଟି *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ଅଚଳ ଅକ୍ଷର *zStart,  /* Start of SQL text */
  ଅଚଳ ଅକ୍ଷର *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  ଯଦି( pTriggerStep ){
    SrcList *pFromDup = 0;
    ଯଦି( IN_RENAME_OBJECT ){
      pTriggerStep->p_ପଦସମଷ୍ଟି_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }ଅନ୍ୟଥା{
      pTriggerStep->p_ପଦସମଷ୍ଟି_List = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    ଯଦି( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    ଯଦି( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }ଅନ୍ୟଥା{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pEList);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  ଫେରନ୍ତୁ pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  ପଦସମଷ୍ଟି *pWhere,           /* The WHERE clause */
  ଅଚଳ ଅକ୍ଷର *zStart,     /* Start of SQL text */
  ଅଚଳ ଅକ୍ଷର *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  ଯଦି( pTriggerStep ){
    ଯଦି( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }ଅନ୍ୟଥା{
      pTriggerStep->pWhere = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhere);
  ଫେରନ୍ତୁ pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
ଶୂନ୍ୟ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  ଯଦି( pTrigger==0 || pTrigger->bReturning ) ଫେରନ୍ତୁ;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, pTrigger->pWhen);
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
ଶୂନ୍ୟ sqlite3DropTrigger(Parse *pParse, SrcList *pName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା noErr){
  Trigger *pTrigger = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zDb;
  ଅଚଳ ଅକ୍ଷର *zName;
  sqlite3 *db = pParse->db;

  ଯଦି( db->mallocFailed ) ଯାଅ drop_trigger_cleanup;
  ଯଦି( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ଯାଅ drop_trigger_cleanup;
  }

  ପ୍ରତିଜ୍ଞା( pName->nSrc==1 );
  ପ୍ରତିଜ୍ଞା( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ପ୍ରତିଜ୍ଞା( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  ପାଇଁ(i=OMIT_TEMPDB; i<db->nDb; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    ଯଦି( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    ଯଦି( pTrigger ) ଭାଙ୍ଗ;
  }
  ଯଦି( !pTrigger ){
    ଯଦି( !noErr ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such trigger: %S", pName->a);
    }ଅନ୍ୟଥା{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    ଯାଅ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ସ୍ଥିର Table *tableOfTrigger(Trigger *pTrigger){
  ଫେରନ୍ତୁ sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
ଶୂନ୍ୟ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  ପ୍ରତିଜ୍ଞା( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  ପ୍ରତିଜ୍ଞା( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  ଯଦି( pTable ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = SQLITE_DROP_TRIGGER;
    ଅଚଳ ଅକ୍ଷର *zDb = db->aDb[iDb].zDbSName;
    ଅଚଳ ଅକ୍ଷର *zTab = SCHEMA_TABLE(iDb);
    ଯଦି( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    ଯଦି( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      ଫେରନ୍ତୁ;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  ଯଦି( (v = sqlite3GetVdbe(pParse))!=0 ){
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
ଶୂନ୍ୟ sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb, ଅଚଳ ଅକ୍ଷର *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ପ୍ରତିଜ୍ଞା( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ଭର୍ତ୍ତି(pHash, zName, 0);
  ଯଦି( ALWAYS(pTrigger) ){
    ଯଦି( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      ଯଦି( pTab ){
        Trigger **pp;
        ପାଇଁ(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          ଯଦି( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            ଭାଙ୍ଗ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkColumnOverlap(IdList *pIdList, ପଦସମଷ୍ଟି_List *pEList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା e;
  ଯଦି( pIdList==0 || NEVER(pEList==0) ) ଫେରନ୍ତୁ 1;
  ପାଇଁ(e=0; e<pEList->n_ପଦସମଷ୍ଟି; e++){
    ଯଦି( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tempTriggersExist(sqlite3 *db){
  ଯଦି( NEVER(db->aDb[1].pSchema==0) ) ଫେରନ୍ତୁ 0;
  ଯଦି( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ସ୍ଥିର SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ପଦସମଷ୍ଟି_List *pChanges,     /* Columns that change in an UPDATE statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ପ୍ରତିଜ୍ଞା( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  ଯଦି( pList!=0 ){
    p = pList;
    ଯଦି( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      ଯଦି( pList==pTab->pTrigger ){
        pList = 0;
        ଯାଅ exit_triggers_exist;
      }
      ଯେତେବେଳେ( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    କର{
      ଯଦି( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }ଅନ୍ୟଥା ଯଦି( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ପ୍ରତିଜ୍ଞା( sqlite3IsToplevel(pParse) );
        p->op = op;
        ଯଦି( IsVirtual(pTab) ){
          ଯଦି( op!=TK_ଭର୍ତ୍ତି ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }ଅନ୍ୟଥା{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }ଅନ୍ୟଥା ଯଦି( p->bReturning && p->op==TK_ଭର୍ତ୍ତି && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }ଯେତେବେଳେ( p );
  }
exit_triggers_exist:
  ଯଦି( pMask ){
    *pMask = mask;
  }
  ଫେରନ୍ତୁ (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ପଦସମଷ୍ଟି_List *pChanges,     /* Columns that change in an UPDATE statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ଯଦି( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    ଯଦି( pMask ) *pMask = 0;
    ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  ପଦସମଷ୍ଟି *pTerm         /* A term in the RETURNING clause */
){
  ପ୍ରତିଜ୍ଞା( pTerm!=0 );
  ଯଦି( pTerm->op==TK_ASTERISK ) ଫେରନ୍ତୁ 1;
  ଯଦି( pTerm->op!=TK_DOT ) ଫେରନ୍ତୁ 0;
  ପ୍ରତିଜ୍ଞା( pTerm->p_ଡାହାଣ!=0 );
  ପ୍ରତିଜ୍ଞା( pTerm->p_ବାମ!=0 );
  ଯଦି( pTerm->p_ଡାହାଣ->op!=TK_ASTERISK ) ଫେରନ୍ତୁ 0;
  sqlite3_Error_ସନ୍ଦେଶ(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  ଫେରନ୍ତୁ 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ସ୍ଥିର ପଦସମଷ୍ଟି_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  ପଦସମଷ୍ଟି_List *pNew = 0;
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *p_Old_ପଦସମଷ୍ଟି = pList->a[i].p_ପଦସମଷ୍ଟି;
    ଯଦି( NEVER(p_Old_ପଦସମଷ୍ଟି==0) ) ଜାରି;
    ଯଦି( isAsteriskTerm(pParse, p_Old_ପଦସମଷ୍ଟି) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
      ପାଇଁ(jj=0; jj<pTab->nCol; jj++){
        ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି;
        ଯଦି( IsHiddenColumn(pTab->aCol+jj) ) ଜାରି;
        p_New_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pNew, p_New_ପଦସମଷ୍ଟି);
        ଯଦି( !db->mallocFailed ){
          ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &pNew->a[pNew->n_ପଦସମଷ୍ଟି-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }ଅନ୍ୟଥା{
      ପଦସମଷ୍ଟି *p_New_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_Old_ପଦସମଷ୍ଟି, 0);
      pNew = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pNew, p_New_ପଦସମଷ୍ଟି);
      ଯଦି( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &pNew->a[pNew->n_ପଦସମଷ୍ଟି-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  ଫେରନ୍ତୁ pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  UNUSED_PARAMETER(NotUsed);
  ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି)
   && (p_ପଦସମଷ୍ଟି->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_VarSelect) );
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_VarSelect);
  }
  ଫେରନ୍ତୁ WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  SrcList *pSrc;
  ପ୍ରତିଜ୍ଞା( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ପ୍ରତିଜ୍ଞା( pSrc!=0 );
  ପାଇଁ(i=0; i<pSrc->nSrc; i++){
    ଯଦି( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ଶୂନ୍ୟ sqlite3ProcessReturningSubqueries(
  ପଦସମଷ୍ଟି_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, ଆକାର(w));
  w.x_ପଦସମଷ୍ଟି_Callback = sqlite3_ପଦସମଷ୍ଟି_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_ପଦସମଷ୍ଟି_List(&w, pEList);
  ଯଦି( w.eCode ){
    w.x_ପଦସମଷ୍ଟି_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_ପଦସମଷ୍ଟି_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ସ୍ଥିର ଶୂନ୍ୟ codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  ସଂଘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ପ୍ରତିଜ୍ଞା( v!=0 );
  ଯଦି( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    ଫେରନ୍ତୁ;
  }
  ପ୍ରତିଜ୍ଞା( db->pParse==pParse );
  ପ୍ରତିଜ୍ଞା( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  ଯଦି( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    ଫେରନ୍ତୁ;
  }
  memset(&sSelect, 0, ଆକାର(sSelect));
  memset(&uSrc, 0, ଆକାର(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  ଯଦି( pParse->nErr==0 ){
    ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  ଯଦି( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, ଆକାର(sNC));
    ଯଦି( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_ପଦସମଷ୍ଟି;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = pNew->n_ପଦସମଷ୍ଟି;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      ପାଇଁ(i=0; i<nCol; i++){
        ପଦସମଷ୍ଟି *pCol = pNew->a[i].p_ପଦସମଷ୍ଟି;
        ପ୍ରତିଜ୍ଞା( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_ପଦସମଷ୍ଟି_Code_Factorable(pParse, pCol, reg+i);
        ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ପ୍ରତିଜ୍ଞା( pParse->pTriggerTab && pParse->pToplevel );
  ପ୍ରତିଜ୍ଞା( pStepList );
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପାଇଁ(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    ପ୍ରତିଜ୍ଞା( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    ଯଦି( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    ବାଛ( pStep->op ){
      ସ୍ଥିତି TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pStep->p_ପଦସମଷ୍ଟି_List, 0), 
          sqlite3_ପଦସମଷ୍ଟି_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ଗଣନା);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_ଭର୍ତ୍ତି: {
        sqlite3_ଭର୍ତ୍ତି(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ଗଣନା);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ପଦସମଷ୍ଟି_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ଗଣନା);
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ପ୍ରତିଜ୍ଞା( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ଭାଙ୍ଗ;
      }
    } 
  }

  ଫେରନ୍ତୁ 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *onErrorText(ପୂର୍ଣ୍ଣସଂଖ୍ୟା onError){
  ବାଛ( onError ){
    ସ୍ଥିତି OE_Abort:    ଫେରନ୍ତୁ "abort";
    ସ୍ଥିତି OE_Rollback: ଫେରନ୍ତୁ "rollback";
    ସ୍ଥିତି OE_Fail:     ଫେରନ୍ତୁ "fail";
    ସ୍ଥିତି OE_Replace:  ଫେରନ୍ତୁ "replace";
    ସ୍ଥିତି OE_Ignore:   ଫେରନ୍ତୁ "ignore";
    ସ୍ଥିତି OE_Default:  ଫେରନ୍ତୁ "default";
  }
  ଫେରନ୍ତୁ "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ସ୍ଥିର ଶୂନ୍ୟ transferParseError(Parse *pTo, Parse *pFrom){
  ପ୍ରତିଜ୍ଞା( pFrom->z_Err_ସନ୍ଦେଶ==0 || pFrom->nErr );
  ପ୍ରତିଜ୍ଞା( pTo->z_Err_ସନ୍ଦେଶ==0 || pTo->nErr );
  ଯଦି( pTo->nErr==0 ){
    pTo->z_Err_ସନ୍ଦେଶ = pFrom->z_Err_ସନ୍ଦେଶ;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }ଅନ୍ୟଥା{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_ସନ୍ଦେଶ);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ସ୍ଥିର TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_ଶୀର୍ଷ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  ପଦସମଷ୍ଟି *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_ଶୀର୍ଷ = pParse;
  ପାଇଁ(nDepth=0; p_ଶୀର୍ଷ->pOuterParse; p_ଶୀର୍ଷ = p_ଶୀର୍ଷ->pOuterParse, nDepth++){}
  ଯଦି( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "triggers nested too deep");
    ଫେରନ୍ତୁ 0;
  }

  p_ଶୀର୍ଷ = sqlite3ParseToplevel(pParse);
  ପ୍ରତିଜ୍ଞା( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  ପ୍ରତିଜ୍ଞା( p_ଶୀର୍ଷ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, ଆକାର(TriggerPrg));
  ଯଦି( !pPrg ) ଫେରନ୍ତୁ 0;
  pPrg->pNext = p_ଶୀର୍ଷ->pTriggerPrg;
  p_ଶୀର୍ଷ->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, ଆକାର(SubProgram));
  ଯଦି( !pProgram ) ଫେରନ୍ତୁ 0;
  sqlite3VdbeLinkSubProgram(p_ଶୀର୍ଷ->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, ଆକାର(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_ଶୀର୍ଷ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  ଯଦି( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ଭର୍ତ୍ତି ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    ଯଦି( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    ଯଦି( pTrigger->pWhen ){
      pWhen = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pTrigger->pWhen, 0);
      ଯଦି( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_ପଦସମଷ୍ଟି_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_ପଦସମଷ୍ଟି_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    ଯଦି( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    ଯଦି( pParse->nErr==0 ){
      ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_ଶୀର୍ଷ->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (ଶୂନ୍ୟ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }ଅନ୍ୟଥା{
    transferParseError(pParse, &sSubParse);
  }

  ପ୍ରତିଜ୍ଞା( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  ଫେରନ୍ତୁ pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ସ୍ଥିର TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_ମୂଳ = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ପ୍ରତିଜ୍ଞା( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  ପାଇଁ(pPrg=p_ମୂଳ->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  ଯଦି( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  ଫେରନ୍ତୁ pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
ଶୂନ୍ୟ sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg,             /* Reg array containing OLD.* and NEW.* values */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf,          /* ON CONFLICT policy */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  ପ୍ରତିଜ୍ଞା( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  ଯଦି( pPrg ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (ଅଚଳ ଅକ୍ଷର *)pPrg->pProgram, P4_SUBPROGRAM);
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
ଶୂନ୍ୟ sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ପଦସମଷ୍ଟି_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg,             /* The first in an array of registers (see above) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf,          /* ON CONFLICT policy */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ପ୍ରତିଜ୍ଞା( op==TK_UPDATE || op==TK_ଭର୍ତ୍ତି || op==TK_DELETE );
  ପ୍ରତିଜ୍ଞା( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ପ୍ରତିଜ୍ଞା( (op==TK_UPDATE)==(pChanges!=0) );

  ପାଇଁ(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    ପ୍ରତିଜ୍ଞା( p->pSchema!=0 );
    ପ୍ରତିଜ୍ଞା( p->pTabSchema!=0 );
    ପ୍ରତିଜ୍ଞା( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    ଯଦି( (p->op==op || (p->bReturning && p->op==TK_ଭର୍ତ୍ତି && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      ଯଦି( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }ଅନ୍ୟଥା ଯଦି( sqlite3IsToplevel(pParse) ){
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
  ପଦସମଷ୍ଟି_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  ପ୍ରତିଜ୍ଞା( isNew==1 || isNew==0 );
  ଯଦି( IsView(pTab) ){
    ଫେରନ୍ତୁ 0xffffffff;
  }
  ପାଇଁ(p=pTrigger; p; p=p->pNext){
    ଯଦି( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      ଯଦି( p->bReturning ){
        mask = 0xffffffff;
      }ଅନ୍ୟଥା{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        ଯଦି( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  ଫେରନ୍ତୁ mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
