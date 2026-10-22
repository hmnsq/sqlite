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
#অন্তর্ভাব "sqliteInt.h"
#যদি defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#অন্তর্ভাব "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
প্রকারলেখ কাঠামো bytecodevtab bytecodevtab;
কাঠামো bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  পূর্ণসংখ্যা bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
প্রকারলেখ কাঠামো bytecodevtab_cursor bytecodevtab_cursor;
কাঠামো bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  পূর্ণসংখ্যা iRowid;                /* The rowid of the output table */
  পূর্ণসংখ্যা iAddr;                 /* Address */
  পূর্ণসংখ্যা needFinalize;          /* Cursors owns pStmt and must finalize it */
  পূর্ণসংখ্যা showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  অক্ষর *zP4;                 /* Rendered P4 value */
  ধ্রুবক অক্ষর *zType;         /* tables_used.type */
  ধ্রুবক অক্ষর *zSchema;       /* tables_used.schema */
  ধ্রুবক অক্ষর *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
স্থির পূর্ণসংখ্যা bytecodevtabConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  bytecodevtab *pNew;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা isTabUsed = pAux!=0;
  ধ্রুবক অক্ষর *azSchema[2] = {
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
    pNew = sqlite3_malloc( আকার(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    যদি( pNew==0 ) ফেরত SQLITE_NOMEM;
    memset(pNew, 0, আকার(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  ফেরত rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
স্থির পূর্ণসংখ্যা bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ফেরত SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
স্থির পূর্ণসংখ্যা bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( আকার(*pCur) );
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  memset(pCur, 0, আকার(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
স্থির পূর্ণসংখ্যা bytecodevtab_পরবর্তী(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  পূর্ণসংখ্যা rc;
  যদি( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  যদি( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_পরবর্তী_Opcode(
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
  ফেরত SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
স্থির পূর্ণসংখ্যা bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ফেরত pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
স্থির পূর্ণসংখ্যা bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  পূর্ণসংখ্যা i                       /* Which column to return */
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
        পূর্ণসংখ্যা iDb = pOp->p3;
        Pgno i_মূল = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        জন্য(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_পরবর্তী(k)){
          Table *pTab = (Table*)sqlite_Hash_তথ্য(k);
          যদি( !IsVirtual(pTab) && pTab->tnum==i_মূল ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            বিরতি;
          }
        }
        যদি( pCur->zName==0 ){
          জন্য(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqlite_Hash_পরবর্তী(k)){
            সূচক *pIdx = (সূচক*)sqlite_Hash_তথ্য(k);
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
  নির্বাচন( i ){
    ক্ষেত্র 0:   /* addr */
      sqlite3_ফলাফল_int(ctx, pCur->iAddr);
      বিরতি;
    ক্ষেত্র 1:   /* opcode */
      sqlite3_ফলাফল_text(ctx, (অক্ষর*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      বিরতি;
    ক্ষেত্র 2:   /* p1 */
      sqlite3_ফলাফল_int(ctx, pOp->p1);
      বিরতি;
    ক্ষেত্র 3:   /* p2 */
      sqlite3_ফলাফল_int(ctx, pOp->p2);
      বিরতি;
    ক্ষেত্র 4:   /* p3 */
      sqlite3_ফলাফল_int(ctx, pOp->p3);
      বিরতি;
    ক্ষেত্র 5:   /* p4 */
    ক্ষেত্র 7:   /* comment */
      যদি( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      যদি( i==5 ){
        sqlite3_ফলাফল_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }অন্যথা{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        অক্ষর *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_ফলাফল_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      বিরতি;
    ক্ষেত্র 6:     /* p5 */
      sqlite3_ফলাফল_int(ctx, pOp->p5);
      বিরতি;
    ক্ষেত্র 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      প্রতিজ্ঞা( aOp[0].opcode==OP_Init );
      প্রতিজ্ঞা( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      যদি( pCur->iRowid==pCur->iAddr+1 ){
        বিরতি;  /* Result is NULL for the main program */
      }অন্যথা যদি( aOp[0].p4.z!=0 ){
         sqlite3_ফলাফল_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }অন্যথা{
         sqlite3_ফলাফল_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      বিরতি;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    ক্ষেত্র 9:     /* nexec */
      sqlite3_ফলাফল_int64(ctx, pOp->nExec);
      বিরতি;
    ক্ষেত্র 10:    /* ncycle */
      sqlite3_ফলাফল_int64(ctx, pOp->nCycle);
      বিরতি;
#অন্যথা
    ক্ষেত্র 9:     /* nexec */
    ক্ষেত্র 10:    /* ncycle */
      sqlite3_ফলাফল_int(ctx, 0);
      বিরতি;
#endif

    ক্ষেত্র 20:  /* tables_used.type */
      sqlite3_ফলাফল_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      বিরতি;
    ক্ষেত্র 21:  /* tables_used.schema */
      sqlite3_ফলাফল_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      বিরতি;
    ক্ষেত্র 22:  /* tables_used.name */
      sqlite3_ফলাফল_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      বিরতি;
    ক্ষেত্র 23:  /* tables_used.wr */
      sqlite3_ফলাফল_int(ctx, pOp->opcode==OP_OpenWrite);
      বিরতি;
  }
  ফেরত SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
স্থির পূর্ণসংখ্যা bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ফেরত SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
স্থির পূর্ণসংখ্যা bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  (শূন্য)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_অঙ্ক==0;
  প্রতিজ্ঞা( argc==1 );
  যদি( sqlite3_মান_type(argv[0])==SQLITE_TEXT ){
    ধ্রুবক অক্ষর *zSql = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
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
    pVTab->base.z_Err_বার্তা = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }অন্যথা{
    bytecodevtab_পরবর্তী(pVtabCursor);
  }
  ফেরত rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
স্থির পূর্ণসংখ্যা bytecodevtab_Best_সূচক(
  sqlite3_vtab *tab,
  sqlite3_সূচক_info *pIdxInfo
){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc = SQLITE_CONSTRAINT;
  কাঠামো sqlite3_সূচক_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  পূর্ণসংখ্যা iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (দ্বিগুণ)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_অঙ্ক = 0;
  জন্য(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    যদি( p->usable==0 ) চালু;
    যদি( p->op==SQLITE_সূচক_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_সূচক = 1;
    }
    যদি( p->op==SQLITE_সূচক_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_অঙ্ক = 1;
    }
  }
  ফেরত rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
স্থির sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_Best_সূচক,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_পরবর্তী,
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


পূর্ণসংখ্যা sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  পূর্ণসংখ্যা rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ফেরত rc;
}
#নতুবা_যদি defined(SQLITE_ENABLE_BYTECODE_VTAB)
পূর্ণসংখ্যা sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ফেরত SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
