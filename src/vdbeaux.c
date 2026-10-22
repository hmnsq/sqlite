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
#include "sqliteInt.h"
#include "vdbeInt.h"

/* Forward references */
static void freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
static void vdbeFreeOpArray(sqlite3 *, Op *, 정수);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, sizeof(Vdbe) );
  만약( p==0 ) 반환 0;
  memset(&p->aOp, 0, sizeof(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  만약( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  assert( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  assert( pParse->aLabel==0 );
  assert( pParse->nLabel==0 );
  assert( p->nOpAlloc==0 );
  assert( pParse->szOpAlloc==0 );
  sqlite3VdbeAddOp2(p, OP_Init, 0, 1);
  반환 p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  반환 p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
void sqlite3VdbeError(Vdbe *p, const char *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->zErrMsg);
  va_start(ap, zFormat);
  p->zErrMsg = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
void sqlite3VdbeSetSql(Vdbe *p, const char *z, 정수 n, u8 prepFlags){
  만약( p==0 ) 반환;
  p->prepFlags = prepFlags;
  만약( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  assert( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
void sqlite3VdbeAddDblquoteStr(sqlite3 *db, Vdbe *p, const char *z){
  만약( p ){
    정수 n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            sizeof(*pStr)+n+1-sizeof(pStr->z));
    만약( pStr ){
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
정수 sqlite3VdbeUsesDoubleQuotedString(
  Vdbe *pVdbe,            /* The prepared statement */
  const char *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  assert( zId!=0 );
  만약( pVdbe->pDblStr==0 ) 반환 0;
  위해(pStr=pVdbe->pDblStr; pStr; pStr=pStr->pNextStr){
    만약( strcmp(zId, pStr->z)==0 ) 반환 1;
  }
  반환 0;
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
void sqlite3VdbeSwap(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  char *zTmp;
  assert( pA->db==pB->db );
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
  memcpy(pB->aCounter, pA->aCounter, sizeof(pB->aCounter));
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
static 정수 growOpArray(Vdbe *v, 정수 nOp){
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
#아니면
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/sizeof(Op)));
  UNUSED_PARAMETER(nOp);
#endif

  /* Ensure that the size of a VDBE does not grow too large */
  만약( nNew > p->db->aLimit[SQLITE_LIMIT_VDBE_OP] ){
    sqlite3OomFault(p->db);
    반환 SQLITE_NOMEM;
  }

  assert( nOp<=(정수)(1024/sizeof(Op)) );
  assert( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*sizeof(Op));
  만약( pNew ){
    p->szOpAlloc = sqlite3DbMallocSize(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/sizeof(Op);
    v->aOp = pNew;
  }
  반환 (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
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
static void test_addop_breakpoint(정수 pc, Op *pOp){
  static u64 n = 0;
  (void)pc;
  (void)pOp;
  n++;
  만약( n==LARGEST_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#endif

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
static SQLITE_NOINLINE 정수 growOp3(Vdbe *p, 정수 op, 정수 p1, 정수 p2, 정수 p3){
  assert( p->nOpAlloc<=p->nOp );
  만약( growOpArray(p, 1) ) 반환 1;
  assert( p->nOpAlloc>p->nOp );
  반환 sqlite3VdbeAddOp3(p, op, p1, p2, p3);
}
static SQLITE_NOINLINE 정수 addOp4IntSlow(
  Vdbe *p,            /* Add the opcode to this VM */
  정수 op,             /* The new opcode */
  정수 p1,             /* The P1 operand */
  정수 p2,             /* The P2 operand */
  정수 p3,             /* The P3 operand */
  정수 p4              /* The P4 operand as an integer */
){
  정수 addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  만약( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  반환 addr;
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
정수 sqlite3VdbeAddOp0(Vdbe *p, 정수 op){
  반환 sqlite3VdbeAddOp3(p, op, 0, 0, 0);
}
정수 sqlite3VdbeAddOp1(Vdbe *p, 정수 op, 정수 p1){
  반환 sqlite3VdbeAddOp3(p, op, p1, 0, 0);
}
정수 sqlite3VdbeAddOp2(Vdbe *p, 정수 op, 정수 p1, 정수 p2){
  반환 sqlite3VdbeAddOp3(p, op, p1, p2, 0);
}
정수 sqlite3VdbeAddOp3(Vdbe *p, 정수 op, 정수 p1, 정수 p2, 정수 p3){
  정수 i;
  VdbeOp *pOp;

  i = p->nOp;
  assert( p->eVdbeState==VDBE_INIT_STATE );
  assert( op>=0 && op<0xff );
  만약( p->nOpAlloc<=i ){
    반환 growOp3(p, op, p1, p2, p3);
  }
  assert( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  assert( pOp!=0 );
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
#만약 defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#endif
#ifdef SQLITE_DEBUG
  만약( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#endif
#ifdef SQLITE_VDBE_COVERAGE
  pOp->iSrcLine = 0;
#endif
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  반환 i;
}
정수 sqlite3VdbeAddOp4Int(
  Vdbe *p,            /* Add the opcode to this VM */
  정수 op,             /* The new opcode */
  정수 p1,             /* The P1 operand */
  정수 p2,             /* The P2 operand */
  정수 p3,             /* The P3 operand */
  정수 p4              /* The P4 operand as an integer */
){
  정수 i;
  VdbeOp *pOp;

  i = p->nOp;
  만약( p->nOpAlloc<=i ){
    반환 addOp4IntSlow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  assert( pOp!=0 );
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
#만약 defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#endif
#ifdef SQLITE_DEBUG
  만약( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#endif
#ifdef SQLITE_VDBE_COVERAGE
  pOp->iSrcLine = 0;
#endif
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  반환 i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
정수 sqlite3VdbeAddInt64(Vdbe *p, 정수 iDest, i64 iVal){
  반환 sqlite3VdbeAddOp3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
정수 sqlite3VdbeAddDouble(Vdbe *p, 정수 iDest, double rVal){
  i64 iVal;
  memcpy(&iVal,&rVal,8);
  반환 sqlite3VdbeAddOp3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
정수 sqlite3VdbeGoto(Vdbe *p, 정수 iDest){
  반환 sqlite3VdbeAddOp3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
정수 sqlite3VdbeLoadString(Vdbe *p, 정수 iDest, const char *zStr){
  반환 sqlite3VdbeAddOp4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
void sqlite3VdbeMultiLoad(Vdbe *p, 정수 iDest, const char *zTypes, ...){
  va_list ap;
  정수 i;
  char c;
  va_start(ap, zTypes);
  위해(i=0; (c = zTypes[i])!=0; i++){
    만약( c=='s' ){
      const char *z = va_arg(ap, const char*);
      sqlite3VdbeAddOp4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }아니면 만약( c=='i' ){
      sqlite3VdbeAddOp2(p, OP_Integer, va_arg(ap, 정수), iDest+i);
    }아니면{
      goto skip_op_resultrow;
    }
  }
  sqlite3VdbeAddOp2(p, OP_ResultRow, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
정수 sqlite3VdbeAddOp4(
  Vdbe *p,            /* Add the opcode to this VM */
  정수 op,             /* The new opcode */
  정수 p1,             /* The P1 operand */
  정수 p2,             /* The P2 operand */
  정수 p3,             /* The P3 operand */
  const char *zP4,    /* The P4 operand */
  정수 p4type          /* P4 operand type */
){
  정수 addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  반환 addr;
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
정수 sqlite3VdbeAddFunctionCall(
  Parse *pParse,        /* Parsing context */
  정수 p1,               /* Constant argument mask */
  정수 p2,               /* First argument register */
  정수 p3,               /* Register into which results are written */
  정수 nArg,             /* Number of argument */
  const FuncDef *pFunc, /* The function to be invoked */
  정수 eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  정수 addr;
  sqlite3_context *pCtx;
  assert( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  만약( pCtx==0 ){
    assert( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    반환 0;
  }
  pCtx->pOut = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3VdbeAddOp4(v, eCallCtx ? OP_PureFunc : OP_Function,
                           p1, p2, p3, (char*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  반환 addr;
}

#ifndef SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
정수 sqlite3VdbeExplainParent(Parse *pParse){
  VdbeOp *pOp;
  만약( pParse->addrExplain==0 ) 반환 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  반환 pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#만약 defined(SQLITE_DEBUG)
void sqlite3ExplainBreakpoint(const char *z1, const char *z2){
  (void)z1;
  (void)z2;
}
#endif

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
정수 sqlite3VdbeExplain(Parse *pParse, u8 bPush, const char *zFmt, ...){
  정수 addr = 0;
#만약 !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  만약( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    char *zMsg;
    Vdbe *v;
    va_list ap;
    정수 iThis;
    va_start(ap, zFmt);
    zMsg = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3VdbeAddOp4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      zMsg, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(bPush?"PUSH":"", sqlite3VdbeGetLastOp(v)->p4.z);
    만약( bPush){
      pParse->addrExplain = iThis;
    }
    sqlite3VdbeScanStatus(v, iThis, -1, -1, 0, 0);
  }
  반환 addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
void sqlite3VdbeExplainPop(Parse *pParse){
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
void sqlite3VdbeAddParseSchemaOp(Vdbe *p, 정수 iDb, char *zWhere, u16 p5){
  정수 j;
  assert( p5==0 || zWhere==0 );
  sqlite3VdbeAddOp4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  위해(j=0; j<p->db->nDb; j++) sqlite3VdbeUsesBtree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
void sqlite3VdbeEndCoroutine(Vdbe *v, 정수 regYield){
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
정수 sqlite3VdbeMakeLabel(Parse *pParse){
  반환 --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
static SQLITE_NOINLINE void resizeResolveLabel(Parse *p, Vdbe *v, 정수 j){
  정수 nNewSize = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     nNewSize*sizeof(p->aLabel[0]));
  만약( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }아니면{
#ifdef SQLITE_DEBUG
    정수 i;
    위해(i=p->nLabelAlloc; i<nNewSize; i++) p->aLabel[i] = -1;
#endif
    만약( nNewSize>=100 && (nNewSize/100)>(p->nLabelAlloc/100) ){
      sqlite3ProgressCheck(p);
    }
    p->nLabelAlloc = nNewSize;
    p->aLabel[j] = v->nOp;
  }
}
void sqlite3VdbeResolveLabel(Vdbe *v, 정수 x){
  Parse *p = v->pParse;
  정수 j = ADDR(x);
  assert( v->eVdbeState==VDBE_INIT_STATE );
  assert( j<-p->nLabel );
  assert( j>=0 );
#ifdef SQLITE_DEBUG
  만약( p->db->flags & SQLITE_VdbeAddopTrace ){
    printf("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#endif
  만약( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }아니면{
    assert( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
void sqlite3VdbeRunOnlyOnce(Vdbe *p){
  sqlite3VdbeAddOp2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
void sqlite3VdbeReusable(Vdbe *p){
  정수 i;
  위해(i=1; ALWAYS(i<p->nOp); i++){
    만약( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      break;
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
typedef struct VdbeOpIter VdbeOpIter;
struct VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  SubProgram **apSub;        /* Array of subprograms */
  정수 nSub;                  /* Number of entries in apSub */
  정수 iAddr;                 /* Address of next instruction to return */
  정수 iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
static Op *opIterNext(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  정수 nOp;

  만약( p->iSub<=p->nSub ){

    만약( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }아니면{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    assert( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    만약( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    만약( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*sizeof(SubProgram*);
      정수 j;
      위해(j=0; j<p->nSub; j++){
        만약( p->apSub[j]==pRet->p4.pProgram ) break;
      }
      만약( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        만약( !p->apSub ){
          pRet = 0;
        }아니면{
          p->apSub[p->nSub++] = pRet->p4.pProgram;
        }
      }
    }
  }

  반환 pRet;
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
정수 sqlite3VdbeAssertMayAbort(Vdbe *v, 정수 mayAbort){
  정수 hasAbort = 0;
  정수 hasFkCounter = 0;
  정수 hasCreateTable = 0;
  정수 hasCreateIndex = 0;
  정수 hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  만약( v==0 ) 반환 0;
  memset(&sIter, 0, sizeof(sIter));
  sIter.v = v;

  동안( (pOp = opIterNext(&sIter))!=0 ){
    정수 opcode = pOp->opcode;
    만약( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_PureFunc
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      break;
    }
    만약( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) hasCreateTable = 1;
    만약( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      만약( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) hasCreateIndex = 1;
      만약( opcode==OP_Clear ) hasCreateIndex = 1;
    }
    만약( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#ifndef SQLITE_OMIT_FOREIGN_KEY
    만약( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
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
  반환 ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (hasCreateTable && hasInitCoroutine) || hasCreateIndex
  );
}
#endif /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#ifdef SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
void sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  만약( pC==0
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
void sqlite3VdbeAssertAbortable(Vdbe *p){
  assert( p->nWrite==0 || p->usesStmtJournal );
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
static void resolveP2Values(Vdbe *p, 정수 *pMaxVtabArgs){
  정수 nMaxVtabArgs = *pMaxVtabArgs;
  Op *pOp;
  Parse *pParse = p->pParse;
  정수 *aLabel = pParse->aLabel;

  assert( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  assert( p->aOp[0].opcode==OP_Init );
  동안( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    만약( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      switch( pOp->opcode ){
        case OP_Transaction: {
          만약( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        case OP_AutoCommit:
        case OP_Savepoint: {
          p->bIsReader = 1;
          break;
        }
#ifndef SQLITE_OMIT_WAL
        case OP_Checkpoint:
#endif
        case OP_Vacuum:
        case OP_JournalMode: {
          p->readOnly = 0;
          p->bIsReader = 1;
          break;
        }
        case OP_Init: {
          assert( pOp->p2>=0 );
          goto resolve_p2_values_loop_exit;
        }
#ifndef SQLITE_OMIT_VIRTUALTABLE
        case OP_VUpdate: {
          만약( pOp->p2>nMaxVtabArgs ) nMaxVtabArgs = pOp->p2;
          break;
        }
        case OP_VFilter: {
          정수 n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          assert( (pOp - p->aOp) >= 3 );
          assert( pOp[-1].opcode==OP_Integer );
          assert( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          만약( n>nMaxVtabArgs ) nMaxVtabArgs = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#endif
        default: {
          만약( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            assert( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            assert( ADDR(pOp->p2)<-pParse->nLabel );
            assert( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          assert( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          assert( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          break;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      assert( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    assert( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_values_loop_exit:
  만약( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *pMaxVtabArgs = nMaxVtabArgs;
  assert( p->bIsReader!=0 || DbMaskAllZero(p->btreeMask) );
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
void sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  정수 iFirst,       /* First opcode of the subroutine */
  정수 iLast,        /* Last opcode of the subroutine */
  정수 iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  정수 i;
  sqlite3_str *pErr = 0;
  assert( v!=0 );
  pParse = v->pParse;
  assert( pParse!=0 );
  만약( pParse->nErr ) 반환;
  assert( iLast>=iFirst );
  assert( iLast<v->nOp );
  pOp = &v->aOp[iFirst];
  위해(i=iFirst; i<=iLast; i++, pOp++){
    만약( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      정수 iDest = pOp->p2;   /* Jump destination */
      만약( iDest==0 ) continue;
      만약( pOp->opcode==OP_Gosub ) continue;
      만약( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        continue;
      }
      만약( iDest<0 ){
        정수 j = ADDR(iDest);
        assert( j>=0 );
        만약( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          continue;
        }
        iDest = pParse->aLabel[j];
      }
      만약( iDest<iFirst || iDest>iLast ){
        정수 j = iDest;
        위해(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          만약( pX->opcode==OP_Return ){
            만약( pX->p1==iRetReg ) break;
            continue;
          }
          만약( pX->opcode==OP_Noop ) continue;
          만약( pX->opcode==OP_Explain ) continue;
          만약( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }아니면{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, iFirst, iLast);
          break;
        }
      }
    }
  }
  만약( pErr ){
    char *zErr = sqlite3_str_finish(pErr);
    sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_INTERNAL, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#endif /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
정수 sqlite3VdbeCurrentAddr(Vdbe *p){
  assert( p->eVdbeState==VDBE_INIT_STATE );
  반환 p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#만약 defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
void sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, 정수 N){
  assert( p->nOp + N <= p->nOpAlloc );
}
#endif

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#만약 defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
void sqlite3VdbeVerifyNoResultRow(Vdbe *p){
  정수 i;
  위해(i=0; i<p->nOp; i++){
    assert( p->aOp[i].opcode!=OP_ResultRow );
  }
}
#endif

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#만약 defined(SQLITE_DEBUG)
void sqlite3VdbeVerifyAbortable(Vdbe *p, 정수 onError){
  만약( onError==OE_Abort ) sqlite3VdbeAddOp0(p, OP_Abortable);
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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, 정수 *pnOp, 정수 *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  assert( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  assert( DbMaskAllZero(p->btreeMask) );

  resolveP2Values(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  반환 aOp;
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
  정수 nOp,                     /* Number of opcodes to add */
  VdbeOpList const *aOp,       /* The opcodes to be added */
  정수 iLineno                  /* Source-file line number of first opcode */
){
  정수 i;
  VdbeOp *pOut, *pFirst;
  assert( nOp>0 );
  assert( p->eVdbeState==VDBE_INIT_STATE );
  만약( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    반환 0;
  }
  pFirst = pOut = &p->aOp[p->nOp];
  위해(i=0; i<nOp; i++, aOp++, pOut++){
    pOut->opcode = aOp->opcode;
    pOut->p1 = aOp->p1;
    pOut->p2 = aOp->p2;
    assert( aOp->p2>=0 );
    만약( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
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
    pOut->iSrcLine = iLineno+i;
#아니면
    (void)iLineno;
#endif
#ifdef SQLITE_DEBUG
    만약( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#endif
  }
  p->nOp += nOp;
  반환 pFirst;
}

#만약 defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
void sqlite3VdbeScanStatus(
  Vdbe *p,                        /* VM to add scanstatus() to */
  정수 addrExplain,                /* Address of OP_Explain (or 0) */
  정수 addrLoop,                   /* Address of loop counter */
  정수 addrVisit,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  const char *zName               /* Name of table or index being scanned */
){
  만약( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->nScan) * sizeof(ScanStatus);
    ScanStatus *aNew;
    aNew = (ScanStatus*)sqlite3DbRealloc(p->db, p->aScan, nByte);
    만약( aNew ){
      ScanStatus *pNew = &aNew[p->nScan++];
      memset(pNew, 0, sizeof(ScanStatus));
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
void sqlite3VdbeScanStatusRange(
  Vdbe *p,
  정수 addrExplain,
  정수 addrStart,
  정수 addrEnd
){
  만약( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    정수 ii;
    위해(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      만약( pScan->addrExplain==addrExplain ) break;
      pScan = 0;
    }
    만약( pScan ){
      만약( addrEnd<0 ) addrEnd = sqlite3VdbeCurrentAddr(p)-1;
      위해(ii=0; ii<ArraySize(pScan->aAddrRange); ii+=2){
        만약( pScan->aAddrRange[ii]==0 ){
          pScan->aAddrRange[ii] = addrStart;
          pScan->aAddrRange[ii+1] = addrEnd;
          break;
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
void sqlite3VdbeScanStatusCounters(
  Vdbe *p,
  정수 addrExplain,
  정수 addrLoop,
  정수 addrVisit
){
  만약( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    정수 ii;
    위해(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      만약( pScan->addrExplain==addrExplain ) break;
      pScan = 0;
    }
    만약( pScan ){
      만약( addrLoop>0 ) pScan->addrLoop = addrLoop;
      만약( addrVisit>0 ) pScan->addrVisit = addrVisit;
    }
  }
}
#endif /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
void sqlite3VdbeChangeOpcode(Vdbe *p, 정수 addr, u8 iNewOpcode){
  assert( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
void sqlite3VdbeChangeP1(Vdbe *p, 정수 addr, 정수 val){
  assert( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
void sqlite3VdbeChangeP2(Vdbe *p, 정수 addr, 정수 val){
  assert( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
void sqlite3VdbeChangeP3(Vdbe *p, 정수 addr, 정수 val){
  assert( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
void sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  assert( p->nOp>0 || p->db->mallocFailed );
  만약( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
void sqlite3VdbeTypeofColumn(Vdbe *p, 정수 iDest){
  VdbeOp *pOp = sqlite3VdbeGetLastOp(p);
#ifdef SQLITE_DEBUG
  동안( pOp->opcode==OP_ReleaseReg ) pOp--;
#endif
  만약( pOp->p3==iDest && pOp->opcode==OP_Column ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
void sqlite3VdbeJumpHere(Vdbe *p, 정수 addr){
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
void sqlite3VdbeJumpHereOrPopInst(Vdbe *p, 정수 addr){
  만약( addr==p->nOp-1 ){
    assert( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    assert( p->aOp[addr].p4type==0 );
#ifdef SQLITE_VDBE_COVERAGE
    sqlite3VdbeGetLastOp(p)->iSrcLine = 0;  /* Erase VdbeCoverage() macros */
#endif
    p->nOp--;
  }아니면{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
static void freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  assert( db!=0 );
  만약( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
static SQLITE_NOINLINE void freeP4Mem(sqlite3 *db, Mem *p){
  만약( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
static SQLITE_NOINLINE void freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  assert( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
static void freeP4(sqlite3 *db, 정수 p4type, void *p4){
  assert( db );
  switch( p4type ){
    case P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      break;
    }
    case P4_DYNAMIC:
    case P4_INTARRAY: {
      만약( p4 ) sqlite3DbNNFreeNN(db, p4);
      break;
    }
    case P4_KEYINFO: {
      만약( db->pnBytesFreed==0 ) sqlite3KeyInfoUnref((KeyInfo*)p4);
      break;
    }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
    case P4_EXPR: {
      sqlite3ExprDelete(db, (Expr*)p4);
      break;
    }
#endif
    case P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      break;
    }
    case P4_MEM: {
      만약( db->pnBytesFreed==0 ){
        sqlite3ValueFree((sqlite3_value*)p4);
      }아니면{
        freeP4Mem(db, (Mem*)p4);
      }
      break;
    }
    case P4_VTAB : {
      만약( db->pnBytesFreed==0 ) sqlite3VtabUnlock((VTable *)p4);
      break;
    }
    case P4_TABLEREF: {
      만약( db->pnBytesFreed==0 ) sqlite3DeleteTable(db, (Table*)p4);
      break;
    }
    case P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      break;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
static void vdbeFreeOpArray(sqlite3 *db, Op *aOp, 정수 nOp){
  assert( nOp>=0 );
  assert( db!=0 );
  만약( aOp ){
    Op *pOp = &aOp[nOp-1];
    동안(1){  /* Exit via break */
      만약( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#endif    
      만약( pOp==aOp ) break;
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
void sqlite3VdbeLinkSubProgram(Vdbe *pVdbe, SubProgram *p){
  p->pNext = pVdbe->pProgram;
  pVdbe->pProgram = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
정수 sqlite3VdbeHasSubProgram(Vdbe *pVdbe){
  반환 pVdbe->pProgram!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
정수 sqlite3VdbeChangeToNoop(Vdbe *p, 정수 addr){
  VdbeOp *pOp;
  만약( p->db->mallocFailed ) 반환 0;
  assert( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  반환 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
정수 sqlite3VdbeDeletePriorOpcode(Vdbe *p, u8 op){
  만약( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    반환 sqlite3VdbeChangeToNoop(p, p->nOp-1);
  }아니면{
    반환 0;
  }
}

#ifdef SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
void sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  정수 iFirst,          /* Index of first register to be released */
  정수 N,               /* Number of registers to release */
  u32 mask,            /* Mask of registers to NOT release */
  정수 bUndefine        /* If true, mark registers as undefined */
){
  만약( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) 반환;
  assert( pParse->pVdbe );
  assert( iFirst>=1 );
  assert( iFirst+N-1<=pParse->nMem );
  만약( N<=31 && mask!=0 ){
    동안( N>0 && (mask&1)!=0 ){
      mask >>= 1;
      iFirst++;
      N--;
    }
    동안( N>0 && N<=32 && (mask & MASKBIT32(N-1))!=0 ){
      mask &= ~MASKBIT32(N-1);
      N--;
    }
  }
  만약( N>0 ){
    sqlite3VdbeAddOp3(pParse->pVdbe, OP_ReleaseReg, iFirst, N, *(정수*)&mask);
    만약( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
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
static void SQLITE_NOINLINE vdbeChangeP4Full(
  Vdbe *p,
  Op *pOp,
  const char *zP4,
  정수 n
){
  만약( pOp->p4type ){
    assert( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  만약( n<0 ){
    sqlite3VdbeChangeP4(p, (정수)(pOp - p->aOp), zP4, n);
  }아니면{
    만약( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
void sqlite3VdbeChangeP4(Vdbe *p, 정수 addr, const char *zP4, 정수 n){
  Op *pOp;
  sqlite3 *db;
  assert( p!=0 );
  db = p->db;
  assert( p->eVdbeState==VDBE_INIT_STATE );
  assert( p->aOp!=0 || db->mallocFailed );
  만약( db->mallocFailed ){
    만약( n!=P4_VTAB ) freeP4(db, n, (void*)*(char**)&zP4);
    반환;
  }
  assert( p->nOp>0 );
  assert( addr<p->nOp );
  만약( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  만약( n>=0 || pOp->p4type ){
    vdbeChangeP4Full(p, pOp, zP4, n);
    반환;
  }
  만약( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_TO_INT(zP4);
    pOp->p4type = P4_INT32;
  }아니면 만약( zP4!=0 ){
    assert( n<0 );
    pOp->p4.p = (void*)zP4;
    pOp->p4type = (signed char)n;
    만약( n==P4_VTAB ) sqlite3VtabLock((VTable*)zP4);
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
void sqlite3VdbeAppendP4(Vdbe *p, void *pP4, 정수 n){
  VdbeOp *pOp;
  assert( n!=P4_INT32 && n!=P4_VTAB );
  assert( n<=0 );
  만약( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }아니면{
    assert( pP4!=0 || n==P4_DYNAMIC );
    assert( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    assert( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
void sqlite3VdbeSetP4KeyInfo(Parse *pParse, Index *pIdx){
  Vdbe *v = pParse->pVdbe;
  KeyInfo *pKeyInfo;
  assert( v!=0 );
  assert( pIdx!=0 );
  pKeyInfo = sqlite3KeyInfoOfIndex(pParse, pIdx);
  만약( pKeyInfo ) sqlite3VdbeAppendP4(v, pKeyInfo, P4_KEYINFO);
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
static void vdbeVComment(Vdbe *p, const char *zFormat, va_list ap){
  assert( p->nOp>0 || p->aOp==0 );
  assert( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  만약( p->nOp ){
    assert( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
void sqlite3VdbeComment(Vdbe *p, const char *zFormat, ...){
  va_list ap;
  만약( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
void sqlite3VdbeNoopComment(Vdbe *p, const char *zFormat, ...){
  va_list ap;
  만약( p ){
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
void sqlite3VdbeSetLineNumber(Vdbe *v, 정수 iLine){
  sqlite3VdbeGetLastOp(v)->iSrcLine = iLine;
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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, 정수 addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  static VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  assert( p->eVdbeState==VDBE_INIT_STATE );
  assert( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  만약( p->db->mallocFailed ){
    반환 (VdbeOp*)&dummy;
  }아니면{
    반환 &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3VdbeGetLastOp(Vdbe *p){
  반환 sqlite3VdbeGetOp(p, p->nOp - 1);
}

#만약 defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
static 정수 translateP(char c, const Op *pOp){
  만약( c=='1' ) 반환 pOp->p1;
  만약( c=='2' ) 반환 pOp->p2;
  만약( c=='3' ) 반환 pOp->p3;
  만약( c=='4' ) 반환 pOp->p4.i;
  반환 pOp->p5;
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
char *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  const Op *pOp,     /* The opcode to be commented */
  const char *zP4    /* Previously obtained value for P4 */
){
  const char *zOpName;
  const char *zSynopsis;
  정수 nOpName;
  정수 ii;
  char zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_LENGTH);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  만약( zOpName[nOpName+1] ){
    정수 seenCom = 0;
    char c;
    zSynopsis = zOpName + nOpName + 1;
    만약( strncmp(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(sizeof(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    위해(ii=0; (c = zSynopsis[ii])!=0; ii++){
      만약( c=='P' ){
        c = zSynopsis[++ii];
        만약( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }아니면 만약( c=='X' ){
          만약( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            seenCom = 1;
            break;
          }
        }아니면 만약( strncmp(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_TO_64(pOp->p1,pOp->p3));
          ii += 4;
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
        }아니면 만약( strncmp(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_TO_64(pOp->p2,pOp->p3));
          ii += 4;
#endif
        }아니면 만약( strncmp(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_TO_64(pOp->p1,pOp->p3);
          double r;
          memcpy(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }아니면{
          정수 v1 = translateP(c, pOp);
          정수 v2;
          만약( strncmp(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            만약( strncmp(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            만약( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }아니면{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }아니면 만약( strncmp(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            만약( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }아니면 만약( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }아니면 만약( x.accError==0 ){
              assert( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }아니면{
            sqlite3_str_appendf(&x, "%d", v1);
            만약( strncmp(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }아니면{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    만약( !seenCom && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }아니면 만약( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  만약( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  반환 sqlite3StrAccumFinish(&x);
}
#endif /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#만약 VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
static void displayP4Expr(StrAccum *p, Expr *pExpr){
  const char *zOp = 0;
  switch( pExpr->op ){
    case TK_STRING:
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3_str_appendf(p, "%Q", pExpr->u.zToken);
      break;
    case TK_INTEGER:
      sqlite3_str_appendf(p, "%d", pExpr->u.iValue);
      break;
    case TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      break;
    case TK_REGISTER: {
      sqlite3_str_appendf(p, "r[%d]", pExpr->iTable);
      break;
    }
    case TK_COLUMN: {
      만약( pExpr->iColumn<0 ){
        sqlite3_str_appendf(p, "rowid");
      }아니면{
        sqlite3_str_appendf(p, "c%d", (정수)pExpr->iColumn);
      }
      break;
    }
    case TK_LT:      zOp = "LT";      break;
    case TK_LE:      zOp = "LE";      break;
    case TK_GT:      zOp = "GT";      break;
    case TK_GE:      zOp = "GE";      break;
    case TK_NE:      zOp = "NE";      break;
    case TK_EQ:      zOp = "EQ";      break;
    case TK_IS:      zOp = "IS";      break;
    case TK_ISNOT:   zOp = "ISNOT";   break;
    case TK_AND:     zOp = "AND";     break;
    case TK_OR:      zOp = "OR";      break;
    case TK_PLUS:    zOp = "ADD";     break;
    case TK_STAR:    zOp = "MUL";     break;
    case TK_MINUS:   zOp = "SUB";     break;
    case TK_REM:     zOp = "REM";     break;
    case TK_BITAND:  zOp = "BITAND";  break;
    case TK_BITOR:   zOp = "BITOR";   break;
    case TK_SLASH:   zOp = "DIV";     break;
    case TK_LSHIFT:  zOp = "LSHIFT";  break;
    case TK_RSHIFT:  zOp = "RSHIFT";  break;
    case TK_CONCAT:  zOp = "CONCAT";  break;
    case TK_UMINUS:  zOp = "MINUS";   break;
    case TK_UPLUS:   zOp = "PLUS";    break;
    case TK_BITNOT:  zOp = "BITNOT";  break;
    case TK_NOT:     zOp = "NOT";     break;
    case TK_ISNULL:  zOp = "ISNULL";  break;
    case TK_NOTNULL: zOp = "NOTNULL"; break;

    default:
      sqlite3_str_appendf(p, "%s", "expr");
      break;
  }

  만약( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    displayP4Expr(p, pExpr->pLeft);
    만약( pExpr->pRight ){
      sqlite3_str_append(p, ",", 1);
      displayP4Expr(p, pExpr->pRight);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#endif /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#만약 VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
char *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  char *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_LENGTH);
  switch( pOp->p4type ){
    case P4_KEYINFO: {
      정수 j;
      KeyInfo *pKeyInfo = pOp->p4.pKeyInfo;
      assert( pKeyInfo->aSortFlags!=0 );
      sqlite3_str_appendf(&x, "k(%d", pKeyInfo->nKeyField);
      위해(j=0; j<pKeyInfo->nKeyField; j++){
        CollSeq *pColl = pKeyInfo->aColl[j];
        const char *zColl = pColl ? pColl->zName : "";
        만약( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (pKeyInfo->aSortFlags[j] & KEYINFO_ORDER_DESC) ? "-" : "",
               (pKeyInfo->aSortFlags[j] & KEYINFO_ORDER_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      break;
    }
#ifdef SQLITE_ENABLE_CURSOR_HINTS
    case P4_EXPR: {
      displayP4Expr(&x, pOp->p4.pExpr);
      break;
    }
#endif
    case P4_COLLSEQ: {
      static const char *const encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      assert( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      break;
    }
    case P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      break;
    }
    case P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      break;
    }
    case P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      break;
    }
    case P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      만약( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }아니면 만약( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }아니면 만약( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }아니면 만약( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }아니면{
        assert( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      break;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    case P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      break;
    }
#endif
    case P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      위해(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      break;
    }
    case P4_SUBPROGRAM: {
      zP4 = "program";
      break;
    }
    case P4_TABLE: {
      zP4 = pOp->p4.pTab->zName;
      break;
    }
    case P4_INDEX: {
      zP4 = pOp->p4.pIdx->zName;
      break;
    }
    case P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      break;
    }
    default: {
      zP4 = pOp->p4.z;
    }
  }
  만약( zP4 ) sqlite3_str_appendall(&x, zP4);
  만약( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  반환 sqlite3StrAccumFinish(&x);
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
void sqlite3VdbeUsesBtree(Vdbe *p, 정수 i){
  assert( i>=0 && i<p->db->nDb && i<(정수)sizeof(yDbMask)*8 );
  assert( i<(정수)sizeof(p->btreeMask)*8 );
  DbMaskSet(p->btreeMask, i);
  만약( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    DbMaskSet(p->lockMask, i);
  }
}

#만약 !defined(SQLITE_OMIT_SHARED_CACHE)
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
void sqlite3VdbeEnter(Vdbe *p){
  정수 i;
  sqlite3 *db;
  Db *aDb;
  정수 nDb;
  만약( DbMaskAllZero(p->lockMask) ) 반환;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  위해(i=0; i<nDb; i++){
    만약( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#endif

#만약 !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
static SQLITE_NOINLINE void vdbeLeave(Vdbe *p){
  정수 i;
  sqlite3 *db;
  Db *aDb;
  정수 nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  위해(i=0; i<nDb; i++){
    만약( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
void sqlite3VdbeLeave(Vdbe *p){
  만약( DbMaskAllZero(p->lockMask) ) 반환;  /* The common case */
  vdbeLeave(p);
}
#endif

#만약 defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
void sqlite3VdbePrintOp(FILE *pOut, 정수 pc, VdbeOp *pOp){
  char *zP4;
  char *zCom;
  sqlite3 dummyDb;
  static const char *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  만약( pOut==0 ) pOut = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#아니면
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
static void initMemArray(Mem *p, 정수 N, sqlite3 *db, u16 flags){
  assert( db!=0 );
  만약( N>0 ){
    do{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#ifdef SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#endif
      p++;
    }동안( (--N)>0 );
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
static void releaseMemArray(Mem *p, 정수 N){
  만약( p && N ){
    Mem *pEnd = &p[N];
    sqlite3 *db = p->db;
    assert( db!=0 );
    만약( db->pnBytesFreed ){
      do{
        만약( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }동안( (++p)<pEnd );
      반환;
    }
    do{
      assert( (&p[1])==pEnd || p[0].db==p[1].db );
      assert( sqlite3VdbeCheckMemInvariants(p) );

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
      만약( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3VdbeFrameMemDel );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }아니면 만약( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#ifdef SQLITE_DEBUG
      아니면{
        p->flags = MEM_Undefined;
      }
#endif
    }동안( (++p)<pEnd );
  }
}

#ifdef SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
정수 sqlite3VdbeFrameIsValid(VdbeFrame *pFrame){
  만약( pFrame->iFrameMagic!=SQLITE_FRAME_MAGIC ) 반환 0;
  반환 1;
}
#endif


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
void sqlite3VdbeFrameMemDel(void *pArg){
  VdbeFrame *pFrame = (VdbeFrame*)pArg;
  assert( sqlite3VdbeFrameIsValid(pFrame) );
  pFrame->pParent = pFrame->v->pDelFrame;
  pFrame->v->pDelFrame = pFrame;
}

#만약 defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
정수 sqlite3VdbeNextOpcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  정수 eMode,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  정수 *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  정수 *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  정수 nRow;                            /* Stop when row count reaches this */
  정수 nSub = 0;                        /* Number of sub-vdbes seen so far */
  SubProgram **apSub = 0;              /* Array of sub-vdbes */
  정수 i;                               /* Next instruction address */
  정수 rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  정수 iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  nRow = p->nOp;
  만약( pSub!=0 ){
    만약( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/sizeof(Vdbe*);
      apSub = (SubProgram **)pSub->z;
    }
    위해(i=0; i<nSub; i++){
      nRow += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  동안(1){  /* Loop exits via break */
    i = iPc++;
    만약( i>=nRow ){
      p->rc = SQLITE_OK;
      rc = SQLITE_DONE;
      break;
    }
    만약( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }아니면{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      정수 j;
      i -= p->nOp;
      assert( apSub!=0 );
      assert( nSub>0 );
      위해(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        assert( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    만약( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      정수 nByte = (nSub+1)*sizeof(SubProgram*);
      정수 j;
      위해(j=0; j<nSub; j++){
        만약( apSub[j]==aOp[i].p4.pProgram ) break;
      }
      만약( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        만약( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          break;
        }
        apSub = (SubProgram **)pSub->z;
        apSub[nSub++] = aOp[i].p4.pProgram;
        MemSetTypeFlag(pSub, MEM_Blob);
        pSub->n = nSub*sizeof(SubProgram*);
        nRow += aOp[i].p4.pProgram->nOp;
      }
    }
    만약( eMode==0 ) break;
#ifdef SQLITE_ENABLE_BYTECODE_VTAB
    만약( eMode==2 ){
      Op *pOp = aOp + i;
      만약( pOp->opcode==OP_OpenRead ) break;
      만약( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) break;
      만약( pOp->opcode==OP_ReopenIdx ) break;     
    }아니면
#endif
    {
      assert( eMode==1 );
      만약( aOp[i].opcode==OP_Explain ) break;
      만약( aOp[i].opcode==OP_Init && iPc>1 ) break;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  반환 rc;
}
#endif /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
void sqlite3VdbeFrameDelete(VdbeFrame *p){
  정수 i;
  Mem *aMem = VdbeFrameMem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->nChildMem];
  assert( sqlite3VdbeFrameIsValid(p) );
  위해(i=0; i<p->nChildCsr; i++){
    만약( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
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
정수 sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  정수 i;                               /* Loop counter */
  정수 rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  정수 bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  assert( p->explain );
  assert( p->eVdbeState==VDBE_RUN_STATE );
  assert( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  만약( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    반환 SQLITE_ERROR;
  }

  만약( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    assert( p->nMem>9 );
    pSub = &p->aMem[9];
  }아니면{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3VdbeNextOpcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  만약( rc==SQLITE_OK ){
    pOp = aOp + i;
    만약( AtomicLoad(&db->u1.isInterrupted) ){
      p->rc = SQLITE_INTERRUPT;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }아니면{
      char *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      만약( p->explain==2 ){
        sqlite3VdbeMemSetInt64(pMem, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+1, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p3);
        sqlite3VdbeMemSetStr(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        assert( p->nResColumn==4 );
      }아니면{
        sqlite3VdbeMemSetInt64(pMem+0, i);
        sqlite3VdbeMemSetStr(pMem+1, (char*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+3, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3VdbeMemSetInt64(pMem+6, pOp->p5);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          char *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3VdbeMemSetStr(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#아니면
        sqlite3VdbeMemSetNull(pMem+7);
#endif
        sqlite3VdbeMemSetStr(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        assert( p->nResColumn==8 );
      }
      p->pResultRow = pMem;
      만약( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }아니면{
        p->rc = SQLITE_OK;
        rc = SQLITE_ROW;
      }
    }
  }
  반환 rc;
}
#endif /* SQLITE_OMIT_EXPLAIN */

#ifdef SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
void sqlite3VdbePrintSql(Vdbe *p){
  const char *z = 0;
  만약( p->zSql ){
    z = p->zSql;
  }아니면 만약( p->nOp>=1 ){
    const VdbeOp *pOp = &p->aOp[0];
    만약( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      동안( sqlite3Isspace(*z) ) z++;
    }
  }
  만약( z ) printf("SQL: [%s]\n", z);
}
#endif

#만약 !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
void sqlite3VdbeIOTraceSql(Vdbe *p){
  정수 nOp = p->nOp;
  VdbeOp *pOp;
  만약( sqlite3IoTrace==0 ) 반환;
  만약( nOp<1 ) 반환;
  pOp = &p->aOp[0];
  만약( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    정수 i, j;
    char z[1000];
    sqlite3_snprintf(sizeof(z), z, "%s", pOp->p4.z);
    위해(i=0; sqlite3Isspace(z[i]); i++){}
    위해(j=0; z[i]; i++){
      만약( sqlite3Isspace(z[i]) ){
        만약( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }아니면{
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
struct ReusableSpace {
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
static void *allocSpace(
  struct ReusableSpace *p,  /* Bulk memory available for allocation */
  void *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  assert( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  만약( pBuf==0 ){
    nByte = ROUND8P(nByte);
    만약( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }아니면{
      p->nNeeded += nByte;
    }
  }
  assert( EIGHT_BYTE_ALIGNMENT(pBuf) );
  반환 pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
void sqlite3VdbeRewind(Vdbe *p){
#만약 defined(SQLITE_DEBUG)
  정수 i;
#endif
  assert( p!=0 );
  assert( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  assert( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#ifdef SQLITE_DEBUG
  위해(i=0; i<p->nMem; i++){
    assert( p->aMem[i].db==p->db );
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
  위해(i=0; i<p->nOp; i++){
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
void sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  정수 nVar;                      /* Number of parameters */
  정수 nMem;                      /* Number of VM memory registers */
  정수 nCursor;                   /* Number of cursors required */
  정수 nArg;                      /* Max number args to xFilter or xUpdate */
  정수 n;                         /* Loop counter */
  struct ReusableSpace x;        /* Reusable bulk memory */

  assert( p!=0 );
  assert( p->nOp>0 );
  assert( pParse!=0 );
  assert( p->eVdbeState==VDBE_INIT_STATE );
  assert( pParse==p->pParse );
  assert( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  assert( db->mallocFailed==0 );
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
  만약( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(sizeof(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  assert( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  assert( x.nFree>=0 );
  assert( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolveP2Values(p, &nArg);
  p->usesStmtJournal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  만약( pParse->explain ){
    만약( nMem<10 ) nMem = 10;
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
  p->aMem = allocSpace(&x, 0, nMem*sizeof(Mem));
  p->aVar = allocSpace(&x, 0, nVar*sizeof(Mem));
  p->apArg = allocSpace(&x, 0, nArg*sizeof(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*sizeof(VdbeCursor*));
  만약( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    만약( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*sizeof(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*sizeof(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*sizeof(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*sizeof(VdbeCursor*));
    }
  }
#ifdef SQLITE_DEBUG
  p->napArg = nArg;
#endif

  만약( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }아니면{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*sizeof(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
void sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  만약( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
static SQLITE_NOINLINE void freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  assert( pCx->colCache );
  pCx->colCache = 0;
  pCx->pCache = 0;
  만약( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
void sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  만약( pCx->colCache ){
    freeCursorWithCache(p, pCx);
    반환;
  }
  switch( pCx->eCurType ){
    case CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      break;
    }
    case CURTYPE_BTREE: {
      assert( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      break;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    case CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      const sqlite3_module *pModule = pVCur->pVtab->pModule;
      assert( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      break;
    }
#endif
  }
}

/*
** Close all cursors in the current frame.
*/
static void closeCursorsInFrame(Vdbe *p){
  정수 i;
  위해(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    만약( pC ){
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
정수 sqlite3VdbeFrameRestore(VdbeFrame *pFrame){
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
  반환 pFrame->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
static void closeAllCursors(Vdbe *p){
  만약( p->pFrame ){
    VdbeFrame *pFrame;
    위해(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    sqlite3VdbeFrameRestore(pFrame);
    p->pFrame = 0;
    p->nFrame = 0;
  }
  assert( p->nFrame==0 );
  closeCursorsInFrame(p);
  releaseMemArray(p->aMem, p->nMem);
  동안( p->pDelFrame ){
    VdbeFrame *pDel = p->pDelFrame;
    p->pDelFrame = pDel->pParent;
    sqlite3VdbeFrameDelete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  만약( p->pAuxData ) sqlite3VdbeDeleteAuxData(p->db, &p->pAuxData, -1, 0);
  assert( p->pAuxData==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
void sqlite3VdbeSetNumCols(Vdbe *p, 정수 nResColumn){
  정수 n;
  sqlite3 *db = p->db;

  만약( p->nResAlloc ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->aColName);
  }
  n = nResColumn*COLNAME_N;
  p->nResColumn = p->nResAlloc = (u16)nResColumn;
  p->aColName = (Mem*)sqlite3DbMallocRawNN(db, sizeof(Mem)*n );
  만약( p->aColName==0 ) 반환;
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
정수 sqlite3VdbeSetColName(
  Vdbe *p,                         /* Vdbe being configured */
  정수 idx,                         /* Index of column zName applies to */
  정수 var,                         /* One of the COLNAME_* constants */
  const char *zName,               /* Pointer to buffer containing name */
  void (*xDel)(void*)              /* Memory management strategy for zName */
){
  정수 rc;
  Mem *pColName;
  assert( idx<p->nResAlloc );
  assert( var<COLNAME_N );
  만약( p->db->mallocFailed ){
    assert( !zName || xDel!=SQLITE_DYNAMIC );
    반환 SQLITE_NOMEM_BKPT;
  }
  assert( p->aColName!=0 );
  pColName = &(p->aColName[idx+var*p->nResAlloc]);
  rc = sqlite3VdbeMemSetText(pColName, zName, -1, xDel);
  assert( rc!=0 || !zName || (pColName->flags&MEM_Term)!=0 );
  반환 rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
static 정수 vdbeCommit(sqlite3 *db, Vdbe *p){
  정수 i;
  정수 nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  정수 rc = SQLITE_OK;
  정수 needXcommit = 0;

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
  위해(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    만약( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      static const u8 aMJNeeded[] = {
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
      만약( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3PagerGetJournalMode(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        assert( i!=1 );
        nTrans++;
      }
      rc = sqlite3PagerExclusiveLock(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  만약( rc!=SQLITE_OK ){
    반환 rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  만약( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    만약( rc ){
      반환 SQLITE_CONSTRAINT_COMMITHOOK;
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
  만약( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    만약( needXcommit ){
      위해(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        만약( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    위해(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      정수 txn = sqlite3BtreeTxnState(pBt);
      만약( txn!=SQLITE_TXN_NONE ){
        assert( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    만약( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#ifndef SQLITE_OMIT_DISKIO
  아니면{
    sqlite3_vfs *pVfs = db->pVfs;
    char *zSuper = 0;   /* File-name for the super-journal */
    char const *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 offset = 0;
    정수 res;
    정수 retryCount = 0;
    정수 nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    만약( zSuper==0 ) 반환 SQLITE_NOMEM_BKPT;
    zSuper += 4;
    do {
      u32 iRandom;
      만약( retryCount ){
        만약( retryCount>100 ){
          sqlite3_log(SQLITE_FULL, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          break;
        }아니면 만약( retryCount==1 ){
          sqlite3_log(SQLITE_FULL, "MJ collide: %s", zSuper);
        }
      }
      retryCount++;
      sqlite3_randomness(sizeof(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      assert( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }동안( rc==SQLITE_OK && res );
    만약( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    만약( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      반환 rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    위해(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      만약( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        char const *zFile = sqlite3BtreeGetJournalname(pBt);
        만약( zFile==0 ){
          continue;  /* Ignore TEMP and :memory: databases */
        }
        assert( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,offset);
        offset += sqlite3Strlen30(zFile)+1;
        만약( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          반환 rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    만약( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_NORMAL))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      반환 rc;
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
    위해(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      만약( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    assert( rc!=SQLITE_BUSY );
    만약( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      반환 rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    만약( rc ){
      반환 rc;
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
    위해(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      만약( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3EndBenignMalloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#endif

  반환 rc;
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
static void checkActiveVdbeCnt(sqlite3 *db){
  Vdbe *p;
  정수 cnt = 0;
  정수 nWrite = 0;
  정수 nRead = 0;
  p = db->pVdbe;
  동안( p ){
    만약( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      만약( p->readOnly==0 ) nWrite++;
      만약( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  assert( cnt==db->nVdbeActive );
  assert( nWrite==db->nVdbeWrite );
  assert( nRead==db->nVdbeRead );
}
#아니면
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
static SQLITE_NOINLINE 정수 vdbeCloseStatement(Vdbe *p, 정수 eOp){
  sqlite3 *const db = p->db;
  정수 rc = SQLITE_OK;
  정수 i;
  const 정수 iSavepoint = p->iStatement-1;

  assert( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  assert( db->nStatement>0 );
  assert( p->iStatement==(db->nStatement+db->nSavepoint) );

  위해(i=0; i<db->nDb; i++){
    정수 rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    만약( pBt ){
      만약( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      만약( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      만약( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  만약( rc==SQLITE_OK ){
    만약( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    만약( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  만약( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  반환 rc;
}
정수 sqlite3VdbeCloseStatement(Vdbe *p, 정수 eOp){
  만약( p->db->nStatement && p->iStatement ){
    반환 vdbeCloseStatement(p, eOp);
  }
  반환 SQLITE_OK;
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
#ifndef SQLITE_OMIT_FOREIGN_KEY
static SQLITE_NOINLINE 정수 vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  만약( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) 반환 SQLITE_ERROR;
  반환 SQLITE_CONSTRAINT_FOREIGNKEY;
}
정수 sqlite3VdbeCheckFkImmediate(Vdbe *p){
  만약( p->nFkConstraint==0 ) 반환 SQLITE_OK;
  반환 vdbeFkError(p);
}
정수 sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  만약( (db->nDeferredCons+db->nDeferredImmCons)==0 ) 반환 SQLITE_OK;
  반환 vdbeFkError(p);
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
정수 sqlite3VdbeHalt(Vdbe *p){
  정수 rc;                         /* Used to store transient return codes */
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

  assert( p->eVdbeState==VDBE_RUN_STATE );
  만약( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  checkActiveVdbeCnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  만약( p->bIsReader ){
    정수 mrc;   /* Primary error code from p->rc */
    정수 eStatementOp = 0;
    정수 isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    만약( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_INTERRUPT
                    || mrc==SQLITE_FULL;
    }아니면{
      mrc = isSpecialError = 0;
    }
    만약( isSpecialError ){
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
      만약( !p->readOnly || mrc!=SQLITE_INTERRUPT ){
        만약( (mrc==SQLITE_NOMEM || mrc==SQLITE_FULL) && p->usesStmtJournal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }아니면{
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
    만약( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (void)sqlite3VdbeCheckFkImmediate(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    만약( !sqlite3VtabInSync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      만약( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        만약( rc!=SQLITE_OK ){
          만약( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            반환 SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }아니면 만약( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }아니면{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        만약( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          반환 SQLITE_BUSY;
        }아니면 만약( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }아니면{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3CommitInternalChanges(db);
        }
      }아니면 만약( p->rc==SQLITE_SCHEMA && db->nVdbeActive>1 ){
        p->nChange = 0;
      }아니면{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }아니면 만약( eStatementOp==0 ){
      만약( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }아니면 만약( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }아니면{
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
    만약( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      만약( rc ){
        만약( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->zErrMsg);
          p->zErrMsg = 0;
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
    만약( p->changeCntOn ){
      만약( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3VdbeSetChanges(db, p->nChange);
      }아니면{
        sqlite3VdbeSetChanges(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->nVdbeActive--;
  만약( !p->readOnly ) db->nVdbeWrite--;
  만약( p->bIsReader ) db->nVdbeRead--;
  assert( db->nVdbeActive>=db->nVdbeRead );
  assert( db->nVdbeRead>=db->nVdbeWrite );
  assert( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  checkActiveVdbeCnt(db);
  만약( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  만약( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  assert( db->nVdbeActive>0 || db->autoCommit==0 || db->nStatement==0 );
  반환 (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
void sqlite3VdbeResetStepResult(Vdbe *p){
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
정수 sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  정수 rc = p->rc;
  만약( p->zErrMsg ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    만약( db->pErr==0 ) db->pErr = sqlite3ValueNew(db);
    sqlite3ValueSetStr(db->pErr, -1, p->zErrMsg, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3EndBenignMalloc();
    db->bBenignMalloc--;
  }아니면 만약( db->pErr ){
    sqlite3ValueSetNull(db->pErr);
  }
  db->errCode = rc;
  db->errByteOffset = -1;
  반환 rc;
}

#ifdef SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
static void vdbeInvokeSqllog(Vdbe *v){
  만약( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    char *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    assert( v->db->init.busy==0 );
    만약( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#아니면
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
정수 sqlite3VdbeReset(Vdbe *p){
#만약 defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  정수 i;
#endif

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  만약( p->eVdbeState==VDBE_RUN_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  만약( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    만약( db->pErr || p->zErrMsg ){
      sqlite3VdbeTransferError(p);
    }아니면{
      db->errCode = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#ifdef SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  만약( p->apCsr ) 위해(i=0; i<p->nCursor; i++) assert( p->apCsr[i]==0 );
  만약( p->aMem ){
    위해(i=0; i<p->nMem; i++) assert( p->aMem[i].flags==MEM_Undefined );
  }
#endif
  만약( p->zErrMsg ){
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = 0;
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
    만약( out ){
      fprintf(out, "---- ");
      위해(i=0; i<p->nOp; i++){
        fprintf(out, "%02x", p->aOp[i].opcode);
      }
      fprintf(out, "\n");
      만약( p->zSql ){
        char c, pc = 0;
        fprintf(out, "-- ");
        위해(i=0; (c = p->zSql[i])!=0; i++){
          만약( pc=='\n' ) fprintf(out, "-- ");
          putc(c, out);
          pc = c;
        }
        만약( pc!='\n' ) fprintf(out, "\n");
      }
      위해(i=0; i<p->nOp; i++){
        char zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 cycles = p->aOp[i].nCycle;
        sqlite3_snprintf(sizeof(zHdr), zHdr, "%6u %12llu %8llu ",
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
  반환 p->rc & db->errMask;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
정수 sqlite3VdbeFinalize(Vdbe *p){
  정수 rc = SQLITE_OK;
  assert( VDBE_RUN_STATE>VDBE_READY_STATE );
  assert( VDBE_HALT_STATE>VDBE_READY_STATE );
  assert( VDBE_INIT_STATE<VDBE_READY_STATE );
  만약( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    assert( (rc & p->db->errMask)==rc );
  }
  sqlite3VdbeDelete(p);
  반환 rc;
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
void sqlite3VdbeDeleteAuxData(sqlite3 *db, AuxData **pp, 정수 iOp, 정수 mask){
  동안( *pp ){
    AuxData *pAux = *pp;
    만약( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(mask & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      만약( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->pNextAux;
      sqlite3DbFree(db, pAux);
    }아니면{
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
static void sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  SubProgram *pSub, *pNext;
  assert( db!=0 );
  assert( p->db==0 || p->db==db );
  만약( p->aColName ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->aColName);
  }
  위해(pSub=p->pProgram; pSub; pSub=pNext){
    pNext = pSub->pNext;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  만약( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    만약( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    만약( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  만약( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#ifdef SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    위해(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->pNextStr;
      sqlite3DbFree(db, pThis);
    }
  }
#endif
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  {
    정수 i;
    위해(i=0; i<p->nScan; i++){
      sqlite3DbFree(db, p->aScan[i].zName);
    }
    sqlite3DbFree(db, p->aScan);
  }
#endif
}

/*
** Delete an entire VDBE.
*/
void sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  assert( p!=0 );
  db = p->db;
  assert( db!=0 );
  assert( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  만약( db->pnBytesFreed==0 ){
    assert( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    만약( p->pVNext ){
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
정수 SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor *p){
  정수 res, rc;
#ifdef SQLITE_TEST
  extern 정수 sqlite3_search_count;
#endif
  assert( p->deferredMoveto );
  assert( p->isTable );
  assert( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3BtreeTableMoveto(p->uc.pCursor, p->movetoTarget, 0, &res);
  만약( rc ) 반환 rc;
  만약( res!=0 ) 반환 SQLITE_CORRUPT_BKPT;
#ifdef SQLITE_TEST
  sqlite3_search_count++;
#endif
  p->deferredMoveto = 0;
  p->cacheStatus = CACHE_STALE;
  반환 SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
정수 SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  정수 isDifferentRow, rc;
  assert( p->eCurType==CURTYPE_BTREE );
  assert( p->uc.pCursor!=0 );
  assert( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &isDifferentRow);
  p->cacheStatus = CACHE_STALE;
  만약( isDifferentRow ) p->nullRow = 1;
  반환 rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
정수 sqlite3VdbeCursorRestore(VdbeCursor *p){
  assert( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  만약( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    반환 sqlite3VdbeHandleMovedCursor(p);
  }
  반환 SQLITE_OK;
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

#만약 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, 정수 file_format, u32 *pLen){
  정수 flags = pMem->flags;
  u32 n;

  assert( pLen!=0 );
  만약( flags&MEM_Null ){
    *pLen = 0;
    반환 0;
  }
  만약( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    만약( i<0 ){
      u = ~i;
    }아니면{
      u = i;
    }
    만약( u<=127 ){
      만약( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        반환 8+(u32)u;
      }아니면{
        *pLen = 1;
        반환 1;
      }
    }
    만약( u<=32767 ){ *pLen = 2; 반환 2; }
    만약( u<=8388607 ){ *pLen = 3; 반환 3; }
    만약( u<=2147483647 ){ *pLen = 4; 반환 4; }
    만약( u<=MAX_6BYTE ){ *pLen = 6; 반환 5; }
    *pLen = 8;
    만약( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (double)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      반환 7;
    }
    반환 6;
  }
  만약( flags&MEM_Real ){
    *pLen = 8;
    반환 7;
  }
  assert( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  assert( pMem->n>=0 );
  n = (u32)pMem->n;
  만약( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  반환 ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#endif /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
const u8 sqlite3SmallTypeSizes[128] = {
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
  만약( serial_type>=128 ){
    반환 (serial_type-12)/2;
  }아니면{
    assert( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    반환 sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  assert( serial_type<128 );
  반환 sqlite3SmallTypeSizes[serial_type]; 
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
  union {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = in;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  반환 u.r;
}
#endif /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
static 정수 sqlite3VdbeSerialGet7(
  const unsigned char *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  assert( sizeof(x)==8 && sizeof(pMem->u.r)==8 );
  swapMixedEndianFloat(x);
  memcpy(&pMem->u.r, &x, sizeof(x));
  만약( IsNaN(x) ){
    pMem->flags = MEM_Null;
    반환 1;
  }
  pMem->flags = MEM_Real;
  반환 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
void sqlite3VdbeSerialGet(
  const unsigned char *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  switch( serial_type ){
    case 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      반환;
    }
    case 11:   /* Reserved for future use */
    case 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      반환;
    }
    case 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#ifdef __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      만약( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#endif
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      반환;
    }
    case 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      반환;
    }
    case 8:    /* Integer 0 */
    case 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      반환;
    }
    default: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      static const u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (char *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      반환;
    }
  }
  반환;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(
  KeyInfo *pKeyInfo               /* Description of the record */
){
  UnpackedRecord *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  assert( sizeof(UnpackedRecord) + sizeof(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(sizeof(UnpackedRecord)) + sizeof(Mem)*(pKeyInfo->nKeyField+1);
  p = (UnpackedRecord *)sqlite3DbMallocRaw(pKeyInfo->db, nByte);
  만약( !p ) 반환 0;
  p->aMem = (Mem*)&((char*)p)[ROUND8P(sizeof(UnpackedRecord))];
  p->pKeyInfo = pKeyInfo;
  p->nField = pKeyInfo->nKeyField + 1;
  반환 p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
void sqlite3VdbeRecordUnpack(
  정수 nKey,              /* Size of the binary record */
  const void *pKey,      /* The binary record */
  UnpackedRecord *p      /* Populate this structure before returning. */
){
  const unsigned char *aKey = (const unsigned char *)pKey;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  KeyInfo *pKeyInfo = p->pKeyInfo;

  p->default_rc = 0;
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(aKey, szHdr);
  d = szHdr;
  u = 0;
  동안( idx<szHdr && d<=(u32)nKey ){
    u32 serial_type;

    idx += getVarint32(&aKey[idx], serial_type);
    pMem->enc = pKeyInfo->enc;
    pMem->db = pKeyInfo->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&aKey[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    만약( (++u)>=p->nField ) break;
    pMem++;
  }
  만약( d>(u32)nKey && u ){
    assert( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3VdbeMemSetNull(pMem-(u<p->nField));
  }
  testcase( u == pKeyInfo->nKeyField + 1 );
  testcase( u < pKeyInfo->nKeyField + 1 );
  assert( u<=pKeyInfo->nKeyField + 1 );
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
static 정수 vdbeRecordCompareDebug(
  정수 nKey1, const void *pKey1, /* Left key */
  const UnpackedRecord *pPKey2, /* Right key */
  정수 desiredResult             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  정수 i = 0;
  정수 rc = 0;
  const unsigned char *aKey1 = (const unsigned char *)pKey1;
  KeyInfo *pKeyInfo;
  Mem mem1;

  pKeyInfo = pPKey2->pKeyInfo;
  만약( pKeyInfo->db==0 ) 반환 1;
  mem1.enc = pKeyInfo->enc;
  mem1.db = pKeyInfo->db;
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
  만약( szHdr1>98307 ) 반환 SQLITE_CORRUPT;
  d1 = szHdr1;
  assert( pKeyInfo->nAllField>=pPKey2->nField || CORRUPT_DB );
  assert( pKeyInfo->aSortFlags!=0 );
  assert( pKeyInfo->nKeyField>0 );
  assert( idx1<=szHdr1 || CORRUPT_DB );
  do{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    만약( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      만약( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        반환 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      break;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           pKeyInfo->nAllField>i ? pKeyInfo->aColl[i] : 0);
    만약( rc!=0 ){
      assert( mem1.szMalloc==0 );  /* See comment below */
      만약( (pKeyInfo->aSortFlags[i] & KEYINFO_ORDER_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      만약( pKeyInfo->aSortFlags[i] & KEYINFO_ORDER_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      goto debugCompareEnd;
    }
    i++;
  }동안( idx1<szHdr1 && i<pPKey2->nField );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  assert( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debugCompareEnd:
  만약( desiredResult==0 && rc==0 ) 반환 1;
  만약( desiredResult<0 && rc<0 ) 반환 1;
  만약( desiredResult>0 && rc>0 ) 반환 1;
  만약( CORRUPT_DB ) 반환 1;
  만약( pKeyInfo->db->mallocFailed ) 반환 1;
  반환 0;
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
static void vdbeAssertFieldCountWithinLimits(
  정수 nKey, const void *pKey,   /* The record to verify */
  const KeyInfo *pKeyInfo       /* Compare size with this KeyInfo */
){
  정수 nField = 0;
  u32 szHdr;
  u32 idx;
  u32 notUsed;
  const unsigned char *aKey = (const unsigned char*)pKey;

  만약( CORRUPT_DB ) 반환;
  idx = getVarint32(aKey, szHdr);
  assert( nKey>=0 );
  assert( szHdr<=(u32)nKey );
  동안( idx<szHdr ){
    idx += getVarint32(aKey+idx, notUsed);
    nField++;
  }
  assert( nField <= pKeyInfo->nAllField );
}
#아니면
# define vdbeAssertFieldCountWithinLimits(A,B,C)
#endif

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
static SQLITE_NOINLINE 정수 vdbeCompareMemStringWithEncodingChange(
  const Mem *pMem1,
  const Mem *pMem2,
  const CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  정수 rc;
  const void *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3ValueText((sqlite3_value*)&c1, pColl->enc);
  v2 = sqlite3ValueText((sqlite3_value*)&c2, pColl->enc);
  만약( (v1==0 || v2==0) ){
    만약( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }아니면{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  반환 rc;
}
static 정수 vdbeCompareMemString(
  const Mem *pMem1,
  const Mem *pMem2,
  const CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  만약( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    반환 pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }아니면{
    반환 vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
static 정수 isAllZero(const char *z, 정수 n){
  정수 i;
  위해(i=0; i<n; i++){
    만약( z[i] ) 반환 0;
  }
  반환 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE 정수 sqlite3BlobCompare(const Mem *pB1, const Mem *pB2){
  정수 c;
  정수 n1 = pB1->n;
  정수 n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  assert( (pB1->flags & MEM_Zero)==0 || n1==0 );
  assert( (pB2->flags & MEM_Zero)==0 || n2==0 );

  만약( (pB1->flags|pB2->flags) & MEM_Zero ){
    만약( pB1->flags & pB2->flags & MEM_Zero ){
      반환 pB1->u.nZero - pB2->u.nZero;
    }아니면 만약( pB1->flags & MEM_Zero ){
      만약( !isAllZero(pB2->z, pB2->n) ) 반환 -1;
      반환 pB1->u.nZero - n2;
    }아니면{
      만약( !isAllZero(pB1->z, pB1->n) ) 반환 +1;
      반환 n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  만약( c ) 반환 c;
  반환 n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#만약 defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
static 정수 SQLITE_NOINLINE doubleLt(double a, double b){ 반환 a<b; }
static 정수 SQLITE_NOINLINE doubleEq(double a, double b){ 반환 a==b; }
#endif

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
정수 sqlite3IntFloatCompare(i64 i, double r){
  만약( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    반환 1;
  }아니면{
    i64 y;
    만약( r<-9223372036854775808.0 ) 반환 +1;
    만약( r>=9223372036854775808.0 ) 반환 -1;
    y = (i64)r;
    만약( i<y ) 반환 -1;
    만약( i>y ) 반환 +1;
    testcase( doubleLt(((double)i),r) );
    testcase( doubleLt(r,((double)i)) );
    testcase( doubleEq(r,((double)i)) );
    반환 (((double)i)<r) ? -1 : (((double)i)>r);
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
정수 sqlite3MemCompare(const Mem *pMem1, const Mem *pMem2, const CollSeq *pColl){
  정수 f1, f2;
  정수 combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  assert( !sqlite3VdbeMemIsRowSet(pMem1) && !sqlite3VdbeMemIsRowSet(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  만약( combined_flags&MEM_Null ){
    반환 (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  만약( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    만약( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      만약( pMem1->u.i < pMem2->u.i ) 반환 -1;
      만약( pMem1->u.i > pMem2->u.i ) 반환 +1;
      반환 0;
    }
    만약( (f1 & f2 & MEM_Real)!=0 ){
      만약( pMem1->u.r < pMem2->u.r ) 반환 -1;
      만약( pMem1->u.r > pMem2->u.r ) 반환 +1;
      반환 0;
    }
    만약( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      만약( (f2&MEM_Real)!=0 ){
        반환 sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }아니면 만약( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        만약( pMem1->u.i < pMem2->u.i ) 반환 -1;
        만약( pMem1->u.i > pMem2->u.i ) 반환 +1;
        반환 0;
      }아니면{
        반환 -1;
      }
    }
    만약( (f1&MEM_Real)!=0 ){
      만약( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        반환 -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }아니면{
        반환 -1;
      }
    }
    반환 +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  만약( combined_flags&MEM_Str ){
    만약( (f1 & MEM_Str)==0 ){
      반환 1;
    }
    만약( (f2 & MEM_Str)==0 ){
      반환 -1;
    }

    assert( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    assert( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    assert( !pColl || pColl->xCmp );

    만약( pColl ){
      반환 vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  반환 sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
static i64 vdbeRecordDecodeInt(u32 serial_type, const u8 *aKey){
  u32 y;
  assert( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  switch( serial_type ){
    case 0:
    case 1:
      testcase( aKey[0]&0x80 );
      반환 ONE_BYTE_INT(aKey);
    case 2:
      testcase( aKey[0]&0x80 );
      반환 TWO_BYTE_INT(aKey);
    case 3:
      testcase( aKey[0]&0x80 );
      반환 THREE_BYTE_INT(aKey);
    case 4: {
      testcase( aKey[0]&0x80 );
      y = FOUR_BYTE_UINT(aKey);
      반환 (i64)*(정수*)&y;
    }
    case 5: {
      testcase( aKey[0]&0x80 );
      반환 FOUR_BYTE_UINT(aKey+2) + (((i64)1)<<32)*TWO_BYTE_INT(aKey);
    }
    case 6: {
      u64 x = FOUR_BYTE_UINT(aKey);
      testcase( aKey[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(aKey+4);
      반환 (i64)*(i64*)&x;
    }
  }

  반환 (serial_type - 8);
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
정수 sqlite3VdbeRecordCompareWithSkip(
  정수 nKey1, const void *pKey1,   /* Left key */
  UnpackedRecord *pPKey2,         /* Right key */
  정수 bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  정수 i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  정수 rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  KeyInfo *pKeyInfo;
  const unsigned char *aKey1 = (const unsigned char *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  만약( bSkip ){
    u32 s1 = aKey1[1];
    만약( s1<0x80 ){
      idx1 = 2;
    }아니면{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }아니면{
    만약( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }아니면{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  만약( d1>(unsigned)nKey1 ){
    pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
    반환 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  assert( pPKey2->pKeyInfo->nAllField>=pPKey2->nField
       || CORRUPT_DB );
  assert( pPKey2->pKeyInfo->aSortFlags!=0 );
  assert( pPKey2->pKeyInfo->nKeyField>0 );
  assert( idx1<=szHdr1 || CORRUPT_DB );
  동안( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    만약( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      만약( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }아니면 만약( serial_type==0 ){
        rc = -1;
      }아니면 만약( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }아니면{
        i64 lhs = vdbeRecordDecodeInt(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        만약( lhs<rhs ){
          rc = -1;
        }아니면 만약( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    아니면 만약( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      만약( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }아니면 만약( serial_type==0 ){
        rc = -1;
      }아니면{
        만약( serial_type==7 ){
          만약( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }아니면 만약( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }아니면 만약( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }아니면{
            assert( rc==0 );
          }
        }아니면{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    아니면 만약( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      만약( serial_type<12 ){
        rc = -1;
      }아니면 만약( !(serial_type & 0x01) ){
        rc = +1;
      }아니면{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(unsigned)nKey1 );
        testcase( (d1+mem1.n+1)==(unsigned)nKey1 );
        만약( (d1+mem1.n) > (unsigned)nKey1
         || (pKeyInfo = pPKey2->pKeyInfo)->nAllField<=i
        ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          반환 0;                /* Corruption */
        }아니면 만약( pKeyInfo->aColl[i] ){
          mem1.enc = pKeyInfo->enc;
          mem1.db = pKeyInfo->db;
          mem1.flags = MEM_Str;
          mem1.z = (char*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, pKeyInfo->aColl[i], &pPKey2->errCode
          );
        }아니면{
          정수 nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          만약( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    아니면 만약( pRhs->flags & MEM_Blob ){
      assert( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      만약( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }아니면{
        정수 nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(unsigned)nKey1 );
        testcase( (d1+nStr+1)==(unsigned)nKey1 );
        만약( (d1+nStr) > (unsigned)nKey1 ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          반환 0;                /* Corruption */
        }아니면 만약( pRhs->flags & MEM_Zero ){
          만약( !isAllZero((const char*)&aKey1[d1],nStr) ){
            rc = 1;
          }아니면{
            rc = nStr - pRhs->u.nZero;
          }
        }아니면{
          정수 nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          만약( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    아니면{
      serial_type = aKey1[idx1];
      만약( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        assert( rc==0 );
      }아니면{
        rc = 1;
      }
    }

    만약( rc!=0 ){
      정수 sortFlags = pPKey2->pKeyInfo->aSortFlags[i];
      만약( sortFlags ){
        만약( (sortFlags & KEYINFO_ORDER_BIGNULL)==0
         || ((sortFlags & KEYINFO_ORDER_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      assert( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, rc) );
      assert( mem1.szMalloc==0 );  /* See comment below */
      반환 rc;
    }

    i++;
    만약( i==pPKey2->nField ) break;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    만약( d1>(unsigned)nKey1 ) break;
    idx1 += sqlite3VarintLen(serial_type);
    만약( idx1>=(unsigned)szHdr1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      반환 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  assert( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  assert( CORRUPT_DB
       || vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->pKeyInfo->db->mallocFailed
  );
  pPKey2->eqSeen = 1;
  반환 pPKey2->default_rc;
}
정수 sqlite3VdbeRecordCompare(
  정수 nKey1, const void *pKey1,   /* Left key */
  UnpackedRecord *pPKey2          /* Right key */
){
  반환 sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 0);
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
static 정수 vdbeRecordCompareInt(
  정수 nKey1, const void *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  const u8 *aKey = &((const u8*)pKey1)[*(const u8*)pKey1 & 0x3F];
  정수 serial_type = ((const u8*)pKey1)[1];
  정수 res;
  i64 v;
  i64 lhs;

  vdbeAssertFieldCountWithinLimits(nKey1, pKey1, pPKey2->pKeyInfo);
  assert( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  만약( (u32)(serial_type-1)<=5 ){
    static const u8 aShift[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(aKey)) >> aShift[serial_type];
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
    assert( 0<=(i64)sqlite3Get8byte(aKey) || lhs<0 );
    testcase( lhs<0 );
  }아니면 만약( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }아니면{
    반환 sqlite3VdbeRecordCompare(nKey1, pKey1, pPKey2);
  }

  assert( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  만약( v>lhs ){
    res = pPKey2->r1;
  }아니면 만약( v<lhs ){
    res = pPKey2->r2;
  }아니면 만약( pPKey2->nField>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
  }아니면{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eqSeen = 1;
  }

  assert( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res) );
  반환 res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
static 정수 vdbeRecordCompareString(
  정수 nKey1, const void *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  const u8 *aKey1 = (const u8*)pKey1;
  정수 serial_type;
  정수 res;

  assert( pPKey2->aMem[0].flags & MEM_Str );
  assert( pPKey2->aMem[0].n == pPKey2->n );
  assert( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbeAssertFieldCountWithinLimits(nKey1, pKey1, pPKey2->pKeyInfo);
  serial_type = (signed char)(aKey1[1]);

vrcs_restart:
  만약( serial_type<12 ){
    만약( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      만약( serial_type>=12 ) goto vrcs_restart;
      assert( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }아니면 만약( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }아니면{
    정수 nCmp;
    정수 nStr;
    정수 szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    만약( (szHdr + nStr) > nKey1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      반환 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    만약( res>0 ){
      res = pPKey2->r2;
    }아니면 만약( res<0 ){
      res = pPKey2->r1;
    }아니면{
      res = nStr - pPKey2->n;
      만약( res==0 ){
        만약( pPKey2->nField>1 ){
          res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
        }아니면{
          res = pPKey2->default_rc;
          pPKey2->eqSeen = 1;
        }
      }아니면 만약( res>0 ){
        res = pPKey2->r2;
      }아니면{
        res = pPKey2->r1;
      }
    }
  }

  assert( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->pKeyInfo->db->mallocFailed
  );
  반환 res;
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
  assert( p->pKeyInfo->aSortFlags!=0 );
  만약( p->pKeyInfo->nAllField<=13 ){
    정수 flags = p->aMem[0].flags;
    만약( p->pKeyInfo->aSortFlags[0] ){
      만약( p->pKeyInfo->aSortFlags[0] & KEYINFO_ORDER_BIGNULL ){
        반환 sqlite3VdbeRecordCompare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }아니면{
      p->r1 = -1;
      p->r2 = 1;
    }
    만약( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      반환 vdbeRecordCompareInt;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    만약( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3IsBinary(p->pKeyInfo->aColl[0])
    ){
      assert( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      반환 vdbeRecordCompareString;
    }
  }

  반환 sqlite3VdbeRecordCompare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
정수 sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 nCellKey = 0;
  정수 rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  assert( sqlite3BtreeCursorIsValid(pCur) );
  nCellKey = sqlite3BtreePayloadSize(pCur);
  assert( (nCellKey & SQLITE_MAX_U32)==(u64)nCellKey );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)nCellKey, &m);
  만약( rc ){
    반환 rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  assert( m.n>=0 );
  만약( unlikely(szHdr<3 || szHdr>(unsigned)m.n) ){
    goto idx_rowid_corruption;
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
  만약( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    goto idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  만약( unlikely((u32)m.n<szHdr+lenRowid) ){
    goto idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  반환 SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  반환 SQLITE_CORRUPT_BKPT;
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
정수 sqlite3VdbeIdxKeyCompare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  UnpackedRecord *pUnpacked,       /* Unpacked version of key */
  정수 *res                         /* Write the comparison result here */
){
  i64 nCellKey = 0;
  정수 rc;
  BtCursor *pCur;
  Mem m;

  assert( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  assert( sqlite3BtreeCursorIsValid(pCur) );
  nCellKey = sqlite3BtreePayloadSize(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  만약( nCellKey<=0 || nCellKey>0x7fffffff ){
    *res = 0;
    반환 SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)nCellKey, &m);
  만약( rc ){
    반환 rc;
  }
  *res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  반환 SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
void sqlite3VdbeSetChanges(sqlite3 *db, i64 nChange){
  assert( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->nTotalChange += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
void sqlite3VdbeCountChanges(Vdbe *v){
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
void sqlite3ExpirePreparedStatements(sqlite3 *db, 정수 iCode){
  Vdbe *p;
  위해(p = db->pVdbe; p; p=p->pVNext){
    p->expired = iCode+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  반환 v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  반환 v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_value *sqlite3VdbeGetBoundValue(Vdbe *v, 정수 iVar, u8 aff){
  assert( iVar>0 );
  만약( v ){
    Mem *pMem = &v->aVar[iVar-1];
    assert( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );
    만약( 0==(pMem->flags & MEM_Null) ){
      sqlite3_value *pRet = sqlite3ValueNew(v->db);
      만약( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3ValueApplyAffinity(pRet, aff, SQLITE_UTF8);
      }
      반환 pRet;
    }
  }
  반환 0;
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
void sqlite3VdbeReprepareOnBind(Vdbe *v, 정수 iVar, 정수 bSmallint){
  u32 m;
  assert( iVar>0 );
  assert( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  만약( bSmallint ){
    만약( (v->expmask & m)==0 ) v->smimask |= m;
  }아니면{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  assert( (v->smimask & v->expmask) == v->smimask );
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
static 정수 vdbeSkipField(
  Bitmask mask,                   /* Mask of indexed expression fields */
  정수 iCol,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  정수 bIntegrity                  /* True if running PRAGMA integrity_check */
){
#define BTREE_ULPDISTORTION 2
  만약( iCol>=BMS || (mask & MASKBIT(iCol))==0 ) 반환 0;
  만약( bIntegrity==0 ) 반환 1;
  만약( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    memcpy(&m1,&pMem1->u.r,8);
    memcpy(&m2,&pMem2->u.r,8);
    만약( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      반환 1;
    }
  }
  반환 0;
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
static 정수 vdbeIsMatchingIndexKey(
  BtCursor *pCur,            /* Cursor open on index */
  정수 bInt,                  /* True for integrity_check-style search */
  Bitmask mask,              /* Mask of columns to skip */
  UnpackedRecord *p,         /* Index key being deleted */
  정수 *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  정수 rc = SQLITE_OK;

  memset(&m, 0, sizeof(m));
  m.enc = p->pKeyInfo->enc;
  m.db = p->pKeyInfo->db;
  nRec = sqlite3BtreePayloadSize(pCur);
  만약( nRec>0x7fffffff ){
    반환 SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  만약( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }아니면{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  만약( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    만약( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }아니면{
      정수 res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      정수 ii = 0;                 /* Iterator variable */

      정수 nCol = p->pKeyInfo->nAllField;
      위해(ii=0; ii<nCol && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        정수 nSerial = 0;

        만약( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          break;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        만약( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }아니면{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          만약( vdbeSkipField(mask, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->pKeyInfo->aColl[ii]);
            만약( res!=0 ) break;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  반환 rc;
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
정수 sqlite3VdbeFindIndexKey(
  BtCursor *pCur, 
  Index *pIdx,
  UnpackedRecord *p, 
  정수 *pRes,
  정수 bIntegrity
){
#define BTREE_FDK_RANGE 10
  정수 nStep = 0;
  정수 res = 1;
  정수 rc = SQLITE_OK;
  정수 ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask mask = 0;
  위해(ii=0; ii<MIN(pIdx->nColumn, BMS); ii++){
    정수 iCol = pIdx->aiColumn[ii];
    만약( (iCol==XN_EXPR)
     || (iCol>=0 && (pIdx->pTable->aCol[iCol].colFlags & COLFLAG_VIRTUAL))
    ){
      mask |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  만약( mask!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    위해(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3BtreePrevious(pCur, 0);
    }
    만약( rc==SQLITE_DONE ){
      rc = sqlite3BtreeFirst(pCur, &res);
      nStep = -1;
    }아니면{
      nStep = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    동안( sqlite3BtreeCursorIsValidNN(pCur) ){
      위해(ii=0; rc==SQLITE_OK && (ii<nStep || nStep<0); ii++){
        rc = vdbeIsMatchingIndexKey(pCur, bIntegrity, mask, p, &res);
        만약( res==0 || rc!=SQLITE_OK ) break;
        rc = sqlite3BtreeNext(pCur, 0);
      }
      만약( rc==SQLITE_DONE ){
        rc = SQLITE_OK;
        assert( res!=0 );
      }
      만약( nStep<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) break;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      nStep = -1;
      rc = sqlite3BtreeFirst(pCur, &res);
    }
  }

  *pRes = res;
  반환 rc;
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
정수 sqlite3NotPureFunc(sqlite3_context *pCtx){
  const VdbeOp *pOp;
#ifdef SQLITE_ENABLE_STAT4
  만약( pCtx->pVdbe==0 ) 반환 1;
#endif
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  만약( pOp->opcode==OP_PureFunc ){
    const char *zContext;
    char *zMsg;
    만약( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }아니면 만약( pOp->p5 & NC_GenCol ){
      zContext = "a generated column";
    }아니면{
      zContext = "an index";
    }
    zMsg = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_result_error(pCtx, zMsg, -1);
    sqlite3_free(zMsg);
    반환 0;
  }
  반환 1;
}
#endif /* SQLITE_OMIT_DATETIME_FUNCS */

#만약 defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
정수 sqlite3CursorRangeHintExprCheck(Walker *pWalker, Expr *pExpr){
  만약( pExpr->op==TK_REGISTER ){
    assert( (pWalker->u.aMem[pExpr->iTable].flags & MEM_Undefined)==0 );
  }
  반환 WRC_Continue;
}
#endif /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
void sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  만약( pVtab->zErrMsg ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = sqlite3DbStrDup(db, pVtab->zErrMsg);
    sqlite3_free(pVtab->zErrMsg);
    pVtab->zErrMsg = 0;
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
static void vdbeFreeUnpacked(sqlite3 *db, 정수 nField, UnpackedRecord *p){
  assert( db!=0 );
  만약( p ){
    정수 i;
    위해(i=0; i<nField; i++){
      Mem *pMem = &p->aMem[i];
      만약( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
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
void sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  정수 op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  const char *zDb,                /* Database name */
  Table *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  정수 iReg,                       /* Register for new.* record */
  정수 iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  const char *zTbl = pTab->zName;
#ifdef SQLITE_DEBUG
  정수 nRealCol;
  만약( pTab->tabFlags & TF_WithoutRowid ){
    nRealCol = sqlite3PrimaryKeyIndex(pTab)->nColumn;
  }아니면 만약( pTab->tabFlags & TF_HasVirtual ){
    nRealCol = pTab->nNVCol;
  }아니면{
    nRealCol = pTab->nCol;
  }
#endif

  assert( db->pPreUpdate==0 );
  memset(&preupdate, 0, sizeof(PreUpdate));
  만약( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3PrimaryKeyIndex(pTab);
  }아니면{
    만약( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }아니면{
      iKey2 = iKey1;
    }
  }

  assert( pCsr!=0 );
  assert( pCsr->eCurType==CURTYPE_BTREE );
  assert( pCsr->nField==nRealCol
       || (pCsr->nField==nRealCol+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (KeyInfo*)&preupdate.uKey;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->nKeyField = pTab->nCol;
  preupdate.pKeyinfo->aSortFlags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.aRecord);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->nKeyField+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->nKeyField+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  만약( preupdate.aNew ){
    정수 i;
    위해(i=0; i<pCsr->nField; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  만약( preupdate.apDflt ){
    정수 i;
    위해(i=0; i<pTab->nCol; i++){
      sqlite3ValueFree(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

#ifdef SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
const char *sqlite3VdbeFuncName(const sqlite3_context *pCtx){
  assert( pCtx!=0 );
  assert( pCtx->pFunc!=0 );
  반환 pCtx->pFunc->zName;
}
#endif /* SQLITE_ENABLE_PERCENTILE */
