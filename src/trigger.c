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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
ಶೂನ್ಯ sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  ಯಾವತ್( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pTmp->pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pTmp->p_ಅಭಿವ್ಯಕ್ತಿ_List);
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

  ಪ್ರತಿಜ್ಞಾ( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  ಯಾವತ್( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    ಯದಿ( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }ಅನ್ಯಥಾ ಯದಿ( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      ಪ್ರತಿಜ್ಞಾ( pParse->db->pVtabCtx==0 );
#endif
      ಪ್ರತಿಜ್ಞಾ( pParse->bReturning );
      ಪ್ರತಿಜ್ಞಾ( !pParse->isCreate );
      ಪ್ರತಿಜ್ಞಾ( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#ಯದಿ 0
  ಯದಿ( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    ಪ್ರತಿ(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
ಶೂನ್ಯ sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  ಪೂರ್ಣಾಙ್ಕ tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  ಪೂರ್ಣಾಙ್ಕ op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  ಅಭಿವ್ಯಕ್ತಿ *pWhen,        /* WHEN clause */
  ಪೂರ್ಣಾಙ್ಕ isTemp,         /* True if the TEMPORARY keyword is present */
  ಪೂರ್ಣಾಙ್ಕ noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  ವರ್ಣ *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  ಪೂರ್ಣಾಙ್ಕ iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ಪ್ರತಿಜ್ಞಾ( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ಪ್ರತಿಜ್ಞಾ( pName2!=0 );
  ಪ್ರತಿಜ್ಞಾ( op==TK_ಸೇರಿಸು || op==TK_UPDATE || op==TK_DELETE );
  ಪ್ರತಿಜ್ಞಾ( op>0 && op<0xff );
  ಯದಿ( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    ಯದಿ( pName2->n>0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "temporary trigger may not have qualified name");
      ಗಚ್ಛ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }ಅನ್ಯಥಾ{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    ಯದಿ( iDb<0 ){
      ಗಚ್ಛ trigger_cleanup;
    }
  }
  ಯದಿ( !pTableName || db->mallocFailed ){
    ಗಚ್ಛ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  ಯದಿ( db->init.busy && iDb!=1 ){
    ಪ್ರತಿಜ್ಞಾ( pTableName->a[0].fg.fixedSchema==0 );
    ಪ್ರತಿಜ್ಞಾ( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  ಯದಿ( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  ಯದಿ( db->mallocFailed ) ಗಚ್ಛ trigger_cleanup;
  ಪ್ರತಿಜ್ಞಾ( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  ಯದಿ( sqlite3FixSrcList(&sFix, pTableName) ){
    ಗಚ್ಛ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  ಯದಿ( !pTab ){
    /* The table does not exist. */
    ಗಚ್ಛ trigger_orphan_error;
  }
  ಯದಿ( IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create triggers on virtual tables");
    ಗಚ್ಛ trigger_orphan_error;
  }
  ಯದಿ( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create triggers on shadow tables");
    ಗಚ್ಛ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  ಯದಿ( zName==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಗಚ್ಛ trigger_cleanup;
  }
  ಯದಿ( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    ಗಚ್ಛ trigger_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಯದಿ( !IN_RENAME_OBJECT ){
    ಯದಿ( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      ಯದಿ( !noErr ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "trigger %T already exists", pName);
      }ಅನ್ಯಥಾ ಯದಿ( db->init.busy ){
        sqlite3_Error_ಸಂದೇಶ(pParse,""); /* Err msg generated by corruptSchema() */
      }ಅನ್ಯಥಾ{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      ಗಚ್ಛ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#ಯದಿ !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  ಯದಿ( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create trigger on system table");
    ಗಚ್ಛ trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  ಯದಿ( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    ಗಚ್ಛ trigger_orphan_error;
  }
  ಯದಿ( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    ಗಚ್ಛ trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  ಯದಿ( !IN_RENAME_OBJECT ){
    ಪೂರ್ಣಾಙ್ಕ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    ಪೂರ್ಣಾಙ್ಕ code = SQLITE_CREATE_TRIGGER;
    ಧ್ರುವ ವರ್ಣ *zDb = db->aDb[iTabDb].zDbSName;
    ಧ್ರುವ ವರ್ಣ *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    ಯದಿ( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    ಯದಿ( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      ಗಚ್ಛ trigger_cleanup;
    }
    ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ಸೇರಿಸು, SCHEMA_TABLE(iTabDb),0,zDb)){
      ಗಚ್ಛ trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  ಯದಿ (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Trigger));
  ಯದಿ( pTrigger==0 ) ಗಚ್ಛ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  ಯದಿ( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }ಅನ್ಯಥಾ{
    pTrigger->pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhen);
  ಯದಿ( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==pTrigger );
  }
  ಪ್ರತ್ಯಾಗಮ;

trigger_orphan_error:
  ಯದಿ( db->init.iDb==1 ){
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
  ಗಚ್ಛ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
ಶೂನ್ಯ sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  ವರ್ಣ *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  ಪೂರ್ಣಾಙ್ಕ iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  ಯದಿ( NEVER(pParse->nErr) || !pTrig ) ಗಚ್ಛ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  ಪ್ರತಿ(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  ಯದಿ( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "trigger \"%w\" contains too many steps", zName);
    ಗಚ್ಛ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  ಯದಿ( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_ಅಭಿವ್ಯಕ್ತಿ(&sFix, pTrig->pWhen) 
  ){
    ಗಚ್ಛ triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  ಯದಿ( IN_RENAME_OBJECT ){
    ಪ್ರತಿಜ್ಞಾ( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }ಅನ್ಯಥಾ
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  ಯದಿ( !db->init.busy ){
    Vdbe *v;
    ವರ್ಣ *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    ಯದಿ( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      ಪ್ರತಿ(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        ಯದಿ( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          ಗಚ್ಛ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    ಯದಿ( v==0 ) ಗಚ್ಛ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (ವರ್ಣ*)pAll->z, pAll->n);
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

  ಯದಿ( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಪ್ರತಿಜ್ಞಾ( pLink!=0 );
    pTrig = sqlite3_Hash_ಸೇರಿಸು(pHash, zName, pTrig);
    ಯದಿ( pTrig ){
      sqlite3OomFault(db);
    }ಅನ್ಯಥಾ ಯದಿ( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ಪ್ರತಿಜ್ಞಾ( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ಸ್ಥಿರ ವರ್ಣ *triggerSpanDup(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zStart, ಧ್ರುವ ವರ್ಣ *zEnd){
  ವರ್ಣ *z = sqlite3DbSpanDup(db, zStart, zEnd);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( z ) ಪ್ರತಿ(i=0; z[i]; i++) ಯದಿ( sqlite3Isspace(z[i]) ) z[i] = ' ';
  ಪ್ರತ್ಯಾಗಮ z;
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
  ಧ್ರುವ ವರ್ಣ *zStart,         /* Start of SQL text */
  ಧ್ರುವ ವರ್ಣ *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(TriggerStep));
  ಯದಿ( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  ಪ್ರತ್ಯಾಗಮ pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ಸ್ಥಿರ TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ಧ್ರುವ ವರ್ಣ *zStart,         /* Start of SQL text */
  ಧ್ರುವ ವರ್ಣ *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  ಯದಿ( pParse->nErr==0 ){
    ಯದಿ( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }ಅನ್ಯಥಾ{
      pTriggerStep = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(TriggerStep));
      ಯದಿ( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        ಯದಿ( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  ಪ್ರತ್ಯಾಗಮ pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_ಸೇರಿಸು_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ಧ್ರುವ ವರ್ಣ *zStart, /* Start of SQL text */
  ಧ್ರುವ ವರ್ಣ *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  ಪ್ರತಿಜ್ಞಾ(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_ಸೇರಿಸು, pTabList, zStart, zEnd);
  ಯದಿ( pTriggerStep ){
    ಯದಿ( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }ಅನ್ಯಥಾ{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    ಯದಿ( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }ಅನ್ಯಥಾ{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  ಪ್ರತ್ಯಾಗಮ pTriggerStep;
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
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,    /* The SET clause: list of column and new values */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ಧ್ರುವ ವರ್ಣ *zStart,  /* Start of SQL text */
  ಧ್ರುವ ವರ್ಣ *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  ಯದಿ( pTriggerStep ){
    SrcList *pFromDup = 0;
    ಯದಿ( IN_RENAME_OBJECT ){
      pTriggerStep->p_ಅಭಿವ್ಯಕ್ತಿ_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }ಅನ್ಯಥಾ{
      pTriggerStep->p_ಅಭಿವ್ಯಕ್ತಿ_List = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    ಯದಿ( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    ಯದಿ( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }ಅನ್ಯಥಾ{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pEList);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  ಪ್ರತ್ಯಾಗಮ pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,           /* The WHERE clause */
  ಧ್ರುವ ವರ್ಣ *zStart,     /* Start of SQL text */
  ಧ್ರುವ ವರ್ಣ *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  ಯದಿ( pTriggerStep ){
    ಯದಿ( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }ಅನ್ಯಥಾ{
      pTriggerStep->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
  ಪ್ರತ್ಯಾಗಮ pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
ಶೂನ್ಯ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  ಯದಿ( pTrigger==0 || pTrigger->bReturning ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pTrigger->pWhen);
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
ಶೂನ್ಯ sqlite3DropTrigger(Parse *pParse, SrcList *pName, ಪೂರ್ಣಾಙ್ಕ noErr){
  Trigger *pTrigger = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಧ್ರುವ ವರ್ಣ *zDb;
  ಧ್ರುವ ವರ್ಣ *zName;
  sqlite3 *db = pParse->db;

  ಯದಿ( db->mallocFailed ) ಗಚ್ಛ drop_trigger_cleanup;
  ಯದಿ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ಗಚ್ಛ drop_trigger_cleanup;
  }

  ಪ್ರತಿಜ್ಞಾ( pName->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ಪ್ರತಿಜ್ಞಾ( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  ಪ್ರತಿ(i=OMIT_TEMPDB; i<db->nDb; i++){
    ಪೂರ್ಣಾಙ್ಕ j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    ಯದಿ( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    ಯದಿ( pTrigger ) ಭಙ್ಗ;
  }
  ಯದಿ( !pTrigger ){
    ಯದಿ( !noErr ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such trigger: %S", pName->a);
    }ಅನ್ಯಥಾ{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    ಗಚ್ಛ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ಸ್ಥಿರ Table *tableOfTrigger(Trigger *pTrigger){
  ಪ್ರತ್ಯಾಗಮ sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
ಶೂನ್ಯ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  ಪ್ರತಿಜ್ಞಾ( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  ಯದಿ( pTable ){
    ಪೂರ್ಣಾಙ್ಕ code = SQLITE_DROP_TRIGGER;
    ಧ್ರುವ ವರ್ಣ *zDb = db->aDb[iDb].zDbSName;
    ಧ್ರುವ ವರ್ಣ *zTab = SCHEMA_TABLE(iDb);
    ಯದಿ( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    ಯದಿ( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  ಯದಿ( (v = sqlite3GetVdbe(pParse))!=0 ){
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
ಶೂನ್ಯ sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iDb, ಧ್ರುವ ವರ್ಣ *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ಸೇರಿಸು(pHash, zName, 0);
  ಯದಿ( ALWAYS(pTrigger) ){
    ಯದಿ( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      ಯದಿ( pTab ){
        Trigger **pp;
        ಪ್ರತಿ(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          ಯದಿ( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            ಭಙ್ಗ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkColumnOverlap(IdList *pIdList, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಪೂರ್ಣಾಙ್ಕ e;
  ಯದಿ( pIdList==0 || NEVER(pEList==0) ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತಿ(e=0; e<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; e++){
    ಯದಿ( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tempTriggersExist(sqlite3 *db){
  ಯದಿ( NEVER(db->aDb[1].pSchema==0) ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ಸ್ಥಿರ SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  ಪೂರ್ಣಾಙ್ಕ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ಪೂರ್ಣಾಙ್ಕ *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ಪೂರ್ಣಾಙ್ಕ mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ಪ್ರತಿಜ್ಞಾ( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  ಯದಿ( pList!=0 ){
    p = pList;
    ಯದಿ( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      ಯದಿ( pList==pTab->pTrigger ){
        pList = 0;
        ಗಚ್ಛ exit_triggers_exist;
      }
      ಯಾವತ್( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    ಕುರು{
      ಯದಿ( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }ಅನ್ಯಥಾ ಯದಿ( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ಪ್ರತಿಜ್ಞಾ( sqlite3IsToplevel(pParse) );
        p->op = op;
        ಯದಿ( IsVirtual(pTab) ){
          ಯದಿ( op!=TK_ಸೇರಿಸು ){
            sqlite3_Error_ಸಂದೇಶ(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }ಅನ್ಯಥಾ{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }ಅನ್ಯಥಾ ಯದಿ( p->bReturning && p->op==TK_ಸೇರಿಸು && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }ಯಾವತ್( p );
  }
exit_triggers_exist:
  ಯದಿ( pMask ){
    *pMask = mask;
  }
  ಪ್ರತ್ಯಾಗಮ (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  ಪೂರ್ಣಾಙ್ಕ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ಪೂರ್ಣಾಙ್ಕ *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಯದಿ( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    ಯದಿ( pMask ) *pMask = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತ್ಯಾಗಮ triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *pTerm         /* A term in the RETURNING clause */
){
  ಪ್ರತಿಜ್ಞಾ( pTerm!=0 );
  ಯದಿ( pTerm->op==TK_ASTERISK ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( pTerm->op!=TK_DOT ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತಿಜ್ಞಾ( pTerm->p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTerm->p_ಎಡ!=0 );
  ಯದಿ( pTerm->p_ಬಲ->op!=TK_ASTERISK ) ಪ್ರತ್ಯಾಗಮ 0;
  sqlite3_Error_ಸಂದೇಶ(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew = 0;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Old_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( NEVER(p_Old_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಅನುವರ್ತ;
    ಯದಿ( isAsteriskTerm(pParse, p_Old_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಪೂರ್ಣಾಙ್ಕ jj;
      ಪ್ರತಿ(jj=0; jj<pTab->nCol; jj++){
        ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
        ಯದಿ( IsHiddenColumn(pTab->aCol+jj) ) ಅನುವರ್ತ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_New_ಅಭಿವ್ಯಕ್ತಿ);
        ಯದಿ( !db->mallocFailed ){
          ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }ಅನ್ಯಥಾ{
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Old_ಅಭಿವ್ಯಕ್ತಿ, 0);
      pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_New_ಅಭಿವ್ಯಕ್ತಿ);
      ಯದಿ( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  UNUSED_PARAMETER(NotUsed);
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ)
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) );
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  ಪೂರ್ಣಾಙ್ಕ i;
  SrcList *pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
    ಯದಿ( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3ProcessReturningSubqueries(
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, pEList);
  ಯದಿ( w.eCode ){
    w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಙ್ಕ regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  ಸಙ್ಘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಯದಿ( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  ಯದಿ( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    ಪ್ರತ್ಯಾಗಮ;
  }
  memset(&sSelect, 0, ಪರಿಮಾಣ(sSelect));
  memset(&uSrc, 0, ಪರಿಮಾಣ(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  ಯದಿ( pParse->nErr==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  ಯದಿ( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
    ಯದಿ( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_ಅಭಿವ್ಯಕ್ತಿ;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪೂರ್ಣಾಙ್ಕ nCol = pNew->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಪೂರ್ಣಾಙ್ಕ reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      ಪ್ರತಿ(i=0; i<nCol; i++){
        ಅಭಿವ್ಯಕ್ತಿ *pCol = pNew->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಪ್ರತಿಜ್ಞಾ( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Factorable(pParse, pCol, reg+i);
        ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  ಪೂರ್ಣಾಙ್ಕ orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ಪ್ರತಿಜ್ಞಾ( pParse->pTriggerTab && pParse->pToplevel );
  ಪ್ರತಿಜ್ಞಾ( pStepList );
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿ(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    ಪ್ರತಿಜ್ಞಾ( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    ಯದಿ( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    ವಿಕಲ್ಪ( pStep->op ){
      ಪಕ್ಷ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pStep->p_ಅಭಿವ್ಯಕ್ತಿ_List, 0), 
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ಎಣಿಕೆ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_ಸೇರಿಸು: {
        sqlite3_ಸೇರಿಸು(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ಎಣಿಕೆ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ಎಣಿಕೆ);
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: ಪ್ರತಿಜ್ಞಾ( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ಭಙ್ಗ;
      }
    } 
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *onErrorText(ಪೂರ್ಣಾಙ್ಕ onError){
  ವಿಕಲ್ಪ( onError ){
    ಪಕ್ಷ OE_Abort:    ಪ್ರತ್ಯಾಗಮ "abort";
    ಪಕ್ಷ OE_Rollback: ಪ್ರತ್ಯಾಗಮ "rollback";
    ಪಕ್ಷ OE_Fail:     ಪ್ರತ್ಯಾಗಮ "fail";
    ಪಕ್ಷ OE_Replace:  ಪ್ರತ್ಯಾಗಮ "replace";
    ಪಕ್ಷ OE_Ignore:   ಪ್ರತ್ಯಾಗಮ "ignore";
    ಪಕ್ಷ OE_Default:  ಪ್ರತ್ಯಾಗಮ "default";
  }
  ಪ್ರತ್ಯಾಗಮ "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ಸ್ಥಿರ ಶೂನ್ಯ transferParseError(Parse *pTo, Parse *pFrom){
  ಪ್ರತಿಜ್ಞಾ( pFrom->z_Err_ಸಂದೇಶ==0 || pFrom->nErr );
  ಪ್ರತಿಜ್ಞಾ( pTo->z_Err_ಸಂದೇಶ==0 || pTo->nErr );
  ಯದಿ( pTo->nErr==0 ){
    pTo->z_Err_ಸಂದೇಶ = pFrom->z_Err_ಸಂದೇಶ;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }ಅನ್ಯಥಾ{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_ಸಂದೇಶ);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ಸ್ಥಿರ TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  ಪೂರ್ಣಾಙ್ಕ orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_ತುದಿ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  ಅಭಿವ್ಯಕ್ತಿ *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  ಪೂರ್ಣಾಙ್ಕ iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  ಪೂರ್ಣಾಙ್ಕ nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_ತುದಿ = pParse;
  ಪ್ರತಿ(nDepth=0; p_ತುದಿ->pOuterParse; p_ತುದಿ = p_ತುದಿ->pOuterParse, nDepth++){}
  ಯದಿ( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "triggers nested too deep");
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  p_ತುದಿ = sqlite3ParseToplevel(pParse);
  ಪ್ರತಿಜ್ಞಾ( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  ಪ್ರತಿಜ್ಞಾ( p_ತುದಿ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(TriggerPrg));
  ಯದಿ( !pPrg ) ಪ್ರತ್ಯಾಗಮ 0;
  pPrg->pNext = p_ತುದಿ->pTriggerPrg;
  p_ತುದಿ->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(SubProgram));
  ಯದಿ( !pProgram ) ಪ್ರತ್ಯಾಗಮ 0;
  sqlite3VdbeLinkSubProgram(p_ತುದಿ->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_ತುದಿ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  ಯದಿ( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ಸೇರಿಸು ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    ಯದಿ( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    ಯದಿ( pTrigger->pWhen ){
      pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pTrigger->pWhen, 0);
      ಯದಿ( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    ಯದಿ( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    ಯದಿ( pParse->nErr==0 ){
      ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_ತುದಿ->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (ಶೂನ್ಯ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }ಅನ್ಯಥಾ{
    transferParseError(pParse, &sSubParse);
  }

  ಪ್ರತಿಜ್ಞಾ( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  ಪ್ರತ್ಯಾಗಮ pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ಸ್ಥಿರ TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  ಪೂರ್ಣಾಙ್ಕ orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_ಬೇರು = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ಪ್ರತಿಜ್ಞಾ( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  ಪ್ರತಿ(pPrg=p_ಬೇರು->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  ಯದಿ( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  ಪ್ರತ್ಯಾಗಮ pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
ಶೂನ್ಯ sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಙ್ಕ reg,             /* Reg array containing OLD.* and NEW.* values */
  ಪೂರ್ಣಾಙ್ಕ orconf,          /* ON CONFLICT policy */
  ಪೂರ್ಣಾಙ್ಕ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  ಪ್ರತಿಜ್ಞಾ( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  ಯದಿ( pPrg ){
    ಪೂರ್ಣಾಙ್ಕ bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (ಧ್ರುವ ವರ್ಣ *)pPrg->pProgram, P4_SUBPROGRAM);
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
ಶೂನ್ಯ sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  ಪೂರ್ಣಾಙ್ಕ op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ಪೂರ್ಣಾಙ್ಕ tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಙ್ಕ reg,             /* The first in an array of registers (see above) */
  ಪೂರ್ಣಾಙ್ಕ orconf,          /* ON CONFLICT policy */
  ಪೂರ್ಣಾಙ್ಕ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ಪ್ರತಿಜ್ಞಾ( op==TK_UPDATE || op==TK_ಸೇರಿಸು || op==TK_DELETE );
  ಪ್ರತಿಜ್ಞಾ( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ಪ್ರತಿಜ್ಞಾ( (op==TK_UPDATE)==(pChanges!=0) );

  ಪ್ರತಿ(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    ಪ್ರತಿಜ್ಞಾ( p->pSchema!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->pTabSchema!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    ಯದಿ( (p->op==op || (p->bReturning && p->op==TK_ಸೇರಿಸು && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      ಯದಿ( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3IsToplevel(pParse) ){
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
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ಪೂರ್ಣಾಙ್ಕ isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  ಪೂರ್ಣಾಙ್ಕ tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಙ್ಕ orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  ಪ್ರತಿಜ್ಞಾ( isNew==1 || isNew==0 );
  ಯದಿ( IsView(pTab) ){
    ಪ್ರತ್ಯಾಗಮ 0xffffffff;
  }
  ಪ್ರತಿ(p=pTrigger; p; p=p->pNext){
    ಯದಿ( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      ಯದಿ( p->bReturning ){
        mask = 0xffffffff;
      }ಅನ್ಯಥಾ{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        ಯದಿ( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
