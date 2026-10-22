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
#nếu defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#include "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
typedef struct bytecodevtab bytecodevtab;
struct bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  số_nguyên bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
typedef struct bytecodevtab_cursor bytecodevtab_cursor;
struct bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  số_nguyên iRowid;                /* The rowid of the output table */
  số_nguyên iAddr;                 /* Address */
  số_nguyên needFinalize;          /* Cursors owns pStmt and must finalize it */
  số_nguyên showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  char *zP4;                 /* Rendered P4 value */
  const char *zType;         /* tables_used.type */
  const char *zSchema;       /* tables_used.schema */
  const char *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
static số_nguyên bytecodevtabConnect(
  sqlite3 *db,
  void *pAux,
  số_nguyên argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  bytecodevtab *pNew;
  số_nguyên rc;
  số_nguyên isTabUsed = pAux!=0;
  const char *azSchema[2] = {
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
  nếu( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( sizeof(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    nếu( pNew==0 ) trả_về SQLITE_NOMEM;
    memset(pNew, 0, sizeof(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  trả_về rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
static số_nguyên bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  trả_về SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
static số_nguyên bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( sizeof(*pCur) );
  nếu( pCur==0 ) trả_về SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  trả_về SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
static void bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  nếu( pCur->needFinalize ){
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
static số_nguyên bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  trả_về SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
static số_nguyên bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  số_nguyên rc;
  nếu( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  nếu( pCur->zName ){
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
  nếu( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  trả_về SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static số_nguyên bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  trả_về pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
static số_nguyên bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  số_nguyên i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  nếu( pVTab->bTablesUsed ){
    nếu( i==4 ){
      i = 8;
    }không_thì{
      nếu( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        số_nguyên iDb = pOp->p3;
        Pgno iRoot = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        cho(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          nếu( !IsVirtual(pTab) && pTab->tnum==iRoot ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            break;
          }
        }
        nếu( pCur->zName==0 ){
          cho(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            nếu( pIdx->tnum==iRoot ){
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
    case 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      break;
    case 1:   /* opcode */
      sqlite3_result_text(ctx, (char*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      break;
    case 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      break;
    case 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      break;
    case 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      break;
    case 5:   /* p4 */
    case 7:   /* comment */
      nếu( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      nếu( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }không_thì{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        char *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      break;
    case 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      break;
    case 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      assert( aOp[0].opcode==OP_Init );
      assert( aOp[0].p4.z==0 || strncmp(aOp[0].p4.z,"-" "- ",3)==0 );
      nếu( pCur->iRowid==pCur->iAddr+1 ){
        break;  /* Result is NULL for the main program */
      }không_thì nếu( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }không_thì{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      break;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    case 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      break;
    case 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      break;
#không_thì
    case 9:     /* nexec */
    case 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      break;
#endif

    case 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      break;
    case 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      break;
    case 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      break;
    case 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      break;
  }
  trả_về SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static số_nguyên bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  trả_về SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
static số_nguyên bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  số_nguyên idxNum, const char *idxStr,
  số_nguyên argc, sqlite3_value **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  số_nguyên rc = SQLITE_OK;
  (void)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idxNum==0;
  assert( argc==1 );
  nếu( sqlite3_value_type(argv[0])==SQLITE_TEXT ){
    const char *zSql = (const char*)sqlite3_value_text(argv[0]);
    nếu( zSql==0 ){
      rc = SQLITE_NOMEM;
    }không_thì{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }không_thì{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_value_pointer(argv[0],"stmt-pointer");
  }
  nếu( pCur->pStmt==0 ){
    pVTab->base.zErrMsg = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }không_thì{
    bytecodevtabNext(pVtabCursor);
  }
  trả_về rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
static số_nguyên bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  số_nguyên i;
  số_nguyên rc = SQLITE_CONSTRAINT;
  struct sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  số_nguyên iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (double)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idxNum = 0;
  cho(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    nếu( p->usable==0 ) continue;
    nếu( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    nếu( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 1;
    }
  }
  trả_về rc;
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


số_nguyên sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  số_nguyên rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  nếu( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  trả_về rc;
}
#elif defined(SQLITE_ENABLE_BYTECODE_VTAB)
số_nguyên sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ trả_về SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
