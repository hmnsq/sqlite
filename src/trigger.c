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
#അന്തര്ഭാവ "sqliteInt.h"

#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
ശൂന്യം sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep){
  ആകുമ്പോൾ( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3_വ്യഞ്ജകം_Delete(db, pTmp->pWhere);
    sqlite3_വ്യഞ്ജകം_List_Delete(db, pTmp->p_വ്യഞ്ജകം_List);
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

  പ്രതിജ്ഞാ( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqliteHashFirst(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  ആകുമ്പോൾ( p ){
    Trigger *pTrig = (Trigger *)sqliteHashData(p);
    എങ്കിൽ( pTrig->pTabSchema==pTab->pSchema
     && pTrig->table
     && 0==sqlite3StrICmp(pTrig->table, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->pNext = pList;
      pList = pTrig;
    }അല്ലെങ്കിൽ എങ്കിൽ( pTrig->op==TK_RETURNING ){
#ifndef SQLITE_OMIT_VIRTUALTABLE
      പ്രതിജ്ഞാ( pParse->db->pVtabCtx==0 );
#endif
      പ്രതിജ്ഞാ( pParse->bReturning );
      പ്രതിജ്ഞാ( !pParse->isCreate );
      പ്രതിജ്ഞാ( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }        
    p = sqliteHashNext(p);    
  }
#എങ്കിൽ 0
  എങ്കിൽ( pList ){
    Trigger *pX;
    printf("Triggers for %s:", pTab->zName);
    വേണ്ടി(pX=pList; pX; pX=pX->pNext){
      printf(" %s", pX->zName);
    }
    printf("\n");
    fflush(stdout);
  }
#endif
  തിരികെ pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
ശൂന്യം sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  പൂർണ്ണസംഖ്യ tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  പൂർണ്ണസംഖ്യ op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  വ്യഞ്ജകം *pWhen,        /* WHEN clause */
  പൂർണ്ണസംഖ്യ isTemp,         /* True if the TEMPORARY keyword is present */
  പൂർണ്ണസംഖ്യ noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  Table *pTab;            /* Table that the trigger fires off of */
  അക്ഷരം *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  പൂർണ്ണസംഖ്യ iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  പ്രതിജ്ഞാ( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  പ്രതിജ്ഞാ( pName2!=0 );
  പ്രതിജ്ഞാ( op==TK_തിരുകുക || op==TK_UPDATE || op==TK_DELETE );
  പ്രതിജ്ഞാ( op>0 && op<0xff );
  എങ്കിൽ( isTemp ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    എങ്കിൽ( pName2->n>0 ){
      sqlite3_Error_സന്ദേശം(pParse, "temporary trigger may not have qualified name");
      പോകുക trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }അല്ലെങ്കിൽ{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    എങ്കിൽ( iDb<0 ){
      പോകുക trigger_cleanup;
    }
  }
  എങ്കിൽ( !pTableName || db->mallocFailed ){
    പോകുക trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  എങ്കിൽ( db->init.busy && iDb!=1 ){
    പ്രതിജ്ഞാ( pTableName->a[0].fg.fixedSchema==0 );
    പ്രതിജ്ഞാ( pTableName->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  എങ്കിൽ( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  എങ്കിൽ( db->mallocFailed ) പോകുക trigger_cleanup;
  പ്രതിജ്ഞാ( pTableName->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  എങ്കിൽ( sqlite3FixSrcList(&sFix, pTableName) ){
    പോകുക trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  എങ്കിൽ( !pTab ){
    /* The table does not exist. */
    പോകുക trigger_orphan_error;
  }
  എങ്കിൽ( IsVirtual(pTab) ){
    sqlite3_Error_സന്ദേശം(pParse, "cannot create triggers on virtual tables");
    പോകുക trigger_orphan_error;
  }
  എങ്കിൽ( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_സന്ദേശം(pParse, "cannot create triggers on shadow tables");
    പോകുക trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  എങ്കിൽ( zName==0 ){
    പ്രതിജ്ഞാ( db->mallocFailed );
    പോകുക trigger_cleanup;
  }
  എങ്കിൽ( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    പോകുക trigger_cleanup;
  }
  പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  എങ്കിൽ( !IN_RENAME_OBJECT ){
    എങ്കിൽ( sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      എങ്കിൽ( !noErr ){
        sqlite3_Error_സന്ദേശം(pParse, "trigger %T already exists", pName);
      }അല്ലെങ്കിൽ എങ്കിൽ( db->init.busy ){
        sqlite3_Error_സന്ദേശം(pParse,""); /* Err msg generated by corruptSchema() */
      }അല്ലെങ്കിൽ{
        sqlite3CodeVerifySchema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      പോകുക trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#എങ്കിൽ !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  എങ്കിൽ( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_സന്ദേശം(pParse, "cannot create trigger on system table");
    പോകുക trigger_cleanup;
  }
#endif

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  എങ്കിൽ( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_സന്ദേശം(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", pTableName->a);
    പോകുക trigger_orphan_error;
  }
  എങ്കിൽ( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_സന്ദേശം(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", pTableName->a);
    പോകുക trigger_orphan_error;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  എങ്കിൽ( !IN_RENAME_OBJECT ){
    പൂർണ്ണസംഖ്യ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    പൂർണ്ണസംഖ്യ code = SQLITE_CREATE_TRIGGER;
    സ്ഥിരം അക്ഷരം *zDb = db->aDb[iTabDb].zDbSName;
    സ്ഥിരം അക്ഷരം *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    എങ്കിൽ( iTabDb==1 || isTemp ) code = SQLITE_CREATE_TEMP_TRIGGER;
    എങ്കിൽ( sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig) ){
      പോകുക trigger_cleanup;
    }
    എങ്കിൽ( sqlite3AuthCheck(pParse, SQLITE_തിരുകുക, SCHEMA_TABLE(iTabDb),0,zDb)){
      പോകുക trigger_cleanup;
    }
  }
#endif

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  എങ്കിൽ (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, വലുപ്പം(Trigger));
  എങ്കിൽ( pTrigger==0 ) പോകുക trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  എങ്കിൽ( IN_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }അല്ലെങ്കിൽ{
    pTrigger->pWhen = sqlite3_വ്യഞ്ജകം_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  പ്രതിജ്ഞാ( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_വ്യഞ്ജകം_Delete(db, pWhen);
  എങ്കിൽ( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( pParse->pNewTrigger==pTrigger );
  }
  തിരികെ;

trigger_orphan_error:
  എങ്കിൽ( db->init.iDb==1 ){
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
  പോകുക trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
ശൂന്യം sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  അക്ഷരം *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  പൂർണ്ണസംഖ്യ iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  എങ്കിൽ( NEVER(pParse->nErr) || !pTrig ) പോകുക triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
  പ്രതിജ്ഞാ( iDb>=00 && iDb<db->nDb );
  pTrig->step_list = pStepList;
  വേണ്ടി(n=0; pStepList; n++){
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  എങ്കിൽ( n>pParse->db->aLimit[SQLITE_LIMIT_TRIGGER_STEPS] ){
    sqlite3_Error_സന്ദേശം(pParse, "trigger \"%w\" contains too many steps", zName);
    പോകുക triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  എങ്കിൽ( sqlite3FixTriggerStep(&sFix, pTrig->step_list) 
   || sqlite3_Fix_വ്യഞ്ജകം(&sFix, pTrig->pWhen) 
  ){
    പോകുക triggerfinish_cleanup;
  }

#ifndef SQLITE_OMIT_ALTERTABLE
  എങ്കിൽ( IN_RENAME_OBJECT ){
    പ്രതിജ്ഞാ( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }അല്ലെങ്കിൽ
#endif

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  എങ്കിൽ( !db->init.busy ){
    Vdbe *v;
    അക്ഷരം *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    എങ്കിൽ( sqlite3ReadOnlyShadowTables(db) ){
      TriggerStep *pStep;
      വേണ്ടി(pStep=pTrig->step_list; pStep; pStep=pStep->pNext){
        എങ്കിൽ( pStep->pSrc!=0
         && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)
        ){
          sqlite3_Error_സന്ദേശം(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, pStep->pSrc->a[0].zName);
          പോകുക triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    എങ്കിൽ( v==0 ) പോകുക triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (അക്ഷരം*)pAll->z, pAll->n);
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

  എങ്കിൽ( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    പ്രതിജ്ഞാ( pLink!=0 );
    pTrig = sqlite3_Hash_തിരുകുക(pHash, zName, pTrig);
    എങ്കിൽ( pTrig ){
      sqlite3OomFault(db);
    }അല്ലെങ്കിൽ എങ്കിൽ( pLink->pSchema==pLink->pTabSchema ){
      Table *pTab;
      pTab = sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table);
      പ്രതിജ്ഞാ( pTab!=0 );
      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  പ്രതിജ്ഞാ( IN_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3DeleteTriggerStep(db, pStepList);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
സ്ഥിതം അക്ഷരം *triggerSpanDup(sqlite3 *db, സ്ഥിരം അക്ഷരം *zStart, സ്ഥിരം അക്ഷരം *zEnd){
  അക്ഷരം *z = sqlite3DbSpanDup(db, zStart, zEnd);
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( z ) വേണ്ടി(i=0; z[i]; i++) എങ്കിൽ( sqlite3Isspace(z[i]) ) z[i] = ' ';
  തിരികെ z;
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
  സ്ഥിരം അക്ഷരം *zStart,         /* Start of SQL text */
  സ്ഥിരം അക്ഷരം *zEnd            /* End of SQL text */
){
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, വലുപ്പം(TriggerStep));
  എങ്കിൽ( pTriggerStep==0 ) {
    sqlite3SelectDelete(db, pSelect);
    തിരികെ 0;
  }
  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  തിരികെ pTriggerStep;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
സ്ഥിതം TriggerStep *triggerStepAllocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  സ്ഥിരം അക്ഷരം *zStart,         /* Start of SQL text */
  സ്ഥിരം അക്ഷരം *zEnd            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  എങ്കിൽ( pParse->nErr==0 ){
    എങ്കിൽ( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_സന്ദേശം(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }അല്ലെങ്കിൽ{
      pTriggerStep = sqlite3DbMallocZero(db, വലുപ്പം(TriggerStep));
      എങ്കിൽ( pTriggerStep ){
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        എങ്കിൽ( pTriggerStep->pSrc && IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              pTriggerStep->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  തിരികെ pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3_Trigger_തിരുകുക_Step(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  സ്ഥിരം അക്ഷരം *zStart, /* Start of SQL text */
  സ്ഥിരം അക്ഷരം *zEnd    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  പ്രതിജ്ഞാ(pSelect != 0 || db->mallocFailed);

  pTriggerStep = triggerStepAllocate(pParse, TK_തിരുകുക, pTabList, zStart, zEnd);
  എങ്കിൽ( pTriggerStep ){
    എങ്കിൽ( IN_RENAME_OBJECT ){
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    }അല്ലെങ്കിൽ{
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    എങ്കിൽ( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  }അല്ലെങ്കിൽ{
    testcase( pColumn );
    sqlite3IdListDelete(db, pColumn);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  തിരികെ pTriggerStep;
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
  വ്യഞ്ജകം_List *pEList,    /* The SET clause: list of column and new values */
  വ്യഞ്ജകം *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  സ്ഥിരം അക്ഷരം *zStart,  /* Start of SQL text */
  സ്ഥിരം അക്ഷരം *zEnd     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_UPDATE, pTabList, zStart, zEnd);
  എങ്കിൽ( pTriggerStep ){
    SrcList *pFromDup = 0;
    എങ്കിൽ( IN_RENAME_OBJECT ){
      pTriggerStep->p_വ്യഞ്ജകം_List = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }അല്ലെങ്കിൽ{
      pTriggerStep->p_വ്യഞ്ജകം_List = sqlite3_വ്യഞ്ജകം_List_Dup(db, pEList, EXPRDUP_REDUCE);
      pTriggerStep->pWhere = sqlite3_വ്യഞ്ജകം_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = orconf;

    എങ്കിൽ( pFromDup && !IN_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub ,0);
    }
    എങ്കിൽ( pFromDup && pTriggerStep->pSrc ){
      pTriggerStep->pSrc = sqlite3SrcListAppendList(
          pParse, pTriggerStep->pSrc, pFromDup
      );
    }അല്ലെങ്കിൽ{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_വ്യഞ്ജകം_List_Delete(db, pEList);
  sqlite3_വ്യഞ്ജകം_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  തിരികെ pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  വ്യഞ്ജകം *pWhere,           /* The WHERE clause */
  സ്ഥിരം അക്ഷരം *zStart,     /* Start of SQL text */
  സ്ഥിരം അക്ഷരം *zEnd        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, TK_DELETE, pTabList, zStart, zEnd);
  എങ്കിൽ( pTriggerStep ){
    എങ്കിൽ( IN_RENAME_OBJECT ){
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    }അല്ലെങ്കിൽ{
      pTriggerStep->pWhere = sqlite3_വ്യഞ്ജകം_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    pTriggerStep->orconf = OE_Default;
  }
  sqlite3_വ്യഞ്ജകം_Delete(db, pWhere);
  തിരികെ pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
ശൂന്യം sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  എങ്കിൽ( pTrigger==0 || pTrigger->bReturning ) തിരികെ;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3_വ്യഞ്ജകം_Delete(db, pTrigger->pWhen);
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
ശൂന്യം sqlite3DropTrigger(Parse *pParse, SrcList *pName, പൂർണ്ണസംഖ്യ noErr){
  Trigger *pTrigger = 0;
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zDb;
  സ്ഥിരം അക്ഷരം *zName;
  sqlite3 *db = pParse->db;

  എങ്കിൽ( db->mallocFailed ) പോകുക drop_trigger_cleanup;
  എങ്കിൽ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    പോകുക drop_trigger_cleanup;
  }

  പ്രതിജ്ഞാ( pName->nSrc==1 );
  പ്രതിജ്ഞാ( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  പ്രതിജ്ഞാ( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  വേണ്ടി(i=OMIT_TEMPDB; i<db->nDb; i++){
    പൂർണ്ണസംഖ്യ j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    എങ്കിൽ( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) തുടരുക;
    പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName);
    എങ്കിൽ( pTrigger ) നിർത്തുക;
  }
  എങ്കിൽ( !pTrigger ){
    എങ്കിൽ( !noErr ){
      sqlite3_Error_സന്ദേശം(pParse, "no such trigger: %S", pName->a);
    }അല്ലെങ്കിൽ{
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    പോകുക drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
സ്ഥിതം Table *tableOfTrigger(Trigger *pTrigger){
  തിരികെ sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
ശൂന്യം sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  Table   *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  പൂർണ്ണസംഖ്യ iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);
  പ്രതിജ്ഞാ( iDb>=0 && iDb<db->nDb );
  pTable = tableOfTrigger(pTrigger);
  പ്രതിജ്ഞാ( (pTable && pTable->pSchema==pTrigger->pSchema) || iDb==1 );
#ifndef SQLITE_OMIT_AUTHORIZATION
  എങ്കിൽ( pTable ){
    പൂർണ്ണസംഖ്യ code = SQLITE_DROP_TRIGGER;
    സ്ഥിരം അക്ഷരം *zDb = db->aDb[iDb].zDbSName;
    സ്ഥിരം അക്ഷരം *zTab = SCHEMA_TABLE(iDb);
    എങ്കിൽ( iDb==1 ) code = SQLITE_DROP_TEMP_TRIGGER;
    എങ്കിൽ( sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      തിരികെ;
    }
  }
#endif

  /* Generate code to destroy the database record of the trigger.
  */
  എങ്കിൽ( (v = sqlite3GetVdbe(pParse))!=0 ){
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
ശൂന്യം sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, പൂർണ്ണസംഖ്യ iDb, സ്ഥിരം അക്ഷരം *zName){
  Trigger *pTrigger;
  Hash *pHash;

  പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_തിരുകുക(pHash, zName, 0);
  എങ്കിൽ( ALWAYS(pTrigger) ){
    എങ്കിൽ( pTrigger->pSchema==pTrigger->pTabSchema ){
      Table *pTab = tableOfTrigger(pTrigger);
      എങ്കിൽ( pTab ){
        Trigger **pp;
        വേണ്ടി(pp=&pTab->pTrigger; *pp; pp=&((*pp)->pNext)){
          എങ്കിൽ( *pp==pTrigger ){
            *pp = (*pp)->pNext;
            നിർത്തുക;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ checkColumnOverlap(IdList *pIdList, വ്യഞ്ജകം_List *pEList){
  പൂർണ്ണസംഖ്യ e;
  എങ്കിൽ( pIdList==0 || NEVER(pEList==0) ) തിരികെ 1;
  വേണ്ടി(e=0; e<pEList->n_വ്യഞ്ജകം; e++){
    എങ്കിൽ( sqlite3IdListIndex(pIdList, pEList->a[e].zEName)>=0 ) തിരികെ 1;
  }
  തിരികെ 0; 
}

/*
** Return true if any TEMP triggers exist
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tempTriggersExist(sqlite3 *db){
  എങ്കിൽ( NEVER(db->aDb[1].pSchema==0) ) തിരികെ 0;
  എങ്കിൽ( sqliteHashFirst(&db->aDb[1].pSchema->trigHash)==0 ) തിരികെ 0;
  തിരികെ 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
സ്ഥിതം SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  പൂർണ്ണസംഖ്യ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  വ്യഞ്ജകം_List *pChanges,     /* Columns that change in an UPDATE statement */
  പൂർണ്ണസംഖ്യ *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  പൂർണ്ണസംഖ്യ mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  പ്രതിജ്ഞാ( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->pNext==0) );
  എങ്കിൽ( pList!=0 ){
    p = pList;
    എങ്കിൽ( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      എങ്കിൽ( pList==pTab->pTrigger ){
        pList = 0;
        പോകുക exit_triggers_exist;
      }
      ആകുമ്പോൾ( ALWAYS(p->pNext) && p->pNext!=pTab->pTrigger ) p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    ചെയ്യുക{
      എങ്കിൽ( p->op==op && checkColumnOverlap(p->pColumns, pChanges) ){
        mask |= p->tr_tm;
      }അല്ലെങ്കിൽ എങ്കിൽ( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        പ്രതിജ്ഞാ( sqlite3IsToplevel(pParse) );
        p->op = op;
        എങ്കിൽ( IsVirtual(pTab) ){
          എങ്കിൽ( op!=TK_തിരുകുക ){
            sqlite3_Error_സന്ദേശം(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }അല്ലെങ്കിൽ{
          p->tr_tm = TRIGGER_AFTER;
        }
        mask |= p->tr_tm;
      }അല്ലെങ്കിൽ എങ്കിൽ( p->bReturning && p->op==TK_തിരുകുക && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        mask |= p->tr_tm;
      }
      p = p->pNext;
    }ആകുമ്പോൾ( p );
  }
exit_triggers_exist:
  എങ്കിൽ( pMask ){
    *pMask = mask;
  }
  തിരികെ (mask ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  Table *pTab,            /* The table the contains the triggers */
  പൂർണ്ണസംഖ്യ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  വ്യഞ്ജകം_List *pChanges,     /* Columns that change in an UPDATE statement */
  പൂർണ്ണസംഖ്യ *pMask              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  പ്രതിജ്ഞാ( pTab!=0 );
  എങ്കിൽ( (pTab->pTrigger==0 && !tempTriggersExist(pParse->db))
   || pParse->disableTriggers
  ){
    എങ്കിൽ( pMask ) *pMask = 0;
    തിരികെ 0;
  }
  തിരികെ triggersReallyExist(pParse,pTab,op,pChanges,pMask);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isAsteriskTerm(
  Parse *pParse,      /* Parsing context */
  വ്യഞ്ജകം *pTerm         /* A term in the RETURNING clause */
){
  പ്രതിജ്ഞാ( pTerm!=0 );
  എങ്കിൽ( pTerm->op==TK_ASTERISK ) തിരികെ 1;
  എങ്കിൽ( pTerm->op!=TK_DOT ) തിരികെ 0;
  പ്രതിജ്ഞാ( pTerm->p_വലത്!=0 );
  പ്രതിജ്ഞാ( pTerm->p_ഇടത്!=0 );
  എങ്കിൽ( pTerm->p_വലത്->op!=TK_ASTERISK ) തിരികെ 0;
  sqlite3_Error_സന്ദേശം(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  തിരികെ 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
സ്ഥിതം വ്യഞ്ജകം_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  വ്യഞ്ജകം_List *pList,      /* The arguments to RETURNING */
  Table *pTab           /* The table being updated */
){
  വ്യഞ്ജകം_List *pNew = 0;
  sqlite3 *db = pParse->db;
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
    വ്യഞ്ജകം *p_Old_വ്യഞ്ജകം = pList->a[i].p_വ്യഞ്ജകം;
    എങ്കിൽ( NEVER(p_Old_വ്യഞ്ജകം==0) ) തുടരുക;
    എങ്കിൽ( isAsteriskTerm(pParse, p_Old_വ്യഞ്ജകം) ){
      പൂർണ്ണസംഖ്യ jj;
      വേണ്ടി(jj=0; jj<pTab->nCol; jj++){
        വ്യഞ്ജകം *p_New_വ്യഞ്ജകം;
        എങ്കിൽ( IsHiddenColumn(pTab->aCol+jj) ) തുടരുക;
        p_New_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം(db, TK_ID, pTab->aCol[jj].zCnName);
        pNew = sqlite3_വ്യഞ്ജകം_List_Append(pParse, pNew, p_New_വ്യഞ്ജകം);
        എങ്കിൽ( !db->mallocFailed ){
          ഘടന വ്യഞ്ജകം_List_item *pItem = &pNew->a[pNew->n_വ്യഞ്ജകം-1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = ENAME_NAME;
        }
      }
    }അല്ലെങ്കിൽ{
      വ്യഞ്ജകം *p_New_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Dup(db, p_Old_വ്യഞ്ജകം, 0);
      pNew = sqlite3_വ്യഞ്ജകം_List_Append(pParse, pNew, p_New_വ്യഞ്ജകം);
      എങ്കിൽ( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ഘടന വ്യഞ്ജകം_List_item *pItem = &pNew->a[pNew->n_വ്യഞ്ജകം-1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  തിരികെ pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  UNUSED_PARAMETER(NotUsed);
  എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം)
   && (p_വ്യഞ്ജകം->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_VarSelect) );
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_VarSelect);
  }
  തിരികെ WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  പൂർണ്ണസംഖ്യ i;
  SrcList *pSrc;
  പ്രതിജ്ഞാ( pSelect!=0 );
  pSrc = pSelect->pSrc;
  പ്രതിജ്ഞാ( pSrc!=0 );
  വേണ്ടി(i=0; i<pSrc->nSrc; i++){
    എങ്കിൽ( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->eCode = 1;
      നിർത്തുക;
    }
  }
  തിരികെ WRC_Continue;
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
സ്ഥിതം ശൂന്യം sqlite3ProcessReturningSubqueries(
  വ്യഞ്ജകം_List *pEList,
  Table *pTab
){
  Walker w;
  memset(&w, 0, വലുപ്പം(w));
  w.x_വ്യഞ്ജകം_Callback = sqlite3_വ്യഞ്ജകം_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_വ്യഞ്ജകം_List(&w, pEList);
  എങ്കിൽ( w.eCode ){
    w.x_വ്യഞ്ജകം_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_വ്യഞ്ജകം_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
സ്ഥിതം ശൂന്യം codeReturningTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  Table *pTab,         /* The table to code triggers from */
  പൂർണ്ണസംഖ്യ regIn            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  വ്യഞ്ജകം_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  സംഘം {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  പ്രതിജ്ഞാ( v!=0 );
  എങ്കിൽ( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    തിരികെ;
  }
  പ്രതിജ്ഞാ( db->pParse==pParse );
  പ്രതിജ്ഞാ( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  എങ്കിൽ( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    തിരികെ;
  }
  memset(&sSelect, 0, വലുപ്പം(sSelect));
  memset(&uSrc, 0, വലുപ്പം(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_വ്യഞ്ജകം_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  എങ്കിൽ( pParse->nErr==0 ){
    പ്രതിജ്ഞാ( db->mallocFailed==0 );
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3_വ്യഞ്ജകം_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  എങ്കിൽ( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, വലുപ്പം(sNC));
    എങ്കിൽ( pReturning->nRetCol==0 ){
      pReturning->nRetCol = pNew->n_വ്യഞ്ജകം;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      പൂർണ്ണസംഖ്യ i;
      പൂർണ്ണസംഖ്യ nCol = pNew->n_വ്യഞ്ജകം;
      പൂർണ്ണസംഖ്യ reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol+2;
      pReturning->iRetReg = reg;
      വേണ്ടി(i=0; i<nCol; i++){
        വ്യഞ്ജകം *pCol = pNew->a[i].p_വ്യഞ്ജകം;
        പ്രതിജ്ഞാ( pCol!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_വ്യഞ്ജകം_Code_Factorable(pParse, pCol, reg+i);
        എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Affinity(pCol)==SQLITE_AFF_REAL ){
          sqlite3VdbeAddOp1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, reg, i, reg+i);
      sqlite3VdbeAddOp2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3VdbeAddOp3(v, OP_തിരുകുക, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_വ്യഞ്ജകം_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codeTriggerProgram(
  Parse *pParse,            /* The parser context */
  TriggerStep *pStepList,   /* List of statements inside the trigger body */
  പൂർണ്ണസംഖ്യ orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  പ്രതിജ്ഞാ( pParse->pTriggerTab && pParse->pToplevel );
  പ്രതിജ്ഞാ( pStepList );
  പ്രതിജ്ഞാ( v!=0 );
  വേണ്ടി(pStep=pStepList; pStep; pStep=pStep->pNext){
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
    പ്രതിജ്ഞാ( pParse->okConstFactor==0 );

#ifndef SQLITE_OMIT_TRACE
    എങ്കിൽ( pStep->zSpan ){
      sqlite3VdbeAddOp4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", pStep->zSpan),
                        P4_DYNAMIC);
    }
#endif

    തിരഞ്ഞെടുപ്പ്( pStep->op ){
      അവസ്ഥ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_വ്യഞ്ജകം_List_Dup(db, pStep->p_വ്യഞ്ജകം_List, 0), 
          sqlite3_വ്യഞ്ജകം_Dup(db, pStep->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_എണ്ണം);
        നിർത്തുക;
      }
      അവസ്ഥ TK_തിരുകുക: {
        sqlite3_തിരുകുക(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3SelectDup(db, pStep->pSelect, 0), 
          sqlite3IdListDup(db, pStep->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, pStep->pUpsert)
        );
        sqlite3VdbeAddOp0(v, OP_Reset_എണ്ണം);
        നിർത്തുക;
      }
      അവസ്ഥ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, pStep->pSrc, 0),
          sqlite3_വ്യഞ്ജകം_Dup(db, pStep->pWhere, 0), 0, 0
        );
        sqlite3VdbeAddOp0(v, OP_Reset_എണ്ണം);
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: പ്രതിജ്ഞാ( pStep->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        നിർത്തുക;
      }
    } 
  }

  തിരികെ 0;
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *onErrorText(പൂർണ്ണസംഖ്യ onError){
  തിരഞ്ഞെടുപ്പ്( onError ){
    അവസ്ഥ OE_Abort:    തിരികെ "abort";
    അവസ്ഥ OE_Rollback: തിരികെ "rollback";
    അവസ്ഥ OE_Fail:     തിരികെ "fail";
    അവസ്ഥ OE_Replace:  തിരികെ "replace";
    അവസ്ഥ OE_Ignore:   തിരികെ "ignore";
    അവസ്ഥ OE_Default:  തിരികെ "default";
  }
  തിരികെ "n/a";
}
#endif

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
സ്ഥിതം ശൂന്യം transferParseError(Parse *pTo, Parse *pFrom){
  പ്രതിജ്ഞാ( pFrom->z_Err_സന്ദേശം==0 || pFrom->nErr );
  പ്രതിജ്ഞാ( pTo->z_Err_സന്ദേശം==0 || pTo->nErr );
  എങ്കിൽ( pTo->nErr==0 ){
    pTo->z_Err_സന്ദേശം = pFrom->z_Err_സന്ദേശം;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  }അല്ലെങ്കിൽ{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_സന്ദേശം);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
സ്ഥിതം TriggerPrg *codeRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table pTrigger is attached to */
  പൂർണ്ണസംഖ്യ orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_മുകൾ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  വ്യഞ്ജകം *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  SubProgram *pProgram = 0;   /* Sub-vdbe for trigger program */
  പൂർണ്ണസംഖ്യ iEndTrigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  പൂർണ്ണസംഖ്യ nDepth;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_മുകൾ = pParse;
  വേണ്ടി(nDepth=0; p_മുകൾ->pOuterParse; p_മുകൾ = p_മുകൾ->pOuterParse, nDepth++){}
  എങ്കിൽ( nDepth>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    sqlite3_Error_സന്ദേശം(pParse, "triggers nested too deep");
    തിരികെ 0;
  }

  p_മുകൾ = sqlite3ParseToplevel(pParse);
  പ്രതിജ്ഞാ( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );
  പ്രതിജ്ഞാ( p_മുകൾ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, വലുപ്പം(TriggerPrg));
  എങ്കിൽ( !pPrg ) തിരികെ 0;
  pPrg->pNext = p_മുകൾ->pTriggerPrg;
  p_മുകൾ->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = sqlite3DbMallocZero(db, വലുപ്പം(SubProgram));
  എങ്കിൽ( !pProgram ) തിരികെ 0;
  sqlite3VdbeLinkSubProgram(p_മുകൾ->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, വലുപ്പം(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_മുകൾ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  എങ്കിൽ( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, onErrorText(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_തിരുകുക ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ifndef SQLITE_OMIT_TRACE
    എങ്കിൽ( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#endif

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    എങ്കിൽ( pTrigger->pWhen ){
      pWhen = sqlite3_വ്യഞ്ജകം_Dup(db, pTrigger->pWhen, 0);
      എങ്കിൽ( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, pWhen) 
      ){
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_വ്യഞ്ജകം_If_False(&sSubParse, pWhen, iEndTrigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_വ്യഞ്ജകം_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    എങ്കിൽ( iEndTrigger ){
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, onErrorText(orconf)));
    transferParseError(pParse, &sSubParse);

    എങ്കിൽ( pParse->nErr==0 ){
      പ്രതിജ്ഞാ( db->mallocFailed==0 );
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &p_മുകൾ->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (ശൂന്യം *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }അല്ലെങ്കിൽ{
    transferParseError(pParse, &sSubParse);
  }

  പ്രതിജ്ഞാ( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  തിരികെ pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
സ്ഥിതം TriggerPrg *getRowTrigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  Table *pTab,         /* The table trigger pTrigger is attached to */
  പൂർണ്ണസംഖ്യ orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_വേര് = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  പ്രതിജ്ഞാ( pTrigger->zName==0 || pTab==tableOfTrigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  വേണ്ടി(pPrg=p_വേര്->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->pNext
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  എങ്കിൽ( !pPrg ){
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  തിരികെ pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
ശൂന്യം sqlite3CodeRowTriggerDirect(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  Table *pTab,         /* The table to code triggers from */
  പൂർണ്ണസംഖ്യ reg,             /* Reg array containing OLD.* and NEW.* values */
  പൂർണ്ണസംഖ്യ orconf,          /* ON CONFLICT policy */
  പൂർണ്ണസംഖ്യ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);
  പ്രതിജ്ഞാ( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  എങ്കിൽ( pPrg ){
    പൂർണ്ണസംഖ്യ bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3VdbeAddOp4(v, OP_Program, reg, ignoreJump, ++pParse->nMem,
                      (സ്ഥിരം അക്ഷരം *)pPrg->pProgram, P4_SUBPROGRAM);
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
ശൂന്യം sqlite3CodeRowTrigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  പൂർണ്ണസംഖ്യ op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  വ്യഞ്ജകം_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  പൂർണ്ണസംഖ്യ tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  പൂർണ്ണസംഖ്യ reg,             /* The first in an array of registers (see above) */
  പൂർണ്ണസംഖ്യ orconf,          /* ON CONFLICT policy */
  പൂർണ്ണസംഖ്യ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  പ്രതിജ്ഞാ( op==TK_UPDATE || op==TK_തിരുകുക || op==TK_DELETE );
  പ്രതിജ്ഞാ( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  പ്രതിജ്ഞാ( (op==TK_UPDATE)==(pChanges!=0) );

  വേണ്ടി(p=pTrigger; p; p=p->pNext){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    പ്രതിജ്ഞാ( p->pSchema!=0 );
    പ്രതിജ്ഞാ( p->pTabSchema!=0 );
    പ്രതിജ്ഞാ( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    എങ്കിൽ( (p->op==op || (p->bReturning && p->op==TK_തിരുകുക && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && checkColumnOverlap(p->pColumns, pChanges)
    ){
      എങ്കിൽ( !p->bReturning ){
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3IsToplevel(pParse) ){
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
  വ്യഞ്ജകം_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  പൂർണ്ണസംഖ്യ isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  പൂർണ്ണസംഖ്യ tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  Table *pTab,         /* The table to code triggers from */
  പൂർണ്ണസംഖ്യ orconf           /* Default ON CONFLICT policy for trigger steps */
){
  സ്ഥിരം പൂർണ്ണസംഖ്യ op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 mask = 0;
  Trigger *p;

  പ്രതിജ്ഞാ( isNew==1 || isNew==0 );
  എങ്കിൽ( IsView(pTab) ){
    തിരികെ 0xffffffff;
  }
  വേണ്ടി(p=pTrigger; p; p=p->pNext){
    എങ്കിൽ( p->op==op
     && (tr_tm&p->tr_tm)
     && checkColumnOverlap(p->pColumns,pChanges)
    ){
      എങ്കിൽ( p->bReturning ){
        mask = 0xffffffff;
      }അല്ലെങ്കിൽ{
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        എങ്കിൽ( pPrg ){
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  തിരികെ mask;
}

#endif /* !defined(SQLITE_OMIT_TRIGGER) */
