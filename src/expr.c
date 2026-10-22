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
** This file contains routines used for analyzing expressions and
** for generating VDBE code that evaluates expressions in SQLite.
*/
#ضمن "sqliteInt.h"

/* Forward declarations */
ساكن فراغ exprCodeBetween(Parse*,Expr*,صحيح,فراغ(*)(Parse*,Expr*,صحيح,صحيح),صحيح);
ساكن صحيح exprCodeVector(Parse *pParse, Expr *p, صحيح *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
حرف sqlite3TableColumnAffinity(ثابت Table *pTab, صحيح iCol){
  ܐܢ( iCol<0 || NEVER(iCol>=pTab->nCol) ) ܦܢܐ SQLITE_AFF_INTEGER;
  ܦܢܐ pTab->aCol[iCol].affinity;
}

/*
** Return the 'affinity' of the expression pExpr if any.
**
** If pExpr is a column, a reference to a column via an 'AS' alias,
** or a sub-select with a column as the return value, then the
** affinity of that column is returned. Otherwise, 0x00 is returned,
** indicating no affinity for the expression.
**
** i.e. the WHERE clause expressions in the following statements all
** have an affinity:
**
** CREATE TABLE t1(a);
** SELECT * FROM t1 WHERE a;
** SELECT a AS b FROM t1 WHERE b;
** SELECT * FROM t1 WHERE (select a from t1);
*/
حرف sqlite3ExprAffinity(ثابت Expr *pExpr){
  صحيح op;
  op = pExpr->op;
  ܟܡܐ( 1 /* exit-by-break */ ){
    ܐܢ( op==TK_COLUMN || (op==TK_AGG_COLUMN && pExpr->y.pTab!=0) ){
      أكد( ExprUseYTab(pExpr) );
      أكد( pExpr->y.pTab!=0 );
      ܦܢܐ sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
    }
    ܐܢ( op==TK_SELECT ){
      أكد( ExprUseXSelect(pExpr) );
      أكد( pExpr->x.pSelect!=0 );
      أكد( pExpr->x.pSelect->pEList!=0 );
      أكد( pExpr->x.pSelect->pEList->a[0].pExpr!=0 );
      ܦܢܐ sqlite3ExprAffinity(pExpr->x.pSelect->pEList->a[0].pExpr);
    }
#ifndef SQLITE_OMIT_CAST
    ܐܢ( op==TK_CAST ){
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      ܦܢܐ sqlite3AffinityType(pExpr->u.zToken, 0);
    }
#endif
    ܐܢ( op==TK_SELECT_COLUMN ){
      أكد( pExpr->pLeft!=0 && ExprUseXSelect(pExpr->pLeft) );
      أكد( pExpr->iColumn < pExpr->iTable );
      أكد( pExpr->iColumn >= 0 );
      أكد( pExpr->iTable==pExpr->pLeft->x.pSelect->pEList->nExpr );
      ܦܢܐ sqlite3ExprAffinity(
          pExpr->pLeft->x.pSelect->pEList->a[pExpr->iColumn].pExpr
      );
    }
    ܐܢ( op==TK_VECTOR
     || (op==TK_FUNCTION && pExpr->affExpr==SQLITE_AFF_DEFER)
    ){
      أكد( ExprUseXList(pExpr) );
      ܦܢܐ sqlite3ExprAffinity(pExpr->x.pList->a[0].pExpr);
    }
    ܐܢ( ExprHasProperty(pExpr, EP_Skip|EP_IfNullRow) ){
      أكد( pExpr->op==TK_COLLATE
           || pExpr->op==TK_IF_NULL_ROW
           || (pExpr->op==TK_REGISTER && pExpr->op2==TK_IF_NULL_ROW) );
      pExpr = pExpr->pLeft;
      op = pExpr->op;
      استمر;
    }
    ܐܢ( op!=TK_REGISTER ) اكسر;
    op = pExpr->op2;
    ܐܢ( NEVER( op==TK_REGISTER ) ) اكسر;
  }
  ܦܢܐ pExpr->affExpr;
}

/*
** Make a guess at all the possible datatypes of the result that could
** be returned by an expression.  Return a bitmask indicating the answer:
**
**     0x01         Numeric
**     0x02         Text
**     0x04         Blob
**
** If the expression must return NULL, then 0x00 is returned.
*/
صحيح sqlite3ExprDataType(ثابت Expr *pExpr){
  ܟܡܐ( pExpr ){
    بدل( pExpr->op ){
      حالة TK_COLLATE:
      حالة TK_IF_NULL_ROW:
      حالة TK_UPLUS:  {
        pExpr = pExpr->pLeft;
        اكسر;
      }
      حالة TK_NULL: {
        pExpr = 0;
        اكسر;
      }
      حالة TK_STRING: {
        ܦܢܐ 0x02;
      }
      حالة TK_BLOB: {
        ܦܢܐ 0x04;
      }
      حالة TK_CONCAT: {
        ܦܢܐ 0x06;
      }
      حالة TK_VARIABLE:
      حالة TK_AGG_FUNCTION:
      حالة TK_FUNCTION: {
        ܦܢܐ 0x07;
      }
      حالة TK_COLUMN:
      حالة TK_AGG_COLUMN:
      حالة TK_SELECT:
      حالة TK_CAST:
      حالة TK_SELECT_COLUMN:
      حالة TK_VECTOR:  {
        صحيح aff = sqlite3ExprAffinity(pExpr);
        ܐܢ( aff>=SQLITE_AFF_NUMERIC ) ܦܢܐ 0x05;
        ܐܢ( aff==SQLITE_AFF_TEXT )    ܦܢܐ 0x06;
        ܦܢܐ 0x07;
      }
      حالة TK_CASE: {
        صحيح res = 0;
        صحيح ii;
        ExprList *pList = pExpr->x.pList;
        أكد( ExprUseXList(pExpr) && pList!=0 );
        أكد( pList->nExpr > 0);
        لكل(ii=1; ii<pList->nExpr; ii+=2){
          res |= sqlite3ExprDataType(pList->a[ii].pExpr);
        }
        ܐܢ( pList->nExpr % 2 ){
          res |= sqlite3ExprDataType(pList->a[pList->nExpr-1].pExpr);
        }
        ܦܢܐ res;
      }
      افتراضي: {
        ܦܢܐ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  ܦܢܐ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
Expr *sqlite3ExprAddCollateToken(
  Parse *pParse,           /* Parsing context */
  Expr *pExpr,             /* Add the "COLLATE" clause to this expression */
  ثابت Token *pCollName,  /* Name of collating sequence */
  صحيح dequote              /* True to dequote pCollName */
){
  ܐܢ( pCollName->n>0 && pParse->nErr==0 ){
    Expr *pNew = sqlite3ExprAlloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( pExpr!=0 );
    ܐܢ( pNew ){
      pNew->pLeft = pExpr;
      pNew->flags |= EP_Collate|EP_Skip;
#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
      pNew->nHeight = pExpr->nHeight+1;
      sqlite3ExprCheckHeight(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      pExpr = pNew;
    }
  }
  ܦܢܐ pExpr;
}
Expr *sqlite3ExprAddCollateString(
  Parse *pParse,        /* Parsing context */
  Expr *pExpr,          /* Add the "COLLATE" clause to this expression */
  ثابت حرف *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (حرف*)zC);
  ܦܢܐ sqlite3ExprAddCollateToken(pParse, pExpr, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
Expr *sqlite3ExprSkipCollate(Expr *pExpr){
  ܟܡܐ( pExpr && ExprHasProperty(pExpr, EP_Skip) ){
    أكد( pExpr->op==TK_COLLATE );
    pExpr = pExpr->pLeft;
  }  
  ܦܢܐ pExpr;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
Expr *sqlite3ExprSkipCollateAndLikely(Expr *pExpr){
  ܟܡܐ( pExpr && ExprHasProperty(pExpr, EP_Skip|EP_Unlikely) ){
    ܐܢ( ExprHasProperty(pExpr, EP_Unlikely) ){
      أكد( ExprUseXList(pExpr) );
      أكد( pExpr->x.pList->nExpr>0 );
      أكد( pExpr->op==TK_FUNCTION );
      pExpr = pExpr->x.pList->a[0].pExpr;
    }ܐܚܪܢܐ ܐܢ( pExpr->op==TK_COLLATE ){
      pExpr = pExpr->pLeft;
    }ܐܚܪܢܐ{
      اكسر;
    }
  }  
  ܦܢܐ pExpr;
}

/*
** Return the collation sequence for the expression pExpr. If
** there is no defined collating sequence, return NULL.
**
** See also: sqlite3ExprNNCollSeq()
**
** The sqlite3ExprNNCollSeq() works the same exact that it returns the
** default collation if pExpr has no defined collation.
**
** The collating sequence might be determined by a COLLATE operator
** or by the presence of a column with a defined collating sequence.
** COLLATE operators take first precedence.  Left operands take
** precedence over right operands.
*/
CollSeq *sqlite3ExprCollSeq(Parse *pParse, ثابت Expr *pExpr){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ثابت Expr *p = pExpr;
  ܟܡܐ( p ){
    صحيح op = p->op;
    ܐܢ( op==TK_REGISTER ) op = p->op2;
    ܐܢ( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      صحيح j;
      أكد( ExprUseYTab(p) );
      أكد( p->y.pTab!=0 );
      ܐܢ( (j = p->iColumn)>=0 ){
        ثابت حرف *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      اكسر;
    }
    ܐܢ( op==TK_CAST || op==TK_UPLUS ){
      p = p->pLeft;
      استمر;
    }
    ܐܢ( op==TK_VECTOR
     || (op==TK_FUNCTION && p->affExpr==SQLITE_AFF_DEFER)
    ){
      أكد( ExprUseXList(p) );
      p = p->x.pList->a[0].pExpr;
      استمر;
    }
    ܐܢ( op==TK_COLLATE ){
      أكد( !ExprHasProperty(p, EP_IntValue) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      اكسر;
    }
    ܐܢ( p->flags & EP_Collate ){
      ܐܢ( p->pLeft && (p->pLeft->flags & EP_Collate)!=0 ){
        p = p->pLeft;
      }ܐܚܪܢܐ{
        Expr *pNext  = p->pRight;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !ExprUseXList(p) || p->x.pList==0 || p->pRight==0 );
        ܐܢ( ExprUseXList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحيح i;
          لكل(i=0; i<p->x.pList->nExpr; i++){
            ܐܢ( ExprHasProperty(p->x.pList->a[i].pExpr, EP_Collate) ){
              pNext = p->x.pList->a[i].pExpr;
              اكسر;
            }
          }
        }
        p = pNext;
      }
    }ܐܚܪܢܐ{
      اكسر;
    }
  }
  ܐܢ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  ܦܢܐ pColl;
}

/*
** Return the collation sequence for the expression pExpr. If
** there is no defined collating sequence, return a pointer to the
** default collation sequence.
**
** See also: sqlite3ExprCollSeq()
**
** The sqlite3ExprCollSeq() routine works the same except that it
** returns NULL if there is no defined collation.
*/
CollSeq *sqlite3ExprNNCollSeq(Parse *pParse, ثابت Expr *pExpr){
  CollSeq *p = sqlite3ExprCollSeq(pParse, pExpr);
  ܐܢ( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  ܦܢܐ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحيح sqlite3ExprCollSeqMatch(Parse *pParse, ثابت Expr *pE1, ثابت Expr *pE2){
  CollSeq *pColl1 = sqlite3ExprNNCollSeq(pParse, pE1);
  CollSeq *pColl2 = sqlite3ExprNNCollSeq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  ܦܢܐ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
حرف sqlite3CompareAffinity(ثابت Expr *pExpr, حرف aff2){
  حرف aff1 = sqlite3ExprAffinity(pExpr);
  ܐܢ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ܐܢ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      ܦܢܐ SQLITE_AFF_NUMERIC;
    }ܐܚܪܢܐ{
      ܦܢܐ SQLITE_AFF_BLOB;
    }
  }ܐܚܪܢܐ{
    /* One side is a column, the other is not. Use the columns affinity. */
    أكد( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    ܦܢܐ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ساكن حرف comparisonAffinity(ثابت Expr *pExpr){
  حرف aff;
  أكد( pExpr->op==TK_EQ || pExpr->op==TK_IN || pExpr->op==TK_LT ||
          pExpr->op==TK_GT || pExpr->op==TK_GE || pExpr->op==TK_LE ||
          pExpr->op==TK_NE || pExpr->op==TK_IS || pExpr->op==TK_ISNOT );
  أكد( pExpr->pLeft );
  أكد( !ExprUseXSelect(pExpr) );
  aff = sqlite3ExprAffinity(pExpr->pLeft);
  ܐܢ( pExpr->pRight ){
    aff = sqlite3CompareAffinity(pExpr->pRight, aff);
  }ܐܚܪܢܐ ܐܢ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  ܦܢܐ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
صحيح sqlite3IndexAffinityOk(ثابت Expr *pExpr, حرف idx_affinity){
  حرف aff = comparisonAffinity(pExpr);
  ܐܢ( aff<SQLITE_AFF_TEXT ){
    ܦܢܐ 1;
  }
  ܐܢ( aff==SQLITE_AFF_TEXT ){
    ܦܢܐ idx_affinity==SQLITE_AFF_TEXT;
  }
  ܦܢܐ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ساكن u8 binaryCompareP5(
  ثابت Expr *pExpr1,   /* Left operand */
  ثابت Expr *pExpr2,   /* Right operand */
  صحيح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (حرف)sqlite3ExprAffinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  ܦܢܐ aff;
}

/*
** Return a pointer to the collation sequence that should be used by
** a binary comparison operator comparing pLeft and pRight.
**
** If the left hand expression has a collating sequence type, then it is
** used. Otherwise the collation sequence for the right hand expression
** is used, or the default (BINARY) if neither expression has a collating
** type.
**
** Argument pRight (but not pLeft) may be a null pointer. In this case,
** it is not considered.
*/
CollSeq *sqlite3BinaryCompareCollSeq(
  Parse *pParse,
  ثابت Expr *pLeft,
  ثابت Expr *pRight
){
  CollSeq *pColl;
  أكد( pLeft );
  ܐܢ( pLeft->flags & EP_Collate ){
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
  }ܐܚܪܢܐ ܐܢ( pRight && (pRight->flags & EP_Collate)!=0 ){
    pColl = sqlite3ExprCollSeq(pParse, pRight);
  }ܐܚܪܢܐ{
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
    ܐܢ( !pColl ){
      pColl = sqlite3ExprCollSeq(pParse, pRight);
    }
  }
  ܦܢܐ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3ExprCompareCollSeq(Parse *pParse, ثابت Expr *p){
  ܐܢ( ExprHasProperty(p, EP_Commuted) ){
    ܦܢܐ sqlite3BinaryCompareCollSeq(pParse, p->pRight, p->pLeft);
  }ܐܚܪܢܐ{
    ܦܢܐ sqlite3BinaryCompareCollSeq(pParse, p->pLeft, p->pRight);
  }
}

/*
** Generate code for a comparison operator.
*/
ساكن صحيح codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  Expr *pLeft,      /* The left operand */
  Expr *pRight,     /* The right operand */
  صحيح opcode,       /* The comparison opcode */
  صحيح in1, صحيح in2, /* Register holding operands */
  صحيح dest,         /* Jump here if true.  */
  صحيح jumpIfNull,   /* If true, jump if either operand is NULL */
  صحيح isCommuted    /* The comparison has been commuted */
){
  صحيح p5;
  صحيح addr;
  CollSeq *p4;

  ܐܢ( pParse->nErr ) ܦܢܐ 0;
  ܐܢ( isCommuted ){
    p4 = sqlite3BinaryCompareCollSeq(pParse, pRight, pLeft);
  }ܐܚܪܢܐ{
    p4 = sqlite3BinaryCompareCollSeq(pParse, pLeft, pRight);
  }
  p5 = binaryCompareP5(pLeft, pRight, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (فراغ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  ܦܢܐ addr;
}

/*
** Return true if expression pExpr is a vector, or false otherwise.
**
** A vector is defined as any expression that results in two or more
** columns of result.  Every TK_VECTOR node is an vector because the
** parser will not generate a TK_VECTOR with fewer than two entries.
** But a TK_SELECT might be either a vector or a scalar. It is only
** considered a vector if it has two or more result columns.
*/
صحيح sqlite3ExprIsVector(ثابت Expr *pExpr){
  ܦܢܐ sqlite3ExprVectorSize(pExpr)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحيح sqlite3ExprVectorSize(ثابت Expr *pExpr){
  u8 op = pExpr->op;
  ܐܢ( op==TK_REGISTER ) op = pExpr->op2;
  ܐܢ( op==TK_VECTOR ){
    أكد( ExprUseXList(pExpr) );
    ܦܢܐ pExpr->x.pList->nExpr;
  }ܐܚܪܢܐ ܐܢ( op==TK_SELECT ){
    أكد( ExprUseXSelect(pExpr) );
    ܦܢܐ pExpr->x.pSelect->pEList->nExpr;
  }ܐܚܪܢܐ{
    ܦܢܐ 1;
  }
}

/*
** Return a pointer to a subexpression of pVector that is the i-th
** column of the vector (numbered starting with 0).  The caller must
** ensure that i is within range.
**
** If pVector is really a scalar (and "scalar" here includes subqueries
** that return a single column!) then return pVector unmodified.
**
** pVector retains ownership of the returned subexpression.
**
** If the vector is a (SELECT ...) then the expression returned is
** just the expression for the i-th term of the result set, and may
** not be ready for evaluation because the table cursor has not yet
** been positioned.
*/
Expr *sqlite3VectorFieldSubexpr(Expr *pVector, صحيح i){
  أكد( i<sqlite3ExprVectorSize(pVector) || pVector->op==TK_ERROR );
  ܐܢ( sqlite3ExprIsVector(pVector) ){
    أكد( pVector->op2==0 || pVector->op==TK_REGISTER );
    ܐܢ( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      أكد( ExprUseXSelect(pVector) );
      ܦܢܐ pVector->x.pSelect->pEList->a[i].pExpr;
    }ܐܚܪܢܐ{
      أكد( ExprUseXList(pVector) );
      ܦܢܐ pVector->x.pList->a[i].pExpr;
    }
  }
  ܦܢܐ pVector;
}

/*
** Compute and return a new Expr object which when passed to
** sqlite3ExprCode() will generate all necessary code to compute
** the iField-th column of the vector expression pVector.
**
** It is ok for pVector to be a scalar (as long as iField==0). 
** In that case, this routine works like sqlite3ExprDup().
**
** The caller owns the returned Expr object and is responsible for
** ensuring that the returned value eventually gets freed.
**
** The caller retains ownership of pVector.  If pVector is a TK_SELECT,
** then the returned object will reference pVector and so pVector must remain
** valid for the life of the returned object.  If pVector is a TK_VECTOR
** or a scalar expression, then it can be deleted as soon as this routine
** returns.
**
** A trick to cause a TK_SELECT pVector to be deleted together with
** the returned Expr object is to attach the pVector to the pRight field
** of the returned TK_SELECT_COLUMN Expr object.
*/
Expr *sqlite3ExprForVectorField(
  Parse *pParse,       /* Parsing context */
  Expr *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  صحيح iField,          /* Which column of the vector to return */
  صحيح nField           /* Total number of columns in the vector */
){
  Expr *pRet;
  ܐܢ( pVector->op==TK_SELECT ){
    أكد( ExprUseXSelect(pVector) );
    /* The TK_SELECT_COLUMN Expr node:
    **
    ** pLeft:           pVector containing TK_SELECT.  Not deleted.
    ** pRight:          not used.  But recursively deleted.
    ** iColumn:         Index of a column in pVector
    ** iTable:          0 or the number of columns on the LHS of an assignment
    ** pLeft->iTable:   First in an array of register holding result, or 0
    **                  if the result is not yet computed.
    **
    ** sqlite3ExprDelete() specifically skips the recursive delete of
    ** pLeft on TK_SELECT_COLUMN nodes.  But pRight is followed, so pVector
    ** can be attached to pRight to cause this node to take ownership of
    ** pVector.  Typically there will be multiple TK_SELECT_COLUMN nodes
    ** with the same pLeft pointer to the pVector, but only one of them
    ** will own the pVector.
    */
    pRet = sqlite3PExpr(pParse, TK_SELECT_COLUMN, 0, 0);
    ܐܢ( pRet ){
      ExprSetProperty(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->pLeft = pVector;
    }
  }ܐܚܪܢܐ{
    ܐܢ( pVector->op==TK_VECTOR ){
      Expr **ppVector;
      أكد( ExprUseXList(pVector) );
      ppVector = &pVector->x.pList->a[iField].pExpr;
      pVector = *ppVector;
      ܐܢ( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        ܦܢܐ pVector;
      }
    }
    pRet = sqlite3ExprDup(pParse->db, pVector, 0);
  }
  ܦܢܐ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ساكن صحيح exprCodeSubselect(Parse *pParse, Expr *pExpr){
  صحيح reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  ܐܢ( pExpr->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, pExpr);
  }
#endif
  ܦܢܐ reg;
}

/*
** Argument pVector points to a vector expression - either a TK_VECTOR
** or TK_SELECT that returns more than one column. This function returns
** the register number of a register that contains the value of
** element iField of the vector.
**
** If pVector is a TK_SELECT expression, then code for it must have
** already been generated using the exprCodeSubselect() routine. In this
** case parameter regSelect should be the first in an array of registers
** containing the results of the sub-select.
**
** If pVector is of type TK_VECTOR, then code for the requested field
** is generated. In this case (*pRegFree) may be set to the number of
** a temporary register to be freed by the caller before returning.
**
** Before returning, output parameter (*ppExpr) is set to point to the
** Expr object corresponding to element iElem of the vector.
*/
ساكن صحيح exprVectorRegister(
  Parse *pParse,                  /* Parse context */
  Expr *pVector,                  /* Vector to extract element from */
  صحيح iField,                     /* Field to extract from pVector */
  صحيح regSelect,                  /* First in array of registers */
  Expr *pTmp,                     /* Temporary space */
  Expr **ppExpr,                  /* OUT: Expression element */
  صحيح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  أكد( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  ܐܢ( op==TK_REGISTER ){
    *ppExpr = sqlite3VectorFieldSubexpr(pVector, iField);
    ܦܢܐ pVector->iTable+iField;
  }
  ܐܢ( op==TK_SELECT ){
    أكد( ExprUseXSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, حجم(Expr));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->pLeft = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->nExpr;
    *ppExpr = pTmp;
    ܦܢܐ regSelect+iField;
  }
  ܐܢ( op==TK_VECTOR ){
    أكد( ExprUseXList(pVector) );
    *ppExpr = pVector->x.pList->a[iField].pExpr;
    ܦܢܐ sqlite3ExprCodeTemp(pParse, *ppExpr, pRegFree);
  }
  ܦܢܐ 0;
}

/*
** Expression pExpr is a comparison between two vector values. Compute
** the result of the comparison (1, 0, or NULL) and write that
** result into register dest.
**
** The caller must satisfy the following preconditions:
**
**    if pExpr->op==TK_IS:      op==TK_EQ and p5==SQLITE_NULLEQ
**    if pExpr->op==TK_ISNOT:   op==TK_NE and p5==SQLITE_NULLEQ
**    otherwise:                op==pExpr->op and p5==0
*/
ساكن فراغ codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  Expr *pExpr,          /* The comparison operation */
  صحيح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  Expr *pLeft = pExpr->pLeft;
  Expr *pRight = pExpr->pRight;
  صحيح nLeft = sqlite3ExprVectorSize(pLeft);
  صحيح i;
  صحيح regLeft = 0;
  صحيح regRight = 0;
  u8 opx = op;
  صحيح addrCmp = 0;
  صحيح addrDone = sqlite3VdbeMakeLabel(pParse);
  صحيح isCommuted = ExprHasProperty(pExpr,EP_Commuted);

  أكد( !ExprHasVVAProperty(pExpr,EP_Immutable) );
  ܐܢ( pParse->nErr ) ܦܢܐ;
  ܐܢ( nLeft!=sqlite3ExprVectorSize(pRight) ){
    sqlite3ErrorMsg(pParse, "row value misused");
    ܦܢܐ;
  }
  أكد( pExpr->op==TK_EQ || pExpr->op==TK_NE
       || pExpr->op==TK_IS || pExpr->op==TK_ISNOT
       || pExpr->op==TK_LT || pExpr->op==TK_GT
       || pExpr->op==TK_LE || pExpr->op==TK_GE
  );
  أكد( pExpr->op==op || (pExpr->op==TK_IS && op==TK_EQ)
            || (pExpr->op==TK_ISNOT && op==TK_NE) );
  أكد( p5==0 || pExpr->op!=op );
  أكد( p5==SQLITE_NULLEQ || pExpr->op==op );

  ܐܢ( op==TK_LE ) opx = TK_LT;
  ܐܢ( op==TK_GE ) opx = TK_GT;
  ܐܢ( op==TK_NE ) opx = TK_EQ;

  regLeft = exprCodeSubselect(pParse, pLeft);
  regRight = exprCodeSubselect(pParse, pRight);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  لكل(i=0; 1 /*Loop exits by "break"*/; i++){
    صحيح regFree1 = 0, regFree2 = 0;
    Expr *pL = 0, *pR = 0;
    Expr tmp1, tmp2;
    صحيح r1, r2;
    أكد( i>=0 && i<nLeft );
    ܐܢ( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = exprVectorRegister(pParse, pLeft, i, regLeft, &tmp1, &pL, &regFree1);
    r2 = exprVectorRegister(pParse, pRight, i, regRight, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    codeCompare(pParse, pL, pR, opx, r1, r2, addrDone, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3ReleaseTempReg(pParse, regFree1);
    sqlite3ReleaseTempReg(pParse, regFree2);
    ܐܢ( (opx==TK_LT || opx==TK_GT) && i<nLeft-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ܐܢ( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }ܐܚܪܢܐ{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ܐܢ( i==nLeft-1 ){
      اكسر;
    }
    ܐܢ( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }ܐܚܪܢܐ{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      ܐܢ( i==nLeft-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  ܐܢ( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحيح sqlite3ExprCheckHeight(Parse *pParse, صحيح nHeight){
  صحيح rc = SQLITE_OK;
  صحيح mxHeight = pParse->db->aLimit[SQLITE_LIMIT_EXPR_DEPTH];
  ܐܢ( nHeight>mxHeight ){
    sqlite3ErrorMsg(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  ܦܢܐ rc;
}

/* The following three functions, heightOfExpr(), heightOfExprList()
** and heightOfSelect(), are used to determine the maximum height
** of any expression tree referenced by the structure passed as the
** first argument.
**
** If this maximum height is greater than the current value pointed
** to by pnHeight, the second parameter, then set *pnHeight to that
** value.
*/
ساكن فراغ heightOfExpr(ثابت Expr *p, صحيح *pnHeight){
  ܐܢ( p ){
    ܐܢ( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
ساكن فراغ heightOfExprList(ثابت ExprList *p, صحيح *pnHeight){
  ܐܢ( p ){
    صحيح i;
    لكل(i=0; i<p->nExpr; i++){
      heightOfExpr(p->a[i].pExpr, pnHeight);
    }
  }
}
ساكن فراغ heightOfSelect(ثابت Select *pSelect, صحيح *pnHeight){
  ثابت Select *p;
  لكل(p=pSelect; p; p=p->pPrior){
    heightOfExpr(p->pWhere, pnHeight);
    heightOfExpr(p->pHaving, pnHeight);
    heightOfExpr(p->pLimit, pnHeight);
    heightOfExprList(p->pEList, pnHeight);
    heightOfExprList(p->pGroupBy, pnHeight);
    heightOfExprList(p->pOrderBy, pnHeight);
  }
}

/*
** Set the Expr.nHeight variable in the structure passed as an
** argument. An expression with no children, Expr.pList or
** Expr.pSelect member has a height of 1. Any other expression
** has a height equal to the maximum height of any other
** referenced Expr plus one.
**
** Also propagate EP_Propagate flags up from Expr.x.pList to Expr.flags,
** if appropriate.
*/
ساكن فراغ exprSetHeight(Expr *p){
  صحيح nHeight = p->pLeft ? p->pLeft->nHeight : 0;
  ܐܢ( NEVER(p->pRight) && p->pRight->nHeight>nHeight ){
    nHeight = p->pRight->nHeight;
  }
  ܐܢ( ExprUseXSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }ܐܚܪܢܐ ܐܢ( p->x.pList ){
    heightOfExprList(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3ExprListFlags(p->x.pList);
  }
  p->nHeight = nHeight + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p){
  ܐܢ( pParse->nErr ) ܦܢܐ;
  exprSetHeight(p);
  sqlite3ExprCheckHeight(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحيح sqlite3SelectExprHeight(ثابت Select *p){
  صحيح nHeight = 0;
  heightOfSelect(p, &nHeight);
  ܦܢܐ nHeight;
}
#ܐܚܪܢܐ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p){
  ܐܢ( pParse->nErr ) ܦܢܐ;
  ܐܢ( p && ExprUseXList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3ExprListFlags(p->x.pList);
  }
}
#define exprSetHeight(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
فراغ sqlite3ExprSetErrorOffset(Expr *pExpr, صحيح iOfst){
  ܐܢ( pExpr==0 ) ܦܢܐ;
  ܐܢ( NEVER(ExprUseWJoin(pExpr)) ) ܦܢܐ;
  pExpr->w.iOfst = iOfst;
}

/*
** This routine is the core allocator for Expr nodes.
**
** Construct a new expression node and return a pointer to it.  Memory
** for this node and for the pToken argument is a single allocation
** obtained from sqlite3DbMalloc().  The calling function
** is responsible for making sure the node eventually gets freed.
**
** If dequote is true, then the token (if it exists) is dequoted.
** If dequote is false, no dequoting is performed.  The deQuote
** parameter is ignored if pToken is NULL or if the token does not
** appear to be quoted.  If the quotes were of the form "..." (double-quotes)
** then the EP_DblQuoted flag is set on the expression node.
**
** Special case (tag-20240227-a):  If op==TK_INTEGER and pToken points to
** a string that can be translated into a 32-bit integer, then the token is
** not stored in u.zToken.  Instead, the integer values is written
** into u.iValue and the EP_IntValue flag is set. No extra storage
** is allocated to hold the integer text and the dequote flag is ignored.
** See also tag-20240227-b.
*/
Expr *sqlite3ExprAlloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  صحيح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحيح dequote             /* True to dequote */
){
  Expr *pNew;
  صحيح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, حجم(Expr)+nExtra);
  ܐܢ( pNew ){
    memset(pNew, 0, حجم(Expr));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ܐܢ( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (حرف*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      ܐܢ( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ܐܢ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3DequoteExpr(pNew);
      }
    }
#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ܦܢܐ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
Expr *sqlite3Expr(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحيح op,                 /* Expression opcode */
  ثابت حرف *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  ܦܢܐ sqlite3ExprAlloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
Expr *sqlite3ExprInt32(sqlite3 *db, صحيح iVal){
  Expr *pNew = sqlite3DbMallocRawNN(db, حجم(Expr));
  ܐܢ( pNew ){
    memset(pNew, 0, حجم(Expr));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_IntValue|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.iValue = iVal;
#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  ܦܢܐ pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
فراغ sqlite3ExprAttachSubtrees(
  sqlite3 *db,
  Expr *pRoot,
  Expr *pLeft,
  Expr *pRight
){
  ܐܢ( pRoot==0 ){
    أكد( db->mallocFailed );
    sqlite3ExprDelete(db, pLeft);
    sqlite3ExprDelete(db, pRight);
  }ܐܚܪܢܐ{
    أكد( ExprUseXList(pRoot) );
    أكد( pRoot->x.pSelect==0 );
    ܐܢ( pRight ){
      pRoot->pRight = pRight;
      pRoot->flags |= EP_Propagate & pRight->flags;
#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
      pRoot->nHeight = pRight->nHeight+1;
    }ܐܚܪܢܐ{
      pRoot->nHeight = 1;
#endif
    }
    ܐܢ( pLeft ){
      pRoot->pLeft = pLeft;
      pRoot->flags |= EP_Propagate & pLeft->flags;
#ܐܢ SQLITE_MAX_EXPR_DEPTH>0
      ܐܢ( pLeft->nHeight>=pRoot->nHeight ){
        pRoot->nHeight = pLeft->nHeight+1;
      }
#endif
    }
  }
}

/*
** Allocate an Expr node which joins as many as two subtrees.
**
** One or both of the subtrees can be NULL.  Return a pointer to the new
** Expr node.  Or, if an OOM error occurs, set pParse->db->mallocFailed,
** free the subtrees and return NULL.
*/
Expr *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  صحيح op,                 /* Expression opcode */
  Expr *pLeft,            /* Left operand */
  Expr *pRight            /* Right operand */
){
  Expr *p;
  p = sqlite3DbMallocRawNN(pParse->db, حجم(Expr));
  ܐܢ( p ){
    memset(p, 0, حجم(Expr));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3ExprAttachSubtrees(pParse->db, p, pLeft, pRight);
    sqlite3ExprCheckHeight(pParse, p->nHeight);
  }ܐܚܪܢܐ{
    sqlite3ExprDelete(pParse->db, pLeft);
    sqlite3ExprDelete(pParse->db, pRight);
  }
  ܦܢܐ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
فراغ sqlite3PExprAddSelect(Parse *pParse, Expr *pExpr, Select *pSelect){
  ܐܢ( pExpr ){
    pExpr->x.pSelect = pSelect;
    ExprSetProperty(pExpr, EP_xIsSelect|EP_Subquery);
    sqlite3ExprSetHeightAndFlags(pParse, pExpr);
  }ܐܚܪܢܐ{
    أكد( pParse->db->mallocFailed );
    sqlite3SelectDelete(pParse->db, pSelect);
  }
}

/*
** Expression list pEList is a list of vector values. This function
** converts the contents of pEList to a VALUES(...) Select statement
** returning 1 row for each element of the list. For example, the
** expression list:
**
**   ( (1,2), (3,4) (5,6) )
**
** is translated to the equivalent of:
**
**   VALUES(1,2), (3,4), (5,6)
**
** Each of the vector values in pEList must contain exactly nElem terms.
** If a list element that is not a vector or does not contain nElem terms,
** an error message is left in pParse.
**
** This is used as part of processing IN(...) expressions with a list
** of vectors on the RHS. e.g. "... IN ((1,2), (3,4), (5,6))".
*/
Select *sqlite3ExprListToValues(Parse *pParse, صحيح nElem, ExprList *pEList){
  صحيح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  لكل(ii=0; ii<pEList->nExpr; ii++){
    Select *pSel;
    Expr *pExpr = pEList->a[ii].pExpr;
    صحيح nExprElem;
    ܐܢ( pExpr->op==TK_VECTOR ){
      أكد( ExprUseXList(pExpr) );
      nExprElem = pExpr->x.pList->nExpr;
    }ܐܚܪܢܐ{
      nExprElem = 1;
    }
    ܐܢ( nExprElem!=nElem ){
      sqlite3ErrorMsg(pParse, "IN(...) element has %d term%s - expected %d",
          nExprElem, nExprElem>1?"s":"", nElem
      );
      اكسر;
    }
    أكد( ExprUseXList(pExpr) );
    pSel = sqlite3SelectNew(pParse, pExpr->x.pList, 0, 0, 0, 0, 0, SF_Values,0);
    pExpr->x.pList = 0;
    ܐܢ( pSel ){
      ܐܢ( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  ܐܢ( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_MultiValue;
  }
  sqlite3ExprListDelete(pParse->db, pEList);
  ܦܢܐ pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
Expr *sqlite3ExprAnd(Parse *pParse, Expr *pLeft, Expr *pRight){
  sqlite3 *db = pParse->db;
  ܐܢ( pLeft==0  ){
    ܦܢܐ pRight;
  }ܐܚܪܢܐ ܐܢ( pRight==0 ){
    ܦܢܐ pLeft;
  }ܐܚܪܢܐ{
    u32 f = pLeft->flags | pRight->flags;
    ܐܢ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      Expr *p;
      sqlite3ExprDeferredDelete(pParse, pLeft);
      sqlite3ExprDeferredDelete(pParse, pRight);
      p = sqlite3ExprInt32(db, 0);
      ܐܢ( p ) p->op2 = 1;
      ܦܢܐ p;
    }ܐܚܪܢܐ{
      ܦܢܐ sqlite3PExpr(pParse, TK_AND, pLeft, pRight);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
Expr *sqlite3ExprFunction(
  Parse *pParse,        /* Parsing context */
  ExprList *pList,      /* Argument list */
  ثابت Token *pToken,  /* Name of the function */
  صحيح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  Expr *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3ExprAlloc(db, TK_FUNCTION, pToken, 1);
  ܐܢ( pNew==0 ){
    sqlite3ExprListDelete(db, pList); /* Avoid memory leak when malloc fails */
    ܦܢܐ 0;
  }
  أكد( !ExprHasProperty(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحيح)(pToken->z - pParse->zTail);
  ܐܢ( pList
   && pList->nExpr > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ExprSetProperty(pNew, EP_HasFunc);
  أكد( ExprUseXList(pNew) );
  sqlite3ExprSetHeightAndFlags(pParse, pNew);
  ܐܢ( eDistinct==SF_Distinct ) ExprSetProperty(pNew, EP_Distinct);
  ܦܢܐ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
فراغ sqlite3ExprOrderByAggregateError(Parse *pParse, Expr *p){
  sqlite3ErrorMsg(pParse,
     "ORDER BY may not be used with non-aggregate %#T()", p
  );
}

/*
** Attach an ORDER BY clause to a function call.
**
**     functionname( arguments ORDER BY sortlist )
**     \_____________________/          \______/
**             pExpr                    pOrderBy
**
** The ORDER BY clause is inserted into a new Expr node of type TK_ORDER
** and added to the Expr.pLeft field of the parent TK_FUNCTION node.
*/
فراغ sqlite3ExprAddFunctionOrderBy(
  Parse *pParse,        /* Parsing context */
  Expr *pExpr,          /* The function call to which ORDER BY is to be added */
  ExprList *pOrderBy    /* The ORDER BY clause to add */
){
  Expr *pOB;
  sqlite3 *db = pParse->db;
  ܐܢ( NEVER(pOrderBy==0) ){
    أكد( db->mallocFailed );
    ܦܢܐ;
  }
  ܐܢ( pExpr==0 ){
    أكد( db->mallocFailed );
    sqlite3ExprListDelete(db, pOrderBy);
    ܦܢܐ;
  }
  أكد( pExpr->op==TK_FUNCTION );
  أكد( pExpr->pLeft==0 );
  أكد( ExprUseXList(pExpr) );
  ܐܢ( pExpr->x.pList==0 || NEVER(pExpr->x.pList->nExpr==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, pOrderBy);
    ܦܢܐ;
  }
  ܐܢ( IsWindowFunc(pExpr) ){
    sqlite3ExprOrderByAggregateError(pParse, pExpr);
    sqlite3ExprListDelete(db, pOrderBy);
    ܦܢܐ;
  }
  ܐܢ( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    sqlite3ExprListDelete(db, pOrderBy);
    ܦܢܐ;
  }

  pOB = sqlite3ExprAlloc(db, TK_ORDER, 0, 0);
  ܐܢ( pOB==0 ){
    sqlite3ExprListDelete(db, pOrderBy);
    ܦܢܐ;
  }
  pOB->x.pList = pOrderBy;
  أكد( ExprUseXList(pOB) );
  pExpr->pLeft = pOB;
  ExprSetProperty(pOB, EP_FullSize);
}

/*
** Check to see if a function is usable according to current access
** rules:
**
**    SQLITE_FUNC_DIRECT    -     Only usable from top-level SQL
**
**    SQLITE_FUNC_UNSAFE    -     Usable if TRUSTED_SCHEMA or from
**                                top-level SQL
**
** If the function is not usable, create an error.
*/
فراغ sqlite3ExprFunctionUsable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت Expr *pExpr,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !IN_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  ܐܢ( ExprHasProperty(pExpr, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ܐܢ( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3ErrorMsg(pParse, "unsafe use of %#T()", pExpr);
    }
  }
}

/*
** Assign a variable number to an expression that encodes a wildcard
** in the original SQL statement. 
**
** Wildcards consisting of a single "?" are assigned the next sequential
** variable number.
**
** Wildcards of the form "?nnn" are assigned the number "nnn".  We make
** sure "nnn" is not too big to avoid a denial of service attack when
** the SQL statement comes from an external source.
**
** Wildcards of the form ":aaa", "@aaa", or "$aaa" are assigned the same number
** as the previous instance of the same wildcard.  Or if this is the first
** instance of the wildcard, the next sequential variable number is
** assigned.
*/
فراغ sqlite3ExprAssignVarNumber(Parse *pParse, Expr *pExpr, u32 n){
  sqlite3 *db = pParse->db;
  ثابت حرف *z;
  ynVar x;

  ܐܢ( pExpr==0 ) ܦܢܐ;
  أكد( !ExprHasProperty(pExpr, EP_IntValue|EP_Reduced|EP_TokenOnly) );
  z = pExpr->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  ܐܢ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ܐܚܪܢܐ{
    صحيح doAdd = 0;
    ܐܢ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      صحيح bOk;
      ܐܢ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ܐܚܪܢܐ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] );
      ܐܢ( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
        sqlite3ErrorMsg(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]);
        sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
        ܦܢܐ;
      }
      x = (ynVar)i;
      ܐܢ( x>pParse->nVar ){
        pParse->nVar = (صحيح)x;
        doAdd = 1;
      }ܐܚܪܢܐ ܐܢ( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ܐܚܪܢܐ ܐܢ( sqlite3VListNumToName(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }ܐܚܪܢܐ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3VListNameToNum(pParse->pVList, z, n);
      ܐܢ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    ܐܢ( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      ܐܢ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  pExpr->iColumn = x;
  ܐܢ( x>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
    sqlite3ErrorMsg(pParse, "too many SQL variables");
    sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
  }
}

/*
** Recursively delete an expression tree.
*/
ساكن SQLITE_NOINLINE فراغ sqlite3ExprDeleteNN(sqlite3 *db, Expr *p){
  أكد( p!=0 );
  أكد( db!=0 );
exprDeleteRestart:
  أكد( !ExprUseUValue(p) || p->u.iValue>=0 );
  أكد( !ExprUseYWin(p) || !ExprUseYSub(p) );
  أكد( !ExprUseYWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !ExprUseYSub(p) );
#ifdef SQLITE_DEBUG
  ܐܢ( ExprHasProperty(p, EP_Leaf) && !ExprHasProperty(p, EP_TokenOnly) ){
    أكد( p->pLeft==0 );
    أكد( p->pRight==0 );
    أكد( !ExprUseXSelect(p) || p->x.pSelect==0 );
    أكد( !ExprUseXList(p) || p->x.pList==0 );
  }
#endif
  ܐܢ( !ExprHasProperty(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (ExprUseXList(p) && p->x.pList==0) || p->pRight==0 );
    ܐܢ( p->pRight ){
      أكد( !ExprHasProperty(p, EP_WinFunc) );
      sqlite3ExprDeleteNN(db, p->pRight);
    }ܐܚܪܢܐ ܐܢ( ExprUseXSelect(p) ){
      أكد( !ExprHasProperty(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ܐܚܪܢܐ{
      sqlite3ExprListDelete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      ܐܢ( ExprHasProperty(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    ܐܢ( p->pLeft && p->op!=TK_SELECT_COLUMN ){
      Expr *pLeft = p->pLeft;
      ܐܢ( !ExprHasProperty(p, EP_Static)
       && !ExprHasProperty(pLeft, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = pLeft;
        اذهب exprDeleteRestart;
      }ܐܚܪܢܐ{
        sqlite3ExprDeleteNN(db, pLeft);
      }
    }
  }
  ܐܢ( !ExprHasProperty(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
فراغ sqlite3ExprDelete(sqlite3 *db, Expr *p){
  ܐܢ( p ) sqlite3ExprDeleteNN(db, p);
}
فراغ sqlite3ExprDeleteGeneric(sqlite3 *db, فراغ *p){
  ܐܢ( ALWAYS(p) ) sqlite3ExprDeleteNN(db, (Expr*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
فراغ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ܐܢ( p==0 ){
    /* Nothing to clear */
  }ܐܚܪܢܐ ܐܢ( p->pOn ){
    sqlite3ExprDeleteNN(db, p->pOn);
  }ܐܚܪܢܐ ܐܢ( p->pUsing ){
    sqlite3IdListDelete(db, p->pUsing);
  }
}

/*
** Arrange to cause pExpr to be deleted when the pParse is deleted.
** This is similar to sqlite3ExprDelete() except that the delete is
** deferred until the pParse is deleted.
**
** The pExpr might be deleted immediately on an OOM error.
**
** Return 0 if the delete was successfully deferred.  Return non-zero
** if the delete happened immediately because of an OOM.
*/
صحيح sqlite3ExprDeferredDelete(Parse *pParse, Expr *pExpr){
  ܦܢܐ 0==sqlite3ParserAddCleanup(pParse, sqlite3ExprDeleteGeneric, pExpr);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
فراغ sqlite3ExprUnmapAndDelete(Parse *pParse, Expr *p){
  ܐܢ( p ){
    ܐܢ( IN_RENAME_OBJECT ){
      sqlite3RenameExprUnmap(pParse, p);
    }
    sqlite3ExprDeleteNN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ساكن صحيح exprStructSize(ثابت Expr *p){
  ܐܢ( ExprHasProperty(p, EP_TokenOnly) ) ܦܢܐ EXPR_TOKENONLYSIZE;
  ܐܢ( ExprHasProperty(p, EP_Reduced) ) ܦܢܐ EXPR_REDUCEDSIZE;
  ܦܢܐ EXPR_FULLSIZE;
}

/*
** The dupedExpr*Size() routines each return the number of bytes required
** to store a copy of an expression or expression tree.  They differ in
** how much of the tree is measured.
**
**     dupedExprStructSize()     Size of only the Expr structure
**     dupedExprNodeSize()       Size of Expr + space for token
**     dupedExprSize()           Expr + token + subtree components
**
***************************************************************************
**
** The dupedExprStructSize() function returns two values OR-ed together: 
** (1) the space required for a copy of the Expr structure only and
** (2) the EP_xxx flags that indicate what the structure size should be.
** The return values is always one of:
**
**      EXPR_FULLSIZE
**      EXPR_REDUCEDSIZE   | EP_Reduced
**      EXPR_TOKENONLYSIZE | EP_TokenOnly
**
** The size of the structure can be found by masking the return value
** of this routine with 0xfff.  The flags can be found by masking the
** return value with EP_Reduced|EP_TokenOnly.
**
** Note that with flags==EXPRDUP_REDUCE, this routines works on full-size
** (unreduced) Expr objects as they or originally constructed by the parser.
** During expression analysis, extra information is computed and moved into
** later parts of the Expr object and that extra information might get chopped
** off if the expression is reduced.  Note also that it does not work to
** make an EXPRDUP_REDUCE copy of a reduced expression.  It is only legal
** to reduce a pristine expression tree from the parser.  The implementation
** of dupedExprStructSize() contain multiple assert() statements that attempt
** to enforce this constraint.
*/
ساكن صحيح dupedExprStructSize(ثابت Expr *p, صحيح flags){
  صحيح nSize;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( EXPR_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ܐܢ( 0==flags || ExprHasProperty(p, EP_FullSize) ){
    nSize = EXPR_FULLSIZE;
  }ܐܚܪܢܐ{
    أكد( !ExprHasProperty(p, EP_TokenOnly|EP_Reduced) );
    أكد( !ExprHasProperty(p, EP_OuterON) );
    أكد( !ExprHasVVAProperty(p, EP_NoReduce) );
    ܐܢ( p->pLeft || p->x.pList ){
      nSize = EXPR_REDUCEDSIZE | EP_Reduced;
    }ܐܚܪܢܐ{
      أكد( p->pRight==0 );
      nSize = EXPR_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  ܦܢܐ nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ساكن صحيح dupedExprNodeSize(ثابت Expr *p, صحيح flags){
  صحيح nByte = dupedExprStructSize(p, flags) & 0xfff;
  ܐܢ( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  ܦܢܐ ROUND8(nByte);
}

/*
** Return the number of bytes required to create a duplicate of the
** expression passed as the first argument.
**
** The value returned includes space to create a copy of the Expr struct
** itself and the buffer referred to by Expr.u.zToken, if any.
**
** The return value includes space to duplicate all Expr nodes in the
** tree formed by Expr.pLeft and Expr.pRight, but not any other
** substructure such as Expr.x.pList, Expr.x.pSelect, and Expr.y.pWin.
*/
ساكن صحيح dupedExprSize(ثابت Expr *p){
  صحيح nByte;
  أكد( p!=0 );
  nByte = dupedExprNodeSize(p, EXPRDUP_REDUCE);
  ܐܢ( p->pLeft ) nByte += dupedExprSize(p->pLeft);
  ܐܢ( p->pRight ) nByte += dupedExprSize(p->pRight);
  أكد( nByte==ROUND8(nByte) );
  ܦܢܐ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
تعريفنوع بنية EdupBuf EdupBuf;
بنية EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#ifdef SQLITE_DEBUG
  u8 *zEnd;            /* First byte past the end of memory */
#endif
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ساكن Expr *exprDup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت Expr *p,        /* Expr tree to be duplicated */
  صحيح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  Expr *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  صحيح nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  أكد( db!=0 );
  أكد( p );
  أكد( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  أكد( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ܐܢ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }ܐܚܪܢܐ{
    صحيح nAlloc;
    ܐܢ( dupFlags ){
      nAlloc = dupedExprSize(p);
    }ܐܚܪܢܐ ܐܢ( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(EXPR_FULLSIZE + nToken);
    }ܐܚܪܢܐ{
      nToken = 0;
      nAlloc = ROUND8(EXPR_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (Expr *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  ܐܢ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت unsigned nStructSize = dupedExprStructSize(p, dupFlags);
    صحيح nNewSize = nStructSize & 0xfff;
    ܐܢ( nToken<0 ){
      ܐܢ( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ܐܚܪܢܐ{
        nToken = 0;
      }
    }
    ܐܢ( dupFlags ){
      أكد( (صحيح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      أكد( ExprHasProperty(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }ܐܚܪܢܐ{
      u32 nSize = (u32)exprStructSize(p);
      أكد( (صحيح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (صحيح)EXPR_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      ܐܢ( nSize<EXPR_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, EXPR_FULLSIZE-nSize);
      }
      nNewSize = EXPR_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ExprClearVVAProperties(pNew);
    ܐܢ( dupFlags ){
      ExprSetVVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    ܐܢ( nToken>0 ){
      حرف *zToken = pNew->u.zToken = (حرف*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    ܐܢ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ܐܢ( ExprUseXSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ܐܚܪܢܐ{
        pNew->x.pList = sqlite3ExprListDup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      ܐܢ( ExprHasProperty(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        أكد( ExprHasProperty(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ܐܢ( dupFlags ){
        ܐܢ( p->op==TK_SELECT_COLUMN ){
          pNew->pLeft = p->pLeft;
          أكد( p->pRight==0 
               || p->pRight==p->pLeft
               || ExprHasProperty(p->pLeft, EP_Subquery) );
        }ܐܚܪܢܐ{
          pNew->pLeft = p->pLeft ?
                      exprDup(db, p->pLeft, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->pRight = p->pRight ?
                       exprDup(db, p->pRight, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ܐܚܪܢܐ{
        ܐܢ( p->op==TK_SELECT_COLUMN ){
          pNew->pLeft = p->pLeft;
          أكد( p->pRight==0 
               || p->pRight==p->pLeft
               || ExprHasProperty(p->pLeft, EP_Subquery) );
        }ܐܚܪܢܐ{
          pNew->pLeft = sqlite3ExprDup(db, p->pLeft, 0);
        }
        pNew->pRight = sqlite3ExprDup(db, p->pRight, 0);
      }
    }
  }
  ܐܢ( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, حجم(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  ܦܢܐ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ܐܢ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ܐܢ( pRet ){
      صحيح i;
      pRet->nCte = p->nCte;
      لكل(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3ExprListDup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  ܦܢܐ pRet;
}
#ܐܚܪܢܐ
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ساكن صحيح gatherSelectWindowsCallback(Walker *pWalker, Expr *pExpr){
  ܐܢ( pExpr->op==TK_FUNCTION && ExprHasProperty(pExpr, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = pExpr->y.pWin;
    أكد( pWin );
    أكد( IsWindowFunc(pExpr) );
    أكد( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  ܦܢܐ WRC_Continue;
}
ساكن صحيح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  ܦܢܐ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ساكن فراغ gatherSelectWindows(Select *p){
  Walker w;
  w.xExprCallback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#endif


/*
** The following group of routines make deep copies of expressions,
** expression lists, ID lists, and select statements.  The copies can
** be deleted (by being passed to their respective ...Delete() routines)
** without effecting the originals.
**
** The expression list, ID, and source lists return by sqlite3ExprListDup(),
** sqlite3IdListDup(), and sqlite3SrcListDup() can not be further expanded
** by subsequent calls to sqlite*ListAppend() routines.
**
** Any tables that the SrcList might point to are not duplicated.
**
** The flags parameter contains a combination of the EXPRDUP_XXX flags.
** If the EXPRDUP_REDUCE flag is set, then the structure returned is a
** truncated version of the usual Expr structure that will be stored as
** part of the in-memory representation of the database schema.
*/
Expr *sqlite3ExprDup(sqlite3 *db, ثابت Expr *p, صحيح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  ܦܢܐ p ? exprDup(db, p, flags, 0) : 0;
}
ExprList *sqlite3ExprListDup(sqlite3 *db, ثابت ExprList *p, صحيح flags){
  ExprList *pNew;
  بنية ExprList_item *pItem;
  ثابت بنية ExprList_item *pOldItem;
  صحيح i;
  Expr *pPriorSelectColOld = 0;
  Expr *pPriorSelectColNew = 0;
  أكد( db!=0 );
  ܐܢ( p==0 ) ܦܢܐ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  ܐܢ( pNew==0 ) ܦܢܐ 0;
  pNew->nExpr = p->nExpr;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  لكل(i=0; i<p->nExpr; i++, pItem++, pOldItem++){
    Expr *pOldExpr = pOldItem->pExpr;
    Expr *pNewExpr;
    pItem->pExpr = sqlite3ExprDup(db, pOldExpr, flags);
    ܐܢ( pOldExpr
     && pOldExpr->op==TK_SELECT_COLUMN
     && (pNewExpr = pItem->pExpr)!=0
    ){
      ܐܢ( pNewExpr->pRight ){
        pPriorSelectColOld = pOldExpr->pRight;
        pPriorSelectColNew = pNewExpr->pRight;
        pNewExpr->pLeft = pNewExpr->pRight;
      }ܐܚܪܢܐ{
        ܐܢ( pOldExpr->pLeft!=pPriorSelectColOld ){
          pPriorSelectColOld = pOldExpr->pLeft;
          pPriorSelectColNew = sqlite3ExprDup(db, pPriorSelectColOld, flags);
          pNewExpr->pRight = pPriorSelectColNew;
        }
        pNewExpr->pLeft = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  ܦܢܐ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ܐܢ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ثابت SrcList *p, صحيح flags){
  SrcList *pNew;
  صحيح i;
  أكد( db!=0 );
  ܐܢ( p==0 ) ܦܢܐ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ܐܢ( pNew==0 ) ܦܢܐ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  لكل(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ثابت SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    ܐܢ( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, حجم(Subquery));
      ܐܢ( pNewSubq==0 ){
        أكد( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }ܐܚܪܢܐ{
        memcpy(pNewSubq, pOldItem->u4.pSubq, حجم(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ܐܢ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }ܐܚܪܢܐ ܐܢ( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }ܐܚܪܢܐ{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    ܐܢ( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }ܐܚܪܢܐ ܐܢ( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3ExprListDup(db, pOldItem->u1.pFuncArg, flags);
    }ܐܚܪܢܐ{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    ܐܢ( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    ܐܢ( pTab ){
      pTab->nTabRef++;
    }
    ܐܢ( pOldItem->fg.isUsing ){
      أكد( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }ܐܚܪܢܐ{
      pNewItem->u3.pOn = sqlite3ExprDup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  ܦܢܐ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ثابت IdList *p){
  IdList *pNew;
  صحيح i;
  أكد( db!=0 );
  ܐܢ( p==0 ) ܦܢܐ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ܐܢ( pNew==0 ) ܦܢܐ 0;
  pNew->nId = p->nId;
  لكل(i=0; i<p->nId; i++){
    بنية IdList_item *pNewItem = &pNew->a[i];
    ثابت بنية IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  ܦܢܐ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحيح flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  لكل(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, حجم(*p) );
    ܐܢ( pNew==0 ) اكسر;
    pNew->pEList = sqlite3ExprListDup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3ExprDup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3ExprListDup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3ExprDup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3ExprListDup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3ExprDup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    ܐܢ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    ܐܢ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      اكسر;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  ܦܢܐ pRet;
}
#ܐܚܪܢܐ
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحيح flags){
  أكد( p==0 );
  ܦܢܐ 0;
}
#endif


/*
** Add a new element to the end of an expression list.  If pList is
** initially NULL, then create a new expression list.
**
** The pList argument must be either NULL or a pointer to an ExprList
** obtained from a prior call to sqlite3ExprListAppend().
**
** If a memory allocation error occurs, the entire list is freed and
** NULL is returned.  If non-NULL is returned, then it is guaranteed
** that the new entry was successfully appended.
*/
ساكن ثابت بنية ExprList_item zeroItem = {0};
SQLITE_NOINLINE ExprList *sqlite3ExprListAppendNew(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  بنية ExprList_item *pItem;
  ExprList *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ܐܢ( pList==0 ){
    sqlite3ExprDelete(db, pExpr);
    ܦܢܐ 0;
  }
  pList->nAlloc = 4;
  pList->nExpr = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  ܦܢܐ pList;
}
SQLITE_NOINLINE ExprList *sqlite3ExprListAppendGrow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ExprList *pList,        /* List to which to append. Might be NULL */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  بنية ExprList_item *pItem;
  ExprList *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ܐܢ( pNew==0 ){
    sqlite3ExprListDelete(db, pList);
    sqlite3ExprDelete(db, pExpr);
    ܦܢܐ 0;
  }ܐܚܪܢܐ{
    pList = pNew;
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  ܦܢܐ pList;
}
ExprList *sqlite3ExprListAppend(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to append. Might be NULL */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  بنية ExprList_item *pItem;
  ܐܢ( pList==0 ){
    ܦܢܐ sqlite3ExprListAppendNew(pParse->db,pExpr);
  }
  ܐܢ( pList->nAlloc<pList->nExpr+1 ){
    ܦܢܐ sqlite3ExprListAppendGrow(pParse->db,pList,pExpr);
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  ܦܢܐ pList;
}

/*
** pColumns and pExpr form a vector assignment which is part of the SET
** clause of an UPDATE statement.  Like this:
**
**        (a,b,c) = (expr1,expr2,expr3)
** Or:    (a,b,c) = (SELECT x,y,z FROM ....)
**
** For each term of the vector assignment, append new entries to the
** expression list pList.  In the case of a subquery on the RHS, append
** TK_SELECT_COLUMN expressions.
*/
ExprList *sqlite3ExprListAppendVector(
  Parse *pParse,         /* Parsing context */
  ExprList *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  Expr *pExpr            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  صحيح n;
  صحيح i;
  صحيح iFirst = pList ? pList->nExpr : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ܐܢ( NEVER(pColumns==0) ) اذهب vector_append_error;
  ܐܢ( pExpr==0 ) اذهب vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ܐܢ( pExpr->op!=TK_SELECT && pColumns->nId!=(n=sqlite3ExprVectorSize(pExpr)) ){
    sqlite3ErrorMsg(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    اذهب vector_append_error;
  }

  لكل(i=0; i<pColumns->nId; i++){
    Expr *pSubExpr = sqlite3ExprForVectorField(pParse, pExpr, i, pColumns->nId);
    أكد( pSubExpr!=0 || db->mallocFailed );
    ܐܢ( pSubExpr==0 ) استمر;
    pList = sqlite3ExprListAppend(pParse, pList, pSubExpr);
    ܐܢ( pList ){
      أكد( pList->nExpr==iFirst+i+1 );
      pList->a[pList->nExpr-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ܐܢ( !db->mallocFailed && pExpr->op==TK_SELECT && ALWAYS(pList!=0) ){
    Expr *pFirst = pList->a[iFirst].pExpr;
    أكد( pFirst!=0 );
    أكد( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->pRight = pExpr;
    pExpr = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3ExprUnmapAndDelete(pParse, pExpr);
  sqlite3IdListDelete(db, pColumns);
  ܦܢܐ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
فراغ sqlite3ExprListSetSortOrder(ExprList *p, صحيح iSortOrder, صحيح eNulls){
  بنية ExprList_item *pItem;
  ܐܢ( p==0 ) ܦܢܐ;
  أكد( p->nExpr>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->nExpr-1];
  أكد( pItem->fg.bNulls==0 );
  ܐܢ( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  ܐܢ( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    ܐܢ( iSortOrder!=eNulls ){
      pItem->fg.sortFlags |= KEYINFO_ORDER_BIGNULL;
    }
  }
}

/*
** Set the ExprList.a[].zEName element of the most recently added item
** on the expression list.
**
** pList might be NULL following an OOM error.  But pName should never be
** NULL.  If a memory allocation fails, the pParse->db->mallocFailed flag
** is set.
*/
فراغ sqlite3ExprListSetName(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحيح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  ܐܢ( pList ){
    بنية ExprList_item *pItem;
    أكد( pList->nExpr>0 );
    pItem = &pList->a[pList->nExpr-1];
    أكد( pItem->zEName==0 );
    أكد( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ܐܢ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      ܐܢ( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ثابت فراغ*)pItem->zEName, pName);
      }
    }
  }
}

/*
** Set the ExprList.a[].zSpan element of the most recently added item
** on the expression list.
**
** pList might be NULL following an OOM error.  But pSpan should never be
** NULL.  If a memory allocation fails, the pParse->db->mallocFailed flag
** is set.
*/
فراغ sqlite3ExprListSetSpan(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  ثابت حرف *zStart,     /* Start of the span */
  ثابت حرف *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  ܐܢ( pList ){
    بنية ExprList_item *pItem = &pList->a[pList->nExpr-1];
    أكد( pList->nExpr>0 );
    ܐܢ( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
فراغ sqlite3ExprListCheckLength(
  Parse *pParse,
  ExprList *pEList,
  ثابت حرف *zObject
){
  صحيح mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->nExpr==mx );
  testcase( pEList && pEList->nExpr==mx+1 );
  ܐܢ( pEList && pEList->nExpr>mx ){
    sqlite3ErrorMsg(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ساكن SQLITE_NOINLINE فراغ exprListDeleteNN(sqlite3 *db, ExprList *pList){
  صحيح i = pList->nExpr;
  بنية ExprList_item *pItem =  pList->a;
  أكد( pList->nExpr>0 );
  أكد( db!=0 );
  ܥܒܕ{
    sqlite3ExprDelete(db, pItem->pExpr);
    ܐܢ( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }ܟܡܐ( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
فراغ sqlite3ExprListDelete(sqlite3 *db, ExprList *pList){
  ܐܢ( pList ) exprListDeleteNN(db, pList);
}
فراغ sqlite3ExprListDeleteGeneric(sqlite3 *db, فراغ *pList){
  ܐܢ( ALWAYS(pList) ) exprListDeleteNN(db, (ExprList*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3ExprListFlags(ثابت ExprList *pList){
  صحيح i;
  u32 m = 0;
  أكد( pList!=0 );
  لكل(i=0; i<pList->nExpr; i++){
     Expr *pExpr = pList->a[i].pExpr;
     أكد( pExpr!=0 );
     m |= pExpr->flags;
  }
  ܦܢܐ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
صحيح sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  ܦܢܐ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ثابت حرف *zIn){
  ܐܢ( sqlite3StrICmp(zIn, "true")==0  ) ܦܢܐ EP_IsTrue;
  ܐܢ( sqlite3StrICmp(zIn, "false")==0 ) ܦܢܐ EP_IsFalse;
  ܦܢܐ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحيح sqlite3ExprIdToTrueFalse(Expr *pExpr){
  u32 v;
  أكد( pExpr->op==TK_ID || pExpr->op==TK_STRING );
  ܐܢ( !ExprHasProperty(pExpr, EP_Quoted|EP_IntValue)
   && (v = sqlite3IsTrueOrFalse(pExpr->u.zToken))!=0
  ){
    pExpr->op = TK_TRUEFALSE;
    ExprSetProperty(pExpr, v);
    ܦܢܐ 1;
  }
  ܦܢܐ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحيح sqlite3ExprTruthValue(ثابت Expr *pExpr){
  pExpr = sqlite3ExprSkipCollateAndLikely((Expr*)pExpr);
  أكد( pExpr->op==TK_TRUEFALSE );
  أكد( !ExprHasProperty(pExpr, EP_IntValue) );
  أكد( sqlite3StrICmp(pExpr->u.zToken,"true")==0
       || sqlite3StrICmp(pExpr->u.zToken,"false")==0 );
  ܦܢܐ pExpr->u.zToken[4]==0;
}

/*
** If pExpr is an AND or OR expression, try to simplify it by eliminating
** terms that are always true or false.  Return the simplified expression.
** Or return the original expression if no simplification is possible.
**
** Examples:
**
**     (x<10) AND true                =>   (x<10)
**     (x<10) AND false               =>   false
**     (x<10) AND (y=22 OR false)     =>   (x<10) AND (y=22)
**     (x<10) AND (y=22 OR true)      =>   (x<10)
**     (y=22) OR true                 =>   true
*/
Expr *sqlite3ExprSimplifiedAndOr(Expr *pExpr){
  أكد( pExpr!=0 );
  ܐܢ( pExpr->op==TK_AND || pExpr->op==TK_OR ){
    Expr *pRight = sqlite3ExprSimplifiedAndOr(pExpr->pRight);
    Expr *pLeft = sqlite3ExprSimplifiedAndOr(pExpr->pLeft);
    ܐܢ( ExprAlwaysTrue(pLeft) || ExprAlwaysFalse(pRight) ){
      pExpr = pExpr->op==TK_AND ? pRight : pLeft;
    }ܐܚܪܢܐ ܐܢ( ExprAlwaysTrue(pRight) || ExprAlwaysFalse(pLeft) ){
      pExpr = pExpr->op==TK_AND ? pLeft : pRight;
    }
  }
  ܦܢܐ pExpr;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ساكن صحيح exprEvalRhsFirst(Expr *pExpr){
  ܐܢ( ExprHasProperty(pExpr->pLeft, EP_Subquery)
   && !ExprHasProperty(pExpr->pRight, EP_Subquery)
  ){
    ܦܢܐ 1;
  }ܐܚܪܢܐ{
    ܦܢܐ 0;
  }
}

/*
** Compute the two operands of a binary operator.
**
** If either operand contains a subquery, then the code strives to
** compute the operand containing the subquery second.  If the other
** operand evalutes to NULL, then a jump is made.  The address of the
** IsNull operand that does this jump is returned.  The caller can use
** this to optimize the computation so as to avoid doing the potentially
** expensive subquery.
**
** If no optimization opportunities exist, return 0.
*/
ساكن صحيح exprComputeOperands(
  Parse *pParse,     /* Parsing context */
  Expr *pExpr,       /* The comparison expression */
  صحيح *pR1,          /* OUT: Register holding the left operand */
  صحيح *pR2,          /* OUT: Register holding the right operand */
  صحيح *pFree1,       /* OUT: Temp register to free if not zero */
  صحيح *pFree2        /* OUT: Another temp register to free if not zero */
){
  صحيح addrIsNull;
  صحيح r1, r2;
  Vdbe *v = pParse->pVdbe;

  أكد( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ܐܢ( exprEvalRhsFirst(pExpr) && sqlite3ExprCanBeNull(pExpr->pRight) ){
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ܐܚܪܢܐ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pFree1);
  ܐܢ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ܐܢ( ExprHasProperty(pExpr->pRight, EP_Subquery)
     && sqlite3ExprCanBeNull(pExpr->pLeft)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  ܦܢܐ addrIsNull;
}

/*
** pExpr is a TK_FUNCTION node.  Try to determine whether or not the
** function is a constant function.  A function is constant if all of
** the following are true:
**
**    (1)  It is a scalar function (not an aggregate or window function)
**    (2)  It has either the SQLITE_FUNC_CONSTANT or SQLITE_FUNC_SLOCHNG
**         property.
**    (3)  All of its arguments are constants
**
** This routine sets pWalker->eCode to 0 if pExpr is not a constant.
** It makes no changes to pWalker->eCode if pExpr is constant.  In
** every case, it returns WRC_Abort.
**
** Called as a service subroutine from exprNodeIsConstant().
*/
ساكن SQLITE_NOINLINE صحيح exprNodeIsConstantFunction(
  Walker *pWalker,
  Expr *pExpr
){
  صحيح n;             /* Number of arguments */
  ExprList *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( pExpr->op==TK_FUNCTION );
  ܐܢ( ExprHasProperty(pExpr, EP_TokenOnly)
   || (pList = pExpr->x.pList)==0
  ){;
    n = 0;
  }ܐܚܪܢܐ{
    n = pList->nExpr;
    sqlite3WalkExprList(pWalker, pList);
    ܐܢ( pWalker->eCode==0 ) ܦܢܐ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ENC(db), 0);
  ܐܢ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ExprHasProperty(pExpr, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    ܦܢܐ WRC_Abort;
  }
  ܦܢܐ WRC_Prune;
}


/*
** These routines are Walker callbacks used to check expressions to
** see if they are "constant" for some definition of constant.  The
** Walker.eCode value determines the type of "constant" we are looking
** for.
**
** These callback routines are used to implement the following:
**
**     sqlite3ExprIsConstant()                  pWalker->eCode==1
**     sqlite3ExprIsConstantNotJoin()           pWalker->eCode==2
**     sqlite3ExprIsTableConstant()             pWalker->eCode==3
**     sqlite3ExprIsConstantOrFunction()        pWalker->eCode==4 or 5
**     sqlite3ExprListIsConstant()              pWalker->eCode==1 or 6
**
** In all cases, the callbacks set Walker.eCode=0 and abort if the expression
** is found to not be a constant.
**
** The sqlite3ExprIsConstantOrFunction() is used for evaluating DEFAULT
** expressions in a CREATE TABLE statement.  The Walker.eCode value is 5
** when parsing an existing schema out of the sqlite_schema table and 4
** when processing a new CREATE TABLE statement.  A bound parameter raises
** an error for new statements, but is silently converted
** to NULL for existing schemas.  This allows sqlite_schema tables that
** contain a bound parameter because they were generated by older versions
** of SQLite to be parsed by newer versions of SQLite without raising a
** malformed schema error.
*/
ساكن صحيح exprNodeIsConstant(Walker *pWalker, Expr *pExpr){
  أكد( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ܐܢ( pWalker->eCode==2 && ExprHasProperty(pExpr, EP_OuterON) ){
    pWalker->eCode = 0;
    ܦܢܐ WRC_Abort;
  }

  بدل( pExpr->op ){
    حالة TK_FUNCTION:
      ܐܢ( ExprHasProperty(pExpr, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        ܦܢܐ WRC_Abort;
      }ܐܚܪܢܐ ܐܢ( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ܐܢ( pWalker->eCode==5 ) ExprSetProperty(pExpr, EP_FromDDL);
        ܦܢܐ WRC_Continue;
      }ܐܚܪܢܐ ܐܢ( ExprHasProperty(pExpr,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        ܦܢܐ WRC_Continue;
      }ܐܚܪܢܐ ܐܢ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        ܦܢܐ exprNodeIsConstantFunction(pWalker, pExpr);
      }ܐܚܪܢܐ{
        pWalker->eCode = 0;  /* Function call is not constant */
        ܦܢܐ WRC_Abort;
      }
    حالة TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ܐܢ( sqlite3ExprIdToTrueFalse(pExpr) ){
        ܦܢܐ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالة TK_COLUMN:
    حالة TK_AGG_FUNCTION:
    حالة TK_AGG_COLUMN:
      testcase( pExpr->op==TK_ID );
      testcase( pExpr->op==TK_COLUMN );
      testcase( pExpr->op==TK_AGG_FUNCTION );
      testcase( pExpr->op==TK_AGG_COLUMN );
      ܐܢ( ExprHasProperty(pExpr, EP_FixedCol) && pWalker->eCode!=2 ){
        ܦܢܐ WRC_Continue;
      }
      ܐܢ( pWalker->eCode==3 && pExpr->iTable==pWalker->u.iCur ){
        ܦܢܐ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالة TK_IF_NULL_ROW:
    حالة TK_REGISTER:
    حالة TK_DOT:
    حالة TK_RAISE:
      testcase( pExpr->op==TK_REGISTER );
      testcase( pExpr->op==TK_IF_NULL_ROW );
      testcase( pExpr->op==TK_DOT );
      testcase( pExpr->op==TK_RAISE );
      pWalker->eCode = 0;
      ܦܢܐ WRC_Abort;
    حالة TK_IS:
    حالة TK_ISNOT:
      ܐܢ( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        ܦܢܐ WRC_Abort;
      }
      ܦܢܐ WRC_Continue;
    حالة TK_VARIABLE:
      ܐܢ( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        pExpr->op = TK_NULL;
      }ܐܚܪܢܐ ܐܢ( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        ܦܢܐ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    افتراضي:
      testcase( pExpr->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( pExpr->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      ܦܢܐ WRC_Continue;
  }
}
ساكن صحيح exprIsConst(Parse *pParse, Expr *p, صحيح initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.xExprCallback = exprNodeIsConstant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3WalkExpr(&w, p);
  ܦܢܐ w.eCode;
}

/*
** Walk an expression tree.  Return non-zero if the expression is constant
** or return zero if the expression involves variables or function calls.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
**
** The pParse parameter may be NULL.  But if it is NULL, there is no way
** to determine if function calls are constant or not, and hence all
** function calls will be considered to be non-constant.  If pParse is
** not NULL, then a function call might be constant, depending on the
** function and on its parameters.
*/
صحيح sqlite3ExprIsConstant(Parse *pParse, Expr *p){
  ܦܢܐ exprIsConst(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحيح sqlite3ExprListIsConstant(Parse *pParse, ExprList *pList, صحيح bNoIs){
  ثابت صحيح iInit = bNoIs ? 6 : 1;
  صحيح ii;
  لكل(ii=0; ii<pList->nExpr; ii++){
    ܐܢ( 0==exprIsConst(pParse, pList->a[ii].pExpr, iInit) ) ܦܢܐ 0;
  }
  ܦܢܐ 1;
}

/*
** Walk an expression tree.  Return non-zero if
**
**   (1) the expression is constant, and
**   (2) the expression does not originate in the ON or USING clause
**       of a LEFT JOIN, and
**   (3) the expression does not contain any EP_FixedCol TK_COLUMN
**       operands created by the constant propagation optimization.
**
** When this routine returns true, it indicates that the expression
** can be added to the pParse->pConstExpr list and evaluated once when
** the prepared statement starts up.  See sqlite3ExprCodeRunJustOnce().
*/
ساكن صحيح sqlite3ExprIsConstantNotJoin(Parse *pParse, Expr *p){
  ܦܢܐ exprIsConst(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ساكن صحيح exprSelectWalkTableConstant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->eCode==3 || pWalker->eCode==0 );
  ܐܢ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    ܦܢܐ WRC_Abort;
  }
  ܦܢܐ WRC_Prune;
}

/*
** Walk an expression tree.  Return non-zero if the expression is constant
** for any single row of the table with cursor iCur.  In other words, the
** expression must not refer to any non-deterministic function nor any
** table other than iCur.
**
** Consider uncorrelated subqueries to be constants if the bAllowSubq
** parameter is true.
*/
صحيح sqlite3ExprIsTableConstant(Expr *p, صحيح iCur, صحيح bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.xExprCallback = exprNodeIsConstant;
  ܐܢ( bAllowSubq ){
    w.xSelectCallback = exprSelectWalkTableConstant;
  }ܐܚܪܢܐ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3WalkExpr(&w, p);
  ܦܢܐ w.eCode;
}

/*
** Check pExpr to see if it is an constraint on the single data source
** pSrc = &pSrcList->a[iSrc].  In other words, check to see if pExpr
** constrains pSrc but does not depend on any other tables or data
** sources anywhere else in the query.  Return true (non-zero) if pExpr
** is a constraint on pSrc only.
**
** This is an optimization.  False negatives will perhaps cause slower
** queries, but false positives will yield incorrect answers.  So when in
** doubt, return 0.
**
** To be an single-source constraint, the following must be true:
**
**   (1)  pExpr cannot refer to any table other than pSrc->iCursor.
**
**   (2a) pExpr cannot use subqueries unless the bAllowSubq parameter is
**        true and the subquery is non-correlated
**
**   (2b) pExpr cannot use non-deterministic functions.
**
**   (3)  pSrc cannot be part of the left operand for a RIGHT JOIN.
**        (Is there some way to relax this constraint?)
**
**   (4)  If pSrc is the right operand of a LEFT JOIN, then...
**         (4a)  pExpr must come from an ON clause..
**         (4b)  and specifically the ON clause associated with the LEFT JOIN.
**
**   (5)  If pSrc is the right operand of a LEFT JOIN or the left
**        operand of a RIGHT JOIN, then pExpr must be from the WHERE
**        clause, not an ON clause.
**
**   (6) Either:
**
**       (6a) pExpr does not originate in an ON or USING clause, or
**
**       (6b) The ON or USING clause from which pExpr is derived is
**            not to the left of a RIGHT JOIN (or FULL JOIN).
**
**       Without this restriction, accepting pExpr as a single-table
**       constraint might move the the ON/USING filter expression
**       from the left side of a RIGHT JOIN over to the right side,
**       which leads to incorrect answers.  See also restriction (9)
**       on push-down.
*/
صحيح sqlite3ExprIsSingleTableConstraint(
  Expr *pExpr,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحيح iSrc,                    /* Which element of pSrcList to use */
  صحيح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت SrcItem *pSrc = &pSrcList->a[iSrc];
  ܐܢ( pSrc->fg.jointype & JT_LTORJ ){
    ܦܢܐ 0;  /* rule (3) */
  }
  ܐܢ( pSrc->fg.jointype & JT_LEFT ){
    ܐܢ( !ExprHasProperty(pExpr, EP_OuterON) ) ܦܢܐ 0;   /* rule (4a) */
    ܐܢ( pExpr->w.iJoin!=pSrc->iCursor ) ܦܢܐ 0;         /* rule (4b) */
  }ܐܚܪܢܐ{
    ܐܢ( ExprHasProperty(pExpr, EP_OuterON) ) ܦܢܐ 0;    /* rule (5) */
  }
  ܐܢ( ExprHasProperty(pExpr, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      ܐܢ( pExpr->w.iJoin==pSrcList->a[jj].iCursor ){
        ܐܢ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          ܦܢܐ 0;  /* restriction (6) */
        }
        اكسر;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  ܦܢܐ sqlite3ExprIsTableConstant(pExpr, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ساكن صحيح exprNodeIsConstantOrGroupBy(Walker *pWalker, Expr *pExpr){
  ExprList *pGroupBy = pWalker->u.pGroupBy;
  صحيح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  لكل(i=0; i<pGroupBy->nExpr; i++){
    Expr *p = pGroupBy->a[i].pExpr;
    ܐܢ( sqlite3ExprCompare(0, pExpr, p, -1)<2 ){
      CollSeq *pColl = sqlite3ExprNNCollSeq(pWalker->pParse, p);
      ܐܢ( sqlite3IsBinary(pColl) ){
        ܦܢܐ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ܐܢ( ExprUseXSelect(pExpr) ){
    pWalker->eCode = 0;
    ܦܢܐ WRC_Abort;
  }

  ܦܢܐ exprNodeIsConstant(pWalker, pExpr);
}

/*
** Walk the expression tree passed as the first argument. Return non-zero
** if the expression consists entirely of constants or copies of terms
** in pGroupBy that sort with the BINARY collation sequence.
**
** This routine is used to determine if a term of the HAVING clause can
** be promoted into the WHERE clause.  In order for such a promotion to work,
** the value of the HAVING clause term must be the same for all members of
** a "group".  The requirement that the GROUP BY term must be BINARY
** assumes that no other collating sequence will have a finer-grained
** grouping than binary.  In other words (A=B COLLATE binary) implies
** A=B in every other collating sequence.  The requirement that the
** GROUP BY be BINARY is stricter than necessary.  It would also work
** to promote HAVING clauses that use the same alternative collating
** sequence as the GROUP BY term, but that is much harder to check,
** alternative collating sequences are uncommon, and this is only an
** optimization, so we take the easy way out and simply require the
** GROUP BY to use the BINARY collating sequence.
*/
صحيح sqlite3ExprIsConstantOrGroupBy(Parse *pParse, Expr *p, ExprList *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.xExprCallback = exprNodeIsConstantOrGroupBy;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3WalkExpr(&w, p);
  ܦܢܐ w.eCode;
}

/*
** Walk an expression tree for the DEFAULT field of a column definition
** in a CREATE TABLE statement.  Return non-zero if the expression is
** acceptable for use as a DEFAULT.  That is to say, return non-zero if
** the expression is constant or a function call with constant arguments.
** Return and 0 if there are any variables.
**
** isInit is true when parsing from sqlite_schema.  isInit is false when
** processing a new CREATE TABLE statement.  When isInit is true, parameters
** (such as ? or $abc) in the expression are converted into NULL.  When
** isInit is false, parameters raise an error.  Parameters should not be
** allowed in a CREATE TABLE statement, but some legacy versions of SQLite
** allowed it, so we need to support it when reading sqlite_schema for
** backwards compatibility.
**
** If isInit is true, set EP_FromDDL on every TK_FUNCTION node.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
*/
صحيح sqlite3ExprIsConstantOrFunction(Expr *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  ܦܢܐ exprIsConst(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحيح sqlite3ExprContainsSubquery(Expr *p){
  Walker w;
  w.eCode = 1;
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3WalkExpr(&w, p);
  ܦܢܐ w.eCode==0;
}

/*
** If the expression p codes a constant integer between 0 and 0x7fffffff,
** then return 1 and put the value of the integer in *pValue.  If the
** expression is not an integer or if it is an integer that is out side
** the range of 0...0x7fffffff, then return 0 and leave *pValue unchanged.
**
** If the pParse pointer is provided, then allow the expression p to be
** a parameter (TK_VARIABLE) that is bound to an integer between 0 and
** 0x7fffffff.  Variables that hold anything other than integers, or that
** hold integers outside the range of 0..0x7fffffff are not seen.
** But if pParse is NULL, then p must be a pure integer literal between
** 0 and 0x7fffffff.
**
** If pParse is not NULL and expression p is a variable, then the variable
** is marked so as to cause the statement to be reprepared each time a new
** value is bound to it. Except, if parameter bRSI is true, then the statement
** will only be reprepared if the rebind changes the value to or from a
** "small integer" (either 0 or 1).
**
** If pParse is NULL but bRSI is non-zero, then to not accept the input
** as an integer if the TK_INTEGER opcode has its op2 field set to non-zero.
** A non-zero Expr.op2 indicates that the expression is a compile-time
** reduction of a constant expression.  For example, "123 NOTNULL" would
** be rendered by the parser as a TK_INTEGER with value 1, but with op2 set.
**
** Summary:
**
**   pParse    bRsi   What happens
**   --------  ----   ------------------------------------------------
**   NULL      0      Reject if TK_VARIABLE.  Accept if op2!=0
**
**   NULL      1      Reject if TK_VARIABLE or op2!=0
**
**   not-NULL  0      Accept TK_VARIABLE and/or op2!=0 but for TK_VARIABLE,
**                    cause the statement to reprepare on *any*
**                    binding change
**
**   not-NULL  1      As previous, except reprepare only happens if a binding
**                    change move the variable value to or from an integer 0
**                    or 1.
*/
صحيح sqlite3ExprIsInteger(ثابت Expr *p, صحيح *pValue, Parse *pParse, صحيح bRSI){
  صحيح iSign = 1;   /* Either +1 or -1. */
  ܟܡܐ( 1/*exit-by-break*/ ){
    ܐܢ( NEVER(p==0) ) ܦܢܐ 0;
    ܐܢ( ExprUseUValue(p) ){
      أكد( p->op==TK_INTEGER );
      *pValue = p->u.iValue*iSign;
      ܐܢ( bRSI && pParse==0 && p->op2 ){
        ܦܢܐ 0;
      }
      ܦܢܐ 1;
    }
    ܐܢ( p->op==TK_UPLUS ){
      p = p->pLeft;
      pParse = 0;
      استمر;
    }
    ܐܢ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->pLeft;
      pParse = 0;
      استمر;
    }
    ܐܢ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_value *pVal; /* The variable */
      صحيح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      ܐܢ( NEVER(pParse->pVdbe==0) ) اكسر;
      ܐܢ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) اكسر;
      pVal = sqlite3VdbeGetBoundValue(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      ܐܢ( pVal ){
        i64 vv;
        ܐܢ( sqlite3_value_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_value_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *pValue = (صحيح)vv;
          rc = 1;       
        }
        sqlite3ValueFree(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      ܦܢܐ rc;
    }
    اكسر;
  }
  ܦܢܐ 0;
}

/*
** Return FALSE if there is no chance that the expression can be NULL.
**
** If the expression might be NULL or if the expression is too complex
** to tell return TRUE. 
**
** This routine is used as an optimization, to skip OP_IsNull opcodes
** when we know that a value cannot be NULL.  Hence, a false positive
** (returning TRUE when in fact the expression can never be NULL) might
** be a small performance hit but is otherwise harmless.  On the other
** hand, a false negative (returning FALSE when the result could be NULL)
** will likely result in an incorrect answer.  So when in doubt, return
** TRUE.
*/
صحيح sqlite3ExprCanBeNull(ثابت Expr *p){
  u8 op;
  أكد( p!=0 );
  ܟܡܐ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->pLeft;
    أكد( p!=0 );
  }
  op = p->op;
  ܐܢ( op==TK_REGISTER ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER:
    حالة TK_STRING:
    حالة TK_FLOAT:
    حالة TK_BLOB:
      ܦܢܐ 0;
    حالة TK_COLUMN:
      أكد( ExprUseYTab(p) );
      ܦܢܐ ExprHasProperty(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    افتراضي:
      ܦܢܐ 1;
  }
}

/*
** Return TRUE if the given expression is a constant which would be
** unchanged by OP_Affinity with the affinity given in the second
** argument.
**
** This routine is used to determine if the OP_Affinity operation
** can be omitted.  When in doubt return FALSE.  A false negative
** is harmless.  A false positive, however, can result in the wrong
** answer.
*/
صحيح sqlite3ExprNeedsNoAffinityChange(ثابت Expr *p, حرف aff){
  u8 op;
  صحيح unaryMinus = 0;
  ܐܢ( aff==SQLITE_AFF_BLOB ) ܦܢܐ 1;
  ܟܡܐ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ܐܢ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->pLeft;
  }
  op = p->op;
  ܐܢ( op==TK_REGISTER ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER: {
      ܦܢܐ aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_FLOAT: {
      ܦܢܐ aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_STRING: {
      ܦܢܐ !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    حالة TK_BLOB: {
      ܦܢܐ !unaryMinus;
    }
    حالة TK_COLUMN: {
      أكد( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      ܦܢܐ aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    افتراضي: {
      ܦܢܐ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
صحيح sqlite3IsRowid(ثابت حرف *z){
  ܐܢ( sqlite3StrICmp(z, "_ROWID_")==0 ) ܦܢܐ 1;
  ܐܢ( sqlite3StrICmp(z, "ROWID")==0 ) ܦܢܐ 1;
  ܐܢ( sqlite3StrICmp(z, "OID")==0 ) ܦܢܐ 1;
  ܦܢܐ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ثابت حرف *sqlite3RowidAlias(Table *pTab){
  ثابت حرف *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  صحيح ii;
  أكد( VisibleRowid(pTab) );
  لكل(ii=0; ii<ArraySize(azOpt); ii++){
    ܐܢ( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) ܦܢܐ azOpt[ii];
  }
  ܦܢܐ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
ساكن Select *isCandidateForInOpt(ثابت Expr *pX){
  Select *p;
  SrcList *pSrc;
  ExprList *pEList;
  Table *pTab;
  صحيح i;
  ܐܢ( !ExprUseXSelect(pX) ) ܦܢܐ 0;                 /* Not a subquery */
  ܐܢ( ExprHasProperty(pX, EP_VarSelect)  ) ܦܢܐ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ܐܢ( p->pPrior ) ܦܢܐ 0;              /* Not a compound SELECT */
  ܐܢ( p->selFlags & SF_Aggregate ){
    ܦܢܐ 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  ܐܢ( p->pLimit ) ܦܢܐ 0;              /* Has no LIMIT clause */
  ܐܢ( p->pWhere ) ܦܢܐ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  أكد( pSrc!=0 );
  ܐܢ( pSrc->nSrc!=1 ) ܦܢܐ 0;          /* Single term in FROM clause */
  ܐܢ( pSrc->a[0].fg.isSubquery) ܦܢܐ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab)  );              /* FROM clause is not a view */
  ܐܢ( IsVirtual(pTab) ) ܦܢܐ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  أكد( pEList!=0 );
  /* All SELECT results must be columns. */
  لكل(i=0; i<pEList->nExpr; i++){
    Expr *pRes = pEList->a[i].pExpr;
    ܐܢ( pRes->op!=TK_COLUMN ) ܦܢܐ 0;
    أكد( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  ܦܢܐ p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ساكن فراغ sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  صحيح iCur,          /* Cursor for the index */
  صحيح regHasNull,    /* Register in which to store hasNull flag */
  صحيح eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  صحيح addr1;
  صحيح op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  ܐܢ( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ܐܚܪܢܐ{
    op = OP_Last;
  }
  addr1 = sqlite3VdbeAddOp1(v, op, iCur); VdbeCoverage(v);
  sqlite3VdbeAddOp3(v, OP_Column, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_Last?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3VdbeJumpHere(v, addr1);
}
#endif


#ifndef SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ساكن صحيح sqlite3InRhsIsConstant(Parse *pParse, Expr *pIn){
  Expr *pLHS;
  صحيح res;
  أكد( !ExprHasProperty(pIn, EP_xIsSelect) );
  pLHS = pIn->pLeft;
  pIn->pLeft = 0;
  res = sqlite3ExprIsConstant(pParse, pIn);
  pIn->pLeft = pLHS;
  ܦܢܐ res;
}
#endif

/*
** This function is used by the implementation of the IN (...) operator.
** The pX parameter is the expression on the RHS of the IN operator, which
** might be either a list of expressions or a subquery.
**
** The job of this routine is to find or create a b-tree object that can
** be used either to test for membership in the RHS set or to iterate through
** all members of the RHS set, skipping duplicates.
**
** A cursor is opened on the b-tree object that is the RHS of the IN operator
** and the *piTab parameter is set to the index of that cursor.
**
** The returned value of this function indicates the b-tree type, as follows:
**
**   IN_INDEX_ROWID      - The cursor was opened on a database table.
**   IN_INDEX_INDEX_ASC  - The cursor was opened on an ascending index.
**   IN_INDEX_INDEX_DESC - The cursor was opened on a descending index.
**   IN_INDEX_EPH        - The cursor was opened on a specially created and
**                         populated ephemeral table.
**   IN_INDEX_NOOP       - No cursor was allocated.  The IN operator must be
**                         implemented as a sequence of comparisons.
**
** An existing b-tree might be used if the RHS expression pX is a simple
** subquery such as:
**
**     SELECT <column1>, <column2>... FROM <table>
**
** If the RHS of the IN operator is a list or a more complex subquery, then
** an ephemeral table might need to be generated from the RHS and then
** pX->iTable made to point to the ephemeral table instead of an
** existing table.  In this case, the creation and initialization of the
** ephemeral table might be put inside of a subroutine, the EP_Subrtn flag
** will be set on pX and the pX->y.sub fields will be set to show where
** the subroutine is coded.
**
** The inFlags parameter must contain, at a minimum, one of the bits
** IN_INDEX_MEMBERSHIP or IN_INDEX_LOOP but not both.  If inFlags contains
** IN_INDEX_MEMBERSHIP, then the generated table will be used for a fast
** membership test.  When the IN_INDEX_LOOP bit is set, the IN index will
** be used to loop over all values of the RHS of the IN operator.
**
** When IN_INDEX_LOOP is used (and the b-tree will be used to iterate
** through the set members) then the b-tree must not contain duplicates.
** An ephemeral table will be created unless the selected columns are guaranteed
** to be unique - either because it is an INTEGER PRIMARY KEY or due to
** a UNIQUE constraint or index.
**
** When IN_INDEX_MEMBERSHIP is used (and the b-tree will be used
** for fast set membership tests) then an ephemeral table must
** be used unless <columns> is a single INTEGER PRIMARY KEY column or an
** index can be found with the specified <columns> as its left-most.
**
** If the IN_INDEX_NOOP_OK and IN_INDEX_MEMBERSHIP are both set and
** if the RHS of the IN operator is a list (not a subquery) then this
** routine might decide that creating an ephemeral b-tree for membership
** testing is too expensive and return IN_INDEX_NOOP.  In that case, the
** calling routine should implement the IN operator using a sequence
** of Eq or Ne comparison operations.
**
** When the b-tree is being used for membership tests, the calling function
** might need to know whether or not the RHS side of the IN operator
** contains a NULL.  If prRhsHasNull is not a NULL pointer and
** if there is any chance that the (...) might contain a NULL value at
** runtime, then a register is allocated and the register number written
** to *prRhsHasNull. If there is no chance that the (...) contains a
** NULL value, then *prRhsHasNull is left unchanged.
**
** If a register is allocated and its location stored in *prRhsHasNull, then
** the value in that register will be NULL if the b-tree contains one or more
** NULL values, and it will be some non-NULL value if the b-tree contains no
** NULL values.
**
** If the aiMap parameter is not NULL, it must point to an array containing
** one element for each column returned by the SELECT statement on the RHS
** of the IN(...) operator. The i'th entry of the array is populated with the
** offset of the index column that matches the i'th column returned by the
** SELECT. For example, if the expression and selected index are:
**
**   (?,?,?) IN (SELECT a, b, c FROM t1)
**   CREATE INDEX i1 ON t1(b, c, a);
**
** then aiMap[] is populated with {2, 0, 1}.
*/
#ifndef SQLITE_OMIT_SUBQUERY
صحيح sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  Expr *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  صحيح *prRhsHasNull,         /* Register holding NULL status.  See notes */
  صحيح *aiMap,                /* Mapping from Index fields to RHS fields */
  صحيح *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  صحيح eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  صحيح iTab;                             /* Cursor of the RHS table */
  صحيح mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  أكد( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ܐܢ( prRhsHasNull && ExprUseXSelect(pX) ){
    صحيح i;
    ExprList *pEList = pX->x.pSelect->pEList;
    لكل(i=0; i<pEList->nExpr; i++){
      ܐܢ( sqlite3ExprCanBeNull(pEList->a[i].pExpr) ) اكسر;
    }
    ܐܢ( i==pEList->nExpr ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ܐܢ( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    صحيح iDb;                               /* Database idx for pTab */
    ExprList *pEList = p->pEList;
    صحيح nExpr = pEList->nExpr;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].pExpr!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    ܐܢ( nExpr==1 && pEList->a[0].pExpr->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحيح iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }ܐܚܪܢܐ{
      Index *pIdx;                         /* Iterator variable */
      صحيح affinity_ok = 1;
      صحيح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      لكل(i=0; i<nExpr && affinity_ok; i++){
        Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
        صحيح iCol = pEList->a[i].pExpr->iColumn;
        حرف idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        حرف cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        بدل( cmpaff ){
          حالة SQLITE_AFF_BLOB:
            اكسر;
          حالة SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            أكد( idxaff==SQLITE_AFF_TEXT );
            اكسر;
          افتراضي:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ܐܢ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        لكل(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          ܐܢ( pIdx->nColumn<nExpr ) استمر;
          ܐܢ( pIdx->pPartIdxWhere!=0 ) استمر;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          ܐܢ( pIdx->nColumn>=BMS-1 ) استمر;
          ܐܢ( mustBeUnique ){
            ܐܢ( pIdx->nKeyCol>nExpr
             ||(pIdx->nColumn>nExpr && !IsUniqueIndex(pIdx))
            ){
              استمر;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          لكل(i=0; i<nExpr; i++){
            Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
            Expr *pRhs = pEList->a[i].pExpr;
            CollSeq *pReq = sqlite3BinaryCompareCollSeq(pParse, pLhs, pRhs);
            صحيح j;
 
            لكل(j=0; j<nExpr; j++){
              ܐܢ( pIdx->aiColumn[j]!=pRhs->iColumn ) استمر;
              أكد( pIdx->azColl[j] );
              ܐܢ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                استمر;
              }
              اكسر;
            }
            ܐܢ( j==nExpr ) اكسر;
            mCol = MASKBIT(j);
            ܐܢ( mCol & colUsed ) اكسر; /* Each column used only once */
            colUsed |= mCol;
            ܐܢ( aiMap ) aiMap[i] = j;
          }
 
          أكد( nExpr>0 && nExpr<BMS );
          أكد( i==nExpr || colUsed!=(MASKBIT(nExpr)-1) );
          ܐܢ( colUsed==(MASKBIT(nExpr)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحيح iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            ܐܢ( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<nExpr)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              ܐܢ( nExpr==1 ){
                sqlite3SetHasNullFlag(v, iTab, *prRhsHasNull,
                                      pIdx->aSortOrder[0]);
              }
            }
            sqlite3VdbeJumpHere(v, iAddr);
          }
        } /* End loop over indexes */
      } /* End if( affinity_ok ) */
    } /* End if not an rowid index */
  } /* End attempt to optimize using an index */

  /* If no preexisting index is available for the IN clause
  ** and IN_INDEX_NOOP is an allowed reply
  ** and the RHS of the IN operator is a list, not a subquery
  ** and the RHS is not constant or has two or fewer terms,
  ** then it is not worth creating an ephemeral table to evaluate
  ** the IN operator so return IN_INDEX_NOOP.
  */
  ܐܢ( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && ExprUseXList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->nExpr<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  ܐܢ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحيح rMayHaveNull = 0;
    صحيح bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    ܐܢ( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }ܐܚܪܢܐ ܐܢ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_IN );
    ܐܢ( !bloomOk
     && ExprUseXSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    ܐܢ( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ܐܢ( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    صحيح i, n;
    n = sqlite3ExprVectorSize(pX->pLeft);
    لكل(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  ܦܢܐ eType;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ساكن حرف *exprINAffinity(Parse *pParse, ثابت Expr *pExpr){
  Expr *pLeft = pExpr->pLeft;
  صحيح nVal = sqlite3ExprVectorSize(pLeft);
  Select *pSelect = ExprUseXSelect(pExpr) ? pExpr->x.pSelect : 0;
  حرف *zRet;

  أكد( pExpr->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ܐܢ( zRet ){
    صحيح i;
    لكل(i=0; i<nVal; i++){
      Expr *pA = sqlite3VectorFieldSubexpr(pLeft, i);
      حرف a = sqlite3ExprAffinity(pA);
      ܐܢ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].pExpr, a);
      }ܐܚܪܢܐ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  ܦܢܐ zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
فراغ sqlite3SubselectError(Parse *pParse, صحيح nActual, صحيح nExpect){
  ܐܢ( pParse->nErr==0 ){
    ثابت حرف *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3ErrorMsg(pParse, zFmt, nActual, nExpect);
  }
}
#endif

/*
** Expression pExpr is a vector that has been used in a context where
** it is not permitted. If pExpr is a sub-select vector, this routine
** loads the Parse object with a message of the form:
**
**   "sub-select returns N columns - expected 1"
**
** Or, if it is a regular scalar vector:
**
**   "row value misused"
*/  
فراغ sqlite3VectorErrorMsg(Parse *pParse, Expr *pExpr){
#ifndef SQLITE_OMIT_SUBQUERY
  ܐܢ( ExprUseXSelect(pExpr) ){
    sqlite3SubselectError(pParse, pExpr->x.pSelect->pEList->nExpr, 1);
  }ܐܚܪܢܐ
#endif
  {
    sqlite3ErrorMsg(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ساكن صحيح findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  Expr *pExpr,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  ܐܢ( pNewSig==0 ) ܦܢܐ 0;
  ܐܢ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) ܦܢܐ 0;
  أكد( pExpr->op==TK_IN );
  أكد( !ExprUseYSub(pExpr) );
  أكد( ExprUseXSelect(pExpr) );
  أكد( pExpr->x.pSelect!=0 );
  أكد( (pExpr->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  لكل(; pOp<pEnd; pOp++){
    ܐܢ( pOp->p4type!=P4_SUBRTNSIG ) استمر;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    ܐܢ( !pSig->bComplete ) استمر;
    ܐܢ( pNewSig->selId!=pSig->selId ) استمر;
    ܐܢ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) استمر;
    pExpr->y.sub.iAddr = pSig->iAddr;
    pExpr->y.sub.regReturn = pSig->regReturn;
    pExpr->iTable = pSig->iTable;
    ExprSetProperty(pExpr, EP_Subrtn);
    ܦܢܐ 1;
  }
  ܦܢܐ 0;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that will construct an ephemeral table containing all terms
** in the RHS of an IN operator.  The IN operator can be in either of two
** forms:
**
**     x IN (4,5,11)              -- IN operator with list on right-hand side
**     x IN (SELECT a FROM b)     -- IN operator with subquery on the right
**
** The pExpr parameter is the IN operator.  The cursor number for the
** constructed ephemeral table is returned.  The first time the ephemeral
** table is computed, the cursor number is also stored in pExpr->iTable,
** however the cursor number returned might not be the same, as it might
** have been duplicated using OP_OpenDup.
**
** If the LHS expression ("x" in the examples) is a column value, or
** the SELECT statement returns a column value, then the affinity of that
** column is used to build the index keys. If both 'x' and the
** SELECT... statement are columns, then numeric affinity is used
** if either column has NUMERIC or INTEGER affinity. If neither
** 'x' nor the SELECT... statement are columns, then numeric affinity
** is used.
*/
فراغ sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  Expr *pExpr,            /* The IN operator */
  صحيح iTab,               /* Use this cursor number */
  صحيح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحيح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحيح addr;                   /* Address of OP_OpenEphemeral instruction */
  Expr *pLeft;                /* the LHS of the IN operator */
  KeyInfo *pKeyInfo = 0;      /* Key information */
  صحيح nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  أكد( v!=0 );

  /* The evaluation of the IN must be repeated every time it
  ** is encountered if any of the following is true:
  **
  **    *  The right-hand side is a correlated subquery
  **    *  The right-hand side is an expression list containing variables
  **    *  We are inside a trigger
  **
  ** If all of the above are false, then we can compute the RHS just once
  ** and reuse it many names.
  */
  ܐܢ( !ExprHasProperty(pExpr, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !ExprUseXSelect(pExpr) || pExpr->x.pSelect!=0 );
    ܐܢ( ExprUseXSelect(pExpr) && (pExpr->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, حجم(pSig[0]));
      ܐܢ( pSig ){
        pSig->selId = pExpr->x.pSelect->selId;
        pSig->zAff = exprINAffinity(pParse, pExpr);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ܐܢ( ExprHasProperty(pExpr, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, pExpr, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      ܐܢ( ExprUseXSelect(pExpr) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              pExpr->x.pSelect->selId));
      }
      أكد( ExprUseYSub(pExpr) );
      sqlite3VdbeAddOp2(v, OP_Gosub, pExpr->y.sub.regReturn,
                        pExpr->y.sub.iAddr);
      أكد( iTab!=pExpr->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, pExpr->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      ܐܢ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      ܦܢܐ;
    }

    /* Begin coding the subroutine */
    أكد( !ExprUseYWin(pExpr) );
    ExprSetProperty(pExpr, EP_Subrtn);
    أكد( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
    pExpr->y.sub.regReturn = ++pParse->nMem;
    pExpr->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, pExpr->y.sub.regReturn) + 1;
    ܐܢ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = pExpr->y.sub.iAddr;
      pSig->regReturn = pExpr->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  pLeft = pExpr->pLeft;
  nVal = sqlite3ExprVectorSize(pLeft);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  pExpr->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pExpr->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  ܐܢ( ExprUseXSelect(pExpr) ){
    VdbeComment((v, "Result of SELECT %u", pExpr->x.pSelect->selId));
  }ܐܚܪܢܐ{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  pKeyInfo = sqlite3KeyInfoAlloc(pParse->db, nVal, 1);
  أكد( pKeyInfo!=0 || pParse->nErr );
  ܐܢ( pKeyInfo==0 ) ܦܢܐ;

  ܐܢ( ExprUseXSelect(pExpr) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = pExpr->x.pSelect;
    ExprList *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    ܐܢ( ALWAYS(pEList->nExpr==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحيح i;
      صحيح rc;
      صحيح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = exprINAffinity(pParse, pExpr);
      pSelect->iLimit = 0;
      أكد( pEList!=0 );
      أكد( pEList->nExpr>0 );
      أكد( sqlite3KeyInfoIsWriteable(pKeyInfo) );
      لكل(i=0; i<nVal; i++){
        Expr *p = sqlite3VectorFieldSubexpr(pLeft, i);
        CollSeq *pColl;
        pKeyInfo->aColl[i] = pColl = sqlite3BinaryCompareCollSeq(
            pParse, p, pEList->a[i].pExpr
        );
        ܐܢ( !sqlite3IsBinary(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ܐܢ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحيح regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (فراغ *)pKeyInfo, P4_KEYINFO);
        pKeyInfo = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      ܐܢ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ܐܢ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ܐܢ( rc ){
        sqlite3KeyInfoUnref(pKeyInfo);
        ܦܢܐ;
      }
    }
  }ܐܚܪܢܐ ܐܢ( ALWAYS(pExpr->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    حرف affinity;            /* Affinity of the LHS of the IN */
    صحيح i;
    ExprList *pList = pExpr->x.pList;
    بنية ExprList_item *pItem;
    صحيح r1, r2;
    affinity = sqlite3ExprAffinity(pLeft);
    ܐܢ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ܐܚܪܢܐ ܐܢ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( pKeyInfo!=0 );
    أكد( sqlite3KeyInfoIsWriteable(pKeyInfo) );
    pKeyInfo->aColl[0] = sqlite3ExprCollSeq(pParse, pExpr->pLeft);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    لكل(i=pList->nExpr, pItem=pList->a; i>0; i--, pItem++){
      Expr *pE2 = pItem->pExpr;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ܐܢ( addrOnce && !sqlite3ExprIsConstant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        ExprClearProperty(pExpr, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3ExprCode(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_IdxInsert, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  ܐܢ( pSig ) pSig->bComplete = 1;
  ܐܢ( pKeyInfo ){
    sqlite3VdbeChangeP4(v, addr, (فراغ *)pKeyInfo, P4_KEYINFO);
  }
  ܐܢ( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    أكد( ExprUseYSub(pExpr) );
    أكد( sqlite3VdbeGetOp(v,pExpr->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, pExpr->y.sub.regReturn,
                      pExpr->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3ClearTempRegCache(pParse);
  }
}
#endif /* SQLITE_OMIT_SUBQUERY */

/*
** Generate code for scalar subqueries used as a subquery expression
** or EXISTS operator:
**
**     (SELECT a FROM b)          -- subquery
**     EXISTS (SELECT a FROM b)   -- EXISTS subquery
**
** The pExpr parameter is the SELECT or EXISTS operator to be coded.
**
** Return the register that holds the result.  For a multi-column SELECT,
** the result is stored in a contiguous array of registers and the
** return value is the register of the left-most result column.
** Return 0 if an error occurs.
*/
#ifndef SQLITE_OMIT_SUBQUERY
صحيح sqlite3CodeSubselect(Parse *pParse, Expr *pExpr){
  صحيح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحيح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحيح nReg;                   /* Registers to allocate */
  Expr *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  ܐܢ( pParse->nErr ) ܦܢܐ 0;
  testcase( pExpr->op==TK_EXISTS );
  testcase( pExpr->op==TK_SELECT );
  أكد( pExpr->op==TK_EXISTS || pExpr->op==TK_SELECT );
  أكد( ExprUseXSelect(pExpr) );
  pSel = pExpr->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ܐܢ( ExprHasProperty(pExpr, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( ExprUseYSub(pExpr) );
    sqlite3VdbeAddOp2(v, OP_Gosub, pExpr->y.sub.regReturn,
                      pExpr->y.sub.iAddr);
    ܦܢܐ pExpr->iTable;
  }

  /* Begin coding the subroutine */
  أكد( !ExprUseYWin(pExpr) );
  أكد( !ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) );
  ExprSetProperty(pExpr, EP_Subrtn);
  pExpr->y.sub.regReturn = ++pParse->nMem;
  pExpr->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, pExpr->y.sub.regReturn) + 1;

  /* The evaluation of the EXISTS/SELECT must be repeated every time it
  ** is encountered if any of the following is true:
  **
  **    *  The right-hand side is a correlated subquery
  **    *  The right-hand side is an expression list containing variables
  **    *  We are inside a trigger
  **
  ** If all of the above are false, then we can run this code just once
  ** save the results, and reuse the same result on subsequent invocations.
  */
  ܐܢ( !ExprHasProperty(pExpr, EP_VarSelect) ){
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }
 
  /* For a SELECT, generate code to put the values for all columns of
  ** the first row into an array of registers and return the index of
  ** the first register.
  **
  ** If this is an EXISTS, write an integer 0 (not exists) or 1 (exists)
  ** into a register and return that register number.
  **
  ** In both cases, the query is augmented with "LIMIT 1".  Any
  ** preexisting limit is discarded in place of the new LIMIT 1.
  */
  ExplainQueryPlan2(addrExplain, (pParse, 1, "%sSCALAR SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSel->selId));
  sqlite3VdbeScanStatusCounters(v, addrExplain, addrExplain, -1);
  nReg = pExpr->op==TK_SELECT ? pSel->pEList->nExpr : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ܐܢ( pExpr->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ܐܢ( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->pRight ){
      /* If there is both a DISTINCT and an OFFSET clause, then allocate
      ** a separate dest.iSdst array for sqlite3Select() and other
      ** routines to populate. In this case results will be copied over
      ** into the dest.iSDParm array only after OFFSET processing. This
      ** ensures that in the case where OFFSET excludes all rows, the
      ** dest.iSDParm array is not left populated with the contents of the
      ** last row visited - it should be all NULLs if all rows were
      ** excluded by OFFSET.  */ 
      dest.iSdst = pParse->nMem+1;
      pParse->nMem += nReg;
    }ܐܚܪܢܐ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ܐܚܪܢܐ{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ܐܢ( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    Expr *pLeft = pSel->pLimit->pLeft;
    ܐܢ( ExprHasProperty(pLeft, EP_IntValue)==0
     || (pLeft->u.iValue!=1 && pLeft->u.iValue!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3ExprInt32(db, 0);
      ܐܢ( pLimit ){
        pLimit->affExpr = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3ExprDup(db, pLeft, 0), pLimit);
      }
      sqlite3ExprDeferredDelete(pParse, pLeft);
      pSel->pLimit->pLeft = pLimit;
    }
  }ܐܚܪܢܐ{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3ExprInt32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  ܐܢ( sqlite3Select(pParse, pSel, &dest) ){
    pExpr->op2 = pExpr->op;
    pExpr->op = TK_ERROR;
    ܦܢܐ 0;
  }
  pExpr->iTable = rReg = dest.iSDParm;
  ExprSetVVAProperty(pExpr, EP_NoReduce);
  ܐܢ( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  ܐܢ( dest.eDest==SRT_Mem 
   && sqlite3ExprCanReturnSubtype(pParse, pSel->pEList->a[0].pExpr) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  أكد( ExprUseYSub(pExpr) );
  أكد( sqlite3VdbeGetOp(v,pExpr->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, pExpr->y.sub.regReturn,
                    pExpr->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  ܦܢܐ rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحيح sqlite3ExprCheckIN(Parse *pParse, Expr *pIn){
  صحيح nVector = sqlite3ExprVectorSize(pIn->pLeft);
  ܐܢ( ExprUseXSelect(pIn) && !pParse->db->mallocFailed ){
    ܐܢ( nVector!=pIn->x.pSelect->pEList->nExpr ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->nExpr, nVector);
      ܦܢܐ 1;
    }
  }ܐܚܪܢܐ ܐܢ( nVector!=1 ){
    sqlite3VectorErrorMsg(pParse, pIn->pLeft);
    ܦܢܐ 1;
  }
  ܦܢܐ 0;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code for an IN expression.
**
**      x IN (SELECT ...)
**      x IN (value, value, ...)
**
** The left-hand side (LHS) is a scalar or vector expression.  The
** right-hand side (RHS) is an array of zero or more scalar values, or a
** subquery.  If the RHS is a subquery, the number of result columns must
** match the number of columns in the vector on the LHS.  If the RHS is
** a list of values, the LHS must be a scalar.
**
** The IN operator is true if the LHS value is contained within the RHS.
** The result is false if the LHS is definitely not in the RHS.  The
** result is NULL if the presence of the LHS in the RHS cannot be
** determined due to NULLs.
**
** This routine generates code that jumps to destIfFalse if the LHS is not
** contained within the RHS.  If due to NULLs we cannot determine if the LHS
** is contained in the RHS then jump to destIfNull.  If the LHS is contained
** within the RHS then fall through.
**
** See the separate in-operator.md documentation file in the canonical
** SQLite source tree for additional information.
*/
ساكن فراغ sqlite3ExprCodeIN(
  Parse *pParse,        /* Parsing and code generating context */
  Expr *pExpr,          /* The IN expression */
  صحيح destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  صحيح destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  صحيح rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  صحيح eType;            /* Type of the RHS */
  صحيح rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  صحيح *aiMap = 0;       /* Map from vector field to index column */
  حرف *zAff = 0;       /* Affinity string for comparisons */
  صحيح nVector;          /* Size of vectors for this IN operator */
  صحيح iDummy;           /* Dummy parameter to exprCodeVector() */
  Expr *pLeft;          /* The LHS of the IN operator */
  صحيح i;                /* loop counter */
  صحيح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحيح destStep6 = 0;    /* Start of code for Step 6 */
  صحيح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحيح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحيح addrTop;          /* Top of the step-6 loop */
  صحيح iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  أكد( !ExprHasVVAProperty(pExpr,EP_Immutable) );
  pLeft = pExpr->pLeft;
  ܐܢ( sqlite3ExprCheckIN(pParse, pExpr) ) ܦܢܐ;
  zAff = exprINAffinity(pParse, pExpr);
  nVector = sqlite3ExprVectorSize(pExpr->pLeft);
  aiMap = (صحيح*)sqlite3DbMallocZero(pParse->db, nVector*حجم(صحيح));
  ܐܢ( pParse->db->mallocFailed ) اذهب sqlite3ExprCodeIN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, pExpr,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  أكد( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  لكل(i=0; i<nVector; i++){
    صحيح j, cnt;
    لكل(cnt=j=0; j<nVector; j++) ܐܢ( aiMap[j]==i ) cnt++;
    أكد( cnt==1 );
  }
#endif

  /* Code the LHS, the <expr> from "<expr> IN (...)". If the LHS is a
  ** vector, then it is stored in an array of nVector registers starting
  ** at r1.
  **
  ** sqlite3FindInIndex() might have reordered the fields of the LHS vector
  ** so that the fields are in the same order as an existing index.   The
  ** aiMap[] array contains a mapping from the original LHS field order to
  ** the field order that matches the RHS index.
  **
  ** Avoid factoring the LHS of the IN(...) expression out of the loop,
  ** even if it is constant, as OP_Affinity may be used on the register
  ** by code generated below.  */
  أكد( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = exprCodeVector(pParse, pLeft, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ܐܢ( eType==IN_INDEX_NOOP ){
    ExprList *pList;
    CollSeq *pColl;
    صحيح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحيح r2, regToFree;
    صحيح regCkNull = 0;
    صحيح ii;
    أكد( nVector==1 );
    أكد( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    pColl = sqlite3ExprCollSeq(pParse, pExpr->pLeft);
    ܐܢ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    لكل(ii=0; ii<pList->nExpr; ii++){
      r2 = sqlite3ExprCodeTemp(pParse, pList->a[ii].pExpr, &regToFree);
      ܐܢ( regCkNull && sqlite3ExprCanBeNull(pList->a[ii].pExpr) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      ܐܢ( ii<pList->nExpr-1 || destIfNull!=destIfFalse ){
        صحيح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->nExpr-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->nExpr-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->nExpr-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->nExpr-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ܐܚܪܢܐ{
        صحيح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ܐܢ( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    اذهب sqlite3ExprCodeIN_finished;
  }

  ܐܢ( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ܐܢ( nVector==1 ){
      حرف aff = zAff[0];
      ܐܢ( aff>=SQLITE_AFF_TEXT && aff!=sqlite3ExprAffinity(pLeft) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحيح rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ܐܚܪܢܐ{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    لكل(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ܐܢ( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      صحيح rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      لكل(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ܐܢ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ܐܚܪܢܐ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  لكل(i=0; i<nVector; i++){
    Expr *p = sqlite3VectorFieldSubexpr(pExpr->pLeft, i);
    ܐܢ( pParse->nErr ) اذهب sqlite3ExprCodeIN_oom_error;
    ܐܢ( sqlite3ExprCanBeNull(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ܐܢ( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }ܐܚܪܢܐ{
    ܐܢ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ܐܢ( ExprHasProperty(pExpr, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, pExpr->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        ܐܢ( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      اذهب sqlite3ExprCodeIN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ܐܢ( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ܐܢ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ܐܢ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ܐܢ( eType==IN_INDEX_INDEX_DESC ){
    addrTop = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }ܐܚܪܢܐ{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addrTop = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ܐܢ( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ܐܚܪܢܐ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  لكل(i=0; i<nVector; i++){
    Expr *p;
    CollSeq *pColl;
    صحيح r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(pLeft, i);
    ܐܢ( ExprUseXSelect(pExpr) ){
      Expr *pRhs = pExpr->x.pSelect->pEList->a[i].pExpr;
      pColl = sqlite3BinaryCompareCollSeq(pParse, p, pRhs);
    }ܐܚܪܢܐ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3ExprCollSeq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (فراغ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  ܐܢ( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addrTop+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3ExprCodeIN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3ExprCodeIN_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ساكن فراغ codeReal(Vdbe *v, ثابت حرف *z, صحيح negateFlag, صحيح iMem){
  ܐܢ( ALWAYS(z!=0) ){
    مزدوج value;
    sqlite3AtoF(z, &value);
    أكد( !sqlite3IsNaN(value) ); /* The new AtoF never returns NaN */
    ܐܢ( negateFlag ) value = -value;
    sqlite3VdbeAddDouble(v, iMem, value);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ساكن فراغ codeInteger(Parse *pParse, Expr *pExpr, صحيح negFlag, صحيح iMem){
  Vdbe *v = pParse->pVdbe;
  ܐܢ( pExpr->flags & EP_IntValue ){
    صحيح i = pExpr->u.iValue;
    أكد( i>=0 );
    ܐܢ( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }ܐܚܪܢܐ{
    صحيح c;
    i64 value;
    ثابت حرف *z = pExpr->u.zToken;
    أكد( z!=0 );
    c = sqlite3DecOrHexToI64(z, &value);
    ܐܢ( (c==3 && !negFlag) || (c==2) || (negFlag && value==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3ErrorMsg(pParse, "oversized integer: %s%#T", negFlag?"-":"",pExpr);
#ܐܚܪܢܐ
#ifndef SQLITE_OMIT_HEX_INTEGER
      ܐܢ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3ErrorMsg(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",pExpr);
      }ܐܚܪܢܐ
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }ܐܚܪܢܐ{
      ܐܢ( negFlag ){ value = c==3 ? SMALLEST_INT64 : -value; }
      sqlite3VdbeAddInt64(v, iMem, value);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
فراغ sqlite3ExprCodeLoadIndexColumn(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  صحيح iTabCur,    /* Cursor pointing to a table row */
  صحيح iIdxCol,    /* The column of the index to be loaded */
  صحيح regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  ܐܢ( iTabCol==XN_EXPR ){
    أكد( pIdx->aColExpr );
    أكد( pIdx->aColExpr->nExpr>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3ExprCodeCopy(pParse, pIdx->aColExpr->a[iIdxCol].pExpr, regOut);
    pParse->iSelfTab = 0;
  }ܐܚܪܢܐ{
    sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
فراغ sqlite3ExprCodeGeneratedColumn(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  صحيح regOut         /* Put the result in this register */
){
  صحيح iAddr;
  Vdbe *v = pParse->pVdbe;
  صحيح nErr = pParse->nErr;
  أكد( v!=0 );
  أكد( pParse->iSelfTab!=0 );
  ܐܢ( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }ܐܚܪܢܐ{
    iAddr = 0;
  }
  sqlite3ExprCodeCopy(pParse, sqlite3ColumnExpr(pTab,pCol), regOut);
  ܐܢ( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحيح p3 = 2+(صحيح)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (حرف*)pTab, P4_TABLE);
  }ܐܚܪܢܐ ܐܢ( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  ܐܢ( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  ܐܢ( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
فراغ sqlite3ExprCodeGetColumnOfTable(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  صحيح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحيح iCol,       /* Index of the column to extract */
  صحيح regOut      /* Extract the value into this register */
){
  Column *pCol;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( iCol!=XN_EXPR );
  ܐܢ( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ܐܚܪܢܐ{
    صحيح x;
    ܐܢ( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      ܦܢܐ;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }ܐܚܪܢܐ ܐܢ( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ܐܢ( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }ܐܚܪܢܐ{
        صحيح savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      ܦܢܐ;
#endif
    }ܐܚܪܢܐ ܐܢ( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), iCol);
    }ܐܚܪܢܐ{
      x = sqlite3TableColumnToStorage(pTab,iCol);
      testcase( x!=iCol );
    }
    sqlite3VdbeAddOp3(v, OP_Column, iTabCur, x, regOut);
    sqlite3ColumnDefault(v, pTab, iCol, regOut);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
صحيح sqlite3ExprCodeGetColumn(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  صحيح iColumn,     /* Index of the table column */
  صحيح iTable,      /* The cursor pointing to the table */
  صحيح iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  أكد( pParse->pVdbe!=0 );
  أكد( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  أكد( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  ܐܢ( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    ܐܢ( pOp->opcode==OP_Column ) pOp->p5 = p5;
    ܐܢ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  ܦܢܐ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
فراغ sqlite3ExprCodeMove(Parse *pParse, صحيح iFrom, صحيح iTo, صحيح nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
فراغ sqlite3ExprToRegister(Expr *pExpr, صحيح iReg){
  Expr *p = sqlite3ExprSkipCollateAndLikely(pExpr);
  ܐܢ( NEVER(p==0) ) ܦܢܐ;
  ܐܢ( p->op==TK_REGISTER ){
    أكد( p->iTable==iReg );
  }ܐܚܪܢܐ{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    ExprClearProperty(p, EP_Skip);
  }
}

/*
** Evaluate an expression (either a vector or a scalar expression) and store
** the result in contiguous temporary registers.  Return the index of
** the first register used to store the result.
**
** If the returned result register is a temporary scalar, then also write
** that register number into *piFreeable.  If the returned result register
** is not a temporary or if the expression is a vector set *piFreeable
** to 0.
*/
ساكن صحيح exprCodeVector(Parse *pParse, Expr *p, صحيح *piFreeable){
  صحيح iResult;
  صحيح nResult = sqlite3ExprVectorSize(p);
  ܐܢ( nResult==1 ){
    iResult = sqlite3ExprCodeTemp(pParse, p, piFreeable);
  }ܐܚܪܢܐ{
    *piFreeable = 0;
    ܐܢ( p->op==TK_SELECT ){
#ܐܢ SQLITE_OMIT_SUBQUERY
      iResult = 0;
#ܐܚܪܢܐ
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }ܐܚܪܢܐ{
      صحيح i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      أكد( ExprUseXList(p) );
      لكل(i=0; i<nResult; i++){
        sqlite3ExprCodeFactorable(pParse, p->x.pList->a[i].pExpr, i+iResult);
      }
    }
  }
  ܦܢܐ iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ساكن فراغ setDoNotMergeFlagOnCopy(Vdbe *v){
  ܐܢ( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ساكن صحيح exprCodeInlineFunction(
  Parse *pParse,        /* Parsing context */
  ExprList *pFarg,      /* List of function arguments */
  صحيح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحيح target            /* Store function result in this register */
){
  صحيح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->nExpr;
  أكد( nFarg>0 );  /* All in-line functions have at least one argument */
  بدل( iFuncId ){
    حالة INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      صحيح endCoalesce = sqlite3VdbeMakeLabel(pParse);
      صحيح i;
      أكد( nFarg>=2 );
      sqlite3ExprCode(pParse, pFarg->a[0].pExpr, target);
      لكل(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3ExprCode(pParse, pFarg->a[i].pExpr, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      اكسر;
    }
    حالة INLINEFUNC_iif: {
      Expr caseExpr;
      memset(&caseExpr, 0, حجم(caseExpr));
      caseExpr.op = TK_CASE;
      caseExpr.x.pList = pFarg;
      ܦܢܐ sqlite3ExprCodeTarget(pParse, &caseExpr, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    حالة INLINEFUNC_sqlite_offset: {
      Expr *pArg = pFarg->a[0].pExpr;
      ܐܢ( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }ܐܚܪܢܐ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      اكسر;
    }
#endif
    افتراضي: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      أكد( nFarg==1 || nFarg==2 );
      target = sqlite3ExprCodeTarget(pParse, pFarg->a[0].pExpr, target);
      اكسر;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ܐܢ !defined(SQLITE_UNTESTABLE)
    حالة INLINEFUNC_expr_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3ExprCompare(0,pFarg->a[0].pExpr, pFarg->a[1].pExpr,-1),
         target);
      اكسر;
    }

    حالة INLINEFUNC_expr_implies_expr: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3ExprImpliesExpr(pParse,pFarg->a[0].pExpr, pFarg->a[1].pExpr,-1),
         target);
      اكسر;
    }

    حالة INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      Expr *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].pExpr;
      ܐܢ( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3ExprImpliesNonNullRow(pFarg->a[0].pExpr,pA1->iTable,1),
           target);
      }ܐܚܪܢܐ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      اكسر;
    }

    حالة INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ثابت حرف *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      حرف aff;
      أكد( nFarg==1 );
      aff = sqlite3ExprAffinity(pFarg->a[0].pExpr);
      أكد( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      اكسر;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  ܦܢܐ target;
}

/*
** Expression Node callback for sqlite3ExprCanReturnSubtype().  If
** pExpr is able to return a subtype, set pWalker->eCode and abort
** the search.  If pExpr can never return a subtype, prune search.
**
** The only expressions that can return a subtype are:
**
**    1.  A function, including an aggregate function
**    2.  The no-op "+" operator
**    3.  A CASE...END expression
**    4.  A CAST() expression
**    5.  A "expr COLLATE colseq" expression.
**
** For any other kind of expression, prune the search.
**
** For case 1, the expression can yield a subtype if the function has
** the SQLITE_RESULT_SUBTYPE property.  Functions can also return
** a subtype (via sqlite3_result_value()) if any of the arguments can
** return a subtype.
**
** In all cases 1 through 5, the expression might also return a subtype
** if any operand can return a subtype.
*/
ساكن صحيح exprNodeCanReturnSubtype(Walker *pWalker, Expr *pExpr){
  صحيح n;
  FuncDef *pDef;
  sqlite3 *db;
  ܐܢ( pExpr->op==TK_CASE || pExpr->op==TK_UPLUS 
   || pExpr->op==TK_COLLATE || pExpr->op==TK_CAST 
  ){
    ܦܢܐ WRC_Continue;
  }
  ܐܢ( pExpr->op!=TK_FUNCTION && pExpr->op!=TK_AGG_FUNCTION ){
    ܦܢܐ WRC_Prune;
  }
  أكد( ExprUseXList(pExpr) );
  db = pWalker->pParse->db;
  n = pExpr->x.pList ? pExpr->x.pList->nExpr : 0;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ܐܢ( pDef==0 ) ܦܢܐ WRC_Continue;
#ܐܚܪܢܐ
  أكد( pDef!=0 );
#endif
  ܐܢ( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    ܦܢܐ WRC_Abort;
  }
  ܦܢܐ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
صحيح sqlite3ExprCanReturnSubtype(Parse *pParse, Expr *pExpr){
  Walker w;
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.xExprCallback = exprNodeCanReturnSubtype;
  sqlite3WalkExpr(&w, pExpr);
  ܦܢܐ w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ساكن SQLITE_NOINLINE صحيح sqlite3IndexedExprLookup(
  Parse *pParse,   /* The parsing context */
  Expr *pExpr,     /* The expression to potentially bypass */
  صحيح target       /* Where to store the result of the expression */
){
  IndexedExpr *p;
  Vdbe *v;
  لكل(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 exprAff;
    صحيح iDataCur = p->iDataCur;
    ܐܢ( iDataCur<0 ) استمر;
    ܐܢ( pParse->iSelfTab ){
      ܐܢ( p->iDataCur!=pParse->iSelfTab-1 ) استمر;
      iDataCur = -1;
    }
    ܐܢ( sqlite3ExprCompare(0, pExpr, p->pExpr, iDataCur)!=0 ) استمر;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    exprAff = sqlite3ExprAffinity(pExpr);
    ܐܢ( (exprAff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (exprAff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (exprAff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      استمر;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ܐܢ( ExprHasProperty(pExpr, EP_SubtArg)
     && sqlite3ExprCanReturnSubtype(pParse, pExpr) 
    ){
      استمر;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    ܐܢ( p->bMaybeNullRow || sqlite3ExprIsConstant(pParse, pExpr) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      صحيح addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3ExprCode(pParse, pExpr, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }ܐܚܪܢܐ{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    ܦܢܐ target;
  }
  ܦܢܐ -1;  /* Not found */
}


/*
** Expression pExpr is guaranteed to be a TK_COLUMN or equivalent. This
** function checks the Parse.pIdxPartExpr list to see if this column
** can be replaced with a constant value. If so, it generates code to
** put the constant value in a register (ideally, but not necessarily, 
** register iTarget) and returns the register number.
**
** Or, if the TK_COLUMN cannot be replaced by a constant, zero is 
** returned.
*/
ساكن صحيح exprPartidxExprLookup(Parse *pParse, Expr *pExpr, صحيح iTarget){
  IndexedExpr *p;
  لكل(p=pParse->pIdxPartExpr; p; p=p->pIENext){
    ܐܢ( pExpr->iColumn==p->iIdxCol && pExpr->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      صحيح addr = 0;
      صحيح ret;

      ܐܢ( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3ExprCodeTarget(pParse, p->pExpr, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت حرف*)&p->aff, 1);
      ܐܢ( sqlite3ExprCanReturnSubtype(pParse, p->pExpr) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ܐܢ( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      ܦܢܐ ret;
    }
  }
  ܦܢܐ 0;
}

/*
** Generate code that evaluates an AND or OR operator leaving a
** boolean result in a register.  pExpr is the AND/OR expression.
** Store the result in the "target" register.  Use short-circuit
** evaluation to avoid computing both operands, if possible.
**
** The code generated might require the use of a temporary register.
** If it does, then write the number of that temporary register
** into *pTmpReg.  If not, leave *pTmpReg unchanged.
*/
ساكن SQLITE_NOINLINE صحيح exprCodeTargetAndOr(
  Parse *pParse,     /* Parsing context */
  Expr *pExpr,       /* AND or OR expression to be coded */
  صحيح target,        /* Put result in this register, guaranteed */
  صحيح *pTmpReg       /* Write a temporary register here */
){
  صحيح op;            /* The opcode.  TK_AND or TK_OR */
  صحيح skipOp;        /* Opcode for the branch that skips one operand */
  صحيح addrSkip;      /* Branch instruction that skips one of the operands */
  صحيح regSS = 0;     /* Register holding computed operand when other omitted */
  صحيح r1, r2;        /* Registers for left and right operands, respectively */
  Expr *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  أكد( pExpr!=0 );
  op = pExpr->op;
  أكد( op==TK_AND || op==TK_OR );
  أكد( TK_AND==OP_And );            testcase( op==TK_AND );
  أكد( TK_OR==OP_Or );              testcase( op==TK_OR );
  أكد( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
  ܐܢ( pAlt!=pExpr ){
    r1 = sqlite3ExprCodeTarget(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    ܦܢܐ target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ܐܢ( exprEvalRhsFirst(pExpr) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3ExprCodeTarget(pParse, pExpr->pRight, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pTmpReg);
  }ܐܚܪܢܐ{
    /* Compute the left operand first */
    r1 = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
    ܐܢ( ExprHasProperty(pExpr->pRight, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ܐܚܪܢܐ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  ܐܢ( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  ܦܢܐ target;
}



/*
** Generate code into the current Vdbe to evaluate the given
** expression.  Attempt to store the results in register "target".
** Return the register where results are stored.
**
** With this routine, there is no guarantee that results will
** be stored in target.  The result might be stored in some other
** register if it is convenient to do so.  The calling function
** must check the return code and move the results to the desired
** register.
*/
صحيح sqlite3ExprCodeTarget(Parse *pParse, Expr *pExpr, صحيح target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحيح op;                   /* The opcode being coded */
  صحيح inReg = target;       /* Results stored in register inReg */
  صحيح regFree1 = 0;         /* If non-zero free this temporary register */
  صحيح regFree2 = 0;         /* If non-zero free this temporary register */
  صحيح r1, r2;               /* Various register numbers */
  Expr tempX;               /* Temporary expression node */
  صحيح p5 = 0;

  أكد( target>0 && target<=pParse->nMem );
  أكد( v!=0 );

expr_code_doover:
  ܐܢ( pExpr==0 ){
    op = TK_NULL;
  }ܐܚܪܢܐ ܐܢ( pParse->pIdxEpr!=0
   && !ExprHasProperty(pExpr, EP_Leaf)
   && (r1 = sqlite3IndexedExprLookup(pParse, pExpr, target))>=0
  ){
    ܦܢܐ r1;
  }ܐܚܪܢܐ{
    أكد( !ExprHasVVAProperty(pExpr,EP_Immutable) );
    op = pExpr->op;
  }
  أكد( op!=TK_ORDER );
  بدل( op ){
    حالة TK_AGG_COLUMN: {
      AggInfo *pAggInfo = pExpr->pAggInfo;
      بنية AggInfo_col *pCol;
      أكد( pAggInfo!=0 );
      أكد( pExpr->iAgg>=0 );
      ܐܢ( pExpr->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        اكسر;
      }
      pCol = &pAggInfo->aCol[pExpr->iAgg];
      ܐܢ( !pAggInfo->directMode ){
        ܦܢܐ AggInfoColumnReg(pAggInfo, pExpr->iAgg);
      }ܐܚܪܢܐ ܐܢ( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        ܐܢ( pTab==0 ){
          /* No comment added */
        }ܐܚܪܢܐ ܐܢ( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ܐܚܪܢܐ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          ܐܢ( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        ܦܢܐ target;
      }ܐܚܪܢܐ ܐܢ( pExpr->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, pExpr->iTable, pExpr->iColumn, target);
        ܦܢܐ target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالة TK_COLUMN: {
      صحيح iTab = pExpr->iTable;
      صحيح iReg;
      ܐܢ( ExprHasProperty(pExpr, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحيح aff;
        iReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft,target);
        أكد( ExprUseYTab(pExpr) );
        أكد( pExpr->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
        ܐܢ( aff>SQLITE_AFF_BLOB ){
          ساكن ثابت حرف zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        ܦܢܐ iReg;
      }
      ܐܢ( iTab<0 ){
        ܐܢ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          صحيح iSrc;
          صحيح iCol = pExpr->iColumn;
          أكد( ExprUseYTab(pExpr) );
          pTab = pExpr->y.pTab;
          أكد( pTab!=0 );
          أكد( iCol>=XN_ROWID );
          أكد( iCol<pTab->nCol );
          ܐܢ( iCol<0 ){
            ܦܢܐ -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          ܐܢ( pCol->colFlags & COLFLAG_GENERATED ){
            ܐܢ( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              ܦܢܐ 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            ܐܢ( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            ܦܢܐ iSrc;
          }ܐܚܪܢܐ
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          ܐܢ( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            ܦܢܐ target;
          }ܐܚܪܢܐ{
            ܦܢܐ iSrc;
          }
        }ܐܚܪܢܐ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ܐܚܪܢܐ ܐܢ( pParse->pIdxPartExpr 
       && 0!=(r1 = exprPartidxExprLookup(pParse, pExpr, target))
      ){
        ܦܢܐ r1;
      }
      أكد( ExprUseYTab(pExpr) );
      أكد( pExpr->y.pTab!=0 );
      iReg = sqlite3ExprCodeGetColumn(pParse, pExpr->y.pTab,
                               pExpr->iColumn, iTab, target,
                               pExpr->op2);
      ܦܢܐ iReg;
    }
    حالة TK_INTEGER: {
      codeInteger(pParse, pExpr, 0, target);
      ܦܢܐ target;
    }
    حالة TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3ExprTruthValue(pExpr), target);
      ܦܢܐ target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالة TK_FLOAT: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      codeReal(v, pExpr->u.zToken, 0, target);
      ܦܢܐ target;
    }
#endif
    حالة TK_STRING: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3VdbeLoadString(v, target, pExpr->u.zToken);
      ܦܢܐ target;
    }
    حالة TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + pExpr->y.nReg - 1);
      ܦܢܐ target;
    }
    افتراضي: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      ܦܢܐ target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      صحيح n;
      ثابت حرف *z;
      حرف *zBlob;
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      أكد( pExpr->u.zToken[0]=='x' || pExpr->u.zToken[0]=='X' );
      أكد( pExpr->u.zToken[1]=='\'' );
      z = &pExpr->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      ܦܢܐ target;
    }
#endif
    حالة TK_VARIABLE: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      أكد( pExpr->u.zToken!=0 );
      أكد( pExpr->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, pExpr->iColumn, target);
      ܦܢܐ target;
    }
    حالة TK_REGISTER: {
      ܦܢܐ pExpr->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3ExprCode(pParse, pExpr->pLeft, target);
      أكد( inReg==target );
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(pExpr->u.zToken, 0));
      ܦܢܐ inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    حالة TK_IS:
    حالة TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      Expr *pLeft = pExpr->pLeft;
      صحيح addrIsNull = 0;
      ܐܢ( sqlite3ExprIsVector(pLeft) ){
        codeVectorCompare(pParse, pExpr, target, op, p5);
      }ܐܚܪܢܐ{
        ܐܢ( ExprHasProperty(pExpr, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = exprComputeOperands(pParse, pExpr,
                                     &r1, &r2, &regFree1, &regFree2);
        }ܐܚܪܢܐ{
          r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
          r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, pLeft, pExpr->pRight, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ExprHasProperty(pExpr,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ܐܢ( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }ܐܚܪܢܐ{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          ܐܢ( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_AND:
    حالة TK_OR: {
      inReg = exprCodeTargetAndOr(pParse, pExpr, target, &regFree1);
      اكسر;
    }
    حالة TK_PLUS:
    حالة TK_STAR:
    حالة TK_MINUS:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_BITOR:
    حالة TK_SLASH:
    حالة TK_LSHIFT:
    حالة TK_RSHIFT:
    حالة TK_CONCAT: {
      صحيح addrIsNull;
      أكد( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      أكد( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      أكد( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      أكد( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      أكد( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      أكد( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      أكد( TK_LSHIFT==OP_ShiftLeft );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_ShiftRight );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ܐܢ( ExprHasProperty(pExpr, EP_Subquery) ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ܐܢ( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      اكسر;
    }
    حالة TK_UMINUS: {
      Expr *pLeft = pExpr->pLeft;
      أكد( pLeft );
      ܐܢ( pLeft->op==TK_INTEGER ){
        codeInteger(pParse, pLeft, 1, target);
        ܦܢܐ target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }ܐܚܪܢܐ ܐܢ( pLeft->op==TK_FLOAT ){
        أكد( !ExprHasProperty(pExpr, EP_IntValue) );
        codeReal(v, pLeft->u.zToken, 1, target);
        ܦܢܐ target;
#endif
      }ܐܚܪܢܐ{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_IntValue|EP_TokenOnly;
        tempX.u.iValue = 0;
        ExprClearVVAProperties(&tempX);
        r1 = sqlite3ExprCodeTemp(pParse, &tempX, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أكد( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحيح bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      bNormal = pExpr->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      صحيح addr;
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      اكسر;
    }
    حالة TK_AGG_FUNCTION: {
      AggInfo *pInfo = pExpr->pAggInfo;
      ܐܢ( pInfo==0
       || NEVER(pExpr->iAgg<0)
       || NEVER(pExpr->iAgg>=pInfo->nFunc)
      ){
        أكد( !ExprHasProperty(pExpr, EP_IntValue) );
        sqlite3ErrorMsg(pParse, "misuse of aggregate: %#T()", pExpr);
      }ܐܚܪܢܐ{
        ܦܢܐ AggInfoFuncReg(pInfo, pExpr->iAgg);
      }
      اكسر;
    }
    حالة TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      صحيح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت حرف *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      صحيح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      ܐܢ( ExprHasProperty(pExpr, EP_WinFunc) ){
        ܦܢܐ pExpr->y.pWin->regResult;
      }
#endif

      ܐܢ( ConstFactorOk(pParse)
       && sqlite3ExprIsConstantNotJoin(pParse,pExpr)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        ܦܢܐ sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
      }
      أكد( !ExprHasProperty(pExpr, EP_TokenOnly) );
      أكد( ExprUseXList(pExpr) );
      pFarg = pExpr->x.pList;
      nFarg = pFarg ? pFarg->nExpr : 0;
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      zId = pExpr->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ܐܢ( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      ܐܢ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3ErrorMsg(pParse, "unknown function: %#T()", pExpr);
        اكسر;
      }
      ܐܢ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        ܦܢܐ exprCodeInlineFunction(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }ܐܚܪܢܐ ܐܢ( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3ExprFunctionUsable(pParse, pExpr, pDef);
      }

      لكل(i=0; i<nFarg; i++){
        ܐܢ( i<32 && sqlite3ExprIsConstant(pParse, pFarg->a[i].pExpr) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        ܐܢ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3ExprCollSeq(pParse, pFarg->a[i].pExpr);
        }
      }
      ܐܢ( pFarg ){
        ܐܢ( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ܐܚܪܢܐ{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ܐܢ( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 exprOp;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].pExpr!=0 );
          exprOp = pFarg->a[0].pExpr->op;
          ܐܢ( exprOp==TK_COLUMN || exprOp==TK_AGG_COLUMN ){
            أكد( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].pExpr->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3ExprCodeExprList(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }ܐܚܪܢܐ{
        r1 = 0;
      }
#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Possibly overload the function if the first argument is
      ** a virtual table column.
      **
      ** For infix functions (LIKE, GLOB, REGEXP, and MATCH) use the
      ** second argument, not the first, as the argument to test to
      ** see if it is a column in a virtual table.  This is done because
      ** the left operand of infix functions (the operand we want to
      ** control overloading) ends up as the second argument to the
      ** function.  The expression "A glob B" is equivalent to
      ** "glob(B,A).  We want to use the A in "A glob B" to test
      ** for function overloading.  But we use the B term in "glob(B,A)".
      */
      ܐܢ( nFarg>=2 && ExprHasProperty(pExpr, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].pExpr);
      }ܐܚܪܢܐ ܐܢ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].pExpr);
      }
#endif
      ܐܢ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ܐܢ( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, pExpr->op2);
      ܐܢ( nFarg ){
        ܐܢ( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }ܐܚܪܢܐ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      ܦܢܐ target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT: {
      صحيح nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ܐܢ( pParse->db->mallocFailed ){
        ܦܢܐ 0;
      }ܐܚܪܢܐ ܐܢ( op==TK_SELECT
             && ALWAYS( ExprUseXSelect(pExpr) )
             && (nCol = pExpr->x.pSelect->pEList->nExpr)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }ܐܚܪܢܐ{
        ܦܢܐ sqlite3CodeSubselect(pParse, pExpr);
      }
      اكسر;
    }
    حالة TK_SELECT_COLUMN: {
      صحيح n;
      Expr *pLeft = pExpr->pLeft;
      ܐܢ( pLeft->iTable==0 || pParse->withinRJSubrtn > pLeft->op2 ){
        pLeft->iTable = sqlite3CodeSubselect(pParse, pLeft);
        pLeft->op2 = pParse->withinRJSubrtn;
      }
      أكد( pLeft->op==TK_SELECT || pLeft->op==TK_ERROR );
      n = sqlite3ExprVectorSize(pLeft);
      ܐܢ( pExpr->iTable!=n ){
        sqlite3ErrorMsg(pParse, "%d columns assigned %d values",
                                pExpr->iTable, n);
      }
      ܦܢܐ pLeft->iTable + pExpr->iColumn;
    }
    حالة TK_IN: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      ܦܢܐ target;
    }
#endif /* SQLITE_OMIT_SUBQUERY */


    /*
    **    x BETWEEN y AND z
    **
    ** This is equivalent to
    **
    **    x>=y AND x<=z
    **
    ** X is stored in pExpr->pLeft.
    ** Y is stored in pExpr->pList->a[0].pExpr.
    ** Z is stored in pExpr->pList->a[1].pExpr.
    */
    حالة TK_BETWEEN: {
      exprCodeBetween(pParse, pExpr, target, 0, 0);
      ܦܢܐ target;
    }
    حالة TK_COLLATE: {
      ܐܢ( !ExprHasProperty(pExpr, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( pExpr->pLeft );
        sqlite3ExprCode(pParse, pExpr->pLeft, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        ܦܢܐ target;
      }ܐܚܪܢܐ{
        pExpr = pExpr->pLeft;
        اذهب expr_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالة TK_SPAN:
    حالة TK_UPLUS: {
      pExpr = pExpr->pLeft;
      اذهب expr_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    حالة TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      **
      ** The expression is implemented using an OP_Param opcode. The p1
      ** parameter is set to 0 for an old.rowid reference, or to (i+1)
      ** to reference another column of the old.* pseudo-table, where
      ** i is the index of the column. For a new.rowid reference, p1 is
      ** set to (n+1), where n is the number of columns in each pseudo-table.
      ** For a reference to any other column in the new.* pseudo-table, p1
      ** is set to (n+2+i), where n and i are as defined previously. For
      ** example, if the table on which triggers are being fired is
      ** declared as:
      **
      **   CREATE TABLE t1(a, b);
      **
      ** Then p1 is interpreted as follows:
      **
      **   p1==0   ->    old.rowid     p1==3   ->    new.rowid
      **   p1==1   ->    old.a         p1==4   ->    new.a
      **   p1==2   ->    old.b         p1==5   ->    new.b      
      */
      Table *pTab;
      صحيح iCol;
      صحيح p1;

      أكد( ExprUseYTab(pExpr) );
      pTab = pExpr->y.pTab;
      iCol = pExpr->iColumn;
      p1 = pExpr->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      أكد( pExpr->iTable==0 || pExpr->iTable==1 );
      أكد( iCol>=-1 && iCol<pTab->nCol );
      أكد( pTab->iPKey<0 || iCol!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (pExpr->iTable ? "new" : "old"),
        (pExpr->iColumn<0 ? "rowid" : pExpr->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ܐܢ( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      اكسر;
    }

    حالة TK_VECTOR: {
      sqlite3ErrorMsg(pParse, "row value misused");
      اكسر;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    حالة TK_IF_NULL_ROW: {
      صحيح addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = pExpr->pAggInfo;
      ܐܢ( pAggInfo ){
        أكد( pExpr->iAgg>=0 && pExpr->iAgg<pAggInfo->nColumn );
        ܐܢ( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, pExpr->iAgg);
          اكسر;
        }
        ܐܢ( pExpr->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[pExpr->iAgg].iSorterColumn,
                            target);
          inReg = target;
          اكسر;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, pExpr->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3ExprCode(pParse, pExpr->pLeft, target);
      أكد( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      اكسر;
    }

    /*
    ** Form A:
    **   CASE x WHEN e1 THEN r1 WHEN e2 THEN r2 ... WHEN eN THEN rN ELSE y END
    **
    ** Form B:
    **   CASE WHEN e1 THEN r1 WHEN e2 THEN r2 ... WHEN eN THEN rN ELSE y END
    **
    ** Form A is can be transformed into the equivalent form B as follows:
    **   CASE WHEN x=e1 THEN r1 WHEN x=e2 THEN r2 ...
    **        WHEN x=eN THEN rN ELSE y END
    **
    ** X (if it exists) is in pExpr->pLeft.
    ** Y is in the last element of pExpr->x.pList if pExpr->x.pList->nExpr is
    ** odd.  The Y is also optional.  If the number of elements in x.pList
    ** is even, then Y is omitted and the "otherwise" result is NULL.
    ** Ei is in pExpr->pList->a[i*2] and Ri is pExpr->pList->a[i*2+1].
    **
    ** The result of the expression is the Ri for the first matching Ei,
    ** or if there is no matching Ei, the ELSE term Y, or if there is
    ** no ELSE term, NULL.
    */
    حالة TK_CASE: {
      صحيح endLabel;                     /* GOTO label for end of CASE stmt */
      صحيح nextCase;                     /* GOTO label for next WHEN clause */
      صحيح nExpr;                        /* 2x number of WHEN terms */
      صحيح i;                            /* Loop counter */
      ExprList *pEList;                 /* List of WHEN terms */
      بنية ExprList_item *aListelem;  /* Array of WHEN terms */
      Expr opCompare;                   /* The X==Ei expression */
      Expr *pX;                         /* The X expression */
      Expr *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      Expr *pDel = 0;
      sqlite3 *db = pParse->db;

      أكد( ExprUseXList(pExpr) && pExpr->x.pList!=0 );
      أكد(pExpr->x.pList->nExpr > 0);
      pEList = pExpr->x.pList;
      aListelem = pEList->a;
      nExpr = pEList->nExpr;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      ܐܢ( (pX = pExpr->pLeft)!=0 ){
        pDel = sqlite3ExprDup(db, pX, 0);
        ܐܢ( db->mallocFailed ){
          sqlite3ExprDelete(db, pDel);
          اكسر;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3ExprToRegister(pDel, exprCodeVector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, حجم(opCompare));
        opCompare.op = TK_EQ;
        opCompare.pLeft = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      لكل(i=0; i<nExpr-1; i=i+2){
        ܐܢ( pX ){
          أكد( pTest!=0 );
          opCompare.pRight = aListelem[i].pExpr;
        }ܐܚܪܢܐ{
          pTest = aListelem[i].pExpr;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3ExprIfFalse(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].pExpr->op==TK_COLUMN );
        sqlite3ExprCode(pParse, aListelem[i+1].pExpr, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      ܐܢ( (nExpr&1)!=0 ){
        sqlite3ExprCode(pParse, pEList->a[nExpr-1].pExpr, target);
      }ܐܚܪܢܐ{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3ExprDelete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      اكسر;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      أكد( pExpr->affExpr==OE_Rollback
           || pExpr->affExpr==OE_Abort
           || pExpr->affExpr==OE_Fail
           || pExpr->affExpr==OE_Ignore
      );
      ܐܢ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3ErrorMsg(pParse,
                       "RAISE() may only be used within a trigger-program");
        ܦܢܐ 0;
      }
      ܐܢ( pExpr->affExpr==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      ܐܢ( pExpr->affExpr==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             pExpr->affExpr, r1);
      }
      اكسر;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  ܦܢܐ inReg;
}

/*
** Generate code that will evaluate expression pExpr just one time
** per prepared statement execution.
**
** If the expression uses functions (that might throw an exception) then
** guard them with an OP_Once opcode to ensure that the code is only executed
** once. If no functions are involved, then factor the code out and put it at
** the end of the prepared statement in the initialization section.
**
** If regDest>0 then the result is always stored in that register and the
** result is not reusable.  If regDest<0 then this routine is free to
** store the value wherever it wants.  The register where the expression
** is stored is returned.  When regDest<0, two identical expressions might
** code to the same register, if they do not contain function calls and hence
** are factored out into the initialization section at the end of the
** prepared statement.
*/
صحيح sqlite3ExprCodeRunJustOnce(
  Parse *pParse,    /* Parsing context */
  Expr *pExpr,      /* The expression to code when the VDBE initializes */
  صحيح regDest       /* Store the value in this register */
){
  ExprList *p;
  أكد( ConstFactorOk(pParse) );
  أكد( regDest!=0 );
  p = pParse->pConstExpr;
  ܐܢ( regDest<0 && p ){
    بنية ExprList_item *pItem;
    صحيح i;
    لكل(pItem=p->a, i=p->nExpr; i>0; pItem++, i--){
      ܐܢ( pItem->fg.reusable
       && sqlite3ExprCompare(0,pItem->pExpr,pExpr,-1)==0
      ){
        ܦܢܐ pItem->u.iConstExprReg;
      }
    }
  }
  pExpr = sqlite3ExprDup(pParse->db, pExpr, 0);
  ܐܢ( pExpr!=0 && ExprHasProperty(pExpr, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحيح addr;
    أكد( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    ܐܢ( !pParse->db->mallocFailed ){
      ܐܢ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3ExprCode(pParse, pExpr, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3ExprDelete(pParse->db, pExpr);
    sqlite3VdbeJumpHere(v, addr);
  }ܐܚܪܢܐ{
    p = sqlite3ExprListAppend(pParse, p, pExpr);
    ܐܢ( p ){
       بنية ExprList_item *pItem = &p->a[p->nExpr-1];
       pItem->fg.reusable = regDest<0;
       ܐܢ( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.iConstExprReg = regDest;
    }
    pParse->pConstExpr = p;
  }
  ܦܢܐ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE فراغ sqlite3ExprNullRegisterRange(
  Parse *pParse,   /* Parsing context */
  صحيح iReg,        /* First register to set to NULL */
  صحيح nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  Expr t;
  memset(&t, 0, حجم(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3ExprCodeRunJustOnce(pParse, &t, iReg);
  pParse->okConstFactor = okConstFactor;
}

/*
** Generate code to evaluate an expression and store the results
** into a register.  Return the register number where the results
** are stored.
**
** If the register is a temporary register that can be deallocated,
** then write its number into *pReg.  If the result register is not
** a temporary, then set *pReg to zero.
**
** If pExpr is a constant, then this routine might generate this
** code to fill the register in the initialization section of the
** VDBE program, in order to factor it out of the evaluation loop.
*/
صحيح sqlite3ExprCodeTemp(Parse *pParse, Expr *pExpr, صحيح *pReg){
  صحيح r2;
  pExpr = sqlite3ExprSkipCollateAndLikely(pExpr);
  ܐܢ( ConstFactorOk(pParse)
   && ALWAYS(pExpr!=0)
   && pExpr->op!=TK_REGISTER
   && sqlite3ExprIsConstantNotJoin(pParse, pExpr)
  ){
    *pReg  = 0;
    r2 = sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
  }ܐܚܪܢܐ{
    صحيح r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3ExprCodeTarget(pParse, pExpr, r1);
    ܐܢ( r2==r1 ){
      *pReg = r1;
    }ܐܚܪܢܐ{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  ܦܢܐ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
فراغ sqlite3ExprCode(Parse *pParse, Expr *pExpr, صحيح target){
  صحيح inReg;

  أكد( pExpr==0 || !ExprHasVVAProperty(pExpr,EP_Immutable) );
  أكد( target>0 && target<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ܐܢ( pParse->pVdbe==0 ) ܦܢܐ;
  inReg = sqlite3ExprCodeTarget(pParse, pExpr, target);
  ܐܢ( inReg!=target ){
    u8 op;
    Expr *pX = sqlite3ExprSkipCollateAndLikely(pExpr);
    testcase( pX!=pExpr );
    ܐܢ( ALWAYS(pX)
     && (ExprHasProperty(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }ܐܚܪܢܐ{
      op = OP_SCopy;
    }
    sqlite3VdbeAddOp2(pParse->pVdbe, op, inReg, target);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
فراغ sqlite3ExprCodeCopy(Parse *pParse, Expr *pExpr, صحيح target){
  sqlite3 *db = pParse->db;
  pExpr = sqlite3ExprDup(db, pExpr, 0);
  ܐܢ( !db->mallocFailed ) sqlite3ExprCode(pParse, pExpr, target);
  sqlite3ExprDelete(db, pExpr);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
فراغ sqlite3ExprCodeFactorable(Parse *pParse, Expr *pExpr, صحيح target){
  ܐܢ( pParse->okConstFactor && sqlite3ExprIsConstantNotJoin(pParse,pExpr) ){
    sqlite3ExprCodeRunJustOnce(pParse, pExpr, target);
  }ܐܚܪܢܐ{
    sqlite3ExprCodeCopy(pParse, pExpr, target);
  }
}

/*
** Generate code that pushes the value of every element of the given
** expression list into a sequence of registers beginning at target.
**
** Return the number of elements evaluated.  The number returned will
** usually be pList->nExpr but might be reduced if SQLITE_ECEL_OMITREF
** is defined.
**
** The SQLITE_ECEL_DUP flag prevents the arguments from being
** filled using OP_SCopy.  OP_Copy must be used instead.
**
** The SQLITE_ECEL_FACTOR argument allows constant arguments to be
** factored out into initialization code.
**
** The SQLITE_ECEL_REF flag means that expressions in the list with
** ExprList.a[].u.x.iOrderByCol>0 have already been evaluated and stored
** in registers at srcReg, and so the value can be copied from there.
** If SQLITE_ECEL_OMITREF is also set, then the values with u.x.iOrderByCol>0
** are simply omitted rather than being copied from srcReg.
*/
صحيح sqlite3ExprCodeExprList(
  Parse *pParse,     /* Parsing context */
  ExprList *pList,   /* The expression list to be coded */
  صحيح target,        /* Where to write results */
  صحيح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  بنية ExprList_item *pItem;
  صحيح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( target>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->nExpr;
  ܐܢ( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  لكل(pItem=pList->a, i=0; i<n; i++, pItem++){
    Expr *pExpr = pItem->pExpr;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ܐܢ( pItem->fg.bSorterRef ){
      i--;
      n--;
    }ܐܚܪܢܐ
#endif
    ܐܢ( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      ܐܢ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ܐܚܪܢܐ{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }ܐܚܪܢܐ ܐܢ( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3ExprIsConstantNotJoin(pParse,pExpr)
    ){
      sqlite3ExprCodeRunJustOnce(pParse, pExpr, target+i);
    }ܐܚܪܢܐ{
      صحيح inReg = sqlite3ExprCodeTarget(pParse, pExpr, target+i);
      ܐܢ( inReg!=target+i ){
        VdbeOp *pOp;
        ܐܢ( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ܐܚܪܢܐ{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  ܦܢܐ n;
}

/*
** Generate code for a BETWEEN operator.
**
**    x BETWEEN y AND z
**
** The above is equivalent to
**
**    x>=y AND x<=z
**
** Code it as such, taking care to do the common subexpression
** elimination of x.
**
** The xJumpIf parameter determines details:
**
**    NULL:                   Store the boolean result in reg[dest]
**    sqlite3ExprIfTrue:      Jump to dest if true
**    sqlite3ExprIfFalse:     Jump to dest if false
**
** The jumpIfNull parameter is ignored if xJumpIf is NULL.
*/
ساكن فراغ exprCodeBetween(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* The BETWEEN expression */
  صحيح dest,         /* Jump destination or storage location */
  فراغ (*xJump)(Parse*,Expr*,صحيح,صحيح), /* Action to take */
  صحيح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  Expr exprAnd;     /* The AND operator in  x>=y AND x<=z  */
  Expr compLeft;    /* The  x>=y  term */
  Expr compRight;   /* The  x<=z  term */
  صحيح regFree1 = 0; /* Temporary use register */
  Expr *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&compLeft, 0, حجم(Expr));
  memset(&compRight, 0, حجم(Expr));
  memset(&exprAnd, 0, حجم(Expr));

  أكد( ExprUseXList(pExpr) );
  pDel = sqlite3ExprDup(db, pExpr->pLeft, 0);
  ܐܢ( db->mallocFailed==0 ){
    exprAnd.op = TK_AND;
    exprAnd.pLeft = &compLeft;
    exprAnd.pRight = &compRight;
    compLeft.op = TK_GE;
    compLeft.pLeft = pDel;
    compLeft.pRight = pExpr->x.pList->a[0].pExpr;
    compRight.op = TK_LE;
    compRight.pLeft = pDel;
    compRight.pRight = pExpr->x.pList->a[1].pExpr;
    sqlite3ExprToRegister(pDel, exprCodeVector(pParse, pDel, &regFree1));
    ܐܢ( xJump ){
      xJump(pParse, &exprAnd, dest, jumpIfNull);
    }ܐܚܪܢܐ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3ExprCodeTarget(pParse, &exprAnd, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3ExprDelete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3ExprIfTrue  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3ExprIfTrue  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3ExprIfTrue  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3ExprIfTrue  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3ExprIfFalse && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3ExprIfFalse && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3ExprIfFalse && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3ExprIfFalse && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==0 );
}

/*
** Generate code for a boolean expression such that a jump is made
** to the label "dest" if the expression is true but execution
** continues straight thru if the expression is false.
**
** If the expression evaluates to NULL (neither true nor false), then
** take the jump if the jumpIfNull flag is SQLITE_JUMPIFNULL.
**
** This code depends on the fact that certain token values (ex: TK_EQ)
** are the same as opcode values (ex: OP_Eq) that implement the corresponding
** operation.  Special comments in vdbe.c and the mkopcodeh.awk script in
** the make process cause these values to align.  Assert()s in the code
** below verify that the numbers are aligned correctly.
*/
فراغ sqlite3ExprIfTrue(Parse *pParse, Expr *pExpr, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ܐܢ( NEVER(v==0) )     ܦܢܐ;  /* Existence of VDBE checked by caller */
  ܐܢ( NEVER(pExpr==0) ) ܦܢܐ;  /* No way this can happen */
  أكد( !ExprHasVVAProperty(pExpr, EP_Immutable) );
  op = pExpr->op;
  بدل( op ){
    حالة TK_AND:
    حالة TK_OR: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      ܐܢ( pAlt!=pExpr ){
        sqlite3ExprIfTrue(pParse, pAlt, dest, jumpIfNull);
      }ܐܚܪܢܐ{
        Expr *pFirst, *pSecond;
        ܐܢ( exprEvalRhsFirst(pExpr) ){
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        }ܐܚܪܢܐ{
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        ܐܢ( op==TK_AND ){
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3ExprIfFalse(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ܐܚܪܢܐ{
          testcase( jumpIfNull==0 );
          sqlite3ExprIfTrue(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = pExpr->op2==TK_ISNOT;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ܐܢ( isTrue ^ isNot ){
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ܐܚܪܢܐ{
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      صحيح addrIsNull;
      ܐܢ( sqlite3ExprIsVector(pExpr->pLeft) ) اذهب default_expr;
      ܐܢ( ExprHasProperty(pExpr, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull, ExprHasProperty(pExpr,EP_Commuted));
      أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      أكد(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ܐܢ( addrIsNull ){
        ܐܢ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ܐܚܪܢܐ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      ܐܢ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfTrue, jumpIfNull);
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_IN: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      اكسر;
    }
#endif
    افتراضي: {
    default_expr:
      ܐܢ( ExprAlwaysTrue(pExpr) ){
        sqlite3VdbeGoto(v, dest);
      }ܐܚܪܢܐ ܐܢ( ExprAlwaysFalse(pExpr) ){
        /* No-op */
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2); 
}

/*
** Generate code for a boolean expression such that a jump is made
** to the label "dest" if the expression is false but execution
** continues straight thru if the expression is true.
**
** If the expression evaluates to NULL (neither true nor false) then
** jump if jumpIfNull is SQLITE_JUMPIFNULL or fall through if jumpIfNull
** is 0.
*/
فراغ sqlite3ExprIfFalse(Parse *pParse, Expr *pExpr, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ܐܢ( NEVER(v==0) ) ܦܢܐ; /* Existence of VDBE checked by caller */
  ܐܢ( pExpr==0 )    ܦܢܐ;
  أكد( !ExprHasVVAProperty(pExpr,EP_Immutable) );

  /* The value of pExpr->op and op are related as follows:
  **
  **       pExpr->op            op
  **       ---------          ----------
  **       TK_ISNULL          OP_NotNull
  **       TK_NOTNULL         OP_IsNull
  **       TK_NE              OP_Eq
  **       TK_EQ              OP_Ne
  **       TK_GT              OP_Le
  **       TK_LE              OP_Gt
  **       TK_GE              OP_Lt
  **       TK_LT              OP_Ge
  **
  ** For other values of pExpr->op, op is undefined and unused.
  ** The value of TK_ and OP_ constants are arranged such that we
  ** can compute the mapping above using the following expression.
  ** Assert()s verify that the computation is correct.
  */
  op = ((pExpr->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  أكد( pExpr->op!=TK_ISNULL || op==OP_NotNull );
  أكد( pExpr->op!=TK_NOTNULL || op==OP_IsNull );
  أكد( pExpr->op!=TK_NE || op==OP_Eq );
  أكد( pExpr->op!=TK_EQ || op==OP_Ne );
  أكد( pExpr->op!=TK_LT || op==OP_Ge );
  أكد( pExpr->op!=TK_LE || op==OP_Gt );
  أكد( pExpr->op!=TK_GT || op==OP_Le );
  أكد( pExpr->op!=TK_GE || op==OP_Lt );

  بدل( pExpr->op ){
    حالة TK_AND:
    حالة TK_OR: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      ܐܢ( pAlt!=pExpr ){
        sqlite3ExprIfFalse(pParse, pAlt, dest, jumpIfNull);
      }ܐܚܪܢܐ{
        Expr *pFirst, *pSecond;
        ܐܢ( exprEvalRhsFirst(pExpr) ){
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        }ܐܚܪܢܐ{
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        ܐܢ( pExpr->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3ExprIfFalse(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
        }ܐܚܪܢܐ{
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3ExprIfTrue(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = pExpr->op2==TK_ISNOT;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ܐܢ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ܐܚܪܢܐ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT:
      testcase( pExpr->op==TK_IS );
      testcase( pExpr->op==TK_ISNOT );
      op = (pExpr->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      صحيح addrIsNull;
      ܐܢ( sqlite3ExprIsVector(pExpr->pLeft) ) اذهب default_expr;
      ܐܢ( ExprHasProperty(pExpr, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull,ExprHasProperty(pExpr,EP_Commuted));
      أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      أكد(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ܐܢ( addrIsNull ){
        ܐܢ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ܐܚܪܢܐ{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      ܐܢ( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfFalse, jumpIfNull);
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_IN: {
      ܐܢ( jumpIfNull ){
        sqlite3ExprCodeIN(pParse, pExpr, dest, dest);
      }ܐܚܪܢܐ{
        صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3ExprCodeIN(pParse, pExpr, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      اكسر;
    }
#endif
    افتراضي: {
    default_expr:
      ܐܢ( ExprAlwaysFalse(pExpr) ){
        sqlite3VdbeGoto(v, dest);
      }ܐܚܪܢܐ ܐܢ( ExprAlwaysTrue(pExpr) ){
        /* no-op */
      }ܐܚܪܢܐ{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
فراغ sqlite3ExprIfFalseDup(Parse *pParse, Expr *pExpr, صحيح dest,صحيح jumpIfNull){
  sqlite3 *db = pParse->db;
  Expr *pCopy = sqlite3ExprDup(db, pExpr, 0);
  ܐܢ( db->mallocFailed==0 ){
    sqlite3ExprIfFalse(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3ExprDelete(db, pCopy);
}

/*
** Expression pVar is guaranteed to be an SQL variable. pExpr may be any
** type of expression.
**
** If pExpr is a simple SQL value - an integer, real, string, blob
** or NULL value - then the VDBE currently being prepared is configured
** to re-prepare each time a new value is bound to variable pVar.
**
** Additionally, if pExpr is a simple SQL value and the value is the
** same as that currently bound to variable pVar, non-zero is returned.
** Otherwise, if the values are not the same or if pExpr is not a simple
** SQL value, zero is returned.
**
** If the SQLITE_EnableQPSG flag is set on the database connection, then
** this routine always returns false.
*/
ساكن SQLITE_NOINLINE صحيح exprCompareVariable(
  ثابت Parse *pParse,
  ثابت Expr *pVar,
  ثابت Expr *pExpr
){
  صحيح res = 2;
  صحيح iVar;
  sqlite3_value *pL, *pR = 0;
 
  ܐܢ( pExpr->op==TK_VARIABLE && pVar->iColumn==pExpr->iColumn ){
    ܦܢܐ 0;
  }
  ܐܢ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ܦܢܐ 2;
  sqlite3ValueFromExpr(pParse->db, pExpr, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ܐܢ( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3VdbeGetBoundValue(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ܐܢ( pL ){
      ܐܢ( sqlite3_value_type(pL)==SQLITE_TEXT ){
        sqlite3_value_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3ValueFree(pR);
    sqlite3ValueFree(pL);
  }
  ܦܢܐ res;
}

/*
** Do a deep comparison of two expression trees.  Return 0 if the two
** expressions are completely identical.  Return 1 if they differ only
** by a COLLATE operator at the top level.  Return 2 if there are differences
** other than the top-level COLLATE operator.
**
** If any subelement of pB has Expr.iTable==(-1) then it is allowed
** to compare equal to an equivalent element in pA with Expr.iTable==iTab.
**
** The pA side might be using TK_REGISTER.  If that is the case and pB is
** not using TK_REGISTER but is otherwise equivalent, then still return 0.
**
** Sometimes this routine will return 2 even if the two expressions
** really are equivalent.  If we cannot prove that the expressions are
** identical, we return 2 just to be safe.  So if this routine
** returns 2, then you do not really know for certain if the two
** expressions are the same.  But if you get a 0 or 1 return, then you
** can be sure the expressions are the same.  In the places where
** this routine is used, it does not hurt to get an extra 2 - that
** just might result in some slightly slower code.  But returning
** an incorrect 0 or 1 could lead to a malfunction.
**
** If pParse is not NULL and SQLITE_EnableQPSG is off then TK_VARIABLE
** terms in pA with bindings in pParse->pReprepare can be matched against
** literals in pB.  The pParse->pVdbe->expmask bitmask is updated for
** each variable referenced.
*/
صحيح sqlite3ExprCompare(
  ثابت Parse *pParse,
  ثابت Expr *pA,
  ثابت Expr *pB,
  صحيح iTab
){
  u32 combinedFlags;
  ܐܢ( pA==0 || pB==0 ){
    ܦܢܐ pB==pA ? 0 : 2;
  }
  ܐܢ( pParse && pA->op==TK_VARIABLE ){
    ܦܢܐ exprCompareVariable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ܐܢ( combinedFlags & EP_IntValue ){
    ܐܢ( (pA->flags&pB->flags&EP_IntValue)!=0 && pA->u.iValue==pB->u.iValue ){
      ܦܢܐ 0;
    }
    ܦܢܐ 2;
  }
  ܐܢ( pA->op!=pB->op || pA->op==TK_RAISE ){
    ܐܢ( pA->op==TK_COLLATE && sqlite3ExprCompare(pParse, pA->pLeft,pB,iTab)<2 ){
      ܦܢܐ 1;
    }
    ܐܢ( pB->op==TK_COLLATE && sqlite3ExprCompare(pParse, pA,pB->pLeft,iTab)<2 ){
      ܦܢܐ 1;
    }
    ܐܢ( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }ܐܚܪܢܐ{
      ܦܢܐ 2;
    }
  }
  أكد( !ExprHasProperty(pA, EP_IntValue) );
  أكد( !ExprHasProperty(pB, EP_IntValue) );
  ܐܢ( pA->u.zToken ){
    ܐܢ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ܐܢ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) ܦܢܐ 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      ܐܢ( ExprHasProperty(pA,EP_WinFunc)!=ExprHasProperty(pB,EP_WinFunc) ){
        ܦܢܐ 2;
      }
      ܐܢ( ExprHasProperty(pA,EP_WinFunc) ){
        ܐܢ( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          ܦܢܐ 2;
        }
      }
#endif
    }ܐܚܪܢܐ ܐܢ( pA->op==TK_NULL ){
      ܦܢܐ 0;
    }ܐܚܪܢܐ ܐܢ( pA->op==TK_COLLATE ){
      ܐܢ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) ܦܢܐ 2;
    }ܐܚܪܢܐ
    ܐܢ( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      ܦܢܐ 2;
    }
  }
  ܐܢ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) ܦܢܐ 2;
  ܐܢ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ܐܢ( combinedFlags & EP_xIsSelect ) ܦܢܐ 2;
    ܐܢ( (combinedFlags & EP_FixedCol)==0
     && sqlite3ExprCompare(pParse, pA->pLeft, pB->pLeft, iTab) ) ܦܢܐ 2;
    ܐܢ( sqlite3ExprCompare(pParse, pA->pRight, pB->pRight, iTab) ) ܦܢܐ 2;
    ܐܢ( sqlite3ExprListCompare(pA->x.pList, pB->x.pList, iTab) ) ܦܢܐ 2;
    ܐܢ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ܐܢ( pA->iColumn!=pB->iColumn ) ܦܢܐ 2;
      ܐܢ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) ܦܢܐ 2;
      ܐܢ( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        ܦܢܐ 2;
      }
    }
  }
  ܦܢܐ 0;
}

/*
** Compare two ExprList objects.  Return 0 if they are identical, 1
** if they are certainly different, or 2 if it is not possible to
** determine if they are identical or not.
**
** If any subelement of pB has Expr.iTable==(-1) then it is allowed
** to compare equal to an equivalent element in pA with Expr.iTable==iTab.
**
** This routine might return non-zero for equivalent ExprLists.  The
** only consequence will be disabled optimizations.  But this routine
** must never return 0 if the two ExprList objects are different, or
** a malfunction will result.
**
** Two NULL pointers are considered to be the same.  But a NULL pointer
** always differs from a non-NULL pointer.
*/
صحيح sqlite3ExprListCompare(ثابت ExprList *pA, ثابت ExprList *pB, صحيح iTab){
  صحيح i;
  ܐܢ( pA==0 && pB==0 ) ܦܢܐ 0;
  ܐܢ( pA==0 || pB==0 ) ܦܢܐ 1;
  ܐܢ( pA->nExpr!=pB->nExpr ) ܦܢܐ 1;
  لكل(i=0; i<pA->nExpr; i++){
    صحيح res;
    Expr *pExprA = pA->a[i].pExpr;
    Expr *pExprB = pB->a[i].pExpr;
    ܐܢ( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) ܦܢܐ 1;
    ܐܢ( (res = sqlite3ExprCompare(0, pExprA, pExprB, iTab)) ) ܦܢܐ res;
  }
  ܦܢܐ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحيح sqlite3ExprCompareSkip(Expr *pA,Expr *pB, صحيح iTab){
  ܦܢܐ sqlite3ExprCompare(0,
             sqlite3ExprSkipCollate(pA),
             sqlite3ExprSkipCollate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ساكن صحيح exprImpliesNotNull(
  ثابت Parse *pParse,/* Parsing context */
  ثابت Expr *p,      /* The expression to be checked */
  ثابت Expr *pNN,    /* The expression that is NOT NULL */
  صحيح iTab,           /* Table being evaluated */
  صحيح seenNot         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  ܐܢ( sqlite3ExprCompare(pParse, p, pNN, iTab)==0 ){
    ܦܢܐ pNN->op!=TK_NULL;
  }
  بدل( p->op ){
    حالة TK_IN: {
      ܐܢ( seenNot && ExprHasProperty(p, EP_xIsSelect) ) ܦܢܐ 0;
      أكد( ExprUseXSelect(p) || (p->x.pList!=0 && p->x.pList->nExpr>0) );
      ܦܢܐ exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    حالة TK_BETWEEN: {
      ExprList *pList;
      أكد( ExprUseXList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->nExpr==2 );
      ܐܢ( seenNot ) ܦܢܐ 0;
      ܐܢ( exprImpliesNotNull(pParse, pList->a[0].pExpr, pNN, iTab, 1)
       || exprImpliesNotNull(pParse, pList->a[1].pExpr, pNN, iTab, 1)
      ){
        ܦܢܐ 1;
      }
      ܦܢܐ exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    حالة TK_EQ:
    حالة TK_NE:
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_PLUS:
    حالة TK_MINUS:
    حالة TK_BITOR:
    حالة TK_LSHIFT:
    حالة TK_RSHIFT:
    حالة TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    حالة TK_STAR:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_SLASH: {
      ܐܢ( exprImpliesNotNull(pParse, p->pRight, pNN, iTab, seenNot) ) ܦܢܐ 1;
      /* no break */ deliberate_fall_through
    }
    حالة TK_SPAN:
    حالة TK_COLLATE:
    حالة TK_UPLUS:
    حالة TK_UMINUS: {
      ܦܢܐ exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, seenNot);
    }
    حالة TK_TRUTH: {
      ܐܢ( seenNot ) ܦܢܐ 0;
      ܐܢ( p->op2!=TK_IS ) ܦܢܐ 0;
      ܦܢܐ exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      ܦܢܐ exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
  }
  ܦܢܐ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ساكن صحيح sqlite3ExprIsNotTrue(Expr *pExpr){
  صحيح v;
  ܐܢ( pExpr->op==TK_NULL ) ܦܢܐ 1;
  ܐܢ( pExpr->op==TK_TRUEFALSE && sqlite3ExprTruthValue(pExpr)==0 ) ܦܢܐ 1;
  v = 1;
  ܐܢ( sqlite3ExprIsInteger(pExpr, &v, 0, 0) && v==0 ) ܦܢܐ 1;
  ܦܢܐ 0;
}

/*
** Return true if the expression is one of the following:
**
**    CASE WHEN x THEN y END
**    CASE WHEN x THEN y ELSE NULL END
**    CASE WHEN x THEN y ELSE false END
**    iif(x,y)
**    iif(x,y,NULL)
**    iif(x,y,false)
*/
ساكن صحيح sqlite3ExprIsIIF(sqlite3 *db, ثابت Expr *pExpr){
  ExprList *pList;
  ܐܢ( pExpr->op==TK_FUNCTION ){
    ثابت حرف *z = pExpr->u.zToken;
    FuncDef *pDef;
    ܐܢ( (z[0]!='i' && z[0]!='I') ) ܦܢܐ 0;
    ܐܢ( pExpr->x.pList==0 ) ܦܢܐ 0;
    pDef = sqlite3FindFunction(db, z, pExpr->x.pList->nExpr, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ܐܢ( pDef==0 ) ܦܢܐ 0;
#ܐܚܪܢܐ
    ܐܢ( NEVER(pDef==0) ) ܦܢܐ 0;
#endif
    ܐܢ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) ܦܢܐ 0;
    ܐܢ( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) ܦܢܐ 0;
  }ܐܚܪܢܐ ܐܢ( pExpr->op==TK_CASE ){
    ܐܢ( pExpr->pLeft!=0 ) ܦܢܐ 0;
  }ܐܚܪܢܐ{
    ܦܢܐ 0;
  }
  pList = pExpr->x.pList;
  أكد( pList!=0 );
  ܐܢ( pList->nExpr==2 ) ܦܢܐ 1;
  ܐܢ( pList->nExpr==3 && sqlite3ExprIsNotTrue(pList->a[2].pExpr) ) ܦܢܐ 1;
  ܦܢܐ 0;
}

/*
** Return true if we can prove the pE2 will always be true if pE1 is
** true.  Return false if we cannot complete the proof or if pE2 might
** be false.  Examples:
**
**     pE1: x==5        pE2: x==5             Result: true
**     pE1: x>0         pE2: x==5             Result: false
**     pE1: x=21        pE2: x=21 OR y=43     Result: true
**     pE1: x!=123      pE2: x IS NOT NULL    Result: true
**     pE1: x!=?1       pE2: x IS NOT NULL    Result: true
**     pE1: x IS NULL   pE2: x IS NOT NULL    Result: false
**     pE1: x IS ?2     pE2: x IS NOT NULL    Result: false
**     pE1: iif(x,y)    pE2: x                Result: true
**     PE1: iif(x,y,0)  pE2: x                Result: true
**
** When comparing TK_COLUMN nodes between pE1 and pE2, if pE2 has
** Expr.iTable<0 then assume a table number given by iTab.
**
** If pParse is not NULL, then the values of bound variables in pE1 are
** compared against literal values in pE2 and pParse->pVdbe->expmask is
** modified to record which bound variables are referenced.  If pParse
** is NULL, then false will be returned if pE1 contains any bound variables.
**
** When in doubt, return false.  Returning true might give a performance
** improvement.  Returning false might cause a performance reduction, but
** it will always give the correct answer and is hence always safe.
*/
صحيح sqlite3ExprImpliesExpr(
  ثابت Parse *pParse,
  ثابت Expr *pE1,
  ثابت Expr *pE2,
  صحيح iTab
){
  ܐܢ( sqlite3ExprCompare(pParse, pE1, pE2, iTab)==0 ){
    ܦܢܐ 1;
  }
  ܐܢ( pE2->op==TK_OR
   && (sqlite3ExprImpliesExpr(pParse, pE1, pE2->pLeft, iTab)
             || sqlite3ExprImpliesExpr(pParse, pE1, pE2->pRight, iTab) )
  ){
    ܦܢܐ 1;
  }
  ܐܢ( pE2->op==TK_NOTNULL
   && exprImpliesNotNull(pParse, pE1, pE2->pLeft, iTab, 0)
  ){
    ܦܢܐ 1;
  }
  ܐܢ( sqlite3ExprIsIIF(pParse->db, pE1) ){
    ܦܢܐ sqlite3ExprImpliesExpr(pParse,pE1->x.pList->a[0].pExpr,pE2,iTab);
  }
  ܦܢܐ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ساكن فراغ bothImplyNotNullRow(Walker *pWalker, Expr *pE1, Expr *pE2){
  ܐܢ( pWalker->eCode==0 ){
    sqlite3WalkExpr(pWalker, pE1);
    ܐܢ( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3WalkExpr(pWalker, pE2);
    }
  }
}

/*
** This is the Expr node callback for sqlite3ExprImpliesNonNullRow().
** If the expression node requires that the table at pWalker->iCur
** have one or more non-NULL column, then set pWalker->eCode to 1 and abort.
**
** pWalker->mWFlags is non-zero if this inquiry is being undertaking on
** behalf of a RIGHT JOIN (or FULL JOIN).  That makes a difference when
** evaluating terms in the ON clause of an inner join.
**
** This routine controls an optimization.  False positives (setting
** pWalker->eCode to 1 when it should not be) are deadly, but false-negatives
** (never setting pWalker->eCode) is a harmless missed optimization.
*/
ساكن صحيح impliesNotNullRow(Walker *pWalker, Expr *pExpr){
  testcase( pExpr->op==TK_AGG_COLUMN );
  testcase( pExpr->op==TK_AGG_FUNCTION );
  ܐܢ( ExprHasProperty(pExpr, EP_OuterON) ) ܦܢܐ WRC_Prune;
  ܐܢ( ExprHasProperty(pExpr, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    ܦܢܐ WRC_Prune;
  }
  بدل( pExpr->op ){
    حالة TK_ISNOT:
    حالة TK_ISNULL:
    حالة TK_NOTNULL:
    حالة TK_IS:
    حالة TK_VECTOR:
    حالة TK_FUNCTION:
    حالة TK_TRUTH:
    حالة TK_CASE:
      testcase( pExpr->op==TK_ISNOT );
      testcase( pExpr->op==TK_ISNULL );
      testcase( pExpr->op==TK_NOTNULL );
      testcase( pExpr->op==TK_IS );
      testcase( pExpr->op==TK_VECTOR );
      testcase( pExpr->op==TK_FUNCTION );
      testcase( pExpr->op==TK_TRUTH );
      testcase( pExpr->op==TK_CASE );
      ܦܢܐ WRC_Prune;

    حالة TK_COLUMN:
      ܐܢ( pWalker->u.iCur==pExpr->iTable ){
        pWalker->eCode = 1;
        ܦܢܐ WRC_Abort;
      }
      ܦܢܐ WRC_Prune;

    حالة TK_OR:
    حالة TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( pExpr->op==TK_OR );
      testcase( pExpr->op==TK_AND );
      bothImplyNotNullRow(pWalker, pExpr->pLeft, pExpr->pRight);
      ܦܢܐ WRC_Prune;
       
    حالة TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ܐܢ( ExprUseXList(pExpr) && ALWAYS(pExpr->x.pList->nExpr>0) ){
        sqlite3WalkExpr(pWalker, pExpr->pLeft);
      }
      ܦܢܐ WRC_Prune;

    حالة TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( ExprUseXList(pExpr) );
      أكد( pExpr->x.pList->nExpr==2 );
      sqlite3WalkExpr(pWalker, pExpr->pLeft);
      bothImplyNotNullRow(pWalker, pExpr->x.pList->a[0].pExpr,
                                   pExpr->x.pList->a[1].pExpr);
      ܦܢܐ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    حالة TK_EQ:
    حالة TK_NE:
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE: {
      Expr *pLeft = pExpr->pLeft;
      Expr *pRight = pExpr->pRight;
      testcase( pExpr->op==TK_EQ );
      testcase( pExpr->op==TK_NE );
      testcase( pExpr->op==TK_LT );
      testcase( pExpr->op==TK_LE );
      testcase( pExpr->op==TK_GT );
      testcase( pExpr->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      أكد( pLeft->op!=TK_COLUMN || ExprUseYTab(pLeft) );
      أكد( pRight->op!=TK_COLUMN || ExprUseYTab(pRight) );
      ܐܢ( (pLeft->op==TK_COLUMN
           && ALWAYS(pLeft->y.pTab!=0)
           && IsVirtual(pLeft->y.pTab))
       || (pRight->op==TK_COLUMN
           && ALWAYS(pRight->y.pTab!=0)
           && IsVirtual(pRight->y.pTab))
      ){
        ܦܢܐ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    افتراضي:
      ܦܢܐ WRC_Continue;
  }
}

/*
** Return true (non-zero) if expression p can only be true if at least
** one column of table iTab is non-null.  In other words, return true
** if expression p will always be NULL or false if every column of iTab
** is NULL.
**
** False negatives are acceptable.  In other words, it is ok to return
** zero even if expression p will never be true of every column of iTab
** is NULL.  A false negative is merely a missed optimization opportunity.
**
** False positives are not allowed, however.  A false positive may result
** in an incorrect answer.
**
** Terms of p that are marked with EP_OuterON (and hence that come from
** the ON or USING clauses of OUTER JOINS) are excluded from the analysis.
**
** This routine is used to check if a LEFT JOIN can be converted into
** an ordinary JOIN.  The p argument is the WHERE clause.  If the WHERE
** clause requires that some column of the right table of the LEFT JOIN
** be non-NULL, then the LEFT JOIN can be safely converted into an
** ordinary join.
*/
صحيح sqlite3ExprImpliesNonNullRow(Expr *p, صحيح iTab, صحيح isRJ){
  Walker w;
  p = sqlite3ExprSkipCollateAndLikely(p);
  ܐܢ( p==0 ) ܦܢܐ 0;
  ܐܢ( p->op==TK_NOTNULL ){
    p = p->pLeft;
  }ܐܚܪܢܐ{
    ܟܡܐ( p->op==TK_AND ){
      ܐܢ( sqlite3ExprImpliesNonNullRow(p->pLeft, iTab, isRJ) ) ܦܢܐ 1;
      p = p->pRight;
    }
  }
  w.xExprCallback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3WalkExpr(&w, p);
  ܦܢܐ w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
بنية IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  صحيح iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ساكن صحيح exprIdxCover(Walker *pWalker, Expr *pExpr){
  ܐܢ( pExpr->op==TK_COLUMN
   && pExpr->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, pExpr->iColumn)<0
  ){
    pWalker->eCode = 1;
    ܦܢܐ WRC_Abort;
  }
  ܦܢܐ WRC_Continue;
}

/*
** Determine if an index pIdx on table with cursor iCur contains will
** the expression pExpr.  Return true if the index does cover the
** expression and false if the pExpr expression references table columns
** that are not found in the index pIdx.
**
** An index covering an expression means that the expression can be
** evaluated using only the index and without having to lookup the
** corresponding table entry.
*/
صحيح sqlite3ExprCoveredByIndex(
  Expr *pExpr,        /* The index to be tested */
  صحيح iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  بنية IdxCover xcov;
  memset(&w, 0, حجم(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.xExprCallback = exprIdxCover;
  w.u.pIdxCover = &xcov;
  sqlite3WalkExpr(&w, pExpr);
  ܦܢܐ !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
بنية RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  صحيح *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ساكن صحيح selectRefEnter(Walker *pWalker, Select *pSelect){
  بنية RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  صحيح *piNew;
  ܐܢ( pSrc->nSrc==0 ) ܦܢܐ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*حجم(صحيح));
  ܐܢ( piNew==0 ){
    p->nExclude = 0;
    ܦܢܐ WRC_Abort;
  }ܐܚܪܢܐ{
    p->aiExclude = piNew;
  }
  لكل(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  ܦܢܐ WRC_Continue;
}
ساكن فراغ selectRefLeave(Walker *pWalker, Select *pSelect){
  بنية RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ܐܢ( p->nExclude ){
    أكد( p->nExclude>=pSrc->nSrc );
    p->nExclude -= pSrc->nSrc;
  }
}

/* This is the Walker EXPR callback for sqlite3ReferencesSrcList().
**
** Set the 0x01 bit of pWalker->eCode if there is a reference to any
** of the tables shown in RefSrcList.pRef.
**
** Set the 0x02 bit of pWalker->eCode if there is a reference to a
** table is in neither RefSrcList.pRef nor RefSrcList.aiExclude.
*/
ساكن صحيح exprRefToSrcList(Walker *pWalker, Expr *pExpr){
  ܐܢ( pExpr->op==TK_COLUMN
   || pExpr->op==TK_AGG_COLUMN
  ){
    صحيح i;
    بنية RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحيح nSrc = pSrc ? pSrc->nSrc : 0;
    لكل(i=0; i<nSrc; i++){
      ܐܢ( pExpr->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        ܦܢܐ WRC_Continue;
      }
    }
    لكل(i=0; i<p->nExclude && p->aiExclude[i]!=pExpr->iTable; i++){}
    ܐܢ( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  ܦܢܐ WRC_Continue;
}

/*
** Check to see if pExpr references any tables in pSrcList.
** Possible return values:
**
**    1         pExpr does references a table in pSrcList.
**
**    0         pExpr references some table that is not defined in either
**              pSrcList or in subqueries of pExpr itself.
**
**   -1         pExpr only references no tables at all, or it only
**              references tables defined in subqueries of pExpr itself.
**
** As currently used, pExpr is always an aggregate function call.  That
** fact is exploited for efficiency.
*/
صحيح sqlite3ReferencesSrcList(Parse *pParse, Expr *pExpr, SrcList *pSrcList){
  Walker w;
  بنية RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, حجم(w));
  memset(&x, 0, حجم(x));
  w.xExprCallback = exprRefToSrcList;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( pExpr->op==TK_AGG_FUNCTION );
  أكد( ExprUseXList(pExpr) );
  sqlite3WalkExprList(&w, pExpr->x.pList);
  ܐܢ( pExpr->pLeft ){
    أكد( pExpr->pLeft->op==TK_ORDER );
    أكد( ExprUseXList(pExpr->pLeft) );
    أكد( pExpr->pLeft->x.pList!=0 );
    sqlite3WalkExprList(&w, pExpr->pLeft->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ܐܢ( ExprHasProperty(pExpr, EP_WinFunc) ){
    sqlite3WalkExpr(&w, pExpr->y.pWin->pFilter);
  }
#endif
  ܐܢ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ܐܢ( w.eCode & 0x01 ){
    ܦܢܐ 1;
  }ܐܚܪܢܐ ܐܢ( w.eCode ){
    ܦܢܐ 0;
  }ܐܚܪܢܐ{
    ܦܢܐ -1;
  }
}

/*
** This is a Walker expression node callback.
**
** For Expr nodes that contain pAggInfo pointers, make sure the AggInfo
** object that is referenced does not refer directly to the Expr.  If
** it does, make a copy.  This is done because the pExpr argument is
** subject to change.
**
** The copy is scheduled for deletion using the sqlite3ExprDeferredDelete()
** which builds on the sqlite3ParserAddCleanup() mechanism.
*/
ساكن صحيح agginfoPersistExprCb(Walker *pWalker, Expr *pExpr){
  ܐܢ( ALWAYS(!ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced))
   && pExpr->pAggInfo!=0
  ){
    AggInfo *pAggInfo = pExpr->pAggInfo;
    صحيح iAgg = pExpr->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    ܐܢ( pExpr->op!=TK_AGG_FUNCTION ){
      ܐܢ( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==pExpr
      ){
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        ܐܢ( pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr) ){
          pAggInfo->aCol[iAgg].pCExpr = pExpr;
        }
      }
    }ܐܚܪܢܐ{
      أكد( pExpr->op==TK_AGG_FUNCTION );
      ܐܢ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==pExpr
      ){
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        ܐܢ( pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr) ){
          pAggInfo->aFunc[iAgg].pFExpr = pExpr;
        }
      }
    }
  }
  ܦܢܐ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
فراغ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, حجم(*pWalker));
  pWalker->pParse = pParse;
  pWalker->xExprCallback = agginfoPersistExprCb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  صحيح i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       حجم(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  ܦܢܐ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  صحيح i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       حجم(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  ܦܢܐ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ساكن فراغ findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  Expr *pExpr          /* Expr describing the column to find or insert */
){
  بنية AggInfo_col *pCol;
  صحيح k;
  صحيح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  أكد( mxTerm <= SMXV(i16) );
  أكد( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  لكل(k=0; k<pAggInfo->nColumn; k++, pCol++){
    ܐܢ( pCol->pCExpr==pExpr ) ܦܢܐ;
    ܐܢ( pCol->iTable==pExpr->iTable
     && pCol->iColumn==pExpr->iColumn
     && pExpr->op!=TK_IF_NULL_ROW
    ){
      اذهب fix_up_expr;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  ܐܢ( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    ܦܢܐ;
  }
  ܐܢ( k>mxTerm ){
    sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  أكد( ExprUseYTab(pExpr) );
  pCol->pTab = pExpr->y.pTab;
  pCol->iTable = pExpr->iTable;
  pCol->iColumn = pExpr->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = pExpr;
  ܐܢ( pAggInfo->pGroupBy && pExpr->op!=TK_IF_NULL_ROW ){
    صحيح j, n;
    ExprList *pGB = pAggInfo->pGroupBy;
    بنية ExprList_item *pTerm = pGB->a;
    n = pGB->nExpr;
    لكل(j=0; j<n; j++, pTerm++){
      Expr *pE = pTerm->pExpr;
      ܐܢ( pE->op==TK_COLUMN
       && pE->iTable==pExpr->iTable
       && pE->iColumn==pExpr->iColumn
      ){
        pCol->iSorterColumn = j;
        اكسر;
      }
    }
  }
  ܐܢ( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_expr:
  ExprSetVVAProperty(pExpr, EP_NoReduce);
  أكد( pExpr->pAggInfo==0 || pExpr->pAggInfo==pAggInfo );
  pExpr->pAggInfo = pAggInfo;
  ܐܢ( pExpr->op==TK_COLUMN ){
    pExpr->op = TK_AGG_COLUMN;
  }
  أكد( k <= SMXV(pExpr->iAgg) );
  pExpr->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ساكن صحيح analyzeAggregate(Walker *pWalker, Expr *pExpr){
  صحيح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->iFirstReg==0 );
  بدل( pExpr->op ){
    افتراضي: {
      IndexedExpr *pIEpr;
      Expr tmp;
      أكد( pParse->iSelfTab==0 );
      ܐܢ( (pNC->ncFlags & NC_InAggFunc)==0 ) اكسر;
      ܐܢ( pParse->pIdxEpr==0 ) اكسر;
      لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحيح iDataCur = pIEpr->iDataCur;
        ܐܢ( iDataCur<0 ) استمر;
        ܐܢ( sqlite3ExprCompare(0, pExpr, pIEpr->pExpr, iDataCur)==0 ) اكسر;
      }
      ܐܢ( pIEpr==0 ) اكسر;
      ܐܢ( NEVER(!ExprUseYTab(pExpr)) ) اكسر;
      لكل(i=0; i<pSrcList->nSrc; i++){
         ܐܢ( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           اكسر;
         }
      }
      ܐܢ( i>=pSrcList->nSrc ) اكسر;
      ܐܢ( NEVER(pExpr->pAggInfo!=0) ) اكسر; /* Resolved by outer context */
      ܐܢ( pParse->nErr ){ ܦܢܐ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, حجم(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      ܐܢ( pParse->nErr ){ ܦܢܐ WRC_Abort; }
      أكد( pAggInfo->aCol!=0 );
      أكد( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = pExpr;
      pExpr->pAggInfo = pAggInfo;
      pExpr->iAgg = tmp.iAgg;
      ܦܢܐ WRC_Prune;
    }
    حالة TK_IF_NULL_ROW:
    حالة TK_AGG_COLUMN:
    حالة TK_COLUMN: {
      testcase( pExpr->op==TK_AGG_COLUMN );
      testcase( pExpr->op==TK_COLUMN );
      testcase( pExpr->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ܐܢ( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        لكل(i=0; i<pSrcList->nSrc; i++, pItem++){
          أكد( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
          ܐܢ( pExpr->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, pExpr);
            اكسر;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      ܦܢܐ WRC_Continue;
    }
    حالة TK_AGG_FUNCTION: {
      ܐܢ( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==pExpr->op2
       && pExpr->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        بنية AggInfo_func *pItem = pAggInfo->aFunc;
        صحيح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        أكد( mxTerm <= SMXV(i16) );
        لكل(i=0; i<pAggInfo->nFunc; i++, pItem++){
          ܐܢ( NEVER(pItem->pFExpr==pExpr) ) اكسر;
          ܐܢ( sqlite3ExprCompare(0, pItem->pFExpr, pExpr, -1)==0 ){
            اكسر;
          }
        }
        ܐܢ( i>mxTerm ){
          sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          أكد( i<pAggInfo->nFunc );
        }ܐܚܪܢܐ ܐܢ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          ܐܢ( i>=0 ){
            صحيح nArg;
            أكد( !ExprHasProperty(pExpr, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = pExpr;
            أكد( ExprUseUToken(pExpr) );
            nArg = pExpr->x.pList ? pExpr->x.pList->nExpr : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         pExpr->u.zToken, nArg, enc, 0);
            أكد( pItem->bOBUnique==0 );
            ܐܢ( pExpr->pLeft
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ExprList *pOBList;
              أكد( nArg>0 );
              أكد( pExpr->pLeft->op==TK_ORDER );
              أكد( ExprUseXList(pExpr->pLeft) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = pExpr->pLeft->x.pList;
              أكد( pOBList->nExpr>0 );
              أكد( pItem->bOBUnique==0 );
              ܐܢ( pOBList->nExpr==1
               && nArg==1
               && sqlite3ExprCompare(0,pOBList->a[0].pExpr,
                               pExpr->x.pList->a[0].pExpr,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = ExprHasProperty(pExpr, EP_Distinct);
              }ܐܚܪܢܐ{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ܐܚܪܢܐ{
              pItem->iOBTab = -1;
            }
            ܐܢ( ExprHasProperty(pExpr, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }ܐܚܪܢܐ{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        أكد( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
        ExprSetVVAProperty(pExpr, EP_NoReduce);
        أكد( i <= SMXV(pExpr->iAgg) );
        pExpr->iAgg = (i16)i;
        pExpr->pAggInfo = pAggInfo;
        ܦܢܐ WRC_Prune;
      }ܐܚܪܢܐ{
        ܦܢܐ WRC_Continue;
      }
    }
  }
  ܦܢܐ WRC_Continue;
}

/*
** Analyze the pExpr expression looking for aggregate functions and
** for variables that need to be added to AggInfo object that pNC->pAggInfo
** points to.  Additional entries are made on the AggInfo object as
** necessary.
**
** This routine should only be called after the expression has been
** analyzed by sqlite3ResolveExprNames().
*/
فراغ sqlite3ExprAnalyzeAggregates(NameContext *pNC, Expr *pExpr){
  Walker w;
  w.xExprCallback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  أكد( pNC->pSrcList!=0 );
  sqlite3WalkExpr(&w, pExpr);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
فراغ sqlite3ExprAnalyzeAggList(NameContext *pNC, ExprList *pList){
  بنية ExprList_item *pItem;
  صحيح i;
  ܐܢ( pList ){
    لكل(pItem=pList->a, i=0; i<pList->nExpr; i++, pItem++){
      sqlite3ExprAnalyzeAggregates(pNC, pItem->pExpr);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحيح sqlite3GetTempReg(Parse *pParse){
  ܐܢ( pParse->nTempReg==0 ){
    ܦܢܐ ++pParse->nMem;
  }
  ܦܢܐ pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
فراغ sqlite3ReleaseTempReg(Parse *pParse, صحيح iReg){
  ܐܢ( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحيح kk;
    لكل(kk=0; kk<pParse->nTempReg; kk++){
      أكد( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ܐܢ( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحيح sqlite3GetTempRange(Parse *pParse, صحيح nReg){
  صحيح i, n;
  ܐܢ( nReg==1 ) ܦܢܐ sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ܐܢ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ܐܚܪܢܐ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  ܦܢܐ i;
}
فراغ sqlite3ReleaseTempRange(Parse *pParse, صحيح iReg, صحيح nReg){
  ܐܢ( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    ܦܢܐ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ܐܢ( nReg>pParse->nRangeReg ){
    pParse->nRangeReg = nReg;
    pParse->iRangeReg = iReg;
  }
}

/*
** Mark all temporary registers as being unavailable for reuse.
**
** Always invoke this procedure after coding a subroutine or co-routine
** that might be invoked from other parts of the code, to ensure that
** the sub/co-routine does not use registers in common with the code that
** invokes the sub/co-routine.
*/
فراغ sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
فراغ sqlite3TouchRegister(Parse *pParse, صحيح iReg){
  ܐܢ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ܐܢ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحيح sqlite3FirstAvailableRegister(Parse *pParse, صحيح iMin){
  ثابت ExprList *pList = pParse->pConstExpr;
  ܐܢ( pList ){
    صحيح i;
    لكل(i=0; i<pList->nExpr; i++){
      ܐܢ( pList->a[i].u.iConstExprReg>=iMin ){
        iMin = pList->a[i].u.iConstExprReg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  ܦܢܐ iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
صحيح sqlite3NoTempsInRange(Parse *pParse, صحيح iFirst, صحيح iLast){
  صحيح i;
  ܐܢ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     ܦܢܐ 0;
  }
  لكل(i=0; i<pParse->nTempReg; i++){
    ܐܢ( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      ܦܢܐ 0;
    }
  }
  ܐܢ( pParse->pConstExpr ){
    ExprList *pList = pParse->pConstExpr;
    لكل(i=0; i<pList->nExpr; i++){
      صحيح iReg = pList->a[i].u.iConstExprReg;
      ܐܢ( iReg==0 ) استمر;
      ܐܢ( iReg>=iFirst && iReg<=iLast ) ܦܢܐ 0;
    }
  }
  ܦܢܐ 1;
}
#endif /* SQLITE_DEBUG */
