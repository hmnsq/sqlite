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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಆದರೆ defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ bytecodevtab bytecodevtab;
ರಚನೆ bytecodevtab {
  sqlite3_vtab ತಳ;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  ಪೂರ್ಣಾಂಕ b_Tables_ಬಳಸಿದ;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ bytecodevtab_cursor bytecodevtab_cursor;
ರಚನೆ bytecodevtab_cursor {
  sqlite3_vtab_cursor ತಳ;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  ಪೂರ್ಣಾಂಕ iRowid;                /* The rowid of the output table */
  ಪೂರ್ಣಾಂಕ iAddr;                 /*ವಿಳಾಸ*/
  ಪೂರ್ಣಾಂಕ needFinalize;          /* Cursors owns pStmt and must finalize it */
  ಪೂರ್ಣಾಂಕ showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  ಅಕ್ಷರ *zP4;                 /* Rendered P4 value */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;         /* tables_used.type */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema;       /* tables_used.schema */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  bytecodevtab *pNew;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ is_Tab_ಬಳಸಿದ = pAux!=0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azSchema[2] = {
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

  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)argv;
  (ಶೂನ್ಯ)pzErr;
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_ಬಳಸಿದ]);
  ಆದರೆ( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( ಗಾತ್ರ(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    memset(pNew, 0, ಗಾತ್ರ(*pNew));
    pNew->db = db;
    pNew->b_Tables_ಬಳಸಿದ = is_Tab_ಬಳಸಿದ*2;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( ಗಾತ್ರ(*pCur) );
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCur, 0, ಗಾತ್ರ(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
ಸ್ಥಿರ ಶೂನ್ಯ bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_ಗಣ_Null(&pCur->sub);
  ಆದರೆ( pCur->needFinalize ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtab_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  ಆದರೆ( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_ಮುಂದಿನ_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_ಬಳಸಿದ,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_ಗಣ_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  ಹಿಂತಿರುಗಿ pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtab_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಂಕ i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  ಆದರೆ( pVTab->b_Tables_ಬಳಸಿದ ){
    ಆದರೆ( i==4 ){
      i = 8;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        ಪೂರ್ಣಾಂಕ iDb = pOp->p3;
        Pgno i_ಬೇರು = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); k; k=sqlite_Hash_ಮುಂದಿನ(k)){
          ಕೋಷ್ಟಕ *pTab = (ಕೋಷ್ಟಕ*)sqlite_Hash_ದತ್ತಾಂಶ(k);
          ಆದರೆ( !IsVirtual(pTab) && pTab->tnum==i_ಬೇರು ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( pCur->zName==0 ){
          ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(&pSchema->idxHash); k; k=sqlite_Hash_ಮುಂದಿನ(k)){
            ಸೂಚ್ಯಂಕ *pIdx = (ಸೂಚ್ಯಂಕ*)sqlite_Hash_ದತ್ತಾಂಶ(k);
            ಆದರೆ( pIdx->tnum==i_ಬೇರು ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0:   /* addr */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pCur->iAddr);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:   /* opcode */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, (ಅಕ್ಷರ*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:   /* p1 */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pOp->p1);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 3:   /* p2 */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pOp->p2);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:   /* p3 */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pOp->p3);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 5:   /* p4 */
    ಸ್ಥಿತಿ 7:   /* comment */
      ಆದರೆ( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      ಆದರೆ( i==5 ){
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
        ಅಕ್ಷರ *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, zCom, -1, sqlite3_free);
#ಆದರೆ_ಅಂತ್ಯ
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 6:     /* p5 */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pOp->p5);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      ಪ್ರತಿಜ್ಞಾ( aOp[0].opcode==OP_Init );
      ಪ್ರತಿಜ್ಞಾ( aOp[0].p4.z==0 || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(aOp[0].p4.z,"-" "- ",3)==0 );
      ಆದರೆ( pCur->iRowid==pCur->iAddr+1 ){
        ನಿಲ್ಲಿಸು;  /* Result is NULL for the main program */
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( aOp[0].p4.z!=0 ){
         sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }ಇಲ್ಲದಿದ್ದರೆ{
         sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
    ಸ್ಥಿತಿ 9:     /* nexec */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pOp->nExec);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 10:    /* ncycle */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pOp->n_ಚಕ್ರ);
      ನಿಲ್ಲಿಸು;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿತಿ 9:     /* nexec */
    ಸ್ಥಿತಿ 10:    /* ncycle */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, 0);
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ 20:  /* tables_used.type */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCur->zType, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 21:  /* tables_used.schema */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 22:  /* tables_used.name */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCur->zName, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 23:  /* tables_used.wr */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pOp->opcode==OP_OpenWrite);
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  (ಶೂನ್ಯ)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_ಅಂಕೆ==0;
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_ಪಠ್ಯ ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಆದರೆ( zSql==0 ){
      rc = SQLITE_NOMEM;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ(argv[0],"stmt-pointer");
  }
  ಆದರೆ( pCur->pStmt==0 ){
    pVTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_ಬಳಸಿದ ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    bytecodevtab_ಮುಂದಿನ(pVtabCursor);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bytecodevtab_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(
  sqlite3_vtab *tab,
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_CONSTRAINT;
  ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  ಪೂರ್ಣಾಂಕ i_ತಳ_ಲಂಬಸಾಲು = pVTab->b_Tables_ಬಳಸಿದ ? 4 : 10;
  pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)100;
  pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 100;
  pIdxInfo->idx_ಅಂಕೆ = 0;
  ಗಾಗಿ(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    ಆದರೆ( p->usable==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ && p->i_ಸ್ತಂಭ==i_ತಳ_ಲಂಬಸಾಲು+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_ಸೂಚ್ಯಂಕ = 1;
    }
    ಆದರೆ( p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNULL && p->i_ಸ್ತಂಭ==i_ತಳ_ಲಂಬಸಾಲು ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
ಸ್ಥಿರ sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_ಮುಂದಿನ,
  /* xEof        */ bytecodevtabEof,
  /* xColumn     */ bytecodevtab_ಸ್ತಂಭ,
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


ಪೂರ್ಣಾಂಕ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_BYTECODE_VTAB)
ಪೂರ್ಣಾಂಕ sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_BYTECODE_VTAB */
