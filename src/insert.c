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
#ضمن "sqliteInt.h"

/*
** Generate code that will
**
**   (1) acquire a lock for table pTab then
**   (2) open pTab as cursor iCur.
**
** If pTab is a WITHOUT ROWID table, then it is the PRIMARY KEY index
** for that table that is actually opened.
*/
فراغ sqlite3_Open_جدول(
  Parse *pParse,  /* Generate code into this VDBE */
  صحيح iCur,       /* The cursor number of the table */
  صحيح iDb,        /* The database index in sqlite3.aDb[] */
  جدول *pTab,    /* The table to be opened */
  صحيح opcode      /* OP_OpenRead or OP_OpenWrite */
){
  Vdbe *v;
  أكد( !IsVirtual(pTab) );
  أكد( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  أكد( opcode==OP_OpenWrite || opcode==OP_OpenRead );
  إذا( !pParse->db->noSharedCache ){
    sqlite3_جدول_جندرا(pParse, iDb, pTab->tnum,
                     (opcode==OP_OpenWrite)?1:0, pTab->zName);
  }
  إذا( HasRowid(pTab) ){
    sqlite3_Vdbe_جوڑو_Op4_Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
    VdbeComment((v, "%s", pTab->zName));
  }وإلا{
    اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
    أكد( pPk!=0 );
    أكد( pPk->tnum==pTab->tnum || CORRUPT_DB );
    sqlite3_Vdbe_جوڑو_Op3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pPk);
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
ساكن SQLITE_NOINLINE ثابت حرف *compute_اشاریہ_Aff_Str(sqlite3 *db, اشاریہ *pIdx){
  /* The first time a column affinity string for a particular index is
  ** required, it is allocated and populated here. It is then stored as
  ** a member of the Index structure for subsequent use.
  **
  ** The column affinity string will eventually be deleted by
  ** sqliteDeleteIndex() when the Index structure itself is cleaned
  ** up.
  */
  صحيح n;
  جدول *pTab = pIdx->p_جدول;
  pIdx->z_خانہ_Aff = (حرف *)sqlite3DbMallocRaw(0, pIdx->n_کالم+1);
  إذا( !pIdx->z_خانہ_Aff ){
    sqlite3OomFault(db);
    أرجع 0;
  }
  لكل(n=0; n<pIdx->n_کالم; n++){
    i16 x = pIdx->ai_کالم[n];
    حرف aff;
    إذا( x>=0 ){
      aff = pTab->a_خانہ[x].affinity;
    }وإلا إذا( x==XN_ROWID ){
      aff = SQLITE_AFF_INTEGER;
    }وإلا{
      أكد( x==XN_اظہاریہ );
      أكد( pIdx->b_Has_اظہاریہ );
      أكد( pIdx->a_خانہ_اظہاریہ!=0 );
      aff = sqlite3_اظہاریہ_Affinity(pIdx->a_خانہ_اظہاریہ->a[n].p_اظہاریہ);
    }
    إذا( aff<SQLITE_AFF_BLOB ) aff = SQLITE_AFF_BLOB;
    إذا( aff>SQLITE_AFF_NUMERIC) aff = SQLITE_AFF_NUMERIC;
    pIdx->z_خانہ_Aff[n] = aff;
  }
  pIdx->z_خانہ_Aff[n] = 0;
  أرجع pIdx->z_خانہ_Aff;
}
ثابت حرف *sqlite3_اشاریہ_Affinity_Str(sqlite3 *db, اشاریہ *pIdx){
  إذا( !pIdx->z_خانہ_Aff ) أرجع compute_اشاریہ_Aff_Str(db, pIdx);
  أرجع pIdx->z_خانہ_Aff;
}


/*
** Compute an affinity string for a table.   Space is obtained
** from sqlite3DbMalloc().  The caller is responsible for freeing
** the space when done.
*/
حرف *sqlite3_جدول_Affinity_Str(sqlite3 *db, ثابت جدول *pTab){
  حرف *z_خانہ_Aff;
  z_خانہ_Aff = (حرف *)sqlite3DbMallocRaw(db, pTab->n_خانہ+1);
  إذا( z_خانہ_Aff ){
    صحيح i, j;
    لكل(i=j=0; i<pTab->n_خانہ; i++){
      إذا( (pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL)==0 ){
        z_خانہ_Aff[j++] = pTab->a_خانہ[i].affinity;
      }
    }
    افعل{
      z_خانہ_Aff[j--] = 0;
    }طالما( j>=0 && z_خانہ_Aff[j]<=SQLITE_AFF_BLOB );
  }
  أرجع z_خانہ_Aff; 
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
فراغ sqlite3_جدول_Affinity(Vdbe *v, جدول *pTab, صحيح iReg){
  صحيح i;
  حرف *z_خانہ_Aff;
  إذا( pTab->tabFlags & TF_Strict ){
    إذا( iReg==0 ){
      /* Move the previous opcode (which should be OP_MakeRecord) forward
      ** by one slot and insert a new OP_TypeCheck where the current
      ** OP_MakeRecord is found */
      VdbeOp *pPrev;
      صحيح p3;
      sqlite3VdbeAppendP4(v, pTab, P4_جدول);
      pPrev = sqlite3_Vdbe_Get_آخری_Op(v);
      أكد( pPrev!=0 );
      أكد( pPrev->opcode==OP_Make_ریکارڈ || sqlite3VdbeDb(v)->mallocFailed );
      pPrev->opcode = OP_TypeCheck;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, pPrev->p1, pPrev->p2, p3);
    }وإلا{
      /* Insert an isolated OP_Typecheck */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_TypeCheck, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, P4_جدول);
    }
    أرجع;
  }
  z_خانہ_Aff = pTab->z_خانہ_Aff;
  إذا( z_خانہ_Aff==0 ){
    z_خانہ_Aff = sqlite3_جدول_Affinity_Str(0, pTab);
    إذا( !z_خانہ_Aff ){
      sqlite3OomFault(sqlite3VdbeDb(v));
      أرجع;
    }
    pTab->z_خانہ_Aff = z_خانہ_Aff;
  }
  أكد( z_خانہ_Aff!=0 );
  i = sqlite3Strlen30NN(z_خانہ_Aff);
  إذا( i ){
    إذا( iReg ){
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, iReg, i, 0, z_خانہ_Aff, i);
    }وإلا{
      أكد( sqlite3_Vdbe_Get_آخری_Op(v)->opcode==OP_Make_ریکارڈ
              || sqlite3VdbeDb(v)->mallocFailed );
      sqlite3VdbeChangeP4(v, -1, z_خانہ_Aff, i);
    }
  }
}

/*
** Return non-zero if the table pTab in database iDb or any of its indices
** have been opened at any point in the VDBE program. This is used to see if
** a statement of the form  "INSERT INTO <iDb, pTab> SELECT ..." can
** run without using a temporary table for the results of the SELECT.
*/
ساكن صحيح reads_جدول(Parse *p, صحيح iDb, جدول *pTab){
  Vdbe *v = sqlite3GetVdbe(p);
  صحيح i;
  صحيح i_سرا = sqlite3VdbeCurrentAddr(v);
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  VTable *pVTab = IsVirtual(pTab) ? sqlite3GetVTable(p->db, pTab) : 0;
#جے_انت

  لكل(i=1; i<i_سرا; i++){
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);
    أكد( pOp!=0 );
    إذا( pOp->opcode==OP_OpenRead && pOp->p3==iDb ){
      اشاریہ *p_اشاریہ;
      Pgno tnum = pOp->p2;
      إذا( tnum==pTab->tnum ){
        أرجع 1;
      }
      لكل(p_اشاریہ=pTab->p_اشاریہ; p_اشاریہ; p_اشاریہ=p_اشاریہ->p_اگلا){
        إذا( tnum==p_اشاریہ->tnum ){
          أرجع 1;
        }
      }
    }
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
    إذا( pOp->opcode==OP_VOpen && pOp->p4.pVtab==pVTab ){
      أكد( pOp->p4.pVtab!=0 );
      أكد( pOp->p4type==P4_VTAB );
      أرجع 1;
    }
#جے_انت
  }
  أرجع 0;
}

/* This walker callback will compute the union of colFlags flags for all
** referenced columns in a CHECK constraint or generated column expression.
*/
ساكن صحيح اظہاریہ_کالم_Flag_Union(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_کالم && p_اظہاریہ->i_کالم>=0 ){
    أكد( p_اظہاریہ->i_کالم < pWalker->u.pTab->n_خانہ );
    pWalker->e_کوڈ |= pWalker->u.pTab->a_خانہ[p_اظہاریہ->i_کالم].خانہ_Flags;
  }
  أرجع WRC_Continue;
}

#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
/*
** All regular columns for table pTab have been puts into registers
** starting with iRegStore.  The registers that correspond to STORED
** or VIRTUAL columns have not yet been initialized.  This routine goes
** back and computes the values for those columns based on the previously
** computed normal columns.
*/
فراغ sqlite3ComputeGeneratedColumns(
  Parse *pParse,    /* Parsing context */
  صحيح iRegStore,    /* Register holding the first column */
  جدول *pTab       /* The table */
){
  صحيح i;
  Walker w;
  کالم *pRedo;
  صحيح e_ترقی;
  VdbeOp *pOp;

  أكد( pTab->tabFlags & TF_HasGenerated );
  testcase( pTab->tabFlags & TF_HasVirtual );
  testcase( pTab->tabFlags & TF_HasStored );

  /* Before computing generated columns, first go through and make sure
  ** that appropriate affinity has been applied to the regular columns
  */
  sqlite3_جدول_Affinity(pParse->pVdbe, pTab, iRegStore);
  إذا( (pTab->tabFlags & TF_HasStored)!=0 ){
    pOp = sqlite3_Vdbe_Get_آخری_Op(pParse->pVdbe);
    إذا( pOp->opcode==OP_Affinity ){
      /* Change the OP_Affinity argument to '@' (NONE) for all stored
      ** columns.  '@' is the no-op affinity and those columns have not
      ** yet been computed. */
      صحيح ii, jj;
      حرف *zP4 = pOp->p4.z;
      أكد( zP4!=0 );
      أكد( pOp->p4type==P4_DYNAMIC );
      لكل(ii=jj=0; zP4[jj]; ii++){
        إذا( pTab->a_خانہ[ii].خانہ_Flags & COLFLAG_VIRTUAL ){
          استمر;
        }
        إذا( pTab->a_خانہ[ii].خانہ_Flags & COLFLAG_STORED ){
          zP4[jj] = SQLITE_AFF_NONE;
        }
        jj++;
      }
    }وإلا إذا( pOp->opcode==OP_TypeCheck ){
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
  لكل(i=0; i<pTab->n_خانہ; i++){
    إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED ){
      testcase( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL );
      testcase( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_STORED );
      pTab->a_خانہ[i].خانہ_Flags |= COLFLAG_NOTAVAIL;
    }
  }

  w.u.pTab = pTab;
  w.x_اظہاریہ_Callback = اظہاریہ_کالم_Flag_Union;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  /* On the second pass, compute the value of each NOT-AVAILABLE column.
  ** Companion code in the TK_COLUMN case of sqlite3ExprCodeTarget() will
  ** compute dependencies and mark remove the COLSPAN_NOTAVAIL mark, as
  ** they are needed.
  */
  pParse->iSelfTab = -iRegStore;
  افعل{
    e_ترقی = 0;
    pRedo = 0;
    لكل(i=0; i<pTab->n_خانہ; i++){
      کالم *p_خانہ = pTab->a_خانہ + i;
      إذا( (p_خانہ->خانہ_Flags & COLFLAG_NOTAVAIL)!=0 ){
        صحيح x;
        p_خانہ->خانہ_Flags |= COLFLAG_BUSY;
        w.e_کوڈ = 0;
        sqlite3_Walk_اظہاریہ(&w, sqlite3_کالم_اظہاریہ(pTab, p_خانہ));
        p_خانہ->خانہ_Flags &= ~COLFLAG_BUSY;
        إذا( w.e_کوڈ & COLFLAG_NOTAVAIL ){
          pRedo = p_خانہ;
          استمر;
        }
        e_ترقی = 1;
        أكد( p_خانہ->خانہ_Flags & COLFLAG_GENERATED );
        x = sqlite3_جدول_کالم_منزل_Storage(pTab, i) + iRegStore;
        sqlite3_اظہاریہ_کوڈ_Generated_کالم(pParse, pTab, p_خانہ, x);
        p_خانہ->خانہ_Flags &= ~COLFLAG_NOTAVAIL;
      }
    }
  }طالما( pRedo && e_ترقی );
  إذا( pRedo ){
    sqlite3_Error_پیغام(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}
#جے_انت /* SQLITE_OMIT_GENERATED_COLUMNS */


#جے_بنا_تعریف SQLITE_OMIT_AUTOINCREMENT
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
ساكن صحيح autoIncBegin(
  Parse *pParse,      /* Parsing context */
  صحيح iDb,            /* Index of the database holding pTab */
  جدول *pTab         /* The table we are writing to */
){
  صحيح memId = 0;      /* Register holding maximum rowid */
  أكد( pParse->db->aDb[iDb].pSchema!=0 );
  إذا( (pTab->tabFlags & TF_Autoincrement)!=0
   && (pParse->db->mDbFlags & DBFLAG_Vacuum)==0
  ){
    Parse *pToplevel = sqlite3ParseToplevel(pParse);
    AutoincInfo *pInfo;
    جدول *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    /* Verify that the sqlite_sequence table exists and is an ordinary
    ** rowid table with exactly two columns.
    ** Ticket d8dc2b3a58cd5dc2918a1d4acb 2018-05-23 */
    إذا( pSeqTab==0
     || !HasRowid(pSeqTab)
     || NEVER(IsVirtual(pSeqTab))
     || pSeqTab->n_خانہ!=2
    ){
      pParse->nErr++;
      pParse->rc = SQLITE_CORRUPT_SEQUENCE;
      أرجع 0;
    }

    إذا( pToplevel->ورتدا_Ainc==0 ){
      pToplevel->pAinc = 0;
    }
    pInfo = pToplevel->pAinc;
    طالما( pInfo && pInfo->pTab!=pTab ){ pInfo = pInfo->p_اگلا; }
    إذا( pInfo==0 ){
      pInfo = sqlite3DbMallocRawNN(pParse->db, حجم(*pInfo));
      sqlite3_Parser_جوڑو_Cleanup(pToplevel, sqlite3DbFree, pInfo);
      testcase( pParse->earlyCleanup );
      إذا( pParse->db->mallocFailed ) أرجع 0;
      pInfo->p_اگلا = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pToplevel->ورتدا_Ainc = 1;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;                  /* Register to hold name of table */
      pInfo->regCtr = ++pToplevel->nMem;  /* Max rowid register */
      pToplevel->nMem +=2;       /* Rowid in sqlite_sequence + orig max val */
    }
    memId = pInfo->regCtr;
  }
  أرجع memId;
}

/*
** This routine generates code that will initialize all of the
** register used by the autoincrement tracker. 
*/
فراغ sqlite3AutoincrementBegin(Parse *pParse){
  AutoincInfo *p;            /* Information about an AUTOINCREMENT */
  sqlite3 *db = pParse->db;  /* The database connection */
  Db *pDb;                   /* Database only autoinc table */
  صحيح memId;                 /* Register holding max rowid */
  Vdbe *v = pParse->pVdbe;   /* VDBE under construction */

  /* This routine is never called during trigger-generation.  It is
  ** only called from the top-level */
  أكد( pParse->pTriggerTab==0 );
  أكد( sqlite3IsToplevel(pParse) );

  أكد( v );   /* We failed long ago if this is not so */
  أكد( pParse->ورتدا_Ainc );
  لكل(p = pParse->pAinc; p; p = p->p_اگلا){
    ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
    ساكن ثابت VdbeOpList autoInc[] = {
      /* 0  */ {OP_Null,    0,  0, 0},
      /* 1  */ {OP_Rewind,  0, 10, 0},
      /* 2  */ {OP_کالم,  0,  0, 0},
      /* 3  */ {OP_Ne,      0,  9, 0},
      /* 4  */ {OP_Rowid,   0,  0, 0},
      /* 5  */ {OP_کالم,  0,  1, 0},
      /* 6  */ {OP_جوڑو_Imm,  0,  0, 0},
      /* 7  */ {OP_Copy,    0,  0, 0},
      /* 8  */ {OP_Goto,    0, 11, 0},
      /* 9  */ {OP_اگلا,    0,  2, 0},
      /* 10 */ {OP_Integer, 0,  0, 0},
      /* 11 */ {OP_Close,   0,  0, 0}
    };
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;
    أكد( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Open_جدول(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenRead);
    sqlite3_Vdbe_لدو_String(v, memId-1, p->pTab->zName);
    aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(autoInc), autoInc, iLn);
    إذا( aOp==0 ) اكسر;
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
    إذا( pParse->nTab==0 ) pParse->nTab = 1;
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
ساكن فراغ auto_Inc_قدم(Parse *pParse, صحيح memId, صحيح regRowid){
  إذا( memId>0 ){
    sqlite3_Vdbe_جوڑو_Op2(pParse->pVdbe, OP_MemMax, memId, regRowid);
  }
}

/*
** This routine generates the code needed to write autoincrement
** maximum rowid values back into the sqlite_sequence register.
** Every statement that might do an INSERT into an autoincrement
** table (either directly or through triggers) needs to call this
** routine just before the "exit" code.
*/
ساكن SQLITE_NOINLINE فراغ auto_Increment_سرا(Parse *pParse){
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  أكد( v );
  أكد( pParse->ورتدا_Ainc );
  لكل(p = pParse->pAinc; p; p = p->p_اگلا){
    ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
    ساكن ثابت VdbeOpList auto_Inc_سرا[] = {
      /* 0 */ {OP_NotNull,     0, 2, 0},
      /* 1 */ {OP_NewRowid,    0, 0, 0},
      /* 2 */ {OP_Make_ریکارڈ,  0, 2, 0},
      /* 3 */ {OP_داخل_کرو,      0, 0, 0},
      /* 4 */ {OP_Close,       0, 0, 0}
    };
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    صحيح iRec;
    صحيح memId = p->regCtr;

    iRec = sqlite3_Get_عارضی_Reg(pParse);
    أكد( sqlite3SchemaMutexHeld(db, 0, pDb->pSchema) );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Le, memId+2, sqlite3VdbeCurrentAddr(v)+7, memId);
    VdbeCoverage(v);
    sqlite3_Open_جدول(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, OP_OpenWrite);
    aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(auto_Inc_سرا), auto_Inc_سرا, iLn);
    إذا( aOp==0 ) اكسر;
    aOp[0].p1 = memId+1;
    aOp[1].p2 = memId+1;
    aOp[2].p1 = memId-1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId+1;
    aOp[3].p5 = OPFLAG_APPEND;
    sqlite3_Release_عارضی_Reg(pParse, iRec);
  }
}
فراغ sqlite3_Autoincrement_سرا(Parse *pParse){
  إذا( pParse->ورتدا_Ainc ) auto_Increment_سرا(pParse);
}
#وإلا
/*
** If SQLITE_OMIT_AUTOINCREMENT is defined, then the three routines
** above are all no-ops
*/
# define autoIncBegin(A,B,C) (0)
# define auto_Inc_قدم(A,B,C)
#جے_انت /* SQLITE_OMIT_AUTOINCREMENT */

/*
** If argument pVal is a Select object returned by an sqlite3MultiValues()
** that was able to use the co-routine optimization, finish coding the
** co-routine.
*/
فراغ sqlite3_Multi_اقدار_سرا(Parse *pParse, Select *pVal){
  إذا( ALWAYS(pVal) && pVal->pSrc->nSrc>0 ){
    Src_شے *p_شے = &pVal->pSrc->a[0];
    أكد( (p_شے->fg.isSubquery && p_شے->u4.pSubq!=0) || pParse->nErr );
    إذا( p_شے->fg.isSubquery ){
      sqlite3_Vdbe_سرا_Coroutine(pParse->pVdbe, p_شے->u4.pSubq->regReturn);
      sqlite3_Vdbe_Jump_ایتھے(pParse->pVdbe, p_شے->u4.pSubq->addrFillSub - 1);
    }
  }
}

/*
** Return true if all expressions in the expression-list passed as the
** only argument are both constant and have no affinity.
*/
ساكن صحيح اظہاریہ_List_Is_No_Affinity(Parse *pParse, اظہاریہ_List *p_صف){
  صحيح ii;
  إذا( sqlite3_اظہاریہ_List_Is_Constant(pParse, p_صف, 0)==0 ) أرجع 0;
  لكل(ii=0; ii<p_صف->n_اظہاریہ; ii++){
    اظہاریہ *p_اظہاریہ = p_صف->a[ii].p_اظہاریہ;
    أكد( p_اظہاریہ->op!=TK_RAISE );
    أكد( p_اظہاریہ->aff_اظہاریہ==0 );
    إذا( 0!=sqlite3_اظہاریہ_Affinity(p_اظہاریہ) ) أرجع 0;
  }
  أرجع 1;

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
Select *sqlite3_Multi_اقدار(Parse *pParse, Select *p_بایاں, اظہاریہ_List *p_صف){

  إذا( pParse->bHasWith                   /* condition (a) above */
   || pParse->db->init.busy              /* condition (b) above */
   || sqlite3_اظہاریہ_List_Is_Constant(pParse, p_صف, 1)==0   /* condition (c) above */
   || (p_بایاں->pSrc->nSrc==0 &&
       اظہاریہ_List_Is_No_Affinity(pParse,p_بایاں->pEList)==0) /* condition (d) above */
   || وچ_SPECIAL_PARSE
  ){
    /* The co-routine method cannot be used. Fall back to UNION ALL. */
    Select *pSelect = 0;
    صحيح f = SF_اقدار | SF_Multi_قدر;
    إذا( p_بایاں->pSrc->nSrc ){
      sqlite3_Multi_اقدار_سرا(pParse, p_بایاں);
      f = SF_اقدار;
    }وإلا إذا( p_بایاں->p_پہلوں ){
      /* In this case set the SF_MultiValue flag only if it was set on pLeft */
      f = (f & p_بایاں->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, p_صف, 0, 0, 0, 0, 0, f, 0);
    p_بایاں->selFlags &= ~(u32)SF_Multi_قدر;
    إذا( pSelect ){
      pSelect->op = TK_ALL;
      pSelect->p_پہلوں = p_بایاں;
      p_بایاں = pSelect;
    }
  }وإلا{
    Src_شے *p = 0;               /* SrcItem that reads from co-routine */

    إذا( p_بایاں->pSrc->nSrc==0 ){
      /* Co-routine has not yet been started and the special Select object
      ** that accesses the co-routine has not yet been created. This block 
      ** does both those things. */
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      /* Ensure the database schema has been read. This is to ensure we have
      ** the correct text encoding.  */
      إذا( (pParse->db->mDbFlags & DBFLAG_Schema_معلوم_Ok)==0 ){
        sqlite3ReadSchema(pParse);
      }

      إذا( pRet ){
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->p_پہلوں = p_بایاں->p_پہلوں;
        pRet->op = p_بایاں->op;
        إذا( pRet->p_پہلوں ) pRet->selFlags |= SF_اقدار;
        p_بایاں->p_پہلوں = 0;
        p_بایاں->op = TK_SELECT;
        أكد( p_بایاں->p_اگلا==0 );
        أكد( pRet->p_اگلا==0 );
        p = &pRet->pSrc->a[0];
        p->fg.راہیں_Coroutine = 1;
        p->iCursor = -1;
        أكد( !p->fg.is_اشاریہ_دار_By && !p->fg.isTabFunc );
        p->u1.n_صف = 2;
        إذا( sqlite3_Src_شے_Attach_Subquery(pParse, p, p_بایاں, 0) ){
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine,
                            pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);

          /* Allocate registers for the output of the co-routine. Do so so
          ** that there are two unused registers immediately before those
          ** used by the co-routine. This allows the code in sqlite3Insert()
          ** to use these registers directly, instead of copying the output
          ** of the co-routine to a separate array for processing.  */
          dest.iSdst = pParse->nMem + 3; 
          dest.nSdst = p_بایاں->pEList->n_اظہاریہ;
          pParse->nMem += 2 + dest.nSdst;

          p_بایاں->selFlags |= SF_Multi_قدر;
          sqlite3Select(pParse, p_بایاں, &dest);
          pSubq->reg_نتیجہ = dest.iSdst;
          أكد( pParse->nErr || dest.iSdst>0 );
        }
        p_بایاں = pRet;
      }
    }وإلا{
      p = &p_بایاں->pSrc->a[0];
      أكد( !p->fg.isTabFunc && !p->fg.is_اشاریہ_دار_By );
      p->u1.n_صف++;
    }
  
    إذا( pParse->nErr==0 ){
      Subquery *pSubq;
      أكد( p!=0 );
      أكد( p->fg.isSubquery );
      pSubq = p->u4.pSubq;
      أكد( pSubq!=0 );
      أكد( pSubq->pSelect!=0 );
      أكد( pSubq->pSelect->pEList!=0 );
      إذا( pSubq->pSelect->pEList->n_اظہاریہ!=p_صف->n_اظہاریہ ){
        sqlite3_Select_Wrong_عدد_Terms_Error(pParse, pSubq->pSelect);
      }وإلا{
        sqlite3_اظہاریہ_کوڈ_اظہاریہ_List(pParse, p_صف, pSubq->reg_نتیجہ, 0, 0);
        sqlite3_Vdbe_جوڑو_Op1(pParse->pVdbe, OP_Yield, pSubq->regReturn);
      }
    }
    sqlite3_اظہاریہ_List_Delete(pParse->db, p_صف);
  }

  أرجع p_بایاں;
}

/* Forward declaration */
ساكن صحيح xferOptimization(
  Parse *pParse,        /* Parser context */
  جدول *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  صحيح onError,          /* How to handle constraint errors */
  صحيح iDbDest           /* The database of pDest */
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
فراغ sqlite3_داخل_کرو(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *p_کالم,      /* Column names corresponding to IDLIST, or NULL. */
  صحيح onError,          /* How to handle constraint errors */
  Upsert *pUpsert       /* ON CONFLICT clauses for upsert, or NULL */
){
  sqlite3 *db;          /* The main database structure */
  جدول *pTab;          /* The table to insert into.  aka TABLE */
  صحيح i, j;             /* Loop counters */
  Vdbe *v;              /* Generate code into this virtual machine */
  اشاریہ *pIdx;          /* For looping over indices of the table */
  صحيح n_کالم;          /* Number of columns in the data */
  صحيح nHidden = 0;      /* Number of hidden columns if TABLE is virtual */
  صحيح i_انکڑے_Cur = 0;     /* VDBE cursor that is the main data repository */
  صحيح iIdxCur = 0;      /* First index cursor */
  صحيح ipk_کالم = -1;   /* Column that is the INTEGER PRIMARY KEY */
  صحيح سرا_دا_Loop;        /* Label for the end of the insertion loop */
  صحيح srcTab = 0;       /* Data comes from this temporary cursor if >=0 */
  صحيح addr_Ins_چوٹی = 0;   /* Jump to label "D" */
  صحيح addrCont = 0;     /* Top of insert loop. Label "C" in templates 3 and 4 */
  SelectDest dest;      /* Destination for SELECT on rhs of INSERT */
  صحيح iDb;              /* Index of database holding TABLE */
  u8 use_عارضی_جدول = 0;  /* Store SELECT results in intermediate table */
  u8 appendFlag = 0;    /* True if the insert is likely to be an append */
  u8 withoutRowid;      /* 0 for normal table.  1 for WITHOUT ROWID table */
  u8 b_Id_List_وچ_ترتیب;    /* True if IDLIST is in table order */
  اظہاریہ_List *pList = 0;  /* List of VALUES() to be inserted  */
  صحيح iRegStore;        /* Register in which to store next column */

  /* Register allocations */
  صحيح regFromSelect = 0;/* Base register for data coming from SELECT */
  صحيح regAutoinc = 0;   /* Register holding the AUTOINCREMENT counter */
  صحيح reg_صف_گنتی = 0;  /* Memory cell used for the row counter */
  صحيح regIns;           /* Block of regs holding rowid+data being inserted */
  صحيح regRowid;         /* registers holding insert rowid */
  صحيح reg_انکڑے;          /* register holding first column to insert */
  صحيح *aRegIdx = 0;     /* One register allocated to each index */
  صحيح *a_Tab_خانہ_Map = 0;  /* Mapping from pTab columns to pCol entries */

#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
  صحيح isView;                 /* True if attempting to insert into a view */
  Trigger *pTrigger;          /* List of triggers on pTab, if required */
  صحيح tmask;                  /* Mask of trigger times */
#جے_انت

  db = pParse->db;
  أكد( db->pParse==pParse );
  إذا( pParse->nErr ){
    اذهب داخل_کرو_cleanup;
  }
  أكد( db->mallocFailed==0 );
  dest.iSDParm = 0;  /* Suppress a harmless compiler warning */

  /* If the Select object is really just a simple VALUES() list with a
  ** single row (the common case) then keep that one row of values
  ** and discard the other (unused) parts of the pSelect object
  */
  إذا( pSelect && (pSelect->selFlags & SF_اقدار)!=0 && pSelect->p_پہلوں==0 ){
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  /* Locate the table into which we will be inserting new information.
  */
  أكد( pTabList->nSrc==1 );
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  إذا( pTab==0 ){
    اذهب داخل_کرو_cleanup;
  }
  iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
  أكد( iDb<db->nDb );
  إذا( sqlite3AuthCheck(pParse, SQLITE_داخل_کرو, pTab->zName, 0,
                       db->aDb[iDb].zDbSName) ){
    اذهب داخل_کرو_cleanup;
  }
  withoutRowid = !HasRowid(pTab);

  /* Figure out if we have any triggers and if the table being
  ** inserted into is a view
  */
#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_داخل_کرو, 0, &tmask);
  isView = IsView(pTab);
#وإلا
# define pTrigger 0
# define tmask 0
# define isView 0
#جے_انت
#جے_تعریف_شدہ SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#جے_انت
  أكد( (pTrigger && tmask) || (pTrigger==0 && tmask==0) );

#إذا TREETRACE_ENABLED
  إذا( sqlite3_درخت_Trace & 0x10000 ){
    sqlite3_درخت_View_سطر(0, "In sqlite3Insert() at %s:%d", __FILE__, __LINE__);
    sqlite3_درخت_View_داخل_کرو(pParse->pWith, pTabList, p_کالم, pSelect, pList,
                          onError, pUpsert, pTrigger);
  }
#جے_انت

  /* If pTab is really a view, make sure it has been initialized.
  ** ViewGetColumnNames() is a no-op if pTab is not a view.
  */
  إذا( sqlite3_View_Get_کالم_ناں(pParse, pTab) ){
    اذهب داخل_کرو_cleanup;
  }

  /* Cannot insert into a read-only table.
  */
  إذا( sqlite3IsReadOnly(pParse, pTab, pTrigger) ){
    اذهب داخل_کرو_cleanup;
  }

  /* Allocate a VDBE
  */
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 ) اذهب داخل_کرو_cleanup;
  إذا( pParse->nested==0 ) sqlite3_Vdbe_گنتی_Changes(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

#جے_بنا_تعریف SQLITE_OMIT_XFER_OPT
  /* If the statement is of the form
  **
  **       INSERT INTO <table1> SELECT * FROM <table2>;
  **
  ** Then special optimizations can be applied that make the transfer
  ** very fast and which reduce fragmentation of indices.
  **
  ** This is the 2nd template.
  */
  إذا( p_کالم==0
   && pSelect!=0
   && pTrigger==0
   && xferOptimization(pParse, pTab, pSelect, onError, iDb)
  ){
    أكد( !pTrigger );
    أكد( pList==0 );
    اذهب داخل_کرو_سرا;
  }
#جے_انت /* SQLITE_OMIT_XFER_OPT */

  /* If this is an AUTOINCREMENT table, look up the sequence number in the
  ** sqlite_sequence table and store it in memory cell regAutoinc.
  */
  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  /* Allocate a block registers to hold the rowid and the values
  ** for all columns of the new row.
  */
  regRowid = regIns = pParse->nMem+1;
  pParse->nMem += pTab->n_خانہ + 1;
  إذا( IsVirtual(pTab) ){
    regRowid++;
    pParse->nMem++;
  }
  reg_انکڑے = regRowid+1;

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
  b_Id_List_وچ_ترتیب = (pTab->tabFlags & (TF_OOOHidden|TF_HasStored))==0;
  إذا( p_کالم ){
    a_Tab_خانہ_Map = sqlite3DbMallocZero(db, pTab->n_خانہ*حجم(صحيح));
    إذا( a_Tab_خانہ_Map==0 ) اذهب داخل_کرو_cleanup;
    لكل(i=0; i<p_کالم->nId; i++){
      j = sqlite3_کالم_اشاریہ(pTab, p_کالم->a[i].zName);
      إذا( j>=0 ){
        إذا( a_Tab_خانہ_Map[j]==0 ) a_Tab_خانہ_Map[j] = i+1;
        إذا( i!=j ) b_Id_List_وچ_ترتیب = 0;
        إذا( j==pTab->iPKey ){
          ipk_کالم = i;  أكد( !withoutRowid );
        }
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
        إذا( pTab->a_خانہ[j].خانہ_Flags & (COLFLAG_STORED|COLFLAG_VIRTUAL) ){
          sqlite3_Error_پیغام(pParse,
             "cannot INSERT into generated column \"%s\"",
             pTab->a_خانہ[j].zCnName);
          اذهب داخل_کرو_cleanup;
        }
#جے_انت
      }وإلا{
        إذا( sqlite3IsRowid(p_کالم->a[i].zName) && !withoutRowid ){
          ipk_کالم = i;
          b_Id_List_وچ_ترتیب = 0;
        }وإلا{
          sqlite3_Error_پیغام(pParse, "table %S has no column named %s",
              pTabList->a, p_کالم->a[i].zName);
          pParse->checkSchema = 1;
          اذهب داخل_کرو_cleanup;
        }
      }
    }
  }

  /* Figure out how many columns of data are supplied.  If the data
  ** is coming from a SELECT statement, then generate a co-routine that
  ** produces a single row of the SELECT on each invocation.  The
  ** co-routine is the common header to the 3rd and 4th templates.
  */
  إذا( pSelect ){
    /* Data is coming from a SELECT or from a multi-row VALUES clause.
    ** Generate a co-routine to run the SELECT. */
    صحيح rc;             /* Result code */

    إذا( pSelect->pSrc->nSrc==1 
     && pSelect->pSrc->a[0].fg.راہیں_Coroutine 
     && pSelect->p_پہلوں==0
    ){
      Src_شے *p_شے = &pSelect->pSrc->a[0];
      Subquery *pSubq;
      أكد( p_شے->fg.isSubquery );
      pSubq = p_شے->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->reg_نتیجہ;
      أكد( pSubq->pSelect!=0 );
      أكد( pSubq->pSelect->pEList!=0 );
      n_کالم = pSubq->pSelect->pEList->n_اظہاریہ;
      ExplainQueryPlan((pParse, 0, "SCAN %S", p_شے));
      إذا( b_Id_List_وچ_ترتیب && n_کالم==pTab->n_خانہ ){
        reg_انکڑے = regFromSelect;
        regRowid = reg_انکڑے - 1;
        regIns = regRowid - (IsVirtual(pTab) ? 1 : 0);
      }
    }وإلا{
      صحيح addr_چوٹی;        /* Top of the co-routine */
      صحيح regYield = ++pParse->nMem;
      addr_چوٹی = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine, regYield, 0, addr_چوٹی);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      dest.iSdst = b_Id_List_وچ_ترتیب ? reg_انکڑے : 0;
      dest.nSdst = pTab->n_خانہ;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;
      أكد( db->pParse==pParse );
      إذا( rc || pParse->nErr ) اذهب داخل_کرو_cleanup;
      أكد( db->mallocFailed==0 );
      sqlite3_Vdbe_سرا_Coroutine(v, regYield);
      sqlite3_Vdbe_Jump_ایتھے(v, addr_چوٹی - 1);                       /* label B: */
      أكد( pSelect->pEList );
      n_کالم = pSelect->pEList->n_اظہاریہ;
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
    إذا( pTrigger || reads_جدول(pParse, iDb, pTab) ){
      use_عارضی_جدول = 1;
    }

    إذا( use_عارضی_جدول ){
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
      صحيح regRec;          /* Register to hold packed record */
      صحيح reg_عارضی_Rowid;    /* Register to hold temp table ROWID */
      صحيح addrL;           /* Label "L" */

      srcTab = pParse->nTab++;
      regRec = sqlite3_Get_عارضی_Reg(pParse);
      reg_عارضی_Rowid = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, srcTab, n_کالم);
      addrL = sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, dest.iSDParm); VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regFromSelect, n_کالم, regRec);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, srcTab, reg_عارضی_Rowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, srcTab, regRec, reg_عارضی_Rowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3_Vdbe_Jump_ایتھے(v, addrL);
      sqlite3_Release_عارضی_Reg(pParse, regRec);
      sqlite3_Release_عارضی_Reg(pParse, reg_عارضی_Rowid);
    }
  }وإلا{
    /* This is the case if the data for the INSERT is coming from a
    ** single-row VALUES clause
    */
    NameContext sNC;
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    srcTab = -1;
    أكد( use_عارضی_جدول==0 );
    إذا( pList ){
      n_کالم = pList->n_اظہاریہ;
      إذا( sqlite3_Resolve_اظہاریہ_List_ناں(&sNC, pList) ){
        اذهب داخل_کرو_cleanup;
      }
    }وإلا{
      n_کالم = 0;
    }
  }

  /* If there is no IDLIST term but the table has an integer primary
  ** key, the set the ipkColumn variable to the integer primary key
  ** column index in the original table definition.
  */
  إذا( p_کالم==0 && n_کالم>0 ){
    ipk_کالم = pTab->iPKey;
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
    إذا( ipk_کالم>=0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      لكل(i=ipk_کالم-1; i>=0; i--){
        إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_GENERATED ){
          testcase( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_VIRTUAL );
          testcase( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_STORED );
          ipk_کالم--;
        }
      }
    }
#جے_انت

    /* Make sure the number of columns in the source data matches the number
    ** of columns to be inserted into the table.
    */
    أكد( TF_HasHidden==COLFLAG_HIDDEN );
    أكد( TF_HasGenerated==COLFLAG_GENERATED );
    أكد( COLFLAG_NOINSERT==(COLFLAG_GENERATED|COLFLAG_HIDDEN) );
    إذا( (pTab->tabFlags & (TF_HasGenerated|TF_HasHidden))!=0 ){
      لكل(i=0; i<pTab->n_خانہ; i++){
        إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_NOINSERT ) nHidden++;
      }
    }
    إذا( n_کالم!=(pTab->n_خانہ-nHidden) ){
      sqlite3_Error_پیغام(pParse,
         "table %S has %d columns but %d values were supplied",
         pTabList->a, pTab->n_خانہ-nHidden, n_کالم);
     اذهب داخل_کرو_cleanup;
    }
  }
  إذا( p_کالم!=0 && n_کالم!=p_کالم->nId ){
    sqlite3_Error_پیغام(pParse, "%d values for %d columns", n_کالم, p_کالم->nId);
    اذهب داخل_کرو_cleanup;
  }
   
  /* Initialize the count of rows to be inserted
  */
  إذا( (db->flags & SQLITE_گنتی_صفاں)!=0
   && !pParse->nested
   && !pParse->pTriggerTab
   && !pParse->bReturning
  ){
    reg_صف_گنتی = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, reg_صف_گنتی);
  }

  /* If this is not a view, open the table and and all indices */
  إذا( !isView ){
    صحيح nIdx;
    nIdx = sqlite3_Open_جدول_And_Indices(pParse, pTab, OP_OpenWrite, 0, -1, 0,
                                      &i_انکڑے_Cur, &iIdxCur);
    aRegIdx = sqlite3DbMallocRawNN(db, حجم(صحيح)*(nIdx+2));
    إذا( aRegIdx==0 ){
      اذهب داخل_کرو_cleanup;
    }
    لكل(i=0, pIdx=pTab->p_اشاریہ; i<nIdx; pIdx=pIdx->p_اگلا, i++){
      أكد( pIdx );
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->n_کالم;
    }
    aRegIdx[i] = ++pParse->nMem;  /* Register to store the table record */
  }
#جے_بنا_تعریف SQLITE_OMIT_UPSERT
  إذا( pUpsert ){
    Upsert *pNx;
    إذا( IsVirtual(pTab) ){
      sqlite3_Error_پیغام(pParse, "UPSERT not implemented for virtual table \"%s\"",
              pTab->zName);
      اذهب داخل_کرو_cleanup;
    }
    إذا( IsView(pTab) ){
      sqlite3_Error_پیغام(pParse, "cannot UPSERT a view");
      اذهب داخل_کرو_cleanup;
    }
    إذا( sqlite3HasExplicitNulls(pParse, pUpsert->p_Upsert_نشانہ) ){
      اذهب داخل_کرو_cleanup;
    }
    pTabList->a[0].iCursor = i_انکڑے_Cur;
    pNx = pUpsert;
    افعل{
      pNx->pUpsertSrc = pTabList;
      pNx->reg_انکڑے = reg_انکڑے;
      pNx->i_انکڑے_Cur = i_انکڑے_Cur;
      pNx->iIdxCur = iIdxCur;
      إذا( pNx->p_Upsert_نشانہ ){
        إذا( sqlite3_Upsert_Analyze_نشانہ(pParse, pTabList, pNx, pUpsert) ){
          اذهب داخل_کرو_cleanup;
        }
      }
      pNx = pNx->p_اگلا_Upsert;
    }طالما( pNx!=0 );
  }
#جے_انت


  /* This is the top of the main insertion loop */
  إذا( use_عارضی_جدول ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 4):
    **
    **         rewind temp table, if empty goto D
    **      C: loop over rows of intermediate table
    **           transfer values form intermediate table into <table>
    **         end loop
    **      D: ...
    */
    addr_Ins_چوٹی = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, srcTab); VdbeCoverage(v);
    addrCont = sqlite3VdbeCurrentAddr(v);
  }وإلا إذا( pSelect ){
    /* This block codes the top of loop only.  The complete loop is the
    ** following pseudocode (template 3):
    **
    **      C: yield X, at EOF goto D
    **         insert the select result into <table> from R..R+n
    **         goto C
    **      D: ...
    */
    sqlite3VdbeReleaseRegisters(pParse, reg_انکڑے, pTab->n_خانہ, 0, 0);
    addr_Ins_چوٹی = addrCont = sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, dest.iSDParm);
    VdbeCoverage(v);
    إذا( ipk_کالم>=0 ){
      /* tag-20191021-001: If the INTEGER PRIMARY KEY is being generated by the
      ** SELECT, go ahead and copy the value into the rowid slot now, so that
      ** the value does not get overwritten by a NULL at tag-20191021-002. */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Copy, regFromSelect+ipk_کالم, regRowid);
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
  iRegStore = reg_انکڑے;  أكد( reg_انکڑے==regRowid+1 );
  لكل(i=0; i<pTab->n_خانہ; i++, iRegStore++){
    صحيح k;
    u32 خانہ_Flags;
    أكد( i>=nHidden );
    إذا( i==pTab->iPKey ){
      /* tag-20191021-002: References to the INTEGER PRIMARY KEY are filled
      ** using the rowid. So put a NULL in the IPK slot of the record to avoid
      ** using excess space.  The file format definition requires this extra
      ** NULL - we cannot optimize further by skipping the column completely */
      sqlite3_Vdbe_جوڑو_Op1(v, OP_SoftNull, iRegStore);
      استمر;
    }
    إذا( ((خانہ_Flags = pTab->a_خانہ[i].خانہ_Flags) & COLFLAG_NOINSERT)!=0 ){
      nHidden++;
      إذا( (خانہ_Flags & COLFLAG_VIRTUAL)!=0 ){
        /* Virtual columns do not participate in OP_MakeRecord.  So back up
        ** iRegStore by one slot to compensate for the iRegStore++ in the
        ** outer for() loop */
        iRegStore--;
        استمر;
      }وإلا إذا( (خانہ_Flags & COLFLAG_STORED)!=0 ){
        /* Stored columns are computed later.  But if there are BEFORE
        ** triggers, the slots used for stored columns will be OP_Copy-ed
        ** to a second block of registers, so the register needs to be
        ** initialized to NULL to avoid an uninitialized register read */
        إذا( tmask & TRIGGER_BEFORE ){
          sqlite3_Vdbe_جوڑو_Op1(v, OP_SoftNull, iRegStore);
        }
        استمر;
      }وإلا إذا( p_کالم==0 ){
        /* Hidden columns that are not explicitly named in the INSERT
        ** get their default value */
        sqlite3_اظہاریہ_کوڈ_Factorable(pParse,
            sqlite3_کالم_اظہاریہ_Auth(pTab, &pTab->a_خانہ[i], pParse),
            iRegStore);
        استمر;
      }
    }
    إذا( p_کالم ){
      j = a_Tab_خانہ_Map[i];
      أكد( j>=0 && j<=p_کالم->nId );   
      إذا( j==0 ){
        /* A column not named in the insert column list gets its
        ** default value */
        sqlite3_اظہاریہ_کوڈ_Factorable(pParse,
            sqlite3_کالم_اظہاریہ_Auth(pTab, &pTab->a_خانہ[i], pParse),
            iRegStore);
        استمر;
      }
      k = j - 1;
    }وإلا إذا( n_کالم==0 ){
      /* This is INSERT INTO ... DEFAULT VALUES.  Load the default value. */
      sqlite3_اظہاریہ_کوڈ_Factorable(pParse,
          sqlite3_کالم_اظہاریہ_Auth(pTab, &pTab->a_خانہ[i], pParse),
          iRegStore);
      استمر;
    }وإلا{
      k = i - nHidden;
    }

    إذا( use_عارضی_جدول ){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, srcTab, k, iRegStore);
    }وإلا إذا( pSelect ){
      إذا( regFromSelect!=reg_انکڑے ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, regFromSelect+k, iRegStore);
      }
    }وإلا{
      اظہاریہ *pX = pList->a[k].p_اظہاریہ;
      صحيح y = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, pX, iRegStore);
      إذا( y!=iRegStore ){
        sqlite3_Vdbe_جوڑو_Op2(v,
          اظہاریہ_Has_Property(pX, EP_Subquery) ? OP_Copy : OP_SCopy, y, iRegStore);
      }
    }
  }


  /* Run the BEFORE and INSTEAD OF triggers, if there are any
  */
  سرا_دا_Loop = sqlite3VdbeMakeLabel(pParse);
  إذا( tmask & TRIGGER_BEFORE ){
    صحيح reg_خانے_گنتی = sqlite3_Get_عارضی_Range(pParse, pTab->n_خانہ+1);

    /* build the NEW.* reference row.  Note that if there is an INTEGER
    ** PRIMARY KEY into which a NULL is being inserted, that NULL will be
    ** translated into a unique ID for the row.  But on a BEFORE trigger,
    ** we do not know what the unique ID will be (because the insert has
    ** not happened yet) so we substitute a rowid of -1
    */
    إذا( ipk_کالم<0 ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, -1, reg_خانے_گنتی);
    }وإلا{
      صحيح addr1;
      أكد( !withoutRowid );
      إذا( use_عارضی_جدول ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, srcTab, ipk_کالم, reg_خانے_گنتی);
      }وإلا{
        أكد( pSelect==0 );  /* Otherwise useTempTable is true */
        sqlite3_اظہاریہ_کوڈ(pParse, pList->a[ipk_کالم].p_اظہاریہ, reg_خانے_گنتی);
      }
      addr1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_NotNull, reg_خانے_گنتی); VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, -1, reg_خانے_گنتی);
      sqlite3_Vdbe_Jump_ایتھے(v, addr1);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, reg_خانے_گنتی); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
    }

    /* Copy the new data already generated. */
    أكد( pTab->nNVCol>0 || pParse->nErr>0 );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, regRowid+1, reg_خانے_گنتی+1, pTab->nNVCol-1);

#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** refers to the ROWID. */
    إذا( pTab->tabFlags & TF_HasGenerated ){
      testcase( pTab->tabFlags & TF_HasVirtual );
      testcase( pTab->tabFlags & TF_HasStored );
      sqlite3ComputeGeneratedColumns(pParse, reg_خانے_گنتی+1, pTab);
    }
#جے_انت

    /* If this is an INSERT on a view with an INSTEAD OF INSERT trigger,
    ** do not attempt any conversions before assembling the record.
    ** If this is a real table, attempt conversions as required by the
    ** table column affinities.
    */
    إذا( !isView ){
      sqlite3_جدول_Affinity(v, pTab, reg_خانے_گنتی+1);
    }

    /* Fire BEFORE or INSTEAD OF triggers */
    sqlite3_کوڈ_صف_Trigger(pParse, pTrigger, TK_داخل_کرو, 0, TRIGGER_BEFORE,
        pTab, reg_خانے_گنتی-pTab->n_خانہ-1, onError, سرا_دا_Loop);

    sqlite3_Release_عارضی_Range(pParse, reg_خانے_گنتی, pTab->n_خانہ+1);
  }

  إذا( !isView ){
    إذا( IsVirtual(pTab) ){
      /* The row that the VUpdate opcode will delete: none */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, regIns);
    }
    إذا( ipk_کالم>=0 ){
      /* Compute the new rowid */
      إذا( use_عارضی_جدول ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, srcTab, ipk_کالم, regRowid);
      }وإلا إذا( pSelect ){
        /* Rowid already initialized at tag-20191021-001 */
      }وإلا{
        اظہاریہ *pIpk = pList->a[ipk_کالم].p_اظہاریہ;
        إذا( pIpk->op==TK_NULL && !IsVirtual(pTab) ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_NewRowid, i_انکڑے_Cur, regRowid, regAutoinc);
          appendFlag = 1;
        }وإلا{
          sqlite3_اظہاریہ_کوڈ(pParse, pList->a[ipk_کالم].p_اظہاریہ, regRowid);
        }
      }
      /* If the PRIMARY KEY expression is NULL, then use OP_NewRowid
      ** to generate a unique primary key value.
      */
      إذا( !appendFlag ){
        صحيح addr1;
        إذا( !IsVirtual(pTab) ){
          addr1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_NotNull, regRowid); VdbeCoverage(v);
          sqlite3_Vdbe_جوڑو_Op3(v, OP_NewRowid, i_انکڑے_Cur, regRowid, regAutoinc);
          sqlite3_Vdbe_Jump_ایتھے(v, addr1);
        }وإلا{
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, regRowid, addr1+2); VdbeCoverage(v);
        }
        sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, regRowid); VdbeCoverage(v);
        sqlite3MayAbort(pParse);
      }
    }وإلا إذا( IsVirtual(pTab) || withoutRowid ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, regRowid);
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_NewRowid, i_انکڑے_Cur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    auto_Inc_قدم(pParse, regAutoinc, regRowid);

#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
    /* Compute the new value for generated columns after all other
    ** columns have already been computed.  This must be done after
    ** computing the ROWID in case one of the generated columns
    ** is derived from the INTEGER PRIMARY KEY. */
    إذا( pTab->tabFlags & TF_HasGenerated ){
      sqlite3ComputeGeneratedColumns(pParse, regRowid+1, pTab);
    }
#جے_انت

    /* Generate code to check constraints and generate index keys and
    ** do the insertion.
    */
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
    إذا( IsVirtual(pTab) ){
      ثابت حرف *pVTab = (ثابت حرف *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_VUpdate, 1, pTab->n_خانہ+2, regIns, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, onError==OE_Default ? OE_Abort : onError);
      sqlite3MayAbort(pParse);
    }وإلا
#جے_انت
    {
      صحيح isReplace = 0;/* Set to true if constraints may cause a replace */
      صحيح bUseSeek;     /* True to use OPFLAG_SEEKRESULT */
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, i_انکڑے_Cur, iIdxCur,
          regIns, 0, ipk_کالم>=0, onError, سرا_دا_Loop, &isReplace, 0, pUpsert
      );
      إذا( db->flags & SQLITE_Foreign_کلیدیں ){
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
      bUseSeek = (isReplace==0 || !sqlite3_Vdbe_Has_Sub_پروگرام(v));
      sqlite3CompleteInsertion(pParse, pTab, i_انکڑے_Cur, iIdxCur,
          regIns, aRegIdx, 0, appendFlag, bUseSeek
      );
    }
#جے_تعریف_شدہ SQLITE_ALLOW_ROWID_وچ_VIEW
  }وإلا إذا( pParse->bReturning ){
    /* If there is a RETURNING clause, populate the rowid register with
    ** constant value -1, in case one or more of the returned expressions
    ** refer to the "rowid" of the view.  */
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, -1, regRowid);
#جے_انت
  }

  /* Update the count of rows that are inserted
  */
  إذا( reg_صف_گنتی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, reg_صف_گنتی, 1);
  }

  إذا( pTrigger ){
    /* Code AFTER triggers */
    sqlite3_کوڈ_صف_Trigger(pParse, pTrigger, TK_داخل_کرو, 0, TRIGGER_AFTER,
        pTab, reg_انکڑے-2-pTab->n_خانہ, onError, سرا_دا_Loop);
  }

  /* The bottom of the main insertion loop, if the data source
  ** is a SELECT statement.
  */
  sqlite3VdbeResolveLabel(v, سرا_دا_Loop);
  إذا( use_عارضی_جدول ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, srcTab, addrCont); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ایتھے(v, addr_Ins_چوٹی);
    sqlite3_Vdbe_جوڑو_Op1(v, OP_Close, srcTab);
  }وإلا إذا( pSelect ){
    sqlite3VdbeGoto(v, addrCont);
#جے_تعریف_شدہ SQLITE_DEBUG
    /* If we are jumping back to an OP_Yield that is preceded by an
    ** OP_ReleaseReg, set the p5 flag on the OP_Goto so that the
    ** OP_ReleaseReg will be included in the loop. */
    إذا( sqlite3VdbeGetOp(v, addrCont-1)->opcode==OP_ReleaseReg ){
      أكد( sqlite3VdbeGetOp(v, addrCont)->opcode==OP_Yield );
      sqlite3VdbeChangeP5(v, 1);
    }
#جے_انت
    sqlite3_Vdbe_Jump_ایتھے(v, addr_Ins_چوٹی);
  }

#جے_بنا_تعریف SQLITE_OMIT_XFER_OPT
داخل_کرو_سرا:
#جے_انت /* SQLITE_OMIT_XFER_OPT */
  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  إذا( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3_Autoincrement_سرا(pParse);
  }

  /*
  ** Return the number of rows inserted. If this routine is
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  إذا( reg_صف_گنتی ){
    sqlite3_کوڈ_Change_گنتی(v, reg_صف_گنتی, "rows inserted");
  }

داخل_کرو_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3_اظہاریہ_List_Delete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  إذا( p_کالم ){
    sqlite3IdListDelete(db, p_کالم);
    sqlite3DbFree(db, a_Tab_خانہ_Map);
  }
  إذا( aRegIdx ) sqlite3DbNNFreeNN(db, aRegIdx);
}

/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#جے_تعریف_شدہ isView
 #تعریف_رد isView
#جے_انت
#جے_تعریف_شدہ pTrigger
 #تعریف_رد pTrigger
#جے_انت
#جے_تعریف_شدہ tmask
 #تعریف_رد tmask
#جے_انت

/*
** Meanings of bits in of pWalker->eCode for
** sqlite3ExprReferencesUpdatedColumn()
*/
#تعریف_کرو CKCNSTRNT_کالم   0x01    /* CHECK constraint uses a changing column */
#تعریف_کرو CKCNSTRNT_ROWID    0x02    /* CHECK constraint references the ROWID */

/* This is the Walker callback from sqlite3ExprReferencesUpdatedColumn().
*  Set bit 0x01 of pWalker->eCode if pWalker->eCode to 0 and if this
** expression node references any of the
** columns that are being modified by an UPDATE statement.
*/
ساكن صحيح check_Constraint_اظہاریہ_گرہ(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_کالم ){
    أكد( p_اظہاریہ->i_کالم>=0 || p_اظہاریہ->i_کالم==-1 );
    إذا( p_اظہاریہ->i_کالم>=0 ){
      إذا( pWalker->u.ai_خانہ[p_اظہاریہ->i_کالم]>=0 ){
        pWalker->e_کوڈ |= CKCNSTRNT_کالم;
      }
    }وإلا{
      pWalker->e_کوڈ |= CKCNSTRNT_ROWID;
    }
  }
  أرجع WRC_Continue;
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
صحيح sqlite3_اظہاریہ_References_Updated_کالم(
  اظہاریہ *p_اظہاریہ,    /* The expression to be checked */
  صحيح *aiChng,    /* aiChng[x]>=0 if column x changed by the UPDATE */
  صحيح chngRowid   /* True if UPDATE changes the rowid */
){
  Walker w;
  memset(&w, 0, حجم(w));
  w.e_کوڈ = 0;
  w.x_اظہاریہ_Callback = check_Constraint_اظہاریہ_گرہ;
  w.u.ai_خانہ = aiChng;
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  إذا( !chngRowid ){
    testcase( (w.e_کوڈ & CKCNSTRNT_ROWID)!=0 );
    w.e_کوڈ &= ~CKCNSTRNT_ROWID;
  }
  testcase( w.e_کوڈ==0 );
  testcase( w.e_کوڈ==CKCNSTRNT_کالم );
  testcase( w.e_کوڈ==CKCNSTRNT_ROWID );
  testcase( w.e_کوڈ==(CKCNSTRNT_ROWID|CKCNSTRNT_کالم) );
  أرجع w.e_کوڈ!=0;
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
تعريفنوع بنية اشاریہ_List_رکن اشاریہ_List_رکن;
تعريفنوع بنية اشاریہ_Iterator اشاریہ_Iterator;
بنية اشاریہ_Iterator {
  صحيح eType;    /* 0 for Index.pNext list.  1 for an array of IndexListTerm */
  صحيح i;        /* Index of the current item from the list */
  اتحاد {
    بنية {    /* Use this object for eType==0: A Index.pNext list */
      اشاریہ *pIdx;   /* The current Index */
    } lx;     
    بنية {    /* Use this object for eType==1; Array of IndexListTerm */
      صحيح nIdx;               /* Size of the array */
      اشاریہ_List_رکن *aIdx;    /* Array of IndexListTerms */
    } ax;
  } u;
};

/* When IndexIterator.eType==1, then each index is an array of instances
** of the following object
*/
بنية اشاریہ_List_رکن {
  اشاریہ *p;  /* The index */
  صحيح ix;    /* Which entry in the original Table.pIndex list is this index*/
};

/* Return the first index on the list */
ساكن اشاریہ *اشاریہ_Iterator_پہلا(اشاریہ_Iterator *pIter, صحيح *pIx){
  أكد( pIter->i==0 );
  إذا( pIter->eType ){
    *pIx = pIter->u.ax.aIdx[0].ix;
    أرجع pIter->u.ax.aIdx[0].p;
  }وإلا{
    *pIx = 0;
    أرجع pIter->u.lx.pIdx;
  }
}

/* Return the next index from the list.  Return NULL when out of indexes */
ساكن اشاریہ *اشاریہ_Iterator_اگلا(اشاریہ_Iterator *pIter, صحيح *pIx){
  إذا( pIter->eType ){
    صحيح i = ++pIter->i;
    إذا( i>=pIter->u.ax.nIdx ){
      *pIx = i;
      أرجع 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    أرجع pIter->u.ax.aIdx[i].p;
  }وإلا{
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->p_اگلا;
    أرجع pIter->u.lx.pIdx;
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
فراغ sqlite3GenerateConstraintChecks(
  Parse *pParse,       /* The parser context */
  جدول *pTab,         /* The table being inserted or updated */
  صحيح *aRegIdx,        /* Use register aRegIdx[i] for index i.  0 for unused */
  صحيح i_انکڑے_Cur,        /* Canonical data cursor (main table or PK index) */
  صحيح iIdxCur,         /* First index cursor */
  صحيح reg_New_انکڑے,      /* First register in a range holding values to insert */
  صحيح reg_Old_انکڑے,      /* Previous content.  0 for INSERTs */
  u8 pkChng,           /* Non-zero if the rowid or PRIMARY KEY changed */
  u8 overrideError,    /* Override onError to this if not OE_Default */
  صحيح ignoreDest,      /* Jump to this label on an OE_Ignore resolution */
  صحيح *pbMayReplace,   /* OUT: Set to true if constraint may cause a replace */
  صحيح *aiChng,         /* column i is unchanged if aiChng[i]<0 */
  Upsert *pUpsert      /* ON CONFLICT clauses, if any.  NULL otherwise */
){
  Vdbe *v;             /* VDBE under construction */
  اشاریہ *pIdx;         /* Pointer to one of the indices */
  اشاریہ *pPk = 0;      /* The PRIMARY KEY index for WITHOUT ROWID tables */
  sqlite3 *db;         /* Database connection */
  صحيح i;               /* loop counter */
  صحيح ix;              /* Index loop counter */
  صحيح n_خانہ;            /* Number of columns */
  صحيح onError;         /* Conflict resolution strategy */
  صحيح تکیا_Replace = 0; /* True if REPLACE is used to resolve INT PK conflict */
  صحيح n_Pk_میدان;        /* Number of fields in PRIMARY KEY. 1 for ROWID tables */
  Upsert *pUpsertClause = 0;  /* The specific ON CONFLICT clause for pIdx */
  u8 isUpdate;           /* True if this is an UPDATE operation */
  u8 b_Affinity_پورا = 0;  /* True if the OP_Affinity operation has been run */
  صحيح upsertIpkReturn = 0; /* Address of Goto at end of IPK uniqueness check */
  صحيح upsertIpkDelay = 0;  /* Address of Goto to bypass initial IPK check */
  صحيح ipk_چوٹی = 0;        /* Top of the IPK uniqueness check */
  صحيح ipkBottom = 0;     /* OP_Goto at the end of the IPK uniqueness check */
  /* Variables associated with retesting uniqueness constraints after
  ** replace triggers fire have run */
  صحيح regTrigCnt;       /* Register used to count replace trigger invocations */
  صحيح addrRecheck = 0;  /* Jump here to recheck all uniqueness constraints */
  صحيح lblRecheckOk = 0; /* Each recheck jumps to this label if it passes */
  Trigger *pTrigger;    /* List of DELETE triggers on the table pTab */
  صحيح nReplaceTrig = 0; /* Number of replace triggers coded */
  اشاریہ_Iterator sIdxIter;  /* Index iterator */

  isUpdate = reg_Old_انکڑے!=0;
  db = pParse->db;
  v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( !IsView(pTab) );  /* This table is not a VIEW */
  n_خانہ = pTab->n_خانہ;
 
  /* pPk is the PRIMARY KEY index for WITHOUT ROWID tables and NULL for
  ** normal rowid tables.  nPkField is the number of key fields in the
  ** pPk index or 1 for a rowid table.  In other words, nPkField is the
  ** number of fields in the true primary key of the table. */
  إذا( HasRowid(pTab) ){
    pPk = 0;
    n_Pk_میدان = 1;
  }وإلا{
    pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
    n_Pk_میدان = pPk->n_کلید_خانہ;
  }

  /* Record that this module has started */
  VdbeModuleComment((v, "BEGIN: GenCnstCks(%d,%d,%d,%d,%d)",
                     i_انکڑے_Cur, iIdxCur, reg_New_انکڑے, reg_Old_انکڑے, pkChng));

  /* Test all NOT NULL constraints.
  */
  إذا( pTab->tabFlags & TF_HasNotNull ){
    صحيح b2ndPass = 0;         /* True if currently running 2nd pass */
    صحيح n_تکیا_Replace = 0;     /* Number of ON CONFLICT REPLACE operations */
    صحيح nGenerated = 0;       /* Number of generated columns with NOT NULL */
    طالما(1){  /* Make 2 passes over columns. Exit loop via "break" */
      لكل(i=0; i<n_خانہ; i++){
        صحيح iReg;                        /* Register holding column value */
        کالم *p_خانہ = &pTab->a_خانہ[i];   /* The column to check for NOT NULL */
        صحيح isGenerated;                 /* non-zero if column is generated */
        onError = p_خانہ->notNull;
        إذا( onError==OE_None ) استمر; /* No NOT NULL on this column */
        إذا( i==pTab->iPKey ){
          استمر;        /* ROWID is never NULL */
        }
        isGenerated = p_خانہ->خانہ_Flags & COLFLAG_GENERATED;
        إذا( isGenerated && !b2ndPass ){
          nGenerated++;
          استمر;        /* Generated columns processed on 2nd pass */
        }
        إذا( aiChng && aiChng[i]<0 && !isGenerated ){
          /* Do not check NOT NULL on columns that do not change */
          استمر;
        }
        إذا( overrideError!=OE_Default ){
          onError = overrideError;
        }وإلا إذا( onError==OE_Default ){
          onError = OE_Abort;
        }
        إذا( onError==OE_Replace ){
          إذا( b2ndPass        /* REPLACE becomes ABORT on the 2nd pass */
           || p_خانہ->iDflt==0  /* REPLACE is ABORT if no DEFAULT value */
          ){
            testcase( p_خانہ->خانہ_Flags & COLFLAG_VIRTUAL );
            testcase( p_خانہ->خانہ_Flags & COLFLAG_STORED );
            testcase( p_خانہ->خانہ_Flags & COLFLAG_GENERATED );
            onError = OE_Abort;
          }وإلا{
            أكد( !isGenerated );
          }
        }وإلا إذا( b2ndPass && !isGenerated ){
          استمر;
        }
        أكد( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
            || onError==OE_Ignore || onError==OE_Replace );
        testcase( i!=sqlite3_جدول_کالم_منزل_Storage(pTab, i) );
        iReg = sqlite3_جدول_کالم_منزل_Storage(pTab, i) + reg_New_انکڑے + 1;
        بدل( onError ){
          حالة OE_Replace: {
            صحيح addr1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_NotNull, iReg);
            VdbeCoverage(v);
            أكد( (p_خانہ->خانہ_Flags & COLFLAG_GENERATED)==0 );
            n_تکیا_Replace++;
            sqlite3_اظہاریہ_کوڈ_Copy(pParse,
               sqlite3_کالم_اظہاریہ_Auth(pTab, p_خانہ, pParse), iReg);
            sqlite3_Vdbe_Jump_ایتھے(v, addr1);
            اكسر;
          }
          حالة OE_Abort:
            sqlite3MayAbort(pParse);
            /* no break */ deliberate_fall_through
          حالة OE_Rollback:
          حالة OE_Fail: {
            حرف *z_پیغام = sqlite3MPrintf(db, "%s.%s", pTab->zName,
                                        p_خانہ->zCnName);
            testcase( z_پیغام==0 && db->mallocFailed==0 );
            sqlite3_Vdbe_جوڑو_Op3(v, OP_HaltIfNull, SQLITE_CONSTRAINT_NOTNULL,
                              onError, iReg);
            sqlite3VdbeAppendP4(v, z_پیغام, P4_DYNAMIC);
            sqlite3VdbeChangeP5(v, P5_ConstraintNotNull);
            VdbeCoverage(v);
            اكسر;
          }
          افتراضي: {
            أكد( onError==OE_Ignore );
            sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, iReg, ignoreDest);
            VdbeCoverage(v);
            اكسر;
          }
        } /* end switch(onError) */
      } /* end loop i over columns */
      إذا( nGenerated==0 && n_تکیا_Replace==0 ){
        /* If there are no generated columns with NOT NULL constraints
        ** and no NOT NULL ON CONFLICT REPLACE constraints, then a single
        ** pass is sufficient */
        اكسر;
      }
      إذا( b2ndPass ) اكسر;  /* Never need more than 2 passes */
      b2ndPass = 1;
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
      إذا( n_تکیا_Replace>0 && (pTab->tabFlags & TF_HasGenerated)!=0 ){
        /* If any NOT NULL ON CONFLICT REPLACE constraints fired on the
        ** first pass, recomputed values for all generated columns, as
        ** those values might depend on columns affected by the REPLACE.
        */
        sqlite3ComputeGeneratedColumns(pParse, reg_New_انکڑے+1, pTab);
      }
#جے_انت
    } /* end of 2-pass loop */
  } /* end if( has-not-null-constraints ) */

  /* Test all CHECK constraints
  */
#جے_بنا_تعریف SQLITE_OMIT_CHECK
  إذا( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
    اظہاریہ_List *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(reg_New_انکڑے+1);
    onError = overrideError!=OE_Default ? overrideError : OE_Abort;
    لكل(i=0; i<pCheck->n_اظہاریہ; i++){
      صحيح allOk;
      اظہاریہ *pCopy;
      اظہاریہ *p_اظہاریہ = pCheck->a[i].p_اظہاریہ;
      إذا( aiChng
       && !sqlite3_اظہاریہ_References_Updated_کالم(p_اظہاریہ, aiChng, pkChng)
      ){
        /* The check constraints do not reference any of the columns being
        ** updated so there is no point it verifying the check constraint */
        استمر;
      }
      إذا( b_Affinity_پورا==0 ){
        sqlite3_جدول_Affinity(v, pTab, reg_New_انکڑے+1);
        b_Affinity_پورا = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeVerifyAbortable(v, onError);
      pCopy = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
      إذا( !db->mallocFailed ){
        sqlite3_اظہاریہ_If_True(pParse, pCopy, allOk, SQLITE_JUMPIFNULL);
      }
      sqlite3_اظہاریہ_Delete(db, pCopy);
      إذا( onError==OE_Ignore ){
        sqlite3VdbeGoto(v, ignoreDest);
      }وإلا{
        حرف *zName = pCheck->a[i].zEName;
        أكد( zName!=0 || pParse->db->mallocFailed );
        إذا( onError==OE_Replace ) onError = OE_Abort; /* IMP: R-26383-51744 */
        sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_CHECK,
                              onError, zName, P4_TRANSIENT,
                              P5_ConstraintCheck);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }
#جے_انت /* !defined(SQLITE_OMIT_CHECK) */

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
  sIdxIter.u.lx.pIdx = pTab->p_اشاریہ;
  إذا( pUpsert ){
    إذا( pUpsert->p_Upsert_نشانہ==0 ){
      /* There is just on ON CONFLICT clause and it has no constraint-target */
      أكد( pUpsert->p_اگلا_Upsert==0 );
      إذا( pUpsert->isDoUpdate==0 ){
        /* A single ON CONFLICT DO NOTHING clause, without a constraint-target.
        ** Make all unique constraint resolution be OE_Ignore */
        overrideError = OE_Ignore;
        pUpsert = 0;
      }وإلا{
        /* A single ON CONFLICT DO UPDATE.  Make all resolutions OE_Update */
        overrideError = OE_Update;
      }
    }وإلا إذا( pTab->p_اشاریہ!=0 ){
      /* Otherwise, we'll need to run the IndexListTerm array version of the
      ** iterator to ensure that all of the ON CONFLICT conditions are
      ** checked first and in order. */
      صحيح nIdx, jj;
      u64 nByte;
      Upsert *p_رکن;
      u8 *b_ورتیا;
      لكل(nIdx=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, nIdx++){
         أكد( aRegIdx[nIdx]>0 );
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (حجم(اشاریہ_List_رکن)+1)*nIdx + nIdx;
      sIdxIter.u.ax.aIdx = sqlite3DbMallocZero(db, nByte);
      إذا( sIdxIter.u.ax.aIdx==0 ) أرجع; /* OOM */
      b_ورتیا = (u8*)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->p_منزل_Free = sIdxIter.u.ax.aIdx;
      لكل(i=0, p_رکن=pUpsert; p_رکن; p_رکن=p_رکن->p_اگلا_Upsert){
        إذا( p_رکن->p_Upsert_نشانہ==0 ) اكسر;
        إذا( p_رکن->pUpsertIdx==0 ) استمر;  /* Skip ON CONFLICT for the IPK */
        jj = 0;
        pIdx = pTab->p_اشاریہ;
        طالما( ALWAYS(pIdx!=0) && pIdx!=p_رکن->pUpsertIdx ){
           pIdx = pIdx->p_اگلا;
           jj++;
        }
        إذا( b_ورتیا[jj] ) استمر; /* Duplicate ON CONFLICT clause ignored */
        b_ورتیا[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      لكل(jj=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, jj++){
        إذا( b_ورتیا[jj] ) استمر;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      أكد( i==nIdx );
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
  إذا( (db->flags & (SQLITE_RecTriggers|SQLITE_Foreign_کلیدیں))==0 ){
    /* There are not DELETE triggers nor FK constraints.  No constraint
    ** rechecks are needed. */
    pTrigger = 0;
    regTrigCnt = 0;
  }وإلا{
    إذا( db->flags&SQLITE_RecTriggers ){
      pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
      regTrigCnt = pTrigger!=0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    }وإلا{
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    إذا( regTrigCnt ){
      /* At this point regTrigCnt is non-zero if there are DELETE triggers
      ** or FK triggers. But we only care about these things if there is
      ** a chance that a row will be deleted by an ON CONFLICT REPLACE
      ** constraint. So zero regTrigCnt if no such constraint can be found. */
      إذا( overrideError!=OE_Replace ){
        إذا( overrideError!=OE_Default ){
          regTrigCnt = 0;
        }وإلا إذا( pkChng==0 || pPk || pTab->کلید_Conf!=OE_Replace ){
          لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
            إذا( pIdx->onError==OE_Replace ) اكسر;
          }
          إذا( pIdx==0 ) regTrigCnt = 0;
        }
      }
    }
    إذا( regTrigCnt ){
      /* Replace triggers might exist.  Allocate the counter and
      ** initialize it to zero. */
      regTrigCnt = ++pParse->nMem;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, regTrigCnt);
      VdbeComment((v, "trigger count"));
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  /* If rowid is changing, make sure the new rowid does not previously
  ** exist in the table.
  */
  إذا( pkChng && pPk==0 ){
    صحيح addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    /* Figure out what action to take in case of a rowid collision */
    onError = pTab->کلید_Conf;
    إذا( overrideError!=OE_Default ){
      onError = overrideError;
    }وإلا إذا( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* figure out whether or not upsert applies in this case */
    إذا( pUpsert ){
      pUpsertClause = sqlite3_Upsert_دا_اشاریہ(pUpsert,0);
      إذا( pUpsertClause!=0 ){
        إذا( pUpsertClause->isDoUpdate==0 ){
          onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
        }وإلا{
          onError = OE_Update;  /* DO UPDATE */
        }
      }
      إذا( pUpsertClause!=pUpsert ){
        /* The first ON CONFLICT clause has a conflict target other than
        ** the IPK.  We have to jump ahead to that first ON CONFLICT clause
        ** and then come back here and deal with the IPK afterwards */
        upsertIpkDelay = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
      }
    }

    /* If the response to a rowid conflict is REPLACE but the response
    ** to some other UNIQUE constraint is FAIL or IGNORE, then we need
    ** to defer the running of the rowid conflict checking until after
    ** the UNIQUE constraints have run.
    */
    إذا( onError==OE_Replace      /* IPK rule is REPLACE */
     && onError!=overrideError   /* Rules for other constraints are different */
     && pTab->p_اشاریہ             /* There exist other constraints */
    ){
      إذا( upsertIpkDelay ){
        ipk_چوٹی = upsertIpkDelay + 1;
        upsertIpkDelay = 0;
      }وإلا{
        ipk_چوٹی = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto)+1;
      }
      VdbeComment((v, "defer IPK REPLACE until last"));
    }

    إذا( isUpdate ){
      /* pkChng!=0 does not mean that the rowid has changed, only that
      ** it might have changed.  Skip the conflict logic below if the rowid
      ** is unchanged. */
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, reg_New_انکڑے, addrRowidOk, reg_Old_انکڑے);
      sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      VdbeCoverage(v);
    }

    /* Check to see if the new rowid already exists in the table.  Skip
    ** the following conflict logic if it does not. */
    VdbeNoopComment((v, "uniqueness check for ROWID"));
    sqlite3VdbeVerifyAbortable(v, onError);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_انکڑے_Cur, addrRowidOk, reg_New_انکڑے);
    VdbeCoverage(v);

    بدل( onError ){
      افتراضي: {
        onError = OE_Abort;
        /* no break */ deliberate_fall_through
      }
      حالة OE_Rollback:
      حالة OE_Abort:
      حالة OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3RowidConstraint(pParse, onError, pTab);
        اكسر;
      }
      حالة OE_Replace: {
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
        إذا( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          sqlite3_Generate_صف_Delete(pParse, pTab, pTrigger, i_انکڑے_Cur, iIdxCur,
                                   reg_New_انکڑے, 1, 0, OE_Replace, 1, -1);
          sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, regTrigCnt, 1); /* incr trigger cnt */
          nReplaceTrig++;
        }وإلا{
#جے_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
          أكد( HasRowid(pTab) );
          /* This OP_Delete opcode fires the pre-update-hook only. It does
          ** not modify the b-tree. It is more efficient to let the coming
          ** OP_Insert replace the existing entry than it is to delete the
          ** existing entry and then insert a new one. */
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Delete, i_انکڑے_Cur, OPFLAG_ISNOOP);
          sqlite3VdbeAppendP4(v, pTab, P4_جدول);
#جے_انت /* SQLITE_ENABLE_PREUPDATE_HOOK */
          إذا( pTab->p_اشاریہ ){
            sqlite3MultiWrite(pParse);
            sqlite3_Generate_صف_اشاریہ_Delete(pParse, pTab, i_انکڑے_Cur, iIdxCur,0,-1);
          }
        }
        تکیا_Replace = 1;
        اكسر;
      }
#جے_بنا_تعریف SQLITE_OMIT_UPSERT
      حالة OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, i_انکڑے_Cur);
        /* no break */ deliberate_fall_through
      }
#جے_انت
      حالة OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        اكسر;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    إذا( ipk_چوٹی ){
      ipkBottom = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
      sqlite3_Vdbe_Jump_ایتھے(v, ipk_چوٹی-1);
    }وإلا إذا( pUpsert && pUpsertClause!=pUpsert ){
      upsertIpkReturn = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
    }
  }

  /* Test all UNIQUE constraints by creating entries for each UNIQUE
  ** index and making sure that duplicate entries do not already exist.
  ** Compute the revised record entries for indices as we go.
  **
  ** This loop also handles the case of the PRIMARY KEY index for a
  ** WITHOUT ROWID table.
  */
  لكل(pIdx = اشاریہ_Iterator_پہلا(&sIdxIter, &ix);
      pIdx;
      pIdx = اشاریہ_Iterator_اگلا(&sIdxIter, &ix)
  ){
    صحيح regIdx;          /* Range of registers holding content for pIdx */
    صحيح regR;            /* Range of registers holding conflicting PK */
    صحيح iThisCur;        /* Cursor for this UNIQUE index */
    صحيح addrUniqueOk;    /* Jump here if the UNIQUE constraint is satisfied */
    صحيح addrConflictCk;  /* First opcode in the conflict check logic */
    صحيح nConflictCk;     /* Number of opcodes in conflict check logic */

    إذا( aRegIdx[ix]==0 ) استمر;  /* Skip indices that do not change */
    إذا( pUpsert ){
      pUpsertClause = sqlite3_Upsert_دا_اشاریہ(pUpsert, pIdx);
      إذا( upsertIpkDelay && pUpsertClause==pUpsert ){
        sqlite3_Vdbe_Jump_ایتھے(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    إذا( b_Affinity_پورا==0 ){
      sqlite3_جدول_Affinity(v, pTab, reg_New_انکڑے+1);
      b_Affinity_پورا = 1;
    }
    VdbeNoopComment((v, "prep index %s", pIdx->zName));
    iThisCur = iIdxCur+ix;


    /* Skip partial indices for which the WHERE clause is not true */
    إذا( pIdx->pPartIdxWhere ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(reg_New_انکڑے+1);
      sqlite3_اظہاریہ_If_False_Dup(pParse, pIdx->pPartIdxWhere, addrUniqueOk,
                            SQLITE_JUMPIFNULL);
      pParse->iSelfTab = 0;
    }

    /* Create a record for this index entry as it should appear after
    ** the insert or update.  Store that record in the aRegIdx[ix] register
    */
    regIdx = aRegIdx[ix]+1;
    لكل(i=0; i<pIdx->n_کالم; i++){
      صحيح i_میدان = pIdx->ai_کالم[i];
      صحيح x;
      إذا( i_میدان==XN_اظہاریہ ){
        pParse->iSelfTab = -(reg_New_انکڑے+1);
        sqlite3_اظہاریہ_کوڈ_Copy(pParse, pIdx->a_خانہ_اظہاریہ->a[i].p_اظہاریہ, regIdx+i);
        pParse->iSelfTab = 0;
        VdbeComment((v, "%s column %d", pIdx->zName, i));
      }وإلا إذا( i_میدان==XN_ROWID || i_میدان==pTab->iPKey ){
        x = reg_New_انکڑے;
        sqlite3_Vdbe_جوڑو_Op2(v, OP_IntCopy, x, regIdx+i);
        VdbeComment((v, "rowid"));
      }وإلا{
        testcase( sqlite3_جدول_کالم_منزل_Storage(pTab, i_میدان)!=i_میدان );
        x = sqlite3_جدول_کالم_منزل_Storage(pTab, i_میدان) + reg_New_انکڑے + 1;
        sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, x, regIdx+i);
        VdbeComment((v, "%s", pTab->a_خانہ[i_میدان].zCnName));
      }
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regIdx, pIdx->n_کالم, aRegIdx[ix]);
    VdbeComment((v, "for %s", pIdx->zName));
#جے_تعریف_شدہ SQLITE_ENABLE_NULL_TRIM
    إذا( pIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      sqlite3_سیٹ_Make_ریکارڈ_P5(v, pIdx->p_جدول);
    }
#جے_انت
    sqlite3VdbeReleaseRegisters(pParse, regIdx, pIdx->n_کالم, 0, 0);

    /* In an UPDATE operation, if this index is the PRIMARY KEY index
    ** of a WITHOUT ROWID table and there has been no change the
    ** primary key, then no collision is possible.  The collision detection
    ** logic below can all be skipped. */
    إذا( isUpdate && pPk==pIdx && pkChng==0 ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      استمر;
    }

    /* Find out what action to take in case there is a uniqueness conflict */
    onError = pIdx->onError;
    إذا( onError==OE_None ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      استمر;  /* pIdx is not a UNIQUE index */
    }
    إذا( overrideError!=OE_Default ){
      onError = overrideError;
    }وإلا إذا( onError==OE_Default ){
      onError = OE_Abort;
    }

    /* Figure out if the upsert clause applies to this index */
    إذا( pUpsertClause ){
      إذا( pUpsertClause->isDoUpdate==0 ){
        onError = OE_Ignore;  /* DO NOTHING is the same as INSERT OR IGNORE */
      }وإلا{
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
    أكد( Is_Ordinary_جدول(pTab) );
#جے_بنا_تعریف SQLITE_ENABLE_PREUPDATE_HOOK
    إذا( (ix==0 && pIdx->p_اگلا==0)                   /* Condition 3 */
     && pPk==pIdx                                   /* Condition 2 */
     && onError==OE_Replace                         /* Condition 1 */
     && ( 0==(db->flags&SQLITE_RecTriggers) ||      /* Condition 4 */
          0==sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0))
     && ( 0==(db->flags&SQLITE_Foreign_کلیدیں) ||      /* Condition 5 */
         (0==pTab->u.tab.pFKey && 0==sqlite3FkReferences(pTab)))
    ){
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      استمر;
    }
#جے_انت /* ifndef SQLITE_ENABLE_PREUPDATE_HOOK */

    /* Check to see if the new index entry will be unique */
    sqlite3VdbeVerifyAbortable(v, onError);
    addrConflictCk =
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_NoConflict, iThisCur, addrUniqueOk,
                           regIdx, pIdx->n_کلید_خانہ); VdbeCoverage(v);

    /* Generate code to handle collisions */
    regR = pIdx==pPk ? regIdx : sqlite3_Get_عارضی_Range(pParse, n_Pk_میدان);
    إذا( isUpdate || onError==OE_Replace ){
      إذا( HasRowid(pTab) ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_IdxRowid, iThisCur, regR);
        /* Conflict only if the rowid of the existing index entry
        ** is different from old-rowid */
        إذا( isUpdate ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, regR, addrUniqueOk, reg_Old_انکڑے);
          sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
          VdbeCoverage(v);
        }
      }وإلا{
        صحيح x;
        /* Extract the PRIMARY KEY from the end of the index entry and
        ** store it in registers regR..regR+nPk-1 */
        إذا( pIdx!=pPk ){
          لكل(i=0; i<pPk->n_کلید_خانہ; i++){
            أكد( pPk->ai_کالم[i]>=0 );
            x = sqlite3_جدول_کالم_منزل_اشاریہ(pIdx, pPk->ai_کالم[i]);
            sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iThisCur, x, regR+i);
            VdbeComment((v, "%s.%s", pTab->zName,
                         pTab->a_خانہ[pPk->ai_کالم[i]].zCnName));
          }
        }
        إذا( isUpdate ){
          /* If currently processing the PRIMARY KEY of a WITHOUT ROWID
          ** table, only conflict if the new PRIMARY KEY values are actually
          ** different from the old.  See TH3 withoutrowid04.test.
          **
          ** For a UNIQUE index, only conflict if the PRIMARY KEY values
          ** of the matched index row are different from the original PRIMARY
          ** KEY values of this row before the update.  */
          صحيح addrJump = sqlite3VdbeCurrentAddr(v)+pPk->n_کلید_خانہ;
          صحيح op = OP_Ne;
          صحيح regCmp = (Is_Primary_کلید_اشاریہ(pIdx) ? regIdx : regR);
 
          لكل(i=0; i<pPk->n_کلید_خانہ; i++){
            حرف *p4 = (حرف*)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->ai_کالم[i];
            أكد( x>=0 );
            إذا( i==(pPk->n_کلید_خانہ-1) ){
              addrJump = addrUniqueOk;
              op = OP_Eq;
            }
            x = sqlite3_جدول_کالم_منزل_Storage(pTab, x);
            sqlite3_Vdbe_جوڑو_Op4(v, op,
                reg_Old_انکڑے+1+x, addrJump, regCmp+i, p4, P4_COLLSEQ
            );
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            VdbeCoverageIf(v, op==OP_Eq);
            VdbeCoverageIf(v, op==OP_Ne);
          }
        }
      }
    }

    nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;
    أكد( nConflictCk>0 || db->mallocFailed );
    testcase( nConflictCk<=0 );
    testcase( nConflictCk>1 );

    /* Generate code that executes if the new index entry is not unique */
    أكد( onError==OE_Rollback || onError==OE_Abort || onError==OE_Fail
        || onError==OE_Ignore || onError==OE_Replace || onError==OE_Update );
    بدل( onError ){
      حالة OE_Rollback:
      حالة OE_Abort:
      حالة OE_Fail: {
        testcase( onError==OE_Rollback );
        testcase( onError==OE_Abort );
        testcase( onError==OE_Fail );
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        اكسر;
      }
#جے_بنا_تعریف SQLITE_OMIT_UPSERT
      حالة OE_Update: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur+ix);
        /* no break */ deliberate_fall_through
      }
#جے_انت
      حالة OE_Ignore: {
        testcase( onError==OE_Ignore );
        sqlite3VdbeGoto(v, ignoreDest);
        اكسر;
      }
      افتراضي: {
        أكد( onError==OE_Replace );
        إذا( regTrigCnt ){
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        إذا( pTrigger && isUpdate ){
          sqlite3_Vdbe_جوڑو_Op1(v, OP_Cursor_جندرا, i_انکڑے_Cur);
        }
        sqlite3_Generate_صف_Delete(pParse, pTab, pTrigger, i_انکڑے_Cur, iIdxCur,
            regR, n_Pk_میدان, 0, OE_Replace,
            (pIdx==pPk ? ONEPASS_SINGLE : ONEPASS_OFF), iThisCur);
        إذا( pTrigger && isUpdate ){
          sqlite3_Vdbe_جوڑو_Op1(v, OP_CursorUnlock, i_انکڑے_Cur);
        }
        تکیا_Replace = 1;
        اكسر;
      }
    }
    إذا( regTrigCnt ){
      صحيح addrBypass;  /* Jump destination to bypass recheck logic */

      sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, regTrigCnt, 1); /* incr trigger cnt */
      addrBypass = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);  /* Bypass recheck */
      VdbeComment((v, "bypass recheck"));

      /* Here we insert code that will be invoked after all constraint
      ** checks have run, if and only if one or more replace triggers
      ** fired. */
      sqlite3VdbeResolveLabel(v, lblRecheckOk);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      إذا( pIdx->pPartIdxWhere ){
        /* Bypass the recheck if this partial index is not defined
        ** for the current row */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, regIdx-1, lblRecheckOk);
        VdbeCoverage(v);
      }
      /* Copy the constraint check code from above, except change
      ** the constraint-ok jump destination to be the address of
      ** the next retest block */
      طالما( nConflictCk>0 ){
        VdbeOp x;    /* Conflict check opcode to copy */
        /* The sqlite3VdbeAddOp4() call might reallocate the opcode array.
        ** Hence, make a complete copy of the opcode, rather than using
        ** a pointer to the opcode. */
        x = *sqlite3VdbeGetOp(v, addrConflictCk);
        إذا( x.opcode!=OP_IdxRowid || isUpdate ){
          صحيح p2;      /* New P2 value for copied conflict check opcode */
          ثابت حرف *zP4;
          إذا( sqlite3OpcodeProperty[x.opcode]&OPFLG_JUMP ){
            p2 = lblRecheckOk;
          }وإلا{
            p2 = x.p2;
          }
          zP4 = x.p4type==P4_INT32 ? SQLITE_INT_منزل_PTR(x.p4.i) : x.p4.z;
          sqlite3_Vdbe_جوڑو_Op4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
          sqlite3VdbeChangeP5(v, x.p5);
          VdbeCoverageIf(v, p2!=x.p2);
        }
        nConflictCk--;
        addrConflictCk++;
      }
      /* If the retest fails, issue an abort */
      sqlite3UniqueConstraint(pParse, OE_Abort, pIdx);

      sqlite3_Vdbe_Jump_ایتھے(v, addrBypass); /* Terminate the recheck bypass */
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    إذا( regR!=regIdx ) sqlite3_Release_عارضی_Range(pParse, regR, n_Pk_میدان);
    إذا( pUpsertClause
     && upsertIpkReturn
     && sqlite3_Upsert_اگلا_Is_IPK(pUpsertClause)
    ){
      sqlite3VdbeGoto(v, upsertIpkDelay+1);
      sqlite3_Vdbe_Jump_ایتھے(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  /* If the IPK constraint is a REPLACE, run it last */
  إذا( ipk_چوٹی ){
    sqlite3VdbeGoto(v, ipk_چوٹی);
    VdbeComment((v, "Do IPK REPLACE"));
    أكد( ipkBottom>0 );
    sqlite3_Vdbe_Jump_ایتھے(v, ipkBottom);
  }

  /* Recheck all uniqueness constraints after replace triggers have run */
  testcase( regTrigCnt!=0 && nReplaceTrig==0 );
  أكد( regTrigCnt!=0 || nReplaceTrig==0 );
  إذا( nReplaceTrig ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_IfNot, regTrigCnt, lblRecheckOk);VdbeCoverage(v);
    إذا( !pPk ){
      إذا( isUpdate ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, reg_New_انکڑے, addrRecheck, reg_Old_انکڑے);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, i_انکڑے_Cur, addrRecheck, reg_New_انکڑے);
      VdbeCoverage(v);
      sqlite3RowidConstraint(pParse, OE_Abort, pTab);
    }وإلا{
      sqlite3VdbeGoto(v, addrRecheck);
    }
  }
  إذا( regTrigCnt ) sqlite3VdbeResolveLabel(v, lblRecheckOk);

  /* Generate the table record */
  إذا( HasRowid(pTab) ){
    صحيح regRec = aRegIdx[ix];
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_New_انکڑے+1, pTab->nNVCol, regRec);
    sqlite3_سیٹ_Make_ریکارڈ_P5(v, pTab);
    إذا( !b_Affinity_پورا ){
      sqlite3_جدول_Affinity(v, pTab, 0);
    }
  }

  *pbMayReplace = تکیا_Replace;
  VdbeModuleComment((v, "END: GenCnstCks(%d)", تکیا_Replace));
}

#جے_تعریف_شدہ SQLITE_ENABLE_NULL_TRIM
/*
** Change the P5 operand on the last opcode (which should be an OP_MakeRecord)
** to be the number of columns in table pTab that must not be NULL-trimmed.
**
** Or if no columns of pTab may be NULL-trimmed, leave P5 at zero.
*/
فراغ sqlite3_سیٹ_Make_ریکارڈ_P5(Vdbe *v, جدول *pTab){
  u16 i;

  /* Records with omitted columns are only allowed for schema format
  ** version 2 and later (SQLite version 3.1.4, 2005-02-20). */
  إذا( pTab->pSchema->file_format<2 ) أرجع;

  لكل(i=pTab->n_خانہ-1; i>0; i--){
    إذا( pTab->a_خانہ[i].iDflt!=0 ) اكسر;
    إذا( pTab->a_خانہ[i].خانہ_Flags & COLFLAG_PRIMKEY ) اكسر;
  }
  sqlite3VdbeChangeP5(v, i+1);
}
#جے_انت

/*
** Table pTab is a WITHOUT ROWID table that is being written to. The cursor
** number is iCur, and register regData contains the new record for the
** PK index. This function adds code to invoke the pre-update hook,
** if one is registered.
*/
#جے_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
ساكن فراغ کوڈ_Without_Rowid_Preupdate(
  Parse *pParse,                  /* Parse context */
  جدول *pTab,                    /* Table being updated */
  صحيح iCur,                       /* Cursor number for table */
  صحيح reg_انکڑے                     /* Data containing new record */
){
  Vdbe *v = pParse->pVdbe;
  صحيح r = sqlite3_Get_عارضی_Reg(pParse);
  أكد( !HasRowid(pTab) );
  أكد( 0==(pParse->db->mDbFlags & DBFLAG_Vacuum) || CORRUPT_DB );
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, r);
  sqlite3_Vdbe_جوڑو_Op4(v, OP_داخل_کرو, iCur, reg_انکڑے, r, (حرف*)pTab, P4_جدول);
  sqlite3VdbeChangeP5(v, OPFLAG_ISNOOP);
  sqlite3_Release_عارضی_Reg(pParse, r);
}
#وإلا
# define کوڈ_Without_Rowid_Preupdate(a,b,c,d)
#جے_انت

/*
** This routine generates code to finish the INSERT or UPDATE operation
** that was started by a prior call to sqlite3GenerateConstraintChecks.
** A consecutive range of registers starting at regNewData contains the
** rowid and the content to be inserted.
**
** The arguments to this routine should be the same as the first six
** arguments to sqlite3GenerateConstraintChecks.
*/
فراغ sqlite3CompleteInsertion(
  Parse *pParse,      /* The parser context */
  جدول *pTab,        /* the table into which we are inserting */
  صحيح i_انکڑے_Cur,       /* Cursor of the canonical data source */
  صحيح iIdxCur,        /* First index cursor */
  صحيح reg_New_انکڑے,     /* Range of content */
  صحيح *aRegIdx,       /* Register used by each index.  0 for unused indices */
  صحيح update_flags,   /* True for UPDATE, False for INSERT */
  صحيح appendBias,     /* True if this is likely to be an append */
  صحيح use_Seek_نتیجہ   /* True to set the USESEEKRESULT flag on OP_[Idx]Insert */
){
  Vdbe *v;            /* Prepared statements under construction */
  اشاریہ *pIdx;        /* An index being inserted or updated */
  u8 pik_flags;       /* flag values passed to the btree insert */
  صحيح i;              /* Loop counter */

  أكد( update_flags==0
       || update_flags==OPFLAG_ISUPDATE
       || update_flags==(OPFLAG_ISUPDATE|OPFLAG_SAVEPOSITION)
  );

  v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( !IsView(pTab) );  /* This table is not a VIEW */
  لكل(i=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, i++){
    /* All REPLACE indexes are at the end of the list */
    أكد( pIdx->onError!=OE_Replace
         || pIdx->p_اگلا==0
         || pIdx->p_اگلا->onError==OE_Replace );
    إذا( aRegIdx[i]==0 ) استمر;
    إذا( pIdx->pPartIdxWhere || (update_flags && pIdx->b_Has_اظہاریہ) ){
      /* If this is a partial index, or an UPDATE of an index on an
      ** expression, then the record register may be set to NULL to indicate
      ** that no record should be inserted into this index. */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, aRegIdx[i], sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
    }
    pik_flags = (use_Seek_نتیجہ ? OPFLAG_USESEEKRESULT : 0);
    إذا( Is_Primary_کلید_اشاریہ(pIdx) && !HasRowid(pTab) ){
      pik_flags |= OPFLAG_NCHANGE;
      pik_flags |= (update_flags & OPFLAG_SAVEPOSITION);
      إذا( update_flags==0 ){
        کوڈ_Without_Rowid_Preupdate(pParse, pTab, iIdxCur+i, aRegIdx[i]);
      }
    }
    sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iIdxCur+i, aRegIdx[i],
                         aRegIdx[i]+1,
                         pIdx->uniqNotNull ? pIdx->n_کلید_خانہ: pIdx->n_کالم);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  إذا( !HasRowid(pTab) ) أرجع;
  إذا( pParse->nested ){
    pik_flags = 0;
  }وإلا{
    pik_flags = OPFLAG_NCHANGE;
    pik_flags |= (update_flags?update_flags:OPFLAG_LASTROWID);
  }
  إذا( appendBias ){
    pik_flags |= OPFLAG_APPEND;
  }
  إذا( use_Seek_نتیجہ ){
    pik_flags |= OPFLAG_USESEEKRESULT;
  }
  sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, i_انکڑے_Cur, aRegIdx[i], reg_New_انکڑے);
  إذا( !pParse->nested ){
    sqlite3VdbeAppendP4(v, pTab, P4_جدول);
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
صحيح sqlite3_Open_جدول_And_Indices(
  Parse *pParse,   /* Parsing context */
  جدول *pTab,     /* Table to be opened */
  صحيح op,          /* OP_OpenRead or OP_OpenWrite */
  u8 p5,           /* P5 value for OP_Open* opcodes (except on WITHOUT ROWID) */
  صحيح i_ادھار,       /* Use this for the table cursor, if there is one */
  u8 *a_منزل_Open,     /* If not NULL: boolean for each table and index */
  صحيح *pi_انکڑے_Cur,  /* Write the database source cursor number here */
  صحيح *piIdxCur    /* Write the first index cursor number here */
){
  صحيح i;
  صحيح iDb;
  صحيح i_انکڑے_Cur;
  اشاریہ *pIdx;
  Vdbe *v;

  أكد( op==OP_OpenRead || op==OP_OpenWrite );
  أكد( op==OP_OpenWrite || p5==0 );
  أكد( pi_انکڑے_Cur!=0 );
  أكد( piIdxCur!=0 );
  إذا( IsVirtual(pTab) ){
    /* This routine is a no-op for virtual tables. Leave the output
    ** variables *piDataCur and *piIdxCur set to illegal cursor numbers
    ** for improved error detection. */
    *pi_انکڑے_Cur = *piIdxCur = -999;
    أرجع 0;
  }
  iDb = sqlite3_Schema_منزل_اشاریہ(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;
  أكد( v!=0 );
  إذا( i_ادھار<0 ) i_ادھار = pParse->nTab;
  i_انکڑے_Cur = i_ادھار++;
  *pi_انکڑے_Cur = i_انکڑے_Cur;
  إذا( HasRowid(pTab) && (a_منزل_Open==0 || a_منزل_Open[0]) ){
    sqlite3_Open_جدول(pParse, i_انکڑے_Cur, iDb, pTab, op);
  }وإلا إذا( pParse->db->noSharedCache==0 ){
    sqlite3_جدول_جندرا(pParse, iDb, pTab->tnum, op==OP_OpenWrite, pTab->zName);
  }
  *piIdxCur = i_ادھار;
  لكل(i=0, pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا, i++){
    صحيح iIdxCur = i_ادھار++;
    أكد( pIdx->pSchema==pTab->pSchema );
    إذا( Is_Primary_کلید_اشاریہ(pIdx) && !HasRowid(pTab) ){
      *pi_انکڑے_Cur = iIdxCur;
      p5 = 0;
    }
    إذا( a_منزل_Open==0 || a_منزل_Open[i+1] ){
      sqlite3_Vdbe_جوڑو_Op3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
      VdbeComment((v, "%s", pIdx->zName));
    }
  }
  إذا( i_ادھار>pParse->nTab ) pParse->nTab = i_ادھار;
  أرجع i;
}


#جے_تعریف_شدہ SQLITE_TEST
/*
** The following global variable is incremented whenever the
** transfer optimization is used.  This is used for testing
** purposes only - to make sure the transfer optimization really
** is happening when it is supposed to.
*/
صحيح sqlite3_xferopt_گنتی;
#جے_انت /* SQLITE_TEST */


#جے_بنا_تعریف SQLITE_OMIT_XFER_OPT
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
ساكن صحيح xfer_Compatible_اشاریہ(اشاریہ *pDest, اشاریہ *pSrc){
  صحيح i;
  أكد( pDest && pSrc );
  أكد( pDest->p_جدول!=pSrc->p_جدول );
  إذا( pDest->n_کلید_خانہ!=pSrc->n_کلید_خانہ || pDest->n_کالم!=pSrc->n_کالم ){
    أرجع 0;   /* Different number of columns */
  }
  إذا( pDest->onError!=pSrc->onError ){
    أرجع 0;   /* Different conflict resolution strategies */
  }
  لكل(i=0; i<pSrc->n_کلید_خانہ; i++){
    إذا( pSrc->ai_کالم[i]!=pDest->ai_کالم[i] ){
      أرجع 0;   /* Different columns indexed */
    }
    إذا( pSrc->ai_کالم[i]==XN_اظہاریہ ){
      أكد( pSrc->a_خانہ_اظہاریہ!=0 && pDest->a_خانہ_اظہاریہ!=0 );
      إذا( sqlite3_اظہاریہ_Compare(0, pSrc->a_خانہ_اظہاریہ->a[i].p_اظہاریہ,
                             pDest->a_خانہ_اظہاریہ->a[i].p_اظہاریہ, -1)!=0 ){
        أرجع 0;   /* Different expressions in the index */
      }
    }
    إذا( pSrc->a_ترتیب_دیو_ترتیب[i]!=pDest->a_ترتیب_دیو_ترتیب[i] ){
      أرجع 0;   /* Different sort orders */
    }
    إذا( sqlite3_stricmp(pSrc->azColl[i],pDest->azColl[i])!=0 ){
      أرجع 0;   /* Different collating sequences */
    }
  }
  إذا( sqlite3_اظہاریہ_Compare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1) ){
    أرجع 0;     /* Different WHERE clauses */
  }

  /* If no test above fails then the indices must be compatible */
  أرجع 1;
}

/*
** Examine an expression node and abort if it references the ROWID.
** This is a Walker callback used by xferCompatibleCheck()
*/
ساكن صحيح xferCheckRowid(Walker *pWalk, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_کالم && p_اظہاریہ->i_کالم<0 ){
    pWalk->e_کوڈ = 1;
    أرجع WRC_Abort;
  }وإلا{
    أرجع WRC_Continue;
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
ساكن صحيح xferCompatibleCheck(جدول *pDest, جدول *pSrc){
  إذا( sqlite3_اظہاریہ_List_Compare(pSrc->pCheck,pDest->pCheck,-1) ){
    أرجع 0;
  }
  إذا( pDest->iPKey<0 ){
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_اظہاریہ_Callback = xferCheckRowid;
    sqlite3_Walk_اظہاریہ_List(&w,pDest->pCheck);
    إذا( w.e_کوڈ ) أرجع 0;
  }
  أرجع 1;
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
ساكن صحيح xferOptimization(
  Parse *pParse,        /* Parser context */
  جدول *pDest,         /* The table we are inserting into */
  Select *pSelect,      /* A SELECT statement to use as the data source */
  صحيح onError,          /* How to handle constraint errors */
  صحيح iDbDest           /* The database of pDest */
){
  sqlite3 *db = pParse->db;
  اظہاریہ_List *pEList;                /* The result set of the SELECT */
  جدول *pSrc;                     /* The table in the FROM clause of SELECT */
  اشاریہ *pSrcIdx, *pDestIdx;       /* Source and destination indices */
  Src_شے *p_شے;                  /* An element of pSelect->pSrc */
  صحيح i;                           /* Loop counter */
  صحيح iDbSrc;                      /* The database of pSrc */
  صحيح iSrc, iDest;                 /* Cursors from source and destination */
  صحيح addr1, addr2;                /* Loop addresses */
  صحيح خالی_تھاواں_Dest_Test = 0;           /* Address of test for empty pDest */
  صحيح خالی_تھاواں_Src_Test = 0;            /* Address of test for empty pSrc */
  Vdbe *v;                         /* The VDBE we are building */
  صحيح regAutoinc;                  /* Memory register used by AUTOINC */
  صحيح destHasUniqueIdx = 0;        /* True if pDest has a UNIQUE index */
  صحيح reg_انکڑے, regRowid;           /* Registers holding data and rowid */

  أكد( pSelect!=0 );
  إذا( pParse->pWith || pSelect->pWith ){
    /* Do not attempt to process this query if there are an WITH clauses
    ** attached to it. Proceeding may generate a false "no such table: xxx"
    ** error if pSelect reads from a CTE named "xxx".  */
    أرجع 0;
  }
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  إذا( IsVirtual(pDest) ){
    أرجع 0;   /* tab1 must not be a virtual table */
  }
#جے_انت
  إذا( onError==OE_Default ){
    إذا( pDest->iPKey>=0 ) onError = pDest->کلید_Conf;
    إذا( onError==OE_Default ) onError = OE_Abort;
  }
  أكد(pSelect->pSrc);   /* allocated even if there is no FROM clause */
  إذا( pSelect->pSrc->nSrc!=1 ){
    أرجع 0;   /* FROM clause must have exactly one term */
  }
  إذا( pSelect->pSrc->a[0].fg.isSubquery ){
    أرجع 0;   /* FROM clause cannot contain a subquery */
  }
  إذا( pSelect->pWhere ){
    أرجع 0;   /* SELECT may not have a WHERE clause */
  }
  إذا( pSelect->p_ترتیب_By ){
    أرجع 0;   /* SELECT may not have an ORDER BY clause */
  }
  /* Do not need to test for a HAVING clause.  If HAVING is present but
  ** there is no ORDER BY, we will get an error. */
  إذا( pSelect->p_گروہ_By ){
    أرجع 0;   /* SELECT may not have a GROUP BY clause */
  }
  إذا( pSelect->p_حد_بندی ){
    أرجع 0;   /* SELECT may not have a LIMIT clause */
  }
  إذا( pSelect->p_پہلوں ){
    أرجع 0;   /* SELECT may not be a compound query */
  }
  إذا( pSelect->selFlags & SF_Distinct ){
    أرجع 0;   /* SELECT may not be DISTINCT */
  }
  pEList = pSelect->pEList;
  أكد( pEList!=0 );
  إذا( pEList->n_اظہاریہ!=1 ){
    أرجع 0;   /* The result set must have exactly one column */
  }
  أكد( pEList->a[0].p_اظہاریہ );
  إذا( pEList->a[0].p_اظہاریہ->op!=TK_ASTERISK ){
    أرجع 0;   /* The result set must be the special operator "*" */
  }

  /* At this point we have established that the statement is of the
  ** correct syntactic form to participate in this optimization.  Now
  ** we have to check the semantics.
  */
  p_شے = pSelect->pSrc->a;
  pSrc = sqlite3_Locate_جدول_شے(pParse, 0, p_شے);
  إذا( pSrc==0 ){
    أرجع 0;   /* FROM clause does not contain a real table */
  }
  إذا( pSrc->tnum==pDest->tnum && pSrc->pSchema==pDest->pSchema ){
    testcase( pSrc!=pDest ); /* Possible due to bad sqlite_schema.rootpage */
    أرجع 0;   /* tab1 and tab2 may not be the same table */
  }
  إذا( HasRowid(pDest)!=HasRowid(pSrc) ){
    أرجع 0;   /* source and destination must both be WITHOUT ROWID or not */
  }
  إذا( !Is_Ordinary_جدول(pSrc) ){
    أرجع 0;   /* tab2 may not be a view or virtual table */
  }
  إذا( pDest->n_خانہ!=pSrc->n_خانہ ){
    أرجع 0;   /* Number of columns must be the same in tab1 and tab2 */
  }
  إذا( pDest->iPKey!=pSrc->iPKey ){
    أرجع 0;   /* Both tables must have the same INTEGER PRIMARY KEY */
  }
  إذا( (pDest->tabFlags & TF_Strict)!=0 ){
    إذا( (pSrc->tabFlags & TF_Strict)==0 ){
      أرجع 0;  /* Cannot feed from a non-strict into a strict table */
    }
    لكل(i=0; i<pDest->n_خانہ; i++){
      بنانشانی eDestType = pDest->a_خانہ[i].eCType;
      بنانشانی eSrcType = pSrc->a_خانہ[i].eCType;
      إذا( eDestType==COLTYPE_ANY ) استمر;
      إذا( eDestType==eSrcType ) استمر;
      إذا( eDestType==COLTYPE_INT && eSrcType==COLTYPE_INTEGER ) استمر;
      إذا( eDestType==COLTYPE_INTEGER && eSrcType==COLTYPE_INT ) استمر;
      أرجع 0; /* Incompatible types in source and destination */
    }
  }
  لكل(i=0; i<pDest->n_خانہ; i++){
    کالم *p_Dest_خانہ = &pDest->a_خانہ[i];
    کالم *p_Src_خانہ = &pSrc->a_خانہ[i];
#جے_تعریف_شدہ SQLITE_ENABLE_HIDDEN_COLUMNS
    إذا( (db->mDbFlags & DBFLAG_Vacuum)==0
     && (p_Dest_خانہ->خانہ_Flags | p_Src_خانہ->خانہ_Flags) & COLFLAG_HIDDEN
    ){
      أرجع 0;    /* Neither table may have __hidden__ columns */
    }
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
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
    إذا( (p_Dest_خانہ->خانہ_Flags & COLFLAG_GENERATED) !=
        (p_Src_خانہ->خانہ_Flags & COLFLAG_GENERATED) ){
      أرجع 0;    /* Both columns have the same generated-column type */
    }
    /* But the transfer is only allowed if both the source and destination
    ** tables have the exact same expressions for generated columns.
    ** This requirement could be relaxed for VIRTUAL columns, I suppose.
    */
    إذا( (p_Dest_خانہ->خانہ_Flags & COLFLAG_GENERATED)!=0 ){
      إذا( sqlite3_اظہاریہ_Compare(0,
             sqlite3_کالم_اظہاریہ(pSrc, p_Src_خانہ),
             sqlite3_کالم_اظہاریہ(pDest, p_Dest_خانہ), -1)!=0 ){
        testcase( p_Dest_خانہ->خانہ_Flags & COLFLAG_VIRTUAL );
        testcase( p_Dest_خانہ->خانہ_Flags & COLFLAG_STORED );
        أرجع 0;  /* Different generator expressions */
      }
    }
#جے_انت
    إذا( p_Dest_خانہ->affinity!=p_Src_خانہ->affinity ){
      أرجع 0;    /* Affinity must be the same on all columns */
    }
    إذا( sqlite3_stricmp(sqlite3_کالم_Coll(p_Dest_خانہ),
                        sqlite3_کالم_Coll(p_Src_خانہ))!=0 ){
      أرجع 0;    /* Collating sequence must be the same on all columns */
    }
    إذا( p_Dest_خانہ->notNull && !p_Src_خانہ->notNull ){
      أرجع 0;    /* tab2 must be NOT NULL if tab1 is */
    }
    /* Default values for second and subsequent columns need to match. */
    إذا( (p_Dest_خانہ->خانہ_Flags & COLFLAG_GENERATED)==0 && i>0 ){
      اظہاریہ *p_Dest_اظہاریہ = sqlite3_کالم_اظہاریہ(pDest, p_Dest_خانہ);
      اظہاریہ *p_Src_اظہاریہ = sqlite3_کالم_اظہاریہ(pSrc, p_Src_خانہ);
      أكد( p_Dest_اظہاریہ==0 || p_Dest_اظہاریہ->op==TK_وسعت );
      أكد( p_Dest_اظہاریہ==0 || !اظہاریہ_Has_Property(p_Dest_اظہاریہ, EP_Int_قدر) );
      أكد( p_Src_اظہاریہ==0 || p_Src_اظہاریہ->op==TK_وسعت );
      أكد( p_Src_اظہاریہ==0 || !اظہاریہ_Has_Property(p_Src_اظہاریہ, EP_Int_قدر) );
      إذا( (p_Dest_اظہاریہ==0)!=(p_Src_اظہاریہ==0)
       || (p_Dest_اظہاریہ!=0 && strcmp(p_Dest_اظہاریہ->u.zToken,
                                       p_Src_اظہاریہ->u.zToken)!=0)
      ){
        أرجع 0;    /* Default values must be the same for all columns */
      }
    }
  }
  لكل(pDestIdx=pDest->p_اشاریہ; pDestIdx; pDestIdx=pDestIdx->p_اگلا){
    إذا( Is_Unique_اشاریہ(pDestIdx) ){
      destHasUniqueIdx = 1;
    }
    لكل(pSrcIdx=pSrc->p_اشاریہ; pSrcIdx; pSrcIdx=pSrcIdx->p_اگلا){
      إذا( xfer_Compatible_اشاریہ(pDestIdx, pSrcIdx) ) اكسر;
    }
    إذا( pSrcIdx==0 ){
      أرجع 0;    /* pDestIdx has no corresponding index in pSrc */
    }
    إذا( pSrcIdx->tnum==pDestIdx->tnum && pSrc->pSchema==pDest->pSchema
         && sqlite3FaultSim(411)==SQLITE_OK ){
      /* The sqlite3FaultSim() call allows this corruption test to be
      ** bypassed during testing, in order to exercise other corruption tests
      ** further downstream. */
      أرجع 0;   /* Corrupt schema - two indexes on the same btree */
    }
  }
#جے_بنا_تعریف SQLITE_OMIT_CHECK
  إذا( pDest->pCheck
   && (db->mDbFlags & DBFLAG_Vacuum)==0
   && !xferCompatibleCheck(pDest,pSrc)
  ){
    أرجع 0;   /* Tables have different CHECK constraints.  Ticket #2252 */
  }
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
  /* Disallow the transfer optimization if the destination table contains
  ** any foreign key constraints.  This is more restrictive than necessary.
  ** But the main beneficiary of the transfer optimization is the VACUUM
  ** command, and the VACUUM command disables foreign key constraints.  So
  ** the extra complication to make this rule less restrictive is probably
  ** not worth the effort.  Ticket [6284df89debdfa61db8073e062908af0c9b6118e]
  */
  أكد( Is_Ordinary_جدول(pDest) );
  إذا( (db->flags & SQLITE_Foreign_کلیدیں)!=0 && pDest->u.tab.pFKey!=0 ){
    أرجع 0;
  }
#جے_انت
  إذا( (db->flags & SQLITE_گنتی_صفاں)!=0 ){
    أرجع 0;  /* xfer opt does not play well with PRAGMA count_changes */
  }
#جے_بنا_تعریف SQLITE_OMIT_AUTHORIZATION
  إذا( db->xAuth ){
    صحيح iDb = sqlite3_Schema_منزل_اشاریہ(db, pSrc->pSchema);
    إذا( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) أرجع 0;
    لكل(i=0; i<pSrc->n_خانہ; i++){
      کالم *p_Src_خانہ = &pSrc->a_خانہ[i];
      إذا( sqlite3_Auth_Read_خانہ(pParse, pSrc->zName, p_Src_خانہ->zCnName, iDb) ){
        أرجع 0;
      }
    }
  }
#جے_انت

  /* If we get this far, it means that the xfer optimization is at
  ** least a possibility, though it might only work if the destination
  ** table (tab1) is initially empty.
  */
#جے_تعریف_شدہ SQLITE_TEST
  sqlite3_xferopt_گنتی++;
#جے_انت
  iDbSrc = sqlite3_Schema_منزل_اشاریہ(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3_کوڈ_Verify_Schema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  reg_انکڑے = sqlite3_Get_عارضی_Reg(pParse);
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, reg_انکڑے);
  regRowid = sqlite3_Get_عارضی_Reg(pParse);
  sqlite3_Open_جدول(pParse, iDest, iDbDest, pDest, OP_OpenWrite);
  أكد( HasRowid(pDest) || destHasUniqueIdx );
  إذا( (db->mDbFlags & DBFLAG_Vacuum)==0 && (
      (pDest->iPKey<0 && pDest->p_اشاریہ!=0)          /* (1) */
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
    addr1 = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iDest, 0); VdbeCoverage(v);
    خالی_تھاواں_Dest_Test = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
    sqlite3_Vdbe_Jump_ایتھے(v, addr1);
  }
  إذا( HasRowid(pSrc) ){
    u8 insFlags;
    sqlite3_Open_جدول(pParse, iSrc, iDbSrc, pSrc, OP_OpenRead);
    خالی_تھاواں_Src_Test = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    إذا( pDest->iPKey>=0 ){
      addr1 = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iSrc, regRowid);
      إذا( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
        sqlite3VdbeVerifyAbortable(v, onError);
        addr2 = sqlite3_Vdbe_جوڑو_Op3(v, OP_NotExists, iDest, 0, regRowid);
        VdbeCoverage(v);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3_Vdbe_Jump_ایتھے(v, addr2);
      }
      auto_Inc_قدم(pParse, regAutoinc, regRowid);
    }وإلا إذا( pDest->p_اشاریہ==0 && !(db->mDbFlags & DBFLAG_VacuumInto) ){
      addr1 = sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iDest, regRowid);
    }وإلا{
      addr1 = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iSrc, regRowid);
      أكد( (pDest->tabFlags & TF_Autoincrement)==0 );
    }

    إذا( db->mDbFlags & DBFLAG_Vacuum ){
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Seek_سرا, iDest);
      insFlags = OPFLAG_APPEND|OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
    }وإلا{
      insFlags = OPFLAG_NCHANGE|OPFLAG_LASTROWID|OPFLAG_APPEND|OPFLAG_PREFORMAT;
    }
#جے_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    إذا( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_صف_انکڑے, iSrc, reg_انکڑے, 1);
      insFlags &= ~OPFLAG_PREFORMAT;
    }وإلا
#جے_انت
    {
      sqlite3_Vdbe_جوڑو_Op3(v, OP_صف_Cell, iDest, iSrc, regRowid);
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iDest, reg_انکڑے, regRowid);
    إذا( (db->mDbFlags & DBFLAG_Vacuum)==0 ){
      sqlite3VdbeChangeP4(v, -1, (حرف*)pDest, P4_جدول);
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iSrc, addr1); VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, iDest, 0);
  }وإلا{
    sqlite3_جدول_جندرا(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3_جدول_جندرا(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  لكل(pDestIdx=pDest->p_اشاریہ; pDestIdx; pDestIdx=pDestIdx->p_اگلا){
    u8 idxInsFlags = 0;
    لكل(pSrcIdx=pSrc->p_اشاریہ; ALWAYS(pSrcIdx); pSrcIdx=pSrcIdx->p_اگلا){
      إذا( xfer_Compatible_اشاریہ(pDestIdx, pSrcIdx) ) اكسر;
    }
    أكد( pSrcIdx );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenRead, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pSrcIdx);
    VdbeComment((v, "%s", pSrcIdx->zName));
    sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenWrite, iDest, pDestIdx->tnum, iDbDest);
    sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, OPFLAG_BULKCSR);
    VdbeComment((v, "%s", pDestIdx->zName));
    addr1 = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iSrc, 0); VdbeCoverage(v);
    إذا( db->mDbFlags & DBFLAG_Vacuum ){
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
      لكل(i=0; i<pSrcIdx->n_کالم; i++){
        ثابت حرف *zColl = pSrcIdx->azColl[i];
        إذا( sqlite3_stricmp(sqlite3_Str_ثنائی, zColl) ) اكسر;
      }
      إذا( i==pSrcIdx->n_کالم ){
        idxInsFlags = OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT;
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Seek_سرا, iDest);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_صف_Cell, iDest, iSrc);
      }
    }وإلا إذا( !HasRowid(pSrc) && pDestIdx->idxType==SQLITE_IDXTYPE_PRIMARYKEY ){
      idxInsFlags |= OPFLAG_NCHANGE;
    }
    إذا( idxInsFlags!=(OPFLAG_USESEEKRESULT|OPFLAG_PREFORMAT) ){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_صف_انکڑے, iSrc, reg_انکڑے, 1);
      إذا( (db->mDbFlags & DBFLAG_Vacuum)==0
       && !HasRowid(pDest)
       && Is_Primary_کلید_اشاریہ(pDestIdx)
      ){
        کوڈ_Without_Rowid_Preupdate(pParse, pDest, iDest, reg_انکڑے);
      }
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Idx_داخل_کرو, iDest, reg_انکڑے);
    sqlite3VdbeChangeP5(v, idxInsFlags|OPFLAG_APPEND);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iSrc, addr1+1); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ایتھے(v, addr1);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, iSrc, 0);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, iDest, 0);
  }
  إذا( خالی_تھاواں_Src_Test ) sqlite3_Vdbe_Jump_ایتھے(v, خالی_تھاواں_Src_Test);
  sqlite3_Release_عارضی_Reg(pParse, regRowid);
  sqlite3_Release_عارضی_Reg(pParse, reg_انکڑے);
  إذا( خالی_تھاواں_Dest_Test ){
    sqlite3_Autoincrement_سرا(pParse);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Halt, SQLITE_OK, 0);
    sqlite3_Vdbe_Jump_ایتھے(v, خالی_تھاواں_Dest_Test);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Close, iDest, 0);
    أرجع 0;
  }وإلا{
    أرجع 1;
  }
}
#جے_انت /* SQLITE_OMIT_XFER_OPT */
