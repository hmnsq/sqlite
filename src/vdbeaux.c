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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/* Forward references */
ਸਥਿਰ ਖਾਲੀ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
ਸਥਿਰ ਖਾਲੀ vdbeFreeOpArray(sqlite3 *, Op *, ਪੂਰਨ);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, ਆਕਾਰ(Vdbe) );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  memset(&p->aOp, 0, ਆਕਾਰ(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  ਜੇ( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->aLabel==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nLabel==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOpAlloc==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->szOpAlloc==0 );
  sqlite3_Vdbe_ਜੋੜੋ_Op2(p, OP_Init, 0, 1);
  ਵਾਪਸ p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  ਵਾਪਸ p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
ਖਾਲੀ sqlite3VdbeError(Vdbe *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_ਸੁਨੇਹਾ);
  va_start(ap, zFormat);
  p->z_Err_ਸੁਨੇਹਾ = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_Sql(Vdbe *p, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n, u8 prepFlags){
  ਜੇ( p==0 ) ਵਾਪਸ;
  p->prepFlags = prepFlags;
  ਜੇ( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
ਖਾਲੀ sqlite3_Vdbe_ਜੋੜੋ_Dblquote_Str(sqlite3 *db, Vdbe *p, ਅਚਲ ਅੱਖਰ *z){
  ਜੇ( p ){
    ਪੂਰਨ n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            ਆਕਾਰ(*pStr)+n+1-ਆਕਾਰ(pStr->z));
    ਜੇ( pStr ){
      pStr->p_ਅਗਲਾ_Str = p->pDblStr;
      p->pDblStr = pStr;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pStr->z, z, n+1);
    }
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
ਪੂਰਨ sqlite3_Vdbe_ਵਰਤਦਾ_Double_Quoted_String(
  Vdbe *pVdbe,            /* The prepared statement */
  ਅਚਲ ਅੱਖਰ *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  ਪ੍ਰਤਿਜ੍ਞਾ( zId!=0 );
  ਜੇ( pVdbe->pDblStr==0 ) ਵਾਪਸ 0;
  ਲਈ(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_ਅਗਲਾ_Str){
    ਜੇ( strcmp(zId, pStr->z)==0 ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

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
ਖਾਲੀ sqlite3_Vdbe_ਅਦਲਾ_ਬਦਲੀ(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  ਅੱਖਰ *zTmp;
  ਪ੍ਰਤਿਜ੍ਞਾ( pA->db==pB->db );
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#ਜੇ_ਅੰਤ
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pB->aCounter, pA->aCounter, ਆਕਾਰ(pB->aCounter));
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
ਸਥਿਰ ਪੂਰਨ growOpArray(Vdbe *v, ਪੂਰਨ nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#ਨਹੀਂਤਾਂ
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/ਆਕਾਰ(Op)));
  ਅਣਵਰਤਿਆ_PARAMETER(nOp);
#ਜੇ_ਅੰਤ

  /* Ensure that the size of a VDBE does not grow too large */
  ਜੇ( nNew > p->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_VDBE_OP] ){
    sqlite3OomFault(p->db);
    ਵਾਪਸ SQLITE_NOMEM;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( nOp<=(ਪੂਰਨ)(1024/ਆਕਾਰ(Op)) );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*ਆਕਾਰ(Op));
  ਜੇ( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_ਮਾਪ(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/ਆਕਾਰ(Op);
    v->aOp = pNew;
  }
  ਵਾਪਸ (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਖਾਲੀ test_addop_breakpoint(ਪੂਰਨ pc, Op *pOp){
  ਸਥਿਰ u64 n = 0;
  (ਖਾਲੀ)pc;
  (ਖਾਲੀ)pOp;
  n++;
  ਜੇ( n==ਸਭ_ਤੋਂ_ਵੱਡਾ_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#ਜੇ_ਅੰਤ

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ growOp3(Vdbe *p, ਪੂਰਨ op, ਪੂਰਨ p1, ਪੂਰਨ p2, ਪੂਰਨ p3){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOpAlloc<=p->nOp );
  ਜੇ( growOpArray(p, 1) ) ਵਾਪਸ 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOpAlloc>p->nOp );
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, p1, p2, p3);
}
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ ਜੋੜੋ_Op4_Int_Slow(
  Vdbe *p,            /* Add the opcode to this VM */
  ਪੂਰਨ op,             /* The new opcode */
  ਪੂਰਨ p1,             /* The P1 operand */
  ਪੂਰਨ p2,             /* The P2 operand */
  ਪੂਰਨ p3,             /* The P3 operand */
  ਪੂਰਨ p4              /* The P4 operand as an integer */
){
  ਪੂਰਨ addr = sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, p1, p2, p3);
  ਜੇ( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  ਵਾਪਸ addr;
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
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op0(Vdbe *p, ਪੂਰਨ op){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, 0, 0, 0);
}
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op1(Vdbe *p, ਪੂਰਨ op, ਪੂਰਨ p1){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, p1, 0, 0);
}
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op2(Vdbe *p, ਪੂਰਨ op, ਪੂਰਨ p1, ਪੂਰਨ p2){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, p1, p2, 0);
}
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op3(Vdbe *p, ਪੂਰਨ op, ਪੂਰਨ p1, ਪੂਰਨ p2, ਪੂਰਨ p3){
  ਪੂਰਨ i;
  VdbeOp *pOp;

  i = p->nOp;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<0xff );
  ਜੇ( p->nOpAlloc<=i ){
    ਵਾਪਸ growOp3(p, op, p1, p2, p3);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  ਪ੍ਰਤਿਜ੍ਞਾ( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_ਚੱਕਰ = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
  pOp->i_Src_ਸਤਰ = 0;
#ਜੇ_ਅੰਤ
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  ਵਾਪਸ i;
}
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(
  Vdbe *p,            /* Add the opcode to this VM */
  ਪੂਰਨ op,             /* The new opcode */
  ਪੂਰਨ p1,             /* The P1 operand */
  ਪੂਰਨ p2,             /* The P2 operand */
  ਪੂਰਨ p3,             /* The P3 operand */
  ਪੂਰਨ p4              /* The P4 operand as an integer */
){
  ਪੂਰਨ i;
  VdbeOp *pOp;

  i = p->nOp;
  ਜੇ( p->nOpAlloc<=i ){
    ਵਾਪਸ ਜੋੜੋ_Op4_Int_Slow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  ਪ੍ਰਤਿਜ੍ਞਾ( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_ਚੱਕਰ = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
  pOp->i_Src_ਸਤਰ = 0;
#ਜੇ_ਅੰਤ
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  ਵਾਪਸ i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Int64(Vdbe *p, ਪੂਰਨ iDest, i64 iVal){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Double(Vdbe *p, ਪੂਰਨ iDest, ਦੁੱਗਣਾ rVal){
  i64 iVal;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&iVal,&rVal,8);
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
ਪੂਰਨ sqlite3VdbeGoto(Vdbe *p, ਪੂਰਨ iDest){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
ਪੂਰਨ sqlite3_Vdbe_ਲੱਦੋ_String(Vdbe *p, ਪੂਰਨ iDest, ਅਚਲ ਅੱਖਰ *zStr){
  ਵਾਪਸ sqlite3_Vdbe_ਜੋੜੋ_Op4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
ਖਾਲੀ sqlite3_Vdbe_Multi_ਲੱਦੋ(Vdbe *p, ਪੂਰਨ iDest, ਅਚਲ ਅੱਖਰ *zTypes, ...){
  va_list ap;
  ਪੂਰਨ i;
  ਅੱਖਰ c;
  va_start(ap, zTypes);
  ਲਈ(i=0; (c = zTypes[i])!=0; i++){
    ਜੇ( c=='s' ){
      ਅਚਲ ਅੱਖਰ *z = va_arg(ap, ਅਚਲ ਅੱਖਰ*);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }ਨਹੀਂਤਾਂ ਜੇ( c=='i' ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(p, OP_Integer, va_arg(ap, ਪੂਰਨ), iDest+i);
    }ਨਹੀਂਤਾਂ{
      ਜਾਓ skip_op_resultrow;
    }
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op2(p, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Op4(
  Vdbe *p,            /* Add the opcode to this VM */
  ਪੂਰਨ op,             /* The new opcode */
  ਪੂਰਨ p1,             /* The P1 operand */
  ਪੂਰਨ p2,             /* The P2 operand */
  ਪੂਰਨ p3,             /* The P3 operand */
  ਅਚਲ ਅੱਖਰ *zP4,    /* The P4 operand */
  ਪੂਰਨ p4type          /* P4 operand type */
){
  ਪੂਰਨ addr = sqlite3_Vdbe_ਜੋੜੋ_Op3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  ਵਾਪਸ addr;
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
ਪੂਰਨ sqlite3_Vdbe_ਜੋੜੋ_Function_Call(
  Parse *pParse,        /* Parsing context */
  ਪੂਰਨ p1,               /* Constant argument mask */
  ਪੂਰਨ p2,               /* First argument register */
  ਪੂਰਨ p3,               /* Register into which results are written */
  ਪੂਰਨ nArg,             /* Number of argument */
  ਅਚਲ FuncDef *pFunc, /* The function to be invoked */
  ਪੂਰਨ eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ addr;
  sqlite3_context *pCtx;
  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  ਜੇ( pCtx==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    ਵਾਪਸ 0;
  }
  pCtx->p_ਬਾਹਰਲਾ = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, eCallCtx ? OP_ਸ਼ੁੱਧ_Func : OP_Function,
                           p1, p2, p3, (ਅੱਖਰ*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  ਵਾਪਸ addr;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
ਪੂਰਨ sqlite3_Vdbe_Explain_ਮਾਪਾ(Parse *pParse){
  VdbeOp *pOp;
  ਜੇ( pParse->addrExplain==0 ) ਵਾਪਸ 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  ਵਾਪਸ pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#ਜੇ defined(SQLITE_DEBUG)
ਖਾਲੀ sqlite3ExplainBreakpoint(ਅਚਲ ਅੱਖਰ *z1, ਅਚਲ ਅੱਖਰ *z2){
  (ਖਾਲੀ)z1;
  (ਖਾਲੀ)z2;
}
#ਜੇ_ਅੰਤ

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
ਪੂਰਨ sqlite3VdbeExplain(Parse *pParse, u8 b_ਧੱਕੋ, ਅਚਲ ਅੱਖਰ *zFmt, ...){
  ਪੂਰਨ addr = 0;
#ਜੇ !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  ਜੇ( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ਜੇ_ਅੰਤ
  {
    ਅੱਖਰ *z_ਸੁਨੇਹਾ;
    Vdbe *v;
    va_list ap;
    ਪੂਰਨ iThis;
    va_start(ap, zFmt);
    z_ਸੁਨੇਹਾ = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_ਸੁਨੇਹਾ, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_ਧੱਕੋ?"PUSH":"", sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v)->p4.z);
    ਜੇ( b_ਧੱਕੋ){
      pParse->addrExplain = iThis;
    }
    sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ(v, iThis, -1, -1, 0, 0);
  }
  ਵਾਪਸ addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
ਖਾਲੀ sqlite3_Vdbe_Explain_ਕੱਢੋ(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3_Vdbe_Explain_ਮਾਪਾ(pParse);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_EXPLAIN */

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
ਖਾਲੀ sqlite3_Vdbe_ਜੋੜੋ_Parse_Schema_Op(Vdbe *p, ਪੂਰਨ iDb, ਅੱਖਰ *zWhere, u16 p5){
  ਪੂਰਨ j;
  ਪ੍ਰਤਿਜ੍ਞਾ( p5==0 || zWhere==0 );
  sqlite3_Vdbe_ਜੋੜੋ_Op4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  ਲਈ(j=0; j<p->db->nDb; j++) sqlite3_Vdbe_ਵਰਤਦਾ_Btree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
ਖਾਲੀ sqlite3_Vdbe_ਸਿਰਾ_Coroutine(Vdbe *v, ਪੂਰਨ regYield){
  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਸਿਰਾ_Coroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_ਆਰਜ਼ੀ_Reg = 0;
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
ਪੂਰਨ sqlite3VdbeMakeLabel(Parse *pParse){
  ਵਾਪਸ --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ resizeResolveLabel(Parse *p, Vdbe *v, ਪੂਰਨ j){
  ਪੂਰਨ n_New_ਮਾਪ = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_ਮਾਪ*ਆਕਾਰ(p->aLabel[0]));
  ਜੇ( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਪੂਰਨ i;
    ਲਈ(i=p->nLabelAlloc; i<n_New_ਮਾਪ; i++) p->aLabel[i] = -1;
#ਜੇ_ਅੰਤ
    ਜੇ( n_New_ਮਾਪ>=100 && (n_New_ਮਾਪ/100)>(p->nLabelAlloc/100) ){
      sqlite3_ਤਰੱਕੀ_Check(p);
    }
    p->nLabelAlloc = n_New_ਮਾਪ;
    p->aLabel[j] = v->nOp;
  }
}
ਖਾਲੀ sqlite3VdbeResolveLabel(Vdbe *v, ਪੂਰਨ x){
  Parse *p = v->pParse;
  ਪੂਰਨ j = ADDR(x);
  ਪ੍ਰਤਿਜ੍ਞਾ( v->eVdbeState==VDBE_INIT_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( j<-p->nLabel );
  ਪ੍ਰਤਿਜ੍ਞਾ( j>=0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( p->db->flags & SQLITE_VdbeAddopTrace ){
    ਲਿਖੋ("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
ਖਾਲੀ sqlite3_Vdbe_ਲੜੀਵਾਰ_Only_Once(Vdbe *p){
  sqlite3_Vdbe_ਜੋੜੋ_Op2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
ਖਾਲੀ sqlite3VdbeReusable(Vdbe *p){
  ਪੂਰਨ i;
  ਲਈ(i=1; ALWAYS(i<p->nOp); i++){
    ਜੇ( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      ਰੋਕੋ;
    }
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VdbeOpIter VdbeOpIter;
ਬਣਤਰ VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  Sub_ਪ੍ਰੋਗਰਾਮ **apSub;        /* Array of subprograms */
  ਪੂਰਨ nSub;                  /* Number of entries in apSub */
  ਪੂਰਨ iAddr;                 /* Address of next instruction to return */
  ਪੂਰਨ iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
ਸਥਿਰ Op *op_Iter_ਅਗਲਾ(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  ਪੂਰਨ nOp;

  ਜੇ( p->iSub<=p->nSub ){

    ਜੇ( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }ਨਹੀਂਤਾਂ{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    ਜੇ( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    ਜੇ( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*ਆਕਾਰ(Sub_ਪ੍ਰੋਗਰਾਮ*);
      ਪੂਰਨ j;
      ਲਈ(j=0; j<p->nSub; j++){
        ਜੇ( p->apSub[j]==pRet->p4.p_ਪ੍ਰੋਗਰਾਮ ) ਰੋਕੋ;
      }
      ਜੇ( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        ਜੇ( !p->apSub ){
          pRet = 0;
        }ਨਹੀਂਤਾਂ{
          p->apSub[p->nSub++] = pRet->p4.p_ਪ੍ਰੋਗਰਾਮ;
        }
      }
    }
  }

  ਵਾਪਸ pRet;
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
ਪੂਰਨ sqlite3VdbeAssertMayAbort(Vdbe *v, ਪੂਰਨ mayAbort){
  ਪੂਰਨ hasAbort = 0;
  ਪੂਰਨ hasFkCounter = 0;
  ਪੂਰਨ has_Create_ਸਾਰਣੀ = 0;
  ਪੂਰਨ has_Create_ਸੂਚਕਾਂਕ = 0;
  ਪੂਰਨ hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  ਜੇ( v==0 ) ਵਾਪਸ 0;
  memset(&sIter, 0, ਆਕਾਰ(sIter));
  sIter.v = v;

  ਜਦੋਂ( (pOp = op_Iter_ਅਗਲਾ(&sIter))!=0 ){
    ਪੂਰਨ opcode = pOp->opcode;
    ਜੇ( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_ਸ਼ੁੱਧ_Func
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      ਰੋਕੋ;
    }
    ਜੇ( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) has_Create_ਸਾਰਣੀ = 1;
    ਜੇ( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      ਜੇ( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_ਸੂਚਕਾਂਕ = 1;
      ਜੇ( opcode==OP_Clear ) has_Create_ਸੂਚਕਾਂਕ = 1;
    }
    ਜੇ( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
    ਜੇ( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
      hasFkCounter = 1;
    }
#ਜੇ_ਅੰਤ
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  ਵਾਪਸ ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (has_Create_ਸਾਰਣੀ && hasInitCoroutine) || has_Create_ਸੂਚਕਾਂਕ
  );
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
ਖਾਲੀ sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  ਜੇ( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
ਖਾਲੀ sqlite3VdbeAssertAbortable(Vdbe *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nWrite==0 || p->ਵਰਤਦਾ_Stmt_Journal );
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ resolve_P2_ਮੁੱਲਾਂ(Vdbe *p, ਪੂਰਨ *p_Max_Vtab_ਦਲੀਲਾਂ){
  ਪੂਰਨ n_Max_Vtab_ਦਲੀਲਾਂ = *p_Max_Vtab_ਦਲੀਲਾਂ;
  Op *pOp;
  Parse *pParse = p->pParse;
  ਪੂਰਨ *aLabel = pParse->aLabel;

  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp[0].opcode==OP_Init );
  ਜਦੋਂ( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    ਜੇ( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      ਚੋਣ( pOp->opcode ){
        ਹਾਲਤ OP_Transaction: {
          ਜੇ( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        ਹਾਲਤ OP_AutoCommit:
        ਹਾਲਤ OP_Savepoint: {
          p->bIsReader = 1;
          ਰੋਕੋ;
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
        ਹਾਲਤ OP_Checkpoint:
#ਜੇ_ਅੰਤ
        ਹਾਲਤ OP_Vacuum:
        ਹਾਲਤ OP_Journal_ਬਹੁਲਕ: {
          p->readOnly = 0;
          p->bIsReader = 1;
          ਰੋਕੋ;
        }
        ਹਾਲਤ OP_Init: {
          ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p2>=0 );
          ਜਾਓ resolve_p2_ਮੁੱਲਾਂ_loop_exit;
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
        ਹਾਲਤ OP_VUpdate: {
          ਜੇ( pOp->p2>n_Max_Vtab_ਦਲੀਲਾਂ ) n_Max_Vtab_ਦਲੀਲਾਂ = pOp->p2;
          ਰੋਕੋ;
        }
        ਹਾਲਤ OP_VFilter: {
          ਪੂਰਨ n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          ਪ੍ਰਤਿਜ੍ਞਾ( (pOp - p->aOp) >= 3 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pOp[-1].opcode==OP_Integer );
          ਪ੍ਰਤਿਜ੍ਞਾ( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          ਜੇ( n>n_Max_Vtab_ਦਲੀਲਾਂ ) n_Max_Vtab_ਦਲੀਲਾਂ = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#ਜੇ_ਅੰਤ
        ਮੂਲ: {
          ਜੇ( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            ਪ੍ਰਤਿਜ੍ਞਾ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            ਪ੍ਰਤਿਜ੍ਞਾ( ADDR(pOp->p2)<-pParse->nLabel );
            ਪ੍ਰਤਿਜ੍ਞਾ( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          ਰੋਕੋ;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      ਪ੍ਰਤਿਜ੍ਞਾ( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_ਮੁੱਲਾਂ_loop_exit:
  ਜੇ( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *p_Max_Vtab_ਦਲੀਲਾਂ = n_Max_Vtab_ਦਲੀਲਾਂ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->bIsReader!=0 || Db_ਨਕਾਬ_All_Zero(p->btree_ਨਕਾਬ) );
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਖਾਲੀ sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  ਪੂਰਨ i_ਪਹਿਲਾ,       /* First opcode of the subroutine */
  ਪੂਰਨ i_ਆਖਰੀ,        /* Last opcode of the subroutine */
  ਪੂਰਨ iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  ਪੂਰਨ i;
  sqlite3_str *pErr = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  pParse = v->pParse;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse!=0 );
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਆਖਰੀ>=i_ਪਹਿਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਆਖਰੀ<v->nOp );
  pOp = &v->aOp[i_ਪਹਿਲਾ];
  ਲਈ(i=i_ਪਹਿਲਾ; i<=i_ਆਖਰੀ; i++, pOp++){
    ਜੇ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      ਪੂਰਨ iDest = pOp->p2;   /* Jump destination */
      ਜੇ( iDest==0 ) ਜਾਰੀ;
      ਜੇ( pOp->opcode==OP_Gosub ) ਜਾਰੀ;
      ਜੇ( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        ਜਾਰੀ;
      }
      ਜੇ( iDest<0 ){
        ਪੂਰਨ j = ADDR(iDest);
        ਪ੍ਰਤਿਜ੍ਞਾ( j>=0 );
        ਜੇ( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          ਜਾਰੀ;
        }
        iDest = pParse->aLabel[j];
      }
      ਜੇ( iDest<i_ਪਹਿਲਾ || iDest>i_ਆਖਰੀ ){
        ਪੂਰਨ j = iDest;
        ਲਈ(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          ਜੇ( pX->opcode==OP_Return ){
            ਜੇ( pX->p1==iRetReg ) ਰੋਕੋ;
            ਜਾਰੀ;
          }
          ਜੇ( pX->opcode==OP_Noop ) ਜਾਰੀ;
          ਜੇ( pX->opcode==OP_Explain ) ਜਾਰੀ;
          ਜੇ( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }ਨਹੀਂਤਾਂ{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, i_ਪਹਿਲਾ, i_ਆਖਰੀ);
          ਰੋਕੋ;
        }
      }
    }
  }
  ਜੇ( pErr ){
    ਅੱਖਰ *zErr = sqlite3_str_ਅੰਤ(pErr);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Halt, SQLITE_ਅੰਦਰੂਨੀ, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
ਪੂਰਨ sqlite3VdbeCurrentAddr(Vdbe *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਵਾਪਸ p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
ਖਾਲੀ sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, ਪੂਰਨ N){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp + N <= p->nOpAlloc );
}
#ਜੇ_ਅੰਤ

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
ਖਾਲੀ sqlite3_Vdbe_Verify_No_ਨਤੀਜਾ_ਪੰਕਤੀ(Vdbe *p){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->nOp; i++){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp[i].opcode!=OP_ਨਤੀਜਾ_ਪੰਕਤੀ );
  }
}
#ਜੇ_ਅੰਤ

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#ਜੇ defined(SQLITE_DEBUG)
ਖਾਲੀ sqlite3VdbeVerifyAbortable(Vdbe *p, ਪੂਰਨ onError){
  ਜੇ( onError==OE_Abort ) sqlite3_Vdbe_ਜੋੜੋ_Op0(p, OP_Abortable);
}
#ਜੇ_ਅੰਤ

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, ਪੂਰਨ *pnOp, ਪੂਰਨ *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  ਪ੍ਰਤਿਜ੍ਞਾ( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  ਪ੍ਰਤਿਜ੍ਞਾ( Db_ਨਕਾਬ_All_Zero(p->btree_ਨਕਾਬ) );

  resolve_P2_ਮੁੱਲਾਂ(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  ਵਾਪਸ aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3_Vdbe_ਜੋੜੋ_Op_List(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  ਪੂਰਨ nOp,                     /* Number of opcodes to add */
  VdbeOpList ਅਚਲ *aOp,       /* The opcodes to be added */
  ਪੂਰਨ iLineno                  /* Source-file line number of first opcode */
){
  ਪੂਰਨ i;
  VdbeOp *p_ਬਾਹਰਲਾ, *p_ਪਹਿਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nOp>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਜੇ( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    ਵਾਪਸ 0;
  }
  p_ਪਹਿਲਾ = p_ਬਾਹਰਲਾ = &p->aOp[p->nOp];
  ਲਈ(i=0; i<nOp; i++, aOp++, p_ਬਾਹਰਲਾ++){
    p_ਬਾਹਰਲਾ->opcode = aOp->opcode;
    p_ਬਾਹਰਲਾ->p1 = aOp->p1;
    p_ਬਾਹਰਲਾ->p2 = aOp->p2;
    ਪ੍ਰਤਿਜ੍ਞਾ( aOp->p2>=0 );
    ਜੇ( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      p_ਬਾਹਰਲਾ->p2 += p->nOp;
    }
    p_ਬਾਹਰਲਾ->p3 = aOp->p3;
    p_ਬਾਹਰਲਾ->p4type = P4_NOTUSED;
    p_ਬਾਹਰਲਾ->p4.p = 0;
    p_ਬਾਹਰਲਾ->p5 = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
    p_ਬਾਹਰਲਾ->zComment = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
    p_ਬਾਹਰਲਾ->i_Src_ਸਤਰ = iLineno+i;
#ਨਹੀਂਤਾਂ
    (ਖਾਲੀ)iLineno;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਜੇ( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#ਜੇ_ਅੰਤ
  }
  p->nOp += nOp;
  ਵਾਪਸ p_ਪਹਿਲਾ;
}

#ਜੇ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
ਖਾਲੀ sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ(
  Vdbe *p,                        /* VM to add scanstatus() to */
  ਪੂਰਨ addrExplain,                /* Address of OP_Explain (or 0) */
  ਪੂਰਨ addrLoop,                   /* Address of loop counter */
  ਪੂਰਨ addr_ਦੇਖੋ,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  ਅਚਲ ਅੱਖਰ *zName               /* Name of table or index being scanned */
){
  ਜੇ( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->n_ਜਾਂਚ) * ਆਕਾਰ(ਜਾਂਚ_ਸਥਿਤੀ);
    ਜਾਂਚ_ਸਥਿਤੀ *aNew;
    aNew = (ਜਾਂਚ_ਸਥਿਤੀ*)sqlite3DbRealloc(p->db, p->a_ਜਾਂਚ, nByte);
    ਜੇ( aNew ){
      ਜਾਂਚ_ਸਥਿਤੀ *pNew = &aNew[p->n_ਜਾਂਚ++];
      memset(pNew, 0, ਆਕਾਰ(ਜਾਂਚ_ਸਥਿਤੀ));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addr_ਦੇਖੋ = addr_ਦੇਖੋ;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->a_ਜਾਂਚ = aNew;
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
ਖਾਲੀ sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(
  Vdbe *p,
  ਪੂਰਨ addrExplain,
  ਪੂਰਨ addr_ਸ਼ੁਰੂ,
  ਪੂਰਨ addr_ਸਿਰਾ
){
  ਜੇ( IS_STMT_SCANSTATUS(p->db) ){
    ਜਾਂਚ_ਸਥਿਤੀ *p_ਜਾਂਚ = 0;
    ਪੂਰਨ ii;
    ਲਈ(ii=p->n_ਜਾਂਚ-1; ii>=0; ii--){
      p_ਜਾਂਚ = &p->a_ਜਾਂਚ[ii];
      ਜੇ( p_ਜਾਂਚ->addrExplain==addrExplain ) ਰੋਕੋ;
      p_ਜਾਂਚ = 0;
    }
    ਜੇ( p_ਜਾਂਚ ){
      ਜੇ( addr_ਸਿਰਾ<0 ) addr_ਸਿਰਾ = sqlite3VdbeCurrentAddr(p)-1;
      ਲਈ(ii=0; ii<Array_ਮਾਪ(p_ਜਾਂਚ->aAddrRange); ii+=2){
        ਜੇ( p_ਜਾਂਚ->aAddrRange[ii]==0 ){
          p_ਜਾਂਚ->aAddrRange[ii] = addr_ਸ਼ੁਰੂ;
          p_ਜਾਂਚ->aAddrRange[ii+1] = addr_ਸਿਰਾ;
          ਰੋਕੋ;
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
ਖਾਲੀ sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(
  Vdbe *p,
  ਪੂਰਨ addrExplain,
  ਪੂਰਨ addrLoop,
  ਪੂਰਨ addr_ਦੇਖੋ
){
  ਜੇ( IS_STMT_SCANSTATUS(p->db) ){
    ਜਾਂਚ_ਸਥਿਤੀ *p_ਜਾਂਚ = 0;
    ਪੂਰਨ ii;
    ਲਈ(ii=p->n_ਜਾਂਚ-1; ii>=0; ii--){
      p_ਜਾਂਚ = &p->a_ਜਾਂਚ[ii];
      ਜੇ( p_ਜਾਂਚ->addrExplain==addrExplain ) ਰੋਕੋ;
      p_ਜਾਂਚ = 0;
    }
    ਜੇ( p_ਜਾਂਚ ){
      ਜੇ( addrLoop>0 ) p_ਜਾਂਚ->addrLoop = addrLoop;
      ਜੇ( addr_ਦੇਖੋ>0 ) p_ਜਾਂਚ->addr_ਦੇਖੋ = addr_ਦੇਖੋ;
    }
  }
}
#ਜੇ_ਅੰਤ /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
ਖਾਲੀ sqlite3VdbeChangeOpcode(Vdbe *p, ਪੂਰਨ addr, u8 iNewOpcode){
  ਪ੍ਰਤਿਜ੍ਞਾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
ਖਾਲੀ sqlite3VdbeChangeP1(Vdbe *p, ਪੂਰਨ addr, ਪੂਰਨ val){
  ਪ੍ਰਤਿਜ੍ਞਾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
ਖਾਲੀ sqlite3VdbeChangeP2(Vdbe *p, ਪੂਰਨ addr, ਪੂਰਨ val){
  ਪ੍ਰਤਿਜ੍ਞਾ( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
ਖਾਲੀ sqlite3VdbeChangeP3(Vdbe *p, ਪੂਰਨ addr, ਪੂਰਨ val){
  ਪ੍ਰਤਿਜ੍ਞਾ( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
ਖਾਲੀ sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 || p->db->mallocFailed );
  ਜੇ( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
ਖਾਲੀ sqlite3_Vdbe_Typeof_ਕਾਲਮ(Vdbe *p, ਪੂਰਨ iDest){
  VdbeOp *pOp = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(p);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜਦੋਂ( pOp->opcode==OP_ReleaseReg ) pOp--;
#ਜੇ_ਅੰਤ
  ਜੇ( pOp->p3==iDest && pOp->opcode==OP_ਕਾਲਮ ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
ਖਾਲੀ sqlite3_Vdbe_Jump_ਇੱਥੇ(Vdbe *p, ਪੂਰਨ addr){
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
ਖਾਲੀ sqlite3_Vdbe_Jump_ਇੱਥੇ_Or_ਕੱਢੋ_Inst(Vdbe *p, ਪੂਰਨ addr){
  ਜੇ( addr==p->nOp-1 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp[addr].p4type==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
    sqlite3_Vdbe_Get_ਆਖਰੀ_Op(p)->i_Src_ਸਤਰ = 0;  /* Erase VdbeCoverage() macros */
#ਜੇ_ਅੰਤ
    p->nOp--;
  }ਨਹੀਂਤਾਂ{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
ਸਥਿਰ ਖਾਲੀ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ freeP4Mem(sqlite3 *db, Mem *p){
  ਜੇ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
ਸਥਿਰ ਖਾਲੀ freeP4(sqlite3 *db, ਪੂਰਨ p4type, ਖਾਲੀ *p4){
  ਪ੍ਰਤਿਜ੍ਞਾ( db );
  ਚੋਣ( p4type ){
    ਹਾਲਤ P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_DYNAMIC:
    ਹਾਲਤ P4_INTARRAY: {
      ਜੇ( p4 ) sqlite3DbNNFreeNN(db, p4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_KEYINFO: {
      ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ) sqlite3_ਕੁੰਜੀ_Info_Unref((ਕੁੰਜੀ_Info*)p4);
      ਰੋਕੋ;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
    ਹਾਲਤ P4_ਵਿਅੰਜਕ: {
      sqlite3_ਵਿਅੰਜਕ_Delete(db, (ਵਿਅੰਜਕ*)p4);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_MEM: {
      ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ){
        sqlite3_ਮੁੱਲ_Free((sqlite3_ਮੁੱਲ*)p4);
      }ਨਹੀਂਤਾਂ{
        freeP4Mem(db, (Mem*)p4);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_VTAB : {
      ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ) sqlite3VtabUnlock((VTable *)p4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_TABLEREF: {
      ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ) sqlite3_Delete_ਸਾਰਣੀ(db, (ਸਾਰਣੀ*)p4);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      ਰੋਕੋ;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
ਸਥਿਰ ਖਾਲੀ vdbeFreeOpArray(sqlite3 *db, Op *aOp, ਪੂਰਨ nOp){
  ਪ੍ਰਤਿਜ੍ਞਾ( nOp>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( aOp ){
    Op *pOp = &aOp[nOp-1];
    ਜਦੋਂ(1){  /* Exit via break */
      ਜੇ( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#ਜੇ_ਅੰਤ    
      ਜੇ( pOp==aOp ) ਰੋਕੋ;
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
ਖਾਲੀ sqlite3_Vdbe_Link_Sub_ਪ੍ਰੋਗਰਾਮ(Vdbe *pVdbe, Sub_ਪ੍ਰੋਗਰਾਮ *p){
  p->p_ਅਗਲਾ = pVdbe->p_ਪ੍ਰੋਗਰਾਮ;
  pVdbe->p_ਪ੍ਰੋਗਰਾਮ = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
ਪੂਰਨ sqlite3_Vdbe_Has_Sub_ਪ੍ਰੋਗਰਾਮ(Vdbe *pVdbe){
  ਵਾਪਸ pVdbe->p_ਪ੍ਰੋਗਰਾਮ!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
ਪੂਰਨ sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(Vdbe *p, ਪੂਰਨ addr){
  VdbeOp *pOp;
  ਜੇ( p->db->mallocFailed ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  ਵਾਪਸ 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
ਪੂਰਨ sqlite3_Vdbe_Delete_ਪੂਰਵ_Opcode(Vdbe *p, u8 op){
  ਜੇ( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    ਵਾਪਸ sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(p, p->nOp-1);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
ਖਾਲੀ sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  ਪੂਰਨ i_ਪਹਿਲਾ,          /* Index of first register to be released */
  ਪੂਰਨ N,               /* Number of registers to release */
  u32 ਨਕਾਬ,            /* Mask of registers to NOT release */
  ਪੂਰਨ bUndefine        /* If true, mark registers as undefined */
){
  ਜੇ( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVdbe );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਪਹਿਲਾ>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਪਹਿਲਾ+N-1<=pParse->nMem );
  ਜੇ( N<=31 && ਨਕਾਬ!=0 ){
    ਜਦੋਂ( N>0 && (ਨਕਾਬ&1)!=0 ){
      ਨਕਾਬ >>= 1;
      i_ਪਹਿਲਾ++;
      N--;
    }
    ਜਦੋਂ( N>0 && N<=32 && (ਨਕਾਬ & MASKBIT32(N-1))!=0 ){
      ਨਕਾਬ &= ~MASKBIT32(N-1);
      N--;
    }
  }
  ਜੇ( N>0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op3(pParse->pVdbe, OP_ReleaseReg, i_ਪਹਿਲਾ, N, *(ਪੂਰਨ*)&ਨਕਾਬ);
    ਜੇ( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

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
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE vdbe_Change_P4_ਭਰਿਆ(
  Vdbe *p,
  Op *pOp,
  ਅਚਲ ਅੱਖਰ *zP4,
  ਪੂਰਨ n
){
  ਜੇ( pOp->p4type ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  ਜੇ( n<0 ){
    sqlite3VdbeChangeP4(p, (ਪੂਰਨ)(pOp - p->aOp), zP4, n);
  }ਨਹੀਂਤਾਂ{
    ਜੇ( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
ਖਾਲੀ sqlite3VdbeChangeP4(Vdbe *p, ਪੂਰਨ addr, ਅਚਲ ਅੱਖਰ *zP4, ਪੂਰਨ n){
  Op *pOp;
  sqlite3 *db;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  db = p->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp!=0 || db->mallocFailed );
  ਜੇ( db->mallocFailed ){
    ਜੇ( n!=P4_VTAB ) freeP4(db, n, (ਖਾਲੀ*)*(ਅੱਖਰ**)&zP4);
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( addr<p->nOp );
  ਜੇ( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  ਜੇ( n>=0 || pOp->p4type ){
    vdbe_Change_P4_ਭਰਿਆ(p, pOp, zP4, n);
    ਵਾਪਸ;
  }
  ਜੇ( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(zP4);
    pOp->p4type = P4_INT32;
  }ਨਹੀਂਤਾਂ ਜੇ( zP4!=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( n<0 );
    pOp->p4.p = (ਖਾਲੀ*)zP4;
    pOp->p4type = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ)n;
    ਜੇ( n==P4_VTAB ) sqlite3_Vtab_ਤਾਲਾ((VTable*)zP4);
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
ਖਾਲੀ sqlite3VdbeAppendP4(Vdbe *p, ਖਾਲੀ *pP4, ਪੂਰਨ n){
  VdbeOp *pOp;
  ਪ੍ਰਤਿਜ੍ਞਾ( n!=P4_INT32 && n!=P4_VTAB );
  ਪ੍ਰਤਿਜ੍ਞਾ( n<=0 );
  ਜੇ( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pP4!=0 || n==P4_DYNAMIC );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(Parse *pParse, ਸੂਚਕਾਂਕ *pIdx){
  Vdbe *v = pParse->pVdbe;
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );
  p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_ਦਾ_ਸੂਚਕਾਂਕ(pParse, pIdx);
  ਜੇ( p_ਕੁੰਜੀ_Info ) sqlite3VdbeAppendP4(v, p_ਕੁੰਜੀ_Info, P4_KEYINFO);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
ਸਥਿਰ ਖਾਲੀ vdbeVComment(Vdbe *p, ਅਚਲ ਅੱਖਰ *zFormat, va_list ap){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 || p->aOp==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  ਜੇ( p->nOp ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
ਖਾਲੀ sqlite3VdbeComment(Vdbe *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਜੇ( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
ਖਾਲੀ sqlite3VdbeNoopComment(Vdbe *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਜੇ( p ){
    sqlite3_Vdbe_ਜੋੜੋ_Op0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#ਜੇ_ਅੰਤ  /* NDEBUG */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_ਸਤਰ_ਸੰਖਿਆ(Vdbe *v, ਪੂਰਨ i_ਸਤਰ){
  sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v)->i_Src_ਸਤਰ = i_ਸਤਰ;
}
#ਜੇ_ਅੰਤ /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, ਪੂਰਨ addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  ਸਥਿਰ VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  ਜੇ( p->db->mallocFailed ){
    ਵਾਪਸ (VdbeOp*)&dummy;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3_Vdbe_Get_ਆਖਰੀ_Op(Vdbe *p){
  ਵਾਪਸ sqlite3VdbeGetOp(p, p->nOp - 1);
}

#ਜੇ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
ਸਥਿਰ ਪੂਰਨ translateP(ਅੱਖਰ c, ਅਚਲ Op *pOp){
  ਜੇ( c=='1' ) ਵਾਪਸ pOp->p1;
  ਜੇ( c=='2' ) ਵਾਪਸ pOp->p2;
  ਜੇ( c=='3' ) ਵਾਪਸ pOp->p3;
  ਜੇ( c=='4' ) ਵਾਪਸ pOp->p4.i;
  ਵਾਪਸ pOp->p5;
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
ਅੱਖਰ *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  ਅਚਲ Op *pOp,     /* The opcode to be commented */
  ਅਚਲ ਅੱਖਰ *zP4    /* Previously obtained value for P4 */
){
  ਅਚਲ ਅੱਖਰ *zOpName;
  ਅਚਲ ਅੱਖਰ *zSynopsis;
  ਪੂਰਨ nOpName;
  ਪੂਰਨ ii;
  ਅੱਖਰ zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_ਲੰਮਾਈ);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  ਜੇ( zOpName[nOpName+1] ){
    ਪੂਰਨ ਵੇਖਿਆ_Com = 0;
    ਅੱਖਰ c;
    zSynopsis = zOpName + nOpName + 1;
    ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(ਆਕਾਰ(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    ਲਈ(ii=0; (c = zSynopsis[ii])!=0; ii++){
      ਜੇ( c=='P' ){
        c = zSynopsis[++ii];
        ਜੇ( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }ਨਹੀਂਤਾਂ ਜੇ( c=='X' ){
          ਜੇ( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            ਵੇਖਿਆ_Com = 1;
            ਰੋਕੋ;
          }
        }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਤੁਲਨਾ(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_ਮੰਜ਼ਿਲ_64(pOp->p1,pOp->p3));
          ii += 4;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_ਵਰਤਿਆ_ਨਕਾਬ
        }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਤੁਲਨਾ(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_ਮੰਜ਼ਿਲ_64(pOp->p2,pOp->p3));
          ii += 4;
#ਜੇ_ਅੰਤ
        }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਤੁਲਨਾ(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_ਮੰਜ਼ਿਲ_64(pOp->p1,pOp->p3);
          ਦੁੱਗਣਾ r;
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ v1 = translateP(c, pOp);
          ਪੂਰਨ v2;
          ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            ਜੇ( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ਨਹੀਂਤਾਂ{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            ਜੇ( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ਨਹੀਂਤਾਂ ਜੇ( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }ਨਹੀਂਤਾਂ ਜੇ( x.accError==0 ){
              ਪ੍ਰਤਿਜ੍ਞਾ( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }ਨਹੀਂਤਾਂ{
            sqlite3_str_appendf(&x, "%d", v1);
            ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }ਨਹੀਂਤਾਂ{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    ਜੇ( !ਵੇਖਿਆ_Com && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  ਜੇ( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  ਵਾਪਸ sqlite3_Str_Accum_ਅੰਤ(&x);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#ਜੇ VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
ਸਥਿਰ ਖਾਲੀ display_P4_ਵਿਅੰਜਕ(StrAccum *p, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਅਚਲ ਅੱਖਰ *zOp = 0;
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_STRING:
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_str_appendf(p, "%Q", p_ਵਿਅੰਜਕ->u.zToken);
      ਰੋਕੋ;
    ਹਾਲਤ TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_ਵਿਅੰਜਕ->u.i_ਮੁੱਲ);
      ਰੋਕੋ;
    ਹਾਲਤ TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      ਰੋਕੋ;
    ਹਾਲਤ TK_ਰਜਿਸਟਰ: {
      sqlite3_str_appendf(p, "r[%d]", p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਕਾਲਮ: {
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ){
        sqlite3_str_appendf(p, "rowid");
      }ਨਹੀਂਤਾਂ{
        sqlite3_str_appendf(p, "c%d", (ਪੂਰਨ)p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_LT:      zOp = "LT";      ਰੋਕੋ;
    ਹਾਲਤ TK_LE:      zOp = "LE";      ਰੋਕੋ;
    ਹਾਲਤ TK_GT:      zOp = "GT";      ਰੋਕੋ;
    ਹਾਲਤ TK_GE:      zOp = "GE";      ਰੋਕੋ;
    ਹਾਲਤ TK_NE:      zOp = "NE";      ਰੋਕੋ;
    ਹਾਲਤ TK_EQ:      zOp = "EQ";      ਰੋਕੋ;
    ਹਾਲਤ TK_IS:      zOp = "IS";      ਰੋਕੋ;
    ਹਾਲਤ TK_ISNOT:   zOp = "ISNOT";   ਰੋਕੋ;
    ਹਾਲਤ TK_AND:     zOp = "AND";     ਰੋਕੋ;
    ਹਾਲਤ TK_OR:      zOp = "OR";      ਰੋਕੋ;
    ਹਾਲਤ TK_PLUS:    zOp = "ADD";     ਰੋਕੋ;
    ਹਾਲਤ TK_ਤਾਰਾ:    zOp = "MUL";     ਰੋਕੋ;
    ਹਾਲਤ TK_MINUS:   zOp = "SUB";     ਰੋਕੋ;
    ਹਾਲਤ TK_REM:     zOp = "REM";     ਰੋਕੋ;
    ਹਾਲਤ TK_BITAND:  zOp = "BITAND";  ਰੋਕੋ;
    ਹਾਲਤ TK_BITOR:   zOp = "BITOR";   ਰੋਕੋ;
    ਹਾਲਤ TK_SLASH:   zOp = "DIV";     ਰੋਕੋ;
    ਹਾਲਤ TK_LSHIFT:  zOp = "LSHIFT";  ਰੋਕੋ;
    ਹਾਲਤ TK_RSHIFT:  zOp = "RSHIFT";  ਰੋਕੋ;
    ਹਾਲਤ TK_CONCAT:  zOp = "CONCAT";  ਰੋਕੋ;
    ਹਾਲਤ TK_UMINUS:  zOp = "MINUS";   ਰੋਕੋ;
    ਹਾਲਤ TK_UPLUS:   zOp = "PLUS";    ਰੋਕੋ;
    ਹਾਲਤ TK_BITNOT:  zOp = "BITNOT";  ਰੋਕੋ;
    ਹਾਲਤ TK_NOT:     zOp = "NOT";     ਰੋਕੋ;
    ਹਾਲਤ TK_ISNULL:  zOp = "ISNULL";  ਰੋਕੋ;
    ਹਾਲਤ TK_NOTNULL: zOp = "NOTNULL"; ਰੋਕੋ;

    ਮੂਲ:
      sqlite3_str_appendf(p, "%s", "expr");
      ਰੋਕੋ;
  }

  ਜੇ( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_ਵਿਅੰਜਕ(p, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    ਜੇ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ ){
      sqlite3_str_append(p, ",", 1);
      display_P4_ਵਿਅੰਜਕ(p, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#ਜੇ_ਅੰਤ /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#ਜੇ VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
ਅੱਖਰ *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  ਅੱਖਰ *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_ਲੰਮਾਈ);
  ਚੋਣ( pOp->p4type ){
    ਹਾਲਤ P4_KEYINFO: {
      ਪੂਰਨ j;
      ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = pOp->p4.p_ਕੁੰਜੀ_Info;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ);
      ਲਈ(j=0; j<p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ; j++){
        CollSeq *pColl = p_ਕੁੰਜੀ_Info->aColl[j];
        ਅਚਲ ਅੱਖਰ *zColl = pColl ? pColl->zName : "";
        ਜੇ( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[j] & KEYINFO_ਕ੍ਰਮ_DESC) ? "-" : "",
               (p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[j] & KEYINFO_ਕ੍ਰਮ_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      ਰੋਕੋ;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
    ਹਾਲਤ P4_ਵਿਅੰਜਕ: {
      display_P4_ਵਿਅੰਜਕ(&x, pOp->p4.p_ਵਿਅੰਜਕ);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ P4_COLLSEQ: {
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      ਪ੍ਰਤਿਜ੍ਞਾ( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      ਜੇ( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }ਨਹੀਂਤਾਂ ਜੇ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }ਨਹੀਂਤਾਂ ਜੇ( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }ਨਹੀਂਤਾਂ ਜੇ( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਹਾਲਤ P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      ਲਈ(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_SUBPROGRAM: {
      zP4 = "program";
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_ਸਾਰਣੀ: {
      zP4 = pOp->p4.pTab->zName;
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_ਸੂਚਕਾਂਕ: {
      zP4 = pOp->p4.pIdx->zName;
      ਰੋਕੋ;
    }
    ਹਾਲਤ P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      zP4 = pOp->p4.z;
    }
  }
  ਜੇ( zP4 ) sqlite3_str_appendall(&x, zP4);
  ਜੇ( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  ਵਾਪਸ sqlite3_Str_Accum_ਅੰਤ(&x);
}
#ਜੇ_ਅੰਤ /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
ਖਾਲੀ sqlite3_Vdbe_ਵਰਤਦਾ_Btree(Vdbe *p, ਪੂਰਨ i){
  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<p->db->nDb && i<(ਪੂਰਨ)ਆਕਾਰ(y_Db_ਨਕਾਬ)*8 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i<(ਪੂਰਨ)ਆਕਾਰ(p->btree_ਨਕਾਬ)*8 );
  Db_ਨਕਾਬ_ਸੈੱਟ(p->btree_ਨਕਾਬ, i);
  ਜੇ( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    Db_ਨਕਾਬ_ਸੈੱਟ(p->ਤਾਲਾ_ਨਕਾਬ, i);
  }
}

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE)
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
ਖਾਲੀ sqlite3VdbeEnter(Vdbe *p){
  ਪੂਰਨ i;
  sqlite3 *db;
  Db *aDb;
  ਪੂਰਨ nDb;
  ਜੇ( Db_ਨਕਾਬ_All_Zero(p->ਤਾਲਾ_ਨਕਾਬ) ) ਵਾਪਸ;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  ਲਈ(i=0; i<nDb; i++){
    ਜੇ( i!=1 && Db_ਨਕਾਬ_Test(p->ਤਾਲਾ_ਨਕਾਬ,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ vdbeLeave(Vdbe *p){
  ਪੂਰਨ i;
  sqlite3 *db;
  Db *aDb;
  ਪੂਰਨ nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  ਲਈ(i=0; i<nDb; i++){
    ਜੇ( i!=1 && Db_ਨਕਾਬ_Test(p->ਤਾਲਾ_ਨਕਾਬ,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
ਖਾਲੀ sqlite3VdbeLeave(Vdbe *p){
  ਜੇ( Db_ਨਕਾਬ_All_Zero(p->ਤਾਲਾ_ਨਕਾਬ) ) ਵਾਪਸ;  /* The common case */
  vdbeLeave(p);
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
ਖਾਲੀ sqlite3VdbePrintOp(FILE *p_ਬਾਹਰਲਾ, ਪੂਰਨ pc, VdbeOp *pOp){
  ਅੱਖਰ *zP4;
  ਅੱਖਰ *zCom;
  sqlite3 dummyDb;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  ਜੇ( p_ਬਾਹਰਲਾ==0 ) p_ਬਾਹਰਲਾ = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#ਨਹੀਂਤਾਂ
  zCom = 0;
#ਜੇ_ਅੰਤ
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(p_ਬਾਹਰਲਾ, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(p_ਬਾਹਰਲਾ);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3_ਸਿਰਾ_Benign_Malloc();
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ initMemArray(Mem *p, ਪੂਰਨ N, sqlite3 *db, u16 flags){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( N>0 ){
    ਕਰੋ{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#ਜੇ_ਅੰਤ
      p++;
    }ਜਦੋਂ( (--N)>0 );
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
ਸਥਿਰ ਖਾਲੀ releaseMemArray(Mem *p, ਪੂਰਨ N){
  ਜੇ( p && N ){
    Mem *p_ਸਿਰਾ = &p[N];
    sqlite3 *db = p->db;
    ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
    ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed ){
      ਕਰੋ{
        ਜੇ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }ਜਦੋਂ( (++p)<p_ਸਿਰਾ );
      ਵਾਪਸ;
    }
    ਕਰੋ{
      ਪ੍ਰਤਿਜ੍ਞਾ( (&p[1])==p_ਸਿਰਾ || p[0].db==p[1].db );
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeCheckMemInvariants(p) );

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
      ਜੇ( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3_Vdbe_ਚੌਖਟਾ_Mem_Del );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }ਨਹੀਂਤਾਂ ਜੇ( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
      ਨਹੀਂਤਾਂ{
        p->flags = MEM_Undefined;
      }
#ਜੇ_ਅੰਤ
    }ਜਦੋਂ( (++p)<p_ਸਿਰਾ );
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
ਪੂਰਨ sqlite3_Vdbe_ਚੌਖਟਾ_Is_Valid(Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ){
  ਜੇ( p_ਚੌਖਟਾ->i_ਚੌਖਟਾ_Magic!=SQLITE_ਚੌਖਟਾ_MAGIC ) ਵਾਪਸ 0;
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
ਖਾਲੀ sqlite3_Vdbe_ਚੌਖਟਾ_Mem_Del(ਖਾਲੀ *pArg){
  Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ = (Vdbe_ਚੌਖਟਾ*)pArg;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Vdbe_ਚੌਖਟਾ_Is_Valid(p_ਚੌਖਟਾ) );
  p_ਚੌਖਟਾ->p_ਮਾਪਾ = p_ਚੌਖਟਾ->v->p_Del_ਚੌਖਟਾ;
  p_ਚੌਖਟਾ->v->p_Del_ਚੌਖਟਾ = p_ਚੌਖਟਾ;
}

#ਜੇ defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
ਪੂਰਨ sqlite3_Vdbe_ਅਗਲਾ_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  ਪੂਰਨ e_ਬਹੁਲਕ,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  ਪੂਰਨ *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  ਪੂਰਨ *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  ਪੂਰਨ n_ਪੰਕਤੀ;                            /* Stop when row count reaches this */
  ਪੂਰਨ nSub = 0;                        /* Number of sub-vdbes seen so far */
  Sub_ਪ੍ਰੋਗਰਾਮ **apSub = 0;              /* Array of sub-vdbes */
  ਪੂਰਨ i;                               /* Next instruction address */
  ਪੂਰਨ rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  ਪੂਰਨ iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  n_ਪੰਕਤੀ = p->nOp;
  ਜੇ( pSub!=0 ){
    ਜੇ( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/ਆਕਾਰ(Vdbe*);
      apSub = (Sub_ਪ੍ਰੋਗਰਾਮ **)pSub->z;
    }
    ਲਈ(i=0; i<nSub; i++){
      n_ਪੰਕਤੀ += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  ਜਦੋਂ(1){  /* Loop exits via break */
    i = iPc++;
    ਜੇ( i>=n_ਪੰਕਤੀ ){
      p->rc = SQLITE_OK;
      rc = SQLITE_ਪੂਰਾ;
      ਰੋਕੋ;
    }
    ਜੇ( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }ਨਹੀਂਤਾਂ{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      ਪੂਰਨ j;
      i -= p->nOp;
      ਪ੍ਰਤਿਜ੍ਞਾ( apSub!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( nSub>0 );
      ਲਈ(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        ਪ੍ਰਤਿਜ੍ਞਾ( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    ਜੇ( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      ਪੂਰਨ nByte = (nSub+1)*ਆਕਾਰ(Sub_ਪ੍ਰੋਗਰਾਮ*);
      ਪੂਰਨ j;
      ਲਈ(j=0; j<nSub; j++){
        ਜੇ( apSub[j]==aOp[i].p4.p_ਪ੍ਰੋਗਰਾਮ ) ਰੋਕੋ;
      }
      ਜੇ( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        ਜੇ( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          ਰੋਕੋ;
        }
        apSub = (Sub_ਪ੍ਰੋਗਰਾਮ **)pSub->z;
        apSub[nSub++] = aOp[i].p4.p_ਪ੍ਰੋਗਰਾਮ;
        Mem_ਸੈੱਟ_Type_Flag(pSub, MEM_Blob);
        pSub->n = nSub*ਆਕਾਰ(Sub_ਪ੍ਰੋਗਰਾਮ*);
        n_ਪੰਕਤੀ += aOp[i].p4.p_ਪ੍ਰੋਗਰਾਮ->nOp;
      }
    }
    ਜੇ( e_ਬਹੁਲਕ==0 ) ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_BYTECODE_VTAB
    ਜੇ( e_ਬਹੁਲਕ==2 ){
      Op *pOp = aOp + i;
      ਜੇ( pOp->opcode==OP_OpenRead ) ਰੋਕੋ;
      ਜੇ( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) ਰੋਕੋ;
      ਜੇ( pOp->opcode==OP_ReopenIdx ) ਰੋਕੋ;     
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ==1 );
      ਜੇ( aOp[i].opcode==OP_Explain ) ਰੋਕੋ;
      ਜੇ( aOp[i].opcode==OP_Init && iPc>1 ) ਰੋਕੋ;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
ਖਾਲੀ sqlite3_Vdbe_ਚੌਖਟਾ_Delete(Vdbe_ਚੌਖਟਾ *p){
  ਪੂਰਨ i;
  Mem *aMem = Vdbe_ਚੌਖਟਾ_Mem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->n_ਬੱਚਾ_Mem];
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Vdbe_ਚੌਖਟਾ_Is_Valid(p) );
  ਲਈ(i=0; i<p->n_ਬੱਚਾ_Csr; i++){
    ਜੇ( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->n_ਬੱਚਾ_Mem);
  sqlite3_Vdbe_Delete_Aux_ਅੰਕੜੇ(p->v->db, &p->p_Aux_ਅੰਕੜੇ, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
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
ਪੂਰਨ sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  ਪੂਰਨ i;                               /* Loop counter */
  ਪੂਰਨ rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  ਪੂਰਨ bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  ਪ੍ਰਤਿਜ੍ਞਾ( p->explain );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_ਲੜੀਵਾਰ_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  ਜੇ( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nMem>9 );
    pSub = &p->aMem[9];
  }ਨਹੀਂਤਾਂ{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_ਅਗਲਾ_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  ਜੇ( rc==SQLITE_OK ){
    pOp = aOp + i;
    ਜੇ( Atomic_ਲੱਦੋ(&db->u1.isInterrupted) ){
      p->rc = SQLITE_ਰੁਕਾਵਟ;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      ਜੇ( p->explain==2 ){
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem, pOp->p1);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+1, pOp->p2);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+2, pOp->p3);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ਪ੍ਰਤਿਜ੍ਞਾ( p->n_Res_ਕਾਲਮ==4 );
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+0, i);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pMem+1, (ਅੱਖਰ*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+2, pOp->p1);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+3, pOp->p2);
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(pMem+6, pOp->p5);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          ਅੱਖਰ *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#ਨਹੀਂਤਾਂ
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem+7);
#ਜੇ_ਅੰਤ
        sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ਪ੍ਰਤਿਜ੍ਞਾ( p->n_Res_ਕਾਲਮ==8 );
      }
      p->p_ਨਤੀਜਾ_ਪੰਕਤੀ = pMem;
      ਜੇ( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }ਨਹੀਂਤਾਂ{
        p->rc = SQLITE_OK;
        rc = SQLITE_ਪੰਕਤੀ;
      }
    }
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_EXPLAIN */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
ਖਾਲੀ sqlite3VdbePrintSql(Vdbe *p){
  ਅਚਲ ਅੱਖਰ *z = 0;
  ਜੇ( p->zSql ){
    z = p->zSql;
  }ਨਹੀਂਤਾਂ ਜੇ( p->nOp>=1 ){
    ਅਚਲ VdbeOp *pOp = &p->aOp[0];
    ਜੇ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      ਜਦੋਂ( sqlite3Isspace(*z) ) z++;
    }
  }
  ਜੇ( z ) ਲਿਖੋ("SQL: [%s]\n", z);
}
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
ਖਾਲੀ sqlite3VdbeIOTraceSql(Vdbe *p){
  ਪੂਰਨ nOp = p->nOp;
  VdbeOp *pOp;
  ਜੇ( sqlite3IoTrace==0 ) ਵਾਪਸ;
  ਜੇ( nOp<1 ) ਵਾਪਸ;
  pOp = &p->aOp[0];
  ਜੇ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    ਪੂਰਨ i, j;
    ਅੱਖਰ z[1000];
    sqlite3_snprintf(ਆਕਾਰ(z), z, "%s", pOp->p4.z);
    ਲਈ(i=0; sqlite3Isspace(z[i]); i++){}
    ਲਈ(j=0; z[i]; i++){
      ਜੇ( sqlite3Isspace(z[i]) ){
        ਜੇ( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }ਨਹੀਂਤਾਂ{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
ਬਣਤਰ ReusableSpace {
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
ਸਥਿਰ ਖਾਲੀ *allocSpace(
  ਬਣਤਰ ReusableSpace *p,  /* Bulk memory available for allocation */
  ਖਾਲੀ *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  ਜੇ( pBuf==0 ){
    nByte = ROUND8P(nByte);
    ਜੇ( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }ਨਹੀਂਤਾਂ{
      p->nNeeded += nByte;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pBuf) );
  ਵਾਪਸ pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
ਖਾਲੀ sqlite3VdbeRewind(Vdbe *p){
#ਜੇ defined(SQLITE_DEBUG)
  ਪੂਰਨ i;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਲਈ(i=0; i<p->nMem; i++){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->aMem[i].db==p->db );
  }
#ਜੇ_ਅੰਤ
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VDBE_PROFILE
  ਲਈ(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].n_ਚੱਕਰ = 0;
  }
#ਜੇ_ਅੰਤ
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
ਖਾਲੀ sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  ਪੂਰਨ nVar;                      /* Number of parameters */
  ਪੂਰਨ nMem;                      /* Number of VM memory registers */
  ਪੂਰਨ nCursor;                   /* Number of cursors required */
  ਪੂਰਨ nArg;                      /* Max number args to xFilter or xUpdate */
  ਪੂਰਨ n;                         /* Loop counter */
  ਬਣਤਰ ReusableSpace x;        /* Reusable bulk memory */

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nOp>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_INIT_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse==p->pParse );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
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
  ਜੇ( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(ਆਕਾਰ(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  ਪ੍ਰਤਿਜ੍ਞਾ( x.nFree>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_ਮੁੱਲਾਂ(p, &nArg);
  p->ਵਰਤਦਾ_Stmt_Journal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  ਜੇ( pParse->explain ){
    ਜੇ( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->n_Res_ਕਾਲਮ = 12 - 4*p->explain;
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
  p->aMem = allocSpace(&x, 0, nMem*ਆਕਾਰ(Mem));
  p->aVar = allocSpace(&x, 0, nVar*ਆਕਾਰ(Mem));
  p->apArg = allocSpace(&x, 0, nArg*ਆਕਾਰ(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*ਆਕਾਰ(VdbeCursor*));
  ਜੇ( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    ਜੇ( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*ਆਕਾਰ(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*ਆਕਾਰ(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*ਆਕਾਰ(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*ਆਕਾਰ(VdbeCursor*));
    }
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  p->napArg = nArg;
#ਜੇ_ਅੰਤ

  ਜੇ( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }ਨਹੀਂਤਾਂ{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*ਆਕਾਰ(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
ਖਾਲੀ sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  ਜੇ( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCx->ਖਾਨਾ_Cache );
  pCx->ਖਾਨਾ_Cache = 0;
  pCx->pCache = 0;
  ਜੇ( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
ਖਾਲੀ sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  ਜੇ( pCx->ਖਾਨਾ_Cache ){
    freeCursorWithCache(p, pCx);
    ਵਾਪਸ;
  }
  ਚੋਣ( pCx->eCurType ){
    ਹਾਲਤ CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      ਰੋਕੋ;
    }
    ਹਾਲਤ CURTYPE_BTREE: {
      ਪ੍ਰਤਿਜ੍ਞਾ( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਹਾਲਤ CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      ਅਚਲ sqlite3_module *pModule = pVCur->pVtab->pModule;
      ਪ੍ਰਤਿਜ੍ਞਾ( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
  }
}

/*
** Close all cursors in the current frame.
*/
ਸਥਿਰ ਖਾਲੀ close_Cursors_ਵਿੱਚ_ਚੌਖਟਾ(Vdbe *p){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    ਜੇ( pC ){
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
ਪੂਰਨ sqlite3_Vdbe_ਚੌਖਟਾ_Restore(Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ){
  Vdbe *v = p_ਚੌਖਟਾ->v;
  close_Cursors_ਵਿੱਚ_ਚੌਖਟਾ(v);
  v->aOp = p_ਚੌਖਟਾ->aOp;
  v->nOp = p_ਚੌਖਟਾ->nOp;
  v->aMem = p_ਚੌਖਟਾ->aMem;
  v->nMem = p_ਚੌਖਟਾ->nMem;
  v->apCsr = p_ਚੌਖਟਾ->apCsr;
  v->nCursor = p_ਚੌਖਟਾ->nCursor;
  v->db->ਆਖਰੀ_Rowid = p_ਚੌਖਟਾ->ਆਖਰੀ_Rowid;
  v->nChange = p_ਚੌਖਟਾ->nChange;
  v->db->nChange = p_ਚੌਖਟਾ->nDbChange;
  sqlite3_Vdbe_Delete_Aux_ਅੰਕੜੇ(v->db, &v->p_Aux_ਅੰਕੜੇ, -1, 0);
  v->p_Aux_ਅੰਕੜੇ = p_ਚੌਖਟਾ->p_Aux_ਅੰਕੜੇ;
  p_ਚੌਖਟਾ->p_Aux_ਅੰਕੜੇ = 0;
  ਵਾਪਸ p_ਚੌਖਟਾ->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
ਸਥਿਰ ਖਾਲੀ closeAllCursors(Vdbe *p){
  ਜੇ( p->p_ਚੌਖਟਾ ){
    Vdbe_ਚੌਖਟਾ *p_ਚੌਖਟਾ;
    ਲਈ(p_ਚੌਖਟਾ=p->p_ਚੌਖਟਾ; p_ਚੌਖਟਾ->p_ਮਾਪਾ; p_ਚੌਖਟਾ=p_ਚੌਖਟਾ->p_ਮਾਪਾ);
    sqlite3_Vdbe_ਚੌਖਟਾ_Restore(p_ਚੌਖਟਾ);
    p->p_ਚੌਖਟਾ = 0;
    p->n_ਚੌਖਟਾ = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਚੌਖਟਾ==0 );
  close_Cursors_ਵਿੱਚ_ਚੌਖਟਾ(p);
  releaseMemArray(p->aMem, p->nMem);
  ਜਦੋਂ( p->p_Del_ਚੌਖਟਾ ){
    Vdbe_ਚੌਖਟਾ *pDel = p->p_Del_ਚੌਖਟਾ;
    p->p_Del_ਚੌਖਟਾ = pDel->p_ਮਾਪਾ;
    sqlite3_Vdbe_ਚੌਖਟਾ_Delete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  ਜੇ( p->p_Aux_ਅੰਕੜੇ ) sqlite3_Vdbe_Delete_Aux_ਅੰਕੜੇ(p->db, &p->p_Aux_ਅੰਕੜੇ, -1, 0);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_Aux_ਅੰਕੜੇ==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_ਅੰਕ_ਖਾਨੇ_ਗਿਣਤੀ(Vdbe *p, ਪੂਰਨ n_Res_ਕਾਲਮ){
  ਪੂਰਨ n;
  sqlite3 *db = p->db;

  ਜੇ( p->nResAlloc ){
    releaseMemArray(p->a_ਖਾਨਾ_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->a_ਖਾਨਾ_Name);
  }
  n = n_Res_ਕਾਲਮ*COLNAME_N;
  p->n_Res_ਕਾਲਮ = p->nResAlloc = (u16)n_Res_ਕਾਲਮ;
  p->a_ਖਾਨਾ_Name = (Mem*)sqlite3DbMallocRawNN(db, ਆਕਾਰ(Mem)*n );
  ਜੇ( p->a_ਖਾਨਾ_Name==0 ) ਵਾਪਸ;
  initMemArray(p->a_ਖਾਨਾ_Name, n, db, MEM_Null);
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
ਪੂਰਨ sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(
  Vdbe *p,                         /* Vdbe being configured */
  ਪੂਰਨ idx,                         /* Index of column zName applies to */
  ਪੂਰਨ var,                         /* One of the COLNAME_* constants */
  ਅਚਲ ਅੱਖਰ *zName,               /* Pointer to buffer containing name */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*)              /* Memory management strategy for zName */
){
  ਪੂਰਨ rc;
  Mem *p_ਖਾਨਾ_Name;
  ਪ੍ਰਤਿਜ੍ਞਾ( idx<p->nResAlloc );
  ਪ੍ਰਤਿਜ੍ਞਾ( var<COLNAME_N );
  ਜੇ( p->db->mallocFailed ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !zName || xDel!=SQLITE_DYNAMIC );
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->a_ਖਾਨਾ_Name!=0 );
  p_ਖਾਨਾ_Name = &(p->a_ਖਾਨਾ_Name[idx+var*p->nResAlloc]);
  rc = sqlite3_Vdbe_Mem_ਸੈੱਟ_ਪਾਠ(p_ਖਾਨਾ_Name, zName, -1, xDel);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=0 || !zName || (p_ਖਾਨਾ_Name->flags&MEM_ਪਦ)!=0 );
  ਵਾਪਸ rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
ਸਥਿਰ ਪੂਰਨ vdbeCommit(sqlite3 *db, Vdbe *p){
  ਪੂਰਨ i;
  ਪੂਰਨ nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ needXcommit = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  ਅਣਵਰਤਿਆ_PARAMETER(p);
#ਜੇ_ਅੰਤ

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
  ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      ਸਥਿਰ ਅਚਲ u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /*ਸਮ੍ਰਿਤੀ*/ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      ਜੇ( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3_Pager_Get_Journal_ਬਹੁਲਕ(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( i!=1 );
        nTrans++;
      }
      rc = sqlite3_Pager_Exclusive_ਤਾਲਾ(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  ਜੇ( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    ਜੇ( rc ){
      ਵਾਪਸ SQLITE_CONSTRAINT_COMMITHOOK;
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
  ਜੇ( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    ਜੇ( needXcommit ){
      ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ਜੇ( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ਪੂਰਨ txn = sqlite3BtreeTxnState(pBt);
      ਜੇ( txn!=SQLITE_TXN_NONE ){
        ਪ੍ਰਤਿਜ੍ਞਾ( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DISKIO
  ਨਹੀਂਤਾਂ{
    sqlite3_vfs *pVfs = db->pVfs;
    ਅੱਖਰ *zSuper = 0;   /* File-name for the super-journal */
    ਅੱਖਰ ਅਚਲ *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 ਵਿੱਥ = 0;
    ਪੂਰਨ res;
    ਪੂਰਨ retry_ਗਿਣਤੀ = 0;
    ਪੂਰਨ nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    ਜੇ( zSuper==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    zSuper += 4;
    ਕਰੋ {
      u32 iRandom;
      ਜੇ( retry_ਗਿਣਤੀ ){
        ਜੇ( retry_ਗਿਣਤੀ>100 ){
          sqlite3_log(SQLITE_ਭਰਿਆ, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( retry_ਗਿਣਤੀ==1 ){
          sqlite3_log(SQLITE_ਭਰਿਆ, "MJ collide: %s", zSuper);
        }
      }
      retry_ਗਿਣਤੀ++;
      sqlite3_randomness(ਆਕਾਰ(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      ਪ੍ਰਤਿਜ੍ਞਾ( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }ਜਦੋਂ( rc==SQLITE_OK && res );
    ਜੇ( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      ਵਾਪਸ rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    ਲਈ(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ਜੇ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        ਅੱਖਰ ਅਚਲ *zFile = sqlite3BtreeGetJournalname(pBt);
        ਜੇ( zFile==0 ){
          ਜਾਰੀ;  /* Ignore TEMP and :memory: databases */
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,ਵਿੱਥ);
        ਵਿੱਥ += sqlite3Strlen30(zFile)+1;
        ਜੇ( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          ਵਾਪਸ rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    ਜੇ( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_ਸਾਧਾਰਨ))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      ਵਾਪਸ rc;
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
    ਲਈ(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ਜੇ( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_BUSY );
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      ਵਾਪਸ rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    ਜੇ( rc ){
      ਵਾਪਸ rc;
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
    ਲਈ(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ਜੇ( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3_ਸਿਰਾ_Benign_Malloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ rc;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਸਥਿਰ ਖਾਲੀ check_ਸਰਗਰਮ_Vdbe_Cnt(sqlite3 *db){
  Vdbe *p;
  ਪੂਰਨ cnt = 0;
  ਪੂਰਨ nWrite = 0;
  ਪੂਰਨ nRead = 0;
  p = db->pVdbe;
  ਜਦੋਂ( p ){
    ਜੇ( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      ਜੇ( p->readOnly==0 ) nWrite++;
      ਜੇ( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( cnt==db->n_Vdbe_ਸਰਗਰਮ );
  ਪ੍ਰਤਿਜ੍ਞਾ( nWrite==db->nVdbeWrite );
  ਪ੍ਰਤਿਜ੍ਞਾ( nRead==db->nVdbeRead );
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_ਸਰਗਰਮ_Vdbe_Cnt(x)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ vdbeCloseStatement(Vdbe *p, ਪੂਰਨ eOp){
  sqlite3 *ਅਚਲ db = p->db;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i;
  ਅਚਲ ਪੂਰਨ iSavepoint = p->iStatement-1;

  ਪ੍ਰਤਿਜ੍ਞਾ( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nStatement>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iStatement==(db->nStatement+db->nSavepoint) );

  ਲਈ(i=0; i<db->nDb; i++){
    ਪੂਰਨ rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    ਜੇ( pBt ){
      ਜੇ( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      ਜੇ( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    ਜੇ( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  ਜੇ( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3VdbeCloseStatement(Vdbe *p, ਪੂਰਨ eOp){
  ਜੇ( p->db->nStatement && p->iStatement ){
    ਵਾਪਸ vdbeCloseStatement(p, eOp);
  }
  ਵਾਪਸ SQLITE_OK;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  ਜੇ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) ਵਾਪਸ SQLITE_ERROR;
  ਵਾਪਸ SQLITE_CONSTRAINT_FOREIGNKEY;
}
ਪੂਰਨ sqlite3_Vdbe_Check_Fk_ਤੁਰੰਤ(Vdbe *p){
  ਜੇ( p->nFkConstraint==0 ) ਵਾਪਸ SQLITE_OK;
  ਵਾਪਸ vdbeFkError(p);
}
ਪੂਰਨ sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  ਜੇ( (db->nDeferredCons+db->nDeferredImmCons)==0 ) ਵਾਪਸ SQLITE_OK;
  ਵਾਪਸ vdbeFkError(p);
}
#ਜੇ_ਅੰਤ

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
ਪੂਰਨ sqlite3VdbeHalt(Vdbe *p){
  ਪੂਰਨ rc;                         /* Used to store transient return codes */
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

  ਪ੍ਰਤਿਜ੍ਞਾ( p->eVdbeState==VDBE_ਲੜੀਵਾਰ_STATE );
  ਜੇ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  check_ਸਰਗਰਮ_Vdbe_Cnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  ਜੇ( p->bIsReader ){
    ਪੂਰਨ mrc;   /* Primary error code from p->rc */
    ਪੂਰਨ eStatementOp = 0;
    ਪੂਰਨ isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    ਜੇ( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_ਰੁਕਾਵਟ
                    || mrc==SQLITE_ਭਰਿਆ;
    }ਨਹੀਂਤਾਂ{
      mrc = isSpecialError = 0;
    }
    ਜੇ( isSpecialError ){
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
      ਜੇ( !p->readOnly || mrc!=SQLITE_ਰੁਕਾਵਟ ){
        ਜੇ( (mrc==SQLITE_NOMEM || mrc==SQLITE_ਭਰਿਆ) && p->ਵਰਤਦਾ_Stmt_Journal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }ਨਹੀਂਤਾਂ{
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
    ਜੇ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (ਖਾਲੀ)sqlite3_Vdbe_Check_Fk_ਤੁਰੰਤ(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    ਜੇ( !sqlite3_Vtab_ਵਿੱਚ_Sync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      ਜੇ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        ਜੇ( rc!=SQLITE_OK ){
          ਜੇ( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            ਵਾਪਸ SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }ਨਹੀਂਤਾਂ ਜੇ( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }ਨਹੀਂਤਾਂ{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        ਜੇ( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          ਵਾਪਸ SQLITE_BUSY;
        }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }ਨਹੀਂਤਾਂ{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3_Commit_ਅੰਦਰੂਨੀ_Changes(db);
        }
      }ਨਹੀਂਤਾਂ ਜੇ( p->rc==SQLITE_SCHEMA && db->n_Vdbe_ਸਰਗਰਮ>1 ){
        p->nChange = 0;
      }ਨਹੀਂਤਾਂ{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( eStatementOp==0 ){
      ਜੇ( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }ਨਹੀਂਤਾਂ ਜੇ( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }ਨਹੀਂਤਾਂ{
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
    ਜੇ( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      ਜੇ( rc ){
        ਜੇ( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_ਸੁਨੇਹਾ);
          p->z_Err_ਸੁਨੇਹਾ = 0;
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
    ਜੇ( p->changeCntOn ){
      ਜੇ( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3_Vdbe_ਸੈੱਟ_Changes(db, p->nChange);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਸੈੱਟ_Changes(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->n_Vdbe_ਸਰਗਰਮ--;
  ਜੇ( !p->readOnly ) db->nVdbeWrite--;
  ਜੇ( p->bIsReader ) db->nVdbeRead--;
  ਪ੍ਰਤਿਜ੍ਞਾ( db->n_Vdbe_ਸਰਗਰਮ>=db->nVdbeRead );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nVdbeRead>=db->nVdbeWrite );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  check_ਸਰਗਰਮ_Vdbe_Cnt(db);
  ਜੇ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  ਜੇ( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( db->n_Vdbe_ਸਰਗਰਮ>0 || db->autoCommit==0 || db->nStatement==0 );
  ਵਾਪਸ (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
ਖਾਲੀ sqlite3_Vdbe_Reset_ਕਦਮ_ਨਤੀਜਾ(Vdbe *p){
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
ਪੂਰਨ sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  ਪੂਰਨ rc = p->rc;
  ਜੇ( p->z_Err_ਸੁਨੇਹਾ ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    ਜੇ( db->pErr==0 ) db->pErr = sqlite3_ਮੁੱਲ_New(db);
    sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(db->pErr, -1, p->z_Err_ਸੁਨੇਹਾ, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3_ਸਿਰਾ_Benign_Malloc();
    db->bBenignMalloc--;
  }ਨਹੀਂਤਾਂ ਜੇ( db->pErr ){
    sqlite3_ਮੁੱਲ_ਸੈੱਟ_Null(db->pErr);
  }
  db->err_ਕੋਡ = rc;
  db->err_Byte_ਵਿੱਥ = -1;
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
ਸਥਿਰ ਖਾਲੀ vdbeInvokeSqllog(Vdbe *v){
  ਜੇ( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    ਅੱਖਰ *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    ਪ੍ਰਤਿਜ੍ਞਾ( v->db->init.busy==0 );
    ਜੇ( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#ਨਹੀਂਤਾਂ
# define vdbeInvokeSqllog(x)
#ਜੇ_ਅੰਤ

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
ਪੂਰਨ sqlite3VdbeReset(Vdbe *p){
#ਜੇ defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  ਪੂਰਨ i;
#ਜੇ_ਅੰਤ

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  ਜੇ( p->eVdbeState==VDBE_ਲੜੀਵਾਰ_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  ਜੇ( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    ਜੇ( db->pErr || p->z_Err_ਸੁਨੇਹਾ ){
      sqlite3VdbeTransferError(p);
    }ਨਹੀਂਤਾਂ{
      db->err_ਕੋਡ = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  ਜੇ( p->apCsr ) ਲਈ(i=0; i<p->nCursor; i++) ਪ੍ਰਤਿਜ੍ਞਾ( p->apCsr[i]==0 );
  ਜੇ( p->aMem ){
    ਲਈ(i=0; i<p->nMem; i++) ਪ੍ਰਤਿਜ੍ਞਾ( p->aMem[i].flags==MEM_Undefined );
  }
#ਜੇ_ਅੰਤ
  ਜੇ( p->z_Err_ਸੁਨੇਹਾ ){
    sqlite3DbFree(db, p->z_Err_ਸੁਨੇਹਾ);
    p->z_Err_ਸੁਨੇਹਾ = 0;
  }
  p->p_ਨਤੀਜਾ_ਪੰਕਤੀ = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  p->nWrite = 0;
#ਜੇ_ਅੰਤ

  /* Save profiling information from this VDBE run.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VDBE_PROFILE
  {
    FILE *ਬਾਹਰਲਾ = fopen("vdbe_profile.out", "a");
    ਜੇ( ਬਾਹਰਲਾ ){
      fprintf(ਬਾਹਰਲਾ, "---- ");
      ਲਈ(i=0; i<p->nOp; i++){
        fprintf(ਬਾਹਰਲਾ, "%02x", p->aOp[i].opcode);
      }
      fprintf(ਬਾਹਰਲਾ, "\n");
      ਜੇ( p->zSql ){
        ਅੱਖਰ c, pc = 0;
        fprintf(ਬਾਹਰਲਾ, "-- ");
        ਲਈ(i=0; (c = p->zSql[i])!=0; i++){
          ਜੇ( pc=='\n' ) fprintf(ਬਾਹਰਲਾ, "-- ");
          putc(c, ਬਾਹਰਲਾ);
          pc = c;
        }
        ਜੇ( pc!='\n' ) fprintf(ਬਾਹਰਲਾ, "\n");
      }
      ਲਈ(i=0; i<p->nOp; i++){
        ਅੱਖਰ zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 ਚੱਕਰਾਂ = p->aOp[i].n_ਚੱਕਰ;
        sqlite3_snprintf(ਆਕਾਰ(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           ਚੱਕਰਾਂ,
           cnt>0 ? ਚੱਕਰਾਂ/cnt : 0
        );
        fprintf(ਬਾਹਰਲਾ, "%s", zHdr);
        sqlite3VdbePrintOp(ਬਾਹਰਲਾ, i, &p->aOp[i]);
      }
      fclose(ਬਾਹਰਲਾ);
    }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ p->rc & db->err_ਨਕਾਬ;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
ਪੂਰਨ sqlite3VdbeFinalize(Vdbe *p){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪ੍ਰਤਿਜ੍ਞਾ( VDBE_ਲੜੀਵਾਰ_STATE>VDBE_READY_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( VDBE_HALT_STATE>VDBE_READY_STATE );
  ਪ੍ਰਤਿਜ੍ਞਾ( VDBE_INIT_STATE<VDBE_READY_STATE );
  ਜੇ( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    ਪ੍ਰਤਿਜ੍ਞਾ( (rc & p->db->err_ਨਕਾਬ)==rc );
  }
  sqlite3VdbeDelete(p);
  ਵਾਪਸ rc;
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
ਖਾਲੀ sqlite3_Vdbe_Delete_Aux_ਅੰਕੜੇ(sqlite3 *db, Aux_ਅੰਕੜੇ **pp, ਪੂਰਨ iOp, ਪੂਰਨ ਨਕਾਬ){
  ਜਦੋਂ( *pp ){
    Aux_ਅੰਕੜੇ *pAux = *pp;
    ਜੇ( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(ਨਕਾਬ & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      ਜੇ( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_ਅਗਲਾ_Aux;
      sqlite3DbFree(db, pAux);
    }ਨਹੀਂਤਾਂ{
      pp= &pAux->p_ਅਗਲਾ_Aux;
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
ਸਥਿਰ ਖਾਲੀ sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  Sub_ਪ੍ਰੋਗਰਾਮ *pSub, *p_ਅਗਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db==0 || p->db==db );
  ਜੇ( p->a_ਖਾਨਾ_Name ){
    releaseMemArray(p->a_ਖਾਨਾ_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->a_ਖਾਨਾ_Name);
  }
  ਲਈ(pSub=p->p_ਪ੍ਰੋਗਰਾਮ; pSub; pSub=p_ਅਗਲਾ){
    p_ਅਗਲਾ = pSub->p_ਅਗਲਾ;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  ਜੇ( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    ਜੇ( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    ਜੇ( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  ਜੇ( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    ਲਈ(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_ਅਗਲਾ_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  {
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->n_ਜਾਂਚ; i++){
      sqlite3DbFree(db, p->a_ਜਾਂਚ[i].zName);
    }
    sqlite3DbFree(db, p->a_ਜਾਂਚ);
  }
#ਜੇ_ਅੰਤ
}

/*
** Delete an entire VDBE.
*/
ਖਾਲੀ sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  db = p->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    ਜੇ( p->pVNext ){
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
ਪੂਰਨ SQLITE_NOINLINE sqlite3_Vdbe_ਅੰਤ_Moveto(VdbeCursor *p){
  ਪੂਰਨ res, rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_ਖੋਜ_ਗਿਣਤੀ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p->deferredMoveto );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->is_ਸਾਰਣੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3_Btree_ਸਾਰਣੀ_Moveto(p->uc.pCursor, p->moveto_ਨਿਸ਼ਾਨਾ, 0, &res);
  ਜੇ( rc ) ਵਾਪਸ rc;
  ਜੇ( res!=0 ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  sqlite3_ਖੋਜ_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
  p->deferredMoveto = 0;
  p->cache_ਸਥਿਤੀ = CACHE_STALE;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
ਪੂਰਨ SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  ਪੂਰਨ is_Different_ਪੰਕਤੀ, rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eCurType==CURTYPE_BTREE );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->uc.pCursor!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &is_Different_ਪੰਕਤੀ);
  p->cache_ਸਥਿਤੀ = CACHE_STALE;
  ਜੇ( is_Different_ਪੰਕਤੀ ) p->null_ਪੰਕਤੀ = 1;
  ਵਾਪਸ rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
ਪੂਰਨ sqlite3VdbeCursorRestore(VdbeCursor *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  ਜੇ( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    ਵਾਪਸ sqlite3VdbeHandleMovedCursor(p);
  }
  ਵਾਪਸ SQLITE_OK;
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

#ਜੇ 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, ਪੂਰਨ file_format, u32 *pLen){
  ਪੂਰਨ flags = pMem->flags;
  u32 n;

  ਪ੍ਰਤਿਜ੍ਞਾ( pLen!=0 );
  ਜੇ( flags&MEM_Null ){
    *pLen = 0;
    ਵਾਪਸ 0;
  }
  ਜੇ( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    ਜੇ( i<0 ){
      u = ~i;
    }ਨਹੀਂਤਾਂ{
      u = i;
    }
    ਜੇ( u<=127 ){
      ਜੇ( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        ਵਾਪਸ 8+(u32)u;
      }ਨਹੀਂਤਾਂ{
        *pLen = 1;
        ਵਾਪਸ 1;
      }
    }
    ਜੇ( u<=32767 ){ *pLen = 2; ਵਾਪਸ 2; }
    ਜੇ( u<=8388607 ){ *pLen = 3; ਵਾਪਸ 3; }
    ਜੇ( u<=2147483647 ){ *pLen = 4; ਵਾਪਸ 4; }
    ਜੇ( u<=MAX_6BYTE ){ *pLen = 6; ਵਾਪਸ 5; }
    *pLen = 8;
    ਜੇ( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (ਦੁੱਗਣਾ)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      ਵਾਪਸ 7;
    }
    ਵਾਪਸ 6;
  }
  ਜੇ( flags&MEM_Real ){
    *pLen = 8;
    ਵਾਪਸ 7;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->n>=0 );
  n = (u32)pMem->n;
  ਜੇ( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  ਵਾਪਸ ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#ਜੇ_ਅੰਤ /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
ਅਚਲ u8 sqlite3SmallTypeSizes[128] = {
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
  ਜੇ( serial_type>=128 ){
    ਵਾਪਸ (serial_type-12)/2;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    ਵਾਪਸ sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  ਪ੍ਰਤਿਜ੍ਞਾ( serial_type<128 );
  ਵਾਪਸ sqlite3SmallTypeSizes[serial_type]; 
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MIXED_ENDIAN_64_ਦੋਆਧਾਰੀ_ਅੰਕ_FLOAT
u64 sqlite3_Float_ਅਦਲਾ_ਬਦਲੀ(u64 ਵਿੱਚ){
  ਸੰਘ {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = ਵਿੱਚ;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  ਵਾਪਸ u.r;
}
#ਜੇ_ਅੰਤ /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3VdbeSerialGet7(
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(x)==8 && ਆਕਾਰ(pMem->u.r)==8 );
  ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(x);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pMem->u.r, &x, ਆਕਾਰ(x));
  ਜੇ( IsNaN(x) ){
    pMem->flags = MEM_Null;
    ਵਾਪਸ 1;
  }
  pMem->flags = MEM_Real;
  ਵਾਪਸ 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
ਖਾਲੀ sqlite3VdbeSerialGet(
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  ਚੋਣ( serial_type ){
    ਹਾਲਤ 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      ਵਾਪਸ;
    }
    ਹਾਲਤ 11:   /* Reserved for future use */
    ਹਾਲਤ 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      ਵਾਪਸ;
    }
    ਹਾਲਤ 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      ਜੇ( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#ਜੇ_ਅੰਤ
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      ਵਾਪਸ;
    }
    ਹਾਲਤ 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      ਵਾਪਸ;
    }
    ਹਾਲਤ 8:    /* Integer 0 */
    ਹਾਲਤ 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      ਵਾਪਸ;
    }
    ਮੂਲ: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      ਸਥਿਰ ਅਚਲ u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (ਅੱਖਰ *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      ਵਾਪਸ;
    }
  }
  ਵਾਪਸ;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
Unpacked_ਰਿਕਾਰਡ *sqlite3_Vdbe_Alloc_Unpacked_ਰਿਕਾਰਡ(
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info               /* Description of the record */
){
  Unpacked_ਰਿਕਾਰਡ *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(Unpacked_ਰਿਕਾਰਡ) + ਆਕਾਰ(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(ਆਕਾਰ(Unpacked_ਰਿਕਾਰਡ)) + ਆਕਾਰ(Mem)*(p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ+1);
  p = (Unpacked_ਰਿਕਾਰਡ *)sqlite3DbMallocRaw(p_ਕੁੰਜੀ_Info->db, nByte);
  ਜੇ( !p ) ਵਾਪਸ 0;
  p->aMem = (Mem*)&((ਅੱਖਰ*)p)[ROUND8P(ਆਕਾਰ(Unpacked_ਰਿਕਾਰਡ))];
  p->p_ਕੁੰਜੀ_Info = p_ਕੁੰਜੀ_Info;
  p->n_ਖੇਤਰ = p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ + 1;
  ਵਾਪਸ p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
ਖਾਲੀ sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack(
  ਪੂਰਨ n_ਕੁੰਜੀ,              /* Size of the binary record */
  ਅਚਲ ਖਾਲੀ *p_ਕੁੰਜੀ,      /* The binary record */
  Unpacked_ਰਿਕਾਰਡ *p      /* Populate this structure before returning. */
){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਕੁੰਜੀ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)p_ਕੁੰਜੀ;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = p->p_ਕੁੰਜੀ_Info;

  p->default_rc = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_ਕੁੰਜੀ, szHdr);
  d = szHdr;
  u = 0;
  ਜਦੋਂ( idx<szHdr && d<=(u32)n_ਕੁੰਜੀ ){
    u32 serial_type;

    idx += getVarint32(&a_ਕੁੰਜੀ[idx], serial_type);
    pMem->enc = p_ਕੁੰਜੀ_Info->enc;
    pMem->db = p_ਕੁੰਜੀ_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_ਕੁੰਜੀ[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    ਜੇ( (++u)>=p->n_ਖੇਤਰ ) ਰੋਕੋ;
    pMem++;
  }
  ਜੇ( d>(u32)n_ਕੁੰਜੀ && u ){
    ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3_Vdbe_Mem_ਸੈੱਟ_Null(pMem-(u<p->n_ਖੇਤਰ));
  }
  testcase( u == p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ + 1 );
  testcase( u < p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ + 1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( u<=p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ + 1 );
  p->n_ਖੇਤਰ = u;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਪੂਰਨ vdbe_ਰਿਕਾਰਡ_Compare_Debug(
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1, /* Left key */
  ਅਚਲ Unpacked_ਰਿਕਾਰਡ *pPKey2, /* Right key */
  ਪੂਰਨ desired_ਨਤੀਜਾ             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  ਪੂਰਨ i = 0;
  ਪੂਰਨ rc = 0;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *aKey1 = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)pKey1;
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
  Mem mem1;

  p_ਕੁੰਜੀ_Info = pPKey2->p_ਕੁੰਜੀ_Info;
  ਜੇ( p_ਕੁੰਜੀ_Info->db==0 ) ਵਾਪਸ 1;
  mem1.enc = p_ਕੁੰਜੀ_Info->enc;
  mem1.db = p_ਕੁੰਜੀ_Info->db;
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
  ਜੇ( szHdr1>98307 ) ਵਾਪਸ SQLITE_CORRUPT;
  d1 = szHdr1;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ>=pPKey2->n_ਖੇਤਰ || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( idx1<=szHdr1 || CORRUPT_DB );
  ਕਰੋ{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    ਜੇ( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      ਜੇ( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        ਵਾਪਸ 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      ਰੋਕੋ;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ>i ? p_ਕੁੰਜੀ_Info->aColl[i] : 0);
    ਜੇ( rc!=0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( mem1.szMalloc==0 );  /* See comment below */
      ਜੇ( (p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[i] & KEYINFO_ਕ੍ਰਮ_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      ਜੇ( p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[i] & KEYINFO_ਕ੍ਰਮ_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      ਜਾਓ debug_Compare_ਸਿਰਾ;
    }
    i++;
  }ਜਦੋਂ( idx1<szHdr1 && i<pPKey2->n_ਖੇਤਰ );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debug_Compare_ਸਿਰਾ:
  ਜੇ( desired_ਨਤੀਜਾ==0 && rc==0 ) ਵਾਪਸ 1;
  ਜੇ( desired_ਨਤੀਜਾ<0 && rc<0 ) ਵਾਪਸ 1;
  ਜੇ( desired_ਨਤੀਜਾ>0 && rc>0 ) ਵਾਪਸ 1;
  ਜੇ( CORRUPT_DB ) ਵਾਪਸ 1;
  ਜੇ( p_ਕੁੰਜੀ_Info->db->mallocFailed ) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਖਾਲੀ vdbe_Assert_ਖੇਤਰ_ਗਿਣਤੀ_Within_Limits(
  ਪੂਰਨ n_ਕੁੰਜੀ, ਅਚਲ ਖਾਲੀ *p_ਕੁੰਜੀ,   /* The record to verify */
  ਅਚਲ ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info       /* Compare size with this KeyInfo */
){
  ਪੂਰਨ n_ਖੇਤਰ = 0;
  u32 szHdr;
  u32 idx;
  u32 not_ਵਰਤਿਆ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਕੁੰਜੀ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)p_ਕੁੰਜੀ;

  ਜੇ( CORRUPT_DB ) ਵਾਪਸ;
  idx = getVarint32(a_ਕੁੰਜੀ, szHdr);
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( szHdr<=(u32)n_ਕੁੰਜੀ );
  ਜਦੋਂ( idx<szHdr ){
    idx += getVarint32(a_ਕੁੰਜੀ+idx, not_ਵਰਤਿਆ);
    n_ਖੇਤਰ++;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖੇਤਰ <= p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ );
}
#ਨਹੀਂਤਾਂ
# define vdbe_Assert_ਖੇਤਰ_ਗਿਣਤੀ_Within_Limits(A,B,C)
#ਜੇ_ਅੰਤ

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ vdbeCompareMemStringWithEncodingChange(
  ਅਚਲ Mem *pMem1,
  ਅਚਲ Mem *pMem2,
  ਅਚਲ CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  ਪੂਰਨ rc;
  ਅਚਲ ਖਾਲੀ *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_ਮੁੱਲ_ਪਾਠ_2((sqlite3_ਮੁੱਲ*)&c1, pColl->enc);
  v2 = sqlite3_ਮੁੱਲ_ਪਾਠ_2((sqlite3_ਮੁੱਲ*)&c2, pColl->enc);
  ਜੇ( (v1==0 || v2==0) ){
    ਜੇ( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }ਨਹੀਂਤਾਂ{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਪੂਰਨ vdbeCompareMemString(
  ਅਚਲ Mem *pMem1,
  ਅਚਲ Mem *pMem2,
  ਅਚਲ CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  ਜੇ( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    ਵਾਪਸ pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
ਸਥਿਰ ਪੂਰਨ isAllZero(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<n; i++){
    ਜੇ( z[i] ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE ਪੂਰਨ sqlite3BlobCompare(ਅਚਲ Mem *pB1, ਅਚਲ Mem *pB2){
  ਪੂਰਨ c;
  ਪੂਰਨ n1 = pB1->n;
  ਪੂਰਨ n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  ਪ੍ਰਤਿਜ੍ਞਾ( (pB1->flags & MEM_Zero)==0 || n1==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pB2->flags & MEM_Zero)==0 || n2==0 );

  ਜੇ( (pB1->flags|pB2->flags) & MEM_Zero ){
    ਜੇ( pB1->flags & pB2->flags & MEM_Zero ){
      ਵਾਪਸ pB1->u.nZero - pB2->u.nZero;
    }ਨਹੀਂਤਾਂ ਜੇ( pB1->flags & MEM_Zero ){
      ਜੇ( !isAllZero(pB2->z, pB2->n) ) ਵਾਪਸ -1;
      ਵਾਪਸ pB1->u.nZero - n2;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( !isAllZero(pB1->z, pB1->n) ) ਵਾਪਸ +1;
      ਵਾਪਸ n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  ਜੇ( c ) ਵਾਪਸ c;
  ਵਾਪਸ n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#ਜੇ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
ਸਥਿਰ ਪੂਰਨ SQLITE_NOINLINE doubleLt(ਦੁੱਗਣਾ a, ਦੁੱਗਣਾ b){ ਵਾਪਸ a<b; }
ਸਥਿਰ ਪੂਰਨ SQLITE_NOINLINE doubleEq(ਦੁੱਗਣਾ a, ਦੁੱਗਣਾ b){ ਵਾਪਸ a==b; }
#ਜੇ_ਅੰਤ

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
ਪੂਰਨ sqlite3IntFloatCompare(i64 i, ਦੁੱਗਣਾ r){
  ਜੇ( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    i64 y;
    ਜੇ( r<-9223372036854775808.0 ) ਵਾਪਸ +1;
    ਜੇ( r>=9223372036854775808.0 ) ਵਾਪਸ -1;
    y = (i64)r;
    ਜੇ( i<y ) ਵਾਪਸ -1;
    ਜੇ( i>y ) ਵਾਪਸ +1;
    testcase( doubleLt(((ਦੁੱਗਣਾ)i),r) );
    testcase( doubleLt(r,((ਦੁੱਗਣਾ)i)) );
    testcase( doubleEq(r,((ਦੁੱਗਣਾ)i)) );
    ਵਾਪਸ (((ਦੁੱਗਣਾ)i)<r) ? -1 : (((ਦੁੱਗਣਾ)i)>r);
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
ਪੂਰਨ sqlite3MemCompare(ਅਚਲ Mem *pMem1, ਅਚਲ Mem *pMem2, ਅਚਲ CollSeq *pColl){
  ਪੂਰਨ f1, f2;
  ਪੂਰਨ combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  ਪ੍ਰਤਿਜ੍ਞਾ( !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem1) && !sqlite3_Vdbe_Mem_Is_ਪੰਕਤੀ_ਸੈੱਟ(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  ਜੇ( combined_flags&MEM_Null ){
    ਵਾਪਸ (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  ਜੇ( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    ਜੇ( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      ਜੇ( pMem1->u.i < pMem2->u.i ) ਵਾਪਸ -1;
      ਜੇ( pMem1->u.i > pMem2->u.i ) ਵਾਪਸ +1;
      ਵਾਪਸ 0;
    }
    ਜੇ( (f1 & f2 & MEM_Real)!=0 ){
      ਜੇ( pMem1->u.r < pMem2->u.r ) ਵਾਪਸ -1;
      ਜੇ( pMem1->u.r > pMem2->u.r ) ਵਾਪਸ +1;
      ਵਾਪਸ 0;
    }
    ਜੇ( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      ਜੇ( (f2&MEM_Real)!=0 ){
        ਵਾਪਸ sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }ਨਹੀਂਤਾਂ ਜੇ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        ਜੇ( pMem1->u.i < pMem2->u.i ) ਵਾਪਸ -1;
        ਜੇ( pMem1->u.i > pMem2->u.i ) ਵਾਪਸ +1;
        ਵਾਪਸ 0;
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ -1;
      }
    }
    ਜੇ( (f1&MEM_Real)!=0 ){
      ਜੇ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        ਵਾਪਸ -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ -1;
      }
    }
    ਵਾਪਸ +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  ਜੇ( combined_flags&MEM_Str ){
    ਜੇ( (f1 & MEM_Str)==0 ){
      ਵਾਪਸ 1;
    }
    ਜੇ( (f2 & MEM_Str)==0 ){
      ਵਾਪਸ -1;
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    ਪ੍ਰਤਿਜ੍ਞਾ( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( !pColl || pColl->xCmp );

    ਜੇ( pColl ){
      ਵਾਪਸ vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  ਵਾਪਸ sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
ਸਥਿਰ i64 vdbe_ਰਿਕਾਰਡ_Decode_Int(u32 serial_type, ਅਚਲ u8 *a_ਕੁੰਜੀ){
  u32 y;
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  ਚੋਣ( serial_type ){
    ਹਾਲਤ 0:
    ਹਾਲਤ 1:
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      ਵਾਪਸ ONE_BYTE_INT(a_ਕੁੰਜੀ);
    ਹਾਲਤ 2:
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      ਵਾਪਸ TWO_BYTE_INT(a_ਕੁੰਜੀ);
    ਹਾਲਤ 3:
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      ਵਾਪਸ THREE_BYTE_INT(a_ਕੁੰਜੀ);
    ਹਾਲਤ 4: {
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      y = FOUR_BYTE_UINT(a_ਕੁੰਜੀ);
      ਵਾਪਸ (i64)*(ਪੂਰਨ*)&y;
    }
    ਹਾਲਤ 5: {
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      ਵਾਪਸ FOUR_BYTE_UINT(a_ਕੁੰਜੀ+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_ਕੁੰਜੀ);
    }
    ਹਾਲਤ 6: {
      u64 x = FOUR_BYTE_UINT(a_ਕੁੰਜੀ);
      testcase( a_ਕੁੰਜੀ[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_ਕੁੰਜੀ+4);
      ਵਾਪਸ (i64)*(i64*)&x;
    }
  }

  ਵਾਪਸ (serial_type - 8);
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
ਪੂਰਨ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1,   /* Left key */
  Unpacked_ਰਿਕਾਰਡ *pPKey2,         /* Right key */
  ਪੂਰਨ bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  ਪੂਰਨ i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  ਪੂਰਨ rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *aKey1 = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  ਜੇ( bSkip ){
    u32 s1 = aKey1[1];
    ਜੇ( s1<0x80 ){
      idx1 = 2;
    }ਨਹੀਂਤਾਂ{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }ਨਹੀਂਤਾਂ{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  ਜੇ( d1>(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 ){
    pPKey2->err_ਕੋਡ = (u8)SQLITE_CORRUPT_BKPT;
    ਵਾਪਸ 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ>=pPKey2->n_ਖੇਤਰ
       || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( idx1<=szHdr1 || CORRUPT_DB );
  ਜਦੋਂ( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    ਜੇ( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      ਜੇ( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }ਨਹੀਂਤਾਂ ਜੇ( serial_type==0 ){
        rc = -1;
      }ਨਹੀਂਤਾਂ ਜੇ( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }ਨਹੀਂਤਾਂ{
        i64 lhs = vdbe_ਰਿਕਾਰਡ_Decode_Int(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        ਜੇ( lhs<rhs ){
          rc = -1;
        }ਨਹੀਂਤਾਂ ਜੇ( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    ਨਹੀਂਤਾਂ ਜੇ( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      ਜੇ( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }ਨਹੀਂਤਾਂ ਜੇ( serial_type==0 ){
        rc = -1;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( serial_type==7 ){
          ਜੇ( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }ਨਹੀਂਤਾਂ ਜੇ( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }ਨਹੀਂਤਾਂ ਜੇ( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }ਨਹੀਂਤਾਂ{
            ਪ੍ਰਤਿਜ੍ਞਾ( rc==0 );
          }
        }ਨਹੀਂਤਾਂ{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    ਨਹੀਂਤਾਂ ਜੇ( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ਜੇ( serial_type<12 ){
        rc = -1;
      }ਨਹੀਂਤਾਂ ਜੇ( !(serial_type & 0x01) ){
        rc = +1;
      }ਨਹੀਂਤਾਂ{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 );
        testcase( (d1+mem1.n+1)==(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 );
        ਜੇ( (d1+mem1.n) > (ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1
         || (p_ਕੁੰਜੀ_Info = pPKey2->p_ਕੁੰਜੀ_Info)->n_All_ਖੇਤਰ<=i
        ){
          pPKey2->err_ਕੋਡ = (u8)SQLITE_CORRUPT_BKPT;
          ਵਾਪਸ 0;                /* Corruption */
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਕੁੰਜੀ_Info->aColl[i] ){
          mem1.enc = p_ਕੁੰਜੀ_Info->enc;
          mem1.db = p_ਕੁੰਜੀ_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (ਅੱਖਰ*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_ਕੁੰਜੀ_Info->aColl[i], &pPKey2->err_ਕੋਡ
          );
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ਜੇ( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    ਨਹੀਂਤਾਂ ਜੇ( pRhs->flags & MEM_Blob ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ਜੇ( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 );
        testcase( (d1+nStr+1)==(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 );
        ਜੇ( (d1+nStr) > (ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 ){
          pPKey2->err_ਕੋਡ = (u8)SQLITE_CORRUPT_BKPT;
          ਵਾਪਸ 0;                /* Corruption */
        }ਨਹੀਂਤਾਂ ਜੇ( pRhs->flags & MEM_Zero ){
          ਜੇ( !isAllZero((ਅਚਲ ਅੱਖਰ*)&aKey1[d1],nStr) ){
            rc = 1;
          }ਨਹੀਂਤਾਂ{
            rc = nStr - pRhs->u.nZero;
          }
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ਜੇ( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    ਨਹੀਂਤਾਂ{
      serial_type = aKey1[idx1];
      ਜੇ( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        ਪ੍ਰਤਿਜ੍ਞਾ( rc==0 );
      }ਨਹੀਂਤਾਂ{
        rc = 1;
      }
    }

    ਜੇ( rc!=0 ){
      ਪੂਰਨ ਛਾਂਟੋ_Flags = pPKey2->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[i];
      ਜੇ( ਛਾਂਟੋ_Flags ){
        ਜੇ( (ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_BIGNULL)==0
         || ((ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( vdbe_ਰਿਕਾਰਡ_Compare_Debug(nKey1, pKey1, pPKey2, rc) );
      ਪ੍ਰਤਿਜ੍ਞਾ( mem1.szMalloc==0 );  /* See comment below */
      ਵਾਪਸ rc;
    }

    i++;
    ਜੇ( i==pPKey2->n_ਖੇਤਰ ) ਰੋਕੋ;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    ਜੇ( d1>(ਬਿਨਾਨਿਸ਼ਾਨੀ)nKey1 ) ਰੋਕੋ;
    idx1 += sqlite3VarintLen(serial_type);
    ਜੇ( idx1>=(ਬਿਨਾਨਿਸ਼ਾਨੀ)szHdr1 ){
      pPKey2->err_ਕੋਡ = (u8)SQLITE_CORRUPT_BKPT;
      ਵਾਪਸ 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  ਪ੍ਰਤਿਜ੍ਞਾ( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB
       || vdbe_ਰਿਕਾਰਡ_Compare_Debug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_ਕੁੰਜੀ_Info->db->mallocFailed
  );
  pPKey2->eq_ਵੇਖਿਆ = 1;
  ਵਾਪਸ pPKey2->default_rc;
}
ਪੂਰਨ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1,   /* Left key */
  Unpacked_ਰਿਕਾਰਡ *pPKey2          /* Right key */
){
  ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(nKey1, pKey1, pPKey2, 0);
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
ਸਥਿਰ ਪੂਰਨ vdbe_ਰਿਕਾਰਡ_Compare_Int(
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1, /* Left key */
  Unpacked_ਰਿਕਾਰਡ *pPKey2        /* Right key */
){
  ਅਚਲ u8 *a_ਕੁੰਜੀ = &((ਅਚਲ u8*)pKey1)[*(ਅਚਲ u8*)pKey1 & 0x3F];
  ਪੂਰਨ serial_type = ((ਅਚਲ u8*)pKey1)[1];
  ਪੂਰਨ res;
  i64 v;
  i64 lhs;

  vdbe_Assert_ਖੇਤਰ_ਗਿਣਤੀ_Within_Limits(nKey1, pKey1, pPKey2->p_ਕੁੰਜੀ_Info);
  ਪ੍ਰਤਿਜ੍ਞਾ( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  ਜੇ( (u32)(serial_type-1)<=5 ){
    ਸਥਿਰ ਅਚਲ u8 a_ਖਿਸਕਾਓ[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_ਕੁੰਜੀ)) >> a_ਖਿਸਕਾਓ[serial_type];
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
    ਪ੍ਰਤਿਜ੍ਞਾ( 0<=(i64)sqlite3Get8byte(a_ਕੁੰਜੀ) || lhs<0 );
    testcase( lhs<0 );
  }ਨਹੀਂਤਾਂ ਜੇ( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(nKey1, pKey1, pPKey2);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  ਜੇ( v>lhs ){
    res = pPKey2->r1;
  }ਨਹੀਂਤਾਂ ਜੇ( v<lhs ){
    res = pPKey2->r2;
  }ਨਹੀਂਤਾਂ ਜੇ( pPKey2->n_ਖੇਤਰ>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
  }ਨਹੀਂਤਾਂ{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eq_ਵੇਖਿਆ = 1;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( vdbe_ਰਿਕਾਰਡ_Compare_Debug(nKey1, pKey1, pPKey2, res) );
  ਵਾਪਸ res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_ਰਿਕਾਰਡ_Compare_String(
  ਪੂਰਨ nKey1, ਅਚਲ ਖਾਲੀ *pKey1, /* Left key */
  Unpacked_ਰਿਕਾਰਡ *pPKey2        /* Right key */
){
  ਅਚਲ u8 *aKey1 = (ਅਚਲ u8*)pKey1;
  ਪੂਰਨ serial_type;
  ਪੂਰਨ res;

  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->aMem[0].flags & MEM_Str );
  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->aMem[0].n == pPKey2->n );
  ਪ੍ਰਤਿਜ੍ਞਾ( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_ਖੇਤਰ_ਗਿਣਤੀ_Within_Limits(nKey1, pKey1, pPKey2->p_ਕੁੰਜੀ_Info);
  serial_type = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ)(aKey1[1]);

vrcs_restart:
  ਜੇ( serial_type<12 ){
    ਜੇ( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      ਜੇ( serial_type>=12 ) ਜਾਓ vrcs_restart;
      ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }ਨਹੀਂਤਾਂ ਜੇ( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ nCmp;
    ਪੂਰਨ nStr;
    ਪੂਰਨ szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    ਜੇ( (szHdr + nStr) > nKey1 ){
      pPKey2->err_ਕੋਡ = (u8)SQLITE_CORRUPT_BKPT;
      ਵਾਪਸ 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    ਜੇ( res>0 ){
      res = pPKey2->r2;
    }ਨਹੀਂਤਾਂ ਜੇ( res<0 ){
      res = pPKey2->r1;
    }ਨਹੀਂਤਾਂ{
      res = nStr - pPKey2->n;
      ਜੇ( res==0 ){
        ਜੇ( pPKey2->n_ਖੇਤਰ>1 ){
          res = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
        }ਨਹੀਂਤਾਂ{
          res = pPKey2->default_rc;
          pPKey2->eq_ਵੇਖਿਆ = 1;
        }
      }ਨਹੀਂਤਾਂ ਜੇ( res>0 ){
        res = pPKey2->r2;
      }ਨਹੀਂਤਾਂ{
        res = pPKey2->r1;
      }
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( vdbe_ਰਿਕਾਰਡ_Compare_Debug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_ਕੁੰਜੀ_Info->db->mallocFailed
  );
  ਵਾਪਸ res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
ਰਿਕਾਰਡ_Compare sqlite3_Vdbe_ਲੱਭੋ_Compare(Unpacked_ਰਿਕਾਰਡ *p){
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
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
  ਜੇ( p->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ<=13 ){
    ਪੂਰਨ flags = p->aMem[0].flags;
    ਜੇ( p->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] ){
      ਜੇ( p->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] & KEYINFO_ਕ੍ਰਮ_BIGNULL ){
        ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }ਨਹੀਂਤਾਂ{
      p->r1 = -1;
      p->r2 = 1;
    }
    ਜੇ( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      ਵਾਪਸ vdbe_ਰਿਕਾਰਡ_Compare_Int;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    ਜੇ( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_ਦੁਆਧਾਰੀ(p->p_ਕੁੰਜੀ_Info->aColl[0])
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      ਵਾਪਸ vdbe_ਰਿਕਾਰਡ_Compare_String;
    }
  }

  ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
ਪੂਰਨ sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_ਕੁੰਜੀ = 0;
  ਪੂਰਨ rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_ਕੁੰਜੀ = sqlite3_Btree_Payload_ਮਾਪ(pCur);
  ਪ੍ਰਤਿਜ੍ਞਾ( (n_Cell_ਕੁੰਜੀ & SQLITE_MAX_U32)==(u64)n_Cell_ਕੁੰਜੀ );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ਵਿੱਥ(pCur, (u32)n_Cell_ਕੁੰਜੀ, &m);
  ਜੇ( rc ){
    ਵਾਪਸ rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  ਪ੍ਰਤਿਜ੍ਞਾ( m.n>=0 );
  ਜੇ( unlikely(szHdr<3 || szHdr>(ਬਿਨਾਨਿਸ਼ਾਨੀ)m.n) ){
    ਜਾਓ idx_rowid_corruption;
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
  ਜੇ( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    ਜਾਓ idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  ਜੇ( unlikely((u32)m.n<szHdr+lenRowid) ){
    ਜਾਓ idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  ਵਾਪਸ SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  ਵਾਪਸ SQLITE_CORRUPT_BKPT;
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
ਪੂਰਨ sqlite3_Vdbe_Idx_ਕੁੰਜੀ_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  Unpacked_ਰਿਕਾਰਡ *pUnpacked,       /* Unpacked version of key */
  ਪੂਰਨ *res                         /* Write the comparison result here */
){
  i64 n_Cell_ਕੁੰਜੀ = 0;
  ਪੂਰਨ rc;
  BtCursor *pCur;
  Mem m;

  ਪ੍ਰਤਿਜ੍ਞਾ( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_ਕੁੰਜੀ = sqlite3_Btree_Payload_ਮਾਪ(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  ਜੇ( n_Cell_ਕੁੰਜੀ<=0 || n_Cell_ਕੁੰਜੀ>0x7fffffff ){
    *res = 0;
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_ਵਿੱਥ(pCur, (u32)n_Cell_ਕੁੰਜੀ, &m);
  ਜੇ( rc ){
    ਵਾਪਸ rc;
  }
  *res = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  ਵਾਪਸ SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_Changes(sqlite3 *db, i64 nChange){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_ਕੁੱਲ_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
ਖਾਲੀ sqlite3_Vdbe_ਗਿਣਤੀ_Changes(Vdbe *v){
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
ਖਾਲੀ sqlite3ExpirePreparedStatements(sqlite3 *db, ਪੂਰਨ i_ਕੋਡ){
  Vdbe *p;
  ਲਈ(p = db->pVdbe; p; p=p->pVNext){
    p->expired = i_ਕੋਡ+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  ਵਾਪਸ v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  ਵਾਪਸ v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_ਮੁੱਲ *sqlite3_Vdbe_Get_Bound_ਮੁੱਲ(Vdbe *v, ਪੂਰਨ iVar, u8 aff){
  ਪ੍ਰਤਿਜ੍ਞਾ( iVar>0 );
  ਜੇ( v ){
    Mem *pMem = &v->aVar[iVar-1];
    ਪ੍ਰਤਿਜ੍ਞਾ( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_ਅੰਦਰੂਨੀ_Func)!=0 );
    ਜੇ( 0==(pMem->flags & MEM_Null) ){
      sqlite3_ਮੁੱਲ *pRet = sqlite3_ਮੁੱਲ_New(v->db);
      ਜੇ( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_ਮੁੱਲ_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      ਵਾਪਸ pRet;
    }
  }
  ਵਾਪਸ 0;
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
ਖਾਲੀ sqlite3VdbeReprepareOnBind(Vdbe *v, ਪੂਰਨ iVar, ਪੂਰਨ bSmallint){
  u32 m;
  ਪ੍ਰਤਿਜ੍ਞਾ( iVar>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_ਅੰਦਰੂਨੀ_Func)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  ਜੇ( bSmallint ){
    ਜੇ( (v->expmask & m)==0 ) v->smimask |= m;
  }ਨਹੀਂਤਾਂ{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  ਪ੍ਰਤਿਜ੍ਞਾ( (v->smimask & v->expmask) == v->smimask );
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
ਸਥਿਰ ਪੂਰਨ vdbe_Skip_ਖੇਤਰ(
  Bitmask ਨਕਾਬ,                   /* Mask of indexed expression fields */
  ਪੂਰਨ i_ਖਾਨਾ,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  ਪੂਰਨ bIntegrity                  /* True if running PRAGMA integrity_check */
){
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_ULPDISTORTION 2
  ਜੇ( i_ਖਾਨਾ>=BMS || (ਨਕਾਬ & MASKBIT(i_ਖਾਨਾ))==0 ) ਵਾਪਸ 0;
  ਜੇ( bIntegrity==0 ) ਵਾਪਸ 1;
  ਜੇ( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&m1,&pMem1->u.r,8);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&m2,&pMem2->u.r,8);
    ਜੇ( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ vdbe_Is_Matching_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(
  BtCursor *pCur,            /* Cursor open on index */
  ਪੂਰਨ bInt,                  /* True for integrity_check-style search */
  Bitmask ਨਕਾਬ,              /* Mask of columns to skip */
  Unpacked_ਰਿਕਾਰਡ *p,         /* Index key being deleted */
  ਪੂਰਨ *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  ਪੂਰਨ rc = SQLITE_OK;

  memset(&m, 0, ਆਕਾਰ(m));
  m.enc = p->p_ਕੁੰਜੀ_Info->enc;
  m.db = p->p_ਕੁੰਜੀ_Info->db;
  nRec = sqlite3_Btree_Payload_ਮਾਪ(pCur);
  ਜੇ( nRec>0x7fffffff ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  ਜੇ( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  ਜੇ( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    ਜੇ( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      ਪੂਰਨ ii = 0;                 /* Iterator variable */

      ਪੂਰਨ n_ਖਾਨਾ = p->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ;
      ਲਈ(ii=0; ii<n_ਖਾਨਾ && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        ਪੂਰਨ nSerial = 0;

        ਜੇ( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          ਰੋਕੋ;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        ਜੇ( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }ਨਹੀਂਤਾਂ{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          ਜੇ( vdbe_Skip_ਖੇਤਰ(ਨਕਾਬ, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_ਕੁੰਜੀ_Info->aColl[ii]);
            ਜੇ( res!=0 ) ਰੋਕੋ;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3_Vdbe_ਲੱਭੋ_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(
  BtCursor *pCur, 
  ਸੂਚਕਾਂਕ *pIdx,
  Unpacked_ਰਿਕਾਰਡ *p, 
  ਪੂਰਨ *pRes,
  ਪੂਰਨ bIntegrity
){
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_FDK_RANGE 10
  ਪੂਰਨ n_ਕਦਮ = 0;
  ਪੂਰਨ res = 1;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask ਨਕਾਬ = 0;
  ਲਈ(ii=0; ii<MIN(pIdx->n_ਕਾਲਮ, BMS); ii++){
    ਪੂਰਨ i_ਖਾਨਾ = pIdx->ai_ਕਾਲਮ[ii];
    ਜੇ( (i_ਖਾਨਾ==XN_ਵਿਅੰਜਕ)
     || (i_ਖਾਨਾ>=0 && (pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags & COLFLAG_VIRTUAL))
    ){
      ਨਕਾਬ |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  ਜੇ( ਨਕਾਬ!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    ਲਈ(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3_Btree_ਪਿਛਲਾ(pCur, 0);
    }
    ਜੇ( rc==SQLITE_ਪੂਰਾ ){
      rc = sqlite3_Btree_ਪਹਿਲਾ(pCur, &res);
      n_ਕਦਮ = -1;
    }ਨਹੀਂਤਾਂ{
      n_ਕਦਮ = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    ਜਦੋਂ( sqlite3BtreeCursorIsValidNN(pCur) ){
      ਲਈ(ii=0; rc==SQLITE_OK && (ii<n_ਕਦਮ || n_ਕਦਮ<0); ii++){
        rc = vdbe_Is_Matching_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(pCur, bIntegrity, ਨਕਾਬ, p, &res);
        ਜੇ( res==0 || rc!=SQLITE_OK ) ਰੋਕੋ;
        rc = sqlite3_Btree_ਅਗਲਾ(pCur, 0);
      }
      ਜੇ( rc==SQLITE_ਪੂਰਾ ){
        rc = SQLITE_OK;
        ਪ੍ਰਤਿਜ੍ਞਾ( res!=0 );
      }
      ਜੇ( n_ਕਦਮ<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) ਰੋਕੋ;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      n_ਕਦਮ = -1;
      rc = sqlite3_Btree_ਪਹਿਲਾ(pCur, &res);
    }
  }

  *pRes = res;
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
ਪੂਰਨ sqlite3_Not_ਸ਼ੁੱਧ_Func(sqlite3_context *pCtx){
  ਅਚਲ VdbeOp *pOp;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( pCtx->pVdbe==0 ) ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  ਜੇ( pOp->opcode==OP_ਸ਼ੁੱਧ_Func ){
    ਅਚਲ ਅੱਖਰ *zContext;
    ਅੱਖਰ *z_ਸੁਨੇਹਾ;
    ਜੇ( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }ਨਹੀਂਤਾਂ ਜੇ( pOp->p5 & NC_Gen_ਖਾਨਾ ){
      zContext = "a generated column";
    }ਨਹੀਂਤਾਂ{
      zContext = "an index";
    }
    z_ਸੁਨੇਹਾ = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_ਨਤੀਜਾ_error(pCtx, z_ਸੁਨੇਹਾ, -1);
    sqlite3_free(z_ਸੁਨੇਹਾ);
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DATETIME_FUNCS */

#ਜੇ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
ਪੂਰਨ sqlite3_Cursor_Range_Hint_ਵਿਅੰਜਕ_Check(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਰਜਿਸਟਰ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (pWalker->u.aMem[p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ].flags & MEM_Undefined)==0 );
  }
  ਵਾਪਸ WRC_Continue;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
ਖਾਲੀ sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  ਜੇ( pVtab->z_Err_ਸੁਨੇਹਾ ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_ਸੁਨੇਹਾ);
    p->z_Err_ਸੁਨੇਹਾ = sqlite3DbStrDup(db, pVtab->z_Err_ਸੁਨੇਹਾ);
    sqlite3_free(pVtab->z_Err_ਸੁਨੇਹਾ);
    pVtab->z_Err_ਸੁਨੇਹਾ = 0;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
ਸਥਿਰ ਖਾਲੀ vdbeFreeUnpacked(sqlite3 *db, ਪੂਰਨ n_ਖੇਤਰ, Unpacked_ਰਿਕਾਰਡ *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<n_ਖੇਤਰ; i++){
      Mem *pMem = &p->aMem[i];
      ਜੇ( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
ਖਾਲੀ sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  ਪੂਰਨ op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  ਅਚਲ ਅੱਖਰ *zDb,                /* Database name */
  ਸਾਰਣੀ *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  ਪੂਰਨ iReg,                       /* Register for new.* record */
  ਪੂਰਨ iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  ਅਚਲ ਅੱਖਰ *zTbl = pTab->zName;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ n_Real_ਖਾਨਾ;
  ਜੇ( pTab->tabFlags & TF_WithoutRowid ){
    n_Real_ਖਾਨਾ = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab)->n_ਕਾਲਮ;
  }ਨਹੀਂਤਾਂ ਜੇ( pTab->tabFlags & TF_HasVirtual ){
    n_Real_ਖਾਨਾ = pTab->nNVCol;
  }ਨਹੀਂਤਾਂ{
    n_Real_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
  }
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( db->pPreUpdate==0 );
  memset(&preupdate, 0, ਆਕਾਰ(PreUpdate));
  ਜੇ( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
  }ਨਹੀਂਤਾਂ{
    ਜੇ( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }ਨਹੀਂਤਾਂ{
      iKey2 = iKey1;
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_BTREE );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->n_ਖੇਤਰ==n_Real_ਖਾਨਾ
       || (pCsr->n_ਖੇਤਰ==n_Real_ਖਾਨਾ+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (ਕੁੰਜੀ_Info*)&preupdate.u_ਕੁੰਜੀ;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_ਕੁੰਜੀ_ਖੇਤਰ = pTab->n_ਖਾਨਾ;
  preupdate.pKeyinfo->a_ਛਾਂਟੋ_Flags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.a_ਰਿਕਾਰਡ);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_ਕੁੰਜੀ_ਖੇਤਰ+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_ਕੁੰਜੀ_ਖੇਤਰ+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  ਜੇ( preupdate.aNew ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pCsr->n_ਖੇਤਰ; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  ਜੇ( preupdate.apDflt ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pTab->n_ਖਾਨਾ; i++){
      sqlite3_ਮੁੱਲ_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
ਅਚਲ ਅੱਖਰ *sqlite3VdbeFuncName(ਅਚਲ sqlite3_context *pCtx){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCtx->pFunc!=0 );
  ਵਾਪਸ pCtx->pFunc->zName;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PERCENTILE */
