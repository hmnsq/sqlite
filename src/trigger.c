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
#شامل "sqliteInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
خالی sqlite3_Delete_Trigger_قدم(sqlite3 *db, Trigger_قدم *p_Trigger_قدم){
  جب( p_Trigger_قدم ){
    Trigger_قدم * pTmp = p_Trigger_قدم;
    p_Trigger_قدم = p_Trigger_قدم->p_اگلا;

    sqlite3_اظہاریہ_Delete(db, pTmp->pWhere);
    sqlite3_اظہاریہ_List_Delete(db, pTmp->p_اظہاریہ_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_وسعت);

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
Trigger *sqlite3TriggerList(Parse *pParse, جدول *pTab){
  Schema *pTmpSchema;       /* Schema of the pTab table */
  Trigger *pList;           /* List of triggers to return */
  HashElem *p;              /* Loop variable for TEMP triggers */

  أكد( pParse->disableTriggers==0 );
  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = sqlite_Hash_پہلا(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  جب( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_ڈیٹا(p);
    اگر( pTrig->pTabSchema==pTab->pSchema
     && pTrig->جدول_2
     && 0==sqlite3StrICmp(pTrig->جدول_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_اگلا = pList;
      pList = pTrig;
    }ورنہ اگر( pTrig->op==TK_RETURNING ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
      أكد( pParse->db->pVtabCtx==0 );
#اگر_ختم
      أكد( pParse->bReturning );
      أكد( !pParse->isCreate );
      أكد( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->جدول_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_اگلا = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_اگلا(p);    
  }
#اگر 0
  اگر( pList ){
    Trigger *pX;
    لکھو("Triggers for %s:", pTab->zName);
    برائے(pX=pList; pX; pX=pX->p_اگلا){
      لکھو(" %s", pX->zName);
    }
    لکھو("\n");
    fflush(stdout);
  }
#اگر_ختم
  واپس pList;  
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
خالی sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  صحیح tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  صحیح op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *p_جدول_Name,/* The name of the table/view the trigger applies to */
  اظہاریہ *pWhen,        /* WHEN clause */
  صحیح is_عارضی,         /* True if the TEMPORARY keyword is present */
  صحیح noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  جدول *pTab;            /* Table that the trigger fires off of */
  حرف *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  صحیح iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  أكد( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  أكد( pName2!=0 );
  أكد( op==TK_داخل_کرو || op==TK_UPDATE || op==TK_DELETE );
  أكد( op>0 && op<0xff );
  اگر( is_عارضی ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    اگر( pName2->n>0 ){
      sqlite3_Error_پیغام(pParse, "temporary trigger may not have qualified name");
      جاؤ trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }ورنہ{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    اگر( iDb<0 ){
      جاؤ trigger_cleanup;
    }
  }
  اگر( !p_جدول_Name || db->mallocFailed ){
    جاؤ trigger_cleanup;
  }

  /* A long-standing parser bug is that this syntax was allowed:
  **
  **    CREATE TRIGGER attached.demo AFTER INSERT ON attached.tab ....
  **                                                 ^^^^^^^^
  **
  ** To maintain backwards compatibility, ignore the database
  ** name on pTableName if we are reparsing out of the schema table
  */
  اگر( db->init.busy && iDb!=1 ){
    أكد( p_جدول_Name->a[0].fg.fixedSchema==0 );
    أكد( p_جدول_Name->a[0].fg.isSubquery==0 );
    sqlite3DbFree(db, p_جدول_Name->a[0].u4.zDatabase);
    p_جدول_Name->a[0].u4.zDatabase = 0;
  }

  /* If the trigger name was unqualified, and the table is a temp table,
  ** then set iDb to 1 to create the trigger in the temporary database.
  ** If sqlite3SrcListLookup() returns 0, indicating the table does not
  ** exist, the error is caught by the block below.
  */
  pTab = sqlite3SrcListLookup(pParse, p_جدول_Name);
  اگر( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  اگر( db->mallocFailed ) جاؤ trigger_cleanup;
  أكد( p_جدول_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  اگر( sqlite3FixSrcList(&sFix, p_جدول_Name) ){
    جاؤ trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_جدول_Name);
  اگر( !pTab ){
    /* The table does not exist. */
    جاؤ trigger_orphan_error;
  }
  اگر( IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "cannot create triggers on virtual tables");
    جاؤ trigger_orphan_error;
  }
  اگر( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_پیغام(pParse, "cannot create triggers on shadow tables");
    جاؤ trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  اگر( zName==0 ){
    أكد( db->mallocFailed );
    جاؤ trigger_cleanup;
  }
  اگر( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    جاؤ trigger_cleanup;
  }
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  اگر( !اندر_RENAME_OBJECT ){
    اگر( sqlite3_Hash_ڈھونڈو(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      اگر( !noErr ){
        sqlite3_Error_پیغام(pParse, "trigger %T already exists", pName);
      }ورنہ اگر( db->init.busy ){
        sqlite3_Error_پیغام(pParse,""); /* Err msg generated by corruptSchema() */
      }ورنہ{
        sqlite3_رمز_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      جاؤ trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#اگر !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  اگر( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_پیغام(pParse, "cannot create trigger on system table");
    جاؤ trigger_cleanup;
  }
#اگر_ختم

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  اگر( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_پیغام(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_جدول_Name->a);
    جاؤ trigger_orphan_error;
  }
  اگر( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_پیغام(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_جدول_Name->a);
    جاؤ trigger_orphan_error;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  اگر( !اندر_RENAME_OBJECT ){
    صحیح iTabDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
    صحیح رمز = SQLITE_CREATE_TRIGGER;
    ثابت حرف *zDb = db->aDb[iTabDb].zDbSName;
    ثابت حرف *zDbTrig = is_عارضی ? db->aDb[1].zDbSName : zDb;
    اگر( iTabDb==1 || is_عارضی ) رمز = SQLITE_CREATE_عارضی_TRIGGER;
    اگر( sqlite3AuthCheck(pParse, رمز, zName, pTab->zName, zDbTrig) ){
      جاؤ trigger_cleanup;
    }
    اگر( sqlite3AuthCheck(pParse, SQLITE_داخل_کرو, SCHEMA_جدول(iTabDb),0,zDb)){
      جاؤ trigger_cleanup;
    }
  }
#اگر_ختم

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  اگر (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, حجم(Trigger));
  اگر( pTrigger==0 ) جاؤ trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->جدول_2 = sqlite3DbStrDup(db, p_جدول_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  اگر( اندر_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->جدول_2, p_جدول_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }ورنہ{
    pTrigger->pWhen = sqlite3_اظہاریہ_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  أكد( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_جدول_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_اظہاریہ_Delete(db, pWhen);
  اگر( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }ورنہ{
    أكد( pParse->pNewTrigger==pTrigger );
  }
  واپس;

trigger_orphan_error:
  اگر( db->init.iDb==1 ){
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
  جاؤ trigger_cleanup;
}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
خالی sqlite3_اختتام_Trigger(
  Parse *pParse,          /* Parser context */
  Trigger_قدم *p_قدم_List, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){
  Trigger *pTrig = pParse->pNewTrigger;   /* Trigger being finished */
  حرف *zName;                            /* Name of trigger */
  sqlite3 *db = pParse->db;               /* The database */
  DbFixer sFix;                           /* Fixer object */
  صحیح iDb;                                /* Database containing the trigger */
  Token nameToken;                        /* Trigger name for error reporting */
  i64 n;                                  /* Number of steps */

  pParse->pNewTrigger = 0;
  اگر( NEVER(pParse->nErr) || !pTrig ) جاؤ triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTrig->pSchema);
  أكد( iDb>=00 && iDb<db->nDb );
  pTrig->قدم_list = p_قدم_List;
  برائے(n=0; p_قدم_List; n++){
    p_قدم_List->pTrig = pTrig;
    p_قدم_List = p_قدم_List->p_اگلا;
  }
  اگر( n>pParse->db->a_حد_بندی[SQLITE_حد_بندی_TRIGGER_اقدام] ){
    sqlite3_Error_پیغام(pParse, "trigger \"%w\" contains too many steps", zName);
    جاؤ triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  اگر( sqlite3_Fix_Trigger_قدم(&sFix, pTrig->قدم_list) 
   || sqlite3_Fix_اظہاریہ(&sFix, pTrig->pWhen) 
  ){
    جاؤ triggerfinish_cleanup;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_ALTERTABLE
  اگر( اندر_RENAME_OBJECT ){
    أكد( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }ورنہ
#اگر_ختم

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  اگر( !db->init.busy ){
    Vdbe *v;
    حرف *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    اگر( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_قدم *p_قدم;
      برائے(p_قدم=pTrig->قدم_list; p_قدم; p_قدم=p_قدم->p_اگلا){
        اگر( p_قدم->pSrc!=0
         && sqlite3_Shadow_جدول_Name(db, p_قدم->pSrc->a[0].zName)
        ){
          sqlite3_Error_پیغام(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_قدم->pSrc->a[0].zName);
          جاؤ triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    اگر( v==0 ) جاؤ triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (حرف*)pAll->z, pAll->n);
    testcase( z==0 );
    sqlite3NestedParse(pParse,
       "INSERT INTO %Q." LEGACY_SCHEMA_جدول
       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
       db->aDb[iDb].zDbSName, zName,
       pTrig->جدول_2, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_جوڑو_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  اگر( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    أكد( pLink!=0 );
    pTrig = sqlite3_Hash_داخل_کرو(pHash, zName, pTrig);
    اگر( pTrig ){
      sqlite3OomFault(db);
    }ورنہ اگر( pLink->pSchema==pLink->pTabSchema ){
      جدول *pTab;
      pTab = sqlite3_Hash_ڈھونڈو(&pLink->pTabSchema->tblHash, pLink->جدول_2);
      أكد( pTab!=0 );
      pLink->p_اگلا = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  أكد( اندر_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_قدم(db, p_قدم_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ساکن حرف *trigger_وسعت_Dup(sqlite3 *db, ثابت حرف *z_آغاز, ثابت حرف *z_انتہا){
  حرف *z = sqlite3_Db_وسعت_Dup(db, z_آغاز, z_انتہا);
  صحیح i;
  اگر( z ) برائے(i=0; z[i]; i++) اگر( sqlite3Isspace(z[i]) ) z[i] = ' ';
  واپس z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_قدم *sqlite3_Trigger_Select_قدم(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ثابت حرف *z_آغاز,         /* Start of SQL text */
  ثابت حرف *z_انتہا            /* End of SQL text */
){
  Trigger_قدم *p_Trigger_قدم = sqlite3DbMallocZero(db, حجم(Trigger_قدم));
  اگر( p_Trigger_قدم==0 ) {
    sqlite3SelectDelete(db, pSelect);
    واپس 0;
  }
  p_Trigger_قدم->op = TK_SELECT;
  p_Trigger_قدم->pSelect = pSelect;
  p_Trigger_قدم->orconf = OE_Default;
  p_Trigger_قدم->z_وسعت = trigger_وسعت_Dup(db, z_آغاز, z_انتہا);
  واپس p_Trigger_قدم;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ساکن Trigger_قدم *trigger_قدم_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ثابت حرف *z_آغاز,         /* Start of SQL text */
  ثابت حرف *z_انتہا            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_قدم *p_Trigger_قدم = 0;

  اگر( pParse->nErr==0 ){
    اگر( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_پیغام(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }ورنہ{
      p_Trigger_قدم = sqlite3DbMallocZero(db, حجم(Trigger_قدم));
      اگر( p_Trigger_قدم ){
        p_Trigger_قدم->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_قدم->op = op;
        p_Trigger_قدم->z_وسعت = trigger_وسعت_Dup(db, z_آغاز, z_انتہا);
        اگر( p_Trigger_قدم->pSrc && اندر_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_قدم->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  واپس p_Trigger_قدم;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_قدم *sqlite3_Trigger_داخل_کرو_قدم(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *p_عمود,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ثابت حرف *z_آغاز, /* Start of SQL text */
  ثابت حرف *z_انتہا    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_قدم *p_Trigger_قدم;

  أكد(pSelect != 0 || db->mallocFailed);

  p_Trigger_قدم = trigger_قدم_Allocate(pParse, TK_داخل_کرو, pTabList, z_آغاز, z_انتہا);
  اگر( p_Trigger_قدم ){
    اگر( اندر_RENAME_OBJECT ){
      p_Trigger_قدم->pSelect = pSelect;
      pSelect = 0;
    }ورنہ{
      p_Trigger_قدم->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_قدم->pIdList = p_عمود;
    p_Trigger_قدم->pUpsert = pUpsert;
    p_Trigger_قدم->orconf = orconf;
    اگر( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_ہدف);
    }
  }ورنہ{
    testcase( p_عمود );
    sqlite3IdListDelete(db, p_عمود);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  واپس p_Trigger_قدم;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_قدم *sqlite3_Trigger_Update_قدم(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  اظہاریہ_List *pEList,    /* The SET clause: list of column and new values */
  اظہاریہ *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ثابت حرف *z_آغاز,  /* Start of SQL text */
  ثابت حرف *z_انتہا     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_قدم *p_Trigger_قدم;

  p_Trigger_قدم = trigger_قدم_Allocate(pParse, TK_UPDATE, pTabList, z_آغاز, z_انتہا);
  اگر( p_Trigger_قدم ){
    SrcList *pFromDup = 0;
    اگر( اندر_RENAME_OBJECT ){
      p_Trigger_قدم->p_اظہاریہ_List = pEList;
      p_Trigger_قدم->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }ورنہ{
      p_Trigger_قدم->p_اظہاریہ_List = sqlite3_اظہاریہ_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_قدم->pWhere = sqlite3_اظہاریہ_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_قدم->orconf = orconf;

    اگر( pFromDup && !اندر_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_رکن(pParse, 0, 0, 0, &as, pSub ,0);
    }
    اگر( pFromDup && p_Trigger_قدم->pSrc ){
      p_Trigger_قدم->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_قدم->pSrc, pFromDup
      );
    }ورنہ{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_اظہاریہ_List_Delete(db, pEList);
  sqlite3_اظہاریہ_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  واپس p_Trigger_قدم;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_قدم *sqlite3_Trigger_Delete_قدم(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  اظہاریہ *pWhere,           /* The WHERE clause */
  ثابت حرف *z_آغاز,     /* Start of SQL text */
  ثابت حرف *z_انتہا        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_قدم *p_Trigger_قدم;

  p_Trigger_قدم = trigger_قدم_Allocate(pParse, TK_DELETE, pTabList, z_آغاز, z_انتہا);
  اگر( p_Trigger_قدم ){
    اگر( اندر_RENAME_OBJECT ){
      p_Trigger_قدم->pWhere = pWhere;
      pWhere = 0;
    }ورنہ{
      p_Trigger_قدم->pWhere = sqlite3_اظہاریہ_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_قدم->orconf = OE_Default;
  }
  sqlite3_اظہاریہ_Delete(db, pWhere);
  واپس p_Trigger_قدم;
}

/* 
** Recursively delete a Trigger structure
*/
خالی sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  اگر( pTrigger==0 || pTrigger->bReturning ) واپس;
  sqlite3_Delete_Trigger_قدم(db, pTrigger->قدم_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->جدول_2);
  sqlite3_اظہاریہ_Delete(db, pTrigger->pWhen);
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
خالی sqlite3DropTrigger(Parse *pParse, SrcList *pName, صحیح noErr){
  Trigger *pTrigger = 0;
  صحیح i;
  ثابت حرف *zDb;
  ثابت حرف *zName;
  sqlite3 *db = pParse->db;

  اگر( db->mallocFailed ) جاؤ drop_trigger_cleanup;
  اگر( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    جاؤ drop_trigger_cleanup;
  }

  أكد( pName->nSrc==1 );
  أكد( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  أكد( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  برائے(i=OMIT_TEMPDB; i<db->nDb; i++){
    صحیح j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    اگر( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) جاری;
    أكد( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3_Hash_ڈھونڈو(&(db->aDb[j].pSchema->trigHash), zName);
    اگر( pTrigger ) توڑو;
  }
  اگر( !pTrigger ){
    اگر( !noErr ){
      sqlite3_Error_پیغام(pParse, "no such trigger: %S", pName->a);
    }ورنہ{
      sqlite3_رمز_Verify_Named_Schema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    جاؤ drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

/*
** Return a pointer to the Table structure for the table that a trigger
** is set on.
*/
ساکن جدول *جدول_کا_Trigger(Trigger *pTrigger){
  واپس sqlite3_Hash_ڈھونڈو(&pTrigger->pTabSchema->tblHash, pTrigger->جدول_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
خالی sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  جدول   *p_جدول;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  صحیح iDb;

  iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTrigger->pSchema);
  أكد( iDb>=0 && iDb<db->nDb );
  p_جدول = جدول_کا_Trigger(pTrigger);
  أكد( (p_جدول && p_جدول->pSchema==pTrigger->pSchema) || iDb==1 );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  اگر( p_جدول ){
    صحیح رمز = SQLITE_DROP_TRIGGER;
    ثابت حرف *zDb = db->aDb[iDb].zDbSName;
    ثابت حرف *zTab = SCHEMA_جدول(iDb);
    اگر( iDb==1 ) رمز = SQLITE_DROP_عارضی_TRIGGER;
    اگر( sqlite3AuthCheck(pParse, رمز, pTrigger->zName, p_جدول->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      واپس;
    }
  }
#اگر_ختم

  /* Generate code to destroy the database record of the trigger.
  */
  اگر( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_جدول " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
  }
}

/*
** Remove a trigger from the hash tables of the sqlite* pointer.
*/
خالی sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, صحیح iDb, ثابت حرف *zName){
  Trigger *pTrigger;
  Hash *pHash;

  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3_Hash_داخل_کرو(pHash, zName, 0);
  اگر( ALWAYS(pTrigger) ){
    اگر( pTrigger->pSchema==pTrigger->pTabSchema ){
      جدول *pTab = جدول_کا_Trigger(pTrigger);
      اگر( pTab ){
        Trigger **pp;
        برائے(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_اگلا)){
          اگر( *pp==pTrigger ){
            *pp = (*pp)->p_اگلا;
            توڑو;
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
ساکن صحیح check_عمود_Overlap(IdList *pIdList, اظہاریہ_List *pEList){
  صحیح e;
  اگر( pIdList==0 || NEVER(pEList==0) ) واپس 1;
  برائے(e=0; e<pEList->n_اظہاریہ; e++){
    اگر( sqlite3_Id_List_اشاریہ(pIdList, pEList->a[e].zEName)>=0 ) واپس 1;
  }
  واپس 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ساکن صحیح عارضی_Triggers_Exist(sqlite3 *db){
  اگر( NEVER(db->aDb[1].pSchema==0) ) واپس 0;
  اگر( sqlite_Hash_پہلا(&db->aDb[1].pSchema->trigHash)==0 ) واپس 0;
  واپس 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ساکن SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  جدول *pTab,            /* The table the contains the triggers */
  صحیح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  اظہاریہ_List *pChanges,     /* Columns that change in an UPDATE statement */
  صحیح *p_نقاب              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  صحیح نقاب = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  أكد( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_اگلا==0) );
  اگر( pList!=0 ){
    p = pList;
    اگر( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      اگر( pList==pTab->pTrigger ){
        pList = 0;
        جاؤ exit_triggers_exist;
      }
      جب( ALWAYS(p->p_اگلا) && p->p_اگلا!=pTab->pTrigger ) p = p->p_اگلا;
      p->p_اگلا = 0;
      p = pList;
    }
    کرو{
      اگر( p->op==op && check_عمود_Overlap(p->pColumns, pChanges) ){
        نقاب |= p->tr_tm;
      }ورنہ اگر( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        أكد( sqlite3IsToplevel(pParse) );
        p->op = op;
        اگر( IsVirtual(pTab) ){
          اگر( op!=TK_داخل_کرو ){
            sqlite3_Error_پیغام(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }ورنہ{
          p->tr_tm = TRIGGER_AFTER;
        }
        نقاب |= p->tr_tm;
      }ورنہ اگر( p->bReturning && p->op==TK_داخل_کرو && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        نقاب |= p->tr_tm;
      }
      p = p->p_اگلا;
    }جب( p );
  }
exit_triggers_exist:
  اگر( p_نقاب ){
    *p_نقاب = نقاب;
  }
  واپس (نقاب ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  جدول *pTab,            /* The table the contains the triggers */
  صحیح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  اظہاریہ_List *pChanges,     /* Columns that change in an UPDATE statement */
  صحیح *p_نقاب              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  أكد( pTab!=0 );
  اگر( (pTab->pTrigger==0 && !عارضی_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    اگر( p_نقاب ) *p_نقاب = 0;
    واپس 0;
  }
  واپس triggersReallyExist(pParse,pTab,op,pChanges,p_نقاب);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ساکن صحیح is_Asterisk_رکن(
  Parse *pParse,      /* Parsing context */
  اظہاریہ *p_رکن         /* A term in the RETURNING clause */
){
  أكد( p_رکن!=0 );
  اگر( p_رکن->op==TK_ASTERISK ) واپس 1;
  اگر( p_رکن->op!=TK_DOT ) واپس 0;
  أكد( p_رکن->p_دایاں!=0 );
  أكد( p_رکن->p_بایاں!=0 );
  اگر( p_رکن->p_دایاں->op!=TK_ASTERISK ) واپس 0;
  sqlite3_Error_پیغام(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  واپس 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ساکن اظہاریہ_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  اظہاریہ_List *pList,      /* The arguments to RETURNING */
  جدول *pTab           /* The table being updated */
){
  اظہاریہ_List *pNew = 0;
  sqlite3 *db = pParse->db;
  صحیح i;

  برائے(i=0; i<pList->n_اظہاریہ; i++){
    اظہاریہ *p_Old_اظہاریہ = pList->a[i].p_اظہاریہ;
    اگر( NEVER(p_Old_اظہاریہ==0) ) جاری;
    اگر( is_Asterisk_رکن(pParse, p_Old_اظہاریہ) ){
      صحیح jj;
      برائے(jj=0; jj<pTab->n_ستون; jj++){
        اظہاریہ *p_New_اظہاریہ;
        اگر( Is_Hidden_عمود(pTab->a_ستون+jj) ) جاری;
        p_New_اظہاریہ = sqlite3_اظہاریہ(db, TK_ID, pTab->a_ستون[jj].zCnName);
        pNew = sqlite3_اظہاریہ_List_Append(pParse, pNew, p_New_اظہاریہ);
        اگر( !db->mallocFailed ){
          ساخت اظہاریہ_List_شے *p_شے = &pNew->a[pNew->n_اظہاریہ-1];
          p_شے->zEName = sqlite3DbStrDup(db, pTab->a_ستون[jj].zCnName);
          p_شے->fg.eEName = ENAME_NAME;
        }
      }
    }ورنہ{
      اظہاریہ *p_New_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_Old_اظہاریہ, 0);
      pNew = sqlite3_اظہاریہ_List_Append(pParse, pNew, p_New_اظہاریہ);
      اگر( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        ساخت اظہاریہ_List_شے *p_شے = &pNew->a[pNew->n_اظہاریہ-1];
        p_شے->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        p_شے->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  واپس pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ساکن صحیح sqlite3ReturningSubqueryVarSelect(Walker *Not_مستعمل, اظہاریہ *p_اظہاریہ){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  اگر( اظہاریہ_Use_XSelect(p_اظہاریہ)
   && (p_اظہاریہ->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_VarSelect) );
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_VarSelect);
  }
  واپس WRC_Continue;
}


/*
** If the SELECT references the table pWalker->u.pTab, then do two things:
**
**    (1) Mark the SELECT as as SF_Correlated.
**    (2) Set pWalker->eCode to non-zero so that the caller will know
**        that (1) has happened.
*/
ساکن صحیح sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect){
  صحیح i;
  SrcList *pSrc;
  أكد( pSelect!=0 );
  pSrc = pSelect->pSrc;
  أكد( pSrc!=0 );
  برائے(i=0; i<pSrc->nSrc; i++){
    اگر( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_رمز = 1;
      توڑو;
    }
  }
  واپس WRC_Continue;
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
ساکن خالی sqlite3ProcessReturningSubqueries(
  اظہاریہ_List *pEList,
  جدول *pTab
){
  Walker w;
  memset(&w, 0, حجم(w));
  w.x_اظہاریہ_Callback = sqlite3_اظہاریہ_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_اظہاریہ_List(&w, pEList);
  اگر( w.e_رمز ){
    w.x_اظہاریہ_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_اظہاریہ_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ساکن خالی رمز_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  جدول *pTab,         /* The table to code triggers from */
  صحیح reg_اندر            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  اظہاریہ_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  اتحاد {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  أكد( v!=0 );
  اگر( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    واپس;
  }
  أكد( db->pParse==pParse );
  أكد( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  اگر( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    واپس;
  }
  memset(&sSelect, 0, حجم(sSelect));
  memset(&uSrc, 0, حجم(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_اظہاریہ_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  اگر( pParse->nErr==0 ){
    أكد( db->mallocFailed==0 );
    sqlite3_Generate_عمود_نام(pParse, &sSelect);
  }
  sqlite3_اظہاریہ_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  اگر( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, حجم(sNC));
    اگر( pReturning->n_Ret_ستون==0 ){
      pReturning->n_Ret_ستون = pNew->n_اظہاریہ;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_بنیاد_Reg = reg_اندر;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    اگر( sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      صحیح i;
      صحیح n_ستون = pNew->n_اظہاریہ;
      صحیح reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += n_ستون+2;
      pReturning->iRetReg = reg;
      برائے(i=0; i<n_ستون; i++){
        اظہاریہ *p_ستون = pNew->a[i].p_اظہاریہ;
        أكد( p_ستون!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_اظہاریہ_رمز_Factorable(pParse, p_ستون, reg+i);
        اگر( sqlite3_اظہاریہ_Affinity(p_ستون)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg, i, reg+i);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_اظہاریہ_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ساکن صحیح رمز_Trigger_پروگرام(
  Parse *pParse,            /* The parser context */
  Trigger_قدم *p_قدم_List,   /* List of statements inside the trigger body */
  صحیح orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_قدم *p_قدم;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  أكد( pParse->pTriggerTab && pParse->pToplevel );
  أكد( p_قدم_List );
  أكد( v!=0 );
  برائے(p_قدم=p_قدم_List; p_قدم; p_قدم=p_قدم->p_اگلا){
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
    pParse->eOrconf = (orconf==OE_Default)?p_قدم->orconf:(u8)orconf;
    أكد( pParse->ok_Const_ضربی_عدد==0 );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
    اگر( p_قدم->z_وسعت ){
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_قدم->z_وسعت),
                        P4_DYNAMIC);
    }
#اگر_ختم

    انتخاب( p_قدم->op ){
      حالت TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_قدم->pSrc, 0),
          sqlite3_اظہاریہ_List_Dup(db, p_قدم->p_اظہاریہ_List, 0), 
          sqlite3_اظہاریہ_Dup(db, p_قدم->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_جوڑو_Op0(v, OP_Reset_گنتی);
        توڑو;
      }
      حالت TK_داخل_کرو: {
        sqlite3_داخل_کرو(pParse, 
          sqlite3SrcListDup(db, p_قدم->pSrc, 0),
          sqlite3SelectDup(db, p_قدم->pSelect, 0), 
          sqlite3IdListDup(db, p_قدم->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_قدم->pUpsert)
        );
        sqlite3_Vdbe_جوڑو_Op0(v, OP_Reset_گنتی);
        توڑو;
      }
      حالت TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_قدم->pSrc, 0),
          sqlite3_اظہاریہ_Dup(db, p_قدم->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_جوڑو_Op0(v, OP_Reset_گنتی);
        توڑو;
      }
      طےشدہ: أكد( p_قدم->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_قدم->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        توڑو;
      }
    } 
  }

  واپس 0;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ساکن ثابت حرف *on_Error_متن(صحیح onError){
  انتخاب( onError ){
    حالت OE_Abort:    واپس "abort";
    حالت OE_Rollback: واپس "rollback";
    حالت OE_Fail:     واپس "fail";
    حالت OE_Replace:  واپس "replace";
    حالت OE_Ignore:   واپس "ignore";
    حالت OE_Default:  واپس "default";
  }
  واپس "n/a";
}
#اگر_ختم

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ساکن خالی transferParseError(Parse *p_مقصد, Parse *pFrom){
  أكد( pFrom->z_Err_پیغام==0 || pFrom->nErr );
  أكد( p_مقصد->z_Err_پیغام==0 || p_مقصد->nErr );
  اگر( p_مقصد->nErr==0 ){
    p_مقصد->z_Err_پیغام = pFrom->z_Err_پیغام;
    p_مقصد->nErr = pFrom->nErr;
    p_مقصد->rc = pFrom->rc;
  }ورنہ{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_پیغام);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ساکن TriggerPrg *رمز_صف_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  جدول *pTab,         /* The table pTrigger is attached to */
  صحیح orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_چوٹی;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  اظہاریہ *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_پروگرام *p_پروگرام = 0;   /* Sub-vdbe for trigger program */
  صحیح i_انتہا_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  صحیح n_گہرائی;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_چوٹی = pParse;
  برائے(n_گہرائی=0; p_چوٹی->pOuterParse; p_چوٹی = p_چوٹی->pOuterParse, n_گہرائی++){}
  اگر( n_گہرائی>=db->a_حد_بندی[SQLITE_حد_بندی_TRIGGER_گہرائی] ){
    sqlite3_Error_پیغام(pParse, "triggers nested too deep");
    واپس 0;
  }

  p_چوٹی = sqlite3ParseToplevel(pParse);
  أكد( pTrigger->zName==0 || pTab==جدول_کا_Trigger(pTrigger) );
  أكد( p_چوٹی->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, حجم(TriggerPrg));
  اگر( !pPrg ) واپس 0;
  pPrg->p_اگلا = p_چوٹی->pTriggerPrg;
  p_چوٹی->pTriggerPrg = pPrg;
  pPrg->p_پروگرام = p_پروگرام = sqlite3DbMallocZero(db, حجم(Sub_پروگرام));
  اگر( !p_پروگرام ) واپس 0;
  sqlite3_Vdbe_Link_Sub_پروگرام(p_چوٹی->pVdbe, p_پروگرام);
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
  sSubParse.pToplevel = p_چوٹی;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  اگر( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, on_Error_متن(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_داخل_کرو ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
    اگر( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#اگر_ختم

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    اگر( pTrigger->pWhen ){
      pWhen = sqlite3_اظہاریہ_Dup(db, pTrigger->pWhen, 0);
      اگر( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_اظہاریہ_نام(&sNC, pWhen) 
      ){
        i_انتہا_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_اظہاریہ_If_False(&sSubParse, pWhen, i_انتہا_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_اظہاریہ_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    رمز_Trigger_پروگرام(&sSubParse, pTrigger->قدم_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    اگر( i_انتہا_Trigger ){
      sqlite3VdbeResolveLabel(v, i_انتہا_Trigger);
    }
    sqlite3_Vdbe_جوڑو_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, on_Error_متن(orconf)));
    transferParseError(pParse, &sSubParse);

    اگر( pParse->nErr==0 ){
      أكد( db->mallocFailed==0 );
      p_پروگرام->aOp = sqlite3VdbeTakeOpArray(v, &p_پروگرام->nOp, &p_چوٹی->nMaxArg);
    }
    p_پروگرام->nMem = sSubParse.nMem;
    p_پروگرام->nCsr = sSubParse.nTab;
    p_پروگرام->token = (خالی *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }ورنہ{
    transferParseError(pParse, &sSubParse);
  }

  أكد( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  واپس pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ساکن TriggerPrg *get_صف_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  جدول *pTab,         /* The table trigger pTrigger is attached to */
  صحیح orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_جڑ = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  أكد( pTrigger->zName==0 || pTab==جدول_کا_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  برائے(pPrg=p_جڑ->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_اگلا
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  اگر( !pPrg ){
    pPrg = رمز_صف_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_انحراف = -1;
  }

  واپس pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
خالی sqlite3_رمز_صف_Trigger_براہ_راست(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  جدول *pTab,         /* The table to code triggers from */
  صحیح reg,             /* Reg array containing OLD.* and NEW.* values */
  صحیح orconf,          /* ON CONFLICT policy */
  صحیح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = get_صف_Trigger(pParse, p, pTab, orconf);
  أكد( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  اگر( pPrg ){
    صحیح bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_جوڑو_Op4(v, OP_پروگرام, reg, ignoreJump, ++pParse->nMem,
                      (ثابت حرف *)pPrg->p_پروگرام, P4_SUBPROGRAM);
    VdbeComment(
        (v, "Call: %s.%s", (p->zName?p->zName:"fkey"), on_Error_متن(orconf)));

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
خالی sqlite3_رمز_صف_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  صحیح op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  اظہاریہ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحیح tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  جدول *pTab,         /* The table to code triggers from */
  صحیح reg,             /* The first in an array of registers (see above) */
  صحیح orconf,          /* ON CONFLICT policy */
  صحیح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  أكد( op==TK_UPDATE || op==TK_داخل_کرو || op==TK_DELETE );
  أكد( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  أكد( (op==TK_UPDATE)==(pChanges!=0) );

  برائے(p=pTrigger; p; p=p->p_اگلا){

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
    اگر( (p->op==op || (p->bReturning && p->op==TK_داخل_کرو && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && check_عمود_Overlap(p->pColumns, pChanges)
    ){
      اگر( !p->bReturning ){
        sqlite3_رمز_صف_Trigger_براہ_راست(pParse, p, pTab, reg, orconf, ignoreJump);
      }ورنہ اگر( sqlite3IsToplevel(pParse) ){
        رمز_Returning_Trigger(pParse, p, pTab, reg);
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
  اظہاریہ_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحیح isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  صحیح tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  جدول *pTab,         /* The table to code triggers from */
  صحیح orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ثابت صحیح op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 نقاب = 0;
  Trigger *p;

  أكد( isNew==1 || isNew==0 );
  اگر( IsView(pTab) ){
    واپس 0xffffffff;
  }
  برائے(p=pTrigger; p; p=p->p_اگلا){
    اگر( p->op==op
     && (tr_tm&p->tr_tm)
     && check_عمود_Overlap(p->pColumns,pChanges)
    ){
      اگر( p->bReturning ){
        نقاب = 0xffffffff;
      }ورنہ{
        TriggerPrg *pPrg;
        pPrg = get_صف_Trigger(pParse, p, pTab, orconf);
        اگر( pPrg ){
          نقاب |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  واپس نقاب;
}

#اگر_ختم /* !defined(SQLITE_OMIT_TRIGGER) */
