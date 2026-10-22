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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
व्योम sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  जबतक( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->p_अग्र;

    sqlite3_व्यंजक_Delete(db, pTmp->pWhere);
    sqlite3_व्यंजक_List_Delete(db, pTmp->p_व्यंजक_List);
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

  निश्चित( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  जबतक( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_तथ्य(p);
    अगर( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_अग्र = pList;
      pList = pTrig;
    }अन्यथा अगर( pTrig->op==TK_RETURNING ){
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
      निश्चित( pParse->db->pVtabCtx==0 );
#पूर्ण_अगर
      निश्चित( pParse->bReturning );
      निश्चित( !pParse->isCreate );
      निश्चित( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_अग्र = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_अग्र(p);    
  }
#अगर 0
  अगर( pList ){
    Trigger *pX;
    म_लिखो("Triggers for %s:", pTab->zName);
    क्रम(pX=pList; pX; pX=pX->p_अग्र){
      म_लिखो(" %s", pX->zName);
    }
    म_लिखो("\n");
    ख_साफ(मानक_निकास);
  }
#पूर्ण_अगर
  वापस pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
व्योम sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  पूर्णांक tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  पूर्णांक op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  व्यंजक *pWhen,        /* WHEN clause */
  पूर्णांक is_अस्थायी,         /* True if the TEMPORARY keyword is present */
  पूर्णांक noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  अक्षर *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  पूर्णांक iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  निश्चित( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  निश्चित( pName2!=0 );
  निश्चित( op==TK_जोड़ो || op==TK_UPDATE || op==TK_DELETE );
  निश्चित( op>0 && op<0xff );
  अगर( is_अस्थायी ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    अगर( pName2->n>0 ){
      sqlite3_Error_संदेश(pParse, "temporary trigger may not have qualified name");
      जाओ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }अन्यथा{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    अगर( iDb<0 ){
      जाओ trigger_cleanup;
    }
  }
  अगर( !pTableName || db->mallocFailed ){
    जाओ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  अगर( db->init.busy && iDb!=1 ){
    निश्चित( pTableName->a[0].fg.fixedSchema==0 );
    निश्चित( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  अगर( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  अगर( db->mallocFailed ) जाओ trigger_cleanup;
  निश्चित( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  अगर( sqlite3FixSrcList(&sFix, pTableName) ){
    जाओ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  अगर( !pTab ){
    /* The table does not exist. */
    जाओ trigger_orphan_error;
  }
  अगर( IsVirtual(pTab) ){
    sqlite3_Error_संदेश(pParse, "cannot create triggers on virtual tables");
    जाओ trigger_orphan_error;
  }
  अगर( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_संदेश(pParse, "cannot create triggers on shadow tables");
    जाओ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  अगर( zName==0 ){
    निश्चित( db->mallocFailed );
    जाओ trigger_cleanup;
  }
  अगर( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    जाओ trigger_cleanup;
  }
  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
  अगर( !IN_RENAME_OBJECT ){
    अगर( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      अगर( !noErr ){
        sqlite3_Error_संदेश(pParse, "trigger %T already exists", pName);
      }अन्यथा अगर( db->init.busy ){
        sqlite3_Error_संदेश(pParse,""); /* Err msg generated by corruptSchema() */
      }अन्यथा{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      जाओ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#अगर !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  अगर( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_संदेश(pParse, "cannot create trigger on system table");
    जाओ trigger_cleanup;
  }
#पूर्ण_अगर

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  अगर( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_संदेश(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    जाओ trigger_orphan_error;
  }
  अगर( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_संदेश(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    जाओ trigger_orphan_error;
  }

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( !IN_RENAME_OBJECT ){
    पूर्णांक iTabDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
    पूर्णांक code = SQLITE_CREATE_TRIGGER;
    स्थिर अक्षर *zDb = db->aDb[iTabDb].zDbSName;
    स्थिर अक्षर *zDbTrig = is_अस्थायी ? db->aDb[1].zDbSName : zDb;
    अगर( iTabDb==1 || is_अस्थायी ) code = SQLITE_CREATE_अस्थायी_TRIGGER;
    अगर( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      जाओ trigger_cleanup;
    }
    अगर( sqlite3AuthCheck(pParse, SQLITE_जोड़ो, SCHEMA_TABLE(iTabDb),0,zDb)){
      जाओ trigger_cleanup;
    }
  }
#पूर्ण_अगर

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  अगर (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, माप(Trigger));
  अगर( pTrigger==0 ) जाओ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  अगर( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }अन्यथा{
    pTrigger->pWhen = sqlite3_व्यंजक_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  निश्चित( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_व्यंजक_Delete(db, pWhen);
  अगर( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }अन्यथा{
    निश्चित( pParse->pNewTrigger==pTrigger );
  }
  वापस;

trigger_orphan_error:
  अगर( db->init.iDb==1 ){
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
  जाओ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
व्योम sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  अक्षर *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  पूर्णांक iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  अगर( NEVER(pParse->nErr) || !pTrig ) जाओ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTrig->pSchema);
  निश्चित( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  क्रम(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->p_अग्र;
  }
  अगर( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_संदेश(pParse, "trigger \"%w\" contains too many steps", zName);
    जाओ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  अगर( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_व्यंजक(&sFix, pTrig->pWhen) 
  ){
    जाओ triggerfinish_cleanup;
  }

#अगर_अघोषित SQLITE_OMIT_ALTERTABLE
  अगर( IN_RENAME_OBJECT ){
    निश्चित( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }अन्यथा
#पूर्ण_अगर

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  अगर( !db->init.busy ){
    Vdbe *v;
    अक्षर *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    अगर( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      क्रम(pStep=pTrig->step_list; pStep; pStep=pStep->p_अग्र){
        अगर( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_संदेश(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          जाओ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    अगर( v==0 ) जाओ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (अक्षर*)pAll->z, pAll->n);
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

  अगर( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
    निश्चित( pLink!=0 );
    pTrig = sqlite3_Hash_जोड़ो(pHash, zName, pTrig);
    अगर( pTrig ){
      sqlite3OomFault(db);
    }अन्यथा अगर( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      निश्चित( pTab!=0 );
      pLink->p_अग्र = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  निश्चित( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
अटल अक्षर *triggerSpanDup(sqlite3 *db, स्थिर अक्षर *zStart, स्थिर अक्षर *zEnd){
  अक्षर *z = sqlite3DbSpanDup(db, zStart, zEnd);
  पूर्णांक i;
  अगर( z ) क्रम(i=0; z[i]; i++) अगर( sqlite3Isspace(z[i]) ) z[i] = ' ';
  वापस z;
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
  स्थिर अक्षर *zStart,         /* Start of SQL text */
  स्थिर अक्षर *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, माप(TriggerStep));
  अगर( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    वापस 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  वापस pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
अटल TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  स्थिर अक्षर *zStart,         /* Start of SQL text */
  स्थिर अक्षर *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  अगर( pParse->nErr==0 ){
    अगर( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_संदेश(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }अन्यथा{
      pTriggerStep = sqlite3DbMallocZero(db, माप(TriggerStep));
      अगर( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        अगर( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  वापस pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_जोड़ो_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  स्थिर अक्षर *zStart, /* Start of SQL text */
  स्थिर अक्षर *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  निश्चित(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_जोड़ो, pTabList, zStart, zEnd);
  अगर( pTriggerStep ){
    अगर( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }अन्यथा{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    अगर( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }अन्यथा{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  वापस pTriggerStep;
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
  व्यंजक_List *pEList,    /* The SET clause: list of column and new values */
  व्यंजक *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  स्थिर अक्षर *zStart,  /* Start of SQL text */
  स्थिर अक्षर *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  अगर( pTriggerStep ){
    SrcList *pFromDup = 0;
    अगर( IN_RENAME_OBJECT ){
      pTriggerStep->p_व्यंजक_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }अन्यथा{
      pTriggerStep->p_व्यंजक_List = sqlite3_व्यंजक_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_व्यंजक_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    अगर( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    अगर( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }अन्यथा{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_व्यंजक_List_Delete(db, pEList);
  sqlite3_व्यंजक_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  वापस pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  व्यंजक *pWhere,           /* The WHERE clause */
  स्थिर अक्षर *zStart,     /* Start of SQL text */
  स्थिर अक्षर *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  अगर( pTriggerStep ){
    अगर( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }अन्यथा{
      pTriggerStep->pWhere = sqlite3_व्यंजक_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_व्यंजक_Delete(db, pWhere);
  वापस pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
व्योम sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  अगर( pTrigger==0 || pTrigger->bReturning ) वापस;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_व्यंजक_Delete(db, pTrigger->pWhen);
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
व्योम sqlite3DropTrigger(Parse *pParse, SrcList *pName, पूर्णांक noErr){
  Trigger *pTrigger = 0;
  पूर्णांक i;
  स्थिर अक्षर *zDb;
  स्थिर अक्षर *zName;
  sqlite3 *db = pParse->db;

  अगर( db->mallocFailed ) जाओ drop_trigger_cleanup;
  अगर( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    जाओ drop_trigger_cleanup;
  }

  निश्चित( pName->nSrc==1 );
  निश्चित( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  निश्चित( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  क्रम(i=OMIT_TEMPDB; i<db->nDb; i++){
    पूर्णांक j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    अगर( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) जारी;
    निश्चित( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    अगर( pTrigger ) अवरोध;
  }
  अगर( !pTrigger ){
    अगर( !noErr ){
      sqlite3_Error_संदेश(pParse, "no such trigger: %S", pName->a);
    }अन्यथा{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    जाओ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
अटल Table *tableOfTrigger(Trigger *pTrigger){
  वापस sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
व्योम sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  पूर्णांक iDb;

  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTrigger->pSchema);
  निश्चित( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  निश्चित( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( pTable ){
    पूर्णांक code = SQLITE_DROP_TRIGGER;
    स्थिर अक्षर *zDb = db->aDb[iDb].zDbSName;
    स्थिर अक्षर *zTab = SCHEMA_TABLE(iDb);
    अगर( iDb==1 ) code = SQLITE_DROP_अस्थायी_TRIGGER;
    अगर( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      वापस;
    }
  }
#पूर्ण_अगर

  /* Generate code to destroy the database record of the trigger.
  */
  अगर( (v = sqlite3GetVdbe(pParse))!=0 ){
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
व्योम sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, पूर्णांक iDb, स्थिर अक्षर *zName){
  Trigger *pTrigger;
  Hash *pHash;

  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_जोड़ो(pHash, zName, 0);
  अगर( ALWAYS(pTrigger) ){
    अगर( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      अगर( pTab ){
        Trigger **pp;
        क्रम(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_अग्र)){
          अगर( *pp==pTrigger ){
            *pp = (*pp)->p_अग्र;
            अवरोध;
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
अटल पूर्णांक checkColumnOverlap(IdList *pIdList, व्यंजक_List *pEList){
  पूर्णांक e;
  अगर( pIdList==0 || NEVER(pEList==0) ) वापस 1;
  क्रम(e=0; e<pEList->n_व्यंजक; e++){
    अगर( sqlite3_Id_List_सूचकांक(pIdList, pEList->a[e].zEName)>=0 ) वापस 1;
  }
  वापस 0; 
}

/*
** Return true if any TEMP triggers exist
*/
अटल पूर्णांक अस्थायी_Triggers_Exist(sqlite3 *db){
  अगर( NEVER(db->aDb[1].pSchema==0) ) वापस 0;
  अगर( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) वापस 0;
  वापस 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
अटल SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  पूर्णांक op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यंजक_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णांक *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  पूर्णांक mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  निश्चित( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_अग्र==0) );
  अगर( pList!=0 ){
    p = pList;
    अगर( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_To_सूचकांक(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      अगर( pList==pTab->pTrigger ){
        pList = 0;
        जाओ exit_triggers_exist;
      }
      जबतक( ALWAYS(p->p_अग्र) && p->p_अग्र!=pTab->pTrigger ) p = p->p_अग्र;
      p->p_अग्र = 0;
      p = pList;
    }
    करो{
      अगर( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }अन्यथा अगर( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        निश्चित( sqlite3IsToplevel(pParse) );
        p->op = op;
        अगर( IsVirtual(pTab) ){
          अगर( op!=TK_जोड़ो ){
            sqlite3_Error_संदेश(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }अन्यथा{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }अन्यथा अगर( p->bReturning && p->op==TK_जोड़ो && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->p_अग्र;
    }जबतक( p );
  }
exit_triggers_exist:
  अगर( pMask ){
    *pMask = mask;
  }
  वापस (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  पूर्णांक op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  व्यंजक_List *pChanges,     /* Columns that change in an UPDATE statement */
  पूर्णांक *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  निश्चित( pTab!=0 );
  अगर( (pTab->pTrigger==0 && !अस्थायी_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    अगर( pMask ) *pMask = 0;
    वापस 0;
  }
  वापस triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
अटल पूर्णांक isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  व्यंजक *pTerm         /* A term in the RETURNING clause */
){
  निश्चित( pTerm!=0 );
  अगर( pTerm->op==TK_ASTERISK ) वापस 1;
  अगर( pTerm->op!=TK_DOT ) वापस 0;
  निश्चित( pTerm->p_दायाँ!=0 );
  निश्चित( pTerm->p_बायाँ!=0 );
  अगर( pTerm->p_दायाँ->op!=TK_ASTERISK ) वापस 0;
  sqlite3_Error_संदेश(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  वापस 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
अटल व्यंजक_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  व्यंजक_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  व्यंजक_List *pNew = 0;
  sqlite3 *db = pParse->db;
  पूर्णांक i;

  क्रम(i=0; i<pList->n_व्यंजक; i++){
    व्यंजक *p_Old_व्यंजक = pList->a[i].p_व्यंजक;
    अगर( NEVER(p_Old_व्यंजक==0) ) जारी;
    अगर( isAsteriskTerm(pParse, p_Old_व्यंजक) ){
      पूर्णांक jj;
      क्रम(jj=0; jj<pTab->nCol; jj++){
        व्यंजक *p_New_व्यंजक;
        अगर( IsHiddenColumn(pTab->aCol+jj) ) जारी;
        p_New_व्यंजक = sqlite3_व्यंजक(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_व्यंजक_List_Append(pParse, pNew, p_New_व्यंजक);
        अगर( !db->mallocFailed ){
          काष्ठा व्यंजक_List_item *pItem = &pNew->a[pNew->n_व्यंजक-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }अन्यथा{
      व्यंजक *p_New_व्यंजक = sqlite3_व्यंजक_Dup(db, p_Old_व्यंजक, 0);
      pNew = sqlite3_व्यंजक_List_Append(pParse, pNew, p_New_व्यंजक);
      अगर( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        काष्ठा व्यंजक_List_item *pItem = &pNew->a[pNew->n_व्यंजक-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  वापस pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
अटल पूर्णांक sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, व्यंजक *p_व्यंजक){
  UNUSED_PARAMETER(NotUsed);
  अगर( व्यंजक_Use_XSelect(p_व्यंजक)
   && (p_व्यंजक->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( व्यंजक_Has_Property(p_व्यंजक, EP_VarSelect) );
    व्यंजक_Set_Property(p_व्यंजक, EP_VarSelect);
  }
  वापस WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
अटल पूर्णांक sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  पूर्णांक i;
  SrcList *pSrc;
  निश्चित( pSelect!=0 );
  pSrc = pSelect->pSrc;
  निश्चित( pSrc!=0 );
  क्रम(i=0; i<pSrc->nSrc; i++){
    अगर( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      अवरोध;
    }
  }
  वापस WRC_Continue;
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
अटल व्योम sqlite3ProcessReturningSubqueries(
  व्यंजक_List *pEList,
  Table *pTab
){
  Walker w;
  स_रखो(&w, 0, माप(w));
  w.x_व्यंजक_Callback = sqlite3_व्यंजक_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_व्यंजक_List(&w, pEList);
  अगर( w.eCode ){
    w.x_व्यंजक_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_व्यंजक_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
अटल व्योम codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  पूर्णांक regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  व्यंजक_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  जोड़ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  निश्चित( v!=0 );
  अगर( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    वापस;
  }
  निश्चित( db->pParse==pParse );
  निश्चित( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  अगर( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    वापस;
  }
  स_रखो(&sSelect, 0, माप(sSelect));
  स_रखो(&uSrc, 0, माप(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_व्यंजक_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  अगर( pParse->nErr==0 ){
    निश्चित( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_व्यंजक_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  अगर( pParse->nErr==0 ){
    NameContext sNC;
    स_रखो(&sNC, 0, माप(sNC));
    अगर( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_व्यंजक;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    अगर( sqlite3_Resolve_व्यंजक_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      पूर्णांक i;
      पूर्णांक nCol = pNew->n_व्यंजक;
      पूर्णांक reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      क्रम(i=0; i<nCol; i++){
        व्यंजक *pCol = pNew->a[i].p_व्यंजक;
        निश्चित( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_व्यंजक_Code_Factorable(pParse, pCol, reg+i);
        अगर( sqlite3_व्यंजक_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_जोड़ो, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_व्यंजक_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
अटल पूर्णांक codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  पूर्णांक orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  निश्चित( pParse->pTriggerTab && pParse->pToplevel );
  निश्चित( pStepList );
  निश्चित( v!=0 );
  क्रम(pStep=pStepList; pStep; pStep=pStep->p_अग्र){
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
    निश्चित( pParse->okConstFactor==0 );

#अगर_अघोषित SQLITE_OMIT_TRACE
    अगर( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#पूर्ण_अगर

    चयन( pStep->op ){
      हाल TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_व्यंजक_List_Dup(db, pStep->p_व्यंजक_List, 0), 
          sqlite3_व्यंजक_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गिनती);
        अवरोध;
      }
      हाल TK_जोड़ो: {
        sqlite3_जोड़ो(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गिनती);
        अवरोध;
      }
      हाल TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_व्यंजक_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_गिनती);
        अवरोध;
      }
      शेष: निश्चित( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        अवरोध;
      }
    } 
  }

  वापस 0;
}

#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
अटल स्थिर अक्षर *onErrorText(पूर्णांक onError){
  चयन( onError ){
    हाल OE_Abort:    वापस "abort";
    हाल OE_Rollback: वापस "rollback";
    हाल OE_Fail:     वापस "fail";
    हाल OE_Replace:  वापस "replace";
    हाल OE_Ignore:   वापस "ignore";
    हाल OE_Default:  वापस "default";
  }
  वापस "n/a";
}
#पूर्ण_अगर

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
अटल व्योम transferParseError(Parse *pTo, Parse *pFrom){
  निश्चित( pFrom->z_Err_संदेश==0 || pFrom->nErr );
  निश्चित( pTo->z_Err_संदेश==0 || pTo->nErr );
  अगर( pTo->nErr==0 ){
    pTo->z_Err_संदेश = pFrom->z_Err_संदेश;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }अन्यथा{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_संदेश);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
अटल TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  पूर्णांक orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_शीर्ष_2;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  व्यंजक *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  पूर्णांक iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  पूर्णांक nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_शीर्ष_2 = pParse;
  क्रम(nDepth=0; p_शीर्ष_2->pOuterParse; p_शीर्ष_2 = p_शीर्ष_2->pOuterParse, nDepth++){}
  अगर( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_संदेश(pParse, "triggers nested too deep");
    वापस 0;
  }

  p_शीर्ष_2 = sqlite3ParseToplevel(pParse);
  निश्चित( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  निश्चित( p_शीर्ष_2->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, माप(TriggerPrg));
  अगर( !pPrg ) वापस 0;
  pPrg->p_अग्र = p_शीर्ष_2->pTriggerPrg;
  p_शीर्ष_2->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, माप(SubProgram));
  अगर( !pProgram ) वापस 0;
  sqlite3VdbeLinkSubProgram(p_शीर्ष_2->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  स_रखो(&sNC, 0, माप(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_शीर्ष_2;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  अगर( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_जोड़ो ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#अगर_अघोषित SQLITE_OMIT_TRACE
    अगर( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#पूर्ण_अगर

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    अगर( pTrigger->pWhen ){
      pWhen = sqlite3_व्यंजक_Dup(db, pTrigger->pWhen, 0);
      अगर( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_व्यंजक_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_व्यंजक_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_व्यंजक_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    अगर( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    अगर( pParse->nErr==0 ){
      निश्चित( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_शीर्ष_2->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (व्योम *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }अन्यथा{
    transferParseError(pParse, &sSubParse);
  }

  निश्चित( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  वापस pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
अटल TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  पूर्णांक orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_मूल = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  निश्चित( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  क्रम(pPrg=p_मूल->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_अग्र
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  अगर( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  वापस pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
व्योम sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  पूर्णांक reg,             /* Reg array containing OLD.* and NEW.* values */
  पूर्णांक orconf,          /* ON CONFLICT policy */
  पूर्णांक ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  निश्चित( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  अगर( pPrg ){
    पूर्णांक bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (स्थिर अक्षर *)pPrg->pProgram, P4_SUBPROGRAM);
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
व्योम sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  पूर्णांक op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  व्यंजक_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णांक tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  पूर्णांक reg,             /* The first in an array of registers (see above) */
  पूर्णांक orconf,          /* ON CONFLICT policy */
  पूर्णांक ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  निश्चित( op==TK_UPDATE || op==TK_जोड़ो || op==TK_DELETE );
  निश्चित( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  निश्चित( (op==TK_UPDATE)==(pChanges!=0) );

  क्रम(p=pTrigger; p; p=p->p_अग्र){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    निश्चित( p->pSchema!=0 );
    निश्चित( p->pTabSchema!=0 );
    निश्चित( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    अगर( (p->op==op || (p->bReturning && p->op==TK_जोड़ो && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      अगर( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }अन्यथा अगर( sqlite3IsToplevel(pParse) ){
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
  व्यंजक_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  पूर्णांक isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  पूर्णांक tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  पूर्णांक orconf           /* Default ON CONFLICT policy for trigger steps */
){
  स्थिर पूर्णांक op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  निश्चित( isNew==1 || isNew==0 );
  अगर( IsView(pTab) ){
    वापस 0xffffffff;
  }
  क्रम(p=pTrigger; p; p=p->p_अग्र){
    अगर( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      अगर( p->bReturning ){
        mask = 0xffffffff;
      }अन्यथा{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        अगर( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  वापस mask;
}

#पूर्ण_अगर /* !defined(SQLITE_OMIT_TRIGGER) */
