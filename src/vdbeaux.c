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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "vdbeInt.h"

/* Forward references */
நிலையான வெறுமை freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
நிலையான வெறுமை vdbeFreeOpArray(sqlite3 *, Op *, முழு);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, அளவு(Vdbe) );
  எனில்( p==0 ) திரும்பு 0;
  memset(&p->aOp, 0, அளவு(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  எனில்( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  ப்ரதிஜ்ஞா( pParse->aLabel==0 );
  ப்ரதிஜ்ஞா( pParse->nLabel==0 );
  ப்ரதிஜ்ஞா( p->nOpAlloc==0 );
  ப்ரதிஜ்ஞா( pParse->szOpAlloc==0 );
  sqlite3VdbeAddOp2(p, OP_Init, 0, 1);
  திரும்பு p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  திரும்பு p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
வெறுமை sqlite3VdbeError(Vdbe *p, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_செய்தி);
  va_start(ap, zFormat);
  p->z_Err_செய்தி = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
வெறுமை sqlite3VdbeSetSql(Vdbe *p, மாறிலி எழுத்து *z, முழு n, u8 prepFlags){
  எனில்( p==0 ) திரும்பு;
  p->prepFlags = prepFlags;
  எனில்( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  ப்ரதிஜ்ஞா( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
வெறுமை sqlite3VdbeAddDblquoteStr(sqlite3 *db, Vdbe *p, மாறிலி எழுத்து *z){
  எனில்( p ){
    முழு n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            அளவு(*pStr)+n+1-அளவு(pStr->z));
    எனில்( pStr ){
      pStr->pNextStr = p->pDblStr;
      p->pDblStr = pStr;
      memcpy(pStr->z, z, n+1);
    }
  }
}
#endif

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
முழு sqlite3VdbeUsesDoubleQuotedString(
  Vdbe *pVdbe,            /* The prepared statement */
  மாறிலி எழுத்து *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  ப்ரதிஜ்ஞா( zId!=0 );
  எனில்( pVdbe->pDblStr==0 ) திரும்பு 0;
  க்கு(pStr=pVdbe->pDblStr; pStr; pStr=pStr->pNextStr){
    எனில்( strcmp(zId, pStr->z)==0 ) திரும்பு 1;
  }
  திரும்பு 0;
}
#endif

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
வெறுமை sqlite3VdbeSwap(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  எழுத்து *zTmp;
  ப்ரதிஜ்ஞா( pA->db==pB->db );
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
#ifdef SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#endif
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  memcpy(pB->aCounter, pA->aCounter, அளவு(pB->aCounter));
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
நிலையான முழு growOpArray(Vdbe *v, முழு nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#ifdef SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#இல்லையெனில்
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/அளவு(Op)));
  UNUSED_PARAMETER(nOp);
#endif

  /* Ensure that the size of a VDBE does not grow too large */
  எனில்( nNew > p->db->aLimit[SQLITE_LIMIT_VDBE_OP] ){
    sqlite3OomFault(p->db);
    திரும்பு SQLITE_NOMEM;
  }

  ப்ரதிஜ்ஞா( nOp<=(முழு)(1024/அளவு(Op)) );
  ப்ரதிஜ்ஞா( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*அளவு(Op));
  எனில்( pNew ){
    p->szOpAlloc = sqlite3DbMallocSize(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/அளவு(Op);
    v->aOp = pNew;
  }
  திரும்பு (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#ifdef SQLITE_DEBUG
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
நிலையான வெறுமை test_addop_breakpoint(முழு pc, Op *pOp){
  நிலையான u64 n = 0;
  (வெறுமை)pc;
  (வெறுமை)pOp;
  n++;
  எனில்( n==LARGEST_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#endif

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
நிலையான SQLITE_NOINLINE முழு growOp3(Vdbe *p, முழு op, முழு p1, முழு p2, முழு p3){
  ப்ரதிஜ்ஞா( p->nOpAlloc<=p->nOp );
  எனில்( growOpArray(p, 1) ) திரும்பு 1;
  ப்ரதிஜ்ஞா( p->nOpAlloc>p->nOp );
  திரும்பு sqlite3VdbeAddOp3(p, op, p1, p2, p3);
}
நிலையான SQLITE_NOINLINE முழு addOp4IntSlow(
  Vdbe *p,            /* Add the opcode to this VM */
  முழு op,             /* The new opcode */
  முழு p1,             /* The P1 operand */
  முழு p2,             /* The P2 operand */
  முழு p3,             /* The P3 operand */
  முழு p4              /* The P4 operand as an integer */
){
  முழு addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  எனில்( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  திரும்பு addr;
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
முழு sqlite3VdbeAddOp0(Vdbe *p, முழு op){
  திரும்பு sqlite3VdbeAddOp3(p, op, 0, 0, 0);
}
முழு sqlite3VdbeAddOp1(Vdbe *p, முழு op, முழு p1){
  திரும்பு sqlite3VdbeAddOp3(p, op, p1, 0, 0);
}
முழு sqlite3VdbeAddOp2(Vdbe *p, முழு op, முழு p1, முழு p2){
  திரும்பு sqlite3VdbeAddOp3(p, op, p1, p2, 0);
}
முழு sqlite3VdbeAddOp3(Vdbe *p, முழு op, முழு p1, முழு p2, முழு p3){
  முழு i;
  VdbeOp *pOp;

  i = p->nOp;
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  ப்ரதிஜ்ஞா( op>=0 && op<0xff );
  எனில்( p->nOpAlloc<=i ){
    திரும்பு growOp3(p, op, p1, p2, p3);
  }
  ப்ரதிஜ்ஞா( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  ப்ரதிஜ்ஞா( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#endif
#எனில் defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#endif
#ifdef SQLITE_DEBUG
  எனில்( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#endif
#ifdef SQLITE_VDBE_COVERAGE
  pOp->i_Src_வரி = 0;
#endif
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  திரும்பு i;
}
முழு sqlite3VdbeAddOp4Int(
  Vdbe *p,            /* Add the opcode to this VM */
  முழு op,             /* The new opcode */
  முழு p1,             /* The P1 operand */
  முழு p2,             /* The P2 operand */
  முழு p3,             /* The P3 operand */
  முழு p4              /* The P4 operand as an integer */
){
  முழு i;
  VdbeOp *pOp;

  i = p->nOp;
  எனில்( p->nOpAlloc<=i ){
    திரும்பு addOp4IntSlow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  ப்ரதிஜ்ஞா( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#endif
#எனில் defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#endif
#ifdef SQLITE_DEBUG
  எனில்( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#endif
#ifdef SQLITE_VDBE_COVERAGE
  pOp->i_Src_வரி = 0;
#endif
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  திரும்பு i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
முழு sqlite3VdbeAddInt64(Vdbe *p, முழு iDest, i64 iVal){
  திரும்பு sqlite3VdbeAddOp3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
முழு sqlite3VdbeAddDouble(Vdbe *p, முழு iDest, இரட்டை rVal){
  i64 iVal;
  memcpy(&iVal,&rVal,8);
  திரும்பு sqlite3VdbeAddOp3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
முழு sqlite3VdbeGoto(Vdbe *p, முழு iDest){
  திரும்பு sqlite3VdbeAddOp3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
முழு sqlite3VdbeLoadString(Vdbe *p, முழு iDest, மாறிலி எழுத்து *zStr){
  திரும்பு sqlite3VdbeAddOp4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
வெறுமை sqlite3VdbeMultiLoad(Vdbe *p, முழு iDest, மாறிலி எழுத்து *zTypes, ...){
  va_list ap;
  முழு i;
  எழுத்து c;
  va_start(ap, zTypes);
  க்கு(i=0; (c = zTypes[i])!=0; i++){
    எனில்( c=='s' ){
      மாறிலி எழுத்து *z = va_arg(ap, மாறிலி எழுத்து*);
      sqlite3VdbeAddOp4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }இல்லையெனில் எனில்( c=='i' ){
      sqlite3VdbeAddOp2(p, OP_Integer, va_arg(ap, முழு), iDest+i);
    }இல்லையெனில்{
      செல் skip_op_resultrow;
    }
  }
  sqlite3VdbeAddOp2(p, OP_ResultRow, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
முழு sqlite3VdbeAddOp4(
  Vdbe *p,            /* Add the opcode to this VM */
  முழு op,             /* The new opcode */
  முழு p1,             /* The P1 operand */
  முழு p2,             /* The P2 operand */
  முழு p3,             /* The P3 operand */
  மாறிலி எழுத்து *zP4,    /* The P4 operand */
  முழு p4type          /* P4 operand type */
){
  முழு addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  திரும்பு addr;
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
முழு sqlite3VdbeAddFunctionCall(
  Parse *pParse,        /* Parsing context */
  முழு p1,               /* Constant argument mask */
  முழு p2,               /* First argument register */
  முழு p3,               /* Register into which results are written */
  முழு nArg,             /* Number of argument */
  மாறிலி FuncDef *pFunc, /* The function to be invoked */
  முழு eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  முழு addr;
  sqlite3_context *pCtx;
  ப்ரதிஜ்ஞா( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  எனில்( pCtx==0 ){
    ப்ரதிஜ்ஞா( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    திரும்பு 0;
  }
  pCtx->pOut = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3VdbeAddOp4(v, eCallCtx ? OP_PureFunc : OP_Function,
                           p1, p2, p3, (எழுத்து*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  திரும்பு addr;
}

#ifndef SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
முழு sqlite3VdbeExplainParent(Parse *pParse){
  VdbeOp *pOp;
  எனில்( pParse->addrExplain==0 ) திரும்பு 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  திரும்பு pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#எனில் defined(SQLITE_DEBUG)
வெறுமை sqlite3ExplainBreakpoint(மாறிலி எழுத்து *z1, மாறிலி எழுத்து *z2){
  (வெறுமை)z1;
  (வெறுமை)z2;
}
#endif

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
முழு sqlite3VdbeExplain(Parse *pParse, u8 b_தள்ளு, மாறிலி எழுத்து *zFmt, ...){
  முழு addr = 0;
#எனில் !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  எனில்( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    எழுத்து *z_செய்தி;
    Vdbe *v;
    va_list ap;
    முழு iThis;
    va_start(ap, zFmt);
    z_செய்தி = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3VdbeAddOp4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_செய்தி, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_தள்ளு?"PUSH":"", sqlite3VdbeGetLastOp(v)->p4.z);
    எனில்( b_தள்ளு){
      pParse->addrExplain = iThis;
    }
    sqlite3VdbeScanStatus(v, iThis, -1, -1, 0, 0);
  }
  திரும்பு addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
வெறுமை sqlite3_Vdbe_Explain_எடு(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3VdbeExplainParent(pParse);
}
#endif /* SQLITE_OMIT_EXPLAIN */

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
வெறுமை sqlite3VdbeAddParseSchemaOp(Vdbe *p, முழு iDb, எழுத்து *zWhere, u16 p5){
  முழு j;
  ப்ரதிஜ்ஞா( p5==0 || zWhere==0 );
  sqlite3VdbeAddOp4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  க்கு(j=0; j<p->db->nDb; j++) sqlite3VdbeUsesBtree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
வெறுமை sqlite3VdbeEndCoroutine(Vdbe *v, முழு regYield){
  sqlite3VdbeAddOp1(v, OP_EndCoroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->nTempReg = 0;
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
முழு sqlite3VdbeMakeLabel(Parse *pParse){
  திரும்பு --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
நிலையான SQLITE_NOINLINE வெறுமை resizeResolveLabel(Parse *p, Vdbe *v, முழு j){
  முழு nNewSize = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     nNewSize*அளவு(p->aLabel[0]));
  எனில்( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }இல்லையெனில்{
#ifdef SQLITE_DEBUG
    முழு i;
    க்கு(i=p->nLabelAlloc; i<nNewSize; i++) p->aLabel[i] = -1;
#endif
    எனில்( nNewSize>=100 && (nNewSize/100)>(p->nLabelAlloc/100) ){
      sqlite3ProgressCheck(p);
    }
    p->nLabelAlloc = nNewSize;
    p->aLabel[j] = v->nOp;
  }
}
வெறுமை sqlite3VdbeResolveLabel(Vdbe *v, முழு x){
  Parse *p = v->pParse;
  முழு j = ADDR(x);
  ப்ரதிஜ்ஞா( v->eVdbeState==VDBE_INIT_STATE );
  ப்ரதிஜ்ஞா( j<-p->nLabel );
  ப்ரதிஜ்ஞா( j>=0 );
#ifdef SQLITE_DEBUG
  எனில்( p->db->flags & SQLITE_VdbeAddopTrace ){
    printf("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#endif
  எனில்( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
வெறுமை sqlite3VdbeRunOnlyOnce(Vdbe *p){
  sqlite3VdbeAddOp2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
வெறுமை sqlite3VdbeReusable(Vdbe *p){
  முழு i;
  க்கு(i=1; ALWAYS(i<p->nOp); i++){
    எனில்( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      நிறுத்து;
    }
  }
}

#ifdef SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
வகைவரை கட்டமைப்பு VdbeOpIter VdbeOpIter;
கட்டமைப்பு VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  SubProgram **apSub;        /* Array of subprograms */
  முழு nSub;                  /* Number of entries in apSub */
  முழு iAddr;                 /* Address of next instruction to return */
  முழு iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
நிலையான Op *opIterNext(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  முழு nOp;

  எனில்( p->iSub<=p->nSub ){

    எனில்( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }இல்லையெனில்{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    ப்ரதிஜ்ஞா( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    எனில்( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    எனில்( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*அளவு(SubProgram*);
      முழு j;
      க்கு(j=0; j<p->nSub; j++){
        எனில்( p->apSub[j]==pRet->p4.pProgram ) நிறுத்து;
      }
      எனில்( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        எனில்( !p->apSub ){
          pRet = 0;
        }இல்லையெனில்{
          p->apSub[p->nSub++] = pRet->p4.pProgram;
        }
      }
    }
  }

  திரும்பு pRet;
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
முழு sqlite3VdbeAssertMayAbort(Vdbe *v, முழு mayAbort){
  முழு hasAbort = 0;
  முழு hasFkCounter = 0;
  முழு hasCreateTable = 0;
  முழு hasCreateIndex = 0;
  முழு hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  எனில்( v==0 ) திரும்பு 0;
  memset(&sIter, 0, அளவு(sIter));
  sIter.v = v;

  போது( (pOp = opIterNext(&sIter))!=0 ){
    முழு opcode = pOp->opcode;
    எனில்( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_PureFunc
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      நிறுத்து;
    }
    எனில்( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) hasCreateTable = 1;
    எனில்( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      எனில்( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) hasCreateIndex = 1;
      எனில்( opcode==OP_Clear ) hasCreateIndex = 1;
    }
    எனில்( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#ifndef SQLITE_OMIT_FOREIGN_சாவி
    எனில்( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
      hasFkCounter = 1;
    }
#endif
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  திரும்பு ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (hasCreateTable && hasInitCoroutine) || hasCreateIndex
  );
}
#endif /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#ifdef SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
வெறுமை sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  எனில்( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#endif

#ifdef SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
வெறுமை sqlite3VdbeAssertAbortable(Vdbe *p){
  ப்ரதிஜ்ஞா( p->nWrite==0 || p->usesStmtJournal );
}
#endif

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
நிலையான வெறுமை resolve_P2_மதிப்புகள்(Vdbe *p, முழு *pMaxVtabArgs){
  முழு nMaxVtabArgs = *pMaxVtabArgs;
  Op *pOp;
  Parse *pParse = p->pParse;
  முழு *aLabel = pParse->aLabel;

  ப்ரதிஜ்ஞா( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  ப்ரதிஜ்ஞா( p->aOp[0].opcode==OP_Init );
  போது( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    எனில்( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      தேர்வு( pOp->opcode ){
        நிலை OP_Transaction: {
          எனில்( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        நிலை OP_AutoCommit:
        நிலை OP_Savepoint: {
          p->bIsReader = 1;
          நிறுத்து;
        }
#ifndef SQLITE_OMIT_WAL
        நிலை OP_Checkpoint:
#endif
        நிலை OP_Vacuum:
        நிலை OP_JournalMode: {
          p->readOnly = 0;
          p->bIsReader = 1;
          நிறுத்து;
        }
        நிலை OP_Init: {
          ப்ரதிஜ்ஞா( pOp->p2>=0 );
          செல் resolve_p2_மதிப்புகள்_loop_exit;
        }
#ifndef SQLITE_OMIT_VIRTUALTABLE
        நிலை OP_VUpdate: {
          எனில்( pOp->p2>nMaxVtabArgs ) nMaxVtabArgs = pOp->p2;
          நிறுத்து;
        }
        நிலை OP_VFilter: {
          முழு n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          ப்ரதிஜ்ஞா( (pOp - p->aOp) >= 3 );
          ப்ரதிஜ்ஞா( pOp[-1].opcode==OP_Integer );
          ப்ரதிஜ்ஞா( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          எனில்( n>nMaxVtabArgs ) nMaxVtabArgs = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#endif
        இயல்பு: {
          எனில்( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            ப்ரதிஜ்ஞா( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            ப்ரதிஜ்ஞா( ADDR(pOp->p2)<-pParse->nLabel );
            ப்ரதிஜ்ஞா( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          ப்ரதிஜ்ஞா( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          ப்ரதிஜ்ஞா( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          நிறுத்து;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      ப்ரதிஜ்ஞா( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    ப்ரதிஜ்ஞா( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_மதிப்புகள்_loop_exit:
  எனில்( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *pMaxVtabArgs = nMaxVtabArgs;
  ப்ரதிஜ்ஞா( p->bIsReader!=0 || DbMaskAllZero(p->btreeMask) );
}

#ifdef SQLITE_DEBUG
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
வெறுமை sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  முழு iFirst,       /* First opcode of the subroutine */
  முழு iLast,        /* Last opcode of the subroutine */
  முழு iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  முழு i;
  sqlite3_str *pErr = 0;
  ப்ரதிஜ்ஞா( v!=0 );
  pParse = v->pParse;
  ப்ரதிஜ்ஞா( pParse!=0 );
  எனில்( pParse->nErr ) திரும்பு;
  ப்ரதிஜ்ஞா( iLast>=iFirst );
  ப்ரதிஜ்ஞா( iLast<v->nOp );
  pOp = &v->aOp[iFirst];
  க்கு(i=iFirst; i<=iLast; i++, pOp++){
    எனில்( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      முழு iDest = pOp->p2;   /* Jump destination */
      எனில்( iDest==0 ) தொடர்;
      எனில்( pOp->opcode==OP_Gosub ) தொடர்;
      எனில்( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        தொடர்;
      }
      எனில்( iDest<0 ){
        முழு j = ADDR(iDest);
        ப்ரதிஜ்ஞா( j>=0 );
        எனில்( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          தொடர்;
        }
        iDest = pParse->aLabel[j];
      }
      எனில்( iDest<iFirst || iDest>iLast ){
        முழு j = iDest;
        க்கு(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          எனில்( pX->opcode==OP_Return ){
            எனில்( pX->p1==iRetReg ) நிறுத்து;
            தொடர்;
          }
          எனில்( pX->opcode==OP_Noop ) தொடர்;
          எனில்( pX->opcode==OP_Explain ) தொடர்;
          எனில்( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }இல்லையெனில்{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, iFirst, iLast);
          நிறுத்து;
        }
      }
    }
  }
  எனில்( pErr ){
    எழுத்து *zErr = sqlite3_str_finish(pErr);
    sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_INTERNAL, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#endif /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
முழு sqlite3VdbeCurrentAddr(Vdbe *p){
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  திரும்பு p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#எனில் defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
வெறுமை sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, முழு N){
  ப்ரதிஜ்ஞா( p->nOp + N <= p->nOpAlloc );
}
#endif

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#எனில் defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
வெறுமை sqlite3VdbeVerifyNoResultRow(Vdbe *p){
  முழு i;
  க்கு(i=0; i<p->nOp; i++){
    ப்ரதிஜ்ஞா( p->aOp[i].opcode!=OP_ResultRow );
  }
}
#endif

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#எனில் defined(SQLITE_DEBUG)
வெறுமை sqlite3VdbeVerifyAbortable(Vdbe *p, முழு onError){
  எனில்( onError==OE_Abort ) sqlite3VdbeAddOp0(p, OP_Abortable);
}
#endif

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, முழு *pnOp, முழு *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  ப்ரதிஜ்ஞா( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  ப்ரதிஜ்ஞா( DbMaskAllZero(p->btreeMask) );

  resolve_P2_மதிப்புகள்(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  திரும்பு aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3VdbeAddOpList(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  முழு nOp,                     /* Number of opcodes to add */
  VdbeOpList மாறிலி *aOp,       /* The opcodes to be added */
  முழு iLineno                  /* Source-file line number of first opcode */
){
  முழு i;
  VdbeOp *pOut, *pFirst;
  ப்ரதிஜ்ஞா( nOp>0 );
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  எனில்( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    திரும்பு 0;
  }
  pFirst = pOut = &p->aOp[p->nOp];
  க்கு(i=0; i<nOp; i++, aOp++, pOut++){
    pOut->opcode = aOp->opcode;
    pOut->p1 = aOp->p1;
    pOut->p2 = aOp->p2;
    ப்ரதிஜ்ஞா( aOp->p2>=0 );
    எனில்( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      pOut->p2 += p->nOp;
    }
    pOut->p3 = aOp->p3;
    pOut->p4type = P4_NOTUSED;
    pOut->p4.p = 0;
    pOut->p5 = 0;
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
    pOut->zComment = 0;
#endif
#ifdef SQLITE_VDBE_COVERAGE
    pOut->i_Src_வரி = iLineno+i;
#இல்லையெனில்
    (வெறுமை)iLineno;
#endif
#ifdef SQLITE_DEBUG
    எனில்( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#endif
  }
  p->nOp += nOp;
  திரும்பு pFirst;
}

#எனில் defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
வெறுமை sqlite3VdbeScanStatus(
  Vdbe *p,                        /* VM to add scanstatus() to */
  முழு addrExplain,                /* Address of OP_Explain (or 0) */
  முழு addrLoop,                   /* Address of loop counter */
  முழு addrVisit,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  மாறிலி எழுத்து *zName               /* Name of table or index being scanned */
){
  எனில்( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->nScan) * அளவு(ScanStatus);
    ScanStatus *aNew;
    aNew = (ScanStatus*)sqlite3DbRealloc(p->db, p->aScan, nByte);
    எனில்( aNew ){
      ScanStatus *pNew = &aNew[p->nScan++];
      memset(pNew, 0, அளவு(ScanStatus));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addrVisit = addrVisit;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->aScan = aNew;
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
வெறுமை sqlite3VdbeScanStatusRange(
  Vdbe *p,
  முழு addrExplain,
  முழு addrStart,
  முழு addrEnd
){
  எனில்( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    முழு ii;
    க்கு(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      எனில்( pScan->addrExplain==addrExplain ) நிறுத்து;
      pScan = 0;
    }
    எனில்( pScan ){
      எனில்( addrEnd<0 ) addrEnd = sqlite3VdbeCurrentAddr(p)-1;
      க்கு(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
        எனில்( pScan->aAddrRange[ii]==0 ){
          pScan->aAddrRange[ii] = addrStart;
          pScan->aAddrRange[ii+1] = addrEnd;
          நிறுத்து;
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
வெறுமை sqlite3VdbeScanStatusCounters(
  Vdbe *p,
  முழு addrExplain,
  முழு addrLoop,
  முழு addrVisit
){
  எனில்( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    முழு ii;
    க்கு(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      எனில்( pScan->addrExplain==addrExplain ) நிறுத்து;
      pScan = 0;
    }
    எனில்( pScan ){
      எனில்( addrLoop>0 ) pScan->addrLoop = addrLoop;
      எனில்( addrVisit>0 ) pScan->addrVisit = addrVisit;
    }
  }
}
#endif /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
வெறுமை sqlite3VdbeChangeOpcode(Vdbe *p, முழு addr, u8 iNewOpcode){
  ப்ரதிஜ்ஞா( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
வெறுமை sqlite3VdbeChangeP1(Vdbe *p, முழு addr, முழு val){
  ப்ரதிஜ்ஞா( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
வெறுமை sqlite3VdbeChangeP2(Vdbe *p, முழு addr, முழு val){
  ப்ரதிஜ்ஞா( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
வெறுமை sqlite3VdbeChangeP3(Vdbe *p, முழு addr, முழு val){
  ப்ரதிஜ்ஞா( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
வெறுமை sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  ப்ரதிஜ்ஞா( p->nOp>0 || p->db->mallocFailed );
  எனில்( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
வெறுமை sqlite3VdbeTypeofColumn(Vdbe *p, முழு iDest){
  VdbeOp *pOp = sqlite3VdbeGetLastOp(p);
#ifdef SQLITE_DEBUG
  போது( pOp->opcode==OP_ReleaseReg ) pOp--;
#endif
  எனில்( pOp->p3==iDest && pOp->opcode==OP_Column ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
வெறுமை sqlite3VdbeJumpHere(Vdbe *p, முழு addr){
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
வெறுமை sqlite3_Vdbe_Jump_Here_Or_எடு_Inst(Vdbe *p, முழு addr){
  எனில்( addr==p->nOp-1 ){
    ப்ரதிஜ்ஞா( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    ப்ரதிஜ்ஞா( p->aOp[addr].p4type==0 );
#ifdef SQLITE_VDBE_COVERAGE
    sqlite3VdbeGetLastOp(p)->i_Src_வரி = 0;  /* Erase VdbeCoverage() macros */
#endif
    p->nOp--;
  }இல்லையெனில்{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
நிலையான வெறுமை freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
நிலையான SQLITE_NOINLINE வெறுமை freeP4Mem(sqlite3 *db, Mem *p){
  எனில்( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
நிலையான SQLITE_NOINLINE வெறுமை freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  ப்ரதிஜ்ஞா( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
நிலையான வெறுமை freeP4(sqlite3 *db, முழு p4type, வெறுமை *p4){
  ப்ரதிஜ்ஞா( db );
  தேர்வு( p4type ){
    நிலை P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      நிறுத்து;
    }
    நிலை P4_DYNAMIC:
    நிலை P4_INTARRAY: {
      எனில்( p4 ) sqlite3DbNNFreeNN(db, p4);
      நிறுத்து;
    }
    நிலை P4_KEYINFO: {
      எனில்( db->pnBytesFreed==0 ) sqlite3_சாவி_Info_Unref((சாவி_Info*)p4);
      நிறுத்து;
    }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
    நிலை P4_கோவை: {
      sqlite3_கோவை_Delete(db, (கோவை*)p4);
      நிறுத்து;
    }
#endif
    நிலை P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      நிறுத்து;
    }
    நிலை P4_MEM: {
      எனில்( db->pnBytesFreed==0 ){
        sqlite3_மதிப்பு_Free((sqlite3_மதிப்பு*)p4);
      }இல்லையெனில்{
        freeP4Mem(db, (Mem*)p4);
      }
      நிறுத்து;
    }
    நிலை P4_VTAB : {
      எனில்( db->pnBytesFreed==0 ) sqlite3VtabUnlock((VTable *)p4);
      நிறுத்து;
    }
    நிலை P4_TABLEREF: {
      எனில்( db->pnBytesFreed==0 ) sqlite3DeleteTable(db, (Table*)p4);
      நிறுத்து;
    }
    நிலை P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      நிறுத்து;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
நிலையான வெறுமை vdbeFreeOpArray(sqlite3 *db, Op *aOp, முழு nOp){
  ப்ரதிஜ்ஞா( nOp>=0 );
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( aOp ){
    Op *pOp = &aOp[nOp-1];
    போது(1){  /* Exit via break */
      எனில்( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#endif    
      எனில்( pOp==aOp ) நிறுத்து;
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
வெறுமை sqlite3VdbeLinkSubProgram(Vdbe *pVdbe, SubProgram *p){
  p->pNext = pVdbe->pProgram;
  pVdbe->pProgram = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
முழு sqlite3VdbeHasSubProgram(Vdbe *pVdbe){
  திரும்பு pVdbe->pProgram!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
முழு sqlite3VdbeChangeToNoop(Vdbe *p, முழு addr){
  VdbeOp *pOp;
  எனில்( p->db->mallocFailed ) திரும்பு 0;
  ப்ரதிஜ்ஞா( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  திரும்பு 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
முழு sqlite3VdbeDeletePriorOpcode(Vdbe *p, u8 op){
  எனில்( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    திரும்பு sqlite3VdbeChangeToNoop(p, p->nOp-1);
  }இல்லையெனில்{
    திரும்பு 0;
  }
}

#ifdef SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
வெறுமை sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  முழு iFirst,          /* Index of first register to be released */
  முழு N,               /* Number of registers to release */
  u32 mask,            /* Mask of registers to NOT release */
  முழு bUndefine        /* If true, mark registers as undefined */
){
  எனில்( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) திரும்பு;
  ப்ரதிஜ்ஞா( pParse->pVdbe );
  ப்ரதிஜ்ஞா( iFirst>=1 );
  ப்ரதிஜ்ஞா( iFirst+N-1<=pParse->nMem );
  எனில்( N<=31 && mask!=0 ){
    போது( N>0 && (mask&1)!=0 ){
      mask >>= 1;
      iFirst++;
      N--;
    }
    போது( N>0 && N<=32 && (mask & MASKBIT32(N-1))!=0 ){
      mask &= ~MASKBIT32(N-1);
      N--;
    }
  }
  எனில்( N>0 ){
    sqlite3VdbeAddOp3(pParse->pVdbe, OP_ReleaseReg, iFirst, N, *(முழு*)&mask);
    எனில்( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#endif /* SQLITE_DEBUG */

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
நிலையான வெறுமை SQLITE_NOINLINE vdbeChangeP4Full(
  Vdbe *p,
  Op *pOp,
  மாறிலி எழுத்து *zP4,
  முழு n
){
  எனில்( pOp->p4type ){
    ப்ரதிஜ்ஞா( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  எனில்( n<0 ){
    sqlite3VdbeChangeP4(p, (முழு)(pOp - p->aOp), zP4, n);
  }இல்லையெனில்{
    எனில்( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
வெறுமை sqlite3VdbeChangeP4(Vdbe *p, முழு addr, மாறிலி எழுத்து *zP4, முழு n){
  Op *pOp;
  sqlite3 *db;
  ப்ரதிஜ்ஞா( p!=0 );
  db = p->db;
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  ப்ரதிஜ்ஞா( p->aOp!=0 || db->mallocFailed );
  எனில்( db->mallocFailed ){
    எனில்( n!=P4_VTAB ) freeP4(db, n, (வெறுமை*)*(எழுத்து**)&zP4);
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( p->nOp>0 );
  ப்ரதிஜ்ஞா( addr<p->nOp );
  எனில்( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  எனில்( n>=0 || pOp->p4type ){
    vdbeChangeP4Full(p, pOp, zP4, n);
    திரும்பு;
  }
  எனில்( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_TO_INT(zP4);
    pOp->p4type = P4_INT32;
  }இல்லையெனில் எனில்( zP4!=0 ){
    ப்ரதிஜ்ஞா( n<0 );
    pOp->p4.p = (வெறுமை*)zP4;
    pOp->p4type = (signed எழுத்து)n;
    எனில்( n==P4_VTAB ) sqlite3VtabLock((VTable*)zP4);
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
வெறுமை sqlite3VdbeAppendP4(Vdbe *p, வெறுமை *pP4, முழு n){
  VdbeOp *pOp;
  ப்ரதிஜ்ஞா( n!=P4_INT32 && n!=P4_VTAB );
  ப்ரதிஜ்ஞா( n<=0 );
  எனில்( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( pP4!=0 || n==P4_DYNAMIC );
    ப்ரதிஜ்ஞா( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    ப்ரதிஜ்ஞா( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
வெறுமை sqlite3_Vdbe_Set_P4_சாவி_Info(Parse *pParse, Index *pIdx){
  Vdbe *v = pParse->pVdbe;
  சாவி_Info *p_சாவி_Info;
  ப்ரதிஜ்ஞா( v!=0 );
  ப்ரதிஜ்ஞா( pIdx!=0 );
  p_சாவி_Info = sqlite3_சாவி_Info_Of_Index(pParse, pIdx);
  எனில்( p_சாவி_Info ) sqlite3VdbeAppendP4(v, p_சாவி_Info, P4_KEYINFO);
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
நிலையான வெறுமை vdbeVComment(Vdbe *p, மாறிலி எழுத்து *zFormat, va_list ap){
  ப்ரதிஜ்ஞா( p->nOp>0 || p->aOp==0 );
  ப்ரதிஜ்ஞா( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  எனில்( p->nOp ){
    ப்ரதிஜ்ஞா( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
வெறுமை sqlite3VdbeComment(Vdbe *p, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  எனில்( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
வெறுமை sqlite3VdbeNoopComment(Vdbe *p, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  எனில்( p ){
    sqlite3VdbeAddOp0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#endif  /* NDEBUG */

#ifdef SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
வெறுமை sqlite3_Vdbe_Set_வரி_எண்(Vdbe *v, முழு i_வரி){
  sqlite3VdbeGetLastOp(v)->i_Src_வரி = i_வரி;
}
#endif /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, முழு addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  நிலையான VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  ப்ரதிஜ்ஞா( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  எனில்( p->db->mallocFailed ){
    திரும்பு (VdbeOp*)&dummy;
  }இல்லையெனில்{
    திரும்பு &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3VdbeGetLastOp(Vdbe *p){
  திரும்பு sqlite3VdbeGetOp(p, p->nOp - 1);
}

#எனில் defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
நிலையான முழு translateP(எழுத்து c, மாறிலி Op *pOp){
  எனில்( c=='1' ) திரும்பு pOp->p1;
  எனில்( c=='2' ) திரும்பு pOp->p2;
  எனில்( c=='3' ) திரும்பு pOp->p3;
  எனில்( c=='4' ) திரும்பு pOp->p4.i;
  திரும்பு pOp->p5;
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
எழுத்து *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  மாறிலி Op *pOp,     /* The opcode to be commented */
  மாறிலி எழுத்து *zP4    /* Previously obtained value for P4 */
){
  மாறிலி எழுத்து *zOpName;
  மாறிலி எழுத்து *zSynopsis;
  முழு nOpName;
  முழு ii;
  எழுத்து zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_LENGTH);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  எனில்( zOpName[nOpName+1] ){
    முழு seenCom = 0;
    எழுத்து c;
    zSynopsis = zOpName + nOpName + 1;
    எனில்( strncmp(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(அளவு(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    க்கு(ii=0; (c = zSynopsis[ii])!=0; ii++){
      எனில்( c=='P' ){
        c = zSynopsis[++ii];
        எனில்( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }இல்லையெனில் எனில்( c=='X' ){
          எனில்( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            seenCom = 1;
            நிறுத்து;
          }
        }இல்லையெனில் எனில்( strncmp(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_TO_64(pOp->p1,pOp->p3));
          ii += 4;
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
        }இல்லையெனில் எனில்( strncmp(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_TO_64(pOp->p2,pOp->p3));
          ii += 4;
#endif
        }இல்லையெனில் எனில்( strncmp(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_TO_64(pOp->p1,pOp->p3);
          இரட்டை r;
          memcpy(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }இல்லையெனில்{
          முழு v1 = translateP(c, pOp);
          முழு v2;
          எனில்( strncmp(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            எனில்( strncmp(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            எனில்( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }இல்லையெனில்{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }இல்லையெனில் எனில்( strncmp(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            எனில்( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }இல்லையெனில் எனில்( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }இல்லையெனில் எனில்( x.accError==0 ){
              ப்ரதிஜ்ஞா( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }இல்லையெனில்{
            sqlite3_str_appendf(&x, "%d", v1);
            எனில்( strncmp(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }இல்லையெனில்{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    எனில்( !seenCom && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }இல்லையெனில் எனில்( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  எனில்( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  திரும்பு sqlite3StrAccumFinish(&x);
}
#endif /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#எனில் VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
நிலையான வெறுமை display_P4_கோவை(StrAccum *p, கோவை *p_கோவை){
  மாறிலி எழுத்து *zOp = 0;
  தேர்வு( p_கோவை->op ){
    நிலை TK_STRING:
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_str_appendf(p, "%Q", p_கோவை->u.zToken);
      நிறுத்து;
    நிலை TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_கோவை->u.i_மதிப்பு);
      நிறுத்து;
    நிலை TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      நிறுத்து;
    நிலை TK_REGISTER: {
      sqlite3_str_appendf(p, "r[%d]", p_கோவை->iTable);
      நிறுத்து;
    }
    நிலை TK_COLUMN: {
      எனில்( p_கோவை->iColumn<0 ){
        sqlite3_str_appendf(p, "rowid");
      }இல்லையெனில்{
        sqlite3_str_appendf(p, "c%d", (முழு)p_கோவை->iColumn);
      }
      நிறுத்து;
    }
    நிலை TK_LT:      zOp = "LT";      நிறுத்து;
    நிலை TK_LE:      zOp = "LE";      நிறுத்து;
    நிலை TK_GT:      zOp = "GT";      நிறுத்து;
    நிலை TK_GE:      zOp = "GE";      நிறுத்து;
    நிலை TK_NE:      zOp = "NE";      நிறுத்து;
    நிலை TK_EQ:      zOp = "EQ";      நிறுத்து;
    நிலை TK_IS:      zOp = "IS";      நிறுத்து;
    நிலை TK_ISNOT:   zOp = "ISNOT";   நிறுத்து;
    நிலை TK_AND:     zOp = "AND";     நிறுத்து;
    நிலை TK_OR:      zOp = "OR";      நிறுத்து;
    நிலை TK_PLUS:    zOp = "ADD";     நிறுத்து;
    நிலை TK_STAR:    zOp = "MUL";     நிறுத்து;
    நிலை TK_MINUS:   zOp = "SUB";     நிறுத்து;
    நிலை TK_REM:     zOp = "REM";     நிறுத்து;
    நிலை TK_BITAND:  zOp = "BITAND";  நிறுத்து;
    நிலை TK_BITOR:   zOp = "BITOR";   நிறுத்து;
    நிலை TK_SLASH:   zOp = "DIV";     நிறுத்து;
    நிலை TK_LSHIFT:  zOp = "LSHIFT";  நிறுத்து;
    நிலை TK_RSHIFT:  zOp = "RSHIFT";  நிறுத்து;
    நிலை TK_CONCAT:  zOp = "CONCAT";  நிறுத்து;
    நிலை TK_UMINUS:  zOp = "MINUS";   நிறுத்து;
    நிலை TK_UPLUS:   zOp = "PLUS";    நிறுத்து;
    நிலை TK_BITNOT:  zOp = "BITNOT";  நிறுத்து;
    நிலை TK_NOT:     zOp = "NOT";     நிறுத்து;
    நிலை TK_ISNULL:  zOp = "ISNULL";  நிறுத்து;
    நிலை TK_NOTNULL: zOp = "NOTNULL"; நிறுத்து;

    இயல்பு:
      sqlite3_str_appendf(p, "%s", "expr");
      நிறுத்து;
  }

  எனில்( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_கோவை(p, p_கோவை->p_இடது);
    எனில்( p_கோவை->p_வலது ){
      sqlite3_str_append(p, ",", 1);
      display_P4_கோவை(p, p_கோவை->p_வலது);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#endif /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#எனில் VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
எழுத்து *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  எழுத்து *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_LENGTH);
  தேர்வு( pOp->p4type ){
    நிலை P4_KEYINFO: {
      முழு j;
      சாவி_Info *p_சாவி_Info = pOp->p4.p_சாவி_Info;
      ப்ரதிஜ்ஞா( p_சாவி_Info->aSortFlags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_சாவி_Info->n_சாவி_Field);
      க்கு(j=0; j<p_சாவி_Info->n_சாவி_Field; j++){
        CollSeq *pColl = p_சாவி_Info->aColl[j];
        மாறிலி எழுத்து *zColl = pColl ? pColl->zName : "";
        எனில்( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_சாவி_Info->aSortFlags[j] & KEYINFO_ORDER_DESC) ? "-" : "",
               (p_சாவி_Info->aSortFlags[j] & KEYINFO_ORDER_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      நிறுத்து;
    }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
    நிலை P4_கோவை: {
      display_P4_கோவை(&x, pOp->p4.p_கோவை);
      நிறுத்து;
    }
#endif
    நிலை P4_COLLSEQ: {
      நிலையான மாறிலி எழுத்து *மாறிலி encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      ப்ரதிஜ்ஞா( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      நிறுத்து;
    }
    நிலை P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      நிறுத்து;
    }
    நிலை P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      நிறுத்து;
    }
    நிலை P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      நிறுத்து;
    }
    நிலை P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      எனில்( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }இல்லையெனில் எனில்( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }இல்லையெனில் எனில்( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }இல்லையெனில் எனில்( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    நிலை P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      நிறுத்து;
    }
#endif
    நிலை P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      க்கு(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      நிறுத்து;
    }
    நிலை P4_SUBPROGRAM: {
      zP4 = "program";
      நிறுத்து;
    }
    நிலை P4_TABLE: {
      zP4 = pOp->p4.pTab->zName;
      நிறுத்து;
    }
    நிலை P4_INDEX: {
      zP4 = pOp->p4.pIdx->zName;
      நிறுத்து;
    }
    நிலை P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      நிறுத்து;
    }
    இயல்பு: {
      zP4 = pOp->p4.z;
    }
  }
  எனில்( zP4 ) sqlite3_str_appendall(&x, zP4);
  எனில்( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  திரும்பு sqlite3StrAccumFinish(&x);
}
#endif /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
வெறுமை sqlite3VdbeUsesBtree(Vdbe *p, முழு i){
  ப்ரதிஜ்ஞா( i>=0 && i<p->db->nDb && i<(முழு)அளவு(yDbMask)*8 );
  ப்ரதிஜ்ஞா( i<(முழு)அளவு(p->btreeMask)*8 );
  DbMaskSet(p->btreeMask, i);
  எனில்( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    DbMaskSet(p->lockMask, i);
  }
}

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE)
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
வெறுமை sqlite3VdbeEnter(Vdbe *p){
  முழு i;
  sqlite3 *db;
  Db *aDb;
  முழு nDb;
  எனில்( DbMaskAllZero(p->lockMask) ) திரும்பு;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  க்கு(i=0; i<nDb; i++){
    எனில்( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#endif

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
நிலையான SQLITE_NOINLINE வெறுமை vdbeLeave(Vdbe *p){
  முழு i;
  sqlite3 *db;
  Db *aDb;
  முழு nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  க்கு(i=0; i<nDb; i++){
    எனில்( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
வெறுமை sqlite3VdbeLeave(Vdbe *p){
  எனில்( DbMaskAllZero(p->lockMask) ) திரும்பு;  /* The common case */
  vdbeLeave(p);
}
#endif

#எனில் defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
வெறுமை sqlite3VdbePrintOp(FILE *pOut, முழு pc, VdbeOp *pOp){
  எழுத்து *zP4;
  எழுத்து *zCom;
  sqlite3 dummyDb;
  நிலையான மாறிலி எழுத்து *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  எனில்( pOut==0 ) pOut = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#இல்லையெனில்
  zCom = 0;
#endif
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(pOut, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(pOut);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3EndBenignMalloc();
}
#endif

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
நிலையான வெறுமை initMemArray(Mem *p, முழு N, sqlite3 *db, u16 flags){
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( N>0 ){
    செய்{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#ifdef SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#endif
      p++;
    }போது( (--N)>0 );
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
நிலையான வெறுமை releaseMemArray(Mem *p, முழு N){
  எனில்( p && N ){
    Mem *pEnd = &p[N];
    sqlite3 *db = p->db;
    ப்ரதிஜ்ஞா( db!=0 );
    எனில்( db->pnBytesFreed ){
      செய்{
        எனில்( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }போது( (++p)<pEnd );
      திரும்பு;
    }
    செய்{
      ப்ரதிஜ்ஞா( (&p[1])==pEnd || p[0].db==p[1].db );
      ப்ரதிஜ்ஞா( sqlite3VdbeCheckMemInvariants(p) );

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
      எனில்( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3VdbeFrameMemDel );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }இல்லையெனில் எனில்( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#ifdef SQLITE_DEBUG
      இல்லையெனில்{
        p->flags = MEM_Undefined;
      }
#endif
    }போது( (++p)<pEnd );
  }
}

#ifdef SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
முழு sqlite3VdbeFrameIsValid(VdbeFrame *pFrame){
  எனில்( pFrame->iFrameMagic!=SQLITE_FRAME_MAGIC ) திரும்பு 0;
  திரும்பு 1;
}
#endif


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
வெறுமை sqlite3VdbeFrameMemDel(வெறுமை *pArg){
  VdbeFrame *pFrame = (VdbeFrame*)pArg;
  ப்ரதிஜ்ஞா( sqlite3VdbeFrameIsValid(pFrame) );
  pFrame->pParent = pFrame->v->pDelFrame;
  pFrame->v->pDelFrame = pFrame;
}

#எனில் defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
முழு sqlite3VdbeNextOpcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  முழு eMode,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  முழு *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  முழு *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  முழு nRow;                            /* Stop when row count reaches this */
  முழு nSub = 0;                        /* Number of sub-vdbes seen so far */
  SubProgram **apSub = 0;              /* Array of sub-vdbes */
  முழு i;                               /* Next instruction address */
  முழு rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  முழு iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  nRow = p->nOp;
  எனில்( pSub!=0 ){
    எனில்( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/அளவு(Vdbe*);
      apSub = (SubProgram **)pSub->z;
    }
    க்கு(i=0; i<nSub; i++){
      nRow += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  போது(1){  /* Loop exits via break */
    i = iPc++;
    எனில்( i>=nRow ){
      p->rc = SQLITE_OK;
      rc = SQLITE_DONE;
      நிறுத்து;
    }
    எனில்( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }இல்லையெனில்{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      முழு j;
      i -= p->nOp;
      ப்ரதிஜ்ஞா( apSub!=0 );
      ப்ரதிஜ்ஞா( nSub>0 );
      க்கு(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        ப்ரதிஜ்ஞா( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    எனில்( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      முழு nByte = (nSub+1)*அளவு(SubProgram*);
      முழு j;
      க்கு(j=0; j<nSub; j++){
        எனில்( apSub[j]==aOp[i].p4.pProgram ) நிறுத்து;
      }
      எனில்( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        எனில்( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          நிறுத்து;
        }
        apSub = (SubProgram **)pSub->z;
        apSub[nSub++] = aOp[i].p4.pProgram;
        MemSetTypeFlag(pSub, MEM_Blob);
        pSub->n = nSub*அளவு(SubProgram*);
        nRow += aOp[i].p4.pProgram->nOp;
      }
    }
    எனில்( eMode==0 ) நிறுத்து;
#ifdef SQLITE_ENABLE_BYTECODE_VTAB
    எனில்( eMode==2 ){
      Op *pOp = aOp + i;
      எனில்( pOp->opcode==OP_OpenRead ) நிறுத்து;
      எனில்( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) நிறுத்து;
      எனில்( pOp->opcode==OP_ReopenIdx ) நிறுத்து;     
    }இல்லையெனில்
#endif
    {
      ப்ரதிஜ்ஞா( eMode==1 );
      எனில்( aOp[i].opcode==OP_Explain ) நிறுத்து;
      எனில்( aOp[i].opcode==OP_Init && iPc>1 ) நிறுத்து;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  திரும்பு rc;
}
#endif /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
வெறுமை sqlite3VdbeFrameDelete(VdbeFrame *p){
  முழு i;
  Mem *aMem = VdbeFrameMem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->nChildMem];
  ப்ரதிஜ்ஞா( sqlite3VdbeFrameIsValid(p) );
  க்கு(i=0; i<p->nChildCsr; i++){
    எனில்( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->nChildMem);
  sqlite3VdbeDeleteAuxData(p->v->db, &p->pAuxData, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#ifndef SQLITE_OMIT_EXPLAIN
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
முழு sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  முழு i;                               /* Loop counter */
  முழு rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  முழு bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  ப்ரதிஜ்ஞா( p->explain );
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_RUN_STATE );
  ப்ரதிஜ்ஞா( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  எனில்( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    திரும்பு SQLITE_ERROR;
  }

  எனில்( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    ப்ரதிஜ்ஞா( p->nMem>9 );
    pSub = &p->aMem[9];
  }இல்லையெனில்{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3VdbeNextOpcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  எனில்( rc==SQLITE_OK ){
    pOp = aOp + i;
    எனில்( AtomicLoad(&db->u1.isInterrupted) ){
      p->rc = SQLITE_INTERRUPT;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }இல்லையெனில்{
      எழுத்து *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      எனில்( p->explain==2 ){
        sqlite3VdbeMemSetInt64(pMem, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+1, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p3);
        sqlite3VdbeMemSetStr(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ப்ரதிஜ்ஞா( p->nResColumn==4 );
      }இல்லையெனில்{
        sqlite3VdbeMemSetInt64(pMem+0, i);
        sqlite3VdbeMemSetStr(pMem+1, (எழுத்து*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+3, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3VdbeMemSetInt64(pMem+6, pOp->p5);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          எழுத்து *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3VdbeMemSetStr(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#இல்லையெனில்
        sqlite3VdbeMemSetNull(pMem+7);
#endif
        sqlite3VdbeMemSetStr(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        ப்ரதிஜ்ஞா( p->nResColumn==8 );
      }
      p->pResultRow = pMem;
      எனில்( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }இல்லையெனில்{
        p->rc = SQLITE_OK;
        rc = SQLITE_ROW;
      }
    }
  }
  திரும்பு rc;
}
#endif /* SQLITE_OMIT_EXPLAIN */

#ifdef SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
வெறுமை sqlite3VdbePrintSql(Vdbe *p){
  மாறிலி எழுத்து *z = 0;
  எனில்( p->zSql ){
    z = p->zSql;
  }இல்லையெனில் எனில்( p->nOp>=1 ){
    மாறிலி VdbeOp *pOp = &p->aOp[0];
    எனில்( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      போது( sqlite3Isspace(*z) ) z++;
    }
  }
  எனில்( z ) printf("SQL: [%s]\n", z);
}
#endif

#எனில் !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
வெறுமை sqlite3VdbeIOTraceSql(Vdbe *p){
  முழு nOp = p->nOp;
  VdbeOp *pOp;
  எனில்( sqlite3IoTrace==0 ) திரும்பு;
  எனில்( nOp<1 ) திரும்பு;
  pOp = &p->aOp[0];
  எனில்( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    முழு i, j;
    எழுத்து z[1000];
    sqlite3_snprintf(அளவு(z), z, "%s", pOp->p4.z);
    க்கு(i=0; sqlite3Isspace(z[i]); i++){}
    க்கு(j=0; z[i]; i++){
      எனில்( sqlite3Isspace(z[i]) ){
        எனில்( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }இல்லையெனில்{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#endif /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
கட்டமைப்பு ReusableSpace {
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
நிலையான வெறுமை *allocSpace(
  கட்டமைப்பு ReusableSpace *p,  /* Bulk memory available for allocation */
  வெறுமை *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  எனில்( pBuf==0 ){
    nByte = ROUND8P(nByte);
    எனில்( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }இல்லையெனில்{
      p->nNeeded += nByte;
    }
  }
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pBuf) );
  திரும்பு pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
வெறுமை sqlite3VdbeRewind(Vdbe *p){
#எனில் defined(SQLITE_DEBUG)
  முழு i;
#endif
  ப்ரதிஜ்ஞா( p!=0 );
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  ப்ரதிஜ்ஞா( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#ifdef SQLITE_DEBUG
  க்கு(i=0; i<p->nMem; i++){
    ப்ரதிஜ்ஞா( p->aMem[i].db==p->db );
  }
#endif
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#ifdef VDBE_PROFILE
  க்கு(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].nCycle = 0;
  }
#endif
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
வெறுமை sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  முழு nVar;                      /* Number of parameters */
  முழு nMem;                      /* Number of VM memory registers */
  முழு nCursor;                   /* Number of cursors required */
  முழு nArg;                      /* Max number args to xFilter or xUpdate */
  முழு n;                         /* Loop counter */
  கட்டமைப்பு ReusableSpace x;        /* Reusable bulk memory */

  ப்ரதிஜ்ஞா( p!=0 );
  ப்ரதிஜ்ஞா( p->nOp>0 );
  ப்ரதிஜ்ஞா( pParse!=0 );
  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_INIT_STATE );
  ப்ரதிஜ்ஞா( pParse==p->pParse );
  ப்ரதிஜ்ஞா( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  ப்ரதிஜ்ஞா( db->mallocFailed==0 );
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
  எனில்( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(அளவு(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  ப்ரதிஜ்ஞா( x.nFree>=0 );
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_மதிப்புகள்(p, &nArg);
  p->usesStmtJournal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  எனில்( pParse->explain ){
    எனில்( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->nResColumn = 12 - 4*p->explain;
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
  p->aMem = allocSpace(&x, 0, nMem*அளவு(Mem));
  p->aVar = allocSpace(&x, 0, nVar*அளவு(Mem));
  p->apArg = allocSpace(&x, 0, nArg*அளவு(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*அளவு(VdbeCursor*));
  எனில்( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    எனில்( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*அளவு(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*அளவு(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*அளவு(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*அளவு(VdbeCursor*));
    }
  }
#ifdef SQLITE_DEBUG
  p->napArg = nArg;
#endif

  எனில்( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }இல்லையெனில்{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*அளவு(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
வெறுமை sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  எனில்( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
நிலையான SQLITE_NOINLINE வெறுமை freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  ப்ரதிஜ்ஞா( pCx->colCache );
  pCx->colCache = 0;
  pCx->pCache = 0;
  எனில்( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
வெறுமை sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  எனில்( pCx->colCache ){
    freeCursorWithCache(p, pCx);
    திரும்பு;
  }
  தேர்வு( pCx->eCurType ){
    நிலை CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      நிறுத்து;
    }
    நிலை CURTYPE_BTREE: {
      ப்ரதிஜ்ஞா( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    நிலை CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      மாறிலி sqlite3_module *pModule = pVCur->pVtab->pModule;
      ப்ரதிஜ்ஞா( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      நிறுத்து;
    }
#endif
  }
}

/*
** Close all cursors in the current frame.
*/
நிலையான வெறுமை closeCursorsInFrame(Vdbe *p){
  முழு i;
  க்கு(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    எனில்( pC ){
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
முழு sqlite3VdbeFrameRestore(VdbeFrame *pFrame){
  Vdbe *v = pFrame->v;
  closeCursorsInFrame(v);
  v->aOp = pFrame->aOp;
  v->nOp = pFrame->nOp;
  v->aMem = pFrame->aMem;
  v->nMem = pFrame->nMem;
  v->apCsr = pFrame->apCsr;
  v->nCursor = pFrame->nCursor;
  v->db->lastRowid = pFrame->lastRowid;
  v->nChange = pFrame->nChange;
  v->db->nChange = pFrame->nDbChange;
  sqlite3VdbeDeleteAuxData(v->db, &v->pAuxData, -1, 0);
  v->pAuxData = pFrame->pAuxData;
  pFrame->pAuxData = 0;
  திரும்பு pFrame->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
நிலையான வெறுமை closeAllCursors(Vdbe *p){
  எனில்( p->pFrame ){
    VdbeFrame *pFrame;
    க்கு(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    sqlite3VdbeFrameRestore(pFrame);
    p->pFrame = 0;
    p->nFrame = 0;
  }
  ப்ரதிஜ்ஞா( p->nFrame==0 );
  closeCursorsInFrame(p);
  releaseMemArray(p->aMem, p->nMem);
  போது( p->pDelFrame ){
    VdbeFrame *pDel = p->pDelFrame;
    p->pDelFrame = pDel->pParent;
    sqlite3VdbeFrameDelete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  எனில்( p->pAuxData ) sqlite3VdbeDeleteAuxData(p->db, &p->pAuxData, -1, 0);
  ப்ரதிஜ்ஞா( p->pAuxData==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
வெறுமை sqlite3_Vdbe_Set_இலக்கம்_Cols(Vdbe *p, முழு nResColumn){
  முழு n;
  sqlite3 *db = p->db;

  எனில்( p->nResAlloc ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->aColName);
  }
  n = nResColumn*COLNAME_N;
  p->nResColumn = p->nResAlloc = (u16)nResColumn;
  p->aColName = (Mem*)sqlite3DbMallocRawNN(db, அளவு(Mem)*n );
  எனில்( p->aColName==0 ) திரும்பு;
  initMemArray(p->aColName, n, db, MEM_Null);
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
முழு sqlite3VdbeSetColName(
  Vdbe *p,                         /* Vdbe being configured */
  முழு idx,                         /* Index of column zName applies to */
  முழு var,                         /* One of the COLNAME_* constants */
  மாறிலி எழுத்து *zName,               /* Pointer to buffer containing name */
  வெறுமை (*xDel)(வெறுமை*)              /* Memory management strategy for zName */
){
  முழு rc;
  Mem *pColName;
  ப்ரதிஜ்ஞா( idx<p->nResAlloc );
  ப்ரதிஜ்ஞா( var<COLNAME_N );
  எனில்( p->db->mallocFailed ){
    ப்ரதிஜ்ஞா( !zName || xDel!=SQLITE_DYNAMIC );
    திரும்பு SQLITE_NOMEM_BKPT;
  }
  ப்ரதிஜ்ஞா( p->aColName!=0 );
  pColName = &(p->aColName[idx+var*p->nResAlloc]);
  rc = sqlite3VdbeMemSetText(pColName, zName, -1, xDel);
  ப்ரதிஜ்ஞா( rc!=0 || !zName || (pColName->flags&MEM_Term)!=0 );
  திரும்பு rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
நிலையான முழு vdbeCommit(sqlite3 *db, Vdbe *p){
  முழு i;
  முழு nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  முழு rc = SQLITE_OK;
  முழு needXcommit = 0;

#ifdef SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  UNUSED_PARAMETER(p);
#endif

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
  க்கு(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    எனில்( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      நிலையான மாறிலி u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /* MEMORY    */ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      எனில்( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3PagerGetJournalMode(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        ப்ரதிஜ்ஞா( i!=1 );
        nTrans++;
      }
      rc = sqlite3PagerExclusiveLock(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  எனில்( rc!=SQLITE_OK ){
    திரும்பு rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  எனில்( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    எனில்( rc ){
      திரும்பு SQLITE_CONSTRAINT_COMMITHOOK;
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
  எனில்( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    எனில்( needXcommit ){
      க்கு(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        எனில்( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    க்கு(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      முழு txn = sqlite3BtreeTxnState(pBt);
      எனில்( txn!=SQLITE_TXN_NONE ){
        ப்ரதிஜ்ஞா( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    எனில்( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#ifndef SQLITE_OMIT_DISKIO
  இல்லையெனில்{
    sqlite3_vfs *pVfs = db->pVfs;
    எழுத்து *zSuper = 0;   /* File-name for the super-journal */
    எழுத்து மாறிலி *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 offset = 0;
    முழு res;
    முழு retry_மொத்தம் = 0;
    முழு nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    எனில்( zSuper==0 ) திரும்பு SQLITE_NOMEM_BKPT;
    zSuper += 4;
    செய் {
      u32 iRandom;
      எனில்( retry_மொத்தம் ){
        எனில்( retry_மொத்தம்>100 ){
          sqlite3_log(SQLITE_FULL, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          நிறுத்து;
        }இல்லையெனில் எனில்( retry_மொத்தம்==1 ){
          sqlite3_log(SQLITE_FULL, "MJ collide: %s", zSuper);
        }
      }
      retry_மொத்தம்++;
      sqlite3_randomness(அளவு(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      ப்ரதிஜ்ஞா( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }போது( rc==SQLITE_OK && res );
    எனில்( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    எனில்( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      திரும்பு rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    க்கு(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      எனில்( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        எழுத்து மாறிலி *zFile = sqlite3BtreeGetJournalname(pBt);
        எனில்( zFile==0 ){
          தொடர்;  /* Ignore TEMP and :memory: databases */
        }
        ப்ரதிஜ்ஞா( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,offset);
        offset += sqlite3Strlen30(zFile)+1;
        எனில்( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          திரும்பு rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    எனில்( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_NORMAL))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      திரும்பு rc;
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
    க்கு(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      எனில்( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    ப்ரதிஜ்ஞா( rc!=SQLITE_BUSY );
    எனில்( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      திரும்பு rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    எனில்( rc ){
      திரும்பு rc;
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
    க்கு(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      எனில்( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3EndBenignMalloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#endif

  திரும்பு rc;
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
#ifndef NDEBUG
நிலையான வெறுமை checkActiveVdbeCnt(sqlite3 *db){
  Vdbe *p;
  முழு cnt = 0;
  முழு nWrite = 0;
  முழு nRead = 0;
  p = db->pVdbe;
  போது( p ){
    எனில்( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      எனில்( p->readOnly==0 ) nWrite++;
      எனில்( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  ப்ரதிஜ்ஞா( cnt==db->nVdbeActive );
  ப்ரதிஜ்ஞா( nWrite==db->nVdbeWrite );
  ப்ரதிஜ்ஞா( nRead==db->nVdbeRead );
}
#இல்லையெனில்
#define checkActiveVdbeCnt(x)
#endif

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
நிலையான SQLITE_NOINLINE முழு vdbeCloseStatement(Vdbe *p, முழு eOp){
  sqlite3 *மாறிலி db = p->db;
  முழு rc = SQLITE_OK;
  முழு i;
  மாறிலி முழு iSavepoint = p->iStatement-1;

  ப்ரதிஜ்ஞா( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  ப்ரதிஜ்ஞா( db->nStatement>0 );
  ப்ரதிஜ்ஞா( p->iStatement==(db->nStatement+db->nSavepoint) );

  க்கு(i=0; i<db->nDb; i++){
    முழு rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    எனில்( pBt ){
      எனில்( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      எனில்( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      எனில்( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  எனில்( rc==SQLITE_OK ){
    எனில்( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    எனில்( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  எனில்( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  திரும்பு rc;
}
முழு sqlite3VdbeCloseStatement(Vdbe *p, முழு eOp){
  எனில்( p->db->nStatement && p->iStatement ){
    திரும்பு vdbeCloseStatement(p, eOp);
  }
  திரும்பு SQLITE_OK;
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
#ifndef SQLITE_OMIT_FOREIGN_சாவி
நிலையான SQLITE_NOINLINE முழு vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  எனில்( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) திரும்பு SQLITE_ERROR;
  திரும்பு SQLITE_CONSTRAINT_FOREIGNKEY;
}
முழு sqlite3VdbeCheckFkImmediate(Vdbe *p){
  எனில்( p->nFkConstraint==0 ) திரும்பு SQLITE_OK;
  திரும்பு vdbeFkError(p);
}
முழு sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  எனில்( (db->nDeferredCons+db->nDeferredImmCons)==0 ) திரும்பு SQLITE_OK;
  திரும்பு vdbeFkError(p);
}
#endif

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
முழு sqlite3VdbeHalt(Vdbe *p){
  முழு rc;                         /* Used to store transient return codes */
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

  ப்ரதிஜ்ஞா( p->eVdbeState==VDBE_RUN_STATE );
  எனில்( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  checkActiveVdbeCnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  எனில்( p->bIsReader ){
    முழு mrc;   /* Primary error code from p->rc */
    முழு eStatementOp = 0;
    முழு isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    எனில்( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_INTERRUPT
                    || mrc==SQLITE_FULL;
    }இல்லையெனில்{
      mrc = isSpecialError = 0;
    }
    எனில்( isSpecialError ){
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
      எனில்( !p->readOnly || mrc!=SQLITE_INTERRUPT ){
        எனில்( (mrc==SQLITE_NOMEM || mrc==SQLITE_FULL) && p->usesStmtJournal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }இல்லையெனில்{
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
    எனில்( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (வெறுமை)sqlite3VdbeCheckFkImmediate(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    எனில்( !sqlite3VtabInSync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      எனில்( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        எனில்( rc!=SQLITE_OK ){
          எனில்( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            திரும்பு SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }இல்லையெனில் எனில்( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }இல்லையெனில்{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        எனில்( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          திரும்பு SQLITE_BUSY;
        }இல்லையெனில் எனில்( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }இல்லையெனில்{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3CommitInternalChanges(db);
        }
      }இல்லையெனில் எனில்( p->rc==SQLITE_SCHEMA && db->nVdbeActive>1 ){
        p->nChange = 0;
      }இல்லையெனில்{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }இல்லையெனில் எனில்( eStatementOp==0 ){
      எனில்( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }இல்லையெனில் எனில்( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }இல்லையெனில்{
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
    எனில்( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      எனில்( rc ){
        எனில்( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_செய்தி);
          p->z_Err_செய்தி = 0;
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
    எனில்( p->changeCntOn ){
      எனில்( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3VdbeSetChanges(db, p->nChange);
      }இல்லையெனில்{
        sqlite3VdbeSetChanges(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->nVdbeActive--;
  எனில்( !p->readOnly ) db->nVdbeWrite--;
  எனில்( p->bIsReader ) db->nVdbeRead--;
  ப்ரதிஜ்ஞா( db->nVdbeActive>=db->nVdbeRead );
  ப்ரதிஜ்ஞா( db->nVdbeRead>=db->nVdbeWrite );
  ப்ரதிஜ்ஞா( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  checkActiveVdbeCnt(db);
  எனில்( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  எனில்( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  ப்ரதிஜ்ஞா( db->nVdbeActive>0 || db->autoCommit==0 || db->nStatement==0 );
  திரும்பு (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
வெறுமை sqlite3VdbeResetStepResult(Vdbe *p){
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
முழு sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  முழு rc = p->rc;
  எனில்( p->z_Err_செய்தி ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    எனில்( db->pErr==0 ) db->pErr = sqlite3_மதிப்பு_New(db);
    sqlite3_மதிப்பு_Set_Str(db->pErr, -1, p->z_Err_செய்தி, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3EndBenignMalloc();
    db->bBenignMalloc--;
  }இல்லையெனில் எனில்( db->pErr ){
    sqlite3_மதிப்பு_Set_Null(db->pErr);
  }
  db->errCode = rc;
  db->errByteOffset = -1;
  திரும்பு rc;
}

#ifdef SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
நிலையான வெறுமை vdbeInvokeSqllog(Vdbe *v){
  எனில்( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    எழுத்து *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    ப்ரதிஜ்ஞா( v->db->init.busy==0 );
    எனில்( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#இல்லையெனில்
# define vdbeInvokeSqllog(x)
#endif

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
முழு sqlite3VdbeReset(Vdbe *p){
#எனில் defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  முழு i;
#endif

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  எனில்( p->eVdbeState==VDBE_RUN_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  எனில்( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    எனில்( db->pErr || p->z_Err_செய்தி ){
      sqlite3VdbeTransferError(p);
    }இல்லையெனில்{
      db->errCode = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#ifdef SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  எனில்( p->apCsr ) க்கு(i=0; i<p->nCursor; i++) ப்ரதிஜ்ஞா( p->apCsr[i]==0 );
  எனில்( p->aMem ){
    க்கு(i=0; i<p->nMem; i++) ப்ரதிஜ்ஞா( p->aMem[i].flags==MEM_Undefined );
  }
#endif
  எனில்( p->z_Err_செய்தி ){
    sqlite3DbFree(db, p->z_Err_செய்தி);
    p->z_Err_செய்தி = 0;
  }
  p->pResultRow = 0;
#ifdef SQLITE_DEBUG
  p->nWrite = 0;
#endif

  /* Save profiling information from this VDBE run.
  */
#ifdef VDBE_PROFILE
  {
    FILE *out = fopen("vdbe_profile.out", "a");
    எனில்( out ){
      fprintf(out, "---- ");
      க்கு(i=0; i<p->nOp; i++){
        fprintf(out, "%02x", p->aOp[i].opcode);
      }
      fprintf(out, "\n");
      எனில்( p->zSql ){
        எழுத்து c, pc = 0;
        fprintf(out, "-- ");
        க்கு(i=0; (c = p->zSql[i])!=0; i++){
          எனில்( pc=='\n' ) fprintf(out, "-- ");
          putc(c, out);
          pc = c;
        }
        எனில்( pc!='\n' ) fprintf(out, "\n");
      }
      க்கு(i=0; i<p->nOp; i++){
        எழுத்து zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 cycles = p->aOp[i].nCycle;
        sqlite3_snprintf(அளவு(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           cycles,
           cnt>0 ? cycles/cnt : 0
        );
        fprintf(out, "%s", zHdr);
        sqlite3VdbePrintOp(out, i, &p->aOp[i]);
      }
      fclose(out);
    }
  }
#endif
  திரும்பு p->rc & db->errMask;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
முழு sqlite3VdbeFinalize(Vdbe *p){
  முழு rc = SQLITE_OK;
  ப்ரதிஜ்ஞா( VDBE_RUN_STATE>VDBE_READY_STATE );
  ப்ரதிஜ்ஞா( VDBE_HALT_STATE>VDBE_READY_STATE );
  ப்ரதிஜ்ஞா( VDBE_INIT_STATE<VDBE_READY_STATE );
  எனில்( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    ப்ரதிஜ்ஞா( (rc & p->db->errMask)==rc );
  }
  sqlite3VdbeDelete(p);
  திரும்பு rc;
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
வெறுமை sqlite3VdbeDeleteAuxData(sqlite3 *db, AuxData **pp, முழு iOp, முழு mask){
  போது( *pp ){
    AuxData *pAux = *pp;
    எனில்( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(mask & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      எனில்( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->pNextAux;
      sqlite3DbFree(db, pAux);
    }இல்லையெனில்{
      pp= &pAux->pNextAux;
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
நிலையான வெறுமை sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  SubProgram *pSub, *pNext;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( p->db==0 || p->db==db );
  எனில்( p->aColName ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->aColName);
  }
  க்கு(pSub=p->pProgram; pSub; pSub=pNext){
    pNext = pSub->pNext;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  எனில்( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    எனில்( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    எனில்( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  எனில்( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#ifdef SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    க்கு(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->pNextStr;
      sqlite3DbFree(db, pThis);
    }
  }
#endif
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  {
    முழு i;
    க்கு(i=0; i<p->nScan; i++){
      sqlite3DbFree(db, p->aScan[i].zName);
    }
    sqlite3DbFree(db, p->aScan);
  }
#endif
}

/*
** Delete an entire VDBE.
*/
வெறுமை sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  ப்ரதிஜ்ஞா( p!=0 );
  db = p->db;
  ப்ரதிஜ்ஞா( db!=0 );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  எனில்( db->pnBytesFreed==0 ){
    ப்ரதிஜ்ஞா( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    எனில்( p->pVNext ){
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
முழு SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor *p){
  முழு res, rc;
#ifdef SQLITE_TEST
  வெளி முழு sqlite3_தேடல்_மொத்தம்;
#endif
  ப்ரதிஜ்ஞா( p->deferredMoveto );
  ப்ரதிஜ்ஞா( p->isTable );
  ப்ரதிஜ்ஞா( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3BtreeTableMoveto(p->uc.pCursor, p->movetoTarget, 0, &res);
  எனில்( rc ) திரும்பு rc;
  எனில்( res!=0 ) திரும்பு SQLITE_CORRUPT_BKPT;
#ifdef SQLITE_TEST
  sqlite3_தேடல்_மொத்தம்++;
#endif
  p->deferredMoveto = 0;
  p->cacheStatus = CACHE_STALE;
  திரும்பு SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
முழு SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  முழு isDifferentRow, rc;
  ப்ரதிஜ்ஞா( p->eCurType==CURTYPE_BTREE );
  ப்ரதிஜ்ஞா( p->uc.pCursor!=0 );
  ப்ரதிஜ்ஞா( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &isDifferentRow);
  p->cacheStatus = CACHE_STALE;
  எனில்( isDifferentRow ) p->nullRow = 1;
  திரும்பு rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
முழு sqlite3VdbeCursorRestore(VdbeCursor *p){
  ப்ரதிஜ்ஞா( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  எனில்( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    திரும்பு sqlite3VdbeHandleMovedCursor(p);
  }
  திரும்பு SQLITE_OK;
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

#எனில் 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, முழு file_format, u32 *pLen){
  முழு flags = pMem->flags;
  u32 n;

  ப்ரதிஜ்ஞா( pLen!=0 );
  எனில்( flags&MEM_Null ){
    *pLen = 0;
    திரும்பு 0;
  }
  எனில்( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    எனில்( i<0 ){
      u = ~i;
    }இல்லையெனில்{
      u = i;
    }
    எனில்( u<=127 ){
      எனில்( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        திரும்பு 8+(u32)u;
      }இல்லையெனில்{
        *pLen = 1;
        திரும்பு 1;
      }
    }
    எனில்( u<=32767 ){ *pLen = 2; திரும்பு 2; }
    எனில்( u<=8388607 ){ *pLen = 3; திரும்பு 3; }
    எனில்( u<=2147483647 ){ *pLen = 4; திரும்பு 4; }
    எனில்( u<=MAX_6BYTE ){ *pLen = 6; திரும்பு 5; }
    *pLen = 8;
    எனில்( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (இரட்டை)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      திரும்பு 7;
    }
    திரும்பு 6;
  }
  எனில்( flags&MEM_Real ){
    *pLen = 8;
    திரும்பு 7;
  }
  ப்ரதிஜ்ஞா( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  ப்ரதிஜ்ஞா( pMem->n>=0 );
  n = (u32)pMem->n;
  எனில்( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  திரும்பு ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#endif /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
மாறிலி u8 sqlite3SmallTypeSizes[128] = {
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
  எனில்( serial_type>=128 ){
    திரும்பு (serial_type-12)/2;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    திரும்பு sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  ப்ரதிஜ்ஞா( serial_type<128 );
  திரும்பு sqlite3SmallTypeSizes[serial_type]; 
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
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
u64 sqlite3FloatSwap(u64 in){
  ஒன்றிப்பு {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = in;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  திரும்பு u.r;
}
#endif /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
நிலையான முழு sqlite3VdbeSerialGet7(
  மாறிலி unsigned எழுத்து *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  ப்ரதிஜ்ஞா( அளவு(x)==8 && அளவு(pMem->u.r)==8 );
  swapMixedEndianFloat(x);
  memcpy(&pMem->u.r, &x, அளவு(x));
  எனில்( IsNaN(x) ){
    pMem->flags = MEM_Null;
    திரும்பு 1;
  }
  pMem->flags = MEM_Real;
  திரும்பு 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
வெறுமை sqlite3VdbeSerialGet(
  மாறிலி unsigned எழுத்து *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  தேர்வு( serial_type ){
    நிலை 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      திரும்பு;
    }
    நிலை 11:   /* Reserved for future use */
    நிலை 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      திரும்பு;
    }
    நிலை 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#ifdef __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      எனில்( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#endif
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      திரும்பு;
    }
    நிலை 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      திரும்பு;
    }
    நிலை 8:    /* Integer 0 */
    நிலை 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      திரும்பு;
    }
    இயல்பு: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      நிலையான மாறிலி u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (எழுத்து *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      திரும்பு;
    }
  }
  திரும்பு;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(
  சாவி_Info *p_சாவி_Info               /* Description of the record */
){
  UnpackedRecord *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  ப்ரதிஜ்ஞா( அளவு(UnpackedRecord) + அளவு(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(அளவு(UnpackedRecord)) + அளவு(Mem)*(p_சாவி_Info->n_சாவி_Field+1);
  p = (UnpackedRecord *)sqlite3DbMallocRaw(p_சாவி_Info->db, nByte);
  எனில்( !p ) திரும்பு 0;
  p->aMem = (Mem*)&((எழுத்து*)p)[ROUND8P(அளவு(UnpackedRecord))];
  p->p_சாவி_Info = p_சாவி_Info;
  p->nField = p_சாவி_Info->n_சாவி_Field + 1;
  திரும்பு p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
வெறுமை sqlite3VdbeRecordUnpack(
  முழு n_சாவி,              /* Size of the binary record */
  மாறிலி வெறுமை *p_சாவி,      /* The binary record */
  UnpackedRecord *p      /* Populate this structure before returning. */
){
  மாறிலி unsigned எழுத்து *a_சாவி = (மாறிலி unsigned எழுத்து *)p_சாவி;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  சாவி_Info *p_சாவி_Info = p->p_சாவி_Info;

  p->default_rc = 0;
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_சாவி, szHdr);
  d = szHdr;
  u = 0;
  போது( idx<szHdr && d<=(u32)n_சாவி ){
    u32 serial_type;

    idx += getVarint32(&a_சாவி[idx], serial_type);
    pMem->enc = p_சாவி_Info->enc;
    pMem->db = p_சாவி_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_சாவி[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    எனில்( (++u)>=p->nField ) நிறுத்து;
    pMem++;
  }
  எனில்( d>(u32)n_சாவி && u ){
    ப்ரதிஜ்ஞா( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3VdbeMemSetNull(pMem-(u<p->nField));
  }
  testcase( u == p_சாவி_Info->n_சாவி_Field + 1 );
  testcase( u < p_சாவி_Info->n_சாவி_Field + 1 );
  ப்ரதிஜ்ஞா( u<=p_சாவி_Info->n_சாவி_Field + 1 );
  p->nField = u;
}

#ifdef SQLITE_DEBUG
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
நிலையான முழு vdbeRecordCompareDebug(
  முழு nKey1, மாறிலி வெறுமை *pKey1, /* Left key */
  மாறிலி UnpackedRecord *pPKey2, /* Right key */
  முழு desiredResult             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  முழு i = 0;
  முழு rc = 0;
  மாறிலி unsigned எழுத்து *aKey1 = (மாறிலி unsigned எழுத்து *)pKey1;
  சாவி_Info *p_சாவி_Info;
  Mem mem1;

  p_சாவி_Info = pPKey2->p_சாவி_Info;
  எனில்( p_சாவி_Info->db==0 ) திரும்பு 1;
  mem1.enc = p_சாவி_Info->enc;
  mem1.db = p_சாவி_Info->db;
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
  எனில்( szHdr1>98307 ) திரும்பு SQLITE_CORRUPT;
  d1 = szHdr1;
  ப்ரதிஜ்ஞா( p_சாவி_Info->nAllField>=pPKey2->nField || CORRUPT_DB );
  ப்ரதிஜ்ஞா( p_சாவி_Info->aSortFlags!=0 );
  ப்ரதிஜ்ஞா( p_சாவி_Info->n_சாவி_Field>0 );
  ப்ரதிஜ்ஞா( idx1<=szHdr1 || CORRUPT_DB );
  செய்{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    எனில்( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      எனில்( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        திரும்பு 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      நிறுத்து;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_சாவி_Info->nAllField>i ? p_சாவி_Info->aColl[i] : 0);
    எனில்( rc!=0 ){
      ப்ரதிஜ்ஞா( mem1.szMalloc==0 );  /* See comment below */
      எனில்( (p_சாவி_Info->aSortFlags[i] & KEYINFO_ORDER_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      எனில்( p_சாவி_Info->aSortFlags[i] & KEYINFO_ORDER_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      செல் debugCompareEnd;
    }
    i++;
  }போது( idx1<szHdr1 && i<pPKey2->nField );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  ப்ரதிஜ்ஞா( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debugCompareEnd:
  எனில்( desiredResult==0 && rc==0 ) திரும்பு 1;
  எனில்( desiredResult<0 && rc<0 ) திரும்பு 1;
  எனில்( desiredResult>0 && rc>0 ) திரும்பு 1;
  எனில்( CORRUPT_DB ) திரும்பு 1;
  எனில்( p_சாவி_Info->db->mallocFailed ) திரும்பு 1;
  திரும்பு 0;
}
#endif

#ifdef SQLITE_DEBUG
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
நிலையான வெறுமை vdbe_Assert_Field_மொத்தம்_Within_Limits(
  முழு n_சாவி, மாறிலி வெறுமை *p_சாவி,   /* The record to verify */
  மாறிலி சாவி_Info *p_சாவி_Info       /* Compare size with this KeyInfo */
){
  முழு nField = 0;
  u32 szHdr;
  u32 idx;
  u32 notUsed;
  மாறிலி unsigned எழுத்து *a_சாவி = (மாறிலி unsigned எழுத்து*)p_சாவி;

  எனில்( CORRUPT_DB ) திரும்பு;
  idx = getVarint32(a_சாவி, szHdr);
  ப்ரதிஜ்ஞா( n_சாவி>=0 );
  ப்ரதிஜ்ஞா( szHdr<=(u32)n_சாவி );
  போது( idx<szHdr ){
    idx += getVarint32(a_சாவி+idx, notUsed);
    nField++;
  }
  ப்ரதிஜ்ஞா( nField <= p_சாவி_Info->nAllField );
}
#இல்லையெனில்
# define vdbe_Assert_Field_மொத்தம்_Within_Limits(A,B,C)
#endif

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
நிலையான SQLITE_NOINLINE முழு vdbeCompareMemStringWithEncodingChange(
  மாறிலி Mem *pMem1,
  மாறிலி Mem *pMem2,
  மாறிலி CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  முழு rc;
  மாறிலி வெறுமை *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_மதிப்பு_Text((sqlite3_மதிப்பு*)&c1, pColl->enc);
  v2 = sqlite3_மதிப்பு_Text((sqlite3_மதிப்பு*)&c2, pColl->enc);
  எனில்( (v1==0 || v2==0) ){
    எனில்( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }இல்லையெனில்{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  திரும்பு rc;
}
நிலையான முழு vdbeCompareMemString(
  மாறிலி Mem *pMem1,
  மாறிலி Mem *pMem2,
  மாறிலி CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  எனில்( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    திரும்பு pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }இல்லையெனில்{
    திரும்பு vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
நிலையான முழு isAllZero(மாறிலி எழுத்து *z, முழு n){
  முழு i;
  க்கு(i=0; i<n; i++){
    எனில்( z[i] ) திரும்பு 0;
  }
  திரும்பு 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE முழு sqlite3BlobCompare(மாறிலி Mem *pB1, மாறிலி Mem *pB2){
  முழு c;
  முழு n1 = pB1->n;
  முழு n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  ப்ரதிஜ்ஞா( (pB1->flags & MEM_Zero)==0 || n1==0 );
  ப்ரதிஜ்ஞா( (pB2->flags & MEM_Zero)==0 || n2==0 );

  எனில்( (pB1->flags|pB2->flags) & MEM_Zero ){
    எனில்( pB1->flags & pB2->flags & MEM_Zero ){
      திரும்பு pB1->u.nZero - pB2->u.nZero;
    }இல்லையெனில் எனில்( pB1->flags & MEM_Zero ){
      எனில்( !isAllZero(pB2->z, pB2->n) ) திரும்பு -1;
      திரும்பு pB1->u.nZero - n2;
    }இல்லையெனில்{
      எனில்( !isAllZero(pB1->z, pB1->n) ) திரும்பு +1;
      திரும்பு n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  எனில்( c ) திரும்பு c;
  திரும்பு n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#எனில் defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
நிலையான முழு SQLITE_NOINLINE doubleLt(இரட்டை a, இரட்டை b){ திரும்பு a<b; }
நிலையான முழு SQLITE_NOINLINE doubleEq(இரட்டை a, இரட்டை b){ திரும்பு a==b; }
#endif

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
முழு sqlite3IntFloatCompare(i64 i, இரட்டை r){
  எனில்( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    திரும்பு 1;
  }இல்லையெனில்{
    i64 y;
    எனில்( r<-9223372036854775808.0 ) திரும்பு +1;
    எனில்( r>=9223372036854775808.0 ) திரும்பு -1;
    y = (i64)r;
    எனில்( i<y ) திரும்பு -1;
    எனில்( i>y ) திரும்பு +1;
    testcase( doubleLt(((இரட்டை)i),r) );
    testcase( doubleLt(r,((இரட்டை)i)) );
    testcase( doubleEq(r,((இரட்டை)i)) );
    திரும்பு (((இரட்டை)i)<r) ? -1 : (((இரட்டை)i)>r);
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
முழு sqlite3MemCompare(மாறிலி Mem *pMem1, மாறிலி Mem *pMem2, மாறிலி CollSeq *pColl){
  முழு f1, f2;
  முழு combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  ப்ரதிஜ்ஞா( !sqlite3VdbeMemIsRowSet(pMem1) && !sqlite3VdbeMemIsRowSet(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  எனில்( combined_flags&MEM_Null ){
    திரும்பு (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  எனில்( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    எனில்( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      எனில்( pMem1->u.i < pMem2->u.i ) திரும்பு -1;
      எனில்( pMem1->u.i > pMem2->u.i ) திரும்பு +1;
      திரும்பு 0;
    }
    எனில்( (f1 & f2 & MEM_Real)!=0 ){
      எனில்( pMem1->u.r < pMem2->u.r ) திரும்பு -1;
      எனில்( pMem1->u.r > pMem2->u.r ) திரும்பு +1;
      திரும்பு 0;
    }
    எனில்( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      எனில்( (f2&MEM_Real)!=0 ){
        திரும்பு sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }இல்லையெனில் எனில்( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        எனில்( pMem1->u.i < pMem2->u.i ) திரும்பு -1;
        எனில்( pMem1->u.i > pMem2->u.i ) திரும்பு +1;
        திரும்பு 0;
      }இல்லையெனில்{
        திரும்பு -1;
      }
    }
    எனில்( (f1&MEM_Real)!=0 ){
      எனில்( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        திரும்பு -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }இல்லையெனில்{
        திரும்பு -1;
      }
    }
    திரும்பு +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  எனில்( combined_flags&MEM_Str ){
    எனில்( (f1 & MEM_Str)==0 ){
      திரும்பு 1;
    }
    எனில்( (f2 & MEM_Str)==0 ){
      திரும்பு -1;
    }

    ப்ரதிஜ்ஞா( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    ப்ரதிஜ்ஞா( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    ப்ரதிஜ்ஞா( !pColl || pColl->xCmp );

    எனில்( pColl ){
      திரும்பு vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  திரும்பு sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
நிலையான i64 vdbeRecordDecodeInt(u32 serial_type, மாறிலி u8 *a_சாவி){
  u32 y;
  ப்ரதிஜ்ஞா( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  தேர்வு( serial_type ){
    நிலை 0:
    நிலை 1:
      testcase( a_சாவி[0]&0x80 );
      திரும்பு ONE_BYTE_INT(a_சாவி);
    நிலை 2:
      testcase( a_சாவி[0]&0x80 );
      திரும்பு TWO_BYTE_INT(a_சாவி);
    நிலை 3:
      testcase( a_சாவி[0]&0x80 );
      திரும்பு THREE_BYTE_INT(a_சாவி);
    நிலை 4: {
      testcase( a_சாவி[0]&0x80 );
      y = FOUR_BYTE_UINT(a_சாவி);
      திரும்பு (i64)*(முழு*)&y;
    }
    நிலை 5: {
      testcase( a_சாவி[0]&0x80 );
      திரும்பு FOUR_BYTE_UINT(a_சாவி+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_சாவி);
    }
    நிலை 6: {
      u64 x = FOUR_BYTE_UINT(a_சாவி);
      testcase( a_சாவி[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_சாவி+4);
      திரும்பு (i64)*(i64*)&x;
    }
  }

  திரும்பு (serial_type - 8);
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
முழு sqlite3VdbeRecordCompareWithSkip(
  முழு nKey1, மாறிலி வெறுமை *pKey1,   /* Left key */
  UnpackedRecord *pPKey2,         /* Right key */
  முழு bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  முழு i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  முழு rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  சாவி_Info *p_சாவி_Info;
  மாறிலி unsigned எழுத்து *aKey1 = (மாறிலி unsigned எழுத்து *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  எனில்( bSkip ){
    u32 s1 = aKey1[1];
    எனில்( s1<0x80 ){
      idx1 = 2;
    }இல்லையெனில்{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }இல்லையெனில்{
    எனில்( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }இல்லையெனில்{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  எனில்( d1>(unsigned)nKey1 ){
    pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
    திரும்பு 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  ப்ரதிஜ்ஞா( pPKey2->p_சாவி_Info->nAllField>=pPKey2->nField
       || CORRUPT_DB );
  ப்ரதிஜ்ஞா( pPKey2->p_சாவி_Info->aSortFlags!=0 );
  ப்ரதிஜ்ஞா( pPKey2->p_சாவி_Info->n_சாவி_Field>0 );
  ப்ரதிஜ்ஞா( idx1<=szHdr1 || CORRUPT_DB );
  போது( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    எனில்( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      எனில்( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }இல்லையெனில் எனில்( serial_type==0 ){
        rc = -1;
      }இல்லையெனில் எனில்( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }இல்லையெனில்{
        i64 lhs = vdbeRecordDecodeInt(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        எனில்( lhs<rhs ){
          rc = -1;
        }இல்லையெனில் எனில்( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    இல்லையெனில் எனில்( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      எனில்( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }இல்லையெனில் எனில்( serial_type==0 ){
        rc = -1;
      }இல்லையெனில்{
        எனில்( serial_type==7 ){
          எனில்( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }இல்லையெனில் எனில்( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }இல்லையெனில் எனில்( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }இல்லையெனில்{
            ப்ரதிஜ்ஞா( rc==0 );
          }
        }இல்லையெனில்{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    இல்லையெனில் எனில்( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      எனில்( serial_type<12 ){
        rc = -1;
      }இல்லையெனில் எனில்( !(serial_type & 0x01) ){
        rc = +1;
      }இல்லையெனில்{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(unsigned)nKey1 );
        testcase( (d1+mem1.n+1)==(unsigned)nKey1 );
        எனில்( (d1+mem1.n) > (unsigned)nKey1
         || (p_சாவி_Info = pPKey2->p_சாவி_Info)->nAllField<=i
        ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          திரும்பு 0;                /* Corruption */
        }இல்லையெனில் எனில்( p_சாவி_Info->aColl[i] ){
          mem1.enc = p_சாவி_Info->enc;
          mem1.db = p_சாவி_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (எழுத்து*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_சாவி_Info->aColl[i], &pPKey2->errCode
          );
        }இல்லையெனில்{
          முழு nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          எனில்( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    இல்லையெனில் எனில்( pRhs->flags & MEM_Blob ){
      ப்ரதிஜ்ஞா( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      எனில்( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }இல்லையெனில்{
        முழு nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(unsigned)nKey1 );
        testcase( (d1+nStr+1)==(unsigned)nKey1 );
        எனில்( (d1+nStr) > (unsigned)nKey1 ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          திரும்பு 0;                /* Corruption */
        }இல்லையெனில் எனில்( pRhs->flags & MEM_Zero ){
          எனில்( !isAllZero((மாறிலி எழுத்து*)&aKey1[d1],nStr) ){
            rc = 1;
          }இல்லையெனில்{
            rc = nStr - pRhs->u.nZero;
          }
        }இல்லையெனில்{
          முழு nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          எனில்( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    இல்லையெனில்{
      serial_type = aKey1[idx1];
      எனில்( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        ப்ரதிஜ்ஞா( rc==0 );
      }இல்லையெனில்{
        rc = 1;
      }
    }

    எனில்( rc!=0 ){
      முழு sortFlags = pPKey2->p_சாவி_Info->aSortFlags[i];
      எனில்( sortFlags ){
        எனில்( (sortFlags & KEYINFO_ORDER_BIGNULL)==0
         || ((sortFlags & KEYINFO_ORDER_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      ப்ரதிஜ்ஞா( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, rc) );
      ப்ரதிஜ்ஞா( mem1.szMalloc==0 );  /* See comment below */
      திரும்பு rc;
    }

    i++;
    எனில்( i==pPKey2->nField ) நிறுத்து;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    எனில்( d1>(unsigned)nKey1 ) நிறுத்து;
    idx1 += sqlite3VarintLen(serial_type);
    எனில்( idx1>=(unsigned)szHdr1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      திரும்பு 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  ப்ரதிஜ்ஞா( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  ப்ரதிஜ்ஞா( CORRUPT_DB
       || vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_சாவி_Info->db->mallocFailed
  );
  pPKey2->eqSeen = 1;
  திரும்பு pPKey2->default_rc;
}
முழு sqlite3VdbeRecordCompare(
  முழு nKey1, மாறிலி வெறுமை *pKey1,   /* Left key */
  UnpackedRecord *pPKey2          /* Right key */
){
  திரும்பு sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 0);
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
நிலையான முழு vdbeRecordCompareInt(
  முழு nKey1, மாறிலி வெறுமை *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  மாறிலி u8 *a_சாவி = &((மாறிலி u8*)pKey1)[*(மாறிலி u8*)pKey1 & 0x3F];
  முழு serial_type = ((மாறிலி u8*)pKey1)[1];
  முழு res;
  i64 v;
  i64 lhs;

  vdbe_Assert_Field_மொத்தம்_Within_Limits(nKey1, pKey1, pPKey2->p_சாவி_Info);
  ப்ரதிஜ்ஞா( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  எனில்( (u32)(serial_type-1)<=5 ){
    நிலையான மாறிலி u8 aShift[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_சாவி)) >> aShift[serial_type];
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
    ப்ரதிஜ்ஞா( 0<=(i64)sqlite3Get8byte(a_சாவி) || lhs<0 );
    testcase( lhs<0 );
  }இல்லையெனில் எனில்( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }இல்லையெனில்{
    திரும்பு sqlite3VdbeRecordCompare(nKey1, pKey1, pPKey2);
  }

  ப்ரதிஜ்ஞா( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  எனில்( v>lhs ){
    res = pPKey2->r1;
  }இல்லையெனில் எனில்( v<lhs ){
    res = pPKey2->r2;
  }இல்லையெனில் எனில்( pPKey2->nField>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
  }இல்லையெனில்{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eqSeen = 1;
  }

  ப்ரதிஜ்ஞா( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res) );
  திரும்பு res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
நிலையான முழு vdbeRecordCompareString(
  முழு nKey1, மாறிலி வெறுமை *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  மாறிலி u8 *aKey1 = (மாறிலி u8*)pKey1;
  முழு serial_type;
  முழு res;

  ப்ரதிஜ்ஞா( pPKey2->aMem[0].flags & MEM_Str );
  ப்ரதிஜ்ஞா( pPKey2->aMem[0].n == pPKey2->n );
  ப்ரதிஜ்ஞா( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_Field_மொத்தம்_Within_Limits(nKey1, pKey1, pPKey2->p_சாவி_Info);
  serial_type = (signed எழுத்து)(aKey1[1]);

vrcs_restart:
  எனில்( serial_type<12 ){
    எனில்( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      எனில்( serial_type>=12 ) செல் vrcs_restart;
      ப்ரதிஜ்ஞா( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }இல்லையெனில் எனில்( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }இல்லையெனில்{
    முழு nCmp;
    முழு nStr;
    முழு szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    எனில்( (szHdr + nStr) > nKey1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      திரும்பு 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    எனில்( res>0 ){
      res = pPKey2->r2;
    }இல்லையெனில் எனில்( res<0 ){
      res = pPKey2->r1;
    }இல்லையெனில்{
      res = nStr - pPKey2->n;
      எனில்( res==0 ){
        எனில்( pPKey2->nField>1 ){
          res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
        }இல்லையெனில்{
          res = pPKey2->default_rc;
          pPKey2->eqSeen = 1;
        }
      }இல்லையெனில் எனில்( res>0 ){
        res = pPKey2->r2;
      }இல்லையெனில்{
        res = pPKey2->r1;
      }
    }
  }

  ப்ரதிஜ்ஞா( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_சாவி_Info->db->mallocFailed
  );
  திரும்பு res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
RecordCompare sqlite3VdbeFindCompare(UnpackedRecord *p){
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
  ப்ரதிஜ்ஞா( p->p_சாவி_Info->aSortFlags!=0 );
  எனில்( p->p_சாவி_Info->nAllField<=13 ){
    முழு flags = p->aMem[0].flags;
    எனில்( p->p_சாவி_Info->aSortFlags[0] ){
      எனில்( p->p_சாவி_Info->aSortFlags[0] & KEYINFO_ORDER_BIGNULL ){
        திரும்பு sqlite3VdbeRecordCompare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }இல்லையெனில்{
      p->r1 = -1;
      p->r2 = 1;
    }
    எனில்( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      திரும்பு vdbeRecordCompareInt;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    எனில்( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_இருமம்(p->p_சாவி_Info->aColl[0])
    ){
      ப்ரதிஜ்ஞா( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      திரும்பு vdbeRecordCompareString;
    }
  }

  திரும்பு sqlite3VdbeRecordCompare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
முழு sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_சாவி = 0;
  முழு rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  ப்ரதிஜ்ஞா( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_சாவி = sqlite3BtreePayloadSize(pCur);
  ப்ரதிஜ்ஞா( (n_Cell_சாவி & SQLITE_MAX_U32)==(u64)n_Cell_சாவி );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)n_Cell_சாவி, &m);
  எனில்( rc ){
    திரும்பு rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  ப்ரதிஜ்ஞா( m.n>=0 );
  எனில்( unlikely(szHdr<3 || szHdr>(unsigned)m.n) ){
    செல் idx_rowid_corruption;
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
  எனில்( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    செல் idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  எனில்( unlikely((u32)m.n<szHdr+lenRowid) ){
    செல் idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  திரும்பு SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  திரும்பு SQLITE_CORRUPT_BKPT;
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
முழு sqlite3_Vdbe_Idx_சாவி_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  UnpackedRecord *pUnpacked,       /* Unpacked version of key */
  முழு *res                         /* Write the comparison result here */
){
  i64 n_Cell_சாவி = 0;
  முழு rc;
  BtCursor *pCur;
  Mem m;

  ப்ரதிஜ்ஞா( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  ப்ரதிஜ்ஞா( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_சாவி = sqlite3BtreePayloadSize(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  எனில்( n_Cell_சாவி<=0 || n_Cell_சாவி>0x7fffffff ){
    *res = 0;
    திரும்பு SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)n_Cell_சாவி, &m);
  எனில்( rc ){
    திரும்பு rc;
  }
  *res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  திரும்பு SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
வெறுமை sqlite3VdbeSetChanges(sqlite3 *db, i64 nChange){
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->nTotalChange += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
வெறுமை sqlite3_Vdbe_மொத்தம்_Changes(Vdbe *v){
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
வெறுமை sqlite3ExpirePreparedStatements(sqlite3 *db, முழு iCode){
  Vdbe *p;
  க்கு(p = db->pVdbe; p; p=p->pVNext){
    p->expired = iCode+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  திரும்பு v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  திரும்பு v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_மதிப்பு *sqlite3_Vdbe_Get_Bound_மதிப்பு(Vdbe *v, முழு iVar, u8 aff){
  ப்ரதிஜ்ஞா( iVar>0 );
  எனில்( v ){
    Mem *pMem = &v->aVar[iVar-1];
    ப்ரதிஜ்ஞா( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );
    எனில்( 0==(pMem->flags & MEM_Null) ){
      sqlite3_மதிப்பு *pRet = sqlite3_மதிப்பு_New(v->db);
      எனில்( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_மதிப்பு_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      திரும்பு pRet;
    }
  }
  திரும்பு 0;
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
வெறுமை sqlite3VdbeReprepareOnBind(Vdbe *v, முழு iVar, முழு bSmallint){
  u32 m;
  ப்ரதிஜ்ஞா( iVar>0 );
  ப்ரதிஜ்ஞா( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  எனில்( bSmallint ){
    எனில்( (v->expmask & m)==0 ) v->smimask |= m;
  }இல்லையெனில்{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  ப்ரதிஜ்ஞா( (v->smimask & v->expmask) == v->smimask );
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
நிலையான முழு vdbeSkipField(
  Bitmask mask,                   /* Mask of indexed expression fields */
  முழு iCol,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  முழு bIntegrity                  /* True if running PRAGMA integrity_check */
){
#define BTREE_ULPDISTORTION 2
  எனில்( iCol>=BMS || (mask & MASKBIT(iCol))==0 ) திரும்பு 0;
  எனில்( bIntegrity==0 ) திரும்பு 1;
  எனில்( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    memcpy(&m1,&pMem1->u.r,8);
    memcpy(&m2,&pMem2->u.r,8);
    எனில்( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      திரும்பு 1;
    }
  }
  திரும்பு 0;
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
நிலையான முழு vdbe_Is_Matching_Index_சாவி(
  BtCursor *pCur,            /* Cursor open on index */
  முழு bInt,                  /* True for integrity_check-style search */
  Bitmask mask,              /* Mask of columns to skip */
  UnpackedRecord *p,         /* Index key being deleted */
  முழு *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  முழு rc = SQLITE_OK;

  memset(&m, 0, அளவு(m));
  m.enc = p->p_சாவி_Info->enc;
  m.db = p->p_சாவி_Info->db;
  nRec = sqlite3BtreePayloadSize(pCur);
  எனில்( nRec>0x7fffffff ){
    திரும்பு SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  எனில்( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }இல்லையெனில்{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  எனில்( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    எனில்( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }இல்லையெனில்{
      முழு res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      முழு ii = 0;                 /* Iterator variable */

      முழு nCol = p->p_சாவி_Info->nAllField;
      க்கு(ii=0; ii<nCol && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        முழு nSerial = 0;

        எனில்( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          நிறுத்து;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        எனில்( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }இல்லையெனில்{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          எனில்( vdbeSkipField(mask, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_சாவி_Info->aColl[ii]);
            எனில்( res!=0 ) நிறுத்து;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  திரும்பு rc;
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
முழு sqlite3_Vdbe_Find_Index_சாவி(
  BtCursor *pCur, 
  Index *pIdx,
  UnpackedRecord *p, 
  முழு *pRes,
  முழு bIntegrity
){
#define BTREE_FDK_RANGE 10
  முழு nStep = 0;
  முழு res = 1;
  முழு rc = SQLITE_OK;
  முழு ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask mask = 0;
  க்கு(ii=0; ii<MIN(pIdx->nColumn, BMS); ii++){
    முழு iCol = pIdx->aiColumn[ii];
    எனில்( (iCol==XN_கோவை)
     || (iCol>=0 && (pIdx->pTable->aCol[iCol].colFlags & COLFLAG_VIRTUAL))
    ){
      mask |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  எனில்( mask!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    க்கு(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3BtreePrevious(pCur, 0);
    }
    எனில்( rc==SQLITE_DONE ){
      rc = sqlite3BtreeFirst(pCur, &res);
      nStep = -1;
    }இல்லையெனில்{
      nStep = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    போது( sqlite3BtreeCursorIsValidNN(pCur) ){
      க்கு(ii=0; rc==SQLITE_OK && (ii<nStep || nStep<0); ii++){
        rc = vdbe_Is_Matching_Index_சாவி(pCur, bIntegrity, mask, p, &res);
        எனில்( res==0 || rc!=SQLITE_OK ) நிறுத்து;
        rc = sqlite3BtreeNext(pCur, 0);
      }
      எனில்( rc==SQLITE_DONE ){
        rc = SQLITE_OK;
        ப்ரதிஜ்ஞா( res!=0 );
      }
      எனில்( nStep<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) நிறுத்து;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      nStep = -1;
      rc = sqlite3BtreeFirst(pCur, &res);
    }
  }

  *pRes = res;
  திரும்பு rc;
}

#ifndef SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
முழு sqlite3NotPureFunc(sqlite3_context *pCtx){
  மாறிலி VdbeOp *pOp;
#ifdef SQLITE_ENABLE_STAT4
  எனில்( pCtx->pVdbe==0 ) திரும்பு 1;
#endif
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  எனில்( pOp->opcode==OP_PureFunc ){
    மாறிலி எழுத்து *zContext;
    எழுத்து *z_செய்தி;
    எனில்( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }இல்லையெனில் எனில்( pOp->p5 & NC_GenCol ){
      zContext = "a generated column";
    }இல்லையெனில்{
      zContext = "an index";
    }
    z_செய்தி = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_result_error(pCtx, z_செய்தி, -1);
    sqlite3_free(z_செய்தி);
    திரும்பு 0;
  }
  திரும்பு 1;
}
#endif /* SQLITE_OMIT_DATETIME_FUNCS */

#எனில் defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
முழு sqlite3_Cursor_Range_Hint_கோவை_Check(Walker *pWalker, கோவை *p_கோவை){
  எனில்( p_கோவை->op==TK_REGISTER ){
    ப்ரதிஜ்ஞா( (pWalker->u.aMem[p_கோவை->iTable].flags & MEM_Undefined)==0 );
  }
  திரும்பு WRC_Continue;
}
#endif /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
வெறுமை sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  எனில்( pVtab->z_Err_செய்தி ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_செய்தி);
    p->z_Err_செய்தி = sqlite3DbStrDup(db, pVtab->z_Err_செய்தி);
    sqlite3_free(pVtab->z_Err_செய்தி);
    pVtab->z_Err_செய்தி = 0;
  }
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
நிலையான வெறுமை vdbeFreeUnpacked(sqlite3 *db, முழு nField, UnpackedRecord *p){
  ப்ரதிஜ்ஞா( db!=0 );
  எனில்( p ){
    முழு i;
    க்கு(i=0; i<nField; i++){
      Mem *pMem = &p->aMem[i];
      எனில்( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
வெறுமை sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  முழு op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  மாறிலி எழுத்து *zDb,                /* Database name */
  Table *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  முழு iReg,                       /* Register for new.* record */
  முழு iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  மாறிலி எழுத்து *zTbl = pTab->zName;
#ifdef SQLITE_DEBUG
  முழு nRealCol;
  எனில்( pTab->tabFlags & TF_WithoutRowid ){
    nRealCol = sqlite3_Primary_சாவி_Index(pTab)->nColumn;
  }இல்லையெனில் எனில்( pTab->tabFlags & TF_HasVirtual ){
    nRealCol = pTab->nNVCol;
  }இல்லையெனில்{
    nRealCol = pTab->nCol;
  }
#endif

  ப்ரதிஜ்ஞா( db->pPreUpdate==0 );
  memset(&preupdate, 0, அளவு(PreUpdate));
  எனில்( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_சாவி_Index(pTab);
  }இல்லையெனில்{
    எனில்( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }இல்லையெனில்{
      iKey2 = iKey1;
    }
  }

  ப்ரதிஜ்ஞா( pCsr!=0 );
  ப்ரதிஜ்ஞா( pCsr->eCurType==CURTYPE_BTREE );
  ப்ரதிஜ்ஞா( pCsr->nField==nRealCol
       || (pCsr->nField==nRealCol+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (சாவி_Info*)&preupdate.u_சாவி;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_சாவி_Field = pTab->nCol;
  preupdate.pKeyinfo->aSortFlags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.aRecord);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_சாவி_Field+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_சாவி_Field+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  எனில்( preupdate.aNew ){
    முழு i;
    க்கு(i=0; i<pCsr->nField; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  எனில்( preupdate.apDflt ){
    முழு i;
    க்கு(i=0; i<pTab->nCol; i++){
      sqlite3_மதிப்பு_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
மாறிலி எழுத்து *sqlite3VdbeFuncName(மாறிலி sqlite3_context *pCtx){
  ப்ரதிஜ்ஞா( pCtx!=0 );
  ப்ரதிஜ்ஞா( pCtx->pFunc!=0 );
  திரும்பு pCtx->pFunc->zName;
}
#endif /* SQLITE_ENABLE_PERCENTILE */
