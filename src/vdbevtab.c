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
#அந்தர்பாவ "sqliteInt.h"
#எனில் defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#அந்தர்பாவ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
வகைவரை கட்டமைப்பு bytecodevtab bytecodevtab;
கட்டமைப்பு bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  முழு bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
வகைவரை கட்டமைப்பு bytecodevtab_cursor bytecodevtab_cursor;
கட்டமைப்பு bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  முழு iRowid;                /* The rowid of the output table */
  முழு iAddr;                 /* Address */
  முழு needFinalize;          /* Cursors owns pStmt and must finalize it */
  முழு showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  எழுத்து *zP4;                 /* Rendered P4 value */
  மாறிலி எழுத்து *zType;         /* tables_used.type */
  மாறிலி எழுத்து *zSchema;       /* tables_used.schema */
  மாறிலி எழுத்து *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
நிலையான முழு bytecodevtabConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  bytecodevtab *pNew;
  முழு rc;
  முழு isTabUsed = pAux!=0;
  மாறிலி எழுத்து *azSchema[2] = {
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

  (வெறுமை)argc;
  (வெறுமை)argv;
  (வெறுமை)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  எனில்( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( அளவு(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    எனில்( pNew==0 ) திரும்பு SQLITE_NOMEM;
    memset(pNew, 0, அளவு(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  திரும்பு rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
நிலையான முழு bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  திரும்பு SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
நிலையான முழு bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( அளவு(*pCur) );
  எனில்( pCur==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCur, 0, அளவு(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
நிலையான வெறுமை bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  எனில்( pCur->needFinalize ){
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
நிலையான முழு bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  திரும்பு SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
நிலையான முழு bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  முழு rc;
  எனில்( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  எனில்( pCur->zName ){
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
  எனில்( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  திரும்பு SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
நிலையான முழு bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  திரும்பு pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
நிலையான முழு bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  முழு i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  எனில்( pVTab->bTablesUsed ){
    எனில்( i==4 ){
      i = 8;
    }இல்லையெனில்{
      எனில்( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        முழு iDb = pOp->p3;
        Pgno i_வேர் = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        க்கு(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          எனில்( !IsVirtual(pTab) && pTab->tnum==i_வேர் ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            நிறுத்து;
          }
        }
        எனில்( pCur->zName==0 ){
          க்கு(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            எனில்( pIdx->tnum==i_வேர் ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  தேர்வு( i ){
    நிலை 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      நிறுத்து;
    நிலை 1:   /* opcode */
      sqlite3_result_text(ctx, (எழுத்து*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      நிறுத்து;
    நிலை 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      நிறுத்து;
    நிலை 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      நிறுத்து;
    நிலை 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      நிறுத்து;
    நிலை 5:   /* p4 */
    நிலை 7:   /* comment */
      எனில்( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      எனில்( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }இல்லையெனில்{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        எழுத்து *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      நிறுத்து;
    நிலை 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      நிறுத்து;
    நிலை 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ப்ரதிஜ்ஞா( aOp[0].opcode==OP_Init );
      ப்ரதிஜ்ஞா( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      எனில்( pCur->iRowid==pCur->iAddr+1 ){
        நிறுத்து;  /* Result is NULL for the main program */
      }இல்லையெனில் எனில்( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }இல்லையெனில்{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      நிறுத்து;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    நிலை 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      நிறுத்து;
    நிலை 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      நிறுத்து;
#இல்லையெனில்
    நிலை 9:     /* nexec */
    நிலை 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      நிறுத்து;
#endif

    நிலை 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      நிறுத்து;
    நிலை 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      நிறுத்து;
    நிலை 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      நிறுத்து;
    நிலை 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      நிறுத்து;
  }
  திரும்பு SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
நிலையான முழு bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  திரும்பு SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
நிலையான முழு bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  முழு rc = SQLITE_OK;
  (வெறுமை)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_இலக்கம்==0;
  ப்ரதிஜ்ஞா( argc==1 );
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_TEXT ){
    மாறிலி எழுத்து *zSql = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    எனில்( zSql==0 ){
      rc = SQLITE_NOMEM;
    }இல்லையெனில்{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }இல்லையெனில்{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_மதிப்பு_pointer(argv[0],"stmt-pointer");
  }
  எனில்( pCur->pStmt==0 ){
    pVTab->base.z_Err_செய்தி = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    bytecodevtabNext(pVtabCursor);
  }
  திரும்பு rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
நிலையான முழு bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  முழு i;
  முழு rc = SQLITE_CONSTRAINT;
  கட்டமைப்பு sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  முழு iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (இரட்டை)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_இலக்கம் = 0;
  க்கு(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    எனில்( p->usable==0 ) தொடர்;
    எனில்( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    எனில்( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_இலக்கம் = 1;
    }
  }
  திரும்பு rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
நிலையான sqlite3_module bytecodevtabModule = {
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


முழு sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  முழு rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  திரும்பு rc;
}
#அல்லதெனில் defined(SQLITE_ENABLE_BYTECODE_VTAB)
முழு sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ திரும்பு SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
