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
#만약 defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#include "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
typedef struct bytecodevtab bytecodevtab;
struct bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  정수 bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
typedef struct bytecodevtab_cursor bytecodevtab_cursor;
struct bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  정수 iRowid;                /* The rowid of the output table */
  정수 iAddr;                 /* Address */
  정수 needFinalize;          /* Cursors owns pStmt and must finalize it */
  정수 showSubprograms;       /* Provide a listing of subprograms */
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
static 정수 bytecodevtabConnect(
  sqlite3 *db,
  void *pAux,
  정수 argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  bytecodevtab *pNew;
  정수 rc;
  정수 isTabUsed = pAux!=0;
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
  만약( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( sizeof(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    만약( pNew==0 ) 반환 SQLITE_NOMEM;
    memset(pNew, 0, sizeof(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  반환 rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
static 정수 bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  반환 SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
static 정수 bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( sizeof(*pCur) );
  만약( pCur==0 ) 반환 SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  반환 SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
static void bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  만약( pCur->needFinalize ){
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
static 정수 bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  반환 SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
static 정수 bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  정수 rc;
  만약( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  만약( pCur->zName ){
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
  만약( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  반환 SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static 정수 bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  반환 pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
static 정수 bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  정수 i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  만약( pVTab->bTablesUsed ){
    만약( i==4 ){
      i = 8;
    }아니면{
      만약( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        정수 iDb = pOp->p3;
        Pgno iRoot = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        위해(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          만약( !IsVirtual(pTab) && pTab->tnum==iRoot ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            break;
          }
        }
        만약( pCur->zName==0 ){
          위해(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            만약( pIdx->tnum==iRoot ){
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
      만약( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      만약( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }아니면{
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
      만약( pCur->iRowid==pCur->iAddr+1 ){
        break;  /* Result is NULL for the main program */
      }아니면 만약( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }아니면{
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
#아니면
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
  반환 SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static 정수 bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  반환 SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
static 정수 bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  정수 idxNum, const char *idxStr,
  정수 argc, sqlite3_value **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  정수 rc = SQLITE_OK;
  (void)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idxNum==0;
  assert( argc==1 );
  만약( sqlite3_value_type(argv[0])==SQLITE_TEXT ){
    const char *zSql = (const char*)sqlite3_value_text(argv[0]);
    만약( zSql==0 ){
      rc = SQLITE_NOMEM;
    }아니면{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }아니면{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_value_pointer(argv[0],"stmt-pointer");
  }
  만약( pCur->pStmt==0 ){
    pVTab->base.zErrMsg = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }아니면{
    bytecodevtabNext(pVtabCursor);
  }
  반환 rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
static 정수 bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  정수 i;
  정수 rc = SQLITE_CONSTRAINT;
  struct sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  정수 iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (double)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idxNum = 0;
  위해(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    만약( p->usable==0 ) continue;
    만약( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    만약( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 1;
    }
  }
  반환 rc;
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


정수 sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  정수 rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  만약( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  반환 rc;
}
#elif defined(SQLITE_ENABLE_BYTECODE_VTAB)
정수 sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ 반환 SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
