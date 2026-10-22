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
قسمتعریف ساخت bytecodevtab bytecodevtab;
ساخت bytecodevtab {
  sqlite3_vtab بنیاد;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  صحیح b_Tables_مستعمل;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
قسمتعریف ساخت bytecodevtab_cursor bytecodevtab_cursor;
ساخت bytecodevtab_cursor {
  sqlite3_vtab_cursor بنیاد;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  صحیح iRowid;                /* The rowid of the output table */
  صحیح iAddr;                 /*پتہ*/
  صحیح needFinalize;          /* Cursors owns pStmt and must finalize it */
  صحیح showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  حرف *zP4;                 /* Rendered P4 value */
  ثابت حرف *zType;         /* tables_used.type */
  ثابت حرف *zSchema;       /* tables_used.schema */
  ثابت حرف *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ساکن صحیح bytecodevtabConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  bytecodevtab *pNew;
  صحیح rc;
  صحیح is_Tab_مستعمل = pAux!=0;
  ثابت حرف *azSchema[2] = {
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

  (خالی)argc;
  (خالی)argv;
  (خالی)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_مستعمل]);
  اگر( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( حجم(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    اگر( pNew==0 ) واپس SQLITE_NOMEM;
    memset(pNew, 0, حجم(*pNew));
    pNew->db = db;
    pNew->b_Tables_مستعمل = is_Tab_مستعمل*2;
  }
  واپس rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ساکن صحیح bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  واپس SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ساکن صحیح bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( حجم(*pCur) );
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  memset(pCur, 0, حجم(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ساکن خالی bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_سیٹ_Null(&pCur->sub);
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
ساکن صحیح bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ساکن صحیح bytecodevtab_اگلا(sqlite3_vtab_cursor *cur){
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
  rc = sqlite3_Vdbe_اگلا_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_مستعمل,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  اگر( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_سیٹ_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  واپس SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ساکن صحیح bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  واپس pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ساکن صحیح bytecodevtab_عمود(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  اگر( pVTab->b_Tables_مستعمل ){
    اگر( i==4 ){
      i = 8;
    }ورنہ{
      اگر( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        صحیح iDb = pOp->p3;
        Pgno i_جڑ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        برائے(k=sqlite_Hash_پہلا(&pSchema->tblHash); k; k=sqlite_Hash_اگلا(k)){
          جدول *pTab = (جدول*)sqlite_Hash_ڈیٹا(k);
          اگر( !IsVirtual(pTab) && pTab->tnum==i_جڑ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            توڑو;
          }
        }
        اگر( pCur->zName==0 ){
          برائے(k=sqlite_Hash_پہلا(&pSchema->idxHash); k; k=sqlite_Hash_اگلا(k)){
            اشاریہ *pIdx = (اشاریہ*)sqlite_Hash_ڈیٹا(k);
            اگر( pIdx->tnum==i_جڑ ){
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
      sqlite3_نتیجہ_int(ctx, pCur->iAddr);
      توڑو;
    حالت 1:   /* opcode */
      sqlite3_نتیجہ_متن(ctx, (حرف*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      توڑو;
    حالت 2:   /* p1 */
      sqlite3_نتیجہ_int(ctx, pOp->p1);
      توڑو;
    حالت 3:   /* p2 */
      sqlite3_نتیجہ_int(ctx, pOp->p2);
      توڑو;
    حالت 4:   /* p3 */
      sqlite3_نتیجہ_int(ctx, pOp->p3);
      توڑو;
    حالت 5:   /* p4 */
    حالت 7:   /* comment */
      اگر( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      اگر( i==5 ){
        sqlite3_نتیجہ_متن(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
        حرف *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_نتیجہ_متن(ctx, zCom, -1, sqlite3_free);
#اگر_ختم
      }
      توڑو;
    حالت 6:     /* p5 */
      sqlite3_نتیجہ_int(ctx, pOp->p5);
      توڑو;
    حالت 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      أكد( aOp[0].opcode==OP_Init );
      أكد( aOp[0].p4.z==0 || لڑی_موازنہ(aOp[0].p4.z,"-" "- ",3)==0 );
      اگر( pCur->iRowid==pCur->iAddr+1 ){
        توڑو;  /* Result is NULL for the main program */
      }ورنہ اگر( aOp[0].p4.z!=0 ){
         sqlite3_نتیجہ_متن(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ورنہ{
         sqlite3_نتیجہ_متن(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      توڑو;
    }

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
    حالت 9:     /* nexec */
      sqlite3_نتیجہ_int64(ctx, pOp->nExec);
      توڑو;
    حالت 10:    /* ncycle */
      sqlite3_نتیجہ_int64(ctx, pOp->n_چکر);
      توڑو;
#ورنہ
    حالت 9:     /* nexec */
    حالت 10:    /* ncycle */
      sqlite3_نتیجہ_int(ctx, 0);
      توڑو;
#اگر_ختم

    حالت 20:  /* tables_used.type */
      sqlite3_نتیجہ_متن(ctx, pCur->zType, -1, SQLITE_STATIC);
      توڑو;
    حالت 21:  /* tables_used.schema */
      sqlite3_نتیجہ_متن(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      توڑو;
    حالت 22:  /* tables_used.name */
      sqlite3_نتیجہ_متن(ctx, pCur->zName, -1, SQLITE_STATIC);
      توڑو;
    حالت 23:  /* tables_used.wr */
      sqlite3_نتیجہ_int(ctx, pOp->opcode==OP_OpenWrite);
      توڑو;
  }
  واپس SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ساکن صحیح bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  واپس SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ساکن صحیح bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  صحیح rc = SQLITE_OK;
  (خالی)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_عدد==0;
  أكد( argc==1 );
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_متن ){
    ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    اگر( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ورنہ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ورنہ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_قدر_اشارہ_گر(argv[0],"stmt-pointer");
  }
  اگر( pCur->pStmt==0 ){
    pVTab->بنیاد.z_Err_پیغام = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_مستعمل ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ورنہ{
    bytecodevtab_اگلا(pVtabCursor);
  }
  واپس rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ساکن صحیح bytecodevtab_بہترین_اشاریہ(
  sqlite3_vtab *tab,
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحیح i;
  صحیح rc = SQLITE_CONSTRAINT;
  ساخت sqlite3_اشاریہ_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  صحیح i_بنیاد_ستون = pVTab->b_Tables_مستعمل ? 4 : 10;
  pIdxInfo->estimated_لاگت = (دوہرا)100;
  pIdxInfo->estimated_صفیں = 100;
  pIdxInfo->idx_عدد = 0;
  برائے(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    اگر( p->usable==0 ) جاری;
    اگر( p->op==SQLITE_اشاریہ_CONSTRAINT_EQ && p->i_عمود==i_بنیاد_ستون+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_اشاریہ = 1;
    }
    اگر( p->op==SQLITE_اشاریہ_CONSTRAINT_ISNULL && p->i_عمود==i_بنیاد_ستون ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_عدد = 1;
    }
  }
  واپس rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ساکن sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_بہترین_اشاریہ,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_اگلا,
  /* xEof        */ bytecodevtabEof,
  /* xColumn     */ bytecodevtab_عمود,
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
  واپس rc;
}
#وگرنہ_اگر defined(SQLITE_ENABLE_BYTECODE_VTAB)
صحیح sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ واپس SQLITE_OK; }
#اگر_ختم /* SQLITE_ENABLE_BYTECODE_VTAB */
