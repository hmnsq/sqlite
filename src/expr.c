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
#include "sqliteInt.h"

/* Forward declarations */
static void exprCodeBetween(Parse*,Expr*,ціле,void(*)(Parse*,Expr*,ціле,ціле),ціле);
static ціле exprCodeVector(Parse *pParse, Expr *p, ціле *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
char sqlite3TableColumnAffinity(const Table *pTab, ціле iCol){
  якщо( iCol<0 || NEVER(iCol>=pTab->nCol) ) повернути SQLITE_AFF_INTEGER;
  повернути pTab->aCol[iCol].affinity;
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
char sqlite3ExprAffinity(const Expr *pExpr){
  ціле op;
  op = pExpr->op;
  поки( 1 /* exit-by-break */ ){
    якщо( op==TK_COLUMN || (op==TK_AGG_COLUMN && pExpr->y.pTab!=0) ){
      assert( ExprUseYTab(pExpr) );
      assert( pExpr->y.pTab!=0 );
      повернути sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
    }
    якщо( op==TK_SELECT ){
      assert( ExprUseXSelect(pExpr) );
      assert( pExpr->x.pSelect!=0 );
      assert( pExpr->x.pSelect->pEList!=0 );
      assert( pExpr->x.pSelect->pEList->a[0].pExpr!=0 );
      повернути sqlite3ExprAffinity(pExpr->x.pSelect->pEList->a[0].pExpr);
    }
#ifndef SQLITE_OMIT_CAST
    якщо( op==TK_CAST ){
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      повернути sqlite3AffinityType(pExpr->u.zToken, 0);
    }
#endif
    якщо( op==TK_SELECT_COLUMN ){
      assert( pExpr->pLeft!=0 && ExprUseXSelect(pExpr->pLeft) );
      assert( pExpr->iColumn < pExpr->iTable );
      assert( pExpr->iColumn >= 0 );
      assert( pExpr->iTable==pExpr->pLeft->x.pSelect->pEList->nExpr );
      повернути sqlite3ExprAffinity(
          pExpr->pLeft->x.pSelect->pEList->a[pExpr->iColumn].pExpr
      );
    }
    якщо( op==TK_VECTOR
     || (op==TK_FUNCTION && pExpr->affExpr==SQLITE_AFF_DEFER)
    ){
      assert( ExprUseXList(pExpr) );
      повернути sqlite3ExprAffinity(pExpr->x.pList->a[0].pExpr);
    }
    якщо( ExprHasProperty(pExpr, EP_Skip|EP_IfNullRow) ){
      assert( pExpr->op==TK_COLLATE
           || pExpr->op==TK_IF_NULL_ROW
           || (pExpr->op==TK_REGISTER && pExpr->op2==TK_IF_NULL_ROW) );
      pExpr = pExpr->pLeft;
      op = pExpr->op;
      continue;
    }
    якщо( op!=TK_REGISTER ) break;
    op = pExpr->op2;
    якщо( NEVER( op==TK_REGISTER ) ) break;
  }
  повернути pExpr->affExpr;
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
ціле sqlite3ExprDataType(const Expr *pExpr){
  поки( pExpr ){
    switch( pExpr->op ){
      case TK_COLLATE:
      case TK_IF_NULL_ROW:
      case TK_UPLUS:  {
        pExpr = pExpr->pLeft;
        break;
      }
      case TK_NULL: {
        pExpr = 0;
        break;
      }
      case TK_STRING: {
        повернути 0x02;
      }
      case TK_BLOB: {
        повернути 0x04;
      }
      case TK_CONCAT: {
        повернути 0x06;
      }
      case TK_VARIABLE:
      case TK_AGG_FUNCTION:
      case TK_FUNCTION: {
        повернути 0x07;
      }
      case TK_COLUMN:
      case TK_AGG_COLUMN:
      case TK_SELECT:
      case TK_CAST:
      case TK_SELECT_COLUMN:
      case TK_VECTOR:  {
        ціле aff = sqlite3ExprAffinity(pExpr);
        якщо( aff>=SQLITE_AFF_NUMERIC ) повернути 0x05;
        якщо( aff==SQLITE_AFF_TEXT )    повернути 0x06;
        повернути 0x07;
      }
      case TK_CASE: {
        ціле res = 0;
        ціле ii;
        ExprList *pList = pExpr->x.pList;
        assert( ExprUseXList(pExpr) && pList!=0 );
        assert( pList->nExpr > 0);
        для(ii=1; ii<pList->nExpr; ii+=2){
          res |= sqlite3ExprDataType(pList->a[ii].pExpr);
        }
        якщо( pList->nExpr % 2 ){
          res |= sqlite3ExprDataType(pList->a[pList->nExpr-1].pExpr);
        }
        повернути res;
      }
      default: {
        повернути 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  повернути 0x00;
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
  const Token *pCollName,  /* Name of collating sequence */
  ціле dequote              /* True to dequote pCollName */
){
  якщо( pCollName->n>0 && pParse->nErr==0 ){
    Expr *pNew = sqlite3ExprAlloc(pParse->db, TK_COLLATE, pCollName, dequote);
    assert( pExpr!=0 );
    якщо( pNew ){
      pNew->pLeft = pExpr;
      pNew->flags |= EP_Collate|EP_Skip;
#якщо SQLITE_MAX_EXPR_DEPTH>0
      pNew->nHeight = pExpr->nHeight+1;
      sqlite3ExprCheckHeight(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      pExpr = pNew;
    }
  }
  повернути pExpr;
}
Expr *sqlite3ExprAddCollateString(
  Parse *pParse,        /* Parsing context */
  Expr *pExpr,          /* Add the "COLLATE" clause to this expression */
  const char *zC        /* The collating sequence name */
){
  Token s;
  assert( zC!=0 );
  sqlite3TokenInit(&s, (char*)zC);
  повернути sqlite3ExprAddCollateToken(pParse, pExpr, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
Expr *sqlite3ExprSkipCollate(Expr *pExpr){
  поки( pExpr && ExprHasProperty(pExpr, EP_Skip) ){
    assert( pExpr->op==TK_COLLATE );
    pExpr = pExpr->pLeft;
  }  
  повернути pExpr;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
Expr *sqlite3ExprSkipCollateAndLikely(Expr *pExpr){
  поки( pExpr && ExprHasProperty(pExpr, EP_Skip|EP_Unlikely) ){
    якщо( ExprHasProperty(pExpr, EP_Unlikely) ){
      assert( ExprUseXList(pExpr) );
      assert( pExpr->x.pList->nExpr>0 );
      assert( pExpr->op==TK_FUNCTION );
      pExpr = pExpr->x.pList->a[0].pExpr;
    }інакше якщо( pExpr->op==TK_COLLATE ){
      pExpr = pExpr->pLeft;
    }інакше{
      break;
    }
  }  
  повернути pExpr;
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
CollSeq *sqlite3ExprCollSeq(Parse *pParse, const Expr *pExpr){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  const Expr *p = pExpr;
  поки( p ){
    ціле op = p->op;
    якщо( op==TK_REGISTER ) op = p->op2;
    якщо( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      ціле j;
      assert( ExprUseYTab(p) );
      assert( p->y.pTab!=0 );
      якщо( (j = p->iColumn)>=0 ){
        const char *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      break;
    }
    якщо( op==TK_CAST || op==TK_UPLUS ){
      p = p->pLeft;
      continue;
    }
    якщо( op==TK_VECTOR
     || (op==TK_FUNCTION && p->affExpr==SQLITE_AFF_DEFER)
    ){
      assert( ExprUseXList(p) );
      p = p->x.pList->a[0].pExpr;
      continue;
    }
    якщо( op==TK_COLLATE ){
      assert( !ExprHasProperty(p, EP_IntValue) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      break;
    }
    якщо( p->flags & EP_Collate ){
      якщо( p->pLeft && (p->pLeft->flags & EP_Collate)!=0 ){
        p = p->pLeft;
      }інакше{
        Expr *pNext  = p->pRight;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        assert( !ExprUseXList(p) || p->x.pList==0 || p->pRight==0 );
        якщо( ExprUseXList(p) && p->x.pList!=0 && !db->mallocFailed ){
          ціле i;
          для(i=0; i<p->x.pList->nExpr; i++){
            якщо( ExprHasProperty(p->x.pList->a[i].pExpr, EP_Collate) ){
              pNext = p->x.pList->a[i].pExpr;
              break;
            }
          }
        }
        p = pNext;
      }
    }інакше{
      break;
    }
  }
  якщо( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  повернути pColl;
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
CollSeq *sqlite3ExprNNCollSeq(Parse *pParse, const Expr *pExpr){
  CollSeq *p = sqlite3ExprCollSeq(pParse, pExpr);
  якщо( p==0 ) p = pParse->db->pDfltColl;
  assert( p!=0 );
  повернути p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
ціле sqlite3ExprCollSeqMatch(Parse *pParse, const Expr *pE1, const Expr *pE2){
  CollSeq *pColl1 = sqlite3ExprNNCollSeq(pParse, pE1);
  CollSeq *pColl2 = sqlite3ExprNNCollSeq(pParse, pE2);
  assert( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  повернути pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
char sqlite3CompareAffinity(const Expr *pExpr, char aff2){
  char aff1 = sqlite3ExprAffinity(pExpr);
  якщо( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    якщо( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      повернути SQLITE_AFF_NUMERIC;
    }інакше{
      повернути SQLITE_AFF_BLOB;
    }
  }інакше{
    /* One side is a column, the other is not. Use the columns affinity. */
    assert( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    повернути (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
static char comparisonAffinity(const Expr *pExpr){
  char aff;
  assert( pExpr->op==TK_EQ || pExpr->op==TK_IN || pExpr->op==TK_LT ||
          pExpr->op==TK_GT || pExpr->op==TK_GE || pExpr->op==TK_LE ||
          pExpr->op==TK_NE || pExpr->op==TK_IS || pExpr->op==TK_ISNOT );
  assert( pExpr->pLeft );
  assert( !ExprUseXSelect(pExpr) );
  aff = sqlite3ExprAffinity(pExpr->pLeft);
  якщо( pExpr->pRight ){
    aff = sqlite3CompareAffinity(pExpr->pRight, aff);
  }інакше якщо( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  повернути aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
ціле sqlite3IndexAffinityOk(const Expr *pExpr, char idx_affinity){
  char aff = comparisonAffinity(pExpr);
  якщо( aff<SQLITE_AFF_TEXT ){
    повернути 1;
  }
  якщо( aff==SQLITE_AFF_TEXT ){
    повернути idx_affinity==SQLITE_AFF_TEXT;
  }
  повернути sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
static u8 binaryCompareP5(
  const Expr *pExpr1,   /* Left operand */
  const Expr *pExpr2,   /* Right operand */
  ціле jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (char)sqlite3ExprAffinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  повернути aff;
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
  const Expr *pLeft,
  const Expr *pRight
){
  CollSeq *pColl;
  assert( pLeft );
  якщо( pLeft->flags & EP_Collate ){
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
  }інакше якщо( pRight && (pRight->flags & EP_Collate)!=0 ){
    pColl = sqlite3ExprCollSeq(pParse, pRight);
  }інакше{
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
    якщо( !pColl ){
      pColl = sqlite3ExprCollSeq(pParse, pRight);
    }
  }
  повернути pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3ExprCompareCollSeq(Parse *pParse, const Expr *p){
  якщо( ExprHasProperty(p, EP_Commuted) ){
    повернути sqlite3BinaryCompareCollSeq(pParse, p->pRight, p->pLeft);
  }інакше{
    повернути sqlite3BinaryCompareCollSeq(pParse, p->pLeft, p->pRight);
  }
}

/*
** Generate code for a comparison operator.
*/
static ціле codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  Expr *pLeft,      /* The left operand */
  Expr *pRight,     /* The right operand */
  ціле opcode,       /* The comparison opcode */
  ціле in1, ціле in2, /* Register holding operands */
  ціле dest,         /* Jump here if true.  */
  ціле jumpIfNull,   /* If true, jump if either operand is NULL */
  ціле isCommuted    /* The comparison has been commuted */
){
  ціле p5;
  ціле addr;
  CollSeq *p4;

  якщо( pParse->nErr ) повернути 0;
  якщо( isCommuted ){
    p4 = sqlite3BinaryCompareCollSeq(pParse, pRight, pLeft);
  }інакше{
    p4 = sqlite3BinaryCompareCollSeq(pParse, pLeft, pRight);
  }
  p5 = binaryCompareP5(pLeft, pRight, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (void*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  повернути addr;
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
ціле sqlite3ExprIsVector(const Expr *pExpr){
  повернути sqlite3ExprVectorSize(pExpr)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
ціле sqlite3ExprVectorSize(const Expr *pExpr){
  u8 op = pExpr->op;
  якщо( op==TK_REGISTER ) op = pExpr->op2;
  якщо( op==TK_VECTOR ){
    assert( ExprUseXList(pExpr) );
    повернути pExpr->x.pList->nExpr;
  }інакше якщо( op==TK_SELECT ){
    assert( ExprUseXSelect(pExpr) );
    повернути pExpr->x.pSelect->pEList->nExpr;
  }інакше{
    повернути 1;
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
Expr *sqlite3VectorFieldSubexpr(Expr *pVector, ціле i){
  assert( i<sqlite3ExprVectorSize(pVector) || pVector->op==TK_ERROR );
  якщо( sqlite3ExprIsVector(pVector) ){
    assert( pVector->op2==0 || pVector->op==TK_REGISTER );
    якщо( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      assert( ExprUseXSelect(pVector) );
      повернути pVector->x.pSelect->pEList->a[i].pExpr;
    }інакше{
      assert( ExprUseXList(pVector) );
      повернути pVector->x.pList->a[i].pExpr;
    }
  }
  повернути pVector;
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
  ціле iField,          /* Which column of the vector to return */
  ціле nField           /* Total number of columns in the vector */
){
  Expr *pRet;
  якщо( pVector->op==TK_SELECT ){
    assert( ExprUseXSelect(pVector) );
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
    якщо( pRet ){
      ExprSetProperty(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->pLeft = pVector;
    }
  }інакше{
    якщо( pVector->op==TK_VECTOR ){
      Expr **ppVector;
      assert( ExprUseXList(pVector) );
      ppVector = &pVector->x.pList->a[iField].pExpr;
      pVector = *ppVector;
      якщо( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        повернути pVector;
      }
    }
    pRet = sqlite3ExprDup(pParse->db, pVector, 0);
  }
  повернути pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
static ціле exprCodeSubselect(Parse *pParse, Expr *pExpr){
  ціле reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  якщо( pExpr->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, pExpr);
  }
#endif
  повернути reg;
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
static ціле exprVectorRegister(
  Parse *pParse,                  /* Parse context */
  Expr *pVector,                  /* Vector to extract element from */
  ціле iField,                     /* Field to extract from pVector */
  ціле regSelect,                  /* First in array of registers */
  Expr *pTmp,                     /* Temporary space */
  Expr **ppExpr,                  /* OUT: Expression element */
  ціле *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  assert( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  якщо( op==TK_REGISTER ){
    *ppExpr = sqlite3VectorFieldSubexpr(pVector, iField);
    повернути pVector->iTable+iField;
  }
  якщо( op==TK_SELECT ){
    assert( ExprUseXSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, sizeof(Expr));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->pLeft = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->nExpr;
    *ppExpr = pTmp;
    повернути regSelect+iField;
  }
  якщо( op==TK_VECTOR ){
    assert( ExprUseXList(pVector) );
    *ppExpr = pVector->x.pList->a[iField].pExpr;
    повернути sqlite3ExprCodeTemp(pParse, *ppExpr, pRegFree);
  }
  повернути 0;
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
static void codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  Expr *pExpr,          /* The comparison operation */
  ціле dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  Expr *pLeft = pExpr->pLeft;
  Expr *pRight = pExpr->pRight;
  ціле nLeft = sqlite3ExprVectorSize(pLeft);
  ціле i;
  ціле regLeft = 0;
  ціле regRight = 0;
  u8 opx = op;
  ціле addrCmp = 0;
  ціле addrDone = sqlite3VdbeMakeLabel(pParse);
  ціле isCommuted = ExprHasProperty(pExpr,EP_Commuted);

  assert( !ExprHasVVAProperty(pExpr,EP_Immutable) );
  якщо( pParse->nErr ) повернути;
  якщо( nLeft!=sqlite3ExprVectorSize(pRight) ){
    sqlite3ErrorMsg(pParse, "row value misused");
    повернути;
  }
  assert( pExpr->op==TK_EQ || pExpr->op==TK_NE
       || pExpr->op==TK_IS || pExpr->op==TK_ISNOT
       || pExpr->op==TK_LT || pExpr->op==TK_GT
       || pExpr->op==TK_LE || pExpr->op==TK_GE
  );
  assert( pExpr->op==op || (pExpr->op==TK_IS && op==TK_EQ)
            || (pExpr->op==TK_ISNOT && op==TK_NE) );
  assert( p5==0 || pExpr->op!=op );
  assert( p5==SQLITE_NULLEQ || pExpr->op==op );

  якщо( op==TK_LE ) opx = TK_LT;
  якщо( op==TK_GE ) opx = TK_GT;
  якщо( op==TK_NE ) opx = TK_EQ;

  regLeft = exprCodeSubselect(pParse, pLeft);
  regRight = exprCodeSubselect(pParse, pRight);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  для(i=0; 1 /*Loop exits by "break"*/; i++){
    ціле regFree1 = 0, regFree2 = 0;
    Expr *pL = 0, *pR = 0;
    Expr tmp1, tmp2;
    ціле r1, r2;
    assert( i>=0 && i<nLeft );
    якщо( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
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
    якщо( (opx==TK_LT || opx==TK_GT) && i<nLeft-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    якщо( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }інакше{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    якщо( i==nLeft-1 ){
      break;
    }
    якщо( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }інакше{
      assert( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      якщо( i==nLeft-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  якщо( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#якщо SQLITE_MAX_EXPR_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
ціле sqlite3ExprCheckHeight(Parse *pParse, ціле nHeight){
  ціле rc = SQLITE_OK;
  ціле mxHeight = pParse->db->aLimit[SQLITE_LIMIT_EXPR_DEPTH];
  якщо( nHeight>mxHeight ){
    sqlite3ErrorMsg(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  повернути rc;
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
static void heightOfExpr(const Expr *p, ціле *pnHeight){
  якщо( p ){
    якщо( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
static void heightOfExprList(const ExprList *p, ціле *pnHeight){
  якщо( p ){
    ціле i;
    для(i=0; i<p->nExpr; i++){
      heightOfExpr(p->a[i].pExpr, pnHeight);
    }
  }
}
static void heightOfSelect(const Select *pSelect, ціле *pnHeight){
  const Select *p;
  для(p=pSelect; p; p=p->pPrior){
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
static void exprSetHeight(Expr *p){
  ціле nHeight = p->pLeft ? p->pLeft->nHeight : 0;
  якщо( NEVER(p->pRight) && p->pRight->nHeight>nHeight ){
    nHeight = p->pRight->nHeight;
  }
  якщо( ExprUseXSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }інакше якщо( p->x.pList ){
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
void sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p){
  якщо( pParse->nErr ) повернути;
  exprSetHeight(p);
  sqlite3ExprCheckHeight(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
ціле sqlite3SelectExprHeight(const Select *p){
  ціле nHeight = 0;
  heightOfSelect(p, &nHeight);
  повернути nHeight;
}
#інакше /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
void sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p){
  якщо( pParse->nErr ) повернути;
  якщо( p && ExprUseXList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3ExprListFlags(p->x.pList);
  }
}
#define exprSetHeight(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
void sqlite3ExprSetErrorOffset(Expr *pExpr, ціле iOfst){
  якщо( pExpr==0 ) повернути;
  якщо( NEVER(ExprUseWJoin(pExpr)) ) повернути;
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
  ціле op,                 /* Expression opcode */
  const Token *pToken,    /* Token argument.  Might be NULL */
  ціле dequote             /* True to dequote */
){
  Expr *pNew;
  ціле nExtra = pToken ? pToken->n+1 : 0;

  assert( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, sizeof(Expr)+nExtra);
  якщо( pNew ){
    memset(pNew, 0, sizeof(Expr));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    якщо( nExtra ){
      assert( pToken!=0 );
      pNew->u.zToken = (char*)&pNew[1];
      assert( pToken->z!=0 || pToken->n==0 );
      якщо( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      якщо( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3DequoteExpr(pNew);
      }
    }
#якщо SQLITE_MAX_EXPR_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  повернути pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
Expr *sqlite3Expr(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  ціле op,                 /* Expression opcode */
  const char *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  повернути sqlite3ExprAlloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
Expr *sqlite3ExprInt32(sqlite3 *db, ціле iVal){
  Expr *pNew = sqlite3DbMallocRawNN(db, sizeof(Expr));
  якщо( pNew ){
    memset(pNew, 0, sizeof(Expr));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_IntValue|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.iValue = iVal;
#якщо SQLITE_MAX_EXPR_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  повернути pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
void sqlite3ExprAttachSubtrees(
  sqlite3 *db,
  Expr *pRoot,
  Expr *pLeft,
  Expr *pRight
){
  якщо( pRoot==0 ){
    assert( db->mallocFailed );
    sqlite3ExprDelete(db, pLeft);
    sqlite3ExprDelete(db, pRight);
  }інакше{
    assert( ExprUseXList(pRoot) );
    assert( pRoot->x.pSelect==0 );
    якщо( pRight ){
      pRoot->pRight = pRight;
      pRoot->flags |= EP_Propagate & pRight->flags;
#якщо SQLITE_MAX_EXPR_DEPTH>0
      pRoot->nHeight = pRight->nHeight+1;
    }інакше{
      pRoot->nHeight = 1;
#endif
    }
    якщо( pLeft ){
      pRoot->pLeft = pLeft;
      pRoot->flags |= EP_Propagate & pLeft->flags;
#якщо SQLITE_MAX_EXPR_DEPTH>0
      якщо( pLeft->nHeight>=pRoot->nHeight ){
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
  ціле op,                 /* Expression opcode */
  Expr *pLeft,            /* Left operand */
  Expr *pRight            /* Right operand */
){
  Expr *p;
  p = sqlite3DbMallocRawNN(pParse->db, sizeof(Expr));
  якщо( p ){
    memset(p, 0, sizeof(Expr));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3ExprAttachSubtrees(pParse->db, p, pLeft, pRight);
    sqlite3ExprCheckHeight(pParse, p->nHeight);
  }інакше{
    sqlite3ExprDelete(pParse->db, pLeft);
    sqlite3ExprDelete(pParse->db, pRight);
  }
  повернути p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
void sqlite3PExprAddSelect(Parse *pParse, Expr *pExpr, Select *pSelect){
  якщо( pExpr ){
    pExpr->x.pSelect = pSelect;
    ExprSetProperty(pExpr, EP_xIsSelect|EP_Subquery);
    sqlite3ExprSetHeightAndFlags(pParse, pExpr);
  }інакше{
    assert( pParse->db->mallocFailed );
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
Select *sqlite3ExprListToValues(Parse *pParse, ціле nElem, ExprList *pEList){
  ціле ii;
  Select *pRet = 0;
  assert( nElem>1 );
  для(ii=0; ii<pEList->nExpr; ii++){
    Select *pSel;
    Expr *pExpr = pEList->a[ii].pExpr;
    ціле nExprElem;
    якщо( pExpr->op==TK_VECTOR ){
      assert( ExprUseXList(pExpr) );
      nExprElem = pExpr->x.pList->nExpr;
    }інакше{
      nExprElem = 1;
    }
    якщо( nExprElem!=nElem ){
      sqlite3ErrorMsg(pParse, "IN(...) element has %d term%s - expected %d",
          nExprElem, nExprElem>1?"s":"", nElem
      );
      break;
    }
    assert( ExprUseXList(pExpr) );
    pSel = sqlite3SelectNew(pParse, pExpr->x.pList, 0, 0, 0, 0, 0, SF_Values,0);
    pExpr->x.pList = 0;
    якщо( pSel ){
      якщо( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  якщо( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_MultiValue;
  }
  sqlite3ExprListDelete(pParse->db, pEList);
  повернути pRet;
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
  якщо( pLeft==0  ){
    повернути pRight;
  }інакше якщо( pRight==0 ){
    повернути pLeft;
  }інакше{
    u32 f = pLeft->flags | pRight->flags;
    якщо( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      Expr *p;
      sqlite3ExprDeferredDelete(pParse, pLeft);
      sqlite3ExprDeferredDelete(pParse, pRight);
      p = sqlite3ExprInt32(db, 0);
      якщо( p ) p->op2 = 1;
      повернути p;
    }інакше{
      повернути sqlite3PExpr(pParse, TK_AND, pLeft, pRight);
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
  const Token *pToken,  /* Name of the function */
  ціле eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  Expr *pNew;
  sqlite3 *db = pParse->db;
  assert( pToken );
  pNew = sqlite3ExprAlloc(db, TK_FUNCTION, pToken, 1);
  якщо( pNew==0 ){
    sqlite3ExprListDelete(db, pList); /* Avoid memory leak when malloc fails */
    повернути 0;
  }
  assert( !ExprHasProperty(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (ціле)(pToken->z - pParse->zTail);
  якщо( pList
   && pList->nExpr > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ExprSetProperty(pNew, EP_HasFunc);
  assert( ExprUseXList(pNew) );
  sqlite3ExprSetHeightAndFlags(pParse, pNew);
  якщо( eDistinct==SF_Distinct ) ExprSetProperty(pNew, EP_Distinct);
  повернути pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
void sqlite3ExprOrderByAggregateError(Parse *pParse, Expr *p){
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
void sqlite3ExprAddFunctionOrderBy(
  Parse *pParse,        /* Parsing context */
  Expr *pExpr,          /* The function call to which ORDER BY is to be added */
  ExprList *pOrderBy    /* The ORDER BY clause to add */
){
  Expr *pOB;
  sqlite3 *db = pParse->db;
  якщо( NEVER(pOrderBy==0) ){
    assert( db->mallocFailed );
    повернути;
  }
  якщо( pExpr==0 ){
    assert( db->mallocFailed );
    sqlite3ExprListDelete(db, pOrderBy);
    повернути;
  }
  assert( pExpr->op==TK_FUNCTION );
  assert( pExpr->pLeft==0 );
  assert( ExprUseXList(pExpr) );
  якщо( pExpr->x.pList==0 || NEVER(pExpr->x.pList->nExpr==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, pOrderBy);
    повернути;
  }
  якщо( IsWindowFunc(pExpr) ){
    sqlite3ExprOrderByAggregateError(pParse, pExpr);
    sqlite3ExprListDelete(db, pOrderBy);
    повернути;
  }
  якщо( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    sqlite3ExprListDelete(db, pOrderBy);
    повернути;
  }

  pOB = sqlite3ExprAlloc(db, TK_ORDER, 0, 0);
  якщо( pOB==0 ){
    sqlite3ExprListDelete(db, pOrderBy);
    повернути;
  }
  pOB->x.pList = pOrderBy;
  assert( ExprUseXList(pOB) );
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
void sqlite3ExprFunctionUsable(
  Parse *pParse,         /* Parsing and code generating context */
  const Expr *pExpr,     /* The function invocation */
  const FuncDef *pDef    /* The function being invoked */
){
  assert( !IN_RENAME_OBJECT );
  assert( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  якщо( ExprHasProperty(pExpr, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    якщо( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
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
void sqlite3ExprAssignVarNumber(Parse *pParse, Expr *pExpr, u32 n){
  sqlite3 *db = pParse->db;
  const char *z;
  ynVar x;

  якщо( pExpr==0 ) повернути;
  assert( !ExprHasProperty(pExpr, EP_IntValue|EP_Reduced|EP_TokenOnly) );
  z = pExpr->u.zToken;
  assert( z!=0 );
  assert( z[0]!=0 );
  assert( n==(u32)sqlite3Strlen30(z) );
  якщо( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    assert( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }інакше{
    ціле doAdd = 0;
    якщо( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      ціле bOk;
      якщо( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }інакше{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] );
      якщо( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
        sqlite3ErrorMsg(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]);
        sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
        повернути;
      }
      x = (ynVar)i;
      якщо( x>pParse->nVar ){
        pParse->nVar = (ціле)x;
        doAdd = 1;
      }інакше якщо( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }інакше якщо( sqlite3VListNumToName(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }інакше{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3VListNameToNum(pParse->pVList, z, n);
      якщо( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    якщо( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      якщо( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  pExpr->iColumn = x;
  якщо( x>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
    sqlite3ErrorMsg(pParse, "too many SQL variables");
    sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
  }
}

/*
** Recursively delete an expression tree.
*/
static SQLITE_NOINLINE void sqlite3ExprDeleteNN(sqlite3 *db, Expr *p){
  assert( p!=0 );
  assert( db!=0 );
exprDeleteRestart:
  assert( !ExprUseUValue(p) || p->u.iValue>=0 );
  assert( !ExprUseYWin(p) || !ExprUseYSub(p) );
  assert( !ExprUseYWin(p) || p->y.pWin!=0 || db->mallocFailed );
  assert( p->op!=TK_FUNCTION || !ExprUseYSub(p) );
#ifdef SQLITE_DEBUG
  якщо( ExprHasProperty(p, EP_Leaf) && !ExprHasProperty(p, EP_TokenOnly) ){
    assert( p->pLeft==0 );
    assert( p->pRight==0 );
    assert( !ExprUseXSelect(p) || p->x.pSelect==0 );
    assert( !ExprUseXList(p) || p->x.pList==0 );
  }
#endif
  якщо( !ExprHasProperty(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    assert( (ExprUseXList(p) && p->x.pList==0) || p->pRight==0 );
    якщо( p->pRight ){
      assert( !ExprHasProperty(p, EP_WinFunc) );
      sqlite3ExprDeleteNN(db, p->pRight);
    }інакше якщо( ExprUseXSelect(p) ){
      assert( !ExprHasProperty(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }інакше{
      sqlite3ExprListDelete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      якщо( ExprHasProperty(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    якщо( p->pLeft && p->op!=TK_SELECT_COLUMN ){
      Expr *pLeft = p->pLeft;
      якщо( !ExprHasProperty(p, EP_Static)
       && !ExprHasProperty(pLeft, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = pLeft;
        goto exprDeleteRestart;
      }інакше{
        sqlite3ExprDeleteNN(db, pLeft);
      }
    }
  }
  якщо( !ExprHasProperty(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
void sqlite3ExprDelete(sqlite3 *db, Expr *p){
  якщо( p ) sqlite3ExprDeleteNN(db, p);
}
void sqlite3ExprDeleteGeneric(sqlite3 *db, void *p){
  якщо( ALWAYS(p) ) sqlite3ExprDeleteNN(db, (Expr*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
void sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  якщо( p==0 ){
    /* Nothing to clear */
  }інакше якщо( p->pOn ){
    sqlite3ExprDeleteNN(db, p->pOn);
  }інакше якщо( p->pUsing ){
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
ціле sqlite3ExprDeferredDelete(Parse *pParse, Expr *pExpr){
  повернути 0==sqlite3ParserAddCleanup(pParse, sqlite3ExprDeleteGeneric, pExpr);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
void sqlite3ExprUnmapAndDelete(Parse *pParse, Expr *p){
  якщо( p ){
    якщо( IN_RENAME_OBJECT ){
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
static ціле exprStructSize(const Expr *p){
  якщо( ExprHasProperty(p, EP_TokenOnly) ) повернути EXPR_TOKENONLYSIZE;
  якщо( ExprHasProperty(p, EP_Reduced) ) повернути EXPR_REDUCEDSIZE;
  повернути EXPR_FULLSIZE;
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
static ціле dupedExprStructSize(const Expr *p, ціле flags){
  ціле nSize;
  assert( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  assert( EXPR_FULLSIZE<=0xfff );
  assert( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  якщо( 0==flags || ExprHasProperty(p, EP_FullSize) ){
    nSize = EXPR_FULLSIZE;
  }інакше{
    assert( !ExprHasProperty(p, EP_TokenOnly|EP_Reduced) );
    assert( !ExprHasProperty(p, EP_OuterON) );
    assert( !ExprHasVVAProperty(p, EP_NoReduce) );
    якщо( p->pLeft || p->x.pList ){
      nSize = EXPR_REDUCEDSIZE | EP_Reduced;
    }інакше{
      assert( p->pRight==0 );
      nSize = EXPR_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  повернути nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
static ціле dupedExprNodeSize(const Expr *p, ціле flags){
  ціле nByte = dupedExprStructSize(p, flags) & 0xfff;
  якщо( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  повернути ROUND8(nByte);
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
static ціле dupedExprSize(const Expr *p){
  ціле nByte;
  assert( p!=0 );
  nByte = dupedExprNodeSize(p, EXPRDUP_REDUCE);
  якщо( p->pLeft ) nByte += dupedExprSize(p->pLeft);
  якщо( p->pRight ) nByte += dupedExprSize(p->pRight);
  assert( nByte==ROUND8(nByte) );
  повернути nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
typedef struct EdupBuf EdupBuf;
struct EdupBuf {
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
static Expr *exprDup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  const Expr *p,        /* Expr tree to be duplicated */
  ціле dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  Expr *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  ціле nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  assert( db!=0 );
  assert( p );
  assert( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  assert( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  якщо( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    assert( sEdupBuf.zAlloc!=0 );
    assert( dupFlags==EXPRDUP_REDUCE );
  }інакше{
    ціле nAlloc;
    якщо( dupFlags ){
      nAlloc = dupedExprSize(p);
    }інакше якщо( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(EXPR_FULLSIZE + nToken);
    }інакше{
      nToken = 0;
      nAlloc = ROUND8(EXPR_FULLSIZE);
    }
    assert( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (Expr *)sEdupBuf.zAlloc;
  assert( EIGHT_BYTE_ALIGNMENT(pNew) );

  якщо( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    const unsigned nStructSize = dupedExprStructSize(p, dupFlags);
    ціле nNewSize = nStructSize & 0xfff;
    якщо( nToken<0 ){
      якщо( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }інакше{
        nToken = 0;
      }
    }
    якщо( dupFlags ){
      assert( (ціле)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      assert( ExprHasProperty(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }інакше{
      u32 nSize = (u32)exprStructSize(p);
      assert( (ціле)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (ціле)EXPR_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      якщо( nSize<EXPR_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, EXPR_FULLSIZE-nSize);
      }
      nNewSize = EXPR_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ExprClearVVAProperties(pNew);
    якщо( dupFlags ){
      ExprSetVVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    assert( nToken>=0 );
    якщо( nToken>0 ){
      char *zToken = pNew->u.zToken = (char*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    якщо( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      якщо( ExprUseXSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }інакше{
        pNew->x.pList = sqlite3ExprListDup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      якщо( ExprHasProperty(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        assert( ExprHasProperty(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      якщо( dupFlags ){
        якщо( p->op==TK_SELECT_COLUMN ){
          pNew->pLeft = p->pLeft;
          assert( p->pRight==0 
               || p->pRight==p->pLeft
               || ExprHasProperty(p->pLeft, EP_Subquery) );
        }інакше{
          pNew->pLeft = p->pLeft ?
                      exprDup(db, p->pLeft, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->pRight = p->pRight ?
                       exprDup(db, p->pRight, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }інакше{
        якщо( p->op==TK_SELECT_COLUMN ){
          pNew->pLeft = p->pLeft;
          assert( p->pRight==0 
               || p->pRight==p->pLeft
               || ExprHasProperty(p->pLeft, EP_Subquery) );
        }інакше{
          pNew->pLeft = sqlite3ExprDup(db, p->pLeft, 0);
        }
        pNew->pRight = sqlite3ExprDup(db, p->pRight, 0);
      }
    }
  }
  якщо( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, sizeof(sEdupBuf));
  assert( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  повернути pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  якщо( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    якщо( pRet ){
      ціле i;
      pRet->nCte = p->nCte;
      для(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3ExprListDup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  повернути pRet;
}
#інакше
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
static ціле gatherSelectWindowsCallback(Walker *pWalker, Expr *pExpr){
  якщо( pExpr->op==TK_FUNCTION && ExprHasProperty(pExpr, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = pExpr->y.pWin;
    assert( pWin );
    assert( IsWindowFunc(pExpr) );
    assert( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  повернути WRC_Continue;
}
static ціле gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  повернути p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
static void gatherSelectWindows(Select *p){
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
Expr *sqlite3ExprDup(sqlite3 *db, const Expr *p, ціле flags){
  assert( flags==0 || flags==EXPRDUP_REDUCE );
  повернути p ? exprDup(db, p, flags, 0) : 0;
}
ExprList *sqlite3ExprListDup(sqlite3 *db, const ExprList *p, ціле flags){
  ExprList *pNew;
  struct ExprList_item *pItem;
  const struct ExprList_item *pOldItem;
  ціле i;
  Expr *pPriorSelectColOld = 0;
  Expr *pPriorSelectColNew = 0;
  assert( db!=0 );
  якщо( p==0 ) повернути 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  якщо( pNew==0 ) повернути 0;
  pNew->nExpr = p->nExpr;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  для(i=0; i<p->nExpr; i++, pItem++, pOldItem++){
    Expr *pOldExpr = pOldItem->pExpr;
    Expr *pNewExpr;
    pItem->pExpr = sqlite3ExprDup(db, pOldExpr, flags);
    якщо( pOldExpr
     && pOldExpr->op==TK_SELECT_COLUMN
     && (pNewExpr = pItem->pExpr)!=0
    ){
      якщо( pNewExpr->pRight ){
        pPriorSelectColOld = pOldExpr->pRight;
        pPriorSelectColNew = pNewExpr->pRight;
        pNewExpr->pLeft = pNewExpr->pRight;
      }інакше{
        якщо( pOldExpr->pLeft!=pPriorSelectColOld ){
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
  повернути pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#якщо !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, const SrcList *p, ціле flags){
  SrcList *pNew;
  ціле i;
  assert( db!=0 );
  якщо( p==0 ) повернути 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  якщо( pNew==0 ) повернути 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  для(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    const SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    якщо( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, sizeof(Subquery));
      якщо( pNewSubq==0 ){
        assert( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }інакше{
        memcpy(pNewSubq, pOldItem->u4.pSubq, sizeof(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        якщо( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }інакше якщо( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }інакше{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    якщо( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }інакше якщо( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3ExprListDup(db, pOldItem->u1.pFuncArg, flags);
    }інакше{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    якщо( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    якщо( pTab ){
      pTab->nTabRef++;
    }
    якщо( pOldItem->fg.isUsing ){
      assert( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }інакше{
      pNewItem->u3.pOn = sqlite3ExprDup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  повернути pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, const IdList *p){
  IdList *pNew;
  ціле i;
  assert( db!=0 );
  якщо( p==0 ) повернути 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  якщо( pNew==0 ) повернути 0;
  pNew->nId = p->nId;
  для(i=0; i<p->nId; i++){
    struct IdList_item *pNewItem = &pNew->a[i];
    const struct IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  повернути pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, const Select *pDup, ціле flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  const Select *p;

  assert( db!=0 );
  для(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, sizeof(*p) );
    якщо( pNew==0 ) break;
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
    якщо( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    якщо( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      break;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  повернути pRet;
}
#інакше
Select *sqlite3SelectDup(sqlite3 *db, const Select *p, ціле flags){
  assert( p==0 );
  повернути 0;
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
static const struct ExprList_item zeroItem = {0};
SQLITE_NOINLINE ExprList *sqlite3ExprListAppendNew(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  struct ExprList_item *pItem;
  ExprList *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  якщо( pList==0 ){
    sqlite3ExprDelete(db, pExpr);
    повернути 0;
  }
  pList->nAlloc = 4;
  pList->nExpr = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  повернути pList;
}
SQLITE_NOINLINE ExprList *sqlite3ExprListAppendGrow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ExprList *pList,        /* List to which to append. Might be NULL */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  struct ExprList_item *pItem;
  ExprList *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  якщо( pNew==0 ){
    sqlite3ExprListDelete(db, pList);
    sqlite3ExprDelete(db, pExpr);
    повернути 0;
  }інакше{
    pList = pNew;
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  повернути pList;
}
ExprList *sqlite3ExprListAppend(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to append. Might be NULL */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  struct ExprList_item *pItem;
  якщо( pList==0 ){
    повернути sqlite3ExprListAppendNew(pParse->db,pExpr);
  }
  якщо( pList->nAlloc<pList->nExpr+1 ){
    повернути sqlite3ExprListAppendGrow(pParse->db,pList,pExpr);
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  повернути pList;
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
  ціле n;
  ціле i;
  ціле iFirst = pList ? pList->nExpr : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  якщо( NEVER(pColumns==0) ) goto vector_append_error;
  якщо( pExpr==0 ) goto vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  якщо( pExpr->op!=TK_SELECT && pColumns->nId!=(n=sqlite3ExprVectorSize(pExpr)) ){
    sqlite3ErrorMsg(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    goto vector_append_error;
  }

  для(i=0; i<pColumns->nId; i++){
    Expr *pSubExpr = sqlite3ExprForVectorField(pParse, pExpr, i, pColumns->nId);
    assert( pSubExpr!=0 || db->mallocFailed );
    якщо( pSubExpr==0 ) continue;
    pList = sqlite3ExprListAppend(pParse, pList, pSubExpr);
    якщо( pList ){
      assert( pList->nExpr==iFirst+i+1 );
      pList->a[pList->nExpr-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  якщо( !db->mallocFailed && pExpr->op==TK_SELECT && ALWAYS(pList!=0) ){
    Expr *pFirst = pList->a[iFirst].pExpr;
    assert( pFirst!=0 );
    assert( pFirst->op==TK_SELECT_COLUMN );
    
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
  повернути pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
void sqlite3ExprListSetSortOrder(ExprList *p, ціле iSortOrder, ціле eNulls){
  struct ExprList_item *pItem;
  якщо( p==0 ) повернути;
  assert( p->nExpr>0 );

  assert( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  assert( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  assert( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->nExpr-1];
  assert( pItem->fg.bNulls==0 );
  якщо( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  якщо( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    якщо( iSortOrder!=eNulls ){
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
void sqlite3ExprListSetName(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  const Token *pName,     /* Name to be added */
  ціле dequote             /* True to cause the name to be dequoted */
){
  assert( pList!=0 || pParse->db->mallocFailed!=0 );
  assert( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  якщо( pList ){
    struct ExprList_item *pItem;
    assert( pList->nExpr>0 );
    pItem = &pList->a[pList->nExpr-1];
    assert( pItem->zEName==0 );
    assert( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    якщо( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      якщо( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (const void*)pItem->zEName, pName);
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
void sqlite3ExprListSetSpan(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  const char *zStart,     /* Start of the span */
  const char *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  assert( pList!=0 || db->mallocFailed!=0 );
  якщо( pList ){
    struct ExprList_item *pItem = &pList->a[pList->nExpr-1];
    assert( pList->nExpr>0 );
    якщо( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
void sqlite3ExprListCheckLength(
  Parse *pParse,
  ExprList *pEList,
  const char *zObject
){
  ціле mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->nExpr==mx );
  testcase( pEList && pEList->nExpr==mx+1 );
  якщо( pEList && pEList->nExpr>mx ){
    sqlite3ErrorMsg(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
static SQLITE_NOINLINE void exprListDeleteNN(sqlite3 *db, ExprList *pList){
  ціле i = pList->nExpr;
  struct ExprList_item *pItem =  pList->a;
  assert( pList->nExpr>0 );
  assert( db!=0 );
  do{
    sqlite3ExprDelete(db, pItem->pExpr);
    якщо( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }поки( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
void sqlite3ExprListDelete(sqlite3 *db, ExprList *pList){
  якщо( pList ) exprListDeleteNN(db, pList);
}
void sqlite3ExprListDeleteGeneric(sqlite3 *db, void *pList){
  якщо( ALWAYS(pList) ) exprListDeleteNN(db, (ExprList*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3ExprListFlags(const ExprList *pList){
  ціле i;
  u32 m = 0;
  assert( pList!=0 );
  для(i=0; i<pList->nExpr; i++){
     Expr *pExpr = pList->a[i].pExpr;
     assert( pExpr!=0 );
     m |= pExpr->flags;
  }
  повернути m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
ціле sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  повернути WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(const char *zIn){
  якщо( sqlite3StrICmp(zIn, "true")==0  ) повернути EP_IsTrue;
  якщо( sqlite3StrICmp(zIn, "false")==0 ) повернути EP_IsFalse;
  повернути 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
ціле sqlite3ExprIdToTrueFalse(Expr *pExpr){
  u32 v;
  assert( pExpr->op==TK_ID || pExpr->op==TK_STRING );
  якщо( !ExprHasProperty(pExpr, EP_Quoted|EP_IntValue)
   && (v = sqlite3IsTrueOrFalse(pExpr->u.zToken))!=0
  ){
    pExpr->op = TK_TRUEFALSE;
    ExprSetProperty(pExpr, v);
    повернути 1;
  }
  повернути 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
ціле sqlite3ExprTruthValue(const Expr *pExpr){
  pExpr = sqlite3ExprSkipCollateAndLikely((Expr*)pExpr);
  assert( pExpr->op==TK_TRUEFALSE );
  assert( !ExprHasProperty(pExpr, EP_IntValue) );
  assert( sqlite3StrICmp(pExpr->u.zToken,"true")==0
       || sqlite3StrICmp(pExpr->u.zToken,"false")==0 );
  повернути pExpr->u.zToken[4]==0;
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
  assert( pExpr!=0 );
  якщо( pExpr->op==TK_AND || pExpr->op==TK_OR ){
    Expr *pRight = sqlite3ExprSimplifiedAndOr(pExpr->pRight);
    Expr *pLeft = sqlite3ExprSimplifiedAndOr(pExpr->pLeft);
    якщо( ExprAlwaysTrue(pLeft) || ExprAlwaysFalse(pRight) ){
      pExpr = pExpr->op==TK_AND ? pRight : pLeft;
    }інакше якщо( ExprAlwaysTrue(pRight) || ExprAlwaysFalse(pLeft) ){
      pExpr = pExpr->op==TK_AND ? pLeft : pRight;
    }
  }
  повернути pExpr;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
static ціле exprEvalRhsFirst(Expr *pExpr){
  якщо( ExprHasProperty(pExpr->pLeft, EP_Subquery)
   && !ExprHasProperty(pExpr->pRight, EP_Subquery)
  ){
    повернути 1;
  }інакше{
    повернути 0;
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
static ціле exprComputeOperands(
  Parse *pParse,     /* Parsing context */
  Expr *pExpr,       /* The comparison expression */
  ціле *pR1,          /* OUT: Register holding the left operand */
  ціле *pR2,          /* OUT: Register holding the right operand */
  ціле *pFree1,       /* OUT: Temp register to free if not zero */
  ціле *pFree2        /* OUT: Another temp register to free if not zero */
){
  ціле addrIsNull;
  ціле r1, r2;
  Vdbe *v = pParse->pVdbe;

  assert( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  якщо( exprEvalRhsFirst(pExpr) && sqlite3ExprCanBeNull(pExpr->pRight) ){
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }інакше{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pFree1);
  якщо( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    якщо( ExprHasProperty(pExpr->pRight, EP_Subquery)
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
  повернути addrIsNull;
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
static SQLITE_NOINLINE ціле exprNodeIsConstantFunction(
  Walker *pWalker,
  Expr *pExpr
){
  ціле n;             /* Number of arguments */
  ExprList *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  assert( pExpr->op==TK_FUNCTION );
  якщо( ExprHasProperty(pExpr, EP_TokenOnly)
   || (pList = pExpr->x.pList)==0
  ){;
    n = 0;
  }інакше{
    n = pList->nExpr;
    sqlite3WalkExprList(pWalker, pList);
    якщо( pWalker->eCode==0 ) повернути WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ENC(db), 0);
  якщо( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ExprHasProperty(pExpr, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    повернути WRC_Abort;
  }
  повернути WRC_Prune;
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
static ціле exprNodeIsConstant(Walker *pWalker, Expr *pExpr){
  assert( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  якщо( pWalker->eCode==2 && ExprHasProperty(pExpr, EP_OuterON) ){
    pWalker->eCode = 0;
    повернути WRC_Abort;
  }

  switch( pExpr->op ){
    case TK_FUNCTION:
      якщо( ExprHasProperty(pExpr, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        повернути WRC_Abort;
      }інакше якщо( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        якщо( pWalker->eCode==5 ) ExprSetProperty(pExpr, EP_FromDDL);
        повернути WRC_Continue;
      }інакше якщо( ExprHasProperty(pExpr,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        повернути WRC_Continue;
      }інакше якщо( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        повернути exprNodeIsConstantFunction(pWalker, pExpr);
      }інакше{
        pWalker->eCode = 0;  /* Function call is not constant */
        повернути WRC_Abort;
      }
    case TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      якщо( sqlite3ExprIdToTrueFalse(pExpr) ){
        повернути WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    case TK_COLUMN:
    case TK_AGG_FUNCTION:
    case TK_AGG_COLUMN:
      testcase( pExpr->op==TK_ID );
      testcase( pExpr->op==TK_COLUMN );
      testcase( pExpr->op==TK_AGG_FUNCTION );
      testcase( pExpr->op==TK_AGG_COLUMN );
      якщо( ExprHasProperty(pExpr, EP_FixedCol) && pWalker->eCode!=2 ){
        повернути WRC_Continue;
      }
      якщо( pWalker->eCode==3 && pExpr->iTable==pWalker->u.iCur ){
        повернути WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    case TK_IF_NULL_ROW:
    case TK_REGISTER:
    case TK_DOT:
    case TK_RAISE:
      testcase( pExpr->op==TK_REGISTER );
      testcase( pExpr->op==TK_IF_NULL_ROW );
      testcase( pExpr->op==TK_DOT );
      testcase( pExpr->op==TK_RAISE );
      pWalker->eCode = 0;
      повернути WRC_Abort;
    case TK_IS:
    case TK_ISNOT:
      якщо( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        повернути WRC_Abort;
      }
      повернути WRC_Continue;
    case TK_VARIABLE:
      якщо( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        pExpr->op = TK_NULL;
      }інакше якщо( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        повернути WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    default:
      testcase( pExpr->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( pExpr->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      повернути WRC_Continue;
  }
}
static ціле exprIsConst(Parse *pParse, Expr *p, ціле initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.xExprCallback = exprNodeIsConstant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3WalkExpr(&w, p);
  повернути w.eCode;
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
ціле sqlite3ExprIsConstant(Parse *pParse, Expr *p){
  повернути exprIsConst(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
ціле sqlite3ExprListIsConstant(Parse *pParse, ExprList *pList, ціле bNoIs){
  const ціле iInit = bNoIs ? 6 : 1;
  ціле ii;
  для(ii=0; ii<pList->nExpr; ii++){
    якщо( 0==exprIsConst(pParse, pList->a[ii].pExpr, iInit) ) повернути 0;
  }
  повернути 1;
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
static ціле sqlite3ExprIsConstantNotJoin(Parse *pParse, Expr *p){
  повернути exprIsConst(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
static ціле exprSelectWalkTableConstant(Walker *pWalker, Select *pSelect){
  assert( pSelect!=0 );
  assert( pWalker->eCode==3 || pWalker->eCode==0 );
  якщо( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    повернути WRC_Abort;
  }
  повернути WRC_Prune;
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
ціле sqlite3ExprIsTableConstant(Expr *p, ціле iCur, ціле bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.xExprCallback = exprNodeIsConstant;
  якщо( bAllowSubq ){
    w.xSelectCallback = exprSelectWalkTableConstant;
  }інакше{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3WalkExpr(&w, p);
  повернути w.eCode;
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
ціле sqlite3ExprIsSingleTableConstraint(
  Expr *pExpr,                 /* The constraint */
  const SrcList *pSrcList,     /* Complete FROM clause */
  ціле iSrc,                    /* Which element of pSrcList to use */
  ціле bAllowSubq               /* Allow non-correlated subqueries */
){
  const SrcItem *pSrc = &pSrcList->a[iSrc];
  якщо( pSrc->fg.jointype & JT_LTORJ ){
    повернути 0;  /* rule (3) */
  }
  якщо( pSrc->fg.jointype & JT_LEFT ){
    якщо( !ExprHasProperty(pExpr, EP_OuterON) ) повернути 0;   /* rule (4a) */
    якщо( pExpr->w.iJoin!=pSrc->iCursor ) повернути 0;         /* rule (4b) */
  }інакше{
    якщо( ExprHasProperty(pExpr, EP_OuterON) ) повернути 0;    /* rule (5) */
  }
  якщо( ExprHasProperty(pExpr, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    ціле jj;
    для(jj=0; jj<iSrc; jj++){
      якщо( pExpr->w.iJoin==pSrcList->a[jj].iCursor ){
        якщо( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          повернути 0;  /* restriction (6) */
        }
        break;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  повернути sqlite3ExprIsTableConstant(pExpr, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
static ціле exprNodeIsConstantOrGroupBy(Walker *pWalker, Expr *pExpr){
  ExprList *pGroupBy = pWalker->u.pGroupBy;
  ціле i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  для(i=0; i<pGroupBy->nExpr; i++){
    Expr *p = pGroupBy->a[i].pExpr;
    якщо( sqlite3ExprCompare(0, pExpr, p, -1)<2 ){
      CollSeq *pColl = sqlite3ExprNNCollSeq(pWalker->pParse, p);
      якщо( sqlite3IsBinary(pColl) ){
        повернути WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  якщо( ExprUseXSelect(pExpr) ){
    pWalker->eCode = 0;
    повернути WRC_Abort;
  }

  повернути exprNodeIsConstant(pWalker, pExpr);
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
ціле sqlite3ExprIsConstantOrGroupBy(Parse *pParse, Expr *p, ExprList *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.xExprCallback = exprNodeIsConstantOrGroupBy;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3WalkExpr(&w, p);
  повернути w.eCode;
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
ціле sqlite3ExprIsConstantOrFunction(Expr *p, u8 isInit){
  assert( isInit==0 || isInit==1 );
  повернути exprIsConst(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
ціле sqlite3ExprContainsSubquery(Expr *p){
  Walker w;
  w.eCode = 1;
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3WalkExpr(&w, p);
  повернути w.eCode==0;
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
ціле sqlite3ExprIsInteger(const Expr *p, ціле *pValue, Parse *pParse, ціле bRSI){
  ціле iSign = 1;   /* Either +1 or -1. */
  поки( 1/*exit-by-break*/ ){
    якщо( NEVER(p==0) ) повернути 0;
    якщо( ExprUseUValue(p) ){
      assert( p->op==TK_INTEGER );
      *pValue = p->u.iValue*iSign;
      якщо( bRSI && pParse==0 && p->op2 ){
        повернути 0;
      }
      повернути 1;
    }
    якщо( p->op==TK_UPLUS ){
      p = p->pLeft;
      pParse = 0;
      continue;
    }
    якщо( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->pLeft;
      pParse = 0;
      continue;
    }
    якщо( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_value *pVal; /* The variable */
      ціле rc = 0;          /* 1 if successful, 0 if failed */
      assert( iSign==1 );
      якщо( NEVER(pParse->pVdbe==0) ) break;
      якщо( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) break;
      pVal = sqlite3VdbeGetBoundValue(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      якщо( pVal ){
        i64 vv;
        якщо( sqlite3_value_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_value_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *pValue = (ціле)vv;
          rc = 1;       
        }
        sqlite3ValueFree(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      повернути rc;
    }
    break;
  }
  повернути 0;
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
ціле sqlite3ExprCanBeNull(const Expr *p){
  u8 op;
  assert( p!=0 );
  поки( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->pLeft;
    assert( p!=0 );
  }
  op = p->op;
  якщо( op==TK_REGISTER ) op = p->op2;
  switch( op ){
    case TK_INTEGER:
    case TK_STRING:
    case TK_FLOAT:
    case TK_BLOB:
      повернути 0;
    case TK_COLUMN:
      assert( ExprUseYTab(p) );
      повернути ExprHasProperty(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    default:
      повернути 1;
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
ціле sqlite3ExprNeedsNoAffinityChange(const Expr *p, char aff){
  u8 op;
  ціле unaryMinus = 0;
  якщо( aff==SQLITE_AFF_BLOB ) повернути 1;
  поки( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    якщо( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->pLeft;
  }
  op = p->op;
  якщо( op==TK_REGISTER ) op = p->op2;
  switch( op ){
    case TK_INTEGER: {
      повернути aff>=SQLITE_AFF_NUMERIC;
    }
    case TK_FLOAT: {
      повернути aff>=SQLITE_AFF_NUMERIC;
    }
    case TK_STRING: {
      повернути !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    case TK_BLOB: {
      повернути !unaryMinus;
    }
    case TK_COLUMN: {
      assert( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      повернути aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    default: {
      повернути 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
ціле sqlite3IsRowid(const char *z){
  якщо( sqlite3StrICmp(z, "_ROWID_")==0 ) повернути 1;
  якщо( sqlite3StrICmp(z, "ROWID")==0 ) повернути 1;
  якщо( sqlite3StrICmp(z, "OID")==0 ) повернути 1;
  повернути 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
const char *sqlite3RowidAlias(Table *pTab){
  const char *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  ціле ii;
  assert( VisibleRowid(pTab) );
  для(ii=0; ii<ArraySize(azOpt); ii++){
    якщо( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) повернути azOpt[ii];
  }
  повернути 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
static Select *isCandidateForInOpt(const Expr *pX){
  Select *p;
  SrcList *pSrc;
  ExprList *pEList;
  Table *pTab;
  ціле i;
  якщо( !ExprUseXSelect(pX) ) повернути 0;                 /* Not a subquery */
  якщо( ExprHasProperty(pX, EP_VarSelect)  ) повернути 0;  /* Correlated subq */
  p = pX->x.pSelect;
  якщо( p->pPrior ) повернути 0;              /* Not a compound SELECT */
  якщо( p->selFlags & SF_Aggregate ){
    повернути 0; /* No GROUP BY keyword or aggregate functions */
  }
  assert( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  якщо( p->pLimit ) повернути 0;              /* Has no LIMIT clause */
  якщо( p->pWhere ) повернути 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  assert( pSrc!=0 );
  якщо( pSrc->nSrc!=1 ) повернути 0;          /* Single term in FROM clause */
  якщо( pSrc->a[0].fg.isSubquery) повернути 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  assert( pTab!=0 );
  assert( !IsView(pTab)  );              /* FROM clause is not a view */
  якщо( IsVirtual(pTab) ) повернути 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  assert( pEList!=0 );
  /* All SELECT results must be columns. */
  для(i=0; i<pEList->nExpr; i++){
    Expr *pRes = pEList->a[i].pExpr;
    якщо( pRes->op!=TK_COLUMN ) повернути 0;
    assert( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  повернути p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
static void sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  ціле iCur,          /* Cursor for the index */
  ціле regHasNull,    /* Register in which to store hasNull flag */
  ціле eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  ціле addr1;
  ціле op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  якщо( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }інакше{
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
static ціле sqlite3InRhsIsConstant(Parse *pParse, Expr *pIn){
  Expr *pLHS;
  ціле res;
  assert( !ExprHasProperty(pIn, EP_xIsSelect) );
  pLHS = pIn->pLeft;
  pIn->pLeft = 0;
  res = sqlite3ExprIsConstant(pParse, pIn);
  pIn->pLeft = pLHS;
  повернути res;
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
ціле sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  Expr *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  ціле *prRhsHasNull,         /* Register holding NULL status.  See notes */
  ціле *aiMap,                /* Mapping from Index fields to RHS fields */
  ціле *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  ціле eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  ціле iTab;                             /* Cursor of the RHS table */
  ціле mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  assert( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  якщо( prRhsHasNull && ExprUseXSelect(pX) ){
    ціле i;
    ExprList *pEList = pX->x.pSelect->pEList;
    для(i=0; i<pEList->nExpr; i++){
      якщо( sqlite3ExprCanBeNull(pEList->a[i].pExpr) ) break;
    }
    якщо( i==pEList->nExpr ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  якщо( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    ціле iDb;                               /* Database idx for pTab */
    ExprList *pEList = p->pEList;
    ціле nExpr = pEList->nExpr;

    assert( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    assert( p->pEList->a[0].pExpr!=0 ); /* Because of isCandidateForInOpt(p) */
    assert( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    assert( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    assert(v);  /* sqlite3GetVdbe() has always been previously called */
    якщо( nExpr==1 && pEList->a[0].pExpr->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      ціле iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }інакше{
      Index *pIdx;                         /* Iterator variable */
      ціле affinity_ok = 1;
      ціле i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      для(i=0; i<nExpr && affinity_ok; i++){
        Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
        ціле iCol = pEList->a[i].pExpr->iColumn;
        char idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        char cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        switch( cmpaff ){
          case SQLITE_AFF_BLOB:
            break;
          case SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            assert( idxaff==SQLITE_AFF_TEXT );
            break;
          default:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      якщо( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        для(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          якщо( pIdx->nColumn<nExpr ) continue;
          якщо( pIdx->pPartIdxWhere!=0 ) continue;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          якщо( pIdx->nColumn>=BMS-1 ) continue;
          якщо( mustBeUnique ){
            якщо( pIdx->nKeyCol>nExpr
             ||(pIdx->nColumn>nExpr && !IsUniqueIndex(pIdx))
            ){
              continue;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          для(i=0; i<nExpr; i++){
            Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
            Expr *pRhs = pEList->a[i].pExpr;
            CollSeq *pReq = sqlite3BinaryCompareCollSeq(pParse, pLhs, pRhs);
            ціле j;
 
            для(j=0; j<nExpr; j++){
              якщо( pIdx->aiColumn[j]!=pRhs->iColumn ) continue;
              assert( pIdx->azColl[j] );
              якщо( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                continue;
              }
              break;
            }
            якщо( j==nExpr ) break;
            mCol = MASKBIT(j);
            якщо( mCol & colUsed ) break; /* Each column used only once */
            colUsed |= mCol;
            якщо( aiMap ) aiMap[i] = j;
          }
 
          assert( nExpr>0 && nExpr<BMS );
          assert( i==nExpr || colUsed!=(MASKBIT(nExpr)-1) );
          якщо( colUsed==(MASKBIT(nExpr)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            ціле iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            assert( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            якщо( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<nExpr)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              якщо( nExpr==1 ){
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
  якщо( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && ExprUseXList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->nExpr<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  якщо( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    ціле rMayHaveNull = 0;
    ціле bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    якщо( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }інакше якщо( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    assert( pX->op==TK_IN );
    якщо( !bloomOk
     && ExprUseXSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    якщо( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  якщо( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    ціле i, n;
    n = sqlite3ExprVectorSize(pX->pLeft);
    для(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  повернути eType;
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
static char *exprINAffinity(Parse *pParse, const Expr *pExpr){
  Expr *pLeft = pExpr->pLeft;
  ціле nVal = sqlite3ExprVectorSize(pLeft);
  Select *pSelect = ExprUseXSelect(pExpr) ? pExpr->x.pSelect : 0;
  char *zRet;

  assert( pExpr->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  якщо( zRet ){
    ціле i;
    для(i=0; i<nVal; i++){
      Expr *pA = sqlite3VectorFieldSubexpr(pLeft, i);
      char a = sqlite3ExprAffinity(pA);
      якщо( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].pExpr, a);
      }інакше{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  повернути zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
void sqlite3SubselectError(Parse *pParse, ціле nActual, ціле nExpect){
  якщо( pParse->nErr==0 ){
    const char *zFmt = "sub-select returns %d columns - expected %d";
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
void sqlite3VectorErrorMsg(Parse *pParse, Expr *pExpr){
#ifndef SQLITE_OMIT_SUBQUERY
  якщо( ExprUseXSelect(pExpr) ){
    sqlite3SubselectError(pParse, pExpr->x.pSelect->pEList->nExpr, 1);
  }інакше
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
static ціле findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  Expr *pExpr,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  якщо( pNewSig==0 ) повернути 0;
  якщо( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) повернути 0;
  assert( pExpr->op==TK_IN );
  assert( !ExprUseYSub(pExpr) );
  assert( ExprUseXSelect(pExpr) );
  assert( pExpr->x.pSelect!=0 );
  assert( (pExpr->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  assert( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  для(; pOp<pEnd; pOp++){
    якщо( pOp->p4type!=P4_SUBRTNSIG ) continue;
    assert( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    assert( pSig!=0 );
    якщо( !pSig->bComplete ) continue;
    якщо( pNewSig->selId!=pSig->selId ) continue;
    якщо( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) continue;
    pExpr->y.sub.iAddr = pSig->iAddr;
    pExpr->y.sub.regReturn = pSig->regReturn;
    pExpr->iTable = pSig->iTable;
    ExprSetProperty(pExpr, EP_Subrtn);
    повернути 1;
  }
  повернути 0;
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
void sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  Expr *pExpr,            /* The IN operator */
  ціле iTab,               /* Use this cursor number */
  ціле allowBloom          /* True to allow the use of a Bloom filter */
){
  ціле addrOnce = 0;           /* Address of the OP_Once instruction at top */
  ціле addr;                   /* Address of OP_OpenEphemeral instruction */
  Expr *pLeft;                /* the LHS of the IN operator */
  KeyInfo *pKeyInfo = 0;      /* Key information */
  ціле nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  assert( v!=0 );

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
  якщо( !ExprHasProperty(pExpr, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    assert( !ExprUseXSelect(pExpr) || pExpr->x.pSelect!=0 );
    якщо( ExprUseXSelect(pExpr) && (pExpr->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, sizeof(pSig[0]));
      якщо( pSig ){
        pSig->selId = pExpr->x.pSelect->selId;
        pSig->zAff = exprINAffinity(pParse, pExpr);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    якщо( ExprHasProperty(pExpr, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, pExpr, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      якщо( ExprUseXSelect(pExpr) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              pExpr->x.pSelect->selId));
      }
      assert( ExprUseYSub(pExpr) );
      sqlite3VdbeAddOp2(v, OP_Gosub, pExpr->y.sub.regReturn,
                        pExpr->y.sub.iAddr);
      assert( iTab!=pExpr->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, pExpr->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      якщо( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      повернути;
    }

    /* Begin coding the subroutine */
    assert( !ExprUseYWin(pExpr) );
    ExprSetProperty(pExpr, EP_Subrtn);
    assert( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
    pExpr->y.sub.regReturn = ++pParse->nMem;
    pExpr->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, pExpr->y.sub.regReturn) + 1;
    якщо( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = pExpr->y.sub.iAddr;
      pSig->regReturn = pExpr->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (const char*)pSig, P4_SUBRTNSIG);
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
  якщо( ExprUseXSelect(pExpr) ){
    VdbeComment((v, "Result of SELECT %u", pExpr->x.pSelect->selId));
  }інакше{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  pKeyInfo = sqlite3KeyInfoAlloc(pParse->db, nVal, 1);
  assert( pKeyInfo!=0 || pParse->nErr );
  якщо( pKeyInfo==0 ) повернути;

  якщо( ExprUseXSelect(pExpr) ){
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
    якщо( ALWAYS(pEList->nExpr==nVal) ){
      Select *pCopy;
      SelectDest dest;
      ціле i;
      ціле rc;
      ціле addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = exprINAffinity(pParse, pExpr);
      pSelect->iLimit = 0;
      assert( pEList!=0 );
      assert( pEList->nExpr>0 );
      assert( sqlite3KeyInfoIsWriteable(pKeyInfo) );
      для(i=0; i<nVal; i++){
        Expr *p = sqlite3VectorFieldSubexpr(pLeft, i);
        CollSeq *pColl;
        pKeyInfo->aColl[i] = pColl = sqlite3BinaryCompareCollSeq(
            pParse, p, pEList->a[i].pExpr
        );
        якщо( !sqlite3IsBinary(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      якщо( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        ціле regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (void *)pKeyInfo, P4_KEYINFO);
        pKeyInfo = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      якщо( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        якщо( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      якщо( rc ){
        sqlite3KeyInfoUnref(pKeyInfo);
        повернути;
      }
    }
  }інакше якщо( ALWAYS(pExpr->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    char affinity;            /* Affinity of the LHS of the IN */
    ціле i;
    ExprList *pList = pExpr->x.pList;
    struct ExprList_item *pItem;
    ціле r1, r2;
    affinity = sqlite3ExprAffinity(pLeft);
    якщо( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }інакше якщо( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    assert( pKeyInfo!=0 );
    assert( sqlite3KeyInfoIsWriteable(pKeyInfo) );
    pKeyInfo->aColl[0] = sqlite3ExprCollSeq(pParse, pExpr->pLeft);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    для(i=pList->nExpr, pItem=pList->a; i>0; i--, pItem++){
      Expr *pE2 = pItem->pExpr;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      якщо( addrOnce && !sqlite3ExprIsConstant(pParse, pE2) ){
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
  якщо( pSig ) pSig->bComplete = 1;
  якщо( pKeyInfo ){
    sqlite3VdbeChangeP4(v, addr, (void *)pKeyInfo, P4_KEYINFO);
  }
  якщо( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    assert( ExprUseYSub(pExpr) );
    assert( sqlite3VdbeGetOp(v,pExpr->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
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
ціле sqlite3CodeSubselect(Parse *pParse, Expr *pExpr){
  ціле addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  ціле rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  ціле nReg;                   /* Registers to allocate */
  Expr *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ціле addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  assert( v!=0 );
  якщо( pParse->nErr ) повернути 0;
  testcase( pExpr->op==TK_EXISTS );
  testcase( pExpr->op==TK_SELECT );
  assert( pExpr->op==TK_EXISTS || pExpr->op==TK_SELECT );
  assert( ExprUseXSelect(pExpr) );
  pSel = pExpr->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  якщо( ExprHasProperty(pExpr, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    assert( ExprUseYSub(pExpr) );
    sqlite3VdbeAddOp2(v, OP_Gosub, pExpr->y.sub.regReturn,
                      pExpr->y.sub.iAddr);
    повернути pExpr->iTable;
  }

  /* Begin coding the subroutine */
  assert( !ExprUseYWin(pExpr) );
  assert( !ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) );
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
  якщо( !ExprHasProperty(pExpr, EP_VarSelect) ){
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
  якщо( pExpr->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    якщо( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->pRight ){
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
    }інакше{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }інакше{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  якщо( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    Expr *pLeft = pSel->pLimit->pLeft;
    якщо( ExprHasProperty(pLeft, EP_IntValue)==0
     || (pLeft->u.iValue!=1 && pLeft->u.iValue!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3ExprInt32(db, 0);
      якщо( pLimit ){
        pLimit->affExpr = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3ExprDup(db, pLeft, 0), pLimit);
      }
      sqlite3ExprDeferredDelete(pParse, pLeft);
      pSel->pLimit->pLeft = pLimit;
    }
  }інакше{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3ExprInt32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  якщо( sqlite3Select(pParse, pSel, &dest) ){
    pExpr->op2 = pExpr->op;
    pExpr->op = TK_ERROR;
    повернути 0;
  }
  pExpr->iTable = rReg = dest.iSDParm;
  ExprSetVVAProperty(pExpr, EP_NoReduce);
  якщо( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  якщо( dest.eDest==SRT_Mem 
   && sqlite3ExprCanReturnSubtype(pParse, pSel->pEList->a[0].pExpr) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  assert( ExprUseYSub(pExpr) );
  assert( sqlite3VdbeGetOp(v,pExpr->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, pExpr->y.sub.regReturn,
                    pExpr->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  повернути rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
ціле sqlite3ExprCheckIN(Parse *pParse, Expr *pIn){
  ціле nVector = sqlite3ExprVectorSize(pIn->pLeft);
  якщо( ExprUseXSelect(pIn) && !pParse->db->mallocFailed ){
    якщо( nVector!=pIn->x.pSelect->pEList->nExpr ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->nExpr, nVector);
      повернути 1;
    }
  }інакше якщо( nVector!=1 ){
    sqlite3VectorErrorMsg(pParse, pIn->pLeft);
    повернути 1;
  }
  повернути 0;
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
static void sqlite3ExprCodeIN(
  Parse *pParse,        /* Parsing and code generating context */
  Expr *pExpr,          /* The IN expression */
  ціле destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  ціле destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  ціле rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  ціле eType;            /* Type of the RHS */
  ціле rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  ціле *aiMap = 0;       /* Map from vector field to index column */
  char *zAff = 0;       /* Affinity string for comparisons */
  ціле nVector;          /* Size of vectors for this IN operator */
  ціле iDummy;           /* Dummy parameter to exprCodeVector() */
  Expr *pLeft;          /* The LHS of the IN operator */
  ціле i;                /* loop counter */
  ціле destStep2;        /* Where to jump when NULLs seen in step 2 */
  ціле destStep6 = 0;    /* Start of code for Step 6 */
  ціле addrTruthOp;      /* Address of opcode that determines the IN is true */
  ціле destNotNull;      /* Jump here if a comparison is not true in step 6 */
  ціле addrTop;          /* Top of the step-6 loop */
  ціле iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  assert( !ExprHasVVAProperty(pExpr,EP_Immutable) );
  pLeft = pExpr->pLeft;
  якщо( sqlite3ExprCheckIN(pParse, pExpr) ) повернути;
  zAff = exprINAffinity(pParse, pExpr);
  nVector = sqlite3ExprVectorSize(pExpr->pLeft);
  aiMap = (ціле*)sqlite3DbMallocZero(pParse->db, nVector*sizeof(ціле));
  якщо( pParse->db->mallocFailed ) goto sqlite3ExprCodeIN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  assert( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, pExpr,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  assert( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  для(i=0; i<nVector; i++){
    ціле j, cnt;
    для(cnt=j=0; j<nVector; j++) якщо( aiMap[j]==i ) cnt++;
    assert( cnt==1 );
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
  assert( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = exprCodeVector(pParse, pLeft, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  якщо( eType==IN_INDEX_NOOP ){
    ExprList *pList;
    CollSeq *pColl;
    ціле labelOk = sqlite3VdbeMakeLabel(pParse);
    ціле r2, regToFree;
    ціле regCkNull = 0;
    ціле ii;
    assert( nVector==1 );
    assert( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    pColl = sqlite3ExprCollSeq(pParse, pExpr->pLeft);
    якщо( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    для(ii=0; ii<pList->nExpr; ii++){
      r2 = sqlite3ExprCodeTemp(pParse, pList->a[ii].pExpr, &regToFree);
      якщо( regCkNull && sqlite3ExprCanBeNull(pList->a[ii].pExpr) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      якщо( ii<pList->nExpr-1 || destIfNull!=destIfFalse ){
        ціле op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (void*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->nExpr-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->nExpr-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->nExpr-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->nExpr-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }інакше{
        ціле op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        assert( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (void*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    якщо( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    goto sqlite3ExprCodeIN_finished;
  }

  якщо( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    якщо( nVector==1 ){
      char aff = zAff[0];
      якщо( aff>=SQLITE_AFF_TEXT && aff!=sqlite3ExprAffinity(pLeft) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        ціле rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }інакше{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    для(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    якщо( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      ціле rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      для(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  якщо( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }інакше{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  для(i=0; i<nVector; i++){
    Expr *p = sqlite3VectorFieldSubexpr(pExpr->pLeft, i);
    якщо( pParse->nErr ) goto sqlite3ExprCodeIN_oom_error;
    якщо( sqlite3ExprCanBeNull(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  якщо( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    assert( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }інакше{
    якщо( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      якщо( ExprHasProperty(pExpr, EP_Subrtn) ){
        const VdbeOp *pOp = sqlite3VdbeGetOp(v, pExpr->y.sub.iAddr);
        assert( pOp->opcode==OP_Once || pParse->nErr );
        якщо( pOp->p3>0 ){  /* tag-202407032019 */
          assert( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      goto sqlite3ExprCodeIN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  якщо( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  якщо( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  якщо( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  якщо( eType==IN_INDEX_INDEX_DESC ){
    addrTop = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }інакше{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addrTop = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  якщо( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }інакше{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  для(i=0; i<nVector; i++){
    Expr *p;
    CollSeq *pColl;
    ціле r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(pLeft, i);
    якщо( ExprUseXSelect(pExpr) ){
      Expr *pRhs = pExpr->x.pSelect->pEList->a[i].pExpr;
      pColl = sqlite3BinaryCompareCollSeq(pParse, p, pRhs);
    }інакше{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3ExprCollSeq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (void*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  якщо( nVector>1 ){
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
static void codeReal(Vdbe *v, const char *z, ціле negateFlag, ціле iMem){
  якщо( ALWAYS(z!=0) ){
    double value;
    sqlite3AtoF(z, &value);
    assert( !sqlite3IsNaN(value) ); /* The new AtoF never returns NaN */
    якщо( negateFlag ) value = -value;
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
static void codeInteger(Parse *pParse, Expr *pExpr, ціле negFlag, ціле iMem){
  Vdbe *v = pParse->pVdbe;
  якщо( pExpr->flags & EP_IntValue ){
    ціле i = pExpr->u.iValue;
    assert( i>=0 );
    якщо( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }інакше{
    ціле c;
    i64 value;
    const char *z = pExpr->u.zToken;
    assert( z!=0 );
    c = sqlite3DecOrHexToI64(z, &value);
    якщо( (c==3 && !negFlag) || (c==2) || (negFlag && value==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3ErrorMsg(pParse, "oversized integer: %s%#T", negFlag?"-":"",pExpr);
#інакше
#ifndef SQLITE_OMIT_HEX_INTEGER
      якщо( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3ErrorMsg(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",pExpr);
      }інакше
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }інакше{
      якщо( negFlag ){ value = c==3 ? SMALLEST_INT64 : -value; }
      sqlite3VdbeAddInt64(v, iMem, value);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
void sqlite3ExprCodeLoadIndexColumn(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  ціле iTabCur,    /* Cursor pointing to a table row */
  ціле iIdxCol,    /* The column of the index to be loaded */
  ціле regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  якщо( iTabCol==XN_EXPR ){
    assert( pIdx->aColExpr );
    assert( pIdx->aColExpr->nExpr>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3ExprCodeCopy(pParse, pIdx->aColExpr->a[iIdxCol].pExpr, regOut);
    pParse->iSelfTab = 0;
  }інакше{
    sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
void sqlite3ExprCodeGeneratedColumn(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  ціле regOut         /* Put the result in this register */
){
  ціле iAddr;
  Vdbe *v = pParse->pVdbe;
  ціле nErr = pParse->nErr;
  assert( v!=0 );
  assert( pParse->iSelfTab!=0 );
  якщо( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }інакше{
    iAddr = 0;
  }
  sqlite3ExprCodeCopy(pParse, sqlite3ColumnExpr(pTab,pCol), regOut);
  якщо( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    ціле p3 = 2+(ціле)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (char*)pTab, P4_TABLE);
  }інакше якщо( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  якщо( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  якщо( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
void sqlite3ExprCodeGetColumnOfTable(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  ціле iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  ціле iCol,       /* Index of the column to extract */
  ціле regOut      /* Extract the value into this register */
){
  Column *pCol;
  assert( v!=0 );
  assert( pTab!=0 );
  assert( iCol!=XN_EXPR );
  якщо( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }інакше{
    ціле x;
    якщо( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      повернути;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }інакше якщо( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      якщо( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }інакше{
        ціле savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      повернути;
#endif
    }інакше якщо( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), iCol);
    }інакше{
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
ціле sqlite3ExprCodeGetColumn(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  ціле iColumn,     /* Index of the table column */
  ціле iTable,      /* The cursor pointing to the table */
  ціле iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  assert( pParse->pVdbe!=0 );
  assert( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  assert( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  якщо( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    якщо( pOp->opcode==OP_Column ) pOp->p5 = p5;
    якщо( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  повернути iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
void sqlite3ExprCodeMove(Parse *pParse, ціле iFrom, ціле iTo, ціле nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
void sqlite3ExprToRegister(Expr *pExpr, ціле iReg){
  Expr *p = sqlite3ExprSkipCollateAndLikely(pExpr);
  якщо( NEVER(p==0) ) повернути;
  якщо( p->op==TK_REGISTER ){
    assert( p->iTable==iReg );
  }інакше{
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
static ціле exprCodeVector(Parse *pParse, Expr *p, ціле *piFreeable){
  ціле iResult;
  ціле nResult = sqlite3ExprVectorSize(p);
  якщо( nResult==1 ){
    iResult = sqlite3ExprCodeTemp(pParse, p, piFreeable);
  }інакше{
    *piFreeable = 0;
    якщо( p->op==TK_SELECT ){
#якщо SQLITE_OMIT_SUBQUERY
      iResult = 0;
#інакше
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }інакше{
      ціле i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      assert( ExprUseXList(p) );
      для(i=0; i<nResult; i++){
        sqlite3ExprCodeFactorable(pParse, p->x.pList->a[i].pExpr, i+iResult);
      }
    }
  }
  повернути iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
static void setDoNotMergeFlagOnCopy(Vdbe *v){
  якщо( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
static ціле exprCodeInlineFunction(
  Parse *pParse,        /* Parsing context */
  ExprList *pFarg,      /* List of function arguments */
  ціле iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  ціле target            /* Store function result in this register */
){
  ціле nFarg;
  Vdbe *v = pParse->pVdbe;
  assert( v!=0 );
  assert( pFarg!=0 );
  nFarg = pFarg->nExpr;
  assert( nFarg>0 );  /* All in-line functions have at least one argument */
  switch( iFuncId ){
    case INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      ціле endCoalesce = sqlite3VdbeMakeLabel(pParse);
      ціле i;
      assert( nFarg>=2 );
      sqlite3ExprCode(pParse, pFarg->a[0].pExpr, target);
      для(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3ExprCode(pParse, pFarg->a[i].pExpr, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      break;
    }
    case INLINEFUNC_iif: {
      Expr caseExpr;
      memset(&caseExpr, 0, sizeof(caseExpr));
      caseExpr.op = TK_CASE;
      caseExpr.x.pList = pFarg;
      повернути sqlite3ExprCodeTarget(pParse, &caseExpr, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    case INLINEFUNC_sqlite_offset: {
      Expr *pArg = pFarg->a[0].pExpr;
      якщо( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }інакше{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      break;
    }
#endif
    default: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      assert( nFarg==1 || nFarg==2 );
      target = sqlite3ExprCodeTarget(pParse, pFarg->a[0].pExpr, target);
      break;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#якщо !defined(SQLITE_UNTESTABLE)
    case INLINEFUNC_expr_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      assert( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3ExprCompare(0,pFarg->a[0].pExpr, pFarg->a[1].pExpr,-1),
         target);
      break;
    }

    case INLINEFUNC_expr_implies_expr: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      assert( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3ExprImpliesExpr(pParse,pFarg->a[0].pExpr, pFarg->a[1].pExpr,-1),
         target);
      break;
    }

    case INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      Expr *pA1;
      assert( nFarg==2 );
      pA1 = pFarg->a[1].pExpr;
      якщо( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3ExprImpliesNonNullRow(pFarg->a[0].pExpr,pA1->iTable,1),
           target);
      }інакше{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      break;
    }

    case INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      const char *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      char aff;
      assert( nFarg==1 );
      aff = sqlite3ExprAffinity(pFarg->a[0].pExpr);
      assert( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      break;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  повернути target;
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
static ціле exprNodeCanReturnSubtype(Walker *pWalker, Expr *pExpr){
  ціле n;
  FuncDef *pDef;
  sqlite3 *db;
  якщо( pExpr->op==TK_CASE || pExpr->op==TK_UPLUS 
   || pExpr->op==TK_COLLATE || pExpr->op==TK_CAST 
  ){
    повернути WRC_Continue;
  }
  якщо( pExpr->op!=TK_FUNCTION && pExpr->op!=TK_AGG_FUNCTION ){
    повернути WRC_Prune;
  }
  assert( ExprUseXList(pExpr) );
  db = pWalker->pParse->db;
  n = pExpr->x.pList ? pExpr->x.pList->nExpr : 0;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  якщо( pDef==0 ) повернути WRC_Continue;
#інакше
  assert( pDef!=0 );
#endif
  якщо( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    повернути WRC_Abort;
  }
  повернути WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
ціле sqlite3ExprCanReturnSubtype(Parse *pParse, Expr *pExpr){
  Walker w;
  memset(&w, 0, sizeof(w));
  w.pParse = pParse;
  w.xExprCallback = exprNodeCanReturnSubtype;
  sqlite3WalkExpr(&w, pExpr);
  повернути w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
static SQLITE_NOINLINE ціле sqlite3IndexedExprLookup(
  Parse *pParse,   /* The parsing context */
  Expr *pExpr,     /* The expression to potentially bypass */
  ціле target       /* Where to store the result of the expression */
){
  IndexedExpr *p;
  Vdbe *v;
  для(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 exprAff;
    ціле iDataCur = p->iDataCur;
    якщо( iDataCur<0 ) continue;
    якщо( pParse->iSelfTab ){
      якщо( p->iDataCur!=pParse->iSelfTab-1 ) continue;
      iDataCur = -1;
    }
    якщо( sqlite3ExprCompare(0, pExpr, p->pExpr, iDataCur)!=0 ) continue;
    assert( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    exprAff = sqlite3ExprAffinity(pExpr);
    якщо( (exprAff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (exprAff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (exprAff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      continue;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    якщо( ExprHasProperty(pExpr, EP_SubtArg)
     && sqlite3ExprCanReturnSubtype(pParse, pExpr) 
    ){
      continue;
    }

    v = pParse->pVdbe;
    assert( v!=0 );
    якщо( p->bMaybeNullRow || sqlite3ExprIsConstant(pParse, pExpr) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      ціле addr = sqlite3VdbeCurrentAddr(v);
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
    }інакше{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    повернути target;
  }
  повернути -1;  /* Not found */
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
static ціле exprPartidxExprLookup(Parse *pParse, Expr *pExpr, ціле iTarget){
  IndexedExpr *p;
  для(p=pParse->pIdxPartExpr; p; p=p->pIENext){
    якщо( pExpr->iColumn==p->iIdxCol && pExpr->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      ціле addr = 0;
      ціле ret;

      якщо( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3ExprCodeTarget(pParse, p->pExpr, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (const char*)&p->aff, 1);
      якщо( sqlite3ExprCanReturnSubtype(pParse, p->pExpr) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      якщо( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      повернути ret;
    }
  }
  повернути 0;
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
static SQLITE_NOINLINE ціле exprCodeTargetAndOr(
  Parse *pParse,     /* Parsing context */
  Expr *pExpr,       /* AND or OR expression to be coded */
  ціле target,        /* Put result in this register, guaranteed */
  ціле *pTmpReg       /* Write a temporary register here */
){
  ціле op;            /* The opcode.  TK_AND or TK_OR */
  ціле skipOp;        /* Opcode for the branch that skips one operand */
  ціле addrSkip;      /* Branch instruction that skips one of the operands */
  ціле regSS = 0;     /* Register holding computed operand when other omitted */
  ціле r1, r2;        /* Registers for left and right operands, respectively */
  Expr *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  assert( pExpr!=0 );
  op = pExpr->op;
  assert( op==TK_AND || op==TK_OR );
  assert( TK_AND==OP_And );            testcase( op==TK_AND );
  assert( TK_OR==OP_Or );              testcase( op==TK_OR );
  assert( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
  якщо( pAlt!=pExpr ){
    r1 = sqlite3ExprCodeTarget(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    повернути target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  якщо( exprEvalRhsFirst(pExpr) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3ExprCodeTarget(pParse, pExpr->pRight, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pTmpReg);
  }інакше{
    /* Compute the left operand first */
    r1 = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
    якщо( ExprHasProperty(pExpr->pRight, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }інакше{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  якщо( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  повернути target;
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
ціле sqlite3ExprCodeTarget(Parse *pParse, Expr *pExpr, ціле target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  ціле op;                   /* The opcode being coded */
  ціле inReg = target;       /* Results stored in register inReg */
  ціле regFree1 = 0;         /* If non-zero free this temporary register */
  ціле regFree2 = 0;         /* If non-zero free this temporary register */
  ціле r1, r2;               /* Various register numbers */
  Expr tempX;               /* Temporary expression node */
  ціле p5 = 0;

  assert( target>0 && target<=pParse->nMem );
  assert( v!=0 );

expr_code_doover:
  якщо( pExpr==0 ){
    op = TK_NULL;
  }інакше якщо( pParse->pIdxEpr!=0
   && !ExprHasProperty(pExpr, EP_Leaf)
   && (r1 = sqlite3IndexedExprLookup(pParse, pExpr, target))>=0
  ){
    повернути r1;
  }інакше{
    assert( !ExprHasVVAProperty(pExpr,EP_Immutable) );
    op = pExpr->op;
  }
  assert( op!=TK_ORDER );
  switch( op ){
    case TK_AGG_COLUMN: {
      AggInfo *pAggInfo = pExpr->pAggInfo;
      struct AggInfo_col *pCol;
      assert( pAggInfo!=0 );
      assert( pExpr->iAgg>=0 );
      якщо( pExpr->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        break;
      }
      pCol = &pAggInfo->aCol[pExpr->iAgg];
      якщо( !pAggInfo->directMode ){
        повернути AggInfoColumnReg(pAggInfo, pExpr->iAgg);
      }інакше якщо( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        якщо( pTab==0 ){
          /* No comment added */
        }інакше якщо( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }інакше{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          якщо( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        повернути target;
      }інакше якщо( pExpr->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, pExpr->iTable, pExpr->iColumn, target);
        повернути target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    case TK_COLUMN: {
      ціле iTab = pExpr->iTable;
      ціле iReg;
      якщо( ExprHasProperty(pExpr, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        ціле aff;
        iReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft,target);
        assert( ExprUseYTab(pExpr) );
        assert( pExpr->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
        якщо( aff>SQLITE_AFF_BLOB ){
          static const char zAff[] = "B\000C\000D\000E\000F";
          assert( SQLITE_AFF_BLOB=='A' );
          assert( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        повернути iReg;
      }
      якщо( iTab<0 ){
        якщо( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          ціле iSrc;
          ціле iCol = pExpr->iColumn;
          assert( ExprUseYTab(pExpr) );
          pTab = pExpr->y.pTab;
          assert( pTab!=0 );
          assert( iCol>=XN_ROWID );
          assert( iCol<pTab->nCol );
          якщо( iCol<0 ){
            повернути -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          якщо( pCol->colFlags & COLFLAG_GENERATED ){
            якщо( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              повернути 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            якщо( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            повернути iSrc;
          }інакше
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          якщо( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            повернути target;
          }інакше{
            повернути iSrc;
          }
        }інакше{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      інакше якщо( pParse->pIdxPartExpr 
       && 0!=(r1 = exprPartidxExprLookup(pParse, pExpr, target))
      ){
        повернути r1;
      }
      assert( ExprUseYTab(pExpr) );
      assert( pExpr->y.pTab!=0 );
      iReg = sqlite3ExprCodeGetColumn(pParse, pExpr->y.pTab,
                               pExpr->iColumn, iTab, target,
                               pExpr->op2);
      повернути iReg;
    }
    case TK_INTEGER: {
      codeInteger(pParse, pExpr, 0, target);
      повернути target;
    }
    case TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3ExprTruthValue(pExpr), target);
      повернути target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    case TK_FLOAT: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      codeReal(v, pExpr->u.zToken, 0, target);
      повернути target;
    }
#endif
    case TK_STRING: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3VdbeLoadString(v, target, pExpr->u.zToken);
      повернути target;
    }
    case TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + pExpr->y.nReg - 1);
      повернути target;
    }
    default: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      assert( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      повернути target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case TK_BLOB: {
      ціле n;
      const char *z;
      char *zBlob;
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      assert( pExpr->u.zToken[0]=='x' || pExpr->u.zToken[0]=='X' );
      assert( pExpr->u.zToken[1]=='\'' );
      z = &pExpr->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      assert( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      повернути target;
    }
#endif
    case TK_VARIABLE: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      assert( pExpr->u.zToken!=0 );
      assert( pExpr->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, pExpr->iColumn, target);
      повернути target;
    }
    case TK_REGISTER: {
      повернути pExpr->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    case TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3ExprCode(pParse, pExpr->pLeft, target);
      assert( inReg==target );
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(pExpr->u.zToken, 0));
      повернути inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    case TK_IS:
    case TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      Expr *pLeft = pExpr->pLeft;
      ціле addrIsNull = 0;
      якщо( sqlite3ExprIsVector(pLeft) ){
        codeVectorCompare(pParse, pExpr, target, op, p5);
      }інакше{
        якщо( ExprHasProperty(pExpr, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = exprComputeOperands(pParse, pExpr,
                                     &r1, &r2, &regFree1, &regFree2);
        }інакше{
          r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
          r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, pLeft, pExpr->pRight, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ExprHasProperty(pExpr,EP_Commuted));
        assert(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        assert(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        assert(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        assert(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        assert(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        assert(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        якщо( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }інакше{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          якщо( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      break;
    }
    case TK_AND:
    case TK_OR: {
      inReg = exprCodeTargetAndOr(pParse, pExpr, target, &regFree1);
      break;
    }
    case TK_PLUS:
    case TK_STAR:
    case TK_MINUS:
    case TK_REM:
    case TK_BITAND:
    case TK_BITOR:
    case TK_SLASH:
    case TK_LSHIFT:
    case TK_RSHIFT:
    case TK_CONCAT: {
      ціле addrIsNull;
      assert( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      assert( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      assert( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      assert( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      assert( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      assert( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      assert( TK_LSHIFT==OP_ShiftLeft );   testcase( op==TK_LSHIFT );
      assert( TK_RSHIFT==OP_ShiftRight );  testcase( op==TK_RSHIFT );
      assert( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      якщо( ExprHasProperty(pExpr, EP_Subquery) ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      якщо( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      break;
    }
    case TK_UMINUS: {
      Expr *pLeft = pExpr->pLeft;
      assert( pLeft );
      якщо( pLeft->op==TK_INTEGER ){
        codeInteger(pParse, pLeft, 1, target);
        повернути target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }інакше якщо( pLeft->op==TK_FLOAT ){
        assert( !ExprHasProperty(pExpr, EP_IntValue) );
        codeReal(v, pLeft->u.zToken, 1, target);
        повернути target;
#endif
      }інакше{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_IntValue|EP_TokenOnly;
        tempX.u.iValue = 0;
        ExprClearVVAProperties(&tempX);
        r1 = sqlite3ExprCodeTemp(pParse, &tempX, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      break;
    }
    case TK_BITNOT:
    case TK_NOT: {
      assert( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      assert( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      break;
    }
    case TK_TRUTH: {
      ціле isTrue;    /* IS TRUE or IS NOT TRUE */
      ціле bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      bNormal = pExpr->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      ціле addr;
      assert( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      assert( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      break;
    }
    case TK_AGG_FUNCTION: {
      AggInfo *pInfo = pExpr->pAggInfo;
      якщо( pInfo==0
       || NEVER(pExpr->iAgg<0)
       || NEVER(pExpr->iAgg>=pInfo->nFunc)
      ){
        assert( !ExprHasProperty(pExpr, EP_IntValue) );
        sqlite3ErrorMsg(pParse, "misuse of aggregate: %#T()", pExpr);
      }інакше{
        повернути AggInfoFuncReg(pInfo, pExpr->iAgg);
      }
      break;
    }
    case TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      ціле nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      const char *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      ціле i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      якщо( ExprHasProperty(pExpr, EP_WinFunc) ){
        повернути pExpr->y.pWin->regResult;
      }
#endif

      якщо( ConstFactorOk(pParse)
       && sqlite3ExprIsConstantNotJoin(pParse,pExpr)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        повернути sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
      }
      assert( !ExprHasProperty(pExpr, EP_TokenOnly) );
      assert( ExprUseXList(pExpr) );
      pFarg = pExpr->x.pList;
      nFarg = pFarg ? pFarg->nExpr : 0;
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      zId = pExpr->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      якщо( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      якщо( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3ErrorMsg(pParse, "unknown function: %#T()", pExpr);
        break;
      }
      якщо( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        assert( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        assert( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        повернути exprCodeInlineFunction(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }інакше якщо( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3ExprFunctionUsable(pParse, pExpr, pDef);
      }

      для(i=0; i<nFarg; i++){
        якщо( i<32 && sqlite3ExprIsConstant(pParse, pFarg->a[i].pExpr) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        якщо( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3ExprCollSeq(pParse, pFarg->a[i].pExpr);
        }
      }
      якщо( pFarg ){
        якщо( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }інакше{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        якщо( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 exprOp;
          assert( nFarg==1 );
          assert( pFarg->a[0].pExpr!=0 );
          exprOp = pFarg->a[0].pExpr->op;
          якщо( exprOp==TK_COLUMN || exprOp==TK_AGG_COLUMN ){
            assert( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            assert( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            assert( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            assert( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].pExpr->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3ExprCodeExprList(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }інакше{
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
      якщо( nFarg>=2 && ExprHasProperty(pExpr, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].pExpr);
      }інакше якщо( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].pExpr);
      }
#endif
      якщо( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        якщо( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (char *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, pExpr->op2);
      якщо( nFarg ){
        якщо( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }інакше{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      повернути target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_EXISTS:
    case TK_SELECT: {
      ціле nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      якщо( pParse->db->mallocFailed ){
        повернути 0;
      }інакше якщо( op==TK_SELECT
             && ALWAYS( ExprUseXSelect(pExpr) )
             && (nCol = pExpr->x.pSelect->pEList->nExpr)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }інакше{
        повернути sqlite3CodeSubselect(pParse, pExpr);
      }
      break;
    }
    case TK_SELECT_COLUMN: {
      ціле n;
      Expr *pLeft = pExpr->pLeft;
      якщо( pLeft->iTable==0 || pParse->withinRJSubrtn > pLeft->op2 ){
        pLeft->iTable = sqlite3CodeSubselect(pParse, pLeft);
        pLeft->op2 = pParse->withinRJSubrtn;
      }
      assert( pLeft->op==TK_SELECT || pLeft->op==TK_ERROR );
      n = sqlite3ExprVectorSize(pLeft);
      якщо( pExpr->iTable!=n ){
        sqlite3ErrorMsg(pParse, "%d columns assigned %d values",
                                pExpr->iTable, n);
      }
      повернути pLeft->iTable + pExpr->iColumn;
    }
    case TK_IN: {
      ціле destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ціле destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      повернути target;
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
    case TK_BETWEEN: {
      exprCodeBetween(pParse, pExpr, target, 0, 0);
      повернути target;
    }
    case TK_COLLATE: {
      якщо( !ExprHasProperty(pExpr, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        assert( pExpr->pLeft );
        sqlite3ExprCode(pParse, pExpr->pLeft, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        повернути target;
      }інакше{
        pExpr = pExpr->pLeft;
        goto expr_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    case TK_SPAN:
    case TK_UPLUS: {
      pExpr = pExpr->pLeft;
      goto expr_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    case TK_TRIGGER: {
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
      ціле iCol;
      ціле p1;

      assert( ExprUseYTab(pExpr) );
      pTab = pExpr->y.pTab;
      iCol = pExpr->iColumn;
      p1 = pExpr->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      assert( pExpr->iTable==0 || pExpr->iTable==1 );
      assert( iCol>=-1 && iCol<pTab->nCol );
      assert( pTab->iPKey<0 || iCol!=pTab->iPKey );
      assert( p1>=0 && p1<(pTab->nCol*2+2) );

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
      якщо( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      break;
    }

    case TK_VECTOR: {
      sqlite3ErrorMsg(pParse, "row value misused");
      break;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    case TK_IF_NULL_ROW: {
      ціле addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = pExpr->pAggInfo;
      якщо( pAggInfo ){
        assert( pExpr->iAgg>=0 && pExpr->iAgg<pAggInfo->nColumn );
        якщо( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, pExpr->iAgg);
          break;
        }
        якщо( pExpr->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[pExpr->iAgg].iSorterColumn,
                            target);
          inReg = target;
          break;
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
      assert( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      break;
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
    case TK_CASE: {
      ціле endLabel;                     /* GOTO label for end of CASE stmt */
      ціле nextCase;                     /* GOTO label for next WHEN clause */
      ціле nExpr;                        /* 2x number of WHEN terms */
      ціле i;                            /* Loop counter */
      ExprList *pEList;                 /* List of WHEN terms */
      struct ExprList_item *aListelem;  /* Array of WHEN terms */
      Expr opCompare;                   /* The X==Ei expression */
      Expr *pX;                         /* The X expression */
      Expr *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      Expr *pDel = 0;
      sqlite3 *db = pParse->db;

      assert( ExprUseXList(pExpr) && pExpr->x.pList!=0 );
      assert(pExpr->x.pList->nExpr > 0);
      pEList = pExpr->x.pList;
      aListelem = pEList->a;
      nExpr = pEList->nExpr;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      якщо( (pX = pExpr->pLeft)!=0 ){
        pDel = sqlite3ExprDup(db, pX, 0);
        якщо( db->mallocFailed ){
          sqlite3ExprDelete(db, pDel);
          break;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3ExprToRegister(pDel, exprCodeVector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, sizeof(opCompare));
        opCompare.op = TK_EQ;
        opCompare.pLeft = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      для(i=0; i<nExpr-1; i=i+2){
        якщо( pX ){
          assert( pTest!=0 );
          opCompare.pRight = aListelem[i].pExpr;
        }інакше{
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
      якщо( (nExpr&1)!=0 ){
        sqlite3ExprCode(pParse, pEList->a[nExpr-1].pExpr, target);
      }інакше{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3ExprDelete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      break;
    }
#ifndef SQLITE_OMIT_TRIGGER
    case TK_RAISE: {
      assert( pExpr->affExpr==OE_Rollback
           || pExpr->affExpr==OE_Abort
           || pExpr->affExpr==OE_Fail
           || pExpr->affExpr==OE_Ignore
      );
      якщо( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3ErrorMsg(pParse,
                       "RAISE() may only be used within a trigger-program");
        повернути 0;
      }
      якщо( pExpr->affExpr==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      якщо( pExpr->affExpr==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             pExpr->affExpr, r1);
      }
      break;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  повернути inReg;
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
ціле sqlite3ExprCodeRunJustOnce(
  Parse *pParse,    /* Parsing context */
  Expr *pExpr,      /* The expression to code when the VDBE initializes */
  ціле regDest       /* Store the value in this register */
){
  ExprList *p;
  assert( ConstFactorOk(pParse) );
  assert( regDest!=0 );
  p = pParse->pConstExpr;
  якщо( regDest<0 && p ){
    struct ExprList_item *pItem;
    ціле i;
    для(pItem=p->a, i=p->nExpr; i>0; pItem++, i--){
      якщо( pItem->fg.reusable
       && sqlite3ExprCompare(0,pItem->pExpr,pExpr,-1)==0
      ){
        повернути pItem->u.iConstExprReg;
      }
    }
  }
  pExpr = sqlite3ExprDup(pParse->db, pExpr, 0);
  якщо( pExpr!=0 && ExprHasProperty(pExpr, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    ціле addr;
    assert( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    якщо( !pParse->db->mallocFailed ){
      якщо( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3ExprCode(pParse, pExpr, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3ExprDelete(pParse->db, pExpr);
    sqlite3VdbeJumpHere(v, addr);
  }інакше{
    p = sqlite3ExprListAppend(pParse, p, pExpr);
    якщо( p ){
       struct ExprList_item *pItem = &p->a[p->nExpr-1];
       pItem->fg.reusable = regDest<0;
       якщо( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.iConstExprReg = regDest;
    }
    pParse->pConstExpr = p;
  }
  повернути regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE void sqlite3ExprNullRegisterRange(
  Parse *pParse,   /* Parsing context */
  ціле iReg,        /* First register to set to NULL */
  ціле nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  Expr t;
  memset(&t, 0, sizeof(t));
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
ціле sqlite3ExprCodeTemp(Parse *pParse, Expr *pExpr, ціле *pReg){
  ціле r2;
  pExpr = sqlite3ExprSkipCollateAndLikely(pExpr);
  якщо( ConstFactorOk(pParse)
   && ALWAYS(pExpr!=0)
   && pExpr->op!=TK_REGISTER
   && sqlite3ExprIsConstantNotJoin(pParse, pExpr)
  ){
    *pReg  = 0;
    r2 = sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
  }інакше{
    ціле r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3ExprCodeTarget(pParse, pExpr, r1);
    якщо( r2==r1 ){
      *pReg = r1;
    }інакше{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  повернути r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
void sqlite3ExprCode(Parse *pParse, Expr *pExpr, ціле target){
  ціле inReg;

  assert( pExpr==0 || !ExprHasVVAProperty(pExpr,EP_Immutable) );
  assert( target>0 && target<=pParse->nMem );
  assert( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  якщо( pParse->pVdbe==0 ) повернути;
  inReg = sqlite3ExprCodeTarget(pParse, pExpr, target);
  якщо( inReg!=target ){
    u8 op;
    Expr *pX = sqlite3ExprSkipCollateAndLikely(pExpr);
    testcase( pX!=pExpr );
    якщо( ALWAYS(pX)
     && (ExprHasProperty(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }інакше{
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
void sqlite3ExprCodeCopy(Parse *pParse, Expr *pExpr, ціле target){
  sqlite3 *db = pParse->db;
  pExpr = sqlite3ExprDup(db, pExpr, 0);
  якщо( !db->mallocFailed ) sqlite3ExprCode(pParse, pExpr, target);
  sqlite3ExprDelete(db, pExpr);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
void sqlite3ExprCodeFactorable(Parse *pParse, Expr *pExpr, ціле target){
  якщо( pParse->okConstFactor && sqlite3ExprIsConstantNotJoin(pParse,pExpr) ){
    sqlite3ExprCodeRunJustOnce(pParse, pExpr, target);
  }інакше{
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
ціле sqlite3ExprCodeExprList(
  Parse *pParse,     /* Parsing context */
  ExprList *pList,   /* The expression list to be coded */
  ціле target,        /* Where to write results */
  ціле srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  struct ExprList_item *pItem;
  ціле i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  assert( pList!=0 );
  assert( target>0 );
  assert( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->nExpr;
  якщо( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  для(pItem=pList->a, i=0; i<n; i++, pItem++){
    Expr *pExpr = pItem->pExpr;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    якщо( pItem->fg.bSorterRef ){
      i--;
      n--;
    }інакше
#endif
    якщо( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      якщо( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }інакше{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }інакше якщо( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3ExprIsConstantNotJoin(pParse,pExpr)
    ){
      sqlite3ExprCodeRunJustOnce(pParse, pExpr, target+i);
    }інакше{
      ціле inReg = sqlite3ExprCodeTarget(pParse, pExpr, target+i);
      якщо( inReg!=target+i ){
        VdbeOp *pOp;
        якщо( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }інакше{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  повернути n;
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
static void exprCodeBetween(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* The BETWEEN expression */
  ціле dest,         /* Jump destination or storage location */
  void (*xJump)(Parse*,Expr*,ціле,ціле), /* Action to take */
  ціле jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  Expr exprAnd;     /* The AND operator in  x>=y AND x<=z  */
  Expr compLeft;    /* The  x>=y  term */
  Expr compRight;   /* The  x<=z  term */
  ціле regFree1 = 0; /* Temporary use register */
  Expr *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&compLeft, 0, sizeof(Expr));
  memset(&compRight, 0, sizeof(Expr));
  memset(&exprAnd, 0, sizeof(Expr));

  assert( ExprUseXList(pExpr) );
  pDel = sqlite3ExprDup(db, pExpr->pLeft, 0);
  якщо( db->mallocFailed==0 ){
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
    якщо( xJump ){
      xJump(pParse, &exprAnd, dest, jumpIfNull);
    }інакше{
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
void sqlite3ExprIfTrue(Parse *pParse, Expr *pExpr, ціле dest, ціле jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ціле op = 0;
  ціле regFree1 = 0;
  ціле regFree2 = 0;
  ціле r1, r2;

  assert( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  якщо( NEVER(v==0) )     повернути;  /* Existence of VDBE checked by caller */
  якщо( NEVER(pExpr==0) ) повернути;  /* No way this can happen */
  assert( !ExprHasVVAProperty(pExpr, EP_Immutable) );
  op = pExpr->op;
  switch( op ){
    case TK_AND:
    case TK_OR: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      якщо( pAlt!=pExpr ){
        sqlite3ExprIfTrue(pParse, pAlt, dest, jumpIfNull);
      }інакше{
        Expr *pFirst, *pSecond;
        якщо( exprEvalRhsFirst(pExpr) ){
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        }інакше{
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        якщо( op==TK_AND ){
          ціле d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3ExprIfFalse(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }інакше{
          testcase( jumpIfNull==0 );
          sqlite3ExprIfTrue(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
        }
      }
      break;
    }
    case TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case TK_TRUTH: {
      ціле isNot;      /* IS NOT TRUE or IS NOT FALSE */
      ціле isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = pExpr->op2==TK_ISNOT;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      якщо( isTrue ^ isNot ){
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }інакше{
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      break;
    }
    case TK_IS:
    case TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      ціле addrIsNull;
      якщо( sqlite3ExprIsVector(pExpr->pLeft) ) goto default_expr;
      якщо( ExprHasProperty(pExpr, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull, ExprHasProperty(pExpr,EP_Commuted));
      assert(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      assert(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      assert(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      assert(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      assert(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      assert(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      якщо( addrIsNull ){
        якщо( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }інакше{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      assert( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      assert( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      assert( regFree1==0 || regFree1==r1 );
      якщо( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      break;
    }
    case TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfTrue, jumpIfNull);
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_IN: {
      ціле destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ціле destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      break;
    }
#endif
    default: {
    default_expr:
      якщо( ExprAlwaysTrue(pExpr) ){
        sqlite3VdbeGoto(v, dest);
      }інакше якщо( ExprAlwaysFalse(pExpr) ){
        /* No-op */
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      break;
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
void sqlite3ExprIfFalse(Parse *pParse, Expr *pExpr, ціле dest, ціле jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ціле op = 0;
  ціле regFree1 = 0;
  ціле regFree2 = 0;
  ціле r1, r2;

  assert( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  якщо( NEVER(v==0) ) повернути; /* Existence of VDBE checked by caller */
  якщо( pExpr==0 )    повернути;
  assert( !ExprHasVVAProperty(pExpr,EP_Immutable) );

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
  assert( pExpr->op!=TK_ISNULL || op==OP_NotNull );
  assert( pExpr->op!=TK_NOTNULL || op==OP_IsNull );
  assert( pExpr->op!=TK_NE || op==OP_Eq );
  assert( pExpr->op!=TK_EQ || op==OP_Ne );
  assert( pExpr->op!=TK_LT || op==OP_Ge );
  assert( pExpr->op!=TK_LE || op==OP_Gt );
  assert( pExpr->op!=TK_GT || op==OP_Le );
  assert( pExpr->op!=TK_GE || op==OP_Lt );

  switch( pExpr->op ){
    case TK_AND:
    case TK_OR: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      якщо( pAlt!=pExpr ){
        sqlite3ExprIfFalse(pParse, pAlt, dest, jumpIfNull);
      }інакше{
        Expr *pFirst, *pSecond;
        якщо( exprEvalRhsFirst(pExpr) ){
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        }інакше{
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        якщо( pExpr->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3ExprIfFalse(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
        }інакше{
          ціле d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3ExprIfTrue(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      break;
    }
    case TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case TK_TRUTH: {
      ціле isNot;   /* IS NOT TRUE or IS NOT FALSE */
      ціле isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = pExpr->op2==TK_ISNOT;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      якщо( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }інакше{
        /* IS FALSE and IS NOT TRUE */
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      break;
    }
    case TK_IS:
    case TK_ISNOT:
      testcase( pExpr->op==TK_IS );
      testcase( pExpr->op==TK_ISNOT );
      op = (pExpr->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      ціле addrIsNull;
      якщо( sqlite3ExprIsVector(pExpr->pLeft) ) goto default_expr;
      якщо( ExprHasProperty(pExpr, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = exprComputeOperands(pParse, pExpr,
                                   &r1, &r2, &regFree1, &regFree2);
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull,ExprHasProperty(pExpr,EP_Commuted));
      assert(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      assert(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      assert(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      assert(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      assert(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      assert(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      якщо( addrIsNull ){
        якщо( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }інакше{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      assert( regFree1==0 || regFree1==r1 );
      якщо( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      break;
    }
    case TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfFalse, jumpIfNull);
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_IN: {
      якщо( jumpIfNull ){
        sqlite3ExprCodeIN(pParse, pExpr, dest, dest);
      }інакше{
        ціле destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3ExprCodeIN(pParse, pExpr, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      break;
    }
#endif
    default: {
    default_expr:
      якщо( ExprAlwaysFalse(pExpr) ){
        sqlite3VdbeGoto(v, dest);
      }інакше якщо( ExprAlwaysTrue(pExpr) ){
        /* no-op */
      }інакше{
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      break;
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
void sqlite3ExprIfFalseDup(Parse *pParse, Expr *pExpr, ціле dest,ціле jumpIfNull){
  sqlite3 *db = pParse->db;
  Expr *pCopy = sqlite3ExprDup(db, pExpr, 0);
  якщо( db->mallocFailed==0 ){
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
static SQLITE_NOINLINE ціле exprCompareVariable(
  const Parse *pParse,
  const Expr *pVar,
  const Expr *pExpr
){
  ціле res = 2;
  ціле iVar;
  sqlite3_value *pL, *pR = 0;
 
  якщо( pExpr->op==TK_VARIABLE && pVar->iColumn==pExpr->iColumn ){
    повернути 0;
  }
  якщо( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) повернути 2;
  sqlite3ValueFromExpr(pParse->db, pExpr, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  якщо( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3VdbeGetBoundValue(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    якщо( pL ){
      якщо( sqlite3_value_type(pL)==SQLITE_TEXT ){
        sqlite3_value_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3ValueFree(pR);
    sqlite3ValueFree(pL);
  }
  повернути res;
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
ціле sqlite3ExprCompare(
  const Parse *pParse,
  const Expr *pA,
  const Expr *pB,
  ціле iTab
){
  u32 combinedFlags;
  якщо( pA==0 || pB==0 ){
    повернути pB==pA ? 0 : 2;
  }
  якщо( pParse && pA->op==TK_VARIABLE ){
    повернути exprCompareVariable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  якщо( combinedFlags & EP_IntValue ){
    якщо( (pA->flags&pB->flags&EP_IntValue)!=0 && pA->u.iValue==pB->u.iValue ){
      повернути 0;
    }
    повернути 2;
  }
  якщо( pA->op!=pB->op || pA->op==TK_RAISE ){
    якщо( pA->op==TK_COLLATE && sqlite3ExprCompare(pParse, pA->pLeft,pB,iTab)<2 ){
      повернути 1;
    }
    якщо( pB->op==TK_COLLATE && sqlite3ExprCompare(pParse, pA,pB->pLeft,iTab)<2 ){
      повернути 1;
    }
    якщо( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }інакше{
      повернути 2;
    }
  }
  assert( !ExprHasProperty(pA, EP_IntValue) );
  assert( !ExprHasProperty(pB, EP_IntValue) );
  якщо( pA->u.zToken ){
    якщо( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      якщо( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) повернути 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      assert( pA->op==pB->op );
      якщо( ExprHasProperty(pA,EP_WinFunc)!=ExprHasProperty(pB,EP_WinFunc) ){
        повернути 2;
      }
      якщо( ExprHasProperty(pA,EP_WinFunc) ){
        якщо( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          повернути 2;
        }
      }
#endif
    }інакше якщо( pA->op==TK_NULL ){
      повернути 0;
    }інакше якщо( pA->op==TK_COLLATE ){
      якщо( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) повернути 2;
    }інакше
    якщо( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      повернути 2;
    }
  }
  якщо( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) повернути 2;
  якщо( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    якщо( combinedFlags & EP_xIsSelect ) повернути 2;
    якщо( (combinedFlags & EP_FixedCol)==0
     && sqlite3ExprCompare(pParse, pA->pLeft, pB->pLeft, iTab) ) повернути 2;
    якщо( sqlite3ExprCompare(pParse, pA->pRight, pB->pRight, iTab) ) повернути 2;
    якщо( sqlite3ExprListCompare(pA->x.pList, pB->x.pList, iTab) ) повернути 2;
    якщо( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      якщо( pA->iColumn!=pB->iColumn ) повернути 2;
      якщо( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) повернути 2;
      якщо( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        повернути 2;
      }
    }
  }
  повернути 0;
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
ціле sqlite3ExprListCompare(const ExprList *pA, const ExprList *pB, ціле iTab){
  ціле i;
  якщо( pA==0 && pB==0 ) повернути 0;
  якщо( pA==0 || pB==0 ) повернути 1;
  якщо( pA->nExpr!=pB->nExpr ) повернути 1;
  для(i=0; i<pA->nExpr; i++){
    ціле res;
    Expr *pExprA = pA->a[i].pExpr;
    Expr *pExprB = pB->a[i].pExpr;
    якщо( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) повернути 1;
    якщо( (res = sqlite3ExprCompare(0, pExprA, pExprB, iTab)) ) повернути res;
  }
  повернути 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
ціле sqlite3ExprCompareSkip(Expr *pA,Expr *pB, ціле iTab){
  повернути sqlite3ExprCompare(0,
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
static ціле exprImpliesNotNull(
  const Parse *pParse,/* Parsing context */
  const Expr *p,      /* The expression to be checked */
  const Expr *pNN,    /* The expression that is NOT NULL */
  ціле iTab,           /* Table being evaluated */
  ціле seenNot         /* Return true only if p can be any non-NULL value */
){
  assert( p );
  assert( pNN );
  якщо( sqlite3ExprCompare(pParse, p, pNN, iTab)==0 ){
    повернути pNN->op!=TK_NULL;
  }
  switch( p->op ){
    case TK_IN: {
      якщо( seenNot && ExprHasProperty(p, EP_xIsSelect) ) повернути 0;
      assert( ExprUseXSelect(p) || (p->x.pList!=0 && p->x.pList->nExpr>0) );
      повернути exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case TK_BETWEEN: {
      ExprList *pList;
      assert( ExprUseXList(p) );
      pList = p->x.pList;
      assert( pList!=0 );
      assert( pList->nExpr==2 );
      якщо( seenNot ) повернути 0;
      якщо( exprImpliesNotNull(pParse, pList->a[0].pExpr, pNN, iTab, 1)
       || exprImpliesNotNull(pParse, pList->a[1].pExpr, pNN, iTab, 1)
      ){
        повернути 1;
      }
      повернути exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case TK_EQ:
    case TK_NE:
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_PLUS:
    case TK_MINUS:
    case TK_BITOR:
    case TK_LSHIFT:
    case TK_RSHIFT:
    case TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    case TK_STAR:
    case TK_REM:
    case TK_BITAND:
    case TK_SLASH: {
      якщо( exprImpliesNotNull(pParse, p->pRight, pNN, iTab, seenNot) ) повернути 1;
      /* no break */ deliberate_fall_through
    }
    case TK_SPAN:
    case TK_COLLATE:
    case TK_UPLUS:
    case TK_UMINUS: {
      повернути exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, seenNot);
    }
    case TK_TRUTH: {
      якщо( seenNot ) повернути 0;
      якщо( p->op2!=TK_IS ) повернути 0;
      повернути exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case TK_BITNOT:
    case TK_NOT: {
      повернути exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
  }
  повернути 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
static ціле sqlite3ExprIsNotTrue(Expr *pExpr){
  ціле v;
  якщо( pExpr->op==TK_NULL ) повернути 1;
  якщо( pExpr->op==TK_TRUEFALSE && sqlite3ExprTruthValue(pExpr)==0 ) повернути 1;
  v = 1;
  якщо( sqlite3ExprIsInteger(pExpr, &v, 0, 0) && v==0 ) повернути 1;
  повернути 0;
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
static ціле sqlite3ExprIsIIF(sqlite3 *db, const Expr *pExpr){
  ExprList *pList;
  якщо( pExpr->op==TK_FUNCTION ){
    const char *z = pExpr->u.zToken;
    FuncDef *pDef;
    якщо( (z[0]!='i' && z[0]!='I') ) повернути 0;
    якщо( pExpr->x.pList==0 ) повернути 0;
    pDef = sqlite3FindFunction(db, z, pExpr->x.pList->nExpr, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    якщо( pDef==0 ) повернути 0;
#інакше
    якщо( NEVER(pDef==0) ) повернути 0;
#endif
    якщо( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) повернути 0;
    якщо( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) повернути 0;
  }інакше якщо( pExpr->op==TK_CASE ){
    якщо( pExpr->pLeft!=0 ) повернути 0;
  }інакше{
    повернути 0;
  }
  pList = pExpr->x.pList;
  assert( pList!=0 );
  якщо( pList->nExpr==2 ) повернути 1;
  якщо( pList->nExpr==3 && sqlite3ExprIsNotTrue(pList->a[2].pExpr) ) повернути 1;
  повернути 0;
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
ціле sqlite3ExprImpliesExpr(
  const Parse *pParse,
  const Expr *pE1,
  const Expr *pE2,
  ціле iTab
){
  якщо( sqlite3ExprCompare(pParse, pE1, pE2, iTab)==0 ){
    повернути 1;
  }
  якщо( pE2->op==TK_OR
   && (sqlite3ExprImpliesExpr(pParse, pE1, pE2->pLeft, iTab)
             || sqlite3ExprImpliesExpr(pParse, pE1, pE2->pRight, iTab) )
  ){
    повернути 1;
  }
  якщо( pE2->op==TK_NOTNULL
   && exprImpliesNotNull(pParse, pE1, pE2->pLeft, iTab, 0)
  ){
    повернути 1;
  }
  якщо( sqlite3ExprIsIIF(pParse->db, pE1) ){
    повернути sqlite3ExprImpliesExpr(pParse,pE1->x.pList->a[0].pExpr,pE2,iTab);
  }
  повернути 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
static void bothImplyNotNullRow(Walker *pWalker, Expr *pE1, Expr *pE2){
  якщо( pWalker->eCode==0 ){
    sqlite3WalkExpr(pWalker, pE1);
    якщо( pWalker->eCode ){
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
static ціле impliesNotNullRow(Walker *pWalker, Expr *pExpr){
  testcase( pExpr->op==TK_AGG_COLUMN );
  testcase( pExpr->op==TK_AGG_FUNCTION );
  якщо( ExprHasProperty(pExpr, EP_OuterON) ) повернути WRC_Prune;
  якщо( ExprHasProperty(pExpr, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    повернути WRC_Prune;
  }
  switch( pExpr->op ){
    case TK_ISNOT:
    case TK_ISNULL:
    case TK_NOTNULL:
    case TK_IS:
    case TK_VECTOR:
    case TK_FUNCTION:
    case TK_TRUTH:
    case TK_CASE:
      testcase( pExpr->op==TK_ISNOT );
      testcase( pExpr->op==TK_ISNULL );
      testcase( pExpr->op==TK_NOTNULL );
      testcase( pExpr->op==TK_IS );
      testcase( pExpr->op==TK_VECTOR );
      testcase( pExpr->op==TK_FUNCTION );
      testcase( pExpr->op==TK_TRUTH );
      testcase( pExpr->op==TK_CASE );
      повернути WRC_Prune;

    case TK_COLUMN:
      якщо( pWalker->u.iCur==pExpr->iTable ){
        pWalker->eCode = 1;
        повернути WRC_Abort;
      }
      повернути WRC_Prune;

    case TK_OR:
    case TK_AND:
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
      повернути WRC_Prune;
       
    case TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      якщо( ExprUseXList(pExpr) && ALWAYS(pExpr->x.pList->nExpr>0) ){
        sqlite3WalkExpr(pWalker, pExpr->pLeft);
      }
      повернути WRC_Prune;

    case TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      assert( ExprUseXList(pExpr) );
      assert( pExpr->x.pList->nExpr==2 );
      sqlite3WalkExpr(pWalker, pExpr->pLeft);
      bothImplyNotNullRow(pWalker, pExpr->x.pList->a[0].pExpr,
                                   pExpr->x.pList->a[1].pExpr);
      повернути WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    case TK_EQ:
    case TK_NE:
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE: {
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
      assert( pLeft->op!=TK_COLUMN || ExprUseYTab(pLeft) );
      assert( pRight->op!=TK_COLUMN || ExprUseYTab(pRight) );
      якщо( (pLeft->op==TK_COLUMN
           && ALWAYS(pLeft->y.pTab!=0)
           && IsVirtual(pLeft->y.pTab))
       || (pRight->op==TK_COLUMN
           && ALWAYS(pRight->y.pTab!=0)
           && IsVirtual(pRight->y.pTab))
      ){
        повернути WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    default:
      повернути WRC_Continue;
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
ціле sqlite3ExprImpliesNonNullRow(Expr *p, ціле iTab, ціле isRJ){
  Walker w;
  p = sqlite3ExprSkipCollateAndLikely(p);
  якщо( p==0 ) повернути 0;
  якщо( p->op==TK_NOTNULL ){
    p = p->pLeft;
  }інакше{
    поки( p->op==TK_AND ){
      якщо( sqlite3ExprImpliesNonNullRow(p->pLeft, iTab, isRJ) ) повернути 1;
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
  повернути w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
struct IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  ціле iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
static ціле exprIdxCover(Walker *pWalker, Expr *pExpr){
  якщо( pExpr->op==TK_COLUMN
   && pExpr->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, pExpr->iColumn)<0
  ){
    pWalker->eCode = 1;
    повернути WRC_Abort;
  }
  повернути WRC_Continue;
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
ціле sqlite3ExprCoveredByIndex(
  Expr *pExpr,        /* The index to be tested */
  ціле iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  struct IdxCover xcov;
  memset(&w, 0, sizeof(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.xExprCallback = exprIdxCover;
  w.u.pIdxCover = &xcov;
  sqlite3WalkExpr(&w, pExpr);
  повернути !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
struct RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  ціле *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
static ціле selectRefEnter(Walker *pWalker, Select *pSelect){
  struct RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  ціле *piNew;
  якщо( pSrc->nSrc==0 ) повернути WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*sizeof(ціле));
  якщо( piNew==0 ){
    p->nExclude = 0;
    повернути WRC_Abort;
  }інакше{
    p->aiExclude = piNew;
  }
  для(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  повернути WRC_Continue;
}
static void selectRefLeave(Walker *pWalker, Select *pSelect){
  struct RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  якщо( p->nExclude ){
    assert( p->nExclude>=pSrc->nSrc );
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
static ціле exprRefToSrcList(Walker *pWalker, Expr *pExpr){
  якщо( pExpr->op==TK_COLUMN
   || pExpr->op==TK_AGG_COLUMN
  ){
    ціле i;
    struct RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    ціле nSrc = pSrc ? pSrc->nSrc : 0;
    для(i=0; i<nSrc; i++){
      якщо( pExpr->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        повернути WRC_Continue;
      }
    }
    для(i=0; i<p->nExclude && p->aiExclude[i]!=pExpr->iTable; i++){}
    якщо( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  повернути WRC_Continue;
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
ціле sqlite3ReferencesSrcList(Parse *pParse, Expr *pExpr, SrcList *pSrcList){
  Walker w;
  struct RefSrcList x;
  assert( pParse->db!=0 );
  memset(&w, 0, sizeof(w));
  memset(&x, 0, sizeof(x));
  w.xExprCallback = exprRefToSrcList;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  assert( pExpr->op==TK_AGG_FUNCTION );
  assert( ExprUseXList(pExpr) );
  sqlite3WalkExprList(&w, pExpr->x.pList);
  якщо( pExpr->pLeft ){
    assert( pExpr->pLeft->op==TK_ORDER );
    assert( ExprUseXList(pExpr->pLeft) );
    assert( pExpr->pLeft->x.pList!=0 );
    sqlite3WalkExprList(&w, pExpr->pLeft->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  якщо( ExprHasProperty(pExpr, EP_WinFunc) ){
    sqlite3WalkExpr(&w, pExpr->y.pWin->pFilter);
  }
#endif
  якщо( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  якщо( w.eCode & 0x01 ){
    повернути 1;
  }інакше якщо( w.eCode ){
    повернути 0;
  }інакше{
    повернути -1;
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
static ціле agginfoPersistExprCb(Walker *pWalker, Expr *pExpr){
  якщо( ALWAYS(!ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced))
   && pExpr->pAggInfo!=0
  ){
    AggInfo *pAggInfo = pExpr->pAggInfo;
    ціле iAgg = pExpr->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    assert( iAgg>=0 );
    якщо( pExpr->op!=TK_AGG_FUNCTION ){
      якщо( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==pExpr
      ){
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        якщо( pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr) ){
          pAggInfo->aCol[iAgg].pCExpr = pExpr;
        }
      }
    }інакше{
      assert( pExpr->op==TK_AGG_FUNCTION );
      якщо( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==pExpr
      ){
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        якщо( pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr) ){
          pAggInfo->aFunc[iAgg].pFExpr = pExpr;
        }
      }
    }
  }
  повернути WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
void sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, sizeof(*pWalker));
  pWalker->pParse = pParse;
  pWalker->xExprCallback = agginfoPersistExprCb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static ціле addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  ціле i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       sizeof(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  повернути i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static ціле addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  ціле i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       sizeof(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  повернути i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
static void findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  Expr *pExpr          /* Expr describing the column to find or insert */
){
  struct AggInfo_col *pCol;
  ціле k;
  ціле mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  assert( mxTerm <= SMXV(i16) );
  assert( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  для(k=0; k<pAggInfo->nColumn; k++, pCol++){
    якщо( pCol->pCExpr==pExpr ) повернути;
    якщо( pCol->iTable==pExpr->iTable
     && pCol->iColumn==pExpr->iColumn
     && pExpr->op!=TK_IF_NULL_ROW
    ){
      goto fix_up_expr;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  якщо( k<0 ){
    /* OOM on resize */
    assert( pParse->db->mallocFailed );
    повернути;
  }
  якщо( k>mxTerm ){
    sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  assert( ExprUseYTab(pExpr) );
  pCol->pTab = pExpr->y.pTab;
  pCol->iTable = pExpr->iTable;
  pCol->iColumn = pExpr->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = pExpr;
  якщо( pAggInfo->pGroupBy && pExpr->op!=TK_IF_NULL_ROW ){
    ціле j, n;
    ExprList *pGB = pAggInfo->pGroupBy;
    struct ExprList_item *pTerm = pGB->a;
    n = pGB->nExpr;
    для(j=0; j<n; j++, pTerm++){
      Expr *pE = pTerm->pExpr;
      якщо( pE->op==TK_COLUMN
       && pE->iTable==pExpr->iTable
       && pE->iColumn==pExpr->iColumn
      ){
        pCol->iSorterColumn = j;
        break;
      }
    }
  }
  якщо( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_expr:
  ExprSetVVAProperty(pExpr, EP_NoReduce);
  assert( pExpr->pAggInfo==0 || pExpr->pAggInfo==pAggInfo );
  pExpr->pAggInfo = pAggInfo;
  якщо( pExpr->op==TK_COLUMN ){
    pExpr->op = TK_AGG_COLUMN;
  }
  assert( k <= SMXV(pExpr->iAgg) );
  pExpr->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
static ціле analyzeAggregate(Walker *pWalker, Expr *pExpr){
  ціле i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  assert( pNC->ncFlags & NC_UAggInfo );
  assert( pAggInfo->iFirstReg==0 );
  switch( pExpr->op ){
    default: {
      IndexedExpr *pIEpr;
      Expr tmp;
      assert( pParse->iSelfTab==0 );
      якщо( (pNC->ncFlags & NC_InAggFunc)==0 ) break;
      якщо( pParse->pIdxEpr==0 ) break;
      для(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        ціле iDataCur = pIEpr->iDataCur;
        якщо( iDataCur<0 ) continue;
        якщо( sqlite3ExprCompare(0, pExpr, pIEpr->pExpr, iDataCur)==0 ) break;
      }
      якщо( pIEpr==0 ) break;
      якщо( NEVER(!ExprUseYTab(pExpr)) ) break;
      для(i=0; i<pSrcList->nSrc; i++){
         якщо( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           break;
         }
      }
      якщо( i>=pSrcList->nSrc ) break;
      якщо( NEVER(pExpr->pAggInfo!=0) ) break; /* Resolved by outer context */
      якщо( pParse->nErr ){ повернути WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, sizeof(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      якщо( pParse->nErr ){ повернути WRC_Abort; }
      assert( pAggInfo->aCol!=0 );
      assert( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = pExpr;
      pExpr->pAggInfo = pAggInfo;
      pExpr->iAgg = tmp.iAgg;
      повернути WRC_Prune;
    }
    case TK_IF_NULL_ROW:
    case TK_AGG_COLUMN:
    case TK_COLUMN: {
      testcase( pExpr->op==TK_AGG_COLUMN );
      testcase( pExpr->op==TK_COLUMN );
      testcase( pExpr->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      якщо( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        для(i=0; i<pSrcList->nSrc; i++, pItem++){
          assert( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
          якщо( pExpr->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, pExpr);
            break;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      повернути WRC_Continue;
    }
    case TK_AGG_FUNCTION: {
      якщо( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==pExpr->op2
       && pExpr->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        struct AggInfo_func *pItem = pAggInfo->aFunc;
        ціле mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        assert( mxTerm <= SMXV(i16) );
        для(i=0; i<pAggInfo->nFunc; i++, pItem++){
          якщо( NEVER(pItem->pFExpr==pExpr) ) break;
          якщо( sqlite3ExprCompare(0, pItem->pFExpr, pExpr, -1)==0 ){
            break;
          }
        }
        якщо( i>mxTerm ){
          sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          assert( i<pAggInfo->nFunc );
        }інакше якщо( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          якщо( i>=0 ){
            ціле nArg;
            assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = pExpr;
            assert( ExprUseUToken(pExpr) );
            nArg = pExpr->x.pList ? pExpr->x.pList->nExpr : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         pExpr->u.zToken, nArg, enc, 0);
            assert( pItem->bOBUnique==0 );
            якщо( pExpr->pLeft
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ExprList *pOBList;
              assert( nArg>0 );
              assert( pExpr->pLeft->op==TK_ORDER );
              assert( ExprUseXList(pExpr->pLeft) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = pExpr->pLeft->x.pList;
              assert( pOBList->nExpr>0 );
              assert( pItem->bOBUnique==0 );
              якщо( pOBList->nExpr==1
               && nArg==1
               && sqlite3ExprCompare(0,pOBList->a[0].pExpr,
                               pExpr->x.pList->a[0].pExpr,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = ExprHasProperty(pExpr, EP_Distinct);
              }інакше{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }інакше{
              pItem->iOBTab = -1;
            }
            якщо( ExprHasProperty(pExpr, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }інакше{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        assert( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
        ExprSetVVAProperty(pExpr, EP_NoReduce);
        assert( i <= SMXV(pExpr->iAgg) );
        pExpr->iAgg = (i16)i;
        pExpr->pAggInfo = pAggInfo;
        повернути WRC_Prune;
      }інакше{
        повернути WRC_Continue;
      }
    }
  }
  повернути WRC_Continue;
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
void sqlite3ExprAnalyzeAggregates(NameContext *pNC, Expr *pExpr){
  Walker w;
  w.xExprCallback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  assert( pNC->pSrcList!=0 );
  sqlite3WalkExpr(&w, pExpr);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
void sqlite3ExprAnalyzeAggList(NameContext *pNC, ExprList *pList){
  struct ExprList_item *pItem;
  ціле i;
  якщо( pList ){
    для(pItem=pList->a, i=0; i<pList->nExpr; i++, pItem++){
      sqlite3ExprAnalyzeAggregates(pNC, pItem->pExpr);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
ціле sqlite3GetTempReg(Parse *pParse){
  якщо( pParse->nTempReg==0 ){
    повернути ++pParse->nMem;
  }
  повернути pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
void sqlite3ReleaseTempReg(Parse *pParse, ціле iReg){
  якщо( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    ціле kk;
    для(kk=0; kk<pParse->nTempReg; kk++){
      assert( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    якщо( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
ціле sqlite3GetTempRange(Parse *pParse, ціле nReg){
  ціле i, n;
  якщо( nReg==1 ) повернути sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  якщо( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }інакше{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  повернути i;
}
void sqlite3ReleaseTempRange(Parse *pParse, ціле iReg, ціле nReg){
  якщо( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    повернути;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  якщо( nReg>pParse->nRangeReg ){
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
void sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
void sqlite3TouchRegister(Parse *pParse, ціле iReg){
  якщо( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#якщо defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
ціле sqlite3FirstAvailableRegister(Parse *pParse, ціле iMin){
  const ExprList *pList = pParse->pConstExpr;
  якщо( pList ){
    ціле i;
    для(i=0; i<pList->nExpr; i++){
      якщо( pList->a[i].u.iConstExprReg>=iMin ){
        iMin = pList->a[i].u.iConstExprReg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  повернути iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
ціле sqlite3NoTempsInRange(Parse *pParse, ціле iFirst, ціле iLast){
  ціле i;
  якщо( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     повернути 0;
  }
  для(i=0; i<pParse->nTempReg; i++){
    якщо( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      повернути 0;
    }
  }
  якщо( pParse->pConstExpr ){
    ExprList *pList = pParse->pConstExpr;
    для(i=0; i<pList->nExpr; i++){
      ціле iReg = pList->a[i].u.iConstExprReg;
      якщо( iReg==0 ) continue;
      якщо( iReg>=iFirst && iReg<=iLast ) повернути 0;
    }
  }
  повернути 1;
}
#endif /* SQLITE_DEBUG */
