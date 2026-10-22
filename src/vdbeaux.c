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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/* Forward references */
ساكن فراغ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
ساكن فراغ vdbeFreeOpArray(sqlite3 *, Op *, صحيح);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, حجم(Vdbe) );
  إذا( p==0 ) أرجع 0;
  memset(&p->aOp, 0, حجم(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  إذا( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  أكد( pParse->aLabel==0 );
  أكد( pParse->nLabel==0 );
  أكد( p->nOpAlloc==0 );
  أكد( pParse->szOpAlloc==0 );
  sqlite3_Vdbe_جمع_کړه_Op2(p, OP_Init, 0, 1);
  أرجع p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  أرجع p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
فراغ sqlite3VdbeError(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_پیغام);
  va_start(ap, zFormat);
  p->z_Err_پیغام = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
فراغ sqlite3_Vdbe_سېټ_Sql(Vdbe *p, ثابت حرف *z, صحيح n, u8 prepFlags){
  إذا( p==0 ) أرجع;
  p->prepFlags = prepFlags;
  إذا( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  أكد( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
فراغ sqlite3_Vdbe_جمع_کړه_Dblquote_Str(sqlite3 *db, Vdbe *p, ثابت حرف *z){
  إذا( p ){
    صحيح n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            حجم(*pStr)+n+1-حجم(pStr->z));
    إذا( pStr ){
      pStr->p_بل_Str = p->pDblStr;
      p->pDblStr = pStr;
      حافظه_نقل(pStr->z, z, n+1);
    }
  }
}
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
صحيح sqlite3_Vdbe_کاروي_Double_Quoted_String(
  Vdbe *pVdbe,            /* The prepared statement */
  ثابت حرف *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  أكد( zId!=0 );
  إذا( pVdbe->pDblStr==0 ) أرجع 0;
  لكل(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_بل_Str){
    إذا( strcmp(zId, pStr->z)==0 ) أرجع 1;
  }
  أرجع 0;
}
#د_که_پای

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
فراغ sqlite3_Vdbe_بدلون(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  حرف *zTmp;
  أكد( pA->db==pB->db );
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
#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#د_که_پای
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  حافظه_نقل(pB->aCounter, pA->aCounter, حجم(pB->aCounter));
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
ساكن صحيح growOpArray(Vdbe *v, صحيح nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#که_تعریف_شوی SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#وإلا
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/حجم(Op)));
  ناکارول_شوی_PARAMETER(nOp);
#د_که_پای

  /* Ensure that the size of a VDBE does not grow too large */
  إذا( nNew > p->db->a_حد[SQLITE_حد_VDBE_OP] ){
    sqlite3OomFault(p->db);
    أرجع SQLITE_NOMEM;
  }

  أكد( nOp<=(صحيح)(1024/حجم(Op)) );
  أكد( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*حجم(Op));
  إذا( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_اندازه(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/حجم(Op);
    v->aOp = pNew;
  }
  أرجع (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#که_تعریف_شوی SQLITE_DEBUG
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
ساكن فراغ test_addop_breakpoint(صحيح pc, Op *pOp){
  ساكن u64 n = 0;
  (فراغ)pc;
  (فراغ)pOp;
  n++;
  إذا( n==تر_ټولو_لوی_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#د_که_پای

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
ساكن SQLITE_NOINLINE صحيح growOp3(Vdbe *p, صحيح op, صحيح p1, صحيح p2, صحيح p3){
  أكد( p->nOpAlloc<=p->nOp );
  إذا( growOpArray(p, 1) ) أرجع 1;
  أكد( p->nOpAlloc>p->nOp );
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, op, p1, p2, p3);
}
ساكن SQLITE_NOINLINE صحيح جمع_کړه_Op4_Int_Slow(
  Vdbe *p,            /* Add the opcode to this VM */
  صحيح op,             /* The new opcode */
  صحيح p1,             /* The P1 operand */
  صحيح p2,             /* The P2 operand */
  صحيح p3,             /* The P3 operand */
  صحيح p4              /* The P4 operand as an integer */
){
  صحيح addr = sqlite3_Vdbe_جمع_کړه_Op3(p, op, p1, p2, p3);
  إذا( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  أرجع addr;
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
صحيح sqlite3_Vdbe_جمع_کړه_Op0(Vdbe *p, صحيح op){
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, op, 0, 0, 0);
}
صحيح sqlite3_Vdbe_جمع_کړه_Op1(Vdbe *p, صحيح op, صحيح p1){
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, op, p1, 0, 0);
}
صحيح sqlite3_Vdbe_جمع_کړه_Op2(Vdbe *p, صحيح op, صحيح p1, صحيح p2){
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, op, p1, p2, 0);
}
صحيح sqlite3_Vdbe_جمع_کړه_Op3(Vdbe *p, صحيح op, صحيح p1, صحيح p2, صحيح p3){
  صحيح i;
  VdbeOp *pOp;

  i = p->nOp;
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( op>=0 && op<0xff );
  إذا( p->nOpAlloc<=i ){
    أرجع growOp3(p, op, p1, p2, p3);
  }
  أكد( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  أكد( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#د_که_پای
#إذا defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_دوره = 0;
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
  إذا( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#د_که_پای
#که_تعریف_شوی SQLITE_VDBE_COVERAGE
  pOp->i_Src_کرښه = 0;
#د_که_پای
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  أرجع i;
}
صحيح sqlite3_Vdbe_جمع_کړه_Op4_Int(
  Vdbe *p,            /* Add the opcode to this VM */
  صحيح op,             /* The new opcode */
  صحيح p1,             /* The P1 operand */
  صحيح p2,             /* The P2 operand */
  صحيح p3,             /* The P3 operand */
  صحيح p4              /* The P4 operand as an integer */
){
  صحيح i;
  VdbeOp *pOp;

  i = p->nOp;
  إذا( p->nOpAlloc<=i ){
    أرجع جمع_کړه_Op4_Int_Slow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  أكد( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#د_که_پای
#إذا defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_دوره = 0;
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
  إذا( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#د_که_پای
#که_تعریف_شوی SQLITE_VDBE_COVERAGE
  pOp->i_Src_کرښه = 0;
#د_که_پای
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  أرجع i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
صحيح sqlite3_Vdbe_جمع_کړه_Int64(Vdbe *p, صحيح iDest, i64 iVal){
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
صحيح sqlite3_Vdbe_جمع_کړه_Double(Vdbe *p, صحيح iDest, مزدوج rVal){
  i64 iVal;
  حافظه_نقل(&iVal,&rVal,8);
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
صحيح sqlite3VdbeGoto(Vdbe *p, صحيح iDest){
  أرجع sqlite3_Vdbe_جمع_کړه_Op3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
صحيح sqlite3_Vdbe_بارول_String(Vdbe *p, صحيح iDest, ثابت حرف *zStr){
  أرجع sqlite3_Vdbe_جمع_کړه_Op4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
فراغ sqlite3_Vdbe_Multi_بارول(Vdbe *p, صحيح iDest, ثابت حرف *zTypes, ...){
  va_list ap;
  صحيح i;
  حرف c;
  va_start(ap, zTypes);
  لكل(i=0; (c = zTypes[i])!=0; i++){
    إذا( c=='s' ){
      ثابت حرف *z = va_arg(ap, ثابت حرف*);
      sqlite3_Vdbe_جمع_کړه_Op4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }وإلا إذا( c=='i' ){
      sqlite3_Vdbe_جمع_کړه_Op2(p, OP_Integer, va_arg(ap, صحيح), iDest+i);
    }وإلا{
      اذهب skip_op_resultrow;
    }
  }
  sqlite3_Vdbe_جمع_کړه_Op2(p, OP_پایله_لیکه, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
صحيح sqlite3_Vdbe_جمع_کړه_Op4(
  Vdbe *p,            /* Add the opcode to this VM */
  صحيح op,             /* The new opcode */
  صحيح p1,             /* The P1 operand */
  صحيح p2,             /* The P2 operand */
  صحيح p3,             /* The P3 operand */
  ثابت حرف *zP4,    /* The P4 operand */
  صحيح p4type          /* P4 operand type */
){
  صحيح addr = sqlite3_Vdbe_جمع_کړه_Op3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  أرجع addr;
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
صحيح sqlite3_Vdbe_جمع_کړه_Function_Call(
  Parse *pParse,        /* Parsing context */
  صحيح p1,               /* Constant argument mask */
  صحيح p2,               /* First argument register */
  صحيح p3,               /* Register into which results are written */
  صحيح nArg,             /* Number of argument */
  ثابت FuncDef *pFunc, /* The function to be invoked */
  صحيح eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  صحيح addr;
  sqlite3_context *pCtx;
  أكد( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  إذا( pCtx==0 ){
    أكد( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    أرجع 0;
  }
  pCtx->p_بهر = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3_Vdbe_جمع_کړه_Op4(v, eCallCtx ? OP_خالص_Func : OP_Function,
                           p1, p2, p3, (حرف*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  أرجع addr;
}

#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
صحيح sqlite3_Vdbe_Explain_پلار(Parse *pParse){
  VdbeOp *pOp;
  إذا( pParse->addrExplain==0 ) أرجع 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  أرجع pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#إذا defined(SQLITE_DEBUG)
فراغ sqlite3ExplainBreakpoint(ثابت حرف *z1, ثابت حرف *z2){
  (فراغ)z1;
  (فراغ)z2;
}
#د_که_پای

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
صحيح sqlite3VdbeExplain(Parse *pParse, u8 b_ټېل, ثابت حرف *zFmt, ...){
  صحيح addr = 0;
#إذا !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  إذا( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#د_که_پای
  {
    حرف *z_پیغام;
    Vdbe *v;
    va_list ap;
    صحيح iThis;
    va_start(ap, zFmt);
    z_پیغام = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_پیغام, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_ټېل?"PUSH":"", sqlite3_Vdbe_Get_وروستی_Op(v)->p4.z);
    إذا( b_ټېل){
      pParse->addrExplain = iThis;
    }
    sqlite3_Vdbe_لټون_حالت(v, iThis, -1, -1, 0, 0);
  }
  أرجع addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
فراغ sqlite3_Vdbe_Explain_راوباسه(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3_Vdbe_Explain_پلار(pParse);
}
#د_که_پای /* SQLITE_OMIT_EXPLAIN */

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
فراغ sqlite3_Vdbe_جمع_کړه_Parse_Schema_Op(Vdbe *p, صحيح iDb, حرف *zWhere, u16 p5){
  صحيح j;
  أكد( p5==0 || zWhere==0 );
  sqlite3_Vdbe_جمع_کړه_Op4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  لكل(j=0; j<p->db->nDb; j++) sqlite3_Vdbe_کاروي_Btree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
فراغ sqlite3_Vdbe_پای_ټکی_Coroutine(Vdbe *v, صحيح regYield){
  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_پای_ټکی_Coroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_لنډمهاله_Reg = 0;
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
صحيح sqlite3VdbeMakeLabel(Parse *pParse){
  أرجع --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
ساكن SQLITE_NOINLINE فراغ resizeResolveLabel(Parse *p, Vdbe *v, صحيح j){
  صحيح n_New_اندازه = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_اندازه*حجم(p->aLabel[0]));
  إذا( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }وإلا{
#که_تعریف_شوی SQLITE_DEBUG
    صحيح i;
    لكل(i=p->nLabelAlloc; i<n_New_اندازه; i++) p->aLabel[i] = -1;
#د_که_پای
    إذا( n_New_اندازه>=100 && (n_New_اندازه/100)>(p->nLabelAlloc/100) ){
      sqlite3_پرمختګ_Check(p);
    }
    p->nLabelAlloc = n_New_اندازه;
    p->aLabel[j] = v->nOp;
  }
}
فراغ sqlite3VdbeResolveLabel(Vdbe *v, صحيح x){
  Parse *p = v->pParse;
  صحيح j = ADDR(x);
  أكد( v->eVdbeState==VDBE_INIT_STATE );
  أكد( j<-p->nLabel );
  أكد( j>=0 );
#که_تعریف_شوی SQLITE_DEBUG
  إذا( p->db->flags & SQLITE_VdbeAddopTrace ){
    چاپ_کړه("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#د_که_پای
  إذا( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }وإلا{
    أكد( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
فراغ sqlite3_Vdbe_پرلپسې_Only_Once(Vdbe *p){
  sqlite3_Vdbe_جمع_کړه_Op2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
فراغ sqlite3VdbeReusable(Vdbe *p){
  صحيح i;
  لكل(i=1; ALWAYS(i<p->nOp); i++){
    إذا( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      اكسر;
    }
  }
}

#که_تعریف_شوی SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
تعريفنوع بنية VdbeOpIter VdbeOpIter;
بنية VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  Sub_پروګرام **apSub;        /* Array of subprograms */
  صحيح nSub;                  /* Number of entries in apSub */
  صحيح iAddr;                 /* Address of next instruction to return */
  صحيح iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
ساكن Op *op_Iter_بل(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  صحيح nOp;

  إذا( p->iSub<=p->nSub ){

    إذا( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }وإلا{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    أكد( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    إذا( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    إذا( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*حجم(Sub_پروګرام*);
      صحيح j;
      لكل(j=0; j<p->nSub; j++){
        إذا( p->apSub[j]==pRet->p4.p_پروګرام ) اكسر;
      }
      إذا( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        إذا( !p->apSub ){
          pRet = 0;
        }وإلا{
          p->apSub[p->nSub++] = pRet->p4.p_پروګرام;
        }
      }
    }
  }

  أرجع pRet;
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
صحيح sqlite3VdbeAssertMayAbort(Vdbe *v, صحيح mayAbort){
  صحيح hasAbort = 0;
  صحيح hasFkCounter = 0;
  صحيح has_Create_جدول = 0;
  صحيح has_Create_شاخص = 0;
  صحيح hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  إذا( v==0 ) أرجع 0;
  memset(&sIter, 0, حجم(sIter));
  sIter.v = v;

  طالما( (pOp = op_Iter_بل(&sIter))!=0 ){
    صحيح opcode = pOp->opcode;
    إذا( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_خالص_Func
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      اكسر;
    }
    إذا( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) has_Create_جدول = 1;
    إذا( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      إذا( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_شاخص = 1;
      إذا( opcode==OP_Clear ) has_Create_شاخص = 1;
    }
    إذا( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#که_نه_تعریف_شوی SQLITE_OMIT_FOREIGN_کلی
    إذا( opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1 ){
      hasFkCounter = 1;
    }
#د_که_پای
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  أرجع ( v->db->mallocFailed || hasAbort==mayAbort || hasFkCounter
        || (has_Create_جدول && hasInitCoroutine) || has_Create_شاخص
  );
}
#د_که_پای /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#که_تعریف_شوی SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
فراغ sqlite3VdbeIncrWriteCounter(Vdbe *p, VdbeCursor *pC){
  إذا( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
فراغ sqlite3VdbeAssertAbortable(Vdbe *p){
  أكد( p->nWrite==0 || p->کاروي_Stmt_Journal );
}
#د_که_پای

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
ساكن فراغ resolve_P2_ارزښتونه(Vdbe *p, صحيح *p_Max_Vtab_دلیلونه){
  صحيح n_Max_Vtab_دلیلونه = *p_Max_Vtab_دلیلونه;
  Op *pOp;
  Parse *pParse = p->pParse;
  صحيح *aLabel = pParse->aLabel;

  أكد( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  أكد( p->aOp[0].opcode==OP_Init );
  طالما( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    إذا( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      بدل( pOp->opcode ){
        حالة OP_Transaction: {
          إذا( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        حالة OP_AutoCommit:
        حالة OP_Savepoint: {
          p->bIsReader = 1;
          اكسر;
        }
#که_نه_تعریف_شوی SQLITE_OMIT_WAL
        حالة OP_Checkpoint:
#د_که_پای
        حالة OP_Vacuum:
        حالة OP_Journal_ډېرپېښ: {
          p->readOnly = 0;
          p->bIsReader = 1;
          اكسر;
        }
        حالة OP_Init: {
          أكد( pOp->p2>=0 );
          اذهب resolve_p2_ارزښتونه_loop_exit;
        }
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
        حالة OP_VUpdate: {
          إذا( pOp->p2>n_Max_Vtab_دلیلونه ) n_Max_Vtab_دلیلونه = pOp->p2;
          اكسر;
        }
        حالة OP_VFilter: {
          صحيح n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          أكد( (pOp - p->aOp) >= 3 );
          أكد( pOp[-1].opcode==OP_Integer );
          أكد( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          إذا( n>n_Max_Vtab_دلیلونه ) n_Max_Vtab_دلیلونه = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#د_که_پای
        افتراضي: {
          إذا( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            أكد( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            أكد( ADDR(pOp->p2)<-pParse->nLabel );
            أكد( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          أكد( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          أكد( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          اكسر;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      أكد( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    أكد( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_ارزښتونه_loop_exit:
  إذا( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *p_Max_Vtab_دلیلونه = n_Max_Vtab_دلیلونه;
  أكد( p->bIsReader!=0 || Db_ماسک_All_Zero(p->btree_ماسک) );
}

#که_تعریف_شوی SQLITE_DEBUG
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
فراغ sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  صحيح i_لومړی,       /* First opcode of the subroutine */
  صحيح i_وروستی,        /* Last opcode of the subroutine */
  صحيح iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  صحيح i;
  sqlite3_str *pErr = 0;
  أكد( v!=0 );
  pParse = v->pParse;
  أكد( pParse!=0 );
  إذا( pParse->nErr ) أرجع;
  أكد( i_وروستی>=i_لومړی );
  أكد( i_وروستی<v->nOp );
  pOp = &v->aOp[i_لومړی];
  لكل(i=i_لومړی; i<=i_وروستی; i++, pOp++){
    إذا( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      صحيح iDest = pOp->p2;   /* Jump destination */
      إذا( iDest==0 ) استمر;
      إذا( pOp->opcode==OP_Gosub ) استمر;
      إذا( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        استمر;
      }
      إذا( iDest<0 ){
        صحيح j = ADDR(iDest);
        أكد( j>=0 );
        إذا( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          استمر;
        }
        iDest = pParse->aLabel[j];
      }
      إذا( iDest<i_لومړی || iDest>i_وروستی ){
        صحيح j = iDest;
        لكل(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          إذا( pX->opcode==OP_Return ){
            إذا( pX->p1==iRetReg ) اكسر;
            استمر;
          }
          إذا( pX->opcode==OP_Noop ) استمر;
          إذا( pX->opcode==OP_Explain ) استمر;
          إذا( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }وإلا{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, i_لومړی, i_وروستی);
          اكسر;
        }
      }
    }
  }
  إذا( pErr ){
    حرف *zErr = sqlite3_str_پای(pErr);
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Halt, SQLITE_دننی, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#د_که_پای /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
صحيح sqlite3VdbeCurrentAddr(Vdbe *p){
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أرجع p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
فراغ sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, صحيح N){
  أكد( p->nOp + N <= p->nOpAlloc );
}
#د_که_پای

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
فراغ sqlite3_Vdbe_Verify_No_پایله_لیکه(Vdbe *p){
  صحيح i;
  لكل(i=0; i<p->nOp; i++){
    أكد( p->aOp[i].opcode!=OP_پایله_لیکه );
  }
}
#د_که_پای

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#إذا defined(SQLITE_DEBUG)
فراغ sqlite3VdbeVerifyAbortable(Vdbe *p, صحيح onError){
  إذا( onError==OE_Abort ) sqlite3_Vdbe_جمع_کړه_Op0(p, OP_Abortable);
}
#د_که_پای

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, صحيح *pnOp, صحيح *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  أكد( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  أكد( Db_ماسک_All_Zero(p->btree_ماسک) );

  resolve_P2_ارزښتونه(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  أرجع aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3_Vdbe_جمع_کړه_Op_List(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  صحيح nOp,                     /* Number of opcodes to add */
  VdbeOpList ثابت *aOp,       /* The opcodes to be added */
  صحيح iLineno                  /* Source-file line number of first opcode */
){
  صحيح i;
  VdbeOp *p_بهر, *p_لومړی;
  أكد( nOp>0 );
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  إذا( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    أرجع 0;
  }
  p_لومړی = p_بهر = &p->aOp[p->nOp];
  لكل(i=0; i<nOp; i++, aOp++, p_بهر++){
    p_بهر->opcode = aOp->opcode;
    p_بهر->p1 = aOp->p1;
    p_بهر->p2 = aOp->p2;
    أكد( aOp->p2>=0 );
    إذا( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      p_بهر->p2 += p->nOp;
    }
    p_بهر->p3 = aOp->p3;
    p_بهر->p4type = P4_NOTUSED;
    p_بهر->p4.p = 0;
    p_بهر->p5 = 0;
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
    p_بهر->zComment = 0;
#د_که_پای
#که_تعریف_شوی SQLITE_VDBE_COVERAGE
    p_بهر->i_Src_کرښه = iLineno+i;
#وإلا
    (فراغ)iLineno;
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
    إذا( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#د_که_پای
  }
  p->nOp += nOp;
  أرجع p_لومړی;
}

#إذا defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
فراغ sqlite3_Vdbe_لټون_حالت(
  Vdbe *p,                        /* VM to add scanstatus() to */
  صحيح addrExplain,                /* Address of OP_Explain (or 0) */
  صحيح addrLoop,                   /* Address of loop counter */
  صحيح addr_لیدنه,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  ثابت حرف *zName               /* Name of table or index being scanned */
){
  إذا( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->n_لټون) * حجم(لټون_حالت);
    لټون_حالت *aNew;
    aNew = (لټون_حالت*)sqlite3DbRealloc(p->db, p->a_لټون, nByte);
    إذا( aNew ){
      لټون_حالت *pNew = &aNew[p->n_لټون++];
      memset(pNew, 0, حجم(لټون_حالت));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addr_لیدنه = addr_لیدنه;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->a_لټون = aNew;
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
فراغ sqlite3_Vdbe_لټون_حالت_Range(
  Vdbe *p,
  صحيح addrExplain,
  صحيح addr_پیل,
  صحيح addr_پای_ټکی
){
  إذا( IS_STMT_SCANSTATUS(p->db) ){
    لټون_حالت *p_لټون = 0;
    صحيح ii;
    لكل(ii=p->n_لټون-1; ii>=0; ii--){
      p_لټون = &p->a_لټون[ii];
      إذا( p_لټون->addrExplain==addrExplain ) اكسر;
      p_لټون = 0;
    }
    إذا( p_لټون ){
      إذا( addr_پای_ټکی<0 ) addr_پای_ټکی = sqlite3VdbeCurrentAddr(p)-1;
      لكل(ii=0; ii<Array_اندازه(p_لټون->aAddrRange); ii+=2){
        إذا( p_لټون->aAddrRange[ii]==0 ){
          p_لټون->aAddrRange[ii] = addr_پیل;
          p_لټون->aAddrRange[ii+1] = addr_پای_ټکی;
          اكسر;
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
فراغ sqlite3_Vdbe_لټون_حالت_Counters(
  Vdbe *p,
  صحيح addrExplain,
  صحيح addrLoop,
  صحيح addr_لیدنه
){
  إذا( IS_STMT_SCANSTATUS(p->db) ){
    لټون_حالت *p_لټون = 0;
    صحيح ii;
    لكل(ii=p->n_لټون-1; ii>=0; ii--){
      p_لټون = &p->a_لټون[ii];
      إذا( p_لټون->addrExplain==addrExplain ) اكسر;
      p_لټون = 0;
    }
    إذا( p_لټون ){
      إذا( addrLoop>0 ) p_لټون->addrLoop = addrLoop;
      إذا( addr_لیدنه>0 ) p_لټون->addr_لیدنه = addr_لیدنه;
    }
  }
}
#د_که_پای /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
فراغ sqlite3VdbeChangeOpcode(Vdbe *p, صحيح addr, u8 iNewOpcode){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
فراغ sqlite3VdbeChangeP1(Vdbe *p, صحيح addr, صحيح val){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
فراغ sqlite3VdbeChangeP2(Vdbe *p, صحيح addr, صحيح val){
  أكد( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
فراغ sqlite3VdbeChangeP3(Vdbe *p, صحيح addr, صحيح val){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
فراغ sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  أكد( p->nOp>0 || p->db->mallocFailed );
  إذا( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
فراغ sqlite3_Vdbe_Typeof_ستنه(Vdbe *p, صحيح iDest){
  VdbeOp *pOp = sqlite3_Vdbe_Get_وروستی_Op(p);
#که_تعریف_شوی SQLITE_DEBUG
  طالما( pOp->opcode==OP_ReleaseReg ) pOp--;
#د_که_پای
  إذا( pOp->p3==iDest && pOp->opcode==OP_ستنه ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
فراغ sqlite3_Vdbe_Jump_دلته(Vdbe *p, صحيح addr){
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
فراغ sqlite3_Vdbe_Jump_دلته_Or_راوباسه_Inst(Vdbe *p, صحيح addr){
  إذا( addr==p->nOp-1 ){
    أكد( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    أكد( p->aOp[addr].p4type==0 );
#که_تعریف_شوی SQLITE_VDBE_COVERAGE
    sqlite3_Vdbe_Get_وروستی_Op(p)->i_Src_کرښه = 0;  /* Erase VdbeCoverage() macros */
#د_که_پای
    p->nOp--;
  }وإلا{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
ساكن فراغ freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  أكد( db!=0 );
  إذا( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
ساكن SQLITE_NOINLINE فراغ freeP4Mem(sqlite3 *db, Mem *p){
  إذا( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
ساكن SQLITE_NOINLINE فراغ freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  أكد( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
ساكن فراغ freeP4(sqlite3 *db, صحيح p4type, فراغ *p4){
  أكد( db );
  بدل( p4type ){
    حالة P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      اكسر;
    }
    حالة P4_DYNAMIC:
    حالة P4_INTARRAY: {
      إذا( p4 ) sqlite3DbNNFreeNN(db, p4);
      اكسر;
    }
    حالة P4_KEYINFO: {
      إذا( db->pn_بایټونه_Freed==0 ) sqlite3_کلی_Info_Unref((کلی_Info*)p4);
      اكسر;
    }
#که_تعریف_شوی SQLITE_ENABLE_CURSOR_HINTS
    حالة P4_بیان: {
      sqlite3_بیان_Delete(db, (بیان*)p4);
      اكسر;
    }
#د_که_پای
    حالة P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      اكسر;
    }
    حالة P4_MEM: {
      إذا( db->pn_بایټونه_Freed==0 ){
        sqlite3_ارزښت_Free((sqlite3_ارزښت*)p4);
      }وإلا{
        freeP4Mem(db, (Mem*)p4);
      }
      اكسر;
    }
    حالة P4_VTAB : {
      إذا( db->pn_بایټونه_Freed==0 ) sqlite3VtabUnlock((VTable *)p4);
      اكسر;
    }
    حالة P4_TABLEREF: {
      إذا( db->pn_بایټونه_Freed==0 ) sqlite3_Delete_جدول(db, (جدول*)p4);
      اكسر;
    }
    حالة P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      اكسر;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
ساكن فراغ vdbeFreeOpArray(sqlite3 *db, Op *aOp, صحيح nOp){
  أكد( nOp>=0 );
  أكد( db!=0 );
  إذا( aOp ){
    Op *pOp = &aOp[nOp-1];
    طالما(1){  /* Exit via break */
      إذا( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#د_که_پای    
      إذا( pOp==aOp ) اكسر;
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
فراغ sqlite3_Vdbe_Link_Sub_پروګرام(Vdbe *pVdbe, Sub_پروګرام *p){
  p->p_بل = pVdbe->p_پروګرام;
  pVdbe->p_پروګرام = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
صحيح sqlite3_Vdbe_Has_Sub_پروګرام(Vdbe *pVdbe){
  أرجع pVdbe->p_پروګرام!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
صحيح sqlite3_Vdbe_Change_منزل_Noop(Vdbe *p, صحيح addr){
  VdbeOp *pOp;
  إذا( p->db->mallocFailed ) أرجع 0;
  أكد( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  أرجع 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
صحيح sqlite3_Vdbe_Delete_مخکینی_Opcode(Vdbe *p, u8 op){
  إذا( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    أرجع sqlite3_Vdbe_Change_منزل_Noop(p, p->nOp-1);
  }وإلا{
    أرجع 0;
  }
}

#که_تعریف_شوی SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
فراغ sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  صحيح i_لومړی,          /* Index of first register to be released */
  صحيح N,               /* Number of registers to release */
  u32 ماسک,            /* Mask of registers to NOT release */
  صحيح bUndefine        /* If true, mark registers as undefined */
){
  إذا( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) أرجع;
  أكد( pParse->pVdbe );
  أكد( i_لومړی>=1 );
  أكد( i_لومړی+N-1<=pParse->nMem );
  إذا( N<=31 && ماسک!=0 ){
    طالما( N>0 && (ماسک&1)!=0 ){
      ماسک >>= 1;
      i_لومړی++;
      N--;
    }
    طالما( N>0 && N<=32 && (ماسک & MASKBIT32(N-1))!=0 ){
      ماسک &= ~MASKBIT32(N-1);
      N--;
    }
  }
  إذا( N>0 ){
    sqlite3_Vdbe_جمع_کړه_Op3(pParse->pVdbe, OP_ReleaseReg, i_لومړی, N, *(صحيح*)&ماسک);
    إذا( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#د_که_پای /* SQLITE_DEBUG */

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
ساكن فراغ SQLITE_NOINLINE vdbe_Change_P4_ډک(
  Vdbe *p,
  Op *pOp,
  ثابت حرف *zP4,
  صحيح n
){
  إذا( pOp->p4type ){
    أكد( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  إذا( n<0 ){
    sqlite3VdbeChangeP4(p, (صحيح)(pOp - p->aOp), zP4, n);
  }وإلا{
    إذا( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
فراغ sqlite3VdbeChangeP4(Vdbe *p, صحيح addr, ثابت حرف *zP4, صحيح n){
  Op *pOp;
  sqlite3 *db;
  أكد( p!=0 );
  db = p->db;
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( p->aOp!=0 || db->mallocFailed );
  إذا( db->mallocFailed ){
    إذا( n!=P4_VTAB ) freeP4(db, n, (فراغ*)*(حرف**)&zP4);
    أرجع;
  }
  أكد( p->nOp>0 );
  أكد( addr<p->nOp );
  إذا( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  إذا( n>=0 || pOp->p4type ){
    vdbe_Change_P4_ډک(p, pOp, zP4, n);
    أرجع;
  }
  إذا( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_منزل_INT(zP4);
    pOp->p4type = P4_INT32;
  }وإلا إذا( zP4!=0 ){
    أكد( n<0 );
    pOp->p4.p = (فراغ*)zP4;
    pOp->p4type = (نښهدار حرف)n;
    إذا( n==P4_VTAB ) sqlite3_Vtab_قلف((VTable*)zP4);
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
فراغ sqlite3VdbeAppendP4(Vdbe *p, فراغ *pP4, صحيح n){
  VdbeOp *pOp;
  أكد( n!=P4_INT32 && n!=P4_VTAB );
  أكد( n<=0 );
  إذا( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }وإلا{
    أكد( pP4!=0 || n==P4_DYNAMIC );
    أكد( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    أكد( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
فراغ sqlite3_Vdbe_سېټ_P4_کلی_Info(Parse *pParse, شاخص *pIdx){
  Vdbe *v = pParse->pVdbe;
  کلی_Info *p_کلی_Info;
  أكد( v!=0 );
  أكد( pIdx!=0 );
  p_کلی_Info = sqlite3_کلی_Info_د_شاخص(pParse, pIdx);
  إذا( p_کلی_Info ) sqlite3VdbeAppendP4(v, p_کلی_Info, P4_KEYINFO);
}

#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
ساكن فراغ vdbeVComment(Vdbe *p, ثابت حرف *zFormat, va_list ap){
  أكد( p->nOp>0 || p->aOp==0 );
  أكد( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  إذا( p->nOp ){
    أكد( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
فراغ sqlite3VdbeComment(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  إذا( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
فراغ sqlite3VdbeNoopComment(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  إذا( p ){
    sqlite3_Vdbe_جمع_کړه_Op0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#د_که_پای  /* NDEBUG */

#که_تعریف_شوی SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
فراغ sqlite3_Vdbe_سېټ_کرښه_شمېره(Vdbe *v, صحيح i_کرښه){
  sqlite3_Vdbe_Get_وروستی_Op(v)->i_Src_کرښه = i_کرښه;
}
#د_که_پای /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, صحيح addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  ساكن VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  إذا( p->db->mallocFailed ){
    أرجع (VdbeOp*)&dummy;
  }وإلا{
    أرجع &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3_Vdbe_Get_وروستی_Op(Vdbe *p){
  أرجع sqlite3VdbeGetOp(p, p->nOp - 1);
}

#إذا defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
ساكن صحيح translateP(حرف c, ثابت Op *pOp){
  إذا( c=='1' ) أرجع pOp->p1;
  إذا( c=='2' ) أرجع pOp->p2;
  إذا( c=='3' ) أرجع pOp->p3;
  إذا( c=='4' ) أرجع pOp->p4.i;
  أرجع pOp->p5;
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
حرف *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  ثابت Op *pOp,     /* The opcode to be commented */
  ثابت حرف *zP4    /* Previously obtained value for P4 */
){
  ثابت حرف *zOpName;
  ثابت حرف *zSynopsis;
  صحيح nOpName;
  صحيح ii;
  حرف zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_اوږدتیا);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  إذا( zOpName[nOpName+1] ){
    صحيح کتل_شوی_Com = 0;
    حرف c;
    zSynopsis = zOpName + nOpName + 1;
    إذا( مزي_پرتله(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(حجم(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    لكل(ii=0; (c = zSynopsis[ii])!=0; ii++){
      إذا( c=='P' ){
        c = zSynopsis[++ii];
        إذا( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }وإلا إذا( c=='X' ){
          إذا( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            کتل_شوی_Com = 1;
            اكسر;
          }
        }وإلا إذا( مزي_پرتله(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_منزل_64(pOp->p1,pOp->p3));
          ii += 4;
#که_تعریف_شوی SQLITE_ENABLE_ستنه_کارول_شوی_ماسک
        }وإلا إذا( مزي_پرتله(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_منزل_64(pOp->p2,pOp->p3));
          ii += 4;
#د_که_پای
        }وإلا إذا( مزي_پرتله(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_منزل_64(pOp->p1,pOp->p3);
          مزدوج r;
          حافظه_نقل(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }وإلا{
          صحيح v1 = translateP(c, pOp);
          صحيح v2;
          إذا( مزي_پرتله(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            إذا( مزي_پرتله(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            إذا( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }وإلا{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }وإلا إذا( مزي_پرتله(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            إذا( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }وإلا إذا( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }وإلا إذا( x.accError==0 ){
              أكد( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }وإلا{
            sqlite3_str_appendf(&x, "%d", v1);
            إذا( مزي_پرتله(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }وإلا{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    إذا( !کتل_شوی_Com && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }وإلا إذا( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  إذا( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  أرجع sqlite3_Str_Accum_پای(&x);
}
#د_که_پای /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#إذا VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
ساكن فراغ display_P4_بیان(StrAccum *p, بیان *p_بیان){
  ثابت حرف *zOp = 0;
  بدل( p_بیان->op ){
    حالة TK_STRING:
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_str_appendf(p, "%Q", p_بیان->u.zToken);
      اكسر;
    حالة TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_بیان->u.i_ارزښت);
      اكسر;
    حالة TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      اكسر;
    حالة TK_ثبتوونکی: {
      sqlite3_str_appendf(p, "r[%d]", p_بیان->i_جدول);
      اكسر;
    }
    حالة TK_ستنه: {
      إذا( p_بیان->i_ستنه<0 ){
        sqlite3_str_appendf(p, "rowid");
      }وإلا{
        sqlite3_str_appendf(p, "c%d", (صحيح)p_بیان->i_ستنه);
      }
      اكسر;
    }
    حالة TK_LT:      zOp = "LT";      اكسر;
    حالة TK_LE:      zOp = "LE";      اكسر;
    حالة TK_GT:      zOp = "GT";      اكسر;
    حالة TK_GE:      zOp = "GE";      اكسر;
    حالة TK_NE:      zOp = "NE";      اكسر;
    حالة TK_EQ:      zOp = "EQ";      اكسر;
    حالة TK_IS:      zOp = "IS";      اكسر;
    حالة TK_ISNOT:   zOp = "ISNOT";   اكسر;
    حالة TK_AND:     zOp = "AND";     اكسر;
    حالة TK_OR:      zOp = "OR";      اكسر;
    حالة TK_PLUS:    zOp = "ADD";     اكسر;
    حالة TK_ستوری:    zOp = "MUL";     اكسر;
    حالة TK_MINUS:   zOp = "SUB";     اكسر;
    حالة TK_REM:     zOp = "REM";     اكسر;
    حالة TK_BITAND:  zOp = "BITAND";  اكسر;
    حالة TK_BITOR:   zOp = "BITOR";   اكسر;
    حالة TK_SLASH:   zOp = "DIV";     اكسر;
    حالة TK_LSHIFT:  zOp = "LSHIFT";  اكسر;
    حالة TK_RSHIFT:  zOp = "RSHIFT";  اكسر;
    حالة TK_CONCAT:  zOp = "CONCAT";  اكسر;
    حالة TK_UMINUS:  zOp = "MINUS";   اكسر;
    حالة TK_UPLUS:   zOp = "PLUS";    اكسر;
    حالة TK_BITNOT:  zOp = "BITNOT";  اكسر;
    حالة TK_NOT:     zOp = "NOT";     اكسر;
    حالة TK_ISNULL:  zOp = "ISNULL";  اكسر;
    حالة TK_NOTNULL: zOp = "NOTNULL"; اكسر;

    افتراضي:
      sqlite3_str_appendf(p, "%s", "expr");
      اكسر;
  }

  إذا( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_بیان(p, p_بیان->p_کیڼ);
    إذا( p_بیان->p_ښی ){
      sqlite3_str_append(p, ",", 1);
      display_P4_بیان(p, p_بیان->p_ښی);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#د_که_پای /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#إذا VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
حرف *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  حرف *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_اوږدتیا);
  بدل( pOp->p4type ){
    حالة P4_KEYINFO: {
      صحيح j;
      کلی_Info *p_کلی_Info = pOp->p4.p_کلی_Info;
      أكد( p_کلی_Info->a_ترتیبول_Flags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_کلی_Info->n_کلی_ډګر);
      لكل(j=0; j<p_کلی_Info->n_کلی_ډګر; j++){
        CollSeq *pColl = p_کلی_Info->aColl[j];
        ثابت حرف *zColl = pColl ? pColl->zName : "";
        إذا( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_کلی_Info->a_ترتیبول_Flags[j] & KEYINFO_ترتیب_DESC) ? "-" : "",
               (p_کلی_Info->a_ترتیبول_Flags[j] & KEYINFO_ترتیب_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      اكسر;
    }
#که_تعریف_شوی SQLITE_ENABLE_CURSOR_HINTS
    حالة P4_بیان: {
      display_P4_بیان(&x, pOp->p4.p_بیان);
      اكسر;
    }
#د_که_پای
    حالة P4_COLLSEQ: {
      ساكن ثابت حرف *ثابت encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      أكد( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      اكسر;
    }
    حالة P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      اكسر;
    }
    حالة P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      اكسر;
    }
    حالة P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      اكسر;
    }
    حالة P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      إذا( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }وإلا إذا( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }وإلا إذا( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }وإلا إذا( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }وإلا{
        أكد( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    حالة P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      اكسر;
    }
#د_که_پای
    حالة P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      لكل(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      اكسر;
    }
    حالة P4_SUBPROGRAM: {
      zP4 = "program";
      اكسر;
    }
    حالة P4_جدول: {
      zP4 = pOp->p4.pTab->zName;
      اكسر;
    }
    حالة P4_شاخص: {
      zP4 = pOp->p4.pIdx->zName;
      اكسر;
    }
    حالة P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      اكسر;
    }
    افتراضي: {
      zP4 = pOp->p4.z;
    }
  }
  إذا( zP4 ) sqlite3_str_appendall(&x, zP4);
  إذا( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  أرجع sqlite3_Str_Accum_پای(&x);
}
#د_که_پای /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
فراغ sqlite3_Vdbe_کاروي_Btree(Vdbe *p, صحيح i){
  أكد( i>=0 && i<p->db->nDb && i<(صحيح)حجم(y_Db_ماسک)*8 );
  أكد( i<(صحيح)حجم(p->btree_ماسک)*8 );
  Db_ماسک_سېټ(p->btree_ماسک, i);
  إذا( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    Db_ماسک_سېټ(p->قلف_ماسک, i);
  }
}

#إذا !defined(SQLITE_OMIT_SHARED_CACHE)
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
فراغ sqlite3VdbeEnter(Vdbe *p){
  صحيح i;
  sqlite3 *db;
  Db *aDb;
  صحيح nDb;
  إذا( Db_ماسک_All_Zero(p->قلف_ماسک) ) أرجع;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  لكل(i=0; i<nDb; i++){
    إذا( i!=1 && Db_ماسک_Test(p->قلف_ماسک,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#د_که_پای

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
ساكن SQLITE_NOINLINE فراغ vdbeLeave(Vdbe *p){
  صحيح i;
  sqlite3 *db;
  Db *aDb;
  صحيح nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  لكل(i=0; i<nDb; i++){
    إذا( i!=1 && Db_ماسک_Test(p->قلف_ماسک,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
فراغ sqlite3VdbeLeave(Vdbe *p){
  إذا( Db_ماسک_All_Zero(p->قلف_ماسک) ) أرجع;  /* The common case */
  vdbeLeave(p);
}
#د_که_پای

#إذا defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
فراغ sqlite3VdbePrintOp(FILE *p_بهر, صحيح pc, VdbeOp *pOp){
  حرف *zP4;
  حرف *zCom;
  sqlite3 dummyDb;
  ساكن ثابت حرف *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  إذا( p_بهر==0 ) p_بهر = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#وإلا
  zCom = 0;
#د_که_پای
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(p_بهر, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(p_بهر);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3_پای_ټکی_Benign_Malloc();
}
#د_که_پای

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
ساكن فراغ initMemArray(Mem *p, صحيح N, sqlite3 *db, u16 flags){
  أكد( db!=0 );
  إذا( N>0 ){
    افعل{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#که_تعریف_شوی SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#د_که_پای
      p++;
    }طالما( (--N)>0 );
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
ساكن فراغ releaseMemArray(Mem *p, صحيح N){
  إذا( p && N ){
    Mem *p_پای_ټکی = &p[N];
    sqlite3 *db = p->db;
    أكد( db!=0 );
    إذا( db->pn_بایټونه_Freed ){
      افعل{
        إذا( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }طالما( (++p)<p_پای_ټکی );
      أرجع;
    }
    افعل{
      أكد( (&p[1])==p_پای_ټکی || p[0].db==p[1].db );
      أكد( sqlite3VdbeCheckMemInvariants(p) );

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
      إذا( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3_Vdbe_چوکاټ_Mem_Del );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }وإلا إذا( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#که_تعریف_شوی SQLITE_DEBUG
      وإلا{
        p->flags = MEM_Undefined;
      }
#د_که_پای
    }طالما( (++p)<p_پای_ټکی );
  }
}

#که_تعریف_شوی SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
صحيح sqlite3_Vdbe_چوکاټ_Is_Valid(Vdbe_چوکاټ *p_چوکاټ){
  إذا( p_چوکاټ->i_چوکاټ_Magic!=SQLITE_چوکاټ_MAGIC ) أرجع 0;
  أرجع 1;
}
#د_که_پای


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
فراغ sqlite3_Vdbe_چوکاټ_Mem_Del(فراغ *pArg){
  Vdbe_چوکاټ *p_چوکاټ = (Vdbe_چوکاټ*)pArg;
  أكد( sqlite3_Vdbe_چوکاټ_Is_Valid(p_چوکاټ) );
  p_چوکاټ->p_پلار = p_چوکاټ->v->p_Del_چوکاټ;
  p_چوکاټ->v->p_Del_چوکاټ = p_چوکاټ;
}

#إذا defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
صحيح sqlite3_Vdbe_بل_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  صحيح e_ډېرپېښ,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  صحيح *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  صحيح *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  صحيح n_لیکه;                            /* Stop when row count reaches this */
  صحيح nSub = 0;                        /* Number of sub-vdbes seen so far */
  Sub_پروګرام **apSub = 0;              /* Array of sub-vdbes */
  صحيح i;                               /* Next instruction address */
  صحيح rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  صحيح iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  n_لیکه = p->nOp;
  إذا( pSub!=0 ){
    إذا( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/حجم(Vdbe*);
      apSub = (Sub_پروګرام **)pSub->z;
    }
    لكل(i=0; i<nSub; i++){
      n_لیکه += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  طالما(1){  /* Loop exits via break */
    i = iPc++;
    إذا( i>=n_لیکه ){
      p->rc = SQLITE_OK;
      rc = SQLITE_بشپړ;
      اكسر;
    }
    إذا( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }وإلا{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      صحيح j;
      i -= p->nOp;
      أكد( apSub!=0 );
      أكد( nSub>0 );
      لكل(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        أكد( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    إذا( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      صحيح nByte = (nSub+1)*حجم(Sub_پروګرام*);
      صحيح j;
      لكل(j=0; j<nSub; j++){
        إذا( apSub[j]==aOp[i].p4.p_پروګرام ) اكسر;
      }
      إذا( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        إذا( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          اكسر;
        }
        apSub = (Sub_پروګرام **)pSub->z;
        apSub[nSub++] = aOp[i].p4.p_پروګرام;
        Mem_سېټ_Type_Flag(pSub, MEM_Blob);
        pSub->n = nSub*حجم(Sub_پروګرام*);
        n_لیکه += aOp[i].p4.p_پروګرام->nOp;
      }
    }
    إذا( e_ډېرپېښ==0 ) اكسر;
#که_تعریف_شوی SQLITE_ENABLE_BYTECODE_VTAB
    إذا( e_ډېرپېښ==2 ){
      Op *pOp = aOp + i;
      إذا( pOp->opcode==OP_OpenRead ) اكسر;
      إذا( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) اكسر;
      إذا( pOp->opcode==OP_ReopenIdx ) اكسر;     
    }وإلا
#د_که_پای
    {
      أكد( e_ډېرپېښ==1 );
      إذا( aOp[i].opcode==OP_Explain ) اكسر;
      إذا( aOp[i].opcode==OP_Init && iPc>1 ) اكسر;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  أرجع rc;
}
#د_که_پای /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
فراغ sqlite3_Vdbe_چوکاټ_Delete(Vdbe_چوکاټ *p){
  صحيح i;
  Mem *aMem = Vdbe_چوکاټ_Mem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->n_ماشوم_Mem];
  أكد( sqlite3_Vdbe_چوکاټ_Is_Valid(p) );
  لكل(i=0; i<p->n_ماشوم_Csr; i++){
    إذا( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->n_ماشوم_Mem);
  sqlite3_Vdbe_Delete_Aux_معلومات(p->v->db, &p->p_Aux_معلومات, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
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
صحيح sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  صحيح i;                               /* Loop counter */
  صحيح rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  صحيح bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  أكد( p->explain );
  أكد( p->eVdbeState==VDBE_پرلپسې_STATE );
  أكد( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  إذا( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    أرجع SQLITE_ERROR;
  }

  إذا( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    أكد( p->nMem>9 );
    pSub = &p->aMem[9];
  }وإلا{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_بل_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  إذا( rc==SQLITE_OK ){
    pOp = aOp + i;
    إذا( Atomic_بارول(&db->u1.isInterrupted) ){
      p->rc = SQLITE_مداخله;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }وإلا{
      حرف *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      إذا( p->explain==2 ){
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem, pOp->p1);
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+1, pOp->p2);
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+2, pOp->p3);
        sqlite3_Vdbe_Mem_سېټ_Str(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        أكد( p->n_Res_ستنه==4 );
      }وإلا{
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+0, i);
        sqlite3_Vdbe_Mem_سېټ_Str(pMem+1, (حرف*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+2, pOp->p1);
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+3, pOp->p2);
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3_Vdbe_Mem_سېټ_Int64(pMem+6, pOp->p5);
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          حرف *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3_Vdbe_Mem_سېټ_Str(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#وإلا
        sqlite3_Vdbe_Mem_سېټ_Null(pMem+7);
#د_که_پای
        sqlite3_Vdbe_Mem_سېټ_Str(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        أكد( p->n_Res_ستنه==8 );
      }
      p->p_پایله_لیکه = pMem;
      إذا( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }وإلا{
        p->rc = SQLITE_OK;
        rc = SQLITE_لیکه;
      }
    }
  }
  أرجع rc;
}
#د_که_پای /* SQLITE_OMIT_EXPLAIN */

#که_تعریف_شوی SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
فراغ sqlite3VdbePrintSql(Vdbe *p){
  ثابت حرف *z = 0;
  إذا( p->zSql ){
    z = p->zSql;
  }وإلا إذا( p->nOp>=1 ){
    ثابت VdbeOp *pOp = &p->aOp[0];
    إذا( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      طالما( sqlite3Isspace(*z) ) z++;
    }
  }
  إذا( z ) چاپ_کړه("SQL: [%s]\n", z);
}
#د_که_پای

#إذا !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
فراغ sqlite3VdbeIOTraceSql(Vdbe *p){
  صحيح nOp = p->nOp;
  VdbeOp *pOp;
  إذا( sqlite3IoTrace==0 ) أرجع;
  إذا( nOp<1 ) أرجع;
  pOp = &p->aOp[0];
  إذا( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    صحيح i, j;
    حرف z[1000];
    sqlite3_snprintf(حجم(z), z, "%s", pOp->p4.z);
    لكل(i=0; sqlite3Isspace(z[i]); i++){}
    لكل(j=0; z[i]; i++){
      إذا( sqlite3Isspace(z[i]) ){
        إذا( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }وإلا{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#د_که_پای /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
بنية ReusableSpace {
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
ساكن فراغ *allocSpace(
  بنية ReusableSpace *p,  /* Bulk memory available for allocation */
  فراغ *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  أكد( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  إذا( pBuf==0 ){
    nByte = ROUND8P(nByte);
    إذا( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }وإلا{
      p->nNeeded += nByte;
    }
  }
  أكد( EIGHT_BYTE_ALIGNMENT(pBuf) );
  أرجع pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
فراغ sqlite3VdbeRewind(Vdbe *p){
#إذا defined(SQLITE_DEBUG)
  صحيح i;
#د_که_پای
  أكد( p!=0 );
  أكد( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  أكد( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#که_تعریف_شوی SQLITE_DEBUG
  لكل(i=0; i<p->nMem; i++){
    أكد( p->aMem[i].db==p->db );
  }
#د_که_پای
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#که_تعریف_شوی VDBE_PROFILE
  لكل(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].n_دوره = 0;
  }
#د_که_پای
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
فراغ sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  صحيح nVar;                      /* Number of parameters */
  صحيح nMem;                      /* Number of VM memory registers */
  صحيح nCursor;                   /* Number of cursors required */
  صحيح nArg;                      /* Max number args to xFilter or xUpdate */
  صحيح n;                         /* Loop counter */
  بنية ReusableSpace x;        /* Reusable bulk memory */

  أكد( p!=0 );
  أكد( p->nOp>0 );
  أكد( pParse!=0 );
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( pParse==p->pParse );
  أكد( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  أكد( db->mallocFailed==0 );
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
  إذا( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(حجم(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  أكد( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  أكد( x.nFree>=0 );
  أكد( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_ارزښتونه(p, &nArg);
  p->کاروي_Stmt_Journal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  إذا( pParse->explain ){
    إذا( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->n_Res_ستنه = 12 - 4*p->explain;
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
  p->aMem = allocSpace(&x, 0, nMem*حجم(Mem));
  p->aVar = allocSpace(&x, 0, nVar*حجم(Mem));
  p->apArg = allocSpace(&x, 0, nArg*حجم(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*حجم(VdbeCursor*));
  إذا( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    إذا( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*حجم(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*حجم(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*حجم(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*حجم(VdbeCursor*));
    }
  }
#که_تعریف_شوی SQLITE_DEBUG
  p->napArg = nArg;
#د_که_پای

  إذا( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }وإلا{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    memset(p->apCsr, 0, nCursor*حجم(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
فراغ sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  إذا( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
ساكن SQLITE_NOINLINE فراغ freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  أكد( pCx->ستون_Cache );
  pCx->ستون_Cache = 0;
  pCx->pCache = 0;
  إذا( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
فراغ sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  إذا( pCx->ستون_Cache ){
    freeCursorWithCache(p, pCx);
    أرجع;
  }
  بدل( pCx->eCurType ){
    حالة CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      اكسر;
    }
    حالة CURTYPE_BTREE: {
      أكد( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
    حالة CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      ثابت sqlite3_module *pModule = pVCur->pVtab->pModule;
      أكد( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      اكسر;
    }
#د_که_پای
  }
}

/*
** Close all cursors in the current frame.
*/
ساكن فراغ close_Cursors_دننه_چوکاټ(Vdbe *p){
  صحيح i;
  لكل(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    إذا( pC ){
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
صحيح sqlite3_Vdbe_چوکاټ_Restore(Vdbe_چوکاټ *p_چوکاټ){
  Vdbe *v = p_چوکاټ->v;
  close_Cursors_دننه_چوکاټ(v);
  v->aOp = p_چوکاټ->aOp;
  v->nOp = p_چوکاټ->nOp;
  v->aMem = p_چوکاټ->aMem;
  v->nMem = p_چوکاټ->nMem;
  v->apCsr = p_چوکاټ->apCsr;
  v->nCursor = p_چوکاټ->nCursor;
  v->db->وروستی_Rowid = p_چوکاټ->وروستی_Rowid;
  v->nChange = p_چوکاټ->nChange;
  v->db->nChange = p_چوکاټ->nDbChange;
  sqlite3_Vdbe_Delete_Aux_معلومات(v->db, &v->p_Aux_معلومات, -1, 0);
  v->p_Aux_معلومات = p_چوکاټ->p_Aux_معلومات;
  p_چوکاټ->p_Aux_معلومات = 0;
  أرجع p_چوکاټ->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
ساكن فراغ closeAllCursors(Vdbe *p){
  إذا( p->p_چوکاټ ){
    Vdbe_چوکاټ *p_چوکاټ;
    لكل(p_چوکاټ=p->p_چوکاټ; p_چوکاټ->p_پلار; p_چوکاټ=p_چوکاټ->p_پلار);
    sqlite3_Vdbe_چوکاټ_Restore(p_چوکاټ);
    p->p_چوکاټ = 0;
    p->n_چوکاټ = 0;
  }
  أكد( p->n_چوکاټ==0 );
  close_Cursors_دننه_چوکاټ(p);
  releaseMemArray(p->aMem, p->nMem);
  طالما( p->p_Del_چوکاټ ){
    Vdbe_چوکاټ *pDel = p->p_Del_چوکاټ;
    p->p_Del_چوکاټ = pDel->p_پلار;
    sqlite3_Vdbe_چوکاټ_Delete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  إذا( p->p_Aux_معلومات ) sqlite3_Vdbe_Delete_Aux_معلومات(p->db, &p->p_Aux_معلومات, -1, 0);
  أكد( p->p_Aux_معلومات==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
فراغ sqlite3_Vdbe_سېټ_عدد_ستون_شمېر(Vdbe *p, صحيح n_Res_ستنه){
  صحيح n;
  sqlite3 *db = p->db;

  إذا( p->nResAlloc ){
    releaseMemArray(p->a_ستون_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->a_ستون_Name);
  }
  n = n_Res_ستنه*COLNAME_N;
  p->n_Res_ستنه = p->nResAlloc = (u16)n_Res_ستنه;
  p->a_ستون_Name = (Mem*)sqlite3DbMallocRawNN(db, حجم(Mem)*n );
  إذا( p->a_ستون_Name==0 ) أرجع;
  initMemArray(p->a_ستون_Name, n, db, MEM_Null);
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
صحيح sqlite3_Vdbe_سېټ_ستون_Name(
  Vdbe *p,                         /* Vdbe being configured */
  صحيح idx,                         /* Index of column zName applies to */
  صحيح var,                         /* One of the COLNAME_* constants */
  ثابت حرف *zName,               /* Pointer to buffer containing name */
  فراغ (*xDel)(فراغ*)              /* Memory management strategy for zName */
){
  صحيح rc;
  Mem *p_ستون_Name;
  أكد( idx<p->nResAlloc );
  أكد( var<COLNAME_N );
  إذا( p->db->mallocFailed ){
    أكد( !zName || xDel!=SQLITE_DYNAMIC );
    أرجع SQLITE_NOMEM_BKPT;
  }
  أكد( p->a_ستون_Name!=0 );
  p_ستون_Name = &(p->a_ستون_Name[idx+var*p->nResAlloc]);
  rc = sqlite3_Vdbe_Mem_سېټ_متن(p_ستون_Name, zName, -1, xDel);
  أكد( rc!=0 || !zName || (p_ستون_Name->flags&MEM_غړی)!=0 );
  أرجع rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
ساكن صحيح vdbeCommit(sqlite3 *db, Vdbe *p){
  صحيح i;
  صحيح nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  صحيح rc = SQLITE_OK;
  صحيح needXcommit = 0;

#که_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  ناکارول_شوی_PARAMETER(p);
#د_که_پای

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
  لكل(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    إذا( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      ساكن ثابت u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /*حافظه*/ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      إذا( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3_Pager_Get_Journal_ډېرپېښ(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        أكد( i!=1 );
        nTrans++;
      }
      rc = sqlite3_Pager_Exclusive_قلف(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  إذا( rc!=SQLITE_OK ){
    أرجع rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  إذا( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    إذا( rc ){
      أرجع SQLITE_CONSTRAINT_COMMITHOOK;
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
  إذا( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    إذا( needXcommit ){
      لكل(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        إذا( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    لكل(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      صحيح txn = sqlite3BtreeTxnState(pBt);
      إذا( txn!=SQLITE_TXN_NONE ){
        أكد( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    إذا( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#که_نه_تعریف_شوی SQLITE_OMIT_DISKIO
  وإلا{
    sqlite3_vfs *pVfs = db->pVfs;
    حرف *zSuper = 0;   /* File-name for the super-journal */
    حرف ثابت *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 دننه_واټن = 0;
    صحيح res;
    صحيح retry_شمېر = 0;
    صحيح nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    إذا( zSuper==0 ) أرجع SQLITE_NOMEM_BKPT;
    zSuper += 4;
    افعل {
      u32 iRandom;
      إذا( retry_شمېر ){
        إذا( retry_شمېر>100 ){
          sqlite3_log(SQLITE_ډک, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          اكسر;
        }وإلا إذا( retry_شمېر==1 ){
          sqlite3_log(SQLITE_ډک, "MJ collide: %s", zSuper);
        }
      }
      retry_شمېر++;
      sqlite3_randomness(حجم(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      أكد( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }طالما( rc==SQLITE_OK && res );
    إذا( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    إذا( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      أرجع rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    لكل(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      إذا( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        حرف ثابت *zFile = sqlite3BtreeGetJournalname(pBt);
        إذا( zFile==0 ){
          استمر;  /* Ignore TEMP and :memory: databases */
        }
        أكد( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,دننه_واټن);
        دننه_واټن += sqlite3Strlen30(zFile)+1;
        إذا( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          أرجع rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    إذا( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_نورمال))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      أرجع rc;
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
    لكل(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      إذا( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    أكد( rc!=SQLITE_BUSY );
    إذا( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      أرجع rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    إذا( rc ){
      أرجع rc;
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
    لكل(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      إذا( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3_پای_ټکی_Benign_Malloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#د_که_پای

  أرجع rc;
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
#که_نه_تعریف_شوی NDEBUG
ساكن فراغ check_فعال_Vdbe_Cnt(sqlite3 *db){
  Vdbe *p;
  صحيح cnt = 0;
  صحيح nWrite = 0;
  صحيح nRead = 0;
  p = db->pVdbe;
  طالما( p ){
    إذا( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      إذا( p->readOnly==0 ) nWrite++;
      إذا( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  أكد( cnt==db->n_Vdbe_فعال );
  أكد( nWrite==db->nVdbeWrite );
  أكد( nRead==db->nVdbeRead );
}
#وإلا
#تعریف_کړه check_فعال_Vdbe_Cnt(x)
#د_که_پای

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
ساكن SQLITE_NOINLINE صحيح vdbeCloseStatement(Vdbe *p, صحيح eOp){
  sqlite3 *ثابت db = p->db;
  صحيح rc = SQLITE_OK;
  صحيح i;
  ثابت صحيح iSavepoint = p->iStatement-1;

  أكد( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  أكد( db->nStatement>0 );
  أكد( p->iStatement==(db->nStatement+db->nSavepoint) );

  لكل(i=0; i<db->nDb; i++){
    صحيح rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    إذا( pBt ){
      إذا( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      إذا( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      إذا( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  إذا( rc==SQLITE_OK ){
    إذا( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    إذا( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  إذا( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  أرجع rc;
}
صحيح sqlite3VdbeCloseStatement(Vdbe *p, صحيح eOp){
  إذا( p->db->nStatement && p->iStatement ){
    أرجع vdbeCloseStatement(p, eOp);
  }
  أرجع SQLITE_OK;
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
#که_نه_تعریف_شوی SQLITE_OMIT_FOREIGN_کلی
ساكن SQLITE_NOINLINE صحيح vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  إذا( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) أرجع SQLITE_ERROR;
  أرجع SQLITE_CONSTRAINT_FOREIGNKEY;
}
صحيح sqlite3_Vdbe_Check_Fk_سمدستي(Vdbe *p){
  إذا( p->nFkConstraint==0 ) أرجع SQLITE_OK;
  أرجع vdbeFkError(p);
}
صحيح sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  إذا( (db->nDeferredCons+db->nDeferredImmCons)==0 ) أرجع SQLITE_OK;
  أرجع vdbeFkError(p);
}
#د_که_پای

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
صحيح sqlite3VdbeHalt(Vdbe *p){
  صحيح rc;                         /* Used to store transient return codes */
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

  أكد( p->eVdbeState==VDBE_پرلپسې_STATE );
  إذا( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  check_فعال_Vdbe_Cnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  إذا( p->bIsReader ){
    صحيح mrc;   /* Primary error code from p->rc */
    صحيح eStatementOp = 0;
    صحيح isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    إذا( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_مداخله
                    || mrc==SQLITE_ډک;
    }وإلا{
      mrc = isSpecialError = 0;
    }
    إذا( isSpecialError ){
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
      إذا( !p->readOnly || mrc!=SQLITE_مداخله ){
        إذا( (mrc==SQLITE_NOMEM || mrc==SQLITE_ډک) && p->کاروي_Stmt_Journal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }وإلا{
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
    إذا( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (فراغ)sqlite3_Vdbe_Check_Fk_سمدستي(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    إذا( !sqlite3_Vtab_دننه_Sync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      إذا( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        إذا( rc!=SQLITE_OK ){
          إذا( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            أرجع SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }وإلا إذا( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }وإلا{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        إذا( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          أرجع SQLITE_BUSY;
        }وإلا إذا( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }وإلا{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3_Commit_دننی_Changes(db);
        }
      }وإلا إذا( p->rc==SQLITE_SCHEMA && db->n_Vdbe_فعال>1 ){
        p->nChange = 0;
      }وإلا{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }وإلا إذا( eStatementOp==0 ){
      إذا( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }وإلا إذا( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }وإلا{
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
    إذا( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      إذا( rc ){
        إذا( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_پیغام);
          p->z_Err_پیغام = 0;
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
    إذا( p->changeCntOn ){
      إذا( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3_Vdbe_سېټ_Changes(db, p->nChange);
      }وإلا{
        sqlite3_Vdbe_سېټ_Changes(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->n_Vdbe_فعال--;
  إذا( !p->readOnly ) db->nVdbeWrite--;
  إذا( p->bIsReader ) db->nVdbeRead--;
  أكد( db->n_Vdbe_فعال>=db->nVdbeRead );
  أكد( db->nVdbeRead>=db->nVdbeWrite );
  أكد( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  check_فعال_Vdbe_Cnt(db);
  إذا( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  إذا( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  أكد( db->n_Vdbe_فعال>0 || db->autoCommit==0 || db->nStatement==0 );
  أرجع (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
فراغ sqlite3_Vdbe_Reset_ګام_پایله(Vdbe *p){
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
صحيح sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  صحيح rc = p->rc;
  إذا( p->z_Err_پیغام ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    إذا( db->pErr==0 ) db->pErr = sqlite3_ارزښت_New(db);
    sqlite3_ارزښت_سېټ_Str(db->pErr, -1, p->z_Err_پیغام, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3_پای_ټکی_Benign_Malloc();
    db->bBenignMalloc--;
  }وإلا إذا( db->pErr ){
    sqlite3_ارزښت_سېټ_Null(db->pErr);
  }
  db->err_کوډ = rc;
  db->err_Byte_دننه_واټن = -1;
  أرجع rc;
}

#که_تعریف_شوی SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
ساكن فراغ vdbeInvokeSqllog(Vdbe *v){
  إذا( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    حرف *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    أكد( v->db->init.busy==0 );
    إذا( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#وإلا
# define vdbeInvokeSqllog(x)
#د_که_پای

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
صحيح sqlite3VdbeReset(Vdbe *p){
#إذا defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  صحيح i;
#د_که_پای

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  إذا( p->eVdbeState==VDBE_پرلپسې_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  إذا( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    إذا( db->pErr || p->z_Err_پیغام ){
      sqlite3VdbeTransferError(p);
    }وإلا{
      db->err_کوډ = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#که_تعریف_شوی SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  إذا( p->apCsr ) لكل(i=0; i<p->nCursor; i++) أكد( p->apCsr[i]==0 );
  إذا( p->aMem ){
    لكل(i=0; i<p->nMem; i++) أكد( p->aMem[i].flags==MEM_Undefined );
  }
#د_که_پای
  إذا( p->z_Err_پیغام ){
    sqlite3DbFree(db, p->z_Err_پیغام);
    p->z_Err_پیغام = 0;
  }
  p->p_پایله_لیکه = 0;
#که_تعریف_شوی SQLITE_DEBUG
  p->nWrite = 0;
#د_که_پای

  /* Save profiling information from this VDBE run.
  */
#که_تعریف_شوی VDBE_PROFILE
  {
    FILE *بهر = fopen("vdbe_profile.out", "a");
    إذا( بهر ){
      fprintf(بهر, "---- ");
      لكل(i=0; i<p->nOp; i++){
        fprintf(بهر, "%02x", p->aOp[i].opcode);
      }
      fprintf(بهر, "\n");
      إذا( p->zSql ){
        حرف c, pc = 0;
        fprintf(بهر, "-- ");
        لكل(i=0; (c = p->zSql[i])!=0; i++){
          إذا( pc=='\n' ) fprintf(بهر, "-- ");
          putc(c, بهر);
          pc = c;
        }
        إذا( pc!='\n' ) fprintf(بهر, "\n");
      }
      لكل(i=0; i<p->nOp; i++){
        حرف zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 دورې = p->aOp[i].n_دوره;
        sqlite3_snprintf(حجم(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           دورې,
           cnt>0 ? دورې/cnt : 0
        );
        fprintf(بهر, "%s", zHdr);
        sqlite3VdbePrintOp(بهر, i, &p->aOp[i]);
      }
      fclose(بهر);
    }
  }
#د_که_پای
  أرجع p->rc & db->err_ماسک;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
صحيح sqlite3VdbeFinalize(Vdbe *p){
  صحيح rc = SQLITE_OK;
  أكد( VDBE_پرلپسې_STATE>VDBE_READY_STATE );
  أكد( VDBE_HALT_STATE>VDBE_READY_STATE );
  أكد( VDBE_INIT_STATE<VDBE_READY_STATE );
  إذا( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    أكد( (rc & p->db->err_ماسک)==rc );
  }
  sqlite3VdbeDelete(p);
  أرجع rc;
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
فراغ sqlite3_Vdbe_Delete_Aux_معلومات(sqlite3 *db, Aux_معلومات **pp, صحيح iOp, صحيح ماسک){
  طالما( *pp ){
    Aux_معلومات *pAux = *pp;
    إذا( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(ماسک & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      إذا( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_بل_Aux;
      sqlite3DbFree(db, pAux);
    }وإلا{
      pp= &pAux->p_بل_Aux;
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
ساكن فراغ sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  Sub_پروګرام *pSub, *p_بل;
  أكد( db!=0 );
  أكد( p->db==0 || p->db==db );
  إذا( p->a_ستون_Name ){
    releaseMemArray(p->a_ستون_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->a_ستون_Name);
  }
  لكل(pSub=p->p_پروګرام; pSub; pSub=p_بل){
    p_بل = pSub->p_بل;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  إذا( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    إذا( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    إذا( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  إذا( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    لكل(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_بل_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#د_که_پای
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  {
    صحيح i;
    لكل(i=0; i<p->n_لټون; i++){
      sqlite3DbFree(db, p->a_لټون[i].zName);
    }
    sqlite3DbFree(db, p->a_لټون);
  }
#د_که_پای
}

/*
** Delete an entire VDBE.
*/
فراغ sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  أكد( p!=0 );
  db = p->db;
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  إذا( db->pn_بایټونه_Freed==0 ){
    أكد( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    إذا( p->pVNext ){
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
صحيح SQLITE_NOINLINE sqlite3_Vdbe_پای_Moveto(VdbeCursor *p){
  صحيح res, rc;
#که_تعریف_شوی SQLITE_TEST
  خارجي صحيح sqlite3_پلټنه_شمېر;
#د_که_پای
  أكد( p->deferredMoveto );
  أكد( p->is_جدول );
  أكد( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3_Btree_جدول_Moveto(p->uc.pCursor, p->moveto_موخه, 0, &res);
  إذا( rc ) أرجع rc;
  إذا( res!=0 ) أرجع SQLITE_CORRUPT_BKPT;
#که_تعریف_شوی SQLITE_TEST
  sqlite3_پلټنه_شمېر++;
#د_که_پای
  p->deferredMoveto = 0;
  p->cache_حالت = CACHE_STALE;
  أرجع SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
صحيح SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  صحيح is_Different_لیکه, rc;
  أكد( p->eCurType==CURTYPE_BTREE );
  أكد( p->uc.pCursor!=0 );
  أكد( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &is_Different_لیکه);
  p->cache_حالت = CACHE_STALE;
  إذا( is_Different_لیکه ) p->null_لیکه = 1;
  أرجع rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
صحيح sqlite3VdbeCursorRestore(VdbeCursor *p){
  أكد( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  إذا( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    أرجع sqlite3VdbeHandleMovedCursor(p);
  }
  أرجع SQLITE_OK;
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

#إذا 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, صحيح file_format, u32 *pLen){
  صحيح flags = pMem->flags;
  u32 n;

  أكد( pLen!=0 );
  إذا( flags&MEM_Null ){
    *pLen = 0;
    أرجع 0;
  }
  إذا( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    إذا( i<0 ){
      u = ~i;
    }وإلا{
      u = i;
    }
    إذا( u<=127 ){
      إذا( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        أرجع 8+(u32)u;
      }وإلا{
        *pLen = 1;
        أرجع 1;
      }
    }
    إذا( u<=32767 ){ *pLen = 2; أرجع 2; }
    إذا( u<=8388607 ){ *pLen = 3; أرجع 3; }
    إذا( u<=2147483647 ){ *pLen = 4; أرجع 4; }
    إذا( u<=MAX_6BYTE ){ *pLen = 6; أرجع 5; }
    *pLen = 8;
    إذا( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (مزدوج)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      أرجع 7;
    }
    أرجع 6;
  }
  إذا( flags&MEM_Real ){
    *pLen = 8;
    أرجع 7;
  }
  أكد( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  أكد( pMem->n>=0 );
  n = (u32)pMem->n;
  إذا( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  أرجع ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#د_که_پای /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
ثابت u8 sqlite3SmallTypeSizes[128] = {
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
  إذا( serial_type>=128 ){
    أرجع (serial_type-12)/2;
  }وإلا{
    أكد( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    أرجع sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  أكد( serial_type<128 );
  أرجع sqlite3SmallTypeSizes[serial_type]; 
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
#که_تعریف_شوی SQLITE_MIXED_ENDIAN_64_بټ_FLOAT
u64 sqlite3_Float_بدلون(u64 دننه){
  اتحاد {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = دننه;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  أرجع u.r;
}
#د_که_پای /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
ساكن صحيح sqlite3VdbeSerialGet7(
  ثابت بینښه حرف *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  أكد( حجم(x)==8 && حجم(pMem->u.r)==8 );
  بدلون_Mixed_Endian_Float(x);
  حافظه_نقل(&pMem->u.r, &x, حجم(x));
  إذا( IsNaN(x) ){
    pMem->flags = MEM_Null;
    أرجع 1;
  }
  pMem->flags = MEM_Real;
  أرجع 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
فراغ sqlite3VdbeSerialGet(
  ثابت بینښه حرف *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  بدل( serial_type ){
    حالة 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      أرجع;
    }
    حالة 11:   /* Reserved for future use */
    حالة 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      أرجع;
    }
    حالة 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#که_تعریف_شوی __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      إذا( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#د_که_پای
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      أرجع;
    }
    حالة 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      أرجع;
    }
    حالة 8:    /* Integer 0 */
    حالة 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      أرجع;
    }
    افتراضي: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      ساكن ثابت u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (حرف *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      أرجع;
    }
  }
  أرجع;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
Unpacked_ریکارډ *sqlite3_Vdbe_Alloc_Unpacked_ریکارډ(
  کلی_Info *p_کلی_Info               /* Description of the record */
){
  Unpacked_ریکارډ *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  أكد( حجم(Unpacked_ریکارډ) + حجم(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(حجم(Unpacked_ریکارډ)) + حجم(Mem)*(p_کلی_Info->n_کلی_ډګر+1);
  p = (Unpacked_ریکارډ *)sqlite3DbMallocRaw(p_کلی_Info->db, nByte);
  إذا( !p ) أرجع 0;
  p->aMem = (Mem*)&((حرف*)p)[ROUND8P(حجم(Unpacked_ریکارډ))];
  p->p_کلی_Info = p_کلی_Info;
  p->n_ډګر = p_کلی_Info->n_کلی_ډګر + 1;
  أرجع p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
فراغ sqlite3_Vdbe_ریکارډ_Unpack(
  صحيح n_کلی,              /* Size of the binary record */
  ثابت فراغ *p_کلی,      /* The binary record */
  Unpacked_ریکارډ *p      /* Populate this structure before returning. */
){
  ثابت بینښه حرف *a_کلی = (ثابت بینښه حرف *)p_کلی;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  کلی_Info *p_کلی_Info = p->p_کلی_Info;

  p->default_rc = 0;
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_کلی, szHdr);
  d = szHdr;
  u = 0;
  طالما( idx<szHdr && d<=(u32)n_کلی ){
    u32 serial_type;

    idx += getVarint32(&a_کلی[idx], serial_type);
    pMem->enc = p_کلی_Info->enc;
    pMem->db = p_کلی_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_کلی[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    إذا( (++u)>=p->n_ډګر ) اكسر;
    pMem++;
  }
  إذا( d>(u32)n_کلی && u ){
    أكد( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3_Vdbe_Mem_سېټ_Null(pMem-(u<p->n_ډګر));
  }
  testcase( u == p_کلی_Info->n_کلی_ډګر + 1 );
  testcase( u < p_کلی_Info->n_کلی_ډګر + 1 );
  أكد( u<=p_کلی_Info->n_کلی_ډګر + 1 );
  p->n_ډګر = u;
}

#که_تعریف_شوی SQLITE_DEBUG
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
ساكن صحيح vdbe_ریکارډ_Compare_Debug(
  صحيح nKey1, ثابت فراغ *pKey1, /* Left key */
  ثابت Unpacked_ریکارډ *pPKey2, /* Right key */
  صحيح desired_پایله             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  صحيح i = 0;
  صحيح rc = 0;
  ثابت بینښه حرف *aKey1 = (ثابت بینښه حرف *)pKey1;
  کلی_Info *p_کلی_Info;
  Mem mem1;

  p_کلی_Info = pPKey2->p_کلی_Info;
  إذا( p_کلی_Info->db==0 ) أرجع 1;
  mem1.enc = p_کلی_Info->enc;
  mem1.db = p_کلی_Info->db;
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
  إذا( szHdr1>98307 ) أرجع SQLITE_CORRUPT;
  d1 = szHdr1;
  أكد( p_کلی_Info->n_All_ډګر>=pPKey2->n_ډګر || CORRUPT_DB );
  أكد( p_کلی_Info->a_ترتیبول_Flags!=0 );
  أكد( p_کلی_Info->n_کلی_ډګر>0 );
  أكد( idx1<=szHdr1 || CORRUPT_DB );
  افعل{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    إذا( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      إذا( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        أرجع 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      اكسر;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_کلی_Info->n_All_ډګر>i ? p_کلی_Info->aColl[i] : 0);
    إذا( rc!=0 ){
      أكد( mem1.szMalloc==0 );  /* See comment below */
      إذا( (p_کلی_Info->a_ترتیبول_Flags[i] & KEYINFO_ترتیب_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      إذا( p_کلی_Info->a_ترتیبول_Flags[i] & KEYINFO_ترتیب_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      اذهب debug_Compare_پای_ټکی;
    }
    i++;
  }طالما( idx1<szHdr1 && i<pPKey2->n_ډګر );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  أكد( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debug_Compare_پای_ټکی:
  إذا( desired_پایله==0 && rc==0 ) أرجع 1;
  إذا( desired_پایله<0 && rc<0 ) أرجع 1;
  إذا( desired_پایله>0 && rc>0 ) أرجع 1;
  إذا( CORRUPT_DB ) أرجع 1;
  إذا( p_کلی_Info->db->mallocFailed ) أرجع 1;
  أرجع 0;
}
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
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
ساكن فراغ vdbe_Assert_ډګر_شمېر_Within_Limits(
  صحيح n_کلی, ثابت فراغ *p_کلی,   /* The record to verify */
  ثابت کلی_Info *p_کلی_Info       /* Compare size with this KeyInfo */
){
  صحيح n_ډګر = 0;
  u32 szHdr;
  u32 idx;
  u32 not_کارول_شوی;
  ثابت بینښه حرف *a_کلی = (ثابت بینښه حرف*)p_کلی;

  إذا( CORRUPT_DB ) أرجع;
  idx = getVarint32(a_کلی, szHdr);
  أكد( n_کلی>=0 );
  أكد( szHdr<=(u32)n_کلی );
  طالما( idx<szHdr ){
    idx += getVarint32(a_کلی+idx, not_کارول_شوی);
    n_ډګر++;
  }
  أكد( n_ډګر <= p_کلی_Info->n_All_ډګر );
}
#وإلا
# define vdbe_Assert_ډګر_شمېر_Within_Limits(A,B,C)
#د_که_پای

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
ساكن SQLITE_NOINLINE صحيح vdbeCompareMemStringWithEncodingChange(
  ثابت Mem *pMem1,
  ثابت Mem *pMem2,
  ثابت CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  صحيح rc;
  ثابت فراغ *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_ارزښت_متن_2((sqlite3_ارزښت*)&c1, pColl->enc);
  v2 = sqlite3_ارزښت_متن_2((sqlite3_ارزښت*)&c2, pColl->enc);
  إذا( (v1==0 || v2==0) ){
    إذا( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }وإلا{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  أرجع rc;
}
ساكن صحيح vdbeCompareMemString(
  ثابت Mem *pMem1,
  ثابت Mem *pMem2,
  ثابت CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  إذا( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    أرجع pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }وإلا{
    أرجع vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
ساكن صحيح isAllZero(ثابت حرف *z, صحيح n){
  صحيح i;
  لكل(i=0; i<n; i++){
    إذا( z[i] ) أرجع 0;
  }
  أرجع 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE صحيح sqlite3BlobCompare(ثابت Mem *pB1, ثابت Mem *pB2){
  صحيح c;
  صحيح n1 = pB1->n;
  صحيح n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  أكد( (pB1->flags & MEM_Zero)==0 || n1==0 );
  أكد( (pB2->flags & MEM_Zero)==0 || n2==0 );

  إذا( (pB1->flags|pB2->flags) & MEM_Zero ){
    إذا( pB1->flags & pB2->flags & MEM_Zero ){
      أرجع pB1->u.nZero - pB2->u.nZero;
    }وإلا إذا( pB1->flags & MEM_Zero ){
      إذا( !isAllZero(pB2->z, pB2->n) ) أرجع -1;
      أرجع pB1->u.nZero - n2;
    }وإلا{
      إذا( !isAllZero(pB1->z, pB1->n) ) أرجع +1;
      أرجع n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  إذا( c ) أرجع c;
  أرجع n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#إذا defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
ساكن صحيح SQLITE_NOINLINE doubleLt(مزدوج a, مزدوج b){ أرجع a<b; }
ساكن صحيح SQLITE_NOINLINE doubleEq(مزدوج a, مزدوج b){ أرجع a==b; }
#د_که_پای

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
صحيح sqlite3IntFloatCompare(i64 i, مزدوج r){
  إذا( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    أرجع 1;
  }وإلا{
    i64 y;
    إذا( r<-9223372036854775808.0 ) أرجع +1;
    إذا( r>=9223372036854775808.0 ) أرجع -1;
    y = (i64)r;
    إذا( i<y ) أرجع -1;
    إذا( i>y ) أرجع +1;
    testcase( doubleLt(((مزدوج)i),r) );
    testcase( doubleLt(r,((مزدوج)i)) );
    testcase( doubleEq(r,((مزدوج)i)) );
    أرجع (((مزدوج)i)<r) ? -1 : (((مزدوج)i)>r);
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
صحيح sqlite3MemCompare(ثابت Mem *pMem1, ثابت Mem *pMem2, ثابت CollSeq *pColl){
  صحيح f1, f2;
  صحيح combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  أكد( !sqlite3_Vdbe_Mem_Is_لیکه_سېټ(pMem1) && !sqlite3_Vdbe_Mem_Is_لیکه_سېټ(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  إذا( combined_flags&MEM_Null ){
    أرجع (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  إذا( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    إذا( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      إذا( pMem1->u.i < pMem2->u.i ) أرجع -1;
      إذا( pMem1->u.i > pMem2->u.i ) أرجع +1;
      أرجع 0;
    }
    إذا( (f1 & f2 & MEM_Real)!=0 ){
      إذا( pMem1->u.r < pMem2->u.r ) أرجع -1;
      إذا( pMem1->u.r > pMem2->u.r ) أرجع +1;
      أرجع 0;
    }
    إذا( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      إذا( (f2&MEM_Real)!=0 ){
        أرجع sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }وإلا إذا( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        إذا( pMem1->u.i < pMem2->u.i ) أرجع -1;
        إذا( pMem1->u.i > pMem2->u.i ) أرجع +1;
        أرجع 0;
      }وإلا{
        أرجع -1;
      }
    }
    إذا( (f1&MEM_Real)!=0 ){
      إذا( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        أرجع -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }وإلا{
        أرجع -1;
      }
    }
    أرجع +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  إذا( combined_flags&MEM_Str ){
    إذا( (f1 & MEM_Str)==0 ){
      أرجع 1;
    }
    إذا( (f2 & MEM_Str)==0 ){
      أرجع -1;
    }

    أكد( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    أكد( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    أكد( !pColl || pColl->xCmp );

    إذا( pColl ){
      أرجع vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  أرجع sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
ساكن i64 vdbe_ریکارډ_Decode_Int(u32 serial_type, ثابت u8 *a_کلی){
  u32 y;
  أكد( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  بدل( serial_type ){
    حالة 0:
    حالة 1:
      testcase( a_کلی[0]&0x80 );
      أرجع ONE_BYTE_INT(a_کلی);
    حالة 2:
      testcase( a_کلی[0]&0x80 );
      أرجع TWO_BYTE_INT(a_کلی);
    حالة 3:
      testcase( a_کلی[0]&0x80 );
      أرجع THREE_BYTE_INT(a_کلی);
    حالة 4: {
      testcase( a_کلی[0]&0x80 );
      y = FOUR_BYTE_UINT(a_کلی);
      أرجع (i64)*(صحيح*)&y;
    }
    حالة 5: {
      testcase( a_کلی[0]&0x80 );
      أرجع FOUR_BYTE_UINT(a_کلی+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_کلی);
    }
    حالة 6: {
      u64 x = FOUR_BYTE_UINT(a_کلی);
      testcase( a_کلی[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_کلی+4);
      أرجع (i64)*(i64*)&x;
    }
  }

  أرجع (serial_type - 8);
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
صحيح sqlite3_Vdbe_ریکارډ_Compare_With_Skip(
  صحيح nKey1, ثابت فراغ *pKey1,   /* Left key */
  Unpacked_ریکارډ *pPKey2,         /* Right key */
  صحيح bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  صحيح i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  صحيح rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  کلی_Info *p_کلی_Info;
  ثابت بینښه حرف *aKey1 = (ثابت بینښه حرف *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  إذا( bSkip ){
    u32 s1 = aKey1[1];
    إذا( s1<0x80 ){
      idx1 = 2;
    }وإلا{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }وإلا{
    إذا( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }وإلا{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  إذا( d1>(بینښه)nKey1 ){
    pPKey2->err_کوډ = (u8)SQLITE_CORRUPT_BKPT;
    أرجع 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  أكد( pPKey2->p_کلی_Info->n_All_ډګر>=pPKey2->n_ډګر
       || CORRUPT_DB );
  أكد( pPKey2->p_کلی_Info->a_ترتیبول_Flags!=0 );
  أكد( pPKey2->p_کلی_Info->n_کلی_ډګر>0 );
  أكد( idx1<=szHdr1 || CORRUPT_DB );
  طالما( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    إذا( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      إذا( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }وإلا إذا( serial_type==0 ){
        rc = -1;
      }وإلا إذا( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }وإلا{
        i64 lhs = vdbe_ریکارډ_Decode_Int(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        إذا( lhs<rhs ){
          rc = -1;
        }وإلا إذا( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    وإلا إذا( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      إذا( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }وإلا إذا( serial_type==0 ){
        rc = -1;
      }وإلا{
        إذا( serial_type==7 ){
          إذا( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }وإلا إذا( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }وإلا إذا( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }وإلا{
            أكد( rc==0 );
          }
        }وإلا{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    وإلا إذا( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      إذا( serial_type<12 ){
        rc = -1;
      }وإلا إذا( !(serial_type & 0x01) ){
        rc = +1;
      }وإلا{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(بینښه)nKey1 );
        testcase( (d1+mem1.n+1)==(بینښه)nKey1 );
        إذا( (d1+mem1.n) > (بینښه)nKey1
         || (p_کلی_Info = pPKey2->p_کلی_Info)->n_All_ډګر<=i
        ){
          pPKey2->err_کوډ = (u8)SQLITE_CORRUPT_BKPT;
          أرجع 0;                /* Corruption */
        }وإلا إذا( p_کلی_Info->aColl[i] ){
          mem1.enc = p_کلی_Info->enc;
          mem1.db = p_کلی_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (حرف*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_کلی_Info->aColl[i], &pPKey2->err_کوډ
          );
        }وإلا{
          صحيح nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          إذا( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    وإلا إذا( pRhs->flags & MEM_Blob ){
      أكد( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      إذا( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }وإلا{
        صحيح nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(بینښه)nKey1 );
        testcase( (d1+nStr+1)==(بینښه)nKey1 );
        إذا( (d1+nStr) > (بینښه)nKey1 ){
          pPKey2->err_کوډ = (u8)SQLITE_CORRUPT_BKPT;
          أرجع 0;                /* Corruption */
        }وإلا إذا( pRhs->flags & MEM_Zero ){
          إذا( !isAllZero((ثابت حرف*)&aKey1[d1],nStr) ){
            rc = 1;
          }وإلا{
            rc = nStr - pRhs->u.nZero;
          }
        }وإلا{
          صحيح nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          إذا( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    وإلا{
      serial_type = aKey1[idx1];
      إذا( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        أكد( rc==0 );
      }وإلا{
        rc = 1;
      }
    }

    إذا( rc!=0 ){
      صحيح ترتیبول_Flags = pPKey2->p_کلی_Info->a_ترتیبول_Flags[i];
      إذا( ترتیبول_Flags ){
        إذا( (ترتیبول_Flags & KEYINFO_ترتیب_BIGNULL)==0
         || ((ترتیبول_Flags & KEYINFO_ترتیب_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      أكد( vdbe_ریکارډ_Compare_Debug(nKey1, pKey1, pPKey2, rc) );
      أكد( mem1.szMalloc==0 );  /* See comment below */
      أرجع rc;
    }

    i++;
    إذا( i==pPKey2->n_ډګر ) اكسر;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    إذا( d1>(بینښه)nKey1 ) اكسر;
    idx1 += sqlite3VarintLen(serial_type);
    إذا( idx1>=(بینښه)szHdr1 ){
      pPKey2->err_کوډ = (u8)SQLITE_CORRUPT_BKPT;
      أرجع 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  أكد( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  أكد( CORRUPT_DB
       || vdbe_ریکارډ_Compare_Debug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_کلی_Info->db->mallocFailed
  );
  pPKey2->eq_کتل_شوی = 1;
  أرجع pPKey2->default_rc;
}
صحيح sqlite3_Vdbe_ریکارډ_Compare(
  صحيح nKey1, ثابت فراغ *pKey1,   /* Left key */
  Unpacked_ریکارډ *pPKey2          /* Right key */
){
  أرجع sqlite3_Vdbe_ریکارډ_Compare_With_Skip(nKey1, pKey1, pPKey2, 0);
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
ساكن صحيح vdbe_ریکارډ_Compare_Int(
  صحيح nKey1, ثابت فراغ *pKey1, /* Left key */
  Unpacked_ریکارډ *pPKey2        /* Right key */
){
  ثابت u8 *a_کلی = &((ثابت u8*)pKey1)[*(ثابت u8*)pKey1 & 0x3F];
  صحيح serial_type = ((ثابت u8*)pKey1)[1];
  صحيح res;
  i64 v;
  i64 lhs;

  vdbe_Assert_ډګر_شمېر_Within_Limits(nKey1, pKey1, pPKey2->p_کلی_Info);
  أكد( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  إذا( (u32)(serial_type-1)<=5 ){
    ساكن ثابت u8 a_ښويدنه[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_کلی)) >> a_ښويدنه[serial_type];
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
    أكد( 0<=(i64)sqlite3Get8byte(a_کلی) || lhs<0 );
    testcase( lhs<0 );
  }وإلا إذا( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }وإلا{
    أرجع sqlite3_Vdbe_ریکارډ_Compare(nKey1, pKey1, pPKey2);
  }

  أكد( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  إذا( v>lhs ){
    res = pPKey2->r1;
  }وإلا إذا( v<lhs ){
    res = pPKey2->r2;
  }وإلا إذا( pPKey2->n_ډګر>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3_Vdbe_ریکارډ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
  }وإلا{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eq_کتل_شوی = 1;
  }

  أكد( vdbe_ریکارډ_Compare_Debug(nKey1, pKey1, pPKey2, res) );
  أرجع res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
ساكن صحيح vdbe_ریکارډ_Compare_String(
  صحيح nKey1, ثابت فراغ *pKey1, /* Left key */
  Unpacked_ریکارډ *pPKey2        /* Right key */
){
  ثابت u8 *aKey1 = (ثابت u8*)pKey1;
  صحيح serial_type;
  صحيح res;

  أكد( pPKey2->aMem[0].flags & MEM_Str );
  أكد( pPKey2->aMem[0].n == pPKey2->n );
  أكد( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_ډګر_شمېر_Within_Limits(nKey1, pKey1, pPKey2->p_کلی_Info);
  serial_type = (نښهدار حرف)(aKey1[1]);

vrcs_restart:
  إذا( serial_type<12 ){
    إذا( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      إذا( serial_type>=12 ) اذهب vrcs_restart;
      أكد( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }وإلا إذا( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }وإلا{
    صحيح nCmp;
    صحيح nStr;
    صحيح szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    إذا( (szHdr + nStr) > nKey1 ){
      pPKey2->err_کوډ = (u8)SQLITE_CORRUPT_BKPT;
      أرجع 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    إذا( res>0 ){
      res = pPKey2->r2;
    }وإلا إذا( res<0 ){
      res = pPKey2->r1;
    }وإلا{
      res = nStr - pPKey2->n;
      إذا( res==0 ){
        إذا( pPKey2->n_ډګر>1 ){
          res = sqlite3_Vdbe_ریکارډ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
        }وإلا{
          res = pPKey2->default_rc;
          pPKey2->eq_کتل_شوی = 1;
        }
      }وإلا إذا( res>0 ){
        res = pPKey2->r2;
      }وإلا{
        res = pPKey2->r1;
      }
    }
  }

  أكد( vdbe_ریکارډ_Compare_Debug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_کلی_Info->db->mallocFailed
  );
  أرجع res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
ریکارډ_Compare sqlite3_Vdbe_پیدا_کړه_Compare(Unpacked_ریکارډ *p){
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
  أكد( p->p_کلی_Info->a_ترتیبول_Flags!=0 );
  إذا( p->p_کلی_Info->n_All_ډګر<=13 ){
    صحيح flags = p->aMem[0].flags;
    إذا( p->p_کلی_Info->a_ترتیبول_Flags[0] ){
      إذا( p->p_کلی_Info->a_ترتیبول_Flags[0] & KEYINFO_ترتیب_BIGNULL ){
        أرجع sqlite3_Vdbe_ریکارډ_Compare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }وإلا{
      p->r1 = -1;
      p->r2 = 1;
    }
    إذا( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      أرجع vdbe_ریکارډ_Compare_Int;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    إذا( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_دوګونی(p->p_کلی_Info->aColl[0])
    ){
      أكد( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      أرجع vdbe_ریکارډ_Compare_String;
    }
  }

  أرجع sqlite3_Vdbe_ریکارډ_Compare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
صحيح sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_کلی = 0;
  صحيح rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  أكد( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_کلی = sqlite3_Btree_Payload_اندازه(pCur);
  أكد( (n_Cell_کلی & SQLITE_MAX_U32)==(u64)n_Cell_کلی );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_دننه_واټن(pCur, (u32)n_Cell_کلی, &m);
  إذا( rc ){
    أرجع rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  أكد( m.n>=0 );
  إذا( unlikely(szHdr<3 || szHdr>(بینښه)m.n) ){
    اذهب idx_rowid_corruption;
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
  إذا( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    اذهب idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  إذا( unlikely((u32)m.n<szHdr+lenRowid) ){
    اذهب idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  أرجع SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  أرجع SQLITE_CORRUPT_BKPT;
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
صحيح sqlite3_Vdbe_Idx_کلی_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  Unpacked_ریکارډ *pUnpacked,       /* Unpacked version of key */
  صحيح *res                         /* Write the comparison result here */
){
  i64 n_Cell_کلی = 0;
  صحيح rc;
  BtCursor *pCur;
  Mem m;

  أكد( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  أكد( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_کلی = sqlite3_Btree_Payload_اندازه(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  إذا( n_Cell_کلی<=0 || n_Cell_کلی>0x7fffffff ){
    *res = 0;
    أرجع SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_دننه_واټن(pCur, (u32)n_Cell_کلی, &m);
  إذا( rc ){
    أرجع rc;
  }
  *res = sqlite3_Vdbe_ریکارډ_Compare_With_Skip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  أرجع SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
فراغ sqlite3_Vdbe_سېټ_Changes(sqlite3 *db, i64 nChange){
  أكد( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_ټول_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
فراغ sqlite3_Vdbe_شمېر_Changes(Vdbe *v){
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
فراغ sqlite3ExpirePreparedStatements(sqlite3 *db, صحيح i_کوډ){
  Vdbe *p;
  لكل(p = db->pVdbe; p; p=p->pVNext){
    p->expired = i_کوډ+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  أرجع v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  أرجع v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_ارزښت *sqlite3_Vdbe_Get_Bound_ارزښت(Vdbe *v, صحيح iVar, u8 aff){
  أكد( iVar>0 );
  إذا( v ){
    Mem *pMem = &v->aVar[iVar-1];
    أكد( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_دننی_Func)!=0 );
    إذا( 0==(pMem->flags & MEM_Null) ){
      sqlite3_ارزښت *pRet = sqlite3_ارزښت_New(v->db);
      إذا( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_ارزښت_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      أرجع pRet;
    }
  }
  أرجع 0;
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
فراغ sqlite3VdbeReprepareOnBind(Vdbe *v, صحيح iVar, صحيح bSmallint){
  u32 m;
  أكد( iVar>0 );
  أكد( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_دننی_Func)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  إذا( bSmallint ){
    إذا( (v->expmask & m)==0 ) v->smimask |= m;
  }وإلا{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  أكد( (v->smimask & v->expmask) == v->smimask );
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
ساكن صحيح vdbe_Skip_ډګر(
  Bitmask ماسک,                   /* Mask of indexed expression fields */
  صحيح i_ستون,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  صحيح bIntegrity                  /* True if running PRAGMA integrity_check */
){
#تعریف_کړه BTREE_ULPDISTORTION 2
  إذا( i_ستون>=BMS || (ماسک & MASKBIT(i_ستون))==0 ) أرجع 0;
  إذا( bIntegrity==0 ) أرجع 1;
  إذا( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    حافظه_نقل(&m1,&pMem1->u.r,8);
    حافظه_نقل(&m2,&pMem2->u.r,8);
    إذا( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      أرجع 1;
    }
  }
  أرجع 0;
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
ساكن صحيح vdbe_Is_Matching_شاخص_کلی(
  BtCursor *pCur,            /* Cursor open on index */
  صحيح bInt,                  /* True for integrity_check-style search */
  Bitmask ماسک,              /* Mask of columns to skip */
  Unpacked_ریکارډ *p,         /* Index key being deleted */
  صحيح *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  صحيح rc = SQLITE_OK;

  memset(&m, 0, حجم(m));
  m.enc = p->p_کلی_Info->enc;
  m.db = p->p_کلی_Info->db;
  nRec = sqlite3_Btree_Payload_اندازه(pCur);
  إذا( nRec>0x7fffffff ){
    أرجع SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  إذا( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }وإلا{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  إذا( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    إذا( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }وإلا{
      صحيح res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      صحيح ii = 0;                 /* Iterator variable */

      صحيح n_ستون = p->p_کلی_Info->n_All_ډګر;
      لكل(ii=0; ii<n_ستون && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        صحيح nSerial = 0;

        إذا( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          اكسر;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        إذا( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }وإلا{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          إذا( vdbe_Skip_ډګر(ماسک, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_کلی_Info->aColl[ii]);
            إذا( res!=0 ) اكسر;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  أرجع rc;
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
صحيح sqlite3_Vdbe_پیدا_کړه_شاخص_کلی(
  BtCursor *pCur, 
  شاخص *pIdx,
  Unpacked_ریکارډ *p, 
  صحيح *pRes,
  صحيح bIntegrity
){
#تعریف_کړه BTREE_FDK_RANGE 10
  صحيح n_ګام = 0;
  صحيح res = 1;
  صحيح rc = SQLITE_OK;
  صحيح ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask ماسک = 0;
  لكل(ii=0; ii<MIN(pIdx->n_ستنه, BMS); ii++){
    صحيح i_ستون = pIdx->ai_ستنه[ii];
    إذا( (i_ستون==XN_بیان)
     || (i_ستون>=0 && (pIdx->p_جدول->a_ستون[i_ستون].ستون_Flags & COLFLAG_VIRTUAL))
    ){
      ماسک |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  إذا( ماسک!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    لكل(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3_Btree_پخوانی(pCur, 0);
    }
    إذا( rc==SQLITE_بشپړ ){
      rc = sqlite3_Btree_لومړی(pCur, &res);
      n_ګام = -1;
    }وإلا{
      n_ګام = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    طالما( sqlite3BtreeCursorIsValidNN(pCur) ){
      لكل(ii=0; rc==SQLITE_OK && (ii<n_ګام || n_ګام<0); ii++){
        rc = vdbe_Is_Matching_شاخص_کلی(pCur, bIntegrity, ماسک, p, &res);
        إذا( res==0 || rc!=SQLITE_OK ) اكسر;
        rc = sqlite3_Btree_بل(pCur, 0);
      }
      إذا( rc==SQLITE_بشپړ ){
        rc = SQLITE_OK;
        أكد( res!=0 );
      }
      إذا( n_ګام<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) اكسر;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      n_ګام = -1;
      rc = sqlite3_Btree_لومړی(pCur, &res);
    }
  }

  *pRes = res;
  أرجع rc;
}

#که_نه_تعریف_شوی SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
صحيح sqlite3_Not_خالص_Func(sqlite3_context *pCtx){
  ثابت VdbeOp *pOp;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( pCtx->pVdbe==0 ) أرجع 1;
#د_که_پای
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  إذا( pOp->opcode==OP_خالص_Func ){
    ثابت حرف *zContext;
    حرف *z_پیغام;
    إذا( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }وإلا إذا( pOp->p5 & NC_Gen_ستون ){
      zContext = "a generated column";
    }وإلا{
      zContext = "an index";
    }
    z_پیغام = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_پایله_error(pCtx, z_پیغام, -1);
    sqlite3_free(z_پیغام);
    أرجع 0;
  }
  أرجع 1;
}
#د_که_پای /* SQLITE_OMIT_DATETIME_FUNCS */

#إذا defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
صحيح sqlite3_Cursor_Range_Hint_بیان_Check(Walker *pWalker, بیان *p_بیان){
  إذا( p_بیان->op==TK_ثبتوونکی ){
    أكد( (pWalker->u.aMem[p_بیان->i_جدول].flags & MEM_Undefined)==0 );
  }
  أرجع WRC_Continue;
}
#د_که_پای /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
فراغ sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  إذا( pVtab->z_Err_پیغام ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_پیغام);
    p->z_Err_پیغام = sqlite3DbStrDup(db, pVtab->z_Err_پیغام);
    sqlite3_free(pVtab->z_Err_پیغام);
    pVtab->z_Err_پیغام = 0;
  }
}
#د_که_پای /* SQLITE_OMIT_VIRTUALTABLE */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
ساكن فراغ vdbeFreeUnpacked(sqlite3 *db, صحيح n_ډګر, Unpacked_ریکارډ *p){
  أكد( db!=0 );
  إذا( p ){
    صحيح i;
    لكل(i=0; i<n_ډګر; i++){
      Mem *pMem = &p->aMem[i];
      إذا( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
فراغ sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  صحيح op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  ثابت حرف *zDb,                /* Database name */
  جدول *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  صحيح iReg,                       /* Register for new.* record */
  صحيح iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  ثابت حرف *zTbl = pTab->zName;
#که_تعریف_شوی SQLITE_DEBUG
  صحيح n_Real_ستون;
  إذا( pTab->tabFlags & TF_WithoutRowid ){
    n_Real_ستون = sqlite3_Primary_کلی_شاخص(pTab)->n_ستنه;
  }وإلا إذا( pTab->tabFlags & TF_HasVirtual ){
    n_Real_ستون = pTab->nNVCol;
  }وإلا{
    n_Real_ستون = pTab->n_ستون;
  }
#د_که_پای

  أكد( db->pPreUpdate==0 );
  memset(&preupdate, 0, حجم(PreUpdate));
  إذا( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_کلی_شاخص(pTab);
  }وإلا{
    إذا( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }وإلا{
      iKey2 = iKey1;
    }
  }

  أكد( pCsr!=0 );
  أكد( pCsr->eCurType==CURTYPE_BTREE );
  أكد( pCsr->n_ډګر==n_Real_ستون
       || (pCsr->n_ډګر==n_Real_ستون+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (کلی_Info*)&preupdate.u_کلی;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_کلی_ډګر = pTab->n_ستون;
  preupdate.pKeyinfo->a_ترتیبول_Flags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.a_ریکارډ);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_کلی_ډګر+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_کلی_ډګر+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  إذا( preupdate.aNew ){
    صحيح i;
    لكل(i=0; i<pCsr->n_ډګر; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  إذا( preupdate.apDflt ){
    صحيح i;
    لكل(i=0; i<pTab->n_ستون; i++){
      sqlite3_ارزښت_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

#که_تعریف_شوی SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
ثابت حرف *sqlite3VdbeFuncName(ثابت sqlite3_context *pCtx){
  أكد( pCtx!=0 );
  أكد( pCtx->pFunc!=0 );
  أرجع pCtx->pFunc->zName;
}
#د_که_پای /* SQLITE_ENABLE_PERCENTILE */
