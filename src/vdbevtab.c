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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ bytecodevtab bytecodevtab;
ਬਣਤਰ bytecodevtab {
  sqlite3_vtab ਅਧਾਰ;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  ਪੂਰਨ b_Tables_ਵਰਤਿਆ;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ bytecodevtab_cursor bytecodevtab_cursor;
ਬਣਤਰ bytecodevtab_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  ਪੂਰਨ iRowid;                /* The rowid of the output table */
  ਪੂਰਨ iAddr;                 /*ਪਤਾ*/
  ਪੂਰਨ needFinalize;          /* Cursors owns pStmt and must finalize it */
  ਪੂਰਨ showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  ਅੱਖਰ *zP4;                 /* Rendered P4 value */
  ਅਚਲ ਅੱਖਰ *zType;         /* tables_used.type */
  ਅਚਲ ਅੱਖਰ *zSchema;       /* tables_used.schema */
  ਅਚਲ ਅੱਖਰ *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  bytecodevtab *pNew;
  ਪੂਰਨ rc;
  ਪੂਰਨ is_Tab_ਵਰਤਿਆ = pAux!=0;
  ਅਚਲ ਅੱਖਰ *azSchema[2] = {
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

  (ਖਾਲੀ)argc;
  (ਖਾਲੀ)argv;
  (ਖਾਲੀ)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_ਵਰਤਿਆ]);
  ਜੇ( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( ਆਕਾਰ(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM;
    memset(pNew, 0, ਆਕਾਰ(*pNew));
    pNew->db = db;
    pNew->b_Tables_ਵਰਤਿਆ = is_Tab_ਵਰਤਿਆ*2;
  }
  ਵਾਪਸ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( ਆਕਾਰ(*pCur) );
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCur, 0, ਆਕਾਰ(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ਸਥਿਰ ਖਾਲੀ bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(&pCur->sub);
  ਜੇ( pCur->needFinalize ){
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
ਸਥਿਰ ਪੂਰਨ bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtab_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  ਪੂਰਨ rc;
  ਜੇ( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ਜੇ( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_ਅਗਲਾ_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_ਵਰਤਿਆ,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ਵਾਪਸ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtab_ਕਾਲਮ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ਪੂਰਨ i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ਜੇ( pVTab->b_Tables_ਵਰਤਿਆ ){
    ਜੇ( i==4 ){
      i = 8;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        ਪੂਰਨ iDb = pOp->p3;
        Pgno i_ਜੜ੍ਹ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); k; k=sqlite_Hash_ਅਗਲਾ(k)){
          ਸਾਰਣੀ *pTab = (ਸਾਰਣੀ*)sqlite_Hash_ਅੰਕੜੇ(k);
          ਜੇ( !IsVirtual(pTab) && pTab->tnum==i_ਜੜ੍ਹ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ਰੋਕੋ;
          }
        }
        ਜੇ( pCur->zName==0 ){
          ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(&pSchema->idxHash); k; k=sqlite_Hash_ਅਗਲਾ(k)){
            ਸੂਚਕਾਂਕ *pIdx = (ਸੂਚਕਾਂਕ*)sqlite_Hash_ਅੰਕੜੇ(k);
            ਜੇ( pIdx->tnum==i_ਜੜ੍ਹ ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ਚੋਣ( i ){
    ਹਾਲਤ 0:   /* addr */
      sqlite3_ਨਤੀਜਾ_int(ctx, pCur->iAddr);
      ਰੋਕੋ;
    ਹਾਲਤ 1:   /* opcode */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, (ਅੱਖਰ*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ਰੋਕੋ;
    ਹਾਲਤ 2:   /* p1 */
      sqlite3_ਨਤੀਜਾ_int(ctx, pOp->p1);
      ਰੋਕੋ;
    ਹਾਲਤ 3:   /* p2 */
      sqlite3_ਨਤੀਜਾ_int(ctx, pOp->p2);
      ਰੋਕੋ;
    ਹਾਲਤ 4:   /* p3 */
      sqlite3_ਨਤੀਜਾ_int(ctx, pOp->p3);
      ਰੋਕੋ;
    ਹਾਲਤ 5:   /* p4 */
    ਹਾਲਤ 7:   /* comment */
      ਜੇ( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ਜੇ( i==5 ){
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
        ਅੱਖਰ *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, zCom, -1, sqlite3_free);
#ਜੇ_ਅੰਤ
      }
      ਰੋਕੋ;
    ਹਾਲਤ 6:     /* p5 */
      sqlite3_ਨਤੀਜਾ_int(ctx, pOp->p5);
      ਰੋਕੋ;
    ਹਾਲਤ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ਪ੍ਰਤਿਜ੍ਞਾ( aOp[0].opcode==OP_Init );
      ਪ੍ਰਤਿਜ੍ਞਾ( aOp[0].p4.z==0 || ਲੜੀ_ਤੁਲਨਾ(aOp[0].p4.z,"-" "- ",3)==0 );
      ਜੇ( pCur->iRowid==pCur->iAddr+1 ){
        ਰੋਕੋ;  /* Result is NULL for the main program */
      }ਨਹੀਂਤਾਂ ਜੇ( aOp[0].p4.z!=0 ){
         sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ਨਹੀਂਤਾਂ{
         sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ਰੋਕੋ;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
    ਹਾਲਤ 9:     /* nexec */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pOp->nExec);
      ਰੋਕੋ;
    ਹਾਲਤ 10:    /* ncycle */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pOp->n_ਚੱਕਰ);
      ਰੋਕੋ;
#ਨਹੀਂਤਾਂ
    ਹਾਲਤ 9:     /* nexec */
    ਹਾਲਤ 10:    /* ncycle */
      sqlite3_ਨਤੀਜਾ_int(ctx, 0);
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਹਾਲਤ 20:  /* tables_used.type */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCur->zType, -1, SQLITE_STATIC);
      ਰੋਕੋ;
    ਹਾਲਤ 21:  /* tables_used.schema */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ਰੋਕੋ;
    ਹਾਲਤ 22:  /* tables_used.name */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCur->zName, -1, SQLITE_STATIC);
      ਰੋਕੋ;
    ਹਾਲਤ 23:  /* tables_used.wr */
      sqlite3_ਨਤੀਜਾ_int(ctx, pOp->opcode==OP_OpenWrite);
      ਰੋਕੋ;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  ਪੂਰਨ rc = SQLITE_OK;
  (ਖਾਲੀ)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_ਅੰਕ==0;
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_ਪਾਠ ){
    ਅਚਲ ਅੱਖਰ *zSql = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਜੇ( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ਨਹੀਂਤਾਂ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_ਮੁੱਲ_ਸੂਚਕ(argv[0],"stmt-pointer");
  }
  ਜੇ( pCur->pStmt==0 ){
    pVTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_ਵਰਤਿਆ ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    bytecodevtab_ਅਗਲਾ(pVtabCursor);
  }
  ਵਾਪਸ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ਸਥਿਰ ਪੂਰਨ bytecodevtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ(
  sqlite3_vtab *tab,
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo
){
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_CONSTRAINT;
  ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  ਪੂਰਨ i_ਅਧਾਰ_ਖਾਨਾ = pVTab->b_Tables_ਵਰਤਿਆ ? 4 : 10;
  pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)100;
  pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 100;
  pIdxInfo->idx_ਅੰਕ = 0;
  ਲਈ(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ਜੇ( p->usable==0 ) ਜਾਰੀ;
    ਜੇ( p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ && p->i_ਕਾਲਮ==i_ਅਧਾਰ_ਖਾਨਾ+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_ਸੂਚਕਾਂਕ = 1;
    }
    ਜੇ( p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNULL && p->i_ਕਾਲਮ==i_ਅਧਾਰ_ਖਾਨਾ ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ਅੰਕ = 1;
    }
  }
  ਵਾਪਸ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ਸਥਿਰ sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_ਅਗਲਾ,
  /* xEof        */ bytecodevtabEof,
  /* xColumn     */ bytecodevtab_ਕਾਲਮ,
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


ਪੂਰਨ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  ਪੂਰਨ rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ਵਾਪਸ rc;
}
#ਜਾਂਜੇ defined(SQLITE_ENABLE_BYTECODE_VTAB)
ਪੂਰਨ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ਵਾਪਸ SQLITE_OK; }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_BYTECODE_VTAB */
