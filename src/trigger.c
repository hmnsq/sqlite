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

#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
فراغ sqlite3_Delete_Trigger_ګام(sqlite3 *db, Trigger_ګام *p_Trigger_ګام){
  طالما( p_Trigger_ګام ){
    Trigger_ګام * pTmp = p_Trigger_ګام;
    p_Trigger_ګام = p_Trigger_ګام->p_بل;

    sqlite3_بیان_Delete(db, pTmp->pWhere);
    sqlite3_بیان_List_Delete(db, pTmp->p_بیان_List);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->z_پراختیا);

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
  p = sqlite_Hash_لومړی(&pTmpSchema->trigHash);
  pList = pTab->pTrigger;
  طالما( p ){
    Trigger *pTrig = (Trigger *)sqlite_Hash_معلومات(p);
    إذا( pTrig->pTabSchema==pTab->pSchema
     && pTrig->جدول_2
     && 0==sqlite3StrICmp(pTrig->جدول_2, pTab->zName)
     && (pTrig->pTabSchema!=pTmpSchema || pTrig->bReturning)
    ){
      pTrig->p_بل = pList;
      pList = pTrig;
    }وإلا إذا( pTrig->op==TK_RETURNING ){
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
      أكد( pParse->db->pVtabCtx==0 );
#د_که_پای
      أكد( pParse->bReturning );
      أكد( !pParse->isCreate );
      أكد( &(pParse->u1.d.pReturning->retTrig) == pTrig );
      pTrig->جدول_2 = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->p_بل = pList;
      pList = pTrig;
    }        
    p = sqlite_Hash_بل(p);    
  }
#إذا 0
  إذا( pList ){
    Trigger *pX;
    چاپ_کړه("Triggers for %s:", pTab->zName);
    لكل(pX=pList; pX; pX=pX->p_بل){
      چاپ_کړه(" %s", pX->zName);
    }
    چاپ_کړه("\n");
    fflush(stdout);
  }
#د_که_پای
  أرجع pList;  
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
  SrcList *p_جدول_Name,/* The name of the table/view the trigger applies to */
  بیان *pWhen,        /* WHEN clause */
  صحيح is_لنډمهاله,         /* True if the TEMPORARY keyword is present */
  صحيح noErr           /* Suppress errors if the trigger already exists */
){
  Trigger *pTrigger = 0;  /* The new trigger */
  جدول *pTab;            /* Table that the trigger fires off of */
  حرف *zName = 0;        /* Name of the trigger */
  sqlite3 *db = pParse->db;  /* The database connection */
  صحيح iDb;                /* The database to store the trigger in */
  Token *pName;           /* The unqualified db name */
  DbFixer sFix;           /* State vector for the DB fixer */

  أكد( pName1!=0 );   /* pName1->z might be NULL, but not pName1 itself */
  أكد( pName2!=0 );
  أكد( op==TK_ورننباسه || op==TK_UPDATE || op==TK_DELETE );
  أكد( op>0 && op<0xff );
  إذا( is_لنډمهاله ){
    /* If TEMP was specified, then the trigger name may not be qualified. */
    إذا( pName2->n>0 ){
      sqlite3_Error_پیغام(pParse, "temporary trigger may not have qualified name");
      اذهب trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  }وإلا{
    /* Figure out the db that the trigger will be created in */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    إذا( iDb<0 ){
      اذهب trigger_cleanup;
    }
  }
  إذا( !p_جدول_Name || db->mallocFailed ){
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
  إذا( db->init.busy && iDb!=1 ){
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
  إذا( db->init.busy==0 && pName2->n==0 && pTab
        && pTab->pSchema==db->aDb[1].pSchema ){
    iDb = 1;
  }

  /* Ensure the table name matches database name and that the table exists */
  إذا( db->mallocFailed ) اذهب trigger_cleanup;
  أكد( p_جدول_Name->nSrc==1 );
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  إذا( sqlite3FixSrcList(&sFix, p_جدول_Name) ){
    اذهب trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, p_جدول_Name);
  إذا( !pTab ){
    /* The table does not exist. */
    اذهب trigger_orphan_error;
  }
  إذا( IsVirtual(pTab) ){
    sqlite3_Error_پیغام(pParse, "cannot create triggers on virtual tables");
    اذهب trigger_orphan_error;
  }
  إذا( (pTab->tabFlags & TF_Shadow)!=0 && sqlite3ReadOnlyShadowTables(db) ){
    sqlite3_Error_پیغام(pParse, "cannot create triggers on shadow tables");
    اذهب trigger_orphan_error;
  }

  /* Check that the trigger name is not reserved and that no trigger of the
  ** specified name exists */
  zName = sqlite3NameFromToken(db, pName);
  إذا( zName==0 ){
    أكد( db->mallocFailed );
    اذهب trigger_cleanup;
  }
  إذا( sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName) ){
    اذهب trigger_cleanup;
  }
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  إذا( !دننه_RENAME_OBJECT ){
    إذا( sqlite3_Hash_پیدا_کړه(&(db->aDb[iDb].pSchema->trigHash),zName) ){
      إذا( !noErr ){
        sqlite3_Error_پیغام(pParse, "trigger %T already exists", pName);
      }وإلا إذا( db->init.busy ){
        sqlite3_Error_پیغام(pParse,""); /* Err msg generated by corruptSchema() */
      }وإلا{
        sqlite3_کوډ_Verify_Schema(pParse, iDb);
        VVA_ONLY( pParse->ifNotExists = 1; )
      }
      اذهب trigger_cleanup;
    }
  }

  /* NB: The SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES compile-time option is
  ** experimental and unsupported. Do not use it unless understand the
  ** implications and you cannot get by without this capability. */
#إذا !defined(SQLITE_ALLOW_TRIGGERS_ON_SYSTEM_TABLES) /* Experimental */
  /* Do not create a trigger on a system table */
  إذا( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3_Error_پیغام(pParse, "cannot create trigger on system table");
    اذهب trigger_cleanup;
  }
#د_که_پای

  /* INSTEAD of triggers are only for views and views only support INSTEAD
  ** of triggers.
  */
  إذا( IsView(pTab) && tr_tm!=TK_INSTEAD ){
    sqlite3_Error_پیغام(pParse, "cannot create %s trigger on view: %S", 
        (tr_tm == TK_BEFORE)?"BEFORE":"AFTER", p_جدول_Name->a);
    اذهب trigger_orphan_error;
  }
  إذا( !IsView(pTab) && tr_tm==TK_INSTEAD ){
    sqlite3_Error_پیغام(pParse, "cannot create INSTEAD OF"
        " trigger on table: %S", p_جدول_Name->a);
    اذهب trigger_orphan_error;
  }

#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  إذا( !دننه_RENAME_OBJECT ){
    صحيح iTabDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
    صحيح کوډ = SQLITE_CREATE_TRIGGER;
    ثابت حرف *zDb = db->aDb[iTabDb].zDbSName;
    ثابت حرف *zDbTrig = is_لنډمهاله ? db->aDb[1].zDbSName : zDb;
    إذا( iTabDb==1 || is_لنډمهاله ) کوډ = SQLITE_CREATE_لنډمهاله_TRIGGER;
    إذا( sqlite3AuthCheck(pParse, کوډ, zName, pTab->zName, zDbTrig) ){
      اذهب trigger_cleanup;
    }
    إذا( sqlite3AuthCheck(pParse, SQLITE_ورننباسه, SCHEMA_جدول(iTabDb),0,zDb)){
      اذهب trigger_cleanup;
    }
  }
#د_که_پای

  /* INSTEAD OF triggers can only appear on views and BEFORE triggers
  ** cannot appear on views.  So we might as well translate every
  ** INSTEAD OF trigger into a BEFORE trigger.  It simplifies code
  ** elsewhere.
  */
  إذا (tr_tm == TK_INSTEAD){
    tr_tm = TK_BEFORE;
  }

  /* Build the Trigger object */
  pTrigger = (Trigger*)sqlite3DbMallocZero(db, حجم(Trigger));
  إذا( pTrigger==0 ) اذهب trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->جدول_2 = sqlite3DbStrDup(db, p_جدول_Name->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm==TK_BEFORE ? TRIGGER_BEFORE : TRIGGER_AFTER;
  إذا( دننه_RENAME_OBJECT ){
    sqlite3RenameTokenRemap(pParse, pTrigger->جدول_2, p_جدول_Name->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  }وإلا{
    pTrigger->pWhen = sqlite3_بیان_Dup(db, pWhen, EXPRDUP_REDUCE);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;
  أكد( pParse->pNewTrigger==0 );
  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, p_جدول_Name);
  sqlite3IdListDelete(db, pColumns);
  sqlite3_بیان_Delete(db, pWhen);
  إذا( !pParse->pNewTrigger ){
    sqlite3DeleteTrigger(db, pTrigger);
  }وإلا{
    أكد( pParse->pNewTrigger==pTrigger );
  }
  أرجع;

trigger_orphan_error:
  إذا( db->init.iDb==1 ){
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
فراغ sqlite3_پای_Trigger(
  Parse *pParse,          /* Parser context */
  Trigger_ګام *p_ګام_List, /* The triggered program */
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
  إذا( NEVER(pParse->nErr) || !pTrig ) اذهب triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3_Schema_منزل_شاخص(pParse->db, pTrig->pSchema);
  أكد( iDb>=00 && iDb<db->nDb );
  pTrig->ګام_list = p_ګام_List;
  لكل(n=0; p_ګام_List; n++){
    p_ګام_List->pTrig = pTrig;
    p_ګام_List = p_ګام_List->p_بل;
  }
  إذا( n>pParse->db->a_حد[SQLITE_حد_TRIGGER_ګامونه] ){
    sqlite3_Error_پیغام(pParse, "trigger \"%w\" contains too many steps", zName);
    اذهب triggerfinish_cleanup;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  إذا( sqlite3_Fix_Trigger_ګام(&sFix, pTrig->ګام_list) 
   || sqlite3_Fix_بیان(&sFix, pTrig->pWhen) 
  ){
    اذهب triggerfinish_cleanup;
  }

#که_نه_تعریف_شوی SQLITE_OMIT_ALTERTABLE
  إذا( دننه_RENAME_OBJECT ){
    أكد( !db->init.busy );
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  }وإلا
#د_که_پای

  /* if we are not initializing,
  ** build the sqlite_schema entry
  */
  إذا( !db->init.busy ){
    Vdbe *v;
    حرف *z;

    /* If this is a new CREATE TABLE statement, and if shadow tables
    ** are read-only, and the trigger makes a change to a shadow table,
    ** then raise an error - do not allow the trigger to be created. */
    إذا( sqlite3ReadOnlyShadowTables(db) ){
      Trigger_ګام *p_ګام;
      لكل(p_ګام=pTrig->ګام_list; p_ګام; p_ګام=p_ګام->p_بل){
        إذا( p_ګام->pSrc!=0
         && sqlite3_Shadow_جدول_Name(db, p_ګام->pSrc->a[0].zName)
        ){
          sqlite3_Error_پیغام(pParse, 
            "trigger \"%s\" may not write to shadow table \"%s\"",
            pTrig->zName, p_ګام->pSrc->a[0].zName);
          اذهب triggerfinish_cleanup;
        }
      }
    }

    /* Make an entry in the sqlite_schema table */
    v = sqlite3GetVdbe(pParse);
    إذا( v==0 ) اذهب triggerfinish_cleanup;
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
    sqlite3_Vdbe_جمع_کړه_Parse_Schema_Op(v, iDb,
        sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  إذا( db->init.busy ){
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;
    أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
    أكد( pLink!=0 );
    pTrig = sqlite3_Hash_ورننباسه(pHash, zName, pTrig);
    إذا( pTrig ){
      sqlite3OomFault(db);
    }وإلا إذا( pLink->pSchema==pLink->pTabSchema ){
      جدول *pTab;
      pTab = sqlite3_Hash_پیدا_کړه(&pLink->pTabSchema->tblHash, pLink->جدول_2);
      أكد( pTab!=0 );
      pLink->p_بل = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);
  أكد( دننه_RENAME_OBJECT || !pParse->pNewTrigger );
  sqlite3_Delete_Trigger_ګام(db, p_ګام_List);
}

/*
** Duplicate a range of text from an SQL statement, then convert all
** whitespace characters into ordinary space characters.
*/
ساكن حرف *trigger_پراختیا_Dup(sqlite3 *db, ثابت حرف *z_پیل, ثابت حرف *z_پای_ټکی){
  حرف *z = sqlite3_Db_پراختیا_Dup(db, z_پیل, z_پای_ټکی);
  صحيح i;
  إذا( z ) لكل(i=0; z[i]; i++) إذا( sqlite3Isspace(z[i]) ) z[i] = ' ';
  أرجع z;
}    

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
Trigger_ګام *sqlite3_Trigger_Select_ګام(
  sqlite3 *db,                /* Database connection */
  Select *pSelect,            /* The SELECT statement */
  ثابت حرف *z_پیل,         /* Start of SQL text */
  ثابت حرف *z_پای_ټکی            /* End of SQL text */
){
  Trigger_ګام *p_Trigger_ګام = sqlite3DbMallocZero(db, حجم(Trigger_ګام));
  إذا( p_Trigger_ګام==0 ) {
    sqlite3SelectDelete(db, pSelect);
    أرجع 0;
  }
  p_Trigger_ګام->op = TK_SELECT;
  p_Trigger_ګام->pSelect = pSelect;
  p_Trigger_ګام->orconf = OE_Default;
  p_Trigger_ګام->z_پراختیا = trigger_پراختیا_Dup(db, z_پیل, z_پای_ټکی);
  أرجع p_Trigger_ګام;
}

/*
** Allocate space to hold a new trigger step.  The allocated space
** holds both the TriggerStep object and the TriggerStep.target.z string.
**
** If an OOM error occurs, NULL is returned and db->mallocFailed is set.
*/
ساكن Trigger_ګام *trigger_ګام_Allocate(
  Parse *pParse,              /* Parser context */
  u8 op,                      /* Trigger opcode */
  SrcList *pTabList,          /* Target table */
  ثابت حرف *z_پیل,         /* Start of SQL text */
  ثابت حرف *z_پای_ټکی            /* End of SQL text */
){
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  Trigger_ګام *p_Trigger_ګام = 0;

  إذا( pParse->nErr==0 ){
    إذا( pNew 
     && pNew->pSchema!=db->aDb[1].pSchema 
     && pTabList->a[0].u4.zDatabase 
    ){
      sqlite3_Error_پیغام(pParse, 
          "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
          "statements within triggers");
    }وإلا{
      p_Trigger_ګام = sqlite3DbMallocZero(db, حجم(Trigger_ګام));
      إذا( p_Trigger_ګام ){
        p_Trigger_ګام->pSrc = sqlite3SrcListDup(db, pTabList, EXPRDUP_REDUCE);
        p_Trigger_ګام->op = op;
        p_Trigger_ګام->z_پراختیا = trigger_پراختیا_Dup(db, z_پیل, z_پای_ټکی);
        إذا( p_Trigger_ګام->pSrc && دننه_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, 
              p_Trigger_ګام->pSrc->a[0].zName, 
              pTabList->a[0].zName
          );
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  أرجع p_Trigger_ګام;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
Trigger_ګام *sqlite3_Trigger_ورننباسه_ګام(
  Parse *pParse,      /* Parser */
  SrcList *pTabList,  /* Table to INSERT into */
  IdList *p_ستنه,    /* List of columns in pTableName to insert into */
  Select *pSelect,    /* A SELECT statement that supplies values */
  u8 orconf,          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
  Upsert *pUpsert,    /* ON CONFLICT clauses for upsert */
  ثابت حرف *z_پیل, /* Start of SQL text */
  ثابت حرف *z_پای_ټکی    /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ګام *p_Trigger_ګام;

  أكد(pSelect != 0 || db->mallocFailed);

  p_Trigger_ګام = trigger_ګام_Allocate(pParse, TK_ورننباسه, pTabList, z_پیل, z_پای_ټکی);
  إذا( p_Trigger_ګام ){
    إذا( دننه_RENAME_OBJECT ){
      p_Trigger_ګام->pSelect = pSelect;
      pSelect = 0;
    }وإلا{
      p_Trigger_ګام->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
    }
    p_Trigger_ګام->pIdList = p_ستنه;
    p_Trigger_ګام->pUpsert = pUpsert;
    p_Trigger_ګام->orconf = orconf;
    إذا( pUpsert ){
      sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_موخه);
    }
  }وإلا{
    testcase( p_ستنه );
    sqlite3IdListDelete(db, p_ستنه);
    testcase( pUpsert );
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  أرجع p_Trigger_ګام;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ګام *sqlite3_Trigger_Update_ګام(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,   /* Name of the table to be updated */
  SrcList *pFrom,      /* FROM clause for an UPDATE-FROM, or NULL */
  بیان_List *pEList,    /* The SET clause: list of column and new values */
  بیان *pWhere,        /* The WHERE clause */
  u8 orconf,           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
  ثابت حرف *z_پیل,  /* Start of SQL text */
  ثابت حرف *z_پای_ټکی     /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ګام *p_Trigger_ګام;

  p_Trigger_ګام = trigger_ګام_Allocate(pParse, TK_UPDATE, pTabList, z_پیل, z_پای_ټکی);
  إذا( p_Trigger_ګام ){
    SrcList *pFromDup = 0;
    إذا( دننه_RENAME_OBJECT ){
      p_Trigger_ګام->p_بیان_List = pEList;
      p_Trigger_ګام->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    }وإلا{
      p_Trigger_ګام->p_بیان_List = sqlite3_بیان_List_Dup(db, pEList, EXPRDUP_REDUCE);
      p_Trigger_ګام->pWhere = sqlite3_بیان_Dup(db, pWhere, EXPRDUP_REDUCE);
      pFromDup = sqlite3SrcListDup(db, pFrom, EXPRDUP_REDUCE);
    }
    p_Trigger_ګام->orconf = orconf;

    إذا( pFromDup && !دننه_RENAME_OBJECT){
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0,0,0,0, SF_NestedFrom, 0);
      pFromDup = sqlite3_Src_List_Append_From_غړی(pParse, 0, 0, 0, &as, pSub ,0);
    }
    إذا( pFromDup && p_Trigger_ګام->pSrc ){
      p_Trigger_ګام->pSrc = sqlite3SrcListAppendList(
          pParse, p_Trigger_ګام->pSrc, pFromDup
      );
    }وإلا{
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3_بیان_List_Delete(db, pEList);
  sqlite3_بیان_Delete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  أرجع p_Trigger_ګام;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
Trigger_ګام *sqlite3_Trigger_Delete_ګام(
  Parse *pParse,          /* Parser */
  SrcList *pTabList,      /* The table from which rows are deleted */
  بیان *pWhere,           /* The WHERE clause */
  ثابت حرف *z_پیل,     /* Start of SQL text */
  ثابت حرف *z_پای_ټکی        /* End of SQL text */
){
  sqlite3 *db = pParse->db;
  Trigger_ګام *p_Trigger_ګام;

  p_Trigger_ګام = trigger_ګام_Allocate(pParse, TK_DELETE, pTabList, z_پیل, z_پای_ټکی);
  إذا( p_Trigger_ګام ){
    إذا( دننه_RENAME_OBJECT ){
      p_Trigger_ګام->pWhere = pWhere;
      pWhere = 0;
    }وإلا{
      p_Trigger_ګام->pWhere = sqlite3_بیان_Dup(db, pWhere, EXPRDUP_REDUCE);
    }
    p_Trigger_ګام->orconf = OE_Default;
  }
  sqlite3_بیان_Delete(db, pWhere);
  أرجع p_Trigger_ګام;
}

/* 
** Recursively delete a Trigger structure
*/
فراغ sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger){
  إذا( pTrigger==0 || pTrigger->bReturning ) أرجع;
  sqlite3_Delete_Trigger_ګام(db, pTrigger->ګام_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->جدول_2);
  sqlite3_بیان_Delete(db, pTrigger->pWhen);
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

  إذا( db->mallocFailed ) اذهب drop_trigger_cleanup;
  إذا( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    اذهب drop_trigger_cleanup;
  }

  أكد( pName->nSrc==1 );
  أكد( pName->a[0].fg.fixedSchema==0 && pName->a[0].fg.isSubquery==0 );
  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;
  أكد( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  لكل(i=OMIT_TEMPDB; i<db->nDb; i++){
    صحيح j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    إذا( zDb && sqlite3DbIsNamed(db, j, zDb)==0 ) استمر;
    أكد( sqlite3SchemaMutexHeld(db, j, 0) );
    pTrigger = sqlite3_Hash_پیدا_کړه(&(db->aDb[j].pSchema->trigHash), zName);
    إذا( pTrigger ) اكسر;
  }
  إذا( !pTrigger ){
    إذا( !noErr ){
      sqlite3_Error_پیغام(pParse, "no such trigger: %S", pName->a);
    }وإلا{
      sqlite3_کوډ_Verify_Named_Schema(pParse, zDb);
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
ساكن جدول *جدول_د_Trigger(Trigger *pTrigger){
  أرجع sqlite3_Hash_پیدا_کړه(&pTrigger->pTabSchema->tblHash, pTrigger->جدول_2);
}


/*
** Drop a trigger given a pointer to that trigger. 
*/
فراغ sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){
  جدول   *p_جدول;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  صحيح iDb;

  iDb = sqlite3_Schema_منزل_شاخص(pParse->db, pTrigger->pSchema);
  أكد( iDb>=0 && iDb<db->nDb );
  p_جدول = جدول_د_Trigger(pTrigger);
  أكد( (p_جدول && p_جدول->pSchema==pTrigger->pSchema) || iDb==1 );
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  إذا( p_جدول ){
    صحيح کوډ = SQLITE_DROP_TRIGGER;
    ثابت حرف *zDb = db->aDb[iDb].zDbSName;
    ثابت حرف *zTab = SCHEMA_جدول(iDb);
    إذا( iDb==1 ) کوډ = SQLITE_DROP_لنډمهاله_TRIGGER;
    إذا( sqlite3AuthCheck(pParse, کوډ, pTrigger->zName, p_جدول->zName, zDb) ||
      sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      أرجع;
    }
  }
#د_که_پای

  /* Generate code to destroy the database record of the trigger.
  */
  إذا( (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q." LEGACY_SCHEMA_جدول " WHERE name=%Q AND type='trigger'",
       db->aDb[iDb].zDbSName, pTrigger->zName
    );
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_DropTrigger, iDb, 0, 0, pTrigger->zName, 0);
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
  pTrigger = sqlite3_Hash_ورننباسه(pHash, zName, 0);
  إذا( ALWAYS(pTrigger) ){
    إذا( pTrigger->pSchema==pTrigger->pTabSchema ){
      جدول *pTab = جدول_د_Trigger(pTrigger);
      إذا( pTab ){
        Trigger **pp;
        لكل(pp=&pTab->pTrigger; *pp; pp=&((*pp)->p_بل)){
          إذا( *pp==pTrigger ){
            *pp = (*pp)->p_بل;
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
ساكن صحيح check_ستنه_Overlap(IdList *pIdList, بیان_List *pEList){
  صحيح e;
  إذا( pIdList==0 || NEVER(pEList==0) ) أرجع 1;
  لكل(e=0; e<pEList->n_بیان; e++){
    إذا( sqlite3_Id_List_شاخص(pIdList, pEList->a[e].zEName)>=0 ) أرجع 1;
  }
  أرجع 0; 
}

/*
** Return true if any TEMP triggers exist
*/
ساكن صحيح لنډمهاله_Triggers_Exist(sqlite3 *db){
  إذا( NEVER(db->aDb[1].pSchema==0) ) أرجع 0;
  إذا( sqlite_Hash_لومړی(&db->aDb[1].pSchema->trigHash)==0 ) أرجع 0;
  أرجع 1;
}

/*
** Return a list of all triggers on table pTab if there exists at least
** one trigger that must be fired when an operation of type 'op' is 
** performed on the table, and, if that operation is an UPDATE, if at
** least one of the columns in pChanges is being modified.
*/
ساكن SQLITE_NOINLINE Trigger *triggersReallyExist(
  Parse *pParse,          /* Parse context */
  جدول *pTab,            /* The table the contains the triggers */
  صحيح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  بیان_List *pChanges,     /* Columns that change in an UPDATE statement */
  صحيح *p_ماسک              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  صحيح ماسک = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);
  أكد( pList==0 || IsVirtual(pTab)==0 
           || (pList->bReturning && pList->p_بل==0) );
  إذا( pList!=0 ){
    p = pList;
    إذا( (pParse->db->flags & SQLITE_EnableTrigger)==0
     && pTab->pTrigger!=0
     && sqlite3_Schema_منزل_شاخص(pParse->db, pTab->pTrigger->pSchema)!=1
    ){
      /* The SQLITE_DBCONFIG_ENABLE_TRIGGER setting is off.  That means that
      ** only TEMP triggers are allowed.  Truncate the pList so that it
      ** includes only TEMP triggers */
      إذا( pList==pTab->pTrigger ){
        pList = 0;
        اذهب exit_triggers_exist;
      }
      طالما( ALWAYS(p->p_بل) && p->p_بل!=pTab->pTrigger ) p = p->p_بل;
      p->p_بل = 0;
      p = pList;
    }
    افعل{
      إذا( p->op==op && check_ستنه_Overlap(p->pColumns, pChanges) ){
        ماسک |= p->tr_tm;
      }وإلا إذا( p->op==TK_RETURNING ){
        /* The first time a RETURNING trigger is seen, the "op" value tells
        ** us what time of trigger it should be. */
        أكد( sqlite3IsToplevel(pParse) );
        p->op = op;
        إذا( IsVirtual(pTab) ){
          إذا( op!=TK_ورننباسه ){
            sqlite3_Error_پیغام(pParse,
              "%s RETURNING is not available on virtual tables",
              op==TK_DELETE ? "DELETE" : "UPDATE");
          }
          p->tr_tm = TRIGGER_BEFORE;
        }وإلا{
          p->tr_tm = TRIGGER_AFTER;
        }
        ماسک |= p->tr_tm;
      }وإلا إذا( p->bReturning && p->op==TK_ورننباسه && op==TK_UPDATE
                && sqlite3IsToplevel(pParse) ){
        /* Also fire a RETURNING trigger for an UPSERT */
        ماسک |= p->tr_tm;
      }
      p = p->p_بل;
    }طالما( p );
  }
exit_triggers_exist:
  إذا( p_ماسک ){
    *p_ماسک = ماسک;
  }
  أرجع (ماسک ? pList : 0);
}
Trigger *sqlite3TriggersExist(
  Parse *pParse,          /* Parse context */
  جدول *pTab,            /* The table the contains the triggers */
  صحيح op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  بیان_List *pChanges,     /* Columns that change in an UPDATE statement */
  صحيح *p_ماسک              /* OUT: Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
){
  أكد( pTab!=0 );
  إذا( (pTab->pTrigger==0 && !لنډمهاله_Triggers_Exist(pParse->db))
   || pParse->disableTriggers
  ){
    إذا( p_ماسک ) *p_ماسک = 0;
    أرجع 0;
  }
  أرجع triggersReallyExist(pParse,pTab,op,pChanges,p_ماسک);
}

/*
** Return true if the pExpr term from the RETURNING clause argument
** list is of the form "*".  Raise an error if the terms if of the
** form "table.*".
*/
ساكن صحيح is_Asterisk_غړی(
  Parse *pParse,      /* Parsing context */
  بیان *p_غړی         /* A term in the RETURNING clause */
){
  أكد( p_غړی!=0 );
  إذا( p_غړی->op==TK_ASTERISK ) أرجع 1;
  إذا( p_غړی->op!=TK_DOT ) أرجع 0;
  أكد( p_غړی->p_ښی!=0 );
  أكد( p_غړی->p_کیڼ!=0 );
  إذا( p_غړی->p_ښی->op!=TK_ASTERISK ) أرجع 0;
  sqlite3_Error_پیغام(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  أرجع 1;
}

/* The input list pList is the list of result set terms from a RETURNING
** clause.  The table that we are returning from is pTab.
**
** This routine makes a copy of the pList, and at the same time expands
** any "*" wildcards to be the complete set of columns from pTab.
*/
ساكن بیان_List *sqlite3ExpandReturning(
  Parse *pParse,        /* Parsing context */
  بیان_List *pList,      /* The arguments to RETURNING */
  جدول *pTab           /* The table being updated */
){
  بیان_List *pNew = 0;
  sqlite3 *db = pParse->db;
  صحيح i;

  لكل(i=0; i<pList->n_بیان; i++){
    بیان *p_Old_بیان = pList->a[i].p_بیان;
    إذا( NEVER(p_Old_بیان==0) ) استمر;
    إذا( is_Asterisk_غړی(pParse, p_Old_بیان) ){
      صحيح jj;
      لكل(jj=0; jj<pTab->n_ستون; jj++){
        بیان *p_New_بیان;
        إذا( Is_Hidden_ستنه(pTab->a_ستون+jj) ) استمر;
        p_New_بیان = sqlite3_بیان(db, TK_ID, pTab->a_ستون[jj].zCnName);
        pNew = sqlite3_بیان_List_Append(pParse, pNew, p_New_بیان);
        إذا( !db->mallocFailed ){
          بنية بیان_List_توکی *p_توکی = &pNew->a[pNew->n_بیان-1];
          p_توکی->zEName = sqlite3DbStrDup(db, pTab->a_ستون[jj].zCnName);
          p_توکی->fg.eEName = ENAME_NAME;
        }
      }
    }وإلا{
      بیان *p_New_بیان = sqlite3_بیان_Dup(db, p_Old_بیان, 0);
      pNew = sqlite3_بیان_List_Append(pParse, pNew, p_New_بیان);
      إذا( !db->mallocFailed && ALWAYS(pList->a[i].zEName!=0) ){
        بنية بیان_List_توکی *p_توکی = &pNew->a[pNew->n_بیان-1];
        p_توکی->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        p_توکی->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  أرجع pNew;
}

/* If the Expr node is a subquery or an EXISTS operator or an IN operator that
** uses a subquery, and if the subquery is SF_Correlated, then mark the
** expression as EP_VarSelect.
*/
ساكن صحيح sqlite3ReturningSubqueryVarSelect(Walker *Not_کارول_شوی, بیان *p_بیان){
  ناکارول_شوی_PARAMETER(Not_کارول_شوی);
  إذا( بیان_Use_XSelect(p_بیان)
   && (p_بیان->x.pSelect->selFlags & SF_Correlated)!=0
  ){
    testcase( بیان_Has_Property(p_بیان, EP_VarSelect) );
    بیان_سېټ_Property(p_بیان, EP_VarSelect);
  }
  أرجع WRC_Continue;
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
  لكل(i=0; i<pSrc->nSrc; i++){
    إذا( pSrc->a[i].pSTab==pWalker->u.pTab ){
      testcase( pSelect->selFlags & SF_Correlated );
      pSelect->selFlags |= SF_Correlated;
      pWalker->e_کوډ = 1;
      اكسر;
    }
  }
  أرجع WRC_Continue;
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
  بیان_List *pEList,
  جدول *pTab
){
  Walker w;
  memset(&w, 0, حجم(w));
  w.x_بیان_Callback = sqlite3_بیان_Walk_Noop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3_Walk_بیان_List(&w, pEList);
  إذا( w.e_کوډ ){
    w.x_بیان_Callback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3_Walk_بیان_List(&w, pEList);
  }
}

/*
** Generate code for the RETURNING trigger.  Unlike other triggers
** that invoke a subprogram in the bytecode, the code for RETURNING
** is generated in-line.
*/
ساكن فراغ کوډ_Returning_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* The trigger step that defines the RETURNING */
  جدول *pTab,         /* The table to code triggers from */
  صحيح reg_دننه            /* The first in an array of registers */
){
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  بیان_List *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  اتحاد {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;

  أكد( v!=0 );
  إذا( !pParse->bReturning ){
    /* This RETURNING trigger must be for a different statement as
    ** this statement lacks a RETURNING clause. */
    أرجع;
  }
  أكد( db->pParse==pParse );
  أكد( !pParse->isCreate );
  pReturning = pParse->u1.d.pReturning;
  إذا( pTrigger != &(pReturning->retTrig) ){
    /* This RETURNING trigger is for a different statement */
    أرجع;
  }
  memset(&sSelect, 0, حجم(sSelect));
  memset(&uSrc, 0, حجم(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3_بیان_List_Dup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName; /* tag-20240424-1 */
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  إذا( pParse->nErr==0 ){
    أكد( db->mallocFailed==0 );
    sqlite3_Generate_ستنه_نومونه(pParse, &sSelect);
  }
  sqlite3_بیان_List_Delete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  إذا( pParse->nErr==0 ){
    NameContext sNC;
    memset(&sNC, 0, حجم(sNC));
    إذا( pReturning->n_Ret_ستون==0 ){
      pReturning->n_Ret_ستون = pNew->n_بیان;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.i_بنسټ_Reg = reg_دننه;
    sNC.ncFlags = NC_UBaseReg;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    إذا( sqlite3_Resolve_بیان_List_نومونه(&sNC, pNew)==SQLITE_OK
     && ALWAYS(!db->mallocFailed)
    ){
      صحيح i;
      صحيح n_ستون = pNew->n_بیان;
      صحيح reg = pParse->nMem+1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += n_ستون+2;
      pReturning->iRetReg = reg;
      لكل(i=0; i<n_ستون; i++){
        بیان *p_ستون = pNew->a[i].p_بیان;
        أكد( p_ستون!=0 ); /* Due to !db->mallocFailed ~9 lines above */
        sqlite3_بیان_کوډ_Factorable(pParse, p_ستون, reg+i);
        إذا( sqlite3_بیان_Affinity(p_ستون)==SQLITE_AFF_REAL ){
          sqlite3_Vdbe_جمع_کړه_Op1(v, OP_RealAffinity, reg+i);
        }
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg, i, reg+i);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, pReturning->iRetCur, reg+i+1);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, pReturning->iRetCur, reg+i, reg+i+1);
    }
  }
  sqlite3_بیان_List_Delete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}



/*
** Generate VDBE code for the statements inside the body of a single 
** trigger.
*/
ساكن صحيح کوډ_Trigger_پروګرام(
  Parse *pParse,            /* The parser context */
  Trigger_ګام *p_ګام_List,   /* List of statements inside the trigger body */
  صحيح orconf                /* Conflict algorithm. (OE_Abort, etc) */  
){
  Trigger_ګام *p_ګام;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  أكد( pParse->pTriggerTab && pParse->pToplevel );
  أكد( p_ګام_List );
  أكد( v!=0 );
  لكل(p_ګام=p_ګام_List; p_ګام; p_ګام=p_ګام->p_بل){
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
    pParse->eOrconf = (orconf==OE_Default)?p_ګام->orconf:(u8)orconf;
    أكد( pParse->ok_Const_ضریب==0 );

#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
    إذا( p_ګام->z_پراختیا ){
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Trace, 0x7fffffff, 1, 0,
                        sqlite3MPrintf(db, "-- %s", p_ګام->z_پراختیا),
                        P4_DYNAMIC);
    }
#د_که_پای

    بدل( p_ګام->op ){
      حالة TK_UPDATE: {
        sqlite3Update(pParse, 
          sqlite3SrcListDup(db, p_ګام->pSrc, 0),
          sqlite3_بیان_List_Dup(db, p_ګام->p_بیان_List, 0), 
          sqlite3_بیان_Dup(db, p_ګام->pWhere, 0), 
          pParse->eOrconf, 0, 0, 0
        );
        sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Reset_شمېر);
        اكسر;
      }
      حالة TK_ورننباسه: {
        sqlite3_ورننباسه(pParse, 
          sqlite3SrcListDup(db, p_ګام->pSrc, 0),
          sqlite3SelectDup(db, p_ګام->pSelect, 0), 
          sqlite3IdListDup(db, p_ګام->pIdList), 
          pParse->eOrconf,
          sqlite3UpsertDup(db, p_ګام->pUpsert)
        );
        sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Reset_شمېر);
        اكسر;
      }
      حالة TK_DELETE: {
        sqlite3DeleteFrom(pParse, 
          sqlite3SrcListDup(db, p_ګام->pSrc, 0),
          sqlite3_بیان_Dup(db, p_ګام->pWhere, 0), 0, 0
        );
        sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Reset_شمېر);
        اكسر;
      }
      افتراضي: أكد( p_ګام->op==TK_SELECT ); {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, p_ګام->pSelect, 0);
        sqlite3SelectDestInit(&sDest, SRT_Discard, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        اكسر;
      }
    } 
  }

  أرجع 0;
}

#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** This function is used to add VdbeComment() annotations to a VDBE
** program. It is not used in production code, only for debugging.
*/
ساكن ثابت حرف *on_Error_متن(صحيح onError){
  بدل( onError ){
    حالة OE_Abort:    أرجع "abort";
    حالة OE_Rollback: أرجع "rollback";
    حالة OE_Fail:     أرجع "fail";
    حالة OE_Replace:  أرجع "replace";
    حالة OE_Ignore:   أرجع "ignore";
    حالة OE_Default:  أرجع "default";
  }
  أرجع "n/a";
}
#د_که_پای

/*
** Parse context structure pFrom has just been used to create a sub-vdbe
** (trigger program). If an error has occurred, transfer error information
** from pFrom to pTo.
*/
ساكن فراغ transferParseError(Parse *p_منزل, Parse *pFrom){
  أكد( pFrom->z_Err_پیغام==0 || pFrom->nErr );
  أكد( p_منزل->z_Err_پیغام==0 || p_منزل->nErr );
  إذا( p_منزل->nErr==0 ){
    p_منزل->z_Err_پیغام = pFrom->z_Err_پیغام;
    p_منزل->nErr = pFrom->nErr;
    p_منزل->rc = pFrom->rc;
  }وإلا{
    sqlite3DbFree(pFrom->db, pFrom->z_Err_پیغام);
  }
}

/*
** Create and populate a new TriggerPrg object with a sub-program 
** implementing trigger pTrigger with ON CONFLICT policy orconf.
*/
ساكن TriggerPrg *کوډ_لیکه_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  جدول *pTab,         /* The table pTrigger is attached to */
  صحيح orconf           /* ON CONFLICT policy to code trigger program with */
){
  Parse *p_سر_2;                /* Top level Parse object */
  sqlite3 *db = pParse->db;   /* Database handle */
  TriggerPrg *pPrg;           /* Value to return */
  بیان *pWhen = 0;            /* Duplicate of trigger WHEN expression */
  Vdbe *v;                    /* Temporary VM */
  NameContext sNC;            /* Name context for sub-vdbe */
  Sub_پروګرام *p_پروګرام = 0;   /* Sub-vdbe for trigger program */
  صحيح i_پای_ټکی_Trigger = 0;        /* Label to jump to if WHEN is false */
  Parse sSubParse;            /* Parse context for sub-vdbe */
  صحيح n_ژوروالی;                 /* Trigger depth */

  /* Ensure that triggers are not chained too deep.  This test is linear
  ** in the chaining depth, but sensible code ought not be chaining
  ** triggers excessively, so that shouldn't be a problem.
  */  
  p_سر_2 = pParse;
  لكل(n_ژوروالی=0; p_سر_2->pOuterParse; p_سر_2 = p_سر_2->pOuterParse, n_ژوروالی++){}
  إذا( n_ژوروالی>=db->a_حد[SQLITE_حد_TRIGGER_ژوروالی] ){
    sqlite3_Error_پیغام(pParse, "triggers nested too deep");
    أرجع 0;
  }

  p_سر_2 = sqlite3ParseToplevel(pParse);
  أكد( pTrigger->zName==0 || pTab==جدول_د_Trigger(pTrigger) );
  أكد( p_سر_2->pVdbe );


  /* Allocate the TriggerPrg and SubProgram objects. To ensure that they
  ** are freed if an error occurs, link them into the Parse.pTriggerPrg 
  ** list of the top-level Parse object sooner rather than later.  */
  pPrg = sqlite3DbMallocZero(db, حجم(TriggerPrg));
  إذا( !pPrg ) أرجع 0;
  pPrg->p_بل = p_سر_2->pTriggerPrg;
  p_سر_2->pTriggerPrg = pPrg;
  pPrg->p_پروګرام = p_پروګرام = sqlite3DbMallocZero(db, حجم(Sub_پروګرام));
  إذا( !p_پروګرام ) أرجع 0;
  sqlite3_Vdbe_Link_Sub_پروګرام(p_سر_2->pVdbe, p_پروګرام);
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
  sSubParse.pToplevel = p_سر_2;
  sSubParse.pTrigCoded = pTrigger;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  إذا( v ){
    VdbeComment((v, "Start: %s.%s (%s %s%s%s ON %s)", 
      pTrigger->zName, on_Error_متن(orconf),
      (pTrigger->tr_tm==TRIGGER_BEFORE ? "BEFORE" : "AFTER"),
        (pTrigger->op==TK_UPDATE ? "UPDATE" : ""),
        (pTrigger->op==TK_ورننباسه ? "INSERT" : ""),
        (pTrigger->op==TK_DELETE ? "DELETE" : ""),
      pTab->zName
    ));
#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
    إذا( pTrigger->zName ){
      sqlite3VdbeChangeP4(v, -1, 
        sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), P4_DYNAMIC
      );
    }
#د_که_پای

    /* If one was specified, code the WHEN clause. If it evaluates to false
    ** (or NULL) the sub-vdbe is immediately halted by jumping to the 
    ** OP_Halt inserted at the end of the program.  */
    إذا( pTrigger->pWhen ){
      pWhen = sqlite3_بیان_Dup(db, pTrigger->pWhen, 0);
      إذا( db->mallocFailed==0
       && SQLITE_OK==sqlite3_Resolve_بیان_نومونه(&sNC, pWhen) 
      ){
        i_پای_ټکی_Trigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3_بیان_If_False(&sSubParse, pWhen, i_پای_ټکی_Trigger, SQLITE_JUMPIFNULL);
      }
      sqlite3_بیان_Delete(db, pWhen);
    }

    /* Code the trigger program into the sub-vdbe. */
    کوډ_Trigger_پروګرام(&sSubParse, pTrigger->ګام_list, orconf);

    /* Insert an OP_Halt at the end of the sub-program. */
    إذا( i_پای_ټکی_Trigger ){
      sqlite3VdbeResolveLabel(v, i_پای_ټکی_Trigger);
    }
    sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Halt);
    VdbeComment((v, "End: %s.%s", pTrigger->zName, on_Error_متن(orconf)));
    transferParseError(pParse, &sSubParse);

    إذا( pParse->nErr==0 ){
      أكد( db->mallocFailed==0 );
      p_پروګرام->aOp = sqlite3VdbeTakeOpArray(v, &p_پروګرام->nOp, &p_سر_2->nMaxArg);
    }
    p_پروګرام->nMem = sSubParse.nMem;
    p_پروګرام->nCsr = sSubParse.nTab;
    p_پروګرام->token = (فراغ *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  }وإلا{
    transferParseError(pParse, &sSubParse);
  }

  أكد( !sSubParse.pTriggerPrg && !sSubParse.nMaxArg );
  sqlite3ParseObjectReset(&sSubParse);
  أرجع pPrg;
}
    
/*
** Return a pointer to a TriggerPrg object containing the sub-program for
** trigger pTrigger with default ON CONFLICT algorithm orconf. If no such
** TriggerPrg object exists, a new object is allocated and populated before
** being returned.
*/
ساكن TriggerPrg *get_لیکه_Trigger(
  Parse *pParse,       /* Current parse context */
  Trigger *pTrigger,   /* Trigger to code */
  جدول *pTab,         /* The table trigger pTrigger is attached to */
  صحيح orconf           /* ON CONFLICT algorithm. */
){
  Parse *p_ریښه = sqlite3ParseToplevel(pParse);
  TriggerPrg *pPrg;

  أكد( pTrigger->zName==0 || pTab==جدول_د_Trigger(pTrigger) );

  /* It may be that this trigger has already been coded (or is in the
  ** process of being coded). If this is the case, then an entry with
  ** a matching TriggerPrg.pTrigger field will be present somewhere
  ** in the Parse.pTriggerPrg list. Search for such an entry.  */
  لكل(pPrg=p_ریښه->pTriggerPrg; 
      pPrg && (pPrg->pTrigger!=pTrigger || pPrg->orconf!=orconf); 
      pPrg=pPrg->p_بل
  );

  /* If an existing TriggerPrg could not be located, create a new one. */
  إذا( !pPrg ){
    pPrg = کوډ_لیکه_Trigger(pParse, pTrigger, pTab, orconf);
    pParse->db->err_Byte_دننه_واټن = -1;
  }

  أرجع pPrg;
}

/*
** Generate code for the trigger program associated with trigger p on 
** table pTab. The reg, orconf and ignoreJump parameters passed to this
** function are the same as those described in the header function for
** sqlite3CodeRowTrigger()
*/
فراغ sqlite3_کوډ_لیکه_Trigger_مستقیم(
  Parse *pParse,       /* Parse context */
  Trigger *p,          /* Trigger to code */
  جدول *pTab,         /* The table to code triggers from */
  صحيح reg,             /* Reg array containing OLD.* and NEW.* values */
  صحيح orconf,          /* ON CONFLICT policy */
  صحيح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Vdbe *v = sqlite3GetVdbe(pParse); /* Main VM */
  TriggerPrg *pPrg;
  pPrg = get_لیکه_Trigger(pParse, p, pTab, orconf);
  أكد( pPrg || pParse->nErr );

  /* Code the OP_Program opcode in the parent VDBE. P4 of the OP_Program 
  ** is a pointer to the sub-vdbe containing the trigger program.  */
  إذا( pPrg ){
    صحيح bRecursive = (p->zName && 0==(pParse->db->flags&SQLITE_RecTriggers));

    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_پروګرام, reg, ignoreJump, ++pParse->nMem,
                      (ثابت حرف *)pPrg->p_پروګرام, P4_SUBPROGRAM);
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
فراغ sqlite3_کوډ_لیکه_Trigger(
  Parse *pParse,       /* Parse context */
  Trigger *pTrigger,   /* List of triggers on table pTab */
  صحيح op,              /* One of TK_UPDATE, TK_INSERT, TK_DELETE */
  بیان_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحيح tr_tm,           /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  جدول *pTab,         /* The table to code triggers from */
  صحيح reg,             /* The first in an array of registers (see above) */
  صحيح orconf,          /* ON CONFLICT policy */
  صحيح ignoreJump       /* Instruction to jump to for RAISE(IGNORE) */
){
  Trigger *p;          /* Used to iterate through pTrigger list */

  أكد( op==TK_UPDATE || op==TK_ورننباسه || op==TK_DELETE );
  أكد( tr_tm==TRIGGER_BEFORE || tr_tm==TRIGGER_AFTER );
  أكد( (op==TK_UPDATE)==(pChanges!=0) );

  لكل(p=pTrigger; p; p=p->p_بل){

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
    إذا( (p->op==op || (p->bReturning && p->op==TK_ورننباسه && op==TK_UPDATE))
     && p->tr_tm==tr_tm 
     && check_ستنه_Overlap(p->pColumns, pChanges)
    ){
      إذا( !p->bReturning ){
        sqlite3_کوډ_لیکه_Trigger_مستقیم(pParse, p, pTab, reg, orconf, ignoreJump);
      }وإلا إذا( sqlite3IsToplevel(pParse) ){
        کوډ_Returning_Trigger(pParse, p, pTab, reg);
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
  بیان_List *pChanges,  /* Changes list for any UPDATE OF triggers */
  صحيح isNew,           /* 1 for new.* ref mask, 0 for old.* ref mask */
  صحيح tr_tm,           /* Mask of TRIGGER_BEFORE|TRIGGER_AFTER */
  جدول *pTab,         /* The table to code triggers from */
  صحيح orconf           /* Default ON CONFLICT policy for trigger steps */
){
  ثابت صحيح op = pChanges ? TK_UPDATE : TK_DELETE;
  u32 ماسک = 0;
  Trigger *p;

  أكد( isNew==1 || isNew==0 );
  إذا( IsView(pTab) ){
    أرجع 0xffffffff;
  }
  لكل(p=pTrigger; p; p=p->p_بل){
    إذا( p->op==op
     && (tr_tm&p->tr_tm)
     && check_ستنه_Overlap(p->pColumns,pChanges)
    ){
      إذا( p->bReturning ){
        ماسک = 0xffffffff;
      }وإلا{
        TriggerPrg *pPrg;
        pPrg = get_لیکه_Trigger(pParse, p, pTab, orconf);
        إذا( pPrg ){
          ماسک |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  أرجع ماسک;
}

#د_که_پای /* !defined(SQLITE_OMIT_TRIGGER) */
