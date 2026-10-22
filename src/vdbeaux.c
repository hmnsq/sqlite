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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/* Forward references */
స్థిర శూన్య freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
స్థిర శూన్య vdbeFreeOpArray(sqlite3 *, Op *, పూర్ణాంకం);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, పరిమాణం(Vdbe) );
  ఒకవేళ( p==0 ) తిరిగి 0;
  memset(&p->aOp, 0, పరిమాణం(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  ఒకవేళ( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  ప్రతిజ్ఞా( pParse->aLabel==0 );
  ప్రతిజ్ఞా( pParse->nLabel==0 );
  ప్రతిజ్ఞా( p->nOpAlloc==0 );
  ప్రతిజ్ఞా( pParse->szOpAlloc==0 );
  sqlite3_Vdbe_కూడు_Op2(p, OP_Init, 0, 1);
  తిరిగి p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  తిరిగి p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
శూన్య sqlite3VdbeError(Vdbe *p, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_సందేశం);
  va_start(ap, zFormat);
  p->z_Err_సందేశం = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
శూన్య sqlite3_Vdbe_సమితి_Sql(Vdbe *p, స్థిరరాశి అక్షరం *z, పూర్ణాంకం n, u8 prepFlags){
  ఒకవేళ( p==0 ) తిరిగి;
  p->prepFlags = prepFlags;
  ఒకవేళ( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  ప్రతిజ్ఞా( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
శూన్య sqlite3_Vdbe_కూడు_Dblquote_Str(sqlite3 *db, Vdbe *p, స్థిరరాశి అక్షరం *z){
  ఒకవేళ( p ){
    పూర్ణాంకం n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            పరిమాణం(*pStr)+n+1-పరిమాణం(pStr->z));
    ఒకవేళ( pStr ){
      pStr->p_తదుపరి_Str = p->pDblStr;
      p->pDblStr = pStr;
      జ్ఞాపకం_నకలు(pStr->z, z, n+1);
    }
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
పూర్ణాంకం sqlite3_Vdbe_ఉపయోగిస్తుంది_Double_Quoted_String(
  Vdbe *pVdbe,            /* The prepared statement */
  స్థిరరాశి అక్షరం *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  ప్రతిజ్ఞా( zId!=0 );
  ఒకవేళ( pVdbe->pDblStr==0 ) తిరిగి 0;
  కొరకు(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_తదుపరి_Str){
    ఒకవేళ( strcmp(zId, pStr->z)==0 ) తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు

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
శూన్య sqlite3_Vdbe_మార్పిడి(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  అక్షరం *zTmp;
  ప్రతిజ్ఞా( pA->db==pB->db );
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
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#అయితే_ముగింపు
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  జ్ఞాపకం_నకలు(pB->aCounter, pA->aCounter, పరిమాణం(pB->aCounter));
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
స్థిర పూర్ణాంకం growOpArray(Vdbe *v, పూర్ణాంకం nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#నిర్వచితమైతే SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#లేకపోతే
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/పరిమాణం(Op)));
  వాడని_PARAMETER(nOp);
#అయితే_ముగింపు

  /* Ensure that the size of a VDBE does not grow too large */
  ఒకవేళ( nNew > p->db->a_హద్దు[SQLITE_హద్దు_VDBE_OP] ){
    sqlite3OomFault(p->db);
    తిరిగి SQLITE_NOMEM;
  }

  ప్రతిజ్ఞా( nOp<=(పూర్ణాంకం)(1024/పరిమాణం(Op)) );
  ప్రతిజ్ఞా( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*పరిమాణం(Op));
  ఒకవేళ( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_కొలత(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/పరిమాణం(Op);
    v->aOp = pNew;
  }
  తిరిగి (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర శూన్య test_addop_breakpoint(పూర్ణాంకం pc, Op *pOp){
  స్థిర u64 n = 0;
  (శూన్య)pc;
  (శూన్య)pOp;
  n++;
  ఒకవేళ( n==అతిపెద్ద_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#అయితే_ముగింపు

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం growOp3(Vdbe *p, పూర్ణాంకం op, పూర్ణాంకం p1, పూర్ణాంకం p2, పూర్ణాంకం p3){
  ప్రతిజ్ఞా( p->nOpAlloc<=p->nOp );
  ఒకవేళ( growOpArray(p, 1) ) తిరిగి 1;
  ప్రతిజ్ఞా( p->nOpAlloc>p->nOp );
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, op, p1, p2, p3);
}
స్థిర SQLITE_NOINLINE పూర్ణాంకం కూడు_Op4_Int_Slow(
  Vdbe *p,            /* Add the opcode to this VM */
  పూర్ణాంకం op,             /* The new opcode */
  పూర్ణాంకం p1,             /* The P1 operand */
  పూర్ణాంకం p2,             /* The P2 operand */
  పూర్ణాంకం p3,             /* The P3 operand */
  పూర్ణాంకం p4              /* The P4 operand as an integer */
){
  పూర్ణాంకం addr = sqlite3_Vdbe_కూడు_Op3(p, op, p1, p2, p3);
  ఒకవేళ( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  తిరిగి addr;
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
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op0(Vdbe *p, పూర్ణాంకం op){
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, op, 0, 0, 0);
}
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op1(Vdbe *p, పూర్ణాంకం op, పూర్ణాంకం p1){
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, op, p1, 0, 0);
}
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op2(Vdbe *p, పూర్ణాంకం op, పూర్ణాంకం p1, పూర్ణాంకం p2){
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, op, p1, p2, 0);
}
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op3(Vdbe *p, పూర్ణాంకం op, పూర్ణాంకం p1, పూర్ణాంకం p2, పూర్ణాంకం p3){
  పూర్ణాంకం i;
  VdbeOp *pOp;

  i = p->nOp;
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  ప్రతిజ్ఞా( op>=0 && op<0xff );
  ఒకవేళ( p->nOpAlloc<=i ){
    తిరిగి growOp3(p, op, p1, p2, p3);
  }
  ప్రతిజ్ఞా( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  ప్రతిజ్ఞా( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_చక్రం = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
  pOp->i_Src_పంక్తి = 0;
#అయితే_ముగింపు
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  తిరిగి i;
}
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op4_Int(
  Vdbe *p,            /* Add the opcode to this VM */
  పూర్ణాంకం op,             /* The new opcode */
  పూర్ణాంకం p1,             /* The P1 operand */
  పూర్ణాంకం p2,             /* The P2 operand */
  పూర్ణాంకం p3,             /* The P3 operand */
  పూర్ణాంకం p4              /* The P4 operand as an integer */
){
  పూర్ణాంకం i;
  VdbeOp *pOp;

  i = p->nOp;
  ఒకవేళ( p->nOpAlloc<=i ){
    తిరిగి కూడు_Op4_Int_Slow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  ప్రతిజ్ఞా( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_చక్రం = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
  pOp->i_Src_పంక్తి = 0;
#అయితే_ముగింపు
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  తిరిగి i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
పూర్ణాంకం sqlite3_Vdbe_కూడు_Int64(Vdbe *p, పూర్ణాంకం iDest, i64 iVal){
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
పూర్ణాంకం sqlite3_Vdbe_కూడు_Double(Vdbe *p, పూర్ణాంకం iDest, ద్విగుణ rVal){
  i64 iVal;
  జ్ఞాపకం_నకలు(&iVal,&rVal,8);
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
పూర్ణాంకం sqlite3VdbeGoto(Vdbe *p, పూర్ణాంకం iDest){
  తిరిగి sqlite3_Vdbe_కూడు_Op3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
పూర్ణాంకం sqlite3_Vdbe_ఎక్కించు_String(Vdbe *p, పూర్ణాంకం iDest, స్థిరరాశి అక్షరం *zStr){
  తిరిగి sqlite3_Vdbe_కూడు_Op4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
శూన్య sqlite3_Vdbe_Multi_ఎక్కించు(Vdbe *p, పూర్ణాంకం iDest, స్థిరరాశి అక్షరం *zTypes, ...){
  va_list ap;
  పూర్ణాంకం i;
  అక్షరం c;
  va_start(ap, zTypes);
  కొరకు(i=0; (c = zTypes[i])!=0; i++){
    ఒకవేళ( c=='s' ){
      స్థిరరాశి అక్షరం *z = va_arg(ap, స్థిరరాశి అక్షరం*);
      sqlite3_Vdbe_కూడు_Op4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }లేకపోతే ఒకవేళ( c=='i' ){
      sqlite3_Vdbe_కూడు_Op2(p, OP_Integer, va_arg(ap, పూర్ణాంకం), iDest+i);
    }లేకపోతే{
      వెళ్ళు skip_op_resultrow;
    }
  }
  sqlite3_Vdbe_కూడు_Op2(p, OP_ఫలితం_అడ్డువరుస, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
పూర్ణాంకం sqlite3_Vdbe_కూడు_Op4(
  Vdbe *p,            /* Add the opcode to this VM */
  పూర్ణాంకం op,             /* The new opcode */
  పూర్ణాంకం p1,             /* The P1 operand */
  పూర్ణాంకం p2,             /* The P2 operand */
  పూర్ణాంకం p3,             /* The P3 operand */
  స్థిరరాశి అక్షరం *zP4,    /* The P4 operand */
  పూర్ణాంకం p4type          /* P4 operand type */
){
  పూర్ణాంకం addr = sqlite3_Vdbe_కూడు_Op3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  తిరిగి addr;
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
పూర్ణాంకం sqlite3_Vdbe_కూడు_Function_Call(
  Parse *pParse,        /* Parsing context */
  పూర్ణాంకం p1,               /* Constant argument mask */
  పూర్ణాంకం p2,               /* First argument register */
  పూర్ణాంకం p3,               /* Register into which results are written */
  పూర్ణాంకం nArg,             /* Number of argument */
  స్థిరరాశి FuncDef *pFunc, /* The function to be invoked */
  పూర్ణాంకం eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం addr;
  sqlite3_context *pCtx;
  ప్రతిజ్ఞా( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  ఒకవేళ( pCtx==0 ){
    ప్రతిజ్ఞా( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    తిరిగి 0;
  }
  pCtx->p_బహిర్గతం = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3_Vdbe_కూడు_Op4(v, eCallCtx ? OP_శుద్ధ_Func : OP_Function,
                           p1, p2, p3, (అక్షరం*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  తిరిగి addr;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
పూర్ణాంకం sqlite3_Vdbe_Explain_మాతృ(Parse *pParse){
  VdbeOp *pOp;
  ఒకవేళ( pParse->addrExplain==0 ) తిరిగి 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  తిరిగి pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#ఒకవేళ defined(SQLITE_DEBUG)
శూన్య sqlite3ExplainBreakpoint(స్థిరరాశి అక్షరం *z1, స్థిరరాశి అక్షరం *z2){
  (శూన్య)z1;
  (శూన్య)z2;
}
#అయితే_ముగింపు

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
పూర్ణాంకం sqlite3VdbeExplain(Parse *pParse, u8 b_నెట్టు, స్థిరరాశి అక్షరం *zFmt, ...){
  పూర్ణాంకం addr = 0;
#ఒకవేళ !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  ఒకవేళ( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#అయితే_ముగింపు
  {
    అక్షరం *z_సందేశం;
    Vdbe *v;
    va_list ap;
    పూర్ణాంకం iThis;
    va_start(ap, zFmt);
    z_సందేశం = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3_Vdbe_కూడు_Op4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_సందేశం, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_నెట్టు?"PUSH":"", sqlite3_Vdbe_Get_చివరి_Op(v)->p4.z);
    ఒకవేళ( b_నెట్టు){
      pParse->addrExplain = iThis;
    }
    sqlite3_Vdbe_పరిశీలన_స్థితి(v, iThis, -1, -1, 0, 0);
  }
  తిరిగి addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
శూన్య sqlite3_Vdbe_Explain_తీయి(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3_Vdbe_Explain_మాతృ(pParse);
}
#అయితే_ముగింపు /* SQLITE_OMIT_EXPLAIN */

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
శూన్య sqlite3_Vdbe_కూడు_Parse_Schema_Op(Vdbe *p, పూర్ణాంకం iDb, అక్షరం *zWhere, u16 p5){
  పూర్ణాంకం j;
  ప్రతిజ్ఞా( p5==0 || zWhere==0 );
  sqlite3_Vdbe_కూడు_Op4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  కొరకు(j=0; j<p->db->nDb; j++) sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
శూన్య sqlite3_Vdbe_చివర_Coroutine(Vdbe *v, పూర్ణాంకం regYield){
  sqlite3_Vdbe_కూడు_Op1(v, OP_చివర_Coroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_తాత్కాలిక_Reg = 0;
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
పూర్ణాంకం sqlite3VdbeMakeLabel(Parse *pParse){
  తిరిగి --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
స్థిర SQLITE_NOINLINE శూన్య resizeResolveLabel(Parse *p, Vdbe *v, పూర్ణాంకం j){
  పూర్ణాంకం n_New_కొలత = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_కొలత*పరిమాణం(p->aLabel[0]));
  ఒకవేళ( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_DEBUG
    పూర్ణాంకం i;
    కొరకు(i=p->nLabelAlloc; i<n_New_కొలత; i++) p->aLabel[i] = -1;
#అయితే_ముగింపు
    ఒకవేళ( n_New_కొలత>=100 && (n_New_కొలత/100)>(p->nLabelAlloc/100) ){
      sqlite3_పురోగతి_Check(p);
    }
    p->nLabelAlloc = n_New_కొలత;
    p->aLabel[j] = v->nOp;
  }
}
శూన్య sqlite3VdbeResolveLabel(Vdbe *v, పూర్ణాంకం x){
  Parse *p = v->pParse;
  పూర్ణాంకం j = ADDR(x);
  ప్రతిజ్ఞా( v->eVdbeState==VDBE_INIT_STATE );
  ప్రతిజ్ఞా( j<-p->nLabel );
  ప్రతిజ్ఞా( j>=0 );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( p->db->flags & SQLITE_VdbeAddopTrace ){
    రాయి("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#అయితే_ముగింపు
  ఒకవేళ( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }లేకపోతే{
    ప్రతిజ్ఞా( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
శూన్య sqlite3_Vdbe_నిరంతరం_Only_Once(Vdbe *p){
  sqlite3_Vdbe_కూడు_Op2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
శూన్య sqlite3VdbeReusable(Vdbe *p){
  పూర్ణాంకం i;
  కొరకు(i=1; ALWAYS(i<p->nOp); i++){
    ఒకవేళ( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      ఆపు;
    }
  }
}

#నిర్వచితమైతే SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
రకనిర్వచనం నిర్మాణం VdbeOpIter VdbeOpIter;
నిర్మాణం VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  Sub_కార్యక్రమం **apSub;        /* Array of subprograms */
  పూర్ణాంకం nSub;                  /* Number of entries in apSub */
  పూర్ణాంకం iAddr;                 /* Address of next instruction to return */
  పూర్ణాంకం iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
స్థిర Op *op_Iter_తదుపరి(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  పూర్ణాంకం nOp;

  ఒకవేళ( p->iSub<=p->nSub ){

    ఒకవేళ( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }లేకపోతే{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    ప్రతిజ్ఞా( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    ఒకవేళ( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    ఒకవేళ( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*పరిమాణం(Sub_కార్యక్రమం*);
      పూర్ణాంకం j;
      కొరకు(j=0; j<p->nSub; j++){
        ఒకవేళ( p->apSub[j]==pRet->p4.p_కార్యక్రమం ) ఆపు;
      }
      ఒకవేళ( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        ఒకవేళ( !p->apSub ){
          pRet = 0;
        }లేకపోతే{
          p->apSub[p->nSub++] = pRet->p4.p_కార్యక్రమం;
        }
      }
    }
  }

  తిరిగి pRet;
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
పూర్ణాంకం sqlite3VdbeAssertMayAbort(Vdbe *v, పూర్ణాంకం mayAbort){
  పూర్ణాంకం hasAbort = 0;
  పూర్ణాంకం hasFkCounter = 0;
  పూర్ణాంకం has_Create_పట్టిక = 0;
  పూర్ణాంకం has_Create_సూచీ = 0;
  పూర్ణాంకం hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  ఒకవేళ( v==0 ) తిరిగి 0;
  memset(&sIter, 0, పరిమాణం(sIter));
  sIter.v = v;

  అయితే( (pOp = op_Iter_తదుపరి(&sIter))!=0 ){
    పూర్ణాంకం opcode = pOp->opcode;
    ఒకవేళ( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_శుద్ధ_Func
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      ఆపు;
    }
    ఒకవేళ( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) has_Create_పట్టిక = 1;
    ఒకవేళ( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      ఒకవేళ( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_సూచీ = 1;
      ఒకవేళ( opcode==OP_Clear ) has_Create_సూచీ = 1;
    }
    ఒకవేళ( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
    ఒకవేళ( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
      hasFkCounter = 1;
    }
#అయితే_ముగింపు
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  తిరిగి ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (has_Create_పట్టిక && hasInitCoroutine) || has_Create_సూచీ
  );
}
#అయితే_ముగింపు /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
శూన్య sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  ఒకవేళ( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
శూన్య sqlite3VdbeAssertAbortable(Vdbe *p){
  ప్రతిజ్ఞా( p->nWrite==0 || p->ఉపయోగిస్తుంది_Stmt_Journal );
}
#అయితే_ముగింపు

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
స్థిర శూన్య resolve_P2_విలువలు(Vdbe *p, పూర్ణాంకం *p_Max_Vtab_వాదనలు){
  పూర్ణాంకం n_Max_Vtab_వాదనలు = *p_Max_Vtab_వాదనలు;
  Op *pOp;
  Parse *pParse = p->pParse;
  పూర్ణాంకం *aLabel = pParse->aLabel;

  ప్రతిజ్ఞా( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  ప్రతిజ్ఞా( p->aOp[0].opcode==OP_Init );
  అయితే( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    ఒకవేళ( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      ఎంపిక( pOp->opcode ){
        స్థితి OP_Transaction: {
          ఒకవేళ( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        స్థితి OP_AutoCommit:
        స్థితి OP_Savepoint: {
          p->bIsReader = 1;
          ఆపు;
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
        స్థితి OP_Checkpoint:
#అయితే_ముగింపు
        స్థితి OP_Vacuum:
        స్థితి OP_Journal_బహుళకం: {
          p->readOnly = 0;
          p->bIsReader = 1;
          ఆపు;
        }
        స్థితి OP_Init: {
          ప్రతిజ్ఞా( pOp->p2>=0 );
          వెళ్ళు resolve_p2_విలువలు_loop_exit;
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
        స్థితి OP_VUpdate: {
          ఒకవేళ( pOp->p2>n_Max_Vtab_వాదనలు ) n_Max_Vtab_వాదనలు = pOp->p2;
          ఆపు;
        }
        స్థితి OP_VFilter: {
          పూర్ణాంకం n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          ప్రతిజ్ఞా( (pOp - p->aOp) >= 3 );
          ప్రతిజ్ఞా( pOp[-1].opcode==OP_Integer );
          ప్రతిజ్ఞా( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          ఒకవేళ( n>n_Max_Vtab_వాదనలు ) n_Max_Vtab_వాదనలు = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#అయితే_ముగింపు
        అప్రమేయ: {
          ఒకవేళ( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            ప్రతిజ్ఞా( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            ప్రతిజ్ఞా( ADDR(pOp->p2)<-pParse->nLabel );
            ప్రతిజ్ఞా( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          ప్రతిజ్ఞా( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          ప్రతిజ్ఞా( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          ఆపు;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      ప్రతిజ్ఞా( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    ప్రతిజ్ఞా( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_విలువలు_loop_exit:
  ఒకవేళ( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *p_Max_Vtab_వాదనలు = n_Max_Vtab_వాదనలు;
  ప్రతిజ్ఞా( p->bIsReader!=0 || Db_ముసుగు_All_Zero(p->btree_ముసుగు) );
}

#నిర్వచితమైతే SQLITE_DEBUG
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
శూన్య sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  పూర్ణాంకం i_మొదటి,       /* First opcode of the subroutine */
  పూర్ణాంకం i_చివరి,        /* Last opcode of the subroutine */
  పూర్ణాంకం iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  పూర్ణాంకం i;
  sqlite3_str *pErr = 0;
  ప్రతిజ్ఞా( v!=0 );
  pParse = v->pParse;
  ప్రతిజ్ఞా( pParse!=0 );
  ఒకవేళ( pParse->nErr ) తిరిగి;
  ప్రతిజ్ఞా( i_చివరి>=i_మొదటి );
  ప్రతిజ్ఞా( i_చివరి<v->nOp );
  pOp = &v->aOp[i_మొదటి];
  కొరకు(i=i_మొదటి; i<=i_చివరి; i++, pOp++){
    ఒకవేళ( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      పూర్ణాంకం iDest = pOp->p2;   /* Jump destination */
      ఒకవేళ( iDest==0 ) కొనసాగు;
      ఒకవేళ( pOp->opcode==OP_Gosub ) కొనసాగు;
      ఒకవేళ( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        కొనసాగు;
      }
      ఒకవేళ( iDest<0 ){
        పూర్ణాంకం j = ADDR(iDest);
        ప్రతిజ్ఞా( j>=0 );
        ఒకవేళ( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          కొనసాగు;
        }
        iDest = pParse->aLabel[j];
      }
      ఒకవేళ( iDest<i_మొదటి || iDest>i_చివరి ){
        పూర్ణాంకం j = iDest;
        కొరకు(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          ఒకవేళ( pX->opcode==OP_Return ){
            ఒకవేళ( pX->p1==iRetReg ) ఆపు;
            కొనసాగు;
          }
          ఒకవేళ( pX->opcode==OP_Noop ) కొనసాగు;
          ఒకవేళ( pX->opcode==OP_Explain ) కొనసాగు;
          ఒకవేళ( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }లేకపోతే{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, i_మొదటి, i_చివరి);
          ఆపు;
        }
      }
    }
  }
  ఒకవేళ( pErr ){
    అక్షరం *zErr = sqlite3_str_ముగింపు(pErr);
    sqlite3_Vdbe_కూడు_Op4(v, OP_Halt, SQLITE_అంతర్గత, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
పూర్ణాంకం sqlite3VdbeCurrentAddr(Vdbe *p){
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  తిరిగి p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
శూన్య sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, పూర్ణాంకం N){
  ప్రతిజ్ఞా( p->nOp + N <= p->nOpAlloc );
}
#అయితే_ముగింపు

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
శూన్య sqlite3_Vdbe_Verify_No_ఫలితం_అడ్డువరుస(Vdbe *p){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->nOp; i++){
    ప్రతిజ్ఞా( p->aOp[i].opcode!=OP_ఫలితం_అడ్డువరుస );
  }
}
#అయితే_ముగింపు

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#ఒకవేళ defined(SQLITE_DEBUG)
శూన్య sqlite3VdbeVerifyAbortable(Vdbe *p, పూర్ణాంకం onError){
  ఒకవేళ( onError==OE_Abort ) sqlite3_Vdbe_కూడు_Op0(p, OP_Abortable);
}
#అయితే_ముగింపు

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, పూర్ణాంకం *pnOp, పూర్ణాంకం *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  ప్రతిజ్ఞా( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  ప్రతిజ్ఞా( Db_ముసుగు_All_Zero(p->btree_ముసుగు) );

  resolve_P2_విలువలు(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  తిరిగి aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3_Vdbe_కూడు_Op_List(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  పూర్ణాంకం nOp,                     /* Number of opcodes to add */
  VdbeOpList స్థిరరాశి *aOp,       /* The opcodes to be added */
  పూర్ణాంకం iLineno                  /* Source-file line number of first opcode */
){
  పూర్ణాంకం i;
  VdbeOp *p_బహిర్గతం, *p_మొదటి;
  ప్రతిజ్ఞా( nOp>0 );
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  ఒకవేళ( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    తిరిగి 0;
  }
  p_మొదటి = p_బహిర్గతం = &p->aOp[p->nOp];
  కొరకు(i=0; i<nOp; i++, aOp++, p_బహిర్గతం++){
    p_బహిర్గతం->opcode = aOp->opcode;
    p_బహిర్గతం->p1 = aOp->p1;
    p_బహిర్గతం->p2 = aOp->p2;
    ప్రతిజ్ఞా( aOp->p2>=0 );
    ఒకవేళ( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      p_బహిర్గతం->p2 += p->nOp;
    }
    p_బహిర్గతం->p3 = aOp->p3;
    p_బహిర్గతం->p4type = P4_NOTUSED;
    p_బహిర్గతం->p4.p = 0;
    p_బహిర్గతం->p5 = 0;
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
    p_బహిర్గతం->zComment = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
    p_బహిర్గతం->i_Src_పంక్తి = iLineno+i;
#లేకపోతే
    (శూన్య)iLineno;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#అయితే_ముగింపు
  }
  p->nOp += nOp;
  తిరిగి p_మొదటి;
}

#ఒకవేళ defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
శూన్య sqlite3_Vdbe_పరిశీలన_స్థితి(
  Vdbe *p,                        /* VM to add scanstatus() to */
  పూర్ణాంకం addrExplain,                /* Address of OP_Explain (or 0) */
  పూర్ణాంకం addrLoop,                   /* Address of loop counter */
  పూర్ణాంకం addr_సందర్శించు,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  స్థిరరాశి అక్షరం *zName               /* Name of table or index being scanned */
){
  ఒకవేళ( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->n_పరిశీలన) * పరిమాణం(పరిశీలన_స్థితి);
    పరిశీలన_స్థితి *aNew;
    aNew = (పరిశీలన_స్థితి*)sqlite3DbRealloc(p->db, p->a_పరిశీలన, nByte);
    ఒకవేళ( aNew ){
      పరిశీలన_స్థితి *pNew = &aNew[p->n_పరిశీలన++];
      memset(pNew, 0, పరిమాణం(పరిశీలన_స్థితి));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addr_సందర్శించు = addr_సందర్శించు;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->a_పరిశీలన = aNew;
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
శూన్య sqlite3_Vdbe_పరిశీలన_స్థితి_Range(
  Vdbe *p,
  పూర్ణాంకం addrExplain,
  పూర్ణాంకం addr_ప్రారంభం,
  పూర్ణాంకం addr_చివర
){
  ఒకవేళ( IS_STMT_SCANSTATUS(p->db) ){
    పరిశీలన_స్థితి *p_పరిశీలన = 0;
    పూర్ణాంకం ii;
    కొరకు(ii=p->n_పరిశీలన-1; ii>=0; ii--){
      p_పరిశీలన = &p->a_పరిశీలన[ii];
      ఒకవేళ( p_పరిశీలన->addrExplain==addrExplain ) ఆపు;
      p_పరిశీలన = 0;
    }
    ఒకవేళ( p_పరిశీలన ){
      ఒకవేళ( addr_చివర<0 ) addr_చివర = sqlite3VdbeCurrentAddr(p)-1;
      కొరకు(ii=0; ii<Array_కొలత(p_పరిశీలన->aAddrRange); ii+=2){
        ఒకవేళ( p_పరిశీలన->aAddrRange[ii]==0 ){
          p_పరిశీలన->aAddrRange[ii] = addr_ప్రారంభం;
          p_పరిశీలన->aAddrRange[ii+1] = addr_చివర;
          ఆపు;
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
శూన్య sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(
  Vdbe *p,
  పూర్ణాంకం addrExplain,
  పూర్ణాంకం addrLoop,
  పూర్ణాంకం addr_సందర్శించు
){
  ఒకవేళ( IS_STMT_SCANSTATUS(p->db) ){
    పరిశీలన_స్థితి *p_పరిశీలన = 0;
    పూర్ణాంకం ii;
    కొరకు(ii=p->n_పరిశీలన-1; ii>=0; ii--){
      p_పరిశీలన = &p->a_పరిశీలన[ii];
      ఒకవేళ( p_పరిశీలన->addrExplain==addrExplain ) ఆపు;
      p_పరిశీలన = 0;
    }
    ఒకవేళ( p_పరిశీలన ){
      ఒకవేళ( addrLoop>0 ) p_పరిశీలన->addrLoop = addrLoop;
      ఒకవేళ( addr_సందర్శించు>0 ) p_పరిశీలన->addr_సందర్శించు = addr_సందర్శించు;
    }
  }
}
#అయితే_ముగింపు /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
శూన్య sqlite3VdbeChangeOpcode(Vdbe *p, పూర్ణాంకం addr, u8 iNewOpcode){
  ప్రతిజ్ఞా( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
శూన్య sqlite3VdbeChangeP1(Vdbe *p, పూర్ణాంకం addr, పూర్ణాంకం val){
  ప్రతిజ్ఞా( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
శూన్య sqlite3VdbeChangeP2(Vdbe *p, పూర్ణాంకం addr, పూర్ణాంకం val){
  ప్రతిజ్ఞా( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
శూన్య sqlite3VdbeChangeP3(Vdbe *p, పూర్ణాంకం addr, పూర్ణాంకం val){
  ప్రతిజ్ఞా( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
శూన్య sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  ప్రతిజ్ఞా( p->nOp>0 || p->db->mallocFailed );
  ఒకవేళ( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
శూన్య sqlite3_Vdbe_Typeof_స్తంభం(Vdbe *p, పూర్ణాంకం iDest){
  VdbeOp *pOp = sqlite3_Vdbe_Get_చివరి_Op(p);
#నిర్వచితమైతే SQLITE_DEBUG
  అయితే( pOp->opcode==OP_ReleaseReg ) pOp--;
#అయితే_ముగింపు
  ఒకవేళ( pOp->p3==iDest && pOp->opcode==OP_స్తంభం ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
శూన్య sqlite3_Vdbe_Jump_ఇక్కడ(Vdbe *p, పూర్ణాంకం addr){
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
శూన్య sqlite3_Vdbe_Jump_ఇక్కడ_Or_తీయి_Inst(Vdbe *p, పూర్ణాంకం addr){
  ఒకవేళ( addr==p->nOp-1 ){
    ప్రతిజ్ఞా( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    ప్రతిజ్ఞా( p->aOp[addr].p4type==0 );
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
    sqlite3_Vdbe_Get_చివరి_Op(p)->i_Src_పంక్తి = 0;  /* Erase VdbeCoverage() macros */
#అయితే_ముగింపు
    p->nOp--;
  }లేకపోతే{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
స్థిర శూన్య freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
స్థిర SQLITE_NOINLINE శూన్య freeP4Mem(sqlite3 *db, Mem *p){
  ఒకవేళ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
స్థిర SQLITE_NOINLINE శూన్య freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  ప్రతిజ్ఞా( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
స్థిర శూన్య freeP4(sqlite3 *db, పూర్ణాంకం p4type, శూన్య *p4){
  ప్రతిజ్ఞా( db );
  ఎంపిక( p4type ){
    స్థితి P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      ఆపు;
    }
    స్థితి P4_DYNAMIC:
    స్థితి P4_INTARRAY: {
      ఒకవేళ( p4 ) sqlite3DbNNFreeNN(db, p4);
      ఆపు;
    }
    స్థితి P4_KEYINFO: {
      ఒకవేళ( db->pn_బైట్లు_Freed==0 ) sqlite3_కీలకం_Info_Unref((కీలకం_Info*)p4);
      ఆపు;
    }
#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
    స్థితి P4_సమాసం: {
      sqlite3_సమాసం_Delete(db, (సమాసం*)p4);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      ఆపు;
    }
    స్థితి P4_MEM: {
      ఒకవేళ( db->pn_బైట్లు_Freed==0 ){
        sqlite3_విలువ_Free((sqlite3_విలువ*)p4);
      }లేకపోతే{
        freeP4Mem(db, (Mem*)p4);
      }
      ఆపు;
    }
    స్థితి P4_VTAB : {
      ఒకవేళ( db->pn_బైట్లు_Freed==0 ) sqlite3VtabUnlock((VTable *)p4);
      ఆపు;
    }
    స్థితి P4_TABLEREF: {
      ఒకవేళ( db->pn_బైట్లు_Freed==0 ) sqlite3_Delete_పట్టిక(db, (పట్టిక*)p4);
      ఆపు;
    }
    స్థితి P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      ఆపు;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
స్థిర శూన్య vdbeFreeOpArray(sqlite3 *db, Op *aOp, పూర్ణాంకం nOp){
  ప్రతిజ్ఞా( nOp>=0 );
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( aOp ){
    Op *pOp = &aOp[nOp-1];
    అయితే(1){  /* Exit via break */
      ఒకవేళ( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#అయితే_ముగింపు    
      ఒకవేళ( pOp==aOp ) ఆపు;
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
శూన్య sqlite3_Vdbe_Link_Sub_కార్యక్రమం(Vdbe *pVdbe, Sub_కార్యక్రమం *p){
  p->p_తదుపరి = pVdbe->p_కార్యక్రమం;
  pVdbe->p_కార్యక్రమం = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
పూర్ణాంకం sqlite3_Vdbe_Has_Sub_కార్యక్రమం(Vdbe *pVdbe){
  తిరిగి pVdbe->p_కార్యక్రమం!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
పూర్ణాంకం sqlite3_Vdbe_Change_గమ్యం_Noop(Vdbe *p, పూర్ణాంకం addr){
  VdbeOp *pOp;
  ఒకవేళ( p->db->mallocFailed ) తిరిగి 0;
  ప్రతిజ్ఞా( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  తిరిగి 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
పూర్ణాంకం sqlite3_Vdbe_Delete_పూర్వ_Opcode(Vdbe *p, u8 op){
  ఒకవేళ( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    తిరిగి sqlite3_Vdbe_Change_గమ్యం_Noop(p, p->nOp-1);
  }లేకపోతే{
    తిరిగి 0;
  }
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
శూన్య sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  పూర్ణాంకం i_మొదటి,          /* Index of first register to be released */
  పూర్ణాంకం N,               /* Number of registers to release */
  u32 ముసుగు,            /* Mask of registers to NOT release */
  పూర్ణాంకం bUndefine        /* If true, mark registers as undefined */
){
  ఒకవేళ( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) తిరిగి;
  ప్రతిజ్ఞా( pParse->pVdbe );
  ప్రతిజ్ఞా( i_మొదటి>=1 );
  ప్రతిజ్ఞా( i_మొదటి+N-1<=pParse->nMem );
  ఒకవేళ( N<=31 && ముసుగు!=0 ){
    అయితే( N>0 && (ముసుగు&1)!=0 ){
      ముసుగు >>= 1;
      i_మొదటి++;
      N--;
    }
    అయితే( N>0 && N<=32 && (ముసుగు & MASKBIT32(N-1))!=0 ){
      ముసుగు &= ~MASKBIT32(N-1);
      N--;
    }
  }
  ఒకవేళ( N>0 ){
    sqlite3_Vdbe_కూడు_Op3(pParse->pVdbe, OP_ReleaseReg, i_మొదటి, N, *(పూర్ణాంకం*)&ముసుగు);
    ఒకవేళ( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

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
స్థిర శూన్య SQLITE_NOINLINE vdbe_Change_P4_నిండిన(
  Vdbe *p,
  Op *pOp,
  స్థిరరాశి అక్షరం *zP4,
  పూర్ణాంకం n
){
  ఒకవేళ( pOp->p4type ){
    ప్రతిజ్ఞా( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  ఒకవేళ( n<0 ){
    sqlite3VdbeChangeP4(p, (పూర్ణాంకం)(pOp - p->aOp), zP4, n);
  }లేకపోతే{
    ఒకవేళ( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
శూన్య sqlite3VdbeChangeP4(Vdbe *p, పూర్ణాంకం addr, స్థిరరాశి అక్షరం *zP4, పూర్ణాంకం n){
  Op *pOp;
  sqlite3 *db;
  ప్రతిజ్ఞా( p!=0 );
  db = p->db;
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  ప్రతిజ్ఞా( p->aOp!=0 || db->mallocFailed );
  ఒకవేళ( db->mallocFailed ){
    ఒకవేళ( n!=P4_VTAB ) freeP4(db, n, (శూన్య*)*(అక్షరం**)&zP4);
    తిరిగి;
  }
  ప్రతిజ్ఞా( p->nOp>0 );
  ప్రతిజ్ఞా( addr<p->nOp );
  ఒకవేళ( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  ఒకవేళ( n>=0 || pOp->p4type ){
    vdbe_Change_P4_నిండిన(p, pOp, zP4, n);
    తిరిగి;
  }
  ఒకవేళ( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_గమ్యం_INT(zP4);
    pOp->p4type = P4_INT32;
  }లేకపోతే ఒకవేళ( zP4!=0 ){
    ప్రతిజ్ఞా( n<0 );
    pOp->p4.p = (శూన్య*)zP4;
    pOp->p4type = (గుర్తుగల అక్షరం)n;
    ఒకవేళ( n==P4_VTAB ) sqlite3_Vtab_తాళం((VTable*)zP4);
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
శూన్య sqlite3VdbeAppendP4(Vdbe *p, శూన్య *pP4, పూర్ణాంకం n){
  VdbeOp *pOp;
  ప్రతిజ్ఞా( n!=P4_INT32 && n!=P4_VTAB );
  ప్రతిజ్ఞా( n<=0 );
  ఒకవేళ( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }లేకపోతే{
    ప్రతిజ్ఞా( pP4!=0 || n==P4_DYNAMIC );
    ప్రతిజ్ఞా( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    ప్రతిజ్ఞా( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
శూన్య sqlite3_Vdbe_సమితి_P4_కీలకం_Info(Parse *pParse, సూచీ *pIdx){
  Vdbe *v = pParse->pVdbe;
  కీలకం_Info *p_కీలకం_Info;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pIdx!=0 );
  p_కీలకం_Info = sqlite3_కీలకం_Info_యొక్క_సూచీ(pParse, pIdx);
  ఒకవేళ( p_కీలకం_Info ) sqlite3VdbeAppendP4(v, p_కీలకం_Info, P4_KEYINFO);
}

#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
స్థిర శూన్య vdbeVComment(Vdbe *p, స్థిరరాశి అక్షరం *zFormat, va_list ap){
  ప్రతిజ్ఞా( p->nOp>0 || p->aOp==0 );
  ప్రతిజ్ఞా( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  ఒకవేళ( p->nOp ){
    ప్రతిజ్ఞా( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
శూన్య sqlite3VdbeComment(Vdbe *p, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  ఒకవేళ( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
శూన్య sqlite3VdbeNoopComment(Vdbe *p, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  ఒకవేళ( p ){
    sqlite3_Vdbe_కూడు_Op0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#అయితే_ముగింపు  /* NDEBUG */

#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
శూన్య sqlite3_Vdbe_సమితి_పంక్తి_సంఖ్య(Vdbe *v, పూర్ణాంకం i_పంక్తి){
  sqlite3_Vdbe_Get_చివరి_Op(v)->i_Src_పంక్తి = i_పంక్తి;
}
#అయితే_ముగింపు /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, పూర్ణాంకం addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  స్థిర VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  ప్రతిజ్ఞా( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  ఒకవేళ( p->db->mallocFailed ){
    తిరిగి (VdbeOp*)&dummy;
  }లేకపోతే{
    తిరిగి &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3_Vdbe_Get_చివరి_Op(Vdbe *p){
  తిరిగి sqlite3VdbeGetOp(p, p->nOp - 1);
}

#ఒకవేళ defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
స్థిర పూర్ణాంకం translateP(అక్షరం c, స్థిరరాశి Op *pOp){
  ఒకవేళ( c=='1' ) తిరిగి pOp->p1;
  ఒకవేళ( c=='2' ) తిరిగి pOp->p2;
  ఒకవేళ( c=='3' ) తిరిగి pOp->p3;
  ఒకవేళ( c=='4' ) తిరిగి pOp->p4.i;
  తిరిగి pOp->p5;
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
అక్షరం *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  స్థిరరాశి Op *pOp,     /* The opcode to be commented */
  స్థిరరాశి అక్షరం *zP4    /* Previously obtained value for P4 */
){
  స్థిరరాశి అక్షరం *zOpName;
  స్థిరరాశి అక్షరం *zSynopsis;
  పూర్ణాంకం nOpName;
  పూర్ణాంకం ii;
  అక్షరం zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_నిడివి);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  ఒకవేళ( zOpName[nOpName+1] ){
    పూర్ణాంకం చూసిన_Com = 0;
    అక్షరం c;
    zSynopsis = zOpName + nOpName + 1;
    ఒకవేళ( అక్షరశ్రేణి_పోలిక(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(పరిమాణం(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    కొరకు(ii=0; (c = zSynopsis[ii])!=0; ii++){
      ఒకవేళ( c=='P' ){
        c = zSynopsis[++ii];
        ఒకవేళ( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }లేకపోతే ఒకవేళ( c=='X' ){
          ఒకవేళ( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            చూసిన_Com = 1;
            ఆపు;
          }
        }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_పోలిక(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_గమ్యం_64(pOp->p1,pOp->p3));
          ii += 4;
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
        }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_పోలిక(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_గమ్యం_64(pOp->p2,pOp->p3));
          ii += 4;
#అయితే_ముగింపు
        }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_పోలిక(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_గమ్యం_64(pOp->p1,pOp->p3);
          ద్విగుణ r;
          జ్ఞాపకం_నకలు(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }లేకపోతే{
          పూర్ణాంకం v1 = translateP(c, pOp);
          పూర్ణాంకం v2;
          ఒకవేళ( అక్షరశ్రేణి_పోలిక(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            ఒకవేళ( అక్షరశ్రేణి_పోలిక(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            ఒకవేళ( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }లేకపోతే{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_పోలిక(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            ఒకవేళ( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }లేకపోతే ఒకవేళ( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }లేకపోతే ఒకవేళ( x.accError==0 ){
              ప్రతిజ్ఞా( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }లేకపోతే{
            sqlite3_str_appendf(&x, "%d", v1);
            ఒకవేళ( అక్షరశ్రేణి_పోలిక(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }లేకపోతే{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    ఒకవేళ( !చూసిన_Com && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }లేకపోతే ఒకవేళ( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  ఒకవేళ( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  తిరిగి sqlite3_Str_Accum_ముగింపు(&x);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#ఒకవేళ VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
స్థిర శూన్య display_P4_సమాసం(StrAccum *p, సమాసం *p_సమాసం){
  స్థిరరాశి అక్షరం *zOp = 0;
  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_STRING:
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_str_appendf(p, "%Q", p_సమాసం->u.zToken);
      ఆపు;
    స్థితి TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_సమాసం->u.i_విలువ);
      ఆపు;
    స్థితి TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      ఆపు;
    స్థితి TK_రిజిస్టరు: {
      sqlite3_str_appendf(p, "r[%d]", p_సమాసం->i_పట్టిక);
      ఆపు;
    }
    స్థితి TK_స్తంభం: {
      ఒకవేళ( p_సమాసం->i_స్తంభం<0 ){
        sqlite3_str_appendf(p, "rowid");
      }లేకపోతే{
        sqlite3_str_appendf(p, "c%d", (పూర్ణాంకం)p_సమాసం->i_స్తంభం);
      }
      ఆపు;
    }
    స్థితి TK_LT:      zOp = "LT";      ఆపు;
    స్థితి TK_LE:      zOp = "LE";      ఆపు;
    స్థితి TK_GT:      zOp = "GT";      ఆపు;
    స్థితి TK_GE:      zOp = "GE";      ఆపు;
    స్థితి TK_NE:      zOp = "NE";      ఆపు;
    స్థితి TK_EQ:      zOp = "EQ";      ఆపు;
    స్థితి TK_IS:      zOp = "IS";      ఆపు;
    స్థితి TK_ISNOT:   zOp = "ISNOT";   ఆపు;
    స్థితి TK_AND:     zOp = "AND";     ఆపు;
    స్థితి TK_OR:      zOp = "OR";      ఆపు;
    స్థితి TK_PLUS:    zOp = "ADD";     ఆపు;
    స్థితి TK_నక్షత్రం:    zOp = "MUL";     ఆపు;
    స్థితి TK_MINUS:   zOp = "SUB";     ఆపు;
    స్థితి TK_REM:     zOp = "REM";     ఆపు;
    స్థితి TK_BITAND:  zOp = "BITAND";  ఆపు;
    స్థితి TK_BITOR:   zOp = "BITOR";   ఆపు;
    స్థితి TK_SLASH:   zOp = "DIV";     ఆపు;
    స్థితి TK_LSHIFT:  zOp = "LSHIFT";  ఆపు;
    స్థితి TK_RSHIFT:  zOp = "RSHIFT";  ఆపు;
    స్థితి TK_CONCAT:  zOp = "CONCAT";  ఆపు;
    స్థితి TK_UMINUS:  zOp = "MINUS";   ఆపు;
    స్థితి TK_UPLUS:   zOp = "PLUS";    ఆపు;
    స్థితి TK_BITNOT:  zOp = "BITNOT";  ఆపు;
    స్థితి TK_NOT:     zOp = "NOT";     ఆపు;
    స్థితి TK_ISNULL:  zOp = "ISNULL";  ఆపు;
    స్థితి TK_NOTNULL: zOp = "NOTNULL"; ఆపు;

    అప్రమేయ:
      sqlite3_str_appendf(p, "%s", "expr");
      ఆపు;
  }

  ఒకవేళ( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_సమాసం(p, p_సమాసం->p_ఎడమ);
    ఒకవేళ( p_సమాసం->p_కుడి ){
      sqlite3_str_append(p, ",", 1);
      display_P4_సమాసం(p, p_సమాసం->p_కుడి);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#అయితే_ముగింపు /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#ఒకవేళ VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
అక్షరం *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  అక్షరం *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_నిడివి);
  ఎంపిక( pOp->p4type ){
    స్థితి P4_KEYINFO: {
      పూర్ణాంకం j;
      కీలకం_Info *p_కీలకం_Info = pOp->p4.p_కీలకం_Info;
      ప్రతిజ్ఞా( p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_కీలకం_Info->n_కీలకం_క్షేత్రం);
      కొరకు(j=0; j<p_కీలకం_Info->n_కీలకం_క్షేత్రం; j++){
        CollSeq *pColl = p_కీలకం_Info->aColl[j];
        స్థిరరాశి అక్షరం *zColl = pColl ? pColl->zName : "";
        ఒకవేళ( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_కీలకం_Info->a_క్రమీకరించు_Flags[j] & KEYINFO_క్రమం_DESC) ? "-" : "",
               (p_కీలకం_Info->a_క్రమీకరించు_Flags[j] & KEYINFO_క్రమం_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      ఆపు;
    }
#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
    స్థితి P4_సమాసం: {
      display_P4_సమాసం(&x, pOp->p4.p_సమాసం);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి P4_COLLSEQ: {
      స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      ప్రతిజ్ఞా( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      ఆపు;
    }
    స్థితి P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ఆపు;
    }
    స్థితి P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      ఆపు;
    }
    స్థితి P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      ఆపు;
    }
    స్థితి P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      ఒకవేళ( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }లేకపోతే ఒకవేళ( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }లేకపోతే ఒకవేళ( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }లేకపోతే ఒకవేళ( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }లేకపోతే{
        ప్రతిజ్ఞా( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    స్థితి P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      కొరకు(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      ఆపు;
    }
    స్థితి P4_SUBPROGRAM: {
      zP4 = "program";
      ఆపు;
    }
    స్థితి P4_పట్టిక: {
      zP4 = pOp->p4.pTab->zName;
      ఆపు;
    }
    స్థితి P4_సూచీ: {
      zP4 = pOp->p4.pIdx->zName;
      ఆపు;
    }
    స్థితి P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      ఆపు;
    }
    అప్రమేయ: {
      zP4 = pOp->p4.z;
    }
  }
  ఒకవేళ( zP4 ) sqlite3_str_appendall(&x, zP4);
  ఒకవేళ( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  తిరిగి sqlite3_Str_Accum_ముగింపు(&x);
}
#అయితే_ముగింపు /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
శూన్య sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(Vdbe *p, పూర్ణాంకం i){
  ప్రతిజ్ఞా( i>=0 && i<p->db->nDb && i<(పూర్ణాంకం)పరిమాణం(y_Db_ముసుగు)*8 );
  ప్రతిజ్ఞా( i<(పూర్ణాంకం)పరిమాణం(p->btree_ముసుగు)*8 );
  Db_ముసుగు_సమితి(p->btree_ముసుగు, i);
  ఒకవేళ( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    Db_ముసుగు_సమితి(p->తాళం_ముసుగు, i);
  }
}

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE)
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
శూన్య sqlite3VdbeEnter(Vdbe *p){
  పూర్ణాంకం i;
  sqlite3 *db;
  Db *aDb;
  పూర్ణాంకం nDb;
  ఒకవేళ( Db_ముసుగు_All_Zero(p->తాళం_ముసుగు) ) తిరిగి;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  కొరకు(i=0; i<nDb; i++){
    ఒకవేళ( i!=1 && Db_ముసుగు_Test(p->తాళం_ముసుగు,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
స్థిర SQLITE_NOINLINE శూన్య vdbeLeave(Vdbe *p){
  పూర్ణాంకం i;
  sqlite3 *db;
  Db *aDb;
  పూర్ణాంకం nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  కొరకు(i=0; i<nDb; i++){
    ఒకవేళ( i!=1 && Db_ముసుగు_Test(p->తాళం_ముసుగు,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
శూన్య sqlite3VdbeLeave(Vdbe *p){
  ఒకవేళ( Db_ముసుగు_All_Zero(p->తాళం_ముసుగు) ) తిరిగి;  /* The common case */
  vdbeLeave(p);
}
#అయితే_ముగింపు

#ఒకవేళ defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
శూన్య sqlite3VdbePrintOp(FILE *p_బహిర్గతం, పూర్ణాంకం pc, VdbeOp *pOp){
  అక్షరం *zP4;
  అక్షరం *zCom;
  sqlite3 dummyDb;
  స్థిర స్థిరరాశి అక్షరం *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  ఒకవేళ( p_బహిర్గతం==0 ) p_బహిర్గతం = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#లేకపోతే
  zCom = 0;
#అయితే_ముగింపు
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(p_బహిర్గతం, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(p_బహిర్గతం);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3_చివర_Benign_Malloc();
}
#అయితే_ముగింపు

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
స్థిర శూన్య initMemArray(Mem *p, పూర్ణాంకం N, sqlite3 *db, u16 flags){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( N>0 ){
    చేయి{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#నిర్వచితమైతే SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#అయితే_ముగింపు
      p++;
    }అయితే( (--N)>0 );
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
స్థిర శూన్య releaseMemArray(Mem *p, పూర్ణాంకం N){
  ఒకవేళ( p && N ){
    Mem *p_చివర = &p[N];
    sqlite3 *db = p->db;
    ప్రతిజ్ఞా( db!=0 );
    ఒకవేళ( db->pn_బైట్లు_Freed ){
      చేయి{
        ఒకవేళ( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }అయితే( (++p)<p_చివర );
      తిరిగి;
    }
    చేయి{
      ప్రతిజ్ఞా( (&p[1])==p_చివర || p[0].db==p[1].db );
      ప్రతిజ్ఞా( sqlite3VdbeCheckMemInvariants(p) );

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
      ఒకవేళ( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3_Vdbe_చట్రం_Mem_Del );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }లేకపోతే ఒకవేళ( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#నిర్వచితమైతే SQLITE_DEBUG
      లేకపోతే{
        p->flags = MEM_Undefined;
      }
#అయితే_ముగింపు
    }అయితే( (++p)<p_చివర );
  }
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
పూర్ణాంకం sqlite3_Vdbe_చట్రం_Is_Valid(Vdbe_చట్రం *p_చట్రం){
  ఒకవేళ( p_చట్రం->i_చట్రం_Magic!=SQLITE_చట్రం_MAGIC ) తిరిగి 0;
  తిరిగి 1;
}
#అయితే_ముగింపు


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
శూన్య sqlite3_Vdbe_చట్రం_Mem_Del(శూన్య *pArg){
  Vdbe_చట్రం *p_చట్రం = (Vdbe_చట్రం*)pArg;
  ప్రతిజ్ఞా( sqlite3_Vdbe_చట్రం_Is_Valid(p_చట్రం) );
  p_చట్రం->p_మాతృ = p_చట్రం->v->p_Del_చట్రం;
  p_చట్రం->v->p_Del_చట్రం = p_చట్రం;
}

#ఒకవేళ defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
పూర్ణాంకం sqlite3_Vdbe_తదుపరి_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  పూర్ణాంకం e_బహుళకం,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  పూర్ణాంకం *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  పూర్ణాంకం *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  పూర్ణాంకం n_అడ్డువరుస;                            /* Stop when row count reaches this */
  పూర్ణాంకం nSub = 0;                        /* Number of sub-vdbes seen so far */
  Sub_కార్యక్రమం **apSub = 0;              /* Array of sub-vdbes */
  పూర్ణాంకం i;                               /* Next instruction address */
  పూర్ణాంకం rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  పూర్ణాంకం iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  n_అడ్డువరుస = p->nOp;
  ఒకవేళ( pSub!=0 ){
    ఒకవేళ( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/పరిమాణం(Vdbe*);
      apSub = (Sub_కార్యక్రమం **)pSub->z;
    }
    కొరకు(i=0; i<nSub; i++){
      n_అడ్డువరుస += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  అయితే(1){  /* Loop exits via break */
    i = iPc++;
    ఒకవేళ( i>=n_అడ్డువరుస ){
      p->rc = SQLITE_OK;
      rc = SQLITE_పూర్తి;
      ఆపు;
    }
    ఒకవేళ( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }లేకపోతే{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      పూర్ణాంకం j;
      i -= p->nOp;
      ప్రతిజ్ఞా( apSub!=0 );
      ప్రతిజ్ఞా( nSub>0 );
      కొరకు(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        ప్రతిజ్ఞా( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    ఒకవేళ( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      పూర్ణాంకం nByte = (nSub+1)*పరిమాణం(Sub_కార్యక్రమం*);
      పూర్ణాంకం j;
      కొరకు(j=0; j<nSub; j++){
        ఒకవేళ( apSub[j]==aOp[i].p4.p_కార్యక్రమం ) ఆపు;
      }
      ఒకవేళ( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        ఒకవేళ( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          ఆపు;
        }
        apSub = (Sub_కార్యక్రమం **)pSub->z;
        apSub[nSub++] = aOp[i].p4.p_కార్యక్రమం;
        Mem_సమితి_Type_Flag(pSub, MEM_Blob);
        pSub->n = nSub*పరిమాణం(Sub_కార్యక్రమం*);
        n_అడ్డువరుస += aOp[i].p4.p_కార్యక్రమం->nOp;
      }
    }
    ఒకవేళ( e_బహుళకం==0 ) ఆపు;
#నిర్వచితమైతే SQLITE_ENABLE_BYTECODE_VTAB
    ఒకవేళ( e_బహుళకం==2 ){
      Op *pOp = aOp + i;
      ఒకవేళ( pOp->opcode==OP_OpenRead ) ఆపు;
      ఒకవేళ( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) ఆపు;
      ఒకవేళ( pOp->opcode==OP_ReopenIdx ) ఆపు;     
    }లేకపోతే
#అయితే_ముగింపు
    {
      ప్రతిజ్ఞా( e_బహుళకం==1 );
      ఒకవేళ( aOp[i].opcode==OP_Explain ) ఆపు;
      ఒకవేళ( aOp[i].opcode==OP_Init && iPc>1 ) ఆపు;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
శూన్య sqlite3_Vdbe_చట్రం_Delete(Vdbe_చట్రం *p){
  పూర్ణాంకం i;
  Mem *aMem = Vdbe_చట్రం_Mem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->n_బిడ్డ_Mem];
  ప్రతిజ్ఞా( sqlite3_Vdbe_చట్రం_Is_Valid(p) );
  కొరకు(i=0; i<p->n_బిడ్డ_Csr; i++){
    ఒకవేళ( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->n_బిడ్డ_Mem);
  sqlite3_Vdbe_Delete_Aux_దత్తాంశం(p->v->db, &p->p_Aux_దత్తాంశం, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
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
పూర్ణాంకం sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  పూర్ణాంకం i;                               /* Loop counter */
  పూర్ణాంకం rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  పూర్ణాంకం bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  ప్రతిజ్ఞా( p->explain );
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_నిరంతరం_STATE );
  ప్రతిజ్ఞా( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  ఒకవేళ( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    ప్రతిజ్ఞా( p->nMem>9 );
    pSub = &p->aMem[9];
  }లేకపోతే{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_తదుపరి_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  ఒకవేళ( rc==SQLITE_OK ){
    pOp = aOp + i;
    ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
      p->rc = SQLITE_అంతరాయం;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }లేకపోతే{
      అక్షరం *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      ఒకవేళ( p->explain==2 ){
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem, pOp->p1);
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+1, pOp->p2);
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+2, pOp->p3);
        sqlite3_Vdbe_Mem_సమితి_Str(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ప్రతిజ్ఞా( p->n_Res_స్తంభం==4 );
      }లేకపోతే{
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+0, i);
        sqlite3_Vdbe_Mem_సమితి_Str(pMem+1, (అక్షరం*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+2, pOp->p1);
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+3, pOp->p2);
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3_Vdbe_Mem_సమితి_Int64(pMem+6, pOp->p5);
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          అక్షరం *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3_Vdbe_Mem_సమితి_Str(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#లేకపోతే
        sqlite3_Vdbe_Mem_సమితి_Null(pMem+7);
#అయితే_ముగింపు
        sqlite3_Vdbe_Mem_సమితి_Str(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ప్రతిజ్ఞా( p->n_Res_స్తంభం==8 );
      }
      p->p_ఫలితం_అడ్డువరుస = pMem;
      ఒకవేళ( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }లేకపోతే{
        p->rc = SQLITE_OK;
        rc = SQLITE_అడ్డువరుస;
      }
    }
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_EXPLAIN */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
శూన్య sqlite3VdbePrintSql(Vdbe *p){
  స్థిరరాశి అక్షరం *z = 0;
  ఒకవేళ( p->zSql ){
    z = p->zSql;
  }లేకపోతే ఒకవేళ( p->nOp>=1 ){
    స్థిరరాశి VdbeOp *pOp = &p->aOp[0];
    ఒకవేళ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      అయితే( sqlite3Isspace(*z) ) z++;
    }
  }
  ఒకవేళ( z ) రాయి("SQL: [%s]\n", z);
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
శూన్య sqlite3VdbeIOTraceSql(Vdbe *p){
  పూర్ణాంకం nOp = p->nOp;
  VdbeOp *pOp;
  ఒకవేళ( sqlite3IoTrace==0 ) తిరిగి;
  ఒకవేళ( nOp<1 ) తిరిగి;
  pOp = &p->aOp[0];
  ఒకవేళ( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    పూర్ణాంకం i, j;
    అక్షరం z[1000];
    sqlite3_snprintf(పరిమాణం(z), z, "%s", pOp->p4.z);
    కొరకు(i=0; sqlite3Isspace(z[i]); i++){}
    కొరకు(j=0; z[i]; i++){
      ఒకవేళ( sqlite3Isspace(z[i]) ){
        ఒకవేళ( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }లేకపోతే{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#అయితే_ముగింపు /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
నిర్మాణం ReusableSpace {
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
స్థిర శూన్య *allocSpace(
  నిర్మాణం ReusableSpace *p,  /* Bulk memory available for allocation */
  శూన్య *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  ఒకవేళ( pBuf==0 ){
    nByte = ROUND8P(nByte);
    ఒకవేళ( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }లేకపోతే{
      p->nNeeded += nByte;
    }
  }
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pBuf) );
  తిరిగి pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
శూన్య sqlite3VdbeRewind(Vdbe *p){
#ఒకవేళ defined(SQLITE_DEBUG)
  పూర్ణాంకం i;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p!=0 );
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  ప్రతిజ్ఞా( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#నిర్వచితమైతే SQLITE_DEBUG
  కొరకు(i=0; i<p->nMem; i++){
    ప్రతిజ్ఞా( p->aMem[i].db==p->db );
  }
#అయితే_ముగింపు
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#నిర్వచితమైతే VDBE_PROFILE
  కొరకు(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].n_చక్రం = 0;
  }
#అయితే_ముగింపు
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
శూన్య sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  పూర్ణాంకం nVar;                      /* Number of parameters */
  పూర్ణాంకం nMem;                      /* Number of VM memory registers */
  పూర్ణాంకం nCursor;                   /* Number of cursors required */
  పూర్ణాంకం nArg;                      /* Max number args to xFilter or xUpdate */
  పూర్ణాంకం n;                         /* Loop counter */
  నిర్మాణం ReusableSpace x;        /* Reusable bulk memory */

  ప్రతిజ్ఞా( p!=0 );
  ప్రతిజ్ఞా( p->nOp>0 );
  ప్రతిజ్ఞా( pParse!=0 );
  ప్రతిజ్ఞా( p->eVdbeState==VDBE_INIT_STATE );
  ప్రతిజ్ఞా( pParse==p->pParse );
  ప్రతిజ్ఞా( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  ప్రతిజ్ఞా( db->mallocFailed==0 );
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
  ఒకవేళ( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(పరిమాణం(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  ప్రతిజ్ఞా( x.nFree>=0 );
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_విలువలు(p, &nArg);
  p->ఉపయోగిస్తుంది_Stmt_Journal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  ఒకవేళ( pParse->explain ){
    ఒకవేళ( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->n_Res_స్తంభం = 12 - 4*p->explain;
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
  p->aMem = allocSpace(&x, 0, nMem*పరిమాణం(Mem));
  p->aVar = allocSpace(&x, 0, nVar*పరిమాణం(Mem));
  p->apArg = allocSpace(&x, 0, nArg*పరిమాణం(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*పరిమాణం(VdbeCursor*));
  ఒకవేళ( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    ఒకవేళ( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*పరిమాణం(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*పరిమాణం(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*పరిమాణం(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*పరిమాణం(VdbeCursor*));
    }
  }
#నిర్వచితమైతే SQLITE_DEBUG
  p->napArg = nArg;
#అయితే_ముగింపు

  ఒకవేళ( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }లేకపోతే{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*పరిమాణం(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
శూన్య sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  ఒకవేళ( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
స్థిర SQLITE_NOINLINE శూన్య freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  ప్రతిజ్ఞా( pCx->నిలువు_Cache );
  pCx->నిలువు_Cache = 0;
  pCx->pCache = 0;
  ఒకవేళ( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
శూన్య sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  ఒకవేళ( pCx->నిలువు_Cache ){
    freeCursorWithCache(p, pCx);
    తిరిగి;
  }
  ఎంపిక( pCx->eCurType ){
    స్థితి CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      ఆపు;
    }
    స్థితి CURTYPE_BTREE: {
      ప్రతిజ్ఞా( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    స్థితి CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      స్థిరరాశి sqlite3_module *pModule = pVCur->pVtab->pModule;
      ప్రతిజ్ఞా( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      ఆపు;
    }
#అయితే_ముగింపు
  }
}

/*
** Close all cursors in the current frame.
*/
స్థిర శూన్య close_Cursors_లోపల_చట్రం(Vdbe *p){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    ఒకవేళ( pC ){
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
పూర్ణాంకం sqlite3_Vdbe_చట్రం_Restore(Vdbe_చట్రం *p_చట్రం){
  Vdbe *v = p_చట్రం->v;
  close_Cursors_లోపల_చట్రం(v);
  v->aOp = p_చట్రం->aOp;
  v->nOp = p_చట్రం->nOp;
  v->aMem = p_చట్రం->aMem;
  v->nMem = p_చట్రం->nMem;
  v->apCsr = p_చట్రం->apCsr;
  v->nCursor = p_చట్రం->nCursor;
  v->db->చివరి_Rowid = p_చట్రం->చివరి_Rowid;
  v->nChange = p_చట్రం->nChange;
  v->db->nChange = p_చట్రం->nDbChange;
  sqlite3_Vdbe_Delete_Aux_దత్తాంశం(v->db, &v->p_Aux_దత్తాంశం, -1, 0);
  v->p_Aux_దత్తాంశం = p_చట్రం->p_Aux_దత్తాంశం;
  p_చట్రం->p_Aux_దత్తాంశం = 0;
  తిరిగి p_చట్రం->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
స్థిర శూన్య closeAllCursors(Vdbe *p){
  ఒకవేళ( p->p_చట్రం ){
    Vdbe_చట్రం *p_చట్రం;
    కొరకు(p_చట్రం=p->p_చట్రం; p_చట్రం->p_మాతృ; p_చట్రం=p_చట్రం->p_మాతృ);
    sqlite3_Vdbe_చట్రం_Restore(p_చట్రం);
    p->p_చట్రం = 0;
    p->n_చట్రం = 0;
  }
  ప్రతిజ్ఞా( p->n_చట్రం==0 );
  close_Cursors_లోపల_చట్రం(p);
  releaseMemArray(p->aMem, p->nMem);
  అయితే( p->p_Del_చట్రం ){
    Vdbe_చట్రం *pDel = p->p_Del_చట్రం;
    p->p_Del_చట్రం = pDel->p_మాతృ;
    sqlite3_Vdbe_చట్రం_Delete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  ఒకవేళ( p->p_Aux_దత్తాంశం ) sqlite3_Vdbe_Delete_Aux_దత్తాంశం(p->db, &p->p_Aux_దత్తాంశం, -1, 0);
  ప్రతిజ్ఞా( p->p_Aux_దత్తాంశం==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
శూన్య sqlite3_Vdbe_సమితి_అంకె_నిలువుల_సంఖ్య(Vdbe *p, పూర్ణాంకం n_Res_స్తంభం){
  పూర్ణాంకం n;
  sqlite3 *db = p->db;

  ఒకవేళ( p->nResAlloc ){
    releaseMemArray(p->a_నిలువు_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->a_నిలువు_Name);
  }
  n = n_Res_స్తంభం*COLNAME_N;
  p->n_Res_స్తంభం = p->nResAlloc = (u16)n_Res_స్తంభం;
  p->a_నిలువు_Name = (Mem*)sqlite3DbMallocRawNN(db, పరిమాణం(Mem)*n );
  ఒకవేళ( p->a_నిలువు_Name==0 ) తిరిగి;
  initMemArray(p->a_నిలువు_Name, n, db, MEM_Null);
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
పూర్ణాంకం sqlite3_Vdbe_సమితి_నిలువు_Name(
  Vdbe *p,                         /* Vdbe being configured */
  పూర్ణాంకం idx,                         /* Index of column zName applies to */
  పూర్ణాంకం var,                         /* One of the COLNAME_* constants */
  స్థిరరాశి అక్షరం *zName,               /* Pointer to buffer containing name */
  శూన్య (*xDel)(శూన్య*)              /* Memory management strategy for zName */
){
  పూర్ణాంకం rc;
  Mem *p_నిలువు_Name;
  ప్రతిజ్ఞా( idx<p->nResAlloc );
  ప్రతిజ్ఞా( var<COLNAME_N );
  ఒకవేళ( p->db->mallocFailed ){
    ప్రతిజ్ఞా( !zName || xDel!=SQLITE_DYNAMIC );
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( p->a_నిలువు_Name!=0 );
  p_నిలువు_Name = &(p->a_నిలువు_Name[idx+var*p->nResAlloc]);
  rc = sqlite3_Vdbe_Mem_సమితి_పాఠ్యం(p_నిలువు_Name, zName, -1, xDel);
  ప్రతిజ్ఞా( rc!=0 || !zName || (p_నిలువు_Name->flags&MEM_పదం)!=0 );
  తిరిగి rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
స్థిర పూర్ణాంకం vdbeCommit(sqlite3 *db, Vdbe *p){
  పూర్ణాంకం i;
  పూర్ణాంకం nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం needXcommit = 0;

#నిర్వచితమైతే SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  వాడని_PARAMETER(p);
#అయితే_ముగింపు

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
  కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      స్థిర స్థిరరాశి u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /*జ్ఞాపకం*/ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      ఒకవేళ( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3_Pager_Get_Journal_బహుళకం(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        ప్రతిజ్ఞా( i!=1 );
        nTrans++;
      }
      rc = sqlite3_Pager_Exclusive_తాళం(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  ఒకవేళ( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    ఒకవేళ( rc ){
      తిరిగి SQLITE_CONSTRAINT_COMMITHOOK;
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
  ఒకవేళ( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    ఒకవేళ( needXcommit ){
      కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ఒకవేళ( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      పూర్ణాంకం txn = sqlite3BtreeTxnState(pBt);
      ఒకవేళ( txn!=SQLITE_TXN_NONE ){
        ప్రతిజ్ఞా( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DISKIO
  లేకపోతే{
    sqlite3_vfs *pVfs = db->pVfs;
    అక్షరం *zSuper = 0;   /* File-name for the super-journal */
    అక్షరం స్థిరరాశి *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 స్థానభ్రంశం = 0;
    పూర్ణాంకం res;
    పూర్ణాంకం retry_లెక్క = 0;
    పూర్ణాంకం nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    ఒకవేళ( zSuper==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    zSuper += 4;
    చేయి {
      u32 iRandom;
      ఒకవేళ( retry_లెక్క ){
        ఒకవేళ( retry_లెక్క>100 ){
          sqlite3_log(SQLITE_నిండిన, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          ఆపు;
        }లేకపోతే ఒకవేళ( retry_లెక్క==1 ){
          sqlite3_log(SQLITE_నిండిన, "MJ collide: %s", zSuper);
        }
      }
      retry_లెక్క++;
      sqlite3_randomness(పరిమాణం(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      ప్రతిజ్ఞా( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }అయితే( rc==SQLITE_OK && res );
    ఒకవేళ( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      తిరిగి rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    కొరకు(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ఒకవేళ( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        అక్షరం స్థిరరాశి *zFile = sqlite3BtreeGetJournalname(pBt);
        ఒకవేళ( zFile==0 ){
          కొనసాగు;  /* Ignore TEMP and :memory: databases */
        }
        ప్రతిజ్ఞా( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,స్థానభ్రంశం);
        స్థానభ్రంశం += sqlite3Strlen30(zFile)+1;
        ఒకవేళ( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          తిరిగి rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    ఒకవేళ( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_సాధారణ))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      తిరిగి rc;
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
    కొరకు(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ఒకవేళ( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    ప్రతిజ్ఞా( rc!=SQLITE_BUSY );
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      తిరిగి rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    ఒకవేళ( rc ){
      తిరిగి rc;
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
    కొరకు(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      ఒకవేళ( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3_చివర_Benign_Malloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#అయితే_ముగింపు

  తిరిగి rc;
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
#నిర్వచితం_కాకపోతే NDEBUG
స్థిర శూన్య check_చురుకైన_Vdbe_Cnt(sqlite3 *db){
  Vdbe *p;
  పూర్ణాంకం cnt = 0;
  పూర్ణాంకం nWrite = 0;
  పూర్ణాంకం nRead = 0;
  p = db->pVdbe;
  అయితే( p ){
    ఒకవేళ( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      ఒకవేళ( p->readOnly==0 ) nWrite++;
      ఒకవేళ( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  ప్రతిజ్ఞా( cnt==db->n_Vdbe_చురుకైన );
  ప్రతిజ్ఞా( nWrite==db->nVdbeWrite );
  ప్రతిజ్ఞా( nRead==db->nVdbeRead );
}
#లేకపోతే
#నిర్వచించు check_చురుకైన_Vdbe_Cnt(x)
#అయితే_ముగింపు

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
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbeCloseStatement(Vdbe *p, పూర్ణాంకం eOp){
  sqlite3 *స్థిరరాశి db = p->db;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i;
  స్థిరరాశి పూర్ణాంకం iSavepoint = p->iStatement-1;

  ప్రతిజ్ఞా( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  ప్రతిజ్ఞా( db->nStatement>0 );
  ప్రతిజ్ఞా( p->iStatement==(db->nStatement+db->nSavepoint) );

  కొరకు(i=0; i<db->nDb; i++){
    పూర్ణాంకం rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    ఒకవేళ( pBt ){
      ఒకవేళ( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      ఒకవేళ( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    ఒకవేళ( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  ఒకవేళ( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3VdbeCloseStatement(Vdbe *p, పూర్ణాంకం eOp){
  ఒకవేళ( p->db->nStatement && p->iStatement ){
    తిరిగి vdbeCloseStatement(p, eOp);
  }
  తిరిగి SQLITE_OK;
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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  ఒకవేళ( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) తిరిగి SQLITE_ERROR;
  తిరిగి SQLITE_CONSTRAINT_FOREIGNKEY;
}
పూర్ణాంకం sqlite3_Vdbe_Check_Fk_తక్షణ(Vdbe *p){
  ఒకవేళ( p->nFkConstraint==0 ) తిరిగి SQLITE_OK;
  తిరిగి vdbeFkError(p);
}
పూర్ణాంకం sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  ఒకవేళ( (db->nDeferredCons+db->nDeferredImmCons)==0 ) తిరిగి SQLITE_OK;
  తిరిగి vdbeFkError(p);
}
#అయితే_ముగింపు

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
పూర్ణాంకం sqlite3VdbeHalt(Vdbe *p){
  పూర్ణాంకం rc;                         /* Used to store transient return codes */
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

  ప్రతిజ్ఞా( p->eVdbeState==VDBE_నిరంతరం_STATE );
  ఒకవేళ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  check_చురుకైన_Vdbe_Cnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  ఒకవేళ( p->bIsReader ){
    పూర్ణాంకం mrc;   /* Primary error code from p->rc */
    పూర్ణాంకం eStatementOp = 0;
    పూర్ణాంకం isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    ఒకవేళ( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_అంతరాయం
                    || mrc==SQLITE_నిండిన;
    }లేకపోతే{
      mrc = isSpecialError = 0;
    }
    ఒకవేళ( isSpecialError ){
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
      ఒకవేళ( !p->readOnly || mrc!=SQLITE_అంతరాయం ){
        ఒకవేళ( (mrc==SQLITE_NOMEM || mrc==SQLITE_నిండిన) && p->ఉపయోగిస్తుంది_Stmt_Journal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }లేకపోతే{
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
    ఒకవేళ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (శూన్య)sqlite3_Vdbe_Check_Fk_తక్షణ(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    ఒకవేళ( !sqlite3_Vtab_లోపల_Sync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      ఒకవేళ( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        ఒకవేళ( rc!=SQLITE_OK ){
          ఒకవేళ( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            తిరిగి SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }లేకపోతే ఒకవేళ( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }లేకపోతే{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        ఒకవేళ( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          తిరిగి SQLITE_BUSY;
        }లేకపోతే ఒకవేళ( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }లేకపోతే{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3_Commit_అంతర్గత_Changes(db);
        }
      }లేకపోతే ఒకవేళ( p->rc==SQLITE_SCHEMA && db->n_Vdbe_చురుకైన>1 ){
        p->nChange = 0;
      }లేకపోతే{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }లేకపోతే ఒకవేళ( eStatementOp==0 ){
      ఒకవేళ( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }లేకపోతే ఒకవేళ( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }లేకపోతే{
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
    ఒకవేళ( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      ఒకవేళ( rc ){
        ఒకవేళ( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_సందేశం);
          p->z_Err_సందేశం = 0;
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
    ఒకవేళ( p->changeCntOn ){
      ఒకవేళ( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3_Vdbe_సమితి_Changes(db, p->nChange);
      }లేకపోతే{
        sqlite3_Vdbe_సమితి_Changes(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->n_Vdbe_చురుకైన--;
  ఒకవేళ( !p->readOnly ) db->nVdbeWrite--;
  ఒకవేళ( p->bIsReader ) db->nVdbeRead--;
  ప్రతిజ్ఞా( db->n_Vdbe_చురుకైన>=db->nVdbeRead );
  ప్రతిజ్ఞా( db->nVdbeRead>=db->nVdbeWrite );
  ప్రతిజ్ఞా( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  check_చురుకైన_Vdbe_Cnt(db);
  ఒకవేళ( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  ఒకవేళ( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  ప్రతిజ్ఞా( db->n_Vdbe_చురుకైన>0 || db->autoCommit==0 || db->nStatement==0 );
  తిరిగి (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
శూన్య sqlite3_Vdbe_Reset_అడుగు_ఫలితం(Vdbe *p){
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
పూర్ణాంకం sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  పూర్ణాంకం rc = p->rc;
  ఒకవేళ( p->z_Err_సందేశం ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    ఒకవేళ( db->pErr==0 ) db->pErr = sqlite3_విలువ_New(db);
    sqlite3_విలువ_సమితి_Str(db->pErr, -1, p->z_Err_సందేశం, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3_చివర_Benign_Malloc();
    db->bBenignMalloc--;
  }లేకపోతే ఒకవేళ( db->pErr ){
    sqlite3_విలువ_సమితి_Null(db->pErr);
  }
  db->err_సంకేతం = rc;
  db->err_Byte_స్థానభ్రంశం = -1;
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
స్థిర శూన్య vdbeInvokeSqllog(Vdbe *v){
  ఒకవేళ( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    అక్షరం *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    ప్రతిజ్ఞా( v->db->init.busy==0 );
    ఒకవేళ( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#లేకపోతే
# define vdbeInvokeSqllog(x)
#అయితే_ముగింపు

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
పూర్ణాంకం sqlite3VdbeReset(Vdbe *p){
#ఒకవేళ defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  పూర్ణాంకం i;
#అయితే_ముగింపు

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  ఒకవేళ( p->eVdbeState==VDBE_నిరంతరం_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  ఒకవేళ( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    ఒకవేళ( db->pErr || p->z_Err_సందేశం ){
      sqlite3VdbeTransferError(p);
    }లేకపోతే{
      db->err_సంకేతం = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#నిర్వచితమైతే SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  ఒకవేళ( p->apCsr ) కొరకు(i=0; i<p->nCursor; i++) ప్రతిజ్ఞా( p->apCsr[i]==0 );
  ఒకవేళ( p->aMem ){
    కొరకు(i=0; i<p->nMem; i++) ప్రతిజ్ఞా( p->aMem[i].flags==MEM_Undefined );
  }
#అయితే_ముగింపు
  ఒకవేళ( p->z_Err_సందేశం ){
    sqlite3DbFree(db, p->z_Err_సందేశం);
    p->z_Err_సందేశం = 0;
  }
  p->p_ఫలితం_అడ్డువరుస = 0;
#నిర్వచితమైతే SQLITE_DEBUG
  p->nWrite = 0;
#అయితే_ముగింపు

  /* Save profiling information from this VDBE run.
  */
#నిర్వచితమైతే VDBE_PROFILE
  {
    FILE *బహిర్గతం = fopen("vdbe_profile.out", "a");
    ఒకవేళ( బహిర్గతం ){
      fprintf(బహిర్గతం, "---- ");
      కొరకు(i=0; i<p->nOp; i++){
        fprintf(బహిర్గతం, "%02x", p->aOp[i].opcode);
      }
      fprintf(బహిర్గతం, "\n");
      ఒకవేళ( p->zSql ){
        అక్షరం c, pc = 0;
        fprintf(బహిర్గతం, "-- ");
        కొరకు(i=0; (c = p->zSql[i])!=0; i++){
          ఒకవేళ( pc=='\n' ) fprintf(బహిర్గతం, "-- ");
          putc(c, బహిర్గతం);
          pc = c;
        }
        ఒకవేళ( pc!='\n' ) fprintf(బహిర్గతం, "\n");
      }
      కొరకు(i=0; i<p->nOp; i++){
        అక్షరం zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 చక్రాలు = p->aOp[i].n_చక్రం;
        sqlite3_snprintf(పరిమాణం(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           చక్రాలు,
           cnt>0 ? చక్రాలు/cnt : 0
        );
        fprintf(బహిర్గతం, "%s", zHdr);
        sqlite3VdbePrintOp(బహిర్గతం, i, &p->aOp[i]);
      }
      fclose(బహిర్గతం);
    }
  }
#అయితే_ముగింపు
  తిరిగి p->rc & db->err_ముసుగు;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
పూర్ణాంకం sqlite3VdbeFinalize(Vdbe *p){
  పూర్ణాంకం rc = SQLITE_OK;
  ప్రతిజ్ఞా( VDBE_నిరంతరం_STATE>VDBE_READY_STATE );
  ప్రతిజ్ఞా( VDBE_HALT_STATE>VDBE_READY_STATE );
  ప్రతిజ్ఞా( VDBE_INIT_STATE<VDBE_READY_STATE );
  ఒకవేళ( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    ప్రతిజ్ఞా( (rc & p->db->err_ముసుగు)==rc );
  }
  sqlite3VdbeDelete(p);
  తిరిగి rc;
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
శూన్య sqlite3_Vdbe_Delete_Aux_దత్తాంశం(sqlite3 *db, Aux_దత్తాంశం **pp, పూర్ణాంకం iOp, పూర్ణాంకం ముసుగు){
  అయితే( *pp ){
    Aux_దత్తాంశం *pAux = *pp;
    ఒకవేళ( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(ముసుగు & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      ఒకవేళ( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_తదుపరి_Aux;
      sqlite3DbFree(db, pAux);
    }లేకపోతే{
      pp= &pAux->p_తదుపరి_Aux;
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
స్థిర శూన్య sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  Sub_కార్యక్రమం *pSub, *p_తదుపరి;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( p->db==0 || p->db==db );
  ఒకవేళ( p->a_నిలువు_Name ){
    releaseMemArray(p->a_నిలువు_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->a_నిలువు_Name);
  }
  కొరకు(pSub=p->p_కార్యక్రమం; pSub; pSub=p_తదుపరి){
    p_తదుపరి = pSub->p_తదుపరి;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  ఒకవేళ( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    ఒకవేళ( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    ఒకవేళ( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  ఒకవేళ( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    కొరకు(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_తదుపరి_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  {
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->n_పరిశీలన; i++){
      sqlite3DbFree(db, p->a_పరిశీలన[i].zName);
    }
    sqlite3DbFree(db, p->a_పరిశీలన);
  }
#అయితే_ముగింపు
}

/*
** Delete an entire VDBE.
*/
శూన్య sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  ప్రతిజ్ఞా( p!=0 );
  db = p->db;
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  ఒకవేళ( db->pn_బైట్లు_Freed==0 ){
    ప్రతిజ్ఞా( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    ఒకవేళ( p->pVNext ){
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
పూర్ణాంకం SQLITE_NOINLINE sqlite3_Vdbe_ముగింపు_Moveto(VdbeCursor *p){
  పూర్ణాంకం res, rc;
#నిర్వచితమైతే SQLITE_TEST
  బాహ్య పూర్ణాంకం sqlite3_శోధన_లెక్క;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p->deferredMoveto );
  ప్రతిజ్ఞా( p->is_పట్టిక );
  ప్రతిజ్ఞా( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3_Btree_పట్టిక_Moveto(p->uc.pCursor, p->moveto_లక్ష్యం, 0, &res);
  ఒకవేళ( rc ) తిరిగి rc;
  ఒకవేళ( res!=0 ) తిరిగి SQLITE_CORRUPT_BKPT;
#నిర్వచితమైతే SQLITE_TEST
  sqlite3_శోధన_లెక్క++;
#అయితే_ముగింపు
  p->deferredMoveto = 0;
  p->cache_స్థితి = CACHE_STALE;
  తిరిగి SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
పూర్ణాంకం SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  పూర్ణాంకం is_Different_అడ్డువరుస, rc;
  ప్రతిజ్ఞా( p->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( p->uc.pCursor!=0 );
  ప్రతిజ్ఞా( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &is_Different_అడ్డువరుస);
  p->cache_స్థితి = CACHE_STALE;
  ఒకవేళ( is_Different_అడ్డువరుస ) p->null_అడ్డువరుస = 1;
  తిరిగి rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
పూర్ణాంకం sqlite3VdbeCursorRestore(VdbeCursor *p){
  ప్రతిజ్ఞా( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  ఒకవేళ( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    తిరిగి sqlite3VdbeHandleMovedCursor(p);
  }
  తిరిగి SQLITE_OK;
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

#ఒకవేళ 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, పూర్ణాంకం file_format, u32 *pLen){
  పూర్ణాంకం flags = pMem->flags;
  u32 n;

  ప్రతిజ్ఞా( pLen!=0 );
  ఒకవేళ( flags&MEM_Null ){
    *pLen = 0;
    తిరిగి 0;
  }
  ఒకవేళ( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    ఒకవేళ( i<0 ){
      u = ~i;
    }లేకపోతే{
      u = i;
    }
    ఒకవేళ( u<=127 ){
      ఒకవేళ( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        తిరిగి 8+(u32)u;
      }లేకపోతే{
        *pLen = 1;
        తిరిగి 1;
      }
    }
    ఒకవేళ( u<=32767 ){ *pLen = 2; తిరిగి 2; }
    ఒకవేళ( u<=8388607 ){ *pLen = 3; తిరిగి 3; }
    ఒకవేళ( u<=2147483647 ){ *pLen = 4; తిరిగి 4; }
    ఒకవేళ( u<=MAX_6BYTE ){ *pLen = 6; తిరిగి 5; }
    *pLen = 8;
    ఒకవేళ( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (ద్విగుణ)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      తిరిగి 7;
    }
    తిరిగి 6;
  }
  ఒకవేళ( flags&MEM_Real ){
    *pLen = 8;
    తిరిగి 7;
  }
  ప్రతిజ్ఞా( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  ప్రతిజ్ఞా( pMem->n>=0 );
  n = (u32)pMem->n;
  ఒకవేళ( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  తిరిగి ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#అయితే_ముగింపు /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
స్థిరరాశి u8 sqlite3SmallTypeSizes[128] = {
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
  ఒకవేళ( serial_type>=128 ){
    తిరిగి (serial_type-12)/2;
  }లేకపోతే{
    ప్రతిజ్ఞా( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    తిరిగి sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  ప్రతిజ్ఞా( serial_type<128 );
  తిరిగి sqlite3SmallTypeSizes[serial_type]; 
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
#నిర్వచితమైతే SQLITE_MIXED_ENDIAN_64_బిట్టు_FLOAT
u64 sqlite3_Float_మార్పిడి(u64 లోపల){
  సంఘం {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = లోపల;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  తిరిగి u.r;
}
#అయితే_ముగింపు /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
స్థిర పూర్ణాంకం sqlite3VdbeSerialGet7(
  స్థిరరాశి గుర్తులేని అక్షరం *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  ప్రతిజ్ఞా( పరిమాణం(x)==8 && పరిమాణం(pMem->u.r)==8 );
  మార్పిడి_Mixed_Endian_Float(x);
  జ్ఞాపకం_నకలు(&pMem->u.r, &x, పరిమాణం(x));
  ఒకవేళ( IsNaN(x) ){
    pMem->flags = MEM_Null;
    తిరిగి 1;
  }
  pMem->flags = MEM_Real;
  తిరిగి 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
శూన్య sqlite3VdbeSerialGet(
  స్థిరరాశి గుర్తులేని అక్షరం *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  ఎంపిక( serial_type ){
    స్థితి 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      తిరిగి;
    }
    స్థితి 11:   /* Reserved for future use */
    స్థితి 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      తిరిగి;
    }
    స్థితి 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#నిర్వచితమైతే __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      ఒకవేళ( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#అయితే_ముగింపు
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      తిరిగి;
    }
    స్థితి 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      తిరిగి;
    }
    స్థితి 8:    /* Integer 0 */
    స్థితి 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      తిరిగి;
    }
    అప్రమేయ: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      స్థిర స్థిరరాశి u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (అక్షరం *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      తిరిగి;
    }
  }
  తిరిగి;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
Unpacked_రికార్డు *sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(
  కీలకం_Info *p_కీలకం_Info               /* Description of the record */
){
  Unpacked_రికార్డు *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  ప్రతిజ్ఞా( పరిమాణం(Unpacked_రికార్డు) + పరిమాణం(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(పరిమాణం(Unpacked_రికార్డు)) + పరిమాణం(Mem)*(p_కీలకం_Info->n_కీలకం_క్షేత్రం+1);
  p = (Unpacked_రికార్డు *)sqlite3DbMallocRaw(p_కీలకం_Info->db, nByte);
  ఒకవేళ( !p ) తిరిగి 0;
  p->aMem = (Mem*)&((అక్షరం*)p)[ROUND8P(పరిమాణం(Unpacked_రికార్డు))];
  p->p_కీలకం_Info = p_కీలకం_Info;
  p->n_క్షేత్రం = p_కీలకం_Info->n_కీలకం_క్షేత్రం + 1;
  తిరిగి p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
శూన్య sqlite3_Vdbe_రికార్డు_Unpack(
  పూర్ణాంకం n_కీలకం,              /* Size of the binary record */
  స్థిరరాశి శూన్య *p_కీలకం,      /* The binary record */
  Unpacked_రికార్డు *p      /* Populate this structure before returning. */
){
  స్థిరరాశి గుర్తులేని అక్షరం *a_కీలకం = (స్థిరరాశి గుర్తులేని అక్షరం *)p_కీలకం;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  కీలకం_Info *p_కీలకం_Info = p->p_కీలకం_Info;

  p->default_rc = 0;
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_కీలకం, szHdr);
  d = szHdr;
  u = 0;
  అయితే( idx<szHdr && d<=(u32)n_కీలకం ){
    u32 serial_type;

    idx += getVarint32(&a_కీలకం[idx], serial_type);
    pMem->enc = p_కీలకం_Info->enc;
    pMem->db = p_కీలకం_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_కీలకం[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    ఒకవేళ( (++u)>=p->n_క్షేత్రం ) ఆపు;
    pMem++;
  }
  ఒకవేళ( d>(u32)n_కీలకం && u ){
    ప్రతిజ్ఞా( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3_Vdbe_Mem_సమితి_Null(pMem-(u<p->n_క్షేత్రం));
  }
  testcase( u == p_కీలకం_Info->n_కీలకం_క్షేత్రం + 1 );
  testcase( u < p_కీలకం_Info->n_కీలకం_క్షేత్రం + 1 );
  ప్రతిజ్ఞా( u<=p_కీలకం_Info->n_కీలకం_క్షేత్రం + 1 );
  p->n_క్షేత్రం = u;
}

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర పూర్ణాంకం vdbe_రికార్డు_Compare_Debug(
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1, /* Left key */
  స్థిరరాశి Unpacked_రికార్డు *pPKey2, /* Right key */
  పూర్ణాంకం desired_ఫలితం             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  పూర్ణాంకం i = 0;
  పూర్ణాంకం rc = 0;
  స్థిరరాశి గుర్తులేని అక్షరం *aKey1 = (స్థిరరాశి గుర్తులేని అక్షరం *)pKey1;
  కీలకం_Info *p_కీలకం_Info;
  Mem mem1;

  p_కీలకం_Info = pPKey2->p_కీలకం_Info;
  ఒకవేళ( p_కీలకం_Info->db==0 ) తిరిగి 1;
  mem1.enc = p_కీలకం_Info->enc;
  mem1.db = p_కీలకం_Info->db;
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
  ఒకవేళ( szHdr1>98307 ) తిరిగి SQLITE_CORRUPT;
  d1 = szHdr1;
  ప్రతిజ్ఞా( p_కీలకం_Info->n_All_క్షేత్రం>=pPKey2->n_క్షేత్రం || CORRUPT_DB );
  ప్రతిజ్ఞా( p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  ప్రతిజ్ఞా( p_కీలకం_Info->n_కీలకం_క్షేత్రం>0 );
  ప్రతిజ్ఞా( idx1<=szHdr1 || CORRUPT_DB );
  చేయి{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    ఒకవేళ( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      ఒకవేళ( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        తిరిగి 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      ఆపు;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_కీలకం_Info->n_All_క్షేత్రం>i ? p_కీలకం_Info->aColl[i] : 0);
    ఒకవేళ( rc!=0 ){
      ప్రతిజ్ఞా( mem1.szMalloc==0 );  /* See comment below */
      ఒకవేళ( (p_కీలకం_Info->a_క్రమీకరించు_Flags[i] & KEYINFO_క్రమం_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      ఒకవేళ( p_కీలకం_Info->a_క్రమీకరించు_Flags[i] & KEYINFO_క్రమం_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      వెళ్ళు debug_Compare_చివర;
    }
    i++;
  }అయితే( idx1<szHdr1 && i<pPKey2->n_క్షేత్రం );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  ప్రతిజ్ఞా( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debug_Compare_చివర:
  ఒకవేళ( desired_ఫలితం==0 && rc==0 ) తిరిగి 1;
  ఒకవేళ( desired_ఫలితం<0 && rc<0 ) తిరిగి 1;
  ఒకవేళ( desired_ఫలితం>0 && rc>0 ) తిరిగి 1;
  ఒకవేళ( CORRUPT_DB ) తిరిగి 1;
  ఒకవేళ( p_కీలకం_Info->db->mallocFailed ) తిరిగి 1;
  తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర శూన్య vdbe_Assert_క్షేత్రం_లెక్క_Within_Limits(
  పూర్ణాంకం n_కీలకం, స్థిరరాశి శూన్య *p_కీలకం,   /* The record to verify */
  స్థిరరాశి కీలకం_Info *p_కీలకం_Info       /* Compare size with this KeyInfo */
){
  పూర్ణాంకం n_క్షేత్రం = 0;
  u32 szHdr;
  u32 idx;
  u32 not_వాడిన;
  స్థిరరాశి గుర్తులేని అక్షరం *a_కీలకం = (స్థిరరాశి గుర్తులేని అక్షరం*)p_కీలకం;

  ఒకవేళ( CORRUPT_DB ) తిరిగి;
  idx = getVarint32(a_కీలకం, szHdr);
  ప్రతిజ్ఞా( n_కీలకం>=0 );
  ప్రతిజ్ఞా( szHdr<=(u32)n_కీలకం );
  అయితే( idx<szHdr ){
    idx += getVarint32(a_కీలకం+idx, not_వాడిన);
    n_క్షేత్రం++;
  }
  ప్రతిజ్ఞా( n_క్షేత్రం <= p_కీలకం_Info->n_All_క్షేత్రం );
}
#లేకపోతే
# define vdbe_Assert_క్షేత్రం_లెక్క_Within_Limits(A,B,C)
#అయితే_ముగింపు

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbeCompareMemStringWithEncodingChange(
  స్థిరరాశి Mem *pMem1,
  స్థిరరాశి Mem *pMem2,
  స్థిరరాశి CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  పూర్ణాంకం rc;
  స్థిరరాశి శూన్య *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_విలువ_పాఠ్యం_2((sqlite3_విలువ*)&c1, pColl->enc);
  v2 = sqlite3_విలువ_పాఠ్యం_2((sqlite3_విలువ*)&c2, pColl->enc);
  ఒకవేళ( (v1==0 || v2==0) ){
    ఒకవేళ( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }లేకపోతే{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  తిరిగి rc;
}
స్థిర పూర్ణాంకం vdbeCompareMemString(
  స్థిరరాశి Mem *pMem1,
  స్థిరరాశి Mem *pMem2,
  స్థిరరాశి CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  ఒకవేళ( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    తిరిగి pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }లేకపోతే{
    తిరిగి vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
స్థిర పూర్ణాంకం isAllZero(స్థిరరాశి అక్షరం *z, పూర్ణాంకం n){
  పూర్ణాంకం i;
  కొరకు(i=0; i<n; i++){
    ఒకవేళ( z[i] ) తిరిగి 0;
  }
  తిరిగి 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE పూర్ణాంకం sqlite3BlobCompare(స్థిరరాశి Mem *pB1, స్థిరరాశి Mem *pB2){
  పూర్ణాంకం c;
  పూర్ణాంకం n1 = pB1->n;
  పూర్ణాంకం n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  ప్రతిజ్ఞా( (pB1->flags & MEM_Zero)==0 || n1==0 );
  ప్రతిజ్ఞా( (pB2->flags & MEM_Zero)==0 || n2==0 );

  ఒకవేళ( (pB1->flags|pB2->flags) & MEM_Zero ){
    ఒకవేళ( pB1->flags & pB2->flags & MEM_Zero ){
      తిరిగి pB1->u.nZero - pB2->u.nZero;
    }లేకపోతే ఒకవేళ( pB1->flags & MEM_Zero ){
      ఒకవేళ( !isAllZero(pB2->z, pB2->n) ) తిరిగి -1;
      తిరిగి pB1->u.nZero - n2;
    }లేకపోతే{
      ఒకవేళ( !isAllZero(pB1->z, pB1->n) ) తిరిగి +1;
      తిరిగి n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  ఒకవేళ( c ) తిరిగి c;
  తిరిగి n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#ఒకవేళ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
స్థిర పూర్ణాంకం SQLITE_NOINLINE doubleLt(ద్విగుణ a, ద్విగుణ b){ తిరిగి a<b; }
స్థిర పూర్ణాంకం SQLITE_NOINLINE doubleEq(ద్విగుణ a, ద్విగుణ b){ తిరిగి a==b; }
#అయితే_ముగింపు

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
పూర్ణాంకం sqlite3IntFloatCompare(i64 i, ద్విగుణ r){
  ఒకవేళ( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    తిరిగి 1;
  }లేకపోతే{
    i64 y;
    ఒకవేళ( r<-9223372036854775808.0 ) తిరిగి +1;
    ఒకవేళ( r>=9223372036854775808.0 ) తిరిగి -1;
    y = (i64)r;
    ఒకవేళ( i<y ) తిరిగి -1;
    ఒకవేళ( i>y ) తిరిగి +1;
    testcase( doubleLt(((ద్విగుణ)i),r) );
    testcase( doubleLt(r,((ద్విగుణ)i)) );
    testcase( doubleEq(r,((ద్విగుణ)i)) );
    తిరిగి (((ద్విగుణ)i)<r) ? -1 : (((ద్విగుణ)i)>r);
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
పూర్ణాంకం sqlite3MemCompare(స్థిరరాశి Mem *pMem1, స్థిరరాశి Mem *pMem2, స్థిరరాశి CollSeq *pColl){
  పూర్ణాంకం f1, f2;
  పూర్ణాంకం combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  ప్రతిజ్ఞా( !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem1) && !sqlite3_Vdbe_Mem_Is_అడ్డువరుస_సమితి(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  ఒకవేళ( combined_flags&MEM_Null ){
    తిరిగి (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  ఒకవేళ( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    ఒకవేళ( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      ఒకవేళ( pMem1->u.i < pMem2->u.i ) తిరిగి -1;
      ఒకవేళ( pMem1->u.i > pMem2->u.i ) తిరిగి +1;
      తిరిగి 0;
    }
    ఒకవేళ( (f1 & f2 & MEM_Real)!=0 ){
      ఒకవేళ( pMem1->u.r < pMem2->u.r ) తిరిగి -1;
      ఒకవేళ( pMem1->u.r > pMem2->u.r ) తిరిగి +1;
      తిరిగి 0;
    }
    ఒకవేళ( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      ఒకవేళ( (f2&MEM_Real)!=0 ){
        తిరిగి sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }లేకపోతే ఒకవేళ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        ఒకవేళ( pMem1->u.i < pMem2->u.i ) తిరిగి -1;
        ఒకవేళ( pMem1->u.i > pMem2->u.i ) తిరిగి +1;
        తిరిగి 0;
      }లేకపోతే{
        తిరిగి -1;
      }
    }
    ఒకవేళ( (f1&MEM_Real)!=0 ){
      ఒకవేళ( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        తిరిగి -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }లేకపోతే{
        తిరిగి -1;
      }
    }
    తిరిగి +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  ఒకవేళ( combined_flags&MEM_Str ){
    ఒకవేళ( (f1 & MEM_Str)==0 ){
      తిరిగి 1;
    }
    ఒకవేళ( (f2 & MEM_Str)==0 ){
      తిరిగి -1;
    }

    ప్రతిజ్ఞా( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    ప్రతిజ్ఞా( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    ప్రతిజ్ఞా( !pColl || pColl->xCmp );

    ఒకవేళ( pColl ){
      తిరిగి vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  తిరిగి sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
స్థిర i64 vdbe_రికార్డు_Decode_Int(u32 serial_type, స్థిరరాశి u8 *a_కీలకం){
  u32 y;
  ప్రతిజ్ఞా( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  ఎంపిక( serial_type ){
    స్థితి 0:
    స్థితి 1:
      testcase( a_కీలకం[0]&0x80 );
      తిరిగి ONE_BYTE_INT(a_కీలకం);
    స్థితి 2:
      testcase( a_కీలకం[0]&0x80 );
      తిరిగి TWO_BYTE_INT(a_కీలకం);
    స్థితి 3:
      testcase( a_కీలకం[0]&0x80 );
      తిరిగి THREE_BYTE_INT(a_కీలకం);
    స్థితి 4: {
      testcase( a_కీలకం[0]&0x80 );
      y = FOUR_BYTE_UINT(a_కీలకం);
      తిరిగి (i64)*(పూర్ణాంకం*)&y;
    }
    స్థితి 5: {
      testcase( a_కీలకం[0]&0x80 );
      తిరిగి FOUR_BYTE_UINT(a_కీలకం+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_కీలకం);
    }
    స్థితి 6: {
      u64 x = FOUR_BYTE_UINT(a_కీలకం);
      testcase( a_కీలకం[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_కీలకం+4);
      తిరిగి (i64)*(i64*)&x;
    }
  }

  తిరిగి (serial_type - 8);
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
పూర్ణాంకం sqlite3_Vdbe_రికార్డు_Compare_With_Skip(
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1,   /* Left key */
  Unpacked_రికార్డు *pPKey2,         /* Right key */
  పూర్ణాంకం bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  పూర్ణాంకం i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  పూర్ణాంకం rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  కీలకం_Info *p_కీలకం_Info;
  స్థిరరాశి గుర్తులేని అక్షరం *aKey1 = (స్థిరరాశి గుర్తులేని అక్షరం *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  ఒకవేళ( bSkip ){
    u32 s1 = aKey1[1];
    ఒకవేళ( s1<0x80 ){
      idx1 = 2;
    }లేకపోతే{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }లేకపోతే{
    ఒకవేళ( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }లేకపోతే{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  ఒకవేళ( d1>(గుర్తులేని)nKey1 ){
    pPKey2->err_సంకేతం = (u8)SQLITE_CORRUPT_BKPT;
    తిరిగి 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  ప్రతిజ్ఞా( pPKey2->p_కీలకం_Info->n_All_క్షేత్రం>=pPKey2->n_క్షేత్రం
       || CORRUPT_DB );
  ప్రతిజ్ఞా( pPKey2->p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  ప్రతిజ్ఞా( pPKey2->p_కీలకం_Info->n_కీలకం_క్షేత్రం>0 );
  ప్రతిజ్ఞా( idx1<=szHdr1 || CORRUPT_DB );
  అయితే( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    ఒకవేళ( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      ఒకవేళ( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }లేకపోతే ఒకవేళ( serial_type==0 ){
        rc = -1;
      }లేకపోతే ఒకవేళ( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }లేకపోతే{
        i64 lhs = vdbe_రికార్డు_Decode_Int(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        ఒకవేళ( lhs<rhs ){
          rc = -1;
        }లేకపోతే ఒకవేళ( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    లేకపోతే ఒకవేళ( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      ఒకవేళ( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }లేకపోతే ఒకవేళ( serial_type==0 ){
        rc = -1;
      }లేకపోతే{
        ఒకవేళ( serial_type==7 ){
          ఒకవేళ( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }లేకపోతే ఒకవేళ( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }లేకపోతే ఒకవేళ( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }లేకపోతే{
            ప్రతిజ్ఞా( rc==0 );
          }
        }లేకపోతే{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    లేకపోతే ఒకవేళ( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ఒకవేళ( serial_type<12 ){
        rc = -1;
      }లేకపోతే ఒకవేళ( !(serial_type & 0x01) ){
        rc = +1;
      }లేకపోతే{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(గుర్తులేని)nKey1 );
        testcase( (d1+mem1.n+1)==(గుర్తులేని)nKey1 );
        ఒకవేళ( (d1+mem1.n) > (గుర్తులేని)nKey1
         || (p_కీలకం_Info = pPKey2->p_కీలకం_Info)->n_All_క్షేత్రం<=i
        ){
          pPKey2->err_సంకేతం = (u8)SQLITE_CORRUPT_BKPT;
          తిరిగి 0;                /* Corruption */
        }లేకపోతే ఒకవేళ( p_కీలకం_Info->aColl[i] ){
          mem1.enc = p_కీలకం_Info->enc;
          mem1.db = p_కీలకం_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (అక్షరం*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_కీలకం_Info->aColl[i], &pPKey2->err_సంకేతం
          );
        }లేకపోతే{
          పూర్ణాంకం nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ఒకవేళ( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    లేకపోతే ఒకవేళ( pRhs->flags & MEM_Blob ){
      ప్రతిజ్ఞా( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      ఒకవేళ( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }లేకపోతే{
        పూర్ణాంకం nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(గుర్తులేని)nKey1 );
        testcase( (d1+nStr+1)==(గుర్తులేని)nKey1 );
        ఒకవేళ( (d1+nStr) > (గుర్తులేని)nKey1 ){
          pPKey2->err_సంకేతం = (u8)SQLITE_CORRUPT_BKPT;
          తిరిగి 0;                /* Corruption */
        }లేకపోతే ఒకవేళ( pRhs->flags & MEM_Zero ){
          ఒకవేళ( !isAllZero((స్థిరరాశి అక్షరం*)&aKey1[d1],nStr) ){
            rc = 1;
          }లేకపోతే{
            rc = nStr - pRhs->u.nZero;
          }
        }లేకపోతే{
          పూర్ణాంకం nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          ఒకవేళ( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    లేకపోతే{
      serial_type = aKey1[idx1];
      ఒకవేళ( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        ప్రతిజ్ఞా( rc==0 );
      }లేకపోతే{
        rc = 1;
      }
    }

    ఒకవేళ( rc!=0 ){
      పూర్ణాంకం క్రమీకరించు_Flags = pPKey2->p_కీలకం_Info->a_క్రమీకరించు_Flags[i];
      ఒకవేళ( క్రమీకరించు_Flags ){
        ఒకవేళ( (క్రమీకరించు_Flags & KEYINFO_క్రమం_BIGNULL)==0
         || ((క్రమీకరించు_Flags & KEYINFO_క్రమం_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      ప్రతిజ్ఞా( vdbe_రికార్డు_Compare_Debug(nKey1, pKey1, pPKey2, rc) );
      ప్రతిజ్ఞా( mem1.szMalloc==0 );  /* See comment below */
      తిరిగి rc;
    }

    i++;
    ఒకవేళ( i==pPKey2->n_క్షేత్రం ) ఆపు;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    ఒకవేళ( d1>(గుర్తులేని)nKey1 ) ఆపు;
    idx1 += sqlite3VarintLen(serial_type);
    ఒకవేళ( idx1>=(గుర్తులేని)szHdr1 ){
      pPKey2->err_సంకేతం = (u8)SQLITE_CORRUPT_BKPT;
      తిరిగి 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  ప్రతిజ్ఞా( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  ప్రతిజ్ఞా( CORRUPT_DB
       || vdbe_రికార్డు_Compare_Debug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_కీలకం_Info->db->mallocFailed
  );
  pPKey2->eq_చూసిన = 1;
  తిరిగి pPKey2->default_rc;
}
పూర్ణాంకం sqlite3_Vdbe_రికార్డు_Compare(
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1,   /* Left key */
  Unpacked_రికార్డు *pPKey2          /* Right key */
){
  తిరిగి sqlite3_Vdbe_రికార్డు_Compare_With_Skip(nKey1, pKey1, pPKey2, 0);
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
స్థిర పూర్ణాంకం vdbe_రికార్డు_Compare_Int(
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1, /* Left key */
  Unpacked_రికార్డు *pPKey2        /* Right key */
){
  స్థిరరాశి u8 *a_కీలకం = &((స్థిరరాశి u8*)pKey1)[*(స్థిరరాశి u8*)pKey1 & 0x3F];
  పూర్ణాంకం serial_type = ((స్థిరరాశి u8*)pKey1)[1];
  పూర్ణాంకం res;
  i64 v;
  i64 lhs;

  vdbe_Assert_క్షేత్రం_లెక్క_Within_Limits(nKey1, pKey1, pPKey2->p_కీలకం_Info);
  ప్రతిజ్ఞా( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  ఒకవేళ( (u32)(serial_type-1)<=5 ){
    స్థిర స్థిరరాశి u8 a_జరుపు[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_కీలకం)) >> a_జరుపు[serial_type];
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
    ప్రతిజ్ఞా( 0<=(i64)sqlite3Get8byte(a_కీలకం) || lhs<0 );
    testcase( lhs<0 );
  }లేకపోతే ఒకవేళ( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }లేకపోతే{
    తిరిగి sqlite3_Vdbe_రికార్డు_Compare(nKey1, pKey1, pPKey2);
  }

  ప్రతిజ్ఞా( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  ఒకవేళ( v>lhs ){
    res = pPKey2->r1;
  }లేకపోతే ఒకవేళ( v<lhs ){
    res = pPKey2->r2;
  }లేకపోతే ఒకవేళ( pPKey2->n_క్షేత్రం>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3_Vdbe_రికార్డు_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
  }లేకపోతే{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eq_చూసిన = 1;
  }

  ప్రతిజ్ఞా( vdbe_రికార్డు_Compare_Debug(nKey1, pKey1, pPKey2, res) );
  తిరిగి res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
స్థిర పూర్ణాంకం vdbe_రికార్డు_Compare_String(
  పూర్ణాంకం nKey1, స్థిరరాశి శూన్య *pKey1, /* Left key */
  Unpacked_రికార్డు *pPKey2        /* Right key */
){
  స్థిరరాశి u8 *aKey1 = (స్థిరరాశి u8*)pKey1;
  పూర్ణాంకం serial_type;
  పూర్ణాంకం res;

  ప్రతిజ్ఞా( pPKey2->aMem[0].flags & MEM_Str );
  ప్రతిజ్ఞా( pPKey2->aMem[0].n == pPKey2->n );
  ప్రతిజ్ఞా( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_క్షేత్రం_లెక్క_Within_Limits(nKey1, pKey1, pPKey2->p_కీలకం_Info);
  serial_type = (గుర్తుగల అక్షరం)(aKey1[1]);

vrcs_restart:
  ఒకవేళ( serial_type<12 ){
    ఒకవేళ( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      ఒకవేళ( serial_type>=12 ) వెళ్ళు vrcs_restart;
      ప్రతిజ్ఞా( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }లేకపోతే ఒకవేళ( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }లేకపోతే{
    పూర్ణాంకం nCmp;
    పూర్ణాంకం nStr;
    పూర్ణాంకం szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    ఒకవేళ( (szHdr + nStr) > nKey1 ){
      pPKey2->err_సంకేతం = (u8)SQLITE_CORRUPT_BKPT;
      తిరిగి 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    ఒకవేళ( res>0 ){
      res = pPKey2->r2;
    }లేకపోతే ఒకవేళ( res<0 ){
      res = pPKey2->r1;
    }లేకపోతే{
      res = nStr - pPKey2->n;
      ఒకవేళ( res==0 ){
        ఒకవేళ( pPKey2->n_క్షేత్రం>1 ){
          res = sqlite3_Vdbe_రికార్డు_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
        }లేకపోతే{
          res = pPKey2->default_rc;
          pPKey2->eq_చూసిన = 1;
        }
      }లేకపోతే ఒకవేళ( res>0 ){
        res = pPKey2->r2;
      }లేకపోతే{
        res = pPKey2->r1;
      }
    }
  }

  ప్రతిజ్ఞా( vdbe_రికార్డు_Compare_Debug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_కీలకం_Info->db->mallocFailed
  );
  తిరిగి res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
రికార్డు_Compare sqlite3_Vdbe_కనుగొను_Compare(Unpacked_రికార్డు *p){
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
  ప్రతిజ్ఞా( p->p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  ఒకవేళ( p->p_కీలకం_Info->n_All_క్షేత్రం<=13 ){
    పూర్ణాంకం flags = p->aMem[0].flags;
    ఒకవేళ( p->p_కీలకం_Info->a_క్రమీకరించు_Flags[0] ){
      ఒకవేళ( p->p_కీలకం_Info->a_క్రమీకరించు_Flags[0] & KEYINFO_క్రమం_BIGNULL ){
        తిరిగి sqlite3_Vdbe_రికార్డు_Compare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }లేకపోతే{
      p->r1 = -1;
      p->r2 = 1;
    }
    ఒకవేళ( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      తిరిగి vdbe_రికార్డు_Compare_Int;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    ఒకవేళ( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_ద్విమాన(p->p_కీలకం_Info->aColl[0])
    ){
      ప్రతిజ్ఞా( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      తిరిగి vdbe_రికార్డు_Compare_String;
    }
  }

  తిరిగి sqlite3_Vdbe_రికార్డు_Compare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
పూర్ణాంకం sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_కీలకం = 0;
  పూర్ణాంకం rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_కీలకం = sqlite3_Btree_Payload_కొలత(pCur);
  ప్రతిజ్ఞా( (n_Cell_కీలకం & SQLITE_MAX_U32)==(u64)n_Cell_కీలకం );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pCur, (u32)n_Cell_కీలకం, &m);
  ఒకవేళ( rc ){
    తిరిగి rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  ప్రతిజ్ఞా( m.n>=0 );
  ఒకవేళ( unlikely(szHdr<3 || szHdr>(గుర్తులేని)m.n) ){
    వెళ్ళు idx_rowid_corruption;
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
  ఒకవేళ( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    వెళ్ళు idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  ఒకవేళ( unlikely((u32)m.n<szHdr+lenRowid) ){
    వెళ్ళు idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  తిరిగి SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  తిరిగి SQLITE_CORRUPT_BKPT;
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
పూర్ణాంకం sqlite3_Vdbe_Idx_కీలకం_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  Unpacked_రికార్డు *pUnpacked,       /* Unpacked version of key */
  పూర్ణాంకం *res                         /* Write the comparison result here */
){
  i64 n_Cell_కీలకం = 0;
  పూర్ణాంకం rc;
  BtCursor *pCur;
  Mem m;

  ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  ప్రతిజ్ఞా( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_కీలకం = sqlite3_Btree_Payload_కొలత(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  ఒకవేళ( n_Cell_కీలకం<=0 || n_Cell_కీలకం>0x7fffffff ){
    *res = 0;
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_స్థానభ్రంశం(pCur, (u32)n_Cell_కీలకం, &m);
  ఒకవేళ( rc ){
    తిరిగి rc;
  }
  *res = sqlite3_Vdbe_రికార్డు_Compare_With_Skip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  తిరిగి SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
శూన్య sqlite3_Vdbe_సమితి_Changes(sqlite3 *db, i64 nChange){
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_మొత్తం_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
శూన్య sqlite3_Vdbe_లెక్క_Changes(Vdbe *v){
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
శూన్య sqlite3ExpirePreparedStatements(sqlite3 *db, పూర్ణాంకం i_సంకేతం){
  Vdbe *p;
  కొరకు(p = db->pVdbe; p; p=p->pVNext){
    p->expired = i_సంకేతం+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  తిరిగి v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  తిరిగి v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_విలువ *sqlite3_Vdbe_Get_Bound_విలువ(Vdbe *v, పూర్ణాంకం iVar, u8 aff){
  ప్రతిజ్ఞా( iVar>0 );
  ఒకవేళ( v ){
    Mem *pMem = &v->aVar[iVar-1];
    ప్రతిజ్ఞా( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_అంతర్గత_Func)!=0 );
    ఒకవేళ( 0==(pMem->flags & MEM_Null) ){
      sqlite3_విలువ *pRet = sqlite3_విలువ_New(v->db);
      ఒకవేళ( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_విలువ_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      తిరిగి pRet;
    }
  }
  తిరిగి 0;
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
శూన్య sqlite3VdbeReprepareOnBind(Vdbe *v, పూర్ణాంకం iVar, పూర్ణాంకం bSmallint){
  u32 m;
  ప్రతిజ్ఞా( iVar>0 );
  ప్రతిజ్ఞా( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_అంతర్గత_Func)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  ఒకవేళ( bSmallint ){
    ఒకవేళ( (v->expmask & m)==0 ) v->smimask |= m;
  }లేకపోతే{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  ప్రతిజ్ఞా( (v->smimask & v->expmask) == v->smimask );
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
స్థిర పూర్ణాంకం vdbe_Skip_క్షేత్రం(
  Bitmask ముసుగు,                   /* Mask of indexed expression fields */
  పూర్ణాంకం i_నిలువు,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  పూర్ణాంకం bIntegrity                  /* True if running PRAGMA integrity_check */
){
#నిర్వచించు BTREE_ULPDISTORTION 2
  ఒకవేళ( i_నిలువు>=BMS || (ముసుగు & MASKBIT(i_నిలువు))==0 ) తిరిగి 0;
  ఒకవేళ( bIntegrity==0 ) తిరిగి 1;
  ఒకవేళ( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    జ్ఞాపకం_నకలు(&m1,&pMem1->u.r,8);
    జ్ఞాపకం_నకలు(&m2,&pMem2->u.r,8);
    ఒకవేళ( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం vdbe_Is_Matching_సూచీ_కీలకం(
  BtCursor *pCur,            /* Cursor open on index */
  పూర్ణాంకం bInt,                  /* True for integrity_check-style search */
  Bitmask ముసుగు,              /* Mask of columns to skip */
  Unpacked_రికార్డు *p,         /* Index key being deleted */
  పూర్ణాంకం *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  పూర్ణాంకం rc = SQLITE_OK;

  memset(&m, 0, పరిమాణం(m));
  m.enc = p->p_కీలకం_Info->enc;
  m.db = p->p_కీలకం_Info->db;
  nRec = sqlite3_Btree_Payload_కొలత(pCur);
  ఒకవేళ( nRec>0x7fffffff ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  ఒకవేళ( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    ఒకవేళ( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }లేకపోతే{
      పూర్ణాంకం res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      పూర్ణాంకం ii = 0;                 /* Iterator variable */

      పూర్ణాంకం n_నిలువు = p->p_కీలకం_Info->n_All_క్షేత్రం;
      కొరకు(ii=0; ii<n_నిలువు && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        పూర్ణాంకం nSerial = 0;

        ఒకవేళ( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          ఆపు;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        ఒకవేళ( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }లేకపోతే{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          ఒకవేళ( vdbe_Skip_క్షేత్రం(ముసుగు, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_కీలకం_Info->aColl[ii]);
            ఒకవేళ( res!=0 ) ఆపు;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_Vdbe_కనుగొను_సూచీ_కీలకం(
  BtCursor *pCur, 
  సూచీ *pIdx,
  Unpacked_రికార్డు *p, 
  పూర్ణాంకం *pRes,
  పూర్ణాంకం bIntegrity
){
#నిర్వచించు BTREE_FDK_RANGE 10
  పూర్ణాంకం n_అడుగు = 0;
  పూర్ణాంకం res = 1;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask ముసుగు = 0;
  కొరకు(ii=0; ii<MIN(pIdx->n_స్తంభం, BMS); ii++){
    పూర్ణాంకం i_నిలువు = pIdx->ai_స్తంభం[ii];
    ఒకవేళ( (i_నిలువు==XN_సమాసం)
     || (i_నిలువు>=0 && (pIdx->p_పట్టిక->a_నిలువు[i_నిలువు].నిలువు_Flags & COLFLAG_VIRTUAL))
    ){
      ముసుగు |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  ఒకవేళ( ముసుగు!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    కొరకు(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3_Btree_మునుపటి(pCur, 0);
    }
    ఒకవేళ( rc==SQLITE_పూర్తి ){
      rc = sqlite3_Btree_మొదటి(pCur, &res);
      n_అడుగు = -1;
    }లేకపోతే{
      n_అడుగు = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    అయితే( sqlite3BtreeCursorIsValidNN(pCur) ){
      కొరకు(ii=0; rc==SQLITE_OK && (ii<n_అడుగు || n_అడుగు<0); ii++){
        rc = vdbe_Is_Matching_సూచీ_కీలకం(pCur, bIntegrity, ముసుగు, p, &res);
        ఒకవేళ( res==0 || rc!=SQLITE_OK ) ఆపు;
        rc = sqlite3_Btree_తదుపరి(pCur, 0);
      }
      ఒకవేళ( rc==SQLITE_పూర్తి ){
        rc = SQLITE_OK;
        ప్రతిజ్ఞా( res!=0 );
      }
      ఒకవేళ( n_అడుగు<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) ఆపు;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      n_అడుగు = -1;
      rc = sqlite3_Btree_మొదటి(pCur, &res);
    }
  }

  *pRes = res;
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
పూర్ణాంకం sqlite3_Not_శుద్ధ_Func(sqlite3_context *pCtx){
  స్థిరరాశి VdbeOp *pOp;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( pCtx->pVdbe==0 ) తిరిగి 1;
#అయితే_ముగింపు
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  ఒకవేళ( pOp->opcode==OP_శుద్ధ_Func ){
    స్థిరరాశి అక్షరం *zContext;
    అక్షరం *z_సందేశం;
    ఒకవేళ( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }లేకపోతే ఒకవేళ( pOp->p5 & NC_Gen_నిలువు ){
      zContext = "a generated column";
    }లేకపోతే{
      zContext = "an index";
    }
    z_సందేశం = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_ఫలితం_error(pCtx, z_సందేశం, -1);
    sqlite3_free(z_సందేశం);
    తిరిగి 0;
  }
  తిరిగి 1;
}
#అయితే_ముగింపు /* SQLITE_OMIT_DATETIME_FUNCS */

#ఒకవేళ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
పూర్ణాంకం sqlite3_Cursor_Range_Hint_సమాసం_Check(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_రిజిస్టరు ){
    ప్రతిజ్ఞా( (pWalker->u.aMem[p_సమాసం->i_పట్టిక].flags & MEM_Undefined)==0 );
  }
  తిరిగి WRC_Continue;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
శూన్య sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  ఒకవేళ( pVtab->z_Err_సందేశం ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_సందేశం);
    p->z_Err_సందేశం = sqlite3DbStrDup(db, pVtab->z_Err_సందేశం);
    sqlite3_free(pVtab->z_Err_సందేశం);
    pVtab->z_Err_సందేశం = 0;
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
స్థిర శూన్య vdbeFreeUnpacked(sqlite3 *db, పూర్ణాంకం n_క్షేత్రం, Unpacked_రికార్డు *p){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<n_క్షేత్రం; i++){
      Mem *pMem = &p->aMem[i];
      ఒకవేళ( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
శూన్య sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  పూర్ణాంకం op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  స్థిరరాశి అక్షరం *zDb,                /* Database name */
  పట్టిక *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  పూర్ణాంకం iReg,                       /* Register for new.* record */
  పూర్ణాంకం iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  స్థిరరాశి అక్షరం *zTbl = pTab->zName;
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం n_Real_నిలువు;
  ఒకవేళ( pTab->tabFlags & TF_WithoutRowid ){
    n_Real_నిలువు = sqlite3_Primary_కీలకం_సూచీ(pTab)->n_స్తంభం;
  }లేకపోతే ఒకవేళ( pTab->tabFlags & TF_HasVirtual ){
    n_Real_నిలువు = pTab->nNVCol;
  }లేకపోతే{
    n_Real_నిలువు = pTab->n_నిలువు;
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( db->pPreUpdate==0 );
  memset(&preupdate, 0, పరిమాణం(PreUpdate));
  ఒకవేళ( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
  }లేకపోతే{
    ఒకవేళ( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }లేకపోతే{
      iKey2 = iKey1;
    }
  }

  ప్రతిజ్ఞా( pCsr!=0 );
  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_BTREE );
  ప్రతిజ్ఞా( pCsr->n_క్షేత్రం==n_Real_నిలువు
       || (pCsr->n_క్షేత్రం==n_Real_నిలువు+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (కీలకం_Info*)&preupdate.u_కీలకం;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_కీలకం_క్షేత్రం = pTab->n_నిలువు;
  preupdate.pKeyinfo->a_క్రమీకరించు_Flags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.a_రికార్డు);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_కీలకం_క్షేత్రం+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_కీలకం_క్షేత్రం+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  ఒకవేళ( preupdate.aNew ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pCsr->n_క్షేత్రం; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  ఒకవేళ( preupdate.apDflt ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pTab->n_నిలువు; i++){
      sqlite3_విలువ_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

#నిర్వచితమైతే SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
స్థిరరాశి అక్షరం *sqlite3VdbeFuncName(స్థిరరాశి sqlite3_context *pCtx){
  ప్రతిజ్ఞా( pCtx!=0 );
  ప్రతిజ్ఞా( pCtx->pFunc!=0 );
  తిరిగి pCtx->pFunc->zName;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PERCENTILE */
