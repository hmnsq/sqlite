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
#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"

/* Forward references */
अटल व्योम freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
अटल व्योम vdbeFreeOpArray(sqlite3 *, Op *, पूर्णांक);

/*
** Create a new virtual database engine.
*/
Vdbe *sqlite3VdbeCreate(Parse *pParse){
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = sqlite3DbMallocRawNN(db, माप(Vdbe) );
  अगर( p==0 ) वापस 0;
  स_रखो(&p->aOp, 0, माप(Vdbe)-दुरत्व(Vdbe,aOp));
  p->db = db;
  अगर( db->pVdbe ){
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  p->pParse = pParse;
  pParse->pVdbe = p;
  निश्चित( pParse->aLabel==0 );
  निश्चित( pParse->nLabel==0 );
  निश्चित( p->nOpAlloc==0 );
  निश्चित( pParse->szOpAlloc==0 );
  sqlite3VdbeAddOp2(p, OP_Init, 0, 1);
  वापस p;
}

/*
** Return the Parse object that owns a Vdbe object.
*/
Parse *sqlite3VdbeParser(Vdbe *p){
  वापस p->pParse;
}

/*
** Change the error string stored in Vdbe.zErrMsg
*/
व्योम sqlite3VdbeError(Vdbe *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  sqlite3DbFree(p->db, p->z_Err_संदेश);
  बहु_शुरू(ap, zFormat);
  p->z_Err_संदेश = sqlite3VMPrintf(p->db, zFormat, ap);
  बहु_पूर्ण(ap);
}

/*
** Remember the SQL string for a prepared statement.
*/
व्योम sqlite3VdbeSetSql(Vdbe *p, स्थिर अक्षर *z, पूर्णांक n, u8 prepFlags){
  अगर( p==0 ) वापस;
  p->prepFlags = prepFlags;
  अगर( (prepFlags & SQLITE_PREPARE_SAVESQL)==0 ){
    p->expmask = 0;
    p->smimask = 0;
  }
  निश्चित( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

#अगर_घोषित SQLITE_ENABLE_NORMALIZE
/*
** Add a new element to the Vdbe->pDblStr list.
*/
व्योम sqlite3VdbeAddDblquoteStr(sqlite3 *db, Vdbe *p, स्थिर अक्षर *z){
  अगर( p ){
    पूर्णांक n = sqlite3Strlen30(z);
    DblquoteStr *pStr = sqlite3DbMallocRawNN(db,
                            माप(*pStr)+n+1-माप(pStr->z));
    अगर( pStr ){
      pStr->p_अग्र_Str = p->pDblStr;
      p->pDblStr = pStr;
      स_नकल(pStr->z, z, n+1);
    }
  }
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_NORMALIZE
/*
** zId of length nId is a double-quoted identifier.  Check to see if
** that identifier is really used as a string literal.
*/
पूर्णांक sqlite3VdbeUsesDoubleQuotedString(
  Vdbe *pVdbe,            /* The prepared statement */
  स्थिर अक्षर *zId         /* The double-quoted identifier, already dequoted */
){
  DblquoteStr *pStr;
  निश्चित( zId!=0 );
  अगर( pVdbe->pDblStr==0 ) वापस 0;
  क्रम(pStr=pVdbe->pDblStr; pStr; pStr=pStr->p_अग्र_Str){
    अगर( म_भेद(zId, pStr->z)==0 ) वापस 1;
  }
  वापस 0;
}
#पूर्ण_अगर

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
व्योम sqlite3VdbeSwap(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp, **ppTmp;
  अक्षर *zTmp;
  निश्चित( pA->db==pB->db );
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
#अगर_घोषित SQLITE_ENABLE_NORMALIZE
  zTmp = pA->zNormSql;
  pA->zNormSql = pB->zNormSql;
  pB->zNormSql = zTmp;
#पूर्ण_अगर
  pB->expmask |= pA->expmask;
  pB->smimask |= pA->smimask;
  pB->prepFlags = pA->prepFlags;
  स_नकल(pB->a_गणक, pA->a_गणक, माप(pB->a_गणक));
  pB->a_गणक[SQLITE_STMTSTATUS_REPREPARE]++;
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
अटल पूर्णांक growOpArray(Vdbe *v, पूर्णांक nOp){
  VdbeOp *pNew;
  Parse *p = v->pParse;

  /* The SQLITE_TEST_REALLOC_STRESS compile-time option is designed to force
  ** more frequent reallocs and hence provide more opportunities for
  ** simulated OOM faults.  SQLITE_TEST_REALLOC_STRESS is generally used
  ** during testing only.  With SQLITE_TEST_REALLOC_STRESS grow the op array
  ** by the minimum* amount required until the size reaches 512.  Normal
  ** operation (without SQLITE_TEST_REALLOC_STRESS) is to double the current
  ** size of the op array or add 1KB of space, whichever is smaller. */
#अगर_घोषित SQLITE_TEST_REALLOC_STRESS
  sqlite3_int64 nNew = (v->nOpAlloc>=512 ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)v->nOpAlloc+nOp);
#अन्यथा
  sqlite3_int64 nNew = (v->nOpAlloc ? 2*(sqlite3_int64)v->nOpAlloc
                        : (sqlite3_int64)(1024/माप(Op)));
  UNUSED_PARAMETER(nOp);
#पूर्ण_अगर

  /* Ensure that the size of a VDBE does not grow too large */
  अगर( nNew > p->db->aLimit[SQLITE_LIMIT_VDBE_OP] ){
    sqlite3OomFault(p->db);
    वापस SQLITE_NOMEM;
  }

  निश्चित( nOp<=(पूर्णांक)(1024/माप(Op)) );
  निश्चित( nNew>=(v->nOpAlloc+nOp) );
  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew*माप(Op));
  अगर( pNew ){
    p->szOpAlloc = sqlite3_Db_Malloc_आकार(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc/माप(Op);
    v->aOp = pNew;
  }
  वापस (pNew ? SQLITE_OK : SQLITE_NOMEM_BKPT);
}

#अगर_घोषित SQLITE_DEBUG
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
अटल व्योम test_addop_breakpoint(पूर्णांक pc, Op *pOp){
  अटल u64 n = 0;
  (व्योम)pc;
  (व्योम)pOp;
  n++;
  अगर( n==LARGEST_UINT64 ) पात(); /* so that n is used, preventing a warning */
}
#पूर्ण_अगर

/*
** Slow paths for sqlite3VdbeAddOp3() and sqlite3VdbeAddOp4Int() for the
** unusual case when we need to increase the size of the Vdbe.aOp[] array
** before adding the new opcode.
*/
अटल SQLITE_NOINLINE पूर्णांक growOp3(Vdbe *p, पूर्णांक op, पूर्णांक p1, पूर्णांक p2, पूर्णांक p3){
  निश्चित( p->nOpAlloc<=p->nOp );
  अगर( growOpArray(p, 1) ) वापस 1;
  निश्चित( p->nOpAlloc>p->nOp );
  वापस sqlite3VdbeAddOp3(p, op, p1, p2, p3);
}
अटल SQLITE_NOINLINE पूर्णांक addOp4IntSlow(
  Vdbe *p,            /* Add the opcode to this VM */
  पूर्णांक op,             /* The new opcode */
  पूर्णांक p1,             /* The P1 operand */
  पूर्णांक p2,             /* The P2 operand */
  पूर्णांक p3,             /* The P3 operand */
  पूर्णांक p4              /* The P4 operand as an integer */
){
  पूर्णांक addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  अगर( p->db->mallocFailed==0 ){
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = P4_INT32;
    pOp->p4.i = p4;
  }
  वापस addr;
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
पूर्णांक sqlite3VdbeAddOp0(Vdbe *p, पूर्णांक op){
  वापस sqlite3VdbeAddOp3(p, op, 0, 0, 0);
}
पूर्णांक sqlite3VdbeAddOp1(Vdbe *p, पूर्णांक op, पूर्णांक p1){
  वापस sqlite3VdbeAddOp3(p, op, p1, 0, 0);
}
पूर्णांक sqlite3VdbeAddOp2(Vdbe *p, पूर्णांक op, पूर्णांक p1, पूर्णांक p2){
  वापस sqlite3VdbeAddOp3(p, op, p1, p2, 0);
}
पूर्णांक sqlite3VdbeAddOp3(Vdbe *p, पूर्णांक op, पूर्णांक p1, पूर्णांक p2, पूर्णांक p3){
  पूर्णांक i;
  VdbeOp *pOp;

  i = p->nOp;
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  निश्चित( op>=0 && op<0xff );
  अगर( p->nOpAlloc<=i ){
    वापस growOp3(p, op, p1, p2, p3);
  }
  निश्चित( p->aOp!=0 );
  p->nOp++;
  pOp = &p->aOp[i];
  निश्चित( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;

  /* Replicate this logic in sqlite3VdbeAddOp4Int()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
  अगर( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#पूर्ण_अगर
#अगर_घोषित SQLITE_VDBE_COVERAGE
  pOp->i_Src_पंक्ति = 0;
#पूर्ण_अगर
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp4Int() */

  वापस i;
}
पूर्णांक sqlite3VdbeAddOp4Int(
  Vdbe *p,            /* Add the opcode to this VM */
  पूर्णांक op,             /* The new opcode */
  पूर्णांक p1,             /* The P1 operand */
  पूर्णांक p2,             /* The P2 operand */
  पूर्णांक p3,             /* The P3 operand */
  पूर्णांक p4              /* The P4 operand as an integer */
){
  पूर्णांक i;
  VdbeOp *pOp;

  i = p->nOp;
  अगर( p->nOpAlloc<=i ){
    वापस addOp4IntSlow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];
  निश्चित( pOp!=0 );
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = P4_INT32;

  /* Replicate this logic in sqlite3VdbeAddOp3()
  ** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv   */
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
  pOp->zComment = 0;
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_STMT_SCANSTATUS) || defined(VDBE_PROFILE)
  pOp->nExec = 0;
  pOp->nCycle = 0;
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
  अगर( p->db->flags & SQLITE_VdbeAddopTrace ){
    sqlite3VdbePrintOp(0, i, &p->aOp[i]);
    test_addop_breakpoint(i, &p->aOp[i]);
  }
#पूर्ण_अगर
#अगर_घोषित SQLITE_VDBE_COVERAGE
  pOp->i_Src_पंक्ति = 0;
#पूर्ण_अगर
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ** Replicate in sqlite3VdbeAddOp3() */

  वापस i;
}

/* Generate an opcode that loads a 64-bit integer into register iDest
*/
पूर्णांक sqlite3VdbeAddInt64(Vdbe *p, पूर्णांक iDest, i64 iVal){
  वापस sqlite3VdbeAddOp3(p, OP_Int64, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate an opcode that loads a 64-floating point value into register iDest.
*/
पूर्णांक sqlite3VdbeAddDouble(Vdbe *p, पूर्णांक iDest, द्विगुन rVal){
  i64 iVal;
  स_नकल(&iVal,&rVal,8);
  वापस sqlite3VdbeAddOp3(p, OP_Real, LOWER32(iVal), iDest, UPPER32(iVal));
}

/* Generate code for an unconditional jump to instruction iDest
*/
पूर्णांक sqlite3VdbeGoto(Vdbe *p, पूर्णांक iDest){
  वापस sqlite3VdbeAddOp3(p, OP_Goto, 0, iDest, 0);
}

/* Generate code to cause the string zStr to be loaded into
** register iDest
*/
पूर्णांक sqlite3VdbeLoadString(Vdbe *p, पूर्णांक iDest, स्थिर अक्षर *zStr){
  वापस sqlite3VdbeAddOp4(p, OP_String8, 0, iDest, 0, zStr, 0);
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
व्योम sqlite3VdbeMultiLoad(Vdbe *p, पूर्णांक iDest, स्थिर अक्षर *zTypes, ...){
  बहु_सूची ap;
  पूर्णांक i;
  अक्षर c;
  बहु_शुरू(ap, zTypes);
  क्रम(i=0; (c = zTypes[i])!=0; i++){
    अगर( c=='s' ){
      स्थिर अक्षर *z = बहु_तर्क(ap, स्थिर अक्षर*);
      sqlite3VdbeAddOp4(p, z==0 ? OP_Null : OP_String8, 0, iDest+i, 0, z, 0);
    }अन्यथा अगर( c=='i' ){
      sqlite3VdbeAddOp2(p, OP_Integer, बहु_तर्क(ap, पूर्णांक), iDest+i);
    }अन्यथा{
      जाओ skip_op_resultrow;
    }
  }
  sqlite3VdbeAddOp2(p, OP_परिणाम_Row, iDest, i);
skip_op_resultrow:
  बहु_पूर्ण(ap);
}

/*
** Add an opcode that includes the p4 value as a pointer.
*/
पूर्णांक sqlite3VdbeAddOp4(
  Vdbe *p,            /* Add the opcode to this VM */
  पूर्णांक op,             /* The new opcode */
  पूर्णांक p1,             /* The P1 operand */
  पूर्णांक p2,             /* The P2 operand */
  पूर्णांक p3,             /* The P3 operand */
  स्थिर अक्षर *zP4,    /* The P4 operand */
  पूर्णांक p4type          /* P4 operand type */
){
  पूर्णांक addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  वापस addr;
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
पूर्णांक sqlite3VdbeAddFunctionCall(
  Parse *pParse,        /* Parsing context */
  पूर्णांक p1,               /* Constant argument mask */
  पूर्णांक p2,               /* First argument register */
  पूर्णांक p3,               /* Register into which results are written */
  पूर्णांक nArg,             /* Number of argument */
  स्थिर FuncDef *pFunc, /* The function to be invoked */
  पूर्णांक eCallCtx          /* Calling context */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णांक addr;
  sqlite3_context *pCtx;
  निश्चित( v );
  pCtx = sqlite3DbMallocRawNN(pParse->db, SZ_CONTEXT(nArg));
  अगर( pCtx==0 ){
    निश्चित( pParse->db->mallocFailed );
    freeEphemeralFunction(pParse->db, (FuncDef*)pFunc);
    वापस 0;
  }
  pCtx->pOut = 0;
  pCtx->pFunc = (FuncDef*)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3VdbeAddOp4(v, eCallCtx ? OP_PureFunc : OP_Function,
                           p1, p2, p3, (अक्षर*)pCtx, P4_FUNCCTX);
  sqlite3VdbeChangeP5(v, eCallCtx & NC_SelfRef);
  sqlite3MayAbort(pParse);
  वापस addr;
}

#अगर_अघोषित SQLITE_OMIT_EXPLAIN
/*
** Return the address of the current EXPLAIN QUERY PLAN baseline.
** 0 means "none".
*/
पूर्णांक sqlite3VdbeExplainParent(Parse *pParse){
  VdbeOp *pOp;
  अगर( pParse->addrExplain==0 ) वापस 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  वापस pOp->p2;
}

/*
** Set a debugger breakpoint on the following routine in order to
** monitor the EXPLAIN QUERY PLAN code generation.
*/
#अगर defined(SQLITE_DEBUG)
व्योम sqlite3ExplainBreakpoint(स्थिर अक्षर *z1, स्थिर अक्षर *z2){
  (व्योम)z1;
  (व्योम)z2;
}
#पूर्ण_अगर

/*
** Add a new OP_Explain opcode.
**
** If the bPush flag is true, then make this opcode the parent for
** subsequent Explains until sqlite3VdbeExplainPop() is called.
*/
पूर्णांक sqlite3VdbeExplain(Parse *pParse, u8 b_धकेलो, स्थिर अक्षर *zFmt, ...){
  पूर्णांक addr = 0;
#अगर !defined(SQLITE_DEBUG)
  /* Always include the OP_Explain opcodes if SQLITE_DEBUG is defined.
  ** But omit them (for performance) during production builds */
  अगर( pParse->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#पूर्ण_अगर
  {
    अक्षर *z_संदेश;
    Vdbe *v;
    बहु_सूची ap;
    पूर्णांक iThis;
    बहु_शुरू(ap, zFmt);
    z_संदेश = sqlite3VMPrintf(pParse->db, zFmt, ap);
    बहु_पूर्ण(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3VdbeAddOp4(v, OP_Explain, iThis, pParse->addrExplain, 0,
                      z_संदेश, P4_DYNAMIC);
    sqlite3ExplainBreakpoint(b_धकेलो?"PUSH":"", sqlite3VdbeGetLastOp(v)->p4.z);
    अगर( b_धकेलो){
      pParse->addrExplain = iThis;
    }
    sqlite3VdbeScanStatus(v, iThis, -1, -1, 0, 0);
  }
  वापस addr;
}

/*
** Pop the EXPLAIN QUERY PLAN stack one level.
*/
व्योम sqlite3_Vdbe_Explain_निकालो(Parse *pParse){
  sqlite3ExplainBreakpoint("POP", 0);
  pParse->addrExplain = sqlite3VdbeExplainParent(pParse);
}
#पूर्ण_अगर /* SQLITE_OMIT_EXPLAIN */

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
व्योम sqlite3VdbeAddParseSchemaOp(Vdbe *p, पूर्णांक iDb, अक्षर *zWhere, u16 p5){
  पूर्णांक j;
  निश्चित( p5==0 || zWhere==0 );
  sqlite3VdbeAddOp4(p, OP_ParseSchema, iDb, 0, 0, zWhere, P4_DYNAMIC);
  sqlite3VdbeChangeP5(p, p5);
  क्रम(j=0; j<p->db->nDb; j++) sqlite3VdbeUsesBtree(p, j);
  sqlite3MayAbort(p->pParse);
}

/* Insert the end of a co-routine
*/
व्योम sqlite3VdbeEndCoroutine(Vdbe *v, पूर्णांक regYield){
  sqlite3VdbeAddOp1(v, OP_EndCoroutine, regYield);

  /* Clear the temporary register cache, thereby ensuring that each
  ** co-routine has its own independent set of registers, because co-routines
  ** might expect their registers to be preserved across an OP_Yield, and
  ** that could cause problems if two or more co-routines are using the same
  ** temporary register.
  */
  v->pParse->n_अस्थायी_Reg = 0;
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
पूर्णांक sqlite3VdbeMakeLabel(Parse *pParse){
  वापस --pParse->nLabel;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
अटल SQLITE_NOINLINE व्योम resizeResolveLabel(Parse *p, Vdbe *v, पूर्णांक j){
  पूर्णांक n_New_आकार = 25 - p->nLabel;
  p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                     n_New_आकार*माप(p->aLabel[0]));
  अगर( p->aLabel==0 ){
    p->nLabelAlloc = 0;
  }अन्यथा{
#अगर_घोषित SQLITE_DEBUG
    पूर्णांक i;
    क्रम(i=p->nLabelAlloc; i<n_New_आकार; i++) p->aLabel[i] = -1;
#पूर्ण_अगर
    अगर( n_New_आकार>=100 && (n_New_आकार/100)>(p->nLabelAlloc/100) ){
      sqlite3ProgressCheck(p);
    }
    p->nLabelAlloc = n_New_आकार;
    p->aLabel[j] = v->nOp;
  }
}
व्योम sqlite3VdbeResolveLabel(Vdbe *v, पूर्णांक x){
  Parse *p = v->pParse;
  पूर्णांक j = ADDR(x);
  निश्चित( v->eVdbeState==VDBE_INIT_STATE );
  निश्चित( j<-p->nLabel );
  निश्चित( j>=0 );
#अगर_घोषित SQLITE_DEBUG
  अगर( p->db->flags & SQLITE_VdbeAddopTrace ){
    म_लिखो("RESOLVE LABEL %d to %d\n", x, v->nOp);
  }
#पूर्ण_अगर
  अगर( p->nLabelAlloc + p->nLabel < 0 ){
    resizeResolveLabel(p,v,j);
  }अन्यथा{
    निश्चित( p->aLabel[j]==(-1) ); /* Labels may only be resolved once */
    p->aLabel[j] = v->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
व्योम sqlite3VdbeRunOnlyOnce(Vdbe *p){
  sqlite3VdbeAddOp2(p, OP_Expire, 1, 1);
}

/*
** Mark the VDBE as one that can be run multiple times.
*/
व्योम sqlite3VdbeReusable(Vdbe *p){
  पूर्णांक i;
  क्रम(i=1; ALWAYS(i<p->nOp); i++){
    अगर( ALWAYS(p->aOp[i].opcode==OP_Expire) ){
      p->aOp[1].opcode = OP_Noop;
      अवरोध;
    }
  }
}

#अगर_घोषित SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

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
प्रकार काष्ठा VdbeOpIter VdbeOpIter;
काष्ठा VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  SubProgram **apSub;        /* Array of subprograms */
  पूर्णांक nSub;                  /* Number of entries in apSub */
  पूर्णांक iAddr;                 /* Address of next instruction to return */
  पूर्णांक iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
अटल Op *op_Iter_अग्र(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  पूर्णांक nOp;

  अगर( p->iSub<=p->nSub ){

    अगर( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }अन्यथा{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    निश्चित( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    अगर( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
 
    अगर( pRet->p4type==P4_SUBPROGRAM ){
      i64 nByte = (1+(u64)p->nSub)*माप(SubProgram*);
      पूर्णांक j;
      क्रम(j=0; j<p->nSub; j++){
        अगर( p->apSub[j]==pRet->p4.pProgram ) अवरोध;
      }
      अगर( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        अगर( !p->apSub ){
          pRet = 0;
        }अन्यथा{
          p->apSub[p->nSub++] = pRet->p4.pProgram;
        }
      }
    }
  }

  वापस pRet;
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
पूर्णांक sqlite3VdbeAssertMayAbort(Vdbe *v, पूर्णांक mayAbort){
  पूर्णांक hasAbort = 0;
  पूर्णांक has_Fk_गणक = 0;
  पूर्णांक hasCreateTable = 0;
  पूर्णांक has_Create_सूचकांक = 0;
  पूर्णांक hasInitCoroutine = 0;
  Op *pOp;
  VdbeOpIter sIter;

  अगर( v==0 ) वापस 0;
  स_रखो(&sIter, 0, माप(sIter));
  sIter.v = v;

  जबतक( (pOp = op_Iter_अग्र(&sIter))!=0 ){
    पूर्णांक opcode = pOp->opcode;
    अगर( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename
     || opcode==OP_VDestroy
     || opcode==OP_VCreate
     || opcode==OP_ParseSchema
     || opcode==OP_Function || opcode==OP_PureFunc
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull)
      && ((pOp->p1)!=SQLITE_OK && pOp->p2==OE_Abort))
     || (opcode==OP_MustBeInt && pOp->p2==0)
    ){
      hasAbort = 1;
      अवरोध;
    }
    अगर( opcode==OP_CreateBtree && pOp->p3==BTREE_INTKEY ) hasCreateTable = 1;
    अगर( mayAbort ){
      /* hasCreateIndex may also be set for some DELETE statements that use
      ** OP_Clear. So this routine may end up returning true in the case
      ** where a "DELETE FROM tbl" has a statement-journal but does not
      ** require one. This is not so bad - it is an inefficiency, not a bug. */
      अगर( opcode==OP_CreateBtree && pOp->p3==BTREE_BLOBKEY ) has_Create_सूचकांक = 1;
      अगर( opcode==OP_Clear ) has_Create_सूचकांक = 1;
    }
    अगर( opcode==OP_InitCoroutine ) hasInitCoroutine = 1;
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
    अगर( opcode==OP_Fk_गणक && pOp->p1==0 && pOp->p2==1 ){
      has_Fk_गणक = 1;
    }
#पूर्ण_अगर
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occurred.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  वापस ( v->db->mallocFailed || hasAbort==mayAbort || has_Fk_गणक
        || (hasCreateTable && hasInitCoroutine) || has_Create_सूचकांक
  );
}
#पूर्ण_अगर /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

#अगर_घोषित SQLITE_DEBUG
/*
** Increment the nWrite counter in the VDBE if the cursor is not an
** ephemeral cursor, or if the cursor argument is NULL.
*/
व्योम sqlite3_Vdbe_Incr_Write_गणक(Vdbe *p, VdbeCursor *pC){
  अगर( pC==0
   || (pC->eCurType!=CURTYPE_SORTER
       && pC->eCurType!=CURTYPE_PSEUDO
       && !pC->isEphemeral)
  ){
    p->nWrite++;
  }
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
/*
** Assert if an Abort at this point in time might result in a corrupt
** database.
*/
व्योम sqlite3VdbeAssertAbortable(Vdbe *p){
  निश्चित( p->nWrite==0 || p->usesStmtJournal );
}
#पूर्ण_अगर

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
अटल व्योम resolve_P2_मूल्यावली(Vdbe *p, पूर्णांक *pMaxVtabArgs){
  पूर्णांक nMaxVtabArgs = *pMaxVtabArgs;
  Op *pOp;
  Parse *pParse = p->pParse;
  पूर्णांक *aLabel = pParse->aLabel;

  निश्चित( pParse->db->mallocFailed==0 ); /* tag-20230419-1 */
  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp-1];
  निश्चित( p->aOp[0].opcode==OP_Init );
  जबतक( 1 /* Loop terminates when it reaches the OP_Init opcode */ ){
    /* Only JUMP opcodes and the short list of special opcodes in the switch
    ** below need to be considered.  The mkopcodeh.tcl generator script groups
    ** all these opcodes together near the front of the opcode list.  Skip
    ** any opcode that does not need processing by virtual of the fact that
    ** it is larger than SQLITE_MX_JUMP_OPCODE, as a performance optimization.
    */
    अगर( pOp->opcode<=SQLITE_MX_JUMP_OPCODE ){
      /* NOTE: Be sure to update mkopcodeh.tcl when adding or removing
      ** cases from this switch! */
      चयन( pOp->opcode ){
        हाल OP_Transaction: {
          अगर( pOp->p2!=0 ) p->readOnly = 0;
          /* no break */ deliberate_fall_through
        }
        हाल OP_AutoCommit:
        हाल OP_Savepoint: {
          p->bIsReader = 1;
          अवरोध;
        }
#अगर_अघोषित SQLITE_OMIT_WAL
        हाल OP_Checkpoint:
#पूर्ण_अगर
        हाल OP_Vacuum:
        हाल OP_JournalMode: {
          p->readOnly = 0;
          p->bIsReader = 1;
          अवरोध;
        }
        हाल OP_Init: {
          निश्चित( pOp->p2>=0 );
          जाओ resolve_p2_मूल्यावली_loop_exit;
        }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
        हाल OP_VUpdate: {
          अगर( pOp->p2>nMaxVtabArgs ) nMaxVtabArgs = pOp->p2;
          अवरोध;
        }
        हाल OP_VFilter: {
          पूर्णांक n;
          /* The instruction immediately prior to VFilter will be an
          ** OP_Integer that sets the "argc" value for the VFilter.  See
          ** the code where OP_VFilter is generated at tag-20250207a. */
          निश्चित( (pOp - p->aOp) >= 3 );
          निश्चित( pOp[-1].opcode==OP_Integer );
          निश्चित( pOp[-1].p2==pOp->p3+1 );
          n = pOp[-1].p1;
          अगर( n>nMaxVtabArgs ) nMaxVtabArgs = n;
          /* Fall through into the default case */
          /* no break */ deliberate_fall_through
        }
#पूर्ण_अगर
        शेष: {
          अगर( pOp->p2<0 ){
            /* The mkopcodeh.tcl script has so arranged things that the only
            ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
            ** have non-negative values for P2. */
            निश्चित( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 );
            निश्चित( ADDR(pOp->p2)<-pParse->nLabel );
            निश्चित( aLabel!=0 );  /* True because of tag-20230419-1 */
            pOp->p2 = aLabel[ADDR(pOp->p2)];
          }

          /* OPFLG_JUMP opcodes never have P2==0, though OPFLG_JUMP0 opcodes
          ** might */
          निश्चित( pOp->p2>0 
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP0)!=0 );

          /* Jumps never go off the end of the bytecode array */
          निश्चित( pOp->p2<p->nOp
                  || (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)==0 );
          अवरोध;
        }
      }
      /* The mkopcodeh.tcl script has so arranged things that the only
      ** non-jump opcodes less than SQLITE_MX_JUMP_CODE are guaranteed to
      ** have non-negative values for P2. */
      निश्चित( (sqlite3OpcodeProperty[pOp->opcode]&OPFLG_JUMP)==0 || pOp->p2>=0);
    }
    निश्चित( pOp>p->aOp );   
    pOp--;
  }
resolve_p2_मूल्यावली_loop_exit:
  अगर( aLabel ){
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *pMaxVtabArgs = nMaxVtabArgs;
  निश्चित( p->bIsReader!=0 || DbMaskAllZero(p->btreeMask) );
}

#अगर_घोषित SQLITE_DEBUG
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
व्योम sqlite3VdbeNoJumpsOutsideSubrtn(
  Vdbe *v,          /* The byte-code program under construction */
  पूर्णांक iFirst,       /* First opcode of the subroutine */
  पूर्णांक iLast,        /* Last opcode of the subroutine */
  पूर्णांक iRetReg       /* Subroutine return address register */
){
  VdbeOp *pOp;
  Parse *pParse;
  पूर्णांक i;
  sqlite3_str *pErr = 0;
  निश्चित( v!=0 );
  pParse = v->pParse;
  निश्चित( pParse!=0 );
  अगर( pParse->nErr ) वापस;
  निश्चित( iLast>=iFirst );
  निश्चित( iLast<v->nOp );
  pOp = &v->aOp[iFirst];
  क्रम(i=iFirst; i<=iLast; i++, pOp++){
    अगर( (sqlite3OpcodeProperty[pOp->opcode] & OPFLG_JUMP)!=0 ){
      पूर्णांक iDest = pOp->p2;   /* Jump destination */
      अगर( iDest==0 ) जारी;
      अगर( pOp->opcode==OP_Gosub ) जारी;
      अगर( pOp->p3==20230325 && pOp->opcode==OP_NotNull ){
        /* This is a deliberately taken illegal branch.  tag-20230325-2 */
        जारी;
      }
      अगर( iDest<0 ){
        पूर्णांक j = ADDR(iDest);
        निश्चित( j>=0 );
        अगर( j>=-pParse->nLabel || pParse->aLabel[j]<0 ){
          जारी;
        }
        iDest = pParse->aLabel[j];
      }
      अगर( iDest<iFirst || iDest>iLast ){
        पूर्णांक j = iDest;
        क्रम(; j<v->nOp; j++){
          VdbeOp *pX = &v->aOp[j];
          अगर( pX->opcode==OP_Return ){
            अगर( pX->p1==iRetReg ) अवरोध;
            जारी;
          }
          अगर( pX->opcode==OP_Noop ) जारी;
          अगर( pX->opcode==OP_Explain ) जारी;
          अगर( pErr==0 ){
            pErr = sqlite3_str_new(0);
          }अन्यथा{
            sqlite3_str_appendchar(pErr, 1, '\n');
          }
          sqlite3_str_appendf(pErr,
              "Opcode at %d jumps to %d which is outside the "
              "subroutine at %d..%d",
              i, iDest, iFirst, iLast);
          अवरोध;
        }
      }
    }
  }
  अगर( pErr ){
    अक्षर *zErr = sqlite3_str_finish(pErr);
    sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_INTERNAL, OE_Abort, 0, zErr, 0);
    sqlite3_free(zErr);
    sqlite3MayAbort(pParse);
  }
}
#पूर्ण_अगर /* SQLITE_DEBUG */

/*
** Return the address of the next instruction to be inserted.
*/
पूर्णांक sqlite3VdbeCurrentAddr(Vdbe *p){
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  वापस p->nOp;
}

/*
** Verify that at least N opcode slots are available in p without
** having to malloc for more space (except when compiled using
** SQLITE_TEST_REALLOC_STRESS).  This interface is used during testing
** to verify that certain calls to sqlite3VdbeAddOpList() can never
** fail due to a OOM fault and hence that the return value from
** sqlite3VdbeAddOpList() will always be non-NULL.
*/
#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
व्योम sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, पूर्णांक N){
  निश्चित( p->nOp + N <= p->nOpAlloc );
}
#पूर्ण_अगर

/*
** Verify that the VM passed as the only argument does not contain
** an OP_ResultRow opcode. Fail an assert() if it does. This is used
** by code in pragma.c to ensure that the implementation of certain
** pragmas comports with the flags specified in the mkpragmatab.tcl
** script.
*/
#अगर defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
व्योम sqlite3_Vdbe_Verify_No_परिणाम_Row(Vdbe *p){
  पूर्णांक i;
  क्रम(i=0; i<p->nOp; i++){
    निश्चित( p->aOp[i].opcode!=OP_परिणाम_Row );
  }
}
#पूर्ण_अगर

/*
** Generate code (a single OP_Abortable opcode) that will
** verify that the VDBE program can safely call Abort in the current
** context.
*/
#अगर defined(SQLITE_DEBUG)
व्योम sqlite3VdbeVerifyAbortable(Vdbe *p, पूर्णांक onError){
  अगर( onError==OE_Abort ) sqlite3VdbeAddOp0(p, OP_Abortable);
}
#पूर्ण_अगर

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
VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, पूर्णांक *pnOp, पूर्णांक *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  निश्चित( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  निश्चित( DbMaskAllZero(p->btreeMask) );

  resolve_P2_मूल्यावली(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  वापस aOp;
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
  पूर्णांक nOp,                     /* Number of opcodes to add */
  VdbeOpList स्थिर *aOp,       /* The opcodes to be added */
  पूर्णांक iLineno                  /* Source-file line number of first opcode */
){
  पूर्णांक i;
  VdbeOp *pOut, *pFirst;
  निश्चित( nOp>0 );
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  अगर( p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp) ){
    वापस 0;
  }
  pFirst = pOut = &p->aOp[p->nOp];
  क्रम(i=0; i<nOp; i++, aOp++, pOut++){
    pOut->opcode = aOp->opcode;
    pOut->p1 = aOp->p1;
    pOut->p2 = aOp->p2;
    निश्चित( aOp->p2>=0 );
    अगर( (sqlite3OpcodeProperty[aOp->opcode] & OPFLG_JUMP)!=0 && aOp->p2>0 ){
      pOut->p2 += p->nOp;
    }
    pOut->p3 = aOp->p3;
    pOut->p4type = P4_NOTUSED;
    pOut->p4.p = 0;
    pOut->p5 = 0;
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
    pOut->zComment = 0;
#पूर्ण_अगर
#अगर_घोषित SQLITE_VDBE_COVERAGE
    pOut->i_Src_पंक्ति = iLineno+i;
#अन्यथा
    (व्योम)iLineno;
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
    अगर( p->db->flags & SQLITE_VdbeAddopTrace ){
      sqlite3VdbePrintOp(0, i+p->nOp, &p->aOp[i+p->nOp]);
    }
#पूर्ण_अगर
  }
  p->nOp += nOp;
  वापस pFirst;
}

#अगर defined(SQLITE_ENABLE_STMT_SCANSTATUS)
/*
** Add an entry to the array of counters managed by sqlite3_stmt_scanstatus().
*/
व्योम sqlite3VdbeScanStatus(
  Vdbe *p,                        /* VM to add scanstatus() to */
  पूर्णांक addrExplain,                /* Address of OP_Explain (or 0) */
  पूर्णांक addrLoop,                   /* Address of loop counter */
  पूर्णांक addrVisit,                  /* Address of rows visited counter */
  LogEst nEst,                    /* Estimated number of output rows */
  स्थिर अक्षर *zName               /* Name of table or index being scanned */
){
  अगर( IS_STMT_SCANSTATUS(p->db) ){
    i64 nByte = (1+(i64)p->nScan) * माप(ScanStatus);
    ScanStatus *aNew;
    aNew = (ScanStatus*)sqlite3DbRealloc(p->db, p->aScan, nByte);
    अगर( aNew ){
      ScanStatus *pNew = &aNew[p->nScan++];
      स_रखो(pNew, 0, माप(ScanStatus));
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
व्योम sqlite3VdbeScanStatusRange(
  Vdbe *p,
  पूर्णांक addrExplain,
  पूर्णांक addrStart,
  पूर्णांक addrEnd
){
  अगर( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    पूर्णांक ii;
    क्रम(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      अगर( pScan->addrExplain==addrExplain ) अवरोध;
      pScan = 0;
    }
    अगर( pScan ){
      अगर( addrEnd<0 ) addrEnd = sqlite3VdbeCurrentAddr(p)-1;
      क्रम(ii=0; ii<Array_आकार(pScan->aAddrRange); ii+=2){
        अगर( pScan->aAddrRange[ii]==0 ){
          pScan->aAddrRange[ii] = addrStart;
          pScan->aAddrRange[ii+1] = addrEnd;
          अवरोध;
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
व्योम sqlite3VdbeScanStatusCounters(
  Vdbe *p,
  पूर्णांक addrExplain,
  पूर्णांक addrLoop,
  पूर्णांक addrVisit
){
  अगर( IS_STMT_SCANSTATUS(p->db) ){
    ScanStatus *pScan = 0;
    पूर्णांक ii;
    क्रम(ii=p->nScan-1; ii>=0; ii--){
      pScan = &p->aScan[ii];
      अगर( pScan->addrExplain==addrExplain ) अवरोध;
      pScan = 0;
    }
    अगर( pScan ){
      अगर( addrLoop>0 ) pScan->addrLoop = addrLoop;
      अगर( addrVisit>0 ) pScan->addrVisit = addrVisit;
    }
  }
}
#पूर्ण_अगर /* defined(SQLITE_ENABLE_STMT_SCANSTATUS) */


/*
** Change the value of the opcode, or P1, P2, P3, or P5 operands
** for a specific instruction.
*/
व्योम sqlite3VdbeChangeOpcode(Vdbe *p, पूर्णांक addr, u8 iNewOpcode){
  निश्चित( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->opcode = iNewOpcode;
}
व्योम sqlite3VdbeChangeP1(Vdbe *p, पूर्णांक addr, पूर्णांक val){
  निश्चित( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p1 = val;
}
व्योम sqlite3VdbeChangeP2(Vdbe *p, पूर्णांक addr, पूर्णांक val){
  निश्चित( addr>=0 || p->db->mallocFailed );
  sqlite3VdbeGetOp(p,addr)->p2 = val;
}
व्योम sqlite3VdbeChangeP3(Vdbe *p, पूर्णांक addr, पूर्णांक val){
  निश्चित( addr>=0 );
  sqlite3VdbeGetOp(p,addr)->p3 = val;
}
व्योम sqlite3VdbeChangeP5(Vdbe *p, u16 p5){
  निश्चित( p->nOp>0 || p->db->mallocFailed );
  अगर( p->nOp>0 ) p->aOp[p->nOp-1].p5 = p5;
}

/*
** If the previous opcode is an OP_Column that delivers results
** into register iDest, then add the OPFLAG_TYPEOFARG flag to that
** opcode.
*/
व्योम sqlite3VdbeTypeofColumn(Vdbe *p, पूर्णांक iDest){
  VdbeOp *pOp = sqlite3VdbeGetLastOp(p);
#अगर_घोषित SQLITE_DEBUG
  जबतक( pOp->opcode==OP_ReleaseReg ) pOp--;
#पूर्ण_अगर
  अगर( pOp->p3==iDest && pOp->opcode==OP_Column ){
    pOp->p5 |= OPFLAG_TYPEOFARG;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
व्योम sqlite3VdbeJumpHere(Vdbe *p, पूर्णांक addr){
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
व्योम sqlite3_Vdbe_Jump_Here_Or_निकालो_Inst(Vdbe *p, पूर्णांक addr){
  अगर( addr==p->nOp-1 ){
    निश्चित( p->aOp[addr].opcode==OP_Once
         || p->aOp[addr].opcode==OP_If
         || p->aOp[addr].opcode==OP_FkIfZero );
    निश्चित( p->aOp[addr].p4type==0 );
#अगर_घोषित SQLITE_VDBE_COVERAGE
    sqlite3VdbeGetLastOp(p)->i_Src_पंक्ति = 0;  /* Erase VdbeCoverage() macros */
#पूर्ण_अगर
    p->nOp--;
  }अन्यथा{
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephemeral, then do nothing.
*/
अटल व्योम freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  निश्चित( db!=0 );
  अगर( (pDef->funcFlags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbNNFreeNN(db, pDef);
  }
}

/*
** Delete a P4 value if necessary.
*/
अटल SQLITE_NOINLINE व्योम freeP4Mem(sqlite3 *db, Mem *p){
  अगर( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}
अटल SQLITE_NOINLINE व्योम freeP4FuncCtx(sqlite3 *db, sqlite3_context *p){
  निश्चित( db!=0 );
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}
अटल व्योम freeP4(sqlite3 *db, पूर्णांक p4type, व्योम *p4){
  निश्चित( db );
  चयन( p4type ){
    हाल P4_FUNCCTX: {
      freeP4FuncCtx(db, (sqlite3_context*)p4);
      अवरोध;
    }
    हाल P4_DYNAMIC:
    हाल P4_INTARRAY: {
      अगर( p4 ) sqlite3DbNNFreeNN(db, p4);
      अवरोध;
    }
    हाल P4_KEYINFO: {
      अगर( db->pnBytesFreed==0 ) sqlite3_कुंजी_Info_Unref((कुंजी_Info*)p4);
      अवरोध;
    }
#अगर_घोषित SQLITE_ENABLE_CURSOR_HINTS
    हाल P4_व्यंजक: {
      sqlite3_व्यंजक_Delete(db, (व्यंजक*)p4);
      अवरोध;
    }
#पूर्ण_अगर
    हाल P4_FUNCDEF: {
      freeEphemeralFunction(db, (FuncDef*)p4);
      अवरोध;
    }
    हाल P4_MEM: {
      अगर( db->pnBytesFreed==0 ){
        sqlite3_मूल्य_Free((sqlite3_मूल्य*)p4);
      }अन्यथा{
        freeP4Mem(db, (Mem*)p4);
      }
      अवरोध;
    }
    हाल P4_VTAB : {
      अगर( db->pnBytesFreed==0 ) sqlite3VtabUnlock((VTable *)p4);
      अवरोध;
    }
    हाल P4_TABLEREF: {
      अगर( db->pnBytesFreed==0 ) sqlite3DeleteTable(db, (Table*)p4);
      अवरोध;
    }
    हाल P4_SUBRTNSIG: {
      SubrtnSig *pSig = (SubrtnSig*)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      अवरोध;
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain
** nOp entries.
*/
अटल व्योम vdbeFreeOpArray(sqlite3 *db, Op *aOp, पूर्णांक nOp){
  निश्चित( nOp>=0 );
  निश्चित( db!=0 );
  अगर( aOp ){
    Op *pOp = &aOp[nOp-1];
    जबतक(1){  /* Exit via break */
      अगर( pOp->p4type <= P4_FREE_IF_LE ) freeP4(db, pOp->p4type, pOp->p4.p);
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
      sqlite3DbFree(db, pOp->zComment);
#पूर्ण_अगर    
      अगर( pOp==aOp ) अवरोध;
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
व्योम sqlite3VdbeLinkSubProgram(Vdbe *pVdbe, SubProgram *p){
  p->p_अग्र = pVdbe->pProgram;
  pVdbe->pProgram = p;
}

/*
** Return true if the given Vdbe has any SubPrograms.
*/
पूर्णांक sqlite3VdbeHasSubProgram(Vdbe *pVdbe){
  वापस pVdbe->pProgram!=0;
}

/*
** Change the opcode at addr into OP_Noop
*/
पूर्णांक sqlite3VdbeChangeToNoop(Vdbe *p, पूर्णांक addr){
  VdbeOp *pOp;
  अगर( p->db->mallocFailed ) वापस 0;
  निश्चित( addr>=0 && addr<p->nOp );
  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = P4_NOTUSED;
  pOp->p4.z = 0;
  pOp->opcode = OP_Noop;
  वापस 1;
}

/*
** If the last opcode is "op" and it is not a jump destination,
** then remove it.  Return true if and only if an opcode was removed.
*/
पूर्णांक sqlite3VdbeDeletePriorOpcode(Vdbe *p, u8 op){
  अगर( p->nOp>0 && p->aOp[p->nOp-1].opcode==op ){
    वापस sqlite3VdbeChangeToNoop(p, p->nOp-1);
  }अन्यथा{
    वापस 0;
  }
}

#अगर_घोषित SQLITE_DEBUG
/*
** Generate an OP_ReleaseReg opcode to indicate that a range of
** registers, except any identified by mask, are no longer in use.
*/
व्योम sqlite3VdbeReleaseRegisters(
  Parse *pParse,       /* Parsing context */
  पूर्णांक iFirst,          /* Index of first register to be released */
  पूर्णांक N,               /* Number of registers to release */
  u32 mask,            /* Mask of registers to NOT release */
  पूर्णांक bUndefine        /* If true, mark registers as undefined */
){
  अगर( N==0 || OptimizationDisabled(pParse->db, SQLITE_ReleaseReg) ) वापस;
  निश्चित( pParse->pVdbe );
  निश्चित( iFirst>=1 );
  निश्चित( iFirst+N-1<=pParse->nMem );
  अगर( N<=31 && mask!=0 ){
    जबतक( N>0 && (mask&1)!=0 ){
      mask >>= 1;
      iFirst++;
      N--;
    }
    जबतक( N>0 && N<=32 && (mask & MASKBIT32(N-1))!=0 ){
      mask &= ~MASKBIT32(N-1);
      N--;
    }
  }
  अगर( N>0 ){
    sqlite3VdbeAddOp3(pParse->pVdbe, OP_ReleaseReg, iFirst, N, *(पूर्णांक*)&mask);
    अगर( bUndefine ) sqlite3VdbeChangeP5(pParse->pVdbe, 1);
  }
}
#पूर्ण_अगर /* SQLITE_DEBUG */

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
अटल व्योम SQLITE_NOINLINE vdbeChangeP4Full(
  Vdbe *p,
  Op *pOp,
  स्थिर अक्षर *zP4,
  पूर्णांक n
){
  अगर( pOp->p4type ){
    निश्चित( pOp->p4type > P4_FREE_IF_LE );
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  अगर( n<0 ){
    sqlite3VdbeChangeP4(p, (पूर्णांक)(pOp - p->aOp), zP4, n);
  }अन्यथा{
    अगर( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}
व्योम sqlite3VdbeChangeP4(Vdbe *p, पूर्णांक addr, स्थिर अक्षर *zP4, पूर्णांक n){
  Op *pOp;
  sqlite3 *db;
  निश्चित( p!=0 );
  db = p->db;
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  निश्चित( p->aOp!=0 || db->mallocFailed );
  अगर( db->mallocFailed ){
    अगर( n!=P4_VTAB ) freeP4(db, n, (व्योम*)*(अक्षर**)&zP4);
    वापस;
  }
  निश्चित( p->nOp>0 );
  निश्चित( addr<p->nOp );
  अगर( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  अगर( n>=0 || pOp->p4type ){
    vdbeChangeP4Full(p, pOp, zP4, n);
    वापस;
  }
  अगर( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_TO_INT(zP4);
    pOp->p4type = P4_INT32;
  }अन्यथा अगर( zP4!=0 ){
    निश्चित( n<0 );
    pOp->p4.p = (व्योम*)zP4;
    pOp->p4type = (चिन्हित अक्षर)n;
    अगर( n==P4_VTAB ) sqlite3VtabLock((VTable*)zP4);
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
व्योम sqlite3VdbeAppendP4(Vdbe *p, व्योम *pP4, पूर्णांक n){
  VdbeOp *pOp;
  निश्चित( n!=P4_INT32 && n!=P4_VTAB );
  निश्चित( n<=0 );
  अगर( p->db->mallocFailed ){
    freeP4(p->db, n, pP4);
  }अन्यथा{
    निश्चित( pP4!=0 || n==P4_DYNAMIC );
    निश्चित( p->nOp>0 );
    pOp = &p->aOp[p->nOp-1];
    निश्चित( pOp->p4type==P4_NOTUSED );
    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

/*
** Set the P4 on the most recently added opcode to the KeyInfo for the
** index given.
*/
व्योम sqlite3_Vdbe_Set_P4_कुंजी_Info(Parse *pParse, सूचकांक *pIdx){
  Vdbe *v = pParse->pVdbe;
  कुंजी_Info *p_कुंजी_Info;
  निश्चित( v!=0 );
  निश्चित( pIdx!=0 );
  p_कुंजी_Info = sqlite3_कुंजी_Info_Of_सूचकांक(pParse, pIdx);
  अगर( p_कुंजी_Info ) sqlite3VdbeAppendP4(v, p_कुंजी_Info, P4_KEYINFO);
}

#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
/*
** Change the comment on the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
अटल व्योम vdbeVComment(Vdbe *p, स्थिर अक्षर *zFormat, बहु_सूची ap){
  निश्चित( p->nOp>0 || p->aOp==0 );
  निश्चित( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->pParse->nErr>0 );
  अगर( p->nOp ){
    निश्चित( p->aOp );
    sqlite3DbFree(p->db, p->aOp[p->nOp-1].zComment);
    p->aOp[p->nOp-1].zComment = sqlite3VMPrintf(p->db, zFormat, ap);
  }
}
व्योम sqlite3VdbeComment(Vdbe *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  अगर( p ){
    बहु_शुरू(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    बहु_पूर्ण(ap);
  }
}
व्योम sqlite3VdbeNoopComment(Vdbe *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  अगर( p ){
    sqlite3VdbeAddOp0(p, OP_Noop);
    बहु_शुरू(ap, zFormat);
    vdbeVComment(p, zFormat, ap);
    बहु_पूर्ण(ap);
  }
}
#पूर्ण_अगर  /* NDEBUG */

#अगर_घोषित SQLITE_VDBE_COVERAGE
/*
** Set the value if the iSrcLine field for the previously coded instruction.
*/
व्योम sqlite3_Vdbe_Set_पंक्ति_संख्या(Vdbe *v, पूर्णांक i_पंक्ति){
  sqlite3VdbeGetLastOp(v)->i_Src_पंक्ति = i_पंक्ति;
}
#पूर्ण_अगर /* SQLITE_VDBE_COVERAGE */

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
VdbeOp *sqlite3VdbeGetOp(Vdbe *p, पूर्णांक addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  अटल VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  निश्चित( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  अगर( p->db->mallocFailed ){
    वापस (VdbeOp*)&dummy;
  }अन्यथा{
    वापस &p->aOp[addr];
  }
}

/* Return the most recently added opcode
*/
VdbeOp *sqlite3VdbeGetLastOp(Vdbe *p){
  वापस sqlite3VdbeGetOp(p, p->nOp - 1);
}

#अगर defined(SQLITE_ENABLE_EXPLAIN_COMMENTS)
/*
** Return an integer value for one of the parameters to the opcode pOp
** determined by character c.
*/
अटल पूर्णांक translateP(अक्षर c, स्थिर Op *pOp){
  अगर( c=='1' ) वापस pOp->p1;
  अगर( c=='2' ) वापस pOp->p2;
  अगर( c=='3' ) वापस pOp->p3;
  अगर( c=='4' ) वापस pOp->p4.i;
  वापस pOp->p5;
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
अक्षर *sqlite3VdbeDisplayComment(
  sqlite3 *db,       /* Optional - Oom error reporting only */
  स्थिर Op *pOp,     /* The opcode to be commented */
  स्थिर अक्षर *zP4    /* Previously obtained value for P4 */
){
  स्थिर अक्षर *zOpName;
  स्थिर अक्षर *zSynopsis;
  पूर्णांक nOpName;
  पूर्णांक ii;
  अक्षर zAlt[50];
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_लंबाई);
  zOpName = sqlite3OpcodeName(pOp->opcode);
  nOpName = sqlite3Strlen30(zOpName);
  अगर( zOpName[nOpName+1] ){
    पूर्णांक seenCom = 0;
    अक्षर c;
    zSynopsis = zOpName + nOpName + 1;
    अगर( म_भेदन(zSynopsis,"IF ",3)==0 ){
      sqlite3_snprintf(माप(zAlt), zAlt, "if %s goto P2", zSynopsis+3);
      zSynopsis = zAlt;
    }
    क्रम(ii=0; (c = zSynopsis[ii])!=0; ii++){
      अगर( c=='P' ){
        c = zSynopsis[++ii];
        अगर( c=='4' ){
          sqlite3_str_appendall(&x, zP4);
        }अन्यथा अगर( c=='X' ){
          अगर( pOp->zComment && pOp->zComment[0] ){
            sqlite3_str_appendall(&x, pOp->zComment);
            seenCom = 1;
            अवरोध;
          }
        }अन्यथा अगर( म_भेदन(&zSynopsis[ii],"INT13",5)==0 ){
          sqlite3_str_appendf(&x,"%lld",INT32_TO_64(pOp->p1,pOp->p3));
          ii += 4;
#अगर_घोषित SQLITE_ENABLE_COLUMN_USED_MASK
        }अन्यथा अगर( म_भेदन(&zSynopsis[ii],"HEX23",5)==0 ){
          sqlite3_str_appendf(&x,"0x%llx",INT32_TO_64(pOp->p2,pOp->p3));
          ii += 4;
#पूर्ण_अगर
        }अन्यथा अगर( म_भेदन(&zSynopsis[ii],"DBL13",5)==0 ){
          i64 iVal = INT32_TO_64(pOp->p1,pOp->p3);
          द्विगुन r;
          स_नकल(&r, &iVal, 8);
          sqlite3_str_appendf(&x,"%.17g",r);
          ii += 4;
        }अन्यथा{
          पूर्णांक v1 = translateP(c, pOp);
          पूर्णांक v2;
          अगर( म_भेदन(zSynopsis+ii+1, "@P", 2)==0 ){
            ii += 3;
            v2 = translateP(zSynopsis[ii], pOp);
            अगर( म_भेदन(zSynopsis+ii+1,"+1",2)==0 ){
              ii += 2;
              v2++;
            }
            अगर( v2<2 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }अन्यथा{
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+v2-1);
            }
          }अन्यथा अगर( म_भेदन(zSynopsis+ii+1, "@NP", 3)==0 ){
            sqlite3_context *pCtx = pOp->p4.pCtx;
            अगर( pOp->p4type!=P4_FUNCCTX || pCtx->argc==1 ){
              sqlite3_str_appendf(&x, "%d", v1);
            }अन्यथा अगर( pCtx->argc>1 ){
              sqlite3_str_appendf(&x, "%d..%d", v1, v1+pCtx->argc-1);
            }अन्यथा अगर( x.accError==0 ){
              निश्चित( x.nChar>2 );
              x.nChar -= 2;
              ii++;
            }
            ii += 3;
          }अन्यथा{
            sqlite3_str_appendf(&x, "%d", v1);
            अगर( म_भेदन(zSynopsis+ii+1, "..P3", 4)==0 && pOp->p3==0 ){
              ii += 4;
            }
          }
        }
      }अन्यथा{
        sqlite3_str_appendchar(&x, 1, c);
      }
    }
    अगर( !seenCom && pOp->zComment ){
      sqlite3_str_appendf(&x, "; %s", pOp->zComment);
    }
  }अन्यथा अगर( pOp->zComment ){
    sqlite3_str_appendall(&x, pOp->zComment);
  }
  अगर( (x.accError & SQLITE_NOMEM)!=0 && db!=0 ){
    sqlite3OomFault(db);
  }
  वापस sqlite3StrAccumFinish(&x);
}
#पूर्ण_अगर /* SQLITE_ENABLE_EXPLAIN_COMMENTS */

#अगर VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS)
/*
** Translate the P4.pExpr value for an OP_CursorHint opcode into text
** that can be displayed in the P4 column of EXPLAIN output.
*/
अटल व्योम display_P4_व्यंजक(StrAccum *p, व्यंजक *p_व्यंजक){
  स्थिर अक्षर *zOp = 0;
  चयन( p_व्यंजक->op ){
    हाल TK_STRING:
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_str_appendf(p, "%Q", p_व्यंजक->u.zToken);
      अवरोध;
    हाल TK_INTEGER:
      sqlite3_str_appendf(p, "%d", p_व्यंजक->u.i_मूल्य);
      अवरोध;
    हाल TK_NULL:
      sqlite3_str_appendf(p, "NULL");
      अवरोध;
    हाल TK_REGISTER: {
      sqlite3_str_appendf(p, "r[%d]", p_व्यंजक->iTable);
      अवरोध;
    }
    हाल TK_COLUMN: {
      अगर( p_व्यंजक->iColumn<0 ){
        sqlite3_str_appendf(p, "rowid");
      }अन्यथा{
        sqlite3_str_appendf(p, "c%d", (पूर्णांक)p_व्यंजक->iColumn);
      }
      अवरोध;
    }
    हाल TK_LT:      zOp = "LT";      अवरोध;
    हाल TK_LE:      zOp = "LE";      अवरोध;
    हाल TK_GT:      zOp = "GT";      अवरोध;
    हाल TK_GE:      zOp = "GE";      अवरोध;
    हाल TK_NE:      zOp = "NE";      अवरोध;
    हाल TK_EQ:      zOp = "EQ";      अवरोध;
    हाल TK_IS:      zOp = "IS";      अवरोध;
    हाल TK_ISNOT:   zOp = "ISNOT";   अवरोध;
    हाल TK_AND:     zOp = "AND";     अवरोध;
    हाल TK_OR:      zOp = "OR";      अवरोध;
    हाल TK_PLUS:    zOp = "ADD";     अवरोध;
    हाल TK_STAR:    zOp = "MUL";     अवरोध;
    हाल TK_MINUS:   zOp = "SUB";     अवरोध;
    हाल TK_REM:     zOp = "REM";     अवरोध;
    हाल TK_BITAND:  zOp = "BITAND";  अवरोध;
    हाल TK_BITOR:   zOp = "BITOR";   अवरोध;
    हाल TK_SLASH:   zOp = "DIV";     अवरोध;
    हाल TK_LSHIFT:  zOp = "LSHIFT";  अवरोध;
    हाल TK_RSHIFT:  zOp = "RSHIFT";  अवरोध;
    हाल TK_CONCAT:  zOp = "CONCAT";  अवरोध;
    हाल TK_UMINUS:  zOp = "MINUS";   अवरोध;
    हाल TK_UPLUS:   zOp = "PLUS";    अवरोध;
    हाल TK_BITNOT:  zOp = "BITNOT";  अवरोध;
    हाल TK_NOT:     zOp = "NOT";     अवरोध;
    हाल TK_ISNULL:  zOp = "ISNULL";  अवरोध;
    हाल TK_NOTNULL: zOp = "NOTNULL"; अवरोध;

    शेष:
      sqlite3_str_appendf(p, "%s", "expr");
      अवरोध;
  }

  अगर( zOp ){
    sqlite3_str_appendf(p, "%s(", zOp);
    display_P4_व्यंजक(p, p_व्यंजक->p_बायाँ);
    अगर( p_व्यंजक->p_दायाँ ){
      sqlite3_str_append(p, ",", 1);
      display_P4_व्यंजक(p, p_व्यंजक->p_दायाँ);
    }
    sqlite3_str_append(p, ")", 1);
  }
}
#पूर्ण_अगर /* VDBE_DISPLAY_P4 && defined(SQLITE_ENABLE_CURSOR_HINTS) */


#अगर VDBE_DISPLAY_P4
/*
** Compute a string that describes the P4 parameter for an opcode.
*/
अक्षर *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp){
  अक्षर *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, SQLITE_MAX_लंबाई);
  चयन( pOp->p4type ){
    हाल P4_KEYINFO: {
      पूर्णांक j;
      कुंजी_Info *p_कुंजी_Info = pOp->p4.p_कुंजी_Info;
      निश्चित( p_कुंजी_Info->aSortFlags!=0 );
      sqlite3_str_appendf(&x, "k(%d", p_कुंजी_Info->n_कुंजी_Field);
      क्रम(j=0; j<p_कुंजी_Info->n_कुंजी_Field; j++){
        CollSeq *pColl = p_कुंजी_Info->aColl[j];
        स्थिर अक्षर *zColl = pColl ? pColl->zName : "";
        अगर( म_भेद(zColl, "BINARY")==0 ) zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s",
               (p_कुंजी_Info->aSortFlags[j] & KEYINFO_ORDER_DESC) ? "-" : "",
               (p_कुंजी_Info->aSortFlags[j] & KEYINFO_ORDER_BIGNULL)? "N." : "",
               zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      अवरोध;
    }
#अगर_घोषित SQLITE_ENABLE_CURSOR_HINTS
    हाल P4_व्यंजक: {
      display_P4_व्यंजक(&x, pOp->p4.p_व्यंजक);
      अवरोध;
    }
#पूर्ण_अगर
    हाल P4_COLLSEQ: {
      अटल स्थिर अक्षर *स्थिर encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;
      निश्चित( pColl->enc<4 );
      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName,
                          encnames[pColl->enc]);
      अवरोध;
    }
    हाल P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      अवरोध;
    }
    हाल P4_FUNCCTX: {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      अवरोध;
    }
    हाल P4_INT32: {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      अवरोध;
    }
    हाल P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      अगर( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }अन्यथा अगर( pMem->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      }अन्यथा अगर( pMem->flags & MEM_Real ){
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      }अन्यथा अगर( pMem->flags & MEM_Null ){
        zP4 = "NULL";
      }अन्यथा{
        निश्चित( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    हाल P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      अवरोध;
    }
#पूर्ण_अगर
    हाल P4_INTARRAY: {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];   /* The first element of an INTARRAY is always the
                       ** count of the number of elements to follow */
      क्रम(i=1; i<=n; i++){
        sqlite3_str_appendf(&x, "%c%u", (i==1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      अवरोध;
    }
    हाल P4_SUBPROGRAM: {
      zP4 = "program";
      अवरोध;
    }
    हाल P4_TABLE: {
      zP4 = pOp->p4.pTab->zName;
      अवरोध;
    }
    हाल P4_सूचकांक: {
      zP4 = pOp->p4.pIdx->zName;
      अवरोध;
    }
    हाल P4_SUBRTNSIG: {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      अवरोध;
    }
    शेष: {
      zP4 = pOp->p4.z;
    }
  }
  अगर( zP4 ) sqlite3_str_appendall(&x, zP4);
  अगर( (x.accError & SQLITE_NOMEM)!=0 ){
    sqlite3OomFault(db);
  }
  वापस sqlite3StrAccumFinish(&x);
}
#पूर्ण_अगर /* VDBE_DISPLAY_P4 */

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that will be use.  A mask of these databases
** is maintained in p->btreeMask.  The p->lockMask value is the subset of
** p->btreeMask of databases that will require a lock.
*/
व्योम sqlite3VdbeUsesBtree(Vdbe *p, पूर्णांक i){
  निश्चित( i>=0 && i<p->db->nDb && i<(पूर्णांक)माप(yDbMask)*8 );
  निश्चित( i<(पूर्णांक)माप(p->btreeMask)*8 );
  DbMaskSet(p->btreeMask, i);
  अगर( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    DbMaskSet(p->lockMask, i);
  }
}

#अगर !defined(SQLITE_OMIT_SHARED_CACHE)
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
व्योम sqlite3VdbeEnter(Vdbe *p){
  पूर्णांक i;
  sqlite3 *db;
  Db *aDb;
  पूर्णांक nDb;
  अगर( DbMaskAllZero(p->lockMask) ) वापस;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  क्रम(i=0; i<nDb; i++){
    अगर( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
अटल SQLITE_NOINLINE व्योम vdbeLeave(Vdbe *p){
  पूर्णांक i;
  sqlite3 *db;
  Db *aDb;
  पूर्णांक nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  क्रम(i=0; i<nDb; i++){
    अगर( i!=1 && DbMaskTest(p->lockMask,i) && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
व्योम sqlite3VdbeLeave(Vdbe *p){
  अगर( DbMaskAllZero(p->lockMask) ) वापस;  /* The common case */
  vdbeLeave(p);
}
#पूर्ण_अगर

#अगर defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
व्योम sqlite3VdbePrintOp(खाता *pOut, पूर्णांक pc, VdbeOp *pOp){
  अक्षर *zP4;
  अक्षर *zCom;
  sqlite3 dummyDb;
  अटल स्थिर अक्षर *zFormat1 = "%4d %-13s %4d %4d %4d %-13s %.2X %s\n";
  अगर( pOut==0 ) pOut = मानक_निकास;
  sqlite3BeginBenignMalloc();
  dummyDb.mallocFailed = 1;
  zP4 = sqlite3VdbeDisplayP4(&dummyDb, pOp);
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
  zCom = sqlite3VdbeDisplayComment(0, pOp, zP4);
#अन्यथा
  zCom = 0;
#पूर्ण_अगर
  /* NB:  The sqlite3OpcodeName() function is implemented by code created
  ** by the mkopcodeh.awk and mkopcodec.awk scripts which extract the
  ** information from the vdbe.c source text */
  ख_लिखो(pOut, zFormat1, pc,
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3,
      zP4 ? zP4 : "", pOp->p5,
      zCom ? zCom : ""
  );
  ख_साफ(pOut);
  sqlite3_free(zP4);
  sqlite3_free(zCom);
  sqlite3EndBenignMalloc();
}
#पूर्ण_अगर

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
अटल व्योम initMemArray(Mem *p, पूर्णांक N, sqlite3 *db, u16 flags){
  निश्चित( db!=0 );
  अगर( N>0 ){
    करो{
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;
#अगर_घोषित SQLITE_DEBUG
      p->pScopyFrom = 0;
      p->bScopy = 0;
#पूर्ण_अगर
      p++;
    }जबतक( (--N)>0 );
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
अटल व्योम releaseMemArray(Mem *p, पूर्णांक N){
  अगर( p && N ){
    Mem *pEnd = &p[N];
    sqlite3 *db = p->db;
    निश्चित( db!=0 );
    अगर( db->pnBytesFreed ){
      करो{
        अगर( p->szMalloc ) sqlite3DbFree(db, p->zMalloc);
      }जबतक( (++p)<pEnd );
      वापस;
    }
    करो{
      निश्चित( (&p[1])==pEnd || p[0].db==p[1].db );
      निश्चित( sqlite3VdbeCheckMemInvariants(p) );

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
      अगर( p->flags&(MEM_Agg|MEM_Dyn) ){
        testcase( (p->flags & MEM_Dyn)!=0 && p->xDel==sqlite3VdbeFrameMemDel );
        sqlite3VdbeMemRelease(p);
        p->flags = MEM_Undefined;
      }अन्यथा अगर( p->szMalloc ){
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = MEM_Undefined;
      }
#अगर_घोषित SQLITE_DEBUG
      अन्यथा{
        p->flags = MEM_Undefined;
      }
#पूर्ण_अगर
    }जबतक( (++p)<pEnd );
  }
}

#अगर_घोषित SQLITE_DEBUG
/*
** Verify that pFrame is a valid VdbeFrame pointer.  Return true if it is
** and false if something is wrong.
**
** This routine is intended for use inside of assert() statements only.
*/
पूर्णांक sqlite3VdbeFrameIsValid(VdbeFrame *pFrame){
  अगर( pFrame->iFrameMagic!=SQLITE_FRAME_MAGIC ) वापस 0;
  वापस 1;
}
#पूर्ण_अगर


/*
** This is a destructor on a Mem object (which is really an sqlite3_value)
** that deletes the Frame object that is attached to it as a blob.
**
** This routine does not delete the Frame right away.  It merely adds the
** frame to a list of frames to be deleted when the Vdbe halts.
*/
व्योम sqlite3VdbeFrameMemDel(व्योम *pArg){
  VdbeFrame *pFrame = (VdbeFrame*)pArg;
  निश्चित( sqlite3VdbeFrameIsValid(pFrame) );
  pFrame->pParent = pFrame->v->pDelFrame;
  pFrame->v->pDelFrame = pFrame;
}

#अगर defined(SQLITE_ENABLE_BYTECODE_VTAB) || !defined(SQLITE_OMIT_EXPLAIN)
/*
** Locate the next opcode to be displayed in EXPLAIN or EXPLAIN
** QUERY PLAN output.
**
** Return SQLITE_ROW on success.  Return SQLITE_DONE if there are no
** more opcodes to be displayed.
*/
पूर्णांक sqlite3_Vdbe_अग्र_Opcode(
  Vdbe *p,         /* The statement being explained */
  Mem *pSub,       /* Storage for keeping track of subprogram nesting */
  पूर्णांक eMode,       /* 0: normal.  1: EQP.  2:  TablesUsed */
  पूर्णांक *piPc,       /* IN/OUT: Current rowid.  Overwritten with next rowid */
  पूर्णांक *piAddr,     /* OUT: Write index into (*paOp)[] here */
  Op **paOp        /* OUT: Write the opcode array here */
){
  पूर्णांक nRow;                            /* Stop when row count reaches this */
  पूर्णांक nSub = 0;                        /* Number of sub-vdbes seen so far */
  SubProgram **apSub = 0;              /* Array of sub-vdbes */
  पूर्णांक i;                               /* Next instruction address */
  पूर्णांक rc = SQLITE_OK;                  /* Result code */
  Op *aOp = 0;                         /* Opcode array */
  पूर्णांक iPc;                             /* Rowid.  Copy of value in *piPc */

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  nRow = p->nOp;
  अगर( pSub!=0 ){
    अगर( pSub->flags&MEM_Blob ){
      /* pSub is initiallly NULL.  It is initialized to a BLOB by
      ** the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/माप(Vdbe*);
      apSub = (SubProgram **)pSub->z;
    }
    क्रम(i=0; i<nSub; i++){
      nRow += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  जबतक(1){  /* Loop exits via break */
    i = iPc++;
    अगर( i>=nRow ){
      p->rc = SQLITE_OK;
      rc = SQLITE_DONE;
      अवरोध;
    }
    अगर( i<p->nOp ){
      /* The rowid is small enough that we are still in the
      ** main program. */
      aOp = p->aOp;
    }अन्यथा{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      पूर्णांक j;
      i -= p->nOp;
      निश्चित( apSub!=0 );
      निश्चित( nSub>0 );
      क्रम(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
        निश्चित( i<apSub[j]->nOp || j+1<nSub );
      }
      aOp = apSub[j]->aOp;
    }

    /* When an OP_Program opcode is encounter (the only opcode that has
    ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
    ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
    ** has not already been seen.
    */
    अगर( pSub!=0 && aOp[i].p4type==P4_SUBPROGRAM ){
      पूर्णांक nByte = (nSub+1)*माप(SubProgram*);
      पूर्णांक j;
      क्रम(j=0; j<nSub; j++){
        अगर( apSub[j]==aOp[i].p4.pProgram ) अवरोध;
      }
      अगर( j==nSub ){
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub!=0);
        अगर( p->rc!=SQLITE_OK ){
          rc = SQLITE_ERROR;
          अवरोध;
        }
        apSub = (SubProgram **)pSub->z;
        apSub[nSub++] = aOp[i].p4.pProgram;
        MemSetTypeFlag(pSub, MEM_Blob);
        pSub->n = nSub*माप(SubProgram*);
        nRow += aOp[i].p4.pProgram->nOp;
      }
    }
    अगर( eMode==0 ) अवरोध;
#अगर_घोषित SQLITE_ENABLE_BYTECODE_VTAB
    अगर( eMode==2 ){
      Op *pOp = aOp + i;
      अगर( pOp->opcode==OP_OpenRead ) अवरोध;
      अगर( pOp->opcode==OP_OpenWrite && (pOp->p5 & OPFLAG_P2ISREG)==0 ) अवरोध;
      अगर( pOp->opcode==OP_ReopenIdx ) अवरोध;     
    }अन्यथा
#पूर्ण_अगर
    {
      निश्चित( eMode==1 );
      अगर( aOp[i].opcode==OP_Explain ) अवरोध;
      अगर( aOp[i].opcode==OP_Init && iPc>1 ) अवरोध;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_ENABLE_BYTECODE_VTAB || !SQLITE_OMIT_EXPLAIN */


/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
व्योम sqlite3VdbeFrameDelete(VdbeFrame *p){
  पूर्णांक i;
  Mem *aMem = VdbeFrameMem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->nChildMem];
  निश्चित( sqlite3VdbeFrameIsValid(p) );
  क्रम(i=0; i<p->nChildCsr; i++){
    अगर( apCsr[i] ) sqlite3VdbeFreeCursorNN(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->nChildMem);
  sqlite3_Vdbe_Delete_Aux_तथ्य(p->v->db, &p->p_Aux_तथ्य, -1, 0);
  sqlite3DbFree(p->v->db, p);
}

#अगर_अघोषित SQLITE_OMIT_EXPLAIN
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
पूर्णांक sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  पूर्णांक i;                               /* Loop counter */
  पूर्णांक rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = &p->aMem[1];             /* First Mem of result set */
  पूर्णांक bListSubprogs = (p->explain==1 || (db->flags & SQLITE_TriggerEQP)!=0);
  Op *aOp;                             /* Array of opcodes */
  Op *pOp;                             /* Current opcode */

  निश्चित( p->explain );
  निश्चित( p->eVdbeState==VDBE_RUN_STATE );
  निश्चित( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  अगर( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    sqlite3OomFault(db);
    वापस SQLITE_ERROR;
  }

  अगर( bListSubprogs ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    निश्चित( p->nMem>9 );
    pSub = &p->aMem[9];
  }अन्यथा{
    pSub = 0;
  }

  /* Figure out which opcode is next to display */
  rc = sqlite3_Vdbe_अग्र_Opcode(p, pSub, p->explain==2, &p->pc, &i, &aOp);

  अगर( rc==SQLITE_OK ){
    pOp = aOp + i;
    अगर( AtomicLoad(&db->u1.isInterrupted) ){
      p->rc = SQLITE_INTERRUPT;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    }अन्यथा{
      अक्षर *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      अगर( p->explain==2 ){
        sqlite3VdbeMemSetInt64(pMem, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+1, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p3);
        sqlite3VdbeMemSetStr(pMem+3, zP4, -1, SQLITE_UTF8, sqlite3_free);
        निश्चित( p->nResColumn==4 );
      }अन्यथा{
        sqlite3VdbeMemSetInt64(pMem+0, i);
        sqlite3VdbeMemSetStr(pMem+1, (अक्षर*)sqlite3OpcodeName(pOp->opcode),
                             -1, SQLITE_UTF8, SQLITE_STATIC);
        sqlite3VdbeMemSetInt64(pMem+2, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem+3, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem+4, pOp->p3);
        /* pMem+5 for p4 is done last */
        sqlite3VdbeMemSetInt64(pMem+6, pOp->p5);
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
        {
          अक्षर *zCom = sqlite3VdbeDisplayComment(db, pOp, zP4);
          sqlite3VdbeMemSetStr(pMem+7, zCom, -1, SQLITE_UTF8, sqlite3_free);
        }
#अन्यथा
        sqlite3VdbeMemSetNull(pMem+7);
#पूर्ण_अगर
        sqlite3VdbeMemSetStr(pMem+5, zP4, -1, SQLITE_UTF8, sqlite3_free);
        निश्चित( p->nResColumn==8 );
      }
      p->p_परिणाम_Row = pMem;
      अगर( db->mallocFailed ){
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      }अन्यथा{
        p->rc = SQLITE_OK;
        rc = SQLITE_ROW;
      }
    }
  }
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_EXPLAIN */

#अगर_घोषित SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
व्योम sqlite3VdbePrintSql(Vdbe *p){
  स्थिर अक्षर *z = 0;
  अगर( p->zSql ){
    z = p->zSql;
  }अन्यथा अगर( p->nOp>=1 ){
    स्थिर VdbeOp *pOp = &p->aOp[0];
    अगर( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
      z = pOp->p4.z;
      जबतक( sqlite3Isspace(*z) ) z++;
    }
  }
  अगर( z ) म_लिखो("SQL: [%s]\n", z);
}
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
व्योम sqlite3VdbeIOTraceSql(Vdbe *p){
  पूर्णांक nOp = p->nOp;
  VdbeOp *pOp;
  अगर( sqlite3IoTrace==0 ) वापस;
  अगर( nOp<1 ) वापस;
  pOp = &p->aOp[0];
  अगर( pOp->opcode==OP_Init && pOp->p4.z!=0 ){
    पूर्णांक i, j;
    अक्षर z[1000];
    sqlite3_snprintf(माप(z), z, "%s", pOp->p4.z);
    क्रम(i=0; sqlite3Isspace(z[i]); i++){}
    क्रम(j=0; z[i]; i++){
      अगर( sqlite3Isspace(z[i]) ){
        अगर( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }अन्यथा{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#पूर्ण_अगर /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/* An instance of this object describes bulk memory available for use
** by subcomponents of a prepared statement.  Space is allocated out
** of a ReusableSpace object by the allocSpace() routine below.
*/
काष्ठा ReusableSpace {
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
अटल व्योम *allocSpace(
  काष्ठा ReusableSpace *p,  /* Bulk memory available for allocation */
  व्योम *pBuf,               /* Pointer to a prior allocation */
  sqlite3_int64 nByte       /* Bytes of memory needed. */
){
  निश्चित( EIGHT_BYTE_ALIGNMENT(p->pSpace) );
  अगर( pBuf==0 ){
    nByte = ROUND8P(nByte);
    अगर( nByte <= p->nFree ){
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    }अन्यथा{
      p->nNeeded += nByte;
    }
  }
  निश्चित( EIGHT_BYTE_ALIGNMENT(pBuf) );
  वापस pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
व्योम sqlite3VdbeRewind(Vdbe *p){
#अगर defined(SQLITE_DEBUG)
  पूर्णांक i;
#पूर्ण_अगर
  निश्चित( p!=0 );
  निश्चित( p->eVdbeState==VDBE_INIT_STATE
       || p->eVdbeState==VDBE_READY_STATE
       || p->eVdbeState==VDBE_HALT_STATE );

  /* There should be at least one opcode.
  */
  निश्चित( p->nOp>0 );

  p->eVdbeState = VDBE_READY_STATE;

#अगर_घोषित SQLITE_DEBUG
  क्रम(i=0; i<p->nMem; i++){
    निश्चित( p->aMem[i].db==p->db );
  }
#पूर्ण_अगर
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#अगर_घोषित VDBE_PROFILE
  क्रम(i=0; i<p->nOp; i++){
    p->aOp[i].nExec = 0;
    p->aOp[i].nCycle = 0;
  }
#पूर्ण_अगर
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
व्योम sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  पूर्णांक nVar;                      /* Number of parameters */
  पूर्णांक nMem;                      /* Number of VM memory registers */
  पूर्णांक nCursor;                   /* Number of cursors required */
  पूर्णांक nArg;                      /* Max number args to xFilter or xUpdate */
  पूर्णांक n;                         /* Loop counter */
  काष्ठा ReusableSpace x;        /* Reusable bulk memory */

  निश्चित( p!=0 );
  निश्चित( p->nOp>0 );
  निश्चित( pParse!=0 );
  निश्चित( p->eVdbeState==VDBE_INIT_STATE );
  निश्चित( pParse==p->pParse );
  निश्चित( pParse->db==p->db );
  p->pVList = pParse->pVList;
  pParse->pVList =  0;
  db = p->db;
  निश्चित( db->mallocFailed==0 );
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
  अगर( nCursor==0 && nMem>0 ) nMem++;  /* Space for aMem[0] even if not used */

  /* Figure out how much reusable memory is available at the end of the
  ** opcode array.  This extra memory will be reallocated for other elements
  ** of the prepared statement.
  */
  n = ROUND8P(माप(Op)*p->nOp);             /* Bytes of opcode memory used */
  x.pSpace = &((u8*)p->aOp)[n];               /* Unused opcode memory */
  निश्चित( EIGHT_BYTE_ALIGNMENT(x.pSpace) );
  x.nFree = ROUNDDOWN8(pParse->szOpAlloc - n);  /* Bytes unused memory */
  निश्चित( x.nFree>=0 );
  निश्चित( EIGHT_BYTE_ALIGNMENT(&x.pSpace[x.nFree]) );

  resolve_P2_मूल्यावली(p, &nArg);
  p->usesStmtJournal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  अगर( pParse->explain ){
    अगर( nMem<10 ) nMem = 10;
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
  p->aMem = allocSpace(&x, 0, nMem*माप(Mem));
  p->aVar = allocSpace(&x, 0, nVar*माप(Mem));
  p->apArg = allocSpace(&x, 0, nArg*माप(Mem*));
  p->apCsr = allocSpace(&x, 0, nCursor*माप(VdbeCursor*));
  अगर( x.nNeeded ){
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    अगर( !db->mallocFailed ){
      p->aMem = allocSpace(&x, p->aMem, nMem*माप(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar*माप(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg*माप(Mem*));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor*माप(VdbeCursor*));
    }
  }
#अगर_घोषित SQLITE_DEBUG
  p->napArg = nArg;
#पूर्ण_अगर

  अगर( db->mallocFailed ){
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  }अन्यथा{
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, MEM_Null);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, MEM_Undefined);
    स_रखो(p->apCsr, 0, nCursor*माप(VdbeCursor*));
  }
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor
** happens to hold.
*/
व्योम sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  अगर( pCx ) sqlite3VdbeFreeCursorNN(p,pCx);
}
अटल SQLITE_NOINLINE व्योम freeCursorWithCache(Vdbe *p, VdbeCursor *pCx){
  VdbeTxtBlbCache *pCache = pCx->pCache;
  निश्चित( pCx->colCache );
  pCx->colCache = 0;
  pCx->pCache = 0;
  अगर( pCache->pCValue ){
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}
व्योम sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx){
  अगर( pCx->colCache ){
    freeCursorWithCache(p, pCx);
    वापस;
  }
  चयन( pCx->eCurType ){
    हाल CURTYPE_SORTER: {
      sqlite3VdbeSorterClose(p->db, pCx);
      अवरोध;
    }
    हाल CURTYPE_BTREE: {
      निश्चित( pCx->uc.pCursor!=0 );
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
    हाल CURTYPE_VTAB: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      स्थिर sqlite3_module *pModule = pVCur->pVtab->pModule;
      निश्चित( pVCur->pVtab->nRef>0 );
      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      अवरोध;
    }
#पूर्ण_अगर
  }
}

/*
** Close all cursors in the current frame.
*/
अटल व्योम closeCursorsInFrame(Vdbe *p){
  पूर्णांक i;
  क्रम(i=0; i<p->nCursor; i++){
    VdbeCursor *pC = p->apCsr[i];
    अगर( pC ){
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
पूर्णांक sqlite3VdbeFrameRestore(VdbeFrame *pFrame){
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
  sqlite3_Vdbe_Delete_Aux_तथ्य(v->db, &v->p_Aux_तथ्य, -1, 0);
  v->p_Aux_तथ्य = pFrame->p_Aux_तथ्य;
  pFrame->p_Aux_तथ्य = 0;
  वापस pFrame->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
अटल व्योम closeAllCursors(Vdbe *p){
  अगर( p->pFrame ){
    VdbeFrame *pFrame;
    क्रम(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    sqlite3VdbeFrameRestore(pFrame);
    p->pFrame = 0;
    p->nFrame = 0;
  }
  निश्चित( p->nFrame==0 );
  closeCursorsInFrame(p);
  releaseMemArray(p->aMem, p->nMem);
  जबतक( p->pDelFrame ){
    VdbeFrame *pDel = p->pDelFrame;
    p->pDelFrame = pDel->pParent;
    sqlite3VdbeFrameDelete(pDel);
  }

  /* Delete any auxdata allocations made by the VM */
  अगर( p->p_Aux_तथ्य ) sqlite3_Vdbe_Delete_Aux_तथ्य(p->db, &p->p_Aux_तथ्य, -1, 0);
  निश्चित( p->p_Aux_तथ्य==0 );
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
व्योम sqlite3_Vdbe_Set_अंक_Cols(Vdbe *p, पूर्णांक nResColumn){
  पूर्णांक n;
  sqlite3 *db = p->db;

  अगर( p->nResAlloc ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbFree(db, p->aColName);
  }
  n = nResColumn*COLNAME_N;
  p->nResColumn = p->nResAlloc = (u16)nResColumn;
  p->aColName = (Mem*)sqlite3DbMallocRawNN(db, माप(Mem)*n );
  अगर( p->aColName==0 ) वापस;
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
पूर्णांक sqlite3VdbeSetColName(
  Vdbe *p,                         /* Vdbe being configured */
  पूर्णांक idx,                         /* Index of column zName applies to */
  पूर्णांक var,                         /* One of the COLNAME_* constants */
  स्थिर अक्षर *zName,               /* Pointer to buffer containing name */
  व्योम (*xDel)(व्योम*)              /* Memory management strategy for zName */
){
  पूर्णांक rc;
  Mem *pColName;
  निश्चित( idx<p->nResAlloc );
  निश्चित( var<COLNAME_N );
  अगर( p->db->mallocFailed ){
    निश्चित( !zName || xDel!=SQLITE_DYNAMIC );
    वापस SQLITE_NOMEM_BKPT;
  }
  निश्चित( p->aColName!=0 );
  pColName = &(p->aColName[idx+var*p->nResAlloc]);
  rc = sqlite3VdbeMemSetText(pColName, zName, -1, xDel);
  निश्चित( rc!=0 || !zName || (pColName->flags&MEM_Term)!=0 );
  वापस rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the super-journal trickery.
*/
अटल पूर्णांक vdbeCommit(sqlite3 *db, Vdbe *p){
  पूर्णांक i;
  पूर्णांक nTrans = 0;  /* Number of databases with an active write-transaction
                   ** that are candidates for a two-phase commit using a
                   ** super-journal */
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक needXcommit = 0;

#अगर_घोषित SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply
  ** SQLITE_OK so p is not used.
  */
  UNUSED_PARAMETER(p);
#पूर्ण_अगर

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
  क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    अगर( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
      /* Whether or not a database might need a super-journal depends upon
      ** its journal mode (among other things).  This matrix determines which
      ** journal modes use a super-journal and which do not */
      अटल स्थिर u8 aMJNeeded[] = {
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
      अगर( db->aDb[i].safety_level!=PAGER_SYNCHRONOUS_OFF
       && aMJNeeded[sqlite3PagerGetJournalMode(pPager)]
       && sqlite3PagerIsMemdb(pPager)==0
      ){
        निश्चित( i!=1 );
        nTrans++;
      }
      rc = sqlite3PagerExclusiveLock(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  अगर( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    अगर( rc ){
      वापस SQLITE_CONSTRAINT_COMMITHOOK;
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
  अगर( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    अगर( needXcommit ){
      क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        अगर( sqlite3BtreeTxnState(pBt)>=SQLITE_TXN_WRITE ){
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    /* Do the commit only if all databases successfully complete phase 1.
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      पूर्णांक txn = sqlite3BtreeTxnState(pBt);
      अगर( txn!=SQLITE_TXN_NONE ){
        निश्चित( needXcommit || txn==SQLITE_TXN_READ );
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    अगर( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a super-journal file to ensure the transaction is
  ** committed atomically.
  */
#अगर_अघोषित SQLITE_OMIT_DISKIO
  अन्यथा{
    sqlite3_vfs *pVfs = db->pVfs;
    अक्षर *zSuper = 0;   /* File-name for the super-journal */
    अक्षर स्थिर *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 offset = 0;
    पूर्णांक res;
    पूर्णांक retry_गिनती = 0;
    पूर्णांक nMainFile;

    /* Select a super-journal file name */
    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0,zMainFile,0);
    अगर( zSuper==0 ) वापस SQLITE_NOMEM_BKPT;
    zSuper += 4;
    करो {
      u32 iRandom;
      अगर( retry_गिनती ){
        अगर( retry_गिनती>100 ){
          sqlite3_log(SQLITE_FULL, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          अवरोध;
        }अन्यथा अगर( retry_गिनती==1 ){
          sqlite3_log(SQLITE_FULL, "MJ collide: %s", zSuper);
        }
      }
      retry_गिनती++;
      sqlite3_randomness(माप(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X",
                               (iRandom>>8)&0xffffff, iRandom&0xff);
      /* The antipenultimate character of the super-journal name must
      ** be "9" to avoid name collisions when using 8+3 filenames. */
      निश्चित( zSuper[sqlite3Strlen30(zSuper)-3]=='9' );
      sqlite3FileSuffix3(zMainFile, zSuper);
      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    }जबतक( rc==SQLITE_OK && res );
    अगर( rc==SQLITE_OK ){
      /* Open the super-journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_SUPER_JOURNAL, 0
      );
    }
    अगर( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      वापस rc;
    }

    /* Write the name of each database file in the transaction into the new
    ** super-journal file. If an error occurs at this point close
    ** and delete the super-journal file. All the individual journal files
    ** still have 'null' as the super-journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    क्रम(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      अगर( sqlite3BtreeTxnState(pBt)==SQLITE_TXN_WRITE ){
        अक्षर स्थिर *zFile = sqlite3BtreeGetJournalname(pBt);
        अगर( zFile==0 ){
          जारी;  /* Ignore TEMP and :memory: databases */
        }
        निश्चित( zFile[0]!=0 );
        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile)+1,offset);
        offset += sqlite3Strlen30(zFile)+1;
        अगर( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper-4);
          वापस rc;
        }
      }
    }

    /* Sync the super-journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    अगर( 0==(sqlite3OsDeviceCharacteristics(pSuperJrnl)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_NORMAL))
    ){
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper-4);
      वापस rc;
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
    क्रम(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      अगर( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);
    निश्चित( rc!=SQLITE_BUSY );
    अगर( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zSuper-4);
      वापस rc;
    }

    /* Delete the super-journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper-4);
    zSuper = 0;
    अगर( rc ){
      वापस rc;
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
    क्रम(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      अगर( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3EndBenignMalloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#पूर्ण_अगर

  वापस rc;
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
#अगर_अघोषित न_संशोधन
अटल व्योम checkActiveVdbeCnt(sqlite3 *db){
  Vdbe *p;
  पूर्णांक cnt = 0;
  पूर्णांक nWrite = 0;
  पूर्णांक nRead = 0;
  p = db->pVdbe;
  जबतक( p ){
    अगर( sqlite3_stmt_busy((sqlite3_stmt*)p) ){
      cnt++;
      अगर( p->readOnly==0 ) nWrite++;
      अगर( p->bIsReader ) nRead++;
    }
    p = p->pVNext;
  }
  निश्चित( cnt==db->nVdbeActive );
  निश्चित( nWrite==db->nVdbeWrite );
  निश्चित( nRead==db->nVdbeRead );
}
#अन्यथा
#घोषणा checkActiveVdbeCnt(x)
#पूर्ण_अगर

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
अटल SQLITE_NOINLINE पूर्णांक vdbeCloseStatement(Vdbe *p, पूर्णांक eOp){
  sqlite3 *स्थिर db = p->db;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक i;
  स्थिर पूर्णांक iSavepoint = p->iStatement-1;

  निश्चित( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
  निश्चित( db->nStatement>0 );
  निश्चित( p->iStatement==(db->nStatement+db->nSavepoint) );

  क्रम(i=0; i<db->nDb; i++){
    पूर्णांक rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    अगर( pBt ){
      अगर( eOp==SAVEPOINT_ROLLBACK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      अगर( rc2==SQLITE_OK ){
        rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
      }
      अगर( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  अगर( rc==SQLITE_OK ){
    अगर( eOp==SAVEPOINT_ROLLBACK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
    }
    अगर( rc==SQLITE_OK ){
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
    }
  }

  /* If the statement transaction is being rolled back, also restore the
  ** database handles deferred constraint counter to the value it had when
  ** the statement transaction was opened.  */
  अगर( eOp==SAVEPOINT_ROLLBACK ){
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  वापस rc;
}
पूर्णांक sqlite3VdbeCloseStatement(Vdbe *p, पूर्णांक eOp){
  अगर( p->db->nStatement && p->iStatement ){
    वापस vdbeCloseStatement(p, eOp);
  }
  वापस SQLITE_OK;
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
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
अटल SQLITE_NOINLINE पूर्णांक vdbeFkError(Vdbe *p){
  p->rc = SQLITE_CONSTRAINT_FOREIGNKEY;
  p->errorAction = OE_Abort;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  अगर( (p->prepFlags & SQLITE_PREPARE_SAVESQL)==0 ) वापस SQLITE_ERROR;
  वापस SQLITE_CONSTRAINT_FOREIGNKEY;
}
पूर्णांक sqlite3VdbeCheckFkImmediate(Vdbe *p){
  अगर( p->nFkConstraint==0 ) वापस SQLITE_OK;
  वापस vdbeFkError(p);
}
पूर्णांक sqlite3VdbeCheckFkDeferred(Vdbe *p){
  sqlite3 *db = p->db;
  अगर( (db->nDeferredCons+db->nDeferredImmCons)==0 ) वापस SQLITE_OK;
  वापस vdbeFkError(p);
}
#पूर्ण_अगर

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
पूर्णांक sqlite3VdbeHalt(Vdbe *p){
  पूर्णांक rc;                         /* Used to store transient return codes */
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

  निश्चित( p->eVdbeState==VDBE_RUN_STATE );
  अगर( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }
  closeAllCursors(p);
  checkActiveVdbeCnt(db);

  /* No commit or rollback needed if the program never started or if the
  ** SQL statement does not read or write a database file.  */
  अगर( p->bIsReader ){
    पूर्णांक mrc;   /* Primary error code from p->rc */
    पूर्णांक eStatementOp = 0;
    पूर्णांक isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    अगर( p->rc ){
      mrc = p->rc & 0xff;
      isSpecialError = mrc==SQLITE_NOMEM
                    || mrc==SQLITE_IOERR
                    || mrc==SQLITE_INTERRUPT
                    || mrc==SQLITE_FULL;
    }अन्यथा{
      mrc = isSpecialError = 0;
    }
    अगर( isSpecialError ){
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
      अगर( !p->readOnly || mrc!=SQLITE_INTERRUPT ){
        अगर( (mrc==SQLITE_NOMEM || mrc==SQLITE_FULL) && p->usesStmtJournal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }अन्यथा{
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
    अगर( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
      (व्योम)sqlite3VdbeCheckFkImmediate(p);
    }

    /* If the auto-commit flag is set and this is the only active writer
    ** VM, then we do either a commit or rollback of the current transaction.
    **
    ** Note: This block also runs if one of the special errors handled
    ** above has occurred.
    */
    अगर( !sqlite3VtabInSync(db)
     && db->autoCommit
     && db->nVdbeWrite==(p->readOnly==0)
    ){
      अगर( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFkDeferred(p);
        अगर( rc!=SQLITE_OK ){
          अगर( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            वापस SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT_FOREIGNKEY;
        }अन्यथा अगर( db->flags & SQLITE_CorruptRdOnly ){
          rc = SQLITE_CORRUPT;
          db->flags &= ~SQLITE_CorruptRdOnly;
        }अन्यथा{
          /* The auto-commit flag is true, the vdbe program was successful
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        अगर( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          वापस SQLITE_BUSY;
        }अन्यथा अगर( rc!=SQLITE_OK ){
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        }अन्यथा{
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)SQLITE_DeferFKs;
          sqlite3CommitInternalChanges(db);
        }
      }अन्यथा अगर( p->rc==SQLITE_SCHEMA && db->nVdbeActive>1 ){
        p->nChange = 0;
      }अन्यथा{
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    }अन्यथा अगर( eStatementOp==0 ){
      अगर( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }अन्यथा अगर( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }अन्यथा{
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
    अगर( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      अगर( rc ){
        अगर( p->rc==SQLITE_OK || (p->rc&0xff)==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->z_Err_संदेश);
          p->z_Err_संदेश = 0;
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
    अगर( p->changeCntOn ){
      अगर( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3VdbeSetChanges(db, p->nChange);
      }अन्यथा{
        sqlite3VdbeSetChanges(db, 0);
      }
      p->nChange = 0;
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  db->nVdbeActive--;
  अगर( !p->readOnly ) db->nVdbeWrite--;
  अगर( p->bIsReader ) db->nVdbeRead--;
  निश्चित( db->nVdbeActive>=db->nVdbeRead );
  निश्चित( db->nVdbeRead>=db->nVdbeWrite );
  निश्चित( db->nVdbeWrite>=0 );
  p->eVdbeState = VDBE_HALT_STATE;
  checkActiveVdbeCnt(db);
  अगर( db->mallocFailed ){
    p->rc = SQLITE_NOMEM_BKPT;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked()
  ** to invoke any required unlock-notify callbacks.
  */
  अगर( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  निश्चित( db->nVdbeActive>0 || db->autoCommit==0 || db->nStatement==0 );
  वापस (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
व्योम sqlite3_Vdbe_Reset_Step_परिणाम(Vdbe *p){
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
पूर्णांक sqlite3VdbeTransferError(Vdbe *p){
  sqlite3 *db = p->db;
  पूर्णांक rc = p->rc;
  अगर( p->z_Err_संदेश ){
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    अगर( db->pErr==0 ) db->pErr = sqlite3_मूल्य_New(db);
    sqlite3_मूल्य_Set_Str(db->pErr, -1, p->z_Err_संदेश, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3EndBenignMalloc();
    db->bBenignMalloc--;
  }अन्यथा अगर( db->pErr ){
    sqlite3_मूल्य_Set_Null(db->pErr);
  }
  db->errCode = rc;
  db->errByteOffset = -1;
  वापस rc;
}

#अगर_घोषित SQLITE_ENABLE_SQLLOG
/*
** If an SQLITE_CONFIG_SQLLOG hook is registered and the VM has been run,
** invoke it.
*/
अटल व्योम vdbeInvokeSqllog(Vdbe *v){
  अगर( sqlite3GlobalConfig.xSqllog && v->rc==SQLITE_OK && v->zSql && v->pc>=0 ){
    अक्षर *zExpanded = sqlite3VdbeExpandSql(v, v->zSql);
    निश्चित( v->db->init.busy==0 );
    अगर( zExpanded ){
      sqlite3GlobalConfig.xSqllog(
          sqlite3GlobalConfig.pSqllogArg, v->db, zExpanded, 1
      );
      sqlite3DbFree(v->db, zExpanded);
    }
  }
}
#अन्यथा
# define vdbeInvokeSqllog(x)
#पूर्ण_अगर

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
पूर्णांक sqlite3VdbeReset(Vdbe *p){
#अगर defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  पूर्णांक i;
#पूर्ण_अगर

  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  अगर( p->eVdbeState==VDBE_RUN_STATE ) sqlite3VdbeHalt(p);

  /* If the VDBE has been run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  अगर( p->pc>=0 ){
    vdbeInvokeSqllog(p);
    अगर( db->pErr || p->z_Err_संदेश ){
      sqlite3VdbeTransferError(p);
    }अन्यथा{
      db->errCode = p->rc;
    }
  }

  /* Reset register contents and reclaim error message memory.
  */
#अगर_घोषित SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  अगर( p->apCsr ) क्रम(i=0; i<p->nCursor; i++) निश्चित( p->apCsr[i]==0 );
  अगर( p->aMem ){
    क्रम(i=0; i<p->nMem; i++) निश्चित( p->aMem[i].flags==MEM_Undefined );
  }
#पूर्ण_अगर
  अगर( p->z_Err_संदेश ){
    sqlite3DbFree(db, p->z_Err_संदेश);
    p->z_Err_संदेश = 0;
  }
  p->p_परिणाम_Row = 0;
#अगर_घोषित SQLITE_DEBUG
  p->nWrite = 0;
#पूर्ण_अगर

  /* Save profiling information from this VDBE run.
  */
#अगर_घोषित VDBE_PROFILE
  {
    खाता *out = ख_खोलो("vdbe_profile.out", "a");
    अगर( out ){
      ख_लिखो(out, "---- ");
      क्रम(i=0; i<p->nOp; i++){
        ख_लिखो(out, "%02x", p->aOp[i].opcode);
      }
      ख_लिखो(out, "\n");
      अगर( p->zSql ){
        अक्षर c, pc = 0;
        ख_लिखो(out, "-- ");
        क्रम(i=0; (c = p->zSql[i])!=0; i++){
          अगर( pc=='\n' ) ख_लिखो(out, "-- ");
          अ_दो(c, out);
          pc = c;
        }
        अगर( pc!='\n' ) ख_लिखो(out, "\n");
      }
      क्रम(i=0; i<p->nOp; i++){
        अक्षर zHdr[100];
        i64 cnt = p->aOp[i].nExec;
        i64 cycles = p->aOp[i].nCycle;
        sqlite3_snprintf(माप(zHdr), zHdr, "%6u %12llu %8llu ",
           cnt,
           cycles,
           cnt>0 ? cycles/cnt : 0
        );
        ख_लिखो(out, "%s", zHdr);
        sqlite3VdbePrintOp(out, i, &p->aOp[i]);
      }
      ख_बंद(out);
    }
  }
#पूर्ण_अगर
  वापस p->rc & db->errMask;
}

/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
पूर्णांक sqlite3VdbeFinalize(Vdbe *p){
  पूर्णांक rc = SQLITE_OK;
  निश्चित( VDBE_RUN_STATE>VDBE_READY_STATE );
  निश्चित( VDBE_HALT_STATE>VDBE_READY_STATE );
  निश्चित( VDBE_INIT_STATE<VDBE_READY_STATE );
  अगर( p->eVdbeState>=VDBE_READY_STATE ){
    rc = sqlite3VdbeReset(p);
    निश्चित( (rc & p->db->errMask)==rc );
  }
  sqlite3VdbeDelete(p);
  वापस rc;
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
व्योम sqlite3_Vdbe_Delete_Aux_तथ्य(sqlite3 *db, Aux_तथ्य **pp, पूर्णांक iOp, पूर्णांक mask){
  जबतक( *pp ){
    Aux_तथ्य *pAux = *pp;
    अगर( (iOp<0)
     || (pAux->iAuxOp==iOp
          && pAux->iAuxArg>=0
          && (pAux->iAuxArg>31 || !(mask & MASKBIT32(pAux->iAuxArg))))
    ){
      testcase( pAux->iAuxArg==31 );
      अगर( pAux->xDeleteAux ){
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->p_अग्र_Aux;
      sqlite3DbFree(db, pAux);
    }अन्यथा{
      pp= &pAux->p_अग्र_Aux;
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
अटल व्योम sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p){
  SubProgram *pSub, *p_अग्र;
  निश्चित( db!=0 );
  निश्चित( p->db==0 || p->db==db );
  अगर( p->aColName ){
    releaseMemArray(p->aColName, p->nResAlloc*COLNAME_N);
    sqlite3DbNNFreeNN(db, p->aColName);
  }
  क्रम(pSub=p->pProgram; pSub; pSub=p_अग्र){
    p_अग्र = pSub->p_अग्र;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  अगर( p->eVdbeState!=VDBE_INIT_STATE ){
    releaseMemArray(p->aVar, p->nVar);
    अगर( p->pVList ) sqlite3DbNNFreeNN(db, p->pVList);
    अगर( p->pFree ) sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  अगर( p->zSql ) sqlite3DbNNFreeNN(db, p->zSql);
#अगर_घोषित SQLITE_ENABLE_NORMALIZE
  sqlite3DbFree(db, p->zNormSql);
  {
    DblquoteStr *pThis, *pNxt;
    क्रम(pThis=p->pDblStr; pThis; pThis=pNxt){
      pNxt = pThis->p_अग्र_Str;
      sqlite3DbFree(db, pThis);
    }
  }
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
  {
    पूर्णांक i;
    क्रम(i=0; i<p->nScan; i++){
      sqlite3DbFree(db, p->aScan[i].zName);
    }
    sqlite3DbFree(db, p->aScan);
  }
#पूर्ण_अगर
}

/*
** Delete an entire VDBE.
*/
व्योम sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  निश्चित( p!=0 );
  db = p->db;
  निश्चित( db!=0 );
  निश्चित( sqlite3_mutex_held(db->mutex) );
  sqlite3VdbeClearObject(db, p);
  अगर( db->pnBytesFreed==0 ){
    निश्चित( p->ppVPrev!=0 );
    *p->ppVPrev = p->pVNext;
    अगर( p->pVNext ){
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
पूर्णांक SQLITE_NOINLINE sqlite3VdbeFinishMoveto(VdbeCursor *p){
  पूर्णांक res, rc;
#अगर_घोषित SQLITE_TEST
  बाह्य पूर्णांक sqlite3_खोज_गिनती;
#पूर्ण_अगर
  निश्चित( p->deferredMoveto );
  निश्चित( p->isTable );
  निश्चित( p->eCurType==CURTYPE_BTREE );
  rc = sqlite3BtreeTableMoveto(p->uc.pCursor, p->movetoTarget, 0, &res);
  अगर( rc ) वापस rc;
  अगर( res!=0 ) वापस SQLITE_CORRUPT_BKPT;
#अगर_घोषित SQLITE_TEST
  sqlite3_खोज_गिनती++;
#पूर्ण_अगर
  p->deferredMoveto = 0;
  p->cacheStatus = CACHE_STALE;
  वापस SQLITE_OK;
}

/*
** Something has moved cursor "p" out of place.  Maybe the row it was
** pointed to was deleted out from under it.  Or maybe the btree was
** rebalanced.  Whatever the cause, try to restore "p" to the place it
** is supposed to be pointing.  If the row was deleted out from under the
** cursor, set the cursor to point to a NULL row.
*/
पूर्णांक SQLITE_NOINLINE sqlite3VdbeHandleMovedCursor(VdbeCursor *p){
  पूर्णांक isDifferentRow, rc;
  निश्चित( p->eCurType==CURTYPE_BTREE );
  निश्चित( p->uc.pCursor!=0 );
  निश्चित( sqlite3BtreeCursorHasMoved(p->uc.pCursor) );
  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &isDifferentRow);
  p->cacheStatus = CACHE_STALE;
  अगर( isDifferentRow ) p->nullRow = 1;
  वापस rc;
}

/*
** Check to ensure that the cursor is valid.  Restore the cursor
** if need be.  Return any I/O error from the restore operation.
*/
पूर्णांक sqlite3VdbeCursorRestore(VdbeCursor *p){
  निश्चित( p->eCurType==CURTYPE_BTREE || IsNullCursor(p) );
  अगर( sqlite3BtreeCursorHasMoved(p->uc.pCursor) ){
    वापस sqlite3VdbeHandleMovedCursor(p);
  }
  वापस SQLITE_OK;
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

#अगर 0 /* Inlined into the OP_MakeRecord opcode */
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
u32 sqlite3VdbeSerialType(Mem *pMem, पूर्णांक file_format, u32 *pLen){
  पूर्णांक flags = pMem->flags;
  u32 n;

  निश्चित( pLen!=0 );
  अगर( flags&MEM_Null ){
    *pLen = 0;
    वापस 0;
  }
  अगर( flags&(MEM_Int|MEM_IntReal) ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    testcase( flags & MEM_Int );
    testcase( flags & MEM_IntReal );
    अगर( i<0 ){
      u = ~i;
    }अन्यथा{
      u = i;
    }
    अगर( u<=127 ){
      अगर( (i&1)==i && file_format>=4 ){
        *pLen = 0;
        वापस 8+(u32)u;
      }अन्यथा{
        *pLen = 1;
        वापस 1;
      }
    }
    अगर( u<=32767 ){ *pLen = 2; वापस 2; }
    अगर( u<=8388607 ){ *pLen = 3; वापस 3; }
    अगर( u<=2147483647 ){ *pLen = 4; वापस 4; }
    अगर( u<=MAX_6BYTE ){ *pLen = 6; वापस 5; }
    *pLen = 8;
    अगर( flags&MEM_IntReal ){
      /* If the value is IntReal and is going to take up 8 bytes to store
      ** as an integer, then we might as well make it an 8-byte floating
      ** point value */
      pMem->u.r = (द्विगुन)pMem->u.i;
      pMem->flags &= ~MEM_IntReal;
      pMem->flags |= MEM_Real;
      वापस 7;
    }
    वापस 6;
  }
  अगर( flags&MEM_Real ){
    *pLen = 8;
    वापस 7;
  }
  निश्चित( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  निश्चित( pMem->n>=0 );
  n = (u32)pMem->n;
  अगर( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  *pLen = n;
  वापस ((n*2) + 12 + ((flags&MEM_Str)!=0));
}
#पूर्ण_अगर /* inlined into OP_MakeRecord */

/*
** The sizes for serial types less than 128
*/
स्थिर u8 sqlite3SmallTypeSizes[128] = {
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
  अगर( serial_type>=128 ){
    वापस (serial_type-12)/2;
  }अन्यथा{
    निश्चित( serial_type<12
            || sqlite3SmallTypeSizes[serial_type]==(serial_type - 12)/2 );
    वापस sqlite3SmallTypeSizes[serial_type];
  }
}
u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type){
  निश्चित( serial_type<128 );
  वापस sqlite3SmallTypeSizes[serial_type]; 
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
#अगर_घोषित SQLITE_MIXED_ENDIAN_64BIT_FLOAT
u64 sqlite3FloatSwap(u64 in){
  जोड़ {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = in;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  वापस u.r;
}
#पूर्ण_अगर /* SQLITE_MIXED_ENDIAN_64BIT_FLOAT */

/*
** Deserialize the REAL number pointed to by buf and store it in pMem.
*/
अटल पूर्णांक sqlite3VdbeSerialGet7(
  स्थिर अचिन्हित अक्षर *buf,     /* Buffer to deserialize from */
  Mem *pMem                     /* Memory cell to write value into */
){
  /* EVIDENCE-OF: R-57343-49114 Value is a big-endian IEEE 754-2008 64-bit
  ** floating point number. */
  u64 x = sqlite3Get8byte(buf);
  निश्चित( माप(x)==8 && माप(pMem->u.r)==8 );
  swapMixedEndianFloat(x);
  स_नकल(&pMem->u.r, &x, माप(x));
  अगर( IsNaN(x) ){
    pMem->flags = MEM_Null;
    वापस 1;
  }
  pMem->flags = MEM_Real;
  वापस 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.
**
** Similar code is found in the implementation of the OP_Column opcode.
*/
व्योम sqlite3VdbeSerialGet(
  स्थिर अचिन्हित अक्षर *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  चयन( serial_type ){
    हाल 10: { /* Internal use only: NULL with virtual table
               ** UPDATE no-change flag set */
      pMem->flags = MEM_Null|MEM_Zero;
      pMem->n = 0;
      pMem->u.nZero = 0;
      वापस;
    }
    हाल 11:   /* Reserved for future use */
    हाल 0: {  /* Null */
      /* EVIDENCE-OF: R-24078-09375 Value is a NULL. */
      pMem->flags = MEM_Null;
      वापस;
    }
    हाल 1: {
      /* EVIDENCE-OF: R-44885-25196 Value is an 8-bit twos-complement
      ** integer. */
      pMem->u.i = ONE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 2: { /* 2-byte signed integer */
      /* EVIDENCE-OF: R-49794-35026 Value is a big-endian 16-bit
      ** twos-complement integer. */
      pMem->u.i = TWO_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 3: { /* 3-byte signed integer */
      /* EVIDENCE-OF: R-37839-54301 Value is a big-endian 24-bit
      ** twos-complement integer. */
      pMem->u.i = THREE_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 4: { /* 4-byte signed integer */
      /* EVIDENCE-OF: R-01849-26079 Value is a big-endian 32-bit
      ** twos-complement integer. */
      pMem->u.i = FOUR_BYTE_INT(buf);
#अगर_घोषित __HP_cc
      /* Work around a sign-extension bug in the HP compiler for HP/UX */
      अगर( buf[0]&0x80 ) pMem->u.i |= 0xffffffff80000000LL;
#पूर्ण_अगर
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 5: { /* 6-byte signed integer */
      /* EVIDENCE-OF: R-50385-09674 Value is a big-endian 48-bit
      ** twos-complement integer. */
      pMem->u.i = SIX_BYTE_INT(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 6: {   /* 8-byte signed integer */
      /* EVIDENCE-OF: R-29851-52272 Value is a big-endian 64-bit
      ** twos-complement integer. */
      pMem->u.i = (i64)sqlite3Get8byte(buf);
      pMem->flags = MEM_Int;
      testcase( pMem->u.i<0 );
      वापस;
    }
    हाल 7: { /* IEEE floating point */
      sqlite3VdbeSerialGet7(buf, pMem);
      वापस;
    }
    हाल 8:    /* Integer 0 */
    हाल 9: {  /* Integer 1 */
      /* EVIDENCE-OF: R-12976-22893 Value is the integer 0. */
      /* EVIDENCE-OF: R-18143-12121 Value is the integer 1. */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      वापस;
    }
    शेष: {
      /* EVIDENCE-OF: R-14606-31564 Value is a BLOB that is (N-12)/2 bytes in
      ** length.
      ** EVIDENCE-OF: R-28401-00140 Value is a string in the text encoding and
      ** (N-13)/2 bytes in length. */
      अटल स्थिर u16 aFlag[] = { MEM_Blob|MEM_Ephem, MEM_Str|MEM_Ephem };
      pMem->z = (अक्षर *)buf;
      pMem->n = (serial_type-12)/2;
      pMem->flags = aFlag[serial_type&1];
      वापस;
    }
  }
  वापस;
}
/*
** Allocate sufficient space for an UnpackedRecord structure large enough
** to hold a decoded index record for pKeyInfo.
**
** The space is allocated using sqlite3DbMallocRaw().  If an OOM error
** occurs, NULL is returned.
*/
UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(
  कुंजी_Info *p_कुंजी_Info               /* Description of the record */
){
  UnpackedRecord *p;              /* Unpacked record to return */
  u64 nByte;                      /* Number of bytes required for *p */
  निश्चित( माप(UnpackedRecord) + माप(Mem)*65536 < 0x7fffffff );
  nByte = ROUND8P(माप(UnpackedRecord)) + माप(Mem)*(p_कुंजी_Info->n_कुंजी_Field+1);
  p = (UnpackedRecord *)sqlite3DbMallocRaw(p_कुंजी_Info->db, nByte);
  अगर( !p ) वापस 0;
  p->aMem = (Mem*)&((अक्षर*)p)[ROUND8P(माप(UnpackedRecord))];
  p->p_कुंजी_Info = p_कुंजी_Info;
  p->nField = p_कुंजी_Info->n_कुंजी_Field + 1;
  वापस p;
}

/*
** Given the nKey-byte encoding of a record in pKey[], populate the
** UnpackedRecord structure indicated by the fourth argument with the
** contents of the decoded record.
*/
व्योम sqlite3VdbeRecordUnpack(
  पूर्णांक n_कुंजी,              /* Size of the binary record */
  स्थिर व्योम *p_कुंजी,      /* The binary record */
  UnpackedRecord *p      /* Populate this structure before returning. */
){
  स्थिर अचिन्हित अक्षर *a_कुंजी = (स्थिर अचिन्हित अक्षर *)p_कुंजी;
  u32 d;
  u32 idx;                        /* Offset in aKey[] to read from */
  u16 u;                          /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem = p->aMem;
  कुंजी_Info *p_कुंजी_Info = p->p_कुंजी_Info;

  p->default_rc = 0;
  निश्चित( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(a_कुंजी, szHdr);
  d = szHdr;
  u = 0;
  जबतक( idx<szHdr && d<=(u32)n_कुंजी ){
    u32 serial_type;

    idx += getVarint32(&a_कुंजी[idx], serial_type);
    pMem->enc = p_कुंजी_Info->enc;
    pMem->db = p_कुंजी_Info->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&a_कुंजी[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    अगर( (++u)>=p->nField ) अवरोध;
    pMem++;
  }
  अगर( d>(u32)n_कुंजी && u ){
    निश्चित( CORRUPT_DB );
    /* In a corrupt record entry, the last pMem might have been set up using
    ** uninitialized memory. Overwrite its value with NULL, to prevent
    ** warnings from MSAN. */
    sqlite3VdbeMemSetNull(pMem-(u<p->nField));
  }
  testcase( u == p_कुंजी_Info->n_कुंजी_Field + 1 );
  testcase( u < p_कुंजी_Info->n_कुंजी_Field + 1 );
  निश्चित( u<=p_कुंजी_Info->n_कुंजी_Field + 1 );
  p->nField = u;
}

#अगर_घोषित SQLITE_DEBUG
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
अटल पूर्णांक vdbeRecordCompareDebug(
  पूर्णांक nKey1, स्थिर व्योम *pKey1, /* Left key */
  स्थिर UnpackedRecord *pPKey2, /* Right key */
  पूर्णांक desired_परिणाम             /* Correct answer */
){
  u32 d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  पूर्णांक i = 0;
  पूर्णांक rc = 0;
  स्थिर अचिन्हित अक्षर *aKey1 = (स्थिर अचिन्हित अक्षर *)pKey1;
  कुंजी_Info *p_कुंजी_Info;
  Mem mem1;

  p_कुंजी_Info = pPKey2->p_कुंजी_Info;
  अगर( p_कुंजी_Info->db==0 ) वापस 1;
  mem1.enc = p_कुंजी_Info->enc;
  mem1.db = p_कुंजी_Info->db;
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
  अगर( szHdr1>98307 ) वापस SQLITE_CORRUPT;
  d1 = szHdr1;
  निश्चित( p_कुंजी_Info->nAllField>=pPKey2->nField || CORRUPT_DB );
  निश्चित( p_कुंजी_Info->aSortFlags!=0 );
  निश्चित( p_कुंजी_Info->n_कुंजी_Field>0 );
  निश्चित( idx1<=szHdr1 || CORRUPT_DB );
  करो{
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );

    /* Verify that there is enough key space remaining to avoid
    ** a buffer overread.  The "d1+serial_type1+2" subexpression will
    ** always be greater than or equal to the amount of required key space.
    ** Use that approximation to avoid the more expensive call to
    ** sqlite3VdbeSerialTypeLen() in the common case.
    */
    अगर( d1+(u64)serial_type1+2>(u64)nKey1
     && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)>(u64)nKey1
    ){
      अगर( serial_type1>=1
       && serial_type1<=7
       && d1+(u64)sqlite3VdbeSerialTypeLen(serial_type1)<=(u64)nKey1+8
       && CORRUPT_DB
      ){
        वापस 1;  /* corrupt record not detected by
                   ** sqlite3VdbeRecordCompareWithSkip().  Return true
                   ** to avoid firing the assert() */
      }
      अवरोध;
    }

    /* Extract the values to be compared.
    */
    sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);
    d1 += sqlite3VdbeSerialTypeLen(serial_type1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           p_कुंजी_Info->nAllField>i ? p_कुंजी_Info->aColl[i] : 0);
    अगर( rc!=0 ){
      निश्चित( mem1.szMalloc==0 );  /* See comment below */
      अगर( (p_कुंजी_Info->aSortFlags[i] & KEYINFO_ORDER_BIGNULL)
       && ((mem1.flags & MEM_Null) || (pPKey2->aMem[i].flags & MEM_Null))
      ){
        rc = -rc;
      }
      अगर( p_कुंजी_Info->aSortFlags[i] & KEYINFO_ORDER_DESC ){
        rc = -rc;  /* Invert the result for DESC sort order. */
      }
      जाओ debugCompareEnd;
    }
    i++;
  }जबतक( idx1<szHdr1 && i<pPKey2->nField );

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  निश्चित( mem1.szMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  rc = pPKey2->default_rc;

debugCompareEnd:
  अगर( desired_परिणाम==0 && rc==0 ) वापस 1;
  अगर( desired_परिणाम<0 && rc<0 ) वापस 1;
  अगर( desired_परिणाम>0 && rc>0 ) वापस 1;
  अगर( CORRUPT_DB ) वापस 1;
  अगर( p_कुंजी_Info->db->mallocFailed ) वापस 1;
  वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
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
अटल व्योम vdbe_Assert_Field_गिनती_Within_Limits(
  पूर्णांक n_कुंजी, स्थिर व्योम *p_कुंजी,   /* The record to verify */
  स्थिर कुंजी_Info *p_कुंजी_Info       /* Compare size with this KeyInfo */
){
  पूर्णांक nField = 0;
  u32 szHdr;
  u32 idx;
  u32 notUsed;
  स्थिर अचिन्हित अक्षर *a_कुंजी = (स्थिर अचिन्हित अक्षर*)p_कुंजी;

  अगर( CORRUPT_DB ) वापस;
  idx = getVarint32(a_कुंजी, szHdr);
  निश्चित( n_कुंजी>=0 );
  निश्चित( szHdr<=(u32)n_कुंजी );
  जबतक( idx<szHdr ){
    idx += getVarint32(a_कुंजी+idx, notUsed);
    nField++;
  }
  निश्चित( nField <= p_कुंजी_Info->nAllField );
}
#अन्यथा
# define vdbe_Assert_Field_गिनती_Within_Limits(A,B,C)
#पूर्ण_अगर

/*
** Both *pMem1 and *pMem2 contain string values. Compare the two values
** using the collation sequence pColl. As usual, return a negative , zero
** or positive value if *pMem1 is less than, equal to or greater than
** *pMem2, respectively. Similar in spirit to "rc = (*pMem1) - (*pMem2);".
*/
अटल SQLITE_NOINLINE पूर्णांक vdbeCompareMemStringWithEncodingChange(
  स्थिर Mem *pMem1,
  स्थिर Mem *pMem2,
  स्थिर CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  पूर्णांक rc;
  स्थिर व्योम *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, MEM_Null);
  sqlite3VdbeMemInit(&c2, pMem1->db, MEM_Null);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, MEM_Ephem);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, MEM_Ephem);
  v1 = sqlite3_मूल्य_Text((sqlite3_मूल्य*)&c1, pColl->enc);
  v2 = sqlite3_मूल्य_Text((sqlite3_मूल्य*)&c2, pColl->enc);
  अगर( (v1==0 || v2==0) ){
    अगर( prcErr ) *prcErr = SQLITE_NOMEM_BKPT;
    rc = 0;
  }अन्यथा{
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  वापस rc;
}
अटल पूर्णांक vdbeCompareMemString(
  स्थिर Mem *pMem1,
  स्थिर Mem *pMem2,
  स्थिर CollSeq *pColl,
  u8 *prcErr                      /* If an OOM occurs, set to SQLITE_NOMEM */
){
  अगर( pMem1->enc==pColl->enc ){
    /* The strings are already in the correct encoding.  Call the
     ** comparison function directly */
    वापस pColl->xCmp(pColl->pUser,pMem1->n,pMem1->z,pMem2->n,pMem2->z);
  }अन्यथा{
    वापस vdbeCompareMemStringWithEncodingChange(pMem1,pMem2,pColl,prcErr);
  }
}

/*
** The input pBlob is guaranteed to be a Blob that is not marked
** with MEM_Zero.  Return true if it could be a zero-blob.
*/
अटल पूर्णांक isAllZero(स्थिर अक्षर *z, पूर्णांक n){
  पूर्णांक i;
  क्रम(i=0; i<n; i++){
    अगर( z[i] ) वापस 0;
  }
  वापस 1;
}

/*
** Compare two blobs.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second, respectively.
** If one blob is a prefix of the other, then the shorter is the lessor.
*/
SQLITE_NOINLINE पूर्णांक sqlite3BlobCompare(स्थिर Mem *pB1, स्थिर Mem *pB2){
  पूर्णांक c;
  पूर्णांक n1 = pB1->n;
  पूर्णांक n2 = pB2->n;

  /* It is possible to have a Blob value that has some non-zero content
  ** followed by zero content.  But that only comes up for Blobs formed
  ** by the OP_MakeRecord opcode, and such Blobs never get passed into
  ** sqlite3MemCompare(). */
  निश्चित( (pB1->flags & MEM_Zero)==0 || n1==0 );
  निश्चित( (pB2->flags & MEM_Zero)==0 || n2==0 );

  अगर( (pB1->flags|pB2->flags) & MEM_Zero ){
    अगर( pB1->flags & pB2->flags & MEM_Zero ){
      वापस pB1->u.nZero - pB2->u.nZero;
    }अन्यथा अगर( pB1->flags & MEM_Zero ){
      अगर( !isAllZero(pB2->z, pB2->n) ) वापस -1;
      वापस pB1->u.nZero - n2;
    }अन्यथा{
      अगर( !isAllZero(pB1->z, pB1->n) ) वापस +1;
      वापस n1 - pB2->u.nZero;
    }
  }
  c = स_भेद(pB1->z, pB2->z, n1>n2 ? n2 : n1);
  अगर( c ) वापस c;
  वापस n1 - n2;
}

/* The following two functions are used only within testcase() to prove
** test coverage.  These functions do no exist for production builds.
** We must use separate SQLITE_NOINLINE functions here, since otherwise
** optimizer code movement causes gcov to become very confused.
*/
#अगर defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
अटल पूर्णांक SQLITE_NOINLINE doubleLt(द्विगुन a, द्विगुन b){ वापस a<b; }
अटल पूर्णांक SQLITE_NOINLINE doubleEq(द्विगुन a, द्विगुन b){ वापस a==b; }
#पूर्ण_अगर

/*
** Do a comparison between a 64-bit signed integer and a 64-bit floating-point
** number.  Return negative, zero, or positive if the first (i64) is less than,
** equal to, or greater than the second (double).
*/
पूर्णांक sqlite3IntFloatCompare(i64 i, द्विगुन r){
  अगर( sqlite3IsNaN(r) ){
    /* SQLite considers NaN to be a NULL. And all integer values are greater
    ** than NULL */
    वापस 1;
  }अन्यथा{
    i64 y;
    अगर( r<-9223372036854775808.0 ) वापस +1;
    अगर( r>=9223372036854775808.0 ) वापस -1;
    y = (i64)r;
    अगर( i<y ) वापस -1;
    अगर( i>y ) वापस +1;
    testcase( doubleLt(((द्विगुन)i),r) );
    testcase( doubleLt(r,((द्विगुन)i)) );
    testcase( doubleEq(r,((द्विगुन)i)) );
    वापस (((द्विगुन)i)<r) ? -1 : (((द्विगुन)i)>r);
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
पूर्णांक sqlite3MemCompare(स्थिर Mem *pMem1, स्थिर Mem *pMem2, स्थिर CollSeq *pColl){
  पूर्णांक f1, f2;
  पूर्णांक combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1|f2;
  निश्चित( !sqlite3VdbeMemIsRowSet(pMem1) && !sqlite3VdbeMemIsRowSet(pMem2) );

  /* If one value is NULL, it is less than the other. If both values
  ** are NULL, return 0.
  */
  अगर( combined_flags&MEM_Null ){
    वापस (f2&MEM_Null) - (f1&MEM_Null);
  }

  /* At least one of the two values is a number
  */
  अगर( combined_flags&(MEM_Int|MEM_Real|MEM_IntReal) ){
    testcase( combined_flags & MEM_Int );
    testcase( combined_flags & MEM_Real );
    testcase( combined_flags & MEM_IntReal );
    अगर( (f1 & f2 & (MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & f2 & MEM_Int );
      testcase( f1 & f2 & MEM_IntReal );
      अगर( pMem1->u.i < pMem2->u.i ) वापस -1;
      अगर( pMem1->u.i > pMem2->u.i ) वापस +1;
      वापस 0;
    }
    अगर( (f1 & f2 & MEM_Real)!=0 ){
      अगर( pMem1->u.r < pMem2->u.r ) वापस -1;
      अगर( pMem1->u.r > pMem2->u.r ) वापस +1;
      वापस 0;
    }
    अगर( (f1&(MEM_Int|MEM_IntReal))!=0 ){
      testcase( f1 & MEM_Int );
      testcase( f1 & MEM_IntReal );
      अगर( (f2&MEM_Real)!=0 ){
        वापस sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      }अन्यथा अगर( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        अगर( pMem1->u.i < pMem2->u.i ) वापस -1;
        अगर( pMem1->u.i > pMem2->u.i ) वापस +1;
        वापस 0;
      }अन्यथा{
        वापस -1;
      }
    }
    अगर( (f1&MEM_Real)!=0 ){
      अगर( (f2&(MEM_Int|MEM_IntReal))!=0 ){
        testcase( f2 & MEM_Int );
        testcase( f2 & MEM_IntReal );
        वापस -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      }अन्यथा{
        वापस -1;
      }
    }
    वापस +1;
  }

  /* If one value is a string and the other is a blob, the string is less.
  ** If both are strings, compare using the collating functions.
  */
  अगर( combined_flags&MEM_Str ){
    अगर( (f1 & MEM_Str)==0 ){
      वापस 1;
    }
    अगर( (f2 & MEM_Str)==0 ){
      वापस -1;
    }

    निश्चित( pMem1->enc==pMem2->enc || pMem1->db->mallocFailed );
    निश्चित( pMem1->enc==SQLITE_UTF8 ||
            pMem1->enc==SQLITE_UTF16LE || pMem1->enc==SQLITE_UTF16BE );

    /* The collation sequence must be defined at this point, even if
    ** the user deletes the collation sequence after the vdbe program is
    ** compiled (this was not always the case).
    */
    निश्चित( !pColl || pColl->xCmp );

    अगर( pColl ){
      वापस vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
    /* If a NULL pointer was passed as the collate function, fall through
    ** to the blob case and use memcmp().  */
  }

  /* Both values must be blobs.  Compare using memcmp().  */
  वापस sqlite3BlobCompare(pMem1, pMem2);
}


/*
** The first argument passed to this function is a serial-type that
** corresponds to an integer - all values between 1 and 9 inclusive
** except 7. The second points to a buffer containing an integer value
** serialized according to serial_type. This function deserializes
** and returns the value.
*/
अटल i64 vdbeRecordDecodeInt(u32 serial_type, स्थिर u8 *a_कुंजी){
  u32 y;
  निश्चित( CORRUPT_DB || (serial_type>=1 && serial_type<=9 && serial_type!=7) );
  चयन( serial_type ){
    हाल 0:
    हाल 1:
      testcase( a_कुंजी[0]&0x80 );
      वापस ONE_BYTE_INT(a_कुंजी);
    हाल 2:
      testcase( a_कुंजी[0]&0x80 );
      वापस TWO_BYTE_INT(a_कुंजी);
    हाल 3:
      testcase( a_कुंजी[0]&0x80 );
      वापस THREE_BYTE_INT(a_कुंजी);
    हाल 4: {
      testcase( a_कुंजी[0]&0x80 );
      y = FOUR_BYTE_UINT(a_कुंजी);
      वापस (i64)*(पूर्णांक*)&y;
    }
    हाल 5: {
      testcase( a_कुंजी[0]&0x80 );
      वापस FOUR_BYTE_UINT(a_कुंजी+2) + (((i64)1)<<32)*TWO_BYTE_INT(a_कुंजी);
    }
    हाल 6: {
      u64 x = FOUR_BYTE_UINT(a_कुंजी);
      testcase( a_कुंजी[0]&0x80 );
      x = (x<<32) | FOUR_BYTE_UINT(a_कुंजी+4);
      वापस (i64)*(i64*)&x;
    }
  }

  वापस (serial_type - 8);
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
पूर्णांक sqlite3VdbeRecordCompareWithSkip(
  पूर्णांक nKey1, स्थिर व्योम *pKey1,   /* Left key */
  UnpackedRecord *pPKey2,         /* Right key */
  पूर्णांक bSkip                       /* If true, skip the first field */
){
  u32 d1;                         /* Offset into aKey[] of next data element */
  पूर्णांक i;                          /* Index of next field to compare */
  u32 szHdr1;                     /* Size of record header in bytes */
  u32 idx1;                       /* Offset of first type in header */
  पूर्णांक rc = 0;                     /* Return value */
  Mem *pRhs = pPKey2->aMem;       /* Next field of pPKey2 to compare */
  कुंजी_Info *p_कुंजी_Info;
  स्थिर अचिन्हित अक्षर *aKey1 = (स्थिर अचिन्हित अक्षर *)pKey1;
  Mem mem1;

  /* If bSkip is true, then the caller has already determined that the first
  ** two elements in the keys are equal. Fix the various stack variables so
  ** that this routine begins comparing at the second field. */
  अगर( bSkip ){
    u32 s1 = aKey1[1];
    अगर( s1<0x80 ){
      idx1 = 2;
    }अन्यथा{
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  }अन्यथा{
    अगर( (szHdr1 = aKey1[0])<0x80 ){
      idx1 = 1;
    }अन्यथा{
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  अगर( d1>(अचिन्हित)nKey1 ){
    pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
    वापस 0;  /* Corruption */
  }

  VVA_ONLY( mem1.szMalloc = 0; ) /* Only needed by assert() statements */
  निश्चित( pPKey2->p_कुंजी_Info->nAllField>=pPKey2->nField
       || CORRUPT_DB );
  निश्चित( pPKey2->p_कुंजी_Info->aSortFlags!=0 );
  निश्चित( pPKey2->p_कुंजी_Info->n_कुंजी_Field>0 );
  निश्चित( idx1<=szHdr1 || CORRUPT_DB );
  जबतक( 1 /*exit-by-break*/ ){
    u32 serial_type;

    /* RHS is an integer */
    अगर( pRhs->flags & (MEM_Int|MEM_IntReal) ){
      testcase( pRhs->flags & MEM_Int );
      testcase( pRhs->flags & MEM_IntReal );
      serial_type = aKey1[idx1];
      testcase( serial_type==12 );
      अगर( serial_type>=10 ){
        rc = serial_type==10 ? -1 : +1;
      }अन्यथा अगर( serial_type==0 ){
        rc = -1;
      }अन्यथा अगर( serial_type==7 ){
        sqlite3VdbeSerialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      }अन्यथा{
        i64 lhs = vdbeRecordDecodeInt(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        अगर( lhs<rhs ){
          rc = -1;
        }अन्यथा अगर( lhs>rhs ){
          rc = +1;
        }
      }
    }

    /* RHS is real */
    अन्यथा अगर( pRhs->flags & MEM_Real ){
      serial_type = aKey1[idx1];
      अगर( serial_type>=10 ){
        /* Serial types 12 or greater are strings and blobs (greater than
        ** numbers). Types 10 and 11 are currently "reserved for future
        ** use", so it doesn't really matter what the results of comparing
        ** them to numeric values are.  */
        rc = serial_type==10 ? -1 : +1;
      }अन्यथा अगर( serial_type==0 ){
        rc = -1;
      }अन्यथा{
        अगर( serial_type==7 ){
          अगर( sqlite3VdbeSerialGet7(&aKey1[d1], &mem1) ){
            rc = -1;  /* mem1 is a NaN */
          }अन्यथा अगर( mem1.u.r<pRhs->u.r ){
            rc = -1;
          }अन्यथा अगर( mem1.u.r>pRhs->u.r ){
            rc = +1;
          }अन्यथा{
            निश्चित( rc==0 );
          }
        }अन्यथा{
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    /* RHS is a string */
    अन्यथा अगर( pRhs->flags & MEM_Str ){
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      अगर( serial_type<12 ){
        rc = -1;
      }अन्यथा अगर( !(serial_type & 0x01) ){
        rc = +1;
      }अन्यथा{
        mem1.n = (serial_type - 12) / 2;
        testcase( (d1+mem1.n)==(अचिन्हित)nKey1 );
        testcase( (d1+mem1.n+1)==(अचिन्हित)nKey1 );
        अगर( (d1+mem1.n) > (अचिन्हित)nKey1
         || (p_कुंजी_Info = pPKey2->p_कुंजी_Info)->nAllField<=i
        ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          वापस 0;                /* Corruption */
        }अन्यथा अगर( p_कुंजी_Info->aColl[i] ){
          mem1.enc = p_कुंजी_Info->enc;
          mem1.db = p_कुंजी_Info->db;
          mem1.flags = MEM_Str;
          mem1.z = (अक्षर*)&aKey1[d1];
          rc = vdbeCompareMemString(
              &mem1, pRhs, p_कुंजी_Info->aColl[i], &pPKey2->errCode
          );
        }अन्यथा{
          पूर्णांक nCmp = MIN(mem1.n, pRhs->n);
          rc = स_भेद(&aKey1[d1], pRhs->z, nCmp);
          अगर( rc==0 ) rc = mem1.n - pRhs->n;
        }
      }
    }

    /* RHS is a blob */
    अन्यथा अगर( pRhs->flags & MEM_Blob ){
      निश्चित( (pRhs->flags & MEM_Zero)==0 || pRhs->n==0 );
      getVarint32NR(&aKey1[idx1], serial_type);
      testcase( serial_type==12 );
      अगर( serial_type<12 || (serial_type & 0x01) ){
        rc = -1;
      }अन्यथा{
        पूर्णांक nStr = (serial_type - 12) / 2;
        testcase( (d1+nStr)==(अचिन्हित)nKey1 );
        testcase( (d1+nStr+1)==(अचिन्हित)nKey1 );
        अगर( (d1+nStr) > (अचिन्हित)nKey1 ){
          pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
          वापस 0;                /* Corruption */
        }अन्यथा अगर( pRhs->flags & MEM_Zero ){
          अगर( !isAllZero((स्थिर अक्षर*)&aKey1[d1],nStr) ){
            rc = 1;
          }अन्यथा{
            rc = nStr - pRhs->u.nZero;
          }
        }अन्यथा{
          पूर्णांक nCmp = MIN(nStr, pRhs->n);
          rc = स_भेद(&aKey1[d1], pRhs->z, nCmp);
          अगर( rc==0 ) rc = nStr - pRhs->n;
        }
      }
    }

    /* RHS is null */
    अन्यथा{
      serial_type = aKey1[idx1];
      अगर( serial_type==0
       || serial_type==10
       || (serial_type==7 && sqlite3VdbeSerialGet7(&aKey1[d1], &mem1)!=0)
      ){
        निश्चित( rc==0 );
      }अन्यथा{
        rc = 1;
      }
    }

    अगर( rc!=0 ){
      पूर्णांक sortFlags = pPKey2->p_कुंजी_Info->aSortFlags[i];
      अगर( sortFlags ){
        अगर( (sortFlags & KEYINFO_ORDER_BIGNULL)==0
         || ((sortFlags & KEYINFO_ORDER_DESC)
           !=(serial_type==0 || (pRhs->flags&MEM_Null)))
        ){
          rc = -rc;
        }
      }
      निश्चित( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, rc) );
      निश्चित( mem1.szMalloc==0 );  /* See comment below */
      वापस rc;
    }

    i++;
    अगर( i==pPKey2->nField ) अवरोध;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    अगर( d1>(अचिन्हित)nKey1 ) अवरोध;
    idx1 += sqlite3VarintLen(serial_type);
    अगर( idx1>=(अचिन्हित)szHdr1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      वापस 0;  /* Corrupt index */
    }
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).  */
  निश्चित( mem1.szMalloc==0 );

  /* rc==0 here means that one or both of the keys ran out of fields and
  ** all the fields up to that point were equal. Return the default_rc
  ** value.  */
  निश्चित( CORRUPT_DB
       || vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, pPKey2->default_rc)
       || pPKey2->p_कुंजी_Info->db->mallocFailed
  );
  pPKey2->eqSeen = 1;
  वापस pPKey2->default_rc;
}
पूर्णांक sqlite3VdbeRecordCompare(
  पूर्णांक nKey1, स्थिर व्योम *pKey1,   /* Left key */
  UnpackedRecord *pPKey2          /* Right key */
){
  वापस sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 0);
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
अटल पूर्णांक vdbeRecordCompareInt(
  पूर्णांक nKey1, स्थिर व्योम *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  स्थिर u8 *a_कुंजी = &((स्थिर u8*)pKey1)[*(स्थिर u8*)pKey1 & 0x3F];
  पूर्णांक serial_type = ((स्थिर u8*)pKey1)[1];
  पूर्णांक res;
  i64 v;
  i64 lhs;

  vdbe_Assert_Field_गिनती_Within_Limits(nKey1, pKey1, pPKey2->p_कुंजी_Info);
  निश्चित( (*(u8*)pKey1)<=0x3F || CORRUPT_DB );

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
  अगर( (u32)(serial_type-1)<=5 ){
    अटल स्थिर u8 aShift[] = { 0, 56, 48, 40, 32, 16, 0 };
    lhs = ((i64)sqlite3Get8byte(a_कुंजी)) >> aShift[serial_type];
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
    निश्चित( 0<=(i64)sqlite3Get8byte(a_कुंजी) || lhs<0 );
    testcase( lhs<0 );
  }अन्यथा अगर( serial_type==8 || serial_type==9 ){
    lhs = serial_type - 8;
  }अन्यथा{
    वापस sqlite3VdbeRecordCompare(nKey1, pKey1, pPKey2);
  }

  निश्चित( pPKey2->u.i == pPKey2->aMem[0].u.i );
  v = pPKey2->u.i;
  अगर( v>lhs ){
    res = pPKey2->r1;
  }अन्यथा अगर( v<lhs ){
    res = pPKey2->r2;
  }अन्यथा अगर( pPKey2->nField>1 ){
    /* The first fields of the two keys are equal. Compare the trailing
    ** fields.  */
    res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
  }अन्यथा{
    /* The first fields of the two keys are equal and there are no trailing
    ** fields. Return pPKey2->default_rc in this case. */
    res = pPKey2->default_rc;
    pPKey2->eqSeen = 1;
  }

  निश्चित( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res) );
  वापस res;
}

/*
** This function is an optimized version of sqlite3VdbeRecordCompare()
** that (a) the first field of pPKey2 is a string, that (b) the first field
** uses the collation sequence BINARY and (c) that the size-of-header varint
** at the start of (pKey1/nKey1) fits in a single byte.
*/
अटल पूर्णांक vdbeRecordCompareString(
  पूर्णांक nKey1, स्थिर व्योम *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  स्थिर u8 *aKey1 = (स्थिर u8*)pKey1;
  पूर्णांक serial_type;
  पूर्णांक res;

  निश्चित( pPKey2->aMem[0].flags & MEM_Str );
  निश्चित( pPKey2->aMem[0].n == pPKey2->n );
  निश्चित( pPKey2->aMem[0].z == pPKey2->u.z );
  vdbe_Assert_Field_गिनती_Within_Limits(nKey1, pKey1, pPKey2->p_कुंजी_Info);
  serial_type = (चिन्हित अक्षर)(aKey1[1]);

vrcs_restart:
  अगर( serial_type<12 ){
    अगर( serial_type<0 ){
      sqlite3GetVarint32(&aKey1[1], (u32*)&serial_type);
      अगर( serial_type>=12 ) जाओ vrcs_restart;
      निश्चित( CORRUPT_DB );
    }
    res = pPKey2->r1;      /* (pKey1/nKey1) is a number or a null */
  }अन्यथा अगर( !(serial_type & 0x01) ){
    res = pPKey2->r2;      /* (pKey1/nKey1) is a blob */
  }अन्यथा{
    पूर्णांक nCmp;
    पूर्णांक nStr;
    पूर्णांक szHdr = aKey1[0];

    nStr = (serial_type-12) / 2;
    अगर( (szHdr + nStr) > nKey1 ){
      pPKey2->errCode = (u8)SQLITE_CORRUPT_BKPT;
      वापस 0;    /* Corruption */
    }
    nCmp = MIN( pPKey2->n, nStr );
    res = स_भेद(&aKey1[szHdr], pPKey2->u.z, nCmp);

    अगर( res>0 ){
      res = pPKey2->r2;
    }अन्यथा अगर( res<0 ){
      res = pPKey2->r1;
    }अन्यथा{
      res = nStr - pPKey2->n;
      अगर( res==0 ){
        अगर( pPKey2->nField>1 ){
          res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
        }अन्यथा{
          res = pPKey2->default_rc;
          pPKey2->eqSeen = 1;
        }
      }अन्यथा अगर( res>0 ){
        res = pPKey2->r2;
      }अन्यथा{
        res = pPKey2->r1;
      }
    }
  }

  निश्चित( vdbeRecordCompareDebug(nKey1, pKey1, pPKey2, res)
       || CORRUPT_DB
       || pPKey2->p_कुंजी_Info->db->mallocFailed
  );
  वापस res;
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
  निश्चित( p->p_कुंजी_Info->aSortFlags!=0 );
  अगर( p->p_कुंजी_Info->nAllField<=13 ){
    पूर्णांक flags = p->aMem[0].flags;
    अगर( p->p_कुंजी_Info->aSortFlags[0] ){
      अगर( p->p_कुंजी_Info->aSortFlags[0] & KEYINFO_ORDER_BIGNULL ){
        वापस sqlite3VdbeRecordCompare;
      }
      p->r1 = 1;
      p->r2 = -1;
    }अन्यथा{
      p->r1 = -1;
      p->r2 = 1;
    }
    अगर( (flags & MEM_Int) ){
      p->u.i = p->aMem[0].u.i;
      वापस vdbeRecordCompareInt;
    }
    testcase( flags & MEM_Real );
    testcase( flags & MEM_Null );
    testcase( flags & MEM_Blob );
    अगर( (flags & (MEM_Real|MEM_IntReal|MEM_Null|MEM_Blob))==0
     && sqlite3_Is_द्विआधारी(p->p_कुंजी_Info->aColl[0])
    ){
      निश्चित( flags & MEM_Str );
      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      वापस vdbeRecordCompareString;
    }
  }

  वापस sqlite3VdbeRecordCompare;
}

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
पूर्णांक sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 n_Cell_कुंजी = 0;
  पूर्णांक rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits 
  */
  निश्चित( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_कुंजी = sqlite3_Btree_Payload_आकार(pCur);
  निश्चित( (n_Cell_कुंजी & SQLITE_MAX_U32)==(u64)n_Cell_कुंजी );

  /* Read in the complete content of the index entry */
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)n_Cell_कुंजी, &m);
  अगर( rc ){
    वापस rc;
  }

  /* The index entry must begin with a header size */
  getVarint32NR((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==(u32)m.n );
  testcase( szHdr>0x7fffffff );
  निश्चित( m.n>=0 );
  अगर( unlikely(szHdr<3 || szHdr>(अचिन्हित)m.n) ){
    जाओ idx_rowid_corruption;
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
  अगर( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    जाओ idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  testcase( (u32)m.n==szHdr+lenRowid );
  अगर( unlikely((u32)m.n<szHdr+lenRowid) ){
    जाओ idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  वापस SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.szMalloc!=0 );
  sqlite3VdbeMemReleaseMalloc(&m);
  वापस SQLITE_CORRUPT_BKPT;
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
पूर्णांक sqlite3_Vdbe_Idx_कुंजी_Compare(
  sqlite3 *db,                     /* Database connection */
  VdbeCursor *pC,                  /* The cursor to compare against */
  UnpackedRecord *pUnpacked,       /* Unpacked version of key */
  पूर्णांक *res                         /* Write the comparison result here */
){
  i64 n_Cell_कुंजी = 0;
  पूर्णांक rc;
  BtCursor *pCur;
  Mem m;

  निश्चित( pC->eCurType==CURTYPE_BTREE );
  pCur = pC->uc.pCursor;
  निश्चित( sqlite3BtreeCursorIsValid(pCur) );
  n_Cell_कुंजी = sqlite3_Btree_Payload_आकार(pCur);
  /* nCellKey will always be between 0 and 0xffffffff because of the way
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  अगर( n_Cell_कुंजी<=0 || n_Cell_कुंजी>0x7fffffff ){
    *res = 0;
    वापस SQLITE_CORRUPT_BKPT;
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)n_Cell_कुंजी, &m);
  अगर( rc ){
    वापस rc;
  }
  *res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  वापस SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'.
*/
व्योम sqlite3VdbeSetChanges(sqlite3 *db, i64 nChange){
  निश्चित( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->n_योग_Change += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
व्योम sqlite3_Vdbe_गिनती_Changes(Vdbe *v){
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
व्योम sqlite3ExpirePreparedStatements(sqlite3 *db, पूर्णांक iCode){
  Vdbe *p;
  क्रम(p = db->pVdbe; p; p=p->pVNext){
    p->expired = iCode+1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
sqlite3 *sqlite3VdbeDb(Vdbe *v){
  वापस v->db;
}

/*
** Return the SQLITE_PREPARE flags for a Vdbe.
*/
u8 sqlite3VdbePrepareFlags(Vdbe *v){
  वापस v->prepFlags;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
sqlite3_मूल्य *sqlite3_Vdbe_Get_Bound_मूल्य(Vdbe *v, पूर्णांक iVar, u8 aff){
  निश्चित( iVar>0 );
  अगर( v ){
    Mem *pMem = &v->aVar[iVar-1];
    निश्चित( (v->db->flags & SQLITE_EnableQPSG)==0 
         || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );
    अगर( 0==(pMem->flags & MEM_Null) ){
      sqlite3_मूल्य *pRet = sqlite3_मूल्य_New(v->db);
      अगर( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3_मूल्य_Apply_Affinity(pRet, aff, SQLITE_UTF8);
      }
      वापस pRet;
    }
  }
  वापस 0;
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
व्योम sqlite3VdbeReprepareOnBind(Vdbe *v, पूर्णांक iVar, पूर्णांक bSmallint){
  u32 m;
  निश्चित( iVar>0 );
  निश्चित( (v->db->flags & SQLITE_EnableQPSG)==0 
       || (v->db->mDbFlags & DBFLAG_InternalFunc)!=0 );

  m = (iVar>=32) ? 0x80000000 : ((u32)1 << (iVar-1));
  अगर( bSmallint ){
    अगर( (v->expmask & m)==0 ) v->smimask |= m;
  }अन्यथा{
    v->smimask &= ~m;
  }
  v->expmask |= m;

  /* smimask is always a subset of expmask */
  निश्चित( (v->smimask & v->expmask) == v->smimask );
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
अटल पूर्णांक vdbeSkipField(
  Bitmask mask,                   /* Mask of indexed expression fields */
  पूर्णांक iCol,                       /* Column of index being considered */
  Mem *pMem1,                     /* Expected index value */
  Mem *pMem2,                     /* Actual indexed value */
  पूर्णांक bIntegrity                  /* True if running PRAGMA integrity_check */
){
#घोषणा BTREE_ULPDISTORTION 2
  अगर( iCol>=BMS || (mask & MASKBIT(iCol))==0 ) वापस 0;
  अगर( bIntegrity==0 ) वापस 1;
  अगर( (pMem1->flags & MEM_Real) && (pMem2->flags & MEM_Real) ){
    u64 m1, m2;
    स_नकल(&m1,&pMem1->u.r,8);
    स_नकल(&m2,&pMem2->u.r,8);
    अगर( (m1<m2 ? m2-m1 : m1-m2) <= BTREE_ULPDISTORTION ){
      वापस 1;
    }
  }
  वापस 0;
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
अटल पूर्णांक vdbe_Is_Matching_सूचकांक_कुंजी(
  BtCursor *pCur,            /* Cursor open on index */
  पूर्णांक bInt,                  /* True for integrity_check-style search */
  Bitmask mask,              /* Mask of columns to skip */
  UnpackedRecord *p,         /* Index key being deleted */
  पूर्णांक *piRes                 /* 0 for a match, non-zero for not a match */
){
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem m;
  पूर्णांक rc = SQLITE_OK;

  स_रखो(&m, 0, माप(m));
  m.enc = p->p_कुंजी_Info->enc;
  m.db = p->p_कुंजी_Info->db;
  nRec = sqlite3_Btree_Payload_आकार(pCur);
  अगर( nRec>0x7fffffff ){
    वापस SQLITE_CORRUPT_BKPT;
  }

  /* Allocate 5 extra bytes at the end of the buffer. This allows the
  ** getVarint32() call below to read slightly past the end of the buffer 
  ** if the record is corrupt. */
  aRec = sqlite3MallocZero(nRec+5);
  अगर( aRec==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }अन्यथा{
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  अगर( rc==SQLITE_OK ){
    u32 szHdr = 0;                /* Size of record header in bytes */
    u32 idxHdr = 0;               /* Current index in header */

    idxHdr = getVarint32(aRec, szHdr);
    अगर( szHdr>98307 ){
      rc = SQLITE_CORRUPT;
    }अन्यथा{
      पूर्णांक res = 0;                /* Result of this function call */
      u32 idxRec = szHdr;         /* Index of next field in record body */
      पूर्णांक ii = 0;                 /* Iterator variable */

      पूर्णांक nCol = p->p_कुंजी_Info->nAllField;
      क्रम(ii=0; ii<nCol && rc==SQLITE_OK; ii++){
        u32 iSerial = 0;
        पूर्णांक nSerial = 0;

        अगर( idxHdr>=szHdr ){
          rc = SQLITE_CORRUPT_BKPT;
          अवरोध;
        }
        idxHdr += getVarint32(&aRec[idxHdr], iSerial);
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        अगर( (idxRec+nSerial)>nRec ){
          rc = SQLITE_CORRUPT_BKPT;
        }अन्यथा{
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &m);
          अगर( vdbeSkipField(mask, ii, &p->aMem[ii], &m, bInt)==0 ){
            res = sqlite3MemCompare(&m, &p->aMem[ii], p->p_कुंजी_Info->aColl[ii]);
            अगर( res!=0 ) अवरोध;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  वापस rc;
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
पूर्णांक sqlite3_Vdbe_Find_सूचकांक_कुंजी(
  BtCursor *pCur, 
  सूचकांक *pIdx,
  UnpackedRecord *p, 
  पूर्णांक *pRes,
  पूर्णांक bIntegrity
){
#घोषणा BTREE_FDK_RANGE 10
  पूर्णांक nStep = 0;
  पूर्णांक res = 1;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक ii = 0;

  /* Calculate a mask based on the first 64 columns of the index. The mask
  ** bit is set if the corresponding index field is either an expression
  ** or a virtual column of the table.  */
  Bitmask mask = 0;
  क्रम(ii=0; ii<MIN(pIdx->nColumn, BMS); ii++){
    पूर्णांक iCol = pIdx->aiColumn[ii];
    अगर( (iCol==XN_व्यंजक)
     || (iCol>=0 && (pIdx->pTable->aCol[iCol].colFlags & COLFLAG_VIRTUAL))
    ){
      mask |= MASKBIT(ii);
    }
  }

  /* If the mask is 0 at this point, then the index contains no expressions
  ** or virtual columns. So do not search for a match - return so that the
  ** caller may declare the db corrupt immediately. Or, if mask is non-zero,
  ** proceed.  */
  अगर( mask!=0 ){

    /* Move the cursor back BTREE_FDK_RANGE entries. If this hits an EOF, 
    ** position the cursor at the first entry in the index and set nStep
    ** to -1 so that the first loop below scans the entire index. Otherwise,
    ** set nStep to BTREE_FDK_RANGE*2 so that the first loop below scans
    ** just that many entries.  */
    क्रम(ii=0; sqlite3BtreeEof(pCur)==0 && ii<BTREE_FDK_RANGE; ii++){
      rc = sqlite3BtreePrevious(pCur, 0);
    }
    अगर( rc==SQLITE_DONE ){
      rc = sqlite3BtreeFirst(pCur, &res);
      nStep = -1;
    }अन्यथा{
      nStep = BTREE_FDK_RANGE*2;
    }
  
    /* This loop runs at most twice to search for a key with matching PK 
    ** fields in the index. The second iteration always searches the entire 
    ** index. The first iteration searches nStep entries starting with the
    ** current cursor entry if (nStep>=0), or the entire index if (nStep<0).  */
    जबतक( sqlite3BtreeCursorIsValidNN(pCur) ){
      क्रम(ii=0; rc==SQLITE_OK && (ii<nStep || nStep<0); ii++){
        rc = vdbe_Is_Matching_सूचकांक_कुंजी(pCur, bIntegrity, mask, p, &res);
        अगर( res==0 || rc!=SQLITE_OK ) अवरोध;
        rc = sqlite3_Btree_अग्र(pCur, 0);
      }
      अगर( rc==SQLITE_DONE ){
        rc = SQLITE_OK;
        निश्चित( res!=0 );
      }
      अगर( nStep<0 || rc!=SQLITE_OK || res==0 || bIntegrity ) अवरोध;
  
      /* The first, non-exhaustive, search failed to find an entry with 
      ** matching PK fields. So restart for an exhaustive search of the 
      ** entire index.  */
      nStep = -1;
      rc = sqlite3BtreeFirst(pCur, &res);
    }
  }

  *pRes = res;
  वापस rc;
}

#अगर_अघोषित SQLITE_OMIT_DATETIME_FUNCS
/*
** Cause a function to throw an error if it was call from OP_PureFunc
** rather than OP_Function.
**
** OP_PureFunc means that the function must be deterministic, and should
** throw an error if it is given inputs that would make it non-deterministic.
** This routine is invoked by date/time functions that use non-deterministic
** features such as 'now'.
*/
पूर्णांक sqlite3NotPureFunc(sqlite3_context *pCtx){
  स्थिर VdbeOp *pOp;
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( pCtx->pVdbe==0 ) वापस 1;
#पूर्ण_अगर
  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  अगर( pOp->opcode==OP_PureFunc ){
    स्थिर अक्षर *zContext;
    अक्षर *z_संदेश;
    अगर( pOp->p5 & NC_IsCheck ){
      zContext = "a CHECK constraint";
    }अन्यथा अगर( pOp->p5 & NC_GenCol ){
      zContext = "a generated column";
    }अन्यथा{
      zContext = "an index";
    }
    z_संदेश = sqlite3_mprintf("non-deterministic use of %s() in %s",
                           pCtx->pFunc->zName, zContext);
    sqlite3_परिणाम_error(pCtx, z_संदेश, -1);
    sqlite3_free(z_संदेश);
    वापस 0;
  }
  वापस 1;
}
#पूर्ण_अगर /* SQLITE_OMIT_DATETIME_FUNCS */

#अगर defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** This Walker callback is used to help verify that calls to
** sqlite3BtreeCursorHint() with opcode BTREE_HINT_RANGE have
** byte-code register values correctly initialized.
*/
पूर्णांक sqlite3_Cursor_Range_Hint_व्यंजक_Check(Walker *pWalker, व्यंजक *p_व्यंजक){
  अगर( p_व्यंजक->op==TK_REGISTER ){
    निश्चित( (pWalker->u.aMem[p_व्यंजक->iTable].flags & MEM_Undefined)==0 );
  }
  वापस WRC_Continue;
}
#पूर्ण_अगर /* SQLITE_ENABLE_CURSOR_HINTS && SQLITE_DEBUG */

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
व्योम sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab){
  अगर( pVtab->z_Err_संदेश ){
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->z_Err_संदेश);
    p->z_Err_संदेश = sqlite3DbStrDup(db, pVtab->z_Err_संदेश);
    sqlite3_free(pVtab->z_Err_संदेश);
    pVtab->z_Err_संदेश = 0;
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_VIRTUALTABLE */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK

/*
** If the second argument is not NULL, release any allocations associated
** with the memory cells in the p->aMem[] array. Also free the UnpackedRecord
** structure itself, using sqlite3DbFree().
**
** This function is used to free UnpackedRecord structures allocated by
** the vdbeUnpackRecord() function found in vdbeapi.c.
*/
अटल व्योम vdbeFreeUnpacked(sqlite3 *db, पूर्णांक nField, UnpackedRecord *p){
  निश्चित( db!=0 );
  अगर( p ){
    पूर्णांक i;
    क्रम(i=0; i<nField; i++){
      Mem *pMem = &p->aMem[i];
      अगर( pMem->zMalloc ) sqlite3VdbeMemReleaseMalloc(pMem);
    }
    sqlite3DbNNFreeNN(db, p);
  }
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Invoke the pre-update hook. If this is an UPDATE or DELETE pre-update call,
** then cursor passed as the second argument should point to the row about
** to be update or deleted. If the application calls sqlite3_preupdate_old(),
** the required value will be read from the row the cursor points to.
*/
व्योम sqlite3VdbePreUpdateHook(
  Vdbe *v,                        /* Vdbe pre-update hook is invoked by */
  VdbeCursor *pCsr,               /* Cursor to grab old.* values from */
  पूर्णांक op,                         /* SQLITE_INSERT, UPDATE or DELETE */
  स्थिर अक्षर *zDb,                /* Database name */
  Table *pTab,                    /* Modified table */
  i64 iKey1,                      /* Initial key value */
  पूर्णांक iReg,                       /* Register for new.* record */
  पूर्णांक iBlobWrite
){
  sqlite3 *db = v->db;
  i64 iKey2;
  PreUpdate preupdate;
  स्थिर अक्षर *zTbl = pTab->zName;
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक nRealCol;
  अगर( pTab->tabFlags & TF_WithoutRowid ){
    nRealCol = sqlite3_Primary_कुंजी_सूचकांक(pTab)->nColumn;
  }अन्यथा अगर( pTab->tabFlags & TF_HasVirtual ){
    nRealCol = pTab->nNVCol;
  }अन्यथा{
    nRealCol = pTab->nCol;
  }
#पूर्ण_अगर

  निश्चित( db->pPreUpdate==0 );
  स_रखो(&preupdate, 0, माप(PreUpdate));
  अगर( HasRowid(pTab)==0 ){
    iKey1 = iKey2 = 0;
    preupdate.pPk = sqlite3_Primary_कुंजी_सूचकांक(pTab);
  }अन्यथा{
    अगर( op==SQLITE_UPDATE ){
      iKey2 = v->aMem[iReg].u.i;
    }अन्यथा{
      iKey2 = iKey1;
    }
  }

  निश्चित( pCsr!=0 );
  निश्चित( pCsr->eCurType==CURTYPE_BTREE );
  निश्चित( pCsr->nField==nRealCol
       || (pCsr->nField==nRealCol+1 && op==SQLITE_DELETE && iReg==-1)
  );

  preupdate.v = v;
  preupdate.pCsr = pCsr;
  preupdate.op = op;
  preupdate.iNewReg = iReg;
  preupdate.pKeyinfo = (कुंजी_Info*)&preupdate.u_कुंजी;
  preupdate.pKeyinfo->db = db;
  preupdate.pKeyinfo->enc = ENC(db);
  preupdate.pKeyinfo->n_कुंजी_Field = pTab->nCol;
  preupdate.pKeyinfo->aSortFlags = 0; /* Indicate .aColl, .nAllField uninit */
  preupdate.iKey1 = iKey1;
  preupdate.iKey2 = iKey2;
  preupdate.pTab = pTab;
  preupdate.iBlobWrite = iBlobWrite;

  db->pPreUpdate = &preupdate;
  db->xPreUpdateCallback(db->pPreUpdateArg, db, op, zDb, zTbl, iKey1, iKey2);
  db->pPreUpdate = 0;
  sqlite3DbFree(db, preupdate.aRecord);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_कुंजी_Field+1,preupdate.pUnpacked);
  vdbeFreeUnpacked(db, preupdate.pKeyinfo->n_कुंजी_Field+1,preupdate.pNewUnpacked);
  sqlite3VdbeMemRelease(&preupdate.oldipk);
  अगर( preupdate.aNew ){
    पूर्णांक i;
    क्रम(i=0; i<pCsr->nField; i++){
      sqlite3VdbeMemRelease(&preupdate.aNew[i]);
    }
    sqlite3DbNNFreeNN(db, preupdate.aNew);
  }
  अगर( preupdate.apDflt ){
    पूर्णांक i;
    क्रम(i=0; i<pTab->nCol; i++){
      sqlite3_मूल्य_Free(preupdate.apDflt[i]);
    }
    sqlite3DbFree(db, preupdate.apDflt);
  }
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

#अगर_घोषित SQLITE_ENABLE_PERCENTILE
/*
** Return the name of an SQL function associated with the sqlite3_context.
*/
स्थिर अक्षर *sqlite3VdbeFuncName(स्थिर sqlite3_context *pCtx){
  निश्चित( pCtx!=0 );
  निश्चित( pCtx->pFunc!=0 );
  वापस pCtx->pFunc->zName;
}
#पूर्ण_अगर /* SQLITE_ENABLE_PERCENTILE */
