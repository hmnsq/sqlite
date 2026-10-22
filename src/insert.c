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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
ਖਾਲੀ sqlite3_Open_ਸਾਰਣੀ(
  Parse *pParse,  /* Generate code into this VDBE */
  ਪੂਰਨ iCur,       /* The cursor number of the table */
  ਪੂਰਨ iDb,        /* The database index in sqlite3.aDb[] */
  ਸਾਰਣੀ *pTab,    /* The table to be opened */
  ਪੂਰਨ opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsVirtual(pTab) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  ਜੇ( !pParse->db->noSharedCache ){
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  ਜੇ( HasRowid(pTab) ){
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }ਨਹੀਂਤਾਂ{
    ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pPk);
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
ਸਥਿਰ SQLITE_NOINLINE ਅਚਲ ਅੱਖਰ *compute_ਸੂਚਕਾਂਕ_Aff_Str(sqlite3 *db, ਸੂਚਕਾਂਕ *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  ਪੂਰਨ n;
  ਸਾਰਣੀ *pTab = pIdx->p_ਸਾਰਣੀ;
  pIdx->z_ਖਾਨਾ_Aff = (ਅੱਖਰ *)sqlite3DbMallocRaw(0, pIdx->n_ਕਾਲਮ+1);
  ਜੇ( !pIdx->z_ਖਾਨਾ_Aff ){
    sqlite3OomFault(db);
    ਵਾਪਸ 0;
  }
  ਲਈ(n=0; n<pIdx->n_ਕਾਲਮ; n++){
    i16 x = pIdx->ai_ਕਾਲਮ[n];
    ਅੱਖਰ aff;
    ਜੇ( x>=0 ){
      aff = pTab->a_ਖਾਨਾ[x].affinity;
    }ਨਹੀਂਤਾਂ ਜੇ( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( x==XN_ਵਿਅੰਜਕ );
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->b_Has_ਵਿਅੰਜਕ );
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 );
      aff = sqlite3_ਵਿਅੰਜਕ_Affinity(pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[n].p_ਵਿਅੰਜਕ);
    }
    ਜੇ( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    ਜੇ( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->z_ਖਾਨਾ_Aff[n] = aff;
  }
  pIdx->z_ਖਾਨਾ_Aff[n] = 0;
  ਵਾਪਸ pIdx->z_ਖਾਨਾ_Aff;
}
ਅਚਲ ਅੱਖਰ *sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(sqlite3 *db, ਸੂਚਕਾਂਕ *pIdx){
  ਜੇ( !pIdx->z_ਖਾਨਾ_Aff ) ਵਾਪਸ compute_ਸੂਚਕਾਂਕ_Aff_Str(db, pIdx);
  ਵਾਪਸ pIdx->z_ਖਾਨਾ_Aff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
ਅੱਖਰ *sqlite3_ਸਾਰਣੀ_Affinity_Str(sqlite3 *db, ਅਚਲ ਸਾਰਣੀ *pTab){
  ਅੱਖਰ *z_ਖਾਨਾ_Aff;
  z_ਖਾਨਾ_Aff = (ਅੱਖਰ *)sqlite3DbMallocRaw(db, pTab->n_ਖਾਨਾ+1);
  ਜੇ( z_ਖਾਨਾ_Aff ){
    ਪੂਰਨ i, j;
    ਲਈ(i=j=0; i<pTab->n_ਖਾਨਾ; i++){
      ਜੇ( (pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)==0 ){
        z_ਖਾਨਾ_Aff[j++] = pTab->a_ਖਾਨਾ[i].affinity;
      }
    }
    ਕਰੋ{
      z_ਖਾਨਾ_Aff[j--] = 0;
    }ਜਦੋਂ( j>=0 && z_ਖਾਨਾ_Aff[j]<=SQLITE_AFF_BLOB );
  }
  ਵਾਪਸ z_ਖਾਨਾ_Aff; 
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
ਖਾਲੀ sqlite3_ਸਾਰਣੀ_Affinity(Vdbe *v, ਸਾਰਣੀ *pTab, ਪੂਰਨ iReg){
  ਪੂਰਨ i;
  ਅੱਖਰ *z_ਖਾਨਾ_Aff;
  ਜੇ( pTab->tabFlags & TF_Strict ){
    ਜੇ( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *pPrev;
      ਪੂਰਨ p3;
      sqlite3VdbeAppendP4(v, pTab, P4_ਸਾਰਣੀ);
      pPrev = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v);
      ਪ੍ਰਤਿਜ੍ਞਾ( pPrev!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pPrev->opcode==OP_Make_ਰਿਕਾਰਡ || sqlite3VdbeDb(v)->mallocFailed );
      pPrev->opcode = OP_TypeCheck;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, pPrev->p1, pPrev->p2, p3);
    }ਨਹੀਂਤਾਂ{
      /* Insert an isolated OP_Typecheck */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_ਸਾਰਣੀ);
    }
    ਵਾਪਸ;
  }
  z_ਖਾਨਾ_Aff = pTab->z_ਖਾਨਾ_Aff;
  ਜੇ( z_ਖਾਨਾ_Aff==0 ){
    z_ਖਾਨਾ_Aff = sqlite3_ਸਾਰਣੀ_Affinity_Str(0, pTab);
    ਜੇ( !z_ਖਾਨਾ_Aff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      ਵਾਪਸ;
    }
    pTab->z_ਖਾਨਾ_Aff = z_ਖਾਨਾ_Aff;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਖਾਨਾ_Aff!=0 );
  i = sqlite3Strlen30NN(z_ਖਾਨਾ_Aff);
  ਜੇ( i ){
    ਜੇ( iReg ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, iReg, i, 0, z_ਖਾਨਾ_Aff, i);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v)->opcode==OP_Make_ਰਿਕਾਰਡ
              || sqlite3VdbeDb(v)->mallocFailed );
      sqlite3VdbeChangeP4(v, -1, z_ਖਾਨਾ_Aff, i);
    }
  }
}

/*
** Return non-zero if the table pTab in database iDb or any of its indices
** have been opened at any point in the VDBE program. This is used to see if
** a statement of the form  "INSERT INTO <iDb, pTab> SELECT ..." can
** run without using a temporary table for the results of the SELECT.
*/
ਸਥਿਰ ਪੂਰਨ reads_ਸਾਰਣੀ(Parse *p, ਪੂਰਨ iDb, ਸਾਰਣੀ *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  ਪੂਰਨ i;
  ਪੂਰਨ i_ਸਿਰਾ = sqlite3VdbeCurrentAddr(v);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#ਜੇ_ਅੰਤ

  ਲਈ(i=1; i<i_ਸਿਰਾ; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp!=0 );
    ਜੇ( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;
      Pgno tnum = pOp->p2;
      ਜੇ( tnum==pTab->tnum ){
        ਵਾਪਸ 1;
      }
      ਲਈ(p_ਸੂਚਕਾਂਕ=pTab->p_ਸੂਚਕਾਂਕ; p_ਸੂਚਕਾਂਕ; p_ਸੂਚਕਾਂਕ=p_ਸੂਚਕਾਂਕ->p_ਅਗਲਾ){
        ਜੇ( tnum==p_ਸੂਚਕਾਂਕ->tnum ){
          ਵਾਪਸ 1;
        }
      }
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਜੇ( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4.pVtab!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_VTAB );
      ਵਾਪਸ 1;
    }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_ਕਾਲਮ_Flag_Union(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ < pWalker->u.pTab->n_ਖਾਨਾ );
    pWalker->e_ਕੋਡ |= pWalker->u.pTab->a_ਖਾਨਾ[p_ਵਿਅੰਜਕ->i_ਕਾਲਮ].ਖਾਨਾ_Flags;
  }
  ਵਾਪਸ WRC_Continue;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
ਖਾਲੀ sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  ਪੂਰਨ iRegStore,    /* Register holding the first column */
  ਸਾਰਣੀ *pTab       /* The table */
){
  ਪੂਰਨ i;
  Walker w;
  ਕਾਲਮ *pRedo;
  ਪੂਰਨ e_ਤਰੱਕੀ;
  VdbeOp *pOp;

  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3_ਸਾਰਣੀ_Affinity(pParse->pVdbe, pTab, iRegStore);
  ਜੇ( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(pParse->pVdbe);
    ਜੇ( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      ਪੂਰਨ ii, jj;
      ਅੱਖਰ *zP4 = pOp->p4.z;
      ਪ੍ਰਤਿਜ੍ਞਾ( zP4!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_DYNAMIC );
      ਲਈ(ii=jj=0; zP4[jj]; ii++){
        ਜੇ( pTab->a_ਖਾਨਾ[ii].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL ){
          ਜਾਰੀ;
        }
        ਜੇ( pTab->a_ਖਾਨਾ[ii].ਖਾਨਾ_Flags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( pOp->opcode==OP_TypeCheck ){
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
  ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
    ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
      testcase( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
      testcase( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_STORED );
      pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.x_ਵਿਅੰਜਕ_Callback = ਵਿਅੰਜਕ_ਕਾਲਮ_Flag_Union;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  ਕਰੋ{
    e_ਤਰੱਕੀ = 0;
    pRedo = 0;
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      ਕਾਲਮ *p_ਖਾਨਾ = pTab->a_ਖਾਨਾ + i;
      ਜੇ( (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_NOTAVAIL)!=0 ){
        ਪੂਰਨ x;
        p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_BUSY;
        w.e_ਕੋਡ = 0;
        sqlite3_Walk_ਵਿਅੰਜਕ(&w, sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pTab, p_ਖਾਨਾ));
        p_ਖਾਨਾ->ਖਾਨਾ_Flags &= ~COLFLAG_BUSY;
        ਜੇ( w.e_ਕੋਡ & COLFLAG_NOTAVAIL ){
          pRedo = p_ਖਾਨਾ;
          ਜਾਰੀ;
        }
        e_ਤਰੱਕੀ = 1;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED );
        x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i) + iRegStore;
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Generated_ਕਾਲਮ(pParse, pTab, p_ਖਾਨਾ, x);
        p_ਖਾਨਾ->ਖਾਨਾ_Flags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }ਜਦੋਂ( pRedo && e_ਤਰੱਕੀ );
  ਜੇ( pRedo ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_GENERATED_COLUMNS */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINCREMENT
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
ਸਥਿਰ ਪੂਰਨ autoIncBegin(
  Parse *pParse,      /* Parsing context */
  ਪੂਰਨ iDb,            /* Index of the database holding pTab */
  ਸਾਰਣੀ *pTab         /* The table we are writing to */
){
  ਪੂਰਨ memId = 0;      /* Register holding maximum rowid */
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->aDb[iDb].pSchema!=0 );
  ਜੇ( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    ਸਾਰਣੀ *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    ਜੇ( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->n_ਖਾਨਾ!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      ਵਾਪਸ 0;
    }

    ਜੇ( pToplevel->ਵਰਤਦਾ_Ainc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    ਜਦੋਂ( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->p_ਅਗਲਾ; }
    ਜੇ( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, ਆਕਾਰ(*pInfo));
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      ਜੇ( pParse->db->mallocFailed ) ਵਾਪਸ 0;
      pInfo->p_ਅਗਲਾ = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pToplevel->ਵਰਤਦਾ_Ainc = 1;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;                  /* Register to hold name of table */
      pInfo->regCtr = ++pToplevel->nMem;  /* Max rowid register */
      pToplevel->nMem +=2;       /* Rowid in sqlite_sequence + orig max val */
    }
    memId = pInfo->regCtr;
  }
  ਵਾਪਸ memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
ਖਾਲੀ sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  ਪੂਰਨ memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pTriggerTab==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3IsToplevel(pParse) );

  ਪ੍ਰਤਿਜ੍ਞਾ( v );   /* We failed long ago if this is not so */
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->ਵਰਤਦਾ_Ainc );
  ਲਈ(p = pParse->pAinc; p; p = p->p_ਅਗਲਾ){
    ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
    ਸਥਿਰ ਅਚਲ VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_ਕਾਲਮ,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_ਕਾਲਮ,  0,  1, 0},
      /* 6  */ {OP_ਜੋੜੋ_Imm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_ਅਗਲਾ,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Open_ਸਾਰਣੀ(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3_Vdbe_ਲੱਦੋ_String(v, memId-1, p->pTab->zName);
    aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(autoInc), autoInc, iLn);
    ਜੇ( aOp==0 ) ਰੋਕੋ;
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
    ਜੇ( pParse->nTab==0 ) pParse->nTab = 1;
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
ਸਥਿਰ ਖਾਲੀ auto_Inc_ਕਦਮ(Parse *pParse, ਪੂਰਨ memId, ਪੂਰਨ regRowid){
  ਜੇ( memId>0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(pParse->pVdbe, OP_MemMax, memId, regRowid);
  }
}

/*
** This routine generates the code needed to write autoincrement
** maximum rowid values back into the sqlite_sequence register.
** Every statement that might do an INSERT into an autoincrement
** table (either directly or through triggers) needs to call this
** routine just before the "exit" code.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ auto_Increment_ਸਿਰਾ(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->ਵਰਤਦਾ_Ainc );
  ਲਈ(p = pParse->pAinc; p; p = p->p_ਅਗਲਾ){
    ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
    ਸਥਿਰ ਅਚਲ VdbeOpList auto_Inc_ਸਿਰਾ[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_Make_ਰਿਕਾਰਡ,  0, 2, 0},
      /* 3 */ {OP_ਪਾਓ,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    ਪੂਰਨ iRec;
    ਪੂਰਨ memId = p->regCtr;

    iRec = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3_Open_ਸਾਰਣੀ(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(auto_Inc_ਸਿਰਾ), auto_Inc_ਸਿਰਾ, iLn);
    ਜੇ( aOp==0 ) ਰੋਕੋ;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, iRec);
  }
}
ਖਾਲੀ sqlite3_Autoincrement_ਸਿਰਾ(Parse *pParse){
  ਜੇ( pParse->ਵਰਤਦਾ_Ainc ) auto_Increment_ਸਿਰਾ(pParse);
}
#ਨਹੀਂਤਾਂ
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define auto_Inc_ਕਦਮ(A,B,C)
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
ਖਾਲੀ sqlite3_Multi_ਮੁੱਲਾਂ_ਸਿਰਾ(Parse *pParse, Select *pVal){
  ਜੇ( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pVal->pSrc->a[0];
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਵਸਤੂ->fg.isSubquery && p_ਵਸਤੂ->u4.pSubq!=0) || pParse->nErr );
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery ){
      sqlite3_Vdbe_ਸਿਰਾ_Coroutine(pParse->pVdbe, p_ਵਸਤੂ->u4.pSubq->regReturn);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(pParse->pVdbe, p_ਵਸਤੂ->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_List_Is_No_Affinity(Parse *pParse, ਵਿਅੰਜਕ_List *p_ਪੰਕਤੀ){
  ਪੂਰਨ ii;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_List_Is_Constant(pParse, p_ਪੰਕਤੀ, 0)==0 ) ਵਾਪਸ 0;
  ਲਈ(ii=0; ii<p_ਪੰਕਤੀ->n_ਵਿਅੰਜਕ; ii++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪੰਕਤੀ->a[ii].p_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op!=TK_RAISE );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ==0 );
    ਜੇ( 0!=sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ) ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;

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
Select *sqlite3_Multi_ਮੁੱਲਾਂ(Parse *pParse, Select *p_ਖੱਬਾ, ਵਿਅੰਜਕ_List *p_ਪੰਕਤੀ){

  ਜੇ( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3_ਵਿਅੰਜਕ_List_Is_Constant(pParse, p_ਪੰਕਤੀ, 1)==0   /* condition (c) above */
   || (p_ਖੱਬਾ->pSrc->nSrc==0 &&
       ਵਿਅੰਜਕ_List_Is_No_Affinity(pParse,p_ਖੱਬਾ->pEList)==0) /* condition (d) above */
   || ਵਿੱਚ_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    ਪੂਰਨ f = SF_ਮੁੱਲਾਂ | SF_Multi_ਮੁੱਲ;
    ਜੇ( p_ਖੱਬਾ->pSrc->nSrc ){
      sqlite3_Multi_ਮੁੱਲਾਂ_ਸਿਰਾ(pParse, p_ਖੱਬਾ);
      f = SF_ਮੁੱਲਾਂ;
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਖੱਬਾ->p_ਪੂਰਵ ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & p_ਖੱਬਾ->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, p_ਪੰਕਤੀ, 0, 0, 0, 0, 0, f, 0);
    p_ਖੱਬਾ->selFlags &= ~(u32)SF_Multi_ਮੁੱਲ;
    ਜੇ( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->p_ਪੂਰਵ = p_ਖੱਬਾ;
      p_ਖੱਬਾ = pSelect;
    }
  }ਨਹੀਂਤਾਂ{
    Src_ਵਸਤੂ *p = 0;               /* SrcItem that reads from co-routine */

    ਜੇ( p_ਖੱਬਾ->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      ਜੇ( (pParse->db->mDbFlags & DBFLAG_Schema_ਗਿਆਤ_Ok)==0 ){
        sqlite3ReadSchema(pParse);
      }

      ਜੇ( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->p_ਪੂਰਵ = p_ਖੱਬਾ->p_ਪੂਰਵ;
        pRet->op = p_ਖੱਬਾ->op;
        ਜੇ( pRet->p_ਪੂਰਵ ) pRet->selFlags |= SF_ਮੁੱਲਾਂ;
        p_ਖੱਬਾ->p_ਪੂਰਵ = 0;
        p_ਖੱਬਾ->op = TK_SELECT;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->p_ਅਗਲਾ==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pRet->p_ਅਗਲਾ==0 );
        p = &pRet->pSrc->a[0];
        p->fg.ਰਾਹੀਂ_Coroutine = 1;
        p->iCursor = -1;
        ਪ੍ਰਤਿਜ੍ਞਾ( !p->fg.is_ਸੂਚਕਾਂਕਿਤ_By && !p->fg.isTabFunc );
        p->u1.n_ਪੰਕਤੀ = 2;
        ਜੇ( sqlite3_Src_ਵਸਤੂ_Attach_Subquery(pParse, p, p_ਖੱਬਾ, 0) ){
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine,
                            pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);

          /* Allocate registers for the output of the co-routine. Do so so
          ** that there are two unused registers immediately before those
          ** used by the co-routine. This allows the code in sqlite3Insert()
          ** to use these registers directly, instead of copying the output
          ** of the co-routine to a separate array for processing.  */
          dest.iSdst = pParse->nMem + 3; 
          dest.nSdst = p_ਖੱਬਾ->pEList->n_ਵਿਅੰਜਕ;
          pParse->nMem += 2 + dest.nSdst;

          p_ਖੱਬਾ->selFlags |= SF_Multi_ਮੁੱਲ;
          sqlite3Select(pParse, p_ਖੱਬਾ, &dest);
          pSubq->reg_ਨਤੀਜਾ = dest.iSdst;
          ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr || dest.iSdst>0 );
        }
        p_ਖੱਬਾ = pRet;
      }
    }ਨਹੀਂਤਾਂ{
      p = &p_ਖੱਬਾ->pSrc->a[0];
      ਪ੍ਰਤਿਜ੍ਞਾ( !p->fg.isTabFunc && !p->fg.is_ਸੂਚਕਾਂਕਿਤ_By );
      p->u1.n_ਪੰਕਤੀ++;
    }
  
    ਜੇ( pParse->nErr==0 ){
      Subquery *pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect->pEList!=0 );
      ਜੇ( pSubq->pSelect->pEList->n_ਵਿਅੰਜਕ!=p_ਪੰਕਤੀ->n_ਵਿਅੰਜਕ ){
        sqlite3_Select_Wrong_ਅੰਕ_Terms_Error(pParse, pSubq->pSelect);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, p_ਪੰਕਤੀ, pSubq->reg_ਨਤੀਜਾ, 0, 0);
        sqlite3_Vdbe_ਜੋੜੋ_Op1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3_ਵਿਅੰਜਕ_List_Delete(pParse->db, p_ਪੰਕਤੀ);
  }

  ਵਾਪਸ p_ਖੱਬਾ;
}

/* Forward declaration */
ਸਥਿਰ ਪੂਰਨ xferOptimization(
  Parse *pParse,        /* Parser context */
  ਸਾਰਣੀ *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  ਪੂਰਨ onError,          /* How to handle constraint errors */
  ਪੂਰਨ iDbDest           /* The database of pDest */
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
ਖਾਲੀ sqlite3_ਪਾਓ(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *p_ਕਾਲਮ,      /* Column names corresponding to IDLIST, or NULL. */
  ਪੂਰਨ onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  ਸਾਰਣੀ *pTab;          /* The table to insert into.  aka TABLE */
  ਪੂਰਨ i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  ਸੂਚਕਾਂਕ *pIdx;          /* For looping over indices of the table */
  ਪੂਰਨ n_ਕਾਲਮ;          /* Number of columns in the data */
  ਪੂਰਨ nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur = 0;     /* VDBE cursor that is the main data repository */
  ਪੂਰਨ iIdxCur = 0;      /* First index cursor */
  ਪੂਰਨ ipk_ਕਾਲਮ = -1;   /* Column that is the INTEGER PRIMARY KEY */
  ਪੂਰਨ ਸਿਰਾ_ਦਾ_Loop;        /* Label for the end of the insertion loop */
  ਪੂਰਨ srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  ਪੂਰਨ addr_Ins_ਸਿਖਰ = 0;   /* Jump to label "D" */
  ਪੂਰਨ addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  ਪੂਰਨ iDb;              /* Index of database holding TABLE */
  u8 use_ਆਰਜ਼ੀ_ਸਾਰਣੀ = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ;    /* True if IDLIST is in table order */
  ਵਿਅੰਜਕ_List *pList = 0;  /* List of VALUES() to be inserted  */
  ਪੂਰਨ iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  ਪੂਰਨ regFromSelect = 0;/* Base register for data coming from SELECT */
  ਪੂਰਨ regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  ਪੂਰਨ reg_ਪੰਕਤੀ_ਗਿਣਤੀ = 0;  /* Memory cell used for the row counter */
  ਪੂਰਨ regIns;           /* Block of regs holding rowid+data being inserted */
  ਪੂਰਨ regRowid;         /* registers holding insert rowid */
  ਪੂਰਨ reg_ਅੰਕੜੇ;          /* register holding first column to insert */
  ਪੂਰਨ *aRegIdx = 0;     /* One register allocated to each index */
  ਪੂਰਨ *a_Tab_ਖਾਨਾ_Map = 0;  /* Mapping from pTab columns to pCol entries */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਪੂਰਨ isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  ਪੂਰਨ tmask;                  /* Mask of trigger times */
#ਜੇ_ਅੰਤ

  db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
  ਜੇ( pParse->nErr ){
    ਜਾਓ ਪਾਓ_cleanup;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  ਜੇ( pSelect && (pSelect->selFlags & SF_ਮੁੱਲਾਂ)!=0 && pSelect->p_ਪੂਰਵ==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  ਜੇ( pTab==0 ){
    ਜਾਓ ਪਾਓ_cleanup;
  }
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb<db->nDb );
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ਪਾਓ, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    ਜਾਓ ਪਾਓ_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_ਪਾਓ, 0, &tmask);
  isView = IsView(pTab);
#ਨਹੀਂਤਾਂ
# define pTrigger 0
# define tmask 0
# define isView 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x10000 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(0, "In sqlite3Insert() at %s:%d", __FILE__, __LINE__);
    sqlite3_ਰੁੱਖ_View_ਪਾਓ(pParse->pWith, pTabList, p_ਕਾਲਮ, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#ਜੇ_ਅੰਤ

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  ਜੇ( sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab) ){
    ਜਾਓ ਪਾਓ_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  ਜੇ( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    ਜਾਓ ਪਾਓ_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  ਜੇ( v==0 ) ਜਾਓ ਪਾਓ_cleanup;
  ਜੇ( pParse->nested==0 ) sqlite3_Vdbe_ਗਿਣਤੀ_Changes(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  ਜੇ( p_ਕਾਲਮ==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !pTrigger );
    ਪ੍ਰਤਿਜ੍ਞਾ( pList==0 );
    ਜਾਓ ਪਾਓ_ਸਿਰਾ;
  }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->n_ਖਾਨਾ + 1;
  ਜੇ( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  reg_ਅੰਕੜੇ = regRowid+1;

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
  b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ = (pTab->tabFlags & (TF_OOOHidden|TF_HasStored))==0;
  ਜੇ( p_ਕਾਲਮ ){
    a_Tab_ਖਾਨਾ_Map = sqlite3DbMallocZero(db, pTab->n_ਖਾਨਾ*ਆਕਾਰ(ਪੂਰਨ));
    ਜੇ( a_Tab_ਖਾਨਾ_Map==0 ) ਜਾਓ ਪਾਓ_cleanup;
    ਲਈ(i=0; i<p_ਕਾਲਮ->nId; i++){
      j = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, p_ਕਾਲਮ->a[i].zName);
      ਜੇ( j>=0 ){
        ਜੇ( a_Tab_ਖਾਨਾ_Map[j]==0 ) a_Tab_ਖਾਨਾ_Map[j] = i+1;
        ਜੇ( i!=j ) b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ = 0;
        ਜੇ( j==pTab->iPKey ){
          ipk_ਕਾਲਮ = i;  ਪ੍ਰਤਿਜ੍ਞਾ( !withoutRowid );
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
        ਜੇ( pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->a_ਖਾਨਾ[j].zCnName);
          ਜਾਓ ਪਾਓ_cleanup;
        }
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ{
        ਜੇ( sqlite3IsRowid(p_ਕਾਲਮ->a[i].zName) && !withoutRowid ){
          ipk_ਕਾਲਮ = i;
          b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ = 0;
        }ਨਹੀਂਤਾਂ{
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "table %S has no column named %s",
              pTabList->a, p_ਕਾਲਮ->a[i].zName);
          pParse->checkSchema = 1;
          ਜਾਓ ਪਾਓ_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  ਜੇ( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    ਪੂਰਨ rc;             /* Result code */

    ਜੇ( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.ਰਾਹੀਂ_Coroutine 
     && pSelect->p_ਪੂਰਵ==0
    ){
      Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isSubquery );
      pSubq = p_ਵਸਤੂ->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->reg_ਨਤੀਜਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect->pEList!=0 );
      n_ਕਾਲਮ = pSubq->pSelect->pEList->n_ਵਿਅੰਜਕ;
      ExplainQueryPlan((pParse, 0, "SCAN %S", p_ਵਸਤੂ));
      ਜੇ( b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ && n_ਕਾਲਮ==pTab->n_ਖਾਨਾ ){
        reg_ਅੰਕੜੇ = regFromSelect;
        regRowid = reg_ਅੰਕੜੇ - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ addr_ਸਿਖਰ;        /* Top of the co-routine */
      ਪੂਰਨ regYield = ++pParse->nMem;
      addr_ਸਿਖਰ = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, regYield, 0, addr_ਸਿਖਰ);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = b_Id_List_ਵਿੱਚ_ਕ੍ਰਮ ? reg_ਅੰਕੜੇ : 0;
      dest.nSdst = pTab->n_ਖਾਨਾ;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->pParse==pParse );
      ਜੇ( rc || pParse->nErr ) ਜਾਓ ਪਾਓ_cleanup;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
      sqlite3_Vdbe_ਸਿਰਾ_Coroutine(v, regYield);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ - 1);                       /* label B: */
      ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->pEList );
      n_ਕਾਲਮ = pSelect->pEList->n_ਵਿਅੰਜਕ;
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
    ਜੇ( pTrigger || reads_ਸਾਰਣੀ(pParse, iDb, pTab) ){
      use_ਆਰਜ਼ੀ_ਸਾਰਣੀ = 1;
    }

    ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
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
      ਪੂਰਨ regRec;          /* Register to hold packed record */
      ਪੂਰਨ reg_ਆਰਜ਼ੀ_Rowid;    /* Register to hold temp table ROWID */
      ਪੂਰਨ addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      reg_ਆਰਜ਼ੀ_Rowid = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, srcTab, n_ਕਾਲਮ);
      addrL = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regFromSelect, n_ਕਾਲਮ, regRec);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, srcTab, reg_ਆਰਜ਼ੀ_Rowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, srcTab, regRec, reg_ਆਰਜ਼ੀ_Rowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrL);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regRec);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਆਰਜ਼ੀ_Rowid);
    }
  }ਨਹੀਂਤਾਂ{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    memset(&sNC, 0, ਆਕਾਰ(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    ਪ੍ਰਤਿਜ੍ਞਾ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ==0 );
    ਜੇ( pList ){
      n_ਕਾਲਮ = pList->n_ਵਿਅੰਜਕ;
      ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pList) ){
        ਜਾਓ ਪਾਓ_cleanup;
      }
    }ਨਹੀਂਤਾਂ{
      n_ਕਾਲਮ = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  ਜੇ( p_ਕਾਲਮ==0 && n_ਕਾਲਮ>0 ){
    ipk_ਕਾਲਮ = pTab->iPKey;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
    ਜੇ( ipk_ਕਾਲਮ>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      ਲਈ(i=ipk_ਕਾਲਮ-1; i>=0; i--){
        ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_GENERATED ){
          testcase( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
          testcase( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_STORED );
          ipk_ਕਾਲਮ--;
        }
      }
    }
#ਜੇ_ਅੰਤ

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( TF_HasHidden==COLFLAG_HIDDEN );
    ਪ੍ਰਤਿਜ੍ਞਾ( TF_HasGenerated==COLFLAG_GENERATED );
    ਪ੍ਰਤਿਜ੍ਞਾ( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    ਜੇ( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
        ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    ਜੇ( n_ਕਾਲਮ!=(pTab->n_ਖਾਨਾ-nHidden) ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->n_ਖਾਨਾ-nHidden, n_ਕਾਲਮ);
     ਜਾਓ ਪਾਓ_cleanup;
    }
  }
  ਜੇ( p_ਕਾਲਮ!=0 && n_ਕਾਲਮ!=p_ਕਾਲਮ->nId ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%d values for %d columns", n_ਕਾਲਮ, p_ਕਾਲਮ->nId);
    ਜਾਓ ਪਾਓ_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  ਜੇ( (db->flags & SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    reg_ਪੰਕਤੀ_ਗਿਣਤੀ = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, reg_ਪੰਕਤੀ_ਗਿਣਤੀ);
  }

  /* If this is not a view, open the table and and all indices */
  ਜੇ( !isView ){
    ਪੂਰਨ nIdx;
    nIdx = sqlite3_Open_ਸਾਰਣੀ_And_Indices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &i_ਅੰਕੜੇ_Cur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਪੂਰਨ)*(nIdx+2));
    ਜੇ( aRegIdx==0 ){
      ਜਾਓ ਪਾਓ_cleanup;
    }
    ਲਈ(i=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; i<nIdx; pIdx=pIdx->p_ਅਗਲਾ, i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->n_ਕਾਲਮ;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
  ਜੇ( pUpsert ){
    Upsert *pNx;
    ਜੇ( IsVirtual(pTab) ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      ਜਾਓ ਪਾਓ_cleanup;
    }
    ਜੇ( IsView(pTab) ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot UPSERT a view");
      ਜਾਓ ਪਾਓ_cleanup;
    }
    ਜੇ( sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ) ){
      ਜਾਓ ਪਾਓ_cleanup;
    }
    pTabList->a[0].iCursor = i_ਅੰਕੜੇ_Cur;
    pNx = pUpsert;
    ਕਰੋ{
      pNx->pUpsertSrc = pTabList;
      pNx->reg_ਅੰਕੜੇ = reg_ਅੰਕੜੇ;
      pNx->i_ਅੰਕੜੇ_Cur = i_ਅੰਕੜੇ_Cur;
      pNx->iIdxCur = iIdxCur;
      ਜੇ( pNx->p_Upsert_ਨਿਸ਼ਾਨਾ ){
        ਜੇ( sqlite3_Upsert_Analyze_ਨਿਸ਼ਾਨਾ(pParse, pTabList, pNx, pUpsert) ){
          ਜਾਓ ਪਾਓ_cleanup;
        }
      }
      pNx = pNx->p_ਅਗਲਾ_Upsert;
    }ਜਦੋਂ( pNx!=0 );
  }
#ਜੇ_ਅੰਤ


  /* This is the top of the main insertion loop */
  ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addr_Ins_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }ਨਹੀਂਤਾਂ ਜੇ( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, reg_ਅੰਕੜੇ, pTab->n_ਖਾਨਾ, 0, 0);
    addr_Ins_ਸਿਖਰ = addrCont = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    ਜੇ( ipk_ਕਾਲਮ>=0 ){
      /* tag-20191021-001: If the INTEGER PRIMARY KEY is being generated by the
      ** SELECT, go ahead and copy the value into the rowid slot now, so that
      ** the value does not get overwritten by a NULL at tag-20191021-002. */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, regFromSelect+ipk_ਕਾਲਮ, regRowid);
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
  iRegStore = reg_ਅੰਕੜੇ;  ਪ੍ਰਤਿਜ੍ਞਾ( reg_ਅੰਕੜੇ==regRowid+1 );
  ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++, iRegStore++){
    ਪੂਰਨ k;
    u32 ਖਾਨਾ_Flags;
    ਪ੍ਰਤਿਜ੍ਞਾ( i>=nHidden );
    ਜੇ( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_SoftNull, iRegStore);
      ਜਾਰੀ;
    }
    ਜੇ( ((ਖਾਨਾ_Flags = pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      ਜੇ( (ਖਾਨਾ_Flags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( (ਖਾਨਾ_Flags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        ਜੇ( tmask & TRIGGER_BEFORE ){
          sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_SoftNull, iRegStore);
        }
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਕਾਲਮ==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(pParse,
            sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab, &pTab->a_ਖਾਨਾ[i], pParse),
            iRegStore);
        ਜਾਰੀ;
      }
    }
    ਜੇ( p_ਕਾਲਮ ){
      j = a_Tab_ਖਾਨਾ_Map[i];
      ਪ੍ਰਤਿਜ੍ਞਾ( j>=0 && j<=p_ਕਾਲਮ->nId );   
      ਜੇ( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(pParse,
            sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab, &pTab->a_ਖਾਨਾ[i], pParse),
            iRegStore);
        ਜਾਰੀ;
      }
      k = j - 1;
    }ਨਹੀਂਤਾਂ ਜੇ( n_ਕਾਲਮ==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(pParse,
          sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab, &pTab->a_ਖਾਨਾ[i], pParse),
          iRegStore);
      ਜਾਰੀ;
    }ਨਹੀਂਤਾਂ{
      k = i - nHidden;
    }

    ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, srcTab, k, iRegStore);
    }ਨਹੀਂਤਾਂ ਜੇ( pSelect ){
      ਜੇ( regFromSelect!=reg_ਅੰਕੜੇ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }ਨਹੀਂਤਾਂ{
      ਵਿਅੰਜਕ *pX = pList->a[k].p_ਵਿਅੰਜਕ;
      ਪੂਰਨ y = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, pX, iRegStore);
      ਜੇ( y!=iRegStore ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v,
          ਵਿਅੰਜਕ_Has_Property(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  ਸਿਰਾ_ਦਾ_Loop = sqlite3VdbeMakeLabel(pParse);
  ਜੇ( tmask & TRIGGER_BEFORE ){
    ਪੂਰਨ reg_ਖਾਨੇ_ਗਿਣਤੀ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, pTab->n_ਖਾਨਾ+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    ਜੇ( ipk_ਕਾਲਮ<0 ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, -1, reg_ਖਾਨੇ_ਗਿਣਤੀ);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ addr1;
      ਪ੍ਰਤਿਜ੍ਞਾ( !withoutRowid );
      ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, srcTab, ipk_ਕਾਲਮ, reg_ਖਾਨੇ_ਗਿਣਤੀ);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pList->a[ipk_ਕਾਲਮ].p_ਵਿਅੰਜਕ, reg_ਖਾਨੇ_ਗਿਣਤੀ);
      }
      addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_NotNull, reg_ਖਾਨੇ_ਗਿਣਤੀ); VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, -1, reg_ਖਾਨੇ_ਗਿਣਤੀ);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_MustBeInt, reg_ਖਾਨੇ_ਗਿਣਤੀ); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    ਪ੍ਰਤਿਜ੍ਞਾ( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, regRowid+1, reg_ਖਾਨੇ_ਗਿਣਤੀ+1, pTab->nNVCol-1);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    ਜੇ( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, reg_ਖਾਨੇ_ਗਿਣਤੀ+1, pTab);
    }
#ਜੇ_ਅੰਤ

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    ਜੇ( !isView ){
      sqlite3_ਸਾਰਣੀ_Affinity(v, pTab, reg_ਖਾਨੇ_ਗਿਣਤੀ+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger, TK_ਪਾਓ, 0, TRIGGER_BEFORE,
        pTab, reg_ਖਾਨੇ_ਗਿਣਤੀ-pTab->n_ਖਾਨਾ-1, onError, ਸਿਰਾ_ਦਾ_Loop);

    sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਖਾਨੇ_ਗਿਣਤੀ, pTab->n_ਖਾਨਾ+1);
  }

  ਜੇ( !isView ){
    ਜੇ( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, regIns);
    }
    ਜੇ( ipk_ਕਾਲਮ>=0 ){
      /* Compute the new rowid */
      ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, srcTab, ipk_ਕਾਲਮ, regRowid);
      }ਨਹੀਂਤਾਂ ਜੇ( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *pIpk = pList->a[ipk_ਕਾਲਮ].p_ਵਿਅੰਜਕ;
        ਜੇ( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NewRowid, i_ਅੰਕੜੇ_Cur, regRowid, regAutoinc);
          appendFlag = 1;
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pList->a[ipk_ਕਾਲਮ].p_ਵਿਅੰਜਕ, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      ਜੇ( !appendFlag ){
        ਪੂਰਨ addr1;
        ਜੇ( !IsVirtual(pTab) ){
          addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NewRowid, i_ਅੰਕੜੇ_Cur, regRowid, regAutoinc);
          sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
        }ਨਹੀਂਤਾਂ{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( IsVirtual(pTab) || withoutRowid ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, regRowid);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NewRowid, i_ਅੰਕੜੇ_Cur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    auto_Inc_ਕਦਮ(pParse, regAutoinc, regRowid);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    ਜੇ( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#ਜੇ_ਅੰਤ

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਜੇ( IsVirtual(pTab) ){
      ਅਚਲ ਅੱਖਰ *pVTab = (ਅਚਲ ਅੱਖਰ *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VUpdate, 1, pTab->n_ਖਾਨਾ+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਪੂਰਨ isReplace = 0;/* Set to true if constraints may cause a replace */
      ਪੂਰਨ bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_ਅੰਕੜੇ_Cur, iIdxCur,
          regIns, 0, ipk_ਕਾਲਮ>=0, onError, ਸਿਰਾ_ਦਾ_Loop, &isReplace, 0, pUpsert
      );
      ਜੇ( db->flags & SQLITE_Foreign_ਕੁੰਜੀਆਂ ){
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
      bUseSeek = (isReplace==0 || !sqlite3_Vdbe_Has_Sub_ਪ੍ਰੋਗਰਾਮ(v));
      sqlite3CompleteInsertion(pParse, pTab, i_ਅੰਕੜੇ_Cur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
  }ਨਹੀਂਤਾਂ ਜੇ( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, -1, regRowid);
#ਜੇ_ਅੰਤ
  }

  /* Update the count of rows that are inserted
  */
  ਜੇ( reg_ਪੰਕਤੀ_ਗਿਣਤੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, reg_ਪੰਕਤੀ_ਗਿਣਤੀ, 1);
  }

  ਜੇ( pTrigger ){
    /* Code AFTER triggers */
    sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(pParse, pTrigger, TK_ਪਾਓ, 0, TRIGGER_AFTER,
        pTab, reg_ਅੰਕੜੇ-2-pTab->n_ਖਾਨਾ, onError, ਸਿਰਾ_ਦਾ_Loop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, ਸਿਰਾ_ਦਾ_Loop);
  ਜੇ( use_ਆਰਜ਼ੀ_ਸਾਰਣੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, srcTab, addrCont); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_Ins_ਸਿਖਰ);
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Close, srcTab);
  }ਨਹੀਂਤਾਂ ਜੇ( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    ਜੇ( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#ਜੇ_ਅੰਤ
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_Ins_ਸਿਖਰ);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_XFER_OPT
ਪਾਓ_ਸਿਰਾ:
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  ਜੇ( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_ਸਿਰਾ(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  ਜੇ( reg_ਪੰਕਤੀ_ਗਿਣਤੀ ){
    sqlite3_ਕੋਡ_Change_ਗਿਣਤੀ(v, reg_ਪੰਕਤੀ_ਗਿਣਤੀ, "rows inserted");
  }

ਪਾਓ_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  ਜੇ( p_ਕਾਲਮ ){
    sqlite3IdListDelete(db, p_ਕਾਲਮ);
    sqlite3DbFree(db, a_Tab_ਖਾਨਾ_Map);
  }
  ਜੇ( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ isView
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ isView
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ pTrigger
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ pTrigger
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ tmask
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ tmask
#ਜੇ_ਅੰਤ

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CKCNSTRNT_ਕਾਲਮ   0x01    /* CHECK constraint uses a changing column */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
ਸਥਿਰ ਪੂਰਨ check_Constraint_ਵਿਅੰਜਕ_ਗੰਢ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0 || p_ਵਿਅੰਜਕ->i_ਕਾਲਮ==-1 );
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0 ){
      ਜੇ( pWalker->u.ai_ਖਾਨਾ[p_ਵਿਅੰਜਕ->i_ਕਾਲਮ]>=0 ){
        pWalker->e_ਕੋਡ |= CKCNSTRNT_ਕਾਲਮ;
      }
    }ਨਹੀਂਤਾਂ{
      pWalker->e_ਕੋਡ |= CKCNSTRNT_ROWID;
    }
  }
  ਵਾਪਸ WRC_Continue;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,    /* The expression to be checked */
  ਪੂਰਨ *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  ਪੂਰਨ chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  memset(&w, 0, ਆਕਾਰ(w));
  w.e_ਕੋਡ = 0;
  w.x_ਵਿਅੰਜਕ_Callback = check_Constraint_ਵਿਅੰਜਕ_ਗੰਢ;
  w.u.ai_ਖਾਨਾ = aiChng;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
  ਜੇ( !chngRowid ){
    testcase( (w.e_ਕੋਡ & CKCNSTRNT_ROWID)!=0 );
    w.e_ਕੋਡ &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.e_ਕੋਡ==0 );
  testcase( w.e_ਕੋਡ==CKCNSTRNT_ਕਾਲਮ );
  testcase( w.e_ਕੋਡ==CKCNSTRNT_ROWID );
  testcase( w.e_ਕੋਡ==(CKCNSTRNT_ROWID|CKCNSTRNT_ਕਾਲਮ) );
  ਵਾਪਸ w.e_ਕੋਡ!=0;
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੂਚਕਾਂਕ_List_ਪਦ ਸੂਚਕਾਂਕ_List_ਪਦ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੂਚਕਾਂਕ_Iterator ਸੂਚਕਾਂਕ_Iterator;
ਬਣਤਰ ਸੂਚਕਾਂਕ_Iterator {
  ਪੂਰਨ eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  ਪੂਰਨ i;        /* Index of the current item from the list */
  ਸੰਘ {
    ਬਣਤਰ {    /* Use this object for eType==0: A Index.pNext list */
      ਸੂਚਕਾਂਕ *pIdx;   /* The current Index */
    } lx;     
    ਬਣਤਰ {    /* Use this object for eType==1; Array of IndexListTerm */
      ਪੂਰਨ nIdx;               /* Size of the array */
      ਸੂਚਕਾਂਕ_List_ਪਦ *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
ਬਣਤਰ ਸੂਚਕਾਂਕ_List_ਪਦ {
  ਸੂਚਕਾਂਕ *p;  /* The index */
  ਪੂਰਨ ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
ਸਥਿਰ ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ_Iterator_ਪਹਿਲਾ(ਸੂਚਕਾਂਕ_Iterator *pIter, ਪੂਰਨ *pIx){
  ਪ੍ਰਤਿਜ੍ਞਾ( pIter->i==0 );
  ਜੇ( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    ਵਾਪਸ pIter->u.ax.aIdx[0].p;
  }ਨਹੀਂਤਾਂ{
    *pIx = 0;
    ਵਾਪਸ pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
ਸਥਿਰ ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ_Iterator_ਅਗਲਾ(ਸੂਚਕਾਂਕ_Iterator *pIter, ਪੂਰਨ *pIx){
  ਜੇ( pIter->eType ){
    ਪੂਰਨ i = ++pIter->i;
    ਜੇ( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      ਵਾਪਸ 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    ਵਾਪਸ pIter->u.ax.aIdx[i].p;
  }ਨਹੀਂਤਾਂ{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->p_ਅਗਲਾ;
    ਵਾਪਸ pIter->u.lx.pIdx;
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
ਖਾਲੀ sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  ਸਾਰਣੀ *pTab,         /* The table being inserted or updated */
  ਪੂਰਨ *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur,        /* Canonical data cursor (main table or PK index) */
  ਪੂਰਨ iIdxCur,         /* First index cursor */
  ਪੂਰਨ reg_New_ਅੰਕੜੇ,      /* First register in a range holding values to insert */
  ਪੂਰਨ reg_Old_ਅੰਕੜੇ,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  ਪੂਰਨ ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  ਪੂਰਨ *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  ਪੂਰਨ *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  ਸੂਚਕਾਂਕ *pIdx;         /* Pointer to one of the indices */
  ਸੂਚਕਾਂਕ *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  ਪੂਰਨ i;               /* loop counter */
  ਪੂਰਨ ix;              /* Index loop counter */
  ਪੂਰਨ n_ਖਾਨਾ;            /* Number of columns */
  ਪੂਰਨ onError;         /* Conflict resolution strategy */
  ਪੂਰਨ ਵੇਖਿਆ_Replace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  ਪੂਰਨ n_Pk_ਖੇਤਰ;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 b_Affinity_ਪੂਰਾ = 0;  /* True if the OP_Affinity operation has been run */
  ਪੂਰਨ upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  ਪੂਰਨ upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  ਪੂਰਨ ipk_ਸਿਖਰ = 0;        /* Top of the IPK uniqueness check */
  ਪੂਰਨ ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  ਪੂਰਨ regTrigCnt;       /* Register used to count replace trigger invocations */
  ਪੂਰਨ addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  ਪੂਰਨ lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  ਪੂਰਨ nReplaceTrig = 0; /* Number of replace triggers coded */
  ਸੂਚਕਾਂਕ_Iterator sIdxIter;  /* Index iterator */

  isUpdate = reg_Old_ਅੰਕੜੇ!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsView(pTab) );  /* This table is not a VIEW */
  n_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  ਜੇ( HasRowid(pTab) ){
    pPk = 0;
    n_Pk_ਖੇਤਰ = 1;
  }ਨਹੀਂਤਾਂ{
    pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
    n_Pk_ਖੇਤਰ = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     i_ਅੰਕੜੇ_Cur, iIdxCur, reg_New_ਅੰਕੜੇ, reg_Old_ਅੰਕੜੇ, pkChng));

  /* Test all NOT NULL constraints.
  */
  ਜੇ( pTab->tabFlags & TF_HasNotNull ){
    ਪੂਰਨ b2ndPass = 0;         /* True if currently running 2nd pass */
    ਪੂਰਨ n_ਵੇਖਿਆ_Replace = 0;     /* Number of ON CONFLICT REPLACE operations */
    ਪੂਰਨ nGenerated = 0;       /* Number of generated columns with NOT NULL */
    ਜਦੋਂ(1){  /* Make 2 passes over columns. Exit loop via "break" */
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        ਪੂਰਨ iReg;                        /* Register holding column value */
        ਕਾਲਮ *p_ਖਾਨਾ = &pTab->a_ਖਾਨਾ[i];   /* The column to check for NOT NULL */
        ਪੂਰਨ isGenerated;                 /* non-zero if column is generated */
        onError = p_ਖਾਨਾ->notNull;
        ਜੇ( onError==OE_None ) ਜਾਰੀ; /* No NOT NULL on this column */
        ਜੇ( i==pTab->iPKey ){
          ਜਾਰੀ;        /* ROWID is never NULL */
        }
        isGenerated = p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED;
        ਜੇ( isGenerated && !b2ndPass ){
          nGenerated++;
          ਜਾਰੀ;        /* Generated columns processed on 2nd pass */
        }
        ਜੇ( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          ਜਾਰੀ;
        }
        ਜੇ( overrideError!=OE_Default ){
          onError = overrideError;
        }ਨਹੀਂਤਾਂ ਜੇ( onError==OE_Default ){
          onError = OE_Abort;
        }
        ਜੇ( onError==OE_Replace ){
          ਜੇ( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || p_ਖਾਨਾ->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
            testcase( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_STORED );
            testcase( p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }ਨਹੀਂਤਾਂ{
            ਪ੍ਰਤਿਜ੍ਞਾ( !isGenerated );
          }
        }ਨਹੀਂਤਾਂ ਜੇ( b2ndPass && !isGenerated ){
          ਜਾਰੀ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i) );
        iReg = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i) + reg_New_ਅੰਕੜੇ + 1;
        ਚੋਣ( onError ){
          ਹਾਲਤ OE_Replace: {
            ਪੂਰਨ addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED)==0 );
            n_ਵੇਖਿਆ_Replace++;
            sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(pParse,
               sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(pTab, p_ਖਾਨਾ, pParse), iReg);
            sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
            ਰੋਕੋ;
          }
          ਹਾਲਤ OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          ਹਾਲਤ OE_Rollback:
          ਹਾਲਤ OE_Fail: {
            ਅੱਖਰ *z_ਸੁਨੇਹਾ = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        p_ਖਾਨਾ->zCnName);
            testcase( z_ਸੁਨੇਹਾ==0 && db->mallocFailed==0 );
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, z_ਸੁਨੇਹਾ, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            ਰੋਕੋ;
          }
          ਮੂਲ: {
            ਪ੍ਰਤਿਜ੍ਞਾ( onError==OE_Ignore );
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            ਰੋਕੋ;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      ਜੇ( nGenerated==0 && n_ਵੇਖਿਆ_Replace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        ਰੋਕੋ;
      }
      ਜੇ( b2ndPass ) ਰੋਕੋ;  /* Never need more than 2 passes */
      b2ndPass = 1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
      ਜੇ( n_ਵੇਖਿਆ_Replace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, reg_New_ਅੰਕੜੇ+1, pTab);
      }
#ਜੇ_ਅੰਤ
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CHECK
  ਜੇ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    ਵਿਅੰਜਕ_List *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(reg_New_ਅੰਕੜੇ+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    ਲਈ(i=0; i<pCheck->n_ਵਿਅੰਜਕ; i++){
      ਪੂਰਨ allOk;
      ਵਿਅੰਜਕ *pCopy;
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pCheck->a[i].p_ਵਿਅੰਜਕ;
      ਜੇ( aiChng
       && !sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(p_ਵਿਅੰਜਕ, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        ਜਾਰੀ;
      }
      ਜੇ( b_Affinity_ਪੂਰਾ==0 ){
        sqlite3_ਸਾਰਣੀ_Affinity(v, pTab, reg_New_ਅੰਕੜੇ+1);
        b_Affinity_ਪੂਰਾ = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
      ਜੇ( !db->mallocFailed ){
        sqlite3_ਵਿਅੰਜਕ_If_True(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3_ਵਿਅੰਜਕ_Delete(db, pCopy);
      ਜੇ( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }ਨਹੀਂਤਾਂ{
        ਅੱਖਰ *zName = pCheck->a[i].zEName;
        ਪ੍ਰਤਿਜ੍ਞਾ( zName!=0 || pParse->db->mallocFailed );
        ਜੇ( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_CHECK) */

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
  sIdxIter.u.lx.pIdx = pTab->p_ਸੂਚਕਾਂਕ;
  ਜੇ( pUpsert ){
    ਜੇ( pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      ਪ੍ਰਤਿਜ੍ਞਾ( pUpsert->p_ਅਗਲਾ_Upsert==0 );
      ਜੇ( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }ਨਹੀਂਤਾਂ{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( pTab->p_ਸੂਚਕਾਂਕ!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      ਪੂਰਨ nIdx, jj;
      u64 nByte;
      Upsert *p_ਪਦ;
      u8 *b_ਵਰਤਿਆ;
      ਲਈ(nIdx=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, nIdx++){
         ਪ੍ਰਤਿਜ੍ਞਾ( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (ਆਕਾਰ(ਸੂਚਕਾਂਕ_List_ਪਦ)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      ਜੇ( sIdxIter.u.ax.aIdx==0 ) ਵਾਪਸ; /* OOM */
      b_ਵਰਤਿਆ = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->p_ਮੰਜ਼ਿਲ_Free = sIdxIter.u.ax.aIdx;
      ਲਈ(i=0, p_ਪਦ=pUpsert; p_ਪਦ; p_ਪਦ=p_ਪਦ->p_ਅਗਲਾ_Upsert){
        ਜੇ( p_ਪਦ->p_Upsert_ਨਿਸ਼ਾਨਾ==0 ) ਰੋਕੋ;
        ਜੇ( p_ਪਦ->pUpsertIdx==0 ) ਜਾਰੀ;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->p_ਸੂਚਕਾਂਕ;
        ਜਦੋਂ( ALWAYS(pIdx!=0) && pIdx!=p_ਪਦ->pUpsertIdx ){
           pIdx = pIdx->p_ਅਗਲਾ;
           jj++;
        }
        ਜੇ( b_ਵਰਤਿਆ[jj] ) ਜਾਰੀ; /* Duplicate ON CONFLICT clause ignored */
        b_ਵਰਤਿਆ[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      ਲਈ(jj=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, jj++){
        ਜੇ( b_ਵਰਤਿਆ[jj] ) ਜਾਰੀ;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( i==nIdx );
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
  ਜੇ( (db->flags & (SQLITE_RecTriggers|SQLITE_Foreign_ਕੁੰਜੀਆਂ))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }ਨਹੀਂਤਾਂ{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    ਜੇ( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      ਜੇ( overrideError!=OE_Replace ){
        ਜੇ( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }ਨਹੀਂਤਾਂ ਜੇ( pkChng==0 || pPk || pTab->ਕੁੰਜੀ_Conf!=OE_Replace ){
          ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
            ਜੇ( pIdx->onError==OE_Replace ) ਰੋਕੋ;
          }
          ਜੇ( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    ਜੇ( regTrigCnt ){
      /* Replace triggers might exist.  Allocate the counter and
      ** initialize it to zero. */
      regTrigCnt = ++pParse->nMem;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regTrigCnt);
      VdbeComment((v, "trigger count"));
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  /* If rowid is changing, make sure the new rowid does not previously
  ** exist in the table.
  */
  ਜੇ( pkChng && pPk==0 ){
    ਪੂਰਨ addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->ਕੁੰਜੀ_Conf;
    ਜੇ( overrideError!=OE_Default ){
      onError = overrideError;
    }ਨਹੀਂਤਾਂ ਜੇ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    ਜੇ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(pUpsert,0);
      ਜੇ( pUpsertClause!=0 ){
        ਜੇ( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }ਨਹੀਂਤਾਂ{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      ਜੇ( pUpsertClause!=pUpsert ){
        /* The first ON CONFLICT clause has a conflict target other than
        ** the IPK.  We have to jump ahead to that first ON CONFLICT clause
        ** and then come back here and deal with the IPK afterwards */
        upsertIpkDelay = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
      }
    }

    /* If the response to a rowid conflict is REPLACE but the response
    ** to some other UNIQUE constraint is FAIL or IGNORE, then we need
    ** to defer the running of the rowid conflict checking until after
    ** the UNIQUE constraints have run.
    */
    ਜੇ( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->p_ਸੂਚਕਾਂਕ             /* There exist other constraints */
    ){
      ਜੇ( upsertIpkDelay ){
        ipk_ਸਿਖਰ = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }ਨਹੀਂਤਾਂ{
        ipk_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    ਜੇ( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, reg_New_ਅੰਕੜੇ, addrRowidOk, reg_Old_ਅੰਕੜੇ);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, i_ਅੰਕੜੇ_Cur, addrRowidOk, reg_New_ਅੰਕੜੇ);
    VdbeCoverage(v);

    ਚੋਣ( onError ){
      ਮੂਲ: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      ਹਾਲਤ OE_Rollback:
      ਹਾਲਤ OE_Abort:
      ਹਾਲਤ OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        ਰੋਕੋ;
      }
      ਹਾਲਤ OE_Replace: {
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
        ਜੇ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3_Generate_ਪੰਕਤੀ_Delete(pParse, pTab, pTrigger, i_ਅੰਕੜੇ_Cur, iIdxCur,
                                   reg_New_ਅੰਕੜੇ, 1, 0, OE_Replace, 1, -1);
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
          ਪ੍ਰਤਿਜ੍ਞਾ( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Delete, i_ਅੰਕੜੇ_Cur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */
          ਜੇ( pTab->p_ਸੂਚਕਾਂਕ ){
            sqlite3MultiWrite(pParse);
            sqlite3_Generate_ਪੰਕਤੀ_ਸੂਚਕਾਂਕ_Delete(pParse, pTab, i_ਅੰਕੜੇ_Cur, iIdxCur,0,-1);
          }
        }
        ਵੇਖਿਆ_Replace = 1;
        ਰੋਕੋ;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
      ਹਾਲਤ OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, i_ਅੰਕੜੇ_Cur);
        /* no break */ deliberate_fall_through
      }
#ਜੇ_ਅੰਤ
      ਹਾਲਤ OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ਰੋਕੋ;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    ਜੇ( ipk_ਸਿਖਰ ){
      ipkBottom = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, ipk_ਸਿਖਰ-1);
    }ਨਹੀਂਤਾਂ ਜੇ( pUpsert && pUpsertClause!=pUpsert ){
      upsertIpkReturn = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
    }
  }

  /* Test all UNIQUE constraints by creating entries for each UNIQUE
  ** index and making sure that duplicate entries do not already exist.
  ** Compute the revised record entries for indices as we go.
  **
  ** This loop also handles the case of the PRIMARY KEY index for a
  ** WITHOUT ROWID table.
  */
  ਲਈ(pIdx = ਸੂਚਕਾਂਕ_Iterator_ਪਹਿਲਾ(&sIdxIter, &ix);
      pIdx;
      pIdx = ਸੂਚਕਾਂਕ_Iterator_ਅਗਲਾ(&sIdxIter, &ix)
  ){
    ਪੂਰਨ regIdx;          /* Range of registers holding content for pIdx */
    ਪੂਰਨ regR;            /* Range of registers holding conflicting PK */
    ਪੂਰਨ iThisCur;        /* Cursor for this UNIQUE index */
    ਪੂਰਨ addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    ਪੂਰਨ addrConflictCk;  /* First opcode in the conflict check logic */
    ਪੂਰਨ nConflictCk;     /* Number of opcodes in conflict check logic */

    ਜੇ( aRegIdx[ix]==0 ) ਜਾਰੀ;  /* Skip indices that do not change */
    ਜੇ( pUpsert ){
      pUpsertClause = sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(pUpsert, pIdx);
      ਜੇ( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    ਜੇ( b_Affinity_ਪੂਰਾ==0 ){
      sqlite3_ਸਾਰਣੀ_Affinity(v, pTab, reg_New_ਅੰਕੜੇ+1);
      b_Affinity_ਪੂਰਾ = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    ਜੇ( pIdx->pPartIdxWhere ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(reg_New_ਅੰਕੜੇ+1);
      sqlite3_ਵਿਅੰਜਕ_If_False_Dup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    ਲਈ(i=0; i<pIdx->n_ਕਾਲਮ; i++){
      ਪੂਰਨ i_ਖੇਤਰ = pIdx->ai_ਕਾਲਮ[i];
      ਪੂਰਨ x;
      ਜੇ( i_ਖੇਤਰ==XN_ਵਿਅੰਜਕ ){
        pParse->iSelfTab = -(reg_New_ਅੰਕੜੇ+1);
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(pParse, pIdx->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }ਨਹੀਂਤਾਂ ਜੇ( i_ਖੇਤਰ==XN_ROWID || i_ਖੇਤਰ==pTab->iPKey ){
        x = reg_New_ਅੰਕੜੇ;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }ਨਹੀਂਤਾਂ{
        testcase( sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖੇਤਰ)!=i_ਖੇਤਰ );
        x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖੇਤਰ) + reg_New_ਅੰਕੜੇ + 1;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->a_ਖਾਨਾ[i_ਖੇਤਰ].zCnName));
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regIdx, pIdx->n_ਕਾਲਮ, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NULL_TRIM
    ਜੇ( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3_ਸੈੱਟ_Make_ਰਿਕਾਰਡ_P5(v, pIdx->p_ਸਾਰਣੀ);
    }
#ਜੇ_ਅੰਤ
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->n_ਕਾਲਮ, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    ਜੇ( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ਜਾਰੀ;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    ਜੇ( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ਜਾਰੀ;  /* pIdx is not a UNIQUE index */
    }
    ਜੇ( overrideError!=OE_Default ){
      onError = overrideError;
    }ਨਹੀਂਤਾਂ ਜੇ( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    ਜੇ( pUpsertClause ){
      ਜੇ( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }ਨਹੀਂਤਾਂ{
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
    ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    ਜੇ( (ix==0 && pIdx->p_ਅਗਲਾ==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      ਜਾਰੀ;
    }
#ਜੇ_ਅੰਤ /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_Pk_ਖੇਤਰ);
    ਜੇ( isUpdate || onError==OE_Replace ){
      ਜੇ( HasRowid(pTab) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        ਜੇ( isUpdate ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, regR, addrUniqueOk, reg_Old_ਅੰਕੜੇ);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        ਜੇ( pIdx!=pPk ){
          ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
            ਪ੍ਰਤਿਜ੍ਞਾ( pPk->ai_ਕਾਲਮ[i]>=0 );
            x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pIdx, pPk->ai_ਕਾਲਮ[i]);
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->a_ਖਾਨਾ[pPk->ai_ਕਾਲਮ[i]].zCnName));
          }
        }
        ਜੇ( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          ਪੂਰਨ addrJump = sqlite3VdbeCurrentAddr(v)+pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
          ਪੂਰਨ op = OP_Ne;
          ਪੂਰਨ regCmp = (Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ? regIdx : regR);
 
          ਲਈ(i=0; i<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
            ਅੱਖਰ *p4 = (ਅੱਖਰ*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->ai_ਕਾਲਮ[i];
            ਪ੍ਰਤਿਜ੍ਞਾ( x>=0 );
            ਜੇ( i==(pPk->n_ਕੁੰਜੀ_ਖਾਨਾ-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, x);
            sqlite3_Vdbe_ਜੋੜੋ_Op4(v, op,
                reg_Old_ਅੰਕੜੇ+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    ਪ੍ਰਤਿਜ੍ਞਾ( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    ਪ੍ਰਤਿਜ੍ਞਾ( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    ਚੋਣ( onError ){
      ਹਾਲਤ OE_Rollback:
      ਹਾਲਤ OE_Abort:
      ਹਾਲਤ OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        ਰੋਕੋ;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
      ਹਾਲਤ OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#ਜੇ_ਅੰਤ
      ਹਾਲਤ OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        ਪ੍ਰਤਿਜ੍ਞਾ( onError==OE_Replace );
        ਜੇ( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        ਜੇ( pTrigger && isUpdate ){
          sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Cursor_ਤਾਲਾ, i_ਅੰਕੜੇ_Cur);
        }
        sqlite3_Generate_ਪੰਕਤੀ_Delete(pParse, pTab, pTrigger, i_ਅੰਕੜੇ_Cur, iIdxCur,
            regR, n_Pk_ਖੇਤਰ, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        ਜੇ( pTrigger && isUpdate ){
          sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_CursorUnlock, i_ਅੰਕੜੇ_Cur);
        }
        ਵੇਖਿਆ_Replace = 1;
        ਰੋਕੋ;
      }
    }
    ਜੇ( regTrigCnt ){
      ਪੂਰਨ addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      ਜੇ( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      ਜਦੋਂ( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        ਜੇ( x.opcode!=OP_IdxRowid || isUpdate ){
          ਪੂਰਨ p2;      /* New P2 value for copied conflict check opcode */
          ਅਚਲ ਅੱਖਰ *zP4;
          ਜੇ( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }ਨਹੀਂਤਾਂ{
            p2 = x.p2;
          }
          zP4 = x.p4type==P4_INT32 ? SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(x.p4.i) : x.p4.z;
          sqlite3_Vdbe_ਜੋੜੋ_Op4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
          sqlite3VdbeChangeP5(v, x.p5);
          VdbeCoverageIf(v, p2!=x.p2);
        }
        nConflictCk--;
        addrConflictCk++;
      }
      /* If the retest fails, issue an abort */
      sqlite3UniqueConstraint(pParse, OE_Abort, pIdx);

      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrBypass); /* Terminate the recheck bypass */
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    ਜੇ( regR!=regIdx ) sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, regR, n_Pk_ਖੇਤਰ);
    ਜੇ( pUpsertClause
     && upsertIpkReturn
     && sqlite3_Upsert_ਅਗਲਾ_Is_IPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  ਜੇ( ipk_ਸਿਖਰ ){
    sqlite3VdbeGoto(v, ipk_ਸਿਖਰ);
    VdbeComment((v, "Do IPK REPLACE"));
    ਪ੍ਰਤਿਜ੍ਞਾ( ipkBottom>0 );
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( regTrigCnt!=0 || nReplaceTrig==0 );
  ਜੇ( nReplaceTrig ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    ਜੇ( !pPk ){
      ਜੇ( isUpdate ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, reg_New_ਅੰਕੜੇ, addrRecheck, reg_Old_ਅੰਕੜੇ);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, i_ਅੰਕੜੇ_Cur, addrRecheck, reg_New_ਅੰਕੜੇ);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }ਨਹੀਂਤਾਂ{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  ਜੇ( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  ਜੇ( HasRowid(pTab) ){
    ਪੂਰਨ regRec = aRegIdx[ix];
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_New_ਅੰਕੜੇ+1, pTab->nNVCol, regRec);
    sqlite3_ਸੈੱਟ_Make_ਰਿਕਾਰਡ_P5(v, pTab);
    ਜੇ( !b_Affinity_ਪੂਰਾ ){
      sqlite3_ਸਾਰਣੀ_Affinity(v, pTab, 0);
    }
  }

  *pbMayReplace = ਵੇਖਿਆ_Replace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", ਵੇਖਿਆ_Replace));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
ਖਾਲੀ sqlite3_ਸੈੱਟ_Make_ਰਿਕਾਰਡ_P5(Vdbe *v, ਸਾਰਣੀ *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  ਜੇ( pTab->pSchema->file_format<2 ) ਵਾਪਸ;

  ਲਈ(i=pTab->n_ਖਾਨਾ-1; i>0; i--){
    ਜੇ( pTab->a_ਖਾਨਾ[i].iDflt!=0 ) ਰੋਕੋ;
    ਜੇ( pTab->a_ਖਾਨਾ[i].ਖਾਨਾ_Flags & COLFLAG_PRIMKEY ) ਰੋਕੋ;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#ਜੇ_ਅੰਤ

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Without_Rowid_Preupdate(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Table being updated */
  ਪੂਰਨ iCur,                       /* Cursor number for table */
  ਪੂਰਨ reg_ਅੰਕੜੇ                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ r = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  ਪ੍ਰਤਿਜ੍ਞਾ( !HasRowid(pTab) );
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, r);
  sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_ਪਾਓ, iCur, reg_ਅੰਕੜੇ, r, (ਅੱਖਰ*)pTab, P4_ਸਾਰਣੀ);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r);
}
#ਨਹੀਂਤਾਂ
# define ਕੋਡ_Without_Rowid_Preupdate(a,b,c,d)
#ਜੇ_ਅੰਤ

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
ਖਾਲੀ sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  ਸਾਰਣੀ *pTab,        /* the table into which we are inserting */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur,       /* Cursor of the canonical data source */
  ਪੂਰਨ iIdxCur,        /* First index cursor */
  ਪੂਰਨ reg_New_ਅੰਕੜੇ,     /* Range of content */
  ਪੂਰਨ *aRegIdx,       /* Register used by each index.  0 for unused indices */
  ਪੂਰਨ update_flags,   /* True for UPDATE, False for INSERT */
  ਪੂਰਨ appendBias,     /* True if this is likely to be an append */
  ਪੂਰਨ use_Seek_ਨਤੀਜਾ   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  ਸੂਚਕਾਂਕ *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  ਪੂਰਨ i;              /* Loop counter */

  ਪ੍ਰਤਿਜ੍ਞਾ( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsView(pTab) );  /* This table is not a VIEW */
  ਲਈ(i=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, i++){
    /* All REPLACE indexes are at the end of the list */
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->onError!=OE_Replace
         || pIdx->p_ਅਗਲਾ==0
         || pIdx->p_ਅਗਲਾ->onError==OE_Replace );
    ਜੇ( aRegIdx[i]==0 ) ਜਾਰੀ;
    ਜੇ( pIdx->pPartIdxWhere || (update_flags && pIdx->b_Has_ਵਿਅੰਜਕ) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (use_Seek_ਨਤੀਜਾ ? OPFLAG_USESEEKRESULT : 0);
    ਜੇ( Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      ਜੇ( update_flags==0 ){
        ਕੋਡ_Without_Rowid_Preupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ: pIdx->n_ਕਾਲਮ);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  ਜੇ( !HasRowid(pTab) ) ਵਾਪਸ;
  ਜੇ( pParse->nested ){
    pik_flags = 0;
  }ਨਹੀਂਤਾਂ{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  ਜੇ( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  ਜੇ( use_Seek_ਨਤੀਜਾ ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, i_ਅੰਕੜੇ_Cur, aRegIdx[i], reg_New_ਅੰਕੜੇ);
  ਜੇ( !pParse->nested ){
    sqlite3VdbeAppendP4(v, pTab, P4_ਸਾਰਣੀ);
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
ਪੂਰਨ sqlite3_Open_ਸਾਰਣੀ_And_Indices(
  Parse *pParse,   /* Parsing context */
  ਸਾਰਣੀ *pTab,     /* Table to be opened */
  ਪੂਰਨ op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  ਪੂਰਨ i_ਅਧਾਰ,       /* Use this for the table cursor, if there is one */
  u8 *a_ਮੰਜ਼ਿਲ_Open,     /* If not NULL: boolean for each table and index */
  ਪੂਰਨ *pi_ਅੰਕੜੇ_Cur,  /* Write the database source cursor number here */
  ਪੂਰਨ *piIdxCur    /* Write the first index cursor number here */
){
  ਪੂਰਨ i;
  ਪੂਰਨ iDb;
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur;
  ਸੂਚਕਾਂਕ *pIdx;
  Vdbe *v;

  ਪ੍ਰਤਿਜ੍ਞਾ( op==OP_OpenRead || op==OP_OpenWrite );
  ਪ੍ਰਤਿਜ੍ਞਾ( op==OP_OpenWrite || p5==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pi_ਅੰਕੜੇ_Cur!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( piIdxCur!=0 );
  ਜੇ( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *pi_ਅੰਕੜੇ_Cur = *piIdxCur = -999;
    ਵਾਪਸ 0;
  }
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਜੇ( i_ਅਧਾਰ<0 ) i_ਅਧਾਰ = pParse->nTab;
  i_ਅੰਕੜੇ_Cur = i_ਅਧਾਰ++;
  *pi_ਅੰਕੜੇ_Cur = i_ਅੰਕੜੇ_Cur;
  ਜੇ( HasRowid(pTab) && (a_ਮੰਜ਼ਿਲ_Open==0 || a_ਮੰਜ਼ਿਲ_Open[0]) ){
    sqlite3_Open_ਸਾਰਣੀ(pParse, i_ਅੰਕੜੇ_Cur, iDb, pTab, op);
  }ਨਹੀਂਤਾਂ ਜੇ( pParse->db->noSharedCache==0 ){
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = i_ਅਧਾਰ;
  ਲਈ(i=0, pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ, i++){
    ਪੂਰਨ iIdxCur = i_ਅਧਾਰ++;
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->pSchema==pTab->pSchema );
    ਜੇ( Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) && !HasRowid(pTab) ){
      *pi_ਅੰਕੜੇ_Cur = iIdxCur;
      p5 = 0;
    }
    ਜੇ( a_ਮੰਜ਼ਿਲ_Open==0 || a_ਮੰਜ਼ਿਲ_Open[i+1] ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  ਜੇ( i_ਅਧਾਰ>pParse->nTab ) pParse->nTab = i_ਅਧਾਰ;
  ਵਾਪਸ i;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
ਪੂਰਨ sqlite3_xferopt_ਗਿਣਤੀ;
#ਜੇ_ਅੰਤ /* SQLITE_TEST */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_XFER_OPT
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
ਸਥਿਰ ਪੂਰਨ xfer_Compatible_ਸੂਚਕਾਂਕ(ਸੂਚਕਾਂਕ *pDest, ਸੂਚਕਾਂਕ *pSrc){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( pDest && pSrc );
  ਪ੍ਰਤਿਜ੍ਞਾ( pDest->p_ਸਾਰਣੀ!=pSrc->p_ਸਾਰਣੀ );
  ਜੇ( pDest->n_ਕੁੰਜੀ_ਖਾਨਾ!=pSrc->n_ਕੁੰਜੀ_ਖਾਨਾ || pDest->n_ਕਾਲਮ!=pSrc->n_ਕਾਲਮ ){
    ਵਾਪਸ 0;   /* Different number of columns */
  }
  ਜੇ( pDest->onError!=pSrc->onError ){
    ਵਾਪਸ 0;   /* Different conflict resolution strategies */
  }
  ਲਈ(i=0; i<pSrc->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
    ਜੇ( pSrc->ai_ਕਾਲਮ[i]!=pDest->ai_ਕਾਲਮ[i] ){
      ਵਾਪਸ 0;   /* Different columns indexed */
    }
    ਜੇ( pSrc->ai_ਕਾਲਮ[i]==XN_ਵਿਅੰਜਕ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pSrc->a_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 && pDest->a_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 );
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, pSrc->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ,
                             pDest->a_ਖਾਨਾ_ਵਿਅੰਜਕ->a[i].p_ਵਿਅੰਜਕ, -1)!=0 ){
        ਵਾਪਸ 0;   /* Different expressions in the index */
      }
    }
    ਜੇ( pSrc->a_ਛਾਂਟੋ_ਕ੍ਰਮ[i]!=pDest->a_ਛਾਂਟੋ_ਕ੍ਰਮ[i] ){
      ਵਾਪਸ 0;   /* Different sort orders */
    }
    ਜੇ( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      ਵਾਪਸ 0;   /* Different collating sequences */
    }
  }
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    ਵਾਪਸ 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  ਵਾਪਸ 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
ਸਥਿਰ ਪੂਰਨ xferCheckRowid(Walker *pWalk, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ){
    pWalk->e_ਕੋਡ = 1;
    ਵਾਪਸ WRC_Abort;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ xferCompatibleCheck(ਸਾਰਣੀ *pDest, ਸਾਰਣੀ *pSrc){
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_List_Compare(pSrc->pCheck,pDest->pCheck,-1) ){
    ਵਾਪਸ 0;
  }
  ਜੇ( pDest->iPKey<0 ){
    Walker w;
    memset(&w, 0, ਆਕਾਰ(w));
    w.x_ਵਿਅੰਜਕ_Callback = xferCheckRowid;
    sqlite3_Walk_ਵਿਅੰਜਕ_List(&w,pDest->pCheck);
    ਜੇ( w.e_ਕੋਡ ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ xferOptimization(
  Parse *pParse,        /* Parser context */
  ਸਾਰਣੀ *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  ਪੂਰਨ onError,          /* How to handle constraint errors */
  ਪੂਰਨ iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ_List *pEList;                /* The result set of the SELECT */
  ਸਾਰਣੀ *pSrc;                     /* The table in the FROM clause of SELECT */
  ਸੂਚਕਾਂਕ *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  Src_ਵਸਤੂ *p_ਵਸਤੂ;                  /* An element of pSelect->pSrc */
  ਪੂਰਨ i;                           /* Loop counter */
  ਪੂਰਨ iDbSrc;                      /* The database of pSrc */
  ਪੂਰਨ iSrc, iDest;                 /* Cursors from source and destination */
  ਪੂਰਨ addr1, addr2;                /* Loop addresses */
  ਪੂਰਨ ਖਾਲੀ_ਥਾਵਾਂ_Dest_Test = 0;           /* Address of test for empty pDest */
  ਪੂਰਨ ਖਾਲੀ_ਥਾਵਾਂ_Src_Test = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  ਪੂਰਨ regAutoinc;                  /* Memory register used by AUTOINC */
  ਪੂਰਨ destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  ਪੂਰਨ reg_ਅੰਕੜੇ, regRowid;           /* Registers holding data and rowid */

  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect!=0 );
  ਜੇ( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ( IsVirtual(pDest) ){
    ਵਾਪਸ 0;   /* tab1 must not be a virtual table */
  }
#ਜੇ_ਅੰਤ
  ਜੇ( onError==OE_Default ){
    ਜੇ( pDest->iPKey>=0 ) onError = pDest->ਕੁੰਜੀ_Conf;
    ਜੇ( onError==OE_Default ) onError = OE_Abort;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  ਜੇ( pSelect->pSrc->nSrc!=1 ){
    ਵਾਪਸ 0;   /* FROM clause must have exactly one term */
  }
  ਜੇ( pSelect->pSrc->a[0].fg.isSubquery ){
    ਵਾਪਸ 0;   /* FROM clause cannot contain a subquery */
  }
  ਜੇ( pSelect->pWhere ){
    ਵਾਪਸ 0;   /* SELECT may not have a WHERE clause */
  }
  ਜੇ( pSelect->p_ਕ੍ਰਮ_By ){
    ਵਾਪਸ 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  ਜੇ( pSelect->p_ਸਮੂਹ_By ){
    ਵਾਪਸ 0;   /* SELECT may not have a GROUP BY clause */
  }
  ਜੇ( pSelect->p_ਹੱਦਬੰਦੀ ){
    ਵਾਪਸ 0;   /* SELECT may not have a LIMIT clause */
  }
  ਜੇ( pSelect->p_ਪੂਰਵ ){
    ਵਾਪਸ 0;   /* SELECT may not be a compound query */
  }
  ਜੇ( pSelect->selFlags & SF_Distinct ){
    ਵਾਪਸ 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
  ਜੇ( pEList->n_ਵਿਅੰਜਕ!=1 ){
    ਵਾਪਸ 0;   /* The result set must have exactly one column */
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pEList->a[0].p_ਵਿਅੰਜਕ );
  ਜੇ( pEList->a[0].p_ਵਿਅੰਜਕ->op!=TK_ASTERISK ){
    ਵਾਪਸ 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  p_ਵਸਤੂ = pSelect->pSrc->a;
  pSrc = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, p_ਵਸਤੂ);
  ਜੇ( pSrc==0 ){
    ਵਾਪਸ 0;   /* FROM clause does not contain a real table */
  }
  ਜੇ( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    ਵਾਪਸ 0;   /* tab1 and tab2 may not be the same table */
  }
  ਜੇ( HasRowid(pDest)!=HasRowid(pSrc) ){
    ਵਾਪਸ 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pSrc) ){
    ਵਾਪਸ 0;   /* tab2 may not be a view or virtual table */
  }
  ਜੇ( pDest->n_ਖਾਨਾ!=pSrc->n_ਖਾਨਾ ){
    ਵਾਪਸ 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  ਜੇ( pDest->iPKey!=pSrc->iPKey ){
    ਵਾਪਸ 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  ਜੇ( (pDest->tabFlags & TF_Strict)!=0 ){
    ਜੇ( (pSrc->tabFlags & TF_Strict)==0 ){
      ਵਾਪਸ 0;  /* Cannot feed from a non-strict into a strict table */
    }
    ਲਈ(i=0; i<pDest->n_ਖਾਨਾ; i++){
      ਬਿਨਾਨਿਸ਼ਾਨੀ eDestType = pDest->a_ਖਾਨਾ[i].eCType;
      ਬਿਨਾਨਿਸ਼ਾਨੀ eSrcType = pSrc->a_ਖਾਨਾ[i].eCType;
      ਜੇ( eDestType==COLTYPE_ANY ) ਜਾਰੀ;
      ਜੇ( eDestType==eSrcType ) ਜਾਰੀ;
      ਜੇ( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) ਜਾਰੀ;
      ਜੇ( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) ਜਾਰੀ;
      ਵਾਪਸ 0; /* Incompatible types in source and destination */
    }
  }
  ਲਈ(i=0; i<pDest->n_ਖਾਨਾ; i++){
    ਕਾਲਮ *p_Dest_ਖਾਨਾ = &pDest->a_ਖਾਨਾ[i];
    ਕਾਲਮ *p_Src_ਖਾਨਾ = &pSrc->a_ਖਾਨਾ[i];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_HIDDEN_COLUMNS
    ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags | p_Src_ਖਾਨਾ->ਖਾਨਾ_Flags) & COLFLAG_HIDDEN
    ){
      ਵਾਪਸ 0;    /* Neither table may have __hidden__ columns */
    }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
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
    ਜੇ( (p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED) !=
        (p_Src_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED) ){
      ਵਾਪਸ 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    ਜੇ( (p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED)!=0 ){
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0,
             sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pSrc, p_Src_ਖਾਨਾ),
             sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pDest, p_Dest_ਖਾਨਾ), -1)!=0 ){
        testcase( p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_VIRTUAL );
        testcase( p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_STORED );
        ਵਾਪਸ 0;  /* Different generator expressions */
      }
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p_Dest_ਖਾਨਾ->affinity!=p_Src_ਖਾਨਾ->affinity ){
      ਵਾਪਸ 0;    /* Affinity must be the same on all columns */
    }
    ਜੇ( sqlite3_stricmp(sqlite3_ਕਾਲਮ_Coll(p_Dest_ਖਾਨਾ),
                        sqlite3_ਕਾਲਮ_Coll(p_Src_ਖਾਨਾ))!=0 ){
      ਵਾਪਸ 0;    /* Collating sequence must be the same on all columns */
    }
    ਜੇ( p_Dest_ਖਾਨਾ->notNull && !p_Src_ਖਾਨਾ->notNull ){
      ਵਾਪਸ 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    ਜੇ( (p_Dest_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_GENERATED)==0 && i>0 ){
      ਵਿਅੰਜਕ *p_Dest_ਵਿਅੰਜਕ = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pDest, p_Dest_ਖਾਨਾ);
      ਵਿਅੰਜਕ *p_Src_ਵਿਅੰਜਕ = sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(pSrc, p_Src_ਖਾਨਾ);
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Dest_ਵਿਅੰਜਕ==0 || p_Dest_ਵਿਅੰਜਕ->op==TK_ਵਿਸਤਾਰ );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Dest_ਵਿਅੰਜਕ==0 || !ਵਿਅੰਜਕ_Has_Property(p_Dest_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Src_ਵਿਅੰਜਕ==0 || p_Src_ਵਿਅੰਜਕ->op==TK_ਵਿਸਤਾਰ );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Src_ਵਿਅੰਜਕ==0 || !ਵਿਅੰਜਕ_Has_Property(p_Src_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਜੇ( (p_Dest_ਵਿਅੰਜਕ==0)!=(p_Src_ਵਿਅੰਜਕ==0)
       || (p_Dest_ਵਿਅੰਜਕ!=0 && strcmp(p_Dest_ਵਿਅੰਜਕ->u.zToken,
                                       p_Src_ਵਿਅੰਜਕ->u.zToken)!=0)
      ){
        ਵਾਪਸ 0;    /* Default values must be the same for all columns */
      }
    }
  }
  ਲਈ(pDestIdx=pDest->p_ਸੂਚਕਾਂਕ; pDestIdx; pDestIdx=pDestIdx->p_ਅਗਲਾ){
    ਜੇ( Is_Unique_ਸੂਚਕਾਂਕ(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    ਲਈ(pSrcIdx=pSrc->p_ਸੂਚਕਾਂਕ; pSrcIdx; pSrcIdx=pSrcIdx->p_ਅਗਲਾ){
      ਜੇ( xfer_Compatible_ਸੂਚਕਾਂਕ(pDestIdx, pSrcIdx) ) ਰੋਕੋ;
    }
    ਜੇ( pSrcIdx==0 ){
      ਵਾਪਸ 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    ਜੇ( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      ਵਾਪਸ 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CHECK
  ਜੇ( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    ਵਾਪਸ 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pDest) );
  ਜੇ( (db->flags & SQLITE_Foreign_ਕੁੰਜੀਆਂ)!=0 && pDest->u.tab.pFKey!=0 ){
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਜੇ( (db->flags & SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ)!=0 ){
    ਵਾਪਸ 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( db->xAuth ){
    ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pSrc->pSchema);
    ਜੇ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ਵਾਪਸ 0;
    ਲਈ(i=0; i<pSrc->n_ਖਾਨਾ; i++){
      ਕਾਲਮ *p_Src_ਖਾਨਾ = &pSrc->a_ਖਾਨਾ[i];
      ਜੇ( sqlite3_Auth_Read_ਖਾਨਾ(pParse, pSrc->zName, p_Src_ਖਾਨਾ->zCnName, iDb) ){
        ਵਾਪਸ 0;
      }
    }
  }
#ਜੇ_ਅੰਤ

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  sqlite3_xferopt_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
  iDbSrc = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3_ਕੋਡ_Verify_Schema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  reg_ਅੰਕੜੇ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਅੰਕੜੇ);
  regRowid = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
  sqlite3_Open_ਸਾਰਣੀ(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  ਪ੍ਰਤਿਜ੍ਞਾ( HasRowid(pDest) || destHasUniqueIdx );
  ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->p_ਸੂਚਕਾਂਕ!=0)          /* (1) */
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
    addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iDest, 0); VdbeCoverage(v);
    ਖਾਲੀ_ਥਾਵਾਂ_Dest_Test = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
  }
  ਜੇ( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3_Open_ਸਾਰਣੀ(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    ਖਾਲੀ_ਥਾਵਾਂ_Src_Test = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ਜੇ( pDest->iPKey>=0 ){
      addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iSrc, regRowid);
      ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr2);
      }
      auto_Inc_ਕਦਮ(pParse, regAutoinc, regRowid);
    }ਨਹੀਂਤਾਂ ਜੇ( pDest->p_ਸੂਚਕਾਂਕ==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iDest, regRowid);
    }ਨਹੀਂਤਾਂ{
      addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iSrc, regRowid);
      ਪ੍ਰਤਿਜ੍ਞਾ( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    ਜੇ( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Seek_ਸਿਰਾ, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }ਨਹੀਂਤਾਂ{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪੰਕਤੀ_ਅੰਕੜੇ, iSrc, reg_ਅੰਕੜੇ, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪੰਕਤੀ_Cell, iDest, iSrc, regRowid);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iDest, reg_ਅੰਕੜੇ, regRowid);
    ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ*)pDest, P4_ਸਾਰਣੀ);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iSrc, addr1); VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, iDest, 0);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  ਲਈ(pDestIdx=pDest->p_ਸੂਚਕਾਂਕ; pDestIdx; pDestIdx=pDestIdx->p_ਅਗਲਾ){
    u8 idxInsFlags = 0;
    ਲਈ(pSrcIdx=pSrc->p_ਸੂਚਕਾਂਕ; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->p_ਅਗਲਾ){
      ਜੇ( xfer_Compatible_ਸੂਚਕਾਂਕ(pDestIdx, pSrcIdx) ) ਰੋਕੋ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrcIdx );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    ਜੇ( db->mDbFlags & DBFLAG_Vacuum ){
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
      ਲਈ(i=0; i<pSrcIdx->n_ਕਾਲਮ; i++){
        ਅਚਲ ਅੱਖਰ *zColl = pSrcIdx->azColl[i];
        ਜੇ( sqlite3_stricmp(sqlite3_Str_ਦੁਆਧਾਰੀ, zColl) ) ਰੋਕੋ;
      }
      ਜੇ( i==pSrcIdx->n_ਕਾਲਮ ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Seek_ਸਿਰਾ, iDest);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਪੰਕਤੀ_Cell, iDest, iSrc);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    ਜੇ( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪੰਕਤੀ_ਅੰਕੜੇ, iSrc, reg_ਅੰਕੜੇ, 1);
      ਜੇ( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pDestIdx)
      ){
        ਕੋਡ_Without_Rowid_Preupdate(pParse, pDest, iDest, reg_ਅੰਕੜੇ);
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Idx_ਪਾਓ, iDest, reg_ਅੰਕੜੇ);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, iDest, 0);
  }
  ਜੇ( ਖਾਲੀ_ਥਾਵਾਂ_Src_Test ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, ਖਾਲੀ_ਥਾਵਾਂ_Src_Test);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regRowid);
  sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਅੰਕੜੇ);
  ਜੇ( ਖਾਲੀ_ਥਾਵਾਂ_Dest_Test ){
    sqlite3_Autoincrement_ਸਿਰਾ(pParse);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, ਖਾਲੀ_ਥਾਵਾਂ_Dest_Test);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Close, iDest, 0);
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 1;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_XFER_OPT */
