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

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
शून्य sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  याव( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_व्यञ्जकम्_Delete(db, pTmp->pWhere);
    sqlite3_व्यञ्जकम्_List_Delete(db, pTmp->p_व्यञ्जकम्_List);
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

  प्रतिज्ञा( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  याव( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    सचे( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }अञ्ञथा सचे( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      प्रतिज्ञा( pParse->db->pVtabCtx==0 );
#endif
      प्रतिज्ञा( pParse->bReturning );
      प्रतिज्ञा( !pParse->isCreate );
      प्रतिज्ञा( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#सचे 0
  सचे( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    अत्थाय(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  पटिनिवत्त pList;  
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
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  व्यञ्जकम् *pWhen,        /* WHEN clause */
  पूर्णाङ्क isTemp,         /* True if the TEMPORARY keyword is present */
  पूर्णाङ्क noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  वर्ण *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  पूर्णाङ्क iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  प्रतिज्ञा( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  प्रतिज्ञा( pName2!=0 );
  प्रतिज्ञा( op==TK_निवेशय || op==TK_UPDATE || op==TK_DELETE );
  प्रतिज्ञा( op>0 && op<0xff );
  सचे( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    सचे( pName2->n>0 ){
      sqlite3_Error_सन्देशः(pParse, "temporary trigger may not have qualified name");
      गच्छ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }अञ्ञथा{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    सचे( iDb<0 ){
      गच्छ trigger_cleanup;
    }
  }
  सचे( !pTableName || db->mallocFailed ){
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
  सचे( db->init.busy && iDb!=1 ){
    प्रतिज्ञा( pTableName->a[0].fg.fixedSchema==0 );
    प्रतिज्ञा( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  सचे( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  सचे( db->mallocFailed ) गच्छ trigger_cleanup;
  प्रतिज्ञा( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  सचे( sqlite3FixSrcList(&sFix, pTableName) ){
    गच्छ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  सचे( !pTab ){
    /* The table does not exist. */
    गच्छ trigger_orphan_error;
  }
  सचे( IsVirtual(pTab) ){
    sqlite3_Error_सन्देशः(pParse, "cannot create triggers on virtual tables");
    गच्छ trigger_orphan_error;
  }
  सचे( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_सन्देशः(pParse, "cannot create triggers on shadow tables");
    गच्छ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  सचे( zName==0 ){
    प्रतिज्ञा( db->mallocFailed );
    गच्छ trigger_cleanup;
  }
  सचे( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    गच्छ trigger_cleanup;
  }
  प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
  सचे( !IN_RENAME_OBJECT ){
    सचे( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      सचे( !noErr ){
        sqlite3_Error_सन्देशः(pParse, "trigger %T already exists", pName);
      }अञ्ञथा सचे( db->init.busy ){
        sqlite3_Error_सन्देशः(pParse,""); /* Err msg generated by corruptSchema() */
      }अञ्ञथा{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      गच्छ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#सचे !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  सचे( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_सन्देशः(pParse, "cannot create trigger on system table");
    गच्छ trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  सचे( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_सन्देशः(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    गच्छ trigger_orphan_error;
  }
  सचे( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_सन्देशः(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    गच्छ trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  सचे( !IN_RENAME_OBJECT ){
    पूर्णाङ्क iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    पूर्णाङ्क code = SQLITE_CREATE_TRIGGER;
    ध्रुव वर्ण *zDb = db->aDb[iTabDb].zDbSName;
    ध्रुव वर्ण *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    सचे( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    सचे( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      गच्छ trigger_cleanup;
    }
    सचे( sqlite3AuthCheck(pParse, SQLITE_निवेशय, SCHEMA_TABLE(iTabDb),0,zDb)){
      गच्छ trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  सचे (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, परिमाण(Trigger));
  सचे( pTrigger==0 ) गच्छ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  सचे( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }अञ्ञथा{
    pTrigger->pWhen = sqlite3_व्यञ्जकम्_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  प्रतिज्ञा( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_व्यञ्जकम्_Delete(db, pWhen);
  सचे( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }अञ्ञथा{
    प्रतिज्ञा( pParse->pNewTrigger==pTrigger );
  }
  पटिनिवत्त;

trigger_orphan_error:
  सचे( db->init.iDb==1 ){
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
  TriggerStep *pStepList, /* The triggered program */
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
  सचे( NEVER(pParse->nErr) || !pTrig ) गच्छ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  प्रतिज्ञा( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  अत्थाय(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  सचे( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_सन्देशः(pParse, "trigger \"%w\" contains too many steps", zName);
    गच्छ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  सचे( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_व्यञ्जकम्(&sFix, pTrig->pWhen) 
  ){
    गच्छ triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  सचे( IN_RENAME_OBJECT ){
    प्रतिज्ञा( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }अञ्ञथा
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  सचे( !db->init.busy ){
    Vdbe *v;
    वर्ण *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    सचे( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      अत्थाय(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        सचे( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_सन्देशः(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          गच्छ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    सचे( v==0 ) गच्छ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (वर्ण*)pAll->z, pAll->n);
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

  सचे( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
    प्रतिज्ञा( pLink!=0 );
    pTrig = sqlite3_Hash_निवेशय(pHash, zName, pTrig);
    सचे( pTrig ){
      sqlite3OomFault(db);
    }अञ्ञथा सचे( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      प्रतिज्ञा( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  प्रतिज्ञा( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
स्थिर वर्ण *triggerSpanDup(sqlite3 *db, ध्रुव वर्ण *zStart, ध्रुव वर्ण *zEnd){
  वर्ण *z = sqlite3DbSpanDup(db, zStart, zEnd);
  पूर्णाङ्क i;
  सचे( z ) अत्थाय(i=0; z[i]; i++) सचे( sqlite3Isspace(z[i]) ) z[i] = ' ';
  पटिनिवत्त z;
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
  ध्रुव वर्ण *zStart,         /* Start of SQL text */
  ध्रुव वर्ण *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, परिमाण(TriggerStep));
  सचे( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    पटिनिवत्त 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  पटिनिवत्त pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
स्थिर TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ध्रुव वर्ण *zStart,         /* Start of SQL text */
  ध्रुव वर्ण *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  सचे( pParse->nErr==0 ){
    सचे( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_सन्देशः(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }अञ्ञथा{
      pTriggerStep = sqlite3DbMallocZero(db, परिमाण(TriggerStep));
      सचे( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        सचे( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  पटिनिवत्त pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_निवेशय_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ध्रुव वर्ण *zStart, /* Start of SQL text */
  ध्रुव वर्ण *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  प्रतिज्ञा(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_निवेशय, pTabList, zStart, zEnd);
  सचे( pTriggerStep ){
    सचे( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }अञ्ञथा{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    सचे( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }अञ्ञथा{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  पटिनिवत्त pTriggerStep;
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
  व्यञ्जकम्_List *pEList,    /* The SET clause: list of column and new values */
  व्यञ्जकम् *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ध्रुव वर्ण *zStart,  /* Start of SQL text */
  ध्रुव वर्ण *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  सचे( pTriggerStep ){
    SrcList *pFromDup = 0;
    सचे( IN_RENAME_OBJECT ){
      pTriggerStep->p_व्यञ्जकम्_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }अञ्ञथा{
      pTriggerStep->p_व्यञ्जकम्_List = sqlite3_व्यञ्जकम्_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_व्यञ्जकम्_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    सचे( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    सचे( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }अञ्ञथा{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_व्यञ्जकम्_List_Delete(db, pEList);
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  पटिनिवत्त pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  व्यञ्जकम् *pWhere,           /* The WHERE clause */
  ध्रुव वर्ण *zStart,     /* Start of SQL text */
  ध्रुव वर्ण *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  सचे( pTriggerStep ){
    सचे( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }अञ्ञथा{
      pTriggerStep->pWhere = sqlite3_व्यञ्जकम्_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_व्यञ्जकम्_Delete(db, pWhere);
  पटिनिवत्त pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
शून्य sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  सचे( pTrigger==0 || pTrigger->bReturning ) पटिनिवत्त;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
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

  सचे( db->mallocFailed ) गच्छ drop_trigger_cleanup;
  सचे( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    गच्छ drop_trigger_cleanup;
  }

  प्रतिज्ञा( pName->nSrc==1 );
  प्रतिज्ञा( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  प्रतिज्ञा( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  अत्थाय(i=OMIT_TEMPDB; i<db->nDb; i++){
    पूर्णाङ्क j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    सचे( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) अनुवर्त;
    प्रतिज्ञा( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    सचे( pTrigger ) भङ्ग;
  }
  सचे( !pTrigger ){
    सचे( !noErr ){
      sqlite3_Error_सन्देशः(pParse, "no such trigger: %S", pName->a);
    }अञ्ञथा{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
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
स्थिर Table *tableOfTrigger(Trigger *pTrigger){
  पटिनिवत्त sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
शून्य sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  प्रतिज्ञा( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  प्रतिज्ञा( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  सचे( pTable ){
    पूर्णाङ्क code = SQLITE_DROP_TRIGGER;
    ध्रुव वर्ण *zDb = db->aDb[iDb].zDbSName;
    ध्रुव वर्ण *zTab = SCHEMA_TABLE(iDb);
    सचे( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    सचे( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      पटिनिवत्त;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  सचे( (v = sqlite3GetVdbe(pParse))!=0 ){
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
शून्य sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zName){
  Trigger *pTrigger;
  Hash *pHash;

  प्रतिज्ञा( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_निवेशय(pHash, zName, 0);
  सचे( ALWAYS(pTrigger) ){
    सचे( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      सचे( pTab ){
        Trigger **pp;
        अत्थाय(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          सचे( *pp==pTrigger ){
            *pp = (*pp)->pNext;
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
  सचे( pIdList==0 || NEVER(pEList==0) ) पटिनिवत्त 1;
  अत्थाय(e=0; e<pEList->n_व्यञ्जकम्; e++){
    सचे( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) पटिनिवत्त 1;
  }
  पटिनिवत्त 0; 
}

/*
** Return true if any TEMP triggers exist
*/
स्थिर पूर्णाङ्क tempTriggersExist(sqlite3 *db){
  सचे( NEVER(db->aDb[1].pSchema==0) ) पटिनिवत्त 0;
  सचे( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) पटिनिवत्त 0;
  पटिनिवत्त 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
स्थिर SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  पूर्णाङ्क op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यञ्जकम्_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णाङ्क *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  पूर्णाङ्क mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  प्रतिज्ञा( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  सचे( pList!=0 ){
    p = pList;
    सचे( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      सचे( pList==pTab->pTrigger ){
        pList = 0;
        गच्छ exit_triggers_exist;
      }
      याव( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    करोहि{
      सचे( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }अञ्ञथा सचे( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        प्रतिज्ञा( sqlite3IsToplevel(pParse) );
        p->op = op;
        सचे( IsVirtual(pTab) ){
          सचे( op!=TK_निवेशय ){
            sqlite3_Error_सन्देशः(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }अञ्ञथा{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }अञ्ञथा सचे( p->bReturning && p->op==TK_निवेशय && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }याव( p );
  }
exit_triggers_exist:
  सचे( pMask ){
    *pMask = mask;
  }
  पटिनिवत्त (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  पूर्णाङ्क op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यञ्जकम्_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णाङ्क *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  प्रतिज्ञा( pTab!=0 );
  सचे( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    सचे( pMask ) *pMask = 0;
    पटिनिवत्त 0;
  }
  पटिनिवत्त triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
स्थिर पूर्णाङ्क isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  व्यञ्जकम् *pTerm         /* A term in the RETURNING clause */
){
  प्रतिज्ञा( pTerm!=0 );
  सचे( pTerm->op==TK_ASTERISK ) पटिनिवत्त 1;
  सचे( pTerm->op!=TK_DOT ) पटिनिवत्त 0;
  प्रतिज्ञा( pTerm->p_दक्षिणः!=0 );
  प्रतिज्ञा( pTerm->p_वामः!=0 );
  सचे( pTerm->p_दक्षिणः->op!=TK_ASTERISK ) पटिनिवत्त 0;
  sqlite3_Error_सन्देशः(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  पटिनिवत्त 1;
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
  Table *pTab           /* The table being updated */
){
  व्यञ्जकम्_List *pNew = 0;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क i;

  अत्थाय(i=0; i<pList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p_Old_व्यञ्जकम् = pList->a[i].p_व्यञ्जकम्;
    सचे( NEVER(p_Old_व्यञ्जकम्==0) ) अनुवर्त;
    सचे( isAsteriskTerm(pParse, p_Old_व्यञ्जकम्) ){
      पूर्णाङ्क jj;
      अत्थाय(jj=0; jj<pTab->nCol; jj++){
        व्यञ्जकम् *p_New_व्यञ्जकम्;
        सचे( IsHiddenColumn(pTab->aCol+jj) ) अनुवर्त;
        p_New_व्यञ्जकम् = sqlite3_व्यञ्जकम्(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_New_व्यञ्जकम्);
        सचे( !db->mallocFailed ){
          संरचना व्यञ्जकम्_List_item *pItem = &pNew->a[pNew->n_व्यञ्जकम्-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }अञ्ञथा{
      व्यञ्जकम् *p_New_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_Old_व्यञ्जकम्, 0);
      pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_New_व्यञ्जकम्);
      सचे( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        संरचना व्यञ्जकम्_List_item *pItem = &pNew->a[pNew->n_व्यञ्जकम्-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  पटिनिवत्त pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
स्थिर पूर्णाङ्क sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, व्यञ्जकम् *p_व्यञ्जकम्){
  UNUSED_PARAMETER(NotUsed);
  सचे( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्)
   && (p_व्यञ्जकम्->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_VarSelect) );
    व्यञ्जकम्_Set_Property(p_व्यञ्जकम्, EP_VarSelect);
  }
  पटिनिवत्त WRC_Continue;
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
  अत्थाय(i=0; i<pSrc->nSrc; i++){
    सचे( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      भङ्ग;
    }
  }
  पटिनिवत्त WRC_Continue;
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
  Table *pTab
){
  Walker w;
  memset(&w, 0, परिमाण(w));
  w.x_व्यञ्जकम्_Callback = sqlite3_व्यञ्जकम्_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_व्यञ्जकम्_List(&w, pEList);
  सचे( w.eCode ){
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
स्थिर शून्य codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  पूर्णाङ्क regIn            /* The first in an array of registers */
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
  सचे( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    पटिनिवत्त;
  }
  प्रतिज्ञा( db->pParse==pParse );
  प्रतिज्ञा( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  सचे( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    पटिनिवत्त;
  }
  memset(&sSelect, 0, परिमाण(sSelect));
  memset(&uSrc, 0, परिमाण(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_व्यञ्जकम्_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  सचे( pParse->nErr==0 ){
    प्रतिज्ञा( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_व्यञ्जकम्_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  सचे( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, परिमाण(sNC));
    सचे( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_व्यञ्जकम्;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    सचे( sqlite3_Resolve_व्यञ्जकम्_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      पूर्णाङ्क i;
      पूर्णाङ्क nCol = pNew->n_व्यञ्जकम्;
      पूर्णाङ्क reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      अत्थाय(i=0; i<nCol; i++){
        व्यञ्जकम् *pCol = pNew->a[i].p_व्यञ्जकम्;
        प्रतिज्ञा( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_व्यञ्जकम्_Code_Factorable(pParse, pCol, reg+i);
        सचे( sqlite3_व्यञ्जकम्_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_निवेशय, pReturning->iRetCur, reg+i, reg+i+1);
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
स्थिर पूर्णाङ्क codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  पूर्णाङ्क orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  प्रतिज्ञा( pParse->pTriggerTab && pParse->pToplevel );
  प्रतिज्ञा( pStepList );
  प्रतिज्ञा( v!=0 );
  अत्थाय(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    प्रतिज्ञा( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    सचे( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    विकल्प( pStep->op ){
      पक्ष TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_व्यञ्जकम्_List_Dup(db, pStep->p_व्यञ्जकम्_List, 0), 
          sqlite3_व्यञ्जकम्_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      पक्ष TK_निवेशय: {
        sqlite3_निवेशय(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      पक्ष TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_व्यञ्जकम्_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गणनम्);
        भङ्ग;
      }
      स्वभाव: प्रतिज्ञा( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        भङ्ग;
      }
    } 
  }

  पटिनिवत्त 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
स्थिर ध्रुव वर्ण *onErrorText(पूर्णाङ्क onError){
  विकल्प( onError ){
    पक्ष OE_Abort:    पटिनिवत्त "abort";
    पक्ष OE_Rollback: पटिनिवत्त "rollback";
    पक्ष OE_Fail:     पटिनिवत्त "fail";
    पक्ष OE_Replace:  पटिनिवत्त "replace";
    पक्ष OE_Ignore:   पटिनिवत्त "ignore";
    पक्ष OE_Default:  पटिनिवत्त "default";
  }
  पटिनिवत्त "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
स्थिर शून्य transferParseError(Parse *pTo, Parse *pFrom){
  प्रतिज्ञा( pFrom->z_Err_सन्देशः==0 || pFrom->nErr );
  प्रतिज्ञा( pTo->z_Err_सन्देशः==0 || pTo->nErr );
  सचे( pTo->nErr==0 ){
    pTo->z_Err_सन्देशः = pFrom->z_Err_सन्देशः;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }अञ्ञथा{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_सन्देशः);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
स्थिर TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  पूर्णाङ्क orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_शिखरम्;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  व्यञ्जकम् *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  पूर्णाङ्क iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  पूर्णाङ्क nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_शिखरम् = pParse;
  अत्थाय(nDepth=0; p_शिखरम्->pOuterParse; p_शिखरम् = p_शिखरम्->pOuterParse, nDepth++){}
  सचे( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_सन्देशः(pParse, "triggers nested too deep");
    पटिनिवत्त 0;
  }

  p_शिखरम् = sqlite3ParseToplevel(pParse);
  प्रतिज्ञा( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  प्रतिज्ञा( p_शिखरम्->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, परिमाण(TriggerPrg));
  सचे( !pPrg ) पटिनिवत्त 0;
  pPrg->pNext = p_शिखरम्->pTriggerPrg;
  p_शिखरम्->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, परिमाण(SubProgram));
  सचे( !pProgram ) पटिनिवत्त 0;
  sqlite3VdbeLinkSubProgram(p_शिखरम्->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, परिमाण(sNC));
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
  सचे( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_निवेशय ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    सचे( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    सचे( pTrigger->pWhen ){
      pWhen = sqlite3_व्यञ्जकम्_Dup(db, pTrigger->pWhen, 0);
      सचे( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_व्यञ्जकम्_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_व्यञ्जकम्_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_व्यञ्जकम्_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    सचे( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    सचे( pParse->nErr==0 ){
      प्रतिज्ञा( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_शिखरम्->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (शून्य *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }अञ्ञथा{
    transferParseError(pParse, &sSubParse);
  }

  प्रतिज्ञा( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  पटिनिवत्त pPrg;
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
  Table *pTab,         /* The table trigger pTrigger is attached to */
  पूर्णाङ्क orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_मूलम् = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  प्रतिज्ञा( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  अत्थाय(pPrg=p_मूलम्->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  सचे( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  पटिनिवत्त pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
शून्य sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
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
  सचे( pPrg ){
    पूर्णाङ्क bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (ध्रुव वर्ण *)pPrg->pProgram, P4_SUBPROGRAM);
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
शून्य sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  पूर्णाङ्क op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  व्यञ्जकम्_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णाङ्क tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  पूर्णाङ्क reg,             /* The first in an array of registers (see above) */
  पूर्णाङ्क orconf,          /* ON CONFLICT policy */
  पूर्णाङ्क ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  प्रतिज्ञा( op==TK_UPDATE || op==TK_निवेशय || op==TK_DELETE );
  प्रतिज्ञा( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  प्रतिज्ञा( (op==TK_UPDATE)==(pChanges!=0) );

  अत्थाय(p=pTrigger; p; p=p->pNext){

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
    सचे( (p->op==op || (p->bReturning && p->op==TK_निवेशय && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      सचे( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }अञ्ञथा सचे( sqlite3IsToplevel(pParse) ){
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
  व्यञ्जकम्_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णाङ्क isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  पूर्णाङ्क tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  पूर्णाङ्क orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ध्रुव पूर्णाङ्क op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  प्रतिज्ञा( isNew==1 || isNew==0 );
  सचे( IsView(pTab) ){
    पटिनिवत्त 0xffffffff;
  }
  अत्थाय(p=pTrigger; p; p=p->pNext){
    सचे( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      सचे( p->bReturning ){
        mask = 0xffffffff;
      }अञ्ञथा{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        सचे( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  पटिनिवत्त mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
