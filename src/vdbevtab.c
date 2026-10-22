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
#അന്തര്ഭാവ "sqliteInt.h"
#എങ്കിൽ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#അന്തര്ഭാവ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
തരനിർവചനം ഘടന bytecodevtab bytecodevtab;
ഘടന bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  പൂർണ്ണസംഖ്യ bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
തരനിർവചനം ഘടന bytecodevtab_cursor bytecodevtab_cursor;
ഘടന bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  പൂർണ്ണസംഖ്യ iRowid;                /* The rowid of the output table */
  പൂർണ്ണസംഖ്യ iAddr;                 /* Address */
  പൂർണ്ണസംഖ്യ needFinalize;          /* Cursors owns pStmt and must finalize it */
  പൂർണ്ണസംഖ്യ showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  അക്ഷരം *zP4;                 /* Rendered P4 value */
  സ്ഥിരം അക്ഷരം *zType;         /* tables_used.type */
  സ്ഥിരം അക്ഷരം *zSchema;       /* tables_used.schema */
  സ്ഥിരം അക്ഷരം *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  bytecodevtab *pNew;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ isTabUsed = pAux!=0;
  സ്ഥിരം അക്ഷരം *azSchema[2] = {
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

  (ശൂന്യം)argc;
  (ശൂന്യം)argv;
  (ശൂന്യം)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  എങ്കിൽ( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( വലുപ്പം(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    എങ്കിൽ( pNew==0 ) തിരികെ SQLITE_NOMEM;
    memset(pNew, 0, വലുപ്പം(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  തിരികെ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  തിരികെ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( വലുപ്പം(*pCur) );
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCur, 0, വലുപ്പം(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
സ്ഥിതം ശൂന്യം bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  എങ്കിൽ( pCur->needFinalize ){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  എങ്കിൽ( pCur->zName ){
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
  എങ്കിൽ( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  തിരികെ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  തിരികെ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  പൂർണ്ണസംഖ്യ i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  എങ്കിൽ( pVTab->bTablesUsed ){
    എങ്കിൽ( i==4 ){
      i = 8;
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        പൂർണ്ണസംഖ്യ iDb = pOp->p3;
        Pgno i_വേര് = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        വേണ്ടി(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          എങ്കിൽ( !IsVirtual(pTab) && pTab->tnum==i_വേര് ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            നിർത്തുക;
          }
        }
        എങ്കിൽ( pCur->zName==0 ){
          വേണ്ടി(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            എങ്കിൽ( pIdx->tnum==i_വേര് ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      നിർത്തുക;
    അവസ്ഥ 1:   /* opcode */
      sqlite3_result_text(ctx, (അക്ഷരം*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      നിർത്തുക;
    അവസ്ഥ 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      നിർത്തുക;
    അവസ്ഥ 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      നിർത്തുക;
    അവസ്ഥ 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      നിർത്തുക;
    അവസ്ഥ 5:   /* p4 */
    അവസ്ഥ 7:   /* comment */
      എങ്കിൽ( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      എങ്കിൽ( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }അല്ലെങ്കിൽ{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        അക്ഷരം *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      നിർത്തുക;
    അവസ്ഥ 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      നിർത്തുക;
    അവസ്ഥ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      പ്രതിജ്ഞാ( aOp[0].opcode==OP_Init );
      പ്രതിജ്ഞാ( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      എങ്കിൽ( pCur->iRowid==pCur->iAddr+1 ){
        നിർത്തുക;  /* Result is NULL for the main program */
      }അല്ലെങ്കിൽ എങ്കിൽ( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }അല്ലെങ്കിൽ{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      നിർത്തുക;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    അവസ്ഥ 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      നിർത്തുക;
    അവസ്ഥ 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      നിർത്തുക;
#അല്ലെങ്കിൽ
    അവസ്ഥ 9:     /* nexec */
    അവസ്ഥ 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      നിർത്തുക;
#endif

    അവസ്ഥ 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      നിർത്തുക;
    അവസ്ഥ 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      നിർത്തുക;
    അവസ്ഥ 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      നിർത്തുക;
    അവസ്ഥ 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      നിർത്തുക;
  }
  തിരികെ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  തിരികെ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  (ശൂന്യം)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_അക്കം==0;
  പ്രതിജ്ഞാ( argc==1 );
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_TEXT ){
    സ്ഥിരം അക്ഷരം *zSql = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    എങ്കിൽ( zSql==0 ){
      rc = SQLITE_NOMEM;
    }അല്ലെങ്കിൽ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }അല്ലെങ്കിൽ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_മൂല്യം_pointer(argv[0],"stmt-pointer");
  }
  എങ്കിൽ( pCur->pStmt==0 ){
    pVTab->base.z_Err_സന്ദേശം = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    bytecodevtabNext(pVtabCursor);
  }
  തിരികെ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc = SQLITE_CONSTRAINT;
  ഘടന sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  പൂർണ്ണസംഖ്യ iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (ഇരട്ട)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_അക്കം = 0;
  വേണ്ടി(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    എങ്കിൽ( p->usable==0 ) തുടരുക;
    എങ്കിൽ( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    എങ്കിൽ( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_അക്കം = 1;
    }
  }
  തിരികെ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
സ്ഥിതം sqlite3_module bytecodevtabModule = {
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


പൂർണ്ണസംഖ്യ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  പൂർണ്ണസംഖ്യ rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  തിരികെ rc;
}
#അല്ലെങ്കിലെങ്കിൽ defined(SQLITE_ENABLE_BYTECODE_VTAB)
പൂർണ്ണസംഖ്യ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ തിരികെ SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
