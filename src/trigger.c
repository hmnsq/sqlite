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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
ಶೂನ್ಯ sqlite3_Delete_Trigger_ಹೆಜ್ಜೆ(sqlite3 *db, Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ){
  ಆಗ( p_Trigger_ಹೆಜ್ಜೆ ){
    Trigger_ಹೆಜ್ಜೆ * pTmp = p_Trigger_ಹೆಜ್ಜೆ;
    p_Trigger_ಹೆಜ್ಜೆ = p_Trigger_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ;

    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pTmp->pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pTmp->p_ಅಭಿವ್ಯಕ್ತಿ_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_ಹರವು);

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
Trigger *sqlite3TriggerList(Parse *pParse, ಕೋಷ್ಟಕ *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  ಪ್ರತಿಜ್ಞಾ( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqlite_Hash_ಮೊದಲ(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  ಆಗ( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_ದತ್ತಾಂಶ(p);
    ಆದರೆ( pTrig->pTabSchema==pTab->pSchema
     && pTrig->ಕೋಷ್ಟಕ_2
     && 0==sqlite3StrICmp(pTrig->ಕೋಷ್ಟಕ_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_ಮುಂದಿನ = pList;
      pList = pTrig;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTrig->op==TK_RETURNING ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
      ಪ್ರತಿಜ್ಞಾ( pParse->db->pVtabCtx==0 );
#ಆದರೆ_ಅಂತ್ಯ
      ಪ್ರತಿಜ್ಞಾ( pParse->bReturning );
      ಪ್ರತಿಜ್ಞಾ( !pParse->isCreate );
      ಪ್ರತಿಜ್ಞಾ( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->ಕೋಷ್ಟಕ_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_ಮುಂದಿನ = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_ಮುಂದಿನ(p);    
  }
#ಆದರೆ 0
  ಆದರೆ( pList ){
    Trigger *pX;
    ಬರೆ("Triggers for %s:", pTab->zName);
    ಗಾಗಿ(pX=pList; pX; pX=pX->p_ಮುಂದಿನ){
      ಬರೆ(" %s", pX->zName);
    }
    ಬರೆ("\n");
    fflush(stdout);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ pList;  
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
  ಪೂರ್ಣಾಂಕ tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  ಪೂರ್ಣಾಂಕ op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *p_ಕೋಷ್ಟಕ_Name,/* The name of the table/view the trigger applies to */
  ಅಭಿವ್ಯಕ್ತಿ *pWhen,        /* WHEN clause */
  ಪೂರ್ಣಾಂಕ is_ತಾತ್ಕಾಲಿಕ,         /* True if the TEMPORARY keyword is present */
  ಪೂರ್ಣಾಂಕ noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  ಕೋಷ್ಟಕ *pTab;            /* Table that the trigger fires off of */
  ಅಕ್ಷರ *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  ಪೂರ್ಣಾಂಕ iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ಪ್ರತಿಜ್ಞಾ( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ಪ್ರತಿಜ್ಞಾ( pName2!=0 );
  ಪ್ರತಿಜ್ಞಾ( op==TK_ಸೇರಿಸು || op==TK_UPDATE || op==TK_DELETE );
  ಪ್ರತಿಜ್ಞಾ( op>0 && op<0xff );
  ಆದರೆ( is_ತಾತ್ಕಾಲಿಕ ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    ಆದರೆ( pName2->n>0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "temporary trigger may not have qualified name");
      ಹೋಗು trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    ಆದರೆ( iDb<0 ){
      ಹೋಗು trigger_cleanup;
    }
  }
  ಆದರೆ( !p_ಕೋಷ್ಟಕ_Name || db->mallocFailed ){
    ಹೋಗು trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  ಆದರೆ( db->init.busy && iDb!=1 ){
    ಪ್ರತಿಜ್ಞಾ( p_ಕೋಷ್ಟಕ_Name->a[0].fg.fixedSchema==0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಕೋಷ್ಟಕ_Name->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, p_ಕೋಷ್ಟಕ_Name->a[0].u4.zDatabase);
    p_ಕೋಷ್ಟಕ_Name->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, p_ಕೋಷ್ಟಕ_Name);
  ಆದರೆ( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  ಆದರೆ( db->mallocFailed ) ಹೋಗು trigger_cleanup;
  ಪ್ರತಿಜ್ಞಾ( p_ಕೋಷ್ಟಕ_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  ಆದರೆ( sqlite3FixSrcList(&sFix, p_ಕೋಷ್ಟಕ_Name) ){
    ಹೋಗು trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_ಕೋಷ್ಟಕ_Name);
  ಆದರೆ( !pTab ){
    /* The table does not exist. */
    ಹೋಗು trigger_orphan_error;
  }
  ಆದರೆ( IsVirtual(pTab) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create triggers on virtual tables");
    ಹೋಗು trigger_orphan_error;
  }
  ಆದರೆ( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create triggers on shadow tables");
    ಹೋಗು trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  ಆದರೆ( zName==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಹೋಗು trigger_cleanup;
  }
  ಆದರೆ( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    ಹೋಗು trigger_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಆದರೆ( !ಒಳ_RENAME_OBJECT ){
    ಆದರೆ( sqlite3_Hash_ಕಂಡುಹಿಡಿ(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      ಆದರೆ( !noErr ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "trigger %T already exists", pName);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->init.busy ){
        sqlite3_Error_ಸಂದೇಶ(pParse,""); /* Err msg generated by corruptSchema() */
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      ಹೋಗು trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#ಆದರೆ !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  ಆದರೆ( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create trigger on system table");
    ಹೋಗು trigger_cleanup;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  ಆದರೆ( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_ಕೋಷ್ಟಕ_Name->a);
    ಹೋಗು trigger_orphan_error;
  }
  ಆದರೆ( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_ಕೋಷ್ಟಕ_Name->a);
    ಹೋಗು trigger_orphan_error;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( !ಒಳ_RENAME_OBJECT ){
    ಪೂರ್ಣಾಂಕ iTabDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
    ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = SQLITE_CREATE_TRIGGER;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = db->aDb[iTabDb].zDbSName;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbTrig = is_ತಾತ್ಕಾಲಿಕ ? db->aDb[1].zDbSName : zDb;
    ಆದರೆ( iTabDb==1 || is_ತಾತ್ಕಾಲಿಕ ) ಸಂಕೇತ_2 = SQLITE_CREATE_ತಾತ್ಕಾಲಿಕ_TRIGGER;
    ಆದರೆ( sqlite3AuthCheck(pParse, ಸಂಕೇತ_2, zName, pTab->zName, zDbTrig) ){
      ಹೋಗು trigger_cleanup;
    }
    ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ಸೇರಿಸು, SCHEMA_ಕೋಷ್ಟಕ(iTabDb),0,zDb)){
      ಹೋಗು trigger_cleanup;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  ಆದರೆ (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, ಗಾತ್ರ(Trigger));
  ಆದರೆ( pTrigger==0 ) ಹೋಗು trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->ಕೋಷ್ಟಕ_2 = sqlite3DbStrDup(db, p_ಕೋಷ್ಟಕ_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  ಆದರೆ( ಒಳ_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->ಕೋಷ್ಟಕ_2, p_ಕೋಷ್ಟಕ_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pTrigger->pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_ಕೋಷ್ಟಕ_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhen);
  ಆದರೆ( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==pTrigger );
  }
  ಹಿಂತಿರುಗಿ;

trigger_orphan_error:
  ಆದರೆ( db->init.iDb==1 ){
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
  ಹೋಗು trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
ಶೂನ್ಯ sqlite3_ಮುಕ್ತಾಯ_Trigger(
  Parse *pParse,          /* Parser context */
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ_List, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  ಅಕ್ಷರ *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  ಪೂರ್ಣಾಂಕ iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  ಆದರೆ( NEVER(pParse->nErr) || !pTrig ) ಹೋಗು triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTrig->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=00 && iDb<db->nDb );
  pTrig->ಹೆಜ್ಜೆ_list = p_ಹೆಜ್ಜೆ_List;
  ಗಾಗಿ(n=0; p_ಹೆಜ್ಜೆ_List; n++){
    p_ಹೆಜ್ಜೆ_List->pTrig = pTrig;
    p_ಹೆಜ್ಜೆ_List = p_ಹೆಜ್ಜೆ_List->p_ಮುಂದಿನ;
  }
  ಆದರೆ( n>pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_TRIGGER_ಹೆಜ್ಜೆಗಳು] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "trigger \"%w\" contains too many steps", zName);
    ಹೋಗು triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  ಆದರೆ( sqlite3_Fix_Trigger_ಹೆಜ್ಜೆ(&sFix, pTrig->ಹೆಜ್ಜೆ_list) 
   || sqlite3_Fix_ಅಭಿವ್ಯಕ್ತಿ(&sFix, pTrig->pWhen) 
  ){
    ಹೋಗು triggerfinish_cleanup;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ALTERTABLE
  ಆದರೆ( ಒಳ_RENAME_OBJECT ){
    ಪ್ರತಿಜ್ಞಾ( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  ಆದರೆ( !db->init.busy ){
    Vdbe *v;
    ಅಕ್ಷರ *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    ಆದರೆ( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;
      ಗಾಗಿ(p_ಹೆಜ್ಜೆ=pTrig->ಹೆಜ್ಜೆ_list; p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
        ಆದರೆ( p_ಹೆಜ್ಜೆ->pSrc!=0
         && sqlite3_Shadow_ಕೋಷ್ಟಕ_Name(db, p_ಹೆಜ್ಜೆ->pSrc->a[0].zName)
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_ಹೆಜ್ಜೆ->pSrc->a[0].zName);
          ಹೋಗು triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    ಆದರೆ( v==0 ) ಹೋಗು triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (ಅಕ್ಷರ*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_ಕೋಷ್ಟಕ
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->ಕೋಷ್ಟಕ_2, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ಕೂಡಿಸು_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  ಆದರೆ( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಪ್ರತಿಜ್ಞಾ( pLink!=0 );
    pTrig = sqlite3_Hash_ಸೇರಿಸು(pHash, zName, pTrig);
    ಆದರೆ( pTrig ){
      sqlite3OomFault(db);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pLink->pSchema==pLink->pTabSchema ){
      ಕೋಷ್ಟಕ *pTab;
      pTab = sqlite3_Hash_ಕಂಡುಹಿಡಿ(&pLink->pTabSchema->tblHash, pLink->ಕೋಷ್ಟಕ_2);
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
      pLink->p_ಮುಂದಿನ = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ಪ್ರತಿಜ್ಞಾ( ಒಳ_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_ಹೆಜ್ಜೆ(db, p_ಹೆಜ್ಜೆ_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ಸ್ಥಿರ ಅಕ್ಷರ *trigger_ಹರವು_Dup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ){
  ಅಕ್ಷರ *z = sqlite3_Db_ಹರವು_Dup(db, z_ಆರಂಭ, z_ಕೊನೆ);
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( z ) ಗಾಗಿ(i=0; z[i]; i++) ಆದರೆ( sqlite3Isspace(z[i]) ) z[i] = ' ';
  ಹಿಂತಿರುಗಿ z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Select_ಹೆಜ್ಜೆ(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ,         /* Start of SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ            /* End of SQL text */
){
  Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ = sqlite3DbMallocZero(db, ಗಾತ್ರ(Trigger_ಹೆಜ್ಜೆ));
  ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ==0 ) {
    sqlite3SelectDelete(db, pSelect);
    ಹಿಂತಿರುಗಿ 0;
  }
  p_Trigger_ಹೆಜ್ಜೆ->op = TK_SELECT;
  p_Trigger_ಹೆಜ್ಜೆ->pSelect = pSelect;
  p_Trigger_ಹೆಜ್ಜೆ->orconf = OE_Default;
  p_Trigger_ಹೆಜ್ಜೆ->z_ಹರವು = trigger_ಹರವು_Dup(db, z_ಆರಂಭ, z_ಕೊನೆ);
  ಹಿಂತಿರುಗಿ p_Trigger_ಹೆಜ್ಜೆ;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ಸ್ಥಿರ Trigger_ಹೆಜ್ಜೆ *trigger_ಹೆಜ್ಜೆ_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ,         /* Start of SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ = 0;

  ಆದರೆ( pParse->nErr==0 ){
    ಆದರೆ( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_Trigger_ಹೆಜ್ಜೆ = sqlite3DbMallocZero(db, ಗಾತ್ರ(Trigger_ಹೆಜ್ಜೆ));
      ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ ){
        p_Trigger_ಹೆಜ್ಜೆ->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_ಹೆಜ್ಜೆ->op = op;
        p_Trigger_ಹೆಜ್ಜೆ->z_ಹರವು = trigger_ಹರವು_Dup(db, z_ಆರಂಭ, z_ಕೊನೆ);
        ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ->pSrc && ಒಳ_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_ಹೆಜ್ಜೆ->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  ಹಿಂತಿರುಗಿ p_Trigger_ಹೆಜ್ಜೆ;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_ಸೇರಿಸು_ಹೆಜ್ಜೆ(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *p_ಸ್ತಂಭ,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ, /* Start of SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ;

  ಪ್ರತಿಜ್ಞಾ(pSelect != 0 || db->mallocFailed);

  p_Trigger_ಹೆಜ್ಜೆ = trigger_ಹೆಜ್ಜೆ_Allocate(pParse, TK_ಸೇರಿಸು, pTabList, z_ಆರಂಭ, z_ಕೊನೆ);
  ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ ){
    ಆದರೆ( ಒಳ_RENAME_OBJECT ){
      p_Trigger_ಹೆಜ್ಜೆ->pSelect = pSelect;
      pSelect = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_Trigger_ಹೆಜ್ಜೆ->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_ಹೆಜ್ಜೆ->pIdList = p_ಸ್ತಂಭ;
    p_Trigger_ಹೆಜ್ಜೆ->pUpsert = pUpsert;
    p_Trigger_ಹೆಜ್ಜೆ->orconf = orconf;
    ಆದರೆ( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_ಗುರಿ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( p_ಸ್ತಂಭ );
    sqlite3IdListDelete(db, p_ಸ್ತಂಭ);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  ಹಿಂತಿರುಗಿ p_Trigger_ಹೆಜ್ಜೆ;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Update_ಹೆಜ್ಜೆ(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,    /* The SET clause: list of column and new values */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ,  /* Start of SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ;

  p_Trigger_ಹೆಜ್ಜೆ = trigger_ಹೆಜ್ಜೆ_Allocate(pParse, TK_UPDATE, pTabList, z_ಆರಂಭ, z_ಕೊನೆ);
  ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ ){
    SrcList *pFromDup = 0;
    ಆದರೆ( ಒಳ_RENAME_OBJECT ){
      p_Trigger_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List = pEList;
      p_Trigger_ಹೆಜ್ಜೆ->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_Trigger_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_ಹೆಜ್ಜೆ->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_ಹೆಜ್ಜೆ->orconf = orconf;

    ಆದರೆ( pFromDup && !ಒಳ_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_ಪದ(pParse, 0, 0, 0, &as, pSub ,0);
    }
    ಆದರೆ( pFromDup && p_Trigger_ಹೆಜ್ಜೆ->pSrc ){
      p_Trigger_ಹೆಜ್ಜೆ->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_ಹೆಜ್ಜೆ->pSrc, pFromDup
      );
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pEList);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  ಹಿಂತಿರುಗಿ p_Trigger_ಹೆಜ್ಜೆ;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Delete_ಹೆಜ್ಜೆ(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,           /* The WHERE clause */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ,     /* Start of SQL text */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ಹೆಜ್ಜೆ *p_Trigger_ಹೆಜ್ಜೆ;

  p_Trigger_ಹೆಜ್ಜೆ = trigger_ಹೆಜ್ಜೆ_Allocate(pParse, TK_DELETE, pTabList, z_ಆರಂಭ, z_ಕೊನೆ);
  ಆದರೆ( p_Trigger_ಹೆಜ್ಜೆ ){
    ಆದರೆ( ಒಳ_RENAME_OBJECT ){
      p_Trigger_ಹೆಜ್ಜೆ->pWhere = pWhere;
      pWhere = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_Trigger_ಹೆಜ್ಜೆ->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_ಹೆಜ್ಜೆ->orconf = OE_Default;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhere);
  ಹಿಂತಿರುಗಿ p_Trigger_ಹೆಜ್ಜೆ;
}

/* 
** Recursively delete a Trigger structure
*/
ಶೂನ್ಯ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  ಆದರೆ( pTrigger==0 || pTrigger->bReturning ) ಹಿಂತಿರುಗಿ;
  sqlite3_Delete_Trigger_ಹೆಜ್ಜೆ(db, pTrigger->ಹೆಜ್ಜೆ_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->ಕೋಷ್ಟಕ_2);
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
ಶೂನ್ಯ sqlite3DropTrigger(Parse *pParse, SrcList *pName, ಪೂರ್ಣಾಂಕ noErr){
  Trigger *pTrigger = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  sqlite3 *db = pParse->db;

  ಆದರೆ( db->mallocFailed ) ಹೋಗು drop_trigger_cleanup;
  ಆದರೆ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ಹೋಗು drop_trigger_cleanup;
  }

  ಪ್ರತಿಜ್ಞಾ( pName->nSrc==1 );
  ಪ್ರತಿಜ್ಞಾ( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ಪ್ರತಿಜ್ಞಾ( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  ಗಾಗಿ(i=OMIT_TEMPDB; i<db->nDb; i++){
    ಪೂರ್ಣಾಂಕ j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    ಆದರೆ( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3_Hash_ಕಂಡುಹಿಡಿ(&(db->aDb[j].pSchema->trigHash), zName);
    ಆದರೆ( pTrigger ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( !pTrigger ){
    ಆದರೆ( !noErr ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such trigger: %S", pName->a);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಸಂಕೇತ_Verify_Named_Schema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    ಹೋಗು drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ಸ್ಥಿರ ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ_ದ_Trigger(Trigger *pTrigger){
  ಹಿಂತಿರುಗಿ sqlite3_Hash_ಕಂಡುಹಿಡಿ(&pTrigger->pTabSchema->tblHash, pTrigger->ಕೋಷ್ಟಕ_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
ಶೂನ್ಯ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  ಕೋಷ್ಟಕ   *p_ಕೋಷ್ಟಕ;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ iDb;

  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTrigger->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  p_ಕೋಷ್ಟಕ = ಕೋಷ್ಟಕ_ದ_Trigger(pTrigger);
  ಪ್ರತಿಜ್ಞಾ( (p_ಕೋಷ್ಟಕ && p_ಕೋಷ್ಟಕ->pSchema==pTrigger->pSchema) || iDb==1 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( p_ಕೋಷ್ಟಕ ){
    ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = SQLITE_DROP_TRIGGER;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = db->aDb[iDb].zDbSName;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab = SCHEMA_ಕೋಷ್ಟಕ(iDb);
    ಆದರೆ( iDb==1 ) ಸಂಕೇತ_2 = SQLITE_DROP_ತಾತ್ಕಾಲಿಕ_TRIGGER;
    ಆದರೆ( sqlite3AuthCheck(pParse, ಸಂಕೇತ_2, pTrigger->zName, p_ಕೋಷ್ಟಕ->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      ಹಿಂತಿರುಗಿ;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Generate code to destroy the database record of the trigger.
  */
  ಆದರೆ( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_ಕೋಷ್ಟಕ " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
ಶೂನ್ಯ sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ಸೇರಿಸು(pHash, zName, 0);
  ಆದರೆ( ALWAYS(pTrigger) ){
    ಆದರೆ( pTrigger->pSchema==pTrigger->pTabSchema ){
      ಕೋಷ್ಟಕ *pTab = ಕೋಷ್ಟಕ_ದ_Trigger(pTrigger);
      ಆದರೆ( pTab ){
        Trigger **pp;
        ಗಾಗಿ(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_ಮುಂದಿನ)){
          ಆದರೆ( *pp==pTrigger ){
            *pp = (*pp)->p_ಮುಂದಿನ;
            ನಿಲ್ಲಿಸು;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_ಸ್ತಂಭ_Overlap(IdList *pIdList, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಪೂರ್ಣಾಂಕ e;
  ಆದರೆ( pIdList==0 || NEVER(pEList==0) ) ಹಿಂತಿರುಗಿ 1;
  ಗಾಗಿ(e=0; e<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; e++){
    ಆದರೆ( sqlite3_Id_List_ಸೂಚ್ಯಂಕ(pIdList, pEList->a[e].zEName)>=0 ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ_Triggers_Exist(sqlite3 *db){
  ಆದರೆ( NEVER(db->aDb[1].pSchema==0) ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( sqlite_Hash_ಮೊದಲ(&db->aDb[1].pSchema->trigHash)==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ಸ್ಥಿರ SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  ಕೋಷ್ಟಕ *pTab,            /* The table the contains the triggers */
  ಪೂರ್ಣಾಂಕ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ಪೂರ್ಣಾಂಕ *p_ಮುಖವಾಡ              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ಪ್ರತಿಜ್ಞಾ( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_ಮುಂದಿನ==0) );
  ಆದರೆ( pList!=0 ){
    p = pList;
    ಆದರೆ( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      ಆದರೆ( pList==pTab->pTrigger ){
        pList = 0;
        ಹೋಗು exit_triggers_exist;
      }
      ಆಗ( ALWAYS(p->p_ಮುಂದಿನ) && p->p_ಮುಂದಿನ!=pTab->pTrigger ) p = p->p_ಮುಂದಿನ;
      p->p_ಮುಂದಿನ = 0;
      p = pList;
    }
    ಮಾಡು{
      ಆದರೆ( p->op==op && check_ಸ್ತಂಭ_Overlap(p->pColumns, pChanges) ){
        ಮುಖವಾಡ |= p->tr_tm;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ಪ್ರತಿಜ್ಞಾ( sqlite3IsToplevel(pParse) );
        p->op = op;
        ಆದರೆ( IsVirtual(pTab) ){
          ಆದರೆ( op!=TK_ಸೇರಿಸು ){
            sqlite3_Error_ಸಂದೇಶ(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p->tr_tm = TRIGGER_AFTER;
        }
        ಮುಖವಾಡ |= p->tr_tm;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->bReturning && p->op==TK_ಸೇರಿಸು && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        ಮುಖವಾಡ |= p->tr_tm;
      }
      p = p->p_ಮುಂದಿನ;
    }ಆಗ( p );
  }
exit_triggers_exist:
  ಆದರೆ( p_ಮುಖವಾಡ ){
    *p_ಮುಖವಾಡ = ಮುಖವಾಡ;
  }
  ಹಿಂತಿರುಗಿ (ಮುಖವಾಡ ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  ಕೋಷ್ಟಕ *pTab,            /* The table the contains the triggers */
  ಪೂರ್ಣಾಂಕ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ಪೂರ್ಣಾಂಕ *p_ಮುಖವಾಡ              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಆದರೆ( (pTab->pTrigger==0 && !ತಾತ್ಕಾಲಿಕ_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    ಆದರೆ( p_ಮುಖವಾಡ ) *p_ಮುಖವಾಡ = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ triggersReallyExist(pParse,pTab,op,pChanges,p_ಮುಖವಾಡ);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_Asterisk_ಪದ(
  Parse *pParse,      /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಪದ         /* A term in the RETURNING clause */
){
  ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
  ಆದರೆ( p_ಪದ->op==TK_ASTERISK ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p_ಪದ->op!=TK_DOT ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( p_ಪದ->p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪದ->p_ಎಡ!=0 );
  ಆದರೆ( p_ಪದ->p_ಬಲ->op!=TK_ASTERISK ) ಹಿಂತಿರುಗಿ 0;
  sqlite3_Error_ಸಂದೇಶ(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  ಹಿಂತಿರುಗಿ 1;
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
  ಕೋಷ್ಟಕ *pTab           /* The table being updated */
){
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew = 0;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Old_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( NEVER(p_Old_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಮುಂದುವರಿ;
    ಆದರೆ( is_Asterisk_ಪದ(pParse, p_Old_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಪೂರ್ಣಾಂಕ jj;
      ಗಾಗಿ(jj=0; jj<pTab->n_ಲಂಬಸಾಲು; jj++){
        ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( Is_Hidden_ಸ್ತಂಭ(pTab->a_ಲಂಬಸಾಲು+jj) ) ಮುಂದುವರಿ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, pTab->a_ಲಂಬಸಾಲು[jj].zCnName);
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_New_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( !db->mallocFailed ){
          ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
          p_ಅಂಶ->zEName = sqlite3DbStrDup(db, pTab->a_ಲಂಬಸಾಲು[jj].zCnName);
          p_ಅಂಶ->fg.eEName = ENAME_NAME;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Old_ಅಭಿವ್ಯಕ್ತಿ, 0);
      pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_New_ಅಭಿವ್ಯಕ್ತಿ);
      ಆದರೆ( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
        p_ಅಂಶ->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        p_ಅಂಶ->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3ReturningSubqueryVarSelect(Walker *Not_ಬಳಸಿದ, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ)
   && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) );
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  ಪೂರ್ಣಾಂಕ i;
  SrcList *pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
    ಆದರೆ( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_ಸಂಕೇತ = 1;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
  ಕೋಷ್ಟಕ *pTab
){
  Walker w;
  memset(&w, 0, ಗಾತ್ರ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, pEList);
  ಆದರೆ( w.e_ಸಂಕೇತ ){
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
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  ಕೋಷ್ಟಕ *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಂಕ reg_ಒಳ            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  ಒಕ್ಕೂಟ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಆದರೆ( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  ಆದರೆ( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    ಹಿಂತಿರುಗಿ;
  }
  memset(&sSelect, 0, ಗಾತ್ರ(sSelect));
  memset(&uSrc, 0, ಗಾತ್ರ(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  ಆದರೆ( pParse->nErr==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
    sqlite3_Generate_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, &sSelect);
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  ಆದರೆ( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, ಗಾತ್ರ(sNC));
    ಆದರೆ( pReturning->n_Ret_ಲಂಬಸಾಲು==0 ){
      pReturning->n_Ret_ಲಂಬಸಾಲು = pNew->n_ಅಭಿವ್ಯಕ್ತಿ;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_ತಳ_Reg = reg_ಒಳ;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      ಪೂರ್ಣಾಂಕ i;
      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pNew->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಪೂರ್ಣಾಂಕ reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += n_ಲಂಬಸಾಲು+2;
      pReturning->iRetReg = reg;
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಲಂಬಸಾಲು = pNew->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(pParse, p_ಲಂಬಸಾಲು, reg+i);
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಲಂಬಸಾಲು)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg, i, reg+i);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, pReturning->iRetCur, reg+i, reg+i+1);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Trigger_ಕಾರ್ಯಕ್ರಮ(
  Parse *pParse,            /* The parser context */
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ_List,   /* List of statements inside the trigger body */
  ಪೂರ್ಣಾಂಕ orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ಪ್ರತಿಜ್ಞಾ( pParse->pTriggerTab && pParse->pToplevel );
  ಪ್ರತಿಜ್ಞಾ( p_ಹೆಜ್ಜೆ_List );
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಗಾಗಿ(p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ_List; p_ಹೆಜ್ಜೆ; p_ಹೆಜ್ಜೆ=p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ){
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
    pParse->eOrconf = (orconf==OE_Default)?p_ಹೆಜ್ಜೆ->orconf:(u8)orconf;
    ಪ್ರತಿಜ್ಞಾ( pParse->ok_Const_ಗುಣಕ==0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
    ಆದರೆ( p_ಹೆಜ್ಜೆ->z_ಹರವು ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_ಹೆಜ್ಜೆ->z_ಹರವು),
                        P4_DYNAMIC);
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಆಯ್ಕೆ( p_ಹೆಜ್ಜೆ->op ){
      ಸ್ಥಿತಿ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_ಹೆಜ್ಜೆ->pSrc, 0),
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p_ಹೆಜ್ಜೆ->p_ಅಭಿವ್ಯಕ್ತಿ_List, 0), 
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಹೆಜ್ಜೆ->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Reset_ಎಣಿಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_ಸೇರಿಸು: {
        sqlite3_ಸೇರಿಸು(pParse, 
          sqlite3SrcListDup(db, p_ಹೆಜ್ಜೆ->pSrc, 0),
          sqlite3SelectDup(db, p_ಹೆಜ್ಜೆ->pSelect, 0), 
          sqlite3IdListDup(db, p_ಹೆಜ್ಜೆ->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_ಹೆಜ್ಜೆ->pUpsert)
        );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Reset_ಎಣಿಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_ಹೆಜ್ಜೆ->pSrc, 0),
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಹೆಜ್ಜೆ->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Reset_ಎಣಿಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: ಪ್ರತಿಜ್ಞಾ( p_ಹೆಜ್ಜೆ->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_ಹೆಜ್ಜೆ->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ನಿಲ್ಲಿಸು;
      }
    } 
  }

  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *on_Error_ಪಠ್ಯ(ಪೂರ್ಣಾಂಕ onError){
  ಆಯ್ಕೆ( onError ){
    ಸ್ಥಿತಿ OE_Abort:    ಹಿಂತಿರುಗಿ "abort";
    ಸ್ಥಿತಿ OE_Rollback: ಹಿಂತಿರುಗಿ "rollback";
    ಸ್ಥಿತಿ OE_Fail:     ಹಿಂತಿರುಗಿ "fail";
    ಸ್ಥಿತಿ OE_Replace:  ಹಿಂತಿರುಗಿ "replace";
    ಸ್ಥಿತಿ OE_Ignore:   ಹಿಂತಿರುಗಿ "ignore";
    ಸ್ಥಿತಿ OE_Default:  ಹಿಂತಿರುಗಿ "default";
  }
  ಹಿಂತಿರುಗಿ "n/a";
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ಸ್ಥಿರ ಶೂನ್ಯ transferParseError(Parse *p_ಗಮ್ಯ, Parse *pFrom){
  ಪ್ರತಿಜ್ಞಾ( pFrom->z_Err_ಸಂದೇಶ==0 || pFrom->nErr );
  ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ->z_Err_ಸಂದೇಶ==0 || p_ಗಮ್ಯ->nErr );
  ಆದರೆ( p_ಗಮ್ಯ->nErr==0 ){
    p_ಗಮ್ಯ->z_Err_ಸಂದೇಶ = pFrom->z_Err_ಸಂದೇಶ;
    p_ಗಮ್ಯ->nErr = pFrom->nErr;
    p_ಗಮ್ಯ->rc = pFrom->rc;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_ಸಂದೇಶ);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ಸ್ಥಿರ TriggerPrg *ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  ಕೋಷ್ಟಕ *pTab,         /* The table pTrigger is attached to */
  ಪೂರ್ಣಾಂಕ orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_ತುದಿ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  ಅಭಿವ್ಯಕ್ತಿ *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_ಕಾರ್ಯಕ್ರಮ *p_ಕಾರ್ಯಕ್ರಮ = 0;   /* Sub-vdbe for trigger program */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  ಪೂರ್ಣಾಂಕ n_ಆಳ;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_ತುದಿ = pParse;
  ಗಾಗಿ(n_ಆಳ=0; p_ತುದಿ->pOuterParse; p_ತುದಿ = p_ತುದಿ->pOuterParse, n_ಆಳ++){}
  ಆದರೆ( n_ಆಳ>=db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_TRIGGER_ಆಳ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "triggers nested too deep");
    ಹಿಂತಿರುಗಿ 0;
  }

  p_ತುದಿ = sqlite3ParseToplevel(pParse);
  ಪ್ರತಿಜ್ಞಾ( pTrigger->zName==0 || pTab==ಕೋಷ್ಟಕ_ದ_Trigger(pTrigger) );
  ಪ್ರತಿಜ್ಞಾ( p_ತುದಿ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, ಗಾತ್ರ(TriggerPrg));
  ಆದರೆ( !pPrg ) ಹಿಂತಿರುಗಿ 0;
  pPrg->p_ಮುಂದಿನ = p_ತುದಿ->pTriggerPrg;
  p_ತುದಿ->pTriggerPrg = pPrg;
  pPrg->p_ಕಾರ್ಯಕ್ರಮ = p_ಕಾರ್ಯಕ್ರಮ = sqlite3DbMallocZero(db, ಗಾತ್ರ(Sub_ಕಾರ್ಯಕ್ರಮ));
  ಆದರೆ( !p_ಕಾರ್ಯಕ್ರಮ ) ಹಿಂತಿರುಗಿ 0;
  sqlite3_Vdbe_Link_Sub_ಕಾರ್ಯಕ್ರಮ(p_ತುದಿ->pVdbe, p_ಕಾರ್ಯಕ್ರಮ);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
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
  ಆದರೆ( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, on_Error_ಪಠ್ಯ(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ಸೇರಿಸು ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
    ಆದರೆ( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    ಆದರೆ( pTrigger->pWhen ){
      pWhen = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pTrigger->pWhen, 0);
      ಆದರೆ( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, pWhen) 
      ){
        i_ಕೊನೆ_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(&sSubParse, pWhen, i_ಕೊನೆ_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    ಸಂಕೇತ_Trigger_ಕಾರ್ಯಕ್ರಮ(&sSubParse, pTrigger->ಹೆಜ್ಜೆ_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    ಆದರೆ( i_ಕೊನೆ_Trigger ){
      sqlite3VdbeResolveLabel(v, i_ಕೊನೆ_Trigger);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, on_Error_ಪಠ್ಯ(orconf)));
    transferParseError(pParse, &sSubParse);

    ಆದರೆ( pParse->nErr==0 ){
      ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
      p_ಕಾರ್ಯಕ್ರಮ->aOp = sqlite3VdbeTakeOpArray(v, &p_ಕಾರ್ಯಕ್ರಮ->nOp, &p_ತುದಿ->nMaxArg);
    }
    p_ಕಾರ್ಯಕ್ರಮ->nMem = sSubParse.nMem;
    p_ಕಾರ್ಯಕ್ರಮ->nCsr = sSubParse.nTab;
    p_ಕಾರ್ಯಕ್ರಮ->token = (ಶೂನ್ಯ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }ಇಲ್ಲದಿದ್ದರೆ{
    transferParseError(pParse, &sSubParse);
  }

  ಪ್ರತಿಜ್ಞಾ( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  ಹಿಂತಿರುಗಿ pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ಸ್ಥಿರ TriggerPrg *get_ಅಡ್ಡಸಾಲು_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  ಕೋಷ್ಟಕ *pTab,         /* The table trigger pTrigger is attached to */
  ಪೂರ್ಣಾಂಕ orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_ಬೇರು = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ಪ್ರತಿಜ್ಞಾ( pTrigger->zName==0 || pTab==ಕೋಷ್ಟಕ_ದ_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  ಗಾಗಿ(pPrg=p_ಬೇರು->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_ಮುಂದಿನ
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  ಆದರೆ( !pPrg ){
    pPrg = ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_ಸ್ಥಾನಾಂತರ = -1;
  }

  ಹಿಂತಿರುಗಿ pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger_ನೇರ(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  ಕೋಷ್ಟಕ *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಂಕ reg,             /* Reg array containing OLD.* and NEW.* values */
  ಪೂರ್ಣಾಂಕ orconf,          /* ON CONFLICT policy */
  ಪೂರ್ಣಾಂಕ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = get_ಅಡ್ಡಸಾಲು_Trigger(pParse, p, pTab, orconf);
  ಪ್ರತಿಜ್ಞಾ( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  ಆದರೆ( pPrg ){
    ಪೂರ್ಣಾಂಕ bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_ಕಾರ್ಯಕ್ರಮ, reg, ignoreJump, ++pParse->nMem,
                      (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)pPrg->p_ಕಾರ್ಯಕ್ರಮ, P4_SUBPROGRAM);
    VdbeComment(
        (v, "Call: %s.%s", (p->zName?p->zName:"fkey"), on_Error_ಪಠ್ಯ(orconf)));

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
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  ಪೂರ್ಣಾಂಕ op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ಪೂರ್ಣಾಂಕ tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  ಕೋಷ್ಟಕ *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಂಕ reg,             /* The first in an array of registers (see above) */
  ಪೂರ್ಣಾಂಕ orconf,          /* ON CONFLICT policy */
  ಪೂರ್ಣಾಂಕ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ಪ್ರತಿಜ್ಞಾ( op==TK_UPDATE || op==TK_ಸೇರಿಸು || op==TK_DELETE );
  ಪ್ರತಿಜ್ಞಾ( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ಪ್ರತಿಜ್ಞಾ( (op==TK_UPDATE)==(pChanges!=0) );

  ಗಾಗಿ(p=pTrigger; p; p=p->p_ಮುಂದಿನ){

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
    ಆದರೆ( (p->op==op || (p->bReturning && p->op==TK_ಸೇರಿಸು && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && check_ಸ್ತಂಭ_Overlap(p->pColumns, pChanges)
    ){
      ಆದರೆ( !p->bReturning ){
        sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger_ನೇರ(pParse, p, pTab, reg, orconf, ignoreJump);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3IsToplevel(pParse) ){
        ಸಂಕೇತ_Returning_Trigger(pParse, p, pTab, reg);
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
  ಪೂರ್ಣಾಂಕ isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  ಪೂರ್ಣಾಂಕ tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  ಕೋಷ್ಟಕ *pTab,         /* The table to code triggers from */
  ಪೂರ್ಣಾಂಕ orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 ಮುಖವಾಡ = 0;
  Trigger *p;

  ಪ್ರತಿಜ್ಞಾ( isNew==1 || isNew==0 );
  ಆದರೆ( IsView(pTab) ){
    ಹಿಂತಿರುಗಿ 0xffffffff;
  }
  ಗಾಗಿ(p=pTrigger; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( p->op==op
     && (tr_tm&p->tr_tm)
     && check_ಸ್ತಂಭ_Overlap(p->pColumns,pChanges)
    ){
      ಆದರೆ( p->bReturning ){
        ಮುಖವಾಡ = 0xffffffff;
      }ಇಲ್ಲದಿದ್ದರೆ{
        TriggerPrg *pPrg;
        pPrg = get_ಅಡ್ಡಸಾಲು_Trigger(pParse, p, pTab, orconf);
        ಆದರೆ( pPrg ){
          ಮುಖವಾಡ |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ;
}

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_TRIGGER) */
