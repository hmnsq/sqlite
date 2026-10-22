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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଯଦି defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#ଅନ୍ତର୍ଭାଵ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା bytecodevtab bytecodevtab;
ସଂରଚନା bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା bytecodevtab_cursor bytecodevtab_cursor;
ସଂରଚନା bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRowid;                /* The rowid of the output table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAddr;                 /* Address */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା needFinalize;          /* Cursors owns pStmt and must finalize it */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  ଅକ୍ଷର *zP4;                 /* Rendered P4 value */
  ଅଚଳ ଅକ୍ଷର *zType;         /* tables_used.type */
  ଅଚଳ ଅକ୍ଷର *zSchema;       /* tables_used.schema */
  ଅଚଳ ଅକ୍ଷର *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  bytecodevtab *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isTabUsed = pAux!=0;
  ଅଚଳ ଅକ୍ଷର *azSchema[2] = {
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

  (ଶୂନ୍ୟ)argc;
  (ଶୂନ୍ୟ)argv;
  (ଶୂନ୍ୟ)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  ଯଦି( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( ଆକାର(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ଯଦି( pNew==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    memset(pNew, 0, ଆକାର(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( ଆକାର(*pCur) );
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCur, 0, ଆକାର(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ସ୍ଥିର ଶୂନ୍ୟ bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  ଯଦି( pCur->needFinalize ){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ଫେରନ୍ତୁ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ଯଦି( pCur->zName ){
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
  ଯଦି( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ଫେରନ୍ତୁ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ଯଦି( pVTab->bTablesUsed ){
    ଯଦି( i==4 ){
      i = 8;
    }ଅନ୍ୟଥା{
      ଯଦି( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = pOp->p3;
        Pgno i_ମୂଳ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        ପାଇଁ(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          ଯଦି( !IsVirtual(pTab) && pTab->tnum==i_ମୂଳ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ଭାଙ୍ଗ;
          }
        }
        ଯଦି( pCur->zName==0 ){
          ପାଇଁ(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            ଯଦି( pIdx->tnum==i_ମୂଳ ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ବାଛ( i ){
    ସ୍ଥିତି 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 1:   /* opcode */
      sqlite3_result_text(ctx, (ଅକ୍ଷର*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 5:   /* p4 */
    ସ୍ଥିତି 7:   /* comment */
      ଯଦି( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ଯଦି( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ଅନ୍ୟଥା{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        ଅକ୍ଷର *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ପ୍ରତିଜ୍ଞା( aOp[0].opcode==OP_Init );
      ପ୍ରତିଜ୍ଞା( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      ଯଦି( pCur->iRowid==pCur->iAddr+1 ){
        ଭାଙ୍ଗ;  /* Result is NULL for the main program */
      }ଅନ୍ୟଥା ଯଦି( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ଅନ୍ୟଥା{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ଭାଙ୍ଗ;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    ସ୍ଥିତି 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      ଭାଙ୍ଗ;
#ଅନ୍ୟଥା
    ସ୍ଥିତି 9:     /* nexec */
    ସ୍ଥିତି 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      ଭାଙ୍ଗ;
#endif

    ସ୍ଥିତି 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  (ଶୂନ୍ୟ)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_ଅଙ୍କ==0;
  ପ୍ରତିଜ୍ଞା( argc==1 );
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_TEXT ){
    ଅଚଳ ଅକ୍ଷର *zSql = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    ଯଦି( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ଅନ୍ୟଥା{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ଅନ୍ୟଥା{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_ମୂଲ୍ୟ_pointer(argv[0],"stmt-pointer");
  }
  ଯଦି( pCur->pStmt==0 ){
    pVTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    bytecodevtabNext(pVtabCursor);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_CONSTRAINT;
  ସଂରଚନା sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (ଦ୍ୱିଗୁଣ)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_ଅଙ୍କ = 0;
  ପାଇଁ(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ଯଦି( p->usable==0 ) ଜାରି;
    ଯଦି( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    ଯଦି( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ଅଙ୍କ = 1;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ସ୍ଥିର sqlite3_module bytecodevtabModule = {
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


ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ଫେରନ୍ତୁ rc;
}
#ନଚେତଯଦି defined(SQLITE_ENABLE_BYTECODE_VTAB)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ଫେରନ୍ତୁ SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
