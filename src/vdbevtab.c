/*
** 2020-03-23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements virtual-tables for examining the bytecode content
** of a prepared statement.
*/
#অন্তর্ভাৱ "sqliteInt.h"
#যদি defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#অন্তর্ভাৱ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
প্রকারনির্দেশ সংরচনা bytecodevtab bytecodevtab;
সংরচনা bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  পূর্ণাঙ্ক bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
প্রকারনির্দেশ সংরচনা bytecodevtab_cursor bytecodevtab_cursor;
সংরচনা bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  পূর্ণাঙ্ক iRowid;                /* The rowid of the output table */
  পূর্ণাঙ্ক iAddr;                 /* Address */
  পূর্ণাঙ্ক needFinalize;          /* Cursors owns pStmt and must finalize it */
  পূর্ণাঙ্ক showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  ৱর্ণ *zP4;                 /* Rendered P4 value */
  ধ্রুৱ ৱর্ণ *zType;         /* tables_used.type */
  ধ্রুৱ ৱর্ণ *zSchema;       /* tables_used.schema */
  ধ্রুৱ ৱর্ণ *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
স্থির পূর্ণাঙ্ক bytecodevtabConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ*argv,
  sqlite3_vtab **ppVtab,
  ৱর্ণ **pzErr
){
  bytecodevtab *pNew;
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক isTabUsed = pAux!=0;
  ধ্রুৱ ৱর্ণ *azSchema[2] = {
    /* bytecode() schema */
    "CREATE TABLE x("
      "addr INT,"
      "opcode TEXT,"
      "p1 INT,"
      "p2 INT,"
      "p3 INT,"
      "p4 TEXT,"
      "p5 INT,"
      "comment TEXT,"
      "subprog TEXT," 
      "nexec INT,"
      "ncycle INT,"
      "stmt HIDDEN"
    ");",

    /* Tables_used() schema */
    "CREATE TABLE x("
      "type TEXT,"
      "schema TEXT,"
      "name TEXT,"
      "wr INT,"
      "subprog TEXT," 
      "stmt HIDDEN"
   ");"
  };

  (শূন্য)argc;
  (শূন্য)argv;
  (শূন্য)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  যদি( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( পরিমাণ(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    যদি( pNew==0 ) ঘূৰাই SQLITE_NOMEM;
    memset(pNew, 0, পরিমাণ(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  ঘূৰাই rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
স্থির পূর্ণাঙ্ক bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ঘূৰাই SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
স্থির পূর্ণাঙ্ক bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( পরিমাণ(*pCur) );
  যদি( pCur==0 ) ঘূৰাই SQLITE_NOMEM;
  memset(pCur, 0, পরিমাণ(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  ঘূৰাই SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
স্থির শূন্য bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  যদি( pCur->needFinalize ){
    sqlite3_finalize(pCur->pStmt);
  }
  pCur->pStmt = 0;
  pCur->needFinalize = 0;
  pCur->zType = 0;
  pCur->zSchema = 0;
  pCur->zName = 0;
}

/*
** Destructor for a bytecodevtab_cursor.
*/
স্থির পূর্ণাঙ্ক bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ঘূৰাই SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
স্থির পূর্ণাঙ্ক bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  পূর্ণাঙ্ক rc;
  যদি( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  যদি( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3VdbeNextOpcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->bTablesUsed,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  যদি( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
স্থির পূর্ণাঙ্ক bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ঘূৰাই pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
স্থির পূর্ণাঙ্ক bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  পূর্ণাঙ্ক i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  যদি( pVTab->bTablesUsed ){
    যদি( i==4 ){
      i = 8;
    }অন্যথা{
      যদি( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        পূর্ণাঙ্ক iDb = pOp->p3;
        Pgno i_মূল = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        বাবে(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          যদি( !IsVirtual(pTab) && pTab->tnum==i_মূল ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ভঙ্গ;
          }
        }
        যদি( pCur->zName==0 ){
          বাবে(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            যদি( pIdx->tnum==i_মূল ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ৱিকল্প( i ){
    পক্ষ 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      ভঙ্গ;
    পক্ষ 1:   /* opcode */
      sqlite3_result_text(ctx, (ৱর্ণ*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ভঙ্গ;
    পক্ষ 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      ভঙ্গ;
    পক্ষ 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      ভঙ্গ;
    পক্ষ 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      ভঙ্গ;
    পক্ষ 5:   /* p4 */
    পক্ষ 7:   /* comment */
      যদি( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      যদি( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }অন্যথা{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        ৱর্ণ *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      ভঙ্গ;
    পক্ষ 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      ভঙ্গ;
    পক্ষ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      প্রতিজ্ঞা( aOp[0].opcode==OP_Init );
      প্রতিজ্ঞা( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      যদি( pCur->iRowid==pCur->iAddr+1 ){
        ভঙ্গ;  /* Result is NULL for the main program */
      }অন্যথা যদি( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }অন্যথা{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ভঙ্গ;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    পক্ষ 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      ভঙ্গ;
    পক্ষ 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      ভঙ্গ;
#অন্যথা
    পক্ষ 9:     /* nexec */
    পক্ষ 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      ভঙ্গ;
#endif

    পক্ষ 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      ভঙ্গ;
    পক্ষ 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ভঙ্গ;
    পক্ষ 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      ভঙ্গ;
    পক্ষ 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      ভঙ্গ;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
স্থির পূর্ণাঙ্ক bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ঘূৰাই SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
স্থির পূর্ণাঙ্ক bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুৱ ৱর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  (শূন্য)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_অঙ্ক==0;
  প্রতিজ্ঞা( argc==1 );
  যদি( sqlite3_মান_type(argv[0])==SQLITE_TEXT ){
    ধ্রুৱ ৱর্ণ *zSql = (ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(argv[0]);
    যদি( zSql==0 ){
      rc = SQLITE_NOMEM;
    }অন্যথা{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }অন্যথা{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_মান_pointer(argv[0],"stmt-pointer");
  }
  যদি( pCur->pStmt==0 ){
    pVTab->base.z_Err_বাৰ্তা = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }অন্যথা{
    bytecodevtabNext(pVtabCursor);
  }
  ঘূৰাই rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
স্থির পূর্ণাঙ্ক bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc = SQLITE_CONSTRAINT;
  সংরচনা sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  পূর্ণাঙ্ক iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (দ্ৱিগুণ)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_অঙ্ক = 0;
  বাবে(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    যদি( p->usable==0 ) অনুৱর্ত;
    যদি( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    যদি( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_অঙ্ক = 1;
    }
  }
  ঘূৰাই rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
স্থির sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtabBestIndex,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtabNext,
  /* xEof        */ bytecodevtabEof,
  /* xColumn     */ bytecodevtabColumn,
  /* xRowid      */ bytecodevtabRowid,
  /* xUpdate     */ 0,
  /* xBegin      */ 0,
  /* xSync       */ 0,
  /* xCommit     */ 0,
  /* xRollback   */ 0,
  /* xFindMethod */ 0,
  /* xRename     */ 0,
  /* xSavepoint  */ 0,
  /* xRelease    */ 0,
  /* xRollbackTo */ 0,
  /* xShadowName */ 0,
  /* xIntegrity  */ 0
};


পূর্ণাঙ্ক sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  পূর্ণাঙ্ক rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ঘূৰাই rc;
}
#নোচেদ্যদি defined(SQLITE_ENABLE_BYTECODE_VTAB)
পূর্ণাঙ্ক sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ঘূৰাই SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
