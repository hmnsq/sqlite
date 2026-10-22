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
#અન્તર્ભાવ "sqliteInt.h"
#જો defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#અન્તર્ભાવ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
પ્રકારવ્યાખ્યા સંરચના bytecodevtab bytecodevtab;
સંરચના bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  પૂર્ણાંક bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
પ્રકારવ્યાખ્યા સંરચના bytecodevtab_cursor bytecodevtab_cursor;
સંરચના bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  પૂર્ણાંક iRowid;                /* The rowid of the output table */
  પૂર્ણાંક iAddr;                 /* Address */
  પૂર્ણાંક needFinalize;          /* Cursors owns pStmt and must finalize it */
  પૂર્ણાંક showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  અક્ષર *zP4;                 /* Rendered P4 value */
  અચલ અક્ષર *zType;         /* tables_used.type */
  અચલ અક્ષર *zSchema;       /* tables_used.schema */
  અચલ અક્ષર *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
સ્થિર પૂર્ણાંક bytecodevtabConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  bytecodevtab *pNew;
  પૂર્ણાંક rc;
  પૂર્ણાંક isTabUsed = pAux!=0;
  અચલ અક્ષર *azSchema[2] = {
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

  (શૂન્ય)argc;
  (શૂન્ય)argv;
  (શૂન્ય)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  જો( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( કદ(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    જો( pNew==0 ) પાછા SQLITE_NOMEM;
    memset(pNew, 0, કદ(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  પાછા rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
સ્થિર પૂર્ણાંક bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  પાછા SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
સ્થિર પૂર્ણાંક bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( કદ(*pCur) );
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  memset(pCur, 0, કદ(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
સ્થિર શૂન્ય bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  જો( pCur->needFinalize ){
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
સ્થિર પૂર્ણાંક bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
સ્થિર પૂર્ણાંક bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  પૂર્ણાંક rc;
  જો( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  જો( pCur->zName ){
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
  જો( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  પાછા SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
સ્થિર પૂર્ણાંક bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  પાછા pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
સ્થિર પૂર્ણાંક bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  પૂર્ણાંક i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  જો( pVTab->bTablesUsed ){
    જો( i==4 ){
      i = 8;
    }નહીંતર{
      જો( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        પૂર્ણાંક iDb = pOp->p3;
        Pgno i_મૂળ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        માટે(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          જો( !IsVirtual(pTab) && pTab->tnum==i_મૂળ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            તોડો;
          }
        }
        જો( pCur->zName==0 ){
          માટે(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            જો( pIdx->tnum==i_મૂળ ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  પસંદગી( i ){
    સ્થિતિ 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      તોડો;
    સ્થિતિ 1:   /* opcode */
      sqlite3_result_text(ctx, (અક્ષર*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      તોડો;
    સ્થિતિ 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      તોડો;
    સ્થિતિ 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      તોડો;
    સ્થિતિ 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      તોડો;
    સ્થિતિ 5:   /* p4 */
    સ્થિતિ 7:   /* comment */
      જો( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      જો( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }નહીંતર{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        અક્ષર *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      તોડો;
    સ્થિતિ 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      તોડો;
    સ્થિતિ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      પ્રતિજ્ઞા( aOp[0].opcode==OP_Init );
      પ્રતિજ્ઞા( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      જો( pCur->iRowid==pCur->iAddr+1 ){
        તોડો;  /* Result is NULL for the main program */
      }નહીંતર જો( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }નહીંતર{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      તોડો;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    સ્થિતિ 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      તોડો;
    સ્થિતિ 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      તોડો;
#નહીંતર
    સ્થિતિ 9:     /* nexec */
    સ્થિતિ 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      તોડો;
#endif

    સ્થિતિ 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      તોડો;
    સ્થિતિ 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      તોડો;
    સ્થિતિ 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      તોડો;
    સ્થિતિ 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      તોડો;
  }
  પાછા SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
સ્થિર પૂર્ણાંક bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  પાછા SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
સ્થિર પૂર્ણાંક bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  પૂર્ણાંક rc = SQLITE_OK;
  (શૂન્ય)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_અંક==0;
  પ્રતિજ્ઞા( argc==1 );
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_TEXT ){
    અચલ અક્ષર *zSql = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    જો( zSql==0 ){
      rc = SQLITE_NOMEM;
    }નહીંતર{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }નહીંતર{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_કિંમત_pointer(argv[0],"stmt-pointer");
  }
  જો( pCur->pStmt==0 ){
    pVTab->base.z_Err_સંદેશ = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }નહીંતર{
    bytecodevtabNext(pVtabCursor);
  }
  પાછા rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
સ્થિર પૂર્ણાંક bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  પૂર્ણાંક i;
  પૂર્ણાંક rc = SQLITE_CONSTRAINT;
  સંરચના sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  પૂર્ણાંક iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (દ્વિગુણ)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_અંક = 0;
  માટે(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    જો( p->usable==0 ) ચાલુ;
    જો( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    જો( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_અંક = 1;
    }
  }
  પાછા rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
સ્થિર sqlite3_module bytecodevtabModule = {
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


પૂર્ણાંક sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  પૂર્ણાંક rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  જો( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  પાછા rc;
}
#અથવાજો defined(SQLITE_ENABLE_BYTECODE_VTAB)
પૂર્ણાંક sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ પાછા SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
