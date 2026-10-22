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
#அந்தர்பாவ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
வெறுமை sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  போது( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_கோவை_Delete(db, pTmp->pWhere);
    sqlite3_கோவை_List_Delete(db, pTmp->p_கோவை_List);
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

  ப்ரதிஜ்ஞா( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  போது( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    எனில்( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }இல்லையெனில் எனில்( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      ப்ரதிஜ்ஞா( pParse->db->pVtabCtx==0 );
#endif
      ப்ரதிஜ்ஞா( pParse->bReturning );
      ப்ரதிஜ்ஞா( !pParse->isCreate );
      ப்ரதிஜ்ஞா( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#எனில் 0
  எனில்( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    க்கு(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  திரும்பு pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
வெறுமை sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  முழு tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  முழு op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  கோவை *pWhen,        /* WHEN clause */
  முழு isTemp,         /* True if the TEMPORARY keyword is present */
  முழு noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  எழுத்து *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  முழு iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ப்ரதிஜ்ஞா( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ப்ரதிஜ்ஞா( pName2!=0 );
  ப்ரதிஜ்ஞா( op==TK_செருகு || op==TK_UPDATE || op==TK_DELETE );
  ப்ரதிஜ்ஞா( op>0 && op<0xff );
  எனில்( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    எனில்( pName2->n>0 ){
      sqlite3_Error_செய்தி(pParse, "temporary trigger may not have qualified name");
      செல் trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }இல்லையெனில்{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    எனில்( iDb<0 ){
      செல் trigger_cleanup;
    }
  }
  எனில்( !pTableName || db->mallocFailed ){
    செல் trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  எனில்( db->init.busy && iDb!=1 ){
    ப்ரதிஜ்ஞா( pTableName->a[0].fg.fixedSchema==0 );
    ப்ரதிஜ்ஞா( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  எனில்( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  எனில்( db->mallocFailed ) செல் trigger_cleanup;
  ப்ரதிஜ்ஞா( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  எனில்( sqlite3FixSrcList(&sFix, pTableName) ){
    செல் trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  எனில்( !pTab ){
    /* The table does not exist. */
    செல் trigger_orphan_error;
  }
  எனில்( IsVirtual(pTab) ){
    sqlite3_Error_செய்தி(pParse, "cannot create triggers on virtual tables");
    செல் trigger_orphan_error;
  }
  எனில்( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_செய்தி(pParse, "cannot create triggers on shadow tables");
    செல் trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  எனில்( zName==0 ){
    ப்ரதிஜ்ஞா( db->mallocFailed );
    செல் trigger_cleanup;
  }
  எனில்( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    செல் trigger_cleanup;
  }
  ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
  எனில்( !IN_RENAME_OBJECT ){
    எனில்( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      எனில்( !noErr ){
        sqlite3_Error_செய்தி(pParse, "trigger %T already exists", pName);
      }இல்லையெனில் எனில்( db->init.busy ){
        sqlite3_Error_செய்தி(pParse,""); /* Err msg generated by corruptSchema() */
      }இல்லையெனில்{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      செல் trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#எனில் !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  எனில்( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_செய்தி(pParse, "cannot create trigger on system table");
    செல் trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  எனில்( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_செய்தி(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    செல் trigger_orphan_error;
  }
  எனில்( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_செய்தி(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    செல் trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  எனில்( !IN_RENAME_OBJECT ){
    முழு iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    முழு code = SQLITE_CREATE_TRIGGER;
    மாறிலி எழுத்து *zDb = db->aDb[iTabDb].zDbSName;
    மாறிலி எழுத்து *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    எனில்( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    எனில்( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      செல் trigger_cleanup;
    }
    எனில்( sqlite3AuthCheck(pParse, SQLITE_செருகு, SCHEMA_TABLE(iTabDb),0,zDb)){
      செல் trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  எனில் (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, அளவு(Trigger));
  எனில்( pTrigger==0 ) செல் trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  எனில்( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }இல்லையெனில்{
    pTrigger->pWhen = sqlite3_கோவை_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ப்ரதிஜ்ஞா( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_கோவை_Delete(db, pWhen);
  எனில்( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pParse->pNewTrigger==pTrigger );
  }
  திரும்பு;

trigger_orphan_error:
  எனில்( db->init.iDb==1 ){
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
  செல் trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
வெறுமை sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  எழுத்து *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  முழு iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  எனில்( NEVER(pParse->nErr) || !pTrig ) செல் triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  ப்ரதிஜ்ஞா( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  க்கு(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  எனில்( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_செய்தி(pParse, "trigger \"%w\" contains too many steps", zName);
    செல் triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  எனில்( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_கோவை(&sFix, pTrig->pWhen) 
  ){
    செல் triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  எனில்( IN_RENAME_OBJECT ){
    ப்ரதிஜ்ஞா( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }இல்லையெனில்
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  எனில்( !db->init.busy ){
    Vdbe *v;
    எழுத்து *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    எனில்( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      க்கு(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        எனில்( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_செய்தி(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          செல் triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    எனில்( v==0 ) செல் triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (எழுத்து*)pAll->z, pAll->n);
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

  எனில்( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ப்ரதிஜ்ஞா( pLink!=0 );
    pTrig = sqlite3_Hash_செருகு(pHash, zName, pTrig);
    எனில்( pTrig ){
      sqlite3OomFault(db);
    }இல்லையெனில் எனில்( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      ப்ரதிஜ்ஞா( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ப்ரதிஜ்ஞா( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
நிலையான எழுத்து *triggerSpanDup(sqlite3 *db, மாறிலி எழுத்து *zStart, மாறிலி எழுத்து *zEnd){
  எழுத்து *z = sqlite3DbSpanDup(db, zStart, zEnd);
  முழு i;
  எனில்( z ) க்கு(i=0; z[i]; i++) எனில்( sqlite3Isspace(z[i]) ) z[i] = ' ';
  திரும்பு z;
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
  மாறிலி எழுத்து *zStart,         /* Start of SQL text */
  மாறிலி எழுத்து *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, அளவு(TriggerStep));
  எனில்( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    திரும்பு 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  திரும்பு pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
நிலையான TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  மாறிலி எழுத்து *zStart,         /* Start of SQL text */
  மாறிலி எழுத்து *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  எனில்( pParse->nErr==0 ){
    எனில்( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_செய்தி(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }இல்லையெனில்{
      pTriggerStep = sqlite3DbMallocZero(db, அளவு(TriggerStep));
      எனில்( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        எனில்( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  திரும்பு pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_செருகு_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  மாறிலி எழுத்து *zStart, /* Start of SQL text */
  மாறிலி எழுத்து *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  ப்ரதிஜ்ஞா(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_செருகு, pTabList, zStart, zEnd);
  எனில்( pTriggerStep ){
    எனில்( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }இல்லையெனில்{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    எனில்( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }இல்லையெனில்{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  திரும்பு pTriggerStep;
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
  கோவை_List *pEList,    /* The SET clause: list of column and new values */
  கோவை *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  மாறிலி எழுத்து *zStart,  /* Start of SQL text */
  மாறிலி எழுத்து *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  எனில்( pTriggerStep ){
    SrcList *pFromDup = 0;
    எனில்( IN_RENAME_OBJECT ){
      pTriggerStep->p_கோவை_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }இல்லையெனில்{
      pTriggerStep->p_கோவை_List = sqlite3_கோவை_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_கோவை_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    எனில்( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    எனில்( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }இல்லையெனில்{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_கோவை_List_Delete(db, pEList);
  sqlite3_கோவை_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  திரும்பு pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  கோவை *pWhere,           /* The WHERE clause */
  மாறிலி எழுத்து *zStart,     /* Start of SQL text */
  மாறிலி எழுத்து *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  எனில்( pTriggerStep ){
    எனில்( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }இல்லையெனில்{
      pTriggerStep->pWhere = sqlite3_கோவை_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_கோவை_Delete(db, pWhere);
  திரும்பு pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
வெறுமை sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  எனில்( pTrigger==0 || pTrigger->bReturning ) திரும்பு;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_கோவை_Delete(db, pTrigger->pWhen);
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
வெறுமை sqlite3DropTrigger(Parse *pParse, SrcList *pName, முழு noErr){
  Trigger *pTrigger = 0;
  முழு i;
  மாறிலி எழுத்து *zDb;
  மாறிலி எழுத்து *zName;
  sqlite3 *db = pParse->db;

  எனில்( db->mallocFailed ) செல் drop_trigger_cleanup;
  எனில்( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    செல் drop_trigger_cleanup;
  }

  ப்ரதிஜ்ஞா( pName->nSrc==1 );
  ப்ரதிஜ்ஞா( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ப்ரதிஜ்ஞா( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  க்கு(i=OMIT_TEMPDB; i<db->nDb; i++){
    முழு j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    எனில்( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) தொடர்;
    ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    எனில்( pTrigger ) நிறுத்து;
  }
  எனில்( !pTrigger ){
    எனில்( !noErr ){
      sqlite3_Error_செய்தி(pParse, "no such trigger: %S", pName->a);
    }இல்லையெனில்{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    செல் drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
நிலையான Table *tableOfTrigger(Trigger *pTrigger){
  திரும்பு sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
வெறுமை sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  முழு iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  ப்ரதிஜ்ஞா( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  ப்ரதிஜ்ஞா( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  எனில்( pTable ){
    முழு code = SQLITE_DROP_TRIGGER;
    மாறிலி எழுத்து *zDb = db->aDb[iDb].zDbSName;
    மாறிலி எழுத்து *zTab = SCHEMA_TABLE(iDb);
    எனில்( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    எனில்( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      திரும்பு;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  எனில்( (v = sqlite3GetVdbe(pParse))!=0 ){
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
வெறுமை sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, முழு iDb, மாறிலி எழுத்து *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_செருகு(pHash, zName, 0);
  எனில்( ALWAYS(pTrigger) ){
    எனில்( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      எனில்( pTab ){
        Trigger **pp;
        க்கு(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          எனில்( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            நிறுத்து;
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
நிலையான முழு checkColumnOverlap(IdList *pIdList, கோவை_List *pEList){
  முழு e;
  எனில்( pIdList==0 || NEVER(pEList==0) ) திரும்பு 1;
  க்கு(e=0; e<pEList->n_கோவை; e++){
    எனில்( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) திரும்பு 1;
  }
  திரும்பு 0; 
}

/*
** Return true if any TEMP triggers exist
*/
நிலையான முழு tempTriggersExist(sqlite3 *db){
  எனில்( NEVER(db->aDb[1].pSchema==0) ) திரும்பு 0;
  எனில்( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) திரும்பு 0;
  திரும்பு 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
நிலையான SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  முழு op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  கோவை_List *pChanges,     /* Columns that change in an UPDATE statement */
  முழு *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  முழு mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ப்ரதிஜ்ஞா( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  எனில்( pList!=0 ){
    p = pList;
    எனில்( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      எனில்( pList==pTab->pTrigger ){
        pList = 0;
        செல் exit_triggers_exist;
      }
      போது( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    செய்{
      எனில்( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }இல்லையெனில் எனில்( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ப்ரதிஜ்ஞா( sqlite3IsToplevel(pParse) );
        p->op = op;
        எனில்( IsVirtual(pTab) ){
          எனில்( op!=TK_செருகு ){
            sqlite3_Error_செய்தி(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }இல்லையெனில்{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }இல்லையெனில் எனில்( p->bReturning && p->op==TK_செருகு && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }போது( p );
  }
exit_triggers_exist:
  எனில்( pMask ){
    *pMask = mask;
  }
  திரும்பு (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  முழு op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  கோவை_List *pChanges,     /* Columns that change in an UPDATE statement */
  முழு *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ப்ரதிஜ்ஞா( pTab!=0 );
  எனில்( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    எனில்( pMask ) *pMask = 0;
    திரும்பு 0;
  }
  திரும்பு triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
நிலையான முழு isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  கோவை *pTerm         /* A term in the RETURNING clause */
){
  ப்ரதிஜ்ஞா( pTerm!=0 );
  எனில்( pTerm->op==TK_ASTERISK ) திரும்பு 1;
  எனில்( pTerm->op!=TK_DOT ) திரும்பு 0;
  ப்ரதிஜ்ஞா( pTerm->p_வலது!=0 );
  ப்ரதிஜ்ஞா( pTerm->p_இடது!=0 );
  எனில்( pTerm->p_வலது->op!=TK_ASTERISK ) திரும்பு 0;
  sqlite3_Error_செய்தி(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  திரும்பு 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
நிலையான கோவை_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  கோவை_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  கோவை_List *pNew = 0;
  sqlite3 *db = pParse->db;
  முழு i;

  க்கு(i=0; i<pList->n_கோவை; i++){
    கோவை *p_Old_கோவை = pList->a[i].p_கோவை;
    எனில்( NEVER(p_Old_கோவை==0) ) தொடர்;
    எனில்( isAsteriskTerm(pParse, p_Old_கோவை) ){
      முழு jj;
      க்கு(jj=0; jj<pTab->nCol; jj++){
        கோவை *p_New_கோவை;
        எனில்( IsHiddenColumn(pTab->aCol+jj) ) தொடர்;
        p_New_கோவை = sqlite3_கோவை(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_கோவை_List_Append(pParse, pNew, p_New_கோவை);
        எனில்( !db->mallocFailed ){
          கட்டமைப்பு கோவை_List_item *pItem = &pNew->a[pNew->n_கோவை-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }இல்லையெனில்{
      கோவை *p_New_கோவை = sqlite3_கோவை_Dup(db, p_Old_கோவை, 0);
      pNew = sqlite3_கோவை_List_Append(pParse, pNew, p_New_கோவை);
      எனில்( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        கட்டமைப்பு கோவை_List_item *pItem = &pNew->a[pNew->n_கோவை-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  திரும்பு pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
நிலையான முழு sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, கோவை *p_கோவை){
  UNUSED_PARAMETER(NotUsed);
  எனில்( கோவை_Use_XSelect(p_கோவை)
   && (p_கோவை->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( கோவை_Has_Property(p_கோவை, EP_VarSelect) );
    கோவை_Set_Property(p_கோவை, EP_VarSelect);
  }
  திரும்பு WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
நிலையான முழு sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  முழு i;
  SrcList *pSrc;
  ப்ரதிஜ்ஞா( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ப்ரதிஜ்ஞா( pSrc!=0 );
  க்கு(i=0; i<pSrc->nSrc; i++){
    எனில்( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      நிறுத்து;
    }
  }
  திரும்பு WRC_Continue;
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
நிலையான வெறுமை sqlite3ProcessReturningSubqueries(
  கோவை_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, அளவு(w));
  w.x_கோவை_Callback = sqlite3_கோவை_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_கோவை_List(&w, pEList);
  எனில்( w.eCode ){
    w.x_கோவை_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_கோவை_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
நிலையான வெறுமை codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  முழு regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  கோவை_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  ஒன்றிப்பு {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ப்ரதிஜ்ஞா( v!=0 );
  எனில்( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( db->pParse==pParse );
  ப்ரதிஜ்ஞா( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  எனில்( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    திரும்பு;
  }
  memset(&sSelect, 0, அளவு(sSelect));
  memset(&uSrc, 0, அளவு(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_கோவை_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  எனில்( pParse->nErr==0 ){
    ப்ரதிஜ்ஞா( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_கோவை_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  எனில்( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, அளவு(sNC));
    எனில்( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_கோவை;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    எனில்( sqlite3_Resolve_கோவை_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      முழு i;
      முழு nCol = pNew->n_கோவை;
      முழு reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      க்கு(i=0; i<nCol; i++){
        கோவை *pCol = pNew->a[i].p_கோவை;
        ப்ரதிஜ்ஞா( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_கோவை_Code_Factorable(pParse, pCol, reg+i);
        எனில்( sqlite3_கோவை_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_செருகு, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_கோவை_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
நிலையான முழு codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  முழு orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ப்ரதிஜ்ஞா( pParse->pTriggerTab && pParse->pToplevel );
  ப்ரதிஜ்ஞா( pStepList );
  ப்ரதிஜ்ஞா( v!=0 );
  க்கு(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    ப்ரதிஜ்ஞா( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    எனில்( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    தேர்வு( pStep->op ){
      நிலை TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_கோவை_List_Dup(db, pStep->p_கோவை_List, 0), 
          sqlite3_கோவை_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_மொத்தம்);
        நிறுத்து;
      }
      நிலை TK_செருகு: {
        sqlite3_செருகு(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_மொத்தம்);
        நிறுத்து;
      }
      நிலை TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_கோவை_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_மொத்தம்);
        நிறுத்து;
      }
      இயல்பு: ப்ரதிஜ்ஞா( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        நிறுத்து;
      }
    } 
  }

  திரும்பு 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
நிலையான மாறிலி எழுத்து *onErrorText(முழு onError){
  தேர்வு( onError ){
    நிலை OE_Abort:    திரும்பு "abort";
    நிலை OE_Rollback: திரும்பு "rollback";
    நிலை OE_Fail:     திரும்பு "fail";
    நிலை OE_Replace:  திரும்பு "replace";
    நிலை OE_Ignore:   திரும்பு "ignore";
    நிலை OE_Default:  திரும்பு "default";
  }
  திரும்பு "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
நிலையான வெறுமை transferParseError(Parse *pTo, Parse *pFrom){
  ப்ரதிஜ்ஞா( pFrom->z_Err_செய்தி==0 || pFrom->nErr );
  ப்ரதிஜ்ஞா( pTo->z_Err_செய்தி==0 || pTo->nErr );
  எனில்( pTo->nErr==0 ){
    pTo->z_Err_செய்தி = pFrom->z_Err_செய்தி;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }இல்லையெனில்{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_செய்தி);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
நிலையான TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  முழு orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_உச்சி;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  கோவை *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  முழு iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  முழு nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_உச்சி = pParse;
  க்கு(nDepth=0; p_உச்சி->pOuterParse; p_உச்சி = p_உச்சி->pOuterParse, nDepth++){}
  எனில்( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_செய்தி(pParse, "triggers nested too deep");
    திரும்பு 0;
  }

  p_உச்சி = sqlite3ParseToplevel(pParse);
  ப்ரதிஜ்ஞா( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  ப்ரதிஜ்ஞா( p_உச்சி->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, அளவு(TriggerPrg));
  எனில்( !pPrg ) திரும்பு 0;
  pPrg->pNext = p_உச்சி->pTriggerPrg;
  p_உச்சி->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, அளவு(SubProgram));
  எனில்( !pProgram ) திரும்பு 0;
  sqlite3VdbeLinkSubProgram(p_உச்சி->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, அளவு(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_உச்சி;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  எனில்( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_செருகு ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    எனில்( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    எனில்( pTrigger->pWhen ){
      pWhen = sqlite3_கோவை_Dup(db, pTrigger->pWhen, 0);
      எனில்( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_கோவை_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_கோவை_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_கோவை_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    எனில்( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    எனில்( pParse->nErr==0 ){
      ப்ரதிஜ்ஞா( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_உச்சி->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (வெறுமை *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }இல்லையெனில்{
    transferParseError(pParse, &sSubParse);
  }

  ப்ரதிஜ்ஞா( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  திரும்பு pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
நிலையான TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  முழு orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_வேர் = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ப்ரதிஜ்ஞா( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  க்கு(pPrg=p_வேர்->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  எனில்( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  திரும்பு pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
வெறுமை sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  முழு reg,             /* Reg array containing OLD.* and NEW.* values */
  முழு orconf,          /* ON CONFLICT policy */
  முழு ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  ப்ரதிஜ்ஞா( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  எனில்( pPrg ){
    முழு bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (மாறிலி எழுத்து *)pPrg->pProgram, P4_SUBPROGRAM);
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
வெறுமை sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  முழு op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  கோவை_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  முழு tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  முழு reg,             /* The first in an array of registers (see above) */
  முழு orconf,          /* ON CONFLICT policy */
  முழு ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ப்ரதிஜ்ஞா( op==TK_UPDATE || op==TK_செருகு || op==TK_DELETE );
  ப்ரதிஜ்ஞா( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ப்ரதிஜ்ஞா( (op==TK_UPDATE)==(pChanges!=0) );

  க்கு(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    ப்ரதிஜ்ஞா( p->pSchema!=0 );
    ப்ரதிஜ்ஞா( p->pTabSchema!=0 );
    ப்ரதிஜ்ஞா( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    எனில்( (p->op==op || (p->bReturning && p->op==TK_செருகு && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      எனில்( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }இல்லையெனில் எனில்( sqlite3IsToplevel(pParse) ){
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
  கோவை_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  முழு isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  முழு tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  முழு orconf           /* Default ON CONFLICT policy for trigger steps */
){
  மாறிலி முழு op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  ப்ரதிஜ்ஞா( isNew==1 || isNew==0 );
  எனில்( IsView(pTab) ){
    திரும்பு 0xffffffff;
  }
  க்கு(p=pTrigger; p; p=p->pNext){
    எனில்( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      எனில்( p->bReturning ){
        mask = 0xffffffff;
      }இல்லையெனில்{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        எனில்( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  திரும்பு mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
