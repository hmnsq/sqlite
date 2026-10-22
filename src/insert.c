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
#అన్తర్భావ "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
శూన్య sqlite3_Open_పట్టిక(
  Parse *pParse,  /* Generate code into this VDBE */
  పూర్ణాంకం iCur,       /* The cursor number of the table */
  పూర్ణాంకం iDb,        /* The database index in sqlite3.aDb[] */
  పట్టిక *pTab,    /* The table to be opened */
  పూర్ణాంకం opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  ప్రతిజ్ఞా( !IsVirtual(pTab) );
  ప్రతిజ్ఞా( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  ఒకవేళ( !pParse->db->noSharedCache ){
    sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  ఒకవేళ( HasRowid(pTab) ){
    sqlite3_Vdbe_కూడు_Op4_Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }లేకపోతే{
    సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
    ప్రతిజ్ఞా( pPk!=0 );
    ప్రతిజ్ఞా( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3_Vdbe_కూడు_Op3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pPk);
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
స్థిర SQLITE_NOINLINE స్థిరరాశి అక్షరం *compute_సూచీ_Aff_Str(sqlite3 *db, సూచీ *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  పూర్ణాంకం n;
  పట్టిక *pTab = pIdx->p_పట్టిక;
  pIdx->z_నిలువు_Aff = (అక్షరం *)sqlite3DbMallocRaw(0, pIdx->n_స్తంభం+1);
  ఒకవేళ( !pIdx->z_నిలువు_Aff ){
    sqlite3OomFault(db);
    తిరిగి 0;
  }
  కొరకు(n=0; n<pIdx->n_స్తంభం; n++){
    i16 x = pIdx->ai_స్తంభం[n];
    అక్షరం aff;
    ఒకవేళ( x>=0 ){
      aff = pTab->a_నిలువు[x].affinity;
    }లేకపోతే ఒకవేళ( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }లేకపోతే{
      ప్రతిజ్ఞా( x==XN_సమాసం );
      ప్రతిజ్ఞా( pIdx->b_Has_సమాసం );
      ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం!=0 );
      aff = sqlite3_సమాసం_Affinity(pIdx->a_నిలువు_సమాసం->a[n].p_సమాసం);
    }
    ఒకవేళ( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    ఒకవేళ( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->z_నిలువు_Aff[n] = aff;
  }
  pIdx->z_నిలువు_Aff[n] = 0;
  తిరిగి pIdx->z_నిలువు_Aff;
}
స్థిరరాశి అక్షరం *sqlite3_సూచీ_Affinity_Str(sqlite3 *db, సూచీ *pIdx){
  ఒకవేళ( !pIdx->z_నిలువు_Aff ) తిరిగి compute_సూచీ_Aff_Str(db, pIdx);
  తిరిగి pIdx->z_నిలువు_Aff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
అక్షరం *sqlite3_పట్టిక_Affinity_Str(sqlite3 *db, స్థిరరాశి పట్టిక *pTab){
  అక్షరం *z_నిలువు_Aff;
  z_నిలువు_Aff = (అక్షరం *)sqlite3DbMallocRaw(db, pTab->n_నిలువు+1);
  ఒకవేళ( z_నిలువు_Aff ){
    పూర్ణాంకం i, j;
    కొరకు(i=j=0; i<pTab->n_నిలువు; i++){
      ఒకవేళ( (pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL)==0 ){
        z_నిలువు_Aff[j++] = pTab->a_నిలువు[i].affinity;
      }
    }
    చేయి{
      z_నిలువు_Aff[j--] = 0;
    }అయితే( j>=0 && z_నిలువు_Aff[j]<=SQLITE_AFF_BLOB );
  }
  తిరిగి z_నిలువు_Aff; 
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
శూన్య sqlite3_పట్టిక_Affinity(Vdbe *v, పట్టిక *pTab, పూర్ణాంకం iReg){
  పూర్ణాంకం i;
  అక్షరం *z_నిలువు_Aff;
  ఒకవేళ( pTab->tabFlags & TF_Strict ){
    ఒకవేళ( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *pPrev;
      పూర్ణాంకం p3;
      sqlite3VdbeAppendP4(v, pTab, P4_పట్టిక);
      pPrev = sqlite3_Vdbe_Get_చివరి_Op(v);
      ప్రతిజ్ఞా( pPrev!=0 );
      ప్రతిజ్ఞా( pPrev->opcode==OP_Make_రికార్డు || sqlite3VdbeDb(v)->mallocFailed );
      pPrev->opcode = OP_TypeCheck;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, pPrev->p1, pPrev->p2, p3);
    }లేకపోతే{
      /* Insert an isolated OP_Typecheck */
      sqlite3_Vdbe_కూడు_Op2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_పట్టిక);
    }
    తిరిగి;
  }
  z_నిలువు_Aff = pTab->z_నిలువు_Aff;
  ఒకవేళ( z_నిలువు_Aff==0 ){
    z_నిలువు_Aff = sqlite3_పట్టిక_Affinity_Str(0, pTab);
    ఒకవేళ( !z_నిలువు_Aff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      తిరిగి;
    }
    pTab->z_నిలువు_Aff = z_నిలువు_Aff;
  }
  ప్రతిజ్ఞా( z_నిలువు_Aff!=0 );
  i = sqlite3Strlen30NN(z_నిలువు_Aff);
  ఒకవేళ( i ){
    ఒకవేళ( iReg ){
      sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, iReg, i, 0, z_నిలువు_Aff, i);
    }లేకపోతే{
      ప్రతిజ్ఞా( sqlite3_Vdbe_Get_చివరి_Op(v)->opcode==OP_Make_రికార్డు
              || sqlite3VdbeDb(v)->mallocFailed );
      sqlite3VdbeChangeP4(v, -1, z_నిలువు_Aff, i);
    }
  }
}

/*
** Return non-zero if the table pTab in database iDb or any of its indices
** have been opened at any point in the VDBE program. This is used to see if
** a statement of the form  "INSERT INTO <iDb, pTab> SELECT ..." can
** run without using a temporary table for the results of the SELECT.
*/
స్థిర పూర్ణాంకం reads_పట్టిక(Parse *p, పూర్ణాంకం iDb, పట్టిక *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  పూర్ణాంకం i;
  పూర్ణాంకం i_చివర = sqlite3VdbeCurrentAddr(v);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#అయితే_ముగింపు

  కొరకు(i=1; i<i_చివర; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    ప్రతిజ్ఞా( pOp!=0 );
    ఒకవేళ( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      సూచీ *p_సూచీ;
      Pgno tnum = pOp->p2;
      ఒకవేళ( tnum==pTab->tnum ){
        తిరిగి 1;
      }
      కొరకు(p_సూచీ=pTab->p_సూచీ; p_సూచీ; p_సూచీ=p_సూచీ->p_తదుపరి){
        ఒకవేళ( tnum==p_సూచీ->tnum ){
          తిరిగి 1;
        }
      }
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    ఒకవేళ( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      ప్రతిజ్ఞా( pOp->p4.pVtab!=0 );
      ప్రతిజ్ఞా( pOp->p4type==P4_VTAB );
      తిరిగి 1;
    }
#అయితే_ముగింపు
  }
  తిరిగి 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
స్థిర పూర్ణాంకం సమాసం_స్తంభం_Flag_Union(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం && p_సమాసం->i_స్తంభం>=0 ){
    ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం < pWalker->u.pTab->n_నిలువు );
    pWalker->e_సంకేతం |= pWalker->u.pTab->a_నిలువు[p_సమాసం->i_స్తంభం].నిలువు_Flags;
  }
  తిరిగి WRC_Continue;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
శూన్య sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  పూర్ణాంకం iRegStore,    /* Register holding the first column */
  పట్టిక *pTab       /* The table */
){
  పూర్ణాంకం i;
  Walker w;
  స్తంభం *pRedo;
  పూర్ణాంకం e_పురోగతి;
  VdbeOp *pOp;

  ప్రతిజ్ఞా( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3_పట్టిక_Affinity(pParse->pVdbe, pTab, iRegStore);
  ఒకవేళ( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3_Vdbe_Get_చివరి_Op(pParse->pVdbe);
    ఒకవేళ( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      పూర్ణాంకం ii, jj;
      అక్షరం *zP4 = pOp->p4.z;
      ప్రతిజ్ఞా( zP4!=0 );
      ప్రతిజ్ఞా( pOp->p4type==P4_DYNAMIC );
      కొరకు(ii=jj=0; zP4[jj]; ii++){
        ఒకవేళ( pTab->a_నిలువు[ii].నిలువు_Flags & COLFLAG_VIRTUAL ){
          కొనసాగు;
        }
        ఒకవేళ( pTab->a_నిలువు[ii].నిలువు_Flags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }లేకపోతే ఒకవేళ( pOp->opcode==OP_TypeCheck ){
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
  కొరకు(i=0; i<pTab->n_నిలువు; i++){
    ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED ){
      testcase( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL );
      testcase( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_STORED );
      pTab->a_నిలువు[i].నిలువు_Flags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.x_సమాసం_Callback = సమాసం_స్తంభం_Flag_Union;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  చేయి{
    e_పురోగతి = 0;
    pRedo = 0;
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      స్తంభం *p_నిలువు = pTab->a_నిలువు + i;
      ఒకవేళ( (p_నిలువు->నిలువు_Flags & COLFLAG_NOTAVAIL)!=0 ){
        పూర్ణాంకం x;
        p_నిలువు->నిలువు_Flags |= COLFLAG_BUSY;
        w.e_సంకేతం = 0;
        sqlite3_Walk_సమాసం(&w, sqlite3_స్తంభం_సమాసం(pTab, p_నిలువు));
        p_నిలువు->నిలువు_Flags &= ~COLFLAG_BUSY;
        ఒకవేళ( w.e_సంకేతం & COLFLAG_NOTAVAIL ){
          pRedo = p_నిలువు;
          కొనసాగు;
        }
        e_పురోగతి = 1;
        ప్రతిజ్ఞా( p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED );
        x = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i) + iRegStore;
        sqlite3_సమాసం_సంకేతం_Generated_స్తంభం(pParse, pTab, p_నిలువు, x);
        p_నిలువు->నిలువు_Flags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }అయితే( pRedo && e_పురోగతి );
  ఒకవేళ( pRedo ){
    sqlite3_Error_సందేశం(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#అయితే_ముగింపు /* SQLITE_OMIT_GENERATED_COLUMNS */


#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
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
స్థిర పూర్ణాంకం autoIncBegin(
  Parse *pParse,      /* Parsing context */
  పూర్ణాంకం iDb,            /* Index of the database holding pTab */
  పట్టిక *pTab         /* The table we are writing to */
){
  పూర్ణాంకం memId = 0;      /* Register holding maximum rowid */
  ప్రతిజ్ఞా( pParse->db->aDb[iDb].pSchema!=0 );
  ఒకవేళ( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    పట్టిక *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    ఒకవేళ( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->n_నిలువు!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      తిరిగి 0;
    }

    ఒకవేళ( pToplevel->ఉపయోగిస్తుంది_Ainc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    అయితే( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->p_తదుపరి; }
    ఒకవేళ( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, పరిమాణం(*pInfo));
      sqlite3_Parser_కూడు_Cleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      ఒకవేళ( pParse->db->mallocFailed ) తిరిగి 0;
      pInfo->p_తదుపరి = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pToplevel->ఉపయోగిస్తుంది_Ainc = 1;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;                  /* Register to hold name of table */
      pInfo->regCtr = ++pToplevel->nMem;  /* Max rowid register */
      pToplevel->nMem +=2;       /* Rowid in sqlite_sequence + orig max val */
    }
    memId = pInfo->regCtr;
  }
  తిరిగి memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
శూన్య sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  పూర్ణాంకం memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  ప్రతిజ్ఞా( pParse->pTriggerTab==0 );
  ప్రతిజ్ఞా( sqlite3IsToplevel(pParse) );

  ప్రతిజ్ఞా( v );   /* We failed long ago if this is not so */
  ప్రతిజ్ఞా( pParse->ఉపయోగిస్తుంది_Ainc );
  కొరకు(p = pParse->pAinc; p; p = p->p_తదుపరి){
    స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
    స్థిర స్థిరరాశి VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_స్తంభం,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_స్తంభం,  0,  1, 0},
      /* 6  */ {OP_కూడు_Imm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_తదుపరి,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Open_పట్టిక(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3_Vdbe_ఎక్కించు_String(v, memId-1, p->pTab->zName);
    aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(autoInc), autoInc, iLn);
    ఒకవేళ( aOp==0 ) ఆపు;
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
    ఒకవేళ( pParse->nTab==0 ) pParse->nTab = 1;
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
స్థిర శూన్య auto_Inc_అడుగు(Parse *pParse, పూర్ణాంకం memId, పూర్ణాంకం regRowid){
  ఒకవేళ( memId>0 ){
    sqlite3_Vdbe_కూడు_Op2(pParse->pVdbe, OP_MemMax, memId, regRowid);
  }
}

/*
** This routine generates the code needed to write autoincrement
** maximum rowid values back into the sqlite_sequence register.
** Every statement that might do an INSERT into an autoincrement
** table (either directly or through triggers) needs to call this
** routine just before the "exit" code.
*/
స్థిర SQLITE_NOINLINE శూన్య auto_Increment_చివర(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ప్రతిజ్ఞా( v );
  ప్రతిజ్ఞా( pParse->ఉపయోగిస్తుంది_Ainc );
  కొరకు(p = pParse->pAinc; p; p = p->p_తదుపరి){
    స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
    స్థిర స్థిరరాశి VdbeOpList auto_Inc_చివర[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_Make_రికార్డు,  0, 2, 0},
      /* 3 */ {OP_చొప్పించు,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    పూర్ణాంకం iRec;
    పూర్ణాంకం memId = p->regCtr;

    iRec = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Vdbe_కూడు_Op3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3_Open_పట్టిక(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(auto_Inc_చివర), auto_Inc_చివర, iLn);
    ఒకవేళ( aOp==0 ) ఆపు;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3_Release_తాత్కాలిక_Reg(pParse, iRec);
  }
}
శూన్య sqlite3_Autoincrement_చివర(Parse *pParse){
  ఒకవేళ( pParse->ఉపయోగిస్తుంది_Ainc ) auto_Increment_చివర(pParse);
}
#లేకపోతే
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define auto_Inc_అడుగు(A,B,C)
#అయితే_ముగింపు /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
శూన్య sqlite3_Multi_విలువలు_చివర(Parse *pParse, Select *pVal){
  ఒకవేళ( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    Src_అంశం *p_అంశం = &pVal->pSrc->a[0];
    ప్రతిజ్ఞా( (p_అంశం->fg.isSubquery && p_అంశం->u4.pSubq!=0) || pParse->nErr );
    ఒకవేళ( p_అంశం->fg.isSubquery ){
      sqlite3_Vdbe_చివర_Coroutine(pParse->pVdbe, p_అంశం->u4.pSubq->regReturn);
      sqlite3_Vdbe_Jump_ఇక్కడ(pParse->pVdbe, p_అంశం->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
స్థిర పూర్ణాంకం సమాసం_List_Is_No_Affinity(Parse *pParse, సమాసం_List *p_అడ్డువరుస){
  పూర్ణాంకం ii;
  ఒకవేళ( sqlite3_సమాసం_List_Is_Constant(pParse, p_అడ్డువరుస, 0)==0 ) తిరిగి 0;
  కొరకు(ii=0; ii<p_అడ్డువరుస->n_సమాసం; ii++){
    సమాసం *p_సమాసం = p_అడ్డువరుస->a[ii].p_సమాసం;
    ప్రతిజ్ఞా( p_సమాసం->op!=TK_RAISE );
    ప్రతిజ్ఞా( p_సమాసం->aff_సమాసం==0 );
    ఒకవేళ( 0!=sqlite3_సమాసం_Affinity(p_సమాసం) ) తిరిగి 0;
  }
  తిరిగి 1;

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
Select *sqlite3_Multi_విలువలు(Parse *pParse, Select *p_ఎడమ, సమాసం_List *p_అడ్డువరుస){

  ఒకవేళ( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3_సమాసం_List_Is_Constant(pParse, p_అడ్డువరుస, 1)==0   /* condition (c) above */
   || (p_ఎడమ->pSrc->nSrc==0 &&
       సమాసం_List_Is_No_Affinity(pParse,p_ఎడమ->pEList)==0) /* condition (d) above */
   || లోపల_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    పూర్ణాంకం f = SF_విలువలు | SF_Multi_విలువ;
    ఒకవేళ( p_ఎడమ->pSrc->nSrc ){
      sqlite3_Multi_విలువలు_చివర(pParse, p_ఎడమ);
      f = SF_విలువలు;
    }లేకపోతే ఒకవేళ( p_ఎడమ->p_పూర్వ ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & p_ఎడమ->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, p_అడ్డువరుస, 0, 0, 0, 0, 0, f, 0);
    p_ఎడమ->selFlags &= ~(u32)SF_Multi_విలువ;
    ఒకవేళ( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->p_పూర్వ = p_ఎడమ;
      p_ఎడమ = pSelect;
    }
  }లేకపోతే{
    Src_అంశం *p = 0;               /* SrcItem that reads from co-routine */

    ఒకవేళ( p_ఎడమ->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      ఒకవేళ( (pParse->db->mDbFlags & DBFLAG_Schema_తెలిసిన_Ok)==0 ){
        sqlite3ReadSchema(pParse);
      }

      ఒకవేళ( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->p_పూర్వ = p_ఎడమ->p_పూర్వ;
        pRet->op = p_ఎడమ->op;
        ఒకవేళ( pRet->p_పూర్వ ) pRet->selFlags |= SF_విలువలు;
        p_ఎడమ->p_పూర్వ = 0;
        p_ఎడమ->op = TK_SELECT;
        ప్రతిజ్ఞా( p_ఎడమ->p_తదుపరి==0 );
        ప్రతిజ్ఞా( pRet->p_తదుపరి==0 );
        p = &pRet->pSrc->a[0];
        p->fg.ద్వారా_Coroutine = 1;
        p->iCursor = -1;
        ప్రతిజ్ఞా( !p->fg.is_సూచీకృత_By && !p->fg.isTabFunc );
        p->u1.n_అడ్డువరుస = 2;
        ఒకవేళ( sqlite3_Src_అంశం_Attach_Subquery(pParse, p, p_ఎడమ, 0) ){
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine,
                            pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);

          /* Allocate registers for the output of the co-routine. Do so so
          ** that there are two unused registers immediately before those
          ** used by the co-routine. This allows the code in sqlite3Insert()
          ** to use these registers directly, instead of copying the output
          ** of the co-routine to a separate array for processing.  */
          dest.iSdst = pParse->nMem + 3; 
          dest.nSdst = p_ఎడమ->pEList->n_సమాసం;
          pParse->nMem += 2 + dest.nSdst;

          p_ఎడమ->selFlags |= SF_Multi_విలువ;
          sqlite3Select(pParse, p_ఎడమ, &dest);
          pSubq->reg_ఫలితం = dest.iSdst;
          ప్రతిజ్ఞా( pParse->nErr || dest.iSdst>0 );
        }
        p_ఎడమ = pRet;
      }
    }లేకపోతే{
      p = &p_ఎడమ->pSrc->a[0];
      ప్రతిజ్ఞా( !p->fg.isTabFunc && !p->fg.is_సూచీకృత_By );
      p->u1.n_అడ్డువరుస++;
    }
  
    ఒకవేళ( pParse->nErr==0 ){
      Subquery *pSubq;
      ప్రతిజ్ఞా( p!=0 );
      ప్రతిజ్ఞా( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      ప్రతిజ్ఞా( pSubq!=0 );
      ప్రతిజ్ఞా( pSubq->pSelect!=0 );
      ప్రతిజ్ఞా( pSubq->pSelect->pEList!=0 );
      ఒకవేళ( pSubq->pSelect->pEList->n_సమాసం!=p_అడ్డువరుస->n_సమాసం ){
        sqlite3_Select_Wrong_అంకె_Terms_Error(pParse, pSubq->pSelect);
      }లేకపోతే{
        sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, p_అడ్డువరుస, pSubq->reg_ఫలితం, 0, 0);
        sqlite3_Vdbe_కూడు_Op1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3_సమాసం_List_Delete(pParse->db, p_అడ్డువరుస);
  }

  తిరిగి p_ఎడమ;
}

/* Forward declaration */
స్థిర పూర్ణాంకం xferOptimization(
  Parse *pParse,        /* Parser context */
  పట్టిక *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  పూర్ణాంకం onError,          /* How to handle constraint errors */
  పూర్ణాంకం iDbDest           /* The database of pDest */
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
శూన్య sqlite3_చొప్పించు(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *p_స్తంభం,      /* Column names corresponding to IDLIST, or NULL. */
  పూర్ణాంకం onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  పట్టిక *pTab;          /* The table to insert into.  aka TABLE */
  పూర్ణాంకం i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  సూచీ *pIdx;          /* For looping over indices of the table */
  పూర్ణాంకం n_స్తంభం;          /* Number of columns in the data */
  పూర్ణాంకం nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  పూర్ణాంకం i_దత్తాంశం_Cur = 0;     /* VDBE cursor that is the main data repository */
  పూర్ణాంకం iIdxCur = 0;      /* First index cursor */
  పూర్ణాంకం ipk_స్తంభం = -1;   /* Column that is the INTEGER PRIMARY KEY */
  పూర్ణాంకం చివర_యొక్క_Loop;        /* Label for the end of the insertion loop */
  పూర్ణాంకం srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  పూర్ణాంకం addr_Ins_శిఖరం = 0;   /* Jump to label "D" */
  పూర్ణాంకం addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  పూర్ణాంకం iDb;              /* Index of database holding TABLE */
  u8 use_తాత్కాలిక_పట్టిక = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 b_Id_List_లోపల_క్రమం;    /* True if IDLIST is in table order */
  సమాసం_List *pList = 0;  /* List of VALUES() to be inserted  */
  పూర్ణాంకం iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  పూర్ణాంకం regFromSelect = 0;/* Base register for data coming from SELECT */
  పూర్ణాంకం regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  పూర్ణాంకం reg_అడ్డువరుస_లెక్క = 0;  /* Memory cell used for the row counter */
  పూర్ణాంకం regIns;           /* Block of regs holding rowid+data being inserted */
  పూర్ణాంకం regRowid;         /* registers holding insert rowid */
  పూర్ణాంకం reg_దత్తాంశం;          /* register holding first column to insert */
  పూర్ణాంకం *aRegIdx = 0;     /* One register allocated to each index */
  పూర్ణాంకం *a_Tab_నిలువు_Map = 0;  /* Mapping from pTab columns to pCol entries */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  పూర్ణాంకం isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  పూర్ణాంకం tmask;                  /* Mask of trigger times */
#అయితే_ముగింపు

  db = pParse->db;
  ప్రతిజ్ఞా( db->pParse==pParse );
  ఒకవేళ( pParse->nErr ){
    వెళ్ళు చొప్పించు_cleanup;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  ఒకవేళ( pSelect && (pSelect->selFlags & SF_విలువలు)!=0 && pSelect->p_పూర్వ==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  ప్రతిజ్ఞా( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ఒకవేళ( pTab==0 ){
    వెళ్ళు చొప్పించు_cleanup;
  }
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iDb<db->nDb );
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_చొప్పించు, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    వెళ్ళు చొప్పించు_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_చొప్పించు, 0, &tmask);
  isView = IsView(pTab);
#లేకపోతే
# define pTrigger 0
# define tmask 0
# define isView 0
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#అయితే_ముగింపు
  ప్రతిజ్ఞా( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x10000 ){
    sqlite3_వృక్షం_View_పంక్తి(0, "In sqlite3Insert() at %s:%d", __FILE__, __LINE__);
    sqlite3_వృక్షం_View_చొప్పించు(pParse->pWith, pTabList, p_స్తంభం, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#అయితే_ముగింపు

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  ఒకవేళ( sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab) ){
    వెళ్ళు చొప్పించు_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  ఒకవేళ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    వెళ్ళు చొప్పించు_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v==0 ) వెళ్ళు చొప్పించు_cleanup;
  ఒకవేళ( pParse->nested==0 ) sqlite3_Vdbe_లెక్క_Changes(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  ఒకవేళ( p_స్తంభం==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    ప్రతిజ్ఞా( !pTrigger );
    ప్రతిజ్ఞా( pList==0 );
    వెళ్ళు చొప్పించు_చివర;
  }
#అయితే_ముగింపు /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->n_నిలువు + 1;
  ఒకవేళ( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  reg_దత్తాంశం = regRowid+1;

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
  b_Id_List_లోపల_క్రమం = (pTab->tabFlags & (TF_OOOHidden|TF_HasStored))==0;
  ఒకవేళ( p_స్తంభం ){
    a_Tab_నిలువు_Map = sqlite3DbMallocZero(db, pTab->n_నిలువు*పరిమాణం(పూర్ణాంకం));
    ఒకవేళ( a_Tab_నిలువు_Map==0 ) వెళ్ళు చొప్పించు_cleanup;
    కొరకు(i=0; i<p_స్తంభం->nId; i++){
      j = sqlite3_స్తంభం_సూచీ(pTab, p_స్తంభం->a[i].zName);
      ఒకవేళ( j>=0 ){
        ఒకవేళ( a_Tab_నిలువు_Map[j]==0 ) a_Tab_నిలువు_Map[j] = i+1;
        ఒకవేళ( i!=j ) b_Id_List_లోపల_క్రమం = 0;
        ఒకవేళ( j==pTab->iPKey ){
          ipk_స్తంభం = i;  ప్రతిజ్ఞా( !withoutRowid );
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
        ఒకవేళ( pTab->a_నిలువు[j].నిలువు_Flags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3_Error_సందేశం(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->a_నిలువు[j].zCnName);
          వెళ్ళు చొప్పించు_cleanup;
        }
#అయితే_ముగింపు
      }లేకపోతే{
        ఒకవేళ( sqlite3IsRowid(p_స్తంభం->a[i].zName) && !withoutRowid ){
          ipk_స్తంభం = i;
          b_Id_List_లోపల_క్రమం = 0;
        }లేకపోతే{
          sqlite3_Error_సందేశం(pParse, "table %S has no column named %s",
              pTabList->a, p_స్తంభం->a[i].zName);
          pParse->checkSchema = 1;
          వెళ్ళు చొప్పించు_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  ఒకవేళ( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    పూర్ణాంకం rc;             /* Result code */

    ఒకవేళ( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.ద్వారా_Coroutine 
     && pSelect->p_పూర్వ==0
    ){
      Src_అంశం *p_అంశం = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      ప్రతిజ్ఞా( p_అంశం->fg.isSubquery );
      pSubq = p_అంశం->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->reg_ఫలితం;
      ప్రతిజ్ఞా( pSubq->pSelect!=0 );
      ప్రతిజ్ఞా( pSubq->pSelect->pEList!=0 );
      n_స్తంభం = pSubq->pSelect->pEList->n_సమాసం;
      ExplainQueryPlan((pParse, 0, "SCAN %S", p_అంశం));
      ఒకవేళ( b_Id_List_లోపల_క్రమం && n_స్తంభం==pTab->n_నిలువు ){
        reg_దత్తాంశం = regFromSelect;
        regRowid = reg_దత్తాంశం - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }లేకపోతే{
      పూర్ణాంకం addr_శిఖరం;        /* Top of the co-routine */
      పూర్ణాంకం regYield = ++pParse->nMem;
      addr_శిఖరం = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, regYield, 0, addr_శిఖరం);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = b_Id_List_లోపల_క్రమం ? reg_దత్తాంశం : 0;
      dest.nSdst = pTab->n_నిలువు;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      ప్రతిజ్ఞా( db->pParse==pParse );
      ఒకవేళ( rc || pParse->nErr ) వెళ్ళు చొప్పించు_cleanup;
      ప్రతిజ్ఞా( db->mallocFailed==0 );
      sqlite3_Vdbe_చివర_Coroutine(v, regYield);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం - 1);                       /* label B: */
      ప్రతిజ్ఞా( pSelect->pEList );
      n_స్తంభం = pSelect->pEList->n_సమాసం;
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
    ఒకవేళ( pTrigger || reads_పట్టిక(pParse, iDb, pTab) ){
      use_తాత్కాలిక_పట్టిక = 1;
    }

    ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
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
      పూర్ణాంకం regRec;          /* Register to hold packed record */
      పూర్ణాంకం reg_తాత్కాలిక_Rowid;    /* Register to hold temp table ROWID */
      పూర్ణాంకం addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      reg_తాత్కాలిక_Rowid = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, srcTab, n_స్తంభం);
      addrL = sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regFromSelect, n_స్తంభం, regRec);
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, srcTab, reg_తాత్కాలిక_Rowid);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, srcTab, regRec, reg_తాత్కాలిక_Rowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrL);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, regRec);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_తాత్కాలిక_Rowid);
    }
  }లేకపోతే{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    memset(&sNC, 0, పరిమాణం(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    ప్రతిజ్ఞా( use_తాత్కాలిక_పట్టిక==0 );
    ఒకవేళ( pList ){
      n_స్తంభం = pList->n_సమాసం;
      ఒకవేళ( sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pList) ){
        వెళ్ళు చొప్పించు_cleanup;
      }
    }లేకపోతే{
      n_స్తంభం = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  ఒకవేళ( p_స్తంభం==0 && n_స్తంభం>0 ){
    ipk_స్తంభం = pTab->iPKey;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
    ఒకవేళ( ipk_స్తంభం>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      కొరకు(i=ipk_స్తంభం-1; i>=0; i--){
        ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_GENERATED ){
          testcase( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_VIRTUAL );
          testcase( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_STORED );
          ipk_స్తంభం--;
        }
      }
    }
#అయితే_ముగింపు

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    ప్రతిజ్ఞా( TF_HasHidden==COLFLAG_HIDDEN );
    ప్రతిజ్ఞా( TF_HasGenerated==COLFLAG_GENERATED );
    ప్రతిజ్ఞా( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    ఒకవేళ( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      కొరకు(i=0; i<pTab->n_నిలువు; i++){
        ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    ఒకవేళ( n_స్తంభం!=(pTab->n_నిలువు-nHidden) ){
      sqlite3_Error_సందేశం(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->n_నిలువు-nHidden, n_స్తంభం);
     వెళ్ళు చొప్పించు_cleanup;
    }
  }
  ఒకవేళ( p_స్తంభం!=0 && n_స్తంభం!=p_స్తంభం->nId ){
    sqlite3_Error_సందేశం(pParse, "%d values for %d columns", n_స్తంభం, p_స్తంభం->nId);
    వెళ్ళు చొప్పించు_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  ఒకవేళ( (db->flags & SQLITE_లెక్క_అడ్డువరుసలు)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    reg_అడ్డువరుస_లెక్క = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, reg_అడ్డువరుస_లెక్క);
  }

  /* If this is not a view, open the table and and all indices */
  ఒకవేళ( !isView ){
    పూర్ణాంకం nIdx;
    nIdx = sqlite3_Open_పట్టిక_And_Indices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &i_దత్తాంశం_Cur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, పరిమాణం(పూర్ణాంకం)*(nIdx+2));
    ఒకవేళ( aRegIdx==0 ){
      వెళ్ళు చొప్పించు_cleanup;
    }
    కొరకు(i=0, pIdx=pTab->p_సూచీ; i<nIdx; pIdx=pIdx->p_తదుపరి, i++){
      ప్రతిజ్ఞా( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->n_స్తంభం;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
  ఒకవేళ( pUpsert ){
    Upsert *pNx;
    ఒకవేళ( IsVirtual(pTab) ){
      sqlite3_Error_సందేశం(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      వెళ్ళు చొప్పించు_cleanup;
    }
    ఒకవేళ( IsView(pTab) ){
      sqlite3_Error_సందేశం(pParse, "cannot UPSERT a view");
      వెళ్ళు చొప్పించు_cleanup;
    }
    ఒకవేళ( sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_లక్ష్యం) ){
      వెళ్ళు చొప్పించు_cleanup;
    }
    pTabList->a[0].iCursor = i_దత్తాంశం_Cur;
    pNx = pUpsert;
    చేయి{
      pNx->pUpsertSrc = pTabList;
      pNx->reg_దత్తాంశం = reg_దత్తాంశం;
      pNx->i_దత్తాంశం_Cur = i_దత్తాంశం_Cur;
      pNx->iIdxCur = iIdxCur;
      ఒకవేళ( pNx->p_Upsert_లక్ష్యం ){
        ఒకవేళ( sqlite3_Upsert_Analyze_లక్ష్యం(pParse, pTabList, pNx, pUpsert) ){
          వెళ్ళు చొప్పించు_cleanup;
        }
      }
      pNx = pNx->p_తదుపరి_Upsert;
    }అయితే( pNx!=0 );
  }
#అయితే_ముగింపు


  /* This is the top of the main insertion loop */
  ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addr_Ins_శిఖరం = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }లేకపోతే ఒకవేళ( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, reg_దత్తాంశం, pTab->n_నిలువు, 0, 0);
    addr_Ins_శిఖరం = addrCont = sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    ఒకవేళ( ipk_స్తంభం>=0 ){
      /* tag-20191021-001: If the INTEGER PRIMARY KEY is being generated by the
      ** SELECT, go ahead and copy the value into the rowid slot now, so that
      ** the value does not get overwritten by a NULL at tag-20191021-002. */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, regFromSelect+ipk_స్తంభం, regRowid);
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
  iRegStore = reg_దత్తాంశం;  ప్రతిజ్ఞా( reg_దత్తాంశం==regRowid+1 );
  కొరకు(i=0; i<pTab->n_నిలువు; i++, iRegStore++){
    పూర్ణాంకం k;
    u32 నిలువు_Flags;
    ప్రతిజ్ఞా( i>=nHidden );
    ఒకవేళ( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3_Vdbe_కూడు_Op1(v, OP_SoftNull, iRegStore);
      కొనసాగు;
    }
    ఒకవేళ( ((నిలువు_Flags = pTab->a_నిలువు[i].నిలువు_Flags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      ఒకవేళ( (నిలువు_Flags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        కొనసాగు;
      }లేకపోతే ఒకవేళ( (నిలువు_Flags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        ఒకవేళ( tmask & TRIGGER_BEFORE ){
          sqlite3_Vdbe_కూడు_Op1(v, OP_SoftNull, iRegStore);
        }
        కొనసాగు;
      }లేకపోతే ఒకవేళ( p_స్తంభం==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3_సమాసం_సంకేతం_Factorable(pParse,
            sqlite3_స్తంభం_సమాసం_Auth(pTab, &pTab->a_నిలువు[i], pParse),
            iRegStore);
        కొనసాగు;
      }
    }
    ఒకవేళ( p_స్తంభం ){
      j = a_Tab_నిలువు_Map[i];
      ప్రతిజ్ఞా( j>=0 && j<=p_స్తంభం->nId );   
      ఒకవేళ( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3_సమాసం_సంకేతం_Factorable(pParse,
            sqlite3_స్తంభం_సమాసం_Auth(pTab, &pTab->a_నిలువు[i], pParse),
            iRegStore);
        కొనసాగు;
      }
      k = j - 1;
    }లేకపోతే ఒకవేళ( n_స్తంభం==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3_సమాసం_సంకేతం_Factorable(pParse,
          sqlite3_స్తంభం_సమాసం_Auth(pTab, &pTab->a_నిలువు[i], pParse),
          iRegStore);
      కొనసాగు;
    }లేకపోతే{
      k = i - nHidden;
    }

    ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, srcTab, k, iRegStore);
    }లేకపోతే ఒకవేళ( pSelect ){
      ఒకవేళ( regFromSelect!=reg_దత్తాంశం ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }లేకపోతే{
      సమాసం *pX = pList->a[k].p_సమాసం;
      పూర్ణాంకం y = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, pX, iRegStore);
      ఒకవేళ( y!=iRegStore ){
        sqlite3_Vdbe_కూడు_Op2(v,
          సమాసం_Has_Property(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  చివర_యొక్క_Loop = sqlite3VdbeMakeLabel(pParse);
  ఒకవేళ( tmask & TRIGGER_BEFORE ){
    పూర్ణాంకం reg_నిలువుల_సంఖ్య = sqlite3_Get_తాత్కాలిక_Range(pParse, pTab->n_నిలువు+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    ఒకవేళ( ipk_స్తంభం<0 ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, -1, reg_నిలువుల_సంఖ్య);
    }లేకపోతే{
      పూర్ణాంకం addr1;
      ప్రతిజ్ఞా( !withoutRowid );
      ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, srcTab, ipk_స్తంభం, reg_నిలువుల_సంఖ్య);
      }లేకపోతే{
        ప్రతిజ్ఞా( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3_సమాసం_సంకేతం(pParse, pList->a[ipk_స్తంభం].p_సమాసం, reg_నిలువుల_సంఖ్య);
      }
      addr1 = sqlite3_Vdbe_కూడు_Op1(v, OP_NotNull, reg_నిలువుల_సంఖ్య); VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, -1, reg_నిలువుల_సంఖ్య);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
      sqlite3_Vdbe_కూడు_Op1(v, OP_MustBeInt, reg_నిలువుల_సంఖ్య); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    ప్రతిజ్ఞా( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, regRowid+1, reg_నిలువుల_సంఖ్య+1, pTab->nNVCol-1);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    ఒకవేళ( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, reg_నిలువుల_సంఖ్య+1, pTab);
    }
#అయితే_ముగింపు

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    ఒకవేళ( !isView ){
      sqlite3_పట్టిక_Affinity(v, pTab, reg_నిలువుల_సంఖ్య+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger, TK_చొప్పించు, 0, TRIGGER_BEFORE,
        pTab, reg_నిలువుల_సంఖ్య-pTab->n_నిలువు-1, onError, చివర_యొక్క_Loop);

    sqlite3_Release_తాత్కాలిక_Range(pParse, reg_నిలువుల_సంఖ్య, pTab->n_నిలువు+1);
  }

  ఒకవేళ( !isView ){
    ఒకవేళ( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, regIns);
    }
    ఒకవేళ( ipk_స్తంభం>=0 ){
      /* Compute the new rowid */
      ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, srcTab, ipk_స్తంభం, regRowid);
      }లేకపోతే ఒకవేళ( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }లేకపోతే{
        సమాసం *pIpk = pList->a[ipk_స్తంభం].p_సమాసం;
        ఒకవేళ( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_NewRowid, i_దత్తాంశం_Cur, regRowid, regAutoinc);
          appendFlag = 1;
        }లేకపోతే{
          sqlite3_సమాసం_సంకేతం(pParse, pList->a[ipk_స్తంభం].p_సమాసం, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      ఒకవేళ( !appendFlag ){
        పూర్ణాంకం addr1;
        ఒకవేళ( !IsVirtual(pTab) ){
          addr1 = sqlite3_Vdbe_కూడు_Op1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3_Vdbe_కూడు_Op3(v, OP_NewRowid, i_దత్తాంశం_Cur, regRowid, regAutoinc);
          sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
        }లేకపోతే{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3_Vdbe_కూడు_Op1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }లేకపోతే ఒకవేళ( IsVirtual(pTab) || withoutRowid ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, regRowid);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op3(v, OP_NewRowid, i_దత్తాంశం_Cur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    auto_Inc_అడుగు(pParse, regAutoinc, regRowid);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    ఒకవేళ( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#అయితే_ముగింపు

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    ఒకవేళ( IsVirtual(pTab) ){
      స్థిరరాశి అక్షరం *pVTab = (స్థిరరాశి అక్షరం *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3_Vdbe_కూడు_Op4(v, OP_VUpdate, 1, pTab->n_నిలువు+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }లేకపోతే
#అయితే_ముగింపు
    {
      పూర్ణాంకం isReplace = 0;/* Set to true if constraints may cause a replace */
      పూర్ణాంకం bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_దత్తాంశం_Cur, iIdxCur,
          regIns, 0, ipk_స్తంభం>=0, onError, చివర_యొక్క_Loop, &isReplace, 0, pUpsert
      );
      ఒకవేళ( db->flags & SQLITE_Foreign_కీలకాలు ){
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
      bUseSeek = (isReplace==0 || !sqlite3_Vdbe_Has_Sub_కార్యక్రమం(v));
      sqlite3CompleteInsertion(pParse, pTab, i_దత్తాంశం_Cur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
  }లేకపోతే ఒకవేళ( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, -1, regRowid);
#అయితే_ముగింపు
  }

  /* Update the count of rows that are inserted
  */
  ఒకవేళ( reg_అడ్డువరుస_లెక్క ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, reg_అడ్డువరుస_లెక్క, 1);
  }

  ఒకవేళ( pTrigger ){
    /* Code AFTER triggers */
    sqlite3_సంకేతం_అడ్డువరుస_Trigger(pParse, pTrigger, TK_చొప్పించు, 0, TRIGGER_AFTER,
        pTab, reg_దత్తాంశం-2-pTab->n_నిలువు, onError, చివర_యొక్క_Loop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, చివర_యొక్క_Loop);
  ఒకవేళ( use_తాత్కాలిక_పట్టిక ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, srcTab, addrCont); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_Ins_శిఖరం);
    sqlite3_Vdbe_కూడు_Op1(v, OP_Close, srcTab);
  }లేకపోతే ఒకవేళ( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#నిర్వచితమైతే SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    ఒకవేళ( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      ప్రతిజ్ఞా( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#అయితే_ముగింపు
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_Ins_శిఖరం);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_XFER_OPT
చొప్పించు_చివర:
#అయితే_ముగింపు /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ఒకవేళ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_చివర(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ఒకవేళ( reg_అడ్డువరుస_లెక్క ){
    sqlite3_సంకేతం_Change_లెక్క(v, reg_అడ్డువరుస_లెక్క, "rows inserted");
  }

చొప్పించు_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_సమాసం_List_Delete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  ఒకవేళ( p_స్తంభం ){
    sqlite3IdListDelete(db, p_స్తంభం);
    sqlite3DbFree(db, a_Tab_నిలువు_Map);
  }
  ఒకవేళ( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#నిర్వచితమైతే isView
 #నిర్వచనం_రద్దు isView
#అయితే_ముగింపు
#నిర్వచితమైతే pTrigger
 #నిర్వచనం_రద్దు pTrigger
#అయితే_ముగింపు
#నిర్వచితమైతే tmask
 #నిర్వచనం_రద్దు tmask
#అయితే_ముగింపు

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#నిర్వచించు CKCNSTRNT_స్తంభం   0x01    /* CHECK constraint uses a changing column */
#నిర్వచించు CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
స్థిర పూర్ణాంకం check_Constraint_సమాసం_కణుపు(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం ){
    ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం>=0 || p_సమాసం->i_స్తంభం==-1 );
    ఒకవేళ( p_సమాసం->i_స్తంభం>=0 ){
      ఒకవేళ( pWalker->u.ai_నిలువు[p_సమాసం->i_స్తంభం]>=0 ){
        pWalker->e_సంకేతం |= CKCNSTRNT_స్తంభం;
      }
    }లేకపోతే{
      pWalker->e_సంకేతం |= CKCNSTRNT_ROWID;
    }
  }
  తిరిగి WRC_Continue;
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
పూర్ణాంకం sqlite3_సమాసం_References_Updated_స్తంభం(
  సమాసం *p_సమాసం,    /* The expression to be checked */
  పూర్ణాంకం *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  పూర్ణాంకం chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  memset(&w, 0, పరిమాణం(w));
  w.e_సంకేతం = 0;
  w.x_సమాసం_Callback = check_Constraint_సమాసం_కణుపు;
  w.u.ai_నిలువు = aiChng;
  sqlite3_Walk_సమాసం(&w, p_సమాసం);
  ఒకవేళ( !chngRowid ){
    testcase( (w.e_సంకేతం & CKCNSTRNT_ROWID)!=0 );
    w.e_సంకేతం &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.e_సంకేతం==0 );
  testcase( w.e_సంకేతం==CKCNSTRNT_స్తంభం );
  testcase( w.e_సంకేతం==CKCNSTRNT_ROWID );
  testcase( w.e_సంకేతం==(CKCNSTRNT_ROWID|CKCNSTRNT_స్తంభం) );
  తిరిగి w.e_సంకేతం!=0;
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
రకనిర్వచనం నిర్మాణం సూచీ_List_పదం సూచీ_List_పదం;
రకనిర్వచనం నిర్మాణం సూచీ_Iterator సూచీ_Iterator;
నిర్మాణం సూచీ_Iterator {
  పూర్ణాంకం eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  పూర్ణాంకం i;        /* Index of the current item from the list */
  సంఘం {
    నిర్మాణం {    /* Use this object for eType==0: A Index.pNext list */
      సూచీ *pIdx;   /* The current Index */
    } lx;     
    నిర్మాణం {    /* Use this object for eType==1; Array of IndexListTerm */
      పూర్ణాంకం nIdx;               /* Size of the array */
      సూచీ_List_పదం *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
నిర్మాణం సూచీ_List_పదం {
  సూచీ *p;  /* The index */
  పూర్ణాంకం ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
స్థిర సూచీ *సూచీ_Iterator_మొదటి(సూచీ_Iterator *pIter, పూర్ణాంకం *pIx){
  ప్రతిజ్ఞా( pIter->i==0 );
  ఒకవేళ( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    తిరిగి pIter->u.ax.aIdx[0].p;
  }లేకపోతే{
    *pIx = 0;
    తిరిగి pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
స్థిర సూచీ *సూచీ_Iterator_తదుపరి(సూచీ_Iterator *pIter, పూర్ణాంకం *pIx){
  ఒకవేళ( pIter->eType ){
    పూర్ణాంకం i = ++pIter->i;
    ఒకవేళ( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      తిరిగి 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    తిరిగి pIter->u.ax.aIdx[i].p;
  }లేకపోతే{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->p_తదుపరి;
    తిరిగి pIter->u.lx.pIdx;
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
శూన్య sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  పట్టిక *pTab,         /* The table being inserted or updated */
  పూర్ణాంకం *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  పూర్ణాంకం i_దత్తాంశం_Cur,        /* Canonical data cursor (main table or PK index) */
  పూర్ణాంకం iIdxCur,         /* First index cursor */
  పూర్ణాంకం reg_New_దత్తాంశం,      /* First register in a range holding values to insert */
  పూర్ణాంకం reg_Old_దత్తాంశం,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  పూర్ణాంకం ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  పూర్ణాంకం *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  పూర్ణాంకం *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  సూచీ *pIdx;         /* Pointer to one of the indices */
  సూచీ *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  పూర్ణాంకం i;               /* loop counter */
  పూర్ణాంకం ix;              /* Index loop counter */
  పూర్ణాంకం n_నిలువు;            /* Number of columns */
  పూర్ణాంకం onError;         /* Conflict resolution strategy */
  పూర్ణాంకం చూసిన_Replace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  పూర్ణాంకం n_Pk_క్షేత్రం;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 b_Affinity_పూర్తి = 0;  /* True if the OP_Affinity operation has been run */
  పూర్ణాంకం upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  పూర్ణాంకం upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  పూర్ణాంకం ipk_శిఖరం = 0;        /* Top of the IPK uniqueness check */
  పూర్ణాంకం ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  పూర్ణాంకం regTrigCnt;       /* Register used to count replace trigger invocations */
  పూర్ణాంకం addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  పూర్ణాంకం lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  పూర్ణాంకం nReplaceTrig = 0; /* Number of replace triggers coded */
  సూచీ_Iterator sIdxIter;  /* Index iterator */

  isUpdate = reg_Old_దత్తాంశం!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( !IsView(pTab) );  /* This table is not a VIEW */
  n_నిలువు = pTab->n_నిలువు;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  ఒకవేళ( HasRowid(pTab) ){
    pPk = 0;
    n_Pk_క్షేత్రం = 1;
  }లేకపోతే{
    pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
    n_Pk_క్షేత్రం = pPk->n_కీలకం_నిలువు;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     i_దత్తాంశం_Cur, iIdxCur, reg_New_దత్తాంశం, reg_Old_దత్తాంశం, pkChng));

  /* Test all NOT NULL constraints.
  */
  ఒకవేళ( pTab->tabFlags & TF_HasNotNull ){
    పూర్ణాంకం b2ndPass = 0;         /* True if currently running 2nd pass */
    పూర్ణాంకం n_చూసిన_Replace = 0;     /* Number of ON CONFLICT REPLACE operations */
    పూర్ణాంకం nGenerated = 0;       /* Number of generated columns with NOT NULL */
    అయితే(1){  /* Make 2 passes over columns. Exit loop via "break" */
      కొరకు(i=0; i<n_నిలువు; i++){
        పూర్ణాంకం iReg;                        /* Register holding column value */
        స్తంభం *p_నిలువు = &pTab->a_నిలువు[i];   /* The column to check for NOT NULL */
        పూర్ణాంకం isGenerated;                 /* non-zero if column is generated */
        onError = p_నిలువు->notNull;
        ఒకవేళ( onError==OE_None ) కొనసాగు; /* No NOT NULL on this column */
        ఒకవేళ( i==pTab->iPKey ){
          కొనసాగు;        /* ROWID is never NULL */
        }
        isGenerated = p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED;
        ఒకవేళ( isGenerated && !b2ndPass ){
          nGenerated++;
          కొనసాగు;        /* Generated columns processed on 2nd pass */
        }
        ఒకవేళ( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          కొనసాగు;
        }
        ఒకవేళ( overrideError!=OE_Default ){
          onError = overrideError;
        }లేకపోతే ఒకవేళ( onError==OE_Default ){
          onError = OE_Abort;
        }
        ఒకవేళ( onError==OE_Replace ){
          ఒకవేళ( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || p_నిలువు->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( p_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL );
            testcase( p_నిలువు->నిలువు_Flags & COLFLAG_STORED );
            testcase( p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }లేకపోతే{
            ప్రతిజ్ఞా( !isGenerated );
          }
        }లేకపోతే ఒకవేళ( b2ndPass && !isGenerated ){
          కొనసాగు;
        }
        ప్రతిజ్ఞా( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i) );
        iReg = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i) + reg_New_దత్తాంశం + 1;
        ఎంపిక( onError ){
          స్థితి OE_Replace: {
            పూర్ణాంకం addr1 = sqlite3_Vdbe_కూడు_Op1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            ప్రతిజ్ఞా( (p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED)==0 );
            n_చూసిన_Replace++;
            sqlite3_సమాసం_సంకేతం_Copy(pParse,
               sqlite3_స్తంభం_సమాసం_Auth(pTab, p_నిలువు, pParse), iReg);
            sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
            ఆపు;
          }
          స్థితి OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          స్థితి OE_Rollback:
          స్థితి OE_Fail: {
            అక్షరం *z_సందేశం = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        p_నిలువు->zCnName);
            testcase( z_సందేశం==0 && db->mallocFailed==0 );
            sqlite3_Vdbe_కూడు_Op3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, z_సందేశం, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            ఆపు;
          }
          అప్రమేయ: {
            ప్రతిజ్ఞా( onError==OE_Ignore );
            sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            ఆపు;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      ఒకవేళ( nGenerated==0 && n_చూసిన_Replace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        ఆపు;
      }
      ఒకవేళ( b2ndPass ) ఆపు;  /* Never need more than 2 passes */
      b2ndPass = 1;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
      ఒకవేళ( n_చూసిన_Replace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, reg_New_దత్తాంశం+1, pTab);
      }
#అయితే_ముగింపు
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CHECK
  ఒకవేళ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    సమాసం_List *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(reg_New_దత్తాంశం+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    కొరకు(i=0; i<pCheck->n_సమాసం; i++){
      పూర్ణాంకం allOk;
      సమాసం *pCopy;
      సమాసం *p_సమాసం = pCheck->a[i].p_సమాసం;
      ఒకవేళ( aiChng
       && !sqlite3_సమాసం_References_Updated_స్తంభం(p_సమాసం, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        కొనసాగు;
      }
      ఒకవేళ( b_Affinity_పూర్తి==0 ){
        sqlite3_పట్టిక_Affinity(v, pTab, reg_New_దత్తాంశం+1);
        b_Affinity_పూర్తి = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
      ఒకవేళ( !db->mallocFailed ){
        sqlite3_సమాసం_If_True(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3_సమాసం_Delete(db, pCopy);
      ఒకవేళ( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }లేకపోతే{
        అక్షరం *zName = pCheck->a[i].zEName;
        ప్రతిజ్ఞా( zName!=0 || pParse->db->mallocFailed );
        ఒకవేళ( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_CHECK) */

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
  sIdxIter.u.lx.pIdx = pTab->p_సూచీ;
  ఒకవేళ( pUpsert ){
    ఒకవేళ( pUpsert->p_Upsert_లక్ష్యం==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      ప్రతిజ్ఞా( pUpsert->p_తదుపరి_Upsert==0 );
      ఒకవేళ( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }లేకపోతే{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }లేకపోతే ఒకవేళ( pTab->p_సూచీ!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      పూర్ణాంకం nIdx, jj;
      u64 nByte;
      Upsert *p_పదం;
      u8 *b_వాడిన;
      కొరకు(nIdx=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, nIdx++){
         ప్రతిజ్ఞా( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (పరిమాణం(సూచీ_List_పదం)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      ఒకవేళ( sIdxIter.u.ax.aIdx==0 ) తిరిగి; /* OOM */
      b_వాడిన = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->p_గమ్యం_Free = sIdxIter.u.ax.aIdx;
      కొరకు(i=0, p_పదం=pUpsert; p_పదం; p_పదం=p_పదం->p_తదుపరి_Upsert){
        ఒకవేళ( p_పదం->p_Upsert_లక్ష్యం==0 ) ఆపు;
        ఒకవేళ( p_పదం->pUpsertIdx==0 ) కొనసాగు;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->p_సూచీ;
        అయితే( ALWAYS(pIdx!=0) && pIdx!=p_పదం->pUpsertIdx ){
           pIdx = pIdx->p_తదుపరి;
           jj++;
        }
        ఒకవేళ( b_వాడిన[jj] ) కొనసాగు; /* Duplicate ON CONFLICT clause ignored */
        b_వాడిన[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      కొరకు(jj=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, jj++){
        ఒకవేళ( b_వాడిన[jj] ) కొనసాగు;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      ప్రతిజ్ఞా( i==nIdx );
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
  ఒకవేళ( (db->flags & (SQLITE_RecTriggers|SQLITE_Foreign_కీలకాలు))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }లేకపోతే{
    ఒకవేళ( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }లేకపోతే{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    ఒకవేళ( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      ఒకవేళ( overrideError!=OE_Replace ){
        ఒకవేళ( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }లేకపోతే ఒకవేళ( pkChng==0 || pPk || pTab->కీలకం_Conf!=OE_Replace ){
          కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
            ఒకవేళ( pIdx->onError==OE_Replace ) ఆపు;
          }
          ఒకవేళ( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    ఒకవేళ( regTrigCnt ){
      /* Replace triggers might exist.  Allocate the counter and
      ** initialize it to zero. */
      regTrigCnt = ++pParse->nMem;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regTrigCnt);
      VdbeComment((v, "trigger count"));
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  /* If rowid is changing, make sure the new rowid does not previously
  ** exist in the table.
  */
  ఒకవేళ( pkChng && pPk==0 ){
    పూర్ణాంకం addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->కీలకం_Conf;
    ఒకవేళ( overrideError!=OE_Default ){
      onError = overrideError;
    }లేకపోతే ఒకవేళ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    ఒకవేళ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_యొక్క_సూచీ(pUpsert,0);
      ఒకవేళ( pUpsertClause!=0 ){
        ఒకవేళ( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }లేకపోతే{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      ఒకవేళ( pUpsertClause!=pUpsert ){
        /* The first ON CONFLICT clause has a conflict target other than
        ** the IPK.  We have to jump ahead to that first ON CONFLICT clause
        ** and then come back here and deal with the IPK afterwards */
        upsertIpkDelay = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
      }
    }

    /* If the response to a rowid conflict is REPLACE but the response
    ** to some other UNIQUE constraint is FAIL or IGNORE, then we need
    ** to defer the running of the rowid conflict checking until after
    ** the UNIQUE constraints have run.
    */
    ఒకవేళ( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->p_సూచీ             /* There exist other constraints */
    ){
      ఒకవేళ( upsertIpkDelay ){
        ipk_శిఖరం = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }లేకపోతే{
        ipk_శిఖరం = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    ఒకవేళ( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, reg_New_దత్తాంశం, addrRowidOk, reg_Old_దత్తాంశం);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, i_దత్తాంశం_Cur, addrRowidOk, reg_New_దత్తాంశం);
    VdbeCoverage(v);

    ఎంపిక( onError ){
      అప్రమేయ: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      స్థితి OE_Rollback:
      స్థితి OE_Abort:
      స్థితి OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        ఆపు;
      }
      స్థితి OE_Replace: {
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
        ఒకవేళ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3_Generate_అడ్డువరుస_Delete(pParse, pTab, pTrigger, i_దత్తాంశం_Cur, iIdxCur,
                                   reg_New_దత్తాంశం, 1, 0, OE_Replace, 1, -1);
          sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
          ప్రతిజ్ఞా( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3_Vdbe_కూడు_Op2(v, OP_Delete, i_దత్తాంశం_Cur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_పట్టిక);
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */
          ఒకవేళ( pTab->p_సూచీ ){
            sqlite3MultiWrite(pParse);
            sqlite3_Generate_అడ్డువరుస_సూచీ_Delete(pParse, pTab, i_దత్తాంశం_Cur, iIdxCur,0,-1);
          }
        }
        చూసిన_Replace = 1;
        ఆపు;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
      స్థితి OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, i_దత్తాంశం_Cur);
        /* no break */ deliberate_fall_through
      }
#అయితే_ముగింపు
      స్థితి OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ఆపు;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    ఒకవేళ( ipk_శిఖరం ){
      ipkBottom = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, ipk_శిఖరం-1);
    }లేకపోతే ఒకవేళ( pUpsert && pUpsertClause!=pUpsert ){
      upsertIpkReturn = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
    }
  }

  /* Test all UNIQUE constraints by creating entries for each UNIQUE
  ** index and making sure that duplicate entries do not already exist.
  ** Compute the revised record entries for indices as we go.
  **
  ** This loop also handles the case of the PRIMARY KEY index for a
  ** WITHOUT ROWID table.
  */
  కొరకు(pIdx = సూచీ_Iterator_మొదటి(&sIdxIter, &ix);
      pIdx;
      pIdx = సూచీ_Iterator_తదుపరి(&sIdxIter, &ix)
  ){
    పూర్ణాంకం regIdx;          /* Range of registers holding content for pIdx */
    పూర్ణాంకం regR;            /* Range of registers holding conflicting PK */
    పూర్ణాంకం iThisCur;        /* Cursor for this UNIQUE index */
    పూర్ణాంకం addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    పూర్ణాంకం addrConflictCk;  /* First opcode in the conflict check logic */
    పూర్ణాంకం nConflictCk;     /* Number of opcodes in conflict check logic */

    ఒకవేళ( aRegIdx[ix]==0 ) కొనసాగు;  /* Skip indices that do not change */
    ఒకవేళ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_యొక్క_సూచీ(pUpsert, pIdx);
      ఒకవేళ( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3_Vdbe_Jump_ఇక్కడ(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    ఒకవేళ( b_Affinity_పూర్తి==0 ){
      sqlite3_పట్టిక_Affinity(v, pTab, reg_New_దత్తాంశం+1);
      b_Affinity_పూర్తి = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    ఒకవేళ( pIdx->pPartIdxWhere ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(reg_New_దత్తాంశం+1);
      sqlite3_సమాసం_If_False_Dup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    కొరకు(i=0; i<pIdx->n_స్తంభం; i++){
      పూర్ణాంకం i_క్షేత్రం = pIdx->ai_స్తంభం[i];
      పూర్ణాంకం x;
      ఒకవేళ( i_క్షేత్రం==XN_సమాసం ){
        pParse->iSelfTab = -(reg_New_దత్తాంశం+1);
        sqlite3_సమాసం_సంకేతం_Copy(pParse, pIdx->a_నిలువు_సమాసం->a[i].p_సమాసం, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }లేకపోతే ఒకవేళ( i_క్షేత్రం==XN_ROWID || i_క్షేత్రం==pTab->iPKey ){
        x = reg_New_దత్తాంశం;
        sqlite3_Vdbe_కూడు_Op2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }లేకపోతే{
        testcase( sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_క్షేత్రం)!=i_క్షేత్రం );
        x = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_క్షేత్రం) + reg_New_దత్తాంశం + 1;
        sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->a_నిలువు[i_క్షేత్రం].zCnName));
      }
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regIdx, pIdx->n_స్తంభం, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#నిర్వచితమైతే SQLITE_ENABLE_NULL_TRIM
    ఒకవేళ( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3_సమితి_Make_రికార్డు_P5(v, pIdx->p_పట్టిక);
    }
#అయితే_ముగింపు
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->n_స్తంభం, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    ఒకవేళ( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      కొనసాగు;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    ఒకవేళ( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      కొనసాగు;  /* pIdx is not a UNIQUE index */
    }
    ఒకవేళ( overrideError!=OE_Default ){
      onError = overrideError;
    }లేకపోతే ఒకవేళ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    ఒకవేళ( pUpsertClause ){
      ఒకవేళ( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }లేకపోతే{
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
    ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_PREUPDATE_HOOK
    ఒకవేళ( (ix==0 && pIdx->p_తదుపరి==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_Foreign_కీలకాలు) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      కొనసాగు;
    }
#అయితే_ముగింపు /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->n_కీలకం_నిలువు); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3_Get_తాత్కాలిక_Range(pParse, n_Pk_క్షేత్రం);
    ఒకవేళ( isUpdate || onError==OE_Replace ){
      ఒకవేళ( HasRowid(pTab) ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        ఒకవేళ( isUpdate ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, regR, addrUniqueOk, reg_Old_దత్తాంశం);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }లేకపోతే{
        పూర్ణాంకం x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        ఒకవేళ( pIdx!=pPk ){
          కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
            ప్రతిజ్ఞా( pPk->ai_స్తంభం[i]>=0 );
            x = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pIdx, pPk->ai_స్తంభం[i]);
            sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->a_నిలువు[pPk->ai_స్తంభం[i]].zCnName));
          }
        }
        ఒకవేళ( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          పూర్ణాంకం addrJump = sqlite3VdbeCurrentAddr(v)+pPk->n_కీలకం_నిలువు;
          పూర్ణాంకం op = OP_Ne;
          పూర్ణాంకం regCmp = (Is_Primary_కీలకం_సూచీ(pIdx) ? regIdx : regR);
 
          కొరకు(i=0; i<pPk->n_కీలకం_నిలువు; i++){
            అక్షరం *p4 = (అక్షరం*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->ai_స్తంభం[i];
            ప్రతిజ్ఞా( x>=0 );
            ఒకవేళ( i==(pPk->n_కీలకం_నిలువు-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, x);
            sqlite3_Vdbe_కూడు_Op4(v, op,
                reg_Old_దత్తాంశం+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    ప్రతిజ్ఞా( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    ప్రతిజ్ఞా( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    ఎంపిక( onError ){
      స్థితి OE_Rollback:
      స్థితి OE_Abort:
      స్థితి OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        ఆపు;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
      స్థితి OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#అయితే_ముగింపు
      స్థితి OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ఆపు;
      }
      అప్రమేయ: {
        ప్రతిజ్ఞా( onError==OE_Replace );
        ఒకవేళ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        ఒకవేళ( pTrigger && isUpdate ){
          sqlite3_Vdbe_కూడు_Op1(v, OP_Cursor_తాళం, i_దత్తాంశం_Cur);
        }
        sqlite3_Generate_అడ్డువరుస_Delete(pParse, pTab, pTrigger, i_దత్తాంశం_Cur, iIdxCur,
            regR, n_Pk_క్షేత్రం, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        ఒకవేళ( pTrigger && isUpdate ){
          sqlite3_Vdbe_కూడు_Op1(v, OP_CursorUnlock, i_దత్తాంశం_Cur);
        }
        చూసిన_Replace = 1;
        ఆపు;
      }
    }
    ఒకవేళ( regTrigCnt ){
      పూర్ణాంకం addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      ఒకవేళ( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      అయితే( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        ఒకవేళ( x.opcode!=OP_IdxRowid || isUpdate ){
          పూర్ణాంకం p2;      /* New P2 value for copied conflict check opcode */
          స్థిరరాశి అక్షరం *zP4;
          ఒకవేళ( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }లేకపోతే{
            p2 = x.p2;
          }
          zP4 = x.p4type==P4_INT32 ? SQLITE_INT_గమ్యం_PTR(x.p4.i) : x.p4.z;
          sqlite3_Vdbe_కూడు_Op4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
          sqlite3VdbeChangeP5(v, x.p5);
          VdbeCoverageIf(v, p2!=x.p2);
        }
        nConflictCk--;
        addrConflictCk++;
      }
      /* If the retest fails, issue an abort */
      sqlite3UniqueConstraint(pParse, OE_Abort, pIdx);

      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrBypass); /* Terminate the recheck bypass */
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    ఒకవేళ( regR!=regIdx ) sqlite3_Release_తాత్కాలిక_Range(pParse, regR, n_Pk_క్షేత్రం);
    ఒకవేళ( pUpsertClause
     && upsertIpkReturn
     && sqlite3_Upsert_తదుపరి_Is_IPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  ఒకవేళ( ipk_శిఖరం ){
    sqlite3VdbeGoto(v, ipk_శిఖరం);
    VdbeComment((v, "Do IPK REPLACE"));
    ప్రతిజ్ఞా( ipkBottom>0 );
    sqlite3_Vdbe_Jump_ఇక్కడ(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  ప్రతిజ్ఞా( regTrigCnt!=0 || nReplaceTrig==0 );
  ఒకవేళ( nReplaceTrig ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    ఒకవేళ( !pPk ){
      ఒకవేళ( isUpdate ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, reg_New_దత్తాంశం, addrRecheck, reg_Old_దత్తాంశం);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, i_దత్తాంశం_Cur, addrRecheck, reg_New_దత్తాంశం);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }లేకపోతే{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  ఒకవేళ( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  ఒకవేళ( HasRowid(pTab) ){
    పూర్ణాంకం regRec = aRegIdx[ix];
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_New_దత్తాంశం+1, pTab->nNVCol, regRec);
    sqlite3_సమితి_Make_రికార్డు_P5(v, pTab);
    ఒకవేళ( !b_Affinity_పూర్తి ){
      sqlite3_పట్టిక_Affinity(v, pTab, 0);
    }
  }

  *pbMayReplace = చూసిన_Replace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", చూసిన_Replace));
}

#నిర్వచితమైతే SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
శూన్య sqlite3_సమితి_Make_రికార్డు_P5(Vdbe *v, పట్టిక *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  ఒకవేళ( pTab->pSchema->file_format<2 ) తిరిగి;

  కొరకు(i=pTab->n_నిలువు-1; i>0; i--){
    ఒకవేళ( pTab->a_నిలువు[i].iDflt!=0 ) ఆపు;
    ఒకవేళ( pTab->a_నిలువు[i].నిలువు_Flags & COLFLAG_PRIMKEY ) ఆపు;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#అయితే_ముగింపు

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
స్థిర శూన్య సంకేతం_Without_Rowid_Preupdate(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Table being updated */
  పూర్ణాంకం iCur,                       /* Cursor number for table */
  పూర్ణాంకం reg_దత్తాంశం                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం r = sqlite3_Get_తాత్కాలిక_Reg(pParse);
  ప్రతిజ్ఞా( !HasRowid(pTab) );
  ప్రతిజ్ఞా( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, r);
  sqlite3_Vdbe_కూడు_Op4(v, OP_చొప్పించు, iCur, reg_దత్తాంశం, r, (అక్షరం*)pTab, P4_పట్టిక);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, r);
}
#లేకపోతే
# define సంకేతం_Without_Rowid_Preupdate(a,b,c,d)
#అయితే_ముగింపు

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
శూన్య sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  పట్టిక *pTab,        /* the table into which we are inserting */
  పూర్ణాంకం i_దత్తాంశం_Cur,       /* Cursor of the canonical data source */
  పూర్ణాంకం iIdxCur,        /* First index cursor */
  పూర్ణాంకం reg_New_దత్తాంశం,     /* Range of content */
  పూర్ణాంకం *aRegIdx,       /* Register used by each index.  0 for unused indices */
  పూర్ణాంకం update_flags,   /* True for UPDATE, False for INSERT */
  పూర్ణాంకం appendBias,     /* True if this is likely to be an append */
  పూర్ణాంకం use_Seek_ఫలితం   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  సూచీ *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  పూర్ణాంకం i;              /* Loop counter */

  ప్రతిజ్ఞా( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( !IsView(pTab) );  /* This table is not a VIEW */
  కొరకు(i=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, i++){
    /* All REPLACE indexes are at the end of the list */
    ప్రతిజ్ఞా( pIdx->onError!=OE_Replace
         || pIdx->p_తదుపరి==0
         || pIdx->p_తదుపరి->onError==OE_Replace );
    ఒకవేళ( aRegIdx[i]==0 ) కొనసాగు;
    ఒకవేళ( pIdx->pPartIdxWhere || (update_flags && pIdx->b_Has_సమాసం) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (use_Seek_ఫలితం ? OPFLAG_USESEEKRESULT : 0);
    ఒకవేళ( Is_Primary_కీలకం_సూచీ(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      ఒకవేళ( update_flags==0 ){
        సంకేతం_Without_Rowid_Preupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->n_కీలకం_నిలువు: pIdx->n_స్తంభం);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  ఒకవేళ( !HasRowid(pTab) ) తిరిగి;
  ఒకవేళ( pParse->nested ){
    pik_flags = 0;
  }లేకపోతే{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  ఒకవేళ( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  ఒకవేళ( use_Seek_ఫలితం ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, i_దత్తాంశం_Cur, aRegIdx[i], reg_New_దత్తాంశం);
  ఒకవేళ( !pParse->nested ){
    sqlite3VdbeAppendP4(v, pTab, P4_పట్టిక);
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
పూర్ణాంకం sqlite3_Open_పట్టిక_And_Indices(
  Parse *pParse,   /* Parsing context */
  పట్టిక *pTab,     /* Table to be opened */
  పూర్ణాంకం op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  పూర్ణాంకం i_పునాది,       /* Use this for the table cursor, if there is one */
  u8 *a_గమ్యం_Open,     /* If not NULL: boolean for each table and index */
  పూర్ణాంకం *pi_దత్తాంశం_Cur,  /* Write the database source cursor number here */
  పూర్ణాంకం *piIdxCur    /* Write the first index cursor number here */
){
  పూర్ణాంకం i;
  పూర్ణాంకం iDb;
  పూర్ణాంకం i_దత్తాంశం_Cur;
  సూచీ *pIdx;
  Vdbe *v;

  ప్రతిజ్ఞా( op==OP_OpenRead || op==OP_OpenWrite );
  ప్రతిజ్ఞా( op==OP_OpenWrite || p5==0 );
  ప్రతిజ్ఞా( pi_దత్తాంశం_Cur!=0 );
  ప్రతిజ్ఞా( piIdxCur!=0 );
  ఒకవేళ( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *pi_దత్తాంశం_Cur = *piIdxCur = -999;
    తిరిగి 0;
  }
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  ఒకవేళ( i_పునాది<0 ) i_పునాది = pParse->nTab;
  i_దత్తాంశం_Cur = i_పునాది++;
  *pi_దత్తాంశం_Cur = i_దత్తాంశం_Cur;
  ఒకవేళ( HasRowid(pTab) && (a_గమ్యం_Open==0 || a_గమ్యం_Open[0]) ){
    sqlite3_Open_పట్టిక(pParse, i_దత్తాంశం_Cur, iDb, pTab, op);
  }లేకపోతే ఒకవేళ( pParse->db->noSharedCache==0 ){
    sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = i_పునాది;
  కొరకు(i=0, pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి, i++){
    పూర్ణాంకం iIdxCur = i_పునాది++;
    ప్రతిజ్ఞా( pIdx->pSchema==pTab->pSchema );
    ఒకవేళ( Is_Primary_కీలకం_సూచీ(pIdx) && !HasRowid(pTab) ){
      *pi_దత్తాంశం_Cur = iIdxCur;
      p5 = 0;
    }
    ఒకవేళ( a_గమ్యం_Open==0 || a_గమ్యం_Open[i+1] ){
      sqlite3_Vdbe_కూడు_Op3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  ఒకవేళ( i_పునాది>pParse->nTab ) pParse->nTab = i_పునాది;
  తిరిగి i;
}


#నిర్వచితమైతే SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
పూర్ణాంకం sqlite3_xferopt_లెక్క;
#అయితే_ముగింపు /* SQLITE_TEST */


#నిర్వచితం_కాకపోతే SQLITE_OMIT_XFER_OPT
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
స్థిర పూర్ణాంకం xfer_Compatible_సూచీ(సూచీ *pDest, సూచీ *pSrc){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( pDest && pSrc );
  ప్రతిజ్ఞా( pDest->p_పట్టిక!=pSrc->p_పట్టిక );
  ఒకవేళ( pDest->n_కీలకం_నిలువు!=pSrc->n_కీలకం_నిలువు || pDest->n_స్తంభం!=pSrc->n_స్తంభం ){
    తిరిగి 0;   /* Different number of columns */
  }
  ఒకవేళ( pDest->onError!=pSrc->onError ){
    తిరిగి 0;   /* Different conflict resolution strategies */
  }
  కొరకు(i=0; i<pSrc->n_కీలకం_నిలువు; i++){
    ఒకవేళ( pSrc->ai_స్తంభం[i]!=pDest->ai_స్తంభం[i] ){
      తిరిగి 0;   /* Different columns indexed */
    }
    ఒకవేళ( pSrc->ai_స్తంభం[i]==XN_సమాసం ){
      ప్రతిజ్ఞా( pSrc->a_నిలువు_సమాసం!=0 && pDest->a_నిలువు_సమాసం!=0 );
      ఒకవేళ( sqlite3_సమాసం_Compare(0, pSrc->a_నిలువు_సమాసం->a[i].p_సమాసం,
                             pDest->a_నిలువు_సమాసం->a[i].p_సమాసం, -1)!=0 ){
        తిరిగి 0;   /* Different expressions in the index */
      }
    }
    ఒకవేళ( pSrc->a_క్రమీకరించు_క్రమం[i]!=pDest->a_క్రమీకరించు_క్రమం[i] ){
      తిరిగి 0;   /* Different sort orders */
    }
    ఒకవేళ( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      తిరిగి 0;   /* Different collating sequences */
    }
  }
  ఒకవేళ( sqlite3_సమాసం_Compare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    తిరిగి 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  తిరిగి 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
స్థిర పూర్ణాంకం xferCheckRowid(Walker *pWalk, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం && p_సమాసం->i_స్తంభం<0 ){
    pWalk->e_సంకేతం = 1;
    తిరిగి WRC_Abort;
  }లేకపోతే{
    తిరిగి WRC_Continue;
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
స్థిర పూర్ణాంకం xferCompatibleCheck(పట్టిక *pDest, పట్టిక *pSrc){
  ఒకవేళ( sqlite3_సమాసం_List_Compare(pSrc->pCheck,pDest->pCheck,-1) ){
    తిరిగి 0;
  }
  ఒకవేళ( pDest->iPKey<0 ){
    Walker w;
    memset(&w, 0, పరిమాణం(w));
    w.x_సమాసం_Callback = xferCheckRowid;
    sqlite3_Walk_సమాసం_List(&w,pDest->pCheck);
    ఒకవేళ( w.e_సంకేతం ) తిరిగి 0;
  }
  తిరిగి 1;
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
స్థిర పూర్ణాంకం xferOptimization(
  Parse *pParse,        /* Parser context */
  పట్టిక *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  పూర్ణాంకం onError,          /* How to handle constraint errors */
  పూర్ణాంకం iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  సమాసం_List *pEList;                /* The result set of the SELECT */
  పట్టిక *pSrc;                     /* The table in the FROM clause of SELECT */
  సూచీ *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  Src_అంశం *p_అంశం;                  /* An element of pSelect->pSrc */
  పూర్ణాంకం i;                           /* Loop counter */
  పూర్ణాంకం iDbSrc;                      /* The database of pSrc */
  పూర్ణాంకం iSrc, iDest;                 /* Cursors from source and destination */
  పూర్ణాంకం addr1, addr2;                /* Loop addresses */
  పూర్ణాంకం ఖాళీలు_Dest_Test = 0;           /* Address of test for empty pDest */
  పూర్ణాంకం ఖాళీలు_Src_Test = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  పూర్ణాంకం regAutoinc;                  /* Memory register used by AUTOINC */
  పూర్ణాంకం destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  పూర్ణాంకం reg_దత్తాంశం, regRowid;           /* Registers holding data and rowid */

  ప్రతిజ్ఞా( pSelect!=0 );
  ఒకవేళ( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    తిరిగి 0;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ( IsVirtual(pDest) ){
    తిరిగి 0;   /* tab1 must not be a virtual table */
  }
#అయితే_ముగింపు
  ఒకవేళ( onError==OE_Default ){
    ఒకవేళ( pDest->iPKey>=0 ) onError = pDest->కీలకం_Conf;
    ఒకవేళ( onError==OE_Default ) onError = OE_Abort;
  }
  ప్రతిజ్ఞా(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  ఒకవేళ( pSelect->pSrc->nSrc!=1 ){
    తిరిగి 0;   /* FROM clause must have exactly one term */
  }
  ఒకవేళ( pSelect->pSrc->a[0].fg.isSubquery ){
    తిరిగి 0;   /* FROM clause cannot contain a subquery */
  }
  ఒకవేళ( pSelect->pWhere ){
    తిరిగి 0;   /* SELECT may not have a WHERE clause */
  }
  ఒకవేళ( pSelect->p_క్రమం_By ){
    తిరిగి 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  ఒకవేళ( pSelect->p_సమూహం_By ){
    తిరిగి 0;   /* SELECT may not have a GROUP BY clause */
  }
  ఒకవేళ( pSelect->p_హద్దు ){
    తిరిగి 0;   /* SELECT may not have a LIMIT clause */
  }
  ఒకవేళ( pSelect->p_పూర్వ ){
    తిరిగి 0;   /* SELECT may not be a compound query */
  }
  ఒకవేళ( pSelect->selFlags & SF_Distinct ){
    తిరిగి 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  ప్రతిజ్ఞా( pEList!=0 );
  ఒకవేళ( pEList->n_సమాసం!=1 ){
    తిరిగి 0;   /* The result set must have exactly one column */
  }
  ప్రతిజ్ఞా( pEList->a[0].p_సమాసం );
  ఒకవేళ( pEList->a[0].p_సమాసం->op!=TK_ASTERISK ){
    తిరిగి 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  p_అంశం = pSelect->pSrc->a;
  pSrc = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, p_అంశం);
  ఒకవేళ( pSrc==0 ){
    తిరిగి 0;   /* FROM clause does not contain a real table */
  }
  ఒకవేళ( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    తిరిగి 0;   /* tab1 and tab2 may not be the same table */
  }
  ఒకవేళ( HasRowid(pDest)!=HasRowid(pSrc) ){
    తిరిగి 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  ఒకవేళ( !Is_Ordinary_పట్టిక(pSrc) ){
    తిరిగి 0;   /* tab2 may not be a view or virtual table */
  }
  ఒకవేళ( pDest->n_నిలువు!=pSrc->n_నిలువు ){
    తిరిగి 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  ఒకవేళ( pDest->iPKey!=pSrc->iPKey ){
    తిరిగి 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  ఒకవేళ( (pDest->tabFlags & TF_Strict)!=0 ){
    ఒకవేళ( (pSrc->tabFlags & TF_Strict)==0 ){
      తిరిగి 0;  /* Cannot feed from a non-strict into a strict table */
    }
    కొరకు(i=0; i<pDest->n_నిలువు; i++){
      గుర్తులేని eDestType = pDest->a_నిలువు[i].eCType;
      గుర్తులేని eSrcType = pSrc->a_నిలువు[i].eCType;
      ఒకవేళ( eDestType==COLTYPE_ANY ) కొనసాగు;
      ఒకవేళ( eDestType==eSrcType ) కొనసాగు;
      ఒకవేళ( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) కొనసాగు;
      ఒకవేళ( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) కొనసాగు;
      తిరిగి 0; /* Incompatible types in source and destination */
    }
  }
  కొరకు(i=0; i<pDest->n_నిలువు; i++){
    స్తంభం *p_Dest_నిలువు = &pDest->a_నిలువు[i];
    స్తంభం *p_Src_నిలువు = &pSrc->a_నిలువు[i];
#నిర్వచితమైతే SQLITE_ENABLE_HIDDEN_COLUMNS
    ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (p_Dest_నిలువు->నిలువు_Flags | p_Src_నిలువు->నిలువు_Flags) & COLFLAG_HIDDEN
    ){
      తిరిగి 0;    /* Neither table may have __hidden__ columns */
    }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
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
    ఒకవేళ( (p_Dest_నిలువు->నిలువు_Flags & COLFLAG_GENERATED) !=
        (p_Src_నిలువు->నిలువు_Flags & COLFLAG_GENERATED) ){
      తిరిగి 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    ఒకవేళ( (p_Dest_నిలువు->నిలువు_Flags & COLFLAG_GENERATED)!=0 ){
      ఒకవేళ( sqlite3_సమాసం_Compare(0,
             sqlite3_స్తంభం_సమాసం(pSrc, p_Src_నిలువు),
             sqlite3_స్తంభం_సమాసం(pDest, p_Dest_నిలువు), -1)!=0 ){
        testcase( p_Dest_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL );
        testcase( p_Dest_నిలువు->నిలువు_Flags & COLFLAG_STORED );
        తిరిగి 0;  /* Different generator expressions */
      }
    }
#అయితే_ముగింపు
    ఒకవేళ( p_Dest_నిలువు->affinity!=p_Src_నిలువు->affinity ){
      తిరిగి 0;    /* Affinity must be the same on all columns */
    }
    ఒకవేళ( sqlite3_stricmp(sqlite3_స్తంభం_Coll(p_Dest_నిలువు),
                        sqlite3_స్తంభం_Coll(p_Src_నిలువు))!=0 ){
      తిరిగి 0;    /* Collating sequence must be the same on all columns */
    }
    ఒకవేళ( p_Dest_నిలువు->notNull && !p_Src_నిలువు->notNull ){
      తిరిగి 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    ఒకవేళ( (p_Dest_నిలువు->నిలువు_Flags & COLFLAG_GENERATED)==0 && i>0 ){
      సమాసం *p_Dest_సమాసం = sqlite3_స్తంభం_సమాసం(pDest, p_Dest_నిలువు);
      సమాసం *p_Src_సమాసం = sqlite3_స్తంభం_సమాసం(pSrc, p_Src_నిలువు);
      ప్రతిజ్ఞా( p_Dest_సమాసం==0 || p_Dest_సమాసం->op==TK_పరిధి );
      ప్రతిజ్ఞా( p_Dest_సమాసం==0 || !సమాసం_Has_Property(p_Dest_సమాసం, EP_Int_విలువ) );
      ప్రతిజ్ఞా( p_Src_సమాసం==0 || p_Src_సమాసం->op==TK_పరిధి );
      ప్రతిజ్ఞా( p_Src_సమాసం==0 || !సమాసం_Has_Property(p_Src_సమాసం, EP_Int_విలువ) );
      ఒకవేళ( (p_Dest_సమాసం==0)!=(p_Src_సమాసం==0)
       || (p_Dest_సమాసం!=0 && strcmp(p_Dest_సమాసం->u.zToken,
                                       p_Src_సమాసం->u.zToken)!=0)
      ){
        తిరిగి 0;    /* Default values must be the same for all columns */
      }
    }
  }
  కొరకు(pDestIdx=pDest->p_సూచీ; pDestIdx; pDestIdx=pDestIdx->p_తదుపరి){
    ఒకవేళ( Is_Unique_సూచీ(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    కొరకు(pSrcIdx=pSrc->p_సూచీ; pSrcIdx; pSrcIdx=pSrcIdx->p_తదుపరి){
      ఒకవేళ( xfer_Compatible_సూచీ(pDestIdx, pSrcIdx) ) ఆపు;
    }
    ఒకవేళ( pSrcIdx==0 ){
      తిరిగి 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    ఒకవేళ( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      తిరిగి 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CHECK
  ఒకవేళ( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    తిరిగి 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pDest) );
  ఒకవేళ( (db->flags & SQLITE_Foreign_కీలకాలు)!=0 && pDest->u.tab.pFKey!=0 ){
    తిరిగి 0;
  }
#అయితే_ముగింపు
  ఒకవేళ( (db->flags & SQLITE_లెక్క_అడ్డువరుసలు)!=0 ){
    తిరిగి 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( db->xAuth ){
    పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pSrc->pSchema);
    ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) తిరిగి 0;
    కొరకు(i=0; i<pSrc->n_నిలువు; i++){
      స్తంభం *p_Src_నిలువు = &pSrc->a_నిలువు[i];
      ఒకవేళ( sqlite3_Auth_Read_నిలువు(pParse, pSrc->zName, p_Src_నిలువు->zCnName, iDb) ){
        తిరిగి 0;
      }
    }
  }
#అయితే_ముగింపు

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#నిర్వచితమైతే SQLITE_TEST
  sqlite3_xferopt_లెక్క++;
#అయితే_ముగింపు
  iDbSrc = sqlite3_Schema_గమ్యం_సూచీ(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3_సంకేతం_Verify_Schema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  reg_దత్తాంశం = sqlite3_Get_తాత్కాలిక_Reg(pParse);
  sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_దత్తాంశం);
  regRowid = sqlite3_Get_తాత్కాలిక_Reg(pParse);
  sqlite3_Open_పట్టిక(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  ప్రతిజ్ఞా( HasRowid(pDest) || destHasUniqueIdx );
  ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->p_సూచీ!=0)          /* (1) */
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
    addr1 = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iDest, 0); VdbeCoverage(v);
    ఖాళీలు_Dest_Test = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
  }
  ఒకవేళ( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3_Open_పట్టిక(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    ఖాళీలు_Src_Test = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ఒకవేళ( pDest->iPKey>=0 ){
      addr1 = sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iSrc, regRowid);
      ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3_Vdbe_కూడు_Op3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, addr2);
      }
      auto_Inc_అడుగు(pParse, regAutoinc, regRowid);
    }లేకపోతే ఒకవేళ( pDest->p_సూచీ==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iDest, regRowid);
    }లేకపోతే{
      addr1 = sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iSrc, regRowid);
      ప్రతిజ్ఞా( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    ఒకవేళ( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3_Vdbe_కూడు_Op1(v, OP_Seek_చివర, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }లేకపోతే{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
    ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_అడ్డువరుస_దత్తాంశం, iSrc, reg_దత్తాంశం, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }లేకపోతే
#అయితే_ముగింపు
    {
      sqlite3_Vdbe_కూడు_Op3(v, OP_అడ్డువరుస_Cell, iDest, iSrc, regRowid);
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iDest, reg_దత్తాంశం, regRowid);
    ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (అక్షరం*)pDest, P4_పట్టిక);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iSrc, addr1); VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, iDest, 0);
  }లేకపోతే{
    sqlite3_పట్టిక_తాళం(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3_పట్టిక_తాళం(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  కొరకు(pDestIdx=pDest->p_సూచీ; pDestIdx; pDestIdx=pDestIdx->p_తదుపరి){
    u8 idxInsFlags = 0;
    కొరకు(pSrcIdx=pSrc->p_సూచీ; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->p_తదుపరి){
      ఒకవేళ( xfer_Compatible_సూచీ(pDestIdx, pSrcIdx) ) ఆపు;
    }
    ప్రతిజ్ఞా( pSrcIdx );
    sqlite3_Vdbe_కూడు_Op3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3_Vdbe_కూడు_Op3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ఒకవేళ( db->mDbFlags & DBFLAG_Vacuum ){
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
      కొరకు(i=0; i<pSrcIdx->n_స్తంభం; i++){
        స్థిరరాశి అక్షరం *zColl = pSrcIdx->azColl[i];
        ఒకవేళ( sqlite3_stricmp(sqlite3_Str_ద్విమాన, zColl) ) ఆపు;
      }
      ఒకవేళ( i==pSrcIdx->n_స్తంభం ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3_Vdbe_కూడు_Op1(v, OP_Seek_చివర, iDest);
        sqlite3_Vdbe_కూడు_Op2(v, OP_అడ్డువరుస_Cell, iDest, iSrc);
      }
    }లేకపోతే ఒకవేళ( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    ఒకవేళ( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_అడ్డువరుస_దత్తాంశం, iSrc, reg_దత్తాంశం, 1);
      ఒకవేళ( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && Is_Primary_కీలకం_సూచీ(pDestIdx)
      ){
        సంకేతం_Without_Rowid_Preupdate(pParse, pDest, iDest, reg_దత్తాంశం);
      }
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_Idx_చొప్పించు, iDest, reg_దత్తాంశం);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, iDest, 0);
  }
  ఒకవేళ( ఖాళీలు_Src_Test ) sqlite3_Vdbe_Jump_ఇక్కడ(v, ఖాళీలు_Src_Test);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regRowid);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_దత్తాంశం);
  ఒకవేళ( ఖాళీలు_Dest_Test ){
    sqlite3_Autoincrement_చివర(pParse);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, ఖాళీలు_Dest_Test);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Close, iDest, 0);
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి 1;
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_XFER_OPT */
