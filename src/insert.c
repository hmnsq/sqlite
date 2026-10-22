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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
ಶೂನ್ಯ sqlite3_Open_ಕೋಷ್ಟಕ(
  Parse *pParse,  /* Generate code into this VDBE */
  ಪೂರ್ಣಾಂಕ iCur,       /* The cursor number of the table */
  ಪೂರ್ಣಾಂಕ iDb,        /* The database index in sqlite3.aDb[] */
  ಕೋಷ್ಟಕ *pTab,    /* The table to be opened */
  ಪೂರ್ಣಾಂಕ opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  ಪ್ರತಿಜ್ಞಾ( !IsVirtual(pTab) );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  ಆದರೆ( !pParse->db->noSharedCache ){
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  ಆದರೆ( HasRowid(pTab) ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
    ಪ್ರತಿಜ್ಞಾ( pPk!=0 );
    ಪ್ರತಿಜ್ಞಾ( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pPk);
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
ಸ್ಥಿರ SQLITE_NOINLINE ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *compute_ಸೂಚ್ಯಂಕ_Aff_Str(sqlite3 *db, ಸೂಚ್ಯಂಕ *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  ಪೂರ್ಣಾಂಕ n;
  ಕೋಷ್ಟಕ *pTab = pIdx->p_ಕೋಷ್ಟಕ;
  pIdx->z_ಲಂಬಸಾಲು_Aff = (ಅಕ್ಷರ *)sqlite3DbMallocRaw(0, pIdx->n_ಸ್ತಂಭ+1);
  ಆದರೆ( !pIdx->z_ಲಂಬಸಾಲು_Aff ){
    sqlite3OomFault(db);
    ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ(n=0; n<pIdx->n_ಸ್ತಂಭ; n++){
    i16 x = pIdx->ai_ಸ್ತಂಭ[n];
    ಅಕ್ಷರ aff;
    ಆದರೆ( x>=0 ){
      aff = pTab->a_ಲಂಬಸಾಲು[x].affinity;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( x==XN_ಅಭಿವ್ಯಕ್ತಿ );
      ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
      ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[n].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಆದರೆ( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    ಆದರೆ( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->z_ಲಂಬಸಾಲು_Aff[n] = aff;
  }
  pIdx->z_ಲಂಬಸಾಲು_Aff[n] = 0;
  ಹಿಂತಿರುಗಿ pIdx->z_ಲಂಬಸಾಲು_Aff;
}
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(sqlite3 *db, ಸೂಚ್ಯಂಕ *pIdx){
  ಆದರೆ( !pIdx->z_ಲಂಬಸಾಲು_Aff ) ಹಿಂತಿರುಗಿ compute_ಸೂಚ್ಯಂಕ_Aff_Str(db, pIdx);
  ಹಿಂತಿರುಗಿ pIdx->z_ಲಂಬಸಾಲು_Aff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
ಅಕ್ಷರ *sqlite3_ಕೋಷ್ಟಕ_Affinity_Str(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pTab){
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_Aff;
  z_ಲಂಬಸಾಲು_Aff = (ಅಕ್ಷರ *)sqlite3DbMallocRaw(db, pTab->n_ಲಂಬಸಾಲು+1);
  ಆದರೆ( z_ಲಂಬಸಾಲು_Aff ){
    ಪೂರ್ಣಾಂಕ i, j;
    ಗಾಗಿ(i=j=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( (pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)==0 ){
        z_ಲಂಬಸಾಲು_Aff[j++] = pTab->a_ಲಂಬಸಾಲು[i].affinity;
      }
    }
    ಮಾಡು{
      z_ಲಂಬಸಾಲು_Aff[j--] = 0;
    }ಆಗ( j>=0 && z_ಲಂಬಸಾಲು_Aff[j]<=SQLITE_AFF_BLOB );
  }
  ಹಿಂತಿರುಗಿ z_ಲಂಬಸಾಲು_Aff; 
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
ಶೂನ್ಯ sqlite3_ಕೋಷ್ಟಕ_Affinity(Vdbe *v, ಕೋಷ್ಟಕ *pTab, ಪೂರ್ಣಾಂಕ iReg){
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_Aff;
  ಆದರೆ( pTab->tabFlags & TF_Strict ){
    ಆದರೆ( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *pPrev;
      ಪೂರ್ಣಾಂಕ p3;
      sqlite3VdbeAppendP4(v, pTab, P4_ಕೋಷ್ಟಕ);
      pPrev = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v);
      ಪ್ರತಿಜ್ಞಾ( pPrev!=0 );
      ಪ್ರತಿಜ್ಞಾ( pPrev->opcode==OP_Make_ದಾಖಲೆ || sqlite3VdbeDb(v)->mallocFailed );
      pPrev->opcode = OP_TypeCheck;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, pPrev->p1, pPrev->p2, p3);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Insert an isolated OP_Typecheck */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_ಕೋಷ್ಟಕ);
    }
    ಹಿಂತಿರುಗಿ;
  }
  z_ಲಂಬಸಾಲು_Aff = pTab->z_ಲಂಬಸಾಲು_Aff;
  ಆದರೆ( z_ಲಂಬಸಾಲು_Aff==0 ){
    z_ಲಂಬಸಾಲು_Aff = sqlite3_ಕೋಷ್ಟಕ_Affinity_Str(0, pTab);
    ಆದರೆ( !z_ಲಂಬಸಾಲು_Aff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      ಹಿಂತಿರುಗಿ;
    }
    pTab->z_ಲಂಬಸಾಲು_Aff = z_ಲಂಬಸಾಲು_Aff;
  }
  ಪ್ರತಿಜ್ಞಾ( z_ಲಂಬಸಾಲು_Aff!=0 );
  i = sqlite3Strlen30NN(z_ಲಂಬಸಾಲು_Aff);
  ಆದರೆ( i ){
    ಆದರೆ( iReg ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, iReg, i, 0, z_ಲಂಬಸಾಲು_Aff, i);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v)->opcode==OP_Make_ದಾಖಲೆ
              || sqlite3VdbeDb(v)->mallocFailed );
      sqlite3VdbeChangeP4(v, -1, z_ಲಂಬಸಾಲು_Aff, i);
    }
  }
}

/*
** Return non-zero if the table pTab in database iDb or any of its indices
** have been opened at any point in the VDBE program. This is used to see if
** a statement of the form  "INSERT INTO <iDb, pTab> SELECT ..." can
** run without using a temporary table for the results of the SELECT.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reads_ಕೋಷ್ಟಕ(Parse *p, ಪೂರ್ಣಾಂಕ iDb, ಕೋಷ್ಟಕ *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = sqlite3VdbeCurrentAddr(v);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ(i=1; i<i_ಕೊನೆ; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    ಪ್ರತಿಜ್ಞಾ( pOp!=0 );
    ಆದರೆ( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ;
      Pgno tnum = pOp->p2;
      ಆದರೆ( tnum==pTab->tnum ){
        ಹಿಂತಿರುಗಿ 1;
      }
      ಗಾಗಿ(p_ಸೂಚ್ಯಂಕ=pTab->p_ಸೂಚ್ಯಂಕ; p_ಸೂಚ್ಯಂಕ; p_ಸೂಚ್ಯಂಕ=p_ಸೂಚ್ಯಂಕ->p_ಮುಂದಿನ){
        ಆದರೆ( tnum==p_ಸೂಚ್ಯಂಕ->tnum ){
          ಹಿಂತಿರುಗಿ 1;
        }
      }
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಆದರೆ( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      ಪ್ರತಿಜ್ಞಾ( pOp->p4.pVtab!=0 );
      ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_VTAB );
      ಹಿಂತಿರುಗಿ 1;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸ್ತಂಭ_Flag_Union(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0 ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ < pWalker->u.pTab->n_ಲಂಬಸಾಲು );
    pWalker->e_ಸಂಕೇತ |= pWalker->u.pTab->a_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ].ಲಂಬಸಾಲು_Flags;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
ಶೂನ್ಯ sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  ಪೂರ್ಣಾಂಕ iRegStore,    /* Register holding the first column */
  ಕೋಷ್ಟಕ *pTab       /* The table */
){
  ಪೂರ್ಣಾಂಕ i;
  Walker w;
  ಸ್ತಂಭ *pRedo;
  ಪೂರ್ಣಾಂಕ e_ಪ್ರಗತಿ;
  VdbeOp *pOp;

  ಪ್ರತಿಜ್ಞಾ( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3_ಕೋಷ್ಟಕ_Affinity(pParse->pVdbe, pTab, iRegStore);
  ಆದರೆ( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(pParse->pVdbe);
    ಆದರೆ( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      ಪೂರ್ಣಾಂಕ ii, jj;
      ಅಕ್ಷರ *zP4 = pOp->p4.z;
      ಪ್ರತಿಜ್ಞಾ( zP4!=0 );
      ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_DYNAMIC );
      ಗಾಗಿ(ii=jj=0; zP4[jj]; ii++){
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[ii].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ){
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[ii].ಲಂಬಸಾಲು_Flags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->opcode==OP_TypeCheck ){
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
  ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
      testcase( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
      testcase( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
      pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಸ್ತಂಭ_Flag_Union;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  ಮಾಡು{
    e_ಪ್ರಗತಿ = 0;
    pRedo = 0;
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು + i;
      ಆದರೆ( (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_NOTAVAIL)!=0 ){
        ಪೂರ್ಣಾಂಕ x;
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_BUSY;
        w.e_ಸಂಕೇತ = 0;
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pTab, p_ಲಂಬಸಾಲು));
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags &= ~COLFLAG_BUSY;
        ಆದರೆ( w.e_ಸಂಕೇತ & COLFLAG_NOTAVAIL ){
          pRedo = p_ಲಂಬಸಾಲು;
          ಮುಂದುವರಿ;
        }
        e_ಪ್ರಗತಿ = 1;
        ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED );
        x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i) + iRegStore;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Generated_ಸ್ತಂಭ(pParse, pTab, p_ಲಂಬಸಾಲು, x);
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }ಆಗ( pRedo && e_ಪ್ರಗತಿ );
  ಆದರೆ( pRedo ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_GENERATED_COLUMNS */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ autoIncBegin(
  Parse *pParse,      /* Parsing context */
  ಪೂರ್ಣಾಂಕ iDb,            /* Index of the database holding pTab */
  ಕೋಷ್ಟಕ *pTab         /* The table we are writing to */
){
  ಪೂರ್ಣಾಂಕ memId = 0;      /* Register holding maximum rowid */
  ಪ್ರತಿಜ್ಞಾ( pParse->db->aDb[iDb].pSchema!=0 );
  ಆದರೆ( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    ಕೋಷ್ಟಕ *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    ಆದರೆ( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->n_ಲಂಬಸಾಲು!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      ಹಿಂತಿರುಗಿ 0;
    }

    ಆದರೆ( pToplevel->ಬಳಸುತ್ತದೆ_Ainc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    ಆಗ( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->p_ಮುಂದಿನ; }
    ಆದರೆ( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, ಗಾತ್ರ(*pInfo));
      sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      ಆದರೆ( pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ 0;
      pInfo->p_ಮುಂದಿನ = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pToplevel->ಬಳಸುತ್ತದೆ_Ainc = 1;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;                  /* Register to hold name of table */
      pInfo->regCtr = ++pToplevel->nMem;  /* Max rowid register */
      pToplevel->nMem +=2;       /* Rowid in sqlite_sequence + orig max val */
    }
    memId = pInfo->regCtr;
  }
  ಹಿಂತಿರುಗಿ memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
ಶೂನ್ಯ sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  ಪೂರ್ಣಾಂಕ memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  ಪ್ರತಿಜ್ಞಾ( pParse->pTriggerTab==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3IsToplevel(pParse) );

  ಪ್ರತಿಜ್ಞಾ( v );   /* We failed long ago if this is not so */
  ಪ್ರತಿಜ್ಞಾ( pParse->ಬಳಸುತ್ತದೆ_Ainc );
  ಗಾಗಿ(p = pParse->pAinc; p; p = p->p_ಮುಂದಿನ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_ಸ್ತಂಭ,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_ಸ್ತಂಭ,  0,  1, 0},
      /* 6  */ {OP_ಕೂಡಿಸು_Imm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_ಮುಂದಿನ,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3_Vdbe_ತುಂಬು_String(v, memId-1, p->pTab->zName);
    aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(autoInc), autoInc, iLn);
    ಆದರೆ( aOp==0 ) ನಿಲ್ಲಿಸು;
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
    ಆದರೆ( pParse->nTab==0 ) pParse->nTab = 1;
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
ಸ್ಥಿರ ಶೂನ್ಯ auto_Inc_ಹೆಜ್ಜೆ(Parse *pParse, ಪೂರ್ಣಾಂಕ memId, ಪೂರ್ಣಾಂಕ regRowid){
  ಆದರೆ( memId>0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(pParse->pVdbe, OP_MemMax, memId, regRowid);
  }
}

/*
** This routine generates the code needed to write autoincrement
** maximum rowid values back into the sqlite_sequence register.
** Every statement that might do an INSERT into an autoincrement
** table (either directly or through triggers) needs to call this
** routine just before the "exit" code.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ auto_Increment_ಕೊನೆ(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ಪ್ರತಿಜ್ಞಾ( v );
  ಪ್ರತಿಜ್ಞಾ( pParse->ಬಳಸುತ್ತದೆ_Ainc );
  ಗಾಗಿ(p = pParse->pAinc; p; p = p->p_ಮುಂದಿನ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList auto_Inc_ಕೊನೆ[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_Make_ದಾಖಲೆ,  0, 2, 0},
      /* 3 */ {OP_ಸೇರಿಸು,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    ಪೂರ್ಣಾಂಕ iRec;
    ಪೂರ್ಣಾಂಕ memId = p->regCtr;

    iRec = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(auto_Inc_ಕೊನೆ), auto_Inc_ಕೊನೆ, iLn);
    ಆದರೆ( aOp==0 ) ನಿಲ್ಲಿಸು;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, iRec);
  }
}
ಶೂನ್ಯ sqlite3_Autoincrement_ಕೊನೆ(Parse *pParse){
  ಆದರೆ( pParse->ಬಳಸುತ್ತದೆ_Ainc ) auto_Increment_ಕೊನೆ(pParse);
}
#ಇಲ್ಲದಿದ್ದರೆ
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define auto_Inc_ಹೆಜ್ಜೆ(A,B,C)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
ಶೂನ್ಯ sqlite3_Multi_ಮೌಲ್ಯಗಳು_ಕೊನೆ(Parse *pParse, Select *pVal){
  ಆದರೆ( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    Src_ಅಂಶ *p_ಅಂಶ = &pVal->pSrc->a[0];
    ಪ್ರತಿಜ್ಞಾ( (p_ಅಂಶ->fg.isSubquery && p_ಅಂಶ->u4.pSubq!=0) || pParse->nErr );
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery ){
      sqlite3_Vdbe_ಕೊನೆ_Coroutine(pParse->pVdbe, p_ಅಂಶ->u4.pSubq->regReturn);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(pParse->pVdbe, p_ಅಂಶ->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List_Is_No_Affinity(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಡ್ಡಸಾಲು){
  ಪೂರ್ಣಾಂಕ ii;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Is_Constant(pParse, p_ಅಡ್ಡಸಾಲು, 0)==0 ) ಹಿಂತಿರುಗಿ 0;
  ಗಾಗಿ(ii=0; ii<p_ಅಡ್ಡಸಾಲು->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಡ್ಡಸಾಲು->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_RAISE );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==0 );
    ಆದರೆ( 0!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ) ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;

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
Select *sqlite3_Multi_ಮೌಲ್ಯಗಳು(Parse *pParse, Select *p_ಎಡ, ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಡ್ಡಸಾಲು){

  ಆದರೆ( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Is_Constant(pParse, p_ಅಡ್ಡಸಾಲು, 1)==0   /* condition (c) above */
   || (p_ಎಡ->pSrc->nSrc==0 &&
       ಅಭಿವ್ಯಕ್ತಿ_List_Is_No_Affinity(pParse,p_ಎಡ->pEList)==0) /* condition (d) above */
   || ಒಳ_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    ಪೂರ್ಣಾಂಕ f = SF_ಮೌಲ್ಯಗಳು | SF_Multi_ಮೌಲ್ಯ;
    ಆದರೆ( p_ಎಡ->pSrc->nSrc ){
      sqlite3_Multi_ಮೌಲ್ಯಗಳು_ಕೊನೆ(pParse, p_ಎಡ);
      f = SF_ಮೌಲ್ಯಗಳು;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಎಡ->p_ಪೂರ್ವ ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & p_ಎಡ->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, p_ಅಡ್ಡಸಾಲು, 0, 0, 0, 0, 0, f, 0);
    p_ಎಡ->selFlags &= ~(u32)SF_Multi_ಮೌಲ್ಯ;
    ಆದರೆ( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->p_ಪೂರ್ವ = p_ಎಡ;
      p_ಎಡ = pSelect;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    Src_ಅಂಶ *p = 0;               /* SrcItem that reads from co-routine */

    ಆದರೆ( p_ಎಡ->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      ಆದರೆ( (pParse->db->mDbFlags & DBFLAG_Schema_ತಿಳಿದ_Ok)==0 ){
        sqlite3ReadSchema(pParse);
      }

      ಆದರೆ( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->p_ಪೂರ್ವ = p_ಎಡ->p_ಪೂರ್ವ;
        pRet->op = p_ಎಡ->op;
        ಆದರೆ( pRet->p_ಪೂರ್ವ ) pRet->selFlags |= SF_ಮೌಲ್ಯಗಳು;
        p_ಎಡ->p_ಪೂರ್ವ = 0;
        p_ಎಡ->op = TK_SELECT;
        ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->p_ಮುಂದಿನ==0 );
        ಪ್ರತಿಜ್ಞಾ( pRet->p_ಮುಂದಿನ==0 );
        p = &pRet->pSrc->a[0];
        p->fg.ಮೂಲಕ_Coroutine = 1;
        p->iCursor = -1;
        ಪ್ರತಿಜ್ಞಾ( !p->fg.is_ಸೂಚ್ಯಂಕಿತ_By && !p->fg.isTabFunc );
        p->u1.n_ಅಡ್ಡಸಾಲು = 2;
        ಆದರೆ( sqlite3_Src_ಅಂಶ_Attach_Subquery(pParse, p, p_ಎಡ, 0) ){
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine,
                            pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);

          /* Allocate registers for the output of the co-routine. Do so so
          ** that there are two unused registers immediately before those
          ** used by the co-routine. This allows the code in sqlite3Insert()
          ** to use these registers directly, instead of copying the output
          ** of the co-routine to a separate array for processing.  */
          dest.iSdst = pParse->nMem + 3; 
          dest.nSdst = p_ಎಡ->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
          pParse->nMem += 2 + dest.nSdst;

          p_ಎಡ->selFlags |= SF_Multi_ಮೌಲ್ಯ;
          sqlite3Select(pParse, p_ಎಡ, &dest);
          pSubq->reg_ಫಲಿತಾಂಶ = dest.iSdst;
          ಪ್ರತಿಜ್ಞಾ( pParse->nErr || dest.iSdst>0 );
        }
        p_ಎಡ = pRet;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      p = &p_ಎಡ->pSrc->a[0];
      ಪ್ರತಿಜ್ಞಾ( !p->fg.isTabFunc && !p->fg.is_ಸೂಚ್ಯಂಕಿತ_By );
      p->u1.n_ಅಡ್ಡಸಾಲು++;
    }
  
    ಆದರೆ( pParse->nErr==0 ){
      Subquery *pSubq;
      ಪ್ರತಿಜ್ಞಾ( p!=0 );
      ಪ್ರತಿಜ್ಞಾ( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      ಪ್ರತಿಜ್ಞಾ( pSubq!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect->pEList!=0 );
      ಆದರೆ( pSubq->pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=p_ಅಡ್ಡಸಾಲು->n_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(pParse, pSubq->pSelect);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p_ಅಡ್ಡಸಾಲು, pSubq->reg_ಫಲಿತಾಂಶ, 0, 0);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, p_ಅಡ್ಡಸಾಲು);
  }

  ಹಿಂತಿರುಗಿ p_ಎಡ;
}

/* Forward declaration */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xferOptimization(
  Parse *pParse,        /* Parser context */
  ಕೋಷ್ಟಕ *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  ಪೂರ್ಣಾಂಕ onError,          /* How to handle constraint errors */
  ಪೂರ್ಣಾಂಕ iDbDest           /* The database of pDest */
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
ಶೂನ್ಯ sqlite3_ಸೇರಿಸು(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *p_ಸ್ತಂಭ,      /* Column names corresponding to IDLIST, or NULL. */
  ಪೂರ್ಣಾಂಕ onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  ಕೋಷ್ಟಕ *pTab;          /* The table to insert into.  aka TABLE */
  ಪೂರ್ಣಾಂಕ i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  ಸೂಚ್ಯಂಕ *pIdx;          /* For looping over indices of the table */
  ಪೂರ್ಣಾಂಕ n_ಸ್ತಂಭ;          /* Number of columns in the data */
  ಪೂರ್ಣಾಂಕ nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur = 0;     /* VDBE cursor that is the main data repository */
  ಪೂರ್ಣಾಂಕ iIdxCur = 0;      /* First index cursor */
  ಪೂರ್ಣಾಂಕ ipk_ಸ್ತಂಭ = -1;   /* Column that is the INTEGER PRIMARY KEY */
  ಪೂರ್ಣಾಂಕ ಕೊನೆ_ದ_Loop;        /* Label for the end of the insertion loop */
  ಪೂರ್ಣಾಂಕ srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  ಪೂರ್ಣಾಂಕ addr_Ins_ತುದಿ = 0;   /* Jump to label "D" */
  ಪೂರ್ಣಾಂಕ addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  ಪೂರ್ಣಾಂಕ iDb;              /* Index of database holding TABLE */
  u8 use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 b_Id_List_ಒಳ_ಕ್ರಮ;    /* True if IDLIST is in table order */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList = 0;  /* List of VALUES() to be inserted  */
  ಪೂರ್ಣಾಂಕ iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  ಪೂರ್ಣಾಂಕ regFromSelect = 0;/* Base register for data coming from SELECT */
  ಪೂರ್ಣಾಂಕ regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  ಪೂರ್ಣಾಂಕ reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = 0;  /* Memory cell used for the row counter */
  ಪೂರ್ಣಾಂಕ regIns;           /* Block of regs holding rowid+data being inserted */
  ಪೂರ್ಣಾಂಕ regRowid;         /* registers holding insert rowid */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ;          /* register holding first column to insert */
  ಪೂರ್ಣಾಂಕ *aRegIdx = 0;     /* One register allocated to each index */
  ಪೂರ್ಣಾಂಕ *a_Tab_ಲಂಬಸಾಲು_Map = 0;  /* Mapping from pTab columns to pCol entries */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಪೂರ್ಣಾಂಕ isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  ಪೂರ್ಣಾಂಕ tmask;                  /* Mask of trigger times */
#ಆದರೆ_ಅಂತ್ಯ

  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
  ಆದರೆ( pParse->nErr ){
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  ಆದರೆ( pSelect && (pSelect->selFlags & SF_ಮೌಲ್ಯಗಳು)!=0 && pSelect->p_ಪೂರ್ವ==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  ಪ್ರತಿಜ್ಞಾ( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ಆದರೆ( pTab==0 ){
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb<db->nDb );
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ಸೇರಿಸು, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_ಸೇರಿಸು, 0, &tmask);
  isView = IsView(pTab);
#ಇಲ್ಲದಿದ್ದರೆ
# define pTrigger 0
# define tmask 0
# define isView 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x10000 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(0, "In sqlite3Insert() at %s:%d", __FILE__, __LINE__);
    sqlite3_ವೃಕ್ಷ_View_ಸೇರಿಸು(pParse->pWith, pTabList, p_ಸ್ತಂಭ, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  ಆದರೆ( sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab) ){
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  ಆದರೆ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v==0 ) ಹೋಗು ಸೇರಿಸು_cleanup;
  ಆದರೆ( pParse->nested==0 ) sqlite3_Vdbe_ಎಣಿಕೆ_Changes(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  ಆದರೆ( p_ಸ್ತಂಭ==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    ಪ್ರತಿಜ್ಞಾ( !pTrigger );
    ಪ್ರತಿಜ್ಞಾ( pList==0 );
    ಹೋಗು ಸೇರಿಸು_ಕೊನೆ;
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->n_ಲಂಬಸಾಲು + 1;
  ಆದರೆ( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  reg_ದತ್ತಾಂಶ = regRowid+1;

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
  b_Id_List_ಒಳ_ಕ್ರಮ = (pTab->tabFlags & (TF_OOOHidden|TF_HasStored))==0;
  ಆದರೆ( p_ಸ್ತಂಭ ){
    a_Tab_ಲಂಬಸಾಲು_Map = sqlite3DbMallocZero(db, pTab->n_ಲಂಬಸಾಲು*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
    ಆದರೆ( a_Tab_ಲಂಬಸಾಲು_Map==0 ) ಹೋಗು ಸೇರಿಸು_cleanup;
    ಗಾಗಿ(i=0; i<p_ಸ್ತಂಭ->nId; i++){
      j = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, p_ಸ್ತಂಭ->a[i].zName);
      ಆದರೆ( j>=0 ){
        ಆದರೆ( a_Tab_ಲಂಬಸಾಲು_Map[j]==0 ) a_Tab_ಲಂಬಸಾಲು_Map[j] = i+1;
        ಆದರೆ( i!=j ) b_Id_List_ಒಳ_ಕ್ರಮ = 0;
        ಆದರೆ( j==pTab->iPKey ){
          ipk_ಸ್ತಂಭ = i;  ಪ್ರತಿಜ್ಞಾ( !withoutRowid );
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3_Error_ಸಂದೇಶ(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->a_ಲಂಬಸಾಲು[j].zCnName);
          ಹೋಗು ಸೇರಿಸು_cleanup;
        }
#ಆದರೆ_ಅಂತ್ಯ
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( sqlite3IsRowid(p_ಸ್ತಂಭ->a[i].zName) && !withoutRowid ){
          ipk_ಸ್ತಂಭ = i;
          b_Id_List_ಒಳ_ಕ್ರಮ = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Error_ಸಂದೇಶ(pParse, "table %S has no column named %s",
              pTabList->a, p_ಸ್ತಂಭ->a[i].zName);
          pParse->checkSchema = 1;
          ಹೋಗು ಸೇರಿಸು_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  ಆದರೆ( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    ಪೂರ್ಣಾಂಕ rc;             /* Result code */

    ಆದರೆ( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.ಮೂಲಕ_Coroutine 
     && pSelect->p_ಪೂರ್ವ==0
    ){
      Src_ಅಂಶ *p_ಅಂಶ = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isSubquery );
      pSubq = p_ಅಂಶ->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->reg_ಫಲಿತಾಂಶ;
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect->pEList!=0 );
      n_ಸ್ತಂಭ = pSubq->pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ExplainQueryPlan((pParse, 0, "SCAN %S", p_ಅಂಶ));
      ಆದರೆ( b_Id_List_ಒಳ_ಕ್ರಮ && n_ಸ್ತಂಭ==pTab->n_ಲಂಬಸಾಲು ){
        reg_ದತ್ತಾಂಶ = regFromSelect;
        regRowid = reg_ದತ್ತಾಂಶ - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ addr_ತುದಿ;        /* Top of the co-routine */
      ಪೂರ್ಣಾಂಕ regYield = ++pParse->nMem;
      addr_ತುದಿ = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine, regYield, 0, addr_ತುದಿ);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = b_Id_List_ಒಳ_ಕ್ರಮ ? reg_ದತ್ತಾಂಶ : 0;
      dest.nSdst = pTab->n_ಲಂಬಸಾಲು;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      ಪ್ರತಿಜ್ಞಾ( db->pParse==pParse );
      ಆದರೆ( rc || pParse->nErr ) ಹೋಗು ಸೇರಿಸು_cleanup;
      ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
      sqlite3_Vdbe_ಕೊನೆ_Coroutine(v, regYield);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_ತುದಿ - 1);                       /* label B: */
      ಪ್ರತಿಜ್ಞಾ( pSelect->pEList );
      n_ಸ್ತಂಭ = pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
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
    ಆದರೆ( pTrigger || reads_ಕೋಷ್ಟಕ(pParse, iDb, pTab) ){
      use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ = 1;
    }

    ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
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
      ಪೂರ್ಣಾಂಕ regRec;          /* Register to hold packed record */
      ಪೂರ್ಣಾಂಕ reg_ತಾತ್ಕಾಲಿಕ_Rowid;    /* Register to hold temp table ROWID */
      ಪೂರ್ಣಾಂಕ addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      reg_ತಾತ್ಕಾಲಿಕ_Rowid = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, srcTab, n_ಸ್ತಂಭ);
      addrL = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regFromSelect, n_ಸ್ತಂಭ, regRec);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, srcTab, reg_ತಾತ್ಕಾಲಿಕ_Rowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, srcTab, regRec, reg_ತಾತ್ಕಾಲಿಕ_Rowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrL);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regRec);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ತಾತ್ಕಾಲಿಕ_Rowid);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    memset(&sNC, 0, ಗಾತ್ರ(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    ಪ್ರತಿಜ್ಞಾ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ==0 );
    ಆದರೆ( pList ){
      n_ಸ್ತಂಭ = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pList) ){
        ಹೋಗು ಸೇರಿಸು_cleanup;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಸ್ತಂಭ = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  ಆದರೆ( p_ಸ್ತಂಭ==0 && n_ಸ್ತಂಭ>0 ){
    ipk_ಸ್ತಂಭ = pTab->iPKey;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
    ಆದರೆ( ipk_ಸ್ತಂಭ>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      ಗಾಗಿ(i=ipk_ಸ್ತಂಭ-1; i>=0; i--){
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
          testcase( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
          testcase( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
          ipk_ಸ್ತಂಭ--;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    ಪ್ರತಿಜ್ಞಾ( TF_HasHidden==COLFLAG_HIDDEN );
    ಪ್ರತಿಜ್ಞಾ( TF_HasGenerated==COLFLAG_GENERATED );
    ಪ್ರತಿಜ್ಞಾ( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    ಆದರೆ( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
        ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    ಆದರೆ( n_ಸ್ತಂಭ!=(pTab->n_ಲಂಬಸಾಲು-nHidden) ){
      sqlite3_Error_ಸಂದೇಶ(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->n_ಲಂಬಸಾಲು-nHidden, n_ಸ್ತಂಭ);
     ಹೋಗು ಸೇರಿಸು_cleanup;
    }
  }
  ಆದರೆ( p_ಸ್ತಂಭ!=0 && n_ಸ್ತಂಭ!=p_ಸ್ತಂಭ->nId ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "%d values for %d columns", n_ಸ್ತಂಭ, p_ಸ್ತಂಭ->nId);
    ಹೋಗು ಸೇರಿಸು_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  ಆದರೆ( (db->flags & SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ);
  }

  /* If this is not a view, open the table and and all indices */
  ಆದರೆ( !isView ){
    ಪೂರ್ಣಾಂಕ nIdx;
    nIdx = sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &i_ದತ್ತಾಂಶ_Cur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*(nIdx+2));
    ಆದರೆ( aRegIdx==0 ){
      ಹೋಗು ಸೇರಿಸು_cleanup;
    }
    ಗಾಗಿ(i=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; i<nIdx; pIdx=pIdx->p_ಮುಂದಿನ, i++){
      ಪ್ರತಿಜ್ಞಾ( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->n_ಸ್ತಂಭ;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
  ಆದರೆ( pUpsert ){
    Upsert *pNx;
    ಆದರೆ( IsVirtual(pTab) ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      ಹೋಗು ಸೇರಿಸು_cleanup;
    }
    ಆದರೆ( IsView(pTab) ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "cannot UPSERT a view");
      ಹೋಗು ಸೇರಿಸು_cleanup;
    }
    ಆದರೆ( sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_ಗುರಿ) ){
      ಹೋಗು ಸೇರಿಸು_cleanup;
    }
    pTabList->a[0].iCursor = i_ದತ್ತಾಂಶ_Cur;
    pNx = pUpsert;
    ಮಾಡು{
      pNx->pUpsertSrc = pTabList;
      pNx->reg_ದತ್ತಾಂಶ = reg_ದತ್ತಾಂಶ;
      pNx->i_ದತ್ತಾಂಶ_Cur = i_ದತ್ತಾಂಶ_Cur;
      pNx->iIdxCur = iIdxCur;
      ಆದರೆ( pNx->p_Upsert_ಗುರಿ ){
        ಆದರೆ( sqlite3_Upsert_Analyze_ಗುರಿ(pParse, pTabList, pNx, pUpsert) ){
          ಹೋಗು ಸೇರಿಸು_cleanup;
        }
      }
      pNx = pNx->p_ಮುಂದಿನ_Upsert;
    }ಆಗ( pNx!=0 );
  }
#ಆದರೆ_ಅಂತ್ಯ


  /* This is the top of the main insertion loop */
  ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addr_Ins_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, reg_ದತ್ತಾಂಶ, pTab->n_ಲಂಬಸಾಲು, 0, 0);
    addr_Ins_ತುದಿ = addrCont = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    ಆದರೆ( ipk_ಸ್ತಂಭ>=0 ){
      /* tag-20191021-001: If the INTEGER PRIMARY KEY is being generated by the
      ** SELECT, go ahead and copy the value into the rowid slot now, so that
      ** the value does not get overwritten by a NULL at tag-20191021-002. */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, regFromSelect+ipk_ಸ್ತಂಭ, regRowid);
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
  iRegStore = reg_ದತ್ತಾಂಶ;  ಪ್ರತಿಜ್ಞಾ( reg_ದತ್ತಾಂಶ==regRowid+1 );
  ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++, iRegStore++){
    ಪೂರ್ಣಾಂಕ k;
    u32 ಲಂಬಸಾಲು_Flags;
    ಪ್ರತಿಜ್ಞಾ( i>=nHidden );
    ಆದರೆ( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_SoftNull, iRegStore);
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( ((ಲಂಬಸಾಲು_Flags = pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      ಆದರೆ( (ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (ಲಂಬಸಾಲು_Flags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        ಆದರೆ( tmask & TRIGGER_BEFORE ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_SoftNull, iRegStore);
        }
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಸ್ತಂಭ==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(pParse,
            sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab, &pTab->a_ಲಂಬಸಾಲು[i], pParse),
            iRegStore);
        ಮುಂದುವರಿ;
      }
    }
    ಆದರೆ( p_ಸ್ತಂಭ ){
      j = a_Tab_ಲಂಬಸಾಲು_Map[i];
      ಪ್ರತಿಜ್ಞಾ( j>=0 && j<=p_ಸ್ತಂಭ->nId );   
      ಆದರೆ( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(pParse,
            sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab, &pTab->a_ಲಂಬಸಾಲು[i], pParse),
            iRegStore);
        ಮುಂದುವರಿ;
      }
      k = j - 1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n_ಸ್ತಂಭ==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(pParse,
          sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab, &pTab->a_ಲಂಬಸಾಲು[i], pParse),
          iRegStore);
      ಮುಂದುವರಿ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      k = i - nHidden;
    }

    ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, srcTab, k, iRegStore);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSelect ){
      ಆದರೆ( regFromSelect!=reg_ದತ್ತಾಂಶ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಭಿವ್ಯಕ್ತಿ *pX = pList->a[k].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಪೂರ್ಣಾಂಕ y = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, pX, iRegStore);
      ಆದರೆ( y!=iRegStore ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v,
          ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  ಕೊನೆ_ದ_Loop = sqlite3VdbeMakeLabel(pParse);
  ಆದರೆ( tmask & TRIGGER_BEFORE ){
    ಪೂರ್ಣಾಂಕ reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, pTab->n_ಲಂಬಸಾಲು+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    ಆದರೆ( ipk_ಸ್ತಂಭ<0 ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, -1, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ addr1;
      ಪ್ರತಿಜ್ಞಾ( !withoutRowid );
      ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, srcTab, ipk_ಸ್ತಂಭ, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pList->a[ipk_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
      }
      addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_NotNull, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ); VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, -1, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_MustBeInt, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    ಪ್ರತಿಜ್ಞಾ( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, regRowid+1, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ+1, pTab->nNVCol-1);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    ಆದರೆ( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ+1, pTab);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    ಆದರೆ( !isView ){
      sqlite3_ಕೋಷ್ಟಕ_Affinity(v, pTab, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger, TK_ಸೇರಿಸು, 0, TRIGGER_BEFORE,
        pTab, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ-pTab->n_ಲಂಬಸಾಲು-1, onError, ಕೊನೆ_ದ_Loop);

    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, pTab->n_ಲಂಬಸಾಲು+1);
  }

  ಆದರೆ( !isView ){
    ಆದರೆ( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, regIns);
    }
    ಆದರೆ( ipk_ಸ್ತಂಭ>=0 ){
      /* Compute the new rowid */
      ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, srcTab, ipk_ಸ್ತಂಭ, regRowid);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *pIpk = pList->a[ipk_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NewRowid, i_ದತ್ತಾಂಶ_Cur, regRowid, regAutoinc);
          appendFlag = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pList->a[ipk_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      ಆದರೆ( !appendFlag ){
        ಪೂರ್ಣಾಂಕ addr1;
        ಆದರೆ( !IsVirtual(pTab) ){
          addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NewRowid, i_ದತ್ತಾಂಶ_Cur, regRowid, regAutoinc);
          sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( IsVirtual(pTab) || withoutRowid ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, regRowid);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NewRowid, i_ದತ್ತಾಂಶ_Cur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    auto_Inc_ಹೆಜ್ಜೆ(pParse, regAutoinc, regRowid);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    ಆದರೆ( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಆದರೆ( IsVirtual(pTab) ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pVTab = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_VUpdate, 1, pTab->n_ಲಂಬಸಾಲು+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಪೂರ್ಣಾಂಕ isReplace = 0;/* Set to true if constraints may cause a replace */
      ಪೂರ್ಣಾಂಕ bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
          regIns, 0, ipk_ಸ್ತಂಭ>=0, onError, ಕೊನೆ_ದ_Loop, &isReplace, 0, pUpsert
      );
      ಆದರೆ( db->flags & SQLITE_Foreign_ಕೀಲಿಗಳು ){
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
      bUseSeek = (isReplace==0 || !sqlite3_Vdbe_Has_Sub_ಕಾರ್ಯಕ್ರಮ(v));
      sqlite3CompleteInsertion(pParse, pTab, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, -1, regRowid);
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* Update the count of rows that are inserted
  */
  ಆದರೆ( reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ, 1);
  }

  ಆದರೆ( pTrigger ){
    /* Code AFTER triggers */
    sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(pParse, pTrigger, TK_ಸೇರಿಸು, 0, TRIGGER_AFTER,
        pTab, reg_ದತ್ತಾಂಶ-2-pTab->n_ಲಂಬಸಾಲು, onError, ಕೊನೆ_ದ_Loop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, ಕೊನೆ_ದ_Loop);
  ಆದರೆ( use_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, srcTab, addrCont); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_Ins_ತುದಿ);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Close, srcTab);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    ಆದರೆ( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_Ins_ತುದಿ);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_XFER_OPT
ಸೇರಿಸು_ಕೊನೆ:
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ಆದರೆ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_ಕೊನೆ(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ಆದರೆ( reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ ){
    sqlite3_ಸಂಕೇತ_Change_ಎಣಿಕೆ(v, reg_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ, "rows inserted");
  }

ಸೇರಿಸು_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  ಆದರೆ( p_ಸ್ತಂಭ ){
    sqlite3IdListDelete(db, p_ಸ್ತಂಭ);
    sqlite3DbFree(db, a_Tab_ಲಂಬಸಾಲು_Map);
  }
  ಆದರೆ( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ isView
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು isView
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ pTrigger
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು pTrigger
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ tmask
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು tmask
#ಆದರೆ_ಅಂತ್ಯ

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು CKCNSTRNT_ಸ್ತಂಭ   0x01    /* CHECK constraint uses a changing column */
#ವ್ಯಾಖ್ಯಾನಿಸು CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_Constraint_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0 || p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ==-1 );
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0 ){
      ಆದರೆ( pWalker->u.ai_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ]>=0 ){
        pWalker->e_ಸಂಕೇತ |= CKCNSTRNT_ಸ್ತಂಭ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      pWalker->e_ಸಂಕೇತ |= CKCNSTRNT_ROWID;
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,    /* The expression to be checked */
  ಪೂರ್ಣಾಂಕ *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  ಪೂರ್ಣಾಂಕ chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  memset(&w, 0, ಗಾತ್ರ(w));
  w.e_ಸಂಕೇತ = 0;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = check_Constraint_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು;
  w.u.ai_ಲಂಬಸಾಲು = aiChng;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( !chngRowid ){
    testcase( (w.e_ಸಂಕೇತ & CKCNSTRNT_ROWID)!=0 );
    w.e_ಸಂಕೇತ &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.e_ಸಂಕೇತ==0 );
  testcase( w.e_ಸಂಕೇತ==CKCNSTRNT_ಸ್ತಂಭ );
  testcase( w.e_ಸಂಕೇತ==CKCNSTRNT_ROWID );
  testcase( w.e_ಸಂಕೇತ==(CKCNSTRNT_ROWID|CKCNSTRNT_ಸ್ತಂಭ) );
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ!=0;
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸೂಚ್ಯಂಕ_List_ಪದ ಸೂಚ್ಯಂಕ_List_ಪದ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸೂಚ್ಯಂಕ_Iterator ಸೂಚ್ಯಂಕ_Iterator;
ರಚನೆ ಸೂಚ್ಯಂಕ_Iterator {
  ಪೂರ್ಣಾಂಕ eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  ಪೂರ್ಣಾಂಕ i;        /* Index of the current item from the list */
  ಒಕ್ಕೂಟ {
    ರಚನೆ {    /* Use this object for eType==0: A Index.pNext list */
      ಸೂಚ್ಯಂಕ *pIdx;   /* The current Index */
    } lx;     
    ರಚನೆ {    /* Use this object for eType==1; Array of IndexListTerm */
      ಪೂರ್ಣಾಂಕ nIdx;               /* Size of the array */
      ಸೂಚ್ಯಂಕ_List_ಪದ *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
ರಚನೆ ಸೂಚ್ಯಂಕ_List_ಪದ {
  ಸೂಚ್ಯಂಕ *p;  /* The index */
  ಪೂರ್ಣಾಂಕ ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
ಸ್ಥಿರ ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ_Iterator_ಮೊದಲ(ಸೂಚ್ಯಂಕ_Iterator *pIter, ಪೂರ್ಣಾಂಕ *pIx){
  ಪ್ರತಿಜ್ಞಾ( pIter->i==0 );
  ಆದರೆ( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    ಹಿಂತಿರುಗಿ pIter->u.ax.aIdx[0].p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pIx = 0;
    ಹಿಂತಿರುಗಿ pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
ಸ್ಥಿರ ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ_Iterator_ಮುಂದಿನ(ಸೂಚ್ಯಂಕ_Iterator *pIter, ಪೂರ್ಣಾಂಕ *pIx){
  ಆದರೆ( pIter->eType ){
    ಪೂರ್ಣಾಂಕ i = ++pIter->i;
    ಆದರೆ( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      ಹಿಂತಿರುಗಿ 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    ಹಿಂತಿರುಗಿ pIter->u.ax.aIdx[i].p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->p_ಮುಂದಿನ;
    ಹಿಂತಿರುಗಿ pIter->u.lx.pIdx;
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
ಶೂನ್ಯ sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  ಕೋಷ್ಟಕ *pTab,         /* The table being inserted or updated */
  ಪೂರ್ಣಾಂಕ *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur,        /* Canonical data cursor (main table or PK index) */
  ಪೂರ್ಣಾಂಕ iIdxCur,         /* First index cursor */
  ಪೂರ್ಣಾಂಕ reg_New_ದತ್ತಾಂಶ,      /* First register in a range holding values to insert */
  ಪೂರ್ಣಾಂಕ reg_Old_ದತ್ತಾಂಶ,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  ಪೂರ್ಣಾಂಕ ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  ಪೂರ್ಣಾಂಕ *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  ಪೂರ್ಣಾಂಕ *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  ಸೂಚ್ಯಂಕ *pIdx;         /* Pointer to one of the indices */
  ಸೂಚ್ಯಂಕ *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  ಪೂರ್ಣಾಂಕ i;               /* loop counter */
  ಪೂರ್ಣಾಂಕ ix;              /* Index loop counter */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;            /* Number of columns */
  ಪೂರ್ಣಾಂಕ onError;         /* Conflict resolution strategy */
  ಪೂರ್ಣಾಂಕ ನೋಡಿದ_Replace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  ಪೂರ್ಣಾಂಕ n_Pk_ಕ್ಷೇತ್ರ;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 b_Affinity_ಮುಗಿದ = 0;  /* True if the OP_Affinity operation has been run */
  ಪೂರ್ಣಾಂಕ upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  ಪೂರ್ಣಾಂಕ upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  ಪೂರ್ಣಾಂಕ ipk_ತುದಿ = 0;        /* Top of the IPK uniqueness check */
  ಪೂರ್ಣಾಂಕ ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  ಪೂರ್ಣಾಂಕ regTrigCnt;       /* Register used to count replace trigger invocations */
  ಪೂರ್ಣಾಂಕ addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  ಪೂರ್ಣಾಂಕ lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  ಪೂರ್ಣಾಂಕ nReplaceTrig = 0; /* Number of replace triggers coded */
  ಸೂಚ್ಯಂಕ_Iterator sIdxIter;  /* Index iterator */

  isUpdate = reg_Old_ದತ್ತಾಂಶ!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab) );  /* This table is not a VIEW */
  n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  ಆದರೆ( HasRowid(pTab) ){
    pPk = 0;
    n_Pk_ಕ್ಷೇತ್ರ = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
    n_Pk_ಕ್ಷೇತ್ರ = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     i_ದತ್ತಾಂಶ_Cur, iIdxCur, reg_New_ದತ್ತಾಂಶ, reg_Old_ದತ್ತಾಂಶ, pkChng));

  /* Test all NOT NULL constraints.
  */
  ಆದರೆ( pTab->tabFlags & TF_HasNotNull ){
    ಪೂರ್ಣಾಂಕ b2ndPass = 0;         /* True if currently running 2nd pass */
    ಪೂರ್ಣಾಂಕ n_ನೋಡಿದ_Replace = 0;     /* Number of ON CONFLICT REPLACE operations */
    ಪೂರ್ಣಾಂಕ nGenerated = 0;       /* Number of generated columns with NOT NULL */
    ಆಗ(1){  /* Make 2 passes over columns. Exit loop via "break" */
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        ಪೂರ್ಣಾಂಕ iReg;                        /* Register holding column value */
        ಸ್ತಂಭ *p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i];   /* The column to check for NOT NULL */
        ಪೂರ್ಣಾಂಕ isGenerated;                 /* non-zero if column is generated */
        onError = p_ಲಂಬಸಾಲು->notNull;
        ಆದರೆ( onError==OE_None ) ಮುಂದುವರಿ; /* No NOT NULL on this column */
        ಆದರೆ( i==pTab->iPKey ){
          ಮುಂದುವರಿ;        /* ROWID is never NULL */
        }
        isGenerated = p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED;
        ಆದರೆ( isGenerated && !b2ndPass ){
          nGenerated++;
          ಮುಂದುವರಿ;        /* Generated columns processed on 2nd pass */
        }
        ಆದರೆ( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( overrideError!=OE_Default ){
          onError = overrideError;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( onError==OE_Default ){
          onError = OE_Abort;
        }
        ಆದರೆ( onError==OE_Replace ){
          ಆದರೆ( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || p_ಲಂಬಸಾಲು->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
            testcase( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
            testcase( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪ್ರತಿಜ್ಞಾ( !isGenerated );
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( b2ndPass && !isGenerated ){
          ಮುಂದುವರಿ;
        }
        ಪ್ರತಿಜ್ಞಾ( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i) );
        iReg = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i) + reg_New_ದತ್ತಾಂಶ + 1;
        ಆಯ್ಕೆ( onError ){
          ಸ್ಥಿತಿ OE_Replace: {
            ಪೂರ್ಣಾಂಕ addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            ಪ್ರತಿಜ್ಞಾ( (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)==0 );
            n_ನೋಡಿದ_Replace++;
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(pParse,
               sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab, p_ಲಂಬಸಾಲು, pParse), iReg);
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
            ನಿಲ್ಲಿಸು;
          }
          ಸ್ಥಿತಿ OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          ಸ್ಥಿತಿ OE_Rollback:
          ಸ್ಥಿತಿ OE_Fail: {
            ಅಕ್ಷರ *z_ಸಂದೇಶ = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        p_ಲಂಬಸಾಲು->zCnName);
            testcase( z_ಸಂದೇಶ==0 && db->mallocFailed==0 );
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, z_ಸಂದೇಶ, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            ನಿಲ್ಲಿಸು;
          }
          ಪೂರ್ವನಿಯೋಜಿತ: {
            ಪ್ರತಿಜ್ಞಾ( onError==OE_Ignore );
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            ನಿಲ್ಲಿಸು;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      ಆದರೆ( nGenerated==0 && n_ನೋಡಿದ_Replace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( b2ndPass ) ನಿಲ್ಲಿಸು;  /* Never need more than 2 passes */
      b2ndPass = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
      ಆದರೆ( n_ನೋಡಿದ_Replace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, reg_New_ದತ್ತಾಂಶ+1, pTab);
      }
#ಆದರೆ_ಅಂತ್ಯ
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CHECK
  ಆದರೆ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(reg_New_ದತ್ತಾಂಶ+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    ಗಾಗಿ(i=0; i<pCheck->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪೂರ್ಣಾಂಕ allOk;
      ಅಭಿವ್ಯಕ್ತಿ *pCopy;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pCheck->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( aiChng
       && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(p_ಅಭಿವ್ಯಕ್ತಿ, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( b_Affinity_ಮುಗಿದ==0 ){
        sqlite3_ಕೋಷ್ಟಕ_Affinity(v, pTab, reg_New_ದತ್ತಾಂಶ+1);
        b_Affinity_ಮುಗಿದ = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      ಆದರೆ( !db->mallocFailed ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pCopy);
      ಆದರೆ( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಕ್ಷರ *zName = pCheck->a[i].zEName;
        ಪ್ರತಿಜ್ಞಾ( zName!=0 || pParse->db->mallocFailed );
        ಆದರೆ( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_CHECK) */

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
  sIdxIter.u.lx.pIdx = pTab->p_ಸೂಚ್ಯಂಕ;
  ಆದರೆ( pUpsert ){
    ಆದರೆ( pUpsert->p_Upsert_ಗುರಿ==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      ಪ್ರತಿಜ್ಞಾ( pUpsert->p_ಮುಂದಿನ_Upsert==0 );
      ಆದರೆ( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->p_ಸೂಚ್ಯಂಕ!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      ಪೂರ್ಣಾಂಕ nIdx, jj;
      u64 nByte;
      Upsert *p_ಪದ;
      u8 *b_ಬಳಸಿದ;
      ಗಾಗಿ(nIdx=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, nIdx++){
         ಪ್ರತಿಜ್ಞಾ( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (ಗಾತ್ರ(ಸೂಚ್ಯಂಕ_List_ಪದ)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      ಆದರೆ( sIdxIter.u.ax.aIdx==0 ) ಹಿಂತಿರುಗಿ; /* OOM */
      b_ಬಳಸಿದ = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->p_ಗಮ್ಯ_Free = sIdxIter.u.ax.aIdx;
      ಗಾಗಿ(i=0, p_ಪದ=pUpsert; p_ಪದ; p_ಪದ=p_ಪದ->p_ಮುಂದಿನ_Upsert){
        ಆದರೆ( p_ಪದ->p_Upsert_ಗುರಿ==0 ) ನಿಲ್ಲಿಸು;
        ಆದರೆ( p_ಪದ->pUpsertIdx==0 ) ಮುಂದುವರಿ;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->p_ಸೂಚ್ಯಂಕ;
        ಆಗ( ALWAYS(pIdx!=0) && pIdx!=p_ಪದ->pUpsertIdx ){
           pIdx = pIdx->p_ಮುಂದಿನ;
           jj++;
        }
        ಆದರೆ( b_ಬಳಸಿದ[jj] ) ಮುಂದುವರಿ; /* Duplicate ON CONFLICT clause ignored */
        b_ಬಳಸಿದ[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      ಗಾಗಿ(jj=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, jj++){
        ಆದರೆ( b_ಬಳಸಿದ[jj] ) ಮುಂದುವರಿ;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      ಪ್ರತಿಜ್ಞಾ( i==nIdx );
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
  ಆದರೆ( (db->flags & (SQLITE_RecTriggers|SQLITE_Foreign_ಕೀಲಿಗಳು))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    ಆದರೆ( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      ಆದರೆ( overrideError!=OE_Replace ){
        ಆದರೆ( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pkChng==0 || pPk || pTab->ಕೀಲಿ_Conf!=OE_Replace ){
          ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
            ಆದರೆ( pIdx->onError==OE_Replace ) ನಿಲ್ಲಿಸು;
          }
          ಆದರೆ( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    ಆದರೆ( regTrigCnt ){
      /* Replace triggers might exist.  Allocate the counter and
      ** initialize it to zero. */
      regTrigCnt = ++pParse->nMem;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regTrigCnt);
      VdbeComment((v, "trigger count"));
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  /* If rowid is changing, make sure the new rowid does not previously
  ** exist in the table.
  */
  ಆದರೆ( pkChng && pPk==0 ){
    ಪೂರ್ಣಾಂಕ addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->ಕೀಲಿ_Conf;
    ಆದರೆ( overrideError!=OE_Default ){
      onError = overrideError;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    ಆದರೆ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(pUpsert,0);
      ಆದರೆ( pUpsertClause!=0 ){
        ಆದರೆ( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }ಇಲ್ಲದಿದ್ದರೆ{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      ಆದರೆ( pUpsertClause!=pUpsert ){
        /* The first ON CONFLICT clause has a conflict target other than
        ** the IPK.  We have to jump ahead to that first ON CONFLICT clause
        ** and then come back here and deal with the IPK afterwards */
        upsertIpkDelay = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
      }
    }

    /* If the response to a rowid conflict is REPLACE but the response
    ** to some other UNIQUE constraint is FAIL or IGNORE, then we need
    ** to defer the running of the rowid conflict checking until after
    ** the UNIQUE constraints have run.
    */
    ಆದರೆ( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->p_ಸೂಚ್ಯಂಕ             /* There exist other constraints */
    ){
      ಆದರೆ( upsertIpkDelay ){
        ipk_ತುದಿ = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ipk_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    ಆದರೆ( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, reg_New_ದತ್ತಾಂಶ, addrRowidOk, reg_Old_ದತ್ತಾಂಶ);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, addrRowidOk, reg_New_ದತ್ತಾಂಶ);
    VdbeCoverage(v);

    ಆಯ್ಕೆ( onError ){
      ಪೂರ್ವನಿಯೋಜಿತ: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      ಸ್ಥಿತಿ OE_Rollback:
      ಸ್ಥಿತಿ OE_Abort:
      ಸ್ಥಿತಿ OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OE_Replace: {
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
        ಆದರೆ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3_Generate_ಅಡ್ಡಸಾಲು_Delete(pParse, pTab, pTrigger, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
                                   reg_New_ದತ್ತಾಂಶ, 1, 0, OE_Replace, 1, -1);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
          ಪ್ರತಿಜ್ಞಾ( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Delete, i_ದತ್ತಾಂಶ_Cur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */
          ಆದರೆ( pTab->p_ಸೂಚ್ಯಂಕ ){
            sqlite3MultiWrite(pParse);
            sqlite3_Generate_ಅಡ್ಡಸಾಲು_ಸೂಚ್ಯಂಕ_Delete(pParse, pTab, i_ದತ್ತಾಂಶ_Cur, iIdxCur,0,-1);
          }
        }
        ನೋಡಿದ_Replace = 1;
        ನಿಲ್ಲಿಸು;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
      ಸ್ಥಿತಿ OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, i_ದತ್ತಾಂಶ_Cur);
        /* no break */ deliberate_fall_through
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಸ್ಥಿತಿ OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ನಿಲ್ಲಿಸು;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    ಆದರೆ( ipk_ತುದಿ ){
      ipkBottom = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, ipk_ತುದಿ-1);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pUpsert && pUpsertClause!=pUpsert ){
      upsertIpkReturn = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
    }
  }

  /* Test all UNIQUE constraints by creating entries for each UNIQUE
  ** index and making sure that duplicate entries do not already exist.
  ** Compute the revised record entries for indices as we go.
  **
  ** This loop also handles the case of the PRIMARY KEY index for a
  ** WITHOUT ROWID table.
  */
  ಗಾಗಿ(pIdx = ಸೂಚ್ಯಂಕ_Iterator_ಮೊದಲ(&sIdxIter, &ix);
      pIdx;
      pIdx = ಸೂಚ್ಯಂಕ_Iterator_ಮುಂದಿನ(&sIdxIter, &ix)
  ){
    ಪೂರ್ಣಾಂಕ regIdx;          /* Range of registers holding content for pIdx */
    ಪೂರ್ಣಾಂಕ regR;            /* Range of registers holding conflicting PK */
    ಪೂರ್ಣಾಂಕ iThisCur;        /* Cursor for this UNIQUE index */
    ಪೂರ್ಣಾಂಕ addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    ಪೂರ್ಣಾಂಕ addrConflictCk;  /* First opcode in the conflict check logic */
    ಪೂರ್ಣಾಂಕ nConflictCk;     /* Number of opcodes in conflict check logic */

    ಆದರೆ( aRegIdx[ix]==0 ) ಮುಂದುವರಿ;  /* Skip indices that do not change */
    ಆದರೆ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(pUpsert, pIdx);
      ಆದರೆ( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    ಆದರೆ( b_Affinity_ಮುಗಿದ==0 ){
      sqlite3_ಕೋಷ್ಟಕ_Affinity(v, pTab, reg_New_ದತ್ತಾಂಶ+1);
      b_Affinity_ಮುಗಿದ = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    ಆದರೆ( pIdx->pPartIdxWhere ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(reg_New_ದತ್ತಾಂಶ+1);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False_Dup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    ಗಾಗಿ(i=0; i<pIdx->n_ಸ್ತಂಭ; i++){
      ಪೂರ್ಣಾಂಕ i_ಕ್ಷೇತ್ರ = pIdx->ai_ಸ್ತಂಭ[i];
      ಪೂರ್ಣಾಂಕ x;
      ಆದರೆ( i_ಕ್ಷೇತ್ರ==XN_ಅಭಿವ್ಯಕ್ತಿ ){
        pParse->iSelfTab = -(reg_New_ದತ್ತಾಂಶ+1);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(pParse, pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i_ಕ್ಷೇತ್ರ==XN_ROWID || i_ಕ್ಷೇತ್ರ==pTab->iPKey ){
        x = reg_New_ದತ್ತಾಂಶ;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }ಇಲ್ಲದಿದ್ದರೆ{
        testcase( sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಕ್ಷೇತ್ರ)!=i_ಕ್ಷೇತ್ರ );
        x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಕ್ಷೇತ್ರ) + reg_New_ದತ್ತಾಂಶ + 1;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->a_ಲಂಬಸಾಲು[i_ಕ್ಷೇತ್ರ].zCnName));
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regIdx, pIdx->n_ಸ್ತಂಭ, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NULL_TRIM
    ಆದರೆ( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3_ಗಣ_Make_ದಾಖಲೆ_P5(v, pIdx->p_ಕೋಷ್ಟಕ);
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->n_ಸ್ತಂಭ, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    ಆದರೆ( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ಮುಂದುವರಿ;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    ಆದರೆ( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ಮುಂದುವರಿ;  /* pIdx is not a UNIQUE index */
    }
    ಆದರೆ( overrideError!=OE_Default ){
      onError = overrideError;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    ಆದರೆ( pUpsertClause ){
      ಆದರೆ( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }ಇಲ್ಲದಿದ್ದರೆ{
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
    ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    ಆದರೆ( (ix==0 && pIdx->p_ಮುಂದಿನ==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ಮುಂದುವರಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_Pk_ಕ್ಷೇತ್ರ);
    ಆದರೆ( isUpdate || onError==OE_Replace ){
      ಆದರೆ( HasRowid(pTab) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        ಆದರೆ( isUpdate ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, regR, addrUniqueOk, reg_Old_ದತ್ತಾಂಶ);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        ಆದರೆ( pIdx!=pPk ){
          ಗಾಗಿ(i=0; i<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
            ಪ್ರತಿಜ್ಞಾ( pPk->ai_ಸ್ತಂಭ[i]>=0 );
            x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pIdx, pPk->ai_ಸ್ತಂಭ[i]);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->a_ಲಂಬಸಾಲು[pPk->ai_ಸ್ತಂಭ[i]].zCnName));
          }
        }
        ಆದರೆ( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          ಪೂರ್ಣಾಂಕ addrJump = sqlite3VdbeCurrentAddr(v)+pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
          ಪೂರ್ಣಾಂಕ op = OP_Ne;
          ಪೂರ್ಣಾಂಕ regCmp = (Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ? regIdx : regR);
 
          ಗಾಗಿ(i=0; i<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
            ಅಕ್ಷರ *p4 = (ಅಕ್ಷರ*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->ai_ಸ್ತಂಭ[i];
            ಪ್ರತಿಜ್ಞಾ( x>=0 );
            ಆದರೆ( i==(pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, x);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, op,
                reg_Old_ದತ್ತಾಂಶ+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    ಪ್ರತಿಜ್ಞಾ( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    ಪ್ರತಿಜ್ಞಾ( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    ಆಯ್ಕೆ( onError ){
      ಸ್ಥಿತಿ OE_Rollback:
      ಸ್ಥಿತಿ OE_Abort:
      ಸ್ಥಿತಿ OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        ನಿಲ್ಲಿಸು;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
      ಸ್ಥಿತಿ OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಸ್ಥಿತಿ OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಪ್ರತಿಜ್ಞಾ( onError==OE_Replace );
        ಆದರೆ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        ಆದರೆ( pTrigger && isUpdate ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Cursor_ಬೀಗ, i_ದತ್ತಾಂಶ_Cur);
        }
        sqlite3_Generate_ಅಡ್ಡಸಾಲು_Delete(pParse, pTab, pTrigger, i_ದತ್ತಾಂಶ_Cur, iIdxCur,
            regR, n_Pk_ಕ್ಷೇತ್ರ, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        ಆದರೆ( pTrigger && isUpdate ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_CursorUnlock, i_ದತ್ತಾಂಶ_Cur);
        }
        ನೋಡಿದ_Replace = 1;
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( regTrigCnt ){
      ಪೂರ್ಣಾಂಕ addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      ಆದರೆ( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      ಆಗ( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        ಆದರೆ( x.opcode!=OP_IdxRowid || isUpdate ){
          ಪೂರ್ಣಾಂಕ p2;      /* New P2 value for copied conflict check opcode */
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zP4;
          ಆದರೆ( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }ಇಲ್ಲದಿದ್ದರೆ{
            p2 = x.p2;
          }
          zP4 = x.p4type==P4_INT32 ? SQLITE_INT_ಗಮ್ಯ_PTR(x.p4.i) : x.p4.z;
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
          sqlite3VdbeChangeP5(v, x.p5);
          VdbeCoverageIf(v, p2!=x.p2);
        }
        nConflictCk--;
        addrConflictCk++;
      }
      /* If the retest fails, issue an abort */
      sqlite3UniqueConstraint(pParse, OE_Abort, pIdx);

      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrBypass); /* Terminate the recheck bypass */
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    ಆದರೆ( regR!=regIdx ) sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, regR, n_Pk_ಕ್ಷೇತ್ರ);
    ಆದರೆ( pUpsertClause
     && upsertIpkReturn
     && sqlite3_Upsert_ಮುಂದಿನ_Is_IPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  ಆದರೆ( ipk_ತುದಿ ){
    sqlite3VdbeGoto(v, ipk_ತುದಿ);
    VdbeComment((v, "Do IPK REPLACE"));
    ಪ್ರತಿಜ್ಞಾ( ipkBottom>0 );
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  ಪ್ರತಿಜ್ಞಾ( regTrigCnt!=0 || nReplaceTrig==0 );
  ಆದರೆ( nReplaceTrig ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    ಆದರೆ( !pPk ){
      ಆದರೆ( isUpdate ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, reg_New_ದತ್ತಾಂಶ, addrRecheck, reg_Old_ದತ್ತಾಂಶ);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, i_ದತ್ತಾಂಶ_Cur, addrRecheck, reg_New_ದತ್ತಾಂಶ);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  ಆದರೆ( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  ಆದರೆ( HasRowid(pTab) ){
    ಪೂರ್ಣಾಂಕ regRec = aRegIdx[ix];
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_New_ದತ್ತಾಂಶ+1, pTab->nNVCol, regRec);
    sqlite3_ಗಣ_Make_ದಾಖಲೆ_P5(v, pTab);
    ಆದರೆ( !b_Affinity_ಮುಗಿದ ){
      sqlite3_ಕೋಷ್ಟಕ_Affinity(v, pTab, 0);
    }
  }

  *pbMayReplace = ನೋಡಿದ_Replace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", ನೋಡಿದ_Replace));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
ಶೂನ್ಯ sqlite3_ಗಣ_Make_ದಾಖಲೆ_P5(Vdbe *v, ಕೋಷ್ಟಕ *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  ಆದರೆ( pTab->pSchema->file_format<2 ) ಹಿಂತಿರುಗಿ;

  ಗಾಗಿ(i=pTab->n_ಲಂಬಸಾಲು-1; i>0; i--){
    ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].iDflt!=0 ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags & COLFLAG_PRIMKEY ) ನಿಲ್ಲಿಸು;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Without_Rowid_Preupdate(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Table being updated */
  ಪೂರ್ಣಾಂಕ iCur,                       /* Cursor number for table */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ r = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
  ಪ್ರತಿಜ್ಞಾ( !HasRowid(pTab) );
  ಪ್ರತಿಜ್ಞಾ( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, r);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_ಸೇರಿಸು, iCur, reg_ದತ್ತಾಂಶ, r, (ಅಕ್ಷರ*)pTab, P4_ಕೋಷ್ಟಕ);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಸಂಕೇತ_Without_Rowid_Preupdate(a,b,c,d)
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
ಶೂನ್ಯ sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  ಕೋಷ್ಟಕ *pTab,        /* the table into which we are inserting */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur,       /* Cursor of the canonical data source */
  ಪೂರ್ಣಾಂಕ iIdxCur,        /* First index cursor */
  ಪೂರ್ಣಾಂಕ reg_New_ದತ್ತಾಂಶ,     /* Range of content */
  ಪೂರ್ಣಾಂಕ *aRegIdx,       /* Register used by each index.  0 for unused indices */
  ಪೂರ್ಣಾಂಕ update_flags,   /* True for UPDATE, False for INSERT */
  ಪೂರ್ಣಾಂಕ appendBias,     /* True if this is likely to be an append */
  ಪೂರ್ಣಾಂಕ use_Seek_ಫಲಿತಾಂಶ   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  ಸೂಚ್ಯಂಕ *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  ಪೂರ್ಣಾಂಕ i;              /* Loop counter */

  ಪ್ರತಿಜ್ಞಾ( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab) );  /* This table is not a VIEW */
  ಗಾಗಿ(i=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, i++){
    /* All REPLACE indexes are at the end of the list */
    ಪ್ರತಿಜ್ಞಾ( pIdx->onError!=OE_Replace
         || pIdx->p_ಮುಂದಿನ==0
         || pIdx->p_ಮುಂದಿನ->onError==OE_Replace );
    ಆದರೆ( aRegIdx[i]==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdx->pPartIdxWhere || (update_flags && pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (use_Seek_ಫಲಿತಾಂಶ ? OPFLAG_USESEEKRESULT : 0);
    ಆದರೆ( Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      ಆದರೆ( update_flags==0 ){
        ಸಂಕೇತ_Without_Rowid_Preupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು: pIdx->n_ಸ್ತಂಭ);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  ಆದರೆ( !HasRowid(pTab) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pParse->nested ){
    pik_flags = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  ಆದರೆ( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  ಆದರೆ( use_Seek_ಫಲಿತಾಂಶ ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, i_ದತ್ತಾಂಶ_Cur, aRegIdx[i], reg_New_ದತ್ತಾಂಶ);
  ಆದರೆ( !pParse->nested ){
    sqlite3VdbeAppendP4(v, pTab, P4_ಕೋಷ್ಟಕ);
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
ಪೂರ್ಣಾಂಕ sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(
  Parse *pParse,   /* Parsing context */
  ಕೋಷ್ಟಕ *pTab,     /* Table to be opened */
  ಪೂರ್ಣಾಂಕ op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  ಪೂರ್ಣಾಂಕ i_ತಳ,       /* Use this for the table cursor, if there is one */
  u8 *a_ಗಮ್ಯ_Open,     /* If not NULL: boolean for each table and index */
  ಪೂರ್ಣಾಂಕ *pi_ದತ್ತಾಂಶ_Cur,  /* Write the database source cursor number here */
  ಪೂರ್ಣಾಂಕ *piIdxCur    /* Write the first index cursor number here */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur;
  ಸೂಚ್ಯಂಕ *pIdx;
  Vdbe *v;

  ಪ್ರತಿಜ್ಞಾ( op==OP_OpenRead || op==OP_OpenWrite );
  ಪ್ರತಿಜ್ಞಾ( op==OP_OpenWrite || p5==0 );
  ಪ್ರತಿಜ್ಞಾ( pi_ದತ್ತಾಂಶ_Cur!=0 );
  ಪ್ರತಿಜ್ಞಾ( piIdxCur!=0 );
  ಆದರೆ( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *pi_ದತ್ತಾಂಶ_Cur = *piIdxCur = -999;
    ಹಿಂತಿರುಗಿ 0;
  }
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಆದರೆ( i_ತಳ<0 ) i_ತಳ = pParse->nTab;
  i_ದತ್ತಾಂಶ_Cur = i_ತಳ++;
  *pi_ದತ್ತಾಂಶ_Cur = i_ದತ್ತಾಂಶ_Cur;
  ಆದರೆ( HasRowid(pTab) && (a_ಗಮ್ಯ_Open==0 || a_ಗಮ್ಯ_Open[0]) ){
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, i_ದತ್ತಾಂಶ_Cur, iDb, pTab, op);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->db->noSharedCache==0 ){
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = i_ತಳ;
  ಗಾಗಿ(i=0, pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ, i++){
    ಪೂರ್ಣಾಂಕ iIdxCur = i_ತಳ++;
    ಪ್ರತಿಜ್ಞಾ( pIdx->pSchema==pTab->pSchema );
    ಆದರೆ( Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) && !HasRowid(pTab) ){
      *pi_ದತ್ತಾಂಶ_Cur = iIdxCur;
      p5 = 0;
    }
    ಆದರೆ( a_ಗಮ್ಯ_Open==0 || a_ಗಮ್ಯ_Open[i+1] ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  ಆದರೆ( i_ತಳ>pParse->nTab ) pParse->nTab = i_ತಳ;
  ಹಿಂತಿರುಗಿ i;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
ಪೂರ್ಣಾಂಕ sqlite3_xferopt_ಎಣಿಕೆ;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_XFER_OPT
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xfer_Compatible_ಸೂಚ್ಯಂಕ(ಸೂಚ್ಯಂಕ *pDest, ಸೂಚ್ಯಂಕ *pSrc){
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( pDest && pSrc );
  ಪ್ರತಿಜ್ಞಾ( pDest->p_ಕೋಷ್ಟಕ!=pSrc->p_ಕೋಷ್ಟಕ );
  ಆದರೆ( pDest->n_ಕೀಲಿ_ಲಂಬಸಾಲು!=pSrc->n_ಕೀಲಿ_ಲಂಬಸಾಲು || pDest->n_ಸ್ತಂಭ!=pSrc->n_ಸ್ತಂಭ ){
    ಹಿಂತಿರುಗಿ 0;   /* Different number of columns */
  }
  ಆದರೆ( pDest->onError!=pSrc->onError ){
    ಹಿಂತಿರುಗಿ 0;   /* Different conflict resolution strategies */
  }
  ಗಾಗಿ(i=0; i<pSrc->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( pSrc->ai_ಸ್ತಂಭ[i]!=pDest->ai_ಸ್ತಂಭ[i] ){
      ಹಿಂತಿರುಗಿ 0;   /* Different columns indexed */
    }
    ಆದರೆ( pSrc->ai_ಸ್ತಂಭ[i]==XN_ಅಭಿವ್ಯಕ್ತಿ ){
      ಪ್ರತಿಜ್ಞಾ( pSrc->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 && pDest->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pSrc->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ,
                             pDest->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, -1)!=0 ){
        ಹಿಂತಿರುಗಿ 0;   /* Different expressions in the index */
      }
    }
    ಆದರೆ( pSrc->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[i]!=pDest->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[i] ){
      ಹಿಂತಿರುಗಿ 0;   /* Different sort orders */
    }
    ಆದರೆ( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      ಹಿಂತಿರುಗಿ 0;   /* Different collating sequences */
    }
  }
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    ಹಿಂತಿರುಗಿ 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xferCheckRowid(Walker *pWalk, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ){
    pWalk->e_ಸಂಕೇತ = 1;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xferCompatibleCheck(ಕೋಷ್ಟಕ *pDest, ಕೋಷ್ಟಕ *pSrc){
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pSrc->pCheck,pDest->pCheck,-1) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( pDest->iPKey<0 ){
    Walker w;
    memset(&w, 0, ಗಾತ್ರ(w));
    w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = xferCheckRowid;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w,pDest->pCheck);
    ಆದರೆ( w.e_ಸಂಕೇತ ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xferOptimization(
  Parse *pParse,        /* Parser context */
  ಕೋಷ್ಟಕ *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  ಪೂರ್ಣಾಂಕ onError,          /* How to handle constraint errors */
  ಪೂರ್ಣಾಂಕ iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;                /* The result set of the SELECT */
  ಕೋಷ್ಟಕ *pSrc;                     /* The table in the FROM clause of SELECT */
  ಸೂಚ್ಯಂಕ *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  Src_ಅಂಶ *p_ಅಂಶ;                  /* An element of pSelect->pSrc */
  ಪೂರ್ಣಾಂಕ i;                           /* Loop counter */
  ಪೂರ್ಣಾಂಕ iDbSrc;                      /* The database of pSrc */
  ಪೂರ್ಣಾಂಕ iSrc, iDest;                 /* Cursors from source and destination */
  ಪೂರ್ಣಾಂಕ addr1, addr2;                /* Loop addresses */
  ಪೂರ್ಣಾಂಕ ಖಾಲಿಗಳು_Dest_Test = 0;           /* Address of test for empty pDest */
  ಪೂರ್ಣಾಂಕ ಖಾಲಿಗಳು_Src_Test = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  ಪೂರ್ಣಾಂಕ regAutoinc;                  /* Memory register used by AUTOINC */
  ಪೂರ್ಣಾಂಕ destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ, regRowid;           /* Registers holding data and rowid */

  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಆದರೆ( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    ಹಿಂತಿರುಗಿ 0;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ( IsVirtual(pDest) ){
    ಹಿಂತಿರುಗಿ 0;   /* tab1 must not be a virtual table */
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( onError==OE_Default ){
    ಆದರೆ( pDest->iPKey>=0 ) onError = pDest->ಕೀಲಿ_Conf;
    ಆದರೆ( onError==OE_Default ) onError = OE_Abort;
  }
  ಪ್ರತಿಜ್ಞಾ(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  ಆದರೆ( pSelect->pSrc->nSrc!=1 ){
    ಹಿಂತಿರುಗಿ 0;   /* FROM clause must have exactly one term */
  }
  ಆದರೆ( pSelect->pSrc->a[0].fg.isSubquery ){
    ಹಿಂತಿರುಗಿ 0;   /* FROM clause cannot contain a subquery */
  }
  ಆದರೆ( pSelect->pWhere ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not have a WHERE clause */
  }
  ಆದರೆ( pSelect->p_ಕ್ರಮ_By ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  ಆದರೆ( pSelect->p_ಗುಂಪು_By ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not have a GROUP BY clause */
  }
  ಆದರೆ( pSelect->p_ಎಲ್ಲೆ ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not have a LIMIT clause */
  }
  ಆದರೆ( pSelect->p_ಪೂರ್ವ ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not be a compound query */
  }
  ಆದರೆ( pSelect->selFlags & SF_Distinct ){
    ಹಿಂತಿರುಗಿ 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
  ಆದರೆ( pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1 ){
    ಹಿಂತಿರುಗಿ 0;   /* The result set must have exactly one column */
  }
  ಪ್ರತಿಜ್ಞಾ( pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ );
  ಆದರೆ( pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ASTERISK ){
    ಹಿಂತಿರುಗಿ 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  p_ಅಂಶ = pSelect->pSrc->a;
  pSrc = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, p_ಅಂಶ);
  ಆದರೆ( pSrc==0 ){
    ಹಿಂತಿರುಗಿ 0;   /* FROM clause does not contain a real table */
  }
  ಆದರೆ( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    ಹಿಂತಿರುಗಿ 0;   /* tab1 and tab2 may not be the same table */
  }
  ಆದರೆ( HasRowid(pDest)!=HasRowid(pSrc) ){
    ಹಿಂತಿರುಗಿ 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pSrc) ){
    ಹಿಂತಿರುಗಿ 0;   /* tab2 may not be a view or virtual table */
  }
  ಆದರೆ( pDest->n_ಲಂಬಸಾಲು!=pSrc->n_ಲಂಬಸಾಲು ){
    ಹಿಂತಿರುಗಿ 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  ಆದರೆ( pDest->iPKey!=pSrc->iPKey ){
    ಹಿಂತಿರುಗಿ 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  ಆದರೆ( (pDest->tabFlags & TF_Strict)!=0 ){
    ಆದರೆ( (pSrc->tabFlags & TF_Strict)==0 ){
      ಹಿಂತಿರುಗಿ 0;  /* Cannot feed from a non-strict into a strict table */
    }
    ಗಾಗಿ(i=0; i<pDest->n_ಲಂಬಸಾಲು; i++){
      ಚಿಹ್ನೆರಹಿತ eDestType = pDest->a_ಲಂಬಸಾಲು[i].eCType;
      ಚಿಹ್ನೆರಹಿತ eSrcType = pSrc->a_ಲಂಬಸಾಲು[i].eCType;
      ಆದರೆ( eDestType==COLTYPE_ANY ) ಮುಂದುವರಿ;
      ಆದರೆ( eDestType==eSrcType ) ಮುಂದುವರಿ;
      ಆದರೆ( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) ಮುಂದುವರಿ;
      ಆದರೆ( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) ಮುಂದುವರಿ;
      ಹಿಂತಿರುಗಿ 0; /* Incompatible types in source and destination */
    }
  }
  ಗಾಗಿ(i=0; i<pDest->n_ಲಂಬಸಾಲು; i++){
    ಸ್ತಂಭ *p_Dest_ಲಂಬಸಾಲು = &pDest->a_ಲಂಬಸಾಲು[i];
    ಸ್ತಂಭ *p_Src_ಲಂಬಸಾಲು = &pSrc->a_ಲಂಬಸಾಲು[i];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_HIDDEN_COLUMNS
    ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags | p_Src_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags) & COLFLAG_HIDDEN
    ){
      ಹಿಂತಿರುಗಿ 0;    /* Neither table may have __hidden__ columns */
    }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
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
    ಆದರೆ( (p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED) !=
        (p_Src_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED) ){
      ಹಿಂತಿರುಗಿ 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    ಆದರೆ( (p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0 ){
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,
             sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pSrc, p_Src_ಲಂಬಸಾಲು),
             sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pDest, p_Dest_ಲಂಬಸಾಲು), -1)!=0 ){
        testcase( p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL );
        testcase( p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_STORED );
        ಹಿಂತಿರುಗಿ 0;  /* Different generator expressions */
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p_Dest_ಲಂಬಸಾಲು->affinity!=p_Src_ಲಂಬಸಾಲು->affinity ){
      ಹಿಂತಿರುಗಿ 0;    /* Affinity must be the same on all columns */
    }
    ಆದರೆ( sqlite3_stricmp(sqlite3_ಸ್ತಂಭ_Coll(p_Dest_ಲಂಬಸಾಲು),
                        sqlite3_ಸ್ತಂಭ_Coll(p_Src_ಲಂಬಸಾಲು))!=0 ){
      ಹಿಂತಿರುಗಿ 0;    /* Collating sequence must be the same on all columns */
    }
    ಆದರೆ( p_Dest_ಲಂಬಸಾಲು->notNull && !p_Src_ಲಂಬಸಾಲು->notNull ){
      ಹಿಂತಿರುಗಿ 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    ಆದರೆ( (p_Dest_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)==0 && i>0 ){
      ಅಭಿವ್ಯಕ್ತಿ *p_Dest_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pDest, p_Dest_ಲಂಬಸಾಲು);
      ಅಭಿವ್ಯಕ್ತಿ *p_Src_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pSrc, p_Src_ಲಂಬಸಾಲು);
      ಪ್ರತಿಜ್ಞಾ( p_Dest_ಅಭಿವ್ಯಕ್ತಿ==0 || p_Dest_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಹರವು );
      ಪ್ರತಿಜ್ಞಾ( p_Dest_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_Dest_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_Src_ಅಭಿವ್ಯಕ್ತಿ==0 || p_Src_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಹರವು );
      ಪ್ರತಿಜ್ಞಾ( p_Src_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_Src_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಆದರೆ( (p_Dest_ಅಭಿವ್ಯಕ್ತಿ==0)!=(p_Src_ಅಭಿವ್ಯಕ್ತಿ==0)
       || (p_Dest_ಅಭಿವ್ಯಕ್ತಿ!=0 && strcmp(p_Dest_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,
                                       p_Src_ಅಭಿವ್ಯಕ್ತಿ->u.zToken)!=0)
      ){
        ಹಿಂತಿರುಗಿ 0;    /* Default values must be the same for all columns */
      }
    }
  }
  ಗಾಗಿ(pDestIdx=pDest->p_ಸೂಚ್ಯಂಕ; pDestIdx; pDestIdx=pDestIdx->p_ಮುಂದಿನ){
    ಆದರೆ( Is_Unique_ಸೂಚ್ಯಂಕ(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    ಗಾಗಿ(pSrcIdx=pSrc->p_ಸೂಚ್ಯಂಕ; pSrcIdx; pSrcIdx=pSrcIdx->p_ಮುಂದಿನ){
      ಆದರೆ( xfer_Compatible_ಸೂಚ್ಯಂಕ(pDestIdx, pSrcIdx) ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( pSrcIdx==0 ){
      ಹಿಂತಿರುಗಿ 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    ಆದರೆ( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      ಹಿಂತಿರುಗಿ 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CHECK
  ಆದರೆ( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    ಹಿಂತಿರುಗಿ 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pDest) );
  ಆದರೆ( (db->flags & SQLITE_Foreign_ಕೀಲಿಗಳು)!=0 && pDest->u.tab.pFKey!=0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( (db->flags & SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು)!=0 ){
    ಹಿಂತಿರುಗಿ 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( db->xAuth ){
    ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pSrc->pSchema);
    ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ಹಿಂತಿರುಗಿ 0;
    ಗಾಗಿ(i=0; i<pSrc->n_ಲಂಬಸಾಲು; i++){
      ಸ್ತಂಭ *p_Src_ಲಂಬಸಾಲು = &pSrc->a_ಲಂಬಸಾಲು[i];
      ಆದರೆ( sqlite3_Auth_Read_ಲಂಬಸಾಲು(pParse, pSrc->zName, p_Src_ಲಂಬಸಾಲು->zCnName, iDb) ){
        ಹಿಂತಿರುಗಿ 0;
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  sqlite3_xferopt_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
  iDbSrc = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  reg_ದತ್ತಾಂಶ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ದತ್ತಾಂಶ);
  regRowid = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
  sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  ಪ್ರತಿಜ್ಞಾ( HasRowid(pDest) || destHasUniqueIdx );
  ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->p_ಸೂಚ್ಯಂಕ!=0)          /* (1) */
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
    addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iDest, 0); VdbeCoverage(v);
    ಖಾಲಿಗಳು_Dest_Test = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
  }
  ಆದರೆ( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    ಖಾಲಿಗಳು_Src_Test = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ಆದರೆ( pDest->iPKey>=0 ){
      addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iSrc, regRowid);
      ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr2);
      }
      auto_Inc_ಹೆಜ್ಜೆ(pParse, regAutoinc, regRowid);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pDest->p_ಸೂಚ್ಯಂಕ==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iDest, regRowid);
    }ಇಲ್ಲದಿದ್ದರೆ{
      addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iSrc, regRowid);
      ಪ್ರತಿಜ್ಞಾ( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    ಆದರೆ( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Seek_ಕೊನೆ, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ, iSrc, reg_ದತ್ತಾಂಶ, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಅಡ್ಡಸಾಲು_Cell, iDest, iSrc, regRowid);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iDest, reg_ದತ್ತಾಂಶ, regRowid);
    ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ*)pDest, P4_ಕೋಷ್ಟಕ);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iSrc, addr1); VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, iDest, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  ಗಾಗಿ(pDestIdx=pDest->p_ಸೂಚ್ಯಂಕ; pDestIdx; pDestIdx=pDestIdx->p_ಮುಂದಿನ){
    u8 idxInsFlags = 0;
    ಗಾಗಿ(pSrcIdx=pSrc->p_ಸೂಚ್ಯಂಕ; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->p_ಮುಂದಿನ){
      ಆದರೆ( xfer_Compatible_ಸೂಚ್ಯಂಕ(pDestIdx, pSrcIdx) ) ನಿಲ್ಲಿಸು;
    }
    ಪ್ರತಿಜ್ಞಾ( pSrcIdx );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ಆದರೆ( db->mDbFlags & DBFLAG_Vacuum ){
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
      ಗಾಗಿ(i=0; i<pSrcIdx->n_ಸ್ತಂಭ; i++){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl = pSrcIdx->azColl[i];
        ಆದರೆ( sqlite3_stricmp(sqlite3_Str_ದ್ವಿಮಾನ, zColl) ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( i==pSrcIdx->n_ಸ್ತಂಭ ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Seek_ಕೊನೆ, iDest);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಅಡ್ಡಸಾಲು_Cell, iDest, iSrc);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    ಆದರೆ( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ, iSrc, reg_ದತ್ತಾಂಶ, 1);
      ಆದರೆ( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pDestIdx)
      ){
        ಸಂಕೇತ_Without_Rowid_Preupdate(pParse, pDest, iDest, reg_ದತ್ತಾಂಶ);
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Idx_ಸೇರಿಸು, iDest, reg_ದತ್ತಾಂಶ);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, iDest, 0);
  }
  ಆದರೆ( ಖಾಲಿಗಳು_Src_Test ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, ಖಾಲಿಗಳು_Src_Test);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regRowid);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ದತ್ತಾಂಶ);
  ಆದರೆ( ಖಾಲಿಗಳು_Dest_Test ){
    sqlite3_Autoincrement_ಕೊನೆ(pParse);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, ಖಾಲಿಗಳು_Dest_Test);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Close, iDest, 0);
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 1;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_XFER_OPT */
