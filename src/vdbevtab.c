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
#شامل "sqliteInt.h"
#اگر defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#شامل "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
تعریفنوع ساختار bytecodevtab bytecodevtab;
ساختار bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  صحیح bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
تعریفنوع ساختار bytecodevtab_cursor bytecodevtab_cursor;
ساختار bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  صحیح iRowid;                /* The rowid of the output table */
  صحیح iAddr;                 /* Address */
  صحیح needFinalize;          /* Cursors owns pStmt and must finalize it */
  صحیح showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  نویسه *zP4;                 /* Rendered P4 value */
  ثابت نویسه *zType;         /* tables_used.type */
  ثابت نویسه *zSchema;       /* tables_used.schema */
  ثابت نویسه *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ایستا صحیح bytecodevtabConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  bytecodevtab *pNew;
  صحیح rc;
  صحیح isTabUsed = pAux!=0;
  ثابت نویسه *azSchema[2] = {
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

  (تهی)argc;
  (تهی)argv;
  (تهی)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  اگر( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( اندازه(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    اگر( pNew==0 ) بازگشت SQLITE_NOMEM;
    memset(pNew, 0, اندازه(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  بازگشت rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ایستا صحیح bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  بازگشت SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ایستا صحیح bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( اندازه(*pCur) );
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  memset(pCur, 0, اندازه(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ایستا تهی bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  اگر( pCur->needFinalize ){
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
ایستا صحیح bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  بازگشت SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ایستا صحیح bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  صحیح rc;
  اگر( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  اگر( pCur->zName ){
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
  اگر( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  بازگشت SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ایستا صحیح bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  بازگشت pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ایستا صحیح bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  اگر( pVTab->bTablesUsed ){
    اگر( i==4 ){
      i = 8;
    }وگرنه{
      اگر( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        صحیح iDb = pOp->p3;
        Pgno i_ریشه = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        برای(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          اگر( !IsVirtual(pTab) && pTab->tnum==i_ریشه ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            بشکن;
          }
        }
        اگر( pCur->zName==0 ){
          برای(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            اگر( pIdx->tnum==i_ریشه ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  انتخاب( i ){
    حالت 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      بشکن;
    حالت 1:   /* opcode */
      sqlite3_result_text(ctx, (نویسه*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      بشکن;
    حالت 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      بشکن;
    حالت 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      بشکن;
    حالت 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      بشکن;
    حالت 5:   /* p4 */
    حالت 7:   /* comment */
      اگر( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      اگر( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }وگرنه{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        نویسه *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      بشکن;
    حالت 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      بشکن;
    حالت 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      أكد( aOp[0].opcode==OP_Init );
      أكد( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      اگر( pCur->iRowid==pCur->iAddr+1 ){
        بشکن;  /* Result is NULL for the main program */
      }وگرنه اگر( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }وگرنه{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      بشکن;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    حالت 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      بشکن;
    حالت 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      بشکن;
#وگرنه
    حالت 9:     /* nexec */
    حالت 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      بشکن;
#endif

    حالت 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      بشکن;
    حالت 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      بشکن;
    حالت 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      بشکن;
    حالت 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      بشکن;
  }
  بازگشت SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ایستا صحیح bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  بازگشت SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ایستا صحیح bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  صحیح rc = SQLITE_OK;
  (تهی)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_رقم==0;
  أكد( argc==1 );
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_TEXT ){
    ثابت نویسه *zSql = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    اگر( zSql==0 ){
      rc = SQLITE_NOMEM;
    }وگرنه{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }وگرنه{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_مقدار_pointer(argv[0],"stmt-pointer");
  }
  اگر( pCur->pStmt==0 ){
    pVTab->base.z_Err_پیام = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }وگرنه{
    bytecodevtabNext(pVtabCursor);
  }
  بازگشت rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ایستا صحیح bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  صحیح i;
  صحیح rc = SQLITE_CONSTRAINT;
  ساختار sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  صحیح iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (دوگانه)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_رقم = 0;
  برای(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    اگر( p->usable==0 ) ادامه;
    اگر( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    اگر( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_رقم = 1;
    }
  }
  بازگشت rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ایستا sqlite3_module bytecodevtabModule = {
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


صحیح sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  صحیح rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  بازگشت rc;
}
#وگرنهاگر defined(SQLITE_ENABLE_BYTECODE_VTAB)
صحیح sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ بازگشت SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
