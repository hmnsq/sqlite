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
#समावेश "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
व्योम sqlite3OpenTable(
  Parse *pParse,  /* Generate code into this VDBE */
  पूर्णांक iCur,       /* The cursor number of the table */
  पूर्णांक iDb,        /* The database index in sqlite3.aDb[] */
  Table *pTab,    /* The table to be opened */
  पूर्णांक opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  निश्चित( !IsVirtual(pTab) );
  निश्चित( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  निश्चित( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  अगर( !pParse->db->noSharedCache ){
    sqlite3TableLock(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  अगर( HasRowid(pTab) ){
    sqlite3VdbeAddOp4Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }अन्यथा{
    सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
    निश्चित( pPk!=0 );
    निश्चित( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3VdbeAddOp3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pPk);
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
अटल SQLITE_NOINLINE स्थिर अक्षर *compute_सूचकांक_Aff_Str(sqlite3 *db, सूचकांक *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  पूर्णांक n;
  Table *pTab = pIdx->pTable;
  pIdx->zColAff = (अक्षर *)sqlite3DbMallocRaw(0, pIdx->nColumn+1);
  अगर( !pIdx->zColAff ){
    sqlite3OomFault(db);
    वापस 0;
  }
  क्रम(n=0; n<pIdx->nColumn; n++){
    i16 x = pIdx->aiColumn[n];
    अक्षर aff;
    अगर( x>=0 ){
      aff = pTab->aCol[x].affinity;
    }अन्यथा अगर( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }अन्यथा{
      निश्चित( x==XN_व्यंजक );
      निश्चित( pIdx->b_Has_व्यंजक );
      निश्चित( pIdx->a_Col_व्यंजक!=0 );
      aff = sqlite3_व्यंजक_Affinity(pIdx->a_Col_व्यंजक->a[n].p_व्यंजक);
    }
    अगर( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    अगर( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->zColAff[n] = aff;
  }
  pIdx->zColAff[n] = 0;
  वापस pIdx->zColAff;
}
स्थिर अक्षर *sqlite3_सूचकांक_Affinity_Str(sqlite3 *db, सूचकांक *pIdx){
  अगर( !pIdx->zColAff ) वापस compute_सूचकांक_Aff_Str(db, pIdx);
  वापस pIdx->zColAff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
अक्षर *sqlite3TableAffinityStr(sqlite3 *db, स्थिर Table *pTab){
  अक्षर *zColAff;
  zColAff = (अक्षर *)sqlite3DbMallocRaw(db, pTab->nCol+1);
  अगर( zColAff ){
    पूर्णांक i, j;
    क्रम(i=j=0; i<pTab->nCol; i++){
      अगर( (pTab->aCol[i].colFlags & COLFLAG_VIRTUAL)==0 ){
        zColAff[j++] = pTab->aCol[i].affinity;
      }
    }
    करो{
      zColAff[j--] = 0;
    }जबतक( j>=0 && zColAff[j]<=SQLITE_AFF_BLOB );
  }
  वापस zColAff; 
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
व्योम sqlite3TableAffinity(Vdbe *v, Table *pTab, पूर्णांक iReg){
  पूर्णांक i;
  अक्षर *zColAff;
  अगर( pTab->tabFlags & TF_Strict ){
    अगर( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *p_पूर्व;
      पूर्णांक p3;
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
      p_पूर्व = sqlite3VdbeGetLastOp(v);
      निश्चित( p_पूर्व!=0 );
      निश्चित( p_पूर्व->opcode==OP_MakeRecord || sqlite3VdbeDb(v)->mallocFailed );
      p_पूर्व->opcode = OP_TypeCheck;
      p3 = p_पूर्व->p3;
      p_पूर्व->p3 = 0;
      sqlite3VdbeAddOp3(v, OP_MakeRecord, p_पूर्व->p1, p_पूर्व->p2, p3);
    }अन्यथा{
      /* Insert an isolated OP_Typecheck */
      sqlite3VdbeAddOp2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
    }
    वापस;
  }
  zColAff = pTab->zColAff;
  अगर( zColAff==0 ){
    zColAff = sqlite3TableAffinityStr(0, pTab);
    अगर( !zColAff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      वापस;
    }
    pTab->zColAff = zColAff;
  }
  निश्चित( zColAff!=0 );
  i = sqlite3Strlen30NN(zColAff);
  अगर( i ){
    अगर( iReg ){
      sqlite3VdbeAddOp4(v, OP_Affinity, iReg, i, 0, zColAff, i);
    }अन्यथा{
      निश्चित( sqlite3VdbeGetLastOp(v)->opcode==OP_MakeRecord
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
अटल पूर्णांक readsTable(Parse *p, पूर्णांक iDb, Table *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  पूर्णांक i;
  पूर्णांक iEnd = sqlite3VdbeCurrentAddr(v);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#पूर्ण_अगर

  क्रम(i=1; i<iEnd; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    निश्चित( pOp!=0 );
    अगर( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      सूचकांक *p_सूचकांक;
      Pgno tnum = pOp->p2;
      अगर( tnum==pTab->tnum ){
        वापस 1;
      }
      क्रम(p_सूचकांक=pTab->p_सूचकांक; p_सूचकांक; p_सूचकांक=p_सूचकांक->p_अग्र){
        अगर( tnum==p_सूचकांक->tnum ){
          वापस 1;
        }
      }
    }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    अगर( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      निश्चित( pOp->p4.pVtab!=0 );
      निश्चित( pOp->p4type==P4_VTAB );
      वापस 1;
    }
#पूर्ण_अगर
  }
  वापस 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
अटल पूर्णांक व्यंजक_Column_Flag_Union(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_COLUMN && p_व्यंजक->iColumn>=0 ){
    निश्चित( p_व्यंजक->iColumn < pWalker->u.pTab->nCol );
    pWalker->eCode |= pWalker->u.pTab->aCol[p_व्यंजक->iColumn].colFlags;
  }
  वापस WRC_Continue;
}

#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
व्योम sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  पूर्णांक iRegStore,    /* Register holding the first column */
  Table *pTab       /* The table */
){
  पूर्णांक i;
  Walker w;
  Column *pRedo;
  पूर्णांक eProgress;
  VdbeOp *pOp;

  निश्चित( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3TableAffinity(pParse->pVdbe, pTab, iRegStore);
  अगर( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    अगर( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      पूर्णांक ii, jj;
      अक्षर *zP4 = pOp->p4.z;
      निश्चित( zP4!=0 );
      निश्चित( pOp->p4type==P4_DYNAMIC );
      क्रम(ii=jj=0; zP4[jj]; ii++){
        अगर( pTab->aCol[ii].colFlags & COLFLAG_VIRTUAL ){
          जारी;
        }
        अगर( pTab->aCol[ii].colFlags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }अन्यथा अगर( pOp->opcode==OP_TypeCheck ){
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
  क्रम(i=0; i<pTab->nCol; i++){
    अगर( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
      testcase( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL );
      testcase( pTab->aCol[i].colFlags & COLFLAG_STORED );
      pTab->aCol[i].colFlags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.x_व्यंजक_Callback = व्यंजक_Column_Flag_Union;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  करो{
    eProgress = 0;
    pRedo = 0;
    क्रम(i=0; i<pTab->nCol; i++){
      Column *pCol = pTab->aCol + i;
      अगर( (pCol->colFlags & COLFLAG_NOTAVAIL)!=0 ){
        पूर्णांक x;
        pCol->colFlags |= COLFLAG_BUSY;
        w.eCode = 0;
        sqlite3_Walk_व्यंजक(&w, sqlite3_Column_व्यंजक(pTab, pCol));
        pCol->colFlags &= ~COLFLAG_BUSY;
        अगर( w.eCode & COLFLAG_NOTAVAIL ){
          pRedo = pCol;
          जारी;
        }
        eProgress = 1;
        निश्चित( pCol->colFlags & COLFLAG_GENERATED );
        x = sqlite3TableColumnToStorage(pTab, i) + iRegStore;
        sqlite3_व्यंजक_Code_Generated_Column(pParse, pTab, pCol, x);
        pCol->colFlags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }जबतक( pRedo && eProgress );
  अगर( pRedo ){
    sqlite3_Error_संदेश(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#पूर्ण_अगर /* SQLITE_OMIT_GENERATED_COLUMNS */


#अगर_अघोषित SQLITE_OMIT_AUTOINCREMENT
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
अटल पूर्णांक autoIncBegin(
  Parse *pParse,      /* Parsing context */
  पूर्णांक iDb,            /* Index of the database holding pTab */
  Table *pTab         /* The table we are writing to */
){
  पूर्णांक memId = 0;      /* Register holding maximum rowid */
  निश्चित( pParse->db->aDb[iDb].pSchema!=0 );
  अगर( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    Table *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    अगर( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->nCol!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      वापस 0;
    }

    अगर( pToplevel->usesAinc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    जबतक( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->p_अग्र; }
    अगर( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, माप(*pInfo));
      sqlite3ParserAddCleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      अगर( pParse->db->mallocFailed ) वापस 0;
      pInfo->p_अग्र = pToplevel->pAinc;
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
  वापस memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
व्योम sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  पूर्णांक memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  निश्चित( pParse->pTriggerTab==0 );
  निश्चित( sqlite3IsToplevel(pParse) );

  निश्चित( v );   /* We failed long ago if this is not so */
  निश्चित( pParse->usesAinc );
  क्रम(p = pParse->pAinc; p; p = p->p_अग्र){
    अटल स्थिर पूर्णांक iLn = VDBE_OFFSET_LINENO(2);
    अटल स्थिर VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_Column,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_Column,  0,  1, 0},
      /* 6  */ {OP_AddImm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_अग्र,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    निश्चित( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3VdbeLoadString(v, memId-1, p->pTab->zName);
    aOp = sqlite3VdbeAddOpList(v, Array_आकार(autoInc), autoInc, iLn);
    अगर( aOp==0 ) अवरोध;
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
    अगर( pParse->nTab==0 ) pParse->nTab = 1;
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
अटल व्योम autoIncStep(Parse *pParse, पूर्णांक memId, पूर्णांक regRowid){
  अगर( memId>0 ){
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
अटल SQLITE_NOINLINE व्योम autoIncrementEnd(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  निश्चित( v );
  निश्चित( pParse->usesAinc );
  क्रम(p = pParse->pAinc; p; p = p->p_अग्र){
    अटल स्थिर पूर्णांक iLn = VDBE_OFFSET_LINENO(2);
    अटल स्थिर VdbeOpList autoIncEnd[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_MakeRecord,  0, 2, 0},
      /* 3 */ {OP_जोड़ो,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    पूर्णांक iRec;
    पूर्णांक memId = p->regCtr;

    iRec = sqlite3_Get_अस्थायी_Reg(pParse);
    निश्चित( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3VdbeAddOp3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3VdbeAddOpList(v, Array_आकार(autoIncEnd), autoIncEnd, iLn);
    अगर( aOp==0 ) अवरोध;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3_Release_अस्थायी_Reg(pParse, iRec);
  }
}
व्योम sqlite3AutoincrementEnd(Parse *pParse){
  अगर( pParse->usesAinc ) autoIncrementEnd(pParse);
}
#अन्यथा
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define autoIncStep(A,B,C)
#पूर्ण_अगर /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
व्योम sqlite3_Multi_मूल्यावली_End(Parse *pParse, Select *pVal){
  अगर( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    SrcItem *pItem = &pVal->pSrc->a[0];
    निश्चित( (pItem->fg.isSubquery && pItem->u4.pSubq!=0) || pParse->nErr );
    अगर( pItem->fg.isSubquery ){
      sqlite3VdbeEndCoroutine(pParse->pVdbe, pItem->u4.pSubq->regReturn);
      sqlite3VdbeJumpHere(pParse->pVdbe, pItem->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
अटल पूर्णांक व्यंजक_List_Is_No_Affinity(Parse *pParse, व्यंजक_List *pRow){
  पूर्णांक ii;
  अगर( sqlite3_व्यंजक_List_Is_Constant(pParse, pRow, 0)==0 ) वापस 0;
  क्रम(ii=0; ii<pRow->n_व्यंजक; ii++){
    व्यंजक *p_व्यंजक = pRow->a[ii].p_व्यंजक;
    निश्चित( p_व्यंजक->op!=TK_RAISE );
    निश्चित( p_व्यंजक->aff_व्यंजक==0 );
    अगर( 0!=sqlite3_व्यंजक_Affinity(p_व्यंजक) ) वापस 0;
  }
  वापस 1;

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
Select *sqlite3_Multi_मूल्यावली(Parse *pParse, Select *p_बायाँ, व्यंजक_List *pRow){

  अगर( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3_व्यंजक_List_Is_Constant(pParse, pRow, 1)==0   /* condition (c) above */
   || (p_बायाँ->pSrc->nSrc==0 &&
       व्यंजक_List_Is_No_Affinity(pParse,p_बायाँ->pEList)==0) /* condition (d) above */
   || IN_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    पूर्णांक f = SF_मूल्यावली | SF_Multi_मूल्य;
    अगर( p_बायाँ->pSrc->nSrc ){
      sqlite3_Multi_मूल्यावली_End(pParse, p_बायाँ);
      f = SF_मूल्यावली;
    }अन्यथा अगर( p_बायाँ->pPrior ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & p_बायाँ->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, pRow, 0, 0, 0, 0, 0, f, 0);
    p_बायाँ->selFlags &= ~(u32)SF_Multi_मूल्य;
    अगर( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->pPrior = p_बायाँ;
      p_बायाँ = pSelect;
    }
  }अन्यथा{
    SrcItem *p = 0;               /* SrcItem that reads from co-routine */

    अगर( p_बायाँ->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      अगर( (pParse->db->mDbFlags & DBFLAG_SchemaKnownOk)==0 ){
        sqlite3ReadSchema(pParse);
      }

      अगर( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->pPrior = p_बायाँ->pPrior;
        pRet->op = p_बायाँ->op;
        अगर( pRet->pPrior ) pRet->selFlags |= SF_मूल्यावली;
        p_बायाँ->pPrior = 0;
        p_बायाँ->op = TK_SELECT;
        निश्चित( p_बायाँ->p_अग्र==0 );
        निश्चित( pRet->p_अग्र==0 );
        p = &pRet->pSrc->a[0];
        p->fg.viaCoroutine = 1;
        p->iCursor = -1;
        निश्चित( !p->fg.isIndexedBy && !p->fg.isTabFunc );
        p->u1.nRow = 2;
        अगर( sqlite3SrcItemAttachSubquery(pParse, p, p_बायाँ, 0) ){
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
          dest.nSdst = p_बायाँ->pEList->n_व्यंजक;
          pParse->nMem += 2 + dest.nSdst;

          p_बायाँ->selFlags |= SF_Multi_मूल्य;
          sqlite3Select(pParse, p_बायाँ, &dest);
          pSubq->reg_परिणाम = dest.iSdst;
          निश्चित( pParse->nErr || dest.iSdst>0 );
        }
        p_बायाँ = pRet;
      }
    }अन्यथा{
      p = &p_बायाँ->pSrc->a[0];
      निश्चित( !p->fg.isTabFunc && !p->fg.isIndexedBy );
      p->u1.nRow++;
    }
  
    अगर( pParse->nErr==0 ){
      Subquery *pSubq;
      निश्चित( p!=0 );
      निश्चित( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      निश्चित( pSubq!=0 );
      निश्चित( pSubq->pSelect!=0 );
      निश्चित( pSubq->pSelect->pEList!=0 );
      अगर( pSubq->pSelect->pEList->n_व्यंजक!=pRow->n_व्यंजक ){
        sqlite3_Select_Wrong_अंक_Terms_Error(pParse, pSubq->pSelect);
      }अन्यथा{
        sqlite3_व्यंजक_Code_व्यंजक_List(pParse, pRow, pSubq->reg_परिणाम, 0, 0);
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3_व्यंजक_List_Delete(pParse->db, pRow);
  }

  वापस p_बायाँ;
}

/* Forward declaration */
अटल पूर्णांक xferOptimization(
  Parse *pParse,        /* Parser context */
  Table *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  पूर्णांक onError,          /* How to handle constraint errors */
  पूर्णांक iDbDest           /* The database of pDest */
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
व्योम sqlite3_जोड़ो(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *pColumn,      /* Column names corresponding to IDLIST, or NULL. */
  पूर्णांक onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  Table *pTab;          /* The table to insert into.  aka TABLE */
  पूर्णांक i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  सूचकांक *pIdx;          /* For looping over indices of the table */
  पूर्णांक nColumn;          /* Number of columns in the data */
  पूर्णांक nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  पूर्णांक i_तथ्य_Cur = 0;     /* VDBE cursor that is the main data repository */
  पूर्णांक iIdxCur = 0;      /* First index cursor */
  पूर्णांक ipkColumn = -1;   /* Column that is the INTEGER PRIMARY KEY */
  पूर्णांक endOfLoop;        /* Label for the end of the insertion loop */
  पूर्णांक srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  पूर्णांक addr_Ins_शीर्ष = 0;   /* Jump to label "D" */
  पूर्णांक addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  पूर्णांक iDb;              /* Index of database holding TABLE */
  u8 use_अस्थायी_Table = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 bIdListInOrder;    /* True if IDLIST is in table order */
  व्यंजक_List *pList = 0;  /* List of VALUES() to be inserted  */
  पूर्णांक iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  पूर्णांक regFromSelect = 0;/* Base register for data coming from SELECT */
  पूर्णांक regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  पूर्णांक reg_Row_गिनती = 0;  /* Memory cell used for the row counter */
  पूर्णांक regIns;           /* Block of regs holding rowid+data being inserted */
  पूर्णांक regRowid;         /* registers holding insert rowid */
  पूर्णांक reg_तथ्य;          /* register holding first column to insert */
  पूर्णांक *aRegIdx = 0;     /* One register allocated to each index */
  पूर्णांक *aTabColMap = 0;  /* Mapping from pTab columns to pCol entries */

#अगर_अघोषित SQLITE_OMIT_TRIGGER
  पूर्णांक isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  पूर्णांक tmask;                  /* Mask of trigger times */
#पूर्ण_अगर

  db = pParse->db;
  निश्चित( db->pParse==pParse );
  अगर( pParse->nErr ){
    जाओ जोड़ो_cleanup;
  }
  निश्चित( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  अगर( pSelect && (pSelect->selFlags & SF_मूल्यावली)!=0 && pSelect->pPrior==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  निश्चित( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  अगर( pTab==0 ){
    जाओ जोड़ो_cleanup;
  }
  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  निश्चित( iDb<db->nDb );
  अगर( sqlite3AuthCheck(pParse, SQLITE_जोड़ो, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    जाओ जोड़ो_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_जोड़ो, 0, &tmask);
  isView = IsView(pTab);
#अन्यथा
# define pTrigger 0
# define tmask 0
# define isView 0
#पूर्ण_अगर
#अगर_घोषित SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#पूर्ण_अगर
  निश्चित( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#अगर TREETRACE_ENABLED
  अगर( sqlite3_वृक्ष_Trace & 0x10000 ){
    sqlite3_वृक्ष_View_पंक्ति(0, "In sqlite3Insert() at %s:%d", __खाता__, __LINE__);
    sqlite3_वृक्ष_View_जोड़ो(pParse->pWith, pTabList, pColumn, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#पूर्ण_अगर

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  अगर( sqlite3ViewGetColumnNames(pParse, pTab) ){
    जाओ जोड़ो_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  अगर( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    जाओ जोड़ो_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  अगर( v==0 ) जाओ जोड़ो_cleanup;
  अगर( pParse->nested==0 ) sqlite3_Vdbe_गिनती_Changes(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#अगर_अघोषित SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  अगर( pColumn==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    निश्चित( !pTrigger );
    निश्चित( pList==0 );
    जाओ जोड़ो_end;
  }
#पूर्ण_अगर /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->nCol + 1;
  अगर( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  reg_तथ्य = regRowid+1;

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
  अगर( pColumn ){
    aTabColMap = sqlite3DbMallocZero(db, pTab->nCol*माप(पूर्णांक));
    अगर( aTabColMap==0 ) जाओ जोड़ो_cleanup;
    क्रम(i=0; i<pColumn->nId; i++){
      j = sqlite3_Column_सूचकांक(pTab, pColumn->a[i].zName);
      अगर( j>=0 ){
        अगर( aTabColMap[j]==0 ) aTabColMap[j] = i+1;
        अगर( i!=j ) bIdListInOrder = 0;
        अगर( j==pTab->iPKey ){
          ipkColumn = i;  निश्चित( !withoutRowid );
        }
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
        अगर( pTab->aCol[j].colFlags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3_Error_संदेश(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->aCol[j].zCnName);
          जाओ जोड़ो_cleanup;
        }
#पूर्ण_अगर
      }अन्यथा{
        अगर( sqlite3IsRowid(pColumn->a[i].zName) && !withoutRowid ){
          ipkColumn = i;
          bIdListInOrder = 0;
        }अन्यथा{
          sqlite3_Error_संदेश(pParse, "table %S has no column named %s",
              pTabList->a, pColumn->a[i].zName);
          pParse->checkSchema = 1;
          जाओ जोड़ो_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  अगर( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    पूर्णांक rc;             /* Result code */

    अगर( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.viaCoroutine 
     && pSelect->pPrior==0
    ){
      SrcItem *pItem = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      निश्चित( pItem->fg.isSubquery );
      pSubq = pItem->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->reg_परिणाम;
      निश्चित( pSubq->pSelect!=0 );
      निश्चित( pSubq->pSelect->pEList!=0 );
      nColumn = pSubq->pSelect->pEList->n_व्यंजक;
      ExplainQueryPlan((pParse, 0, "SCAN %S", pItem));
      अगर( bIdListInOrder && nColumn==pTab->nCol ){
        reg_तथ्य = regFromSelect;
        regRowid = reg_तथ्य - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }अन्यथा{
      पूर्णांक addr_शीर्ष;        /* Top of the co-routine */
      पूर्णांक regYield = ++pParse->nMem;
      addr_शीर्ष = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, addr_शीर्ष);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = bIdListInOrder ? reg_तथ्य : 0;
      dest.nSdst = pTab->nCol;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      निश्चित( db->pParse==pParse );
      अगर( rc || pParse->nErr ) जाओ जोड़ो_cleanup;
      निश्चित( db->mallocFailed==0 );
      sqlite3VdbeEndCoroutine(v, regYield);
      sqlite3VdbeJumpHere(v, addr_शीर्ष - 1);                       /* label B: */
      निश्चित( pSelect->pEList );
      nColumn = pSelect->pEList->n_व्यंजक;
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
    अगर( pTrigger || readsTable(pParse, iDb, pTab) ){
      use_अस्थायी_Table = 1;
    }

    अगर( use_अस्थायी_Table ){
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
      पूर्णांक regRec;          /* Register to hold packed record */
      पूर्णांक reg_अस्थायी_Rowid;    /* Register to hold temp table ROWID */
      पूर्णांक addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3_Get_अस्थायी_Reg(pParse);
      reg_अस्थायी_Rowid = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3VdbeAddOp2(v, OP_OpenEphemeral, srcTab, nColumn);
      addrL = sqlite3VdbeAddOp1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regFromSelect, nColumn, regRec);
      sqlite3VdbeAddOp2(v, OP_NewRowid, srcTab, reg_अस्थायी_Rowid);
      sqlite3VdbeAddOp3(v, OP_जोड़ो, srcTab, regRec, reg_अस्थायी_Rowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3VdbeJumpHere(v, addrL);
      sqlite3_Release_अस्थायी_Reg(pParse, regRec);
      sqlite3_Release_अस्थायी_Reg(pParse, reg_अस्थायी_Rowid);
    }
  }अन्यथा{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    स_रखो(&sNC, 0, माप(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    निश्चित( use_अस्थायी_Table==0 );
    अगर( pList ){
      nColumn = pList->n_व्यंजक;
      अगर( sqlite3_Resolve_व्यंजक_List_Names(&sNC, pList) ){
        जाओ जोड़ो_cleanup;
      }
    }अन्यथा{
      nColumn = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  अगर( pColumn==0 && nColumn>0 ){
    ipkColumn = pTab->iPKey;
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
    अगर( ipkColumn>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      क्रम(i=ipkColumn-1; i>=0; i--){
        अगर( pTab->aCol[i].colFlags & COLFLAG_GENERATED ){
          testcase( pTab->aCol[i].colFlags & COLFLAG_VIRTUAL );
          testcase( pTab->aCol[i].colFlags & COLFLAG_STORED );
          ipkColumn--;
        }
      }
    }
#पूर्ण_अगर

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    निश्चित( TF_HasHidden==COLFLAG_HIDDEN );
    निश्चित( TF_HasGenerated==COLFLAG_GENERATED );
    निश्चित( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    अगर( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      क्रम(i=0; i<pTab->nCol; i++){
        अगर( pTab->aCol[i].colFlags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    अगर( nColumn!=(pTab->nCol-nHidden) ){
      sqlite3_Error_संदेश(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->nCol-nHidden, nColumn);
     जाओ जोड़ो_cleanup;
    }
  }
  अगर( pColumn!=0 && nColumn!=pColumn->nId ){
    sqlite3_Error_संदेश(pParse, "%d values for %d columns", nColumn, pColumn->nId);
    जाओ जोड़ो_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  अगर( (db->flags & SQLITE_गिनती_Rows)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    reg_Row_गिनती = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, reg_Row_गिनती);
  }

  /* If this is not a view, open the table and and all indices */
  अगर( !isView ){
    पूर्णांक nIdx;
    nIdx = sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &i_तथ्य_Cur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, माप(पूर्णांक)*(nIdx+2));
    अगर( aRegIdx==0 ){
      जाओ जोड़ो_cleanup;
    }
    क्रम(i=0, pIdx=pTab->p_सूचकांक; i<nIdx; pIdx=pIdx->p_अग्र, i++){
      निश्चित( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#अगर_अघोषित SQLITE_OMIT_UPSERT
  अगर( pUpsert ){
    Upsert *pNx;
    अगर( IsVirtual(pTab) ){
      sqlite3_Error_संदेश(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      जाओ जोड़ो_cleanup;
    }
    अगर( IsView(pTab) ){
      sqlite3_Error_संदेश(pParse, "cannot UPSERT a view");
      जाओ जोड़ो_cleanup;
    }
    अगर( sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget) ){
      जाओ जोड़ो_cleanup;
    }
    pTabList->a[0].iCursor = i_तथ्य_Cur;
    pNx = pUpsert;
    करो{
      pNx->pUpsertSrc = pTabList;
      pNx->reg_तथ्य = reg_तथ्य;
      pNx->i_तथ्य_Cur = i_तथ्य_Cur;
      pNx->iIdxCur = iIdxCur;
      अगर( pNx->pUpsertTarget ){
        अगर( sqlite3UpsertAnalyzeTarget(pParse, pTabList, pNx, pUpsert) ){
          जाओ जोड़ो_cleanup;
        }
      }
      pNx = pNx->p_अग्र_Upsert;
    }जबतक( pNx!=0 );
  }
#पूर्ण_अगर


  /* This is the top of the main insertion loop */
  अगर( use_अस्थायी_Table ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addr_Ins_शीर्ष = sqlite3VdbeAddOp1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }अन्यथा अगर( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, reg_तथ्य, pTab->nCol, 0, 0);
    addr_Ins_शीर्ष = addrCont = sqlite3VdbeAddOp1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    अगर( ipkColumn>=0 ){
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
  iRegStore = reg_तथ्य;  निश्चित( reg_तथ्य==regRowid+1 );
  क्रम(i=0; i<pTab->nCol; i++, iRegStore++){
    पूर्णांक k;
    u32 colFlags;
    निश्चित( i>=nHidden );
    अगर( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3VdbeAddOp1(v, OP_SoftNull, iRegStore);
      जारी;
    }
    अगर( ((colFlags = pTab->aCol[i].colFlags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      अगर( (colFlags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        जारी;
      }अन्यथा अगर( (colFlags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        अगर( tmask & TRIGGER_BEFORE ){
          sqlite3VdbeAddOp1(v, OP_SoftNull, iRegStore);
        }
        जारी;
      }अन्यथा अगर( pColumn==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3_व्यंजक_Code_Factorable(pParse,
            sqlite3_Column_व्यंजक_Auth(pTab, &pTab->aCol[i], pParse),
            iRegStore);
        जारी;
      }
    }
    अगर( pColumn ){
      j = aTabColMap[i];
      निश्चित( j>=0 && j<=pColumn->nId );   
      अगर( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3_व्यंजक_Code_Factorable(pParse,
            sqlite3_Column_व्यंजक_Auth(pTab, &pTab->aCol[i], pParse),
            iRegStore);
        जारी;
      }
      k = j - 1;
    }अन्यथा अगर( nColumn==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3_व्यंजक_Code_Factorable(pParse,
          sqlite3_Column_व्यंजक_Auth(pTab, &pTab->aCol[i], pParse),
          iRegStore);
      जारी;
    }अन्यथा{
      k = i - nHidden;
    }

    अगर( use_अस्थायी_Table ){
      sqlite3VdbeAddOp3(v, OP_Column, srcTab, k, iRegStore);
    }अन्यथा अगर( pSelect ){
      अगर( regFromSelect!=reg_तथ्य ){
        sqlite3VdbeAddOp2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }अन्यथा{
      व्यंजक *pX = pList->a[k].p_व्यंजक;
      पूर्णांक y = sqlite3_व्यंजक_Code_Target(pParse, pX, iRegStore);
      अगर( y!=iRegStore ){
        sqlite3VdbeAddOp2(v,
          व्यंजक_Has_Property(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  endOfLoop = sqlite3VdbeMakeLabel(pParse);
  अगर( tmask & TRIGGER_BEFORE ){
    पूर्णांक regCols = sqlite3_Get_अस्थायी_Range(pParse, pTab->nCol+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    अगर( ipkColumn<0 ){
      sqlite3VdbeAddOp2(v, OP_Integer, -1, regCols);
    }अन्यथा{
      पूर्णांक addr1;
      निश्चित( !withoutRowid );
      अगर( use_अस्थायी_Table ){
        sqlite3VdbeAddOp3(v, OP_Column, srcTab, ipkColumn, regCols);
      }अन्यथा{
        निश्चित( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3_व्यंजक_Code(pParse, pList->a[ipkColumn].p_व्यंजक, regCols);
      }
      addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, regCols); VdbeCoverage(v);
      sqlite3VdbeAddOp2(v, OP_Integer, -1, regCols);
      sqlite3VdbeJumpHere(v, addr1);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, regCols); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    निश्चित( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3VdbeAddOp3(v, OP_Copy, regRowid+1, regCols+1, pTab->nNVCol-1);

#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    अगर( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, regCols+1, pTab);
    }
#पूर्ण_अगर

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    अगर( !isView ){
      sqlite3TableAffinity(v, pTab, regCols+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_जोड़ो, 0, TRIGGER_BEFORE,
        pTab, regCols-pTab->nCol-1, onError, endOfLoop);

    sqlite3_Release_अस्थायी_Range(pParse, regCols, pTab->nCol+1);
  }

  अगर( !isView ){
    अगर( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3VdbeAddOp2(v, OP_Null, 0, regIns);
    }
    अगर( ipkColumn>=0 ){
      /* Compute the new rowid */
      अगर( use_अस्थायी_Table ){
        sqlite3VdbeAddOp3(v, OP_Column, srcTab, ipkColumn, regRowid);
      }अन्यथा अगर( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }अन्यथा{
        व्यंजक *pIpk = pList->a[ipkColumn].p_व्यंजक;
        अगर( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3VdbeAddOp3(v, OP_NewRowid, i_तथ्य_Cur, regRowid, regAutoinc);
          appendFlag = 1;
        }अन्यथा{
          sqlite3_व्यंजक_Code(pParse, pList->a[ipkColumn].p_व्यंजक, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      अगर( !appendFlag ){
        पूर्णांक addr1;
        अगर( !IsVirtual(pTab) ){
          addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3VdbeAddOp3(v, OP_NewRowid, i_तथ्य_Cur, regRowid, regAutoinc);
          sqlite3VdbeJumpHere(v, addr1);
        }अन्यथा{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3VdbeAddOp1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }अन्यथा अगर( IsVirtual(pTab) || withoutRowid ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regRowid);
    }अन्यथा{
      sqlite3VdbeAddOp3(v, OP_NewRowid, i_तथ्य_Cur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    autoIncStep(pParse, regAutoinc, regRowid);

#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    अगर( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#पूर्ण_अगर

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    अगर( IsVirtual(pTab) ){
      स्थिर अक्षर *pVTab = (स्थिर अक्षर *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3VdbeAddOp4(v, OP_VUpdate, 1, pTab->nCol+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }अन्यथा
#पूर्ण_अगर
    {
      पूर्णांक isReplace = 0;/* Set to true if constraints may cause a replace */
      पूर्णांक bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_तथ्य_Cur, iIdxCur,
          regIns, 0, ipkColumn>=0, onError, endOfLoop, &isReplace, 0, pUpsert
      );
      अगर( db->flags & SQLITE_Foreign_कुंजियाँ ){
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
      sqlite3CompleteInsertion(pParse, pTab, i_तथ्य_Cur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
  }अन्यथा अगर( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3VdbeAddOp2(v, OP_Integer, -1, regRowid);
#पूर्ण_अगर
  }

  /* Update the count of rows that are inserted
  */
  अगर( reg_Row_गिनती ){
    sqlite3VdbeAddOp2(v, OP_AddImm, reg_Row_गिनती, 1);
  }

  अगर( pTrigger ){
    /* Code AFTER triggers */
    sqlite3CodeRowTrigger(pParse, pTrigger, TK_जोड़ो, 0, TRIGGER_AFTER,
        pTab, reg_तथ्य-2-pTab->nCol, onError, endOfLoop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, endOfLoop);
  अगर( use_अस्थायी_Table ){
    sqlite3VdbeAddOp2(v, OP_अग्र, srcTab, addrCont); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr_Ins_शीर्ष);
    sqlite3VdbeAddOp1(v, OP_Close, srcTab);
  }अन्यथा अगर( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#अगर_घोषित SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    अगर( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      निश्चित( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#पूर्ण_अगर
    sqlite3VdbeJumpHere(v, addr_Ins_शीर्ष);
  }

#अगर_अघोषित SQLITE_OMIT_XFER_OPT
जोड़ो_end:
#पूर्ण_अगर /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  अगर( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  अगर( reg_Row_गिनती ){
    sqlite3_Code_Change_गिनती(v, reg_Row_गिनती, "rows inserted");
  }

जोड़ो_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_व्यंजक_List_Delete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  अगर( pColumn ){
    sqlite3IdListDelete(db, pColumn);
    sqlite3DbFree(db, aTabColMap);
  }
  अगर( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#अगर_घोषित isView
 #अघोषित isView
#पूर्ण_अगर
#अगर_घोषित pTrigger
 #अघोषित pTrigger
#पूर्ण_अगर
#अगर_घोषित tmask
 #अघोषित tmask
#पूर्ण_अगर

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#घोषणा CKCNSTRNT_COLUMN   0x01    /* CHECK constraint uses a changing column */
#घोषणा CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
अटल पूर्णांक check_Constraint_व्यंजक_गाँठ(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_COLUMN ){
    निश्चित( p_व्यंजक->iColumn>=0 || p_व्यंजक->iColumn==-1 );
    अगर( p_व्यंजक->iColumn>=0 ){
      अगर( pWalker->u.aiCol[p_व्यंजक->iColumn]>=0 ){
        pWalker->eCode |= CKCNSTRNT_COLUMN;
      }
    }अन्यथा{
      pWalker->eCode |= CKCNSTRNT_ROWID;
    }
  }
  वापस WRC_Continue;
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
पूर्णांक sqlite3_व्यंजक_References_Updated_Column(
  व्यंजक *p_व्यंजक,    /* The expression to be checked */
  पूर्णांक *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  पूर्णांक chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  स_रखो(&w, 0, माप(w));
  w.eCode = 0;
  w.x_व्यंजक_Callback = check_Constraint_व्यंजक_गाँठ;
  w.u.aiCol = aiChng;
  sqlite3_Walk_व्यंजक(&w, p_व्यंजक);
  अगर( !chngRowid ){
    testcase( (w.eCode & CKCNSTRNT_ROWID)!=0 );
    w.eCode &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.eCode==0 );
  testcase( w.eCode==CKCNSTRNT_COLUMN );
  testcase( w.eCode==CKCNSTRNT_ROWID );
  testcase( w.eCode==(CKCNSTRNT_ROWID|CKCNSTRNT_COLUMN) );
  वापस w.eCode!=0;
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
प्रकार काष्ठा सूचकांक_List_Term सूचकांक_List_Term;
प्रकार काष्ठा सूचकांक_Iterator सूचकांक_Iterator;
काष्ठा सूचकांक_Iterator {
  पूर्णांक eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  पूर्णांक i;        /* Index of the current item from the list */
  जोड़ {
    काष्ठा {    /* Use this object for eType==0: A Index.pNext list */
      सूचकांक *pIdx;   /* The current Index */
    } lx;     
    काष्ठा {    /* Use this object for eType==1; Array of IndexListTerm */
      पूर्णांक nIdx;               /* Size of the array */
      सूचकांक_List_Term *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
काष्ठा सूचकांक_List_Term {
  सूचकांक *p;  /* The index */
  पूर्णांक ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
अटल सूचकांक *सूचकांक_Iterator_First(सूचकांक_Iterator *pIter, पूर्णांक *pIx){
  निश्चित( pIter->i==0 );
  अगर( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    वापस pIter->u.ax.aIdx[0].p;
  }अन्यथा{
    *pIx = 0;
    वापस pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
अटल सूचकांक *सूचकांक_Iterator_अग्र(सूचकांक_Iterator *pIter, पूर्णांक *pIx){
  अगर( pIter->eType ){
    पूर्णांक i = ++pIter->i;
    अगर( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      वापस 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    वापस pIter->u.ax.aIdx[i].p;
  }अन्यथा{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->p_अग्र;
    वापस pIter->u.lx.pIdx;
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
व्योम sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  Table *pTab,         /* The table being inserted or updated */
  पूर्णांक *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  पूर्णांक i_तथ्य_Cur,        /* Canonical data cursor (main table or PK index) */
  पूर्णांक iIdxCur,         /* First index cursor */
  पूर्णांक reg_New_तथ्य,      /* First register in a range holding values to insert */
  पूर्णांक reg_Old_तथ्य,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  पूर्णांक ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  पूर्णांक *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  पूर्णांक *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  सूचकांक *pIdx;         /* Pointer to one of the indices */
  सूचकांक *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  पूर्णांक i;               /* loop counter */
  पूर्णांक ix;              /* Index loop counter */
  पूर्णांक nCol;            /* Number of columns */
  पूर्णांक onError;         /* Conflict resolution strategy */
  पूर्णांक seenReplace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  पूर्णांक nPkField;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 bAffinityDone = 0;  /* True if the OP_Affinity operation has been run */
  पूर्णांक upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  पूर्णांक upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  पूर्णांक ipk_शीर्ष = 0;        /* Top of the IPK uniqueness check */
  पूर्णांक ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  पूर्णांक regTrigCnt;       /* Register used to count replace trigger invocations */
  पूर्णांक addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  पूर्णांक lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  पूर्णांक nReplaceTrig = 0; /* Number of replace triggers coded */
  सूचकांक_Iterator sIdxIter;  /* Index iterator */

  isUpdate = reg_Old_तथ्य!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  निश्चित( v!=0 );
  निश्चित( !IsView(pTab) );  /* This table is not a VIEW */
  nCol = pTab->nCol;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  अगर( HasRowid(pTab) ){
    pPk = 0;
    nPkField = 1;
  }अन्यथा{
    pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
    nPkField = pPk->n_कुंजी_Col;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     i_तथ्य_Cur, iIdxCur, reg_New_तथ्य, reg_Old_तथ्य, pkChng));

  /* Test all NOT NULL constraints.
  */
  अगर( pTab->tabFlags & TF_HasNotNull ){
    पूर्णांक b2ndPass = 0;         /* True if currently running 2nd pass */
    पूर्णांक nSeenReplace = 0;     /* Number of ON CONFLICT REPLACE operations */
    पूर्णांक nGenerated = 0;       /* Number of generated columns with NOT NULL */
    जबतक(1){  /* Make 2 passes over columns. Exit loop via "break" */
      क्रम(i=0; i<nCol; i++){
        पूर्णांक iReg;                        /* Register holding column value */
        Column *pCol = &pTab->aCol[i];   /* The column to check for NOT NULL */
        पूर्णांक isGenerated;                 /* non-zero if column is generated */
        onError = pCol->notNull;
        अगर( onError==OE_None ) जारी; /* No NOT NULL on this column */
        अगर( i==pTab->iPKey ){
          जारी;        /* ROWID is never NULL */
        }
        isGenerated = pCol->colFlags & COLFLAG_GENERATED;
        अगर( isGenerated && !b2ndPass ){
          nGenerated++;
          जारी;        /* Generated columns processed on 2nd pass */
        }
        अगर( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          जारी;
        }
        अगर( overrideError!=OE_Default ){
          onError = overrideError;
        }अन्यथा अगर( onError==OE_Default ){
          onError = OE_Abort;
        }
        अगर( onError==OE_Replace ){
          अगर( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || pCol->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( pCol->colFlags & COLFLAG_VIRTUAL );
            testcase( pCol->colFlags & COLFLAG_STORED );
            testcase( pCol->colFlags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }अन्यथा{
            निश्चित( !isGenerated );
          }
        }अन्यथा अगर( b2ndPass && !isGenerated ){
          जारी;
        }
        निश्चित( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3TableColumnToStorage(pTab, i) );
        iReg = sqlite3TableColumnToStorage(pTab, i) + reg_New_तथ्य + 1;
        चयन( onError ){
          हाल OE_Replace: {
            पूर्णांक addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            निश्चित( (pCol->colFlags & COLFLAG_GENERATED)==0 );
            nSeenReplace++;
            sqlite3_व्यंजक_Code_Copy(pParse,
               sqlite3_Column_व्यंजक_Auth(pTab, pCol, pParse), iReg);
            sqlite3VdbeJumpHere(v, addr1);
            अवरोध;
          }
          हाल OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          हाल OE_Rollback:
          हाल OE_Fail: {
            अक्षर *z_संदेश = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        pCol->zCnName);
            testcase( z_संदेश==0 && db->mallocFailed==0 );
            sqlite3VdbeAddOp3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, z_संदेश, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            अवरोध;
          }
          शेष: {
            निश्चित( onError==OE_Ignore );
            sqlite3VdbeAddOp2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            अवरोध;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      अगर( nGenerated==0 && nSeenReplace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        अवरोध;
      }
      अगर( b2ndPass ) अवरोध;  /* Never need more than 2 passes */
      b2ndPass = 1;
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
      अगर( nSeenReplace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, reg_New_तथ्य+1, pTab);
      }
#पूर्ण_अगर
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#अगर_अघोषित SQLITE_OMIT_CHECK
  अगर( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    व्यंजक_List *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(reg_New_तथ्य+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    क्रम(i=0; i<pCheck->n_व्यंजक; i++){
      पूर्णांक allOk;
      व्यंजक *pCopy;
      व्यंजक *p_व्यंजक = pCheck->a[i].p_व्यंजक;
      अगर( aiChng
       && !sqlite3_व्यंजक_References_Updated_Column(p_व्यंजक, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        जारी;
      }
      अगर( bAffinityDone==0 ){
        sqlite3TableAffinity(v, pTab, reg_New_तथ्य+1);
        bAffinityDone = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3_व्यंजक_Dup(db, p_व्यंजक, 0);
      अगर( !db->mallocFailed ){
        sqlite3_व्यंजक_If_True(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3_व्यंजक_Delete(db, pCopy);
      अगर( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }अन्यथा{
        अक्षर *zName = pCheck->a[i].zEName;
        निश्चित( zName!=0 || pParse->db->mallocFailed );
        अगर( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#पूर्ण_अगर /* !defined(SQLITE_OMIT_CHECK) */

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
  sIdxIter.u.lx.pIdx = pTab->p_सूचकांक;
  अगर( pUpsert ){
    अगर( pUpsert->pUpsertTarget==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      निश्चित( pUpsert->p_अग्र_Upsert==0 );
      अगर( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }अन्यथा{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }अन्यथा अगर( pTab->p_सूचकांक!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      पूर्णांक nIdx, jj;
      u64 nByte;
      Upsert *pTerm;
      u8 *bUsed;
      क्रम(nIdx=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, nIdx++){
         निश्चित( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (माप(सूचकांक_List_Term)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      अगर( sIdxIter.u.ax.aIdx==0 ) वापस; /* OOM */
      bUsed = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->pToFree = sIdxIter.u.ax.aIdx;
      क्रम(i=0, pTerm=pUpsert; pTerm; pTerm=pTerm->p_अग्र_Upsert){
        अगर( pTerm->pUpsertTarget==0 ) अवरोध;
        अगर( pTerm->pUpsertIdx==0 ) जारी;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->p_सूचकांक;
        जबतक( ALWAYS(pIdx!=0) && pIdx!=pTerm->pUpsertIdx ){
           pIdx = pIdx->p_अग्र;
           jj++;
        }
        अगर( bUsed[jj] ) जारी; /* Duplicate ON CONFLICT clause ignored */
        bUsed[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      क्रम(jj=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, jj++){
        अगर( bUsed[jj] ) जारी;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      निश्चित( i==nIdx );
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
  अगर( (db->flags & (SQLITE_RecTriggers|SQLITE_Foreign_कुंजियाँ))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }अन्यथा{
    अगर( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }अन्यथा{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    अगर( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      अगर( overrideError!=OE_Replace ){
        अगर( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }अन्यथा अगर( pkChng==0 || pPk || pTab->कुंजी_Conf!=OE_Replace ){
          क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
            अगर( pIdx->onError==OE_Replace ) अवरोध;
          }
          अगर( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    अगर( regTrigCnt ){
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
  अगर( pkChng && pPk==0 ){
    पूर्णांक addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->कुंजी_Conf;
    अगर( overrideError!=OE_Default ){
      onError = overrideError;
    }अन्यथा अगर( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    अगर( pUpsert ){
      pUpsertClause = sqlite3_Upsert_Of_सूचकांक(pUpsert,0);
      अगर( pUpsertClause!=0 ){
        अगर( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }अन्यथा{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      अगर( pUpsertClause!=pUpsert ){
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
    अगर( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->p_सूचकांक             /* There exist other constraints */
    ){
      अगर( upsertIpkDelay ){
        ipk_शीर्ष = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }अन्यथा{
        ipk_शीर्ष = sqlite3VdbeAddOp0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    अगर( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3VdbeAddOp3(v, OP_Eq, reg_New_तथ्य, addrRowidOk, reg_Old_तथ्य);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3VdbeAddOp3(v, OP_NotExists, i_तथ्य_Cur, addrRowidOk, reg_New_तथ्य);
    VdbeCoverage(v);

    चयन( onError ){
      शेष: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      हाल OE_Rollback:
      हाल OE_Abort:
      हाल OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        अवरोध;
      }
      हाल OE_Replace: {
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
        अगर( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3GenerateRowDelete(pParse, pTab, pTrigger, i_तथ्य_Cur, iIdxCur,
                                   reg_New_तथ्य, 1, 0, OE_Replace, 1, -1);
          sqlite3VdbeAddOp2(v, OP_AddImm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }अन्यथा{
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
          निश्चित( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3VdbeAddOp2(v, OP_Delete, i_तथ्य_Cur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_TABLE);
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */
          अगर( pTab->p_सूचकांक ){
            sqlite3MultiWrite(pParse);
            sqlite3_Generate_Row_सूचकांक_Delete(pParse, pTab, i_तथ्य_Cur, iIdxCur,0,-1);
          }
        }
        seenReplace = 1;
        अवरोध;
      }
#अगर_अघोषित SQLITE_OMIT_UPSERT
      हाल OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, i_तथ्य_Cur);
        /* no break */ deliberate_fall_through
      }
#पूर्ण_अगर
      हाल OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        अवरोध;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    अगर( ipk_शीर्ष ){
      ipkBottom = sqlite3VdbeAddOp0(v, OP_Goto);
      sqlite3VdbeJumpHere(v, ipk_शीर्ष-1);
    }अन्यथा अगर( pUpsert && pUpsertClause!=pUpsert ){
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
  क्रम(pIdx = सूचकांक_Iterator_First(&sIdxIter, &ix);
      pIdx;
      pIdx = सूचकांक_Iterator_अग्र(&sIdxIter, &ix)
  ){
    पूर्णांक regIdx;          /* Range of registers holding content for pIdx */
    पूर्णांक regR;            /* Range of registers holding conflicting PK */
    पूर्णांक iThisCur;        /* Cursor for this UNIQUE index */
    पूर्णांक addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    पूर्णांक addrConflictCk;  /* First opcode in the conflict check logic */
    पूर्णांक nConflictCk;     /* Number of opcodes in conflict check logic */

    अगर( aRegIdx[ix]==0 ) जारी;  /* Skip indices that do not change */
    अगर( pUpsert ){
      pUpsertClause = sqlite3_Upsert_Of_सूचकांक(pUpsert, pIdx);
      अगर( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3VdbeJumpHere(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    अगर( bAffinityDone==0 ){
      sqlite3TableAffinity(v, pTab, reg_New_तथ्य+1);
      bAffinityDone = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    अगर( pIdx->pPartIdxWhere ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(reg_New_तथ्य+1);
      sqlite3_व्यंजक_If_False_Dup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    क्रम(i=0; i<pIdx->nColumn; i++){
      पूर्णांक iField = pIdx->aiColumn[i];
      पूर्णांक x;
      अगर( iField==XN_व्यंजक ){
        pParse->iSelfTab = -(reg_New_तथ्य+1);
        sqlite3_व्यंजक_Code_Copy(pParse, pIdx->a_Col_व्यंजक->a[i].p_व्यंजक, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }अन्यथा अगर( iField==XN_ROWID || iField==pTab->iPKey ){
        x = reg_New_तथ्य;
        sqlite3VdbeAddOp2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }अन्यथा{
        testcase( sqlite3TableColumnToStorage(pTab, iField)!=iField );
        x = sqlite3TableColumnToStorage(pTab, iField) + reg_New_तथ्य + 1;
        sqlite3VdbeAddOp2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->aCol[iField].zCnName));
      }
    }
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regIdx, pIdx->nColumn, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#अगर_घोषित SQLITE_ENABLE_NULL_TRIM
    अगर( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3SetMakeRecordP5(v, pIdx->pTable);
    }
#पूर्ण_अगर
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->nColumn, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    अगर( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      जारी;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    अगर( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      जारी;  /* pIdx is not a UNIQUE index */
    }
    अगर( overrideError!=OE_Default ){
      onError = overrideError;
    }अन्यथा अगर( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    अगर( pUpsertClause ){
      अगर( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }अन्यथा{
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
    निश्चित( IsOrdinaryTable(pTab) );
#अगर_अघोषित SQLITE_ENABLE_PREUPDATE_HOOK
    अगर( (ix==0 && pIdx->p_अग्र==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_Foreign_कुंजियाँ) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      जारी;
    }
#पूर्ण_अगर /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3VdbeAddOp4Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->n_कुंजी_Col); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3_Get_अस्थायी_Range(pParse, nPkField);
    अगर( isUpdate || onError==OE_Replace ){
      अगर( HasRowid(pTab) ){
        sqlite3VdbeAddOp2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        अगर( isUpdate ){
          sqlite3VdbeAddOp3(v, OP_Eq, regR, addrUniqueOk, reg_Old_तथ्य);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }अन्यथा{
        पूर्णांक x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        अगर( pIdx!=pPk ){
          क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
            निश्चित( pPk->aiColumn[i]>=0 );
            x = sqlite3_Table_Column_To_सूचकांक(pIdx, pPk->aiColumn[i]);
            sqlite3VdbeAddOp3(v, OP_Column, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->aCol[pPk->aiColumn[i]].zCnName));
          }
        }
        अगर( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          पूर्णांक addrJump = sqlite3VdbeCurrentAddr(v)+pPk->n_कुंजी_Col;
          पूर्णांक op = OP_Ne;
          पूर्णांक regCmp = (Is_Primary_कुंजी_सूचकांक(pIdx) ? regIdx : regR);
 
          क्रम(i=0; i<pPk->n_कुंजी_Col; i++){
            अक्षर *p4 = (अक्षर*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->aiColumn[i];
            निश्चित( x>=0 );
            अगर( i==(pPk->n_कुंजी_Col-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3TableColumnToStorage(pTab, x);
            sqlite3VdbeAddOp4(v, op,
                reg_Old_तथ्य+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    निश्चित( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    निश्चित( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    चयन( onError ){
      हाल OE_Rollback:
      हाल OE_Abort:
      हाल OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        अवरोध;
      }
#अगर_अघोषित SQLITE_OMIT_UPSERT
      हाल OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#पूर्ण_अगर
      हाल OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        अवरोध;
      }
      शेष: {
        निश्चित( onError==OE_Replace );
        अगर( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        अगर( pTrigger && isUpdate ){
          sqlite3VdbeAddOp1(v, OP_CursorLock, i_तथ्य_Cur);
        }
        sqlite3GenerateRowDelete(pParse, pTab, pTrigger, i_तथ्य_Cur, iIdxCur,
            regR, nPkField, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        अगर( pTrigger && isUpdate ){
          sqlite3VdbeAddOp1(v, OP_CursorUnlock, i_तथ्य_Cur);
        }
        seenReplace = 1;
        अवरोध;
      }
    }
    अगर( regTrigCnt ){
      पूर्णांक addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3VdbeAddOp2(v, OP_AddImm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3VdbeAddOp0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      अगर( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3VdbeAddOp2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      जबतक( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        अगर( x.opcode!=OP_IdxRowid || isUpdate ){
          पूर्णांक p2;      /* New P2 value for copied conflict check opcode */
          स्थिर अक्षर *zP4;
          अगर( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }अन्यथा{
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
    अगर( regR!=regIdx ) sqlite3_Release_अस्थायी_Range(pParse, regR, nPkField);
    अगर( pUpsertClause
     && upsertIpkReturn
     && sqlite3_Upsert_अग्र_Is_IPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3VdbeJumpHere(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  अगर( ipk_शीर्ष ){
    sqlite3VdbeGoto(v, ipk_शीर्ष);
    VdbeComment((v, "Do IPK REPLACE"));
    निश्चित( ipkBottom>0 );
    sqlite3VdbeJumpHere(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  निश्चित( regTrigCnt!=0 || nReplaceTrig==0 );
  अगर( nReplaceTrig ){
    sqlite3VdbeAddOp2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    अगर( !pPk ){
      अगर( isUpdate ){
        sqlite3VdbeAddOp3(v, OP_Eq, reg_New_तथ्य, addrRecheck, reg_Old_तथ्य);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp3(v, OP_NotExists, i_तथ्य_Cur, addrRecheck, reg_New_तथ्य);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }अन्यथा{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  अगर( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  अगर( HasRowid(pTab) ){
    पूर्णांक regRec = aRegIdx[ix];
    sqlite3VdbeAddOp3(v, OP_MakeRecord, reg_New_तथ्य+1, pTab->nNVCol, regRec);
    sqlite3SetMakeRecordP5(v, pTab);
    अगर( !bAffinityDone ){
      sqlite3TableAffinity(v, pTab, 0);
    }
  }

  *pbMayReplace = seenReplace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", seenReplace));
}

#अगर_घोषित SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
व्योम sqlite3SetMakeRecordP5(Vdbe *v, Table *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  अगर( pTab->pSchema->file_format<2 ) वापस;

  क्रम(i=pTab->nCol-1; i>0; i--){
    अगर( pTab->aCol[i].iDflt!=0 ) अवरोध;
    अगर( pTab->aCol[i].colFlags & COLFLAG_PRIMKEY ) अवरोध;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#पूर्ण_अगर

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
अटल व्योम codeWithoutRowidPreupdate(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated */
  पूर्णांक iCur,                       /* Cursor number for table */
  पूर्णांक reg_तथ्य                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णांक r = sqlite3_Get_अस्थायी_Reg(pParse);
  निश्चित( !HasRowid(pTab) );
  निश्चित( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3VdbeAddOp2(v, OP_Integer, 0, r);
  sqlite3VdbeAddOp4(v, OP_जोड़ो, iCur, reg_तथ्य, r, (अक्षर*)pTab, P4_TABLE);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3_Release_अस्थायी_Reg(pParse, r);
}
#अन्यथा
# define codeWithoutRowidPreupdate(a,b,c,d)
#पूर्ण_अगर

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
व्योम sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  Table *pTab,        /* the table into which we are inserting */
  पूर्णांक i_तथ्य_Cur,       /* Cursor of the canonical data source */
  पूर्णांक iIdxCur,        /* First index cursor */
  पूर्णांक reg_New_तथ्य,     /* Range of content */
  पूर्णांक *aRegIdx,       /* Register used by each index.  0 for unused indices */
  पूर्णांक update_flags,   /* True for UPDATE, False for INSERT */
  पूर्णांक appendBias,     /* True if this is likely to be an append */
  पूर्णांक use_Seek_परिणाम   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  सूचकांक *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  पूर्णांक i;              /* Loop counter */

  निश्चित( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  निश्चित( v!=0 );
  निश्चित( !IsView(pTab) );  /* This table is not a VIEW */
  क्रम(i=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, i++){
    /* All REPLACE indexes are at the end of the list */
    निश्चित( pIdx->onError!=OE_Replace
         || pIdx->p_अग्र==0
         || pIdx->p_अग्र->onError==OE_Replace );
    अगर( aRegIdx[i]==0 ) जारी;
    अगर( pIdx->pPartIdxWhere || (update_flags && pIdx->b_Has_व्यंजक) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3VdbeAddOp2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (use_Seek_परिणाम ? OPFLAG_USESEEKRESULT : 0);
    अगर( Is_Primary_कुंजी_सूचकांक(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      अगर( update_flags==0 ){
        codeWithoutRowidPreupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3VdbeAddOp4Int(v, OP_Idx_जोड़ो, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->n_कुंजी_Col: pIdx->nColumn);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  अगर( !HasRowid(pTab) ) वापस;
  अगर( pParse->nested ){
    pik_flags = 0;
  }अन्यथा{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  अगर( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  अगर( use_Seek_परिणाम ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3VdbeAddOp3(v, OP_जोड़ो, i_तथ्य_Cur, aRegIdx[i], reg_New_तथ्य);
  अगर( !pParse->nested ){
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
पूर्णांक sqlite3OpenTableAndIndices(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* Table to be opened */
  पूर्णांक op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  पूर्णांक iBase,       /* Use this for the table cursor, if there is one */
  u8 *aToOpen,     /* If not NULL: boolean for each table and index */
  पूर्णांक *pi_तथ्य_Cur,  /* Write the database source cursor number here */
  पूर्णांक *piIdxCur    /* Write the first index cursor number here */
){
  पूर्णांक i;
  पूर्णांक iDb;
  पूर्णांक i_तथ्य_Cur;
  सूचकांक *pIdx;
  Vdbe *v;

  निश्चित( op==OP_OpenRead || op==OP_OpenWrite );
  निश्चित( op==OP_OpenWrite || p5==0 );
  निश्चित( pi_तथ्य_Cur!=0 );
  निश्चित( piIdxCur!=0 );
  अगर( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *pi_तथ्य_Cur = *piIdxCur = -999;
    वापस 0;
  }
  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  निश्चित( v!=0 );
  अगर( iBase<0 ) iBase = pParse->nTab;
  i_तथ्य_Cur = iBase++;
  *pi_तथ्य_Cur = i_तथ्य_Cur;
  अगर( HasRowid(pTab) && (aToOpen==0 || aToOpen[0]) ){
    sqlite3OpenTable(pParse, i_तथ्य_Cur, iDb, pTab, op);
  }अन्यथा अगर( pParse->db->noSharedCache==0 ){
    sqlite3TableLock(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = iBase;
  क्रम(i=0, pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र, i++){
    पूर्णांक iIdxCur = iBase++;
    निश्चित( pIdx->pSchema==pTab->pSchema );
    अगर( Is_Primary_कुंजी_सूचकांक(pIdx) && !HasRowid(pTab) ){
      *pi_तथ्य_Cur = iIdxCur;
      p5 = 0;
    }
    अगर( aToOpen==0 || aToOpen[i+1] ){
      sqlite3VdbeAddOp3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  अगर( iBase>pParse->nTab ) pParse->nTab = iBase;
  वापस i;
}


#अगर_घोषित SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
पूर्णांक sqlite3_xferopt_गिनती;
#पूर्ण_अगर /* SQLITE_TEST */


#अगर_अघोषित SQLITE_OMIT_XFER_OPT
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
अटल पूर्णांक xfer_Compatible_सूचकांक(सूचकांक *pDest, सूचकांक *pSrc){
  पूर्णांक i;
  निश्चित( pDest && pSrc );
  निश्चित( pDest->pTable!=pSrc->pTable );
  अगर( pDest->n_कुंजी_Col!=pSrc->n_कुंजी_Col || pDest->nColumn!=pSrc->nColumn ){
    वापस 0;   /* Different number of columns */
  }
  अगर( pDest->onError!=pSrc->onError ){
    वापस 0;   /* Different conflict resolution strategies */
  }
  क्रम(i=0; i<pSrc->n_कुंजी_Col; i++){
    अगर( pSrc->aiColumn[i]!=pDest->aiColumn[i] ){
      वापस 0;   /* Different columns indexed */
    }
    अगर( pSrc->aiColumn[i]==XN_व्यंजक ){
      निश्चित( pSrc->a_Col_व्यंजक!=0 && pDest->a_Col_व्यंजक!=0 );
      अगर( sqlite3_व्यंजक_Compare(0, pSrc->a_Col_व्यंजक->a[i].p_व्यंजक,
                             pDest->a_Col_व्यंजक->a[i].p_व्यंजक, -1)!=0 ){
        वापस 0;   /* Different expressions in the index */
      }
    }
    अगर( pSrc->aSortOrder[i]!=pDest->aSortOrder[i] ){
      वापस 0;   /* Different sort orders */
    }
    अगर( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      वापस 0;   /* Different collating sequences */
    }
  }
  अगर( sqlite3_व्यंजक_Compare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    वापस 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  वापस 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
अटल पूर्णांक xferCheckRowid(Walker *pWalk, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_COLUMN && p_व्यंजक->iColumn<0 ){
    pWalk->eCode = 1;
    वापस WRC_Abort;
  }अन्यथा{
    वापस WRC_Continue;
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
अटल पूर्णांक xferCompatibleCheck(Table *pDest, Table *pSrc){
  अगर( sqlite3_व्यंजक_List_Compare(pSrc->pCheck,pDest->pCheck,-1) ){
    वापस 0;
  }
  अगर( pDest->iPKey<0 ){
    Walker w;
    स_रखो(&w, 0, माप(w));
    w.x_व्यंजक_Callback = xferCheckRowid;
    sqlite3_Walk_व्यंजक_List(&w,pDest->pCheck);
    अगर( w.eCode ) वापस 0;
  }
  वापस 1;
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
अटल पूर्णांक xferOptimization(
  Parse *pParse,        /* Parser context */
  Table *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  पूर्णांक onError,          /* How to handle constraint errors */
  पूर्णांक iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  व्यंजक_List *pEList;                /* The result set of the SELECT */
  Table *pSrc;                     /* The table in the FROM clause of SELECT */
  सूचकांक *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  SrcItem *pItem;                  /* An element of pSelect->pSrc */
  पूर्णांक i;                           /* Loop counter */
  पूर्णांक iDbSrc;                      /* The database of pSrc */
  पूर्णांक iSrc, iDest;                 /* Cursors from source and destination */
  पूर्णांक addr1, addr2;                /* Loop addresses */
  पूर्णांक emptyDestTest = 0;           /* Address of test for empty pDest */
  पूर्णांक emptySrcTest = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  पूर्णांक regAutoinc;                  /* Memory register used by AUTOINC */
  पूर्णांक destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  पूर्णांक reg_तथ्य, regRowid;           /* Registers holding data and rowid */

  निश्चित( pSelect!=0 );
  अगर( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    वापस 0;
  }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अगर( IsVirtual(pDest) ){
    वापस 0;   /* tab1 must not be a virtual table */
  }
#पूर्ण_अगर
  अगर( onError==OE_Default ){
    अगर( pDest->iPKey>=0 ) onError = pDest->कुंजी_Conf;
    अगर( onError==OE_Default ) onError = OE_Abort;
  }
  निश्चित(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  अगर( pSelect->pSrc->nSrc!=1 ){
    वापस 0;   /* FROM clause must have exactly one term */
  }
  अगर( pSelect->pSrc->a[0].fg.isSubquery ){
    वापस 0;   /* FROM clause cannot contain a subquery */
  }
  अगर( pSelect->pWhere ){
    वापस 0;   /* SELECT may not have a WHERE clause */
  }
  अगर( pSelect->pOrderBy ){
    वापस 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  अगर( pSelect->pGroupBy ){
    वापस 0;   /* SELECT may not have a GROUP BY clause */
  }
  अगर( pSelect->pLimit ){
    वापस 0;   /* SELECT may not have a LIMIT clause */
  }
  अगर( pSelect->pPrior ){
    वापस 0;   /* SELECT may not be a compound query */
  }
  अगर( pSelect->selFlags & SF_Distinct ){
    वापस 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  निश्चित( pEList!=0 );
  अगर( pEList->n_व्यंजक!=1 ){
    वापस 0;   /* The result set must have exactly one column */
  }
  निश्चित( pEList->a[0].p_व्यंजक );
  अगर( pEList->a[0].p_व्यंजक->op!=TK_ASTERISK ){
    वापस 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  pItem = pSelect->pSrc->a;
  pSrc = sqlite3LocateTableItem(pParse, 0, pItem);
  अगर( pSrc==0 ){
    वापस 0;   /* FROM clause does not contain a real table */
  }
  अगर( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    वापस 0;   /* tab1 and tab2 may not be the same table */
  }
  अगर( HasRowid(pDest)!=HasRowid(pSrc) ){
    वापस 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  अगर( !IsOrdinaryTable(pSrc) ){
    वापस 0;   /* tab2 may not be a view or virtual table */
  }
  अगर( pDest->nCol!=pSrc->nCol ){
    वापस 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  अगर( pDest->iPKey!=pSrc->iPKey ){
    वापस 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  अगर( (pDest->tabFlags & TF_Strict)!=0 ){
    अगर( (pSrc->tabFlags & TF_Strict)==0 ){
      वापस 0;  /* Cannot feed from a non-strict into a strict table */
    }
    क्रम(i=0; i<pDest->nCol; i++){
      अचिन्हित eDestType = pDest->aCol[i].eCType;
      अचिन्हित eSrcType = pSrc->aCol[i].eCType;
      अगर( eDestType==COLTYPE_ANY ) जारी;
      अगर( eDestType==eSrcType ) जारी;
      अगर( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) जारी;
      अगर( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) जारी;
      वापस 0; /* Incompatible types in source and destination */
    }
  }
  क्रम(i=0; i<pDest->nCol; i++){
    Column *pDestCol = &pDest->aCol[i];
    Column *pSrcCol = &pSrc->aCol[i];
#अगर_घोषित SQLITE_ENABLE_HIDDEN_COLUMNS
    अगर( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (pDestCol->colFlags | pSrcCol->colFlags) & COLFLAG_HIDDEN
    ){
      वापस 0;    /* Neither table may have __hidden__ columns */
    }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
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
    अगर( (pDestCol->colFlags & COLFLAG_GENERATED) !=
        (pSrcCol->colFlags & COLFLAG_GENERATED) ){
      वापस 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    अगर( (pDestCol->colFlags & COLFLAG_GENERATED)!=0 ){
      अगर( sqlite3_व्यंजक_Compare(0,
             sqlite3_Column_व्यंजक(pSrc, pSrcCol),
             sqlite3_Column_व्यंजक(pDest, pDestCol), -1)!=0 ){
        testcase( pDestCol->colFlags & COLFLAG_VIRTUAL );
        testcase( pDestCol->colFlags & COLFLAG_STORED );
        वापस 0;  /* Different generator expressions */
      }
    }
#पूर्ण_अगर
    अगर( pDestCol->affinity!=pSrcCol->affinity ){
      वापस 0;    /* Affinity must be the same on all columns */
    }
    अगर( sqlite3_stricmp(sqlite3ColumnColl(pDestCol),
                        sqlite3ColumnColl(pSrcCol))!=0 ){
      वापस 0;    /* Collating sequence must be the same on all columns */
    }
    अगर( pDestCol->notNull && !pSrcCol->notNull ){
      वापस 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    अगर( (pDestCol->colFlags & COLFLAG_GENERATED)==0 && i>0 ){
      व्यंजक *p_Dest_व्यंजक = sqlite3_Column_व्यंजक(pDest, pDestCol);
      व्यंजक *p_Src_व्यंजक = sqlite3_Column_व्यंजक(pSrc, pSrcCol);
      निश्चित( p_Dest_व्यंजक==0 || p_Dest_व्यंजक->op==TK_SPAN );
      निश्चित( p_Dest_व्यंजक==0 || !व्यंजक_Has_Property(p_Dest_व्यंजक, EP_Int_मूल्य) );
      निश्चित( p_Src_व्यंजक==0 || p_Src_व्यंजक->op==TK_SPAN );
      निश्चित( p_Src_व्यंजक==0 || !व्यंजक_Has_Property(p_Src_व्यंजक, EP_Int_मूल्य) );
      अगर( (p_Dest_व्यंजक==0)!=(p_Src_व्यंजक==0)
       || (p_Dest_व्यंजक!=0 && म_भेद(p_Dest_व्यंजक->u.zToken,
                                       p_Src_व्यंजक->u.zToken)!=0)
      ){
        वापस 0;    /* Default values must be the same for all columns */
      }
    }
  }
  क्रम(pDestIdx=pDest->p_सूचकांक; pDestIdx; pDestIdx=pDestIdx->p_अग्र){
    अगर( Is_Unique_सूचकांक(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    क्रम(pSrcIdx=pSrc->p_सूचकांक; pSrcIdx; pSrcIdx=pSrcIdx->p_अग्र){
      अगर( xfer_Compatible_सूचकांक(pDestIdx, pSrcIdx) ) अवरोध;
    }
    अगर( pSrcIdx==0 ){
      वापस 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    अगर( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      वापस 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#अगर_अघोषित SQLITE_OMIT_CHECK
  अगर( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    वापस 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  निश्चित( IsOrdinaryTable(pDest) );
  अगर( (db->flags & SQLITE_Foreign_कुंजियाँ)!=0 && pDest->u.tab.pFKey!=0 ){
    वापस 0;
  }
#पूर्ण_अगर
  अगर( (db->flags & SQLITE_गिनती_Rows)!=0 ){
    वापस 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( db->xAuth ){
    पूर्णांक iDb = sqlite3_Schema_To_सूचकांक(db, pSrc->pSchema);
    अगर( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) वापस 0;
    क्रम(i=0; i<pSrc->nCol; i++){
      Column *pSrcCol = &pSrc->aCol[i];
      अगर( sqlite3AuthReadCol(pParse, pSrc->zName, pSrcCol->zCnName, iDb) ){
        वापस 0;
      }
    }
  }
#पूर्ण_अगर

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#अगर_घोषित SQLITE_TEST
  sqlite3_xferopt_गिनती++;
#पूर्ण_अगर
  iDbSrc = sqlite3_Schema_To_सूचकांक(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3CodeVerifySchema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  reg_तथ्य = sqlite3_Get_अस्थायी_Reg(pParse);
  sqlite3VdbeAddOp2(v, OP_Null, 0, reg_तथ्य);
  regRowid = sqlite3_Get_अस्थायी_Reg(pParse);
  sqlite3OpenTable(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  निश्चित( HasRowid(pDest) || destHasUniqueIdx );
  अगर( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->p_सूचकांक!=0)          /* (1) */
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
  अगर( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3OpenTable(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    emptySrcTest = sqlite3VdbeAddOp2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    अगर( pDest->iPKey>=0 ){
      addr1 = sqlite3VdbeAddOp2(v, OP_Rowid, iSrc, regRowid);
      अगर( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3VdbeAddOp3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3VdbeJumpHere(v, addr2);
      }
      autoIncStep(pParse, regAutoinc, regRowid);
    }अन्यथा अगर( pDest->p_सूचकांक==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3VdbeAddOp2(v, OP_NewRowid, iDest, regRowid);
    }अन्यथा{
      addr1 = sqlite3VdbeAddOp2(v, OP_Rowid, iSrc, regRowid);
      निश्चित( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    अगर( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3VdbeAddOp1(v, OP_SeekEnd, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }अन्यथा{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
    अगर( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeAddOp3(v, OP_Row_तथ्य, iSrc, reg_तथ्य, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }अन्यथा
#पूर्ण_अगर
    {
      sqlite3VdbeAddOp3(v, OP_RowCell, iDest, iSrc, regRowid);
    }
    sqlite3VdbeAddOp3(v, OP_जोड़ो, iDest, reg_तथ्य, regRowid);
    अगर( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (अक्षर*)pDest, P4_TABLE);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3VdbeAddOp2(v, OP_अग्र, iSrc, addr1); VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Close, iSrc, 0);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
  }अन्यथा{
    sqlite3TableLock(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3TableLock(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  क्रम(pDestIdx=pDest->p_सूचकांक; pDestIdx; pDestIdx=pDestIdx->p_अग्र){
    u8 idxInsFlags = 0;
    क्रम(pSrcIdx=pSrc->p_सूचकांक; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->p_अग्र){
      अगर( xfer_Compatible_सूचकांक(pDestIdx, pSrcIdx) ) अवरोध;
    }
    निश्चित( pSrcIdx );
    sqlite3VdbeAddOp3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3VdbeAddOp3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3VdbeAddOp2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    अगर( db->mDbFlags & DBFLAG_Vacuum ){
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
      क्रम(i=0; i<pSrcIdx->nColumn; i++){
        स्थिर अक्षर *zColl = pSrcIdx->azColl[i];
        अगर( sqlite3_stricmp(sqlite3_Str_द्विआधारी, zColl) ) अवरोध;
      }
      अगर( i==pSrcIdx->nColumn ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3VdbeAddOp1(v, OP_SeekEnd, iDest);
        sqlite3VdbeAddOp2(v, OP_RowCell, iDest, iSrc);
      }
    }अन्यथा अगर( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    अगर( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3VdbeAddOp3(v, OP_Row_तथ्य, iSrc, reg_तथ्य, 1);
      अगर( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && Is_Primary_कुंजी_सूचकांक(pDestIdx)
      ){
        codeWithoutRowidPreupdate(pParse, pDest, iDest, reg_तथ्य);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Idx_जोड़ो, iDest, reg_तथ्य);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3VdbeAddOp2(v, OP_अग्र, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp2(v, OP_Close, iSrc, 0);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
  }
  अगर( emptySrcTest ) sqlite3VdbeJumpHere(v, emptySrcTest);
  sqlite3_Release_अस्थायी_Reg(pParse, regRowid);
  sqlite3_Release_अस्थायी_Reg(pParse, reg_तथ्य);
  अगर( emptyDestTest ){
    sqlite3AutoincrementEnd(pParse);
    sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3VdbeJumpHere(v, emptyDestTest);
    sqlite3VdbeAddOp2(v, OP_Close, iDest, 0);
    वापस 0;
  }अन्यथा{
    वापस 1;
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_XFER_OPT */
