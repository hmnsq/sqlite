/*
** 2003 September 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used for creating, destroying, and populating
** a VDBE (or an "sqlite3_stmt" as it is known to the outside world.)
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/* Forward references */
ಸ್ಥಿರ ಶೂನ್ಯ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
ಸ್ಥಿರ ಶೂನ್ಯ vdbeFreeOpArray(sqlite3 *, Op *, ಪೂರ್ಣಾಂಕ);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(Vdbe) );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  memset(&p->aOp, 0, ಗಾತ್ರ(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  ಆದರೆ( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  ಪ್ರತಿಜ್ಞಾ( pParse->aLabel==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->nLabel==0 );
  ಪ್ರತಿಜ್ಞಾ( p->nOpAlloc==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->szOpAlloc==0 );
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(p, OP_Init, 0, 1);
  ಹಿಂತಿರುಗಿ p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  ಹಿಂತಿರುಗಿ p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
ಶೂನ್ಯ sqlite3VdbeError(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_ಸಂದೇಶ);
  va_start(ap, zFormat);
  p->z_Err_ಸಂದೇಶ = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_Sql(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n, u8 prepFlags){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  p->prepFlags = prepFlags;
  ಆದರೆ( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಕೂಡಿಸು_Dblquote_Str(sqlite3 *db, Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಆದರೆ( p ){
    ಪೂರ್ಣಾಂಕ n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            ಗಾತ್ರ(*pStr)+n+1-ಗಾತ್ರ(pStr->z));
    ಆದರೆ( pStr ){
      pStr->p_ಮುಂದಿನ_Str = p->pDblStr;
      p->pDblStr = pStr;
      ಸ್ಮರಣೆ_ನಕಲು(pStr->z, z, n+1);
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Double_Quoted_String(
  Vdbe *pVdbe,            /* The prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  ಪ್ರತಿಜ್ಞಾ( zId!=0 );
  ಆದರೆ( pVdbe->pDblStr==0 ) ಹಿಂತಿರುಗಿ 0;
  ಗಾಗಿ(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_ಮುಂದಿನ_Str){
    ಆದರೆ( strcmp(zId, pStr->z)==0 ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Swap byte-code between two VDBE structures.
**
** This happens after pB was previously run and returned
** SQLITE_SCHEMA.  The statement was then reprepared in pA.
** This routine transfers the new bytecode in pA over to pB
** so that pB can be run again.  The old pB byte code is
** moved back to pA so that it will be cleaned up when pA is
** finalized.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಅದಲುಬದಲು(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  ಅಕ್ಷರ *zTmp;
  ಪ್ರತಿಜ್ಞಾ( pA->db==pB->db );
  tmp = *pA;
  *pA = *pB;
  *pB = tmp;
  pTmp = pA->pVNext;
  pA->pVNext = pB->pVNext;
  pB->pVNext = pTmp;
  ppTmp = pA->ppVPrev;
  pA->ppVPrev = pB->ppVPrev;
  pB->ppVPrev = ppTmp;
  zTmp = pA->zSql;
  pA->zSql = pB->zSql;
  pB->zSql = zTmp;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#ಆದರೆ_ಅಂತ್ಯ
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  ಸ್ಮರಣೆ_ನಕಲು(pB->aCounter, pA->aCounter, ಗಾತ್ರ(pB->aCounter));
  pB->aCounter[SQLITE_STMTSTATUS_REPREPARE]++;
}

/*
** Resize the Vdbe.aOp array so that it is at least nOp elements larger
** than its current size. nOp is guaranteed to be less than or equal
** to 1024/sizeof(Op).
**
** If an out-of-memory error occurs while resizing the array, return
** SQLITE_NOMEM. In this case Vdbe.aOp and Vdbe.nOpAlloc remain
** unchanged (this is so that any opcodes already allocated can be
** correctly deallocated along with the rest of the Vdbe).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ growOpArray(Vdbe *v, ಪೂರ್ಣಾಂಕ nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/ಗಾತ್ರ(Op)));
  ಬಳಸದ_PARAMETER(nOp);
#ಆದರೆ_ಅಂತ್ಯ

  /* Ensure that the size of a VDBE does not grow too large */
  ಆದರೆ( nNew > p->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VDBE_OP] ){
    sqlite3OomFault(p->db);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }

  ಪ್ರತಿಜ್ಞಾ( nOp<=(ಪೂರ್ಣಾಂಕ)(1024/ಗಾತ್ರ(Op)) );
  ಪ್ರತಿಜ್ಞಾ( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*ಗಾತ್ರ(Op));
  ಆದರೆ( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_ಗಾತ್ರ(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/ಗಾತ್ರ(Op);
    v->aOp = pNew;
  }
  ಹಿಂತಿರುಗಿ (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* This routine is just a convenient place to set a breakpoint that will
** fire after each opcode is inserted and displayed using
** "PRAGMA vdbe_addoptrace=on".  Parameters "pc" (program counter) and
** pOp are available to make the breakpoint conditional.
**
** Other useful labels for breakpoints include:
**   test_trace_breakpoint(pc,pOp)
**   sqlite3CorruptError(lineno)
**   sqlite3MisuseError(lineno)
**   sqlite3CantopenError(lineno)
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_addop_breakpoint(ಪೂರ್ಣಾಂಕ pc, Op *pOp){
  ಸ್ಥಿರ u64 n = 0;
  (ಶೂನ್ಯ)pc;
  (ಶೂನ್ಯ)pOp;
  n++;
  ಆದರೆ( n==ಅತಿದೊಡ್ಡ_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ growOp3(Vdbe *p, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ p1, ಪೂರ್ಣಾಂಕ p2, ಪೂರ್ಣಾಂಕ p3){
  ಪ್ರತಿಜ್ಞಾ( p->nOpAlloc<=p->nOp );
  ಆದರೆ( growOpArray(p, 1) ) ಹಿಂತಿರುಗಿ 1;
  ಪ್ರತಿಜ್ಞಾ( p->nOpAlloc>p->nOp );
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, p1, p2, p3);
}
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_Op4_Int_Slow(
  Vdbe *p,            /* Add the opcode to this VM */
  ಪೂರ್ಣಾಂಕ op,             /* The new opcode */
  ಪೂರ್ಣಾಂಕ p1,             /* The P1 operand */
  ಪೂರ್ಣಾಂಕ p2,             /* The P2 operand */
  ಪೂರ್ಣಾಂಕ p3,             /* The P3 operand */
  ಪೂರ್ಣಾಂಕ p4              /* The P4 operand as an integer */
){
  ಪೂರ್ಣಾಂಕ addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, p1, p2, p3);
  ಆದರೆ( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  ಹಿಂತಿರುಗಿ addr;
}


/*
** Add a new instruction to the list of instructions current in the
** VDBE.  Return the address of the new instruction.
**
** Parameters:
**
**    p               Pointer to the VDBE
**
**    op              The opcode for this instruction
**
**    p1, p2, p3, p4  Operands
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op0(Vdbe *p, ಪೂರ್ಣಾಂಕ op){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, 0, 0, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op1(Vdbe *p, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ p1){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, p1, 0, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op2(Vdbe *p, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ p1, ಪೂರ್ಣಾಂಕ p2){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, p1, p2, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(Vdbe *p, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ p1, ಪೂರ್ಣಾಂಕ p2, ಪೂರ್ಣಾಂಕ p3){
  ಪೂರ್ಣಾಂಕ i;
  VdbeOp *pOp;

  i = p->nOp;
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<0xff );
  ಆದರೆ( p->nOpAlloc<=i ){
    ಹಿಂತಿರುಗಿ growOp3(p, op, p1, p2, p3);
  }
  ಪ್ರತಿಜ್ಞಾ( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  ಪ್ರತಿಜ್ಞಾ( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_ಚಕ್ರ = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
  pOp->i_Src_ಸಾಲು = 0;
#ಆದರೆ_ಅಂತ್ಯ
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  ಹಿಂತಿರುಗಿ i;
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(
  Vdbe *p,            /* Add the opcode to this VM */
  ಪೂರ್ಣಾಂಕ op,             /* The new opcode */
  ಪೂರ್ಣಾಂಕ p1,             /* The P1 operand */
  ಪೂರ್ಣಾಂಕ p2,             /* The P2 operand */
  ಪೂರ್ಣಾಂಕ p3,             /* The P3 operand */
  ಪೂರ್ಣಾಂಕ p4              /* The P4 operand as an integer */
){
  ಪೂರ್ಣಾಂಕ i;
  VdbeOp *pOp;

  i = p->nOp;
  ಆದರೆ( p->nOpAlloc<=i ){
    ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_Op4_Int_Slow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  ಪ್ರತಿಜ್ಞಾ( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_ಚಕ್ರ = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
  pOp->i_Src_ಸಾಲು = 0;
#ಆದರೆ_ಅಂತ್ಯ
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  ಹಿಂತಿರುಗಿ i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Int64(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest, i64 iVal){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Double(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest, ದ್ವಿಗುಣ rVal){
  i64 iVal;
  ಸ್ಮರಣೆ_ನಕಲು(&iVal,&rVal,8);
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeGoto(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ತುಂಬು_String(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zStr){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ಕೂಡಿಸು_Op4(p, OP_String8, 0, iDest, 0, zStr, 0);
}

/*
** Generate code that initializes multiple registers to string or integer
** constants.  The registers begin with iDest and increase consecutively.
** One register is initialized for each characgter in zTypes[].  For each
** "s" character in zTypes[], the register is a string if the argument is
** not NULL, or OP_Null if the value is a null pointer.  For each "i" character
** in zTypes[], the register is initialized to an integer.
**
** If the input string does not end with "X" then an OP_ResultRow instruction
** is generated for the values inserted.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Multi_ತುಂಬು(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTypes, ...){
  va_list ap;
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ c;
  va_start(ap, zTypes);
  ಗಾಗಿ(i=0; (c = zTypes[i])!=0; i++){
    ಆದರೆ( c=='s' ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = va_arg(ap, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='i' ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(p, OP_Integer, va_arg(ap, ಪೂರ್ಣಾಂಕ), iDest+i);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹೋಗು skip_op_resultrow;
    }
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(p, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Op4(
  Vdbe *p,            /* Add the opcode to this VM */
  ಪೂರ್ಣಾಂಕ op,             /* The new opcode */
  ಪೂರ್ಣಾಂಕ p1,             /* The P1 operand */
  ಪೂರ್ಣಾಂಕ p2,             /* The P2 operand */
  ಪೂರ್ಣಾಂಕ p3,             /* The P3 operand */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zP4,    /* The P4 operand */
  ಪೂರ್ಣಾಂಕ p4type          /* P4 operand type */
){
  ಪೂರ್ಣಾಂಕ addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  ಹಿಂತಿರುಗಿ addr;
}

/*
** Add an OP_Function or OP_PureFunc opcode.
**
** The eCallCtx argument is information (typically taken from Expr.op2)
** that describes the calling context of the function.  0 means a general
** function call.  NC_IsCheck means called by a check constraint,
** NC_IdxExpr means called as part of an index expression.  NC_PartIdx
** means in the WHERE clause of a partial index.  NC_GenCol means called
** while computing a generated column value.  0 is the usual case.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(
  Parse *pParse,        /* Parsing context */
  ಪೂರ್ಣಾಂಕ p1,               /* Constant argument mask */
  ಪೂರ್ಣಾಂಕ p2,               /* First argument register */
  ಪೂರ್ಣಾಂಕ p3,               /* Register into which results are written */
  ಪೂರ್ಣಾಂಕ nArg,             /* Number of argument */
  ಸ್ಥಿರಾಂಕ FuncDef *pFunc, /* The function to be invoked */
  ಪೂರ್ಣಾಂಕ eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ addr;
  sqlite3_context *pCtx;
  ಪ್ರತಿಜ್ಞಾ( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  ಆದರೆ( pCtx==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    ಹಿಂತಿರುಗಿ 0;
  }
  pCtx->p_ಹೊರಹೋಗು = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, eCallCtx ? OP_ಶುದ್ಧ_Func : OP_Function,
                           p1, p2, p3, (ಅಕ್ಷರ*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  ಹಿಂತಿರುಗಿ addr;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Explain_ಪೋಷಕ(Parse *pParse){
  VdbeOp *pOp;
  ಆದರೆ( pParse->addrExplain==0 ) ಹಿಂತಿರುಗಿ 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  ಹಿಂತಿರುಗಿ pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#ಆದರೆ defined(SQLITE_DEBUG)
ಶೂನ್ಯ sqlite3ExplainBreakpoint(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2){
  (ಶೂನ್ಯ)z1;
  (ಶೂನ್ಯ)z2;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeExplain(Parse *pParse, u8 b_ತಳ್ಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFmt, ...){
  ಪೂರ್ಣಾಂಕ addr = 0;
#ಆದರೆ !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  ಆದರೆ( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ಆದರೆ_ಅಂತ್ಯ
  {
    ಅಕ್ಷರ *z_ಸಂದೇಶ;
    Vdbe *v;
    va_list ap;
    ಪೂರ್ಣಾಂಕ iThis;
    va_start(ap, zFmt);
    z_ಸಂದೇಶ = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_ಸಂದೇಶ, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_ತಳ್ಳು?"PUSH":"", sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v)->p4.z);
    ಆದರೆ( b_ತಳ್ಳು){
      pParse->addrExplain = iThis;
    }
    sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(v, iThis, -1, -1, 0, 0);
  }
  ಹಿಂತಿರುಗಿ addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Explain_ತೆಗೆ(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3_Vdbe_Explain_ಪೋಷಕ(pParse);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_EXPLAIN */

/*
** Add an OP_ParseSchema opcode.  This routine is broken out from
** sqlite3VdbeAddOp4() since it needs to also needs to mark all btrees
** as having been used.
**
** zWhere is a WHERE clause that defines which entries of the schema
** to reparse.  If zWhere==0, that means all entries.  p5 is a mask
** of INITFLAG_* values for the parse.
**
** In the current usage, the following are always true:
**
**     ALTER TABLE:     zWhere==0,  p5!=0
**     Otherwise:       zWhere!=0,  p5==0
**
** The zWhere string must have been obtained from sqlite3DbMalloc().
** This routine will take ownership of the allocated memory.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಕೂಡಿಸು_Parse_Schema_Op(Vdbe *p, ಪೂರ್ಣಾಂಕ iDb, ಅಕ್ಷರ *zWhere, u16 p5){
  ಪೂರ್ಣಾಂಕ j;
  ಪ್ರತಿಜ್ಞಾ( p5==0 || zWhere==0 );
  sqlite3_Vdbe_ಕೂಡಿಸು_Op4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  ಗಾಗಿ(j=0; j<p->db->nDb; j++) sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಕೊನೆ_Coroutine(Vdbe *v, ಪೂರ್ಣಾಂಕ regYield){
  sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ಕೊನೆ_Coroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_ತಾತ್ಕಾಲಿಕ_Reg = 0;
  v->pParse->nRangeReg = 0;
}

/*
** Create a new symbolic label for an instruction that has yet to be
** coded.  The symbolic label is really just a negative number.  The
** label can be used as the P2 value of an operation.  Later, when
** the label is resolved to a specific address, the VDBE will scan
** through its operation list and change all values of P2 which match
** the label into the resolved address.
**
** The VDBE knows that a P2 value is a label because labels are
** always negative and P2 values are suppose to be non-negative.
** Hence, a negative P2 value is a label that has yet to be resolved.
** (Later:) This is only true for opcodes that have the OPFLG_JUMP
** property.
**
** Variable usage notes:
**
**     Parse.aLabel[x]     Stores the address that the x-th label resolves
**                         into.  For testing (SQLITE_DEBUG), unresolved
**                         labels stores -1, but that is not required.
**     Parse.nLabelAlloc   Number of slots allocated to Parse.aLabel[]
**     Parse.nLabel        The *negative* of the number of labels that have
**                         been issued.  The negative is stored because
**                         that gives a performance improvement over storing
**                         the equivalent positive value.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMakeLabel(Parse *pParse){
  ಹಿಂತಿರುಗಿ --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ resizeResolveLabel(Parse *p, Vdbe *v, ಪೂರ್ಣಾಂಕ j){
  ಪೂರ್ಣಾಂಕ n_New_ಗಾತ್ರ = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_ಗಾತ್ರ*ಗಾತ್ರ(p->aLabel[0]));
  ಆದರೆ( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=p->nLabelAlloc; i<n_New_ಗಾತ್ರ; i++) p->aLabel[i] = -1;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( n_New_ಗಾತ್ರ>=100 && (n_New_ಗಾತ್ರ/100)>(p->nLabelAlloc/100) ){
      sqlite3_ಪ್ರಗತಿ_Check(p);
    }
    p->nLabelAlloc = n_New_ಗಾತ್ರ;
    p->aLabel[j] = v->nOp;
  }
}
ಶೂನ್ಯ sqlite3VdbeResolveLabel(Vdbe *v, ಪೂರ್ಣಾಂಕ x){
  Parse *p = v->pParse;
  ಪೂರ್ಣಾಂಕ j = ADDR(x);
  ಪ್ರತಿಜ್ಞಾ( v->eVdbeState==VDBE_INIT_STATE );
  ಪ್ರತಿಜ್ಞಾ( j<-p->nLabel );
  ಪ್ರತಿಜ್ಞಾ( j>=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( p->db->flags & SQLITE_VdbeAddopTrace ){
    ಬರೆ("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಸತತ_Only_Once(Vdbe *p){
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
ಶೂನ್ಯ sqlite3VdbeReusable(Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=1; ALWAYS(i<p->nOp); i++){
    ಆದರೆ( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      ನಿಲ್ಲಿಸು;
    }
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

/*
** The following type and function are used to iterate through all opcodes
** in a Vdbe main program and each of the sub-programs (triggers) it may
** invoke directly or indirectly. It should be used as follows:
**
**   Op *pOp;
**   VdbeOpIter sIter;
**
**   memset(&sIter, 0, sizeof(sIter));
**   sIter.v = v;                            // v is of type Vdbe*
**   while( (pOp = opIterNext(&sIter)) ){
**     // Do something with pOp
**   }
**   sqlite3DbFree(v->db, sIter.apSub);
**
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VdbeOpIter VdbeOpIter;
ರಚನೆ VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  Sub_ಕಾರ್ಯಕ್ರಮ **apSub;        /* Array of subprograms */
  ಪೂರ್ಣಾಂಕ nSub;                  /* Number of entries in apSub */
  ಪೂರ್ಣಾಂಕ iAddr;                 /* Address of next instruction to return */
  ಪೂರ್ಣಾಂಕ iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
ಸ್ಥಿರ Op *op_Iter_ಮುಂದಿನ(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  ಪೂರ್ಣಾಂಕ nOp;

  ಆದರೆ( p->iSub<=p->nSub ){

    ಆದರೆ( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }ಇಲ್ಲದಿದ್ದರೆ{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    ಪ್ರತಿಜ್ಞಾ( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    ಆದರೆ( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    ಆದರೆ( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*ಗಾತ್ರ(Sub_ಕಾರ್ಯಕ್ರಮ*);
      ಪೂರ್ಣಾಂಕ j;
      ಗಾಗಿ(j=0; j<p->nSub; j++){
        ಆದರೆ( p->apSub[j]==pRet->p4.p_ಕಾರ್ಯಕ್ರಮ ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        ಆದರೆ( !p->apSub ){
          pRet = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p->apSub[p->nSub++] = pRet->p4.p_ಕಾರ್ಯಕ್ರಮ;
        }
      }
    }
  }

  ಹಿಂತಿರುಗಿ pRet;
}

/*
** Check if the program stored in the VM associated with pParse may
** throw an ABORT exception (causing the statement, but not entire transaction
** to be rolled back). This condition is true if the main program or any
** sub-programs contains any of the following:
**
**   *  OP_Halt with P1=SQLITE_CONSTRAINT and P2=OE_Abort.
**   *  OP_HaltIfNull with P1=SQLITE_CONSTRAINT and P2=OE_Abort.
**   *  OP_Destroy
**   *  OP_VUpdate
**   *  OP_VCreate
**   *  OP_VRename
**   *  OP_FkCounter with P2==0 (immediate foreign key constraint)
**   *  OP_CreateBtree/BTREE_INTKEY and OP_InitCoroutine
**      (for CREATE TABLE AS SELECT ...)
**
** Then check that the value of Parse.mayAbort is true if an
** ABORT may be thrown, or false otherwise. Return true if it does
** match, or false otherwise. This function is intended to be used as
** part of an assert statement in the compiler. Similar to:
**
**   assert( sqlite3VdbeAssertMayAbort(pParse->pVdbe, pParse->mayAbort) );
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeAssertMayAbort(Vdbe *v, ಪೂರ್ಣಾಂಕ mayAbort){
  ಪೂರ್ಣಾಂಕ hasAbort = 0;
  ಪೂರ್ಣಾಂಕ hasFkCounter = 0;
  ಪೂರ್ಣಾಂಕ has_Create_ಕೋಷ್ಟಕ = 0;
  ಪೂರ್ಣಾಂಕ has_Create_ಸೂಚ್ಯಂಕ = 0;
  ಪೂರ್ಣಾಂಕ hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  ಆದರೆ( v==0 ) ಹಿಂತಿರುಗಿ 0;
  memset(&sIter, 0, ಗಾತ್ರ(sIter));
  sIter.v = v;

  ಆಗ( (pOp = op_Iter_ಮುಂದಿನ(&sIter))!=0 ){
    ಪೂರ್ಣಾಂಕ opcode = pOp->opcode;
    ಆದರೆ( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_ಶುದ್ಧ_Func
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) has_Create_ಕೋಷ್ಟಕ = 1;
    ಆದರೆ( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      ಆದರೆ( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_ಸೂಚ್ಯಂಕ = 1;
      ಆದರೆ( opcode==OP_Clear ) has_Create_ಸೂಚ್ಯಂಕ = 1;
    }
    ಆದರೆ( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
    ಆದರೆ( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
      hasFkCounter = 1;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  ಹಿಂತಿರುಗಿ ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (has_Create_ಕೋಷ್ಟಕ && hasInitCoroutine) || has_Create_ಸೂಚ್ಯಂಕ
  );
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
ಶೂನ್ಯ sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  ಆದರೆ( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
ಶೂನ್ಯ sqlite3VdbeAssertAbortable(Vdbe *p){
  ಪ್ರತಿಜ್ಞಾ( p->nWrite==0 || p->ಬಳಸುತ್ತದೆ_Stmt_Journal );
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine is called after all opcodes have been inserted.  It loops
** through all the opcodes and fixes up some details.
**
** (1) For each jump instruction with a negative P2 value (a label)
**     resolve the P2 value to an actual address.
**
** (2) Compute the maximum number of arguments used by the xUpdate/xFilter
**     methods of any virtual table and store that value in *pMaxVtabArgs.
**
** (3) Update the Vdbe.readOnly and Vdbe.bIsReader flags to accurately
**     indicate what the prepared statement actually does.
**
** (4) (discontinued)
**
** (5) Reclaim the memory allocated for storing labels.
**
** This routine will only function correctly if the mkopcodeh.tcl generator
** script numbers the opcodes correctly.  Changes to this routine must be
** coordinated with changes to mkopcodeh.tcl.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resolve_P2_ಮೌಲ್ಯಗಳು(Vdbe *p, ಪೂರ್ಣಾಂಕ *p_Max_Vtab_ವಾದಗಳು){
  ಪೂರ್ಣಾಂಕ n_Max_Vtab_ವಾದಗಳು = *p_Max_Vtab_ವಾದಗಳು;
  Op *pOp;
  Parse *pParse = p->pParse;
  ಪೂರ್ಣಾಂಕ *aLabel = pParse->aLabel;

  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  ಪ್ರತಿಜ್ಞಾ( p->aOp[0].opcode==OP_Init );
  ಆಗ( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    ಆದರೆ( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      ಆಯ್ಕೆ( pOp->opcode ){
        ಸ್ಥಿತಿ OP_Transaction: {
          ಆದರೆ( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        ಸ್ಥಿತಿ OP_AutoCommit:
        ಸ್ಥಿತಿ OP_Savepoint: {
          p->bIsReader = 1;
          ನಿಲ್ಲಿಸು;
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
        ಸ್ಥಿತಿ OP_Checkpoint:
#ಆದರೆ_ಅಂತ್ಯ
        ಸ್ಥಿತಿ OP_Vacuum:
        ಸ್ಥಿತಿ OP_Journal_ಬಹುಲಕ: {
          p->readOnly = 0;
          p->bIsReader = 1;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ OP_Init: {
          ಪ್ರತಿಜ್ಞಾ( pOp->p2>=0 );
          ಹೋಗು resolve_p2_ಮೌಲ್ಯಗಳು_loop_exit;
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
        ಸ್ಥಿತಿ OP_VUpdate: {
          ಆದರೆ( pOp->p2>n_Max_Vtab_ವಾದಗಳು ) n_Max_Vtab_ವಾದಗಳು = pOp->p2;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ OP_VFilter: {
          ಪೂರ್ಣಾಂಕ n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          ಪ್ರತಿಜ್ಞಾ( (pOp - p->aOp) >= 3 );
          ಪ್ರತಿಜ್ಞಾ( pOp[-1].opcode==OP_Integer );
          ಪ್ರತಿಜ್ಞಾ( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          ಆದರೆ( n>n_Max_Vtab_ವಾದಗಳು ) n_Max_Vtab_ವಾದಗಳು = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#ಆದರೆ_ಅಂತ್ಯ
        ಪೂರ್ವನಿಯೋಜಿತ: {
          ಆದರೆ( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            ಪ್ರತಿಜ್ಞಾ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            ಪ್ರತಿಜ್ಞಾ( ADDR(pOp->p2)<-pParse->nLabel );
            ಪ್ರತಿಜ್ಞಾ( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          ಪ್ರತಿಜ್ಞಾ( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          ಪ್ರತಿಜ್ಞಾ( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          ನಿಲ್ಲಿಸು;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      ಪ್ರತಿಜ್ಞಾ( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    ಪ್ರತಿಜ್ಞಾ( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_ಮೌಲ್ಯಗಳು_loop_exit:
  ಆದರೆ( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *p_Max_Vtab_ವಾದಗಳು = n_Max_Vtab_ವಾದಗಳು;
  ಪ್ರತಿಜ್ಞಾ( p->bIsReader!=0 || Db_ಮುಖವಾಡ_All_Zero(p->btree_ಮುಖವಾಡ) );
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Check to see if a subroutine contains a jump to a location outside of
** the subroutine.  If a jump outside the subroutine is detected, add code
** that will cause the program to halt with an error message.
**
** The subroutine consists of opcodes between iFirst and iLast.  Jumps to
** locations within the subroutine are acceptable.  iRetReg is a register
** that contains the return address.  Jumps to outside the range of iFirst
** through iLast are also acceptable as long as the jump destination is
** an OP_Return to iReturnAddr.
**
** A jump to an unresolved label means that the jump destination will be
** beyond the current address.  That is normally a jump to an early
** termination and is consider acceptable.
**
** This routine only runs during debug builds.  The purpose is (of course)
** to detect invalid escapes out of a subroutine.  The OP_Halt opcode
** is generated rather than an assert() or other error, so that ".eqp full"
** will still work to show the original bytecode, to aid in debugging.
*/
ಶೂನ್ಯ sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ,       /* First opcode of the subroutine */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆಯ,        /* Last opcode of the subroutine */
  ಪೂರ್ಣಾಂಕ iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  ಪೂರ್ಣಾಂಕ i;
  sqlite3_str *pErr = 0;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  pParse = v->pParse;
  ಪ್ರತಿಜ್ಞಾ( pParse!=0 );
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( i_ಕೊನೆಯ>=i_ಮೊದಲ );
  ಪ್ರತಿಜ್ಞಾ( i_ಕೊನೆಯ<v->nOp );
  pOp = &v->aOp[i_ಮೊದಲ];
  ಗಾಗಿ(i=i_ಮೊದಲ; i<=i_ಕೊನೆಯ; i++, pOp++){
    ಆದರೆ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      ಪೂರ್ಣಾಂಕ iDest = pOp->p2;   /* Jump destination */
      ಆದರೆ( iDest==0 ) ಮುಂದುವರಿ;
      ಆದರೆ( pOp->opcode==OP_Gosub ) ಮುಂದುವರಿ;
      ಆದರೆ( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( iDest<0 ){
        ಪೂರ್ಣಾಂಕ j = ADDR(iDest);
        ಪ್ರತಿಜ್ಞಾ( j>=0 );
        ಆದರೆ( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          ಮುಂದುವರಿ;
        }
        iDest = pParse->aLabel[j];
      }
      ಆದರೆ( iDest<i_ಮೊದಲ || iDest>i_ಕೊನೆಯ ){
        ಪೂರ್ಣಾಂಕ j = iDest;
        ಗಾಗಿ(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          ಆದರೆ( pX->opcode==OP_Return ){
            ಆದರೆ( pX->p1==iRetReg ) ನಿಲ್ಲಿಸು;
            ಮುಂದುವರಿ;
          }
          ಆದರೆ( pX->opcode==OP_Noop ) ಮುಂದುವರಿ;
          ಆದರೆ( pX->opcode==OP_Explain ) ಮುಂದುವರಿ;
          ಆದರೆ( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, i_ಮೊದಲ, i_ಕೊನೆಯ);
          ನಿಲ್ಲಿಸು;
        }
      }
    }
  }
  ಆದರೆ( pErr ){
    ಅಕ್ಷರ *zErr = sqlite3_str_ಮುಕ್ತಾಯ(pErr);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Halt, SQLITE_ಆಂತರಿಕ, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeCurrentAddr(Vdbe *p){
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಹಿಂತಿರುಗಿ p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
ಶೂನ್ಯ sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, ಪೂರ್ಣಾಂಕ N){
  ಪ್ರತಿಜ್ಞಾ( p->nOp + N <= p->nOpAlloc );
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
ಶೂನ್ಯ sqlite3_Vdbe_Verify_No_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು(Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->nOp; i++){
    ಪ್ರತಿಜ್ಞಾ( p->aOp[i].opcode!=OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು );
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#ಆದರೆ defined(SQLITE_DEBUG)
ಶೂನ್ಯ sqlite3VdbeVerifyAbortable(Vdbe *p, ಪೂರ್ಣಾಂಕ onError){
  ಆದರೆ( onError==OE_Abort ) sqlite3_Vdbe_ಕೂಡಿಸು_Op0(p, OP_Abortable);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This function returns a pointer to the array of opcodes associated with
** the Vdbe passed as the first argument. It is the callers responsibility
** to arrange for the returned array to be eventually freed using the
** vdbeFreeOpArray() function.
**
** Before returning, *pnOp is set to the number of entries in the returned
** array. Also, *pnMaxArg is set to the larger of its current value and
** the number of entries in the Vdbe.apArg[] array required to execute the
** returned program.
*/
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, ಪೂರ್ಣಾಂಕ *pnOp, ಪೂರ್ಣಾಂಕ *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  ಪ್ರತಿಜ್ಞಾ( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  ಪ್ರತಿಜ್ಞಾ( Db_ಮುಖವಾಡ_All_Zero(p->btree_ಮುಖವಾಡ) );

  resolve_P2_ಮೌಲ್ಯಗಳು(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  ಹಿಂತಿರುಗಿ aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  ಪೂರ್ಣಾಂಕ nOp,                     /* Number of opcodes to add */
  VdbeOpList ಸ್ಥಿರಾಂಕ *aOp,       /* The opcodes to be added */
  ಪೂರ್ಣಾಂಕ iLineno                  /* Source-file line number of first opcode */
){
  ಪೂರ್ಣಾಂಕ i;
  VdbeOp *p_ಹೊರಹೋಗು, *p_ಮೊದಲ;
  ಪ್ರತಿಜ್ಞಾ( nOp>0 );
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಆದರೆ( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  p_ಮೊದಲ = p_ಹೊರಹೋಗು = &p->aOp[p->nOp];
  ಗಾಗಿ(i=0; i<nOp; i++, aOp++, p_ಹೊರಹೋಗು++){
    p_ಹೊರಹೋಗು->opcode = aOp->opcode;
    p_ಹೊರಹೋಗು->p1 = aOp->p1;
    p_ಹೊರಹೋಗು->p2 = aOp->p2;
    ಪ್ರತಿಜ್ಞಾ( aOp->p2>=0 );
    ಆದರೆ( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      p_ಹೊರಹೋಗು->p2 += p->nOp;
    }
    p_ಹೊರಹೋಗು->p3 = aOp->p3;
    p_ಹೊರಹೋಗು->p4type = P4_NOTUSED;
    p_ಹೊರಹೋಗು->p4.p = 0;
    p_ಹೊರಹೋಗು->p5 = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
    p_ಹೊರಹೋಗು->zComment = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
    p_ಹೊರಹೋಗು->i_Src_ಸಾಲು = iLineno+i;
#ಇಲ್ಲದಿದ್ದರೆ
    (ಶೂನ್ಯ)iLineno;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  p->nOp += nOp;
  ಹಿಂತಿರುಗಿ p_ಮೊದಲ;
}

#ಆದರೆ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(
  Vdbe *p,                        /* VM to add scanstatus() to */
  ಪೂರ್ಣಾಂಕ addrExplain,                /* Address of OP_Explain (or 0) */
  ಪೂರ್ಣಾಂಕ addrLoop,                   /* Address of loop counter */
  ಪೂರ್ಣಾಂಕ addr_ಭೇಟಿ,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName               /* Name of table or index being scanned */
){
  ಆದರೆ( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->n_ಪರಿಶೀಲನೆ) * ಗಾತ್ರ(ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ);
    ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *aNew;
    aNew = (ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ*)sqlite3DbRealloc(p->db, p->a_ಪರಿಶೀಲನೆ, nByte);
    ಆದರೆ( aNew ){
      ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *pNew = &aNew[p->n_ಪರಿಶೀಲನೆ++];
      memset(pNew, 0, ಗಾತ್ರ(ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addr_ಭೇಟಿ = addr_ಭೇಟಿ;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->a_ಪರಿಶೀಲನೆ = aNew;
    }
  }
}

/*
** Add the range of instructions from addrStart to addrEnd (inclusive) to
** the set of those corresponding to the sqlite3_stmt_scanstatus() counters
** associated with the OP_Explain instruction at addrExplain. The
** sum of the sqlite3Hwtime() values for each of these instructions
** will be returned for SQLITE_SCANSTAT_NCYCLE requests.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(
  Vdbe *p,
  ಪೂರ್ಣಾಂಕ addrExplain,
  ಪೂರ್ಣಾಂಕ addr_ಆರಂಭ,
  ಪೂರ್ಣಾಂಕ addr_ಕೊನೆ
){
  ಆದರೆ( IS_STMT_SCANSTATUS(p->db) ){
    ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *p_ಪರಿಶೀಲನೆ = 0;
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=p->n_ಪರಿಶೀಲನೆ-1; ii>=0; ii--){
      p_ಪರಿಶೀಲನೆ = &p->a_ಪರಿಶೀಲನೆ[ii];
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrExplain==addrExplain ) ನಿಲ್ಲಿಸು;
      p_ಪರಿಶೀಲನೆ = 0;
    }
    ಆದರೆ( p_ಪರಿಶೀಲನೆ ){
      ಆದರೆ( addr_ಕೊನೆ<0 ) addr_ಕೊನೆ = sqlite3VdbeCurrentAddr(p)-1;
      ಗಾಗಿ(ii=0; ii<Array_ಗಾತ್ರ(p_ಪರಿಶೀಲನೆ->aAddrRange); ii+=2){
        ಆದರೆ( p_ಪರಿಶೀಲನೆ->aAddrRange[ii]==0 ){
          p_ಪರಿಶೀಲನೆ->aAddrRange[ii] = addr_ಆರಂಭ;
          p_ಪರಿಶೀಲನೆ->aAddrRange[ii+1] = addr_ಕೊನೆ;
          ನಿಲ್ಲಿಸು;
        }
      }
    }
  }
}

/*
** Set the addresses for the SQLITE_SCANSTAT_NLOOP and SQLITE_SCANSTAT_NROW
** counters for the query element associated with the OP_Explain at
** addrExplain.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Counters(
  Vdbe *p,
  ಪೂರ್ಣಾಂಕ addrExplain,
  ಪೂರ್ಣಾಂಕ addrLoop,
  ಪೂರ್ಣಾಂಕ addr_ಭೇಟಿ
){
  ಆದರೆ( IS_STMT_SCANSTATUS(p->db) ){
    ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ *p_ಪರಿಶೀಲನೆ = 0;
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=p->n_ಪರಿಶೀಲನೆ-1; ii>=0; ii--){
      p_ಪರಿಶೀಲನೆ = &p->a_ಪರಿಶೀಲನೆ[ii];
      ಆದರೆ( p_ಪರಿಶೀಲನೆ->addrExplain==addrExplain ) ನಿಲ್ಲಿಸು;
      p_ಪರಿಶೀಲನೆ = 0;
    }
    ಆದರೆ( p_ಪರಿಶೀಲನೆ ){
      ಆದರೆ( addrLoop>0 ) p_ಪರಿಶೀಲನೆ->addrLoop = addrLoop;
      ಆದರೆ( addr_ಭೇಟಿ>0 ) p_ಪರಿಶೀಲನೆ->addr_ಭೇಟಿ = addr_ಭೇಟಿ;
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
ಶೂನ್ಯ sqlite3VdbeChangeOpcode(Vdbe *p, ಪೂರ್ಣಾಂಕ addr, u8 iNewOpcode){
  ಪ್ರತಿಜ್ಞಾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
ಶೂನ್ಯ sqlite3VdbeChangeP1(Vdbe *p, ಪೂರ್ಣಾಂಕ addr, ಪೂರ್ಣಾಂಕ val){
  ಪ್ರತಿಜ್ಞಾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
ಶೂನ್ಯ sqlite3VdbeChangeP2(Vdbe *p, ಪೂರ್ಣಾಂಕ addr, ಪೂರ್ಣಾಂಕ val){
  ಪ್ರತಿಜ್ಞಾ( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
ಶೂನ್ಯ sqlite3VdbeChangeP3(Vdbe *p, ಪೂರ್ಣಾಂಕ addr, ಪೂರ್ಣಾಂಕ val){
  ಪ್ರತಿಜ್ಞಾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
ಶೂನ್ಯ sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  ಪ್ರತಿಜ್ಞಾ( p->nOp>0 || p->db->mallocFailed );
  ಆದರೆ( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Typeof_ಸ್ತಂಭ(Vdbe *p, ಪೂರ್ಣಾಂಕ iDest){
  VdbeOp *pOp = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(p);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆಗ( pOp->opcode==OP_ReleaseReg ) pOp--;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pOp->p3==iDest && pOp->opcode==OP_ಸ್ತಂಭ ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(Vdbe *p, ಪೂರ್ಣಾಂಕ addr){
  sqlite3VdbeChangeP2(p, addr, p->nOp);
}

/*
** Change the P2 operand of the jump instruction at addr so that
** the jump lands on the next opcode.  Or if the jump instruction was
** the previous opcode (and is thus a no-op) then simply back up
** the next instruction counter by one slot so that the jump is
** overwritten by the next inserted opcode.
**
** This routine is an optimization of sqlite3VdbeJumpHere() that
** strives to omit useless byte-code like this:
**
**        7   Once 0 8 0
**        8   ...
*/
ಶೂನ್ಯ sqlite3_Vdbe_Jump_ಇಲ್ಲಿ_Or_ತೆಗೆ_Inst(Vdbe *p, ಪೂರ್ಣಾಂಕ addr){
  ಆದರೆ( addr==p->nOp-1 ){
    ಪ್ರತಿಜ್ಞಾ( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    ಪ್ರತಿಜ್ಞಾ( p->aOp[addr].p4type==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
    sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(p)->i_Src_ಸಾಲು = 0;  /* Erase VdbeCoverage() macros */
#ಆದರೆ_ಅಂತ್ಯ
    p->nOp--;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
ಸ್ಥಿರ ಶೂನ್ಯ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ freeP4Mem(sqlite3 *db, Mem *p){
  ಆದರೆ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
ಸ್ಥಿರ ಶೂನ್ಯ freeP4(sqlite3 *db, ಪೂರ್ಣಾಂಕ p4type, ಶೂನ್ಯ *p4){
  ಪ್ರತಿಜ್ಞಾ( db );
  ಆಯ್ಕೆ( p4type ){
    ಸ್ಥಿತಿ P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_DYNAMIC:
    ಸ್ಥಿತಿ P4_INTARRAY: {
      ಆದರೆ( p4 ) sqlite3DbNNFreeNN(db, p4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_KEYINFO: {
      ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ) sqlite3_ಕೀಲಿ_Info_Unref((ಕೀಲಿ_Info*)p4);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
    ಸ್ಥಿತಿ P4_ಅಭಿವ್ಯಕ್ತಿ: {
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, (ಅಭಿವ್ಯಕ್ತಿ*)p4);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_MEM: {
      ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ){
        sqlite3_ಮೌಲ್ಯ_Free((sqlite3_ಮೌಲ್ಯ*)p4);
      }ಇಲ್ಲದಿದ್ದರೆ{
        freeP4Mem(db, (Mem*)p4);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_VTAB : {
      ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ) sqlite3VtabUnlock((VTable *)p4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_TABLEREF: {
      ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ) sqlite3_Delete_ಕೋಷ್ಟಕ(db, (ಕೋಷ್ಟಕ*)p4);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeFreeOpArray(sqlite3 *db, Op *aOp, ಪೂರ್ಣಾಂಕ nOp){
  ಪ್ರತಿಜ್ಞಾ( nOp>=0 );
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( aOp ){
    Op *pOp = &aOp[nOp-1];
    ಆಗ(1){  /* Exit via break */
      ಆದರೆ( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#ಆದರೆ_ಅಂತ್ಯ    
      ಆದರೆ( pOp==aOp ) ನಿಲ್ಲಿಸು;
      pOp--;
    }
    sqlite3DbNNFreeNN(db, aOp);
  }
}

/*
** Link the SubProgram object passed as the second argument into the linked
** list at Vdbe.pSubProgram. This list is used to delete all sub-program
** objects when the VM is no longer required.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Link_Sub_ಕಾರ್ಯಕ್ರಮ(Vdbe *pVdbe, Sub_ಕಾರ್ಯಕ್ರಮ *p){
  p->p_ಮುಂದಿನ = pVdbe->p_ಕಾರ್ಯಕ್ರಮ;
  pVdbe->p_ಕಾರ್ಯಕ್ರಮ = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Has_Sub_ಕಾರ್ಯಕ್ರಮ(Vdbe *pVdbe){
  ಹಿಂತಿರುಗಿ pVdbe->p_ಕಾರ್ಯಕ್ರಮ!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(Vdbe *p, ಪೂರ್ಣಾಂಕ addr){
  VdbeOp *pOp;
  ಆದರೆ( p->db->mallocFailed ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  ಹಿಂತಿರುಗಿ 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Delete_ಪೂರ್ವ_Opcode(Vdbe *p, u8 op){
  ಆದರೆ( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    ಹಿಂತಿರುಗಿ sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(p, p->nOp-1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
ಶೂನ್ಯ sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ,          /* Index of first register to be released */
  ಪೂರ್ಣಾಂಕ N,               /* Number of registers to release */
  u32 ಮುಖವಾಡ,            /* Mask of registers to NOT release */
  ಪೂರ್ಣಾಂಕ bUndefine        /* If true, mark registers as undefined */
){
  ಆದರೆ( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe );
  ಪ್ರತಿಜ್ಞಾ( i_ಮೊದಲ>=1 );
  ಪ್ರತಿಜ್ಞಾ( i_ಮೊದಲ+N-1<=pParse->nMem );
  ಆದರೆ( N<=31 && ಮುಖವಾಡ!=0 ){
    ಆಗ( N>0 && (ಮುಖವಾಡ&1)!=0 ){
      ಮುಖವಾಡ >>= 1;
      i_ಮೊದಲ++;
      N--;
    }
    ಆಗ( N>0 && N<=32 && (ಮುಖವಾಡ & MASKBIT32(N-1))!=0 ){
      ಮುಖವಾಡ &= ~MASKBIT32(N-1);
      N--;
    }
  }
  ಆದರೆ( N>0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(pParse->pVdbe, OP_ReleaseReg, i_ಮೊದಲ, N, *(ಪೂರ್ಣಾಂಕ*)&ಮುಖವಾಡ);
    ಆದರೆ( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** Change the value of the P4 operand for a specific instruction.
** This routine is useful when a large program is loaded from a
** static array using sqlite3VdbeAddOpList but we want to make a
** few minor changes to the program.
**
** If n>=0 then the P4 operand is dynamic, meaning that a copy of
** the string is made into memory obtained from sqlite3_malloc().
** A value of n==0 means copy bytes of zP4 up to and including the
** first null byte.  If n>0 then copy n+1 bytes of zP4.
**
** Other values of n (P4_STATIC, P4_COLLSEQ etc.) indicate that zP4 points
** to a string or structure that is guaranteed to exist for the lifetime of
** the Vdbe. In these cases we can just copy the pointer.
**
** If addr<0 then change P4 on the most recently inserted instruction.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE vdbe_Change_P4_ತುಂಬಿದ(
  Vdbe *p,
  Op *pOp,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zP4,
  ಪೂರ್ಣಾಂಕ n
){
  ಆದರೆ( pOp->p4type ){
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  ಆದರೆ( n<0 ){
    sqlite3VdbeChangeP4(p, (ಪೂರ್ಣಾಂಕ)(pOp - p->aOp), zP4, n);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
ಶೂನ್ಯ sqlite3VdbeChangeP4(Vdbe *p, ಪೂರ್ಣಾಂಕ addr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zP4, ಪೂರ್ಣಾಂಕ n){
  Op *pOp;
  sqlite3 *db;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  db = p->db;
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಪ್ರತಿಜ್ಞಾ( p->aOp!=0 || db->mallocFailed );
  ಆದರೆ( db->mallocFailed ){
    ಆದರೆ( n!=P4_VTAB ) freeP4(db, n, (ಶೂನ್ಯ*)*(ಅಕ್ಷರ**)&zP4);
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( p->nOp>0 );
  ಪ್ರತಿಜ್ಞಾ( addr<p->nOp );
  ಆದರೆ( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  ಆದರೆ( n>=0 || pOp->p4type ){
    vdbe_Change_P4_ತುಂಬಿದ(p, pOp, zP4, n);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_ಗಮ್ಯ_INT(zP4);
    pOp->p4type = P4_INT32;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zP4!=0 ){
    ಪ್ರತಿಜ್ಞಾ( n<0 );
    pOp->p4.p = (ಶೂನ್ಯ*)zP4;
    pOp->p4type = (ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ)n;
    ಆದರೆ( n==P4_VTAB ) sqlite3_Vtab_ಬೀಗ((VTable*)zP4);
  }
}

/*
** Change the P4 operand of the most recently coded instruction
** to the value defined by the arguments.  This is a high-speed
** version of sqlite3VdbeChangeP4().
**
** The P4 operand must not have been previously defined.  And the new
** P4 must not be P4_INT32.  Use sqlite3VdbeChangeP4() in either of
** those cases.
*/
ಶೂನ್ಯ sqlite3VdbeAppendP4(Vdbe *p, ಶೂನ್ಯ *pP4, ಪೂರ್ಣಾಂಕ n){
  VdbeOp *pOp;
  ಪ್ರತಿಜ್ಞಾ( n!=P4_INT32 && n!=P4_VTAB );
  ಪ್ರತಿಜ್ಞಾ( n<=0 );
  ಆದರೆ( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pP4!=0 || n==P4_DYNAMIC );
    ಪ್ರತಿಜ್ಞಾ( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(Parse *pParse, ಸೂಚ್ಯಂಕ *pIdx){
  Vdbe *v = pParse->pVdbe;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 );
  p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_ದ_ಸೂಚ್ಯಂಕ(pParse, pIdx);
  ಆದರೆ( p_ಕೀಲಿ_Info ) sqlite3VdbeAppendP4(v, p_ಕೀಲಿ_Info, P4_KEYINFO);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeVComment(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list ap){
  ಪ್ರತಿಜ್ಞಾ( p->nOp>0 || p->aOp==0 );
  ಪ್ರತಿಜ್ಞಾ( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  ಆದರೆ( p->nOp ){
    ಪ್ರತಿಜ್ಞಾ( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
ಶೂನ್ಯ sqlite3VdbeComment(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಆದರೆ( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
ಶೂನ್ಯ sqlite3VdbeNoopComment(Vdbe *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಆದರೆ( p ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#ಆದರೆ_ಅಂತ್ಯ  /* NDEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_ಸಾಲು_ಸಂಖ್ಯೆ(Vdbe *v, ಪೂರ್ಣಾಂಕ i_ಸಾಲು){
  sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v)->i_Src_ಸಾಲು = i_ಸಾಲು;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_VDBE_COVERAGE */

/*
** Return the opcode for a given address.  The address must be non-negative.
** See sqlite3VdbeGetLastOp() to get the most recently added opcode.
**
** If a memory allocation error has occurred prior to the calling of this
** routine, then a pointer to a dummy VdbeOp will be returned.  That opcode
** is readable but not writable, though it is cast to a writable value.
** The return of a dummy opcode allows the call to continue functioning
** after an OOM fault without having to check to see if the return from
** this routine is a valid pointer.  But because the dummy.opcode is 0,
** dummy will never be written to.  This is verified by code inspection and
** by running with Valgrind.
*/
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, ಪೂರ್ಣಾಂಕ addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  ಸ್ಥಿರ VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಪ್ರತಿಜ್ಞಾ( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  ಆದರೆ( p->db->mallocFailed ){
    ಹಿಂತಿರುಗಿ (VdbeOp*)&dummy;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(Vdbe *p){
  ಹಿಂತಿರುಗಿ sqlite3VdbeGetOp(p, p->nOp - 1);
}

#ಆದರೆ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ translateP(ಅಕ್ಷರ c, ಸ್ಥಿರಾಂಕ Op *pOp){
  ಆದರೆ( c=='1' ) ಹಿಂತಿರುಗಿ pOp->p1;
  ಆದರೆ( c=='2' ) ಹಿಂತಿರುಗಿ pOp->p2;
  ಆದರೆ( c=='3' ) ಹಿಂತಿರುಗಿ pOp->p3;
  ಆದರೆ( c=='4' ) ಹಿಂತಿರುಗಿ pOp->p4.i;
  ಹಿಂತಿರುಗಿ pOp->p5;
}

/*
** Compute a string for the "comment" field of a VDBE opcode listing.
**
** The Synopsis: field in comments in the vdbe.c source file gets converted
** to an extra string that is appended to the sqlite3OpcodeName().  In the
** absence of other comments, this synopsis becomes the comment on the opcode.
** Some translation occurs:
**
**       "PX"      ->  "r[X]"
**       "PX@PY"   ->  "r[X..X+Y-1]"  or "r[x]" if y is 0 or 1
**       "PX@PY+1" ->  "r[X..X+Y]"    or "r[x]" if y is 0
**       "PY..PY"  ->  "r[X..Y]"      or "r[x]" if y<=x
**       "PINT13"  ->  int(P1|P3<<32)
**       "PDBL13"  ->  real(P1|P3<<32)
**       "PHEX23"  ->  hex(P2|P3<<32)
*/
ಅಕ್ಷರ *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  ಸ್ಥಿರಾಂಕ Op *pOp,     /* The opcode to be commented */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zP4    /* Previously obtained value for P4 */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpName;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSynopsis;
  ಪೂರ್ಣಾಂಕ nOpName;
  ಪೂರ್ಣಾಂಕ ii;
  ಅಕ್ಷರ zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_ಉದ್ದಳತೆ);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  ಆದರೆ( zOpName[nOpName+1] ){
    ಪೂರ್ಣಾಂಕ ನೋಡಿದ_Com = 0;
    ಅಕ್ಷರ c;
    zSynopsis = zOpName + nOpName + 1;
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(ಗಾತ್ರ(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    ಗಾಗಿ(ii=0; (c = zSynopsis[ii])!=0; ii++){
      ಆದರೆ( c=='P' ){
        c = zSynopsis[++ii];
        ಆದರೆ( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='X' ){
          ಆದರೆ( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            ನೋಡಿದ_Com = 1;
            ನಿಲ್ಲಿಸು;
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_ಗಮ್ಯ_64(pOp->p1,pOp->p3));
          ii += 4;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_ಬಳಸಿದ_ಮುಖವಾಡ
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_ಗಮ್ಯ_64(pOp->p2,pOp->p3));
          ii += 4;
#ಆದರೆ_ಅಂತ್ಯ
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_ಗಮ್ಯ_64(pOp->p1,pOp->p3);
          ದ್ವಿಗುಣ r;
          ಸ್ಮರಣೆ_ನಕಲು(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ v1 = translateP(c, pOp);
          ಪೂರ್ಣಾಂಕ v2;
          ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            ಆದರೆ( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ಇಲ್ಲದಿದ್ದರೆ{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            ಆದರೆ( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x.accError==0 ){
              ಪ್ರತಿಜ್ಞಾ( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_str_appendf(&x, "%d", v1);
            ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    ಆದರೆ( !ನೋಡಿದ_Com && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  ಆದರೆ( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  ಹಿಂತಿರುಗಿ sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&x);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#ಆದರೆ VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
ಸ್ಥಿರ ಶೂನ್ಯ display_P4_ಅಭಿವ್ಯಕ್ತಿ(StrAccum *p, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp = 0;
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_STRING:
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_str_appendf(p, "%Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ರಿಜಿಸ್ಟರ್: {
      sqlite3_str_appendf(p, "r[%d]", p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ){
        sqlite3_str_appendf(p, "rowid");
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_str_appendf(p, "c%d", (ಪೂರ್ಣಾಂಕ)p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_LT:      zOp = "LT";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_LE:      zOp = "LE";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_GT:      zOp = "GT";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_GE:      zOp = "GE";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NE:      zOp = "NE";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_EQ:      zOp = "EQ";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_IS:      zOp = "IS";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ISNOT:   zOp = "ISNOT";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_AND:     zOp = "AND";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_OR:      zOp = "OR";      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_PLUS:    zOp = "ADD";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ನಕ್ಷತ್ರ:    zOp = "MUL";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_MINUS:   zOp = "SUB";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_REM:     zOp = "REM";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITAND:  zOp = "BITAND";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITOR:   zOp = "BITOR";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_SLASH:   zOp = "DIV";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_LSHIFT:  zOp = "LSHIFT";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_RSHIFT:  zOp = "RSHIFT";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_CONCAT:  zOp = "CONCAT";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_UMINUS:  zOp = "MINUS";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_UPLUS:   zOp = "PLUS";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITNOT:  zOp = "BITNOT";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NOT:     zOp = "NOT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ISNULL:  zOp = "ISNULL";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NOTNULL: zOp = "NOTNULL"; ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      sqlite3_str_appendf(p, "%s", "expr");
      ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_ಅಭಿವ್ಯಕ್ತಿ(p, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
      sqlite3_str_append(p, ",", 1);
      display_P4_ಅಭಿವ್ಯಕ್ತಿ(p, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#ಆದರೆ VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
ಅಕ್ಷರ *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  ಅಕ್ಷರ *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_ಉದ್ದಳತೆ);
  ಆಯ್ಕೆ( pOp->p4type ){
    ಸ್ಥಿತಿ P4_KEYINFO: {
      ಪೂರ್ಣಾಂಕ j;
      ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = pOp->p4.p_ಕೀಲಿ_Info;
      ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ);
      ಗಾಗಿ(j=0; j<p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ; j++){
        CollSeq *pColl = p_ಕೀಲಿ_Info->aColl[j];
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl = pColl ? pColl->zName : "";
        ಆದರೆ( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[j] & KEYINFO_ಕ್ರಮ_DESC) ? "-" : "",
               (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[j] & KEYINFO_ಕ್ರಮ_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
    ಸ್ಥಿತಿ P4_ಅಭಿವ್ಯಕ್ತಿ: {
      display_P4_ಅಭಿವ್ಯಕ್ತಿ(&x, pOp->p4.p_ಅಭಿವ್ಯಕ್ತಿ);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ P4_COLLSEQ: {
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      ಪ್ರತಿಜ್ಞಾ( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      ಆದರೆ( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಸ್ಥಿತಿ P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      ಗಾಗಿ(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_SUBPROGRAM: {
      zP4 = "program";
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_ಕೋಷ್ಟಕ: {
      zP4 = pOp->p4.pTab->zName;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_ಸೂಚ್ಯಂಕ: {
      zP4 = pOp->p4.pIdx->zName;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      zP4 = pOp->p4.z;
    }
  }
  ಆದರೆ( zP4 ) sqlite3_str_appendall(&x, zP4);
  ಆದರೆ( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  ಹಿಂತಿರುಗಿ sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&x);
}
#ಆದರೆ_ಅಂತ್ಯ /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(Vdbe *p, ಪೂರ್ಣಾಂಕ i){
  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<p->db->nDb && i<(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(y_Db_ಮುಖವಾಡ)*8 );
  ಪ್ರತಿಜ್ಞಾ( i<(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(p->btree_ಮುಖವಾಡ)*8 );
  Db_ಮುಖವಾಡ_ಗಣ(p->btree_ಮುಖವಾಡ, i);
  ಆದರೆ( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    Db_ಮುಖವಾಡ_ಗಣ(p->ಬೀಗ_ಮುಖವಾಡ, i);
  }
}

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** If SQLite is compiled to support shared-cache mode and to be threadsafe,
** this routine obtains the mutex associated with each BtShared structure
** that may be accessed by the VM passed as an argument. In doing so it also
** sets the BtShared.db member of each of the BtShared structures, ensuring
** that the correct busy-handler callback is invoked if required.
**
** If SQLite is not threadsafe but does support shared-cache mode, then
** sqlite3BtreeEnter() is invoked to set the BtShared.db variables
** of all of BtShared structures accessible via the database handle
** associated with the VM.
**
** If SQLite is not threadsafe and does not support shared-cache mode, this
** function is a no-op.
**
** The p->btreeMask field is a bitmask of all btrees that the prepared
** statement p will ever use.  Let N be the number of bits in p->btreeMask
** corresponding to btrees that use shared cache.  Then the runtime of
** this routine is N*N.  But as N is rarely more than 1, this should not
** be a problem.
*/
ಶೂನ್ಯ sqlite3VdbeEnter(Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  sqlite3 *db;
  Db *aDb;
  ಪೂರ್ಣಾಂಕ nDb;
  ಆದರೆ( Db_ಮುಖವಾಡ_All_Zero(p->ಬೀಗ_ಮುಖವಾಡ) ) ಹಿಂತಿರುಗಿ;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  ಗಾಗಿ(i=0; i<nDb; i++){
    ಆದರೆ( i!=1 && Db_ಮುಖವಾಡ_Test(p->ಬೀಗ_ಮುಖವಾಡ,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ vdbeLeave(Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  sqlite3 *db;
  Db *aDb;
  ಪೂರ್ಣಾಂಕ nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  ಗಾಗಿ(i=0; i<nDb; i++){
    ಆದರೆ( i!=1 && Db_ಮುಖವಾಡ_Test(p->ಬೀಗ_ಮುಖವಾಡ,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
ಶೂನ್ಯ sqlite3VdbeLeave(Vdbe *p){
  ಆದರೆ( Db_ಮುಖವಾಡ_All_Zero(p->ಬೀಗ_ಮುಖವಾಡ) ) ಹಿಂತಿರುಗಿ;  /* The common case */
  vdbeLeave(p);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
ಶೂನ್ಯ sqlite3VdbePrintOp(FILE *p_ಹೊರಹೋಗು, ಪೂರ್ಣಾಂಕ pc, VdbeOp *pOp){
  ಅಕ್ಷರ *zP4;
  ಅಕ್ಷರ *zCom;
  sqlite3 dummyDb;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  ಆದರೆ( p_ಹೊರಹೋಗು==0 ) p_ಹೊರಹೋಗು = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#ಇಲ್ಲದಿದ್ದರೆ
  zCom = 0;
#ಆದರೆ_ಅಂತ್ಯ
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(p_ಹೊರಹೋಗು, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(p_ಹೊರಹೋಗು);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3_ಕೊನೆ_Benign_Malloc();
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Initialize an array of N Mem element.
**
** This is a high-runner, so only those fields that really do need to
** be initialized are set.  The Mem structure is organized so that
** the fields that get initialized are nearby and hopefully on the same
** cache line.
**
**    Mem.flags = flags
**    Mem.db = db
**    Mem.szMalloc = 0
**
** All other fields of Mem can safely remain uninitialized for now.  They
** will be initialized before use.
*/
ಸ್ಥಿರ ಶೂನ್ಯ initMemArray(Mem *p, ಪೂರ್ಣಾಂಕ N, sqlite3 *db, u16 flags){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( N>0 ){
    ಮಾಡು{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#ಆದರೆ_ಅಂತ್ಯ
      p++;
    }ಆಗ( (--N)>0 );
  }
}

/*
** Release auxiliary memory held in an array of N Mem elements.
**
** After this routine returns, all Mem elements in the array will still
** be valid.  Those Mem elements that were not holding auxiliary resources
** will be unchanged.  Mem elements which had something freed will be
** set to MEM_Undefined.
*/
ಸ್ಥಿರ ಶೂನ್ಯ releaseMemArray(Mem *p, ಪೂರ್ಣಾಂಕ N){
  ಆದರೆ( p && N ){
    Mem *p_ಕೊನೆ = &p[N];
    sqlite3 *db = p->db;
    ಪ್ರತಿಜ್ಞಾ( db!=0 );
    ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed ){
      ಮಾಡು{
        ಆದರೆ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }ಆಗ( (++p)<p_ಕೊನೆ );
      ಹಿಂತಿರುಗಿ;
    }
    ಮಾಡು{
      ಪ್ರತಿಜ್ಞಾ( (&p[1])==p_ಕೊನೆ || p[0].db==p[1].db );
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCheckMemInvariants(p) );

      /* This block is really an inlined version of sqlite3VdbeMemRelease()
      ** that takes advantage of the fact that the memory cell value is
      ** being set to NULL after releasing any dynamic resources.
      **
      ** The justification for duplicating code is that according to
      ** callgrind, this causes a certain test case to hit the CPU 4.7
      ** percent less (x86 linux, gcc version 4.1.2, -O6) than if
      ** sqlite3MemRelease() were called from here. With -O2, this jumps
      ** to 6.6 percent. The test case is inserting 1000 rows into a table
      ** with no indexes using a single prepared INSERT statement, bind()
      ** and reset(). Inserts are grouped into a transaction.
      */
      testcase( p->flags & MEM_Agg );
      testcase( p->flags & MEM_Dyn );
      ಆದರೆ( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3_Vdbe_ಚೌಕಟ್ಟು_Mem_Del );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
      ಇಲ್ಲದಿದ್ದರೆ{
        p->flags = MEM_Undefined;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }ಆಗ( (++p)<p_ಕೊನೆ );
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Is_Valid(Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು){
  ಆದರೆ( p_ಚೌಕಟ್ಟು->i_ಚೌಕಟ್ಟು_Magic!=SQLITE_ಚೌಕಟ್ಟು_MAGIC ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Mem_Del(ಶೂನ್ಯ *pArg){
  Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು = (Vdbe_ಚೌಕಟ್ಟು*)pArg;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Vdbe_ಚೌಕಟ್ಟು_Is_Valid(p_ಚೌಕಟ್ಟು) );
  p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ = p_ಚೌಕಟ್ಟು->v->p_Del_ಚೌಕಟ್ಟು;
  p_ಚೌಕಟ್ಟು->v->p_Del_ಚೌಕಟ್ಟು = p_ಚೌಕಟ್ಟು;
}

#ಆದರೆ defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಮುಂದಿನ_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  ಪೂರ್ಣಾಂಕ *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  ಪೂರ್ಣಾಂಕ *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  ಪೂರ್ಣಾಂಕ n_ಅಡ್ಡಸಾಲು;                            /* Stop when row count reaches this */
  ಪೂರ್ಣಾಂಕ nSub = 0;                        /* Number of sub-vdbes seen so far */
  Sub_ಕಾರ್ಯಕ್ರಮ **apSub = 0;              /* Array of sub-vdbes */
  ಪೂರ್ಣಾಂಕ i;                               /* Next instruction address */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  ಪೂರ್ಣಾಂಕ iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  n_ಅಡ್ಡಸಾಲು = p->nOp;
  ಆದರೆ( pSub!=0 ){
    ಆದರೆ( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/ಗಾತ್ರ(Vdbe*);
      apSub = (Sub_ಕಾರ್ಯಕ್ರಮ **)pSub->z;
    }
    ಗಾಗಿ(i=0; i<nSub; i++){
      n_ಅಡ್ಡಸಾಲು += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  ಆಗ(1){  /* Loop exits via break */
    i = iPc++;
    ಆದರೆ( i>=n_ಅಡ್ಡಸಾಲು ){
      p->rc = SQLITE_OK;
      rc = SQLITE_ಮುಗಿದ;
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      ಪೂರ್ಣಾಂಕ j;
      i -= p->nOp;
      ಪ್ರತಿಜ್ಞಾ( apSub!=0 );
      ಪ್ರತಿಜ್ಞಾ( nSub>0 );
      ಗಾಗಿ(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        ಪ್ರತಿಜ್ಞಾ( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    ಆದರೆ( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      ಪೂರ್ಣಾಂಕ nByte = (nSub+1)*ಗಾತ್ರ(Sub_ಕಾರ್ಯಕ್ರಮ*);
      ಪೂರ್ಣಾಂಕ j;
      ಗಾಗಿ(j=0; j<nSub; j++){
        ಆದರೆ( apSub[j]==aOp[i].p4.p_ಕಾರ್ಯಕ್ರಮ ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        ಆದರೆ( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          ನಿಲ್ಲಿಸು;
        }
        apSub = (Sub_ಕಾರ್ಯಕ್ರಮ **)pSub->z;
        apSub[nSub++] = aOp[i].p4.p_ಕಾರ್ಯಕ್ರಮ;
        Mem_ಗಣ_Type_Flag(pSub, MEM_Blob);
        pSub->n = nSub*ಗಾತ್ರ(Sub_ಕಾರ್ಯಕ್ರಮ*);
        n_ಅಡ್ಡಸಾಲು += aOp[i].p4.p_ಕಾರ್ಯಕ್ರಮ->nOp;
      }
    }
    ಆದರೆ( e_ಬಹುಲಕ==0 ) ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_BYTECODE_VTAB
    ಆದರೆ( e_ಬಹುಲಕ==2 ){
      Op *pOp = aOp + i;
      ಆದರೆ( pOp->opcode==OP_OpenRead ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( pOp->opcode==OP_ReopenIdx ) ನಿಲ್ಲಿಸು;     
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ==1 );
      ಆದರೆ( aOp[i].opcode==OP_Explain ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( aOp[i].opcode==OP_Init && iPc>1 ) ನಿಲ್ಲಿಸು;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Delete(Vdbe_ಚೌಕಟ್ಟು *p){
  ಪೂರ್ಣಾಂಕ i;
  Mem *aMem = Vdbe_ಚೌಕಟ್ಟು_Mem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->n_ಮಗು_Mem];
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Vdbe_ಚೌಕಟ್ಟು_Is_Valid(p) );
  ಗಾಗಿ(i=0; i<p->n_ಮಗು_Csr; i++){
    ಆದರೆ( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->n_ಮಗು_Mem);
  sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(p->v->db, &p->p_Aux_ದತ್ತಾಂಶ, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
/*
** Give a listing of the program in the virtual machine.
**
** The interface is the same as sqlite3VdbeExec().  But instead of
** running the code, it invokes the callback once for each instruction.
** This feature is used to implement "EXPLAIN".
**
** When p->explain==1, each instruction is listed.  When
** p->explain==2, only OP_Explain instructions are listed and these
** are shown in a different format.  p->explain==2 is used to implement
** EXPLAIN QUERY PLAN.
** 2018-04-24:  In p->explain==2 mode, the OP_Init opcodes of triggers
** are also shown, so that the boundaries between the main program and
** each trigger are clear.
**
** When p->explain==1, first the main program is listed, then each of
** the trigger subprograms are listed one by one.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  ಪೂರ್ಣಾಂಕ i;                               /* Loop counter */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  ಪೂರ್ಣಾಂಕ bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  ಪ್ರತಿಜ್ಞಾ( p->explain );
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_ಸತತ_STATE );
  ಪ್ರತಿಜ್ಞಾ( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  ಆದರೆ( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    ಪ್ರತಿಜ್ಞಾ( p->nMem>9 );
    pSub = &p->aMem[9];
  }ಇಲ್ಲದಿದ್ದರೆ{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_ಮುಂದಿನ_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  ಆದರೆ( rc==SQLITE_OK ){
    pOp = aOp + i;
    ಆದರೆ( Atomic_ತುಂಬು(&db->u1.isInterrupted) ){
      p->rc = SQLITE_ಅಡಚಣೆ;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      ಆದರೆ( p->explain==2 ){
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem, pOp->p1);
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+1, pOp->p2);
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+2, pOp->p3);
        sqlite3_Vdbe_Mem_ಗಣ_Str(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ಪ್ರತಿಜ್ಞಾ( p->n_Res_ಸ್ತಂಭ==4 );
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+0, i);
        sqlite3_Vdbe_Mem_ಗಣ_Str(pMem+1, (ಅಕ್ಷರ*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+2, pOp->p1);
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+3, pOp->p2);
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3_Vdbe_Mem_ಗಣ_Int64(pMem+6, pOp->p5);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          ಅಕ್ಷರ *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3_Vdbe_Mem_ಗಣ_Str(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#ಇಲ್ಲದಿದ್ದರೆ
        sqlite3_Vdbe_Mem_ಗಣ_Null(pMem+7);
#ಆದರೆ_ಅಂತ್ಯ
        sqlite3_Vdbe_Mem_ಗಣ_Str(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ಪ್ರತಿಜ್ಞಾ( p->n_Res_ಸ್ತಂಭ==8 );
      }
      p->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು = pMem;
      ಆದರೆ( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p->rc = SQLITE_OK;
        rc = SQLITE_ಅಡ್ಡಸಾಲು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_EXPLAIN */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
ಶೂನ್ಯ sqlite3VdbePrintSql(Vdbe *p){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = 0;
  ಆದರೆ( p->zSql ){
    z = p->zSql;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->nOp>=1 ){
    ಸ್ಥಿರಾಂಕ VdbeOp *pOp = &p->aOp[0];
    ಆದರೆ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      ಆಗ( sqlite3Isspace(*z) ) z++;
    }
  }
  ಆದರೆ( z ) ಬರೆ("SQL: [%s]\n", z);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
ಶೂನ್ಯ sqlite3VdbeIOTraceSql(Vdbe *p){
  ಪೂರ್ಣಾಂಕ nOp = p->nOp;
  VdbeOp *pOp;
  ಆದರೆ( sqlite3IoTrace==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( nOp<1 ) ಹಿಂತಿರುಗಿ;
  pOp = &p->aOp[0];
  ಆದರೆ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    ಪೂರ್ಣಾಂಕ i, j;
    ಅಕ್ಷರ z[1000];
    sqlite3_snprintf(ಗಾತ್ರ(z), z, "%s", pOp->p4.z);
    ಗಾಗಿ(i=0; sqlite3Isspace(z[i]); i++){}
    ಗಾಗಿ(j=0; z[i]; i++){
      ಆದರೆ( sqlite3Isspace(z[i]) ){
        ಆದರೆ( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
ರಚನೆ ReusableSpace {
  u8 *pSpace;            /* Available memory */
  sqlite3_int64 nFree;   /* Bytes of available memory */
  sqlite3_int64 nNeeded; /* Total bytes that could not be allocated */
};

/* Try to allocate nByte bytes of 8-byte aligned bulk memory for pBuf
** from the ReusableSpace object.  Return a pointer to the allocated
** memory on success.  If insufficient memory is available in the
** ReusableSpace object, increase the ReusableSpace.nNeeded
** value by the amount needed and return NULL.
**
** If pBuf is not initially NULL, that means that the memory has already
** been allocated by a prior call to this routine, so just return a copy
** of pBuf and leave ReusableSpace unchanged.
**
** This allocator is employed to repurpose unused slots at the end of the
** opcode array of prepared state for other memory needs of the prepared
** statement.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *allocSpace(
  ರಚನೆ ReusableSpace *p,  /* Bulk memory available for allocation */
  ಶೂನ್ಯ *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  ಆದರೆ( pBuf==0 ){
    nByte = ROUND8P(nByte);
    ಆದರೆ( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->nNeeded += nByte;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pBuf) );
  ಹಿಂತಿರುಗಿ pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
ಶೂನ್ಯ sqlite3VdbeRewind(Vdbe *p){
#ಆದರೆ defined(SQLITE_DEBUG)
  ಪೂರ್ಣಾಂಕ i;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  ಪ್ರತಿಜ್ಞಾ( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಗಾಗಿ(i=0; i<p->nMem; i++){
    ಪ್ರತಿಜ್ಞಾ( p->aMem[i].db==p->db );
  }
#ಆದರೆ_ಅಂತ್ಯ
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VDBE_PROFILE
  ಗಾಗಿ(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].n_ಚಕ್ರ = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Prepare a virtual machine for execution for the first time after
** creating the virtual machine.  This involves things such
** as allocating registers and initializing the program counter.
** After the VDBE has be prepped, it can be executed by one or more
** calls to sqlite3VdbeExec(). 
**
** This function may be called exactly once on each virtual machine.
** After this routine is called the VM has been "packaged" and is ready
** to run.  After this routine is called, further calls to
** sqlite3VdbeAddOp() functions are prohibited.  This routine disconnects
** the Vdbe from the Parse object that helped generate it so that the
** the Vdbe becomes an independent entity and the Parse object can be
** destroyed.
**
** Use the sqlite3VdbeRewind() procedure to restore a virtual machine back
** to its initial state after it has been run.
*/
ಶೂನ್ಯ sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  ಪೂರ್ಣಾಂಕ nVar;                      /* Number of parameters */
  ಪೂರ್ಣಾಂಕ nMem;                      /* Number of VM memory registers */
  ಪೂರ್ಣಾಂಕ nCursor;                   /* Number of cursors required */
  ಪೂರ್ಣಾಂಕ nArg;                      /* Max number args to xFilter or xUpdate */
  ಪೂರ್ಣಾಂಕ n;                         /* Loop counter */
  ರಚನೆ ReusableSpace x;        /* Reusable bulk memory */

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->nOp>0 );
  ಪ್ರತಿಜ್ಞಾ( pParse!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_INIT_STATE );
  ಪ್ರತಿಜ್ಞಾ( pParse==p->pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  nVar = pParse->nVar;
  nMem = pParse->nMem;
  nCursor = pParse->nTab;
  nArg = pParse->nMaxArg;
 
  /* Each cursor uses a memory cell.  The first cursor (cursor 0) can
  ** use aMem[0] which is not otherwise used by the VDBE program.  Allocate
  ** space at the end of aMem[] for cursors 1 and greater.
  ** See also: allocateCursor().
  */
  nMem += nCursor;
  ಆದರೆ( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(ಗಾತ್ರ(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  ಪ್ರತಿಜ್ಞಾ( x.nFree>=0 );
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_ಮೌಲ್ಯಗಳು(p, &nArg);
  p->ಬಳಸುತ್ತದೆ_Stmt_Journal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  ಆದರೆ( pParse->explain ){
    ಆದರೆ( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->n_Res_ಸ್ತಂಭ = 12 - 4*p->explain;
  }
  p->expired = 0;

  /* Memory for registers, parameters, cursor, etc, is allocated in one or two
  ** passes.  On the first pass, we try to reuse unused memory at the
  ** end of the opcode array.  If we are unable to satisfy all memory
  ** requirements by reusing the opcode array tail, then the second
  ** pass will fill in the remainder using a fresh memory allocation. 
  **
  ** This two-pass approach that reuses as much memory as possible from
  ** the leftover memory at the end of the opcode array.  This can significantly
  ** reduce the amount of memory held by a prepared statement.
  */
  x.nNeeded = 0;
  p->aMem = allocSpace(&x, 0, nMem*ಗಾತ್ರ(Mem));
  p->aVar = allocSpace(&x, 0, nVar*ಗಾತ್ರ(Mem));
  p->apArg = allocSpace(&x, 0, nArg*ಗಾತ್ರ(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*ಗಾತ್ರ(VdbeCursor*));
  ಆದರೆ( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    ಆದರೆ( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*ಗಾತ್ರ(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*ಗಾತ್ರ(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*ಗಾತ್ರ(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*ಗಾತ್ರ(VdbeCursor*));
    }
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p->napArg = nArg;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*ಗಾತ್ರ(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
ಶೂನ್ಯ sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  ಆದರೆ( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  ಪ್ರತಿಜ್ಞಾ( pCx->ಲಂಬಸಾಲು_Cache );
  pCx->ಲಂಬಸಾಲು_Cache = 0;
  pCx->pCache = 0;
  ಆದರೆ( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
ಶೂನ್ಯ sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  ಆದರೆ( pCx->ಲಂಬಸಾಲು_Cache ){
    freeCursorWithCache(p, pCx);
    ಹಿಂತಿರುಗಿ;
  }
  ಆಯ್ಕೆ( pCx->eCurType ){
    ಸ್ಥಿತಿ CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ CURTYPE_BTREE: {
      ಪ್ರತಿಜ್ಞಾ( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಸ್ಥಿತಿ CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      ಸ್ಥಿರಾಂಕ sqlite3_module *pModule = pVCur->pVtab->pModule;
      ಪ್ರತಿಜ್ಞಾ( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
}

/*
** Close all cursors in the current frame.
*/
ಸ್ಥಿರ ಶೂನ್ಯ close_Cursors_ಒಳ_ಚೌಕಟ್ಟು(Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    ಆದರೆ( pC ){
      sqlite3VdbeFreeCursorNN(p, pC);
      p->apCsr[i] = 0;
    }
  }
}

/*
** Copy the values stored in the VdbeFrame structure to its Vdbe. This
** is used, for example, when a trigger sub-program is halted to restore
** control to the main program.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಚೌಕಟ್ಟು_Restore(Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು){
  Vdbe *v = p_ಚೌಕಟ್ಟು->v;
  close_Cursors_ಒಳ_ಚೌಕಟ್ಟು(v);
  v->aOp = p_ಚೌಕಟ್ಟು->aOp;
  v->nOp = p_ಚೌಕಟ್ಟು->nOp;
  v->aMem = p_ಚೌಕಟ್ಟು->aMem;
  v->nMem = p_ಚೌಕಟ್ಟು->nMem;
  v->apCsr = p_ಚೌಕಟ್ಟು->apCsr;
  v->nCursor = p_ಚೌಕಟ್ಟು->nCursor;
  v->db->ಕೊನೆಯ_Rowid = p_ಚೌಕಟ್ಟು->ಕೊನೆಯ_Rowid;
  v->nChange = p_ಚೌಕಟ್ಟು->nChange;
  v->db->nChange = p_ಚೌಕಟ್ಟು->nDbChange;
  sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(v->db, &v->p_Aux_ದತ್ತಾಂಶ, -1, 0);
  v->p_Aux_ದತ್ತಾಂಶ = p_ಚೌಕಟ್ಟು->p_Aux_ದತ್ತಾಂಶ;
  p_ಚೌಕಟ್ಟು->p_Aux_ದತ್ತಾಂಶ = 0;
  ಹಿಂತಿರುಗಿ p_ಚೌಕಟ್ಟು->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
ಸ್ಥಿರ ಶೂನ್ಯ closeAllCursors(Vdbe *p){
  ಆದರೆ( p->p_ಚೌಕಟ್ಟು ){
    Vdbe_ಚೌಕಟ್ಟು *p_ಚೌಕಟ್ಟು;
    ಗಾಗಿ(p_ಚೌಕಟ್ಟು=p->p_ಚೌಕಟ್ಟು; p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ; p_ಚೌಕಟ್ಟು=p_ಚೌಕಟ್ಟು->p_ಪೋಷಕ);
    sqlite3_Vdbe_ಚೌಕಟ್ಟು_Restore(p_ಚೌಕಟ್ಟು);
    p->p_ಚೌಕಟ್ಟು = 0;
    p->n_ಚೌಕಟ್ಟು = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( p->n_ಚೌಕಟ್ಟು==0 );
  close_Cursors_ಒಳ_ಚೌಕಟ್ಟು(p);
  releaseMemArray(p->aMem, p->nMem);
  ಆಗ( p->p_Del_ಚೌಕಟ್ಟು ){
    Vdbe_ಚೌಕಟ್ಟು *pDel = p->p_Del_ಚೌಕಟ್ಟು;
    p->p_Del_ಚೌಕಟ್ಟು = pDel->p_ಪೋಷಕ;
    sqlite3_Vdbe_ಚೌಕಟ್ಟು_Delete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  ಆದರೆ( p->p_Aux_ದತ್ತಾಂಶ ) sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(p->db, &p->p_Aux_ದತ್ತಾಂಶ, -1, 0);
  ಪ್ರತಿಜ್ಞಾ( p->p_Aux_ದತ್ತಾಂಶ==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_ಅಂಕೆ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(Vdbe *p, ಪೂರ್ಣಾಂಕ n_Res_ಸ್ತಂಭ){
  ಪೂರ್ಣಾಂಕ n;
  sqlite3 *db = p->db;

  ಆದರೆ( p->nResAlloc ){
    releaseMemArray(p->a_ಲಂಬಸಾಲು_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->a_ಲಂಬಸಾಲು_Name);
  }
  n = n_Res_ಸ್ತಂಭ*COLNAME_N;
  p->n_Res_ಸ್ತಂಭ = p->nResAlloc = (u16)n_Res_ಸ್ತಂಭ;
  p->a_ಲಂಬಸಾಲು_Name = (Mem*)sqlite3DbMallocRawNN(db, ಗಾತ್ರ(Mem)*n );
  ಆದರೆ( p->a_ಲಂಬಸಾಲು_Name==0 ) ಹಿಂತಿರುಗಿ;
  initMemArray(p->a_ಲಂಬಸಾಲು_Name, n, db, MEM_Null);
}

/*
** Set the name of the idx'th column to be returned by the SQL statement.
** zName must be a pointer to a nul terminated string.
**
** This call must be made after a call to sqlite3VdbeSetNumCols().
**
** The final parameter, xDel, must be one of SQLITE_DYNAMIC, SQLITE_STATIC
** or SQLITE_TRANSIENT. If it is SQLITE_DYNAMIC, then the buffer pointed
** to by zName will be freed by sqlite3DbFree() when the vdbe is destroyed.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(
  Vdbe *p,                         /* Vdbe being configured */
  ಪೂರ್ಣಾಂಕ idx,                         /* Index of column zName applies to */
  ಪೂರ್ಣಾಂಕ var,                         /* One of the COLNAME_* constants */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,               /* Pointer to buffer containing name */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*)              /* Memory management strategy for zName */
){
  ಪೂರ್ಣಾಂಕ rc;
  Mem *p_ಲಂಬಸಾಲು_Name;
  ಪ್ರತಿಜ್ಞಾ( idx<p->nResAlloc );
  ಪ್ರತಿಜ್ಞಾ( var<COLNAME_N );
  ಆದರೆ( p->db->mallocFailed ){
    ಪ್ರತಿಜ್ಞಾ( !zName || xDel!=SQLITE_DYNAMIC );
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( p->a_ಲಂಬಸಾಲು_Name!=0 );
  p_ಲಂಬಸಾಲು_Name = &(p->a_ಲಂಬಸಾಲು_Name[idx+var*p->nResAlloc]);
  rc = sqlite3_Vdbe_Mem_ಗಣ_ಪಠ್ಯ(p_ಲಂಬಸಾಲು_Name, zName, -1, xDel);
  ಪ್ರತಿಜ್ಞಾ( rc!=0 || !zName || (p_ಲಂಬಸಾಲು_Name->flags&MEM_ಪದ)!=0 );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeCommit(sqlite3 *db, Vdbe *p){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ needXcommit = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  ಬಳಸದ_PARAMETER(p);
#ಆದರೆ_ಅಂತ್ಯ

  /* Before doing anything else, call the xSync() callback for any
  ** virtual module tables written in this transaction. This has to
  ** be done before determining whether a super-journal file is
  ** required, as an xSync() callback may add an attached database
  ** to the transaction.
  */
  rc = sqlite3VtabSync(db, p);

  /* This loop determines (a) if the commit hook should be invoked and
  ** (b) how many database files have open write transactions, not
  ** including the temp database. (b) is important because if more than
  ** one database file has an open write transaction, a super-journal
  ** file is required for an atomic commit.
  */
  ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /*ಸ್ಮರಣೆ*/ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      ಆದರೆ( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3_Pager_Get_Journal_ಬಹುಲಕ(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        ಪ್ರತಿಜ್ಞಾ( i!=1 );
        nTrans++;
      }
      rc = sqlite3_Pager_Exclusive_ಬೀಗ(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  ಆದರೆ( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    ಆದರೆ( rc ){
      ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT_COMMITHOOK;
    }
  }

  /* The simple case - no more than one database file (not counting the
  ** TEMP database) has a transaction active.   There is no need for the
  ** super-journal.
  **
  ** If the return value of sqlite3BtreeGetFilename() is a zero length
  ** string, it means the main database is :memory: or a temp file.  In
  ** that case we do not support atomic multi-file commits, so use the
  ** simple case then too.
  */
  ಆದರೆ( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    ಆದರೆ( needXcommit ){
      ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ಆದರೆ( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ಪೂರ್ಣಾಂಕ txn = sqlite3BtreeTxnState(pBt);
      ಆದರೆ( txn!=SQLITE_TXN_NONE ){
        ಪ್ರತಿಜ್ಞಾ( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DISKIO
  ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_vfs *pVfs = db->pVfs;
    ಅಕ್ಷರ *zSuper = 0;   /* File-name for the super-journal */
    ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 ಸ್ಥಾನಾಂತರ = 0;
    ಪೂರ್ಣಾಂಕ res;
    ಪೂರ್ಣಾಂಕ retry_ಎಣಿಕೆ = 0;
    ಪೂರ್ಣಾಂಕ nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    ಆದರೆ( zSuper==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    zSuper += 4;
    ಮಾಡು {
      u32 iRandom;
      ಆದರೆ( retry_ಎಣಿಕೆ ){
        ಆದರೆ( retry_ಎಣಿಕೆ>100 ){
          sqlite3_log(SQLITE_ತುಂಬಿದ, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( retry_ಎಣಿಕೆ==1 ){
          sqlite3_log(SQLITE_ತುಂಬಿದ, "MJ collide: %s", zSuper);
        }
      }
      retry_ಎಣಿಕೆ++;
      sqlite3_randomness(ಗಾತ್ರ(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      ಪ್ರತಿಜ್ಞಾ( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }ಆಗ( rc==SQLITE_OK && res );
    ಆದರೆ( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      ಹಿಂತಿರುಗಿ rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ಆದರೆ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zFile = sqlite3BtreeGetJournalname(pBt);
        ಆದರೆ( zFile==0 ){
          ಮುಂದುವರಿ;  /* Ignore TEMP and :memory: databases */
        }
        ಪ್ರತಿಜ್ಞಾ( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,ಸ್ಥಾನಾಂತರ);
        ಸ್ಥಾನಾಂತರ += sqlite3Strlen30(zFile)+1;
        ಆದರೆ( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          ಹಿಂತಿರುಗಿ rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    ಆದರೆ( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_ಸಾಮಾನ್ಯ))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      ಹಿಂತಿರುಗಿ rc;
    }

    /* Sync all the db files involved in the transaction. The same call
    ** sets the super-journal pointer in each individual journal. If
    ** an error occurs here, do not delete the super-journal file.
    **
    ** If the error occurs during the first call to
    ** sqlite3BtreeCommitPhaseOne(), then there is a chance that the
    ** super-journal file will be orphaned. But we cannot delete it,
    ** in case the super-journal file name was written into the journal
    ** file before the failure occurred.
    */
    ಗಾಗಿ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ಆದರೆ( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_BUSY );
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      ಹಿಂತಿರುಗಿ rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    ಆದರೆ( rc ){
      ಹಿಂತಿರುಗಿ rc;
    }

    /* All files and directories have already been synced, so the following
    ** calls to sqlite3BtreeCommitPhaseTwo() are only closing files and
    ** deleting or truncating journals. If something goes wrong while
    ** this is happening we don't really care. The integrity of the
    ** transaction is already guaranteed, but some stray 'cold' journals
    ** may be lying around. Returning an error code won't help matters.
    */
    disable_simulated_io_errors();
    sqlite3BeginBenignMalloc();
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ಆದರೆ( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3_ಕೊನೆ_Benign_Malloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine checks that the sqlite3.nVdbeActive count variable
** matches the number of vdbe's in the list sqlite3.pVdbe that are
** currently active. An assertion fails if the two counts do not match.
** This is an internal self-check only - it is not an essential processing
** step.
**
** This is a no-op if NDEBUG is defined.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಸ್ಥಿರ ಶೂನ್ಯ check_ಸಕ್ರಿಯ_Vdbe_Cnt(sqlite3 *db){
  Vdbe *p;
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಪೂರ್ಣಾಂಕ nWrite = 0;
  ಪೂರ್ಣಾಂಕ nRead = 0;
  p = db->pVdbe;
  ಆಗ( p ){
    ಆದರೆ( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      ಆದರೆ( p->readOnly==0 ) nWrite++;
      ಆದರೆ( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  ಪ್ರತಿಜ್ಞಾ( cnt==db->n_Vdbe_ಸಕ್ರಿಯ );
  ಪ್ರತಿಜ್ಞಾ( nWrite==db->nVdbeWrite );
  ಪ್ರತಿಜ್ಞಾ( nRead==db->nVdbeRead );
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು check_ಸಕ್ರಿಯ_Vdbe_Cnt(x)
#ಆದರೆ_ಅಂತ್ಯ

/*
** If the Vdbe passed as the first argument opened a statement-transaction,
** close it now. Argument eOp must be either SAVEPOINT_ROLLBACK or
** SAVEPOINT_RELEASE. If it is SAVEPOINT_ROLLBACK, then the statement
** transaction is rolled back. If eOp is SAVEPOINT_RELEASE, then the
** statement transaction is committed.
**
** If an IO error occurs, an SQLITE_IOERR_XXX error code is returned.
** Otherwise SQLITE_OK.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbeCloseStatement(Vdbe *p, ಪೂರ್ಣಾಂಕ eOp){
  sqlite3 *ಸ್ಥಿರಾಂಕ db = p->db;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iSavepoint = p->iStatement-1;

  ಪ್ರತಿಜ್ಞಾ( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  ಪ್ರತಿಜ್ಞಾ( db->nStatement>0 );
  ಪ್ರತಿಜ್ಞಾ( p->iStatement==(db->nStatement+db->nSavepoint) );

  ಗಾಗಿ(i=0; i<db->nDb; i++){
    ಪೂರ್ಣಾಂಕ rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    ಆದರೆ( pBt ){
      ಆದರೆ( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      ಆದರೆ( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    ಆದರೆ( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  ಆದರೆ( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3VdbeCloseStatement(Vdbe *p, ಪೂರ್ಣಾಂಕ eOp){
  ಆದರೆ( p->db->nStatement && p->iStatement ){
    ಹಿಂತಿರುಗಿ vdbeCloseStatement(p, eOp);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** These functions are called when a transaction opened by the database
** handle associated with the VM passed as an argument is about to be
** committed. If there are outstanding foreign key constraint violations
** return an error code. Otherwise, SQLITE_OK.
**
** If there are outstanding FK violations and this function returns
** non-zero, set the result of the VM to SQLITE_CONSTRAINT_FOREIGNKEY
** and write an error message to it.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  ಆದರೆ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT_FOREIGNKEY;
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Check_Fk_ತಕ್ಷಣ(Vdbe *p){
  ಆದರೆ( p->nFkConstraint==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಹಿಂತಿರುಗಿ vdbeFkError(p);
}
ಪೂರ್ಣಾಂಕ sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  ಆದರೆ( (db->nDeferredCons+db->nDeferredImmCons)==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಹಿಂತಿರುಗಿ vdbeFkError(p);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine is called the when a VDBE tries to halt.  If the VDBE
** has made changes and is in autocommit mode, then commit those
** changes.  If a rollback is needed, then do the rollback.
**
** This routine is the only way to move the sqlite3eOpenState of a VM from
** SQLITE_STATE_RUN to SQLITE_STATE_HALT.  It is harmless to
** call this on a VM that is in the SQLITE_STATE_HALT state.
**
** Return an error code.  If the commit could not complete because of
** lock contention, return SQLITE_BUSY.  If SQLITE_BUSY is returned, it
** means the close did not happen and needs to be repeated.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeHalt(Vdbe *p){
  ಪೂರ್ಣಾಂಕ rc;                         /* Used to store transient return codes */
  sqlite3 *db = p->db;

  /* This function contains the logic that determines if a statement or
  ** transaction will be committed or rolled back as a result of the
  ** execution of this virtual machine.
  **
  ** If any of the following errors occur:
  **
  **     SQLITE_NOMEM
  **     SQLITE_IOERR
  **     SQLITE_FULL
  **     SQLITE_INTERRUPT
  **
  ** Then the internal cache might have been left in an inconsistent
  ** state.  We need to rollback the statement transaction, if there is
  ** one, or the complete transaction if there is no statement transaction.
  */

  ಪ್ರತಿಜ್ಞಾ( p->eVdbeState==VDBE_ಸತತ_STATE );
  ಆದರೆ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  check_ಸಕ್ರಿಯ_Vdbe_Cnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  ಆದರೆ( p->bIsReader ){
    ಪೂರ್ಣಾಂಕ mrc;   /* Primary error code from p->rc */
    ಪೂರ್ಣಾಂಕ eStatementOp = 0;
    ಪೂರ್ಣಾಂಕ isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    ಆದರೆ( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_ಅಡಚಣೆ
                    || mrc==SQLITE_ತುಂಬಿದ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      mrc = isSpecialError = 0;
    }
    ಆದರೆ( isSpecialError ){
      /* If the query was read-only and the error code is SQLITE_INTERRUPT,
      ** no rollback is necessary. Otherwise, at least a savepoint
      ** transaction must be rolled back to restore the database to a
      ** consistent state.
      **
      ** Even if the statement is read-only, it is important to perform
      ** a statement or transaction rollback operation. If the error
      ** occurred while writing to the journal, sub-journal or database
      ** file as part of an effort to free up cache space (see function
      ** pagerStress() in pager.c), the rollback is required to restore
      ** the pager to a consistent state.
      */
      ಆದರೆ( !p->readOnly || mrc!=SQLITE_ಅಡಚಣೆ ){
        ಆದರೆ( (mrc==SQLITE_NOMEM || mrc==SQLITE_ತುಂಬಿದ) && p->ಬಳಸುತ್ತದೆ_Stmt_Journal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* We are forced to roll back the active transaction. Before doing
          ** so, abort any other statements this handle currently has active.
          */
          sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
          sqlite3CloseSavepoints(db);
          db->autoCommit = 1;
          p->nChange = 0;
        }
      }
    }

    /* Check for immediate foreign key violations. */
    ಆದರೆ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (ಶೂನ್ಯ)sqlite3_Vdbe_Check_Fk_ತಕ್ಷಣ(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    ಆದರೆ( !sqlite3_Vtab_ಒಳ_Sync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      ಆದರೆ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        ಆದರೆ( rc!=SQLITE_OK ){
          ಆದರೆ( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            ಹಿಂತಿರುಗಿ SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        ಆದರೆ( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          ಹಿಂತಿರುಗಿ SQLITE_BUSY;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3_Commit_ಆಂತರಿಕ_Changes(db);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->rc==SQLITE_SCHEMA && db->n_Vdbe_ಸಕ್ರಿಯ>1 ){
        p->nChange = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eStatementOp==0 ){
      ಆದರೆ( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
        p->nChange = 0;
      }
    }
 
    /* If eStatementOp is non-zero, then a statement transaction needs to
    ** be committed or rolled back. Call sqlite3VdbeCloseStatement() to
    ** do so. If this operation returns an error, and the current statement
    ** error code is SQLITE_OK or SQLITE_CONSTRAINT, then promote the
    ** current statement error code.
    */
    ಆದರೆ( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      ಆದರೆ( rc ){
        ಆದರೆ( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_ಸಂದೇಶ);
          p->z_Err_ಸಂದೇಶ = 0;
        }
        sqlite3RollbackAll(db, SQLITE_ABORT_ROLLBACK);
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
        p->nChange = 0;
      }
    }
 
    /* If this was an INSERT, UPDATE or DELETE and no statement transaction
    ** has been rolled back, update the database connection change-counter.
    */
    ಆದರೆ( p->changeCntOn ){
      ಆದರೆ( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3_Vdbe_ಗಣ_Changes(db, p->nChange);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಗಣ_Changes(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->n_Vdbe_ಸಕ್ರಿಯ--;
  ಆದರೆ( !p->readOnly ) db->nVdbeWrite--;
  ಆದರೆ( p->bIsReader ) db->nVdbeRead--;
  ಪ್ರತಿಜ್ಞಾ( db->n_Vdbe_ಸಕ್ರಿಯ>=db->nVdbeRead );
  ಪ್ರತಿಜ್ಞಾ( db->nVdbeRead>=db->nVdbeWrite );
  ಪ್ರತಿಜ್ಞಾ( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  check_ಸಕ್ರಿಯ_Vdbe_Cnt(db);
  ಆದರೆ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  ಆದರೆ( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  ಪ್ರತಿಜ್ಞಾ( db->n_Vdbe_ಸಕ್ರಿಯ>0 || db->autoCommit==0 || db->nStatement==0 );
  ಹಿಂತಿರುಗಿ (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
ಶೂನ್ಯ sqlite3_Vdbe_Reset_ಹೆಜ್ಜೆ_ಫಲಿತಾಂಶ(Vdbe *p){
  p->rc = SQLITE_OK;
}

/*
** Copy the error code and error message belonging to the VDBE passed
** as the first argument to its database handle (so that they will be
** returned by calls to sqlite3_errcode() and sqlite3_errmsg()).
**
** This function does not clear the VDBE error code or message, just
** copies them to the database handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  ಪೂರ್ಣಾಂಕ rc = p->rc;
  ಆದರೆ( p->z_Err_ಸಂದೇಶ ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    ಆದರೆ( db->pErr==0 ) db->pErr = sqlite3_ಮೌಲ್ಯ_New(db);
    sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(db->pErr, -1, p->z_Err_ಸಂದೇಶ, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3_ಕೊನೆ_Benign_Malloc();
    db->bBenignMalloc--;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->pErr ){
    sqlite3_ಮೌಲ್ಯ_ಗಣ_Null(db->pErr);
  }
  db->err_ಸಂಕೇತ = rc;
  db->err_Byte_ಸ್ಥಾನಾಂತರ = -1;
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeInvokeSqllog(Vdbe *v){
  ಆದರೆ( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    ಅಕ್ಷರ *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    ಪ್ರತಿಜ್ಞಾ( v->db->init.busy==0 );
    ಆದರೆ( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define vdbeInvokeSqllog(x)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Clean up a VDBE after execution but do not delete the VDBE just yet.
** Write any error messages into *pzErrMsg.  Return the result code.
**
** After this routine is run, the VDBE should be ready to be executed
** again.
**
** To look at it another way, this routine resets the state of the
** virtual machine from VDBE_RUN_STATE or VDBE_HALT_STATE back to
** VDBE_READY_STATE.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeReset(Vdbe *p){
#ಆದರೆ defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  ಪೂರ್ಣಾಂಕ i;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  ಆದರೆ( p->eVdbeState==VDBE_ಸತತ_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  ಆದರೆ( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    ಆದರೆ( db->pErr || p->z_Err_ಸಂದೇಶ ){
      sqlite3VdbeTransferError(p);
    }ಇಲ್ಲದಿದ್ದರೆ{
      db->err_ಸಂಕೇತ = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  ಆದರೆ( p->apCsr ) ಗಾಗಿ(i=0; i<p->nCursor; i++) ಪ್ರತಿಜ್ಞಾ( p->apCsr[i]==0 );
  ಆದರೆ( p->aMem ){
    ಗಾಗಿ(i=0; i<p->nMem; i++) ಪ್ರತಿಜ್ಞಾ( p->aMem[i].flags==MEM_Undefined );
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->z_Err_ಸಂದೇಶ ){
    sqlite3DbFree(db, p->z_Err_ಸಂದೇಶ);
    p->z_Err_ಸಂದೇಶ = 0;
  }
  p->p_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  p->nWrite = 0;
#ಆದರೆ_ಅಂತ್ಯ

  /* Save profiling information from this VDBE run.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VDBE_PROFILE
  {
    FILE *ಹೊರಹೋಗು = fopen("vdbe_profile.out", "a");
    ಆದರೆ( ಹೊರಹೋಗು ){
      fprintf(ಹೊರಹೋಗು, "---- ");
      ಗಾಗಿ(i=0; i<p->nOp; i++){
        fprintf(ಹೊರಹೋಗು, "%02x", p->aOp[i].opcode);
      }
      fprintf(ಹೊರಹೋಗು, "\n");
      ಆದರೆ( p->zSql ){
        ಅಕ್ಷರ c, pc = 0;
        fprintf(ಹೊರಹೋಗು, "-- ");
        ಗಾಗಿ(i=0; (c = p->zSql[i])!=0; i++){
          ಆದರೆ( pc=='\n' ) fprintf(ಹೊರಹೋಗು, "-- ");
          putc(c, ಹೊರಹೋಗು);
          pc = c;
        }
        ಆದರೆ( pc!='\n' ) fprintf(ಹೊರಹೋಗು, "\n");
      }
      ಗಾಗಿ(i=0; i<p->nOp; i++){
        ಅಕ್ಷರ zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 ಚಕ್ರಗಳು = p->aOp[i].n_ಚಕ್ರ;
        sqlite3_snprintf(ಗಾತ್ರ(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           ಚಕ್ರಗಳು,
           cnt>0 ? ಚಕ್ರಗಳು/cnt : 0
        );
        fprintf(ಹೊರಹೋಗು, "%s", zHdr);
        sqlite3VdbePrintOp(ಹೊರಹೋಗು, i, &p->aOp[i]);
      }
      fclose(ಹೊರಹೋಗು);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ p->rc & db->err_ಮುಖವಾಡ;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeFinalize(Vdbe *p){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( VDBE_ಸತತ_STATE>VDBE_READY_STATE );
  ಪ್ರತಿಜ್ಞಾ( VDBE_HALT_STATE>VDBE_READY_STATE );
  ಪ್ರತಿಜ್ಞಾ( VDBE_INIT_STATE<VDBE_READY_STATE );
  ಆದರೆ( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    ಪ್ರತಿಜ್ಞಾ( (rc & p->db->err_ಮುಖವಾಡ)==rc );
  }
  sqlite3VdbeDelete(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** If parameter iOp is less than zero, then invoke the destructor for
** all auxiliary data pointers currently cached by the VM passed as
** the first argument.
**
** Or, if iOp is greater than or equal to zero, then the destructor is
** only invoked for those auxiliary data pointers created by the user
** function invoked by the OP_Function opcode at instruction iOp of
** VM pVdbe, and only then if:
**
**    * the associated function parameter is the 32nd or later (counting
**      from left to right), or
**
**    * the corresponding bit in argument mask is clear (where the first
**      function parameter corresponds to bit 0 etc.).
*/
ಶೂನ್ಯ sqlite3_Vdbe_Delete_Aux_ದತ್ತಾಂಶ(sqlite3 *db, Aux_ದತ್ತಾಂಶ **pp, ಪೂರ್ಣಾಂಕ iOp, ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ){
  ಆಗ( *pp ){
    Aux_ದತ್ತಾಂಶ *pAux = *pp;
    ಆದರೆ( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(ಮುಖವಾಡ & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      ಆದರೆ( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_ಮುಂದಿನ_Aux;
      sqlite3DbFree(db, pAux);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pp= &pAux->p_ಮುಂದಿನ_Aux;
    }
  }
}

/*
** Free all memory associated with the Vdbe passed as the second argument,
** except for object itself, which is preserved.
**
** The difference between this function and sqlite3VdbeDelete() is that
** VdbeDelete() also unlinks the Vdbe from the list of VMs associated with
** the database connection and frees the object itself.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  Sub_ಕಾರ್ಯಕ್ರಮ *pSub, *p_ಮುಂದಿನ;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->db==0 || p->db==db );
  ಆದರೆ( p->a_ಲಂಬಸಾಲು_Name ){
    releaseMemArray(p->a_ಲಂಬಸಾಲು_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->a_ಲಂಬಸಾಲು_Name);
  }
  ಗಾಗಿ(pSub=p->p_ಕಾರ್ಯಕ್ರಮ; pSub; pSub=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = pSub->p_ಮುಂದಿನ;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  ಆದರೆ( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    ಆದರೆ( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    ಆದರೆ( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  ಆದರೆ( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    ಗಾಗಿ(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_ಮುಂದಿನ_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->n_ಪರಿಶೀಲನೆ; i++){
      sqlite3DbFree(db, p->a_ಪರಿಶೀಲನೆ[i].zName);
    }
    sqlite3DbFree(db, p->a_ಪರಿಶೀಲನೆ);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Delete an entire VDBE.
*/
ಶೂನ್ಯ sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  db = p->db;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ){
    ಪ್ರತಿಜ್ಞಾ( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    ಆದರೆ( p->pVNext ){
      p->pVNext->ppVPrev = p->ppVPrev;
    }
  }
  sqlite3DbNNFreeNN(db, p);
}

/*
** The cursor "p" has a pending seek operation that has not yet been
** carried out.  Seek the cursor now.  If an error occurs, return
** the appropriate error code.
*/
ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE sqlite3_Vdbe_ಮುಕ್ತಾಯ_Moveto(VdbeCursor *p){
  ಪೂರ್ಣಾಂಕ res, rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_ಹುಡುಕು_ಎಣಿಕೆ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p->deferredMoveto );
  ಪ್ರತಿಜ್ಞಾ( p->is_ಕೋಷ್ಟಕ );
  ಪ್ರತಿಜ್ಞಾ( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(p->uc.pCursor, p->moveto_ಗುರಿ, 0, &res);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  ಆದರೆ( res!=0 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  sqlite3_ಹುಡುಕು_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
  p->deferredMoveto = 0;
  p->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  ಪೂರ್ಣಾಂಕ is_Different_ಅಡ್ಡಸಾಲು, rc;
  ಪ್ರತಿಜ್ಞಾ( p->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( p->uc.pCursor!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &is_Different_ಅಡ್ಡಸಾಲು);
  p->cache_ಸ್ಥಿತಿ = CACHE_STALE;
  ಆದರೆ( is_Different_ಅಡ್ಡಸಾಲು ) p->null_ಅಡ್ಡಸಾಲು = 1;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeCursorRestore(VdbeCursor *p){
  ಪ್ರತಿಜ್ಞಾ( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  ಆದರೆ( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    ಹಿಂತಿರುಗಿ sqlite3VdbeHandleMovedCursor(p);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** The following functions:
**
** sqlite3VdbeSerialType()
** sqlite3VdbeSerialTypeLen()
** sqlite3VdbeSerialLen()
** sqlite3VdbeSerialPut()  <--- in-lined into OP_MakeRecord as of 2022-04-02
** sqlite3VdbeSerialGet()
**
** encapsulate the code that serializes values for storage in SQLite
** data and index records. Each serialized value consists of a
** 'serial-type' and a blob of data. The serial type is an 8-byte unsigned
** integer, stored as a varint.
**
** In an SQLite index record, the serial type is stored directly before
** the blob of data that it corresponds to. In a table record, all serial
** types are stored at the start of the record, and the blobs of data at
** the end. Hence these functions allow the caller to handle the
** serial-type and data blob separately.
**
** The following table describes the various storage classes for data:
**
**   serial type        bytes of data      type
**   --------------     ---------------    ---------------
**      0                     0            NULL
**      1                     1            signed integer
**      2                     2            signed integer
**      3                     3            signed integer
**      4                     4            signed integer
**      5                     6            signed integer
**      6                     8            signed integer
**      7                     8            IEEE float
**      8                     0            Integer constant 0
**      9                     0            Integer constant 1
**     10,11                               reserved for expansion
**    N>=12 and even       (N-12)/2        BLOB
**    N>=13 and odd        (N-13)/2        text
**
** The 8 and 9 types were added in 3.3.0, file format 4.  Prior versions
** of SQLite will not understand those serial types.
*/

#ಆದರೆ 0 /* Inlined into the OP_MakeRecord opcode */
/*
** Return the serial-type for the value stored in pMem.
**
** This routine might convert a large MEM_IntReal value into MEM_Real.
**
** 2019-07-11:  The primary user of this subroutine was the OP_MakeRecord
** opcode in the byte-code engine.  But by moving this routine in-line, we
** can omit some redundant tests and make that opcode a lot faster.  So
** this routine is now only used by the STAT3 logic and STAT3 support has
** ended.  The code is kept here for historical reference only.
*/
u32 sqlite3VdbeSerialType(Mem *pMem, ಪೂರ್ಣಾಂಕ file_format, u32 *pLen){
  ಪೂರ್ಣಾಂಕ flags = pMem->flags;
  u32 n;

  ಪ್ರತಿಜ್ಞಾ( pLen!=0 );
  ಆದರೆ( flags&MEM_Null ){
    *pLen = 0;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    ಆದರೆ( i<0 ){
      u = ~i;
    }ಇಲ್ಲದಿದ್ದರೆ{
      u = i;
    }
    ಆದರೆ( u<=127 ){
      ಆದರೆ( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        ಹಿಂತಿರುಗಿ 8+(u32)u;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *pLen = 1;
        ಹಿಂತಿರುಗಿ 1;
      }
    }
    ಆದರೆ( u<=32767 ){ *pLen = 2; ಹಿಂತಿರುಗಿ 2; }
    ಆದರೆ( u<=8388607 ){ *pLen = 3; ಹಿಂತಿರುಗಿ 3; }
    ಆದರೆ( u<=2147483647 ){ *pLen = 4; ಹಿಂತಿರುಗಿ 4; }
    ಆದರೆ( u<=MAX_6BYTE ){ *pLen = 6; ಹಿಂತಿರುಗಿ 5; }
    *pLen = 8;
    ಆದರೆ( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (ದ್ವಿಗುಣ)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      ಹಿಂತಿರುಗಿ 7;
    }
    ಹಿಂತಿರುಗಿ 6;
  }
  ಆದರೆ( flags&MEM_Real ){
    *pLen = 8;
    ಹಿಂತಿರುಗಿ 7;
  }
  ಪ್ರತಿಜ್ಞಾ( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  ಪ್ರತಿಜ್ಞಾ( pMem->n>=0 );
  n = (u32)pMem->n;
  ಆದರೆ( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  ಹಿಂತಿರುಗಿ ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#ಆದರೆ_ಅಂತ್ಯ /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
ಸ್ಥಿರಾಂಕ u8 sqlite3SmallTypeSizes[128] = {
        /*  0   1   2   3   4   5   6   7   8   9 */  
/*   0 */   0,  1,  2,  3,  4,  6,  8,  8,  0,  0,
/*  10 */   0,  0,  0,  0,  1,  1,  2,  2,  3,  3,
/*  20 */   4,  4,  5,  5,  6,  6,  7,  7,  8,  8,
/*  30 */   9,  9, 10, 10, 11, 11, 12, 12, 13, 13,
/*  40 */  14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
/*  50 */  19, 19, 20, 20, 21, 21, 22, 22, 23, 23,
/*  60 */  24, 24, 25, 25, 26, 26, 27, 27, 28, 28,
/*  70 */  29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
/*  80 */  34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
/*  90 */  39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
/* 100 */  44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
/* 110 */  49, 49, 50, 50, 51, 51, 52, 52, 53, 53,
/* 120 */  54, 54, 55, 55, 56, 56, 57, 57
};

/*
** Return the length of the data corresponding to the supplied serial-type.
*/
u32 sqlite3VdbeSerialTypeLen(u32 serial_type){
  ಆದರೆ( serial_type>=128 ){
    ಹಿಂತಿರುಗಿ (serial_type-12)/2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    ಹಿಂತಿರುಗಿ sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  ಪ್ರತಿಜ್ಞಾ( serial_type<128 );
  ಹಿಂತಿರುಗಿ sqlite3SmallTypeSizes[serial_type]; 
}

/*
** If we are on an architecture with mixed-endian floating
** points (ex: ARM7) then swap the lower 4 bytes with the
** upper 4 bytes.  Return the result.
**
** For most architectures, this is a no-op.
**
** (later):  It is reported to me that the mixed-endian problem
** on ARM7 is an issue with GCC, not with the ARM7 chip.  It seems
** that early versions of GCC stored the two words of a 64-bit
** float in the wrong order.  And that error has been propagated
** ever since.  The blame is not necessarily with GCC, though.
** GCC might have just copying the problem from a prior compiler.
** I am also told that newer versions of GCC that follow a different
** ABI get the byte order right.
**
** Developers using SQLite on an ARM7 should compile and run their
** application using -DSQLITE_DEBUG=1 at least once.  With DEBUG
** enabled, some asserts below will ensure that the byte order of
** floating point values is correct.
**
** (2007-08-30)  Frank van Vugt has studied this problem closely
** and has send his findings to the SQLite developers.  Frank
** writes that some Linux kernels offer floating point hardware
** emulation that uses only 32-bit mantissas instead of a full
** 48-bits as required by the IEEE standard.  (This is the
** CONFIG_FPE_FASTFPE option.)  On such systems, floating point
** byte swapping becomes very complicated.  To avoid problems,
** the necessary byte swapping is carried out using a 64-bit integer
** rather than a 64-bit float.  Frank assures us that the code here
** works for him.  We, the developers, have no way to independently
** verify this, but Frank seems to know what he is talking about
** so we trust him.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MIXED_ENDIAN_64_ಬಿಟ್_FLOAT
u64 sqlite3_Float_ಅದಲುಬದಲು(u64 ಒಳ){
  ಒಕ್ಕೂಟ {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = ಒಳ;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  ಹಿಂತಿರುಗಿ u.r;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3VdbeSerialGet7(
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(x)==8 && ಗಾತ್ರ(pMem->u.r)==8 );
  ಅದಲುಬದಲು_Mixed_Endian_Float(x);
  ಸ್ಮರಣೆ_ನಕಲು(&pMem->u.r, &x, ಗಾತ್ರ(x));
  ಆದರೆ( IsNaN(x) ){
    pMem->flags = MEM_Null;
    ಹಿಂತಿರುಗಿ 1;
  }
  pMem->flags = MEM_Real;
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
ಶೂನ್ಯ sqlite3VdbeSerialGet(
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  ಆಯ್ಕೆ( serial_type ){
    ಸ್ಥಿತಿ 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 11:   /* Reserved for future use */
    ಸ್ಥಿತಿ 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      ಆದರೆ( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#ಆದರೆ_ಅಂತ್ಯ
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ 8:    /* Integer 0 */
    ಸ್ಥಿತಿ 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      ಹಿಂತಿರುಗಿ;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (ಅಕ್ಷರ *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಹಿಂತಿರುಗಿ;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
Unpacked_ದಾಖಲೆ *sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info               /* Description of the record */
){
  Unpacked_ದಾಖಲೆ *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(Unpacked_ದಾಖಲೆ) + ಗಾತ್ರ(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(ಗಾತ್ರ(Unpacked_ದಾಖಲೆ)) + ಗಾತ್ರ(Mem)*(p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ+1);
  p = (Unpacked_ದಾಖಲೆ *)sqlite3DbMallocRaw(p_ಕೀಲಿ_Info->db, nByte);
  ಆದರೆ( !p ) ಹಿಂತಿರುಗಿ 0;
  p->aMem = (Mem*)&((ಅಕ್ಷರ*)p)[ROUND8P(ಗಾತ್ರ(Unpacked_ದಾಖಲೆ))];
  p->p_ಕೀಲಿ_Info = p_ಕೀಲಿ_Info;
  p->n_ಕ್ಷೇತ್ರ = p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ + 1;
  ಹಿಂತಿರುಗಿ p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ದಾಖಲೆ_Unpack(
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ,              /* Size of the binary record */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಕೀಲಿ,      /* The binary record */
  Unpacked_ದಾಖಲೆ *p      /* Populate this structure before returning. */
){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಕೀಲಿ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)p_ಕೀಲಿ;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = p->p_ಕೀಲಿ_Info;

  p->default_rc = 0;
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_ಕೀಲಿ, szHdr);
  d = szHdr;
  u = 0;
  ಆಗ( idx<szHdr && d<=(u32)n_ಕೀಲಿ ){
    u32 serial_type;

    idx += getVarint32(&a_ಕೀಲಿ[idx], serial_type);
    pMem->enc = p_ಕೀಲಿ_Info->enc;
    pMem->db = p_ಕೀಲಿ_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_ಕೀಲಿ[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    ಆದರೆ( (++u)>=p->n_ಕ್ಷೇತ್ರ ) ನಿಲ್ಲಿಸು;
    pMem++;
  }
  ಆದರೆ( d>(u32)n_ಕೀಲಿ && u ){
    ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3_Vdbe_Mem_ಗಣ_Null(pMem-(u<p->n_ಕ್ಷೇತ್ರ));
  }
  testcase( u == p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ + 1 );
  testcase( u < p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ + 1 );
  ಪ್ರತಿಜ್ಞಾ( u<=p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ + 1 );
  p->n_ಕ್ಷೇತ್ರ = u;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** This function compares two index or table record keys in the same way
** as the sqlite3VdbeRecordCompare() routine. Unlike VdbeRecordCompare(),
** this function deserializes and compares values using the
** sqlite3VdbeSerialGet() and sqlite3MemCompare() functions. It is used
** in assert() statements to ensure that the optimized code in
** sqlite3VdbeRecordCompare() returns results with these two primitives.
**
** Return true if the result of comparison is equivalent to desiredResult.
** Return false if there is a disagreement.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ದಾಖಲೆ_Compare_Debug(
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, /* Left key */
  ಸ್ಥಿರಾಂಕ Unpacked_ದಾಖಲೆ *pPKey2, /* Right key */
  ಪೂರ್ಣಾಂಕ desired_ಫಲಿತಾಂಶ             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  ಪೂರ್ಣಾಂಕ i = 0;
  ಪೂರ್ಣಾಂಕ rc = 0;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *aKey1 = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)pKey1;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  Mem mem1;

  p_ಕೀಲಿ_Info = pPKey2->p_ಕೀಲಿ_Info;
  ಆದರೆ( p_ಕೀಲಿ_Info->db==0 ) ಹಿಂತಿರುಗಿ 1;
  mem1.enc = p_ಕೀಲಿ_Info->enc;
  mem1.db = p_ಕೀಲಿ_Info->db;
  /* mem1.flags = 0;  // Will be initialized by sqlite3VdbeSerialGet() */
  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */

  /* Compilers may complain that mem1.u.i is potentially uninitialized.
  ** We could initialize it, as shown here, to silence those complaints.
  ** But in fact, mem1.u.i will never actually be used uninitialized, and doing
  ** the unnecessary initialization has a measurable negative performance
  ** impact, since this routine is a very high runner.  And so, we choose
  ** to ignore the compiler warnings and leave this variable uninitialized.
  */
  /*  mem1.u.i = 0;  // not needed, here to silence compiler warning */
 
  idx1 = getVarint32(aKey1, szHdr1);
  ಆದರೆ( szHdr1>98307 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT;
  d1 = szHdr1;
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ>=pPKey2->n_ಕ್ಷೇತ್ರ || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ>0 );
  ಪ್ರತಿಜ್ಞಾ( idx1<=szHdr1 || CORRUPT_DB );
  ಮಾಡು{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    ಆದರೆ( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      ಆದರೆ( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        ಹಿಂತಿರುಗಿ 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      ನಿಲ್ಲಿಸು;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ>i ? p_ಕೀಲಿ_Info->aColl[i] : 0);
    ಆದರೆ( rc!=0 ){
      ಪ್ರತಿಜ್ಞಾ( mem1.szMalloc==0 );  /* See comment below */
      ಆದರೆ( (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[i] & KEYINFO_ಕ್ರಮ_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      ಆದರೆ( p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[i] & KEYINFO_ಕ್ರಮ_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      ಹೋಗು debug_Compare_ಕೊನೆ;
    }
    i++;
  }ಆಗ( idx1<szHdr1 && i<pPKey2->n_ಕ್ಷೇತ್ರ );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  ಪ್ರತಿಜ್ಞಾ( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debug_Compare_ಕೊನೆ:
  ಆದರೆ( desired_ಫಲಿತಾಂಶ==0 && rc==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( desired_ಫಲಿತಾಂಶ<0 && rc<0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( desired_ಫಲಿತಾಂಶ>0 && rc>0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( CORRUPT_DB ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p_ಕೀಲಿ_Info->db->mallocFailed ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Count the number of fields (a.k.a. columns) in the record given by
** pKey,nKey.  The verify that this count is less than or equal to the
** limit given by pKeyInfo->nAllField.
**
** If this constraint is not satisfied, it means that the high-speed
** vdbeRecordCompareInt() and vdbeRecordCompareString() routines will
** not work correctly.  If this assert() ever fires, it probably means
** that the KeyInfo.nKeyField or KeyInfo.nAllField values were computed
** incorrectly.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_Assert_ಕ್ಷೇತ್ರ_ಎಣಿಕೆ_Within_Limits(
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಕೀಲಿ,   /* The record to verify */
  ಸ್ಥಿರಾಂಕ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info       /* Compare size with this KeyInfo */
){
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ = 0;
  u32 szHdr;
  u32 idx;
  u32 not_ಬಳಸಿದ;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಕೀಲಿ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)p_ಕೀಲಿ;

  ಆದರೆ( CORRUPT_DB ) ಹಿಂತಿರುಗಿ;
  idx = getVarint32(a_ಕೀಲಿ, szHdr);
  ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ>=0 );
  ಪ್ರತಿಜ್ಞಾ( szHdr<=(u32)n_ಕೀಲಿ );
  ಆಗ( idx<szHdr ){
    idx += getVarint32(a_ಕೀಲಿ+idx, not_ಬಳಸಿದ);
    n_ಕ್ಷೇತ್ರ++;
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಕ್ಷೇತ್ರ <= p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ );
}
#ಇಲ್ಲದಿದ್ದರೆ
# define vdbe_Assert_ಕ್ಷೇತ್ರ_ಎಣಿಕೆ_Within_Limits(A,B,C)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbeCompareMemStringWithEncodingChange(
  ಸ್ಥಿರಾಂಕ Mem *pMem1,
  ಸ್ಥಿರಾಂಕ Mem *pMem2,
  ಸ್ಥಿರಾಂಕ CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2((sqlite3_ಮೌಲ್ಯ*)&c1, pColl->enc);
  v2 = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2((sqlite3_ಮೌಲ್ಯ*)&c2, pColl->enc);
  ಆದರೆ( (v1==0 || v2==0) ){
    ಆದರೆ( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeCompareMemString(
  ಸ್ಥಿರಾಂಕ Mem *pMem1,
  ಸ್ಥಿರಾಂಕ Mem *pMem2,
  ಸ್ಥಿರಾಂಕ CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  ಆದರೆ( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    ಹಿಂತಿರುಗಿ pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isAllZero(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<n; i++){
    ಆದರೆ( z[i] ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3BlobCompare(ಸ್ಥಿರಾಂಕ Mem *pB1, ಸ್ಥಿರಾಂಕ Mem *pB2){
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ n1 = pB1->n;
  ಪೂರ್ಣಾಂಕ n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  ಪ್ರತಿಜ್ಞಾ( (pB1->flags & MEM_Zero)==0 || n1==0 );
  ಪ್ರತಿಜ್ಞಾ( (pB2->flags & MEM_Zero)==0 || n2==0 );

  ಆದರೆ( (pB1->flags|pB2->flags) & MEM_Zero ){
    ಆದರೆ( pB1->flags & pB2->flags & MEM_Zero ){
      ಹಿಂತಿರುಗಿ pB1->u.nZero - pB2->u.nZero;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pB1->flags & MEM_Zero ){
      ಆದರೆ( !isAllZero(pB2->z, pB2->n) ) ಹಿಂತಿರುಗಿ -1;
      ಹಿಂತಿರುಗಿ pB1->u.nZero - n2;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( !isAllZero(pB1->z, pB1->n) ) ಹಿಂತಿರುಗಿ +1;
      ಹಿಂತಿರುಗಿ n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  ಆದರೆ( c ) ಹಿಂತಿರುಗಿ c;
  ಹಿಂತಿರುಗಿ n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#ಆದರೆ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE doubleLt(ದ್ವಿಗುಣ a, ದ್ವಿಗುಣ b){ ಹಿಂತಿರುಗಿ a<b; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE doubleEq(ದ್ವಿಗುಣ a, ದ್ವಿಗುಣ b){ ಹಿಂತಿರುಗಿ a==b; }
#ಆದರೆ_ಅಂತ್ಯ

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
ಪೂರ್ಣಾಂಕ sqlite3IntFloatCompare(i64 i, ದ್ವಿಗುಣ r){
  ಆದರೆ( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    i64 y;
    ಆದರೆ( r<-9223372036854775808.0 ) ಹಿಂತಿರುಗಿ +1;
    ಆದರೆ( r>=9223372036854775808.0 ) ಹಿಂತಿರುಗಿ -1;
    y = (i64)r;
    ಆದರೆ( i<y ) ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ( i>y ) ಹಿಂತಿರುಗಿ +1;
    testcase( doubleLt(((ದ್ವಿಗುಣ)i),r) );
    testcase( doubleLt(r,((ದ್ವಿಗುಣ)i)) );
    testcase( doubleEq(r,((ದ್ವಿಗುಣ)i)) );
    ಹಿಂತಿರುಗಿ (((ದ್ವಿಗುಣ)i)<r) ? -1 : (((ದ್ವಿಗುಣ)i)>r);
  }
}

/*
** Compare the values contained by the two memory cells, returning
** negative, zero or positive if pMem1 is less than, equal to, or greater
** than pMem2. Sorting order is NULL's first, followed by numbers (integers
** and reals) sorted numerically, followed by text ordered by the collating
** sequence pColl and finally blob's ordered by memcmp().
**
** Two NULL values are considered equal by this function.
*/
ಪೂರ್ಣಾಂಕ sqlite3MemCompare(ಸ್ಥಿರಾಂಕ Mem *pMem1, ಸ್ಥಿರಾಂಕ Mem *pMem2, ಸ್ಥಿರಾಂಕ CollSeq *pColl){
  ಪೂರ್ಣಾಂಕ f1, f2;
  ಪೂರ್ಣಾಂಕ combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  ಪ್ರತಿಜ್ಞಾ( !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem1) && !sqlite3_Vdbe_Mem_Is_ಅಡ್ಡಸಾಲು_ಗಣ(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  ಆದರೆ( combined_flags&MEM_Null ){
    ಹಿಂತಿರುಗಿ (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  ಆದರೆ( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    ಆದರೆ( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      ಆದರೆ( pMem1->u.i < pMem2->u.i ) ಹಿಂತಿರುಗಿ -1;
      ಆದರೆ( pMem1->u.i > pMem2->u.i ) ಹಿಂತಿರುಗಿ +1;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ( (f1 & f2 & MEM_Real)!=0 ){
      ಆದರೆ( pMem1->u.r < pMem2->u.r ) ಹಿಂತಿರುಗಿ -1;
      ಆದರೆ( pMem1->u.r > pMem2->u.r ) ಹಿಂತಿರುಗಿ +1;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      ಆದರೆ( (f2&MEM_Real)!=0 ){
        ಹಿಂತಿರುಗಿ sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        ಆದರೆ( pMem1->u.i < pMem2->u.i ) ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ( pMem1->u.i > pMem2->u.i ) ಹಿಂತಿರುಗಿ +1;
        ಹಿಂತಿರುಗಿ 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ -1;
      }
    }
    ಆದರೆ( (f1&MEM_Real)!=0 ){
      ಆದರೆ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        ಹಿಂತಿರುಗಿ -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ -1;
      }
    }
    ಹಿಂತಿರುಗಿ +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  ಆದರೆ( combined_flags&MEM_Str ){
    ಆದರೆ( (f1 & MEM_Str)==0 ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( (f2 & MEM_Str)==0 ){
      ಹಿಂತಿರುಗಿ -1;
    }

    ಪ್ರತಿಜ್ಞಾ( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    ಪ್ರತಿಜ್ಞಾ( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    ಪ್ರತಿಜ್ಞಾ( !pColl || pColl->xCmp );

    ಆದರೆ( pColl ){
      ಹಿಂತಿರುಗಿ vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  ಹಿಂತಿರುಗಿ sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
ಸ್ಥಿರ i64 vdbe_ದಾಖಲೆ_Decode_Int(u32 serial_type, ಸ್ಥಿರಾಂಕ u8 *a_ಕೀಲಿ){
  u32 y;
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  ಆಯ್ಕೆ( serial_type ){
    ಸ್ಥಿತಿ 0:
    ಸ್ಥಿತಿ 1:
      testcase( a_ಕೀಲಿ[0]&0x80 );
      ಹಿಂತಿರುಗಿ ONE_BYTE_INT(a_ಕೀಲಿ);
    ಸ್ಥಿತಿ 2:
      testcase( a_ಕೀಲಿ[0]&0x80 );
      ಹಿಂತಿರುಗಿ TWO_BYTE_INT(a_ಕೀಲಿ);
    ಸ್ಥಿತಿ 3:
      testcase( a_ಕೀಲಿ[0]&0x80 );
      ಹಿಂತಿರುಗಿ THREE_BYTE_INT(a_ಕೀಲಿ);
    ಸ್ಥಿತಿ 4: {
      testcase( a_ಕೀಲಿ[0]&0x80 );
      y = FOUR_BYTE_UINT(a_ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ (i64)*(ಪೂರ್ಣಾಂಕ*)&y;
    }
    ಸ್ಥಿತಿ 5: {
      testcase( a_ಕೀಲಿ[0]&0x80 );
      ಹಿಂತಿರುಗಿ FOUR_BYTE_UINT(a_ಕೀಲಿ+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_ಕೀಲಿ);
    }
    ಸ್ಥಿತಿ 6: {
      u64 x = FOUR_BYTE_UINT(a_ಕೀಲಿ);
      testcase( a_ಕೀಲಿ[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_ಕೀಲಿ+4);
      ಹಿಂತಿರುಗಿ (i64)*(i64*)&x;
    }
  }

  ಹಿಂತಿರುಗಿ (serial_type - 8);
}

/*
** This function compares the two table rows or index records
** specified by {nKey1, pKey1} and pPKey2.  It returns a negative, zero
** or positive integer if key1 is less than, equal to or
** greater than key2.  The {nKey1, pKey1} key must be a blob
** created by the OP_MakeRecord opcode of the VDBE.  The pPKey2
** key must be a parsed key such as obtained from
** sqlite3VdbeParseRecord.
**
** If argument bSkip is non-zero, it is assumed that the caller has already
** determined that the first fields of the keys are equal.
**
** Key1 and Key2 do not have to contain the same number of fields. If all
** fields that appear in both keys are equal, then pPKey2->default_rc is
** returned.
**
** If database corruption is discovered, set pPKey2->errCode to
** SQLITE_CORRUPT and return 0. If an OOM error is encountered,
** pPKey2->errCode is set to SQLITE_NOMEM and, if it is not NULL, the
** malloc-failed flag set on database handle (pPKey2->pKeyInfo->db).
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1,   /* Left key */
  Unpacked_ದಾಖಲೆ *pPKey2,         /* Right key */
  ಪೂರ್ಣಾಂಕ bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  ಪೂರ್ಣಾಂಕ i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  ಪೂರ್ಣಾಂಕ rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *aKey1 = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  ಆದರೆ( bSkip ){
    u32 s1 = aKey1[1];
    ಆದರೆ( s1<0x80 ){
      idx1 = 2;
    }ಇಲ್ಲದಿದ್ದರೆ{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  ಆದರೆ( d1>(ಚಿಹ್ನೆರಹಿತ)nKey1 ){
    pPKey2->err_ಸಂಕೇತ = (u8)SQLITE_CORRUPT_BKPT;
    ಹಿಂತಿರುಗಿ 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  ಪ್ರತಿಜ್ಞಾ( pPKey2->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ>=pPKey2->n_ಕ್ಷೇತ್ರ
       || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( pPKey2->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  ಪ್ರತಿಜ್ಞಾ( pPKey2->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ>0 );
  ಪ್ರತಿಜ್ಞಾ( idx1<=szHdr1 || CORRUPT_DB );
  ಆಗ( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    ಆದರೆ( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      ಆದರೆ( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( serial_type==0 ){
        rc = -1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }ಇಲ್ಲದಿದ್ದರೆ{
        i64 lhs = vdbe_ದಾಖಲೆ_Decode_Int(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        ಆದರೆ( lhs<rhs ){
          rc = -1;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      ಆದರೆ( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( serial_type==0 ){
        rc = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( serial_type==7 ){
          ಆದರೆ( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪ್ರತಿಜ್ಞಾ( rc==0 );
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ಆದರೆ( serial_type<12 ){
        rc = -1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !(serial_type & 0x01) ){
        rc = +1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(ಚಿಹ್ನೆರಹಿತ)nKey1 );
        testcase( (d1+mem1.n+1)==(ಚಿಹ್ನೆರಹಿತ)nKey1 );
        ಆದರೆ( (d1+mem1.n) > (ಚಿಹ್ನೆರಹಿತ)nKey1
         || (p_ಕೀಲಿ_Info = pPKey2->p_ಕೀಲಿ_Info)->n_All_ಕ್ಷೇತ್ರ<=i
        ){
          pPKey2->err_ಸಂಕೇತ = (u8)SQLITE_CORRUPT_BKPT;
          ಹಿಂತಿರುಗಿ 0;                /* Corruption */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಕೀಲಿ_Info->aColl[i] ){
          mem1.enc = p_ಕೀಲಿ_Info->enc;
          mem1.db = p_ಕೀಲಿ_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (ಅಕ್ಷರ*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_ಕೀಲಿ_Info->aColl[i], &pPKey2->err_ಸಂಕೇತ
          );
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ಆದರೆ( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRhs->flags & MEM_Blob ){
      ಪ್ರತಿಜ್ಞಾ( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ಆದರೆ( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(ಚಿಹ್ನೆರಹಿತ)nKey1 );
        testcase( (d1+nStr+1)==(ಚಿಹ್ನೆರಹಿತ)nKey1 );
        ಆದರೆ( (d1+nStr) > (ಚಿಹ್ನೆರಹಿತ)nKey1 ){
          pPKey2->err_ಸಂಕೇತ = (u8)SQLITE_CORRUPT_BKPT;
          ಹಿಂತಿರುಗಿ 0;                /* Corruption */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pRhs->flags & MEM_Zero ){
          ಆದರೆ( !isAllZero((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&aKey1[d1],nStr) ){
            rc = 1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            rc = nStr - pRhs->u.nZero;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ಆದರೆ( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    ಇಲ್ಲದಿದ್ದರೆ{
      serial_type = aKey1[idx1];
      ಆದರೆ( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        ಪ್ರತಿಜ್ಞಾ( rc==0 );
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = 1;
      }
    }

    ಆದರೆ( rc!=0 ){
      ಪೂರ್ಣಾಂಕ ವಿಂಗಡಿಸು_Flags = pPKey2->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[i];
      ಆದರೆ( ವಿಂಗಡಿಸು_Flags ){
        ಆದರೆ( (ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_BIGNULL)==0
         || ((ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      ಪ್ರತಿಜ್ಞಾ( vdbe_ದಾಖಲೆ_Compare_Debug(nKey1, pKey1, pPKey2, rc) );
      ಪ್ರತಿಜ್ಞಾ( mem1.szMalloc==0 );  /* See comment below */
      ಹಿಂತಿರುಗಿ rc;
    }

    i++;
    ಆದರೆ( i==pPKey2->n_ಕ್ಷೇತ್ರ ) ನಿಲ್ಲಿಸು;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    ಆದರೆ( d1>(ಚಿಹ್ನೆರಹಿತ)nKey1 ) ನಿಲ್ಲಿಸು;
    idx1 += sqlite3VarintLen(serial_type);
    ಆದರೆ( idx1>=(ಚಿಹ್ನೆರಹಿತ)szHdr1 ){
      pPKey2->err_ಸಂಕೇತ = (u8)SQLITE_CORRUPT_BKPT;
      ಹಿಂತಿರುಗಿ 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  ಪ್ರತಿಜ್ಞಾ( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB
       || vdbe_ದಾಖಲೆ_Compare_Debug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_ಕೀಲಿ_Info->db->mallocFailed
  );
  pPKey2->eq_ನೋಡಿದ = 1;
  ಹಿಂತಿರುಗಿ pPKey2->default_rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ದಾಖಲೆ_Compare(
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1,   /* Left key */
  Unpacked_ದಾಖಲೆ *pPKey2          /* Right key */
){
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(nKey1, pKey1, pPKey2, 0);
}


/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is an integer, and (b) the
** size-of-header varint at the start of (pKey1/nKey1) fits in a single
** byte (i.e. is less than 128).
**
** To avoid concerns about buffer overreads, this routine is only used
** on schemas where the maximum valid header size is 63 bytes or less.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ದಾಖಲೆ_Compare_Int(
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, /* Left key */
  Unpacked_ದಾಖಲೆ *pPKey2        /* Right key */
){
  ಸ್ಥಿರಾಂಕ u8 *a_ಕೀಲಿ = &((ಸ್ಥಿರಾಂಕ u8*)pKey1)[*(ಸ್ಥಿರಾಂಕ u8*)pKey1 & 0x3F];
  ಪೂರ್ಣಾಂಕ serial_type = ((ಸ್ಥಿರಾಂಕ u8*)pKey1)[1];
  ಪೂರ್ಣಾಂಕ res;
  i64 v;
  i64 lhs;

  vdbe_Assert_ಕ್ಷೇತ್ರ_ಎಣಿಕೆ_Within_Limits(nKey1, pKey1, pPKey2->p_ಕೀಲಿ_Info);
  ಪ್ರತಿಜ್ಞಾ( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

  /* Serial types 1 through 6 are big-endian integers of 1, 2, 3, 4,
  ** 6, or 8 bytes.  Rather than handle each width in its own switch
  ** case, read 8 bytes and use an arithmetic right shift to drop the
  ** unwanted low-order bytes and sign-extend the value.  This helps
  ** because the switch tends to mispredict when a key column contains
  ** integers of varying sizes.  The first entry of aShift[] is a
  ** placeholder so that the table can be indexed by serial_type
  ** directly.  Reading 8 bytes is always safe, because a buffer passed
  ** to this routine has at least 74 bytes of padding after it, as
  ** explained in sqlite3VdbeFindCompare() below. 
  */
  ಆದರೆ( (u32)(serial_type-1)<=5 ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಸರಿಸು[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_ಕೀಲಿ)) >> a_ಸರಿಸು[serial_type];
    /*                                 ^^--- This shift operator 
    ** needs to be an arithmetic right-shift, which means that
    ** if the left-hand operand (LHS) is negative, it will be sign-extended
    ** so that the final results is also negative.  All modern C
    ** compilers work this way as long as the LHS is a signed integer
    ** (which is why the unsigned result from sqlite3Get8byte() is cast
    ** into i64), but it is not defined by the C standards, or so Claude
    ** tells me.  That the correct result is obtained is verified by the
    ** following assert() and testcase() macros:
    */
    ಪ್ರತಿಜ್ಞಾ( 0<=(i64)sqlite3Get8byte(a_ಕೀಲಿ) || lhs<0 );
    testcase( lhs<0 );
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare(nKey1, pKey1, pPKey2);
  }

  ಪ್ರತಿಜ್ಞಾ( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  ಆದರೆ( v>lhs ){
    res = pPKey2->r1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v<lhs ){
    res = pPKey2->r2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pPKey2->n_ಕ್ಷೇತ್ರ>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eq_ನೋಡಿದ = 1;
  }

  ಪ್ರತಿಜ್ಞಾ( vdbe_ದಾಖಲೆ_Compare_Debug(nKey1, pKey1, pPKey2, res) );
  ಹಿಂತಿರುಗಿ res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ದಾಖಲೆ_Compare_String(
  ಪೂರ್ಣಾಂಕ nKey1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, /* Left key */
  Unpacked_ದಾಖಲೆ *pPKey2        /* Right key */
){
  ಸ್ಥಿರಾಂಕ u8 *aKey1 = (ಸ್ಥಿರಾಂಕ u8*)pKey1;
  ಪೂರ್ಣಾಂಕ serial_type;
  ಪೂರ್ಣಾಂಕ res;

  ಪ್ರತಿಜ್ಞಾ( pPKey2->aMem[0].flags & MEM_Str );
  ಪ್ರತಿಜ್ಞಾ( pPKey2->aMem[0].n == pPKey2->n );
  ಪ್ರತಿಜ್ಞಾ( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_ಕ್ಷೇತ್ರ_ಎಣಿಕೆ_Within_Limits(nKey1, pKey1, pPKey2->p_ಕೀಲಿ_Info);
  serial_type = (ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ)(aKey1[1]);

vrcs_restart:
  ಆದರೆ( serial_type<12 ){
    ಆದರೆ( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      ಆದರೆ( serial_type>=12 ) ಹೋಗು vrcs_restart;
      ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ nCmp;
    ಪೂರ್ಣಾಂಕ nStr;
    ಪೂರ್ಣಾಂಕ szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    ಆದರೆ( (szHdr + nStr) > nKey1 ){
      pPKey2->err_ಸಂಕೇತ = (u8)SQLITE_CORRUPT_BKPT;
      ಹಿಂತಿರುಗಿ 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    ಆದರೆ( res>0 ){
      res = pPKey2->r2;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( res<0 ){
      res = pPKey2->r1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      res = nStr - pPKey2->n;
      ಆದರೆ( res==0 ){
        ಆದರೆ( pPKey2->n_ಕ್ಷೇತ್ರ>1 ){
          res = sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          res = pPKey2->default_rc;
          pPKey2->eq_ನೋಡಿದ = 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( res>0 ){
        res = pPKey2->r2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        res = pPKey2->r1;
      }
    }
  }

  ಪ್ರತಿಜ್ಞಾ( vdbe_ದಾಖಲೆ_Compare_Debug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_ಕೀಲಿ_Info->db->mallocFailed
  );
  ಹಿಂತಿರುಗಿ res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
ದಾಖಲೆ_Compare sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_Compare(Unpacked_ದಾಖಲೆ *p){
  /* varintRecordCompareInt() and varintRecordCompareString() both assume
  ** that the size-of-header varint that occurs at the start of each record
  ** fits in a single byte (i.e. is 127 or less). varintRecordCompareInt()
  ** also assumes that it is safe to overread a buffer by at least the
  ** maximum possible legal header size plus 8 bytes. Because there is
  ** guaranteed to be at least 74 (but not 136) bytes of padding following each
  ** buffer passed to varintRecordCompareInt() this makes it convenient to
  ** limit the size of the header to 64 bytes in cases where the first field
  ** is an integer.
  **
  ** The easiest way to enforce this limit is to consider only records with
  ** 13 fields or less. If the first field is an integer, the maximum legal
  ** header size is (12*5 + 1 + 1) bytes.  */
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  ಆದರೆ( p->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ<=13 ){
    ಪೂರ್ಣಾಂಕ flags = p->aMem[0].flags;
    ಆದರೆ( p->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] ){
      ಆದರೆ( p->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] & KEYINFO_ಕ್ರಮ_BIGNULL ){
        ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->r1 = -1;
      p->r2 = 1;
    }
    ಆದರೆ( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      ಹಿಂತಿರುಗಿ vdbe_ದಾಖಲೆ_Compare_Int;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    ಆದರೆ( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_ದ್ವಿಮಾನ(p->p_ಕೀಲಿ_Info->aColl[0])
    ){
      ಪ್ರತಿಜ್ಞಾ( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      ಹಿಂತಿರುಗಿ vdbe_ದಾಖಲೆ_Compare_String;
    }
  }

  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_ಕೀಲಿ = 0;
  ಪೂರ್ಣಾಂಕ rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_ಕೀಲಿ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
  ಪ್ರತಿಜ್ಞಾ( (n_Cell_ಕೀಲಿ & SQLITE_MAX_U32)==(u64)n_Cell_ಕೀಲಿ );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pCur, (u32)n_Cell_ಕೀಲಿ, &m);
  ಆದರೆ( rc ){
    ಹಿಂತಿರುಗಿ rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  ಪ್ರತಿಜ್ಞಾ( m.n>=0 );
  ಆದರೆ( unlikely(szHdr<3 || szHdr>(ಚಿಹ್ನೆರಹಿತ)m.n) ){
    ಹೋಗು idx_rowid_corruption;
  }

  /* The last field of the index should be an integer - the ROWID.
  ** Verify that the last entry really is an integer. */
  getVarint32NR((u8*)&m.z[szHdr-1], typeRowid);
  testcase( typeRowid==1 );
  testcase( typeRowid==2 );
  testcase( typeRowid==3 );
  testcase( typeRowid==4 );
  testcase( typeRowid==5 );
  testcase( typeRowid==6 );
  testcase( typeRowid==8 );
  testcase( typeRowid==9 );
  ಆದರೆ( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    ಹೋಗು idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  ಆದರೆ( unlikely((u32)m.n<szHdr+lenRowid) ){
    ಹೋಗು idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  ಹಿಂತಿರುಗಿ SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
}

/*
** Compare the key of the index entry that cursor pC is pointing to against
** the key string in pUnpacked.  Write into *pRes a number
** that is negative, zero, or positive if pC is less than, equal to,
** or greater than pUnpacked.  Return SQLITE_OK on success.
**
** pUnpacked is either created without a rowid or is truncated so that it
** omits the rowid at the end.  The rowid at the end of the index entry
** is ignored as well.  Hence, this routine only compares the prefixes
** of the keys prior to the final rowid, not the entire key.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Idx_ಕೀಲಿ_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  Unpacked_ದಾಖಲೆ *pUnpacked,       /* Unpacked version of key */
  ಪೂರ್ಣಾಂಕ *res                         /* Write the comparison result here */
){
  i64 n_Cell_ಕೀಲಿ = 0;
  ಪೂರ್ಣಾಂಕ rc;
  BtCursor *pCur;
  Mem m;

  ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_ಕೀಲಿ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  ಆದರೆ( n_Cell_ಕೀಲಿ<=0 || n_Cell_ಕೀಲಿ>0x7fffffff ){
    *res = 0;
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ಸ್ಥಾನಾಂತರ(pCur, (u32)n_Cell_ಕೀಲಿ, &m);
  ಆದರೆ( rc ){
    ಹಿಂತಿರುಗಿ rc;
  }
  *res = sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_Changes(sqlite3 *db, i64 nChange){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_ಒಟ್ಟು_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
ಶೂನ್ಯ sqlite3_Vdbe_ಎಣಿಕೆ_Changes(Vdbe *v){
  v->changeCntOn = 1;
}

/*
** Mark every prepared statement associated with a database connection
** as expired.
**
** An expired statement means that recompilation of the statement is
** recommend.  Statements expire when things happen that make their
** programs obsolete.  Removing user-defined functions or collating
** sequences, or changing an authorization function are the types of
** things that make prepared statements obsolete.
**
** If iCode is 1, then expiration is advisory.  The statement should
** be reprepared before being restarted, but if it is already running
** it is allowed to run to completion.
**
** Internally, this function just sets the Vdbe.expired flag on all
** prepared statements.  The flag is set to 1 for an immediate expiration
** and set to 2 for an advisory expiration.
*/
ಶೂನ್ಯ sqlite3ExpirePreparedStatements(sqlite3 *db, ಪೂರ್ಣಾಂಕ i_ಸಂಕೇತ){
  Vdbe *p;
  ಗಾಗಿ(p = db->pVdbe; p; p=p->pVNext){
    p->expired = i_ಸಂಕೇತ+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  ಹಿಂತಿರುಗಿ v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  ಹಿಂತಿರುಗಿ v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_ಮೌಲ್ಯ *sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(Vdbe *v, ಪೂರ್ಣಾಂಕ iVar, u8 aff){
  ಪ್ರತಿಜ್ಞಾ( iVar>0 );
  ಆದರೆ( v ){
    Mem *pMem = &v->aVar[iVar-1];
    ಪ್ರತಿಜ್ಞಾ( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_ಆಂತರಿಕ_Func)!=0 );
    ಆದರೆ( 0==(pMem->flags & MEM_Null) ){
      sqlite3_ಮೌಲ್ಯ *pRet = sqlite3_ಮೌಲ್ಯ_New(v->db);
      ಆದರೆ( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_ಮೌಲ್ಯ_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      ಹಿಂತಿರುಗಿ pRet;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Configure SQL variable iVar so that binding a new value to it signals
** to sqlite3_reoptimize() that re-preparing the statement may result
** in a better query plan. If parameter bSmallint is true, then the
** statement is only re-prepared if the new value is integer value 0 or 1.
**
** The v->expmask bit is always set.  expmask means that a reprepare is
** possible.  The v->smimask bit is only set if we want to restrict
** reprepare when the value changes from (0,1) to something else, or from
** something else to (0,1).
*/
ಶೂನ್ಯ sqlite3VdbeReprepareOnBind(Vdbe *v, ಪೂರ್ಣಾಂಕ iVar, ಪೂರ್ಣಾಂಕ bSmallint){
  u32 m;
  ಪ್ರತಿಜ್ಞಾ( iVar>0 );
  ಪ್ರತಿಜ್ಞಾ( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_ಆಂತರಿಕ_Func)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  ಆದರೆ( bSmallint ){
    ಆದರೆ( (v->expmask & m)==0 ) v->smimask |= m;
  }ಇಲ್ಲದಿದ್ದರೆ{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  ಪ್ರತಿಜ್ಞಾ( (v->smimask & v->expmask) == v->smimask );
}

/*
** Helper function for vdbeIsMatchingIndexKey(). Return true if column
** iCol should be ignored when comparing a record with a record from 
** an index on disk. The field should be ignored if:
**
**   * the corresponding bit in mask is set, and
**   * either:
**       - bIntegrity is false, or
**       - the two Mem values are both real values that differ by 
**         BTREE_ULPDISTORTION or fewer ULPs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Skip_ಕ್ಷೇತ್ರ(
  Bitmask ಮುಖವಾಡ,                   /* Mask of indexed expression fields */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  ಪೂರ್ಣಾಂಕ bIntegrity                  /* True if running PRAGMA integrity_check */
){
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_ULPDISTORTION 2
  ಆದರೆ( i_ಲಂಬಸಾಲು>=BMS || (ಮುಖವಾಡ & MASKBIT(i_ಲಂಬಸಾಲು))==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( bIntegrity==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    ಸ್ಮರಣೆ_ನಕಲು(&m1,&pMem1->u.r,8);
    ಸ್ಮರಣೆ_ನಕಲು(&m2,&pMem2->u.r,8);
    ಆದರೆ( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** This function compares the unpacked record with the current key that
** cursor pCur points to. If bInt is false, all fields for which the 
** corresponding bit in parameter "mask" is set are ignored. Or, if
** bInt is true, then a difference of BTREE_ULPDISTORTION or fewer ULPs
** in real values is overlooked for fields with the corresponding bit
** set in mask.  
**
** Return the usual less than zero, zero, or greater than zero if the 
** remaining fields of the cursor cursor key are less than, equal to or 
** greater than those in (*p).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Is_Matching_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(
  BtCursor *pCur,            /* Cursor open on index */
  ಪೂರ್ಣಾಂಕ bInt,                  /* True for integrity_check-style search */
  Bitmask ಮುಖವಾಡ,              /* Mask of columns to skip */
  Unpacked_ದಾಖಲೆ *p,         /* Index key being deleted */
  ಪೂರ್ಣಾಂಕ *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  memset(&m, 0, ಗಾತ್ರ(m));
  m.enc = p->p_ಕೀಲಿ_Info->enc;
  m.db = p->p_ಕೀಲಿ_Info->db;
  nRec = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
  ಆದರೆ( nRec>0x7fffffff ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  ಆದರೆ( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    ಆದರೆ( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      ಪೂರ್ಣಾಂಕ ii = 0;                 /* Iterator variable */

      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = p->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ;
      ಗಾಗಿ(ii=0; ii<n_ಲಂಬಸಾಲು && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        ಪೂರ್ಣಾಂಕ nSerial = 0;

        ಆದರೆ( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          ನಿಲ್ಲಿಸು;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        ಆದರೆ( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          ಆದರೆ( vdbe_Skip_ಕ್ಷೇತ್ರ(ಮುಖವಾಡ, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_ಕೀಲಿ_Info->aColl[ii]);
            ಆದರೆ( res!=0 ) ನಿಲ್ಲಿಸು;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This is called when the record in (*p) should be found in the index 
** opened by cursor pCur, but was not. This may happen as part of a DELETE
** operation or an integrity check.
**
** One reason that an exact match was not found may be the EIIB bug - that
** a text-to-float conversion may have caused a real value in record (*p)
** to be slightly different from its counterpart on disk. This function
** attempts to find the right index record. If it does find the right
** record, it leaves *pCur pointing to it and sets (*pRes) to 0 before
** returning. Otherwise, (*pRes) is set to non-zero and an SQLite error
** code returned.
**
** The algorithm used to find the correct record is:
**
**   * Scan up to BTREE_FDK_RANGE entries either side of the current entry.
**     If parameter bIntegrity is false, then all fields that are indexed
**     expressions or virtual table columns are omitted from the comparison.
**     If bIntegrity is true, then small differences in real values in
**     such fields are overlooked, but they are not omitted from the comparison
**     altogether.
**
**   * If the above fails to find an entry and bIntegrity is false, search 
**     the entire index.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(
  BtCursor *pCur, 
  ಸೂಚ್ಯಂಕ *pIdx,
  Unpacked_ದಾಖಲೆ *p, 
  ಪೂರ್ಣಾಂಕ *pRes,
  ಪೂರ್ಣಾಂಕ bIntegrity
){
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_FDK_RANGE 10
  ಪೂರ್ಣಾಂಕ n_ಹೆಜ್ಜೆ = 0;
  ಪೂರ್ಣಾಂಕ res = 1;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask ಮುಖವಾಡ = 0;
  ಗಾಗಿ(ii=0; ii<MIN(pIdx->n_ಸ್ತಂಭ, BMS); ii++){
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[ii];
    ಆದರೆ( (i_ಲಂಬಸಾಲು==XN_ಅಭಿವ್ಯಕ್ತಿ)
     || (i_ಲಂಬಸಾಲು>=0 && (pIdx->p_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL))
    ){
      ಮುಖವಾಡ |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  ಆದರೆ( ಮುಖವಾಡ!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    ಗಾಗಿ(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3_Btree_ಹಿಂದಿನ(pCur, 0);
    }
    ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
      rc = sqlite3_Btree_ಮೊದಲ(pCur, &res);
      n_ಹೆಜ್ಜೆ = -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಹೆಜ್ಜೆ = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    ಆಗ( sqlite3BtreeCursorIsValidNN(pCur) ){
      ಗಾಗಿ(ii=0; rc==SQLITE_OK && (ii<n_ಹೆಜ್ಜೆ || n_ಹೆಜ್ಜೆ<0); ii++){
        rc = vdbe_Is_Matching_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(pCur, bIntegrity, ಮುಖವಾಡ, p, &res);
        ಆದರೆ( res==0 || rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
        rc = sqlite3_Btree_ಮುಂದಿನ(pCur, 0);
      }
      ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
        rc = SQLITE_OK;
        ಪ್ರತಿಜ್ಞಾ( res!=0 );
      }
      ಆದರೆ( n_ಹೆಜ್ಜೆ<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) ನಿಲ್ಲಿಸು;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      n_ಹೆಜ್ಜೆ = -1;
      rc = sqlite3_Btree_ಮೊದಲ(pCur, &res);
    }
  }

  *pRes = res;
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Not_ಶುದ್ಧ_Func(sqlite3_context *pCtx){
  ಸ್ಥಿರಾಂಕ VdbeOp *pOp;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pCtx->pVdbe==0 ) ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  ಆದರೆ( pOp->opcode==OP_ಶುದ್ಧ_Func ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zContext;
    ಅಕ್ಷರ *z_ಸಂದೇಶ;
    ಆದರೆ( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pOp->p5 & NC_Gen_ಲಂಬಸಾಲು ){
      zContext = "a generated column";
    }ಇಲ್ಲದಿದ್ದರೆ{
      zContext = "an index";
    }
    z_ಸಂದೇಶ = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx, z_ಸಂದೇಶ, -1);
    sqlite3_free(z_ಸಂದೇಶ);
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DATETIME_FUNCS */

#ಆದರೆ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Cursor_Range_Hint_ಅಭಿವ್ಯಕ್ತಿ_Check(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ರಿಜಿಸ್ಟರ್ ){
    ಪ್ರತಿಜ್ಞಾ( (pWalker->u.aMem[p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ].flags & MEM_Undefined)==0 );
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
ಶೂನ್ಯ sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  ಆದರೆ( pVtab->z_Err_ಸಂದೇಶ ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_ಸಂದೇಶ);
    p->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, pVtab->z_Err_ಸಂದೇಶ);
    sqlite3_free(pVtab->z_Err_ಸಂದೇಶ);
    pVtab->z_Err_ಸಂದೇಶ = 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeFreeUnpacked(sqlite3 *db, ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ, Unpacked_ದಾಖಲೆ *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<n_ಕ್ಷೇತ್ರ; i++){
      Mem *pMem = &p->aMem[i];
      ಆದರೆ( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
ಶೂನ್ಯ sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  ಪೂರ್ಣಾಂಕ op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,                /* Database name */
  ಕೋಷ್ಟಕ *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  ಪೂರ್ಣಾಂಕ iReg,                       /* Register for new.* record */
  ಪೂರ್ಣಾಂಕ iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTbl = pTab->zName;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ n_Real_ಲಂಬಸಾಲು;
  ಆದರೆ( pTab->tabFlags & TF_WithoutRowid ){
    n_Real_ಲಂಬಸಾಲು = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab)->n_ಸ್ತಂಭ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTab->tabFlags & TF_HasVirtual ){
    n_Real_ಲಂಬಸಾಲು = pTab->nNVCol;
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_Real_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( db->pPreUpdate==0 );
  memset(&preupdate, 0, ಗಾತ್ರ(PreUpdate));
  ಆದರೆ( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }ಇಲ್ಲದಿದ್ದರೆ{
      iKey2 = iKey1;
    }
  }

  ಪ್ರತಿಜ್ಞಾ( pCsr!=0 );
  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_BTREE );
  ಪ್ರತಿಜ್ಞಾ( pCsr->n_ಕ್ಷೇತ್ರ==n_Real_ಲಂಬಸಾಲು
       || (pCsr->n_ಕ್ಷೇತ್ರ==n_Real_ಲಂಬಸಾಲು+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (ಕೀಲಿ_Info*)&preupdate.u_ಕೀಲಿ;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ = pTab->n_ಲಂಬಸಾಲು;
  preupdate.pKeyinfo->a_ವಿಂಗಡಿಸು_Flags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.a_ದಾಖಲೆ);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  ಆದರೆ( preupdate.aNew ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pCsr->n_ಕ್ಷೇತ್ರ; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  ಆದರೆ( preupdate.apDflt ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pTab->n_ಲಂಬಸಾಲು; i++){
      sqlite3_ಮೌಲ್ಯ_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3VdbeFuncName(ಸ್ಥಿರಾಂಕ sqlite3_context *pCtx){
  ಪ್ರತಿಜ್ಞಾ( pCtx!=0 );
  ಪ್ರತಿಜ್ಞಾ( pCtx->pFunc!=0 );
  ಹಿಂತಿರುಗಿ pCtx->pFunc->zName;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PERCENTILE */
