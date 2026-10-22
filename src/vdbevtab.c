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
#అన్తర్భావ "sqliteInt.h"
#ఒకవేళ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#అన్తర్భావ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
రకనిర్వచనం నిర్మాణం bytecodevtab bytecodevtab;
నిర్మాణం bytecodevtab {
  sqlite3_vtab పునాది;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  పూర్ణాంకం b_Tables_వాడిన;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
రకనిర్వచనం నిర్మాణం bytecodevtab_cursor bytecodevtab_cursor;
నిర్మాణం bytecodevtab_cursor {
  sqlite3_vtab_cursor పునాది;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  పూర్ణాంకం iRowid;                /* The rowid of the output table */
  పూర్ణాంకం iAddr;                 /*చిరునామా*/
  పూర్ణాంకం needFinalize;          /* Cursors owns pStmt and must finalize it */
  పూర్ణాంకం showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  అక్షరం *zP4;                 /* Rendered P4 value */
  స్థిరరాశి అక్షరం *zType;         /* tables_used.type */
  స్థిరరాశి అక్షరం *zSchema;       /* tables_used.schema */
  స్థిరరాశి అక్షరం *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
స్థిర పూర్ణాంకం bytecodevtabConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  bytecodevtab *pNew;
  పూర్ణాంకం rc;
  పూర్ణాంకం is_Tab_వాడిన = pAux!=0;
  స్థిరరాశి అక్షరం *azSchema[2] = {
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

  (శూన్య)argc;
  (శూన్య)argv;
  (శూన్య)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_వాడిన]);
  ఒకవేళ( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( పరిమాణం(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM;
    memset(pNew, 0, పరిమాణం(*pNew));
    pNew->db = db;
    pNew->b_Tables_వాడిన = is_Tab_వాడిన*2;
  }
  తిరిగి rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
స్థిర పూర్ణాంకం bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  తిరిగి SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
స్థిర పూర్ణాంకం bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( పరిమాణం(*pCur) );
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCur, 0, పరిమాణం(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
స్థిర శూన్య bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_సమితి_Null(&pCur->sub);
  ఒకవేళ( pCur->needFinalize ){
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
స్థిర పూర్ణాంకం bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
స్థిర పూర్ణాంకం bytecodevtab_తదుపరి(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  పూర్ణాంకం rc;
  ఒకవేళ( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ఒకవేళ( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_తదుపరి_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_వాడిన,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_సమితి_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
స్థిర పూర్ణాంకం bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  తిరిగి pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
స్థిర పూర్ణాంకం bytecodevtab_స్తంభం(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  పూర్ణాంకం i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ఒకవేళ( pVTab->b_Tables_వాడిన ){
    ఒకవేళ( i==4 ){
      i = 8;
    }లేకపోతే{
      ఒకవేళ( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        పూర్ణాంకం iDb = pOp->p3;
        Pgno i_మూలం = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        కొరకు(k=sqlite_Hash_మొదటి(&pSchema->tblHash); k; k=sqlite_Hash_తదుపరి(k)){
          పట్టిక *pTab = (పట్టిక*)sqlite_Hash_దత్తాంశం(k);
          ఒకవేళ( !IsVirtual(pTab) && pTab->tnum==i_మూలం ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ఆపు;
          }
        }
        ఒకవేళ( pCur->zName==0 ){
          కొరకు(k=sqlite_Hash_మొదటి(&pSchema->idxHash); k; k=sqlite_Hash_తదుపరి(k)){
            సూచీ *pIdx = (సూచీ*)sqlite_Hash_దత్తాంశం(k);
            ఒకవేళ( pIdx->tnum==i_మూలం ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ఎంపిక( i ){
    స్థితి 0:   /* addr */
      sqlite3_ఫలితం_int(ctx, pCur->iAddr);
      ఆపు;
    స్థితి 1:   /* opcode */
      sqlite3_ఫలితం_పాఠ్యం(ctx, (అక్షరం*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ఆపు;
    స్థితి 2:   /* p1 */
      sqlite3_ఫలితం_int(ctx, pOp->p1);
      ఆపు;
    స్థితి 3:   /* p2 */
      sqlite3_ఫలితం_int(ctx, pOp->p2);
      ఆపు;
    స్థితి 4:   /* p3 */
      sqlite3_ఫలితం_int(ctx, pOp->p3);
      ఆపు;
    స్థితి 5:   /* p4 */
    స్థితి 7:   /* comment */
      ఒకవేళ( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ఒకవేళ( i==5 ){
        sqlite3_ఫలితం_పాఠ్యం(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
        అక్షరం *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_ఫలితం_పాఠ్యం(ctx, zCom, -1, sqlite3_free);
#అయితే_ముగింపు
      }
      ఆపు;
    స్థితి 6:     /* p5 */
      sqlite3_ఫలితం_int(ctx, pOp->p5);
      ఆపు;
    స్థితి 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ప్రతిజ్ఞా( aOp[0].opcode==OP_Init );
      ప్రతిజ్ఞా( aOp[0].p4.z==0 || అక్షరశ్రేణి_పోలిక(aOp[0].p4.z,"-" "- ",3)==0 );
      ఒకవేళ( pCur->iRowid==pCur->iAddr+1 ){
        ఆపు;  /* Result is NULL for the main program */
      }లేకపోతే ఒకవేళ( aOp[0].p4.z!=0 ){
         sqlite3_ఫలితం_పాఠ్యం(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }లేకపోతే{
         sqlite3_ఫలితం_పాఠ్యం(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ఆపు;
    }

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
    స్థితి 9:     /* nexec */
      sqlite3_ఫలితం_int64(ctx, pOp->nExec);
      ఆపు;
    స్థితి 10:    /* ncycle */
      sqlite3_ఫలితం_int64(ctx, pOp->n_చక్రం);
      ఆపు;
#లేకపోతే
    స్థితి 9:     /* nexec */
    స్థితి 10:    /* ncycle */
      sqlite3_ఫలితం_int(ctx, 0);
      ఆపు;
#అయితే_ముగింపు

    స్థితి 20:  /* tables_used.type */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCur->zType, -1, SQLITE_STATIC);
      ఆపు;
    స్థితి 21:  /* tables_used.schema */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ఆపు;
    స్థితి 22:  /* tables_used.name */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCur->zName, -1, SQLITE_STATIC);
      ఆపు;
    స్థితి 23:  /* tables_used.wr */
      sqlite3_ఫలితం_int(ctx, pOp->opcode==OP_OpenWrite);
      ఆపు;
  }
  తిరిగి SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
స్థిర పూర్ణాంకం bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  తిరిగి SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
స్థిర పూర్ణాంకం bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  పూర్ణాంకం rc = SQLITE_OK;
  (శూన్య)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_అంకె==0;
  ప్రతిజ్ఞా( argc==1 );
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_పాఠ్యం ){
    స్థిరరాశి అక్షరం *zSql = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    ఒకవేళ( zSql==0 ){
      rc = SQLITE_NOMEM;
    }లేకపోతే{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }లేకపోతే{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_విలువ_సూచిక(argv[0],"stmt-pointer");
  }
  ఒకవేళ( pCur->pStmt==0 ){
    pVTab->పునాది.z_Err_సందేశం = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_వాడిన ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }లేకపోతే{
    bytecodevtab_తదుపరి(pVtabCursor);
  }
  తిరిగి rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
స్థిర పూర్ణాంకం bytecodevtab_ఉత్తమ_సూచీ(
  sqlite3_vtab *tab,
  sqlite3_సూచీ_info *pIdxInfo
){
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_CONSTRAINT;
  నిర్మాణం sqlite3_సూచీ_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  పూర్ణాంకం i_పునాది_నిలువు = pVTab->b_Tables_వాడిన ? 4 : 10;
  pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)100;
  pIdxInfo->estimated_అడ్డువరుసలు = 100;
  pIdxInfo->idx_అంకె = 0;
  కొరకు(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ఒకవేళ( p->usable==0 ) కొనసాగు;
    ఒకవేళ( p->op==SQLITE_సూచీ_CONSTRAINT_EQ && p->i_స్తంభం==i_పునాది_నిలువు+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_సూచీ = 1;
    }
    ఒకవేళ( p->op==SQLITE_సూచీ_CONSTRAINT_ISNULL && p->i_స్తంభం==i_పునాది_నిలువు ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_అంకె = 1;
    }
  }
  తిరిగి rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
స్థిర sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_ఉత్తమ_సూచీ,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_తదుపరి,
  /* xEof        */ bytecodevtabEof,
  /* xColumn     */ bytecodevtab_స్తంభం,
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


పూర్ణాంకం sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  పూర్ణాంకం rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  తిరిగి rc;
}
#లేదావేళ defined(SQLITE_ENABLE_BYTECODE_VTAB)
పూర్ణాంకం sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ తిరిగి SQLITE_OK; }
#అయితే_ముగింపు /* SQLITE_ENABLE_BYTECODE_VTAB */
