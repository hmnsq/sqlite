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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

/* Forward references */
ساکن خالی freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
ساکن خالی vdbeFreeOpArray(sqlite3 *, Op *, صحیح);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, حجم(Vdbe) );
  اگر( p==0 ) واپس 0;
  memset(&p->aOp, 0, حجم(Vdbe)-offsetof(Vdbe,aOp));
  p->db = db;
  اگر( db->pVdbe ){
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
  sqlite3_Vdbe_جوڑو_Op2(p, OP_Init, 0, 1);
  واپس p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  واپس p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
خالی sqlite3VdbeError(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  sqlite3DbFree(p->db, p->z_Err_پیغام);
  va_start(ap, zFormat);
  p->z_Err_پیغام = sqlite3VMPrintf(p->db, zFormat, ap);
  va_end(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
خالی sqlite3_Vdbe_سیٹ_Sql(Vdbe *p, ثابت حرف *z, صحیح n, u8 prepFlags){
  اگر( p==0 ) واپس;
  p->prepFlags = prepFlags;
  اگر( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  أكد( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
خالی sqlite3_Vdbe_جوڑو_Dblquote_Str(sqlite3 *db, Vdbe *p, ثابت حرف *z){
  اگر( p ){
    صحیح n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            حجم(*pStr)+n+1-حجم(pStr->z));
    اگر( pStr ){
      pStr->p_اگلا_Str = p->pDblStr;
      p->pDblStr = pStr;
      حافظہ_نقل(pStr->z, z, n+1);
    }
  }
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
صحیح sqlite3_Vdbe_استعمال_کرتا_Double_Quoted_String(
  Vdbe *pVdbe,            /* The prepared statement */
  ثابت حرف *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  أكد( zId!=0 );
  اگر( pVdbe->pDblStr==0 ) واپس 0;
  برائے(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_اگلا_Str){
    اگر( strcmp(zId, pStr->z)==0 ) واپس 1;
  }
  واپس 0;
}
#اگر_ختم

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
خالی sqlite3_Vdbe_تبادلہ(Vdbe *pA, Vdbe *pB){
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
#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#اگر_ختم
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  حافظہ_نقل(pB->a_شمار, pA->a_شمار, حجم(pB->a_شمار));
  pB->a_شمار[SQLITE_STMTSTATUS_REPREPARE]++;
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
ساکن صحیح growOpArray(Vdbe *v, صحیح nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#اگر_تعریف_شدہ SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#ورنہ
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/حجم(Op)));
  غیر_مستعمل_PARAMETER(nOp);
#اگر_ختم

  /* Ensure that the size of a VDBE does not grow too large */
  اگر( nNew > p->db->a_حد_بندی[SQLITE_حد_بندی_VDBE_OP] ){
    sqlite3OomFault(p->db);
    واپس SQLITE_NOMEM;
  }

  أكد( nOp<=(صحیح)(1024/حجم(Op)) );
  أكد( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*حجم(Op));
  اگر( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_سائز(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/حجم(Op);
    v->aOp = pNew;
  }
  واپس (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن خالی test_addop_breakpoint(صحیح pc, Op *pOp){
  ساکن u64 n = 0;
  (خالی)pc;
  (خالی)pOp;
  n++;
  اگر( n==سب_سے_بڑا_UINT64 ) abort(); /* so that n is used, preventing a warning */
}
#اگر_ختم

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
ساکن SQLITE_NOINLINE صحیح growOp3(Vdbe *p, صحیح op, صحیح p1, صحیح p2, صحیح p3){
  أكد( p->nOpAlloc<=p->nOp );
  اگر( growOpArray(p, 1) ) واپس 1;
  أكد( p->nOpAlloc>p->nOp );
  واپس sqlite3_Vdbe_جوڑو_Op3(p, op, p1, p2, p3);
}
ساکن SQLITE_NOINLINE صحیح جوڑو_Op4_Int_Slow(
  Vdbe *p,            /* Add the opcode to this VM */
  صحیح op,             /* The new opcode */
  صحیح p1,             /* The P1 operand */
  صحیح p2,             /* The P2 operand */
  صحیح p3,             /* The P3 operand */
  صحیح p4              /* The P4 operand as an integer */
){
  صحیح addr = sqlite3_Vdbe_جوڑو_Op3(p, op, p1, p2, p3);
  اگر( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  واپس addr;
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
صحیح sqlite3_Vdbe_جوڑو_Op0(Vdbe *p, صحیح op){
  واپس sqlite3_Vdbe_جوڑو_Op3(p, op, 0, 0, 0);
}
صحیح sqlite3_Vdbe_جوڑو_Op1(Vdbe *p, صحیح op, صحیح p1){
  واپس sqlite3_Vdbe_جوڑو_Op3(p, op, p1, 0, 0);
}
صحیح sqlite3_Vdbe_جوڑو_Op2(Vdbe *p, صحیح op, صحیح p1, صحیح p2){
  واپس sqlite3_Vdbe_جوڑو_Op3(p, op, p1, p2, 0);
}
صحیح sqlite3_Vdbe_جوڑو_Op3(Vdbe *p, صحیح op, صحیح p1, صحیح p2, صحیح p3){
  صحیح i;
  VdbeOp *pOp;

  i = p->nOp;
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( op>=0 && op<0xff );
  اگر( p->nOpAlloc<=i ){
    واپس growOp3(p, op, p1, p2, p3);
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
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#اگر_ختم
#اگر defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_چکر = 0;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
  pOp->i_Src_سطر = 0;
#اگر_ختم
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  واپس i;
}
صحیح sqlite3_Vdbe_جوڑو_Op4_Int(
  Vdbe *p,            /* Add the opcode to this VM */
  صحیح op,             /* The new opcode */
  صحیح p1,             /* The P1 operand */
  صحیح p2,             /* The P2 operand */
  صحیح p3,             /* The P3 operand */
  صحیح p4              /* The P4 operand as an integer */
){
  صحیح i;
  VdbeOp *pOp;

  i = p->nOp;
  اگر( p->nOpAlloc<=i ){
    واپس جوڑو_Op4_Int_Slow(p, op, p1, p2, p3, p4);
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
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#اگر_ختم
#اگر defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->n_چکر = 0;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
  pOp->i_Src_سطر = 0;
#اگر_ختم
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  واپس i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
صحیح sqlite3_Vdbe_جوڑو_Int64(Vdbe *p, صحیح iDest, i64 iVal){
  واپس sqlite3_Vdbe_جوڑو_Op3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
صحیح sqlite3_Vdbe_جوڑو_Double(Vdbe *p, صحیح iDest, دوہرا rVal){
  i64 iVal;
  حافظہ_نقل(&iVal,&rVal,8);
  واپس sqlite3_Vdbe_جوڑو_Op3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
صحیح sqlite3VdbeGoto(Vdbe *p, صحیح iDest){
  واپس sqlite3_Vdbe_جوڑو_Op3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
صحیح sqlite3_Vdbe_بھرو_String(Vdbe *p, صحیح iDest, ثابت حرف *zStr){
  واپس sqlite3_Vdbe_جوڑو_Op4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
خالی sqlite3_Vdbe_Multi_بھرو(Vdbe *p, صحیح iDest, ثابت حرف *zTypes, ...){
  va_list ap;
  صحیح i;
  حرف c;
  va_start(ap, zTypes);
  برائے(i=0; (c = zTypes[i])!=0; i++){
    اگر( c=='s' ){
      ثابت حرف *z = va_arg(ap, ثابت حرف*);
      sqlite3_Vdbe_جوڑو_Op4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }ورنہ اگر( c=='i' ){
      sqlite3_Vdbe_جوڑو_Op2(p, OP_Integer, va_arg(ap, صحیح), iDest+i);
    }ورنہ{
      جاؤ skip_op_resultrow;
    }
  }
  sqlite3_Vdbe_جوڑو_Op2(p, OP_نتیجہ_صف, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
صحیح sqlite3_Vdbe_جوڑو_Op4(
  Vdbe *p,            /* Add the opcode to this VM */
  صحیح op,             /* The new opcode */
  صحیح p1,             /* The P1 operand */
  صحیح p2,             /* The P2 operand */
  صحیح p3,             /* The P3 operand */
  ثابت حرف *zP4,    /* The P4 operand */
  صحیح p4type          /* P4 operand type */
){
  صحیح addr = sqlite3_Vdbe_جوڑو_Op3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  واپس addr;
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
صحیح sqlite3_Vdbe_جوڑو_Function_Call(
  Parse *pParse,        /* Parsing context */
  صحیح p1,               /* Constant argument mask */
  صحیح p2,               /* First argument register */
  صحیح p3,               /* Register into which results are written */
  صحیح nArg,             /* Number of argument */
  ثابت FuncDef *pFunc, /* The function to be invoked */
  صحیح eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  صحیح addr;
  sqlite3_context *pCtx;
  أكد( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  اگر( pCtx==0 ){
    أكد( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    واپس 0;
  }
  pCtx->p_برآمد = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3_Vdbe_جوڑو_Op4(v, eCallCtx ? OP_خالص_Func : OP_Function,
                           p1, p2, p3, (حرف*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  واپس addr;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
صحیح sqlite3_Vdbe_Explain_والد(Parse *pParse){
  VdbeOp *pOp;
  اگر( pParse->addrExplain==0 ) واپس 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  واپس pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#اگر defined(SQLITE_DEBUG)
خالی sqlite3ExplainBreakpoint(ثابت حرف *z1, ثابت حرف *z2){
  (خالی)z1;
  (خالی)z2;
}
#اگر_ختم

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
صحیح sqlite3VdbeExplain(Parse *pParse, u8 b_ڈالو, ثابت حرف *zFmt, ...){
  صحیح addr = 0;
#اگر !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  اگر( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#اگر_ختم
  {
    حرف *z_پیغام;
    Vdbe *v;
    va_list ap;
    صحیح iThis;
    va_start(ap, zFmt);
    z_پیغام = sqlite3VMPrintf(pParse->db, zFmt, ap);
    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3_Vdbe_جوڑو_Op4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_پیغام, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_ڈالو?"PUSH":"", sqlite3_Vdbe_Get_آخری_Op(v)->p4.z);
    اگر( b_ڈالو){
      pParse->addrExplain = iThis;
    }
    sqlite3_Vdbe_جائزہ_حالت(v, iThis, -1, -1, 0, 0);
  }
  واپس addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
خالی sqlite3_Vdbe_Explain_نکالو(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3_Vdbe_Explain_والد(pParse);
}
#اگر_ختم /* SQLITE_OMIT_EXPLAIN */

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
خالی sqlite3_Vdbe_جوڑو_Parse_Schema_Op(Vdbe *p, صحیح iDb, حرف *zWhere, u16 p5){
  صحیح j;
  أكد( p5==0 || zWhere==0 );
  sqlite3_Vdbe_جوڑو_Op4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  برائے(j=0; j<p->db->nDb; j++) sqlite3_Vdbe_استعمال_کرتا_Btree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
خالی sqlite3_Vdbe_انتہا_Coroutine(Vdbe *v, صحیح regYield){
  sqlite3_Vdbe_جوڑو_Op1(v, OP_انتہا_Coroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_عارضی_Reg = 0;
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
صحیح sqlite3VdbeMakeLabel(Parse *pParse){
  واپس --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
ساکن SQLITE_NOINLINE خالی resizeResolveLabel(Parse *p, Vdbe *v, صحیح j){
  صحیح n_New_سائز = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_سائز*حجم(p->aLabel[0]));
  اگر( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_DEBUG
    صحیح i;
    برائے(i=p->nLabelAlloc; i<n_New_سائز; i++) p->aLabel[i] = -1;
#اگر_ختم
    اگر( n_New_سائز>=100 && (n_New_سائز/100)>(p->nLabelAlloc/100) ){
      sqlite3_پیش_رفت_Check(p);
    }
    p->nLabelAlloc = n_New_سائز;
    p->aLabel[j] = v->nOp;
  }
}
خالی sqlite3VdbeResolveLabel(Vdbe *v, صحیح x){
  Parse *p = v->pParse;
  صحیح j = ADDR(x);
  أكد( v->eVdbeState==VDBE_INIT_STATE );
  أكد( j<-p->nLabel );
  أكد( j>=0 );
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( p->db->flags & SQLITE_VdbeAddopTrace ){
    لکھو("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#اگر_ختم
  اگر( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }ورنہ{
    أكد( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
خالی sqlite3_Vdbe_تسلسل_Only_Once(Vdbe *p){
  sqlite3_Vdbe_جوڑو_Op2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
خالی sqlite3VdbeReusable(Vdbe *p){
  صحیح i;
  برائے(i=1; ALWAYS(i<p->nOp); i++){
    اگر( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      توڑو;
    }
  }
}

#اگر_تعریف_شدہ SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
قسمتعریف ساخت VdbeOpIter VdbeOpIter;
ساخت VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  Sub_پروگرام **apSub;        /* Array of subprograms */
  صحیح nSub;                  /* Number of entries in apSub */
  صحیح iAddr;                 /* Address of next instruction to return */
  صحیح iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
ساکن Op *op_Iter_اگلا(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  صحیح nOp;

  اگر( p->iSub<=p->nSub ){

    اگر( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }ورنہ{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    أكد( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    اگر( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    اگر( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*حجم(Sub_پروگرام*);
      صحیح j;
      برائے(j=0; j<p->nSub; j++){
        اگر( p->apSub[j]==pRet->p4.p_پروگرام ) توڑو;
      }
      اگر( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        اگر( !p->apSub ){
          pRet = 0;
        }ورنہ{
          p->apSub[p->nSub++] = pRet->p4.p_پروگرام;
        }
      }
    }
  }

  واپس pRet;
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
صحیح sqlite3VdbeAssertMayAbort(Vdbe *v, صحیح mayAbort){
  صحیح hasAbort = 0;
  صحیح has_Fk_شمار = 0;
  صحیح has_Create_جدول = 0;
  صحیح has_Create_اشاریہ = 0;
  صحیح hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  اگر( v==0 ) واپس 0;
  memset(&sIter, 0, حجم(sIter));
  sIter.v = v;

  جب( (pOp = op_Iter_اگلا(&sIter))!=0 ){
    صحیح opcode = pOp->opcode;
    اگر( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_خالص_Func
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      توڑو;
    }
    اگر( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) has_Create_جدول = 1;
    اگر( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      اگر( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_اشاریہ = 1;
      اگر( opcode==OP_Clear ) has_Create_اشاریہ = 1;
    }
    اگر( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
    اگر( opcode==OP_Fk_شمار && pOp->p1==0 && pOp->p2==1 ){
      has_Fk_شمار = 1;
    }
#اگر_ختم
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  واپس ( v->db->mallocFailed || hasAbort==mayAbort || has_Fk_شمار
        || (has_Create_جدول && hasInitCoroutine) || has_Create_اشاریہ
  );
}
#اگر_ختم /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
خالی sqlite3_Vdbe_Incr_Write_شمار(Vdbe *p, VdbeCursor *pC){
  اگر( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
خالی sqlite3VdbeAssertAbortable(Vdbe *p){
  أكد( p->nWrite==0 || p->استعمال_کرتا_Stmt_Journal );
}
#اگر_ختم

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
ساکن خالی resolve_P2_اقدار(Vdbe *p, صحیح *p_Max_Vtab_دلائل){
  صحیح n_Max_Vtab_دلائل = *p_Max_Vtab_دلائل;
  Op *pOp;
  Parse *pParse = p->pParse;
  صحیح *aLabel = pParse->aLabel;

  أكد( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  أكد( p->aOp[0].opcode==OP_Init );
  جب( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    اگر( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      انتخاب( pOp->opcode ){
        حالت OP_Transaction: {
          اگر( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        حالت OP_AutoCommit:
        حالت OP_Savepoint: {
          p->bIsReader = 1;
          توڑو;
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
        حالت OP_Checkpoint:
#اگر_ختم
        حالت OP_Vacuum:
        حالت OP_Journal_منوال: {
          p->readOnly = 0;
          p->bIsReader = 1;
          توڑو;
        }
        حالت OP_Init: {
          أكد( pOp->p2>=0 );
          جاؤ resolve_p2_اقدار_loop_exit;
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
        حالت OP_VUpdate: {
          اگر( pOp->p2>n_Max_Vtab_دلائل ) n_Max_Vtab_دلائل = pOp->p2;
          توڑو;
        }
        حالت OP_VFilter: {
          صحیح n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          أكد( (pOp - p->aOp) >= 3 );
          أكد( pOp[-1].opcode==OP_Integer );
          أكد( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          اگر( n>n_Max_Vtab_دلائل ) n_Max_Vtab_دلائل = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#اگر_ختم
        طےشدہ: {
          اگر( pOp->p2<0 ){
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
          توڑو;
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
resolve_p2_اقدار_loop_exit:
  اگر( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *p_Max_Vtab_دلائل = n_Max_Vtab_دلائل;
  أكد( p->bIsReader!=0 || Db_نقاب_All_Zero(p->btree_نقاب) );
}

#اگر_تعریف_شدہ SQLITE_DEBUG
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
خالی sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  صحیح i_پہلا,       /* First opcode of the subroutine */
  صحیح i_آخری,        /* Last opcode of the subroutine */
  صحیح iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  صحیح i;
  sqlite3_str *pErr = 0;
  أكد( v!=0 );
  pParse = v->pParse;
  أكد( pParse!=0 );
  اگر( pParse->nErr ) واپس;
  أكد( i_آخری>=i_پہلا );
  أكد( i_آخری<v->nOp );
  pOp = &v->aOp[i_پہلا];
  برائے(i=i_پہلا; i<=i_آخری; i++, pOp++){
    اگر( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      صحیح iDest = pOp->p2;   /* Jump destination */
      اگر( iDest==0 ) جاری;
      اگر( pOp->opcode==OP_Gosub ) جاری;
      اگر( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        جاری;
      }
      اگر( iDest<0 ){
        صحیح j = ADDR(iDest);
        أكد( j>=0 );
        اگر( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          جاری;
        }
        iDest = pParse->aLabel[j];
      }
      اگر( iDest<i_پہلا || iDest>i_آخری ){
        صحیح j = iDest;
        برائے(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          اگر( pX->opcode==OP_Return ){
            اگر( pX->p1==iRetReg ) توڑو;
            جاری;
          }
          اگر( pX->opcode==OP_Noop ) جاری;
          اگر( pX->opcode==OP_Explain ) جاری;
          اگر( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }ورنہ{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, i_پہلا, i_آخری);
          توڑو;
        }
      }
    }
  }
  اگر( pErr ){
    حرف *zErr = sqlite3_str_اختتام(pErr);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Halt, SQLITE_اندرونی, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#اگر_ختم /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
صحیح sqlite3VdbeCurrentAddr(Vdbe *p){
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  واپس p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
خالی sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, صحیح N){
  أكد( p->nOp + N <= p->nOpAlloc );
}
#اگر_ختم

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#اگر defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
خالی sqlite3_Vdbe_Verify_No_نتیجہ_صف(Vdbe *p){
  صحیح i;
  برائے(i=0; i<p->nOp; i++){
    أكد( p->aOp[i].opcode!=OP_نتیجہ_صف );
  }
}
#اگر_ختم

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#اگر defined(SQLITE_DEBUG)
خالی sqlite3VdbeVerifyAbortable(Vdbe *p, صحیح onError){
  اگر( onError==OE_Abort ) sqlite3_Vdbe_جوڑو_Op0(p, OP_Abortable);
}
#اگر_ختم

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, صحیح *pnOp, صحیح *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  أكد( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  أكد( Db_نقاب_All_Zero(p->btree_نقاب) );

  resolve_P2_اقدار(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  واپس aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return a
** pointer to the first operation inserted.
**
** Non-zero P2 arguments to jump instructions are automatically adjusted
** so that the jump target is relative to the first operation inserted.
*/
VdbeOp *sqlite3_Vdbe_جوڑو_Op_List(
  Vdbe *p,                     /* Add opcodes to the prepared statement */
  صحیح nOp,                     /* Number of opcodes to add */
  VdbeOpList ثابت *aOp,       /* The opcodes to be added */
  صحیح iLineno                  /* Source-file line number of first opcode */
){
  صحیح i;
  VdbeOp *p_برآمد, *p_پہلا;
  أكد( nOp>0 );
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  اگر( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    واپس 0;
  }
  p_پہلا = p_برآمد = &p->aOp[p->nOp];
  برائے(i=0; i<nOp; i++, aOp++, p_برآمد++){
    p_برآمد->opcode = aOp->opcode;
    p_برآمد->p1 = aOp->p1;
    p_برآمد->p2 = aOp->p2;
    أكد( aOp->p2>=0 );
    اگر( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      p_برآمد->p2 += p->nOp;
    }
    p_برآمد->p3 = aOp->p3;
    p_برآمد->p4type = P4_NOTUSED;
    p_برآمد->p4.p = 0;
    p_برآمد->p5 = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
    p_برآمد->zComment = 0;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
    p_برآمد->i_Src_سطر = iLineno+i;
#ورنہ
    (خالی)iLineno;
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
    اگر( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#اگر_ختم
  }
  p->nOp += nOp;
  واپس p_پہلا;
}

#اگر defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
خالی sqlite3_Vdbe_جائزہ_حالت(
  Vdbe *p,                        /* VM to add scanstatus() to */
  صحیح addrExplain,                /* Address of OP_Explain (or 0) */
  صحیح addrLoop,                   /* Address of loop counter */
  صحیح addr_دورہ,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  ثابت حرف *zName               /* Name of table or index being scanned */
){
  اگر( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->n_جائزہ) * حجم(جائزہ_حالت);
    جائزہ_حالت *aNew;
    aNew = (جائزہ_حالت*)sqlite3DbRealloc(p->db, p->a_جائزہ, nByte);
    اگر( aNew ){
      جائزہ_حالت *pNew = &aNew[p->n_جائزہ++];
      memset(pNew, 0, حجم(جائزہ_حالت));
      pNew->addrExplain = addrExplain;
      pNew->addrLoop = addrLoop;
      pNew->addr_دورہ = addr_دورہ;
      pNew->nEst = nEst;
      pNew->zName = sqlite3DbStrDup(p->db, zName);
      p->a_جائزہ = aNew;
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
خالی sqlite3_Vdbe_جائزہ_حالت_Range(
  Vdbe *p,
  صحیح addrExplain,
  صحیح addr_آغاز,
  صحیح addr_انتہا
){
  اگر( IS_STMT_SCANSTATUS(p->db) ){
    جائزہ_حالت *p_جائزہ = 0;
    صحیح ii;
    برائے(ii=p->n_جائزہ-1; ii>=0; ii--){
      p_جائزہ = &p->a_جائزہ[ii];
      اگر( p_جائزہ->addrExplain==addrExplain ) توڑو;
      p_جائزہ = 0;
    }
    اگر( p_جائزہ ){
      اگر( addr_انتہا<0 ) addr_انتہا = sqlite3VdbeCurrentAddr(p)-1;
      برائے(ii=0; ii<Array_سائز(p_جائزہ->aAddrRange); ii+=2){
        اگر( p_جائزہ->aAddrRange[ii]==0 ){
          p_جائزہ->aAddrRange[ii] = addr_آغاز;
          p_جائزہ->aAddrRange[ii+1] = addr_انتہا;
          توڑو;
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
خالی sqlite3_Vdbe_جائزہ_حالت_Counters(
  Vdbe *p,
  صحیح addrExplain,
  صحیح addrLoop,
  صحیح addr_دورہ
){
  اگر( IS_STMT_SCANSTATUS(p->db) ){
    جائزہ_حالت *p_جائزہ = 0;
    صحیح ii;
    برائے(ii=p->n_جائزہ-1; ii>=0; ii--){
      p_جائزہ = &p->a_جائزہ[ii];
      اگر( p_جائزہ->addrExplain==addrExplain ) توڑو;
      p_جائزہ = 0;
    }
    اگر( p_جائزہ ){
      اگر( addrLoop>0 ) p_جائزہ->addrLoop = addrLoop;
      اگر( addr_دورہ>0 ) p_جائزہ->addr_دورہ = addr_دورہ;
    }
  }
}
#اگر_ختم /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
خالی sqlite3VdbeChangeOpcode(Vdbe *p, صحیح addr, u8 iNewOpcode){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
خالی sqlite3VdbeChangeP1(Vdbe *p, صحیح addr, صحیح val){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
خالی sqlite3VdbeChangeP2(Vdbe *p, صحیح addr, صحیح val){
  أكد( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
خالی sqlite3VdbeChangeP3(Vdbe *p, صحیح addr, صحیح val){
  أكد( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
خالی sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  أكد( p->nOp>0 || p->db->mallocFailed );
  اگر( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
خالی sqlite3_Vdbe_Typeof_عمود(Vdbe *p, صحیح iDest){
  VdbeOp *pOp = sqlite3_Vdbe_Get_آخری_Op(p);
#اگر_تعریف_شدہ SQLITE_DEBUG
  جب( pOp->opcode==OP_ReleaseReg ) pOp--;
#اگر_ختم
  اگر( pOp->p3==iDest && pOp->opcode==OP_عمود ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
خالی sqlite3_Vdbe_Jump_یہاں(Vdbe *p, صحیح addr){
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
خالی sqlite3_Vdbe_Jump_یہاں_Or_نکالو_Inst(Vdbe *p, صحیح addr){
  اگر( addr==p->nOp-1 ){
    أكد( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    أكد( p->aOp[addr].p4type==0 );
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
    sqlite3_Vdbe_Get_آخری_Op(p)->i_Src_سطر = 0;  /* Erase VdbeCoverage() macros */
#اگر_ختم
    p->nOp--;
  }ورنہ{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
ساکن خالی freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  أكد( db!=0 );
  اگر( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
ساکن SQLITE_NOINLINE خالی freeP4Mem(sqlite3 *db, Mem *p){
  اگر( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
ساکن SQLITE_NOINLINE خالی freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  أكد( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
ساکن خالی freeP4(sqlite3 *db, صحیح p4type, خالی *p4){
  أكد( db );
  انتخاب( p4type ){
    حالت P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      توڑو;
    }
    حالت P4_DYNAMIC:
    حالت P4_INTARRAY: {
      اگر( p4 ) sqlite3DbNNFreeNN(db, p4);
      توڑو;
    }
    حالت P4_KEYINFO: {
      اگر( db->pn_بائٹیں_Freed==0 ) sqlite3_کلید_Info_Unref((کلید_Info*)p4);
      توڑو;
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
    حالت P4_اظہاریہ: {
      sqlite3_اظہاریہ_Delete(db, (اظہاریہ*)p4);
      توڑو;
    }
#اگر_ختم
    حالت P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      توڑو;
    }
    حالت P4_MEM: {
      اگر( db->pn_بائٹیں_Freed==0 ){
        sqlite3_قدر_Free((sqlite3_قدر*)p4);
      }ورنہ{
        freeP4Mem(db, (Mem*)p4);
      }
      توڑو;
    }
    حالت P4_VTAB : {
      اگر( db->pn_بائٹیں_Freed==0 ) sqlite3VtabUnlock((VTable *)p4);
      توڑو;
    }
    حالت P4_TABLEREF: {
      اگر( db->pn_بائٹیں_Freed==0 ) sqlite3_Delete_جدول(db, (جدول*)p4);
      توڑو;
    }
    حالت P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      توڑو;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
ساکن خالی vdbeFreeOpArray(sqlite3 *db, Op *aOp, صحیح nOp){
  أكد( nOp>=0 );
  أكد( db!=0 );
  اگر( aOp ){
    Op *pOp = &aOp[nOp-1];
    جب(1){  /* Exit via break */
      اگر( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#اگر_ختم    
      اگر( pOp==aOp ) توڑو;
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
خالی sqlite3_Vdbe_Link_Sub_پروگرام(Vdbe *pVdbe, Sub_پروگرام *p){
  p->p_اگلا = pVdbe->p_پروگرام;
  pVdbe->p_پروگرام = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
صحیح sqlite3_Vdbe_Has_Sub_پروگرام(Vdbe *pVdbe){
  واپس pVdbe->p_پروگرام!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
صحیح sqlite3_Vdbe_Change_مقصد_Noop(Vdbe *p, صحیح addr){
  VdbeOp *pOp;
  اگر( p->db->mallocFailed ) واپس 0;
  أكد( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  واپس 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
صحیح sqlite3_Vdbe_Delete_پیشگی_Opcode(Vdbe *p, u8 op){
  اگر( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    واپس sqlite3_Vdbe_Change_مقصد_Noop(p, p->nOp-1);
  }ورنہ{
    واپس 0;
  }
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
خالی sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  صحیح i_پہلا,          /* Index of first register to be released */
  صحیح N,               /* Number of registers to release */
  u32 نقاب,            /* Mask of registers to NOT release */
  صحیح bUndefine        /* If true, mark registers as undefined */
){
  اگر( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) واپس;
  أكد( pParse->pVdbe );
  أكد( i_پہلا>=1 );
  أكد( i_پہلا+N-1<=pParse->nMem );
  اگر( N<=31 && نقاب!=0 ){
    جب( N>0 && (نقاب&1)!=0 ){
      نقاب >>= 1;
      i_پہلا++;
      N--;
    }
    جب( N>0 && N<=32 && (نقاب & MASKBIT32(N-1))!=0 ){
      نقاب &= ~MASKBIT32(N-1);
      N--;
    }
  }
  اگر( N>0 ){
    sqlite3_Vdbe_جوڑو_Op3(pParse->pVdbe, OP_ReleaseReg, i_پہلا, N, *(صحیح*)&نقاب);
    اگر( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#اگر_ختم /* SQLITE_DEBUG */

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
ساکن خالی SQLITE_NOINLINE vdbe_Change_P4_بھرا(
  Vdbe *p,
  Op *pOp,
  ثابت حرف *zP4,
  صحیح n
){
  اگر( pOp->p4type ){
    أكد( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  اگر( n<0 ){
    sqlite3VdbeChangeP4(p, (صحیح)(pOp - p->aOp), zP4, n);
  }ورنہ{
    اگر( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
خالی sqlite3VdbeChangeP4(Vdbe *p, صحیح addr, ثابت حرف *zP4, صحیح n){
  Op *pOp;
  sqlite3 *db;
  أكد( p!=0 );
  db = p->db;
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( p->aOp!=0 || db->mallocFailed );
  اگر( db->mallocFailed ){
    اگر( n!=P4_VTAB ) freeP4(db, n, (خالی*)*(حرف**)&zP4);
    واپس;
  }
  أكد( p->nOp>0 );
  أكد( addr<p->nOp );
  اگر( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  اگر( n>=0 || pOp->p4type ){
    vdbe_Change_P4_بھرا(p, pOp, zP4, n);
    واپس;
  }
  اگر( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_مقصد_INT(zP4);
    pOp->p4type = P4_INT32;
  }ورنہ اگر( zP4!=0 ){
    أكد( n<0 );
    pOp->p4.p = (خالی*)zP4;
    pOp->p4type = (علامتی حرف)n;
    اگر( n==P4_VTAB ) sqlite3_Vtab_تالا((VTable*)zP4);
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
خالی sqlite3VdbeAppendP4(Vdbe *p, خالی *pP4, صحیح n){
  VdbeOp *pOp;
  أكد( n!=P4_INT32 && n!=P4_VTAB );
  أكد( n<=0 );
  اگر( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }ورنہ{
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
خالی sqlite3_Vdbe_سیٹ_P4_کلید_Info(Parse *pParse, اشاریہ *pIdx){
  Vdbe *v = pParse->pVdbe;
  کلید_Info *p_کلید_Info;
  أكد( v!=0 );
  أكد( pIdx!=0 );
  p_کلید_Info = sqlite3_کلید_Info_کا_اشاریہ(pParse, pIdx);
  اگر( p_کلید_Info ) sqlite3VdbeAppendP4(v, p_کلید_Info, P4_KEYINFO);
}

#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
ساکن خالی vdbeVComment(Vdbe *p, ثابت حرف *zFormat, va_list ap){
  أكد( p->nOp>0 || p->aOp==0 );
  أكد( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  اگر( p->nOp ){
    أكد( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
خالی sqlite3VdbeComment(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  اگر( p ){
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
خالی sqlite3VdbeNoopComment(Vdbe *p, ثابت حرف *zFormat, ...){
  va_list ap;
  اگر( p ){
    sqlite3_Vdbe_جوڑو_Op0(p, OP_Noop);
    va_start(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    va_end(ap);
  }
}
#اگر_ختم  /* NDEBUG */

#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
خالی sqlite3_Vdbe_سیٹ_سطر_نمبر(Vdbe *v, صحیح i_سطر){
  sqlite3_Vdbe_Get_آخری_Op(v)->i_Src_سطر = i_سطر;
}
#اگر_ختم /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, صحیح addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  ساکن VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  أكد( p->eVdbeState==VDBE_INIT_STATE );
  أكد( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  اگر( p->db->mallocFailed ){
    واپس (VdbeOp*)&dummy;
  }ورنہ{
    واپس &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3_Vdbe_Get_آخری_Op(Vdbe *p){
  واپس sqlite3VdbeGetOp(p, p->nOp - 1);
}

#اگر defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
ساکن صحیح translateP(حرف c, ثابت Op *pOp){
  اگر( c=='1' ) واپس pOp->p1;
  اگر( c=='2' ) واپس pOp->p2;
  اگر( c=='3' ) واپس pOp->p3;
  اگر( c=='4' ) واپس pOp->p4.i;
  واپس pOp->p5;
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
  صحیح nOpName;
  صحیح ii;
  حرف zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_لمبائی);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  اگر( zOpName[nOpName+1] ){
    صحیح نظر_آیا_Com = 0;
    حرف c;
    zSynopsis = zOpName + nOpName + 1;
    اگر( لڑی_موازنہ(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(حجم(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    برائے(ii=0; (c = zSynopsis[ii])!=0; ii++){
      اگر( c=='P' ){
        c = zSynopsis[++ii];
        اگر( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }ورنہ اگر( c=='X' ){
          اگر( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            نظر_آیا_Com = 1;
            توڑو;
          }
        }ورنہ اگر( لڑی_موازنہ(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_مقصد_64(pOp->p1,pOp->p3));
          ii += 4;
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_مستعمل_نقاب
        }ورنہ اگر( لڑی_موازنہ(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_مقصد_64(pOp->p2,pOp->p3));
          ii += 4;
#اگر_ختم
        }ورنہ اگر( لڑی_موازنہ(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_مقصد_64(pOp->p1,pOp->p3);
          دوہرا r;
          حافظہ_نقل(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }ورنہ{
          صحیح v1 = translateP(c, pOp);
          صحیح v2;
          اگر( لڑی_موازنہ(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            اگر( لڑی_موازنہ(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            اگر( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ورنہ{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }ورنہ اگر( لڑی_موازنہ(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            اگر( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }ورنہ اگر( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }ورنہ اگر( x.accError==0 ){
              أكد( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }ورنہ{
            sqlite3_str_appendf(&x, "%d", v1);
            اگر( لڑی_موازنہ(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }ورنہ{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    اگر( !نظر_آیا_Com && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }ورنہ اگر( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  اگر( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  واپس sqlite3_Str_Accum_اختتام(&x);
}
#اگر_ختم /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#اگر VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
ساکن خالی display_P4_اظہاریہ(StrAccum *p, اظہاریہ *p_اظہاریہ){
  ثابت حرف *zOp = 0;
  انتخاب( p_اظہاریہ->op ){
    حالت TK_STRING:
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_str_appendf(p, "%Q", p_اظہاریہ->u.zToken);
      توڑو;
    حالت TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_اظہاریہ->u.i_قدر);
      توڑو;
    حالت TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      توڑو;
    حالت TK_رجسٹر: {
      sqlite3_str_appendf(p, "r[%d]", p_اظہاریہ->i_جدول);
      توڑو;
    }
    حالت TK_عمود: {
      اگر( p_اظہاریہ->i_عمود<0 ){
        sqlite3_str_appendf(p, "rowid");
      }ورنہ{
        sqlite3_str_appendf(p, "c%d", (صحیح)p_اظہاریہ->i_عمود);
      }
      توڑو;
    }
    حالت TK_LT:      zOp = "LT";      توڑو;
    حالت TK_LE:      zOp = "LE";      توڑو;
    حالت TK_GT:      zOp = "GT";      توڑو;
    حالت TK_GE:      zOp = "GE";      توڑو;
    حالت TK_NE:      zOp = "NE";      توڑو;
    حالت TK_EQ:      zOp = "EQ";      توڑو;
    حالت TK_IS:      zOp = "IS";      توڑو;
    حالت TK_ISNOT:   zOp = "ISNOT";   توڑو;
    حالت TK_AND:     zOp = "AND";     توڑو;
    حالت TK_OR:      zOp = "OR";      توڑو;
    حالت TK_PLUS:    zOp = "ADD";     توڑو;
    حالت TK_ستارہ:    zOp = "MUL";     توڑو;
    حالت TK_MINUS:   zOp = "SUB";     توڑو;
    حالت TK_REM:     zOp = "REM";     توڑو;
    حالت TK_BITAND:  zOp = "BITAND";  توڑو;
    حالت TK_BITOR:   zOp = "BITOR";   توڑو;
    حالت TK_SLASH:   zOp = "DIV";     توڑو;
    حالت TK_LSHIFT:  zOp = "LSHIFT";  توڑو;
    حالت TK_RSHIFT:  zOp = "RSHIFT";  توڑو;
    حالت TK_CONCAT:  zOp = "CONCAT";  توڑو;
    حالت TK_UMINUS:  zOp = "MINUS";   توڑو;
    حالت TK_UPLUS:   zOp = "PLUS";    توڑو;
    حالت TK_BITNOT:  zOp = "BITNOT";  توڑو;
    حالت TK_NOT:     zOp = "NOT";     توڑو;
    حالت TK_ISNULL:  zOp = "ISNULL";  توڑو;
    حالت TK_NOTNULL: zOp = "NOTNULL"; توڑو;

    طےشدہ:
      sqlite3_str_appendf(p, "%s", "expr");
      توڑو;
  }

  اگر( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_اظہاریہ(p, p_اظہاریہ->p_بایاں);
    اگر( p_اظہاریہ->p_دایاں ){
      sqlite3_str_append(p, ",", 1);
      display_P4_اظہاریہ(p, p_اظہاریہ->p_دایاں);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#اگر_ختم /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#اگر VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
حرف *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  حرف *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_لمبائی);
  انتخاب( pOp->p4type ){
    حالت P4_KEYINFO: {
      صحیح j;
      کلید_Info *p_کلید_Info = pOp->p4.p_کلید_Info;
      أكد( p_کلید_Info->a_ترتیب_دو_Flags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_کلید_Info->n_کلید_میدان);
      برائے(j=0; j<p_کلید_Info->n_کلید_میدان; j++){
        CollSeq *pColl = p_کلید_Info->aColl[j];
        ثابت حرف *zColl = pColl ? pColl->zName : "";
        اگر( strcmp(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_کلید_Info->a_ترتیب_دو_Flags[j] & KEYINFO_ترتیب_DESC) ? "-" : "",
               (p_کلید_Info->a_ترتیب_دو_Flags[j] & KEYINFO_ترتیب_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      توڑو;
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
    حالت P4_اظہاریہ: {
      display_P4_اظہاریہ(&x, pOp->p4.p_اظہاریہ);
      توڑو;
    }
#اگر_ختم
    حالت P4_COLLSEQ: {
      ساکن ثابت حرف *ثابت encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      أكد( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      توڑو;
    }
    حالت P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      توڑو;
    }
    حالت P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      توڑو;
    }
    حالت P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      توڑو;
    }
    حالت P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      اگر( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }ورنہ اگر( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }ورنہ اگر( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }ورنہ اگر( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }ورنہ{
        أكد( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
    حالت P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      توڑو;
    }
#اگر_ختم
    حالت P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      برائے(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      توڑو;
    }
    حالت P4_SUBPROGRAM: {
      zP4 = "program";
      توڑو;
    }
    حالت P4_جدول: {
      zP4 = pOp->p4.pTab->zName;
      توڑو;
    }
    حالت P4_اشاریہ: {
      zP4 = pOp->p4.pIdx->zName;
      توڑو;
    }
    حالت P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      توڑو;
    }
    طےشدہ: {
      zP4 = pOp->p4.z;
    }
  }
  اگر( zP4 ) sqlite3_str_appendall(&x, zP4);
  اگر( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  واپس sqlite3_Str_Accum_اختتام(&x);
}
#اگر_ختم /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
خالی sqlite3_Vdbe_استعمال_کرتا_Btree(Vdbe *p, صحیح i){
  أكد( i>=0 && i<p->db->nDb && i<(صحیح)حجم(y_Db_نقاب)*8 );
  أكد( i<(صحیح)حجم(p->btree_نقاب)*8 );
  Db_نقاب_سیٹ(p->btree_نقاب, i);
  اگر( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    Db_نقاب_سیٹ(p->تالا_نقاب, i);
  }
}

#اگر !defined(SQLITE_OMIT_SHARED_CACHE)
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
خالی sqlite3VdbeEnter(Vdbe *p){
  صحیح i;
  sqlite3 *db;
  Db *aDb;
  صحیح nDb;
  اگر( Db_نقاب_All_Zero(p->تالا_نقاب) ) واپس;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  برائے(i=0; i<nDb; i++){
    اگر( i!=1 && Db_نقاب_Test(p->تالا_نقاب,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
ساکن SQLITE_NOINLINE خالی vdbeLeave(Vdbe *p){
  صحیح i;
  sqlite3 *db;
  Db *aDb;
  صحیح nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  برائے(i=0; i<nDb; i++){
    اگر( i!=1 && Db_نقاب_Test(p->تالا_نقاب,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
خالی sqlite3VdbeLeave(Vdbe *p){
  اگر( Db_نقاب_All_Zero(p->تالا_نقاب) ) واپس;  /* The common case */
  vdbeLeave(p);
}
#اگر_ختم

#اگر defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
خالی sqlite3VdbePrintOp(FILE *p_برآمد, صحیح pc, VdbeOp *pOp){
  حرف *zP4;
  حرف *zCom;
  sqlite3 dummyDb;
  ساکن ثابت حرف *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  اگر( p_برآمد==0 ) p_برآمد = stdout;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#ورنہ
  zCom = 0;
#اگر_ختم
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  fprintf(p_برآمد, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  fflush(p_برآمد);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3_انتہا_Benign_Malloc();
}
#اگر_ختم

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
ساکن خالی initMemArray(Mem *p, صحیح N, sqlite3 *db, u16 flags){
  أكد( db!=0 );
  اگر( N>0 ){
    کرو{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#اگر_تعریف_شدہ SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#اگر_ختم
      p++;
    }جب( (--N)>0 );
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
ساکن خالی releaseMemArray(Mem *p, صحیح N){
  اگر( p && N ){
    Mem *p_انتہا = &p[N];
    sqlite3 *db = p->db;
    أكد( db!=0 );
    اگر( db->pn_بائٹیں_Freed ){
      کرو{
        اگر( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }جب( (++p)<p_انتہا );
      واپس;
    }
    کرو{
      أكد( (&p[1])==p_انتہا || p[0].db==p[1].db );
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
      اگر( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3_Vdbe_چوکھٹا_Mem_Del );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }ورنہ اگر( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#اگر_تعریف_شدہ SQLITE_DEBUG
      ورنہ{
        p->flags = MEM_Undefined;
      }
#اگر_ختم
    }جب( (++p)<p_انتہا );
  }
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
صحیح sqlite3_Vdbe_چوکھٹا_Is_Valid(Vdbe_چوکھٹا *p_چوکھٹا){
  اگر( p_چوکھٹا->i_چوکھٹا_Magic!=SQLITE_چوکھٹا_MAGIC ) واپس 0;
  واپس 1;
}
#اگر_ختم


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
خالی sqlite3_Vdbe_چوکھٹا_Mem_Del(خالی *pArg){
  Vdbe_چوکھٹا *p_چوکھٹا = (Vdbe_چوکھٹا*)pArg;
  أكد( sqlite3_Vdbe_چوکھٹا_Is_Valid(p_چوکھٹا) );
  p_چوکھٹا->p_والد = p_چوکھٹا->v->p_Del_چوکھٹا;
  p_چوکھٹا->v->p_Del_چوکھٹا = p_چوکھٹا;
}

#اگر defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
صحیح sqlite3_Vdbe_اگلا_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  صحیح e_منوال,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  صحیح *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  صحیح *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  صحیح n_صف;                            /* Stop when row count reaches this */
  صحیح nSub = 0;                        /* Number of sub-vdbes seen so far */
  Sub_پروگرام **apSub = 0;              /* Array of sub-vdbes */
  صحیح i;                               /* Next instruction address */
  صحیح rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  صحیح iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  n_صف = p->nOp;
  اگر( pSub!=0 ){
    اگر( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/حجم(Vdbe*);
      apSub = (Sub_پروگرام **)pSub->z;
    }
    برائے(i=0; i<nSub; i++){
      n_صف += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  جب(1){  /* Loop exits via break */
    i = iPc++;
    اگر( i>=n_صف ){
      p->rc = SQLITE_OK;
      rc = SQLITE_مکمل;
      توڑو;
    }
    اگر( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }ورنہ{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      صحیح j;
      i -= p->nOp;
      أكد( apSub!=0 );
      أكد( nSub>0 );
      برائے(j=0; i>=apSub[j]->nOp; j++){
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
    اگر( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      صحیح nByte = (nSub+1)*حجم(Sub_پروگرام*);
      صحیح j;
      برائے(j=0; j<nSub; j++){
        اگر( apSub[j]==aOp[i].p4.p_پروگرام ) توڑو;
      }
      اگر( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        اگر( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          توڑو;
        }
        apSub = (Sub_پروگرام **)pSub->z;
        apSub[nSub++] = aOp[i].p4.p_پروگرام;
        Mem_سیٹ_Type_Flag(pSub, MEM_Blob);
        pSub->n = nSub*حجم(Sub_پروگرام*);
        n_صف += aOp[i].p4.p_پروگرام->nOp;
      }
    }
    اگر( e_منوال==0 ) توڑو;
#اگر_تعریف_شدہ SQLITE_ENABLE_BYTECODE_VTAB
    اگر( e_منوال==2 ){
      Op *pOp = aOp + i;
      اگر( pOp->opcode==OP_OpenRead ) توڑو;
      اگر( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) توڑو;
      اگر( pOp->opcode==OP_ReopenIdx ) توڑو;     
    }ورنہ
#اگر_ختم
    {
      أكد( e_منوال==1 );
      اگر( aOp[i].opcode==OP_Explain ) توڑو;
      اگر( aOp[i].opcode==OP_Init && iPc>1 ) توڑو;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
خالی sqlite3_Vdbe_چوکھٹا_Delete(Vdbe_چوکھٹا *p){
  صحیح i;
  Mem *aMem = Vdbe_چوکھٹا_Mem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->n_بچہ_Mem];
  أكد( sqlite3_Vdbe_چوکھٹا_Is_Valid(p) );
  برائے(i=0; i<p->n_بچہ_Csr; i++){
    اگر( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->n_بچہ_Mem);
  sqlite3_Vdbe_Delete_Aux_ڈیٹا(p->v->db, &p->p_Aux_ڈیٹا, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
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
صحیح sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  صحیح i;                               /* Loop counter */
  صحیح rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  صحیح bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  أكد( p->explain );
  أكد( p->eVdbeState==VDBE_تسلسل_STATE );
  أكد( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  اگر( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    واپس SQLITE_ERROR;
  }

  اگر( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    أكد( p->nMem>9 );
    pSub = &p->aMem[9];
  }ورنہ{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_اگلا_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  اگر( rc==SQLITE_OK ){
    pOp = aOp + i;
    اگر( Atomic_بھرو(&db->u1.isInterrupted) ){
      p->rc = SQLITE_مداخلت;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }ورنہ{
      حرف *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      اگر( p->explain==2 ){
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem, pOp->p1);
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+1, pOp->p2);
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+2, pOp->p3);
        sqlite3_Vdbe_Mem_سیٹ_Str(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        أكد( p->n_Res_عمود==4 );
      }ورنہ{
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+0, i);
        sqlite3_Vdbe_Mem_سیٹ_Str(pMem+1, (حرف*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+2, pOp->p1);
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+3, pOp->p2);
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3_Vdbe_Mem_سیٹ_Int64(pMem+6, pOp->p5);
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          حرف *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3_Vdbe_Mem_سیٹ_Str(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#ورنہ
        sqlite3_Vdbe_Mem_سیٹ_Null(pMem+7);
#اگر_ختم
        sqlite3_Vdbe_Mem_سیٹ_Str(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        أكد( p->n_Res_عمود==8 );
      }
      p->p_نتیجہ_صف = pMem;
      اگر( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }ورنہ{
        p->rc = SQLITE_OK;
        rc = SQLITE_صف;
      }
    }
  }
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_EXPLAIN */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
خالی sqlite3VdbePrintSql(Vdbe *p){
  ثابت حرف *z = 0;
  اگر( p->zSql ){
    z = p->zSql;
  }ورنہ اگر( p->nOp>=1 ){
    ثابت VdbeOp *pOp = &p->aOp[0];
    اگر( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      جب( sqlite3Isspace(*z) ) z++;
    }
  }
  اگر( z ) لکھو("SQL: [%s]\n", z);
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
خالی sqlite3VdbeIOTraceSql(Vdbe *p){
  صحیح nOp = p->nOp;
  VdbeOp *pOp;
  اگر( sqlite3IoTrace==0 ) واپس;
  اگر( nOp<1 ) واپس;
  pOp = &p->aOp[0];
  اگر( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    صحیح i, j;
    حرف z[1000];
    sqlite3_snprintf(حجم(z), z, "%s", pOp->p4.z);
    برائے(i=0; sqlite3Isspace(z[i]); i++){}
    برائے(j=0; z[i]; i++){
      اگر( sqlite3Isspace(z[i]) ){
        اگر( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }ورنہ{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#اگر_ختم /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
ساخت ReusableSpace {
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
ساکن خالی *allocSpace(
  ساخت ReusableSpace *p,  /* Bulk memory available for allocation */
  خالی *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  أكد( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  اگر( pBuf==0 ){
    nByte = ROUND8P(nByte);
    اگر( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }ورنہ{
      p->nNeeded += nByte;
    }
  }
  أكد( EIGHT_BYTE_ALIGNMENT(pBuf) );
  واپس pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
خالی sqlite3VdbeRewind(Vdbe *p){
#اگر defined(SQLITE_DEBUG)
  صحیح i;
#اگر_ختم
  أكد( p!=0 );
  أكد( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  أكد( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#اگر_تعریف_شدہ SQLITE_DEBUG
  برائے(i=0; i<p->nMem; i++){
    أكد( p->aMem[i].db==p->db );
  }
#اگر_ختم
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#اگر_تعریف_شدہ VDBE_PROFILE
  برائے(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].n_چکر = 0;
  }
#اگر_ختم
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
خالی sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  صحیح nVar;                      /* Number of parameters */
  صحیح nMem;                      /* Number of VM memory registers */
  صحیح nCursor;                   /* Number of cursors required */
  صحیح nArg;                      /* Max number args to xFilter or xUpdate */
  صحیح n;                         /* Loop counter */
  ساخت ReusableSpace x;        /* Reusable bulk memory */

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
  اگر( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

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

  resolve_P2_اقدار(p, &nArg);
  p->استعمال_کرتا_Stmt_Journal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  اگر( pParse->explain ){
    اگر( nMem<10 ) nMem = 10;
    p->explain = pParse->explain;
    p->n_Res_عمود = 12 - 4*p->explain;
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
  اگر( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    اگر( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*حجم(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*حجم(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*حجم(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*حجم(VdbeCursor*));
    }
  }
#اگر_تعریف_شدہ SQLITE_DEBUG
  p->napArg = nArg;
#اگر_ختم

  اگر( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }ورنہ{
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
خالی sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  اگر( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
ساکن SQLITE_NOINLINE خالی freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  أكد( pCx->ستون_Cache );
  pCx->ستون_Cache = 0;
  pCx->pCache = 0;
  اگر( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
خالی sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  اگر( pCx->ستون_Cache ){
    freeCursorWithCache(p, pCx);
    واپس;
  }
  انتخاب( pCx->eCurType ){
    حالت CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      توڑو;
    }
    حالت CURTYPE_BTREE: {
      أكد( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
    حالت CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      ثابت sqlite3_module *pModule = pVCur->pVtab->pModule;
      أكد( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      توڑو;
    }
#اگر_ختم
  }
}

/*
** Close all cursors in the current frame.
*/
ساکن خالی close_Cursors_اندر_چوکھٹا(Vdbe *p){
  صحیح i;
  برائے(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    اگر( pC ){
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
صحیح sqlite3_Vdbe_چوکھٹا_Restore(Vdbe_چوکھٹا *p_چوکھٹا){
  Vdbe *v = p_چوکھٹا->v;
  close_Cursors_اندر_چوکھٹا(v);
  v->aOp = p_چوکھٹا->aOp;
  v->nOp = p_چوکھٹا->nOp;
  v->aMem = p_چوکھٹا->aMem;
  v->nMem = p_چوکھٹا->nMem;
  v->apCsr = p_چوکھٹا->apCsr;
  v->nCursor = p_چوکھٹا->nCursor;
  v->db->آخری_Rowid = p_چوکھٹا->آخری_Rowid;
  v->nChange = p_چوکھٹا->nChange;
  v->db->nChange = p_چوکھٹا->nDbChange;
  sqlite3_Vdbe_Delete_Aux_ڈیٹا(v->db, &v->p_Aux_ڈیٹا, -1, 0);
  v->p_Aux_ڈیٹا = p_چوکھٹا->p_Aux_ڈیٹا;
  p_چوکھٹا->p_Aux_ڈیٹا = 0;
  واپس p_چوکھٹا->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
ساکن خالی closeAllCursors(Vdbe *p){
  اگر( p->p_چوکھٹا ){
    Vdbe_چوکھٹا *p_چوکھٹا;
    برائے(p_چوکھٹا=p->p_چوکھٹا; p_چوکھٹا->p_والد; p_چوکھٹا=p_چوکھٹا->p_والد);
    sqlite3_Vdbe_چوکھٹا_Restore(p_چوکھٹا);
    p->p_چوکھٹا = 0;
    p->n_چوکھٹا = 0;
  }
  أكد( p->n_چوکھٹا==0 );
  close_Cursors_اندر_چوکھٹا(p);
  releaseMemArray(p->aMem, p->nMem);
  جب( p->p_Del_چوکھٹا ){
    Vdbe_چوکھٹا *pDel = p->p_Del_چوکھٹا;
    p->p_Del_چوکھٹا = pDel->p_والد;
    sqlite3_Vdbe_چوکھٹا_Delete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  اگر( p->p_Aux_ڈیٹا ) sqlite3_Vdbe_Delete_Aux_ڈیٹا(p->db, &p->p_Aux_ڈیٹا, -1, 0);
  أكد( p->p_Aux_ڈیٹا==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
خالی sqlite3_Vdbe_سیٹ_عدد_ستون_تعداد(Vdbe *p, صحیح n_Res_عمود){
  صحیح n;
  sqlite3 *db = p->db;

  اگر( p->nResAlloc ){
    releaseMemArray(p->a_ستون_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->a_ستون_Name);
  }
  n = n_Res_عمود*COLNAME_N;
  p->n_Res_عمود = p->nResAlloc = (u16)n_Res_عمود;
  p->a_ستون_Name = (Mem*)sqlite3DbMallocRawNN(db, حجم(Mem)*n );
  اگر( p->a_ستون_Name==0 ) واپس;
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
صحیح sqlite3_Vdbe_سیٹ_ستون_Name(
  Vdbe *p,                         /* Vdbe being configured */
  صحیح idx,                         /* Index of column zName applies to */
  صحیح var,                         /* One of the COLNAME_* constants */
  ثابت حرف *zName,               /* Pointer to buffer containing name */
  خالی (*xDel)(خالی*)              /* Memory management strategy for zName */
){
  صحیح rc;
  Mem *p_ستون_Name;
  أكد( idx<p->nResAlloc );
  أكد( var<COLNAME_N );
  اگر( p->db->mallocFailed ){
    أكد( !zName || xDel!=SQLITE_DYNAMIC );
    واپس SQLITE_NOMEM_BKPT;
  }
  أكد( p->a_ستون_Name!=0 );
  p_ستون_Name = &(p->a_ستون_Name[idx+var*p->nResAlloc]);
  rc = sqlite3_Vdbe_Mem_سیٹ_متن(p_ستون_Name, zName, -1, xDel);
  أكد( rc!=0 || !zName || (p_ستون_Name->flags&MEM_رکن)!=0 );
  واپس rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
ساکن صحیح vdbeCommit(sqlite3 *db, Vdbe *p){
  صحیح i;
  صحیح nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  صحیح rc = SQLITE_OK;
  صحیح needXcommit = 0;

#اگر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  غیر_مستعمل_PARAMETER(p);
#اگر_ختم

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
  برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    اگر( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      ساکن ثابت u8 aMJNeeded[] = {
        /* DELETE   */  1,
        /* PERSIST   */ 1,
        /* OFF       */ 0,
        /* TRUNCATE  */ 1,
        /*حافظہ*/ 0,
        /* WAL       */ 0
      };
      Pager *pPager;   /* Pager associated with pBt */
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      اگر( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3_Pager_Get_Journal_منوال(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        أكد( i!=1 );
        nTrans++;
      }
      rc = sqlite3_Pager_Exclusive_تالا(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  اگر( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    اگر( rc ){
      واپس SQLITE_CONSTRAINT_COMMITHOOK;
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
  اگر( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    اگر( needXcommit ){
      برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        اگر( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      صحیح txn = sqlite3BtreeTxnState(pBt);
      اگر( txn!=SQLITE_TXN_NONE ){
        أكد( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    اگر( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DISKIO
  ورنہ{
    sqlite3_vfs *pVfs = db->pVfs;
    حرف *zSuper = 0;   /* File-name for the super-journal */
    حرف ثابت *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 انحراف = 0;
    صحیح res;
    صحیح retry_گنتی = 0;
    صحیح nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    اگر( zSuper==0 ) واپس SQLITE_NOMEM_BKPT;
    zSuper += 4;
    کرو {
      u32 iRandom;
      اگر( retry_گنتی ){
        اگر( retry_گنتی>100 ){
          sqlite3_log(SQLITE_بھرا, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          توڑو;
        }ورنہ اگر( retry_گنتی==1 ){
          sqlite3_log(SQLITE_بھرا, "MJ collide: %s", zSuper);
        }
      }
      retry_گنتی++;
      sqlite3_randomness(حجم(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      أكد( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }جب( rc==SQLITE_OK && res );
    اگر( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    اگر( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      واپس rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    برائے(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      اگر( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        حرف ثابت *zFile = sqlite3BtreeGetJournalname(pBt);
        اگر( zFile==0 ){
          جاری;  /* Ignore TEMP and :memory: databases */
        }
        أكد( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,انحراف);
        انحراف += sqlite3Strlen30(zFile)+1;
        اگر( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          واپس rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    اگر( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_معمول))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      واپس rc;
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
    برائے(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      اگر( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    أكد( rc!=SQLITE_BUSY );
    اگر( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      واپس rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    اگر( rc ){
      واپس rc;
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
    برائے(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      اگر( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3_انتہا_Benign_Malloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#اگر_ختم

  واپس rc;
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
#اگر_غیر_تعریف_شدہ NDEBUG
ساکن خالی check_فعال_Vdbe_Cnt(sqlite3 *db){
  Vdbe *p;
  صحیح cnt = 0;
  صحیح nWrite = 0;
  صحیح nRead = 0;
  p = db->pVdbe;
  جب( p ){
    اگر( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      اگر( p->readOnly==0 ) nWrite++;
      اگر( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  أكد( cnt==db->n_Vdbe_فعال );
  أكد( nWrite==db->nVdbeWrite );
  أكد( nRead==db->nVdbeRead );
}
#ورنہ
#تعریف_کرو check_فعال_Vdbe_Cnt(x)
#اگر_ختم

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
ساکن SQLITE_NOINLINE صحیح vdbeCloseStatement(Vdbe *p, صحیح eOp){
  sqlite3 *ثابت db = p->db;
  صحیح rc = SQLITE_OK;
  صحیح i;
  ثابت صحیح iSavepoint = p->iStatement-1;

  أكد( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  أكد( db->nStatement>0 );
  أكد( p->iStatement==(db->nStatement+db->nSavepoint) );

  برائے(i=0; i<db->nDb; i++){
    صحیح rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    اگر( pBt ){
      اگر( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      اگر( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      اگر( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  اگر( rc==SQLITE_OK ){
    اگر( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    اگر( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  اگر( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  واپس rc;
}
صحیح sqlite3VdbeCloseStatement(Vdbe *p, صحیح eOp){
  اگر( p->db->nStatement && p->iStatement ){
    واپس vdbeCloseStatement(p, eOp);
  }
  واپس SQLITE_OK;
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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
ساکن SQLITE_NOINLINE صحیح vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  اگر( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) واپس SQLITE_ERROR;
  واپس SQLITE_CONSTRAINT_FOREIGNKEY;
}
صحیح sqlite3_Vdbe_Check_Fk_فوری(Vdbe *p){
  اگر( p->nFkConstraint==0 ) واپس SQLITE_OK;
  واپس vdbeFkError(p);
}
صحیح sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  اگر( (db->nDeferredCons+db->nDeferredImmCons)==0 ) واپس SQLITE_OK;
  واپس vdbeFkError(p);
}
#اگر_ختم

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
صحیح sqlite3VdbeHalt(Vdbe *p){
  صحیح rc;                         /* Used to store transient return codes */
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

  أكد( p->eVdbeState==VDBE_تسلسل_STATE );
  اگر( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  check_فعال_Vdbe_Cnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  اگر( p->bIsReader ){
    صحیح mrc;   /* Primary error code from p->rc */
    صحیح eStatementOp = 0;
    صحیح isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    اگر( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_مداخلت
                    || mrc==SQLITE_بھرا;
    }ورنہ{
      mrc = isSpecialError = 0;
    }
    اگر( isSpecialError ){
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
      اگر( !p->readOnly || mrc!=SQLITE_مداخلت ){
        اگر( (mrc==SQLITE_NOMEM || mrc==SQLITE_بھرا) && p->استعمال_کرتا_Stmt_Journal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }ورنہ{
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
    اگر( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (خالی)sqlite3_Vdbe_Check_Fk_فوری(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    اگر( !sqlite3_Vtab_اندر_Sync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      اگر( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        اگر( rc!=SQLITE_OK ){
          اگر( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            واپس SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }ورنہ اگر( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }ورنہ{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        اگر( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          واپس SQLITE_BUSY;
        }ورنہ اگر( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }ورنہ{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3_Commit_اندرونی_Changes(db);
        }
      }ورنہ اگر( p->rc==SQLITE_SCHEMA && db->n_Vdbe_فعال>1 ){
        p->nChange = 0;
      }ورنہ{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }ورنہ اگر( eStatementOp==0 ){
      اگر( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }ورنہ اگر( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }ورنہ{
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
    اگر( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      اگر( rc ){
        اگر( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
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
    اگر( p->changeCntOn ){
      اگر( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3_Vdbe_سیٹ_Changes(db, p->nChange);
      }ورنہ{
        sqlite3_Vdbe_سیٹ_Changes(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->n_Vdbe_فعال--;
  اگر( !p->readOnly ) db->nVdbeWrite--;
  اگر( p->bIsReader ) db->nVdbeRead--;
  أكد( db->n_Vdbe_فعال>=db->nVdbeRead );
  أكد( db->nVdbeRead>=db->nVdbeWrite );
  أكد( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  check_فعال_Vdbe_Cnt(db);
  اگر( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  اگر( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  أكد( db->n_Vdbe_فعال>0 || db->autoCommit==0 || db->nStatement==0 );
  واپس (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
خالی sqlite3_Vdbe_Reset_قدم_نتیجہ(Vdbe *p){
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
صحیح sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  صحیح rc = p->rc;
  اگر( p->z_Err_پیغام ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    اگر( db->pErr==0 ) db->pErr = sqlite3_قدر_New(db);
    sqlite3_قدر_سیٹ_Str(db->pErr, -1, p->z_Err_پیغام, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3_انتہا_Benign_Malloc();
    db->bBenignMalloc--;
  }ورنہ اگر( db->pErr ){
    sqlite3_قدر_سیٹ_Null(db->pErr);
  }
  db->err_رمز = rc;
  db->err_Byte_انحراف = -1;
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
ساکن خالی vdbeInvokeSqllog(Vdbe *v){
  اگر( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    حرف *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    أكد( v->db->init.busy==0 );
    اگر( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#ورنہ
# define vdbeInvokeSqllog(x)
#اگر_ختم

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
صحیح sqlite3VdbeReset(Vdbe *p){
#اگر defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  صحیح i;
#اگر_ختم

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  اگر( p->eVdbeState==VDBE_تسلسل_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  اگر( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    اگر( db->pErr || p->z_Err_پیغام ){
      sqlite3VdbeTransferError(p);
    }ورنہ{
      db->err_رمز = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  اگر( p->apCsr ) برائے(i=0; i<p->nCursor; i++) أكد( p->apCsr[i]==0 );
  اگر( p->aMem ){
    برائے(i=0; i<p->nMem; i++) أكد( p->aMem[i].flags==MEM_Undefined );
  }
#اگر_ختم
  اگر( p->z_Err_پیغام ){
    sqlite3DbFree(db, p->z_Err_پیغام);
    p->z_Err_پیغام = 0;
  }
  p->p_نتیجہ_صف = 0;
#اگر_تعریف_شدہ SQLITE_DEBUG
  p->nWrite = 0;
#اگر_ختم

  /* Save profiling information from this VDBE run.
  */
#اگر_تعریف_شدہ VDBE_PROFILE
  {
    FILE *برآمد = fopen("vdbe_profile.out", "a");
    اگر( برآمد ){
      fprintf(برآمد, "---- ");
      برائے(i=0; i<p->nOp; i++){
        fprintf(برآمد, "%02x", p->aOp[i].opcode);
      }
      fprintf(برآمد, "\n");
      اگر( p->zSql ){
        حرف c, pc = 0;
        fprintf(برآمد, "-- ");
        برائے(i=0; (c = p->zSql[i])!=0; i++){
          اگر( pc=='\n' ) fprintf(برآمد, "-- ");
          putc(c, برآمد);
          pc = c;
        }
        اگر( pc!='\n' ) fprintf(برآمد, "\n");
      }
      برائے(i=0; i<p->nOp; i++){
        حرف zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 ادوار = p->aOp[i].n_چکر;
        sqlite3_snprintf(حجم(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           ادوار,
           cnt>0 ? ادوار/cnt : 0
        );
        fprintf(برآمد, "%s", zHdr);
        sqlite3VdbePrintOp(برآمد, i, &p->aOp[i]);
      }
      fclose(برآمد);
    }
  }
#اگر_ختم
  واپس p->rc & db->err_نقاب;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
صحیح sqlite3VdbeFinalize(Vdbe *p){
  صحیح rc = SQLITE_OK;
  أكد( VDBE_تسلسل_STATE>VDBE_READY_STATE );
  أكد( VDBE_HALT_STATE>VDBE_READY_STATE );
  أكد( VDBE_INIT_STATE<VDBE_READY_STATE );
  اگر( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    أكد( (rc & p->db->err_نقاب)==rc );
  }
  sqlite3VdbeDelete(p);
  واپس rc;
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
خالی sqlite3_Vdbe_Delete_Aux_ڈیٹا(sqlite3 *db, Aux_ڈیٹا **pp, صحیح iOp, صحیح نقاب){
  جب( *pp ){
    Aux_ڈیٹا *pAux = *pp;
    اگر( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(نقاب & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      اگر( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_اگلا_Aux;
      sqlite3DbFree(db, pAux);
    }ورنہ{
      pp= &pAux->p_اگلا_Aux;
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
ساکن خالی sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  Sub_پروگرام *pSub, *p_اگلا;
  أكد( db!=0 );
  أكد( p->db==0 || p->db==db );
  اگر( p->a_ستون_Name ){
    releaseMemArray(p->a_ستون_Name, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->a_ستون_Name);
  }
  برائے(pSub=p->p_پروگرام; pSub; pSub=p_اگلا){
    p_اگلا = pSub->p_اگلا;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  اگر( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    اگر( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    اگر( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  اگر( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    برائے(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_اگلا_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  {
    صحیح i;
    برائے(i=0; i<p->n_جائزہ; i++){
      sqlite3DbFree(db, p->a_جائزہ[i].zName);
    }
    sqlite3DbFree(db, p->a_جائزہ);
  }
#اگر_ختم
}

/*
** Delete an entire VDBE.
*/
خالی sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  أكد( p!=0 );
  db = p->db;
  أكد( db!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  اگر( db->pn_بائٹیں_Freed==0 ){
    أكد( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    اگر( p->pVNext ){
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
صحیح SQLITE_NOINLINE sqlite3_Vdbe_اختتام_Moveto(VdbeCursor *p){
  صحیح res, rc;
#اگر_تعریف_شدہ SQLITE_TEST
  خارجی صحیح sqlite3_تلاش_گنتی;
#اگر_ختم
  أكد( p->deferredMoveto );
  أكد( p->is_جدول );
  أكد( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3_Btree_جدول_Moveto(p->uc.pCursor, p->moveto_ہدف, 0, &res);
  اگر( rc ) واپس rc;
  اگر( res!=0 ) واپس SQLITE_CORRUPT_BKPT;
#اگر_تعریف_شدہ SQLITE_TEST
  sqlite3_تلاش_گنتی++;
#اگر_ختم
  p->deferredMoveto = 0;
  p->cache_حالت = CACHE_STALE;
  واپس SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
صحیح SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  صحیح is_Different_صف, rc;
  أكد( p->eCurType==CURTYPE_BTREE );
  أكد( p->uc.pCursor!=0 );
  أكد( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &is_Different_صف);
  p->cache_حالت = CACHE_STALE;
  اگر( is_Different_صف ) p->null_صف = 1;
  واپس rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
صحیح sqlite3VdbeCursorRestore(VdbeCursor *p){
  أكد( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  اگر( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    واپس sqlite3VdbeHandleMovedCursor(p);
  }
  واپس SQLITE_OK;
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

#اگر 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, صحیح file_format, u32 *pLen){
  صحیح flags = pMem->flags;
  u32 n;

  أكد( pLen!=0 );
  اگر( flags&MEM_Null ){
    *pLen = 0;
    واپس 0;
  }
  اگر( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    اگر( i<0 ){
      u = ~i;
    }ورنہ{
      u = i;
    }
    اگر( u<=127 ){
      اگر( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        واپس 8+(u32)u;
      }ورنہ{
        *pLen = 1;
        واپس 1;
      }
    }
    اگر( u<=32767 ){ *pLen = 2; واپس 2; }
    اگر( u<=8388607 ){ *pLen = 3; واپس 3; }
    اگر( u<=2147483647 ){ *pLen = 4; واپس 4; }
    اگر( u<=MAX_6BYTE ){ *pLen = 6; واپس 5; }
    *pLen = 8;
    اگر( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (دوہرا)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      واپس 7;
    }
    واپس 6;
  }
  اگر( flags&MEM_Real ){
    *pLen = 8;
    واپس 7;
  }
  أكد( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  أكد( pMem->n>=0 );
  n = (u32)pMem->n;
  اگر( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  واپس ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#اگر_ختم /* inlined into OP_MakeRecord */

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
  اگر( serial_type>=128 ){
    واپس (serial_type-12)/2;
  }ورنہ{
    أكد( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    واپس sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  أكد( serial_type<128 );
  واپس sqlite3SmallTypeSizes[serial_type]; 
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
#اگر_تعریف_شدہ SQLITE_MIXED_ENDIAN_64_ثنائی_ہندسہ_FLOAT
u64 sqlite3_Float_تبادلہ(u64 اندر){
  اتحاد {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = اندر;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  واپس u.r;
}
#اگر_ختم /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
ساکن صحیح sqlite3VdbeSerialGet7(
  ثابت غیرعلامتی حرف *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  أكد( حجم(x)==8 && حجم(pMem->u.r)==8 );
  تبادلہ_Mixed_Endian_Float(x);
  حافظہ_نقل(&pMem->u.r, &x, حجم(x));
  اگر( IsNaN(x) ){
    pMem->flags = MEM_Null;
    واپس 1;
  }
  pMem->flags = MEM_Real;
  واپس 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
خالی sqlite3VdbeSerialGet(
  ثابت غیرعلامتی حرف *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  انتخاب( serial_type ){
    حالت 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      واپس;
    }
    حالت 11:   /* Reserved for future use */
    حالت 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      واپس;
    }
    حالت 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#اگر_تعریف_شدہ __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      اگر( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#اگر_ختم
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      واپس;
    }
    حالت 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      واپس;
    }
    حالت 8:    /* Integer 0 */
    حالت 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      واپس;
    }
    طےشدہ: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      ساکن ثابت u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (حرف *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      واپس;
    }
  }
  واپس;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
Unpacked_ریکارڈ *sqlite3_Vdbe_Alloc_Unpacked_ریکارڈ(
  کلید_Info *p_کلید_Info               /* Description of the record */
){
  Unpacked_ریکارڈ *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  أكد( حجم(Unpacked_ریکارڈ) + حجم(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(حجم(Unpacked_ریکارڈ)) + حجم(Mem)*(p_کلید_Info->n_کلید_میدان+1);
  p = (Unpacked_ریکارڈ *)sqlite3DbMallocRaw(p_کلید_Info->db, nByte);
  اگر( !p ) واپس 0;
  p->aMem = (Mem*)&((حرف*)p)[ROUND8P(حجم(Unpacked_ریکارڈ))];
  p->p_کلید_Info = p_کلید_Info;
  p->n_میدان = p_کلید_Info->n_کلید_میدان + 1;
  واپس p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
خالی sqlite3_Vdbe_ریکارڈ_Unpack(
  صحیح n_کلید,              /* Size of the binary record */
  ثابت خالی *p_کلید,      /* The binary record */
  Unpacked_ریکارڈ *p      /* Populate this structure before returning. */
){
  ثابت غیرعلامتی حرف *a_کلید = (ثابت غیرعلامتی حرف *)p_کلید;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  کلید_Info *p_کلید_Info = p->p_کلید_Info;

  p->default_rc = 0;
  أكد( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_کلید, szHdr);
  d = szHdr;
  u = 0;
  جب( idx<szHdr && d<=(u32)n_کلید ){
    u32 serial_type;

    idx += getVarint32(&a_کلید[idx], serial_type);
    pMem->enc = p_کلید_Info->enc;
    pMem->db = p_کلید_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_کلید[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    اگر( (++u)>=p->n_میدان ) توڑو;
    pMem++;
  }
  اگر( d>(u32)n_کلید && u ){
    أكد( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3_Vdbe_Mem_سیٹ_Null(pMem-(u<p->n_میدان));
  }
  testcase( u == p_کلید_Info->n_کلید_میدان + 1 );
  testcase( u < p_کلید_Info->n_کلید_میدان + 1 );
  أكد( u<=p_کلید_Info->n_کلید_میدان + 1 );
  p->n_میدان = u;
}

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن صحیح vdbe_ریکارڈ_Compare_Debug(
  صحیح nKey1, ثابت خالی *pKey1, /* Left key */
  ثابت Unpacked_ریکارڈ *pPKey2, /* Right key */
  صحیح desired_نتیجہ             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  صحیح i = 0;
  صحیح rc = 0;
  ثابت غیرعلامتی حرف *aKey1 = (ثابت غیرعلامتی حرف *)pKey1;
  کلید_Info *p_کلید_Info;
  Mem mem1;

  p_کلید_Info = pPKey2->p_کلید_Info;
  اگر( p_کلید_Info->db==0 ) واپس 1;
  mem1.enc = p_کلید_Info->enc;
  mem1.db = p_کلید_Info->db;
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
  اگر( szHdr1>98307 ) واپس SQLITE_CORRUPT;
  d1 = szHdr1;
  أكد( p_کلید_Info->n_All_میدان>=pPKey2->n_میدان || CORRUPT_DB );
  أكد( p_کلید_Info->a_ترتیب_دو_Flags!=0 );
  أكد( p_کلید_Info->n_کلید_میدان>0 );
  أكد( idx1<=szHdr1 || CORRUPT_DB );
  کرو{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    اگر( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      اگر( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        واپس 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      توڑو;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_کلید_Info->n_All_میدان>i ? p_کلید_Info->aColl[i] : 0);
    اگر( rc!=0 ){
      أكد( mem1.szMalloc==0 );  /* See comment below */
      اگر( (p_کلید_Info->a_ترتیب_دو_Flags[i] & KEYINFO_ترتیب_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      اگر( p_کلید_Info->a_ترتیب_دو_Flags[i] & KEYINFO_ترتیب_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      جاؤ debug_Compare_انتہا;
    }
    i++;
  }جب( idx1<szHdr1 && i<pPKey2->n_میدان );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  أكد( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debug_Compare_انتہا:
  اگر( desired_نتیجہ==0 && rc==0 ) واپس 1;
  اگر( desired_نتیجہ<0 && rc<0 ) واپس 1;
  اگر( desired_نتیجہ>0 && rc>0 ) واپس 1;
  اگر( CORRUPT_DB ) واپس 1;
  اگر( p_کلید_Info->db->mallocFailed ) واپس 1;
  واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن خالی vdbe_Assert_میدان_گنتی_Within_Limits(
  صحیح n_کلید, ثابت خالی *p_کلید,   /* The record to verify */
  ثابت کلید_Info *p_کلید_Info       /* Compare size with this KeyInfo */
){
  صحیح n_میدان = 0;
  u32 szHdr;
  u32 idx;
  u32 not_مستعمل;
  ثابت غیرعلامتی حرف *a_کلید = (ثابت غیرعلامتی حرف*)p_کلید;

  اگر( CORRUPT_DB ) واپس;
  idx = getVarint32(a_کلید, szHdr);
  أكد( n_کلید>=0 );
  أكد( szHdr<=(u32)n_کلید );
  جب( idx<szHdr ){
    idx += getVarint32(a_کلید+idx, not_مستعمل);
    n_میدان++;
  }
  أكد( n_میدان <= p_کلید_Info->n_All_میدان );
}
#ورنہ
# define vdbe_Assert_میدان_گنتی_Within_Limits(A,B,C)
#اگر_ختم

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
ساکن SQLITE_NOINLINE صحیح vdbeCompareMemStringWithEncodingChange(
  ثابت Mem *pMem1,
  ثابت Mem *pMem2,
  ثابت CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  صحیح rc;
  ثابت خالی *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_قدر_متن_2((sqlite3_قدر*)&c1, pColl->enc);
  v2 = sqlite3_قدر_متن_2((sqlite3_قدر*)&c2, pColl->enc);
  اگر( (v1==0 || v2==0) ){
    اگر( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }ورنہ{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  واپس rc;
}
ساکن صحیح vdbeCompareMemString(
  ثابت Mem *pMem1,
  ثابت Mem *pMem2,
  ثابت CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  اگر( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    واپس pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }ورنہ{
    واپس vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
ساکن صحیح isAllZero(ثابت حرف *z, صحیح n){
  صحیح i;
  برائے(i=0; i<n; i++){
    اگر( z[i] ) واپس 0;
  }
  واپس 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE صحیح sqlite3BlobCompare(ثابت Mem *pB1, ثابت Mem *pB2){
  صحیح c;
  صحیح n1 = pB1->n;
  صحیح n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  أكد( (pB1->flags & MEM_Zero)==0 || n1==0 );
  أكد( (pB2->flags & MEM_Zero)==0 || n2==0 );

  اگر( (pB1->flags|pB2->flags) & MEM_Zero ){
    اگر( pB1->flags & pB2->flags & MEM_Zero ){
      واپس pB1->u.nZero - pB2->u.nZero;
    }ورنہ اگر( pB1->flags & MEM_Zero ){
      اگر( !isAllZero(pB2->z, pB2->n) ) واپس -1;
      واپس pB1->u.nZero - n2;
    }ورنہ{
      اگر( !isAllZero(pB1->z, pB1->n) ) واپس +1;
      واپس n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  اگر( c ) واپس c;
  واپس n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#اگر defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
ساکن صحیح SQLITE_NOINLINE doubleLt(دوہرا a, دوہرا b){ واپس a<b; }
ساکن صحیح SQLITE_NOINLINE doubleEq(دوہرا a, دوہرا b){ واپس a==b; }
#اگر_ختم

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
صحیح sqlite3IntFloatCompare(i64 i, دوہرا r){
  اگر( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    واپس 1;
  }ورنہ{
    i64 y;
    اگر( r<-9223372036854775808.0 ) واپس +1;
    اگر( r>=9223372036854775808.0 ) واپس -1;
    y = (i64)r;
    اگر( i<y ) واپس -1;
    اگر( i>y ) واپس +1;
    testcase( doubleLt(((دوہرا)i),r) );
    testcase( doubleLt(r,((دوہرا)i)) );
    testcase( doubleEq(r,((دوہرا)i)) );
    واپس (((دوہرا)i)<r) ? -1 : (((دوہرا)i)>r);
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
صحیح sqlite3MemCompare(ثابت Mem *pMem1, ثابت Mem *pMem2, ثابت CollSeq *pColl){
  صحیح f1, f2;
  صحیح combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  أكد( !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem1) && !sqlite3_Vdbe_Mem_Is_صف_سیٹ(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  اگر( combined_flags&MEM_Null ){
    واپس (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  اگر( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    اگر( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      اگر( pMem1->u.i < pMem2->u.i ) واپس -1;
      اگر( pMem1->u.i > pMem2->u.i ) واپس +1;
      واپس 0;
    }
    اگر( (f1 & f2 & MEM_Real)!=0 ){
      اگر( pMem1->u.r < pMem2->u.r ) واپس -1;
      اگر( pMem1->u.r > pMem2->u.r ) واپس +1;
      واپس 0;
    }
    اگر( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      اگر( (f2&MEM_Real)!=0 ){
        واپس sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }ورنہ اگر( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        اگر( pMem1->u.i < pMem2->u.i ) واپس -1;
        اگر( pMem1->u.i > pMem2->u.i ) واپس +1;
        واپس 0;
      }ورنہ{
        واپس -1;
      }
    }
    اگر( (f1&MEM_Real)!=0 ){
      اگر( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        واپس -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }ورنہ{
        واپس -1;
      }
    }
    واپس +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  اگر( combined_flags&MEM_Str ){
    اگر( (f1 & MEM_Str)==0 ){
      واپس 1;
    }
    اگر( (f2 & MEM_Str)==0 ){
      واپس -1;
    }

    أكد( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    أكد( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    أكد( !pColl || pColl->xCmp );

    اگر( pColl ){
      واپس vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  واپس sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
ساکن i64 vdbe_ریکارڈ_Decode_Int(u32 serial_type, ثابت u8 *a_کلید){
  u32 y;
  أكد( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  انتخاب( serial_type ){
    حالت 0:
    حالت 1:
      testcase( a_کلید[0]&0x80 );
      واپس ONE_BYTE_INT(a_کلید);
    حالت 2:
      testcase( a_کلید[0]&0x80 );
      واپس TWO_BYTE_INT(a_کلید);
    حالت 3:
      testcase( a_کلید[0]&0x80 );
      واپس THREE_BYTE_INT(a_کلید);
    حالت 4: {
      testcase( a_کلید[0]&0x80 );
      y = FOUR_BYTE_UINT(a_کلید);
      واپس (i64)*(صحیح*)&y;
    }
    حالت 5: {
      testcase( a_کلید[0]&0x80 );
      واپس FOUR_BYTE_UINT(a_کلید+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_کلید);
    }
    حالت 6: {
      u64 x = FOUR_BYTE_UINT(a_کلید);
      testcase( a_کلید[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_کلید+4);
      واپس (i64)*(i64*)&x;
    }
  }

  واپس (serial_type - 8);
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
صحیح sqlite3_Vdbe_ریکارڈ_Compare_With_Skip(
  صحیح nKey1, ثابت خالی *pKey1,   /* Left key */
  Unpacked_ریکارڈ *pPKey2,         /* Right key */
  صحیح bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  صحیح i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  صحیح rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  کلید_Info *p_کلید_Info;
  ثابت غیرعلامتی حرف *aKey1 = (ثابت غیرعلامتی حرف *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  اگر( bSkip ){
    u32 s1 = aKey1[1];
    اگر( s1<0x80 ){
      idx1 = 2;
    }ورنہ{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }ورنہ{
    اگر( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }ورنہ{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  اگر( d1>(غیرعلامتی)nKey1 ){
    pPKey2->err_رمز = (u8)SQLITE_CORRUPT_BKPT;
    واپس 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  أكد( pPKey2->p_کلید_Info->n_All_میدان>=pPKey2->n_میدان
       || CORRUPT_DB );
  أكد( pPKey2->p_کلید_Info->a_ترتیب_دو_Flags!=0 );
  أكد( pPKey2->p_کلید_Info->n_کلید_میدان>0 );
  أكد( idx1<=szHdr1 || CORRUPT_DB );
  جب( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    اگر( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      اگر( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }ورنہ اگر( serial_type==0 ){
        rc = -1;
      }ورنہ اگر( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }ورنہ{
        i64 lhs = vdbe_ریکارڈ_Decode_Int(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        اگر( lhs<rhs ){
          rc = -1;
        }ورنہ اگر( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    ورنہ اگر( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      اگر( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }ورنہ اگر( serial_type==0 ){
        rc = -1;
      }ورنہ{
        اگر( serial_type==7 ){
          اگر( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }ورنہ اگر( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }ورنہ اگر( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }ورنہ{
            أكد( rc==0 );
          }
        }ورنہ{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    ورنہ اگر( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      اگر( serial_type<12 ){
        rc = -1;
      }ورنہ اگر( !(serial_type & 0x01) ){
        rc = +1;
      }ورنہ{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(غیرعلامتی)nKey1 );
        testcase( (d1+mem1.n+1)==(غیرعلامتی)nKey1 );
        اگر( (d1+mem1.n) > (غیرعلامتی)nKey1
         || (p_کلید_Info = pPKey2->p_کلید_Info)->n_All_میدان<=i
        ){
          pPKey2->err_رمز = (u8)SQLITE_CORRUPT_BKPT;
          واپس 0;                /* Corruption */
        }ورنہ اگر( p_کلید_Info->aColl[i] ){
          mem1.enc = p_کلید_Info->enc;
          mem1.db = p_کلید_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (حرف*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_کلید_Info->aColl[i], &pPKey2->err_رمز
          );
        }ورنہ{
          صحیح nCmp = MIN(mem1.n, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          اگر( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    ورنہ اگر( pRhs->flags & MEM_Blob ){
      أكد( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      اگر( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }ورنہ{
        صحیح nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(غیرعلامتی)nKey1 );
        testcase( (d1+nStr+1)==(غیرعلامتی)nKey1 );
        اگر( (d1+nStr) > (غیرعلامتی)nKey1 ){
          pPKey2->err_رمز = (u8)SQLITE_CORRUPT_BKPT;
          واپس 0;                /* Corruption */
        }ورنہ اگر( pRhs->flags & MEM_Zero ){
          اگر( !isAllZero((ثابت حرف*)&aKey1[d1],nStr) ){
            rc = 1;
          }ورنہ{
            rc = nStr - pRhs->u.nZero;
          }
        }ورنہ{
          صحیح nCmp = MIN(nStr, pRhs->n);
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          اگر( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    ورنہ{
      serial_type = aKey1[idx1];
      اگر( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        أكد( rc==0 );
      }ورنہ{
        rc = 1;
      }
    }

    اگر( rc!=0 ){
      صحیح ترتیب_دو_Flags = pPKey2->p_کلید_Info->a_ترتیب_دو_Flags[i];
      اگر( ترتیب_دو_Flags ){
        اگر( (ترتیب_دو_Flags & KEYINFO_ترتیب_BIGNULL)==0
         || ((ترتیب_دو_Flags & KEYINFO_ترتیب_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      أكد( vdbe_ریکارڈ_Compare_Debug(nKey1, pKey1, pPKey2, rc) );
      أكد( mem1.szMalloc==0 );  /* See comment below */
      واپس rc;
    }

    i++;
    اگر( i==pPKey2->n_میدان ) توڑو;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    اگر( d1>(غیرعلامتی)nKey1 ) توڑو;
    idx1 += sqlite3VarintLen(serial_type);
    اگر( idx1>=(غیرعلامتی)szHdr1 ){
      pPKey2->err_رمز = (u8)SQLITE_CORRUPT_BKPT;
      واپس 0;  /* Corrupt index */
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
       || vdbe_ریکارڈ_Compare_Debug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_کلید_Info->db->mallocFailed
  );
  pPKey2->eq_نظر_آیا = 1;
  واپس pPKey2->default_rc;
}
صحیح sqlite3_Vdbe_ریکارڈ_Compare(
  صحیح nKey1, ثابت خالی *pKey1,   /* Left key */
  Unpacked_ریکارڈ *pPKey2          /* Right key */
){
  واپس sqlite3_Vdbe_ریکارڈ_Compare_With_Skip(nKey1, pKey1, pPKey2, 0);
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
ساکن صحیح vdbe_ریکارڈ_Compare_Int(
  صحیح nKey1, ثابت خالی *pKey1, /* Left key */
  Unpacked_ریکارڈ *pPKey2        /* Right key */
){
  ثابت u8 *a_کلید = &((ثابت u8*)pKey1)[*(ثابت u8*)pKey1 & 0x3F];
  صحیح serial_type = ((ثابت u8*)pKey1)[1];
  صحیح res;
  i64 v;
  i64 lhs;

  vdbe_Assert_میدان_گنتی_Within_Limits(nKey1, pKey1, pPKey2->p_کلید_Info);
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
  اگر( (u32)(serial_type-1)<=5 ){
    ساکن ثابت u8 a_سرکاؤ[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_کلید)) >> a_سرکاؤ[serial_type];
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
    أكد( 0<=(i64)sqlite3Get8byte(a_کلید) || lhs<0 );
    testcase( lhs<0 );
  }ورنہ اگر( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }ورنہ{
    واپس sqlite3_Vdbe_ریکارڈ_Compare(nKey1, pKey1, pPKey2);
  }

  أكد( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  اگر( v>lhs ){
    res = pPKey2->r1;
  }ورنہ اگر( v<lhs ){
    res = pPKey2->r2;
  }ورنہ اگر( pPKey2->n_میدان>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3_Vdbe_ریکارڈ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
  }ورنہ{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eq_نظر_آیا = 1;
  }

  أكد( vdbe_ریکارڈ_Compare_Debug(nKey1, pKey1, pPKey2, res) );
  واپس res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
ساکن صحیح vdbe_ریکارڈ_Compare_String(
  صحیح nKey1, ثابت خالی *pKey1, /* Left key */
  Unpacked_ریکارڈ *pPKey2        /* Right key */
){
  ثابت u8 *aKey1 = (ثابت u8*)pKey1;
  صحیح serial_type;
  صحیح res;

  أكد( pPKey2->aMem[0].flags & MEM_Str );
  أكد( pPKey2->aMem[0].n == pPKey2->n );
  أكد( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_میدان_گنتی_Within_Limits(nKey1, pKey1, pPKey2->p_کلید_Info);
  serial_type = (علامتی حرف)(aKey1[1]);

vrcs_restart:
  اگر( serial_type<12 ){
    اگر( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      اگر( serial_type>=12 ) جاؤ vrcs_restart;
      أكد( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }ورنہ اگر( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }ورنہ{
    صحیح nCmp;
    صحیح nStr;
    صحیح szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    اگر( (szHdr + nStr) > nKey1 ){
      pPKey2->err_رمز = (u8)SQLITE_CORRUPT_BKPT;
      واپس 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    اگر( res>0 ){
      res = pPKey2->r2;
    }ورنہ اگر( res<0 ){
      res = pPKey2->r1;
    }ورنہ{
      res = nStr - pPKey2->n;
      اگر( res==0 ){
        اگر( pPKey2->n_میدان>1 ){
          res = sqlite3_Vdbe_ریکارڈ_Compare_With_Skip(nKey1, pKey1, pPKey2, 1);
        }ورنہ{
          res = pPKey2->default_rc;
          pPKey2->eq_نظر_آیا = 1;
        }
      }ورنہ اگر( res>0 ){
        res = pPKey2->r2;
      }ورنہ{
        res = pPKey2->r1;
      }
    }
  }

  أكد( vdbe_ریکارڈ_Compare_Debug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_کلید_Info->db->mallocFailed
  );
  واپس res;
}

/*
** Return a pointer to an sqlite3VdbeRecordCompare() compatible function
** suitable for comparing serialized records to the unpacked record passed
** as the only argument.
*/
ریکارڈ_Compare sqlite3_Vdbe_ڈھونڈو_Compare(Unpacked_ریکارڈ *p){
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
  أكد( p->p_کلید_Info->a_ترتیب_دو_Flags!=0 );
  اگر( p->p_کلید_Info->n_All_میدان<=13 ){
    صحیح flags = p->aMem[0].flags;
    اگر( p->p_کلید_Info->a_ترتیب_دو_Flags[0] ){
      اگر( p->p_کلید_Info->a_ترتیب_دو_Flags[0] & KEYINFO_ترتیب_BIGNULL ){
        واپس sqlite3_Vdbe_ریکارڈ_Compare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }ورنہ{
      p->r1 = -1;
      p->r2 = 1;
    }
    اگر( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      واپس vdbe_ریکارڈ_Compare_Int;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    اگر( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_ثنائی(p->p_کلید_Info->aColl[0])
    ){
      أكد( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      واپس vdbe_ریکارڈ_Compare_String;
    }
  }

  واپس sqlite3_Vdbe_ریکارڈ_Compare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
صحیح sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_کلید = 0;
  صحیح rc;
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
  n_Cell_کلید = sqlite3_Btree_Payload_سائز(pCur);
  أكد( (n_Cell_کلید & SQLITE_MAX_U32)==(u64)n_Cell_کلید );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_انحراف(pCur, (u32)n_Cell_کلید, &m);
  اگر( rc ){
    واپس rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  أكد( m.n>=0 );
  اگر( unlikely(szHdr<3 || szHdr>(غیرعلامتی)m.n) ){
    جاؤ idx_rowid_corruption;
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
  اگر( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    جاؤ idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  اگر( unlikely((u32)m.n<szHdr+lenRowid) ){
    جاؤ idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  واپس SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  واپس SQLITE_CORRUPT_BKPT;
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
صحیح sqlite3_Vdbe_Idx_کلید_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  Unpacked_ریکارڈ *pUnpacked,       /* Unpacked version of key */
  صحیح *res                         /* Write the comparison result here */
){
  i64 n_Cell_کلید = 0;
  صحیح rc;
  BtCursor *pCur;
  Mem m;

  أكد( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  أكد( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_کلید = sqlite3_Btree_Payload_سائز(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  اگر( n_Cell_کلید<=0 || n_Cell_کلید>0x7fffffff ){
    *res = 0;
    واپس SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3_Vdbe_Mem_From_Btree_Zero_انحراف(pCur, (u32)n_Cell_کلید, &m);
  اگر( rc ){
    واپس rc;
  }
  *res = sqlite3_Vdbe_ریکارڈ_Compare_With_Skip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  واپس SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
خالی sqlite3_Vdbe_سیٹ_Changes(sqlite3 *db, i64 nChange){
  أكد( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_کل_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
خالی sqlite3_Vdbe_گنتی_Changes(Vdbe *v){
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
خالی sqlite3ExpirePreparedStatements(sqlite3 *db, صحیح i_رمز){
  Vdbe *p;
  برائے(p = db->pVdbe; p; p=p->pVNext){
    p->expired = i_رمز+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  واپس v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  واپس v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_قدر *sqlite3_Vdbe_Get_Bound_قدر(Vdbe *v, صحیح iVar, u8 aff){
  أكد( iVar>0 );
  اگر( v ){
    Mem *pMem = &v->aVar[iVar-1];
    أكد( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_اندرونی_Func)!=0 );
    اگر( 0==(pMem->flags & MEM_Null) ){
      sqlite3_قدر *pRet = sqlite3_قدر_New(v->db);
      اگر( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_قدر_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      واپس pRet;
    }
  }
  واپس 0;
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
خالی sqlite3VdbeReprepareOnBind(Vdbe *v, صحیح iVar, صحیح bSmallint){
  u32 m;
  أكد( iVar>0 );
  أكد( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_اندرونی_Func)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  اگر( bSmallint ){
    اگر( (v->expmask & m)==0 ) v->smimask |= m;
  }ورنہ{
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
ساکن صحیح vdbe_Skip_میدان(
  Bitmask نقاب,                   /* Mask of indexed expression fields */
  صحیح i_ستون,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  صحیح bIntegrity                  /* True if running PRAGMA integrity_check */
){
#تعریف_کرو BTREE_ULPDISTORTION 2
  اگر( i_ستون>=BMS || (نقاب & MASKBIT(i_ستون))==0 ) واپس 0;
  اگر( bIntegrity==0 ) واپس 1;
  اگر( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    حافظہ_نقل(&m1,&pMem1->u.r,8);
    حافظہ_نقل(&m2,&pMem2->u.r,8);
    اگر( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      واپس 1;
    }
  }
  واپس 0;
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
ساکن صحیح vdbe_Is_Matching_اشاریہ_کلید(
  BtCursor *pCur,            /* Cursor open on index */
  صحیح bInt,                  /* True for integrity_check-style search */
  Bitmask نقاب,              /* Mask of columns to skip */
  Unpacked_ریکارڈ *p,         /* Index key being deleted */
  صحیح *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  صحیح rc = SQLITE_OK;

  memset(&m, 0, حجم(m));
  m.enc = p->p_کلید_Info->enc;
  m.db = p->p_کلید_Info->db;
  nRec = sqlite3_Btree_Payload_سائز(pCur);
  اگر( nRec>0x7fffffff ){
    واپس SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  اگر( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }ورنہ{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  اگر( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    اگر( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }ورنہ{
      صحیح res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      صحیح ii = 0;                 /* Iterator variable */

      صحیح n_ستون = p->p_کلید_Info->n_All_میدان;
      برائے(ii=0; ii<n_ستون && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        صحیح nSerial = 0;

        اگر( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          توڑو;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        اگر( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }ورنہ{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          اگر( vdbe_Skip_میدان(نقاب, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_کلید_Info->aColl[ii]);
            اگر( res!=0 ) توڑو;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  واپس rc;
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
صحیح sqlite3_Vdbe_ڈھونڈو_اشاریہ_کلید(
  BtCursor *pCur, 
  اشاریہ *pIdx,
  Unpacked_ریکارڈ *p, 
  صحیح *pRes,
  صحیح bIntegrity
){
#تعریف_کرو BTREE_FDK_RANGE 10
  صحیح n_قدم = 0;
  صحیح res = 1;
  صحیح rc = SQLITE_OK;
  صحیح ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask نقاب = 0;
  برائے(ii=0; ii<MIN(pIdx->n_عمود, BMS); ii++){
    صحیح i_ستون = pIdx->ai_عمود[ii];
    اگر( (i_ستون==XN_اظہاریہ)
     || (i_ستون>=0 && (pIdx->p_جدول->a_ستون[i_ستون].ستون_Flags & COLFLAG_VIRTUAL))
    ){
      نقاب |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  اگر( نقاب!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    برائے(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3_Btree_پچھلا(pCur, 0);
    }
    اگر( rc==SQLITE_مکمل ){
      rc = sqlite3_Btree_پہلا(pCur, &res);
      n_قدم = -1;
    }ورنہ{
      n_قدم = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    جب( sqlite3BtreeCursorIsValidNN(pCur) ){
      برائے(ii=0; rc==SQLITE_OK && (ii<n_قدم || n_قدم<0); ii++){
        rc = vdbe_Is_Matching_اشاریہ_کلید(pCur, bIntegrity, نقاب, p, &res);
        اگر( res==0 || rc!=SQLITE_OK ) توڑو;
        rc = sqlite3_Btree_اگلا(pCur, 0);
      }
      اگر( rc==SQLITE_مکمل ){
        rc = SQLITE_OK;
        أكد( res!=0 );
      }
      اگر( n_قدم<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) توڑو;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      n_قدم = -1;
      rc = sqlite3_Btree_پہلا(pCur, &res);
    }
  }

  *pRes = res;
  واپس rc;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
صحیح sqlite3_Not_خالص_Func(sqlite3_context *pCtx){
  ثابت VdbeOp *pOp;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( pCtx->pVdbe==0 ) واپس 1;
#اگر_ختم
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  اگر( pOp->opcode==OP_خالص_Func ){
    ثابت حرف *zContext;
    حرف *z_پیغام;
    اگر( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }ورنہ اگر( pOp->p5 & NC_Gen_ستون ){
      zContext = "a generated column";
    }ورنہ{
      zContext = "an index";
    }
    z_پیغام = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_نتیجہ_error(pCtx, z_پیغام, -1);
    sqlite3_free(z_پیغام);
    واپس 0;
  }
  واپس 1;
}
#اگر_ختم /* SQLITE_OMIT_DATETIME_FUNCS */

#اگر defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
صحیح sqlite3_Cursor_Range_Hint_اظہاریہ_Check(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_رجسٹر ){
    أكد( (pWalker->u.aMem[p_اظہاریہ->i_جدول].flags & MEM_Undefined)==0 );
  }
  واپس WRC_Continue;
}
#اگر_ختم /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
خالی sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  اگر( pVtab->z_Err_پیغام ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_پیغام);
    p->z_Err_پیغام = sqlite3DbStrDup(db, pVtab->z_Err_پیغام);
    sqlite3_free(pVtab->z_Err_پیغام);
    pVtab->z_Err_پیغام = 0;
  }
}
#اگر_ختم /* SQLITE_OMIT_VIRTUALTABLE */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
ساکن خالی vdbeFreeUnpacked(sqlite3 *db, صحیح n_میدان, Unpacked_ریکارڈ *p){
  أكد( db!=0 );
  اگر( p ){
    صحیح i;
    برائے(i=0; i<n_میدان; i++){
      Mem *pMem = &p->aMem[i];
      اگر( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
خالی sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  صحیح op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  ثابت حرف *zDb,                /* Database name */
  جدول *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  صحیح iReg,                       /* Register for new.* record */
  صحیح iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  ثابت حرف *zTbl = pTab->zName;
#اگر_تعریف_شدہ SQLITE_DEBUG
  صحیح n_Real_ستون;
  اگر( pTab->tabFlags & TF_WithoutRowid ){
    n_Real_ستون = sqlite3_Primary_کلید_اشاریہ(pTab)->n_عمود;
  }ورنہ اگر( pTab->tabFlags & TF_HasVirtual ){
    n_Real_ستون = pTab->nNVCol;
  }ورنہ{
    n_Real_ستون = pTab->n_ستون;
  }
#اگر_ختم

  أكد( db->pPreUpdate==0 );
  memset(&preupdate, 0, حجم(PreUpdate));
  اگر( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
  }ورنہ{
    اگر( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }ورنہ{
      iKey2 = iKey1;
    }
  }

  أكد( pCsr!=0 );
  أكد( pCsr->eCurType==CURTYPE_BTREE );
  أكد( pCsr->n_میدان==n_Real_ستون
       || (pCsr->n_میدان==n_Real_ستون+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (کلید_Info*)&preupdate.u_کلید;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_کلید_میدان = pTab->n_ستون;
  preupdate.pKeyinfo->a_ترتیب_دو_Flags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.a_ریکارڈ);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_کلید_میدان+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_کلید_میدان+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  اگر( preupdate.aNew ){
    صحیح i;
    برائے(i=0; i<pCsr->n_میدان; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  اگر( preupdate.apDflt ){
    صحیح i;
    برائے(i=0; i<pTab->n_ستون; i++){
      sqlite3_قدر_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

#اگر_تعریف_شدہ SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
ثابت حرف *sqlite3VdbeFuncName(ثابت sqlite3_context *pCtx){
  أكد( pCtx!=0 );
  أكد( pCtx->pFunc!=0 );
  واپس pCtx->pFunc->zName;
}
#اگر_ختم /* SQLITE_ENABLE_PERCENTILE */
