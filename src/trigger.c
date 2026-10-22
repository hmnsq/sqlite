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
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
શૂન્ય sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  જ્યારે( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_પદાવલિ_Delete(db, pTmp->pWhere);
    sqlite3_પદાવલિ_List_Delete(db, pTmp->p_પદાવલિ_List);
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

  પ્રતિજ્ઞા( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  જ્યારે( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    જો( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }નહીંતર જો( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      પ્રતિજ્ઞા( pParse->db->pVtabCtx==0 );
#endif
      પ્રતિજ્ઞા( pParse->bReturning );
      પ્રતિજ્ઞા( !pParse->isCreate );
      પ્રતિજ્ઞા( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#જો 0
  જો( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    માટે(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  પાછા pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
શૂન્ય sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  પૂર્ણાંક tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  પૂર્ણાંક op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  પદાવલિ *pWhen,        /* WHEN clause */
  પૂર્ણાંક isTemp,         /* True if the TEMPORARY keyword is present */
  પૂર્ણાંક noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  અક્ષર *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  પૂર્ણાંક iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  પ્રતિજ્ઞા( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  પ્રતિજ્ઞા( pName2!=0 );
  પ્રતિજ્ઞા( op==TK_ઉમેરો || op==TK_UPDATE || op==TK_DELETE );
  પ્રતિજ્ઞા( op>0 && op<0xff );
  જો( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    જો( pName2->n>0 ){
      sqlite3_Error_સંદેશ(pParse, "temporary trigger may not have qualified name");
      જાઓ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }નહીંતર{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    જો( iDb<0 ){
      જાઓ trigger_cleanup;
    }
  }
  જો( !pTableName || db->mallocFailed ){
    જાઓ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  જો( db->init.busy && iDb!=1 ){
    પ્રતિજ્ઞા( pTableName->a[0].fg.fixedSchema==0 );
    પ્રતિજ્ઞા( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  જો( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  જો( db->mallocFailed ) જાઓ trigger_cleanup;
  પ્રતિજ્ઞા( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  જો( sqlite3FixSrcList(&sFix, pTableName) ){
    જાઓ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  જો( !pTab ){
    /* The table does not exist. */
    જાઓ trigger_orphan_error;
  }
  જો( IsVirtual(pTab) ){
    sqlite3_Error_સંદેશ(pParse, "cannot create triggers on virtual tables");
    જાઓ trigger_orphan_error;
  }
  જો( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_સંદેશ(pParse, "cannot create triggers on shadow tables");
    જાઓ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  જો( zName==0 ){
    પ્રતિજ્ઞા( db->mallocFailed );
    જાઓ trigger_cleanup;
  }
  જો( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    જાઓ trigger_cleanup;
  }
  પ્રતિજ્ઞા( sqlite3SchemaMutexHeld(db, iDb, 0) );
  જો( !IN_RENAME_OBJECT ){
    જો( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      જો( !noErr ){
        sqlite3_Error_સંદેશ(pParse, "trigger %T already exists", pName);
      }નહીંતર જો( db->init.busy ){
        sqlite3_Error_સંદેશ(pParse,""); /* Err msg generated by corruptSchema() */
      }નહીંતર{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      જાઓ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#જો !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  જો( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_સંદેશ(pParse, "cannot create trigger on system table");
    જાઓ trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  જો( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_સંદેશ(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    જાઓ trigger_orphan_error;
  }
  જો( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_સંદેશ(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    જાઓ trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  જો( !IN_RENAME_OBJECT ){
    પૂર્ણાંક iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    પૂર્ણાંક code = SQLITE_CREATE_TRIGGER;
    અચલ અક્ષર *zDb = db->aDb[iTabDb].zDbSName;
    અચલ અક્ષર *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    જો( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    જો( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      જાઓ trigger_cleanup;
    }
    જો( sqlite3AuthCheck(pParse, SQLITE_ઉમેરો, SCHEMA_TABLE(iTabDb),0,zDb)){
      જાઓ trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  જો (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, કદ(Trigger));
  જો( pTrigger==0 ) જાઓ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  જો( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }નહીંતર{
    pTrigger->pWhen = sqlite3_પદાવલિ_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  પ્રતિજ્ઞા( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_પદાવલિ_Delete(db, pWhen);
  જો( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }નહીંતર{
    પ્રતિજ્ઞા( pParse->pNewTrigger==pTrigger );
  }
  પાછા;

trigger_orphan_error:
  જો( db->init.iDb==1 ){
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
  જાઓ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
શૂન્ય sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  અક્ષર *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  પૂર્ણાંક iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  જો( NEVER(pParse->nErr) || !pTrig ) જાઓ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  પ્રતિજ્ઞા( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  માટે(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  જો( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_સંદેશ(pParse, "trigger \"%w\" contains too many steps", zName);
    જાઓ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  જો( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_પદાવલિ(&sFix, pTrig->pWhen) 
  ){
    જાઓ triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  જો( IN_RENAME_OBJECT ){
    પ્રતિજ્ઞા( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }નહીંતર
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  જો( !db->init.busy ){
    Vdbe *v;
    અક્ષર *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    જો( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      માટે(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        જો( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_સંદેશ(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          જાઓ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    જો( v==0 ) જાઓ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (અક્ષર*)pAll->z, pAll->n);
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

  જો( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    પ્રતિજ્ઞા( sqlite3SchemaMutexHeld(db, iDb, 0) );
    પ્રતિજ્ઞા( pLink!=0 );
    pTrig = sqlite3_Hash_ઉમેરો(pHash, zName, pTrig);
    જો( pTrig ){
      sqlite3OomFault(db);
    }નહીંતર જો( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      પ્રતિજ્ઞા( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  પ્રતિજ્ઞા( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
સ્થિર અક્ષર *triggerSpanDup(sqlite3 *db, અચલ અક્ષર *zStart, અચલ અક્ષર *zEnd){
  અક્ષર *z = sqlite3DbSpanDup(db, zStart, zEnd);
  પૂર્ણાંક i;
  જો( z ) માટે(i=0; z[i]; i++) જો( sqlite3Isspace(z[i]) ) z[i] = ' ';
  પાછા z;
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
  અચલ અક્ષર *zStart,         /* Start of SQL text */
  અચલ અક્ષર *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, કદ(TriggerStep));
  જો( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    પાછા 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  પાછા pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
સ્થિર TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  અચલ અક્ષર *zStart,         /* Start of SQL text */
  અચલ અક્ષર *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  જો( pParse->nErr==0 ){
    જો( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_સંદેશ(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }નહીંતર{
      pTriggerStep = sqlite3DbMallocZero(db, કદ(TriggerStep));
      જો( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        જો( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  પાછા pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_ઉમેરો_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  અચલ અક્ષર *zStart, /* Start of SQL text */
  અચલ અક્ષર *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  પ્રતિજ્ઞા(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_ઉમેરો, pTabList, zStart, zEnd);
  જો( pTriggerStep ){
    જો( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }નહીંતર{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    જો( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }નહીંતર{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  પાછા pTriggerStep;
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
  પદાવલિ_List *pEList,    /* The SET clause: list of column and new values */
  પદાવલિ *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  અચલ અક્ષર *zStart,  /* Start of SQL text */
  અચલ અક્ષર *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  જો( pTriggerStep ){
    SrcList *pFromDup = 0;
    જો( IN_RENAME_OBJECT ){
      pTriggerStep->p_પદાવલિ_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }નહીંતર{
      pTriggerStep->p_પદાવલિ_List = sqlite3_પદાવલિ_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_પદાવલિ_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    જો( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    જો( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }નહીંતર{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_પદાવલિ_List_Delete(db, pEList);
  sqlite3_પદાવલિ_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  પાછા pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  પદાવલિ *pWhere,           /* The WHERE clause */
  અચલ અક્ષર *zStart,     /* Start of SQL text */
  અચલ અક્ષર *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  જો( pTriggerStep ){
    જો( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }નહીંતર{
      pTriggerStep->pWhere = sqlite3_પદાવલિ_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_પદાવલિ_Delete(db, pWhere);
  પાછા pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
શૂન્ય sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  જો( pTrigger==0 || pTrigger->bReturning ) પાછા;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_પદાવલિ_Delete(db, pTrigger->pWhen);
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
શૂન્ય sqlite3DropTrigger(Parse *pParse, SrcList *pName, પૂર્ણાંક noErr){
  Trigger *pTrigger = 0;
  પૂર્ણાંક i;
  અચલ અક્ષર *zDb;
  અચલ અક્ષર *zName;
  sqlite3 *db = pParse->db;

  જો( db->mallocFailed ) જાઓ drop_trigger_cleanup;
  જો( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    જાઓ drop_trigger_cleanup;
  }

  પ્રતિજ્ઞા( pName->nSrc==1 );
  પ્રતિજ્ઞા( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  પ્રતિજ્ઞા( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  માટે(i=OMIT_TEMPDB; i<db->nDb; i++){
    પૂર્ણાંક j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    જો( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) ચાલુ;
    પ્રતિજ્ઞા( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    જો( pTrigger ) તોડો;
  }
  જો( !pTrigger ){
    જો( !noErr ){
      sqlite3_Error_સંદેશ(pParse, "no such trigger: %S", pName->a);
    }નહીંતર{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    જાઓ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
સ્થિર Table *tableOfTrigger(Trigger *pTrigger){
  પાછા sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
શૂન્ય sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  પ્રતિજ્ઞા( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  પ્રતિજ્ઞા( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  જો( pTable ){
    પૂર્ણાંક code = SQLITE_DROP_TRIGGER;
    અચલ અક્ષર *zDb = db->aDb[iDb].zDbSName;
    અચલ અક્ષર *zTab = SCHEMA_TABLE(iDb);
    જો( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    જો( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      પાછા;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  જો( (v = sqlite3GetVdbe(pParse))!=0 ){
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
શૂન્ય sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, પૂર્ણાંક iDb, અચલ અક્ષર *zName){
  Trigger *pTrigger;
  Hash *pHash;

  પ્રતિજ્ઞા( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ઉમેરો(pHash, zName, 0);
  જો( ALWAYS(pTrigger) ){
    જો( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      જો( pTab ){
        Trigger **pp;
        માટે(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          જો( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            તોડો;
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
સ્થિર પૂર્ણાંક checkColumnOverlap(IdList *pIdList, પદાવલિ_List *pEList){
  પૂર્ણાંક e;
  જો( pIdList==0 || NEVER(pEList==0) ) પાછા 1;
  માટે(e=0; e<pEList->n_પદાવલિ; e++){
    જો( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) પાછા 1;
  }
  પાછા 0; 
}

/*
** Return true if any TEMP triggers exist
*/
સ્થિર પૂર્ણાંક tempTriggersExist(sqlite3 *db){
  જો( NEVER(db->aDb[1].pSchema==0) ) પાછા 0;
  જો( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) પાછા 0;
  પાછા 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
સ્થિર SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  પૂર્ણાંક op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  પદાવલિ_List *pChanges,     /* Columns that change in an UPDATE statement */
  પૂર્ણાંક *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  પૂર્ણાંક mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  પ્રતિજ્ઞા( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  જો( pList!=0 ){
    p = pList;
    જો( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      જો( pList==pTab->pTrigger ){
        pList = 0;
        જાઓ exit_triggers_exist;
      }
      જ્યારે( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    કરો{
      જો( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }નહીંતર જો( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        પ્રતિજ્ઞા( sqlite3IsToplevel(pParse) );
        p->op = op;
        જો( IsVirtual(pTab) ){
          જો( op!=TK_ઉમેરો ){
            sqlite3_Error_સંદેશ(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }નહીંતર{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }નહીંતર જો( p->bReturning && p->op==TK_ઉમેરો && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }જ્યારે( p );
  }
exit_triggers_exist:
  જો( pMask ){
    *pMask = mask;
  }
  પાછા (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  પૂર્ણાંક op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  પદાવલિ_List *pChanges,     /* Columns that change in an UPDATE statement */
  પૂર્ણાંક *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  પ્રતિજ્ઞા( pTab!=0 );
  જો( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    જો( pMask ) *pMask = 0;
    પાછા 0;
  }
  પાછા triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
સ્થિર પૂર્ણાંક isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  પદાવલિ *pTerm         /* A term in the RETURNING clause */
){
  પ્રતિજ્ઞા( pTerm!=0 );
  જો( pTerm->op==TK_ASTERISK ) પાછા 1;
  જો( pTerm->op!=TK_DOT ) પાછા 0;
  પ્રતિજ્ઞા( pTerm->p_જમણું!=0 );
  પ્રતિજ્ઞા( pTerm->p_ડાબું!=0 );
  જો( pTerm->p_જમણું->op!=TK_ASTERISK ) પાછા 0;
  sqlite3_Error_સંદેશ(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  પાછા 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
સ્થિર પદાવલિ_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  પદાવલિ_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  પદાવલિ_List *pNew = 0;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક i;

  માટે(i=0; i<pList->n_પદાવલિ; i++){
    પદાવલિ *p_Old_પદાવલિ = pList->a[i].p_પદાવલિ;
    જો( NEVER(p_Old_પદાવલિ==0) ) ચાલુ;
    જો( isAsteriskTerm(pParse, p_Old_પદાવલિ) ){
      પૂર્ણાંક jj;
      માટે(jj=0; jj<pTab->nCol; jj++){
        પદાવલિ *p_New_પદાવલિ;
        જો( IsHiddenColumn(pTab->aCol+jj) ) ચાલુ;
        p_New_પદાવલિ = sqlite3_પદાવલિ(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_પદાવલિ_List_Append(pParse, pNew, p_New_પદાવલિ);
        જો( !db->mallocFailed ){
          સંરચના પદાવલિ_List_item *pItem = &pNew->a[pNew->n_પદાવલિ-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }નહીંતર{
      પદાવલિ *p_New_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_Old_પદાવલિ, 0);
      pNew = sqlite3_પદાવલિ_List_Append(pParse, pNew, p_New_પદાવલિ);
      જો( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        સંરચના પદાવલિ_List_item *pItem = &pNew->a[pNew->n_પદાવલિ-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  પાછા pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
સ્થિર પૂર્ણાંક sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, પદાવલિ *p_પદાવલિ){
  UNUSED_PARAMETER(NotUsed);
  જો( પદાવલિ_Use_XSelect(p_પદાવલિ)
   && (p_પદાવલિ->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_VarSelect) );
    પદાવલિ_Set_Property(p_પદાવલિ, EP_VarSelect);
  }
  પાછા WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
સ્થિર પૂર્ણાંક sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  પૂર્ણાંક i;
  SrcList *pSrc;
  પ્રતિજ્ઞા( pSelect!=0 );
  pSrc = pSelect->pSrc;
  પ્રતિજ્ઞા( pSrc!=0 );
  માટે(i=0; i<pSrc->nSrc; i++){
    જો( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      તોડો;
    }
  }
  પાછા WRC_Continue;
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
સ્થિર શૂન્ય sqlite3ProcessReturningSubqueries(
  પદાવલિ_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, કદ(w));
  w.x_પદાવલિ_Callback = sqlite3_પદાવલિ_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_પદાવલિ_List(&w, pEList);
  જો( w.eCode ){
    w.x_પદાવલિ_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_પદાવલિ_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
સ્થિર શૂન્ય codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  પૂર્ણાંક regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  પદાવલિ_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  સંઘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  પ્રતિજ્ઞા( v!=0 );
  જો( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    પાછા;
  }
  પ્રતિજ્ઞા( db->pParse==pParse );
  પ્રતિજ્ઞા( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  જો( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    પાછા;
  }
  memset(&sSelect, 0, કદ(sSelect));
  memset(&uSrc, 0, કદ(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_પદાવલિ_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  જો( pParse->nErr==0 ){
    પ્રતિજ્ઞા( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_પદાવલિ_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  જો( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, કદ(sNC));
    જો( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_પદાવલિ;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    જો( sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      પૂર્ણાંક i;
      પૂર્ણાંક nCol = pNew->n_પદાવલિ;
      પૂર્ણાંક reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      માટે(i=0; i<nCol; i++){
        પદાવલિ *pCol = pNew->a[i].p_પદાવલિ;
        પ્રતિજ્ઞા( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_પદાવલિ_Code_Factorable(pParse, pCol, reg+i);
        જો( sqlite3_પદાવલિ_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_ઉમેરો, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_પદાવલિ_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
સ્થિર પૂર્ણાંક codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  પૂર્ણાંક orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  પ્રતિજ્ઞા( pParse->pTriggerTab && pParse->pToplevel );
  પ્રતિજ્ઞા( pStepList );
  પ્રતિજ્ઞા( v!=0 );
  માટે(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    પ્રતિજ્ઞા( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    જો( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    પસંદગી( pStep->op ){
      સ્થિતિ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_પદાવલિ_List_Dup(db, pStep->p_પદાવલિ_List, 0), 
          sqlite3_પદાવલિ_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ગણતરી);
        તોડો;
      }
      સ્થિતિ TK_ઉમેરો: {
        sqlite3_ઉમેરો(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ગણતરી);
        તોડો;
      }
      સ્થિતિ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_પદાવલિ_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_ગણતરી);
        તોડો;
      }
      મૂળભૂત: પ્રતિજ્ઞા( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        તોડો;
      }
    } 
  }

  પાછા 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
સ્થિર અચલ અક્ષર *onErrorText(પૂર્ણાંક onError){
  પસંદગી( onError ){
    સ્થિતિ OE_Abort:    પાછા "abort";
    સ્થિતિ OE_Rollback: પાછા "rollback";
    સ્થિતિ OE_Fail:     પાછા "fail";
    સ્થિતિ OE_Replace:  પાછા "replace";
    સ્થિતિ OE_Ignore:   પાછા "ignore";
    સ્થિતિ OE_Default:  પાછા "default";
  }
  પાછા "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
સ્થિર શૂન્ય transferParseError(Parse *pTo, Parse *pFrom){
  પ્રતિજ્ઞા( pFrom->z_Err_સંદેશ==0 || pFrom->nErr );
  પ્રતિજ્ઞા( pTo->z_Err_સંદેશ==0 || pTo->nErr );
  જો( pTo->nErr==0 ){
    pTo->z_Err_સંદેશ = pFrom->z_Err_સંદેશ;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }નહીંતર{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_સંદેશ);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
સ્થિર TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  પૂર્ણાંક orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_ટોચ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  પદાવલિ *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  પૂર્ણાંક iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  પૂર્ણાંક nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_ટોચ = pParse;
  માટે(nDepth=0; p_ટોચ->pOuterParse; p_ટોચ = p_ટોચ->pOuterParse, nDepth++){}
  જો( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_સંદેશ(pParse, "triggers nested too deep");
    પાછા 0;
  }

  p_ટોચ = sqlite3ParseToplevel(pParse);
  પ્રતિજ્ઞા( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  પ્રતિજ્ઞા( p_ટોચ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, કદ(TriggerPrg));
  જો( !pPrg ) પાછા 0;
  pPrg->pNext = p_ટોચ->pTriggerPrg;
  p_ટોચ->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, કદ(SubProgram));
  જો( !pProgram ) પાછા 0;
  sqlite3VdbeLinkSubProgram(p_ટોચ->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, કદ(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_ટોચ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  જો( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ઉમેરો ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    જો( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    જો( pTrigger->pWhen ){
      pWhen = sqlite3_પદાવલિ_Dup(db, pTrigger->pWhen, 0);
      જો( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_પદાવલિ_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_પદાવલિ_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_પદાવલિ_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    જો( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    જો( pParse->nErr==0 ){
      પ્રતિજ્ઞા( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_ટોચ->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (શૂન્ય *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }નહીંતર{
    transferParseError(pParse, &sSubParse);
  }

  પ્રતિજ્ઞા( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  પાછા pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
સ્થિર TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  પૂર્ણાંક orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_મૂળ = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  પ્રતિજ્ઞા( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  માટે(pPrg=p_મૂળ->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  જો( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  પાછા pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
શૂન્ય sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  પૂર્ણાંક reg,             /* Reg array containing OLD.* and NEW.* values */
  પૂર્ણાંક orconf,          /* ON CONFLICT policy */
  પૂર્ણાંક ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  પ્રતિજ્ઞા( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  જો( pPrg ){
    પૂર્ણાંક bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (અચલ અક્ષર *)pPrg->pProgram, P4_SUBPROGRAM);
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
શૂન્ય sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  પૂર્ણાંક op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  પદાવલિ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  પૂર્ણાંક tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  પૂર્ણાંક reg,             /* The first in an array of registers (see above) */
  પૂર્ણાંક orconf,          /* ON CONFLICT policy */
  પૂર્ણાંક ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  પ્રતિજ્ઞા( op==TK_UPDATE || op==TK_ઉમેરો || op==TK_DELETE );
  પ્રતિજ્ઞા( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  પ્રતિજ્ઞા( (op==TK_UPDATE)==(pChanges!=0) );

  માટે(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    પ્રતિજ્ઞા( p->pSchema!=0 );
    પ્રતિજ્ઞા( p->pTabSchema!=0 );
    પ્રતિજ્ઞા( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    જો( (p->op==op || (p->bReturning && p->op==TK_ઉમેરો && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      જો( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }નહીંતર જો( sqlite3IsToplevel(pParse) ){
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
  પદાવલિ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  પૂર્ણાંક isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  પૂર્ણાંક tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  પૂર્ણાંક orconf           /* Default ON CONFLICT policy for trigger steps */
){
  અચલ પૂર્ણાંક op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  પ્રતિજ્ઞા( isNew==1 || isNew==0 );
  જો( IsView(pTab) ){
    પાછા 0xffffffff;
  }
  માટે(p=pTrigger; p; p=p->pNext){
    જો( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      જો( p->bReturning ){
        mask = 0xffffffff;
      }નહીંતર{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        જો( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  પાછા mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
