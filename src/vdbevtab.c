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
#अन्तर्भाव "sqliteInt.h"
#जर defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#अन्तर्भाव "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
प्रकारव्याख्या संरचना bytecodevtab bytecodevtab;
संरचना bytecodevtab {
  sqlite3_vtab base;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  पूर्णांक bTablesUsed;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
प्रकारव्याख्या संरचना bytecodevtab_cursor bytecodevtab_cursor;
संरचना bytecodevtab_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  पूर्णांक iRowid;                /* The rowid of the output table */
  पूर्णांक iAddr;                 /* Address */
  पूर्णांक needFinalize;          /* Cursors owns pStmt and must finalize it */
  पूर्णांक showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  अक्षर *zP4;                 /* Rendered P4 value */
  स्थिरांक अक्षर *zType;         /* tables_used.type */
  स्थिरांक अक्षर *zSchema;       /* tables_used.schema */
  स्थिरांक अक्षर *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
स्थिर पूर्णांक bytecodevtabConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  bytecodevtab *pNew;
  पूर्णांक rc;
  पूर्णांक isTabUsed = pAux!=0;
  स्थिरांक अक्षर *azSchema[2] = {
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

  (शून्य)argc;
  (शून्य)argv;
  (शून्य)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[isTabUsed]);
  जर( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( आकार(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    जर( pNew==0 ) परत SQLITE_NOMEM;
    स_रखो(pNew, 0, आकार(*pNew));
    pNew->db = db;
    pNew->bTablesUsed = isTabUsed*2;
  }
  परत rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
स्थिर पूर्णांक bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  परत SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
स्थिर पूर्णांक bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( आकार(*pCur) );
  जर( pCur==0 ) परत SQLITE_NOMEM;
  स_रखो(pCur, 0, आकार(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->base;
  परत SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
स्थिर शून्य bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3VdbeMemSetNull(&pCur->sub);
  जर( pCur->needFinalize ){
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
स्थिर पूर्णांक bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  परत SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
स्थिर पूर्णांक bytecodevtabNext(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  पूर्णांक rc;
  जर( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  जर( pCur->zName ){
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
  जर( rc!=SQLITE_OK ){
    sqlite3VdbeMemSetNull(&pCur->sub);
    pCur->aOp = 0;
  }
  परत SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
स्थिर पूर्णांक bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  परत pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
स्थिर पूर्णांक bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णांक i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  जर( pVTab->bTablesUsed ){
    जर( i==4 ){
      i = 8;
    }अन्यथा{
      जर( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        पूर्णांक iDb = pOp->p3;
        Pgno i_मूळ = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        साठी(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
          Table *pTab = (Table*)sqliteHashData(k);
          जर( !IsVirtual(pTab) && pTab->tnum==i_मूळ ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            थांबा;
          }
        }
        जर( pCur->zName==0 ){
          साठी(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqliteHashNext(k)){
            Index *pIdx = (Index*)sqliteHashData(k);
            जर( pIdx->tnum==i_मूळ ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  निवड( i ){
    स्थिती 0:   /* addr */
      sqlite3_result_int(ctx, pCur->iAddr);
      थांबा;
    स्थिती 1:   /* opcode */
      sqlite3_result_text(ctx, (अक्षर*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      थांबा;
    स्थिती 2:   /* p1 */
      sqlite3_result_int(ctx, pOp->p1);
      थांबा;
    स्थिती 3:   /* p2 */
      sqlite3_result_int(ctx, pOp->p2);
      थांबा;
    स्थिती 4:   /* p3 */
      sqlite3_result_int(ctx, pOp->p3);
      थांबा;
    स्थिती 5:   /* p4 */
    स्थिती 7:   /* comment */
      जर( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      जर( i==5 ){
        sqlite3_result_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }अन्यथा{
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        अक्षर *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_result_text(ctx, zCom, -1, sqlite3_free);
#endif
      }
      थांबा;
    स्थिती 6:     /* p5 */
      sqlite3_result_int(ctx, pOp->p5);
      थांबा;
    स्थिती 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      प्रतिज्ञा( aOp[0].opcode==OP_Init );
      प्रतिज्ञा( aOp[0].p4.z==0 || म_भेदन(aOp[0].p4.z,"-" "- ",3)==0 );
      जर( pCur->iRowid==pCur->iAddr+1 ){
        थांबा;  /* Result is NULL for the main program */
      }अन्यथा जर( aOp[0].p4.z!=0 ){
         sqlite3_result_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }अन्यथा{
         sqlite3_result_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      थांबा;
    }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
    स्थिती 9:     /* nexec */
      sqlite3_result_int64(ctx, pOp->nExec);
      थांबा;
    स्थिती 10:    /* ncycle */
      sqlite3_result_int64(ctx, pOp->nCycle);
      थांबा;
#अन्यथा
    स्थिती 9:     /* nexec */
    स्थिती 10:    /* ncycle */
      sqlite3_result_int(ctx, 0);
      थांबा;
#endif

    स्थिती 20:  /* tables_used.type */
      sqlite3_result_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      थांबा;
    स्थिती 21:  /* tables_used.schema */
      sqlite3_result_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      थांबा;
    स्थिती 22:  /* tables_used.name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      थांबा;
    स्थिती 23:  /* tables_used.wr */
      sqlite3_result_int(ctx, pOp->opcode==OP_OpenWrite);
      थांबा;
  }
  परत SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
स्थिर पूर्णांक bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  परत SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
स्थिर पूर्णांक bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  पूर्णांक rc = SQLITE_OK;
  (शून्य)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_अंक==0;
  प्रतिज्ञा( argc==1 );
  जर( sqlite3_मूल्य_type(argv[0])==SQLITE_TEXT ){
    स्थिरांक अक्षर *zSql = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
    जर( zSql==0 ){
      rc = SQLITE_NOMEM;
    }अन्यथा{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }अन्यथा{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_मूल्य_pointer(argv[0],"stmt-pointer");
  }
  जर( pCur->pStmt==0 ){
    pVTab->base.z_Err_संदेश = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->bTablesUsed ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }अन्यथा{
    bytecodevtabNext(pVtabCursor);
  }
  परत rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
स्थिर पूर्णांक bytecodevtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  पूर्णांक i;
  पूर्णांक rc = SQLITE_CONSTRAINT;
  संरचना sqlite3_index_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  पूर्णांक iBaseCol = pVTab->bTablesUsed ? 4 : 10;
  pIdxInfo->estimatedCost = (द्विगुण)100;
  pIdxInfo->estimatedRows = 100;
  pIdxInfo->idx_अंक = 0;
  साठी(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    जर( p->usable==0 ) सुरूठेवा;
    जर( p->op==SQLITE_INDEX_CONSTRAINT_EQ && p->iColumn==iBaseCol+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    }
    जर( p->op==SQLITE_INDEX_CONSTRAINT_ISNULL && p->iColumn==iBaseCol ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_अंक = 1;
    }
  }
  परत rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
स्थिर sqlite3_module bytecodevtabModule = {
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


पूर्णांक sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  पूर्णांक rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  जर( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  परत rc;
}
#किंवाजर defined(SQLITE_ENABLE_BYTECODE_VTAB)
पूर्णांक sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ परत SQLITE_OK; }
#endif /* SQLITE_ENABLE_BYTECODE_VTAB */
