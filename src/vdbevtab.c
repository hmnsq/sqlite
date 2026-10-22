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
#include "sqliteInt.h"
#ᤀᤡ᤺ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#include "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
typedef sa_nracanaa bytecodevtab bytecodevtab;
sa_nracanaa bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  puur_naa_nka bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
typedef sa_nracanaa bytecodevtab_cursor bytecodevtab_cursor;
sa_nracanaa bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  puur_naa_nka iRowid;                /* The rowid of the output table */
  puur_naa_nka iAddr;                 /* Address */
  puur_naa_nka needFinalize;          /* Cursors owns pStmt and must finalize it */
  puur_naa_nka showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  var_na *zP4;                 /* Rendered P4 value */
  const var_na *zType;         /* tables_used.type */
  const var_na *zSchema;       /* tables_used.schema */
  const var_na *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
static puur_naa_nka bytecodevtabConnect(
  sqlite3 *db,
  void *pAux,
  puur_naa_nka argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  bytecodevtab *pNew;
  puur_naa_nka rc;
  puur_naa_nka isTabUsed = pAux!=0;
  const var_na *azSchema[2] = {
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

  (void)argc;
  (void)argv;
  (void)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  ᤀᤡ᤺( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( parimaa_na(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ᤀᤡ᤺( pNew==0 ) ᤀᤡᤴ SQLITE_NOMEM;
    memset(pNew, 0, parimaa_na(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  ᤀᤡᤴ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
static puur_naa_nka bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
static puur_naa_nka bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( parimaa_na(*pCur) );
  ᤀᤡ᤺( pCur==0 ) ᤀᤡᤴ SQLITE_NOMEM;
  memset(pCur, 0, parimaa_na(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
static void bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  ᤀᤡ᤺( pCur->needFinalize ){
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
static puur_naa_nka bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ᤀᤡᤴ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
static puur_naa_nka bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  puur_naa_nka rc;
  ᤀᤡ᤺( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ᤀᤡ᤺( pCur->zName ){
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
  ᤀᤡ᤺( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static puur_naa_nka bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ᤀᤡᤴ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
static puur_naa_nka bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  puur_naa_nka i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ᤀᤡ᤺( pVTab->bTablesUsed ){
    ᤀᤡ᤺( i==4 ){
      i = 8;
    }ᤀᤡ᤺ᤆᤡ{
      ᤀᤡ᤺( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        puur_naa_nka iDb = pOp->p3;
        Pgno iRoot = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        for(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          ᤀᤡ᤺( !IsVirtual(pTab) && pTab->tnum==iRoot ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            bha_nga;
          }
        }
        ᤀᤡ᤺( pCur->zName==0 ){
          for(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            ᤀᤡ᤺( pIdx->tnum==iRoot ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  switch( i ){
    pak_sa 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      bha_nga;
    pak_sa 1:   /* opcode */
      sqlite3_result_text(ctx, (var_na*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      bha_nga;
    pak_sa 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      bha_nga;
    pak_sa 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      bha_nga;
    pak_sa 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      bha_nga;
    pak_sa 5:   /* p4 */
    pak_sa 7:   /* comment */
      ᤀᤡ᤺( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ᤀᤡ᤺( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ᤀᤡ᤺ᤆᤡ{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        var_na *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      bha_nga;
    pak_sa 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      bha_nga;
    pak_sa 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      assert( aOp[0].opcode==OP_Init );
      assert( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      ᤀᤡ᤺( pCur->iRowid==pCur->iAddr+1 ){
        bha_nga;  /* Result is NULL for the main program */
      }ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ᤀᤡ᤺ᤆᤡ{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      bha_nga;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    pak_sa 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      bha_nga;
    pak_sa 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      bha_nga;
#ᤀᤡ᤺ᤆᤡ
    pak_sa 9:     /* nexec */
    pak_sa 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      bha_nga;
#endif

    pak_sa 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      bha_nga;
    pak_sa 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      bha_nga;
    pak_sa 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      bha_nga;
    pak_sa 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      bha_nga;
  }
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static puur_naa_nka bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ᤀᤡᤴ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
static puur_naa_nka bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  puur_naa_nka idxNum, const var_na *idxStr,
  puur_naa_nka argc, sqlite3_value **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  puur_naa_nka rc = SQLITE_OK;
  (void)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idxNum==0;
  assert( argc==1 );
  ᤀᤡ᤺( sqlite3_value_type(argv[0])==SQLITE_TEXT ){
    const var_na *zSql = (const var_na*)sqlite3_value_text(argv[0]);
    ᤀᤡ᤺( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ᤀᤡ᤺ᤆᤡ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ᤀᤡ᤺ᤆᤡ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_value_pointer(argv[0],"stmt-pointer");
  }
  ᤀᤡ᤺( pCur->pStmt==0 ){
    pVTab->base.zErrMsg = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ᤀᤡ᤺ᤆᤡ{
    bytecodevtabNext(pVtabCursor);
  }
  ᤀᤡᤴ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
static puur_naa_nka bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  puur_naa_nka i;
  puur_naa_nka rc = SQLITE_CONSTRAINT;
  sa_nracanaa sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  puur_naa_nka iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (dviguu_na)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idxNum = 0;
  for(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ᤀᤡ᤺( p->usable==0 ) continue;
    ᤀᤡ᤺( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    ᤀᤡ᤺( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 1;
    }
  }
  ᤀᤡᤴ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
static sqlite3_module bytecodevtabModule = {
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


puur_naa_nka sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  puur_naa_nka rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ᤀᤡ᤺( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ᤀᤡᤴ rc;
}
#elif defined(SQLITE_ENABLE_BYTECODE_VTAB)
puur_naa_nka sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ᤀᤡᤴ SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
