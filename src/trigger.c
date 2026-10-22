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
#ضمن "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
فراغ sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  כלעוד( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3ExprDelete(db, pTmp->pWhere);
    sqlite3ExprListDelete(db, pTmp->pExprList);
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

  أكد( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  כלעוד( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    אם( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }אחרת אם( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      أكد( pParse->db->pVtabCtx==0 );
#endif
      أكد( pParse->bReturning );
      أكد( !pParse->isCreate );
      أكد( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#אם 0
  אם( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    עבור(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  החזר pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
فراغ sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  صحيح tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  صحيح op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  Expr *pWhen,        /* WHEN clause */
  صحيح isTemp,         /* True if the TEMPORARY keyword is present */
  صحيح noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  حرف *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  صحيح iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  أكد( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  أكد( pName2!=0 );
  أكد( op==TK_INSERT || op==TK_UPDATE || op==TK_DELETE );
  أكد( op>0 && op<0xff );
  אם( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    אם( pName2->n>0 ){
      sqlite3ErrorMsg(pParse, "temporary trigger may not have qualified name");
      اذهب trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }אחרת{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    אם( iDb<0 ){
      اذهب trigger_cleanup;
    }
  }
  אם( !pTableName || db->mallocFailed ){
    اذهب trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  אם( db->init.busy && iDb!=1 ){
    أكد( pTableName->a[0].fg.fixedSchema==0 );
    أكد( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  אם( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  אם( db->mallocFailed ) اذهب trigger_cleanup;
  أكد( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  אם( sqlite3FixSrcList(&sFix, pTableName) ){
    اذهب trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  אם( !pTab ){
    /* The table does not exist. */
    اذهب trigger_orphan_error;
  }
  אם( IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "cannot create triggers on virtual tables");
    اذهب trigger_orphan_error;
  }
  אם( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3ErrorMsg(pParse, "cannot create triggers on shadow tables");
    اذهب trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  אם( zName==0 ){
    أكد( db->mallocFailed );
    اذهب trigger_cleanup;
  }
  אם( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    اذهب trigger_cleanup;
  }
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  אם( !IN_RENAME_OBJECT ){
    אם( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      אם( !noErr ){
        sqlite3ErrorMsg(pParse, "trigger %T already exists", pName);
      }אחרת אם( db->init.busy ){
        sqlite3ErrorMsg(pParse,""); /* Err msg generated by corruptSchema() */
      }אחרת{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      اذهب trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#אם !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  אם( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3ErrorMsg(pParse, "cannot create trigger on system table");
    اذهب trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  אם( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3ErrorMsg(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    اذهب trigger_orphan_error;
  }
  אם( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3ErrorMsg(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    اذهب trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  אם( !IN_RENAME_OBJECT ){
    صحيح iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    صحيح code = SQLITE_CREATE_TRIGGER;
    ثابت حرف *zDb = db->aDb[iTabDb].zDbSName;
    ثابت حرف *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    אם( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    אם( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      اذهب trigger_cleanup;
    }
    אם( sqlite3AuthCheck(pParse, SQLITE_INSERT, SCHEMA_TABLE(iTabDb),0,zDb)){
      اذهب trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  אם (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, حجم(Trigger));
  אם( pTrigger==0 ) اذهب trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  אם( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }אחרת{
    pTrigger->pWhen = sqlite3ExprDup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  أكد( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3ExprDelete(db, pWhen);
  אם( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }אחרת{
    أكد( pParse->pNewTrigger==pTrigger );
  }
  החזר;

trigger_orphan_error:
  אם( db->init.iDb==1 ){
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
  اذهب trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
فراغ sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  حرف *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  صحيح iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  אם( NEVER(pParse->nErr) || !pTrig ) اذهب triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  أكد( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  עבור(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  אם( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3ErrorMsg(pParse, "trigger \"%w\" contains too many steps", zName);
    اذهب triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  אם( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3FixExpr(&sFix, pTrig->pWhen) 
  ){
    اذهب triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  אם( IN_RENAME_OBJECT ){
    أكد( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }אחרת
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  אם( !db->init.busy ){
    Vdbe *v;
    حرف *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    אם( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      עבור(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        אם( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3ErrorMsg(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          اذهب triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    אם( v==0 ) اذهب triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (حرف*)pAll->z, pAll->n);
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

  אם( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    أكد( pLink!=0 );
    pTrig = sqlite3HashInsert(pHash, zName, pTrig);
    אם( pTrig ){
      sqlite3OomFault(db);
    }אחרת אם( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      أكد( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  أكد( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ساكن حرف *triggerSpanDup(sqlite3 *db, ثابت حرف *zStart, ثابت حرف *zEnd){
  حرف *z = sqlite3DbSpanDup(db, zStart, zEnd);
  صحيح i;
  אם( z ) עבור(i=0; z[i]; i++) אם( sqlite3Isspace(z[i]) ) z[i] = ' ';
  החזר z;
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
  ثابت حرف *zStart,         /* Start of SQL text */
  ثابت حرف *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, حجم(TriggerStep));
  אם( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    החזר 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  החזר pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ساكن TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ثابت حرف *zStart,         /* Start of SQL text */
  ثابت حرف *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  אם( pParse->nErr==0 ){
    אם( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3ErrorMsg(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }אחרת{
      pTriggerStep = sqlite3DbMallocZero(db, حجم(TriggerStep));
      אם( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        אם( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  החזר pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3TriggerInsertStep(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ثابت حرف *zStart, /* Start of SQL text */
  ثابت حرف *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  أكد(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_INSERT, pTabList, zStart, zEnd);
  אם( pTriggerStep ){
    אם( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }אחרת{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    אם( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }אחרת{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  החזר pTriggerStep;
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
  ExprList *pEList,    /* The SET clause: list of column and new values */
  Expr *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ثابت حرف *zStart,  /* Start of SQL text */
  ثابت حرف *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  אם( pTriggerStep ){
    SrcList *pFromDup = 0;
    אם( IN_RENAME_OBJECT ){
      pTriggerStep->pExprList = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }אחרת{
      pTriggerStep->pExprList = sqlite3ExprListDup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3ExprDup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    אם( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    אם( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }אחרת{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3ExprListDelete(db, pEList);
  sqlite3ExprDelete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  החזר pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  Expr *pWhere,           /* The WHERE clause */
  ثابت حرف *zStart,     /* Start of SQL text */
  ثابت حرف *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  אם( pTriggerStep ){
    אם( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }אחרת{
      pTriggerStep->pWhere = sqlite3ExprDup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3ExprDelete(db, pWhere);
  החזר pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
فراغ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  אם( pTrigger==0 || pTrigger->bReturning ) החזר;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3ExprDelete(db, pTrigger->pWhen);
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
فراغ sqlite3DropTrigger(Parse *pParse, SrcList *pName, صحيح noErr){
  Trigger *pTrigger = 0;
  صحيح i;
  ثابت حرف *zDb;
  ثابت حرف *zName;
  sqlite3 *db = pParse->db;

  אם( db->mallocFailed ) اذهب drop_trigger_cleanup;
  אם( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    اذهب drop_trigger_cleanup;
  }

  أكد( pName->nSrc==1 );
  أكد( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  أكد( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  עבור(i=OMIT_TEMPDB; i<db->nDb; i++){
    صحيح j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    אם( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) استمر;
    أكد( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    אם( pTrigger ) اكسر;
  }
  אם( !pTrigger ){
    אם( !noErr ){
      sqlite3ErrorMsg(pParse, "no such trigger: %S", pName->a);
    }אחרת{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    اذهب drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ساكن Table *tableOfTrigger(Trigger *pTrigger){
  החזר sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
فراغ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  صحيح iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  أكد( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  أكد( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  אם( pTable ){
    صحيح code = SQLITE_DROP_TRIGGER;
    ثابت حرف *zDb = db->aDb[iDb].zDbSName;
    ثابت حرف *zTab = SCHEMA_TABLE(iDb);
    אם( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    אם( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      החזר;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  אם( (v = sqlite3GetVdbe(pParse))!=0 ){
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
فراغ sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, صحيح iDb, ثابت حرف *zName){
  Trigger *pTrigger;
  Hash *pHash;

  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3HashInsert(pHash, zName, 0);
  אם( ALWAYS(pTrigger) ){
    אם( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      אם( pTab ){
        Trigger **pp;
        עבור(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          אם( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            اكسر;
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
ساكن صحيح checkColumnOverlap(IdList *pIdList, ExprList *pEList){
  صحيح e;
  אם( pIdList==0 || NEVER(pEList==0) ) החזר 1;
  עבור(e=0; e<pEList->nExpr; e++){
    אם( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) החזר 1;
  }
  החזר 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ساكن صحيح tempTriggersExist(sqlite3 *db){
  אם( NEVER(db->aDb[1].pSchema==0) ) החזר 0;
  אם( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) החזר 0;
  החזר 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ساكن SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  صحيح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ExprList *pChanges,     /* Columns that change in an UPDATE statement */
  صحيح *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  صحيح mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  أكد( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  אם( pList!=0 ){
    p = pList;
    אם( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      אם( pList==pTab->pTrigger ){
        pList = 0;
        اذهب exit_triggers_exist;
      }
      כלעוד( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    עשה{
      אם( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }אחרת אם( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        أكد( sqlite3IsToplevel(pParse) );
        p->op = op;
        אם( IsVirtual(pTab) ){
          אם( op!=TK_INSERT ){
            sqlite3ErrorMsg(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }אחרת{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }אחרת אם( p->bReturning && p->op==TK_INSERT && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }כלעוד( p );
  }
exit_triggers_exist:
  אם( pMask ){
    *pMask = mask;
  }
  החזר (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  صحيح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ExprList *pChanges,     /* Columns that change in an UPDATE statement */
  صحيح *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  أكد( pTab!=0 );
  אם( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    אם( pMask ) *pMask = 0;
    החזר 0;
  }
  החזר triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ساكن صحيح isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  Expr *pTerm         /* A term in the RETURNING clause */
){
  أكد( pTerm!=0 );
  אם( pTerm->op==TK_ASTERISK ) החזר 1;
  אם( pTerm->op!=TK_DOT ) החזר 0;
  أكد( pTerm->pRight!=0 );
  أكد( pTerm->pLeft!=0 );
  אם( pTerm->pRight->op!=TK_ASTERISK ) החזר 0;
  sqlite3ErrorMsg(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  החזר 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ساكن ExprList *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  ExprList *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  ExprList *pNew = 0;
  sqlite3 *db = pParse->db;
  صحيح i;

  עבור(i=0; i<pList->nExpr; i++){
    Expr *pOldExpr = pList->a[i].pExpr;
    אם( NEVER(pOldExpr==0) ) استمر;
    אם( isAsteriskTerm(pParse, pOldExpr) ){
      صحيح jj;
      עבור(jj=0; jj<pTab->nCol; jj++){
        Expr *pNewExpr;
        אם( IsHiddenColumn(pTab->aCol+jj) ) استمر;
        pNewExpr = sqlite3Expr(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3ExprListAppend(pParse, pNew, pNewExpr);
        אם( !db->mallocFailed ){
          بنية ExprList_item *pItem = &pNew->a[pNew->nExpr-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }אחרת{
      Expr *pNewExpr = sqlite3ExprDup(db, pOldExpr, 0);
      pNew = sqlite3ExprListAppend(pParse, pNew, pNewExpr);
      אם( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        بنية ExprList_item *pItem = &pNew->a[pNew->nExpr-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  החזר pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ساكن صحيح sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, Expr *pExpr){
  UNUSED_PARAMETER(NotUsed);
  אם( ExprUseXSelect(pExpr)
   && (pExpr->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ExprHasProperty(pExpr, EP_VarSelect) );
    ExprSetProperty(pExpr, EP_VarSelect);
  }
  החזר WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ساكن صحيح sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  صحيح i;
  SrcList *pSrc;
  أكد( pSelect!=0 );
  pSrc = pSelect->pSrc;
  أكد( pSrc!=0 );
  עבור(i=0; i<pSrc->nSrc; i++){
    אם( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      اكسر;
    }
  }
  החזר WRC_Continue;
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
ساكن فراغ sqlite3ProcessReturningSubqueries(
  ExprList *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, حجم(w));
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3WalkExprList(&w, pEList);
  אם( w.eCode ){
    w.xExprCallback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3WalkExprList(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ساكن فراغ codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  صحيح regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ExprList *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  اتحاد {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  أكد( v!=0 );
  אם( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    החזר;
  }
  أكد( db->pParse==pParse );
  أكد( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  אם( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    החזר;
  }
  memset(&sSelect, 0, حجم(sSelect));
  memset(&uSrc, 0, حجم(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3ExprListDup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  אם( pParse->nErr==0 ){
    أكد( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3ExprListDelete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  אם( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, حجم(sNC));
    אם( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->nExpr;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    אם( sqlite3ResolveExprListNames(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      صحيح i;
      صحيح nCol = pNew->nExpr;
      صحيح reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      עבור(i=0; i<nCol; i++){
        Expr *pCol = pNew->a[i].pExpr;
        أكد( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3ExprCodeFactorable(pParse, pCol, reg+i);
        אם( sqlite3ExprAffinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_Insert, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3ExprListDelete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ساكن صحيح codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  صحيح orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  أكد( pParse->pTriggerTab && pParse->pToplevel );
  أكد( pStepList );
  أكد( v!=0 );
  עבור(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    أكد( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    אם( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    بدل( pStep->op ){
      حالة TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3ExprListDup(db, pStep->pExprList, 0), 
          sqlite3ExprDup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_ResetCount);
        اكسر;
      }
      حالة TK_INSERT: {
        sqlite3Insert(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_ResetCount);
        اكسر;
      }
      حالة TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3ExprDup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_ResetCount);
        اكسر;
      }
      افتراضي: أكد( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        اكسر;
      }
    } 
  }

  החזר 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ساكن ثابت حرف *onErrorText(صحيح onError){
  بدل( onError ){
    حالة OE_Abort:    החזר "abort";
    حالة OE_Rollback: החזר "rollback";
    حالة OE_Fail:     החזר "fail";
    حالة OE_Replace:  החזר "replace";
    حالة OE_Ignore:   החזר "ignore";
    حالة OE_Default:  החזר "default";
  }
  החזר "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ساكن فراغ transferParseError(Parse *pTo, Parse *pFrom){
  أكد( pFrom->zErrMsg==0 || pFrom->nErr );
  أكد( pTo->zErrMsg==0 || pTo->nErr );
  אם( pTo->nErr==0 ){
    pTo->zErrMsg = pFrom->zErrMsg;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }אחרת{
    sqlite3DbFree(pFrom->db, pFrom->zErrMsg);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ساكن TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  صحيح orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *pTop;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  Expr *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  صحيح iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  صحيح nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  pTop = pParse;
  עבור(nDepth=0; pTop->pOuterParse; pTop = pTop->pOuterParse, nDepth++){}
  אם( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3ErrorMsg(pParse, "triggers nested too deep");
    החזר 0;
  }

  pTop = sqlite3ParseToplevel(pParse);
  أكد( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  أكد( pTop->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, حجم(TriggerPrg));
  אם( !pPrg ) החזר 0;
  pPrg->pNext = pTop->pTriggerPrg;
  pTop->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, حجم(SubProgram));
  אם( !pProgram ) החזר 0;
  sqlite3VdbeLinkSubProgram(pTop->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, حجم(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = pTop;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  אם( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_INSERT ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    אם( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    אם( pTrigger->pWhen ){
      pWhen = sqlite3ExprDup(db, pTrigger->pWhen, 0);
      אם( db->mallocFailed==0
       && SQLITE_OK==sqlite3ResolveExprNames(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3ExprIfFalse(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3ExprDelete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    אם( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    אם( pParse->nErr==0 ){
      أكد( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &pTop->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (فراغ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }אחרת{
    transferParseError(pParse, &sSubParse);
  }

  أكد( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  החזר pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ساكن TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  صحيح orconf           /* ON CONFLICT algorithm. */
){
  Parse *pRoot = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  أكد( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  עבור(pPrg=pRoot->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  אם( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  החזר pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
فراغ sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  صحيح reg,             /* Reg array containing OLD.* and NEW.* values */
  صحيح orconf,          /* ON CONFLICT policy */
  صحيح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  أكد( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  אם( pPrg ){
    صحيح bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (ثابت حرف *)pPrg->pProgram, P4_SUBPROGRAM);
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
فراغ sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  صحيح op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ExprList *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحيح tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  صحيح reg,             /* The first in an array of registers (see above) */
  صحيح orconf,          /* ON CONFLICT policy */
  صحيح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  أكد( op==TK_UPDATE || op==TK_INSERT || op==TK_DELETE );
  أكد( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  أكد( (op==TK_UPDATE)==(pChanges!=0) );

  עבור(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    أكد( p->pSchema!=0 );
    أكد( p->pTabSchema!=0 );
    أكد( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    אם( (p->op==op || (p->bReturning && p->op==TK_INSERT && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      אם( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }אחרת אם( sqlite3IsToplevel(pParse) ){
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
  ExprList *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحيح isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  صحيح tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  صحيح orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ثابت صحيح op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  أكد( isNew==1 || isNew==0 );
  אם( IsView(pTab) ){
    החזר 0xffffffff;
  }
  עבור(p=pTrigger; p; p=p->pNext){
    אם( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      אם( p->bReturning ){
        mask = 0xffffffff;
      }אחרת{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        אם( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  החזר mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
