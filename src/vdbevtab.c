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
#ضمن "sqliteInt.h"
#إذا defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#ضمن "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
تعريفنوع بنية bytecodevtab bytecodevtab;
بنية bytecodevtab {
  sqlite3_vtab ادھار;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  صحيح b_Tables_ورتیا;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
تعريفنوع بنية bytecodevtab_cursor bytecodevtab_cursor;
بنية bytecodevtab_cursor {
  sqlite3_vtab_cursor ادھار;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  صحيح iRowid;                /* The rowid of the output table */
  صحيح iAddr;                 /*پتہ*/
  صحيح needFinalize;          /* Cursors owns pStmt and must finalize it */
  صحيح showSubprograms;       /* Provide a listing of subprograms */
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
ساكن صحيح bytecodevtabConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  bytecodevtab *pNew;
  صحيح rc;
  صحيح is_Tab_ورتیا = pAux!=0;
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

  (فراغ)argc;
  (فراغ)argv;
  (فراغ)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_ورتیا]);
  إذا( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( حجم(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    إذا( pNew==0 ) أرجع SQLITE_NOMEM;
    memset(pNew, 0, حجم(*pNew));
    pNew->db = db;
    pNew->b_Tables_ورتیا = is_Tab_ورتیا*2;
  }
  أرجع rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ساكن صحيح bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  أرجع SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ساكن صحيح bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( حجم(*pCur) );
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  memset(pCur, 0, حجم(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->ادھار;
  أرجع SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ساكن فراغ bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_سیٹ_Null(&pCur->sub);
  إذا( pCur->needFinalize ){
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
ساكن صحيح bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  أرجع SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ساكن صحيح bytecodevtab_اگلا(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  صحيح rc;
  إذا( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  إذا( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_اگلا_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_ورتیا,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  إذا( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_سیٹ_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  أرجع SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ساكن صحيح bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  أرجع pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ساكن صحيح bytecodevtab_کالم(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحيح i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  إذا( pVTab->b_Tables_ورتیا ){
    إذا( i==4 ){
      i = 8;
    }وإلا{
      إذا( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        صحيح iDb = pOp->p3;
        Pgno i_جڑ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        لكل(k=sqlite_Hash_پہلا(&pSchema->tblHash); k; k=sqlite_Hash_اگلا(k)){
          جدول *pTab = (جدول*)sqlite_Hash_انکڑے(k);
          إذا( !IsVirtual(pTab) && pTab->tnum==i_جڑ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            اكسر;
          }
        }
        إذا( pCur->zName==0 ){
          لكل(k=sqlite_Hash_پہلا(&pSchema->idxHash); k; k=sqlite_Hash_اگلا(k)){
            اشاریہ *pIdx = (اشاریہ*)sqlite_Hash_انکڑے(k);
            إذا( pIdx->tnum==i_جڑ ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  بدل( i ){
    حالة 0:   /* addr */
      sqlite3_نتیجہ_int(ctx, pCur->iAddr);
      اكسر;
    حالة 1:   /* opcode */
      sqlite3_نتیجہ_متن(ctx, (حرف*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      اكسر;
    حالة 2:   /* p1 */
      sqlite3_نتیجہ_int(ctx, pOp->p1);
      اكسر;
    حالة 3:   /* p2 */
      sqlite3_نتیجہ_int(ctx, pOp->p2);
      اكسر;
    حالة 4:   /* p3 */
      sqlite3_نتیجہ_int(ctx, pOp->p3);
      اكسر;
    حالة 5:   /* p4 */
    حالة 7:   /* comment */
      إذا( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      إذا( i==5 ){
        sqlite3_نتیجہ_متن(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }وإلا{
#جے_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
        حرف *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_نتیجہ_متن(ctx, zCom, -1, sqlite3_free);
#جے_انت
      }
      اكسر;
    حالة 6:     /* p5 */
      sqlite3_نتیجہ_int(ctx, pOp->p5);
      اكسر;
    حالة 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      أكد( aOp[0].opcode==OP_Init );
      أكد( aOp[0].p4.z==0 || لڑی_مقابلہ(aOp[0].p4.z,"-" "- ",3)==0 );
      إذا( pCur->iRowid==pCur->iAddr+1 ){
        اكسر;  /* Result is NULL for the main program */
      }وإلا إذا( aOp[0].p4.z!=0 ){
         sqlite3_نتیجہ_متن(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }وإلا{
         sqlite3_نتیجہ_متن(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      اكسر;
    }

#جے_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
    حالة 9:     /* nexec */
      sqlite3_نتیجہ_int64(ctx, pOp->nExec);
      اكسر;
    حالة 10:    /* ncycle */
      sqlite3_نتیجہ_int64(ctx, pOp->n_چکر);
      اكسر;
#وإلا
    حالة 9:     /* nexec */
    حالة 10:    /* ncycle */
      sqlite3_نتیجہ_int(ctx, 0);
      اكسر;
#جے_انت

    حالة 20:  /* tables_used.type */
      sqlite3_نتیجہ_متن(ctx, pCur->zType, -1, SQLITE_STATIC);
      اكسر;
    حالة 21:  /* tables_used.schema */
      sqlite3_نتیجہ_متن(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      اكسر;
    حالة 22:  /* tables_used.name */
      sqlite3_نتیجہ_متن(ctx, pCur->zName, -1, SQLITE_STATIC);
      اكسر;
    حالة 23:  /* tables_used.wr */
      sqlite3_نتیجہ_int(ctx, pOp->opcode==OP_OpenWrite);
      اكسر;
  }
  أرجع SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ساكن صحيح bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  أرجع SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ساكن صحيح bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  صحيح rc = SQLITE_OK;
  (فراغ)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_عدد==0;
  أكد( argc==1 );
  إذا( sqlite3_قدر_type(argv[0])==SQLITE_متن ){
    ثابت حرف *zSql = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    إذا( zSql==0 ){
      rc = SQLITE_NOMEM;
    }وإلا{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }وإلا{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_قدر_اشارہ_گر(argv[0],"stmt-pointer");
  }
  إذا( pCur->pStmt==0 ){
    pVTab->ادھار.z_Err_پیغام = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_ورتیا ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }وإلا{
    bytecodevtab_اگلا(pVtabCursor);
  }
  أرجع rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ساكن صحيح bytecodevtab_بہترین_اشاریہ(
  sqlite3_vtab *tab,
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحيح i;
  صحيح rc = SQLITE_CONSTRAINT;
  بنية sqlite3_اشاریہ_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  صحيح i_ادھار_خانہ = pVTab->b_Tables_ورتیا ? 4 : 10;
  pIdxInfo->estimated_لاگت = (مزدوج)100;
  pIdxInfo->estimated_صفاں = 100;
  pIdxInfo->idx_عدد = 0;
  لكل(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    إذا( p->usable==0 ) استمر;
    إذا( p->op==SQLITE_اشاریہ_CONSTRAINT_EQ && p->i_کالم==i_ادھار_خانہ+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_اشاریہ = 1;
    }
    إذا( p->op==SQLITE_اشاریہ_CONSTRAINT_ISNULL && p->i_کالم==i_ادھار_خانہ ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_عدد = 1;
    }
  }
  أرجع rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ساكن sqlite3_module bytecodevtabModule = {
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
  /* xColumn     */ bytecodevtab_کالم,
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


صحيح sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  صحيح rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  إذا( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  أرجع rc;
}
#وإلاإذا defined(SQLITE_ENABLE_BYTECODE_VTAB)
صحيح sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ أرجع SQLITE_OK; }
#جے_انت /* SQLITE_ENABLE_BYTECODE_VTAB */
