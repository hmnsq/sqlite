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
#यदि defined(SQLITE_ENABLE_BYTECODE_VTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
#अन्तर्भाव "vdbeInt.h"

/* An instance of the bytecode() table-valued function.
*/
प्रकारनिर्देश संरचना bytecodevtab bytecodevtab;
संरचना bytecodevtab {
  sqlite3_vtab आधारः;     /* Base class - must be first */
  sqlite3 *db;           /* Database connection */
  पूर्णाङ्क b_Tables_प्रयुक्तम्;       /* 2 for tables_used().  0 for bytecode(). */
};

/* A cursor for scanning through the bytecode
*/
प्रकारनिर्देश संरचना bytecodevtab_cursor bytecodevtab_cursor;
संरचना bytecodevtab_cursor {
  sqlite3_vtab_cursor आधारः;  /* Base class - must be first */
  sqlite3_stmt *pStmt;       /* The statement whose bytecode is displayed */
  पूर्णाङ्क iRowid;                /* The rowid of the output table */
  पूर्णाङ्क iAddr;                 /*स्थानाङ्कः*/
  पूर्णाङ्क needFinalize;          /* Cursors owns pStmt and must finalize it */
  पूर्णाङ्क showSubprograms;       /* Provide a listing of subprograms */
  Op *aOp;                   /* Operand array */
  वर्ण *zP4;                 /* Rendered P4 value */
  ध्रुव वर्ण *zType;         /* tables_used.type */
  ध्रुव वर्ण *zSchema;       /* tables_used.schema */
  ध्रुव वर्ण *zName;         /* tables_used.name */
  Mem sub;                   /* Subprograms */
};

/*
** Create a new bytecode() table-valued function.
*/
स्थिर पूर्णाङ्क bytecodevtabConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  bytecodevtab *pNew;
  पूर्णाङ्क rc;
  पूर्णाङ्क is_Tab_प्रयुक्तम् = pAux!=0;
  ध्रुव वर्ण *azSchema[2] = {
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
  rc = sqlite3_declare_vtab(db, azSchema[is_Tab_प्रयुक्तम्]);
  यदि( rc==SQLITE_OK ){
    pNew = sqlite3_malloc( परिमाण(*pNew) );
    *ppVtab = (sqlite3_vtab*)pNew;
    यदि( pNew==0 ) प्रत्यागम SQLITE_NOMEM;
    स्मृतिं_पूरय(pNew, 0, परिमाण(*pNew));
    pNew->db = db;
    pNew->b_Tables_प्रयुक्तम् = is_Tab_प्रयुक्तम्*2;
  }
  प्रत्यागम rc;
}

/*
** This method is the destructor for bytecodevtab objects.
*/
स्थिर पूर्णाङ्क bytecodevtabDisconnect(sqlite3_vtab *pVtab){
  bytecodevtab *p = (bytecodevtab*)pVtab;
  sqlite3_free(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Constructor for a new bytecodevtab_cursor object.
*/
स्थिर पूर्णाङ्क bytecodevtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bytecodevtab *pVTab = (bytecodevtab*)p;
  bytecodevtab_cursor *pCur;
  pCur = sqlite3_malloc( परिमाण(*pCur) );
  यदि( pCur==0 ) प्रत्यागम SQLITE_NOMEM;
  स्मृतिं_पूरय(pCur, 0, परिमाण(*pCur));
  sqlite3VdbeMemInit(&pCur->sub, pVTab->db, 1);
  *ppCursor = &pCur->आधारः;
  प्रत्यागम SQLITE_OK;
}

/*
** Clear all internal content from a bytecodevtab cursor.
*/
स्थिर शून्य bytecodevtabCursorClear(bytecodevtab_cursor *pCur){
  sqlite3_free(pCur->zP4);
  pCur->zP4 = 0;
  sqlite3VdbeMemRelease(&pCur->sub);
  sqlite3_Vdbe_Mem_समुच्चयः_Null(&pCur->sub);
  यदि( pCur->needFinalize ){
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
स्थिर पूर्णाङ्क bytecodevtabClose(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtabCursorClear(pCur);
  sqlite3_free(pCur);
  प्रत्यागम SQLITE_OK;
}


/*
** Advance a bytecodevtab_cursor to its next row of output.
*/
स्थिर पूर्णाङ्क bytecodevtab_अग्र(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pTab = (bytecodevtab*)cur->pVtab;
  पूर्णाङ्क rc;
  यदि( pCur->zP4 ){
    sqlite3_free(pCur->zP4);
    pCur->zP4 = 0;
  }
  यदि( pCur->zName ){
    pCur->zName = 0;
    pCur->zType = 0;
    pCur->zSchema = 0;
  }
  rc = sqlite3_Vdbe_अग्र_Opcode(
           (Vdbe*)pCur->pStmt, 
           pCur->showSubprograms ? &pCur->sub : 0,
           pTab->b_Tables_प्रयुक्तम्,
           &pCur->iRowid,
           &pCur->iAddr,
           &pCur->aOp);
  यदि( rc!=SQLITE_OK ){
    sqlite3_Vdbe_Mem_समुच्चयः_Null(&pCur->sub);
    pCur->aOp = 0;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
स्थिर पूर्णाङ्क bytecodevtabEof(sqlite3_vtab_cursor *cur){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  प्रत्यागम pCur->aOp==0;
}

/*
** Return values of columns for the row at which the bytecodevtab_cursor
** is currently pointing.
*/
स्थिर पूर्णाङ्क bytecodevtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णाङ्क i                       /* Which column to return */
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  bytecodevtab *pVTab = (bytecodevtab*)cur->pVtab;
  Op *pOp = pCur->aOp + pCur->iAddr;
  यदि( pVTab->b_Tables_प्रयुक्तम् ){
    यदि( i==4 ){
      i = 8;
    }अन्यथा{
      यदि( i<=2 && pCur->zType==0 ){
        Schema *pSchema;
        HashElem *k;
        पूर्णाङ्क iDb = pOp->p3;
        Pgno i_मूलम् = (Pgno)pOp->p2;
        sqlite3 *db = pVTab->db;
        pSchema = db->aDb[iDb].pSchema;
        pCur->zSchema = db->aDb[iDb].zDbSName;
        प्रति(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_अग्र(k)){
          सारणी *pTab = (सारणी*)sqlite_Hash_दत्त(k);
          यदि( !IsVirtual(pTab) && pTab->tnum==i_मूलम् ){
            pCur->zName = pTab->zName;
            pCur->zType = "table";
            भङ्ग;
          }
        }
        यदि( pCur->zName==0 ){
          प्रति(k=sqliteHashFirst(&pSchema->idxHash); k; k=sqlite_Hash_अग्र(k)){
            सूचकांक *pIdx = (सूचकांक*)sqlite_Hash_दत्त(k);
            यदि( pIdx->tnum==i_मूलम् ){
              pCur->zName = pIdx->zName;
              pCur->zType = "index";
            }
          }
        }
      }
      i += 20;
    }
  }
  विकल्प( i ){
    पक्ष 0:   /* addr */
      sqlite3_परिणाम_int(ctx, pCur->iAddr);
      भङ्ग;
    पक्ष 1:   /* opcode */
      sqlite3_परिणाम_text(ctx, (वर्ण*)sqlite3OpcodeName(pOp->opcode),
                          -1, SQLITE_STATIC);
      भङ्ग;
    पक्ष 2:   /* p1 */
      sqlite3_परिणाम_int(ctx, pOp->p1);
      भङ्ग;
    पक्ष 3:   /* p2 */
      sqlite3_परिणाम_int(ctx, pOp->p2);
      भङ्ग;
    पक्ष 4:   /* p3 */
      sqlite3_परिणाम_int(ctx, pOp->p3);
      भङ्ग;
    पक्ष 5:   /* p4 */
    पक्ष 7:   /* comment */
      यदि( pCur->zP4==0 ){
        pCur->zP4 = sqlite3VdbeDisplayP4(pVTab->db, pOp);
      }
      यदि( i==5 ){
        sqlite3_परिणाम_text(ctx, pCur->zP4, -1, SQLITE_STATIC);
      }अन्यथा{
#यदि_परिभाषितम् SQLITE_ENABLE_EXPLAIN_COMMENTS
        वर्ण *zCom = sqlite3VdbeDisplayComment(pVTab->db, pOp, pCur->zP4);
        sqlite3_परिणाम_text(ctx, zCom, -1, sqlite3_free);
#यदि_अन्तः
      }
      भङ्ग;
    पक्ष 6:     /* p5 */
      sqlite3_परिणाम_int(ctx, pOp->p5);
      भङ्ग;
    पक्ष 8: {   /* subprog */
      Op *aOp = pCur->aOp;
      प्रतिज्ञा( aOp[0].opcode==OP_Init );
      प्रतिज्ञा( aOp[0].p4.z==0 || वाक्ये_तुलय(aOp[0].p4.z,"-" "- ",3)==0 );
      यदि( pCur->iRowid==pCur->iAddr+1 ){
        भङ्ग;  /* Result is NULL for the main program */
      }अन्यथा यदि( aOp[0].p4.z!=0 ){
         sqlite3_परिणाम_text(ctx, aOp[0].p4.z+3, -1, SQLITE_STATIC);
      }अन्यथा{
         sqlite3_परिणाम_text(ctx, "(FK)", 4, SQLITE_STATIC);
      }
      भङ्ग;
    }

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
    पक्ष 9:     /* nexec */
      sqlite3_परिणाम_int64(ctx, pOp->nExec);
      भङ्ग;
    पक्ष 10:    /* ncycle */
      sqlite3_परिणाम_int64(ctx, pOp->n_आवृत्तिः);
      भङ्ग;
#अन्यथा
    पक्ष 9:     /* nexec */
    पक्ष 10:    /* ncycle */
      sqlite3_परिणाम_int(ctx, 0);
      भङ्ग;
#यदि_अन्तः

    पक्ष 20:  /* tables_used.type */
      sqlite3_परिणाम_text(ctx, pCur->zType, -1, SQLITE_STATIC);
      भङ्ग;
    पक्ष 21:  /* tables_used.schema */
      sqlite3_परिणाम_text(ctx, pCur->zSchema, -1, SQLITE_STATIC);
      भङ्ग;
    पक्ष 22:  /* tables_used.name */
      sqlite3_परिणाम_text(ctx, pCur->zName, -1, SQLITE_STATIC);
      भङ्ग;
    पक्ष 23:  /* tables_used.wr */
      sqlite3_परिणाम_int(ctx, pOp->opcode==OP_OpenWrite);
      भङ्ग;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
स्थिर पूर्णाङ्क bytecodevtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor*)cur;
  *pRowid = pCur->iRowid;
  प्रत्यागम SQLITE_OK;
}

/*
** Initialize a cursor.
**
**    idxNum==0     means show all subprograms
**    idxNum==1     means show only the main bytecode and omit subprograms.
*/
स्थिर पूर्णाङ्क bytecodevtabFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अङ्कः, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मानम् **argv
){
  bytecodevtab_cursor *pCur = (bytecodevtab_cursor *)pVtabCursor;
  bytecodevtab *pVTab = (bytecodevtab *)pVtabCursor->pVtab;
  पूर्णाङ्क rc = SQLITE_OK;
  (शून्य)idxStr;

  bytecodevtabCursorClear(pCur);
  pCur->iRowid = 0;
  pCur->iAddr = 0;
  pCur->showSubprograms = idx_अङ्कः==0;
  प्रतिज्ञा( argc==1 );
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_TEXT ){
    ध्रुव वर्ण *zSql = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
    यदि( zSql==0 ){
      rc = SQLITE_NOMEM;
    }अन्यथा{
      rc = sqlite3_prepare_v2(pVTab->db, zSql, -1, &pCur->pStmt, 0);
      pCur->needFinalize = 1;
    }
  }अन्यथा{
    pCur->pStmt = (sqlite3_stmt*)sqlite3_मानम्_pointer(argv[0],"stmt-pointer");
  }
  यदि( pCur->pStmt==0 ){
    pVTab->आधारः.z_Err_सन्देशः = sqlite3_mprintf(
       "argument to %s() is not a valid SQL statement",
       pVTab->b_Tables_प्रयुक्तम् ? "tables_used" : "bytecode"
    );
    rc = SQLITE_ERROR;
  }अन्यथा{
    bytecodevtab_अग्र(pVtabCursor);
  }
  प्रत्यागम rc;
}

/*
** We must have a single stmt=? constraint that will be passed through
** into the xFilter method.  If there is no valid stmt=? constraint,
** then return an SQLITE_CONSTRAINT error.
*/
स्थिर पूर्णाङ्क bytecodevtab_Best_सूचकांक(
  sqlite3_vtab *tab,
  sqlite3_सूचकांक_info *pIdxInfo
){
  पूर्णाङ्क i;
  पूर्णाङ्क rc = SQLITE_CONSTRAINT;
  संरचना sqlite3_सूचकांक_constraint *p;
  bytecodevtab *pVTab = (bytecodevtab*)tab;
  पूर्णाङ्क i_आधारः_Col = pVTab->b_Tables_प्रयुक्तम् ? 4 : 10;
  pIdxInfo->estimatedCost = (द्विगुण)100;
  pIdxInfo->estimated_पङ्क्तिकाः = 100;
  pIdxInfo->idx_अङ्कः = 0;
  प्रति(i=0, p=pIdxInfo->aConstraint; i<pIdxInfo->nConstraint; i++, p++){
    यदि( p->usable==0 ) अनुवर्त;
    यदि( p->op==SQLITE_सूचकांक_CONSTRAINT_EQ && p->iColumn==i_आधारः_Col+1 ){
      rc = SQLITE_OK;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->aConstraintUsage[i].argv_सूचकांक = 1;
    }
    यदि( p->op==SQLITE_सूचकांक_CONSTRAINT_ISNULL && p->iColumn==i_आधारः_Col ){
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idx_अङ्कः = 1;
    }
  }
  प्रत्यागम rc;
}

/*
** This following structure defines all the methods for the 
** virtual table.
*/
स्थिर sqlite3_module bytecodevtabModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ bytecodevtabConnect,
  /* xBestIndex  */ bytecodevtab_Best_सूचकांक,
  /* xDisconnect */ bytecodevtabDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ bytecodevtabOpen,
  /* xClose      */ bytecodevtabClose,
  /* xFilter     */ bytecodevtabFilter,
  /* xNext       */ bytecodevtab_अग्र,
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


पूर्णाङ्क sqlite3VdbeBytecodeVtabInit(sqlite3 *db){
  पूर्णाङ्क rc;
  rc = sqlite3_create_module(db, "bytecode", &bytecodevtabModule, 0);
  यदि( rc==SQLITE_OK ){
    rc = sqlite3_create_module(db, "tables_used", &bytecodevtabModule, &db);
  }
  प्रत्यागम rc;
}
#नोचेद्यदि defined(SQLITE_ENABLE_BYTECODE_VTAB)
पूर्णाङ्क sqlite3VdbeBytecodeVtabInit(sqlite3 *db){ प्रत्यागम SQLITE_OK; }
#यदि_अन्तः /* SQLITE_ENABLE_BYTECODE_VTAB */
