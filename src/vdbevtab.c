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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಯದಿ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ bytecodevtab bytecodevtab;
ಸಂರಚನಾ bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ bytecodevtab_cursor bytecodevtab_cursor;
ಸಂರಚನಾ bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  ಪೂರ್ಣಾಙ್ಕ iRowid;                /* The rowid of the output table */
  ಪೂರ್ಣಾಙ್ಕ iAddr;                 /* Address */
  ಪೂರ್ಣಾಙ್ಕ needFinalize;          /* Cursors owns pStmt and must finalize it */
  ಪೂರ್ಣಾಙ್ಕ showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  ವರ್ಣ *zP4;                 /* Rendered P4 value */
  ಧ್ರುವ ವರ್ಣ *zType;         /* tables_used.type */
  ಧ್ರುವ ವರ್ಣ *zSchema;       /* tables_used.schema */
  ಧ್ರುವ ವರ್ಣ *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  bytecodevtab *pNew;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ isTabUsed = pAux!=0;
  ಧ್ರುವ ವರ್ಣ *azSchema[2] = {
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

  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)argv;
  (ಶೂನ್ಯ)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  ಯದಿ( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( ಪರಿಮಾಣ(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    memset(pNew, 0, ಪರಿಮಾಣ(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( ಪರಿಮಾಣ(*pCur) );
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCur, 0, ಪರಿಮಾಣ(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ಸ್ಥಿರ ಶೂನ್ಯ bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  ಯದಿ( pCur->needFinalize ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ಯದಿ( pCur->zName ){
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
  ಯದಿ( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ಪ್ರತ್ಯಾಗಮ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಙ್ಕ i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ಯದಿ( pVTab->bTablesUsed ){
    ಯದಿ( i==4 ){
      i = 8;
    }ಅನ್ಯಥಾ{
      ಯದಿ( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        ಪೂರ್ಣಾಙ್ಕ iDb = pOp->p3;
        Pgno i_ಬೇರು = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        ಪ್ರತಿ(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          ಯದಿ( !IsVirtual(pTab) && pTab->tnum==i_ಬೇರು ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ಭಙ್ಗ;
          }
        }
        ಯದಿ( pCur->zName==0 ){
          ಪ್ರತಿ(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            ಯದಿ( pIdx->tnum==i_ಬೇರು ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      ಭಙ್ಗ;
    ಪಕ್ಷ 1:   /* opcode */
      sqlite3_result_text(ctx, (ವರ್ಣ*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    ಪಕ್ಷ 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      ಭಙ್ಗ;
    ಪಕ್ಷ 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      ಭಙ್ಗ;
    ಪಕ್ಷ 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      ಭಙ್ಗ;
    ಪಕ್ಷ 5:   /* p4 */
    ಪಕ್ಷ 7:   /* comment */
      ಯದಿ( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ಯದಿ( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ಅನ್ಯಥಾ{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        ವರ್ಣ *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      ಭಙ್ಗ;
    ಪಕ್ಷ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ಪ್ರತಿಜ್ಞಾ( aOp[0].opcode==OP_Init );
      ಪ್ರತಿಜ್ಞಾ( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      ಯದಿ( pCur->iRowid==pCur->iAddr+1 ){
        ಭಙ್ಗ;  /* Result is NULL for the main program */
      }ಅನ್ಯಥಾ ಯದಿ( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ಅನ್ಯಥಾ{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ಭಙ್ಗ;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    ಪಕ್ಷ 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      ಭಙ್ಗ;
    ಪಕ್ಷ 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      ಭಙ್ಗ;
#ಅನ್ಯಥಾ
    ಪಕ್ಷ 9:     /* nexec */
    ಪಕ್ಷ 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      ಭಙ್ಗ;
#endif

    ಪಕ್ಷ 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    ಪಕ್ಷ 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    ಪಕ್ಷ 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      ಭಙ್ಗ;
    ಪಕ್ಷ 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  (ಶೂನ್ಯ)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_ಅಂಕೆ==0;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_TEXT ){
    ಧ್ರುವ ವರ್ಣ *zSql = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ಯದಿ( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ಅನ್ಯಥಾ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ಅನ್ಯಥಾ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_ಮೌಲ್ಯ_pointer(argv[0],"stmt-pointer");
  }
  ಯದಿ( pCur->pStmt==0 ){
    pVTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    bytecodevtabNext(pVtabCursor);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_CONSTRAINT;
  ಸಂರಚನಾ sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  ಪೂರ್ಣಾಙ್ಕ iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_ಅಂಕೆ = 0;
  ಪ್ರತಿ(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ಯದಿ( p->usable==0 ) ಅನುವರ್ತ;
    ಯದಿ( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    ಯದಿ( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ಸ್ಥಿರ sqlite3_module bytecodevtabModule = {
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


ಪೂರ್ಣಾಙ್ಕ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
#ನೋಚೇದ್ಯದಿ defined(SQLITE_ENABLE_BYTECODE_VTAB)
ಪೂರ್ಣಾಙ್ಕ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
