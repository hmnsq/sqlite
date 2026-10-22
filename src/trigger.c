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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
ਖਾਲੀ sqlite3_Delete_Trigger_ਕਦਮ(sqlite3 *db, Trigger_ਕਦਮ *p_Trigger_ਕਦਮ){
  ਜਦੋਂ( p_Trigger_ਕਦਮ ){
    Trigger_ਕਦਮ * pTmp = p_Trigger_ਕਦਮ;
    p_Trigger_ਕਦਮ = p_Trigger_ਕਦਮ->p_ਅਗਲਾ;

    sqlite3_ਵਿਅੰਜਕ_Delete(db, pTmp->pWhere);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pTmp->p_ਵਿਅੰਜਕ_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_ਵਿਸਤਾਰ);

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
Trigger *sqlite3TriggerList(Parse *pParse, ਸਾਰਣੀ *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqlite_Hash_ਪਹਿਲਾ(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  ਜਦੋਂ( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_ਅੰਕੜੇ(p);
    ਜੇ( pTrig->pTabSchema==pTab->pSchema
     && pTrig->ਸਾਰਣੀ_2
     && 0==sqlite3StrICmp(pTrig->ਸਾਰਣੀ_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_ਅਗਲਾ = pList;
      pList = pTrig;
    }ਨਹੀਂਤਾਂ ਜੇ( pTrig->op==TK_RETURNING ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->pVtabCtx==0 );
#ਜੇ_ਅੰਤ
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->bReturning );
      ਪ੍ਰਤਿਜ੍ਞਾ( !pParse->isCreate );
      ਪ੍ਰਤਿਜ੍ਞਾ( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->ਸਾਰਣੀ_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_ਅਗਲਾ = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_ਅਗਲਾ(p);    
  }
#ਜੇ 0
  ਜੇ( pList ){
    Trigger *pX;
    ਲਿਖੋ("Triggers for %s:", pTab->zName);
    ਲਈ(pX=pList; pX; pX=pX->p_ਅਗਲਾ){
      ਲਿਖੋ(" %s", pX->zName);
    }
    ਲਿਖੋ("\n");
    fflush(stdout);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
ਖਾਲੀ sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  ਪੂਰਨ tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  ਪੂਰਨ op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *p_ਸਾਰਣੀ_Name,/* The name of the table/view the trigger applies to */
  ਵਿਅੰਜਕ *pWhen,        /* WHEN clause */
  ਪੂਰਨ is_ਆਰਜ਼ੀ,         /* True if the TEMPORARY keyword is present */
  ਪੂਰਨ noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  ਸਾਰਣੀ *pTab;            /* Table that the trigger fires off of */
  ਅੱਖਰ *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  ਪੂਰਨ iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  ਪ੍ਰਤਿਜ੍ਞਾ( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  ਪ੍ਰਤਿਜ੍ਞਾ( pName2!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_ਪਾਓ || op==TK_UPDATE || op==TK_DELETE );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>0 && op<0xff );
  ਜੇ( is_ਆਰਜ਼ੀ ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    ਜੇ( pName2->n>0 ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "temporary trigger may not have qualified name");
      ਜਾਓ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }ਨਹੀਂਤਾਂ{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    ਜੇ( iDb<0 ){
      ਜਾਓ trigger_cleanup;
    }
  }
  ਜੇ( !p_ਸਾਰਣੀ_Name || db->mallocFailed ){
    ਜਾਓ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  ਜੇ( db->init.busy && iDb!=1 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਾਰਣੀ_Name->a[0].fg.fixedSchema==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਾਰਣੀ_Name->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, p_ਸਾਰਣੀ_Name->a[0].u4.zDatabase);
    p_ਸਾਰਣੀ_Name->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, p_ਸਾਰਣੀ_Name);
  ਜੇ( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  ਜੇ( db->mallocFailed ) ਜਾਓ trigger_cleanup;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਾਰਣੀ_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  ਜੇ( sqlite3FixSrcList(&sFix, p_ਸਾਰਣੀ_Name) ){
    ਜਾਓ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_ਸਾਰਣੀ_Name);
  ਜੇ( !pTab ){
    /* The table does not exist. */
    ਜਾਓ trigger_orphan_error;
  }
  ਜੇ( IsVirtual(pTab) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot create triggers on virtual tables");
    ਜਾਓ trigger_orphan_error;
  }
  ਜੇ( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot create triggers on shadow tables");
    ਜਾਓ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  ਜੇ( zName==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed );
    ਜਾਓ trigger_cleanup;
  }
  ਜੇ( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    ਜਾਓ trigger_cleanup;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ਜੇ( !ਵਿੱਚ_RENAME_OBJECT ){
    ਜੇ( sqlite3_Hash_ਲੱਭੋ(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      ਜੇ( !noErr ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "trigger %T already exists", pName);
      }ਨਹੀਂਤਾਂ ਜੇ( db->init.busy ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse,""); /* Err msg generated by corruptSchema() */
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      ਜਾਓ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#ਜੇ !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  ਜੇ( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot create trigger on system table");
    ਜਾਓ trigger_cleanup;
  }
#ਜੇ_ਅੰਤ

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  ਜੇ( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_ਸਾਰਣੀ_Name->a);
    ਜਾਓ trigger_orphan_error;
  }
  ਜੇ( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_ਸਾਰਣੀ_Name->a);
    ਜਾਓ trigger_orphan_error;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( !ਵਿੱਚ_RENAME_OBJECT ){
    ਪੂਰਨ iTabDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
    ਪੂਰਨ ਕੋਡ = SQLITE_CREATE_TRIGGER;
    ਅਚਲ ਅੱਖਰ *zDb = db->aDb[iTabDb].zDbSName;
    ਅਚਲ ਅੱਖਰ *zDbTrig = is_ਆਰਜ਼ੀ ? db->aDb[1].zDbSName : zDb;
    ਜੇ( iTabDb==1 || is_ਆਰਜ਼ੀ ) ਕੋਡ = SQLITE_CREATE_ਆਰਜ਼ੀ_TRIGGER;
    ਜੇ( sqlite3AuthCheck(pParse, ਕੋਡ, zName, pTab->zName, zDbTrig) ){
      ਜਾਓ trigger_cleanup;
    }
    ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ਪਾਓ, SCHEMA_ਸਾਰਣੀ(iTabDb),0,zDb)){
      ਜਾਓ trigger_cleanup;
    }
  }
#ਜੇ_ਅੰਤ

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  ਜੇ (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, ਆਕਾਰ(Trigger));
  ਜੇ( pTrigger==0 ) ਜਾਓ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->ਸਾਰਣੀ_2 = sqlite3DbStrDup(db, p_ਸਾਰਣੀ_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->ਸਾਰਣੀ_2, p_ਸਾਰਣੀ_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }ਨਹੀਂਤਾਂ{
    pTrigger->pWhen = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_ਸਾਰਣੀ_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhen);
  ਜੇ( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pNewTrigger==pTrigger );
  }
  ਵਾਪਸ;

trigger_orphan_error:
  ਜੇ( db->init.iDb==1 ){
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
  ਜਾਓ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
ਖਾਲੀ sqlite3_ਅੰਤ_Trigger(
  Parse *pParse,          /* Parser context */
  Trigger_ਕਦਮ *p_ਕਦਮ_List, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  ਅੱਖਰ *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  ਪੂਰਨ iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  ਜੇ( NEVER(pParse->nErr) || !pTrig ) ਜਾਓ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTrig->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=00 && iDb<db->nDb );
  pTrig->ਕਦਮ_list = p_ਕਦਮ_List;
  ਲਈ(n=0; p_ਕਦਮ_List; n++){
    p_ਕਦਮ_List->pTrig = pTrig;
    p_ਕਦਮ_List = p_ਕਦਮ_List->p_ਅਗਲਾ;
  }
  ਜੇ( n>pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_TRIGGER_ਕਦਮਾਂ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "trigger \"%w\" contains too many steps", zName);
    ਜਾਓ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  ਜੇ( sqlite3_Fix_Trigger_ਕਦਮ(&sFix, pTrig->ਕਦਮ_list) 
   || sqlite3_Fix_ਵਿਅੰਜਕ(&sFix, pTrig->pWhen) 
  ){
    ਜਾਓ triggerfinish_cleanup;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ALTERTABLE
  ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  ਜੇ( !db->init.busy ){
    Vdbe *v;
    ਅੱਖਰ *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    ਜੇ( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_ਕਦਮ *p_ਕਦਮ;
      ਲਈ(p_ਕਦਮ=pTrig->ਕਦਮ_list; p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
        ਜੇ( p_ਕਦਮ->pSrc!=0
         && sqlite3_Shadow_ਸਾਰਣੀ_Name(db, p_ਕਦਮ->pSrc->a[0].zName)
        ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_ਕਦਮ->pSrc->a[0].zName);
          ਜਾਓ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    ਜੇ( v==0 ) ਜਾਓ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (ਅੱਖਰ*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_ਸਾਰਣੀ
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->ਸਾਰਣੀ_2, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ਜੋੜੋ_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  ਜੇ( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pLink!=0 );
    pTrig = sqlite3_Hash_ਪਾਓ(pHash, zName, pTrig);
    ਜੇ( pTrig ){
      sqlite3OomFault(db);
    }ਨਹੀਂਤਾਂ ਜੇ( pLink->pSchema==pLink->pTabSchema ){
      ਸਾਰਣੀ *pTab;
      pTab = sqlite3_Hash_ਲੱਭੋ(&pLink->pTabSchema->tblHash, pLink->ਸਾਰਣੀ_2);
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
      pLink->p_ਅਗਲਾ = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਚ_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_ਕਦਮ(db, p_ਕਦਮ_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ਸਥਿਰ ਅੱਖਰ *trigger_ਵਿਸਤਾਰ_Dup(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ, ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ){
  ਅੱਖਰ *z = sqlite3_Db_ਵਿਸਤਾਰ_Dup(db, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
  ਪੂਰਨ i;
  ਜੇ( z ) ਲਈ(i=0; z[i]; i++) ਜੇ( sqlite3Isspace(z[i]) ) z[i] = ' ';
  ਵਾਪਸ z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_ਕਦਮ *sqlite3_Trigger_Select_ਕਦਮ(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ,         /* Start of SQL text */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ            /* End of SQL text */
){
  Trigger_ਕਦਮ *p_Trigger_ਕਦਮ = sqlite3DbMallocZero(db, ਆਕਾਰ(Trigger_ਕਦਮ));
  ਜੇ( p_Trigger_ਕਦਮ==0 ) {
    sqlite3SelectDelete(db, pSelect);
    ਵਾਪਸ 0;
  }
  p_Trigger_ਕਦਮ->op = TK_SELECT;
  p_Trigger_ਕਦਮ->pSelect = pSelect;
  p_Trigger_ਕਦਮ->orconf = OE_Default;
  p_Trigger_ਕਦਮ->z_ਵਿਸਤਾਰ = trigger_ਵਿਸਤਾਰ_Dup(db, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
  ਵਾਪਸ p_Trigger_ਕਦਮ;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ਸਥਿਰ Trigger_ਕਦਮ *trigger_ਕਦਮ_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ,         /* Start of SQL text */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_ਕਦਮ *p_Trigger_ਕਦਮ = 0;

  ਜੇ( pParse->nErr==0 ){
    ਜੇ( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }ਨਹੀਂਤਾਂ{
      p_Trigger_ਕਦਮ = sqlite3DbMallocZero(db, ਆਕਾਰ(Trigger_ਕਦਮ));
      ਜੇ( p_Trigger_ਕਦਮ ){
        p_Trigger_ਕਦਮ->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_ਕਦਮ->op = op;
        p_Trigger_ਕਦਮ->z_ਵਿਸਤਾਰ = trigger_ਵਿਸਤਾਰ_Dup(db, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
        ਜੇ( p_Trigger_ਕਦਮ->pSrc && ਵਿੱਚ_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_ਕਦਮ->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  ਵਾਪਸ p_Trigger_ਕਦਮ;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_ਕਦਮ *sqlite3_Trigger_ਪਾਓ_ਕਦਮ(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *p_ਕਾਲਮ,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ, /* Start of SQL text */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ਕਦਮ *p_Trigger_ਕਦਮ;

  ਪ੍ਰਤਿਜ੍ਞਾ(pSelect != 0 || db->mallocFailed);

  p_Trigger_ਕਦਮ = trigger_ਕਦਮ_Allocate(pParse, TK_ਪਾਓ, pTabList, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
  ਜੇ( p_Trigger_ਕਦਮ ){
    ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
      p_Trigger_ਕਦਮ->pSelect = pSelect;
      pSelect = 0;
    }ਨਹੀਂਤਾਂ{
      p_Trigger_ਕਦਮ->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_ਕਦਮ->pIdList = p_ਕਾਲਮ;
    p_Trigger_ਕਦਮ->pUpsert = pUpsert;
    p_Trigger_ਕਦਮ->orconf = orconf;
    ਜੇ( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ);
    }
  }ਨਹੀਂਤਾਂ{
    testcase( p_ਕਾਲਮ );
    sqlite3IdListDelete(db, p_ਕਾਲਮ);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  ਵਾਪਸ p_Trigger_ਕਦਮ;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ਕਦਮ *sqlite3_Trigger_Update_ਕਦਮ(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  ਵਿਅੰਜਕ_List *pEList,    /* The SET clause: list of column and new values */
  ਵਿਅੰਜਕ *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ,  /* Start of SQL text */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ਕਦਮ *p_Trigger_ਕਦਮ;

  p_Trigger_ਕਦਮ = trigger_ਕਦਮ_Allocate(pParse, TK_UPDATE, pTabList, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
  ਜੇ( p_Trigger_ਕਦਮ ){
    SrcList *pFromDup = 0;
    ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
      p_Trigger_ਕਦਮ->p_ਵਿਅੰਜਕ_List = pEList;
      p_Trigger_ਕਦਮ->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }ਨਹੀਂਤਾਂ{
      p_Trigger_ਕਦਮ->p_ਵਿਅੰਜਕ_List = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_ਕਦਮ->pWhere = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_ਕਦਮ->orconf = orconf;

    ਜੇ( pFromDup && !ਵਿੱਚ_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_ਪਦ(pParse, 0, 0, 0, &as, pSub ,0);
    }
    ਜੇ( pFromDup && p_Trigger_ਕਦਮ->pSrc ){
      p_Trigger_ਕਦਮ->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_ਕਦਮ->pSrc, pFromDup
      );
    }ਨਹੀਂਤਾਂ{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pEList);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  ਵਾਪਸ p_Trigger_ਕਦਮ;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ਕਦਮ *sqlite3_Trigger_Delete_ਕਦਮ(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  ਵਿਅੰਜਕ *pWhere,           /* The WHERE clause */
  ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ,     /* Start of SQL text */
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ਕਦਮ *p_Trigger_ਕਦਮ;

  p_Trigger_ਕਦਮ = trigger_ਕਦਮ_Allocate(pParse, TK_DELETE, pTabList, z_ਸ਼ੁਰੂ, z_ਸਿਰਾ);
  ਜੇ( p_Trigger_ਕਦਮ ){
    ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
      p_Trigger_ਕਦਮ->pWhere = pWhere;
      pWhere = 0;
    }ਨਹੀਂਤਾਂ{
      p_Trigger_ਕਦਮ->pWhere = sqlite3_ਵਿਅੰਜਕ_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_ਕਦਮ->orconf = OE_Default;
  }
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhere);
  ਵਾਪਸ p_Trigger_ਕਦਮ;
}

/* 
** Recursively delete a Trigger structure
*/
ਖਾਲੀ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  ਜੇ( pTrigger==0 || pTrigger->bReturning ) ਵਾਪਸ;
  sqlite3_Delete_Trigger_ਕਦਮ(db, pTrigger->ਕਦਮ_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->ਸਾਰਣੀ_2);
  sqlite3_ਵਿਅੰਜਕ_Delete(db, pTrigger->pWhen);
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
ਖਾਲੀ sqlite3DropTrigger(Parse *pParse, SrcList *pName, ਪੂਰਨ noErr){
  Trigger *pTrigger = 0;
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zDb;
  ਅਚਲ ਅੱਖਰ *zName;
  sqlite3 *db = pParse->db;

  ਜੇ( db->mallocFailed ) ਜਾਓ drop_trigger_cleanup;
  ਜੇ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ਜਾਓ drop_trigger_cleanup;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pName->nSrc==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  ਪ੍ਰਤਿਜ੍ਞਾ( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  ਲਈ(i=OMIT_TEMPDB; i<db->nDb; i++){
    ਪੂਰਨ j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    ਜੇ( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3_Hash_ਲੱਭੋ(&(db->aDb[j].pSchema->trigHash), zName);
    ਜੇ( pTrigger ) ਰੋਕੋ;
  }
  ਜੇ( !pTrigger ){
    ਜੇ( !noErr ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such trigger: %S", pName->a);
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਕੋਡ_Verify_Named_Schema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    ਜਾਓ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ਸਥਿਰ ਸਾਰਣੀ *ਸਾਰਣੀ_ਦਾ_Trigger(Trigger *pTrigger){
  ਵਾਪਸ sqlite3_Hash_ਲੱਭੋ(&pTrigger->pTabSchema->tblHash, pTrigger->ਸਾਰਣੀ_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
ਖਾਲੀ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  ਸਾਰਣੀ   *p_ਸਾਰਣੀ;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ iDb;

  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTrigger->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<db->nDb );
  p_ਸਾਰਣੀ = ਸਾਰਣੀ_ਦਾ_Trigger(pTrigger);
  ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਸਾਰਣੀ && p_ਸਾਰਣੀ->pSchema==pTrigger->pSchema) || iDb==1 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( p_ਸਾਰਣੀ ){
    ਪੂਰਨ ਕੋਡ = SQLITE_DROP_TRIGGER;
    ਅਚਲ ਅੱਖਰ *zDb = db->aDb[iDb].zDbSName;
    ਅਚਲ ਅੱਖਰ *zTab = SCHEMA_ਸਾਰਣੀ(iDb);
    ਜੇ( iDb==1 ) ਕੋਡ = SQLITE_DROP_ਆਰਜ਼ੀ_TRIGGER;
    ਜੇ( sqlite3AuthCheck(pParse, ਕੋਡ, pTrigger->zName, p_ਸਾਰਣੀ->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      ਵਾਪਸ;
    }
  }
#ਜੇ_ਅੰਤ

  /* Generate code to destroy the database record of the trigger.
  */
  ਜੇ( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_ਸਾਰਣੀ " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
ਖਾਲੀ sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, ਪੂਰਨ iDb, ਅਚਲ ਅੱਖਰ *zName){
  Trigger *pTrigger;
  Hash *pHash;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_ਪਾਓ(pHash, zName, 0);
  ਜੇ( ALWAYS(pTrigger) ){
    ਜੇ( pTrigger->pSchema==pTrigger->pTabSchema ){
      ਸਾਰਣੀ *pTab = ਸਾਰਣੀ_ਦਾ_Trigger(pTrigger);
      ਜੇ( pTab ){
        Trigger **pp;
        ਲਈ(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_ਅਗਲਾ)){
          ਜੇ( *pp==pTrigger ){
            *pp = (*pp)->p_ਅਗਲਾ;
            ਰੋਕੋ;
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
ਸਥਿਰ ਪੂਰਨ check_ਕਾਲਮ_Overlap(IdList *pIdList, ਵਿਅੰਜਕ_List *pEList){
  ਪੂਰਨ e;
  ਜੇ( pIdList==0 || NEVER(pEList==0) ) ਵਾਪਸ 1;
  ਲਈ(e=0; e<pEList->n_ਵਿਅੰਜਕ; e++){
    ਜੇ( sqlite3_Id_List_ਸੂਚਕਾਂਕ(pIdList, pEList->a[e].zEName)>=0 ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ਸਥਿਰ ਪੂਰਨ ਆਰਜ਼ੀ_Triggers_Exist(sqlite3 *db){
  ਜੇ( NEVER(db->aDb[1].pSchema==0) ) ਵਾਪਸ 0;
  ਜੇ( sqlite_Hash_ਪਹਿਲਾ(&db->aDb[1].pSchema->trigHash)==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ਸਥਿਰ SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  ਸਾਰਣੀ *pTab,            /* The table the contains the triggers */
  ਪੂਰਨ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ਵਿਅੰਜਕ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ਪੂਰਨ *p_ਨਕਾਬ              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ਪੂਰਨ ਨਕਾਬ = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  ਪ੍ਰਤਿਜ੍ਞਾ( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_ਅਗਲਾ==0) );
  ਜੇ( pList!=0 ){
    p = pList;
    ਜੇ( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      ਜੇ( pList==pTab->pTrigger ){
        pList = 0;
        ਜਾਓ exit_triggers_exist;
      }
      ਜਦੋਂ( ALWAYS(p->p_ਅਗਲਾ) && p->p_ਅਗਲਾ!=pTab->pTrigger ) p = p->p_ਅਗਲਾ;
      p->p_ਅਗਲਾ = 0;
      p = pList;
    }
    ਕਰੋ{
      ਜੇ( p->op==op && check_ਕਾਲਮ_Overlap(p->pColumns, pChanges) ){
        ਨਕਾਬ |= p->tr_tm;
      }ਨਹੀਂਤਾਂ ਜੇ( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3IsToplevel(pParse) );
        p->op = op;
        ਜੇ( IsVirtual(pTab) ){
          ਜੇ( op!=TK_ਪਾਓ ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }ਨਹੀਂਤਾਂ{
          p->tr_tm = TRIGGER_AFTER;
        }
        ਨਕਾਬ |= p->tr_tm;
      }ਨਹੀਂਤਾਂ ਜੇ( p->bReturning && p->op==TK_ਪਾਓ && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        ਨਕਾਬ |= p->tr_tm;
      }
      p = p->p_ਅਗਲਾ;
    }ਜਦੋਂ( p );
  }
exit_triggers_exist:
  ਜੇ( p_ਨਕਾਬ ){
    *p_ਨਕਾਬ = ਨਕਾਬ;
  }
  ਵਾਪਸ (ਨਕਾਬ ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  ਸਾਰਣੀ *pTab,            /* The table the contains the triggers */
  ਪੂਰਨ op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ਵਿਅੰਜਕ_List *pChanges,     /* Columns that change in an UPDATE statement */
  ਪੂਰਨ *p_ਨਕਾਬ              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਜੇ( (pTab->pTrigger==0 && !ਆਰਜ਼ੀ_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    ਜੇ( p_ਨਕਾਬ ) *p_ਨਕਾਬ = 0;
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ triggersReallyExist(pParse,pTab,op,pChanges,p_ਨਕਾਬ);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ਸਥਿਰ ਪੂਰਨ is_Asterisk_ਪਦ(
  Parse *pParse,      /* Parsing context */
  ਵਿਅੰਜਕ *p_ਪਦ         /* A term in the RETURNING clause */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
  ਜੇ( p_ਪਦ->op==TK_ASTERISK ) ਵਾਪਸ 1;
  ਜੇ( p_ਪਦ->op!=TK_DOT ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->p_ਸੱਜਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->p_ਖੱਬਾ!=0 );
  ਜੇ( p_ਪਦ->p_ਸੱਜਾ->op!=TK_ASTERISK ) ਵਾਪਸ 0;
  sqlite3_Error_ਸੁਨੇਹਾ(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  ਵਾਪਸ 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ਸਥਿਰ ਵਿਅੰਜਕ_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,      /* The arguments to RETURNING */
  ਸਾਰਣੀ *pTab           /* The table being updated */
){
  ਵਿਅੰਜਕ_List *pNew = 0;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ i;

  ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p_Old_ਵਿਅੰਜਕ = pList->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( NEVER(p_Old_ਵਿਅੰਜਕ==0) ) ਜਾਰੀ;
    ਜੇ( is_Asterisk_ਪਦ(pParse, p_Old_ਵਿਅੰਜਕ) ){
      ਪੂਰਨ jj;
      ਲਈ(jj=0; jj<pTab->n_ਖਾਨਾ; jj++){
        ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ;
        ਜੇ( Is_Hidden_ਕਾਲਮ(pTab->a_ਖਾਨਾ+jj) ) ਜਾਰੀ;
        p_New_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, pTab->a_ਖਾਨਾ[jj].zCnName);
        pNew = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pNew, p_New_ਵਿਅੰਜਕ);
        ਜੇ( !db->mallocFailed ){
          ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &pNew->a[pNew->n_ਵਿਅੰਜਕ-1];
          p_ਵਸਤੂ->zEName = sqlite3DbStrDup(db, pTab->a_ਖਾਨਾ[jj].zCnName);
          p_ਵਸਤੂ->fg.eEName = ENAME_NAME;
        }
      }
    }ਨਹੀਂਤਾਂ{
      ਵਿਅੰਜਕ *p_New_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_Old_ਵਿਅੰਜਕ, 0);
      pNew = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pNew, p_New_ਵਿਅੰਜਕ);
      ਜੇ( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &pNew->a[pNew->n_ਵਿਅੰਜਕ-1];
        p_ਵਸਤੂ->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        p_ਵਸਤੂ->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  ਵਾਪਸ pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3ReturningSubqueryVarSelect(Walker *Not_ਵਰਤਿਆ, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ)
   && (p_ਵਿਅੰਜਕ->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_VarSelect) );
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_VarSelect);
  }
  ਵਾਪਸ WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  ਪੂਰਨ i;
  SrcList *pSrc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect!=0 );
  pSrc = pSelect->pSrc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc!=0 );
  ਲਈ(i=0; i<pSrc->nSrc; i++){
    ਜੇ( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_ਕੋਡ = 1;
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਖਾਲੀ sqlite3ProcessReturningSubqueries(
  ਵਿਅੰਜਕ_List *pEList,
  ਸਾਰਣੀ *pTab
){
  Walker w;
  memset(&w, 0, ਆਕਾਰ(w));
  w.x_ਵਿਅੰਜਕ_Callback = sqlite3_ਵਿਅੰਜਕ_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_ਵਿਅੰਜਕ_List(&w, pEList);
  ਜੇ( w.e_ਕੋਡ ){
    w.x_ਵਿਅੰਜਕ_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_ਵਿਅੰਜਕ_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  ਸਾਰਣੀ *pTab,         /* The table to code triggers from */
  ਪੂਰਨ reg_ਵਿੱਚ            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  ਸੰਘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਜੇ( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਪ੍ਰਤਿਜ੍ਞਾ( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  ਜੇ( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    ਵਾਪਸ;
  }
  memset(&sSelect, 0, ਆਕਾਰ(sSelect));
  memset(&uSrc, 0, ਆਕਾਰ(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  ਜੇ( pParse->nErr==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
    sqlite3_Generate_ਕਾਲਮ_ਨਾਂ(pParse, &sSelect);
  }
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  ਜੇ( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, ਆਕਾਰ(sNC));
    ਜੇ( pReturning->n_Ret_ਖਾਨਾ==0 ){
      pReturning->n_Ret_ਖਾਨਾ = pNew->n_ਵਿਅੰਜਕ;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_ਅਧਾਰ_Reg = reg_ਵਿੱਚ;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      ਪੂਰਨ i;
      ਪੂਰਨ n_ਖਾਨਾ = pNew->n_ਵਿਅੰਜਕ;
      ਪੂਰਨ reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += n_ਖਾਨਾ+2;
      pReturning->iRetReg = reg;
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        ਵਿਅੰਜਕ *p_ਖਾਨਾ = pNew->a[i].p_ਵਿਅੰਜਕ;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(pParse, p_ਖਾਨਾ, reg+i);
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਖਾਨਾ)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg, i, reg+i);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Trigger_ਪ੍ਰੋਗਰਾਮ(
  Parse *pParse,            /* The parser context */
  Trigger_ਕਦਮ *p_ਕਦਮ_List,   /* List of statements inside the trigger body */
  ਪੂਰਨ orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_ਕਦਮ *p_ਕਦਮ;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pTriggerTab && pParse->pToplevel );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕਦਮ_List );
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਲਈ(p_ਕਦਮ=p_ਕਦਮ_List; p_ਕਦਮ; p_ਕਦਮ=p_ਕਦਮ->p_ਅਗਲਾ){
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
    pParse->eOrconf = (orconf==OE_Default)?p_ਕਦਮ->orconf:(u8)orconf;
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->ok_Const_ਗੁਣਕ==0 );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
    ਜੇ( p_ਕਦਮ->z_ਵਿਸਤਾਰ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_ਕਦਮ->z_ਵਿਸਤਾਰ),
                        P4_DYNAMIC);
    }
#ਜੇ_ਅੰਤ

    ਚੋਣ( p_ਕਦਮ->op ){
      ਹਾਲਤ TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_ਕਦਮ->pSrc, 0),
          sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p_ਕਦਮ->p_ਵਿਅੰਜਕ_List, 0), 
          sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਕਦਮ->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Reset_ਗਿਣਤੀ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_ਪਾਓ: {
        sqlite3_ਪਾਓ(pParse, 
          sqlite3SrcListDup(db, p_ਕਦਮ->pSrc, 0),
          sqlite3SelectDup(db, p_ਕਦਮ->pSelect, 0), 
          sqlite3IdListDup(db, p_ਕਦਮ->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_ਕਦਮ->pUpsert)
        );
        sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Reset_ਗਿਣਤੀ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_ਕਦਮ->pSrc, 0),
          sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਕਦਮ->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Reset_ਗਿਣਤੀ);
        ਰੋਕੋ;
      }
      ਮੂਲ: ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕਦਮ->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_ਕਦਮ->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        ਰੋਕੋ;
      }
    } 
  }

  ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *on_Error_ਪਾਠ(ਪੂਰਨ onError){
  ਚੋਣ( onError ){
    ਹਾਲਤ OE_Abort:    ਵਾਪਸ "abort";
    ਹਾਲਤ OE_Rollback: ਵਾਪਸ "rollback";
    ਹਾਲਤ OE_Fail:     ਵਾਪਸ "fail";
    ਹਾਲਤ OE_Replace:  ਵਾਪਸ "replace";
    ਹਾਲਤ OE_Ignore:   ਵਾਪਸ "ignore";
    ਹਾਲਤ OE_Default:  ਵਾਪਸ "default";
  }
  ਵਾਪਸ "n/a";
}
#ਜੇ_ਅੰਤ

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ਸਥਿਰ ਖਾਲੀ transferParseError(Parse *p_ਮੰਜ਼ਿਲ, Parse *pFrom){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->z_Err_ਸੁਨੇਹਾ==0 || pFrom->nErr );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ->z_Err_ਸੁਨੇਹਾ==0 || p_ਮੰਜ਼ਿਲ->nErr );
  ਜੇ( p_ਮੰਜ਼ਿਲ->nErr==0 ){
    p_ਮੰਜ਼ਿਲ->z_Err_ਸੁਨੇਹਾ = pFrom->z_Err_ਸੁਨੇਹਾ;
    p_ਮੰਜ਼ਿਲ->nErr = pFrom->nErr;
    p_ਮੰਜ਼ਿਲ->rc = pFrom->rc;
  }ਨਹੀਂਤਾਂ{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_ਸੁਨੇਹਾ);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ਸਥਿਰ TriggerPrg *ਕੋਡ_ਪੰਕਤੀ_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  ਸਾਰਣੀ *pTab,         /* The table pTrigger is attached to */
  ਪੂਰਨ orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_ਸਿਖਰ;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  ਵਿਅੰਜਕ *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_ਪ੍ਰੋਗਰਾਮ *p_ਪ੍ਰੋਗਰਾਮ = 0;   /* Sub-vdbe for trigger program */
  ਪੂਰਨ i_ਸਿਰਾ_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  ਪੂਰਨ n_ਡੂੰਘਾਈ;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_ਸਿਖਰ = pParse;
  ਲਈ(n_ਡੂੰਘਾਈ=0; p_ਸਿਖਰ->pOuterParse; p_ਸਿਖਰ = p_ਸਿਖਰ->pOuterParse, n_ਡੂੰਘਾਈ++){}
  ਜੇ( n_ਡੂੰਘਾਈ>=db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_TRIGGER_ਡੂੰਘਾਈ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "triggers nested too deep");
    ਵਾਪਸ 0;
  }

  p_ਸਿਖਰ = sqlite3ParseToplevel(pParse);
  ਪ੍ਰਤਿਜ੍ਞਾ( pTrigger->zName==0 || pTab==ਸਾਰਣੀ_ਦਾ_Trigger(pTrigger) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਿਖਰ->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, ਆਕਾਰ(TriggerPrg));
  ਜੇ( !pPrg ) ਵਾਪਸ 0;
  pPrg->p_ਅਗਲਾ = p_ਸਿਖਰ->pTriggerPrg;
  p_ਸਿਖਰ->pTriggerPrg = pPrg;
  pPrg->p_ਪ੍ਰੋਗਰਾਮ = p_ਪ੍ਰੋਗਰਾਮ = sqlite3DbMallocZero(db, ਆਕਾਰ(Sub_ਪ੍ਰੋਗਰਾਮ));
  ਜੇ( !p_ਪ੍ਰੋਗਰਾਮ ) ਵਾਪਸ 0;
  sqlite3_Vdbe_Link_Sub_ਪ੍ਰੋਗਰਾਮ(p_ਸਿਖਰ->pVdbe, p_ਪ੍ਰੋਗਰਾਮ);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  /* Allocate and populate a new Parse context to use for coding the 
  ** trigger sub-program.  */
  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = p_ਸਿਖਰ;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  ਜੇ( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, on_Error_ਪਾਠ(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ਪਾਓ ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
    ਜੇ( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#ਜੇ_ਅੰਤ

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    ਜੇ( pTrigger->pWhen ){
      pWhen = sqlite3_ਵਿਅੰਜਕ_Dup(db, pTrigger->pWhen, 0);
      ਜੇ( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, pWhen) 
      ){
        i_ਸਿਰਾ_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_ਵਿਅੰਜਕ_If_False(&sSubParse, pWhen, i_ਸਿਰਾ_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_ਵਿਅੰਜਕ_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    ਕੋਡ_Trigger_ਪ੍ਰੋਗਰਾਮ(&sSubParse, pTrigger->ਕਦਮ_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    ਜੇ( i_ਸਿਰਾ_Trigger ){
      sqlite3VdbeResolveLabel(v, i_ਸਿਰਾ_Trigger);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, on_Error_ਪਾਠ(orconf)));
    transferParseError(pParse, &sSubParse);

    ਜੇ( pParse->nErr==0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
      p_ਪ੍ਰੋਗਰਾਮ->aOp = sqlite3VdbeTakeOpArray(v, &p_ਪ੍ਰੋਗਰਾਮ->nOp, &p_ਸਿਖਰ->nMaxArg);
    }
    p_ਪ੍ਰੋਗਰਾਮ->nMem = sSubParse.nMem;
    p_ਪ੍ਰੋਗਰਾਮ->nCsr = sSubParse.nTab;
    p_ਪ੍ਰੋਗਰਾਮ->token = (ਖਾਲੀ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }ਨਹੀਂਤਾਂ{
    transferParseError(pParse, &sSubParse);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  ਵਾਪਸ pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ਸਥਿਰ TriggerPrg *get_ਪੰਕਤੀ_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  ਸਾਰਣੀ *pTab,         /* The table trigger pTrigger is attached to */
  ਪੂਰਨ orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_ਜੜ੍ਹ = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  ਪ੍ਰਤਿਜ੍ਞਾ( pTrigger->zName==0 || pTab==ਸਾਰਣੀ_ਦਾ_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  ਲਈ(pPrg=p_ਜੜ੍ਹ->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_ਅਗਲਾ
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  ਜੇ( !pPrg ){
    pPrg = ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_ਵਿੱਥ = -1;
  }

  ਵਾਪਸ pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
ਖਾਲੀ sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger_ਸਿੱਧਾ(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  ਸਾਰਣੀ *pTab,         /* The table to code triggers from */
  ਪੂਰਨ reg,             /* Reg array containing OLD.* and NEW.* values */
  ਪੂਰਨ orconf,          /* ON CONFLICT policy */
  ਪੂਰਨ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = get_ਪੰਕਤੀ_Trigger(pParse, p, pTab, orconf);
  ਪ੍ਰਤਿਜ੍ਞਾ( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  ਜੇ( pPrg ){
    ਪੂਰਨ bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_ਪ੍ਰੋਗਰਾਮ, reg, ignoreJump, ++pParse->nMem,
                      (ਅਚਲ ਅੱਖਰ *)pPrg->p_ਪ੍ਰੋਗਰਾਮ, P4_SUBPROGRAM);
    VdbeComment(
        (v, "Call: %s.%s", (p->zName?p->zName:"fkey"), on_Error_ਪਾਠ(orconf)));

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
ਖਾਲੀ sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  ਪੂਰਨ op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  ਵਿਅੰਜਕ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ਪੂਰਨ tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  ਸਾਰਣੀ *pTab,         /* The table to code triggers from */
  ਪੂਰਨ reg,             /* The first in an array of registers (see above) */
  ਪੂਰਨ orconf,          /* ON CONFLICT policy */
  ਪੂਰਨ ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_UPDATE || op==TK_ਪਾਓ || op==TK_DELETE );
  ਪ੍ਰਤਿਜ੍ਞਾ( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  ਪ੍ਰਤਿਜ੍ਞਾ( (op==TK_UPDATE)==(pChanges!=0) );

  ਲਈ(p=pTrigger; p; p=p->p_ਅਗਲਾ){

    /* Sanity checking:  The schema for the trigger and for the table are
    ** always defined.  The trigger must be in the same schema as the table
    ** or else it must be a TEMP trigger. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pSchema!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pTabSchema!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pSchema==p->pTabSchema 
         || p->pSchema==pParse->db->aDb[1].pSchema );

    /* Determine whether we should code this trigger.  One of two choices:
    **   1. The trigger is an exact match to the current DML statement
    **   2. This is a RETURNING trigger for INSERT but we are currently
    **      doing the UPDATE part of an UPSERT.
    */
    ਜੇ( (p->op==op || (p->bReturning && p->op==TK_ਪਾਓ && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && check_ਕਾਲਮ_Overlap(p->pColumns, pChanges)
    ){
      ਜੇ( !p->bReturning ){
        sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger_ਸਿੱਧਾ(pParse, p, pTab, reg, orconf, ignoreJump);
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3IsToplevel(pParse) ){
        ਕੋਡ_Returning_Trigger(pParse, p, pTab, reg);
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
  ਵਿਅੰਜਕ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  ਪੂਰਨ isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  ਪੂਰਨ tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  ਸਾਰਣੀ *pTab,         /* The table to code triggers from */
  ਪੂਰਨ orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ਅਚਲ ਪੂਰਨ op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 ਨਕਾਬ = 0;
  Trigger *p;

  ਪ੍ਰਤਿਜ੍ਞਾ( isNew==1 || isNew==0 );
  ਜੇ( IsView(pTab) ){
    ਵਾਪਸ 0xffffffff;
  }
  ਲਈ(p=pTrigger; p; p=p->p_ਅਗਲਾ){
    ਜੇ( p->op==op
     && (tr_tm&p->tr_tm)
     && check_ਕਾਲਮ_Overlap(p->pColumns,pChanges)
    ){
      ਜੇ( p->bReturning ){
        ਨਕਾਬ = 0xffffffff;
      }ਨਹੀਂਤਾਂ{
        TriggerPrg *pPrg;
        pPrg = get_ਪੰਕਤੀ_Trigger(pParse, p, pTab, orconf);
        ਜੇ( pPrg ){
          ਨਕਾਬ |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  ਵਾਪਸ ਨਕਾਬ;
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_TRIGGER) */
