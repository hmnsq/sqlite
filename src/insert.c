/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the parser
** to handle INSERT statements in SQLite.
*/
#include "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
void sqlite3OpenTable(
  Parse *pParse,  /* Generate code into this VDBE */
  පූර්ණ_සංඛ්‍යා iCur,       /* The cursor number of the table */
  පූර්ණ_සංඛ්‍යා iDb,        /* The database index in sqlite3.aDb[] */
  Table *pTab,    /* The table to be opened */
  පූර්ණ_සංඛ්‍යා opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  assert( !IsVirtual(pTab) );
  assert( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  assert( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  නම්( !pParse->db->noSharedCache ){
    sqlite3TableLock(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  නම්( HasRowid(pTab) ){
    sqlite3VdbeAddOp4Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }නැතහොත්{
    Index *pPk = sqlite3PrimaryKeyIndex(pTab);
    assert( pPk!=0 );
    assert( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3VdbeAddOp3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3VdbeSetP4KeyInfo(pParse, pPk);
    VdbeComment((v, "%s", pTab->zName));
  }
}

/*
** Return a pointer to the column affinity string associated with index
** pIdx. A column affinity string has one character for each column in
** the table, according to the affinity of the column:
**
**  Character      Column affinity
**  ------------------------------
**  'A'            BLOB
**  'B'            TEXT
**  'C'            NUMERIC
**  'D'            INTEGER
**  'F'            REAL
**
** An extra 'D' is appended to the end of the string to cover the
** rowid that appears as the last column in every index.
**
** Memory for the buffer containing the column index affinity string
** is managed along with the rest of the Index structure. It will be
** released when sqlite3DeleteIndex() is called.
*/
static SQLITE_NOINLINE const var_na *computeIndexAffStr(sqlite3 *db, Index *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  පූර්ණ_සංඛ්‍යා n;
  Table *pTab = pIdx->pTable;
  pIdx->zColAff = (var_na *)sqlite3DbMallocRaw(0, pIdx->nColumn+1);
  නම්( !pIdx->zColAff ){
    sqlite3OomFault(db);
    ආපසු 0;
  }
  සඳහා(n=0; n<pIdx->nColumn; n++){
    i16 x = pIdx->aiColumn[n];
    var_na aff;
    නම්( x>=0 ){
      aff = pTab->aCol[x].affinity;
    }නැතහොත් නම්( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }නැතහොත්{
      assert( x==XN_EXPR );
      assert( pIdx->bHasExpr );
      assert( pIdx->aColExpr!=0 );
      aff = sqlite3ExprAffinity(pIdx->aColExpr->a[n].pExpr);
    }
    නම්( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    නම්( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->zColAff[n] = aff;
  }
  pIdx->zColAff[n] = 0;
  ආපසු pIdx->zColAff;
}
const var_na *sqlite3IndexAffinityStr(sqlite3 *db, Index *pIdx){
  නම්( !pIdx->zColAff ) ආපසු computeIndexAffStr(db, pIdx);
  ආපසු pIdx->zColAff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
var_na *sqlite3TableAffinityStr(sqlite3 *db, const Table *pTab){
  var_na *zColAff;
  zColAff = (var_na *)sqlite3DbMallocRaw(db, pTab->nCol+1);
  නම්( zColAff ){
    පූර්ණ_සංඛ්‍යා i, j;
    සඳහා(i=j=0; i<pTab->nCol; i++){
      නම්( (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        zColAff[j++] = pTab->aCol[i].affinity;
      }
    }
    do{
      zColAff[j--] = 0;
    }අතරතුර( j>=0 && zColAff[j]<=SQLITE_AFF_BLOB );
  }
  ආපසු zColAff; 
}

/*
** Make changes to the evolving bytecode to do affinity transformations
** of values that are about to be gathered into a row for table pTab.
**
** For ordinary (legacy, non-strict) tables:
** -----------------------------------------
**
** Compute the affinity string for table pTab, if it has not already been
** computed.  As an optimization, omit trailing SQLITE_AFF_BLOB affinities.
**
** If the affinity string is empty (because it was all SQLITE_AFF_BLOB entries
** which were then optimized out) then this routine becomes a no-op.
**
** Otherwise if iReg>0 then code an OP_Affinity opcode that will set the
** affinities for register iReg and following.  Or if iReg==0,
** then just set the P4 operand of the previous opcode (which should  be
** an OP_MakeRecord) to the affinity string.
**
** A column affinity string has one character per column:
**
**    Character      Column affinity
**    ---------      ---------------
**    'A'            BLOB
**    'B'            TEXT
**    'C'            NUMERIC
**    'D'            INTEGER
**    'E'            REAL
**
** For STRICT tables:
** ------------------
**
** Generate an appropriate OP_TypeCheck opcode that will verify the
** datatypes against the column definitions in pTab.  If iReg==0, that
** means an OP_MakeRecord opcode has already been generated and should be
** the last opcode generated.  The new OP_TypeCheck needs to be inserted
** before the OP_MakeRecord.  The new OP_TypeCheck should use the same
** register set as the OP_MakeRecord.  If iReg>0 then register iReg is
** the first of a series of registers that will form the new record.
** Apply the type checking to that array of registers.
*/
void sqlite3TableAffinity(Vdbe *v, Table *pTab, පූර්ණ_සංඛ්‍යා iReg){
  පූර්ණ_සංඛ්‍යා i;
  var_na *zColAff;
  නම්( pTab->tabFlags & TF_Strict ){
    නම්( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *pPrev;
      පූර්ණ_සංඛ්‍යා p3;
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
      pPrev = sqlite3VdbeGetLastOp(v);
      assert( pPrev!=0 );
      assert( pPrev->opcode==OP_MakeRecord || sqlite3VdbeDb(v)->mallocFailed );
      pPrev->opcode = OP_TypeCheck;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3VdbeAddOp3(v, OP_MakeRecord, pPrev->p1, pPrev->p2, p3);
    }නැතහොත්{
      /* Insert an isolated OP_Typecheck */
      sqlite3VdbeAddOp2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
    ආපසු;
  }
  zColAff = pTab->zColAff;
  නම්( zColAff==0 ){
    zColAff = sqlite3TableAffinityStr(0, pTab);
    නම්( !zColAff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      ආපසු;
    }
    pTab->zColAff = zColAff;
  }
  assert( zColAff!=0 );
  i = sqlite3Strlen30NN(zColAff);
  නම්( i ){
    නම්( iReg ){
      sqlite3VdbeAddOp4(v, OP_Affinity, iReg, i, 0, zColAff, i);
    }නැතහොත්{
      assert( sqlite3VdbeGetLastOp(v)->opcode==OP_MakeRecord
              || sqlite3VdbeDb(v)->mallocFailed );
      sqlite3VdbeChangeP4(v, -1, zColAff, i);
    }
  }
}

/*
** Return non-zero if the table pTab in database iDb or any of its indices
** have been opened at any point in the VDBE program. This is used to see if
** a statement of the form  "INSERT INTO <iDb, pTab> SELECT ..." can
** run without using a temporary table for the results of the SELECT.
*/
static පූර්ණ_සංඛ්‍යා readsTable(Parse *p, පූර්ණ_සංඛ්‍යා iDb, Table *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා iEnd = sqlite3VdbeCurrentAddr(v);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#endif

  සඳහා(i=1; i<iEnd; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    assert( pOp!=0 );
    නම්( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      Index *pIndex;
      Pgno tnum = pOp->p2;
      නම්( tnum==pTab->tnum ){
        ආපසු 1;
      }
      සඳහා(pIndex=pTab->pIndex; pIndex; pIndex=pIndex->pNext){
        නම්( tnum==pIndex->tnum ){
          ආපසු 1;
        }
      }
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    නම්( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      assert( pOp->p4.pVtab!=0 );
      assert( pOp->p4type==P4_VTAB );
      ආපසු 1;
    }
#endif
  }
  ආපසු 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
static පූර්ණ_සංඛ්‍යා exprColumnFlagUnion(Walker *pWalker, Expr *pExpr){
  නම්( pExpr->op==TK_COLUMN && pExpr->iColumn>=0 ){
    assert( pExpr->iColumn < pWalker->u.pTab->nCol );
    pWalker->eCode |= pWalker->u.pTab->aCol[pExpr->iColumn].colFlags;
  }
  ආපසු WRC_Continue;
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
void sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  පූර්ණ_සංඛ්‍යා iRegStore,    /* Register holding the first column */
  Table *pTab       /* The table */
){
  පූර්ණ_සංඛ්‍යා i;
  Walker w;
  Column *pRedo;
  පූර්ණ_සංඛ්‍යා eProgress;
  VdbeOp *pOp;

  assert( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3TableAffinity(pParse->pVdbe, pTab, iRegStore);
  නම්( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    නම්( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      පූර්ණ_සංඛ්‍යා ii, jj;
      var_na *zP4 = pOp->p4.z;
      assert( zP4!=0 );
      assert( pOp->p4type==P4_DYNAMIC );
      සඳහා(ii=jj=0; zP4[jj]; ii++){
        නම්( pTab->aCol[ii].colFlags & COLFLAG_VIRTUAL ){
          continue;
        }
        නම්( pTab->aCol[ii].colFlags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }නැතහොත් නම්( pOp->opcode==OP_TypeCheck ){
      /* If an OP_TypeCheck was generated because the table is STRICT,
      ** then set the P3 operand to indicate that generated columns should
      ** not be checked */
      pOp->p3 = 1;
    }
  }

  /* Because there can be multiple generated columns that refer to one another,
  ** this is a two-pass algorithm.  On the first pass, mark all generated
  ** columns as "not available".
  */
  සඳහා(i=0; i<pTab->nCol; i++){
    නම්( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
      testcase( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL );
      testcase( pTab->aCol[i].colFlags & COLFLAG_STORED );
      pTab->aCol[i].colFlags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.xExprCallback = exprColumnFlagUnion;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  do{
    eProgress = 0;
    pRedo = 0;
    සඳහා(i=0; i<pTab->nCol; i++){
      Column *pCol = pTab->aCol + i;
      නම්( (pCol->colFlags & COLFLAG_NOTAVAIL)!=0 ){
        පූර්ණ_සංඛ්‍යා x;
        pCol->colFlags |= COLFLAG_BUSY;
        w.eCode = 0;
        sqlite3WalkExpr(&w, sqlite3ColumnExpr(pTab, pCol));
        pCol->colFlags &= ~COLFLAG_BUSY;
        නම්( w.eCode & COLFLAG_NOTAVAIL ){
          pRedo = pCol;
          continue;
        }
        eProgress = 1;
        assert( pCol->colFlags & COLFLAG_GENERATED );
        x = sqlite3TableColumnToStorage(pTab, i) + iRegStore;
        sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, x);
        pCol->colFlags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }අතරතුර( pRedo && eProgress );
  නම්( pRedo ){
    sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */


#ifndef SQLITE_OMIT_AUTOINCREMENT
/*
** Locate or create an AutoincInfo structure associated with table pTab
** which is in database iDb.  Return the register number for the register
** that holds the maximum rowid.  Return zero if pTab is not an AUTOINCREMENT
** table.  (Also return zero when doing a VACUUM since we do not want to
** update the AUTOINCREMENT counters during a VACUUM.)
**
** There is at most one AutoincInfo structure per table even if the
** same table is autoincremented multiple times due to inserts within
** triggers.  A new AutoincInfo structure is created if this is the
** first use of table pTab.  On 2nd and subsequent uses, the original
** AutoincInfo structure is used.
**
** Four consecutive registers are allocated:
**
**   (1)  The name of the pTab table.
**   (2)  The maximum ROWID of pTab.
**   (3)  The rowid in sqlite_sequence of pTab
**   (4)  The original value of the max ROWID in pTab, or NULL if none
**
** The 2nd register is the one that is returned.  That is all the
** insert routine needs to know about.
*/
static පූර්ණ_සංඛ්‍යා autoIncBegin(
  Parse *pParse,      /* Parsing context */
  පූර්ණ_සංඛ්‍යා iDb,            /* Index of the database holding pTab */
  Table *pTab         /* The table we are writing to */
){
  පූර්ණ_සංඛ්‍යා memId = 0;      /* Register holding maximum rowid */
  assert( pParse->db->aDb[iDb].pSchema!=0 );
  නම්( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    Table *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    නම්( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->nCol!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      ආපසු 0;
    }

    නම්( pToplevel->usesAinc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    අතරතුර( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->pNext; }
    නම්( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, parimaa_na(*pInfo));
      sqlite3ParserAddCleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      නම්( pParse->db->mallocFailed ) ආපසු 0;
      pInfo->pNext = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pToplevel->usesAinc = 1;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;                  /* Register to hold name of table */
      pInfo->regCtr = ++pToplevel->nMem;  /* Max rowid register */
      pToplevel->nMem +=2;       /* Rowid in sqlite_sequence + orig max val */
    }
    memId = pInfo->regCtr;
  }
  ආපසු memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
void sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  පූර්ණ_සංඛ්‍යා memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  assert( pParse->pTriggerTab==0 );
  assert( sqlite3IsToplevel(pParse) );

  assert( v );   /* We failed long ago if this is not so */
  assert( pParse->usesAinc );
  සඳහා(p = pParse->pAinc; p; p = p->pNext){
    static const පූර්ණ_සංඛ්‍යා iLn = VDBE_OFFSET_LINENO(2);
    static const VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_Column,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_Column,  0,  1, 0},
      /* 6  */ {OP_AddImm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_Next,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    assert( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3VdbeLoadString(v, memId-1, p->pTab->zName);
    aOp = sqlite3VdbeAddOpList(v, ArraySize(autoInc), autoInc, iLn);
    නම්( aOp==0 ) bha_nga;
    aOp[0].p2 = memId;
    aOp[0].p3 = memId+2;
    aOp[2].p3 = memId;
    aOp[3].p1 = memId-1;
    aOp[3].p3 = memId;
    aOp[3].p5 = SQLITE_JUMPIFNULL;
    aOp[4].p2 = memId+1;
    aOp[5].p3 = memId;
    aOp[6].p1 = memId;
    aOp[7].p2 = memId+2;
    aOp[7].p1 = memId;
    aOp[10].p2 = memId;
    නම්( pParse->nTab==0 ) pParse->nTab = 1;
  }
}

/*
** Update the maximum rowid for an autoincrement calculation.
**
** This routine should be called when the regRowid register holds a
** new rowid that is about to be inserted.  If that new rowid is
** larger than the maximum rowid in the memId memory cell, then the
** memory cell is updated.
*/
static void autoIncStep(Parse *pParse, පූර්ණ_සංඛ්‍යා memId, පූර්ණ_සංඛ්‍යා regRowid){
  නම්( memId>0 ){
    sqlite3VdbeAddOp2(pParse->pVdbe, OP_MemMax, memId, regRowid);
  }
}

/*
** This routine generates the code needed to write autoincrement
** maximum rowid values back into the sqlite_sequence register.
** Every statement that might do an INSERT into an autoincrement
** table (either directly or through triggers) needs to call this
** routine just before the "exit" code.
*/
static SQLITE_NOINLINE void autoIncrementEnd(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  assert( v );
  assert( pParse->usesAinc );
  සඳහා(p = pParse->pAinc; p; p = p->pNext){
    static const පූර්ණ_සංඛ්‍යා iLn = VDBE_OFFSET_LINENO(2);
    static const VdbeOpList autoIncEnd[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_MakeRecord,  0, 2, 0},
      /* 3 */ {OP_Insert,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    පූර්ණ_සංඛ්‍යා iRec;
    පූර්ණ_සංඛ්‍යා memId = p->regCtr;

    iRec = sqlite3GetTempReg(pParse);
    assert( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3VdbeAddOp3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3VdbeAddOpList(v, ArraySize(autoIncEnd), autoIncEnd, iLn);
    නම්( aOp==0 ) bha_nga;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3ReleaseTempReg(pParse, iRec);
  }
}
void sqlite3AutoincrementEnd(Parse *pParse){
  නම්( pParse->usesAinc ) autoIncrementEnd(pParse);
}
#නැතහොත්
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define autoIncStep(A,B,C)
#endif /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
void sqlite3MultiValuesEnd(Parse *pParse, Select *pVal){
  නම්( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    SrcItem *pItem = &pVal->pSrc->a[0];
    assert( (pItem->fg.isSubquery && pItem->u4.pSubq!=0) || pParse->nErr );
    නම්( pItem->fg.isSubquery ){
      sqlite3VdbeEndCoroutine(pParse->pVdbe, pItem->u4.pSubq->regReturn);
      sqlite3VdbeJumpHere(pParse->pVdbe, pItem->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
static පූර්ණ_සංඛ්‍යා exprListIsNoAffinity(Parse *pParse, ExprList *pRow){
  පූර්ණ_සංඛ්‍යා ii;
  නම්( sqlite3ExprListIsConstant(pParse, pRow, 0)==0 ) ආපසු 0;
  සඳහා(ii=0; ii<pRow->nExpr; ii++){
    Expr *pExpr = pRow->a[ii].pExpr;
    assert( pExpr->op!=TK_RAISE );
    assert( pExpr->affExpr==0 );
    නම්( 0!=sqlite3ExprAffinity(pExpr) ) ආපසු 0;
  }
  ආපසු 1;

}

/*
** This function is called by the parser for the second and subsequent
** rows of a multi-row VALUES clause. Argument pLeft is the part of
** the VALUES clause already parsed, argument pRow is the vector of values
** for the new row. The Select object returned represents the complete
** VALUES clause, including the new row.
**
** There are two ways in which this may be achieved - by incremental 
** coding of a co-routine (the "co-routine" method) or by returning a
** Select object equivalent to the following (the "UNION ALL" method):
**
**        "pLeft UNION ALL SELECT pRow"
**
** If the VALUES clause contains a lot of rows, this compound Select
** object may consume a lot of memory.
**
** When the co-routine method is used, each row that will be returned
** by the VALUES clause is coded into part of a co-routine as it is 
** passed to this function. The returned Select object is equivalent to:
**
**     SELECT * FROM (
**       Select object to read co-routine
**     )
**
** The co-routine method is used in most cases. Exceptions are:
**
**    a) If the current statement has a WITH clause. This is to avoid
**       statements like:
**
**            WITH cte AS ( VALUES('x'), ('y') ... )
**            SELECT * FROM cte AS a, cte AS b;
**
**       This will not work, as the co-routine uses a hard-coded register
**       for its OP_Yield instructions, and so it is not possible for two
**       cursors to iterate through it concurrently.
**
**    b) The schema is currently being parsed (i.e. the VALUES clause is part 
**       of a schema item like a VIEW or TRIGGER). In this case there is no VM
**       being generated when parsing is taking place, and so generating 
**       a co-routine is not possible.
**
**    c) There are non-constant expressions in the VALUES clause (e.g.
**       the VALUES clause is part of a correlated sub-query).
**
**    d) One or more of the values in the first row of the VALUES clause
**       has an affinity (i.e. is a CAST expression). This causes problems
**       because the complex rules SQLite uses (see function 
**       sqlite3SubqueryColumnTypes() in select.c) to determine the effective
**       affinity of such a column for all rows require access to all values in
**       the column simultaneously. 
*/
Select *sqlite3MultiValues(Parse *pParse, Select *pLeft, ExprList *pRow){

  නම්( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3ExprListIsConstant(pParse, pRow, 1)==0   /* condition (c) above */
   || (pLeft->pSrc->nSrc==0 &&
       exprListIsNoAffinity(pParse,pLeft->pEList)==0) /* condition (d) above */
   || IN_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    පූර්ණ_සංඛ්‍යා f = SF_Values | SF_MultiValue;
    නම්( pLeft->pSrc->nSrc ){
      sqlite3MultiValuesEnd(pParse, pLeft);
      f = SF_Values;
    }නැතහොත් නම්( pLeft->pPrior ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & pLeft->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, pRow, 0, 0, 0, 0, 0, f, 0);
    pLeft->selFlags &= ~(u32)SF_MultiValue;
    නම්( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->pPrior = pLeft;
      pLeft = pSelect;
    }
  }නැතහොත්{
    SrcItem *p = 0;               /* SrcItem that reads from co-routine */

    නම්( pLeft->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      නම්( (pParse->db->mDbFlags & DBFLAG_SchemaKnownOk)==0 ){
        sqlite3ReadSchema(pParse);
      }

      නම්( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->pPrior = pLeft->pPrior;
        pRet->op = pLeft->op;
        නම්( pRet->pPrior ) pRet->selFlags |= SF_Values;
        pLeft->pPrior = 0;
        pLeft->op = TK_SELECT;
        assert( pLeft->pNext==0 );
        assert( pRet->pNext==0 );
        p = &pRet->pSrc->a[0];
        p->fg.viaCoroutine = 1;
        p->iCursor = -1;
        assert( !p->fg.isIndexedBy && !p->fg.isTabFunc );
        p->u1.nRow = 2;
        නම්( sqlite3SrcItemAttachSubquery(pParse, p, pLeft, 0) ){
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3VdbeAddOp3(v, OP_InitCoroutine,
                            pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);

          /* Allocate registers for the output of the co-routine. Do so so
          ** that there are two unused registers immediately before those
          ** used by the co-routine. This allows the code in sqlite3Insert()
          ** to use these registers directly, instead of copying the output
          ** of the co-routine to a separate array for processing.  */
          dest.iSdst = pParse->nMem + 3; 
          dest.nSdst = pLeft->pEList->nExpr;
          pParse->nMem += 2 + dest.nSdst;

          pLeft->selFlags |= SF_MultiValue;
          sqlite3Select(pParse, pLeft, &dest);
          pSubq->regResult = dest.iSdst;
          assert( pParse->nErr || dest.iSdst>0 );
        }
        pLeft = pRet;
      }
    }නැතහොත්{
      p = &pLeft->pSrc->a[0];
      assert( !p->fg.isTabFunc && !p->fg.isIndexedBy );
      p->u1.nRow++;
    }
  
    නම්( pParse->nErr==0 ){
      Subquery *pSubq;
      assert( p!=0 );
      assert( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      assert( pSubq!=0 );
      assert( pSubq->pSelect!=0 );
      assert( pSubq->pSelect->pEList!=0 );
      නම්( pSubq->pSelect->pEList->nExpr!=pRow->nExpr ){
        sqlite3SelectWrongNumTermsError(pParse, pSubq->pSelect);
      }නැතහොත්{
        sqlite3ExprCodeExprList(pParse, pRow, pSubq->regResult, 0, 0);
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3ExprListDelete(pParse->db, pRow);
  }

  ආපසු pLeft;
}

/* Forward declaration */
static පූර්ණ_සංඛ්‍යා xferOptimization(
  Parse *pParse,        /* Parser context */
  Table *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  පූර්ණ_සංඛ්‍යා onError,          /* How to handle constraint errors */
  පූර්ණ_සංඛ්‍යා iDbDest           /* The database of pDest */
);

/*
** This routine is called to handle SQL of the following forms:
**
**    insert into TABLE (IDLIST) values(EXPRLIST),(EXPRLIST),...
**    insert into TABLE (IDLIST) select
**    insert into TABLE (IDLIST) default values
**
** The IDLIST following the table name is always optional.  If omitted,
** then a list of all (non-hidden) columns for the table is substituted.
** The IDLIST appears in the pColumn parameter.  pColumn is NULL if IDLIST
** is omitted.
**
** For the pSelect parameter holds the values to be inserted for the
** first two forms shown above.  A VALUES clause is really just short-hand
** for a SELECT statement that omits the FROM clause and everything else
** that follows.  If the pSelect parameter is NULL, that means that the
** DEFAULT VALUES form of the INSERT statement is intended.
**
** The code generated follows one of four templates.  For a simple
** insert with data coming from a single-row VALUES clause, the code executes
** once straight down through.  Pseudo-code follows (we call this
** the "1st template"):
**
**         open write cursor to <table> and its indices
**         put VALUES clause expressions into registers
**         write the resulting record into <table>
**         cleanup
**
** The three remaining templates assume the statement is of the form
**
**   INSERT INTO <table> SELECT ...
**
** If the SELECT clause is of the restricted form "SELECT * FROM <table2>" -
** in other words if the SELECT pulls all columns from a single table
** and there is no WHERE or LIMIT or GROUP BY or ORDER BY clauses, and
** if <table2> and <table1> are distinct tables but have identical
** schemas, including all the same indices, then a special optimization
** is invoked that copies raw records from <table2> over to <table1>.
** See the xferOptimization() function for the implementation of this
** template.  This is the 2nd template.
**
**         open a write cursor to <table>
**         open read cursor on <table2>
**         transfer all records in <table2> over to <table>
**         close cursors
**         foreach index on <table>
**           open a write cursor on the <table> index
**           open a read cursor on the corresponding <table2> index
**           transfer all records from the read to the write cursors
**           close cursors
**         end foreach
**
** The 3rd template is for when the second template does not apply
** and the SELECT clause does not read from <table> at any time.
** The generated code follows this template:
**
**         X <- A
**         goto B
**      A: setup for the SELECT
**         loop over the rows in the SELECT
**           load values into registers R..R+n
**           yield X
**         end loop
**         cleanup after the SELECT
**         end-coroutine X
**      B: open write cursor to <table> and its indices
**      C: yield X, at EOF goto D
**         insert the select result into <table> from R..R+n
**         goto C
**      D: cleanup
**
** The 4th template is used if the insert statement takes its
** values from a SELECT but the data is being inserted into a table
** that is also read as part of the SELECT.  In the third form,
** we have to use an intermediate table to store the results of
** the select.  The template is like this:
**
**         X <- A
**         goto B
**      A: setup for the SELECT
**         loop over the tables in the SELECT
**           load value into register R..R+n
**           yield X
**         end loop
**         cleanup after the SELECT
**         end co-routine R
**      B: open temp table
**      L: yield X, at EOF goto M
**         insert row from R..R+n into temp table
**         goto L
**      M: open write cursor to <table> and its indices
**         rewind temp table
**      C: loop over rows of intermediate table
**           transfer values form intermediate table into <table>
**         end loop
**      D: cleanup
*/
void sqlite3Insert(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *pColumn,      /* Column names corresponding to IDLIST, or NULL. */
  පූර්ණ_සංඛ්‍යා onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  Table *pTab;          /* The table to insert into.  aka TABLE */
  පූර්ණ_සංඛ්‍යා i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  Index *pIdx;          /* For looping over indices of the table */
  පූර්ණ_සංඛ්‍යා nColumn;          /* Number of columns in the data */
  පූර්ණ_සංඛ්‍යා nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  පූර්ණ_සංඛ්‍යා iDataCur = 0;     /* VDBE cursor that is the main data repository */
  පූර්ණ_සංඛ්‍යා iIdxCur = 0;      /* First index cursor */
  පූර්ණ_සංඛ්‍යා ipkColumn = -1;   /* Column that is the INTEGER PRIMARY KEY */
  පූර්ණ_සංඛ්‍යා endOfLoop;        /* Label for the end of the insertion loop */
  පූර්ණ_සංඛ්‍යා srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  පූර්ණ_සංඛ්‍යා addrInsTop = 0;   /* Jump to label "D" */
  පූර්ණ_සංඛ්‍යා addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  පූර්ණ_සංඛ්‍යා iDb;              /* Index of database holding TABLE */
  u8 useTempTable = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 bIdListInOrder;    /* True if IDLIST is in table order */
  ExprList *pList = 0;  /* List of VALUES() to be inserted  */
  පූර්ණ_සංඛ්‍යා iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  පූර්ණ_සංඛ්‍යා regFromSelect = 0;/* Base register for data coming from SELECT */
  පූර්ණ_සංඛ්‍යා regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  පූර්ණ_සංඛ්‍යා regRowCount = 0;  /* Memory cell used for the row counter */
  පූර්ණ_සංඛ්‍යා regIns;           /* Block of regs holding rowid+data being inserted */
  පූර්ණ_සංඛ්‍යා regRowid;         /* registers holding insert rowid */
  පූර්ණ_සංඛ්‍යා regData;          /* register holding first column to insert */
  පූර්ණ_සංඛ්‍යා *aRegIdx = 0;     /* One register allocated to each index */
  පූර්ණ_සංඛ්‍යා *aTabColMap = 0;  /* Mapping from pTab columns to pCol entries */

#ifndef SQLITE_OMIT_TRIGGER
  පූර්ණ_සංඛ්‍යා isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  පූර්ණ_සංඛ්‍යා tmask;                  /* Mask of trigger times */
#endif

  db = pParse->db;
  assert( db->pParse==pParse );
  නම්( pParse->nErr ){
    goto insert_cleanup;
  }
  assert( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  නම්( pSelect && (pSelect->selFlags & SF_Values)!=0 && pSelect->pPrior==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  assert( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  නම්( pTab==0 ){
    goto insert_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iDb<db->nDb );
  නම්( sqlite3AuthCheck(pParse, SQLITE_INSERT, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    goto insert_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_INSERT, 0, &tmask);
  isView = IsView(pTab);
#නැතහොත්
# define pTrigger 0
# define tmask 0
# define isView 0
#endif
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif
  assert( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#නම් TREETRACE_ENABLED
  නම්( sqlite3TreeTrace & 0x10000 ){
    sqlite3TreeViewLine(0, "In sqlite3Insert() at %s:%d", __FILE__, __LINE__);
    sqlite3TreeViewInsert(pParse->pWith, pTabList, pColumn, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#endif

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  නම්( sqlite3ViewGetColumnNames(pParse, pTab) ){
    goto insert_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  නම්( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    goto insert_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  නම්( v==0 ) goto insert_cleanup;
  නම්( pParse->nested==0 ) sqlite3VdbeCountChanges(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#ifndef SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  නම්( pColumn==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    assert( !pTrigger );
    assert( pList==0 );
    goto insert_end;
  }
#endif /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->nCol + 1;
  නම්( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  regData = regRowid+1;

  /* If the INSERT statement included an IDLIST term, then make sure
  ** all elements of the IDLIST really are columns of the table and
  ** remember the column indices.
  **
  ** If the table has an INTEGER PRIMARY KEY column and that column
  ** is named in the IDLIST, then record in the ipkColumn variable
  ** the index into IDLIST of the primary key column.  ipkColumn is
  ** the index of the primary key as it appears in IDLIST, not as
  ** is appears in the original table.  (The index of the INTEGER
  ** PRIMARY KEY in the original table is pTab->iPKey.)  After this
  ** loop, if ipkColumn==(-1), that means that integer primary key
  ** is unspecified, and hence the table is either WITHOUT ROWID or
  ** it will automatically generated an integer primary key.
  **
  ** bIdListInOrder is true if the columns in IDLIST are in storage
  ** order.  This enables an optimization that avoids shuffling the
  ** columns into storage order.  False negatives are harmless,
  ** but false positives will cause database corruption.
  */
  bIdListInOrder = (pTab->tabFlags & (TF_OOOHidden|TF_HasStored))==0;
  නම්( pColumn ){
    aTabColMap = sqlite3DbMallocZero(db, pTab->nCol*parimaa_na(පූර්ණ_සංඛ්‍යා));
    නම්( aTabColMap==0 ) goto insert_cleanup;
    සඳහා(i=0; i<pColumn->nId; i++){
      j = sqlite3ColumnIndex(pTab, pColumn->a[i].zName);
      නම්( j>=0 ){
        නම්( aTabColMap[j]==0 ) aTabColMap[j] = i+1;
        නම්( i!=j ) bIdListInOrder = 0;
        නම්( j==pTab->iPKey ){
          ipkColumn = i;  assert( !withoutRowid );
        }
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
        නම්( pTab->aCol[j].colFlags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3ErrorMsg(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->aCol[j].zCnName);
          goto insert_cleanup;
        }
#endif
      }නැතහොත්{
        නම්( sqlite3IsRowid(pColumn->a[i].zName) && !withoutRowid ){
          ipkColumn = i;
          bIdListInOrder = 0;
        }නැතහොත්{
          sqlite3ErrorMsg(pParse, "table %S has no column named %s",
              pTabList->a, pColumn->a[i].zName);
          pParse->checkSchema = 1;
          goto insert_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  නම්( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    පූර්ණ_සංඛ්‍යා rc;             /* Result code */

    නම්( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.viaCoroutine 
     && pSelect->pPrior==0
    ){
      SrcItem *pItem = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      assert( pItem->fg.isSubquery );
      pSubq = pItem->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->regResult;
      assert( pSubq->pSelect!=0 );
      assert( pSubq->pSelect->pEList!=0 );
      nColumn = pSubq->pSelect->pEList->nExpr;
      ExplainQueryPlan((pParse, 0, "SCAN %S", pItem));
      නම්( bIdListInOrder && nColumn==pTab->nCol ){
        regData = regFromSelect;
        regRowid = regData - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }නැතහොත්{
      පූර්ණ_සංඛ්‍යා addrTop;        /* Top of the co-routine */
      පූර්ණ_සංඛ්‍යා regYield = ++pParse->nMem;
      addrTop = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, addrTop);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = bIdListInOrder ? regData : 0;
      dest.nSdst = pTab->nCol;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      assert( db->pParse==pParse );
      නම්( rc || pParse->nErr ) goto insert_cleanup;
      assert( db->mallocFailed==0 );
      sqlite3VdbeEndCoroutine(v, regYield);
      sqlite3VdbeJumpHere(v, addrTop - 1);                       /* label B: */
      assert( pSelect->pEList );
      nColumn = pSelect->pEList->nExpr;
    }

    /* Set useTempTable to TRUE if the result of the SELECT statement
    ** should be written into a temporary table (template 4).  Set to
    ** FALSE if each output row of the SELECT can be written directly into
    ** the destination table (template 3).
    **
    ** A temp table must be used if the table being updated is also one
    ** of the tables being read by the SELECT statement.  Also use a
    ** temp table in the case of row triggers.
    */
    නම්( pTrigger || readsTable(pParse, iDb, pTab) ){
      useTempTable = 1;
    }

    නම්( useTempTable ){
      /* Invoke the coroutine to extract information from the SELECT
      ** and add it to a transient table srcTab.  The code generated
      ** here is from the 4th template:
      **
      **      B: open temp table
      **      L: yield X, goto M at EOF
      **         insert row from R..R+n into temp table
      **         goto L
      **      M: ...
      */
      පූර්ණ_සංඛ්‍යා regRec;          /* Register to hold packed record */
      පූර්ණ_සංඛ්‍යා regTempRowid;    /* Register to hold temp table ROWID */
      පූර්ණ_සංඛ්‍යා addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3GetTempReg(pParse);
      regTempRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, OP_OpenEphemeral, srcTab, nColumn);
      addrL = sqlite3VdbeAddOp1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regFromSelect, nColumn, regRec);
      sqlite3VdbeAddOp2(v, OP_NewRowid, srcTab, regTempRowid);
      sqlite3VdbeAddOp3(v, OP_Insert, srcTab, regRec, regTempRowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3VdbeJumpHere(v, addrL);
      sqlite3ReleaseTempReg(pParse, regRec);
      sqlite3ReleaseTempReg(pParse, regTempRowid);
    }
  }නැතහොත්{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    memset(&sNC, 0, parimaa_na(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    assert( useTempTable==0 );
    නම්( pList ){
      nColumn = pList->nExpr;
      නම්( sqlite3ResolveExprListNames(&sNC, pList) ){
        goto insert_cleanup;
      }
    }නැතහොත්{
      nColumn = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  නම්( pColumn==0 && nColumn>0 ){
    ipkColumn = pTab->iPKey;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    නම්( ipkColumn>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      සඳහා(i=ipkColumn-1; i>=0; i--){
        නම්( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          testcase( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL );
          testcase( pTab->aCol[i].colFlags & COLFLAG_STORED );
          ipkColumn--;
        }
      }
    }
#endif

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    assert( TF_HasHidden==COLFLAG_HIDDEN );
    assert( TF_HasGenerated==COLFLAG_GENERATED );
    assert( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    නම්( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      සඳහා(i=0; i<pTab->nCol; i++){
        නම්( pTab->aCol[i].colFlags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    නම්( nColumn!=(pTab->nCol-nHidden) ){
      sqlite3ErrorMsg(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->nCol-nHidden, nColumn);
     goto insert_cleanup;
    }
  }
  නම්( pColumn!=0 && nColumn!=pColumn->nId ){
    sqlite3ErrorMsg(pParse, "%d values for %d columns", nColumn, pColumn->nId);
    goto insert_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  නම්( (db->flags & SQLITE_CountRows)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    regRowCount = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regRowCount);
  }

  /* If this is not a view, open the table and and all indices */
  නම්( !isView ){
    පූර්ණ_සංඛ්‍යා nIdx;
    nIdx = sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &iDataCur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, parimaa_na(පූර්ණ_සංඛ්‍යා)*(nIdx+2));
    නම්( aRegIdx==0 ){
      goto insert_cleanup;
    }
    සඳහා(i=0, pIdx=pTab->pIndex; i<nIdx; pIdx=pIdx->pNext, i++){
      assert( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#ifndef SQLITE_OMIT_UPSERT
  නම්( pUpsert ){
    Upsert *pNx;
    නම්( IsVirtual(pTab) ){
      sqlite3ErrorMsg(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      goto insert_cleanup;
    }
    නම්( IsView(pTab) ){
      sqlite3ErrorMsg(pParse, "cannot UPSERT a view");
      goto insert_cleanup;
    }
    නම්( sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget) ){
      goto insert_cleanup;
    }
    pTabList->a[0].iCursor = iDataCur;
    pNx = pUpsert;
    do{
      pNx->pUpsertSrc = pTabList;
      pNx->regData = regData;
      pNx->iDataCur = iDataCur;
      pNx->iIdxCur = iIdxCur;
      නම්( pNx->pUpsertTarget ){
        නම්( sqlite3UpsertAnalyzeTarget(pParse, pTabList, pNx, pUpsert) ){
          goto insert_cleanup;
        }
      }
      pNx = pNx->pNextUpsert;
    }අතරතුර( pNx!=0 );
  }
#endif


  /* This is the top of the main insertion loop */
  නම්( useTempTable ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addrInsTop = sqlite3VdbeAddOp1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }නැතහොත් නම්( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, regData, pTab->nCol, 0, 0);
    addrInsTop = addrCont = sqlite3VdbeAddOp1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    නම්( ipkColumn>=0 ){
      /* tag-20191021-001: If the INTEGER PRIMARY KEY is being generated by the
      ** SELECT, go ahead and copy the value into the rowid slot now, so that
      ** the value does not get overwritten by a NULL at tag-20191021-002. */
      sqlite3VdbeAddOp2(v, OP_Copy, regFromSelect+ipkColumn, regRowid);
    }
  }

  /* Compute data for ordinary columns of the new entry.  Values
  ** are written in storage order into registers starting with regData.
  ** Only ordinary columns are computed in this loop. The rowid
  ** (if there is one) is computed later and generated columns are
  ** computed after the rowid since they might depend on the value
  ** of the rowid.
  */
  nHidden = 0;
  iRegStore = regData;  assert( regData==regRowid+1 );
  සඳහා(i=0; i<pTab->nCol; i++, iRegStore++){
    පූර්ණ_සංඛ්‍යා k;
    u32 colFlags;
    assert( i>=nHidden );
    නම්( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3VdbeAddOp1(v, OP_SoftNull, iRegStore);
      continue;
    }
    නම්( ((colFlags = pTab->aCol[i].colFlags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      නම්( (colFlags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        continue;
      }නැතහොත් නම්( (colFlags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        නම්( tmask & TRIGGER_BEFORE ){
          sqlite3VdbeAddOp1(v, OP_SoftNull, iRegStore);
        }
        continue;
      }නැතහොත් නම්( pColumn==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3ExprCodeFactorable(pParse,
            sqlite3ColumnExprAuth(pTab, &pTab->aCol[i], pParse),
            iRegStore);
        continue;
      }
    }
    නම්( pColumn ){
      j = aTabColMap[i];
      assert( j>=0 && j<=pColumn->nId );   
      නම්( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3ExprCodeFactorable(pParse,
            sqlite3ColumnExprAuth(pTab, &pTab->aCol[i], pParse),
            iRegStore);
        continue;
      }
      k = j - 1;
    }නැතහොත් නම්( nColumn==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3ExprCodeFactorable(pParse,
          sqlite3ColumnExprAuth(pTab, &pTab->aCol[i], pParse),
          iRegStore);
      continue;
    }නැතහොත්{
      k = i - nHidden;
    }

    නම්( useTempTable ){
      sqlite3VdbeAddOp3(v, OP_Column, srcTab, k, iRegStore);
    }නැතහොත් නම්( pSelect ){
      නම්( regFromSelect!=regData ){
        sqlite3VdbeAddOp2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }නැතහොත්{
      Expr *pX = pList->a[k].pExpr;
      පූර්ණ_සංඛ්‍යා y = sqlite3ExprCodeTarget(pParse, pX, iRegStore);
      නම්( y!=iRegStore ){
        sqlite3VdbeAddOp2(v,
          ExprHasProperty(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  endOfLoop = sqlite3VdbeMakeLabel(pParse);
  නම්( tmask & TRIGGER_BEFORE ){
    පූර්ණ_සංඛ්‍යා regCols = sqlite3GetTempRange(pParse, pTab->nCol+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    නම්( ipkColumn<0 ){
      sqlite3VdbeAddOp2(v, OP_Integer, -1, regCols);
    }නැතහොත්{
      පූර්ණ_සංඛ්‍යා addr1;
      assert( !withoutRowid );
      නම්( useTempTable ){
        sqlite3VdbeAddOp3(v, OP_Column, srcTab, ipkColumn, regCols);
      }නැතහොත්{
        assert( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3ExprCode(pParse, pList->a[ipkColumn].pExpr, regCols);
      }
      addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, regCols); VdbeCoverage(v);
      sqlite3VdbeAddOp2(v, OP_Integer, -1, regCols);
      sqlite3VdbeJumpHere(v, addr1);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, regCols); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    assert( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3VdbeAddOp3(v, OP_Copy, regRowid+1, regCols+1, pTab->nNVCol-1);

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    නම්( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, regCols+1, pTab);
    }
#endif

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    නම්( !isView ){
      sqlite3TableAffinity(v, pTab, regCols+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_INSERT, 0, TRIGGER_BEFORE,
        pTab, regCols-pTab->nCol-1, onError, endOfLoop);

    sqlite3ReleaseTempRange(pParse, regCols, pTab->nCol+1);
  }

  නම්( !isView ){
    නම්( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3VdbeAddOp2(v, OP_Null, 0, regIns);
    }
    නම්( ipkColumn>=0 ){
      /* Compute the new rowid */
      නම්( useTempTable ){
        sqlite3VdbeAddOp3(v, OP_Column, srcTab, ipkColumn, regRowid);
      }නැතහොත් නම්( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }නැතහොත්{
        Expr *pIpk = pList->a[ipkColumn].pExpr;
        නම්( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3VdbeAddOp3(v, OP_NewRowid, iDataCur, regRowid, regAutoinc);
          appendFlag = 1;
        }නැතහොත්{
          sqlite3ExprCode(pParse, pList->a[ipkColumn].pExpr, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      නම්( !appendFlag ){
        පූර්ණ_සංඛ්‍යා addr1;
        නම්( !IsVirtual(pTab) ){
          addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3VdbeAddOp3(v, OP_NewRowid, iDataCur, regRowid, regAutoinc);
          sqlite3VdbeJumpHere(v, addr1);
        }නැතහොත්{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3VdbeAddOp1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }නැතහොත් නම්( IsVirtual(pTab) || withoutRowid ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regRowid);
    }නැතහොත්{
      sqlite3VdbeAddOp3(v, OP_NewRowid, iDataCur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    autoIncStep(pParse, regAutoinc, regRowid);

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    නම්( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#endif

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#ifndef SQLITE_OMIT_VIRTUALTABLE
    නම්( IsVirtual(pTab) ){
      const var_na *pVTab = (const var_na *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3VdbeAddOp4(v, OP_VUpdate, 1, pTab->nCol+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }නැතහොත්
#endif
    {
      පූර්ණ_සංඛ්‍යා isReplace = 0;/* Set to true if constraints may cause a replace */
      පූර්ණ_සංඛ්‍යා bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, iDataCur, iIdxCur,
          regIns, 0, ipkColumn>=0, onError, endOfLoop, &isReplace, 0, pUpsert
      );
      නම්( db->flags & SQLITE_ForeignKeys ){
        sqlite3FkCheck(pParse, pTab, 0, regIns, 0, 0);
      }

      /* Set the OPFLAG_USESEEKRESULT flag if either (a) there are no REPLACE
      ** constraints or (b) there are no triggers and this table is not a
      ** parent table in a foreign key constraint. It is safe to set the
      ** flag in the second case as if any REPLACE constraint is hit, an
      ** OP_Delete or OP_IdxDelete instruction will be executed on each
      ** cursor that is disturbed. And these instructions both clear the
      ** VdbeCursor.seekResult variable, disabling the OPFLAG_USESEEKRESULT
      ** functionality.  */
      bUseSeek = (isReplace==0 || !sqlite3VdbeHasSubProgram(v));
      sqlite3CompleteInsertion(pParse, pTab, iDataCur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
  }නැතහොත් නම්( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3VdbeAddOp2(v, OP_Integer, -1, regRowid);
#endif
  }

  /* Update the count of rows that are inserted
  */
  නම්( regRowCount ){
    sqlite3VdbeAddOp2(v, OP_AddImm, regRowCount, 1);
  }

  නම්( pTrigger ){
    /* Code AFTER triggers */
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_INSERT, 0, TRIGGER_AFTER,
        pTab, regData-2-pTab->nCol, onError, endOfLoop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, endOfLoop);
  නම්( useTempTable ){
    sqlite3VdbeAddOp2(v, OP_Next, srcTab, addrCont); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addrInsTop);
    sqlite3VdbeAddOp1(v, OP_Close, srcTab);
  }නැතහොත් නම්( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#ifdef SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    නම්( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      assert( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#endif
    sqlite3VdbeJumpHere(v, addrInsTop);
  }

#ifndef SQLITE_OMIT_XFER_OPT
insert_end:
#endif /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  නම්( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  නම්( regRowCount ){
    sqlite3CodeChangeCount(v, regRowCount, "rows inserted");
  }

insert_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3ExprListDelete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  නම්( pColumn ){
    sqlite3IdListDelete(db, pColumn);
    sqlite3DbFree(db, aTabColMap);
  }
  නම්( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ifdef isView
 #undef isView
#endif
#ifdef pTrigger
 #undef pTrigger
#endif
#ifdef tmask
 #undef tmask
#endif

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#define CKCNSTRNT_COLUMN   0x01    /* CHECK constraint uses a changing column */
#define CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
static පූර්ණ_සංඛ්‍යා checkConstraintExprNode(Walker *pWalker, Expr *pExpr){
  නම්( pExpr->op==TK_COLUMN ){
    assert( pExpr->iColumn>=0 || pExpr->iColumn==-1 );
    නම්( pExpr->iColumn>=0 ){
      නම්( pWalker->u.aiCol[pExpr->iColumn]>=0 ){
        pWalker->eCode |= CKCNSTRNT_COLUMN;
      }
    }නැතහොත්{
      pWalker->eCode |= CKCNSTRNT_ROWID;
    }
  }
  ආපසු WRC_Continue;
}

/*
** pExpr is a CHECK constraint on a row that is being UPDATE-ed.  The
** only columns that are modified by the UPDATE are those for which
** aiChng[i]>=0, and also the ROWID is modified if chngRowid is true.
**
** Return true if CHECK constraint pExpr uses any of the
** changing columns (or the rowid if it is changing).  In other words,
** return true if this CHECK constraint must be validated for
** the new row in the UPDATE statement.
**
** 2018-09-15: pExpr might also be an expression for an index-on-expressions.
** The operation of this routine is the same - return true if an only if
** the expression uses one or more of columns identified by the second and
** third arguments.
*/
පූර්ණ_සංඛ්‍යා sqlite3ExprReferencesUpdatedColumn(
  Expr *pExpr,    /* The expression to be checked */
  පූර්ණ_සංඛ්‍යා *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  පූර්ණ_සංඛ්‍යා chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  memset(&w, 0, parimaa_na(w));
  w.eCode = 0;
  w.xExprCallback = checkConstraintExprNode;
  w.u.aiCol = aiChng;
  sqlite3WalkExpr(&w, pExpr);
  නම්( !chngRowid ){
    testcase( (w.eCode & CKCNSTRNT_ROWID)!=0 );
    w.eCode &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.eCode==0 );
  testcase( w.eCode==CKCNSTRNT_COLUMN );
  testcase( w.eCode==CKCNSTRNT_ROWID );
  testcase( w.eCode==(CKCNSTRNT_ROWID|CKCNSTRNT_COLUMN) );
  ආපසු w.eCode!=0;
}

/*
** The sqlite3GenerateConstraintChecks() routine usually wants to visit
** the indexes of a table in the order provided in the Table->pIndex list.
** However, sometimes (rarely - when there is an upsert) it wants to visit
** the indexes in a different order.  The following data structures accomplish
** this.
**
** The IndexIterator object is used to walk through all of the indexes
** of a table in either Index.pNext order, or in some other order established
** by an array of IndexListTerm objects.
*/
typedef sa_nracanaa IndexListTerm IndexListTerm;
typedef sa_nracanaa IndexIterator IndexIterator;
sa_nracanaa IndexIterator {
  පූර්ණ_සංඛ්‍යා eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  පූර්ණ_සංඛ්‍යා i;        /* Index of the current item from the list */
  sa_ngha {
    sa_nracanaa {    /* Use this object for eType==0: A Index.pNext list */
      Index *pIdx;   /* The current Index */
    } lx;     
    sa_nracanaa {    /* Use this object for eType==1; Array of IndexListTerm */
      පූර්ණ_සංඛ්‍යා nIdx;               /* Size of the array */
      IndexListTerm *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
sa_nracanaa IndexListTerm {
  Index *p;  /* The index */
  පූර්ණ_සංඛ්‍යා ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
static Index *indexIteratorFirst(IndexIterator *pIter, පූර්ණ_සංඛ්‍යා *pIx){
  assert( pIter->i==0 );
  නම්( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    ආපසු pIter->u.ax.aIdx[0].p;
  }නැතහොත්{
    *pIx = 0;
    ආපසු pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
static Index *indexIteratorNext(IndexIterator *pIter, පූර්ණ_සංඛ්‍යා *pIx){
  නම්( pIter->eType ){
    පූර්ණ_සංඛ්‍යා i = ++pIter->i;
    නම්( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      ආපසු 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    ආපසු pIter->u.ax.aIdx[i].p;
  }නැතහොත්{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->pNext;
    ආපසු pIter->u.lx.pIdx;
  }
}
 
/*
** Generate code to do constraint checks prior to an INSERT or an UPDATE
** on table pTab.
**
** The regNewData parameter is the first register in a range that contains
** the data to be inserted or the data after the update.  There will be
** pTab->nCol+1 registers in this range.  The first register (the one
** that regNewData points to) will contain the new rowid, or NULL in the
** case of a WITHOUT ROWID table.  The second register in the range will
** contain the content of the first table column.  The third register will
** contain the content of the second table column.  And so forth.
**
** The regOldData parameter is similar to regNewData except that it contains
** the data prior to an UPDATE rather than afterwards.  regOldData is zero
** for an INSERT.  This routine can distinguish between UPDATE and INSERT by
** checking regOldData for zero.
**
** For an UPDATE, the pkChng boolean is true if the true primary key (the
** rowid for a normal table or the PRIMARY KEY for a WITHOUT ROWID table)
** might be modified by the UPDATE.  If pkChng is false, then the key of
** the iDataCur content table is guaranteed to be unchanged by the UPDATE.
**
** For an INSERT, the pkChng boolean indicates whether or not the rowid
** was explicitly specified as part of the INSERT statement.  If pkChng
** is zero, it means that the either rowid is computed automatically or
** that the table is a WITHOUT ROWID table and has no rowid.  On an INSERT,
** pkChng will only be true if the INSERT statement provides an integer
** value for either the rowid column or its INTEGER PRIMARY KEY alias.
**
** The code generated by this routine will store new index entries into
** registers identified by aRegIdx[].  No index entry is created for
** indices where aRegIdx[i]==0.  The order of indices in aRegIdx[] is
** the same as the order of indices on the linked list of indices
** at pTab->pIndex.
**
** (2019-05-07) The generated code also creates a new record for the
** main table, if pTab is a rowid table, and stores that record in the
** register identified by aRegIdx[nIdx] - in other words in the first
** entry of aRegIdx[] past the last index.  It is important that the
** record be generated during constraint checks to avoid affinity changes
** to the register content that occur after constraint checks but before
** the new record is inserted.
**
** The caller must have already opened writeable cursors on the main
** table and all applicable indices (that is to say, all indices for which
** aRegIdx[] is not zero).  iDataCur is the cursor for the main table when
** inserting or updating a rowid table, or the cursor for the PRIMARY KEY
** index when operating on a WITHOUT ROWID table.  iIdxCur is the cursor
** for the first index in the pTab->pIndex list.  Cursors for other indices
** are at iIdxCur+N for the N-th element of the pTab->pIndex list.
**
** This routine also generates code to check constraints.  NOT NULL,
** CHECK, and UNIQUE constraints are all checked.  If a constraint fails,
** then the appropriate action is performed.  There are five possible
** actions: ROLLBACK, ABORT, FAIL, REPLACE, and IGNORE.
**
**  Constraint type  Action       What Happens
**  ---------------  ----------   ----------------------------------------
**  any              ROLLBACK     The current transaction is rolled back and
**                                sqlite3_step() returns immediately with a
**                                return code of SQLITE_CONSTRAINT.
**
**  any              ABORT        Back out changes from the current command
**                                only (do not do a complete rollback) then
**                                cause sqlite3_step() to return immediately
**                                with SQLITE_CONSTRAINT.
**
**  any              FAIL         Sqlite3_step() returns immediately with a
**                                return code of SQLITE_CONSTRAINT.  The
**                                transaction is not rolled back and any
**                                changes to prior rows are retained.
**
**  any              IGNORE       The attempt in insert or update the current
**                                row is skipped, without throwing an error.
**                                Processing continues with the next row.
**                                (There is an immediate jump to ignoreDest.)
**
**  NOT NULL         REPLACE      The NULL value is replace by the default
**                                value for that column.  If the default value
**                                is NULL, the action is the same as ABORT.
**
**  UNIQUE           REPLACE      The other row that conflicts with the row
**                                being inserted is removed.
**
**  CHECK            REPLACE      Illegal.  The results in an exception.
**
** Which action to take is determined by the overrideError parameter.
** Or if overrideError==OE_Default, then the pParse->onError parameter
** is used.  Or if pParse->onError==OE_Default then the onError value
** for the constraint is used.
*/
void sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  Table *pTab,         /* The table being inserted or updated */
  පූර්ණ_සංඛ්‍යා *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  පූර්ණ_සංඛ්‍යා iDataCur,        /* Canonical data cursor (main table or PK index) */
  පූර්ණ_සංඛ්‍යා iIdxCur,         /* First index cursor */
  පූර්ණ_සංඛ්‍යා regNewData,      /* First register in a range holding values to insert */
  පූර්ණ_සංඛ්‍යා regOldData,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  පූර්ණ_සංඛ්‍යා ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  පූර්ණ_සංඛ්‍යා *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  පූර්ණ_සංඛ්‍යා *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  Index *pIdx;         /* Pointer to one of the indices */
  Index *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  පූර්ණ_සංඛ්‍යා i;               /* loop counter */
  පූර්ණ_සංඛ්‍යා ix;              /* Index loop counter */
  පූර්ණ_සංඛ්‍යා nCol;            /* Number of columns */
  පූර්ණ_සංඛ්‍යා onError;         /* Conflict resolution strategy */
  පූර්ණ_සංඛ්‍යා seenReplace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  පූර්ණ_සංඛ්‍යා nPkField;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 bAffinityDone = 0;  /* True if the OP_Affinity operation has been run */
  පූර්ණ_සංඛ්‍යා upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  පූර්ණ_සංඛ්‍යා upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  පූර්ණ_සංඛ්‍යා ipkTop = 0;        /* Top of the IPK uniqueness check */
  පූර්ණ_සංඛ්‍යා ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  පූර්ණ_සංඛ්‍යා regTrigCnt;       /* Register used to count replace trigger invocations */
  පූර්ණ_සංඛ්‍යා addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  පූර්ණ_සංඛ්‍යා lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  පූර්ණ_සංඛ්‍යා nReplaceTrig = 0; /* Number of replace triggers coded */
  IndexIterator sIdxIter;  /* Index iterator */

  isUpdate = regOldData!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  assert( v!=0 );
  assert( !IsView(pTab) );  /* This table is not a VIEW */
  nCol = pTab->nCol;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  නම්( HasRowid(pTab) ){
    pPk = 0;
    nPkField = 1;
  }නැතහොත්{
    pPk = sqlite3PrimaryKeyIndex(pTab);
    nPkField = pPk->nKeyCol;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     iDataCur, iIdxCur, regNewData, regOldData, pkChng));

  /* Test all NOT NULL constraints.
  */
  නම්( pTab->tabFlags & TF_HasNotNull ){
    පූර්ණ_සංඛ්‍යා b2ndPass = 0;         /* True if currently running 2nd pass */
    පූර්ණ_සංඛ්‍යා nSeenReplace = 0;     /* Number of ON CONFLICT REPLACE operations */
    පූර්ණ_සංඛ්‍යා nGenerated = 0;       /* Number of generated columns with NOT NULL */
    අතරතුර(1){  /* Make 2 passes over columns. Exit loop via "break" */
      සඳහා(i=0; i<nCol; i++){
        පූර්ණ_සංඛ්‍යා iReg;                        /* Register holding column value */
        Column *pCol = &pTab->aCol[i];   /* The column to check for NOT NULL */
        පූර්ණ_සංඛ්‍යා isGenerated;                 /* non-zero if column is generated */
        onError = pCol->notNull;
        නම්( onError==OE_None ) continue; /* No NOT NULL on this column */
        නම්( i==pTab->iPKey ){
          continue;        /* ROWID is never NULL */
        }
        isGenerated = pCol->colFlags & COLFLAG_GENERATED;
        නම්( isGenerated && !b2ndPass ){
          nGenerated++;
          continue;        /* Generated columns processed on 2nd pass */
        }
        නම්( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          continue;
        }
        නම්( overrideError!=OE_Default ){
          onError = overrideError;
        }නැතහොත් නම්( onError==OE_Default ){
          onError = OE_Abort;
        }
        නම්( onError==OE_Replace ){
          නම්( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || pCol->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( pCol->colFlags & COLFLAG_VIRTUAL );
            testcase( pCol->colFlags & COLFLAG_STORED );
            testcase( pCol->colFlags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }නැතහොත්{
            assert( !isGenerated );
          }
        }නැතහොත් නම්( b2ndPass && !isGenerated ){
          continue;
        }
        assert( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3TableColumnToStorage(pTab, i) );
        iReg = sqlite3TableColumnToStorage(pTab, i) + regNewData + 1;
        switch( onError ){
          pak_sa OE_Replace: {
            පූර්ණ_සංඛ්‍යා addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            assert( (pCol->colFlags & COLFLAG_GENERATED)==0 );
            nSeenReplace++;
            sqlite3ExprCodeCopy(pParse,
               sqlite3ColumnExprAuth(pTab, pCol, pParse), iReg);
            sqlite3VdbeJumpHere(v, addr1);
            bha_nga;
          }
          pak_sa OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          pak_sa OE_Rollback:
          pak_sa OE_Fail: {
            var_na *zMsg = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        pCol->zCnName);
            testcase( zMsg==0 && db->mallocFailed==0 );
            sqlite3VdbeAddOp3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, zMsg, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            bha_nga;
          }
          default: {
            assert( onError==OE_Ignore );
            sqlite3VdbeAddOp2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            bha_nga;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      නම්( nGenerated==0 && nSeenReplace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        bha_nga;
      }
      නම්( b2ndPass ) bha_nga;  /* Never need more than 2 passes */
      b2ndPass = 1;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
      නම්( nSeenReplace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, regNewData+1, pTab);
      }
#endif
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#ifndef SQLITE_OMIT_CHECK
  නම්( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    ExprList *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(regNewData+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    සඳහා(i=0; i<pCheck->nExpr; i++){
      පූර්ණ_සංඛ්‍යා allOk;
      Expr *pCopy;
      Expr *pExpr = pCheck->a[i].pExpr;
      නම්( aiChng
       && !sqlite3ExprReferencesUpdatedColumn(pExpr, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        continue;
      }
      නම්( bAffinityDone==0 ){
        sqlite3TableAffinity(v, pTab, regNewData+1);
        bAffinityDone = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3ExprDup(db, pExpr, 0);
      නම්( !db->mallocFailed ){
        sqlite3ExprIfTrue(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3ExprDelete(db, pCopy);
      නම්( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }නැතහොත්{
        var_na *zName = pCheck->a[i].zEName;
        assert( zName!=0 || pParse->db->mallocFailed );
        නම්( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#endif /* !defined(SQLITE_OMIT_CHECK) */

  /* UNIQUE and PRIMARY KEY constraints should be handled in the following
  ** order:
  **
  **   (1)  OE_Update
  **   (2)  OE_Abort, OE_Fail, OE_Rollback, OE_Ignore
  **   (3)  OE_Replace
  **
  ** OE_Fail and OE_Ignore must happen before any changes are made.
  ** OE_Update guarantees that only a single row will change, so it
  ** must happen before OE_Replace.  Technically, OE_Abort and OE_Rollback
  ** could happen in any order, but they are grouped up front for
  ** convenience.
  **
  ** 2018-08-14: Ticket https://sqlite.org/src/info/908f001483982c43
  ** The order of constraints used to have OE_Update as (2) and OE_Abort
  ** and so forth as (1). But apparently PostgreSQL checks the OE_Update
  ** constraint before any others, so it had to be moved.
  **
  ** Constraint checking code is generated in this order:
  **   (A)  The rowid constraint
  **   (B)  Unique index constraints that do not have OE_Replace as their
  **        default conflict resolution strategy
  **   (C)  Unique index that do use OE_Replace by default.
  **
  ** The ordering of (2) and (3) is accomplished by making sure the linked
  ** list of indexes attached to a table puts all OE_Replace indexes last
  ** in the list.  See sqlite3CreateIndex() for where that happens.
  */
  sIdxIter.eType = 0;
  sIdxIter.i = 0;
  sIdxIter.u.ax.aIdx = 0;  /* Silence harmless compiler warning */
  sIdxIter.u.lx.pIdx = pTab->pIndex;
  නම්( pUpsert ){
    නම්( pUpsert->pUpsertTarget==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      assert( pUpsert->pNextUpsert==0 );
      නම්( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }නැතහොත්{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }නැතහොත් නම්( pTab->pIndex!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      පූර්ණ_සංඛ්‍යා nIdx, jj;
      u64 nByte;
      Upsert *pTerm;
      u8 *bUsed;
      සඳහා(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
         assert( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (parimaa_na(IndexListTerm)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      නම්( sIdxIter.u.ax.aIdx==0 ) ආපසු; /* OOM */
      bUsed = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->pToFree = sIdxIter.u.ax.aIdx;
      සඳහා(i=0, pTerm=pUpsert; pTerm; pTerm=pTerm->pNextUpsert){
        නම්( pTerm->pUpsertTarget==0 ) bha_nga;
        නම්( pTerm->pUpsertIdx==0 ) continue;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->pIndex;
        අතරතුර( ALWAYS(pIdx!=0) && pIdx!=pTerm->pUpsertIdx ){
           pIdx = pIdx->pNext;
           jj++;
        }
        නම්( bUsed[jj] ) continue; /* Duplicate ON CONFLICT clause ignored */
        bUsed[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      සඳහා(jj=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, jj++){
        නම්( bUsed[jj] ) continue;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      assert( i==nIdx );
    }
  }

  /* Determine if it is possible that triggers (either explicitly coded
  ** triggers or FK resolution actions) might run as a result of deletes
  ** that happen when OE_Replace conflict resolution occurs. (Call these
  ** "replace triggers".)  If any replace triggers run, we will need to
  ** recheck all of the uniqueness constraints after they have all run.
  ** But on the recheck, the resolution is OE_Abort instead of OE_Replace.
  **
  ** If replace triggers are a possibility, then
  **
  **   (1) Allocate register regTrigCnt and initialize it to zero.
  **       That register will count the number of replace triggers that
  **       fire.  Constraint recheck only occurs if the number is positive.
  **   (2) Initialize pTrigger to the list of all DELETE triggers on pTab.
  **   (3) Initialize addrRecheck and lblRecheckOk
  **
  ** The uniqueness rechecking code will create a series of tests to run
  ** in a second pass.  The addrRecheck and lblRecheckOk variables are
  ** used to link together these tests which are separated from each other
  ** in the generate bytecode.
  */
  නම්( (db->flags & (SQLITE_RecTriggers|SQLITE_ForeignKeys))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }නැතහොත්{
    නම්( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }නැතහොත්{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    නම්( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      නම්( overrideError!=OE_Replace ){
        නම්( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }නැතහොත් නම්( pkChng==0 || pPk || pTab->keyConf!=OE_Replace ){
          සඳහා(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
            නම්( pIdx->onError==OE_Replace ) bha_nga;
          }
          නම්( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    නම්( regTrigCnt ){
      /* Replace triggers might exist.  Allocate the counter and
      ** initialize it to zero. */
      regTrigCnt = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Integer, 0, regTrigCnt);
      VdbeComment((v, "trigger count"));
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  /* If rowid is changing, make sure the new rowid does not previously
  ** exist in the table.
  */
  නම්( pkChng && pPk==0 ){
    පූර්ණ_සංඛ්‍යා addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->keyConf;
    නම්( overrideError!=OE_Default ){
      onError = overrideError;
    }නැතහොත් නම්( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    නම්( pUpsert ){
      pUpsertClause = sqlite3UpsertOfIndex(pUpsert,0);
      නම්( pUpsertClause!=0 ){
        නම්( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }නැතහොත්{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      නම්( pUpsertClause!=pUpsert ){
        /* The first ON CONFLICT clause has a conflict target other than
        ** the IPK.  We have to jump ahead to that first ON CONFLICT clause
        ** and then come back here and deal with the IPK afterwards */
        upsertIpkDelay = sqlite3VdbeAddOp0(v, OP_Goto);
      }
    }

    /* If the response to a rowid conflict is REPLACE but the response
    ** to some other UNIQUE constraint is FAIL or IGNORE, then we need
    ** to defer the running of the rowid conflict checking until after
    ** the UNIQUE constraints have run.
    */
    නම්( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->pIndex             /* There exist other constraints */
    ){
      නම්( upsertIpkDelay ){
        ipkTop = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }නැතහොත්{
        ipkTop = sqlite3VdbeAddOp0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    නම්( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3VdbeAddOp3(v, OP_Eq, regNewData, addrRowidOk, regOldData);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, addrRowidOk, regNewData);
    VdbeCoverage(v);

    switch( onError ){
      default: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      pak_sa OE_Rollback:
      pak_sa OE_Abort:
      pak_sa OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        bha_nga;
      }
      pak_sa OE_Replace: {
        /* If there are DELETE triggers on this table and the
        ** recursive-triggers flag is set, call GenerateRowDelete() to
        ** remove the conflicting row from the table. This will fire
        ** the triggers and remove both the table and index b-tree entries.
        **
        ** Otherwise, if there are no triggers or the recursive-triggers
        ** flag is not set, but the table has one or more indexes, call
        ** GenerateRowIndexDelete(). This removes the index b-tree entries
        ** only. The table b-tree entry will be replaced by the new entry
        ** when it is inserted. 
        **
        ** If either GenerateRowDelete() or GenerateRowIndexDelete() is called,
        ** also invoke MultiWrite() to indicate that this VDBE may require
        ** statement rollback (if the statement is aborted after the delete
        ** takes place). Earlier versions called sqlite3MultiWrite() regardless,
        ** but being more selective here allows statements like:
        **
        **   REPLACE INTO t(rowid) VALUES($newrowid)
        **
        ** to run without a statement journal if there are no indexes on the
        ** table.
        */
        නම්( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur,
                                   regNewData, 1, 0, OE_Replace, 1, -1);
          sqlite3VdbeAddOp2(v, OP_AddImm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }නැතහොත්{
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
          assert( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
          නම්( pTab->pIndex ){
            sqlite3MultiWrite(pParse);
            sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur,0,-1);
          }
        }
        seenReplace = 1;
        bha_nga;
      }
#ifndef SQLITE_OMIT_UPSERT
      pak_sa OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, iDataCur);
        /* no break */ deliberate_fall_through
      }
#endif
      pak_sa OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        bha_nga;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    නම්( ipkTop ){
      ipkBottom = sqlite3VdbeAddOp0(v, OP_Goto);
      sqlite3VdbeJumpHere(v, ipkTop-1);
    }නැතහොත් නම්( pUpsert && pUpsertClause!=pUpsert ){
      upsertIpkReturn = sqlite3VdbeAddOp0(v, OP_Goto);
    }
  }

  /* Test all UNIQUE constraints by creating entries for each UNIQUE
  ** index and making sure that duplicate entries do not already exist.
  ** Compute the revised record entries for indices as we go.
  **
  ** This loop also handles the case of the PRIMARY KEY index for a
  ** WITHOUT ROWID table.
  */
  සඳහා(pIdx = indexIteratorFirst(&sIdxIter, &ix);
      pIdx;
      pIdx = indexIteratorNext(&sIdxIter, &ix)
  ){
    පූර්ණ_සංඛ්‍යා regIdx;          /* Range of registers holding content for pIdx */
    පූර්ණ_සංඛ්‍යා regR;            /* Range of registers holding conflicting PK */
    පූර්ණ_සංඛ්‍යා iThisCur;        /* Cursor for this UNIQUE index */
    පූර්ණ_සංඛ්‍යා addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    පූර්ණ_සංඛ්‍යා addrConflictCk;  /* First opcode in the conflict check logic */
    පූර්ණ_සංඛ්‍යා nConflictCk;     /* Number of opcodes in conflict check logic */

    නම්( aRegIdx[ix]==0 ) continue;  /* Skip indices that do not change */
    නම්( pUpsert ){
      pUpsertClause = sqlite3UpsertOfIndex(pUpsert, pIdx);
      නම්( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3VdbeJumpHere(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    නම්( bAffinityDone==0 ){
      sqlite3TableAffinity(v, pTab, regNewData+1);
      bAffinityDone = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    නම්( pIdx->pPartIdxWhere ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(regNewData+1);
      sqlite3ExprIfFalseDup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    සඳහා(i=0; i<pIdx->nColumn; i++){
      පූර්ණ_සංඛ්‍යා iField = pIdx->aiColumn[i];
      පූර්ණ_සංඛ්‍යා x;
      නම්( iField==XN_EXPR ){
        pParse->iSelfTab = -(regNewData+1);
        sqlite3ExprCodeCopy(pParse, pIdx->aColExpr->a[i].pExpr, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }නැතහොත් නම්( iField==XN_ROWID || iField==pTab->iPKey ){
        x = regNewData;
        sqlite3VdbeAddOp2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }නැතහොත්{
        testcase( sqlite3TableColumnToStorage(pTab, iField)!=iField );
        x = sqlite3TableColumnToStorage(pTab, iField) + regNewData + 1;
        sqlite3VdbeAddOp2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->aCol[iField].zCnName));
      }
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regIdx, pIdx->nColumn, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#ifdef SQLITE_ENABLE_NULL_TRIM
    නම්( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3SetMakeRecordP5(v, pIdx->pTable);
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->nColumn, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    නම්( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    නම්( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;  /* pIdx is not a UNIQUE index */
    }
    නම්( overrideError!=OE_Default ){
      onError = overrideError;
    }නැතහොත් නම්( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    නම්( pUpsertClause ){
      නම්( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }නැතහොත්{
        onError = OE_Update;  /* DO UPDATE */
      }
    }

    /* Collision detection may be omitted if all of the following are true:
    **   (1) The conflict resolution algorithm is REPLACE
    **   (2) The table is a WITHOUT ROWID table
    **   (3) There are no secondary indexes on the table
    **   (4) No delete triggers need to be fired if there is a conflict
    **   (5) No FK constraint counters need to be updated if a conflict occurs.
    **
    ** This is not possible for ENABLE_PREUPDATE_HOOK builds, as the row
    ** must be explicitly deleted in order to ensure any pre-update hook
    ** is invoked.  */
    assert( IsOrdinaryTable(pTab) );
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    නම්( (ix==0 && pIdx->pNext==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_ForeignKeys) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;
    }
#endif /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3VdbeAddOp4Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->nKeyCol); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3GetTempRange(pParse, nPkField);
    නම්( isUpdate || onError==OE_Replace ){
      නම්( HasRowid(pTab) ){
        sqlite3VdbeAddOp2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        නම්( isUpdate ){
          sqlite3VdbeAddOp3(v, OP_Eq, regR, addrUniqueOk, regOldData);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }නැතහොත්{
        පූර්ණ_සංඛ්‍යා x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        නම්( pIdx!=pPk ){
          සඳහා(i=0; i<pPk->nKeyCol; i++){
            assert( pPk->aiColumn[i]>=0 );
            x = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[i]);
            sqlite3VdbeAddOp3(v, OP_Column, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->aCol[pPk->aiColumn[i]].zCnName));
          }
        }
        නම්( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          පූර්ණ_සංඛ්‍යා addrJump = sqlite3VdbeCurrentAddr(v)+pPk->nKeyCol;
          පූර්ණ_සංඛ්‍යා op = OP_Ne;
          පූර්ණ_සංඛ්‍යා regCmp = (IsPrimaryKeyIndex(pIdx) ? regIdx : regR);
 
          සඳහා(i=0; i<pPk->nKeyCol; i++){
            var_na *p4 = (var_na*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->aiColumn[i];
            assert( x>=0 );
            නම්( i==(pPk->nKeyCol-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3TableColumnToStorage(pTab, x);
            sqlite3VdbeAddOp4(v, op,
                regOldData+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    assert( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    assert( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    switch( onError ){
      pak_sa OE_Rollback:
      pak_sa OE_Abort:
      pak_sa OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        bha_nga;
      }
#ifndef SQLITE_OMIT_UPSERT
      pak_sa OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#endif
      pak_sa OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        bha_nga;
      }
      default: {
        assert( onError==OE_Replace );
        නම්( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        නම්( pTrigger && isUpdate ){
          sqlite3VdbeAddOp1(v, OP_CursorLock, iDataCur);
        }
        sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur,
            regR, nPkField, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        නම්( pTrigger && isUpdate ){
          sqlite3VdbeAddOp1(v, OP_CursorUnlock, iDataCur);
        }
        seenReplace = 1;
        bha_nga;
      }
    }
    නම්( regTrigCnt ){
      පූර්ණ_සංඛ්‍යා addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3VdbeAddOp2(v, OP_AddImm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3VdbeAddOp0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      නම්( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3VdbeAddOp2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      අතරතුර( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        නම්( x.opcode!=OP_IdxRowid || isUpdate ){
          පූර්ණ_සංඛ්‍යා p2;      /* New P2 value for copied conflict check opcode */
          const var_na *zP4;
          නම්( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }නැතහොත්{
            p2 = x.p2;
          }
          zP4 = x.p4type==P4_INT32 ? SQLITE_INT_TO_PTR(x.p4.i) : x.p4.z;
          sqlite3VdbeAddOp4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
          sqlite3VdbeChangeP5(v, x.p5);
          VdbeCoverageIf(v, p2!=x.p2);
        }
        nConflictCk--;
        addrConflictCk++;
      }
      /* If the retest fails, issue an abort */
      sqlite3UniqueConstraint(pParse, OE_Abort, pIdx);

      sqlite3VdbeJumpHere(v, addrBypass); /* Terminate the recheck bypass */
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    නම්( regR!=regIdx ) sqlite3ReleaseTempRange(pParse, regR, nPkField);
    නම්( pUpsertClause
     && upsertIpkReturn
     && sqlite3UpsertNextIsIPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3VdbeJumpHere(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  නම්( ipkTop ){
    sqlite3VdbeGoto(v, ipkTop);
    VdbeComment((v, "Do IPK REPLACE"));
    assert( ipkBottom>0 );
    sqlite3VdbeJumpHere(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  assert( regTrigCnt!=0 || nReplaceTrig==0 );
  නම්( nReplaceTrig ){
    sqlite3VdbeAddOp2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    නම්( !pPk ){
      නම්( isUpdate ){
        sqlite3VdbeAddOp3(v, OP_Eq, regNewData, addrRecheck, regOldData);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp3(v, OP_NotExists, iDataCur, addrRecheck, regNewData);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }නැතහොත්{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  නම්( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  නම්( HasRowid(pTab) ){
    පූර්ණ_සංඛ්‍යා regRec = aRegIdx[ix];
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regNewData+1, pTab->nNVCol, regRec);
    sqlite3SetMakeRecordP5(v, pTab);
    නම්( !bAffinityDone ){
      sqlite3TableAffinity(v, pTab, 0);
    }
  }

  *pbMayReplace = seenReplace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", seenReplace));
}

#ifdef SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
void sqlite3SetMakeRecordP5(Vdbe *v, Table *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  නම්( pTab->pSchema->file_format<2 ) ආපසු;

  සඳහා(i=pTab->nCol-1; i>0; i--){
    නම්( pTab->aCol[i].iDflt!=0 ) bha_nga;
    නම්( pTab->aCol[i].colFlags & COLFLAG_PRIMKEY ) bha_nga;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#endif

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
static void codeWithoutRowidPreupdate(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated */
  පූර්ණ_සංඛ්‍යා iCur,                       /* Cursor number for table */
  පූර්ණ_සංඛ්‍යා regData                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  පූර්ණ_සංඛ්‍යා r = sqlite3GetTempReg(pParse);
  assert( !HasRowid(pTab) );
  assert( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3VdbeAddOp2(v, OP_Integer, 0, r);
  sqlite3VdbeAddOp4(v, OP_Insert, iCur, regData, r, (var_na*)pTab, P4_TABLE);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3ReleaseTempReg(pParse, r);
}
#නැතහොත්
# define codeWithoutRowidPreupdate(a,b,c,d)
#endif

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
void sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  Table *pTab,        /* the table into which we are inserting */
  පූර්ණ_සංඛ්‍යා iDataCur,       /* Cursor of the canonical data source */
  පූර්ණ_සංඛ්‍යා iIdxCur,        /* First index cursor */
  පූර්ණ_සංඛ්‍යා regNewData,     /* Range of content */
  පූර්ණ_සංඛ්‍යා *aRegIdx,       /* Register used by each index.  0 for unused indices */
  පූර්ණ_සංඛ්‍යා update_flags,   /* True for UPDATE, False for INSERT */
  පූර්ණ_සංඛ්‍යා appendBias,     /* True if this is likely to be an append */
  පූර්ණ_සංඛ්‍යා useSeekResult   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  Index *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  පූර්ණ_සංඛ්‍යා i;              /* Loop counter */

  assert( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  assert( v!=0 );
  assert( !IsView(pTab) );  /* This table is not a VIEW */
  සඳහා(i=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, i++){
    /* All REPLACE indexes are at the end of the list */
    assert( pIdx->onError!=OE_Replace
         || pIdx->pNext==0
         || pIdx->pNext->onError==OE_Replace );
    නම්( aRegIdx[i]==0 ) continue;
    නම්( pIdx->pPartIdxWhere || (update_flags && pIdx->bHasExpr) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3VdbeAddOp2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (useSeekResult ? OPFLAG_USESEEKRESULT : 0);
    නම්( IsPrimaryKeyIndex(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      නම්( update_flags==0 ){
        codeWithoutRowidPreupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3VdbeAddOp4Int(v, OP_IdxInsert, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->nKeyCol: pIdx->nColumn);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  නම්( !HasRowid(pTab) ) ආපසු;
  නම්( pParse->nested ){
    pik_flags = 0;
  }නැතහොත්{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  නම්( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  නම්( useSeekResult ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3VdbeAddOp3(v, OP_Insert, iDataCur, aRegIdx[i], regNewData);
  නම්( !pParse->nested ){
    sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
  }
  sqlite3VdbeChangeP5(v, pik_flags);
}

/*
** Allocate cursors for the pTab table and all its indices and generate
** code to open and initialized those cursors.
**
** The cursor for the object that contains the complete data (normally
** the table itself, but the PRIMARY KEY index in the case of a WITHOUT
** ROWID table) is returned in *piDataCur.  The first index cursor is
** returned in *piIdxCur.  The number of indices is returned.
**
** Use iBase as the first cursor (either the *piDataCur for rowid tables
** or the first index for WITHOUT ROWID tables) if it is non-negative.
** If iBase is negative, then allocate the next available cursor.
**
** For a rowid table, *piDataCur will be exactly one less than *piIdxCur.
** For a WITHOUT ROWID table, *piDataCur will be somewhere in the range
** of *piIdxCurs, depending on where the PRIMARY KEY index appears on the
** pTab->pIndex list.
**
** If pTab is a virtual table, then this routine is a no-op and the
** *piDataCur and *piIdxCur values are left uninitialized.
*/
පූර්ණ_සංඛ්‍යා sqlite3OpenTableAndIndices(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* Table to be opened */
  පූර්ණ_සංඛ්‍යා op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  පූර්ණ_සංඛ්‍යා iBase,       /* Use this for the table cursor, if there is one */
  u8 *aToOpen,     /* If not NULL: boolean for each table and index */
  පූර්ණ_සංඛ්‍යා *piDataCur,  /* Write the database source cursor number here */
  පූර්ණ_සංඛ්‍යා *piIdxCur    /* Write the first index cursor number here */
){
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා iDb;
  පූර්ණ_සංඛ්‍යා iDataCur;
  Index *pIdx;
  Vdbe *v;

  assert( op==OP_OpenRead || op==OP_OpenWrite );
  assert( op==OP_OpenWrite || p5==0 );
  assert( piDataCur!=0 );
  assert( piIdxCur!=0 );
  නම්( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *piDataCur = *piIdxCur = -999;
    ආපසු 0;
  }
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  assert( v!=0 );
  නම්( iBase<0 ) iBase = pParse->nTab;
  iDataCur = iBase++;
  *piDataCur = iDataCur;
  නම්( HasRowid(pTab) && (aToOpen==0 || aToOpen[0]) ){
    sqlite3OpenTable(pParse, iDataCur, iDb, pTab, op);
  }නැතහොත් නම්( pParse->db->noSharedCache==0 ){
    sqlite3TableLock(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = iBase;
  සඳහා(i=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, i++){
    පූර්ණ_සංඛ්‍යා iIdxCur = iBase++;
    assert( pIdx->pSchema==pTab->pSchema );
    නම්( IsPrimaryKeyIndex(pIdx) && !HasRowid(pTab) ){
      *piDataCur = iIdxCur;
      p5 = 0;
    }
    නම්( aToOpen==0 || aToOpen[i+1] ){
      sqlite3VdbeAddOp3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  නම්( iBase>pParse->nTab ) pParse->nTab = iBase;
  ආපසු i;
}


#ifdef SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
පූර්ණ_සංඛ්‍යා sqlite3_xferopt_count;
#endif /* SQLITE_TEST */


#ifndef SQLITE_OMIT_XFER_OPT
/*
** Check to see if index pSrc is compatible as a source of data
** for index pDest in an insert transfer optimization.  The rules
** for a compatible index:
**
**    *   The index is over the same set of columns
**    *   The same DESC and ASC markings occurs on all columns
**    *   The same onError processing (OE_Abort, OE_Ignore, etc)
**    *   The same collating sequence on each column
**    *   The index has the exact same WHERE clause
*/
static පූර්ණ_සංඛ්‍යා xferCompatibleIndex(Index *pDest, Index *pSrc){
  පූර්ණ_සංඛ්‍යා i;
  assert( pDest && pSrc );
  assert( pDest->pTable!=pSrc->pTable );
  නම්( pDest->nKeyCol!=pSrc->nKeyCol || pDest->nColumn!=pSrc->nColumn ){
    ආපසු 0;   /* Different number of columns */
  }
  නම්( pDest->onError!=pSrc->onError ){
    ආපසු 0;   /* Different conflict resolution strategies */
  }
  සඳහා(i=0; i<pSrc->nKeyCol; i++){
    නම්( pSrc->aiColumn[i]!=pDest->aiColumn[i] ){
      ආපසු 0;   /* Different columns indexed */
    }
    නම්( pSrc->aiColumn[i]==XN_EXPR ){
      assert( pSrc->aColExpr!=0 && pDest->aColExpr!=0 );
      නම්( sqlite3ExprCompare(0, pSrc->aColExpr->a[i].pExpr,
                             pDest->aColExpr->a[i].pExpr, -1)!=0 ){
        ආපසු 0;   /* Different expressions in the index */
      }
    }
    නම්( pSrc->aSortOrder[i]!=pDest->aSortOrder[i] ){
      ආපසු 0;   /* Different sort orders */
    }
    නම්( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      ආපසු 0;   /* Different collating sequences */
    }
  }
  නම්( sqlite3ExprCompare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    ආපසු 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  ආපසු 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
static පූර්ණ_සංඛ්‍යා xferCheckRowid(Walker *pWalk, Expr *pExpr){
  නම්( pExpr->op==TK_COLUMN && pExpr->iColumn<0 ){
    pWalk->eCode = 1;
    ආපසු WRC_Abort;
  }නැතහොත්{
    ආපසු WRC_Continue;
  }
}

/*
** Analyze CHECK constraints on the source and destination tables and
** return true if those CHECK constraints are compatible with the
** xfer-optimization.
**
**    *  The pDest and pSrc tables must have identical CHECK constraints.
**
**    *  If the destination table, pDest, does not have an
**       INTEGER PRIMARY KEY column, then no CHECK constraint may
**       referenced the ROWID.  (See forum post 2026-05-11T13:15:57Z)
*/
static පූර්ණ_සංඛ්‍යා xferCompatibleCheck(Table *pDest, Table *pSrc){
  නම්( sqlite3ExprListCompare(pSrc->pCheck,pDest->pCheck,-1) ){
    ආපසු 0;
  }
  නම්( pDest->iPKey<0 ){
    Walker w;
    memset(&w, 0, parimaa_na(w));
    w.xExprCallback = xferCheckRowid;
    sqlite3WalkExprList(&w,pDest->pCheck);
    නම්( w.eCode ) ආපසු 0;
  }
  ආපසු 1;
}

/*
** Attempt the transfer optimization on INSERTs of the form
**
**     INSERT INTO tab1 SELECT * FROM tab2;
**
** The xfer optimization transfers raw records from tab2 over to tab1. 
** Columns are not decoded and reassembled, which greatly improves
** performance.  Raw index records are transferred in the same way.
**
** The xfer optimization is only attempted if tab1 and tab2 are compatible.
** There are lots of rules for determining compatibility - see comments
** embedded in the code for details.
**
** This routine returns TRUE if the optimization is guaranteed to be used.
** Sometimes the xfer optimization will only work if the destination table
** is empty - a factor that can only be determined at run-time.  In that
** case, this routine generates code for the xfer optimization but also
** does a test to see if the destination table is empty and jumps over the
** xfer optimization code if the test fails.  In that case, this routine
** returns FALSE so that the caller will know to go ahead and generate
** an unoptimized transfer.  This routine also returns FALSE if there
** is no chance that the xfer optimization can be applied.
**
** This optimization is particularly useful at making VACUUM run faster.
*/
static පූර්ණ_සංඛ්‍යා xferOptimization(
  Parse *pParse,        /* Parser context */
  Table *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  පූර්ණ_සංඛ්‍යා onError,          /* How to handle constraint errors */
  පූර්ණ_සංඛ්‍යා iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  ExprList *pEList;                /* The result set of the SELECT */
  Table *pSrc;                     /* The table in the FROM clause of SELECT */
  Index *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  SrcItem *pItem;                  /* An element of pSelect->pSrc */
  පූර්ණ_සංඛ්‍යා i;                           /* Loop counter */
  පූර්ණ_සංඛ්‍යා iDbSrc;                      /* The database of pSrc */
  පූර්ණ_සංඛ්‍යා iSrc, iDest;                 /* Cursors from source and destination */
  පූර්ණ_සංඛ්‍යා addr1, addr2;                /* Loop addresses */
  පූර්ණ_සංඛ්‍යා emptyDestTest = 0;           /* Address of test for empty pDest */
  පූර්ණ_සංඛ්‍යා emptySrcTest = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  පූර්ණ_සංඛ්‍යා regAutoinc;                  /* Memory register used by AUTOINC */
  පූර්ණ_සංඛ්‍යා destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  පූර්ණ_සංඛ්‍යා regData, regRowid;           /* Registers holding data and rowid */

  assert( pSelect!=0 );
  නම්( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    ආපසු 0;
  }
#ifndef SQLITE_OMIT_VIRTUALTABLE
  නම්( IsVirtual(pDest) ){
    ආපසු 0;   /* tab1 must not be a virtual table */
  }
#endif
  නම්( onError==OE_Default ){
    නම්( pDest->iPKey>=0 ) onError = pDest->keyConf;
    නම්( onError==OE_Default ) onError = OE_Abort;
  }
  assert(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  නම්( pSelect->pSrc->nSrc!=1 ){
    ආපසු 0;   /* FROM clause must have exactly one term */
  }
  නම්( pSelect->pSrc->a[0].fg.isSubquery ){
    ආපසු 0;   /* FROM clause cannot contain a subquery */
  }
  නම්( pSelect->pWhere ){
    ආපසු 0;   /* SELECT may not have a WHERE clause */
  }
  නම්( pSelect->pOrderBy ){
    ආපසු 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  නම්( pSelect->pGroupBy ){
    ආපසු 0;   /* SELECT may not have a GROUP BY clause */
  }
  නම්( pSelect->pLimit ){
    ආපසු 0;   /* SELECT may not have a LIMIT clause */
  }
  නම්( pSelect->pPrior ){
    ආපසු 0;   /* SELECT may not be a compound query */
  }
  නම්( pSelect->selFlags & SF_Distinct ){
    ආපසු 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  assert( pEList!=0 );
  නම්( pEList->nExpr!=1 ){
    ආපසු 0;   /* The result set must have exactly one column */
  }
  assert( pEList->a[0].pExpr );
  නම්( pEList->a[0].pExpr->op!=TK_ASTERISK ){
    ආපසු 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  pItem = pSelect->pSrc->a;
  pSrc = sqlite3LocateTableItem(pParse, 0, pItem);
  නම්( pSrc==0 ){
    ආපසු 0;   /* FROM clause does not contain a real table */
  }
  නම්( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    ආපසු 0;   /* tab1 and tab2 may not be the same table */
  }
  නම්( HasRowid(pDest)!=HasRowid(pSrc) ){
    ආපසු 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  නම්( !IsOrdinaryTable(pSrc) ){
    ආපසු 0;   /* tab2 may not be a view or virtual table */
  }
  නම්( pDest->nCol!=pSrc->nCol ){
    ආපසු 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  නම්( pDest->iPKey!=pSrc->iPKey ){
    ආපසු 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  නම්( (pDest->tabFlags & TF_Strict)!=0 ){
    නම්( (pSrc->tabFlags & TF_Strict)==0 ){
      ආපසු 0;  /* Cannot feed from a non-strict into a strict table */
    }
    සඳහා(i=0; i<pDest->nCol; i++){
      unsigned eDestType = pDest->aCol[i].eCType;
      unsigned eSrcType = pSrc->aCol[i].eCType;
      නම්( eDestType==COLTYPE_ANY ) continue;
      නම්( eDestType==eSrcType ) continue;
      නම්( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) continue;
      නම්( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) continue;
      ආපසු 0; /* Incompatible types in source and destination */
    }
  }
  සඳහා(i=0; i<pDest->nCol; i++){
    Column *pDestCol = &pDest->aCol[i];
    Column *pSrcCol = &pSrc->aCol[i];
#ifdef SQLITE_ENABLE_HIDDEN_COLUMNS
    නම්( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (pDestCol->colFlags | pSrcCol->colFlags) & COLFLAG_HIDDEN
    ){
      ආපසු 0;    /* Neither table may have __hidden__ columns */
    }
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    /* Even if tables t1 and t2 have identical schemas, if they contain
    ** generated columns, then this statement is semantically incorrect:
    **
    **     INSERT INTO t2 SELECT * FROM t1;
    **
    ** The reason is that generated column values are returned by the
    ** the SELECT statement on the right but the INSERT statement on the
    ** left wants them to be omitted.
    **
    ** Nevertheless, this is a useful notational shorthand to tell SQLite
    ** to do a bulk transfer all of the content from t1 over to t2.
    **
    ** We could, in theory, disable this (except for internal use by the
    ** VACUUM command where it is actually needed).  But why do that?  It
    ** seems harmless enough, and provides a useful service.
    */
    නම්( (pDestCol->colFlags & COLFLAG_GENERATED) !=
        (pSrcCol->colFlags & COLFLAG_GENERATED) ){
      ආපසු 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    නම්( (pDestCol->colFlags & COLFLAG_GENERATED)!=0 ){
      නම්( sqlite3ExprCompare(0,
             sqlite3ColumnExpr(pSrc, pSrcCol),
             sqlite3ColumnExpr(pDest, pDestCol), -1)!=0 ){
        testcase( pDestCol->colFlags & COLFLAG_VIRTUAL );
        testcase( pDestCol->colFlags & COLFLAG_STORED );
        ආපසු 0;  /* Different generator expressions */
      }
    }
#endif
    නම්( pDestCol->affinity!=pSrcCol->affinity ){
      ආපසු 0;    /* Affinity must be the same on all columns */
    }
    නම්( sqlite3_stricmp(sqlite3ColumnColl(pDestCol),
                        sqlite3ColumnColl(pSrcCol))!=0 ){
      ආපසු 0;    /* Collating sequence must be the same on all columns */
    }
    නම්( pDestCol->notNull && !pSrcCol->notNull ){
      ආපසු 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    නම්( (pDestCol->colFlags & COLFLAG_GENERATED)==0 && i>0 ){
      Expr *pDestExpr = sqlite3ColumnExpr(pDest, pDestCol);
      Expr *pSrcExpr = sqlite3ColumnExpr(pSrc, pSrcCol);
      assert( pDestExpr==0 || pDestExpr->op==TK_SPAN );
      assert( pDestExpr==0 || !ExprHasProperty(pDestExpr, EP_IntValue) );
      assert( pSrcExpr==0 || pSrcExpr->op==TK_SPAN );
      assert( pSrcExpr==0 || !ExprHasProperty(pSrcExpr, EP_IntValue) );
      නම්( (pDestExpr==0)!=(pSrcExpr==0)
       || (pDestExpr!=0 && strcmp(pDestExpr->u.zToken,
                                       pSrcExpr->u.zToken)!=0)
      ){
        ආපසු 0;    /* Default values must be the same for all columns */
      }
    }
  }
  සඳහා(pDestIdx=pDest->pIndex; pDestIdx; pDestIdx=pDestIdx->pNext){
    නම්( IsUniqueIndex(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    සඳහා(pSrcIdx=pSrc->pIndex; pSrcIdx; pSrcIdx=pSrcIdx->pNext){
      නම්( xferCompatibleIndex(pDestIdx, pSrcIdx) ) bha_nga;
    }
    නම්( pSrcIdx==0 ){
      ආපසු 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    නම්( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      ආපසු 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#ifndef SQLITE_OMIT_CHECK
  නම්( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    ආපසු 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#endif
#ifndef SQLITE_OMIT_FOREIGN_KEY
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  assert( IsOrdinaryTable(pDest) );
  නම්( (db->flags & SQLITE_ForeignKeys)!=0 && pDest->u.tab.pFKey!=0 ){
    ආපසු 0;
  }
#endif
  නම්( (db->flags & SQLITE_CountRows)!=0 ){
    ආපසු 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  නම්( db->xAuth ){
    පූර්ණ_සංඛ්‍යා iDb = sqlite3SchemaToIndex(db, pSrc->pSchema);
    නම්( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ආපසු 0;
    සඳහා(i=0; i<pSrc->nCol; i++){
      Column *pSrcCol = &pSrc->aCol[i];
      නම්( sqlite3AuthReadCol(pParse, pSrc->zName, pSrcCol->zCnName, iDb) ){
        ආපසු 0;
      }
    }
  }
#endif

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#ifdef SQLITE_TEST
  sqlite3_xferopt_count++;
#endif
  iDbSrc = sqlite3SchemaToIndex(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3CodeVerifySchema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  regData = sqlite3GetTempReg(pParse);
  sqlite3VdbeAddOp2(v, OP_Null, 0, regData);
  regRowid = sqlite3GetTempReg(pParse);
  sqlite3OpenTable(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  assert( HasRowid(pDest) || destHasUniqueIdx );
  නම්( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->pIndex!=0)          /* (1) */
   || destHasUniqueIdx                              /* (2) */
   || (onError!=OE_Abort && onError!=OE_Rollback)   /* (3) */
  )){
    /* In some circumstances, we are able to run the xfer optimization
    ** only if the destination table is initially empty. Unless the
    ** DBFLAG_Vacuum flag is set, this block generates code to make
    ** that determination. If DBFLAG_Vacuum is set, then the destination
    ** table is always empty.
    **
    ** Conditions under which the destination must be empty:
    **
    ** (1) There is no INTEGER PRIMARY KEY but there are indices.
    **     (If the destination is not initially empty, the rowid fields
    **     of index entries might need to change.)
    **
    ** (2) The destination has a unique index.  (The xfer optimization
    **     is unable to test uniqueness.)
    **
    ** (3) onError is something other than OE_Abort and OE_Rollback.
    */
    addr1 = sqlite3VdbeAddOp2(v, OP_Rewind, iDest, 0); VdbeCoverage(v);
    emptyDestTest = sqlite3VdbeAddOp0(v, OP_Goto);
    sqlite3VdbeJumpHere(v, addr1);
  }
  නම්( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3OpenTable(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    emptySrcTest = sqlite3VdbeAddOp2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    නම්( pDest->iPKey>=0 ){
      addr1 = sqlite3VdbeAddOp2(v, OP_Rowid, iSrc, regRowid);
      නම්( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3VdbeAddOp3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3VdbeJumpHere(v, addr2);
      }
      autoIncStep(pParse, regAutoinc, regRowid);
    }නැතහොත් නම්( pDest->pIndex==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3VdbeAddOp2(v, OP_NewRowid, iDest, regRowid);
    }නැතහොත්{
      addr1 = sqlite3VdbeAddOp2(v, OP_Rowid, iSrc, regRowid);
      assert( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    නම්( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3VdbeAddOp1(v, OP_SeekEnd, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }නැතහොත්{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    නම්( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeAddOp3(v, OP_RowData, iSrc, regData, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }නැතහොත්
#endif
    {
      sqlite3VdbeAddOp3(v, OP_RowCell, iDest, iSrc, regRowid);
    }
    sqlite3VdbeAddOp3(v, OP_Insert, iDest, regData, regRowid);
    නම්( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (var_na*)pDest, P4_TABLE);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3VdbeAddOp2(v, OP_Next, iSrc, addr1); VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Close, iSrc, 0);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
  }නැතහොත්{
    sqlite3TableLock(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3TableLock(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  සඳහා(pDestIdx=pDest->pIndex; pDestIdx; pDestIdx=pDestIdx->pNext){
    u8 idxInsFlags = 0;
    සඳහා(pSrcIdx=pSrc->pIndex; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->pNext){
      නම්( xferCompatibleIndex(pDestIdx, pSrcIdx) ) bha_nga;
    }
    assert( pSrcIdx );
    sqlite3VdbeAddOp3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3VdbeSetP4KeyInfo(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3VdbeAddOp3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3VdbeSetP4KeyInfo(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3VdbeAddOp2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    නම්( db->mDbFlags & DBFLAG_Vacuum ){
      /* This INSERT command is part of a VACUUM operation, which guarantees
      ** that the destination table is empty. If all indexed columns use
      ** collation sequence BINARY, then it can also be assumed that the
      ** index will be populated by inserting keys in strictly sorted
      ** order. In this case, instead of seeking within the b-tree as part
      ** of every OP_IdxInsert opcode, an OP_SeekEnd is added before the
      ** OP_IdxInsert to seek to the point within the b-tree where each key
      ** should be inserted. This is faster.
      **
      ** If any of the indexed columns use a collation sequence other than
      ** BINARY, this optimization is disabled. This is because the user
      ** might change the definition of a collation sequence and then run
      ** a VACUUM command. In that case keys may not be written in strictly
      ** sorted order.  */
      සඳහා(i=0; i<pSrcIdx->nColumn; i++){
        const var_na *zColl = pSrcIdx->azColl[i];
        නම්( sqlite3_stricmp(sqlite3StrBINARY, zColl) ) bha_nga;
      }
      නම්( i==pSrcIdx->nColumn ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3VdbeAddOp1(v, OP_SeekEnd, iDest);
        sqlite3VdbeAddOp2(v, OP_RowCell, iDest, iSrc);
      }
    }නැතහොත් නම්( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    නම්( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3VdbeAddOp3(v, OP_RowData, iSrc, regData, 1);
      නම්( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && IsPrimaryKeyIndex(pDestIdx)
      ){
        codeWithoutRowidPreupdate(pParse, pDest, iDest, regData);
      }
    }
    sqlite3VdbeAddOp2(v, OP_IdxInsert, iDest, regData);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3VdbeAddOp2(v, OP_Next, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp2(v, OP_Close, iSrc, 0);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
  }
  නම්( emptySrcTest ) sqlite3VdbeJumpHere(v, emptySrcTest);
  sqlite3ReleaseTempReg(pParse, regRowid);
  sqlite3ReleaseTempReg(pParse, regData);
  නම්( emptyDestTest ){
    sqlite3AutoincrementEnd(pParse);
    sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3VdbeJumpHere(v, emptyDestTest);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
    ආපසු 0;
  }නැතහොත්{
    ආපසු 1;
  }
}
#endif /* SQLITE_OMIT_XFER_OPT */
