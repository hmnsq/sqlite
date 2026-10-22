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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
శూన్య sqlite3_Delete_Trigger_అడుగు(sqlite3 *db, Trigger_అడుగు *p_Trigger_అడుగు){
  అయితే( p_Trigger_అడుగు ){
    Trigger_అడుగు * pTmp = p_Trigger_అడుగు;
    p_Trigger_అడుగు = p_Trigger_అడుగు->p_తదుపరి;

    sqlite3_సమాసం_Delete(db, pTmp->pWhere);
    sqlite3_సమాసం_List_Delete(db, pTmp->p_సమాసం_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_పరిధి);

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
Trigger *sqlite3TriggerList(Parse *pParse, పట్టిక *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  ప్రతిజ్ఞా( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqlite_Hash_మొదటి(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  అయితే( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_దత్తాంశం(p);
    ఒకవేళ( pTrig->pTabSchema==pTab->pSchema
     && pTrig->పట్టిక_2
     && 0==sqlite3StrICmp(pTrig->పట్టిక_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_తదుపరి = pList;
      pList = pTrig;
    }లేకపోతే ఒకవేళ( pTrig->op==TK_RETURNING ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
      ప్రతిజ్ఞా( pParse->db->pVtabCtx==0 );
#అయితే_ముగింపు
      ప్రతిజ్ఞా( pParse->bReturning );
      ప్రతిజ్ఞా( !pParse->isCreate );
      ప్రతిజ్ఞా( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->పట్టిక_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_తదుపరి = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_తదుపరి(p);    
  }
#ఒకవేళ 0
  ఒకవేళ( pList ){
    Trigger *pX;
    రాయి("Triggers for %s:", pTab->zName);
    కొరకు(pX=pList; pX; pX=pX->p_తదుపరి){
      రాయి(" %s", pX->zName);
    }
    రాయి("\n");
    fflush(stdout);
  }
#అయితే_ముగింపు
  తిరిగి pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
శూన్య sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  పూర్ణాంకం tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  పూర్ణాంకం op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *p_పట్టిక_Name,/* The name of the table/view the trigger applies to */
  సమాసం *pWhen,        /* WHEN clause */
  పూర్ణాంకం is_తాత్కాలిక,         /* True if the TEMPORARY keyword is present */
  పూర్ణాంకం noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  పట్టిక *pTab;            /* Table that the trigger fires off of */
  అక్షరం *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  పూర్ణాంకం iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ప్రతిజ్ఞా( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ప్రతిజ్ఞా( pName2!=0 );
  ప్రతిజ్ఞా( op==TK_చొప్పించు || op==TK_UPDATE || op==TK_DELETE );
  ప్రతిజ్ఞా( op>0 && op<0xff );
  ఒకవేళ( is_తాత్కాలిక ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    ఒకవేళ( pName2->n>0 ){
      sqlite3_Error_సందేశం(pParse, "temporary trigger may not have qualified name");
      వెళ్ళు trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }లేకపోతే{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    ఒకవేళ( iDb<0 ){
      వెళ్ళు trigger_cleanup;
    }
  }
  ఒకవేళ( !p_పట్టిక_Name || db->mallocFailed ){
    వెళ్ళు trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  ఒకవేళ( db->init.busy && iDb!=1 ){
    ప్రతిజ్ఞా( p_పట్టిక_Name->a[0].fg.fixedSchema==0 );
    ప్రతిజ్ఞా( p_పట్టిక_Name->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, p_పట్టిక_Name->a[0].u4.zDatabase);
    p_పట్టిక_Name->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, p_పట్టిక_Name);
  ఒకవేళ( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  ఒకవేళ( db->mallocFailed ) వెళ్ళు trigger_cleanup;
  ప్రతిజ్ఞా( p_పట్టిక_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  ఒకవేళ( sqlite3FixSrcList(&sFix, p_పట్టిక_Name) ){
    వెళ్ళు trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_పట్టిక_Name);
  ఒకవేళ( !pTab ){
    /* The table does not exist. */
    వెళ్ళు trigger_orphan_error;
  }
  ఒకవేళ( IsVirtual(pTab) ){
    sqlite3_Error_సందేశం(pParse, "cannot create triggers on virtual tables");
    వెళ్ళు trigger_orphan_error;
  }
  ఒకవేళ( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_సందేశం(pParse, "cannot create triggers on shadow tables");
    వెళ్ళు trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  ఒకవేళ( zName==0 ){
    ప్రతిజ్ఞా( db->mallocFailed );
    వెళ్ళు trigger_cleanup;
  }
  ఒకవేళ( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    వెళ్ళు trigger_cleanup;
  }
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ఒకవేళ( !లోపల_RENAME_OBJECT ){
    ఒకవేళ( sqlite3_Hash_కనుగొను(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      ఒకవేళ( !noErr ){
        sqlite3_Error_సందేశం(pParse, "trigger %T already exists", pName);
      }లేకపోతే ఒకవేళ( db->init.busy ){
        sqlite3_Error_సందేశం(pParse,""); /* Err msg generated by corruptSchema() */
      }లేకపోతే{
        sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      వెళ్ళు trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#ఒకవేళ !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  ఒకవేళ( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_సందేశం(pParse, "cannot create trigger on system table");
    వెళ్ళు trigger_cleanup;
  }
#అయితే_ముగింపు

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  ఒకవేళ( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_సందేశం(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_పట్టిక_Name->a);
    వెళ్ళు trigger_orphan_error;
  }
  ఒకవేళ( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_సందేశం(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_పట్టిక_Name->a);
    వెళ్ళు trigger_orphan_error;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( !లోపల_RENAME_OBJECT ){
    పూర్ణాంకం iTabDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
    పూర్ణాంకం సంకేతం_2 = SQLITE_CREATE_TRIGGER;
    స్థిరరాశి అక్షరం *zDb = db->aDb[iTabDb].zDbSName;
    స్థిరరాశి అక్షరం *zDbTrig = is_తాత్కాలిక ? db->aDb[1].zDbSName : zDb;
    ఒకవేళ( iTabDb==1 || is_తాత్కాలిక ) సంకేతం_2 = SQLITE_CREATE_తాత్కాలిక_TRIGGER;
    ఒకవేళ( sqlite3AuthCheck(pParse, సంకేతం_2, zName, pTab->zName, zDbTrig) ){
      వెళ్ళు trigger_cleanup;
    }
    ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_చొప్పించు, SCHEMA_పట్టిక(iTabDb),0,zDb)){
      వెళ్ళు trigger_cleanup;
    }
  }
#అయితే_ముగింపు

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  ఒకవేళ (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, పరిమాణం(Trigger));
  ఒకవేళ( pTrigger==0 ) వెళ్ళు trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->పట్టిక_2 = sqlite3DbStrDup(db, p_పట్టిక_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  ఒకవేళ( లోపల_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->పట్టిక_2, p_పట్టిక_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }లేకపోతే{
    pTrigger->pWhen = sqlite3_సమాసం_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ప్రతిజ్ఞా( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_పట్టిక_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_సమాసం_Delete(db, pWhen);
  ఒకవేళ( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }లేకపోతే{
    ప్రతిజ్ఞా( pParse->pNewTrigger==pTrigger );
  }
  తిరిగి;

trigger_orphan_error:
  ఒకవేళ( db->init.iDb==1 ){
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
  వెళ్ళు trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
శూన్య sqlite3_ముగింపు_Trigger(
  Parse *pParse,          /* Parser context */
  Trigger_అడుగు *p_అడుగు_List, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  అక్షరం *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  పూర్ణాంకం iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  ఒకవేళ( NEVER(pParse->nErr) || !pTrig ) వెళ్ళు triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTrig->pSchema);
  ప్రతిజ్ఞా( iDb>=00 && iDb<db->nDb );
  pTrig->అడుగు_list = p_అడుగు_List;
  కొరకు(n=0; p_అడుగు_List; n++){
    p_అడుగు_List->pTrig = pTrig;
    p_అడుగు_List = p_అడుగు_List->p_తదుపరి;
  }
  ఒకవేళ( n>pParse->db->a_హద్దు[SQLITE_హద్దు_TRIGGER_అడుగులు] ){
    sqlite3_Error_సందేశం(pParse, "trigger \"%w\" contains too many steps", zName);
    వెళ్ళు triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  ఒకవేళ( sqlite3_Fix_Trigger_అడుగు(&sFix, pTrig->అడుగు_list) 
   || sqlite3_Fix_సమాసం(&sFix, pTrig->pWhen) 
  ){
    వెళ్ళు triggerfinish_cleanup;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ALTERTABLE
  ఒకవేళ( లోపల_RENAME_OBJECT ){
    ప్రతిజ్ఞా( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }లేకపోతే
#అయితే_ముగింపు

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  ఒకవేళ( !db->init.busy ){
    Vdbe *v;
    అక్షరం *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    ఒకవేళ( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_అడుగు *p_అడుగు;
      కొరకు(p_అడుగు=pTrig->అడుగు_list; p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
        ఒకవేళ( p_అడుగు->pSrc!=0
         && sqlite3_Shadow_పట్టిక_Name(db, p_అడుగు->pSrc->a[0].zName)
        ){
          sqlite3_Error_సందేశం(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_అడుగు->pSrc->a[0].zName);
          వెళ్ళు triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    ఒకవేళ( v==0 ) వెళ్ళు triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (అక్షరం*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_పట్టిక
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->పట్టిక_2, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_కూడు_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  ఒకవేళ( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ప్రతిజ్ఞా( pLink!=0 );
    pTrig = sqlite3_Hash_చొప్పించు(pHash, zName, pTrig);
    ఒకవేళ( pTrig ){
      sqlite3OomFault(db);
    }లేకపోతే ఒకవేళ( pLink->pSchema==pLink->pTabSchema ){
      పట్టిక *pTab;
      pTab = sqlite3_Hash_కనుగొను(&pLink->pTabSchema->tblHash, pLink->పట్టిక_2);
      ప్రతిజ్ఞా( pTab!=0 );
      pLink->p_తదుపరి = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ప్రతిజ్ఞా( లోపల_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_అడుగు(db, p_అడుగు_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
స్థిర అక్షరం *trigger_పరిధి_Dup(sqlite3 *db, స్థిరరాశి అక్షరం *z_ప్రారంభం, స్థిరరాశి అక్షరం *z_చివర){
  అక్షరం *z = sqlite3_Db_పరిధి_Dup(db, z_ప్రారంభం, z_చివర);
  పూర్ణాంకం i;
  ఒకవేళ( z ) కొరకు(i=0; z[i]; i++) ఒకవేళ( sqlite3Isspace(z[i]) ) z[i] = ' ';
  తిరిగి z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_అడుగు *sqlite3_Trigger_Select_అడుగు(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  స్థిరరాశి అక్షరం *z_ప్రారంభం,         /* Start of SQL text */
  స్థిరరాశి అక్షరం *z_చివర            /* End of SQL text */
){
  Trigger_అడుగు *p_Trigger_అడుగు = sqlite3DbMallocZero(db, పరిమాణం(Trigger_అడుగు));
  ఒకవేళ( p_Trigger_అడుగు==0 ) {
    sqlite3SelectDelete(db, pSelect);
    తిరిగి 0;
  }
  p_Trigger_అడుగు->op = TK_SELECT;
  p_Trigger_అడుగు->pSelect = pSelect;
  p_Trigger_అడుగు->orconf = OE_Default;
  p_Trigger_అడుగు->z_పరిధి = trigger_పరిధి_Dup(db, z_ప్రారంభం, z_చివర);
  తిరిగి p_Trigger_అడుగు;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
స్థిర Trigger_అడుగు *trigger_అడుగు_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  స్థిరరాశి అక్షరం *z_ప్రారంభం,         /* Start of SQL text */
  స్థిరరాశి అక్షరం *z_చివర            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_అడుగు *p_Trigger_అడుగు = 0;

  ఒకవేళ( pParse->nErr==0 ){
    ఒకవేళ( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_సందేశం(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }లేకపోతే{
      p_Trigger_అడుగు = sqlite3DbMallocZero(db, పరిమాణం(Trigger_అడుగు));
      ఒకవేళ( p_Trigger_అడుగు ){
        p_Trigger_అడుగు->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_అడుగు->op = op;
        p_Trigger_అడుగు->z_పరిధి = trigger_పరిధి_Dup(db, z_ప్రారంభం, z_చివర);
        ఒకవేళ( p_Trigger_అడుగు->pSrc && లోపల_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_అడుగు->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  తిరిగి p_Trigger_అడుగు;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_అడుగు *sqlite3_Trigger_చొప్పించు_అడుగు(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *p_స్తంభం,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  స్థిరరాశి అక్షరం *z_ప్రారంభం, /* Start of SQL text */
  స్థిరరాశి అక్షరం *z_చివర    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_అడుగు *p_Trigger_అడుగు;

  ప్రతిజ్ఞా(pSelect != 0 || db->mallocFailed);

  p_Trigger_అడుగు = trigger_అడుగు_Allocate(pParse, TK_చొప్పించు, pTabList, z_ప్రారంభం, z_చివర);
  ఒకవేళ( p_Trigger_అడుగు ){
    ఒకవేళ( లోపల_RENAME_OBJECT ){
      p_Trigger_అడుగు->pSelect = pSelect;
      pSelect = 0;
    }లేకపోతే{
      p_Trigger_అడుగు->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_అడుగు->pIdList = p_స్తంభం;
    p_Trigger_అడుగు->pUpsert = pUpsert;
    p_Trigger_అడుగు->orconf = orconf;
    ఒకవేళ( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_లక్ష్యం);
    }
  }లేకపోతే{
    testcase( p_స్తంభం );
    sqlite3IdListDelete(db, p_స్తంభం);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  తిరిగి p_Trigger_అడుగు;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_అడుగు *sqlite3_Trigger_Update_అడుగు(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  సమాసం_List *pEList,    /* The SET clause: list of column and new values */
  సమాసం *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  స్థిరరాశి అక్షరం *z_ప్రారంభం,  /* Start of SQL text */
  స్థిరరాశి అక్షరం *z_చివర     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_అడుగు *p_Trigger_అడుగు;

  p_Trigger_అడుగు = trigger_అడుగు_Allocate(pParse, TK_UPDATE, pTabList, z_ప్రారంభం, z_చివర);
  ఒకవేళ( p_Trigger_అడుగు ){
    SrcList *pFromDup = 0;
    ఒకవేళ( లోపల_RENAME_OBJECT ){
      p_Trigger_అడుగు->p_సమాసం_List = pEList;
      p_Trigger_అడుగు->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }లేకపోతే{
      p_Trigger_అడుగు->p_సమాసం_List = sqlite3_సమాసం_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_అడుగు->pWhere = sqlite3_సమాసం_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_అడుగు->orconf = orconf;

    ఒకవేళ( pFromDup && !లోపల_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_పదం(pParse, 0, 0, 0, &as, pSub ,0);
    }
    ఒకవేళ( pFromDup && p_Trigger_అడుగు->pSrc ){
      p_Trigger_అడుగు->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_అడుగు->pSrc, pFromDup
      );
    }లేకపోతే{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_సమాసం_List_Delete(db, pEList);
  sqlite3_సమాసం_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  తిరిగి p_Trigger_అడుగు;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_అడుగు *sqlite3_Trigger_Delete_అడుగు(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  సమాసం *pWhere,           /* The WHERE clause */
  స్థిరరాశి అక్షరం *z_ప్రారంభం,     /* Start of SQL text */
  స్థిరరాశి అక్షరం *z_చివర        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_అడుగు *p_Trigger_అడుగు;

  p_Trigger_అడుగు = trigger_అడుగు_Allocate(pParse, TK_DELETE, pTabList, z_ప్రారంభం, z_చివర);
  ఒకవేళ( p_Trigger_అడుగు ){
    ఒకవేళ( లోపల_RENAME_OBJECT ){
      p_Trigger_అడుగు->pWhere = pWhere;
      pWhere = 0;
    }లేకపోతే{
      p_Trigger_అడుగు->pWhere = sqlite3_సమాసం_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_అడుగు->orconf = OE_Default;
  }
  sqlite3_సమాసం_Delete(db, pWhere);
  తిరిగి p_Trigger_అడుగు;
}

/* 
** Recursively delete a Trigger structure
*/
శూన్య sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  ఒకవేళ( pTrigger==0 || pTrigger->bReturning ) తిరిగి;
  sqlite3_Delete_Trigger_అడుగు(db, pTrigger->అడుగు_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->పట్టిక_2);
  sqlite3_సమాసం_Delete(db, pTrigger->pWhen);
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
శూన్య sqlite3DropTrigger(Parse *pParse, SrcList *pName, పూర్ణాంకం noErr){
  Trigger *pTrigger = 0;
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zDb;
  స్థిరరాశి అక్షరం *zName;
  sqlite3 *db = pParse->db;

  ఒకవేళ( db->mallocFailed ) వెళ్ళు drop_trigger_cleanup;
  ఒకవేళ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    వెళ్ళు drop_trigger_cleanup;
  }

  ప్రతిజ్ఞా( pName->nSrc==1 );
  ప్రతిజ్ఞా( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ప్రతిజ్ఞా( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  కొరకు(i=OMIT_TEMPDB; i<db->nDb; i++){
    పూర్ణాంకం j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    ఒకవేళ( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) కొనసాగు;
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3_Hash_కనుగొను(&(db->aDb[j].pSchema->trigHash), zName);
    ఒకవేళ( pTrigger ) ఆపు;
  }
  ఒకవేళ( !pTrigger ){
    ఒకవేళ( !noErr ){
      sqlite3_Error_సందేశం(pParse, "no such trigger: %S", pName->a);
    }లేకపోతే{
      sqlite3_సంకేతం_Verify_Named_Schema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    వెళ్ళు drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
స్థిర పట్టిక *పట్టిక_యొక్క_Trigger(Trigger *pTrigger){
  తిరిగి sqlite3_Hash_కనుగొను(&pTrigger->pTabSchema->tblHash, pTrigger->పట్టిక_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
శూన్య sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  పట్టిక   *p_పట్టిక;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం iDb;

  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTrigger->pSchema);
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  p_పట్టిక = పట్టిక_యొక్క_Trigger(pTrigger);
  ప్రతిజ్ఞా( (p_పట్టిక && p_పట్టిక->pSchema==pTrigger->pSchema) || iDb==1 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( p_పట్టిక ){
    పూర్ణాంకం సంకేతం_2 = SQLITE_DROP_TRIGGER;
    స్థిరరాశి అక్షరం *zDb = db->aDb[iDb].zDbSName;
    స్థిరరాశి అక్షరం *zTab = SCHEMA_పట్టిక(iDb);
    ఒకవేళ( iDb==1 ) సంకేతం_2 = SQLITE_DROP_తాత్కాలిక_TRIGGER;
    ఒకవేళ( sqlite3AuthCheck(pParse, సంకేతం_2, pTrigger->zName, p_పట్టిక->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      తిరిగి;
    }
  }
#అయితే_ముగింపు

  /* Generate code to destroy the database record of the trigger.
  */
  ఒకవేళ( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_పట్టిక " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_కూడు_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
శూన్య sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, పూర్ణాంకం iDb, స్థిరరాశి అక్షరం *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_చొప్పించు(pHash, zName, 0);
  ఒకవేళ( ALWAYS(pTrigger) ){
    ఒకవేళ( pTrigger->pSchema==pTrigger->pTabSchema ){
      పట్టిక *pTab = పట్టిక_యొక్క_Trigger(pTrigger);
      ఒకవేళ( pTab ){
        Trigger **pp;
        కొరకు(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_తదుపరి)){
          ఒకవేళ( *pp==pTrigger ){
            *pp = (*pp)->p_తదుపరి;
            ఆపు;
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
స్థిర పూర్ణాంకం check_స్తంభం_Overlap(IdList *pIdList, సమాసం_List *pEList){
  పూర్ణాంకం e;
  ఒకవేళ( pIdList==0 || NEVER(pEList==0) ) తిరిగి 1;
  కొరకు(e=0; e<pEList->n_సమాసం; e++){
    ఒకవేళ( sqlite3_Id_List_సూచీ(pIdList, pEList->a[e].zEName)>=0 ) తిరిగి 1;
  }
  తిరిగి 0; 
}

/*
** Return true if any TEMP triggers exist
*/
స్థిర పూర్ణాంకం తాత్కాలిక_Triggers_Exist(sqlite3 *db){
  ఒకవేళ( NEVER(db->aDb[1].pSchema==0) ) తిరిగి 0;
  ఒకవేళ( sqlite_Hash_మొదటి(&db->aDb[1].pSchema->trigHash)==0 ) తిరిగి 0;
  తిరిగి 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
స్థిర SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  పట్టిక *pTab,            /* The table the contains the triggers */
  పూర్ణాంకం op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  సమాసం_List *pChanges,     /* Columns that change in an UPDATE statement */
  పూర్ణాంకం *p_ముసుగు              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  పూర్ణాంకం ముసుగు = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ప్రతిజ్ఞా( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_తదుపరి==0) );
  ఒకవేళ( pList!=0 ){
    p = pList;
    ఒకవేళ( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      ఒకవేళ( pList==pTab->pTrigger ){
        pList = 0;
        వెళ్ళు exit_triggers_exist;
      }
      అయితే( ALWAYS(p->p_తదుపరి) && p->p_తదుపరి!=pTab->pTrigger ) p = p->p_తదుపరి;
      p->p_తదుపరి = 0;
      p = pList;
    }
    చేయి{
      ఒకవేళ( p->op==op && check_స్తంభం_Overlap(p->pColumns, pChanges) ){
        ముసుగు |= p->tr_tm;
      }లేకపోతే ఒకవేళ( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ప్రతిజ్ఞా( sqlite3IsToplevel(pParse) );
        p->op = op;
        ఒకవేళ( IsVirtual(pTab) ){
          ఒకవేళ( op!=TK_చొప్పించు ){
            sqlite3_Error_సందేశం(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }లేకపోతే{
          p->tr_tm = TRIGGER_AFTER;
        }
        ముసుగు |= p->tr_tm;
      }లేకపోతే ఒకవేళ( p->bReturning && p->op==TK_చొప్పించు && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        ముసుగు |= p->tr_tm;
      }
      p = p->p_తదుపరి;
    }అయితే( p );
  }
exit_triggers_exist:
  ఒకవేళ( p_ముసుగు ){
    *p_ముసుగు = ముసుగు;
  }
  తిరిగి (ముసుగు ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  పట్టిక *pTab,            /* The table the contains the triggers */
  పూర్ణాంకం op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  సమాసం_List *pChanges,     /* Columns that change in an UPDATE statement */
  పూర్ణాంకం *p_ముసుగు              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ప్రతిజ్ఞా( pTab!=0 );
  ఒకవేళ( (pTab->pTrigger==0 && !తాత్కాలిక_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    ఒకవేళ( p_ముసుగు ) *p_ముసుగు = 0;
    తిరిగి 0;
  }
  తిరిగి triggersReallyExist(pParse,pTab,op,pChanges,p_ముసుగు);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
స్థిర పూర్ణాంకం is_Asterisk_పదం(
  Parse *pParse,      /* Parsing context */
  సమాసం *p_పదం         /* A term in the RETURNING clause */
){
  ప్రతిజ్ఞా( p_పదం!=0 );
  ఒకవేళ( p_పదం->op==TK_ASTERISK ) తిరిగి 1;
  ఒకవేళ( p_పదం->op!=TK_DOT ) తిరిగి 0;
  ప్రతిజ్ఞా( p_పదం->p_కుడి!=0 );
  ప్రతిజ్ఞా( p_పదం->p_ఎడమ!=0 );
  ఒకవేళ( p_పదం->p_కుడి->op!=TK_ASTERISK ) తిరిగి 0;
  sqlite3_Error_సందేశం(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  తిరిగి 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
స్థిర సమాసం_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  సమాసం_List *pList,      /* The arguments to RETURNING */
  పట్టిక *pTab           /* The table being updated */
){
  సమాసం_List *pNew = 0;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం i;

  కొరకు(i=0; i<pList->n_సమాసం; i++){
    సమాసం *p_Old_సమాసం = pList->a[i].p_సమాసం;
    ఒకవేళ( NEVER(p_Old_సమాసం==0) ) కొనసాగు;
    ఒకవేళ( is_Asterisk_పదం(pParse, p_Old_సమాసం) ){
      పూర్ణాంకం jj;
      కొరకు(jj=0; jj<pTab->n_నిలువు; jj++){
        సమాసం *p_New_సమాసం;
        ఒకవేళ( Is_Hidden_స్తంభం(pTab->a_నిలువు+jj) ) కొనసాగు;
        p_New_సమాసం = sqlite3_సమాసం(db, TK_ID, pTab->a_నిలువు[jj].zCnName);
        pNew = sqlite3_సమాసం_List_Append(pParse, pNew, p_New_సమాసం);
        ఒకవేళ( !db->mallocFailed ){
          నిర్మాణం సమాసం_List_అంశం *p_అంశం = &pNew->a[pNew->n_సమాసం-1];
          p_అంశం->zEName = sqlite3DbStrDup(db, pTab->a_నిలువు[jj].zCnName);
          p_అంశం->fg.eEName = ENAME_NAME;
        }
      }
    }లేకపోతే{
      సమాసం *p_New_సమాసం = sqlite3_సమాసం_Dup(db, p_Old_సమాసం, 0);
      pNew = sqlite3_సమాసం_List_Append(pParse, pNew, p_New_సమాసం);
      ఒకవేళ( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        నిర్మాణం సమాసం_List_అంశం *p_అంశం = &pNew->a[pNew->n_సమాసం-1];
        p_అంశం->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        p_అంశం->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  తిరిగి pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
స్థిర పూర్ణాంకం sqlite3ReturningSubqueryVarSelect(Walker *Not_వాడిన, సమాసం *p_సమాసం){
  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం)
   && (p_సమాసం->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( సమాసం_Has_Property(p_సమాసం, EP_VarSelect) );
    సమాసం_సమితి_Property(p_సమాసం, EP_VarSelect);
  }
  తిరిగి WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
స్థిర పూర్ణాంకం sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  పూర్ణాంకం i;
  SrcList *pSrc;
  ప్రతిజ్ఞా( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ప్రతిజ్ఞా( pSrc!=0 );
  కొరకు(i=0; i<pSrc->nSrc; i++){
    ఒకవేళ( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_సంకేతం = 1;
      ఆపు;
    }
  }
  తిరిగి WRC_Continue;
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
స్థిర శూన్య sqlite3ProcessReturningSubqueries(
  సమాసం_List *pEList,
  పట్టిక *pTab
){
  Walker w;
  memset(&w, 0, పరిమాణం(w));
  w.x_సమాసం_Callback = sqlite3_సమాసం_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_సమాసం_List(&w, pEList);
  ఒకవేళ( w.e_సంకేతం ){
    w.x_సమాసం_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_సమాసం_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
స్థిర శూన్య సంకేతం_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  పట్టిక *pTab,         /* The table to code triggers from */
  పూర్ణాంకం reg_లోపల            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  సమాసం_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  సంఘం {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ప్రతిజ్ఞా( v!=0 );
  ఒకవేళ( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    తిరిగి;
  }
  ప్రతిజ్ఞా( db->pParse==pParse );
  ప్రతిజ్ఞా( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  ఒకవేళ( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    తిరిగి;
  }
  memset(&sSelect, 0, పరిమాణం(sSelect));
  memset(&uSrc, 0, పరిమాణం(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_సమాసం_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  ఒకవేళ( pParse->nErr==0 ){
    ప్రతిజ్ఞా( db->mallocFailed==0 );
    sqlite3_Generate_స్తంభం_పేర్లు(pParse, &sSelect);
  }
  sqlite3_సమాసం_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  ఒకవేళ( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, పరిమాణం(sNC));
    ఒకవేళ( pReturning->n_Ret_నిలువు==0 ){
      pReturning->n_Ret_నిలువు = pNew->n_సమాసం;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_పునాది_Reg = reg_లోపల;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    ఒకవేళ( sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      పూర్ణాంకం i;
      పూర్ణాంకం n_నిలువు = pNew->n_సమాసం;
      పూర్ణాంకం reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += n_నిలువు+2;
      pReturning->iRetReg = reg;
      కొరకు(i=0; i<n_నిలువు; i++){
        సమాసం *p_నిలువు = pNew->a[i].p_సమాసం;
        ప్రతిజ్ఞా( p_నిలువు!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_సమాసం_సంకేతం_Factorable(pParse, p_నిలువు, reg+i);
        ఒకవేళ( sqlite3_సమాసం_Affinity(p_నిలువు)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg, i, reg+i);
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_సమాసం_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
స్థిర పూర్ణాంకం సంకేతం_Trigger_కార్యక్రమం(
  Parse *pParse,            /* The parser context */
  Trigger_అడుగు *p_అడుగు_List,   /* List of statements inside the trigger body */
  పూర్ణాంకం orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_అడుగు *p_అడుగు;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ప్రతిజ్ఞా( pParse->pTriggerTab && pParse->pToplevel );
  ప్రతిజ్ఞా( p_అడుగు_List );
  ప్రతిజ్ఞా( v!=0 );
  కొరకు(p_అడుగు=p_అడుగు_List; p_అడుగు; p_అడుగు=p_అడుగు->p_తదుపరి){
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
    pParse->eOrconf = (orconf==OE_Default)?p_అడుగు->orconf:(u8)orconf;
    ప్రతిజ్ఞా( pParse->ok_Const_గుణకం==0 );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
    ఒకవేళ( p_అడుగు->z_పరిధి ){
      sqlite3_Vdbe_కూడు_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_అడుగు->z_పరిధి),
                        P4_DYNAMIC);
    }
#అయితే_ముగింపు

    ఎంపిక( p_అడుగు->op ){
      స్థితి TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_అడుగు->pSrc, 0),
          sqlite3_సమాసం_List_Dup(db, p_అడుగు->p_సమాసం_List, 0), 
          sqlite3_సమాసం_Dup(db, p_అడుగు->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_కూడు_Op0(v, OP_Reset_లెక్క);
        ఆపు;
      }
      స్థితి TK_చొప్పించు: {
        sqlite3_చొప్పించు(pParse, 
          sqlite3SrcListDup(db, p_అడుగు->pSrc, 0),
          sqlite3SelectDup(db, p_అడుగు->pSelect, 0), 
          sqlite3IdListDup(db, p_అడుగు->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_అడుగు->pUpsert)
        );
        sqlite3_Vdbe_కూడు_Op0(v, OP_Reset_లెక్క);
        ఆపు;
      }
      స్థితి TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_అడుగు->pSrc, 0),
          sqlite3_సమాసం_Dup(db, p_అడుగు->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_కూడు_Op0(v, OP_Reset_లెక్క);
        ఆపు;
      }
      అప్రమేయ: ప్రతిజ్ఞా( p_అడుగు->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_అడుగు->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ఆపు;
      }
    } 
  }

  తిరిగి 0;
}

#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
స్థిర స్థిరరాశి అక్షరం *on_Error_పాఠ్యం(పూర్ణాంకం onError){
  ఎంపిక( onError ){
    స్థితి OE_Abort:    తిరిగి "abort";
    స్థితి OE_Rollback: తిరిగి "rollback";
    స్థితి OE_Fail:     తిరిగి "fail";
    స్థితి OE_Replace:  తిరిగి "replace";
    స్థితి OE_Ignore:   తిరిగి "ignore";
    స్థితి OE_Default:  తిరిగి "default";
  }
  తిరిగి "n/a";
}
#అయితే_ముగింపు

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
స్థిర శూన్య transferParseError(Parse *p_గమ్యం, Parse *pFrom){
  ప్రతిజ్ఞా( pFrom->z_Err_సందేశం==0 || pFrom->nErr );
  ప్రతిజ్ఞా( p_గమ్యం->z_Err_సందేశం==0 || p_గమ్యం->nErr );
  ఒకవేళ( p_గమ్యం->nErr==0 ){
    p_గమ్యం->z_Err_సందేశం = pFrom->z_Err_సందేశం;
    p_గమ్యం->nErr = pFrom->nErr;
    p_గమ్యం->rc = pFrom->rc;
  }లేకపోతే{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_సందేశం);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
స్థిర TriggerPrg *సంకేతం_అడ్డువరుస_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  పట్టిక *pTab,         /* The table pTrigger is attached to */
  పూర్ణాంకం orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_శిఖరం;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  సమాసం *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_కార్యక్రమం *p_కార్యక్రమం = 0;   /* Sub-vdbe for trigger program */
  పూర్ణాంకం i_చివర_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  పూర్ణాంకం n_లోతు;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_శిఖరం = pParse;
  కొరకు(n_లోతు=0; p_శిఖరం->pOuterParse; p_శిఖరం = p_శిఖరం->pOuterParse, n_లోతు++){}
  ఒకవేళ( n_లోతు>=db->a_హద్దు[SQLITE_హద్దు_TRIGGER_లోతు] ){
    sqlite3_Error_సందేశం(pParse, "triggers nested too deep");
    తిరిగి 0;
  }

  p_శిఖరం = sqlite3ParseToplevel(pParse);
  ప్రతిజ్ఞా( pTrigger->zName==0 || pTab==పట్టిక_యొక్క_Trigger(pTrigger) );
  ప్రతిజ్ఞా( p_శిఖరం->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, పరిమాణం(TriggerPrg));
  ఒకవేళ( !pPrg ) తిరిగి 0;
  pPrg->p_తదుపరి = p_శిఖరం->pTriggerPrg;
  p_శిఖరం->pTriggerPrg = pPrg;
  pPrg->p_కార్యక్రమం = p_కార్యక్రమం = sqlite3DbMallocZero(db, పరిమాణం(Sub_కార్యక్రమం));
  ఒకవేళ( !p_కార్యక్రమం ) తిరిగి 0;
  sqlite3_Vdbe_Link_Sub_కార్యక్రమం(p_శిఖరం->pVdbe, p_కార్యక్రమం);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_శిఖరం;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  ఒకవేళ( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, on_Error_పాఠ్యం(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_చొప్పించు ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
    ఒకవేళ( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#అయితే_ముగింపు

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    ఒకవేళ( pTrigger->pWhen ){
      pWhen = sqlite3_సమాసం_Dup(db, pTrigger->pWhen, 0);
      ఒకవేళ( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_సమాసం_పేర్లు(&sNC, pWhen) 
      ){
        i_చివర_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_సమాసం_If_False(&sSubParse, pWhen, i_చివర_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_సమాసం_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    సంకేతం_Trigger_కార్యక్రమం(&sSubParse, pTrigger->అడుగు_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    ఒకవేళ( i_చివర_Trigger ){
      sqlite3VdbeResolveLabel(v, i_చివర_Trigger);
    }
    sqlite3_Vdbe_కూడు_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, on_Error_పాఠ్యం(orconf)));
    transferParseError(pParse, &sSubParse);

    ఒకవేళ( pParse->nErr==0 ){
      ప్రతిజ్ఞా( db->mallocFailed==0 );
      p_కార్యక్రమం->aOp = sqlite3VdbeTakeOpArray(v, &p_కార్యక్రమం->nOp, &p_శిఖరం->nMaxArg);
    }
    p_కార్యక్రమం->nMem = sSubParse.nMem;
    p_కార్యక్రమం->nCsr = sSubParse.nTab;
    p_కార్యక్రమం->token = (శూన్య *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }లేకపోతే{
    transferParseError(pParse, &sSubParse);
  }

  ప్రతిజ్ఞా( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  తిరిగి pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
స్థిర TriggerPrg *get_అడ్డువరుస_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  పట్టిక *pTab,         /* The table trigger pTrigger is attached to */
  పూర్ణాంకం orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_మూలం = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ప్రతిజ్ఞా( pTrigger->zName==0 || pTab==పట్టిక_యొక్క_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  కొరకు(pPrg=p_మూలం->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_తదుపరి
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  ఒకవేళ( !pPrg ){
    pPrg = సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_స్థానభ్రంశం = -1;
  }

  తిరిగి pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
శూన్య sqlite3_సంకేతం_అడ్డువరుస_Trigger_ప్రత్యక్ష(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  పట్టిక *pTab,         /* The table to code triggers from */
  పూర్ణాంకం reg,             /* Reg array containing OLD.* and NEW.* values */
  పూర్ణాంకం orconf,          /* ON CONFLICT policy */
  పూర్ణాంకం ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = get_అడ్డువరుస_Trigger(pParse, p, pTab, orconf);
  ప్రతిజ్ఞా( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  ఒకవేళ( pPrg ){
    పూర్ణాంకం bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_కూడు_Op4(v, OP_కార్యక్రమం, reg, ignoreJump, ++pParse->nMem,
                      (స్థిరరాశి అక్షరం *)pPrg->p_కార్యక్రమం, P4_SUBPROGRAM);
    VdbeComment(
        (v, "Call: %s.%s", (p->zName?p->zName:"fkey"), on_Error_పాఠ్యం(orconf)));

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
శూన్య sqlite3_సంకేతం_అడ్డువరుస_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  పూర్ణాంకం op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  సమాసం_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  పూర్ణాంకం tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  పట్టిక *pTab,         /* The table to code triggers from */
  పూర్ణాంకం reg,             /* The first in an array of registers (see above) */
  పూర్ణాంకం orconf,          /* ON CONFLICT policy */
  పూర్ణాంకం ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ప్రతిజ్ఞా( op==TK_UPDATE || op==TK_చొప్పించు || op==TK_DELETE );
  ప్రతిజ్ఞా( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ప్రతిజ్ఞా( (op==TK_UPDATE)==(pChanges!=0) );

  కొరకు(p=pTrigger; p; p=p->p_తదుపరి){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    ప్రతిజ్ఞా( p->pSchema!=0 );
    ప్రతిజ్ఞా( p->pTabSchema!=0 );
    ప్రతిజ్ఞా( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    ఒకవేళ( (p->op==op || (p->bReturning && p->op==TK_చొప్పించు && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && check_స్తంభం_Overlap(p->pColumns, pChanges)
    ){
      ఒకవేళ( !p->bReturning ){
        sqlite3_సంకేతం_అడ్డువరుస_Trigger_ప్రత్యక్ష(pParse, p, pTab, reg, orconf, ignoreJump);
      }లేకపోతే ఒకవేళ( sqlite3IsToplevel(pParse) ){
        సంకేతం_Returning_Trigger(pParse, p, pTab, reg);
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
  సమాసం_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  పూర్ణాంకం isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  పూర్ణాంకం tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  పట్టిక *pTab,         /* The table to code triggers from */
  పూర్ణాంకం orconf           /* Default ON CONFLICT policy for trigger steps */
){
  స్థిరరాశి పూర్ణాంకం op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 ముసుగు = 0;
  Trigger *p;

  ప్రతిజ్ఞా( isNew==1 || isNew==0 );
  ఒకవేళ( IsView(pTab) ){
    తిరిగి 0xffffffff;
  }
  కొరకు(p=pTrigger; p; p=p->p_తదుపరి){
    ఒకవేళ( p->op==op
     && (tr_tm&p->tr_tm)
     && check_స్తంభం_Overlap(p->pColumns,pChanges)
    ){
      ఒకవేళ( p->bReturning ){
        ముసుగు = 0xffffffff;
      }లేకపోతే{
        TriggerPrg *pPrg;
        pPrg = get_అడ్డువరుస_Trigger(pParse, p, pTab, orconf);
        ఒకవేళ( pPrg ){
          ముసుగు |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  తిరిగి ముసుగు;
}

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_TRIGGER) */
