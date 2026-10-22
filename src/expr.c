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
#شامل "sqliteInt.h"

/* Forward declarations */
ایستا تهی عبارت_Code_Between(Parse*,عبارت*,صحیح,تهی(*)(Parse*,عبارت*,صحیح,صحیح),صحیح);
ایستا صحیح عبارت_Code_Vector(Parse *pParse, عبارت *p, صحیح *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
نویسه sqlite3TableColumnAffinity(ثابت Table *pTab, صحیح iCol){
  اگر( iCol<0 || NEVER(iCol>=pTab->nCol) ) بازگشت SQLITE_AFF_INTEGER;
  بازگشت pTab->aCol[iCol].affinity;
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
نویسه sqlite3_عبارت_Affinity(ثابت عبارت *p_عبارت){
  صحیح op;
  op = p_عبارت->op;
  وقتی( 1 /* exit-by-break */ ){
    اگر( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_عبارت->y.pTab!=0) ){
      أكد( عبارت_Use_YTab(p_عبارت) );
      أكد( p_عبارت->y.pTab!=0 );
      بازگشت sqlite3TableColumnAffinity(p_عبارت->y.pTab, p_عبارت->iColumn);
    }
    اگر( op==TK_SELECT ){
      أكد( عبارت_Use_XSelect(p_عبارت) );
      أكد( p_عبارت->x.pSelect!=0 );
      أكد( p_عبارت->x.pSelect->pEList!=0 );
      أكد( p_عبارت->x.pSelect->pEList->a[0].p_عبارت!=0 );
      بازگشت sqlite3_عبارت_Affinity(p_عبارت->x.pSelect->pEList->a[0].p_عبارت);
    }
#ifndef SQLITE_OMIT_CAST
    اگر( op==TK_CAST ){
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      بازگشت sqlite3AffinityType(p_عبارت->u.zToken, 0);
    }
#endif
    اگر( op==TK_SELECT_COLUMN ){
      أكد( p_عبارت->p_چپ!=0 && عبارت_Use_XSelect(p_عبارت->p_چپ) );
      أكد( p_عبارت->iColumn < p_عبارت->iTable );
      أكد( p_عبارت->iColumn >= 0 );
      أكد( p_عبارت->iTable==p_عبارت->p_چپ->x.pSelect->pEList->n_عبارت );
      بازگشت sqlite3_عبارت_Affinity(
          p_عبارت->p_چپ->x.pSelect->pEList->a[p_عبارت->iColumn].p_عبارت
      );
    }
    اگر( op==TK_VECTOR
     || (op==TK_FUNCTION && p_عبارت->aff_عبارت==SQLITE_AFF_DEFER)
    ){
      أكد( عبارت_Use_XList(p_عبارت) );
      بازگشت sqlite3_عبارت_Affinity(p_عبارت->x.pList->a[0].p_عبارت);
    }
    اگر( عبارت_Has_Property(p_عبارت, EP_Skip|EP_IfNullRow) ){
      أكد( p_عبارت->op==TK_COLLATE
           || p_عبارت->op==TK_IF_NULL_ROW
           || (p_عبارت->op==TK_REGISTER && p_عبارت->op2==TK_IF_NULL_ROW) );
      p_عبارت = p_عبارت->p_چپ;
      op = p_عبارت->op;
      ادامه;
    }
    اگر( op!=TK_REGISTER ) بشکن;
    op = p_عبارت->op2;
    اگر( NEVER( op==TK_REGISTER ) ) بشکن;
  }
  بازگشت p_عبارت->aff_عبارت;
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
صحیح sqlite3_عبارت_Data_Type(ثابت عبارت *p_عبارت){
  وقتی( p_عبارت ){
    انتخاب( p_عبارت->op ){
      حالت TK_COLLATE:
      حالت TK_IF_NULL_ROW:
      حالت TK_UPLUS:  {
        p_عبارت = p_عبارت->p_چپ;
        بشکن;
      }
      حالت TK_NULL: {
        p_عبارت = 0;
        بشکن;
      }
      حالت TK_STRING: {
        بازگشت 0x02;
      }
      حالت TK_BLOB: {
        بازگشت 0x04;
      }
      حالت TK_CONCAT: {
        بازگشت 0x06;
      }
      حالت TK_VARIABLE:
      حالت TK_AGG_FUNCTION:
      حالت TK_FUNCTION: {
        بازگشت 0x07;
      }
      حالت TK_COLUMN:
      حالت TK_AGG_COLUMN:
      حالت TK_SELECT:
      حالت TK_CAST:
      حالت TK_SELECT_COLUMN:
      حالت TK_VECTOR:  {
        صحیح aff = sqlite3_عبارت_Affinity(p_عبارت);
        اگر( aff>=SQLITE_AFF_NUMERIC ) بازگشت 0x05;
        اگر( aff==SQLITE_AFF_TEXT )    بازگشت 0x06;
        بازگشت 0x07;
      }
      حالت TK_CASE: {
        صحیح res = 0;
        صحیح ii;
        عبارت_List *pList = p_عبارت->x.pList;
        أكد( عبارت_Use_XList(p_عبارت) && pList!=0 );
        أكد( pList->n_عبارت > 0);
        برای(ii=1; ii<pList->n_عبارت; ii+=2){
          res |= sqlite3_عبارت_Data_Type(pList->a[ii].p_عبارت);
        }
        اگر( pList->n_عبارت % 2 ){
          res |= sqlite3_عبارت_Data_Type(pList->a[pList->n_عبارت-1].p_عبارت);
        }
        بازگشت res;
      }
      پیشفرض: {
        بازگشت 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  بازگشت 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
عبارت *sqlite3_عبارت_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  عبارت *p_عبارت,             /* Add the "COLLATE" clause to this expression */
  ثابت Token *pCollName,  /* Name of collating sequence */
  صحیح dequote              /* True to dequote pCollName */
){
  اگر( pCollName->n>0 && pParse->nErr==0 ){
    عبارت *pNew = sqlite3_عبارت_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( p_عبارت!=0 );
    اگر( pNew ){
      pNew->p_چپ = p_عبارت;
      pNew->flags |= EP_Collate|EP_Skip;
#اگر SQLITE_MAX_عبارت_DEPTH>0
      pNew->nHeight = p_عبارت->nHeight+1;
      sqlite3_عبارت_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_عبارت = pNew;
    }
  }
  بازگشت p_عبارت;
}
عبارت *sqlite3_عبارت_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  عبارت *p_عبارت,          /* Add the "COLLATE" clause to this expression */
  ثابت نویسه *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (نویسه*)zC);
  بازگشت sqlite3_عبارت_Add_Collate_Token(pParse, p_عبارت, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
عبارت *sqlite3_عبارت_Skip_Collate(عبارت *p_عبارت){
  وقتی( p_عبارت && عبارت_Has_Property(p_عبارت, EP_Skip) ){
    أكد( p_عبارت->op==TK_COLLATE );
    p_عبارت = p_عبارت->p_چپ;
  }  
  بازگشت p_عبارت;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
عبارت *sqlite3_عبارت_Skip_Collate_And_Likely(عبارت *p_عبارت){
  وقتی( p_عبارت && عبارت_Has_Property(p_عبارت, EP_Skip|EP_Unlikely) ){
    اگر( عبارت_Has_Property(p_عبارت, EP_Unlikely) ){
      أكد( عبارت_Use_XList(p_عبارت) );
      أكد( p_عبارت->x.pList->n_عبارت>0 );
      أكد( p_عبارت->op==TK_FUNCTION );
      p_عبارت = p_عبارت->x.pList->a[0].p_عبارت;
    }وگرنه اگر( p_عبارت->op==TK_COLLATE ){
      p_عبارت = p_عبارت->p_چپ;
    }وگرنه{
      بشکن;
    }
  }  
  بازگشت p_عبارت;
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
CollSeq *sqlite3_عبارت_Coll_Seq(Parse *pParse, ثابت عبارت *p_عبارت){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ثابت عبارت *p = p_عبارت;
  وقتی( p ){
    صحیح op = p->op;
    اگر( op==TK_REGISTER ) op = p->op2;
    اگر( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      صحیح j;
      أكد( عبارت_Use_YTab(p) );
      أكد( p->y.pTab!=0 );
      اگر( (j = p->iColumn)>=0 ){
        ثابت نویسه *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      بشکن;
    }
    اگر( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_چپ;
      ادامه;
    }
    اگر( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_عبارت==SQLITE_AFF_DEFER)
    ){
      أكد( عبارت_Use_XList(p) );
      p = p->x.pList->a[0].p_عبارت;
      ادامه;
    }
    اگر( op==TK_COLLATE ){
      أكد( !عبارت_Has_Property(p, EP_Int_مقدار) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      بشکن;
    }
    اگر( p->flags & EP_Collate ){
      اگر( p->p_چپ && (p->p_چپ->flags & EP_Collate)!=0 ){
        p = p->p_چپ;
      }وگرنه{
        عبارت *pNext  = p->p_راست;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !عبارت_Use_XList(p) || p->x.pList==0 || p->p_راست==0 );
        اگر( عبارت_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحیح i;
          برای(i=0; i<p->x.pList->n_عبارت; i++){
            اگر( عبارت_Has_Property(p->x.pList->a[i].p_عبارت, EP_Collate) ){
              pNext = p->x.pList->a[i].p_عبارت;
              بشکن;
            }
          }
        }
        p = pNext;
      }
    }وگرنه{
      بشکن;
    }
  }
  اگر( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  بازگشت pColl;
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
CollSeq *sqlite3_عبارت_NNColl_Seq(Parse *pParse, ثابت عبارت *p_عبارت){
  CollSeq *p = sqlite3_عبارت_Coll_Seq(pParse, p_عبارت);
  اگر( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  بازگشت p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحیح sqlite3_عبارت_Coll_Seq_Match(Parse *pParse, ثابت عبارت *pE1, ثابت عبارت *pE2){
  CollSeq *pColl1 = sqlite3_عبارت_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_عبارت_NNColl_Seq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  بازگشت pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
نویسه sqlite3CompareAffinity(ثابت عبارت *p_عبارت, نویسه aff2){
  نویسه aff1 = sqlite3_عبارت_Affinity(p_عبارت);
  اگر( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    اگر( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      بازگشت SQLITE_AFF_NUMERIC;
    }وگرنه{
      بازگشت SQLITE_AFF_BLOB;
    }
  }وگرنه{
    /* One side is a column, the other is not. Use the columns affinity. */
    أكد( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    بازگشت (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ایستا نویسه comparisonAffinity(ثابت عبارت *p_عبارت){
  نویسه aff;
  أكد( p_عبارت->op==TK_EQ || p_عبارت->op==TK_IN || p_عبارت->op==TK_LT ||
          p_عبارت->op==TK_GT || p_عبارت->op==TK_GE || p_عبارت->op==TK_LE ||
          p_عبارت->op==TK_NE || p_عبارت->op==TK_IS || p_عبارت->op==TK_ISNOT );
  أكد( p_عبارت->p_چپ );
  أكد( !عبارت_Use_XSelect(p_عبارت) );
  aff = sqlite3_عبارت_Affinity(p_عبارت->p_چپ);
  اگر( p_عبارت->p_راست ){
    aff = sqlite3CompareAffinity(p_عبارت->p_راست, aff);
  }وگرنه اگر( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  بازگشت aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
صحیح sqlite3IndexAffinityOk(ثابت عبارت *p_عبارت, نویسه idx_affinity){
  نویسه aff = comparisonAffinity(p_عبارت);
  اگر( aff<SQLITE_AFF_TEXT ){
    بازگشت 1;
  }
  اگر( aff==SQLITE_AFF_TEXT ){
    بازگشت idx_affinity==SQLITE_AFF_TEXT;
  }
  بازگشت sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ایستا u8 دودویی_Compare_P5(
  ثابت عبارت *pExpr1,   /* Left operand */
  ثابت عبارت *pExpr2,   /* Right operand */
  صحیح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (نویسه)sqlite3_عبارت_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  بازگشت aff;
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
CollSeq *sqlite3_دودویی_Compare_Coll_Seq(
  Parse *pParse,
  ثابت عبارت *p_چپ,
  ثابت عبارت *p_راست
){
  CollSeq *pColl;
  أكد( p_چپ );
  اگر( p_چپ->flags & EP_Collate ){
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_چپ);
  }وگرنه اگر( p_راست && (p_راست->flags & EP_Collate)!=0 ){
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_راست);
  }وگرنه{
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_چپ);
    اگر( !pColl ){
      pColl = sqlite3_عبارت_Coll_Seq(pParse, p_راست);
    }
  }
  بازگشت pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_عبارت_Compare_Coll_Seq(Parse *pParse, ثابت عبارت *p){
  اگر( عبارت_Has_Property(p, EP_Commuted) ){
    بازگشت sqlite3_دودویی_Compare_Coll_Seq(pParse, p->p_راست, p->p_چپ);
  }وگرنه{
    بازگشت sqlite3_دودویی_Compare_Coll_Seq(pParse, p->p_چپ, p->p_راست);
  }
}

/*
** Generate code for a comparison operator.
*/
ایستا صحیح codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  عبارت *p_چپ,      /* The left operand */
  عبارت *p_راست,     /* The right operand */
  صحیح opcode,       /* The comparison opcode */
  صحیح in1, صحیح in2, /* Register holding operands */
  صحیح dest,         /* Jump here if true.  */
  صحیح jumpIfNull,   /* If true, jump if either operand is NULL */
  صحیح isCommuted    /* The comparison has been commuted */
){
  صحیح p5;
  صحیح addr;
  CollSeq *p4;

  اگر( pParse->nErr ) بازگشت 0;
  اگر( isCommuted ){
    p4 = sqlite3_دودویی_Compare_Coll_Seq(pParse, p_راست, p_چپ);
  }وگرنه{
    p4 = sqlite3_دودویی_Compare_Coll_Seq(pParse, p_چپ, p_راست);
  }
  p5 = دودویی_Compare_P5(p_چپ, p_راست, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (تهی*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  بازگشت addr;
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
صحیح sqlite3_عبارت_Is_Vector(ثابت عبارت *p_عبارت){
  بازگشت sqlite3_عبارت_Vector_Size(p_عبارت)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحیح sqlite3_عبارت_Vector_Size(ثابت عبارت *p_عبارت){
  u8 op = p_عبارت->op;
  اگر( op==TK_REGISTER ) op = p_عبارت->op2;
  اگر( op==TK_VECTOR ){
    أكد( عبارت_Use_XList(p_عبارت) );
    بازگشت p_عبارت->x.pList->n_عبارت;
  }وگرنه اگر( op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(p_عبارت) );
    بازگشت p_عبارت->x.pSelect->pEList->n_عبارت;
  }وگرنه{
    بازگشت 1;
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
عبارت *sqlite3VectorFieldSubexpr(عبارت *pVector, صحیح i){
  أكد( i<sqlite3_عبارت_Vector_Size(pVector) || pVector->op==TK_ERROR );
  اگر( sqlite3_عبارت_Is_Vector(pVector) ){
    أكد( pVector->op2==0 || pVector->op==TK_REGISTER );
    اگر( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      أكد( عبارت_Use_XSelect(pVector) );
      بازگشت pVector->x.pSelect->pEList->a[i].p_عبارت;
    }وگرنه{
      أكد( عبارت_Use_XList(pVector) );
      بازگشت pVector->x.pList->a[i].p_عبارت;
    }
  }
  بازگشت pVector;
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
عبارت *sqlite3_عبارت_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  عبارت *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  صحیح iField,          /* Which column of the vector to return */
  صحیح nField           /* Total number of columns in the vector */
){
  عبارت *pRet;
  اگر( pVector->op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(pVector) );
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
    اگر( pRet ){
      عبارت_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_چپ = pVector;
    }
  }وگرنه{
    اگر( pVector->op==TK_VECTOR ){
      عبارت **ppVector;
      أكد( عبارت_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_عبارت;
      pVector = *ppVector;
      اگر( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        بازگشت pVector;
      }
    }
    pRet = sqlite3_عبارت_Dup(pParse->db, pVector, 0);
  }
  بازگشت pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ایستا صحیح عبارت_Code_Subselect(Parse *pParse, عبارت *p_عبارت){
  صحیح reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  اگر( p_عبارت->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_عبارت);
  }
#endif
  بازگشت reg;
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
ایستا صحیح عبارت_Vector_Register(
  Parse *pParse,                  /* Parse context */
  عبارت *pVector,                  /* Vector to extract element from */
  صحیح iField,                     /* Field to extract from pVector */
  صحیح regSelect,                  /* First in array of registers */
  عبارت *pTmp,                     /* Temporary space */
  عبارت **pp_عبارت,                  /* OUT: Expression element */
  صحیح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  أكد( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  اگر( op==TK_REGISTER ){
    *pp_عبارت = sqlite3VectorFieldSubexpr(pVector, iField);
    بازگشت pVector->iTable+iField;
  }
  اگر( op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, اندازه(عبارت));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_چپ = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_عبارت;
    *pp_عبارت = pTmp;
    بازگشت regSelect+iField;
  }
  اگر( op==TK_VECTOR ){
    أكد( عبارت_Use_XList(pVector) );
    *pp_عبارت = pVector->x.pList->a[iField].p_عبارت;
    بازگشت sqlite3_عبارت_Code_Temp(pParse, *pp_عبارت, pRegFree);
  }
  بازگشت 0;
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
ایستا تهی codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  عبارت *p_عبارت,          /* The comparison operation */
  صحیح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  عبارت *p_چپ = p_عبارت->p_چپ;
  عبارت *p_راست = p_عبارت->p_راست;
  صحیح n_چپ = sqlite3_عبارت_Vector_Size(p_چپ);
  صحیح i;
  صحیح reg_چپ = 0;
  صحیح reg_راست = 0;
  u8 opx = op;
  صحیح addrCmp = 0;
  صحیح addrDone = sqlite3VdbeMakeLabel(pParse);
  صحیح isCommuted = عبارت_Has_Property(p_عبارت,EP_Commuted);

  أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  اگر( pParse->nErr ) بازگشت;
  اگر( n_چپ!=sqlite3_عبارت_Vector_Size(p_راست) ){
    sqlite3_Error_پیام(pParse, "row value misused");
    بازگشت;
  }
  أكد( p_عبارت->op==TK_EQ || p_عبارت->op==TK_NE
       || p_عبارت->op==TK_IS || p_عبارت->op==TK_ISNOT
       || p_عبارت->op==TK_LT || p_عبارت->op==TK_GT
       || p_عبارت->op==TK_LE || p_عبارت->op==TK_GE
  );
  أكد( p_عبارت->op==op || (p_عبارت->op==TK_IS && op==TK_EQ)
            || (p_عبارت->op==TK_ISNOT && op==TK_NE) );
  أكد( p5==0 || p_عبارت->op!=op );
  أكد( p5==SQLITE_NULLEQ || p_عبارت->op==op );

  اگر( op==TK_LE ) opx = TK_LT;
  اگر( op==TK_GE ) opx = TK_GT;
  اگر( op==TK_NE ) opx = TK_EQ;

  reg_چپ = عبارت_Code_Subselect(pParse, p_چپ);
  reg_راست = عبارت_Code_Subselect(pParse, p_راست);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  برای(i=0; 1 /*Loop exits by "break"*/; i++){
    صحیح regFree1 = 0, regFree2 = 0;
    عبارت *pL = 0, *pR = 0;
    عبارت tmp1, tmp2;
    صحیح r1, r2;
    أكد( i>=0 && i<n_چپ );
    اگر( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = عبارت_Vector_Register(pParse, p_چپ, i, reg_چپ, &tmp1, &pL, &regFree1);
    r2 = عبارت_Vector_Register(pParse, p_راست, i, reg_راست, &tmp2, &pR, &regFree2);
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
    اگر( (opx==TK_LT || opx==TK_GT) && i<n_چپ-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    اگر( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }وگرنه{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    اگر( i==n_چپ-1 ){
      بشکن;
    }
    اگر( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }وگرنه{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      اگر( i==n_چپ-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  اگر( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#اگر SQLITE_MAX_عبارت_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحیح sqlite3_عبارت_Check_Height(Parse *pParse, صحیح nHeight){
  صحیح rc = SQLITE_OK;
  صحیح mxHeight = pParse->db->aLimit[SQLITE_LIMIT_عبارت_DEPTH];
  اگر( nHeight>mxHeight ){
    sqlite3_Error_پیام(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  بازگشت rc;
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
ایستا تهی height_Of_عبارت(ثابت عبارت *p, صحیح *pnHeight){
  اگر( p ){
    اگر( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
ایستا تهی height_Of_عبارت_List(ثابت عبارت_List *p, صحیح *pnHeight){
  اگر( p ){
    صحیح i;
    برای(i=0; i<p->n_عبارت; i++){
      height_Of_عبارت(p->a[i].p_عبارت, pnHeight);
    }
  }
}
ایستا تهی heightOfSelect(ثابت Select *pSelect, صحیح *pnHeight){
  ثابت Select *p;
  برای(p=pSelect; p; p=p->pPrior){
    height_Of_عبارت(p->pWhere, pnHeight);
    height_Of_عبارت(p->pHaving, pnHeight);
    height_Of_عبارت(p->pLimit, pnHeight);
    height_Of_عبارت_List(p->pEList, pnHeight);
    height_Of_عبارت_List(p->pGroupBy, pnHeight);
    height_Of_عبارت_List(p->pOrderBy, pnHeight);
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
ایستا تهی عبارت_Set_Height(عبارت *p){
  صحیح nHeight = p->p_چپ ? p->p_چپ->nHeight : 0;
  اگر( NEVER(p->p_راست) && p->p_راست->nHeight>nHeight ){
    nHeight = p->p_راست->nHeight;
  }
  اگر( عبارت_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }وگرنه اگر( p->x.pList ){
    height_Of_عبارت_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_عبارت_List_Flags(p->x.pList);
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
تهی sqlite3_عبارت_Set_Height_And_Flags(Parse *pParse, عبارت *p){
  اگر( pParse->nErr ) بازگشت;
  عبارت_Set_Height(p);
  sqlite3_عبارت_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحیح sqlite3_Select_عبارت_Height(ثابت Select *p){
  صحیح nHeight = 0;
  heightOfSelect(p, &nHeight);
  بازگشت nHeight;
}
#وگرنه /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
تهی sqlite3_عبارت_Set_Height_And_Flags(Parse *pParse, عبارت *p){
  اگر( pParse->nErr ) بازگشت;
  اگر( p && عبارت_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_عبارت_List_Flags(p->x.pList);
  }
}
#define عبارت_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
تهی sqlite3_عبارت_Set_Error_Offset(عبارت *p_عبارت, صحیح iOfst){
  اگر( p_عبارت==0 ) بازگشت;
  اگر( NEVER(عبارت_Use_WJoin(p_عبارت)) ) بازگشت;
  p_عبارت->w.iOfst = iOfst;
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
عبارت *sqlite3_عبارت_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  صحیح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحیح dequote             /* True to dequote */
){
  عبارت *pNew;
  صحیح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, اندازه(عبارت)+nExtra);
  اگر( pNew ){
    memset(pNew, 0, اندازه(عبارت));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    اگر( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (نویسه*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      اگر( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      اگر( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_عبارت(pNew);
      }
    }
#اگر SQLITE_MAX_عبارت_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  بازگشت pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
عبارت *sqlite3_عبارت(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحیح op,                 /* Expression opcode */
  ثابت نویسه *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  بازگشت sqlite3_عبارت_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
عبارت *sqlite3_عبارت_Int32(sqlite3 *db, صحیح iVal){
  عبارت *pNew = sqlite3DbMallocRawNN(db, اندازه(عبارت));
  اگر( pNew ){
    memset(pNew, 0, اندازه(عبارت));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_مقدار|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_مقدار = iVal;
#اگر SQLITE_MAX_عبارت_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  بازگشت pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
تهی sqlite3_عبارت_Attach_Subtrees(
  sqlite3 *db,
  عبارت *p_ریشه,
  عبارت *p_چپ,
  عبارت *p_راست
){
  اگر( p_ریشه==0 ){
    أكد( db->mallocFailed );
    sqlite3_عبارت_Delete(db, p_چپ);
    sqlite3_عبارت_Delete(db, p_راست);
  }وگرنه{
    أكد( عبارت_Use_XList(p_ریشه) );
    أكد( p_ریشه->x.pSelect==0 );
    اگر( p_راست ){
      p_ریشه->p_راست = p_راست;
      p_ریشه->flags |= EP_Propagate & p_راست->flags;
#اگر SQLITE_MAX_عبارت_DEPTH>0
      p_ریشه->nHeight = p_راست->nHeight+1;
    }وگرنه{
      p_ریشه->nHeight = 1;
#endif
    }
    اگر( p_چپ ){
      p_ریشه->p_چپ = p_چپ;
      p_ریشه->flags |= EP_Propagate & p_چپ->flags;
#اگر SQLITE_MAX_عبارت_DEPTH>0
      اگر( p_چپ->nHeight>=p_ریشه->nHeight ){
        p_ریشه->nHeight = p_چپ->nHeight+1;
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
عبارت *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  صحیح op,                 /* Expression opcode */
  عبارت *p_چپ,            /* Left operand */
  عبارت *p_راست            /* Right operand */
){
  عبارت *p;
  p = sqlite3DbMallocRawNN(pParse->db, اندازه(عبارت));
  اگر( p ){
    memset(p, 0, اندازه(عبارت));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_عبارت_Attach_Subtrees(pParse->db, p, p_چپ, p_راست);
    sqlite3_عبارت_Check_Height(pParse, p->nHeight);
  }وگرنه{
    sqlite3_عبارت_Delete(pParse->db, p_چپ);
    sqlite3_عبارت_Delete(pParse->db, p_راست);
  }
  بازگشت p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
تهی sqlite3PExprAddSelect(Parse *pParse, عبارت *p_عبارت, Select *pSelect){
  اگر( p_عبارت ){
    p_عبارت->x.pSelect = pSelect;
    عبارت_Set_Property(p_عبارت, EP_xIsSelect|EP_Subquery);
    sqlite3_عبارت_Set_Height_And_Flags(pParse, p_عبارت);
  }وگرنه{
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
Select *sqlite3_عبارت_List_To_مقادیر(Parse *pParse, صحیح nElem, عبارت_List *pEList){
  صحیح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  برای(ii=0; ii<pEList->n_عبارت; ii++){
    Select *pSel;
    عبارت *p_عبارت = pEList->a[ii].p_عبارت;
    صحیح n_عبارت_Elem;
    اگر( p_عبارت->op==TK_VECTOR ){
      أكد( عبارت_Use_XList(p_عبارت) );
      n_عبارت_Elem = p_عبارت->x.pList->n_عبارت;
    }وگرنه{
      n_عبارت_Elem = 1;
    }
    اگر( n_عبارت_Elem!=nElem ){
      sqlite3_Error_پیام(pParse, "IN(...) element has %d term%s - expected %d",
          n_عبارت_Elem, n_عبارت_Elem>1?"s":"", nElem
      );
      بشکن;
    }
    أكد( عبارت_Use_XList(p_عبارت) );
    pSel = sqlite3SelectNew(pParse, p_عبارت->x.pList, 0, 0, 0, 0, 0, SF_مقادیر,0);
    p_عبارت->x.pList = 0;
    اگر( pSel ){
      اگر( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  اگر( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_مقدار;
  }
  sqlite3_عبارت_List_Delete(pParse->db, pEList);
  بازگشت pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
عبارت *sqlite3_عبارت_And(Parse *pParse, عبارت *p_چپ, عبارت *p_راست){
  sqlite3 *db = pParse->db;
  اگر( p_چپ==0  ){
    بازگشت p_راست;
  }وگرنه اگر( p_راست==0 ){
    بازگشت p_چپ;
  }وگرنه{
    u32 f = p_چپ->flags | p_راست->flags;
    اگر( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      عبارت *p;
      sqlite3_عبارت_Deferred_Delete(pParse, p_چپ);
      sqlite3_عبارت_Deferred_Delete(pParse, p_راست);
      p = sqlite3_عبارت_Int32(db, 0);
      اگر( p ) p->op2 = 1;
      بازگشت p;
    }وگرنه{
      بازگشت sqlite3PExpr(pParse, TK_AND, p_چپ, p_راست);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
عبارت *sqlite3_عبارت_Function(
  Parse *pParse,        /* Parsing context */
  عبارت_List *pList,      /* Argument list */
  ثابت Token *pToken,  /* Name of the function */
  صحیح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  عبارت *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3_عبارت_Alloc(db, TK_FUNCTION, pToken, 1);
  اگر( pNew==0 ){
    sqlite3_عبارت_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    بازگشت 0;
  }
  أكد( !عبارت_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحیح)(pToken->z - pParse->zTail);
  اگر( pList
   && pList->n_عبارت > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_پیام(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  عبارت_Set_Property(pNew, EP_HasFunc);
  أكد( عبارت_Use_XList(pNew) );
  sqlite3_عبارت_Set_Height_And_Flags(pParse, pNew);
  اگر( eDistinct==SF_Distinct ) عبارت_Set_Property(pNew, EP_Distinct);
  بازگشت pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
تهی sqlite3_عبارت_Order_By_Aggregate_Error(Parse *pParse, عبارت *p){
  sqlite3_Error_پیام(pParse,
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
تهی sqlite3_عبارت_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  عبارت *p_عبارت,          /* The function call to which ORDER BY is to be added */
  عبارت_List *pOrderBy    /* The ORDER BY clause to add */
){
  عبارت *pOB;
  sqlite3 *db = pParse->db;
  اگر( NEVER(pOrderBy==0) ){
    أكد( db->mallocFailed );
    بازگشت;
  }
  اگر( p_عبارت==0 ){
    أكد( db->mallocFailed );
    sqlite3_عبارت_List_Delete(db, pOrderBy);
    بازگشت;
  }
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( p_عبارت->p_چپ==0 );
  أكد( عبارت_Use_XList(p_عبارت) );
  اگر( p_عبارت->x.pList==0 || NEVER(p_عبارت->x.pList->n_عبارت==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_عبارت_List_Delete_Generic, pOrderBy);
    بازگشت;
  }
  اگر( IsWindowFunc(p_عبارت) ){
    sqlite3_عبارت_Order_By_Aggregate_Error(pParse, p_عبارت);
    sqlite3_عبارت_List_Delete(db, pOrderBy);
    بازگشت;
  }
  اگر( pOrderBy->n_عبارت>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_پیام(pParse, "too many terms in ORDER BY clause");
    sqlite3_عبارت_List_Delete(db, pOrderBy);
    بازگشت;
  }

  pOB = sqlite3_عبارت_Alloc(db, TK_ORDER, 0, 0);
  اگر( pOB==0 ){
    sqlite3_عبارت_List_Delete(db, pOrderBy);
    بازگشت;
  }
  pOB->x.pList = pOrderBy;
  أكد( عبارت_Use_XList(pOB) );
  p_عبارت->p_چپ = pOB;
  عبارت_Set_Property(pOB, EP_FullSize);
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
تهی sqlite3_عبارت_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت عبارت *p_عبارت,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !IN_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  اگر( عبارت_Has_Property(p_عبارت, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    اگر( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_پیام(pParse, "unsafe use of %#T()", p_عبارت);
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
تهی sqlite3_عبارت_Assign_Var_عدد(Parse *pParse, عبارت *p_عبارت, u32 n){
  sqlite3 *db = pParse->db;
  ثابت نویسه *z;
  ynVar x;

  اگر( p_عبارت==0 ) بازگشت;
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار|EP_Reduced|EP_TokenOnly) );
  z = p_عبارت->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  اگر( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }وگرنه{
    صحیح doAdd = 0;
    اگر( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      صحیح bOk;
      اگر( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }وگرنه{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_عدد]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_عدد] );
      اگر( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_عدد] ){
        sqlite3_Error_پیام(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_عدد]);
        sqlite3_Record_Error_Offset_Of_عبارت(pParse->db, p_عبارت);
        بازگشت;
      }
      x = (ynVar)i;
      اگر( x>pParse->nVar ){
        pParse->nVar = (صحیح)x;
        doAdd = 1;
      }وگرنه اگر( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }وگرنه اگر( sqlite3_VList_رقم_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }وگرنه{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_رقم(pParse->pVList, z, n);
      اگر( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    اگر( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      اگر( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_عبارت->iColumn = x;
  اگر( x>db->aLimit[SQLITE_LIMIT_VARIABLE_عدد] ){
    sqlite3_Error_پیام(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_عبارت(pParse->db, p_عبارت);
  }
}

/*
** Recursively delete an expression tree.
*/
ایستا SQLITE_NOINLINE تهی sqlite3_عبارت_Delete_NN(sqlite3 *db, عبارت *p){
  أكد( p!=0 );
  أكد( db!=0 );
عبارت_Delete_Restart:
  أكد( !عبارت_Use_UValue(p) || p->u.i_مقدار>=0 );
  أكد( !عبارت_Use_YWin(p) || !عبارت_Use_YSub(p) );
  أكد( !عبارت_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !عبارت_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  اگر( عبارت_Has_Property(p, EP_Leaf) && !عبارت_Has_Property(p, EP_TokenOnly) ){
    أكد( p->p_چپ==0 );
    أكد( p->p_راست==0 );
    أكد( !عبارت_Use_XSelect(p) || p->x.pSelect==0 );
    أكد( !عبارت_Use_XList(p) || p->x.pList==0 );
  }
#endif
  اگر( !عبارت_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (عبارت_Use_XList(p) && p->x.pList==0) || p->p_راست==0 );
    اگر( p->p_راست ){
      أكد( !عبارت_Has_Property(p, EP_WinFunc) );
      sqlite3_عبارت_Delete_NN(db, p->p_راست);
    }وگرنه اگر( عبارت_Use_XSelect(p) ){
      أكد( !عبارت_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }وگرنه{
      sqlite3_عبارت_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      اگر( عبارت_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    اگر( p->p_چپ && p->op!=TK_SELECT_COLUMN ){
      عبارت *p_چپ = p->p_چپ;
      اگر( !عبارت_Has_Property(p, EP_Static)
       && !عبارت_Has_Property(p_چپ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_چپ;
        برو عبارت_Delete_Restart;
      }وگرنه{
        sqlite3_عبارت_Delete_NN(db, p_چپ);
      }
    }
  }
  اگر( !عبارت_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
تهی sqlite3_عبارت_Delete(sqlite3 *db, عبارت *p){
  اگر( p ) sqlite3_عبارت_Delete_NN(db, p);
}
تهی sqlite3_عبارت_Delete_Generic(sqlite3 *db, تهی *p){
  اگر( ALWAYS(p) ) sqlite3_عبارت_Delete_NN(db, (عبارت*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
تهی sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  اگر( p==0 ){
    /* Nothing to clear */
  }وگرنه اگر( p->pOn ){
    sqlite3_عبارت_Delete_NN(db, p->pOn);
  }وگرنه اگر( p->pUsing ){
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
صحیح sqlite3_عبارت_Deferred_Delete(Parse *pParse, عبارت *p_عبارت){
  بازگشت 0==sqlite3ParserAddCleanup(pParse, sqlite3_عبارت_Delete_Generic, p_عبارت);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
تهی sqlite3_عبارت_Unmap_And_Delete(Parse *pParse, عبارت *p){
  اگر( p ){
    اگر( IN_RENAME_OBJECT ){
      sqlite3_Rename_عبارت_Unmap(pParse, p);
    }
    sqlite3_عبارت_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ایستا صحیح عبارت_Struct_Size(ثابت عبارت *p){
  اگر( عبارت_Has_Property(p, EP_TokenOnly) ) بازگشت عبارت_TOKENONLYSIZE;
  اگر( عبارت_Has_Property(p, EP_Reduced) ) بازگشت عبارت_REDUCEDSIZE;
  بازگشت عبارت_FULLSIZE;
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
ایستا صحیح duped_عبارت_Struct_Size(ثابت عبارت *p, صحیح flags){
  صحیح nSize;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( عبارت_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  اگر( 0==flags || عبارت_Has_Property(p, EP_FullSize) ){
    nSize = عبارت_FULLSIZE;
  }وگرنه{
    أكد( !عبارت_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    أكد( !عبارت_Has_Property(p, EP_OuterON) );
    أكد( !عبارت_Has_VVAProperty(p, EP_NoReduce) );
    اگر( p->p_چپ || p->x.pList ){
      nSize = عبارت_REDUCEDSIZE | EP_Reduced;
    }وگرنه{
      أكد( p->p_راست==0 );
      nSize = عبارت_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  بازگشت nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ایستا صحیح duped_عبارت_گره_Size(ثابت عبارت *p, صحیح flags){
  صحیح nByte = duped_عبارت_Struct_Size(p, flags) & 0xfff;
  اگر( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  بازگشت ROUND8(nByte);
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
ایستا صحیح duped_عبارت_Size(ثابت عبارت *p){
  صحیح nByte;
  أكد( p!=0 );
  nByte = duped_عبارت_گره_Size(p, EXPRDUP_REDUCE);
  اگر( p->p_چپ ) nByte += duped_عبارت_Size(p->p_چپ);
  اگر( p->p_راست ) nByte += duped_عبارت_Size(p->p_راست);
  أكد( nByte==ROUND8(nByte) );
  بازگشت nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
تعریفنوع ساختار EdupBuf EdupBuf;
ساختار EdupBuf {
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
ایستا عبارت *عبارت_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت عبارت *p,        /* Expr tree to be duplicated */
  صحیح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  عبارت *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  صحیح nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  أكد( db!=0 );
  أكد( p );
  أكد( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  أكد( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  اگر( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }وگرنه{
    صحیح nAlloc;
    اگر( dupFlags ){
      nAlloc = duped_عبارت_Size(p);
    }وگرنه اگر( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(عبارت_FULLSIZE + nToken);
    }وگرنه{
      nToken = 0;
      nAlloc = ROUND8(عبارت_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (عبارت *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  اگر( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت unsigned nStructSize = duped_عبارت_Struct_Size(p, dupFlags);
    صحیح nNewSize = nStructSize & 0xfff;
    اگر( nToken<0 ){
      اگر( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }وگرنه{
        nToken = 0;
      }
    }
    اگر( dupFlags ){
      أكد( (صحیح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      أكد( عبارت_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }وگرنه{
      u32 nSize = (u32)عبارت_Struct_Size(p);
      أكد( (صحیح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (صحیح)عبارت_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      اگر( nSize<عبارت_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, عبارت_FULLSIZE-nSize);
      }
      nNewSize = عبارت_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    عبارت_Clear_VVAProperties(pNew);
    اگر( dupFlags ){
      عبارت_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    اگر( nToken>0 ){
      نویسه *zToken = pNew->u.zToken = (نویسه*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    اگر( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      اگر( عبارت_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }وگرنه{
        pNew->x.pList = sqlite3_عبارت_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      اگر( عبارت_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        أكد( عبارت_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      اگر( dupFlags ){
        اگر( p->op==TK_SELECT_COLUMN ){
          pNew->p_چپ = p->p_چپ;
          أكد( p->p_راست==0 
               || p->p_راست==p->p_چپ
               || عبارت_Has_Property(p->p_چپ, EP_Subquery) );
        }وگرنه{
          pNew->p_چپ = p->p_چپ ?
                      عبارت_Dup(db, p->p_چپ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_راست = p->p_راست ?
                       عبارت_Dup(db, p->p_راست, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }وگرنه{
        اگر( p->op==TK_SELECT_COLUMN ){
          pNew->p_چپ = p->p_چپ;
          أكد( p->p_راست==0 
               || p->p_راست==p->p_چپ
               || عبارت_Has_Property(p->p_چپ, EP_Subquery) );
        }وگرنه{
          pNew->p_چپ = sqlite3_عبارت_Dup(db, p->p_چپ, 0);
        }
        pNew->p_راست = sqlite3_عبارت_Dup(db, p->p_راست, 0);
      }
    }
  }
  اگر( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, اندازه(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  بازگشت pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  اگر( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    اگر( pRet ){
      صحیح i;
      pRet->nCte = p->nCte;
      برای(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_عبارت_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  بازگشت pRet;
}
#وگرنه
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ایستا صحیح gatherSelectWindowsCallback(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_FUNCTION && عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_عبارت->y.pWin;
    أكد( pWin );
    أكد( IsWindowFunc(p_عبارت) );
    أكد( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  بازگشت WRC_Continue;
}
ایستا صحیح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  بازگشت p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ایستا تهی gatherSelectWindows(Select *p){
  Walker w;
  w.x_عبارت_Callback = gatherSelectWindowsCallback;
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
عبارت *sqlite3_عبارت_Dup(sqlite3 *db, ثابت عبارت *p, صحیح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  بازگشت p ? عبارت_Dup(db, p, flags, 0) : 0;
}
عبارت_List *sqlite3_عبارت_List_Dup(sqlite3 *db, ثابت عبارت_List *p, صحیح flags){
  عبارت_List *pNew;
  ساختار عبارت_List_item *pItem;
  ثابت ساختار عبارت_List_item *pOldItem;
  صحیح i;
  عبارت *pPriorSelectColOld = 0;
  عبارت *pPriorSelectColNew = 0;
  أكد( db!=0 );
  اگر( p==0 ) بازگشت 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  اگر( pNew==0 ) بازگشت 0;
  pNew->n_عبارت = p->n_عبارت;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  برای(i=0; i<p->n_عبارت; i++, pItem++, pOldItem++){
    عبارت *p_Old_عبارت = pOldItem->p_عبارت;
    عبارت *p_New_عبارت;
    pItem->p_عبارت = sqlite3_عبارت_Dup(db, p_Old_عبارت, flags);
    اگر( p_Old_عبارت
     && p_Old_عبارت->op==TK_SELECT_COLUMN
     && (p_New_عبارت = pItem->p_عبارت)!=0
    ){
      اگر( p_New_عبارت->p_راست ){
        pPriorSelectColOld = p_Old_عبارت->p_راست;
        pPriorSelectColNew = p_New_عبارت->p_راست;
        p_New_عبارت->p_چپ = p_New_عبارت->p_راست;
      }وگرنه{
        اگر( p_Old_عبارت->p_چپ!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_عبارت->p_چپ;
          pPriorSelectColNew = sqlite3_عبارت_Dup(db, pPriorSelectColOld, flags);
          p_New_عبارت->p_راست = pPriorSelectColNew;
        }
        p_New_عبارت->p_چپ = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  بازگشت pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#اگر !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ثابت SrcList *p, صحیح flags){
  SrcList *pNew;
  صحیح i;
  أكد( db!=0 );
  اگر( p==0 ) بازگشت 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  اگر( pNew==0 ) بازگشت 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  برای(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ثابت SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    اگر( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, اندازه(Subquery));
      اگر( pNewSubq==0 ){
        أكد( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }وگرنه{
        memcpy(pNewSubq, pOldItem->u4.pSubq, اندازه(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        اگر( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }وگرنه اگر( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }وگرنه{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    اگر( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }وگرنه اگر( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_عبارت_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }وگرنه{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    اگر( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    اگر( pTab ){
      pTab->nTabRef++;
    }
    اگر( pOldItem->fg.isUsing ){
      أكد( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }وگرنه{
      pNewItem->u3.pOn = sqlite3_عبارت_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  بازگشت pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ثابت IdList *p){
  IdList *pNew;
  صحیح i;
  أكد( db!=0 );
  اگر( p==0 ) بازگشت 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  اگر( pNew==0 ) بازگشت 0;
  pNew->nId = p->nId;
  برای(i=0; i<p->nId; i++){
    ساختار IdList_item *pNewItem = &pNew->a[i];
    ثابت ساختار IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  بازگشت pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحیح flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  برای(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, اندازه(*p) );
    اگر( pNew==0 ) بشکن;
    pNew->pEList = sqlite3_عبارت_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_عبارت_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_عبارت_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_عبارت_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_عبارت_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_عبارت_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    اگر( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    اگر( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      بشکن;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  بازگشت pRet;
}
#وگرنه
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحیح flags){
  أكد( p==0 );
  بازگشت 0;
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
ایستا ثابت ساختار عبارت_List_item zeroItem = {0};
SQLITE_NOINLINE عبارت_List *sqlite3_عبارت_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  ساختار عبارت_List_item *pItem;
  عبارت_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  اگر( pList==0 ){
    sqlite3_عبارت_Delete(db, p_عبارت);
    بازگشت 0;
  }
  pList->nAlloc = 4;
  pList->n_عبارت = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_عبارت = p_عبارت;
  بازگشت pList;
}
SQLITE_NOINLINE عبارت_List *sqlite3_عبارت_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  عبارت_List *pList,        /* List to which to append. Might be NULL */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  ساختار عبارت_List_item *pItem;
  عبارت_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  اگر( pNew==0 ){
    sqlite3_عبارت_List_Delete(db, pList);
    sqlite3_عبارت_Delete(db, p_عبارت);
    بازگشت 0;
  }وگرنه{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_عبارت++];
  *pItem = zeroItem;
  pItem->p_عبارت = p_عبارت;
  بازگشت pList;
}
عبارت_List *sqlite3_عبارت_List_Append(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to append. Might be NULL */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  ساختار عبارت_List_item *pItem;
  اگر( pList==0 ){
    بازگشت sqlite3_عبارت_List_Append_New(pParse->db,p_عبارت);
  }
  اگر( pList->nAlloc<pList->n_عبارت+1 ){
    بازگشت sqlite3_عبارت_List_Append_Grow(pParse->db,pList,p_عبارت);
  }
  pItem = &pList->a[pList->n_عبارت++];
  *pItem = zeroItem;
  pItem->p_عبارت = p_عبارت;
  بازگشت pList;
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
عبارت_List *sqlite3_عبارت_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  عبارت_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  عبارت *p_عبارت            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  صحیح n;
  صحیح i;
  صحیح iFirst = pList ? pList->n_عبارت : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  اگر( NEVER(pColumns==0) ) برو vector_append_error;
  اگر( p_عبارت==0 ) برو vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  اگر( p_عبارت->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_عبارت_Vector_Size(p_عبارت)) ){
    sqlite3_Error_پیام(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    برو vector_append_error;
  }

  برای(i=0; i<pColumns->nId; i++){
    عبارت *p_Sub_عبارت = sqlite3_عبارت_For_Vector_Field(pParse, p_عبارت, i, pColumns->nId);
    أكد( p_Sub_عبارت!=0 || db->mallocFailed );
    اگر( p_Sub_عبارت==0 ) ادامه;
    pList = sqlite3_عبارت_List_Append(pParse, pList, p_Sub_عبارت);
    اگر( pList ){
      أكد( pList->n_عبارت==iFirst+i+1 );
      pList->a[pList->n_عبارت-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  اگر( !db->mallocFailed && p_عبارت->op==TK_SELECT && ALWAYS(pList!=0) ){
    عبارت *pFirst = pList->a[iFirst].p_عبارت;
    أكد( pFirst!=0 );
    أكد( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_راست = p_عبارت;
    p_عبارت = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_عبارت_Unmap_And_Delete(pParse, p_عبارت);
  sqlite3IdListDelete(db, pColumns);
  بازگشت pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
تهی sqlite3_عبارت_List_Set_Sort_Order(عبارت_List *p, صحیح iSortOrder, صحیح eNulls){
  ساختار عبارت_List_item *pItem;
  اگر( p==0 ) بازگشت;
  أكد( p->n_عبارت>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_عبارت-1];
  أكد( pItem->fg.bNulls==0 );
  اگر( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  اگر( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    اگر( iSortOrder!=eNulls ){
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
تهی sqlite3_عبارت_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحیح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  اگر( pList ){
    ساختار عبارت_List_item *pItem;
    أكد( pList->n_عبارت>0 );
    pItem = &pList->a[pList->n_عبارت-1];
    أكد( pItem->zEName==0 );
    أكد( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    اگر( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      اگر( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ثابت تهی*)pItem->zEName, pName);
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
تهی sqlite3_عبارت_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to add the span. */
  ثابت نویسه *zStart,     /* Start of the span */
  ثابت نویسه *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  اگر( pList ){
    ساختار عبارت_List_item *pItem = &pList->a[pList->n_عبارت-1];
    أكد( pList->n_عبارت>0 );
    اگر( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
تهی sqlite3_عبارت_List_Check_Length(
  Parse *pParse,
  عبارت_List *pEList,
  ثابت نویسه *zObject
){
  صحیح mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_عبارت==mx );
  testcase( pEList && pEList->n_عبارت==mx+1 );
  اگر( pEList && pEList->n_عبارت>mx ){
    sqlite3_Error_پیام(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ایستا SQLITE_NOINLINE تهی عبارت_List_Delete_NN(sqlite3 *db, عبارت_List *pList){
  صحیح i = pList->n_عبارت;
  ساختار عبارت_List_item *pItem =  pList->a;
  أكد( pList->n_عبارت>0 );
  أكد( db!=0 );
  انجامبده{
    sqlite3_عبارت_Delete(db, pItem->p_عبارت);
    اگر( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }وقتی( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
تهی sqlite3_عبارت_List_Delete(sqlite3 *db, عبارت_List *pList){
  اگر( pList ) عبارت_List_Delete_NN(db, pList);
}
تهی sqlite3_عبارت_List_Delete_Generic(sqlite3 *db, تهی *pList){
  اگر( ALWAYS(pList) ) عبارت_List_Delete_NN(db, (عبارت_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_عبارت_List_Flags(ثابت عبارت_List *pList){
  صحیح i;
  u32 m = 0;
  أكد( pList!=0 );
  برای(i=0; i<pList->n_عبارت; i++){
     عبارت *p_عبارت = pList->a[i].p_عبارت;
     أكد( p_عبارت!=0 );
     m |= p_عبارت->flags;
  }
  بازگشت m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
صحیح sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  بازگشت WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ثابت نویسه *zIn){
  اگر( sqlite3StrICmp(zIn, "true")==0  ) بازگشت EP_IsTrue;
  اگر( sqlite3StrICmp(zIn, "false")==0 ) بازگشت EP_IsFalse;
  بازگشت 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحیح sqlite3_عبارت_Id_To_True_False(عبارت *p_عبارت){
  u32 v;
  أكد( p_عبارت->op==TK_ID || p_عبارت->op==TK_STRING );
  اگر( !عبارت_Has_Property(p_عبارت, EP_Quoted|EP_Int_مقدار)
   && (v = sqlite3IsTrueOrFalse(p_عبارت->u.zToken))!=0
  ){
    p_عبارت->op = TK_TRUEFALSE;
    عبارت_Set_Property(p_عبارت, v);
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحیح sqlite3_عبارت_Truth_مقدار(ثابت عبارت *p_عبارت){
  p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely((عبارت*)p_عبارت);
  أكد( p_عبارت->op==TK_TRUEFALSE );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  أكد( sqlite3StrICmp(p_عبارت->u.zToken,"true")==0
       || sqlite3StrICmp(p_عبارت->u.zToken,"false")==0 );
  بازگشت p_عبارت->u.zToken[4]==0;
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
عبارت *sqlite3_عبارت_Simplified_And_Or(عبارت *p_عبارت){
  أكد( p_عبارت!=0 );
  اگر( p_عبارت->op==TK_AND || p_عبارت->op==TK_OR ){
    عبارت *p_راست = sqlite3_عبارت_Simplified_And_Or(p_عبارت->p_راست);
    عبارت *p_چپ = sqlite3_عبارت_Simplified_And_Or(p_عبارت->p_چپ);
    اگر( عبارت_Always_True(p_چپ) || عبارت_Always_False(p_راست) ){
      p_عبارت = p_عبارت->op==TK_AND ? p_راست : p_چپ;
    }وگرنه اگر( عبارت_Always_True(p_راست) || عبارت_Always_False(p_چپ) ){
      p_عبارت = p_عبارت->op==TK_AND ? p_چپ : p_راست;
    }
  }
  بازگشت p_عبارت;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ایستا صحیح عبارت_Eval_Rhs_First(عبارت *p_عبارت){
  اگر( عبارت_Has_Property(p_عبارت->p_چپ, EP_Subquery)
   && !عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery)
  ){
    بازگشت 1;
  }وگرنه{
    بازگشت 0;
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
ایستا صحیح عبارت_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  عبارت *p_عبارت,       /* The comparison expression */
  صحیح *pR1,          /* OUT: Register holding the left operand */
  صحیح *pR2,          /* OUT: Register holding the right operand */
  صحیح *pFree1,       /* OUT: Temp register to free if not zero */
  صحیح *pFree2        /* OUT: Another temp register to free if not zero */
){
  صحیح addrIsNull;
  صحیح r1, r2;
  Vdbe *v = pParse->pVdbe;

  أكد( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  اگر( عبارت_Eval_Rhs_First(p_عبارت) && sqlite3_عبارت_Can_Be_Null(p_عبارت->p_راست) ){
    r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }وگرنه{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, pFree1);
  اگر( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    اگر( عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery)
     && sqlite3_عبارت_Can_Be_Null(p_عبارت->p_چپ)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  بازگشت addrIsNull;
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
ایستا SQLITE_NOINLINE صحیح عبارت_گره_Is_Constant_Function(
  Walker *pWalker,
  عبارت *p_عبارت
){
  صحیح n;             /* Number of arguments */
  عبارت_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( p_عبارت->op==TK_FUNCTION );
  اگر( عبارت_Has_Property(p_عبارت, EP_TokenOnly)
   || (pList = p_عبارت->x.pList)==0
  ){;
    n = 0;
  }وگرنه{
    n = pList->n_عبارت;
    sqlite3_Walk_عبارت_List(pWalker, pList);
    اگر( pWalker->eCode==0 ) بازگشت WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_عبارت->u.zToken, n, ENC(db), 0);
  اگر( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(عبارت_Has_Property(p_عبارت, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    بازگشت WRC_Abort;
  }
  بازگشت WRC_Prune;
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
ایستا صحیح عبارت_گره_Is_Constant(Walker *pWalker, عبارت *p_عبارت){
  أكد( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  اگر( pWalker->eCode==2 && عبارت_Has_Property(p_عبارت, EP_OuterON) ){
    pWalker->eCode = 0;
    بازگشت WRC_Abort;
  }

  انتخاب( p_عبارت->op ){
    حالت TK_FUNCTION:
      اگر( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        بازگشت WRC_Abort;
      }وگرنه اگر( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        اگر( pWalker->eCode==5 ) عبارت_Set_Property(p_عبارت, EP_FromDDL);
        بازگشت WRC_Continue;
      }وگرنه اگر( عبارت_Has_Property(p_عبارت,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        بازگشت WRC_Continue;
      }وگرنه اگر( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        بازگشت عبارت_گره_Is_Constant_Function(pWalker, p_عبارت);
      }وگرنه{
        pWalker->eCode = 0;  /* Function call is not constant */
        بازگشت WRC_Abort;
      }
    حالت TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      اگر( sqlite3_عبارت_Id_To_True_False(p_عبارت) ){
        بازگشت WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالت TK_COLUMN:
    حالت TK_AGG_FUNCTION:
    حالت TK_AGG_COLUMN:
      testcase( p_عبارت->op==TK_ID );
      testcase( p_عبارت->op==TK_COLUMN );
      testcase( p_عبارت->op==TK_AGG_FUNCTION );
      testcase( p_عبارت->op==TK_AGG_COLUMN );
      اگر( عبارت_Has_Property(p_عبارت, EP_FixedCol) && pWalker->eCode!=2 ){
        بازگشت WRC_Continue;
      }
      اگر( pWalker->eCode==3 && p_عبارت->iTable==pWalker->u.iCur ){
        بازگشت WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالت TK_IF_NULL_ROW:
    حالت TK_REGISTER:
    حالت TK_DOT:
    حالت TK_RAISE:
      testcase( p_عبارت->op==TK_REGISTER );
      testcase( p_عبارت->op==TK_IF_NULL_ROW );
      testcase( p_عبارت->op==TK_DOT );
      testcase( p_عبارت->op==TK_RAISE );
      pWalker->eCode = 0;
      بازگشت WRC_Abort;
    حالت TK_IS:
    حالت TK_ISNOT:
      اگر( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        بازگشت WRC_Abort;
      }
      بازگشت WRC_Continue;
    حالت TK_VARIABLE:
      اگر( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_عبارت->op = TK_NULL;
      }وگرنه اگر( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        بازگشت WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    پیشفرض:
      testcase( p_عبارت->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_عبارت->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      بازگشت WRC_Continue;
  }
}
ایستا صحیح عبارت_Is_Const(Parse *pParse, عبارت *p, صحیح initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode;
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
صحیح sqlite3_عبارت_Is_Constant(Parse *pParse, عبارت *p){
  بازگشت عبارت_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحیح sqlite3_عبارت_List_Is_Constant(Parse *pParse, عبارت_List *pList, صحیح bNoIs){
  ثابت صحیح iInit = bNoIs ? 6 : 1;
  صحیح ii;
  برای(ii=0; ii<pList->n_عبارت; ii++){
    اگر( 0==عبارت_Is_Const(pParse, pList->a[ii].p_عبارت, iInit) ) بازگشت 0;
  }
  بازگشت 1;
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
ایستا صحیح sqlite3_عبارت_Is_Constant_Not_Join(Parse *pParse, عبارت *p){
  بازگشت عبارت_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ایستا صحیح عبارت_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->eCode==3 || pWalker->eCode==0 );
  اگر( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    بازگشت WRC_Abort;
  }
  بازگشت WRC_Prune;
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
صحیح sqlite3_عبارت_Is_Table_Constant(عبارت *p, صحیح iCur, صحیح bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant;
  اگر( bAllowSubq ){
    w.xSelectCallback = عبارت_Select_Walk_Table_Constant;
  }وگرنه{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode;
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
صحیح sqlite3_عبارت_Is_Single_Table_Constraint(
  عبارت *p_عبارت,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحیح iSrc,                    /* Which element of pSrcList to use */
  صحیح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت SrcItem *pSrc = &pSrcList->a[iSrc];
  اگر( pSrc->fg.jointype & JT_LTORJ ){
    بازگشت 0;  /* rule (3) */
  }
  اگر( pSrc->fg.jointype & JT_چپ ){
    اگر( !عبارت_Has_Property(p_عبارت, EP_OuterON) ) بازگشت 0;   /* rule (4a) */
    اگر( p_عبارت->w.iJoin!=pSrc->iCursor ) بازگشت 0;         /* rule (4b) */
  }وگرنه{
    اگر( عبارت_Has_Property(p_عبارت, EP_OuterON) ) بازگشت 0;    /* rule (5) */
  }
  اگر( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحیح jj;
    برای(jj=0; jj<iSrc; jj++){
      اگر( p_عبارت->w.iJoin==pSrcList->a[jj].iCursor ){
        اگر( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          بازگشت 0;  /* restriction (6) */
        }
        بشکن;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  بازگشت sqlite3_عبارت_Is_Table_Constant(p_عبارت, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ایستا صحیح عبارت_گره_Is_Constant_Or_Group_By(Walker *pWalker, عبارت *p_عبارت){
  عبارت_List *pGroupBy = pWalker->u.pGroupBy;
  صحیح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  برای(i=0; i<pGroupBy->n_عبارت; i++){
    عبارت *p = pGroupBy->a[i].p_عبارت;
    اگر( sqlite3_عبارت_Compare(0, p_عبارت, p, -1)<2 ){
      CollSeq *pColl = sqlite3_عبارت_NNColl_Seq(pWalker->pParse, p);
      اگر( sqlite3_Is_دودویی(pColl) ){
        بازگشت WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  اگر( عبارت_Use_XSelect(p_عبارت) ){
    pWalker->eCode = 0;
    بازگشت WRC_Abort;
  }

  بازگشت عبارت_گره_Is_Constant(pWalker, p_عبارت);
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
صحیح sqlite3_عبارت_Is_Constant_Or_Group_By(Parse *pParse, عبارت *p, عبارت_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode;
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
صحیح sqlite3_عبارت_Is_Constant_Or_Function(عبارت *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  بازگشت عبارت_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحیح sqlite3_عبارت_Contains_Subquery(عبارت *p){
  Walker w;
  w.eCode = 1;
  w.x_عبارت_Callback = sqlite3_عبارت_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode==0;
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
صحیح sqlite3_عبارت_Is_Integer(ثابت عبارت *p, صحیح *p_مقدار, Parse *pParse, صحیح bRSI){
  صحیح iSign = 1;   /* Either +1 or -1. */
  وقتی( 1/*exit-by-break*/ ){
    اگر( NEVER(p==0) ) بازگشت 0;
    اگر( عبارت_Use_UValue(p) ){
      أكد( p->op==TK_INTEGER );
      *p_مقدار = p->u.i_مقدار*iSign;
      اگر( bRSI && pParse==0 && p->op2 ){
        بازگشت 0;
      }
      بازگشت 1;
    }
    اگر( p->op==TK_UPLUS ){
      p = p->p_چپ;
      pParse = 0;
      ادامه;
    }
    اگر( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_چپ;
      pParse = 0;
      ادامه;
    }
    اگر( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_مقدار *pVal; /* The variable */
      صحیح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      اگر( NEVER(pParse->pVdbe==0) ) بشکن;
      اگر( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) بشکن;
      pVal = sqlite3_Vdbe_Get_Bound_مقدار(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      اگر( pVal ){
        i64 vv;
        اگر( sqlite3_مقدار_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_مقدار_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_مقدار = (صحیح)vv;
          rc = 1;       
        }
        sqlite3_مقدار_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      بازگشت rc;
    }
    بشکن;
  }
  بازگشت 0;
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
صحیح sqlite3_عبارت_Can_Be_Null(ثابت عبارت *p){
  u8 op;
  أكد( p!=0 );
  وقتی( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_چپ;
    أكد( p!=0 );
  }
  op = p->op;
  اگر( op==TK_REGISTER ) op = p->op2;
  انتخاب( op ){
    حالت TK_INTEGER:
    حالت TK_STRING:
    حالت TK_FLOAT:
    حالت TK_BLOB:
      بازگشت 0;
    حالت TK_COLUMN:
      أكد( عبارت_Use_YTab(p) );
      بازگشت عبارت_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    پیشفرض:
      بازگشت 1;
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
صحیح sqlite3_عبارت_Needs_No_Affinity_Change(ثابت عبارت *p, نویسه aff){
  u8 op;
  صحیح unaryMinus = 0;
  اگر( aff==SQLITE_AFF_BLOB ) بازگشت 1;
  وقتی( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    اگر( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_چپ;
  }
  op = p->op;
  اگر( op==TK_REGISTER ) op = p->op2;
  انتخاب( op ){
    حالت TK_INTEGER: {
      بازگشت aff>=SQLITE_AFF_NUMERIC;
    }
    حالت TK_FLOAT: {
      بازگشت aff>=SQLITE_AFF_NUMERIC;
    }
    حالت TK_STRING: {
      بازگشت !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    حالت TK_BLOB: {
      بازگشت !unaryMinus;
    }
    حالت TK_COLUMN: {
      أكد( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      بازگشت aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    پیشفرض: {
      بازگشت 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
صحیح sqlite3IsRowid(ثابت نویسه *z){
  اگر( sqlite3StrICmp(z, "_ROWID_")==0 ) بازگشت 1;
  اگر( sqlite3StrICmp(z, "ROWID")==0 ) بازگشت 1;
  اگر( sqlite3StrICmp(z, "OID")==0 ) بازگشت 1;
  بازگشت 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ثابت نویسه *sqlite3RowidAlias(Table *pTab){
  ثابت نویسه *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  صحیح ii;
  أكد( VisibleRowid(pTab) );
  برای(ii=0; ii<ArraySize(azOpt); ii++){
    اگر( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) بازگشت azOpt[ii];
  }
  بازگشت 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
ایستا Select *isCandidateForInOpt(ثابت عبارت *pX){
  Select *p;
  SrcList *pSrc;
  عبارت_List *pEList;
  Table *pTab;
  صحیح i;
  اگر( !عبارت_Use_XSelect(pX) ) بازگشت 0;                 /* Not a subquery */
  اگر( عبارت_Has_Property(pX, EP_VarSelect)  ) بازگشت 0;  /* Correlated subq */
  p = pX->x.pSelect;
  اگر( p->pPrior ) بازگشت 0;              /* Not a compound SELECT */
  اگر( p->selFlags & SF_Aggregate ){
    بازگشت 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  اگر( p->pLimit ) بازگشت 0;              /* Has no LIMIT clause */
  اگر( p->pWhere ) بازگشت 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  أكد( pSrc!=0 );
  اگر( pSrc->nSrc!=1 ) بازگشت 0;          /* Single term in FROM clause */
  اگر( pSrc->a[0].fg.isSubquery) بازگشت 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab)  );              /* FROM clause is not a view */
  اگر( IsVirtual(pTab) ) بازگشت 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  أكد( pEList!=0 );
  /* All SELECT results must be columns. */
  برای(i=0; i<pEList->n_عبارت; i++){
    عبارت *pRes = pEList->a[i].p_عبارت;
    اگر( pRes->op!=TK_COLUMN ) بازگشت 0;
    أكد( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  بازگشت p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ایستا تهی sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  صحیح iCur,          /* Cursor for the index */
  صحیح regHasNull,    /* Register in which to store hasNull flag */
  صحیح eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  صحیح addr1;
  صحیح op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  اگر( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }وگرنه{
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
ایستا صحیح sqlite3InRhsIsConstant(Parse *pParse, عبارت *pIn){
  عبارت *pLHS;
  صحیح res;
  أكد( !عبارت_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_چپ;
  pIn->p_چپ = 0;
  res = sqlite3_عبارت_Is_Constant(pParse, pIn);
  pIn->p_چپ = pLHS;
  بازگشت res;
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
صحیح sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  عبارت *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  صحیح *prRhsHasNull,         /* Register holding NULL status.  See notes */
  صحیح *aiMap,                /* Mapping from Index fields to RHS fields */
  صحیح *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  صحیح eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  صحیح iTab;                             /* Cursor of the RHS table */
  صحیح mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  أكد( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  اگر( prRhsHasNull && عبارت_Use_XSelect(pX) ){
    صحیح i;
    عبارت_List *pEList = pX->x.pSelect->pEList;
    برای(i=0; i<pEList->n_عبارت; i++){
      اگر( sqlite3_عبارت_Can_Be_Null(pEList->a[i].p_عبارت) ) بشکن;
    }
    اگر( i==pEList->n_عبارت ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  اگر( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    صحیح iDb;                               /* Database idx for pTab */
    عبارت_List *pEList = p->pEList;
    صحیح n_عبارت = pEList->n_عبارت;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].p_عبارت!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    اگر( n_عبارت==1 && pEList->a[0].p_عبارت->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحیح iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }وگرنه{
      Index *pIdx;                         /* Iterator variable */
      صحیح affinity_ok = 1;
      صحیح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      برای(i=0; i<n_عبارت && affinity_ok; i++){
        عبارت *pLhs = sqlite3VectorFieldSubexpr(pX->p_چپ, i);
        صحیح iCol = pEList->a[i].p_عبارت->iColumn;
        نویسه idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        نویسه cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        انتخاب( cmpaff ){
          حالت SQLITE_AFF_BLOB:
            بشکن;
          حالت SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            أكد( idxaff==SQLITE_AFF_TEXT );
            بشکن;
          پیشفرض:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      اگر( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        برای(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          اگر( pIdx->nColumn<n_عبارت ) ادامه;
          اگر( pIdx->pPartIdxWhere!=0 ) ادامه;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          اگر( pIdx->nColumn>=BMS-1 ) ادامه;
          اگر( mustBeUnique ){
            اگر( pIdx->n_کلید_Col>n_عبارت
             ||(pIdx->nColumn>n_عبارت && !IsUniqueIndex(pIdx))
            ){
              ادامه;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          برای(i=0; i<n_عبارت; i++){
            عبارت *pLhs = sqlite3VectorFieldSubexpr(pX->p_چپ, i);
            عبارت *pRhs = pEList->a[i].p_عبارت;
            CollSeq *pReq = sqlite3_دودویی_Compare_Coll_Seq(pParse, pLhs, pRhs);
            صحیح j;
 
            برای(j=0; j<n_عبارت; j++){
              اگر( pIdx->aiColumn[j]!=pRhs->iColumn ) ادامه;
              أكد( pIdx->azColl[j] );
              اگر( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ادامه;
              }
              بشکن;
            }
            اگر( j==n_عبارت ) بشکن;
            mCol = MASKBIT(j);
            اگر( mCol & colUsed ) بشکن; /* Each column used only once */
            colUsed |= mCol;
            اگر( aiMap ) aiMap[i] = j;
          }
 
          أكد( n_عبارت>0 && n_عبارت<BMS );
          أكد( i==n_عبارت || colUsed!=(MASKBIT(n_عبارت)-1) );
          اگر( colUsed==(MASKBIT(n_عبارت)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحیح iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_کلید_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            اگر( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_عبارت)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              اگر( n_عبارت==1 ){
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
  اگر( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && عبارت_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_عبارت<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  اگر( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحیح rMayHaveNull = 0;
    صحیح bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    اگر( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }وگرنه اگر( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_IN );
    اگر( !bloomOk
     && عبارت_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    اگر( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  اگر( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    صحیح i, n;
    n = sqlite3_عبارت_Vector_Size(pX->p_چپ);
    برای(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  بازگشت eType;
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
ایستا نویسه *عبارت_INAffinity(Parse *pParse, ثابت عبارت *p_عبارت){
  عبارت *p_چپ = p_عبارت->p_چپ;
  صحیح nVal = sqlite3_عبارت_Vector_Size(p_چپ);
  Select *pSelect = عبارت_Use_XSelect(p_عبارت) ? p_عبارت->x.pSelect : 0;
  نویسه *zRet;

  أكد( p_عبارت->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  اگر( zRet ){
    صحیح i;
    برای(i=0; i<nVal; i++){
      عبارت *pA = sqlite3VectorFieldSubexpr(p_چپ, i);
      نویسه a = sqlite3_عبارت_Affinity(pA);
      اگر( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_عبارت, a);
      }وگرنه{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  بازگشت zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
تهی sqlite3SubselectError(Parse *pParse, صحیح nActual, صحیح nExpect){
  اگر( pParse->nErr==0 ){
    ثابت نویسه *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_پیام(pParse, zFmt, nActual, nExpect);
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
تهی sqlite3_Vector_Error_پیام(Parse *pParse, عبارت *p_عبارت){
#ifndef SQLITE_OMIT_SUBQUERY
  اگر( عبارت_Use_XSelect(p_عبارت) ){
    sqlite3SubselectError(pParse, p_عبارت->x.pSelect->pEList->n_عبارت, 1);
  }وگرنه
#endif
  {
    sqlite3_Error_پیام(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ایستا صحیح findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  عبارت *p_عبارت,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  اگر( pNewSig==0 ) بازگشت 0;
  اگر( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) بازگشت 0;
  أكد( p_عبارت->op==TK_IN );
  أكد( !عبارت_Use_YSub(p_عبارت) );
  أكد( عبارت_Use_XSelect(p_عبارت) );
  أكد( p_عبارت->x.pSelect!=0 );
  أكد( (p_عبارت->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  برای(; pOp<pEnd; pOp++){
    اگر( pOp->p4type!=P4_SUBRTNSIG ) ادامه;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    اگر( !pSig->bComplete ) ادامه;
    اگر( pNewSig->selId!=pSig->selId ) ادامه;
    اگر( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ادامه;
    p_عبارت->y.sub.iAddr = pSig->iAddr;
    p_عبارت->y.sub.regReturn = pSig->regReturn;
    p_عبارت->iTable = pSig->iTable;
    عبارت_Set_Property(p_عبارت, EP_Subrtn);
    بازگشت 1;
  }
  بازگشت 0;
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
تهی sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  عبارت *p_عبارت,            /* The IN operator */
  صحیح iTab,               /* Use this cursor number */
  صحیح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحیح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحیح addr;                   /* Address of OP_OpenEphemeral instruction */
  عبارت *p_چپ;                /* the LHS of the IN operator */
  کلید_Info *p_کلید_Info = 0;      /* Key information */
  صحیح nVal;                   /* Size of vector pLeft */
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
  اگر( !عبارت_Has_Property(p_عبارت, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !عبارت_Use_XSelect(p_عبارت) || p_عبارت->x.pSelect!=0 );
    اگر( عبارت_Use_XSelect(p_عبارت) && (p_عبارت->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, اندازه(pSig[0]));
      اگر( pSig ){
        pSig->selId = p_عبارت->x.pSelect->selId;
        pSig->zAff = عبارت_INAffinity(pParse, p_عبارت);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    اگر( عبارت_Has_Property(p_عبارت, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_عبارت, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      اگر( عبارت_Use_XSelect(p_عبارت) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_عبارت->x.pSelect->selId));
      }
      أكد( عبارت_Use_YSub(p_عبارت) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_عبارت->y.sub.regReturn,
                        p_عبارت->y.sub.iAddr);
      أكد( iTab!=p_عبارت->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_عبارت->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      اگر( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      بازگشت;
    }

    /* Begin coding the subroutine */
    أكد( !عبارت_Use_YWin(p_عبارت) );
    عبارت_Set_Property(p_عبارت, EP_Subrtn);
    أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
    p_عبارت->y.sub.regReturn = ++pParse->nMem;
    p_عبارت->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_عبارت->y.sub.regReturn) + 1;
    اگر( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_عبارت->y.sub.iAddr;
      pSig->regReturn = p_عبارت->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ثابت نویسه*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_چپ = p_عبارت->p_چپ;
  nVal = sqlite3_عبارت_Vector_Size(p_چپ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_عبارت->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_عبارت->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  اگر( عبارت_Use_XSelect(p_عبارت) ){
    VdbeComment((v, "Result of SELECT %u", p_عبارت->x.pSelect->selId));
  }وگرنه{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_کلید_Info = sqlite3_کلید_Info_Alloc(pParse->db, nVal, 1);
  أكد( p_کلید_Info!=0 || pParse->nErr );
  اگر( p_کلید_Info==0 ) بازگشت;

  اگر( عبارت_Use_XSelect(p_عبارت) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_عبارت->x.pSelect;
    عبارت_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    اگر( ALWAYS(pEList->n_عبارت==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحیح i;
      صحیح rc;
      صحیح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = عبارت_INAffinity(pParse, p_عبارت);
      pSelect->iLimit = 0;
      أكد( pEList!=0 );
      أكد( pEList->n_عبارت>0 );
      أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
      برای(i=0; i<nVal; i++){
        عبارت *p = sqlite3VectorFieldSubexpr(p_چپ, i);
        CollSeq *pColl;
        p_کلید_Info->aColl[i] = pColl = sqlite3_دودویی_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_عبارت
        );
        اگر( !sqlite3_Is_دودویی(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      اگر( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحیح regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (تهی *)p_کلید_Info, P4_KEYINFO);
        p_کلید_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      اگر( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        اگر( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      اگر( rc ){
        sqlite3_کلید_Info_Unref(p_کلید_Info);
        بازگشت;
      }
    }
  }وگرنه اگر( ALWAYS(p_عبارت->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    نویسه affinity;            /* Affinity of the LHS of the IN */
    صحیح i;
    عبارت_List *pList = p_عبارت->x.pList;
    ساختار عبارت_List_item *pItem;
    صحیح r1, r2;
    affinity = sqlite3_عبارت_Affinity(p_چپ);
    اگر( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }وگرنه اگر( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( p_کلید_Info!=0 );
    أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
    p_کلید_Info->aColl[0] = sqlite3_عبارت_Coll_Seq(pParse, p_عبارت->p_چپ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    برای(i=pList->n_عبارت, pItem=pList->a; i>0; i--, pItem++){
      عبارت *pE2 = pItem->p_عبارت;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      اگر( addrOnce && !sqlite3_عبارت_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        عبارت_Clear_Property(p_عبارت, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_عبارت_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_درج, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  اگر( pSig ) pSig->bComplete = 1;
  اگر( p_کلید_Info ){
    sqlite3VdbeChangeP4(v, addr, (تهی *)p_کلید_Info, P4_KEYINFO);
  }
  اگر( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    أكد( عبارت_Use_YSub(p_عبارت) );
    أكد( sqlite3VdbeGetOp(v,p_عبارت->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_عبارت->y.sub.regReturn,
                      p_عبارت->y.sub.iAddr, 1);
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
صحیح sqlite3CodeSubselect(Parse *pParse, عبارت *p_عبارت){
  صحیح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحیح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحیح nReg;                   /* Registers to allocate */
  عبارت *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  اگر( pParse->nErr ) بازگشت 0;
  testcase( p_عبارت->op==TK_EXISTS );
  testcase( p_عبارت->op==TK_SELECT );
  أكد( p_عبارت->op==TK_EXISTS || p_عبارت->op==TK_SELECT );
  أكد( عبارت_Use_XSelect(p_عبارت) );
  pSel = p_عبارت->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  اگر( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( عبارت_Use_YSub(p_عبارت) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_عبارت->y.sub.regReturn,
                      p_عبارت->y.sub.iAddr);
    بازگشت p_عبارت->iTable;
  }

  /* Begin coding the subroutine */
  أكد( !عبارت_Use_YWin(p_عبارت) );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Reduced|EP_TokenOnly) );
  عبارت_Set_Property(p_عبارت, EP_Subrtn);
  p_عبارت->y.sub.regReturn = ++pParse->nMem;
  p_عبارت->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_عبارت->y.sub.regReturn) + 1;

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
  اگر( !عبارت_Has_Property(p_عبارت, EP_VarSelect) ){
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
  nReg = p_عبارت->op==TK_SELECT ? pSel->pEList->n_عبارت : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  اگر( p_عبارت->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    اگر( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_راست ){
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
    }وگرنه{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }وگرنه{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  اگر( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    عبارت *p_چپ = pSel->pLimit->p_چپ;
    اگر( عبارت_Has_Property(p_چپ, EP_Int_مقدار)==0
     || (p_چپ->u.i_مقدار!=1 && p_چپ->u.i_مقدار!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_عبارت_Int32(db, 0);
      اگر( pLimit ){
        pLimit->aff_عبارت = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_عبارت_Dup(db, p_چپ, 0), pLimit);
      }
      sqlite3_عبارت_Deferred_Delete(pParse, p_چپ);
      pSel->pLimit->p_چپ = pLimit;
    }
  }وگرنه{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_عبارت_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  اگر( sqlite3Select(pParse, pSel, &dest) ){
    p_عبارت->op2 = p_عبارت->op;
    p_عبارت->op = TK_ERROR;
    بازگشت 0;
  }
  p_عبارت->iTable = rReg = dest.iSDParm;
  عبارت_Set_VVAProperty(p_عبارت, EP_NoReduce);
  اگر( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  اگر( dest.eDest==SRT_Mem 
   && sqlite3_عبارت_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_عبارت) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  أكد( عبارت_Use_YSub(p_عبارت) );
  أكد( sqlite3VdbeGetOp(v,p_عبارت->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_عبارت->y.sub.regReturn,
                    p_عبارت->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  بازگشت rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحیح sqlite3_عبارت_Check_IN(Parse *pParse, عبارت *pIn){
  صحیح nVector = sqlite3_عبارت_Vector_Size(pIn->p_چپ);
  اگر( عبارت_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    اگر( nVector!=pIn->x.pSelect->pEList->n_عبارت ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_عبارت, nVector);
      بازگشت 1;
    }
  }وگرنه اگر( nVector!=1 ){
    sqlite3_Vector_Error_پیام(pParse, pIn->p_چپ);
    بازگشت 1;
  }
  بازگشت 0;
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
ایستا تهی sqlite3_عبارت_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  عبارت *p_عبارت,          /* The IN expression */
  صحیح destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  صحیح destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  صحیح rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  صحیح eType;            /* Type of the RHS */
  صحیح rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  صحیح *aiMap = 0;       /* Map from vector field to index column */
  نویسه *zAff = 0;       /* Affinity string for comparisons */
  صحیح nVector;          /* Size of vectors for this IN operator */
  صحیح iDummy;           /* Dummy parameter to exprCodeVector() */
  عبارت *p_چپ;          /* The LHS of the IN operator */
  صحیح i;                /* loop counter */
  صحیح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحیح destStep6 = 0;    /* Start of code for Step 6 */
  صحیح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحیح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحیح addr_بالاترین;          /* Top of the step-6 loop */
  صحیح iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  p_چپ = p_عبارت->p_چپ;
  اگر( sqlite3_عبارت_Check_IN(pParse, p_عبارت) ) بازگشت;
  zAff = عبارت_INAffinity(pParse, p_عبارت);
  nVector = sqlite3_عبارت_Vector_Size(p_عبارت->p_چپ);
  aiMap = (صحیح*)sqlite3DbMallocZero(pParse->db, nVector*اندازه(صحیح));
  اگر( pParse->db->mallocFailed ) برو sqlite3_عبارت_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_عبارت,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  أكد( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  برای(i=0; i<nVector; i++){
    صحیح j, cnt;
    برای(cnt=j=0; j<nVector; j++) اگر( aiMap[j]==i ) cnt++;
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
  rLhs = عبارت_Code_Vector(pParse, p_چپ, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  اگر( eType==IN_INDEX_NOOP ){
    عبارت_List *pList;
    CollSeq *pColl;
    صحیح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحیح r2, regToFree;
    صحیح regCkNull = 0;
    صحیح ii;
    أكد( nVector==1 );
    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_عبارت->p_چپ);
    اگر( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    برای(ii=0; ii<pList->n_عبارت; ii++){
      r2 = sqlite3_عبارت_Code_Temp(pParse, pList->a[ii].p_عبارت, &regToFree);
      اگر( regCkNull && sqlite3_عبارت_Can_Be_Null(pList->a[ii].p_عبارت) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      اگر( ii<pList->n_عبارت-1 || destIfNull!=destIfFalse ){
        صحیح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (تهی*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_عبارت-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_عبارت-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_عبارت-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_عبارت-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }وگرنه{
        صحیح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (تهی*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    اگر( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    برو sqlite3_عبارت_Code_IN_finished;
  }

  اگر( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    اگر( nVector==1 ){
      نویسه aff = zAff[0];
      اگر( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_عبارت_Affinity(p_چپ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحیح rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }وگرنه{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    برای(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    اگر( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      صحیح rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      برای(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  اگر( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }وگرنه{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  برای(i=0; i<nVector; i++){
    عبارت *p = sqlite3VectorFieldSubexpr(p_عبارت->p_چپ, i);
    اگر( pParse->nErr ) برو sqlite3_عبارت_Code_IN_oom_error;
    اگر( sqlite3_عبارت_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  اگر( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }وگرنه{
    اگر( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      اگر( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, p_عبارت->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        اگر( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      برو sqlite3_عبارت_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  اگر( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  اگر( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  اگر( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  اگر( eType==IN_INDEX_INDEX_DESC ){
    addr_بالاترین = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }وگرنه{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_بالاترین = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  اگر( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }وگرنه{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  برای(i=0; i<nVector; i++){
    عبارت *p;
    CollSeq *pColl;
    صحیح r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_چپ, i);
    اگر( عبارت_Use_XSelect(p_عبارت) ){
      عبارت *pRhs = p_عبارت->x.pSelect->pEList->a[i].p_عبارت;
      pColl = sqlite3_دودویی_Compare_Coll_Seq(pParse, p, pRhs);
    }وگرنه{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_عبارت_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (تهی*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  اگر( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_بالاترین+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_عبارت_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_عبارت_Code_IN_oom_error:
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
ایستا تهی codeReal(Vdbe *v, ثابت نویسه *z, صحیح negateFlag, صحیح iMem){
  اگر( ALWAYS(z!=0) ){
    دوگانه مقدار;
    sqlite3AtoF(z, &مقدار);
    أكد( !sqlite3IsNaN(مقدار) ); /* The new AtoF never returns NaN */
    اگر( negateFlag ) مقدار = -مقدار;
    sqlite3VdbeAddDouble(v, iMem, مقدار);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ایستا تهی codeInteger(Parse *pParse, عبارت *p_عبارت, صحیح negFlag, صحیح iMem){
  Vdbe *v = pParse->pVdbe;
  اگر( p_عبارت->flags & EP_Int_مقدار ){
    صحیح i = p_عبارت->u.i_مقدار;
    أكد( i>=0 );
    اگر( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }وگرنه{
    صحیح c;
    i64 مقدار;
    ثابت نویسه *z = p_عبارت->u.zToken;
    أكد( z!=0 );
    c = sqlite3DecOrHexToI64(z, &مقدار);
    اگر( (c==3 && !negFlag) || (c==2) || (negFlag && مقدار==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_پیام(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_عبارت);
#وگرنه
#ifndef SQLITE_OMIT_HEX_INTEGER
      اگر( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_پیام(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_عبارت);
      }وگرنه
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }وگرنه{
      اگر( negFlag ){ مقدار = c==3 ? SMALLEST_INT64 : -مقدار; }
      sqlite3VdbeAddInt64(v, iMem, مقدار);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
تهی sqlite3_عبارت_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  صحیح iTabCur,    /* Cursor pointing to a table row */
  صحیح iIdxCol,    /* The column of the index to be loaded */
  صحیح regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  اگر( iTabCol==XN_عبارت ){
    أكد( pIdx->a_Col_عبارت );
    أكد( pIdx->a_Col_عبارت->n_عبارت>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_عبارت_Code_Copy(pParse, pIdx->a_Col_عبارت->a[iIdxCol].p_عبارت, regOut);
    pParse->iSelfTab = 0;
  }وگرنه{
    sqlite3_عبارت_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
تهی sqlite3_عبارت_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  صحیح regOut         /* Put the result in this register */
){
  صحیح iAddr;
  Vdbe *v = pParse->pVdbe;
  صحیح nErr = pParse->nErr;
  أكد( v!=0 );
  أكد( pParse->iSelfTab!=0 );
  اگر( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }وگرنه{
    iAddr = 0;
  }
  sqlite3_عبارت_Code_Copy(pParse, sqlite3_Column_عبارت(pTab,pCol), regOut);
  اگر( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحیح p3 = 2+(صحیح)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (نویسه*)pTab, P4_TABLE);
  }وگرنه اگر( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  اگر( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  اگر( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
تهی sqlite3_عبارت_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  صحیح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحیح iCol,       /* Index of the column to extract */
  صحیح regOut      /* Extract the value into this register */
){
  Column *pCol;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( iCol!=XN_عبارت );
  اگر( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }وگرنه{
    صحیح x;
    اگر( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      بازگشت;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }وگرنه اگر( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      اگر( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_پیام(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }وگرنه{
        صحیح savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_عبارت_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      بازگشت;
#endif
    }وگرنه اگر( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_کلید_Index(pTab), iCol);
    }وگرنه{
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
صحیح sqlite3_عبارت_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  صحیح iColumn,     /* Index of the table column */
  صحیح iTable,      /* The cursor pointing to the table */
  صحیح iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  أكد( pParse->pVdbe!=0 );
  أكد( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  أكد( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_عبارت_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  اگر( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    اگر( pOp->opcode==OP_Column ) pOp->p5 = p5;
    اگر( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  بازگشت iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
تهی sqlite3_عبارت_Code_Move(Parse *pParse, صحیح iFrom, صحیح iTo, صحیح nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
تهی sqlite3_عبارت_To_Register(عبارت *p_عبارت, صحیح iReg){
  عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  اگر( NEVER(p==0) ) بازگشت;
  اگر( p->op==TK_REGISTER ){
    أكد( p->iTable==iReg );
  }وگرنه{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    عبارت_Clear_Property(p, EP_Skip);
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
ایستا صحیح عبارت_Code_Vector(Parse *pParse, عبارت *p, صحیح *piFreeable){
  صحیح iResult;
  صحیح nResult = sqlite3_عبارت_Vector_Size(p);
  اگر( nResult==1 ){
    iResult = sqlite3_عبارت_Code_Temp(pParse, p, piFreeable);
  }وگرنه{
    *piFreeable = 0;
    اگر( p->op==TK_SELECT ){
#اگر SQLITE_OMIT_SUBQUERY
      iResult = 0;
#وگرنه
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }وگرنه{
      صحیح i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      أكد( عبارت_Use_XList(p) );
      برای(i=0; i<nResult; i++){
        sqlite3_عبارت_Code_Factorable(pParse, p->x.pList->a[i].p_عبارت, i+iResult);
      }
    }
  }
  بازگشت iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ایستا تهی setDoNotMergeFlagOnCopy(Vdbe *v){
  اگر( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ایستا صحیح عبارت_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  عبارت_List *pFarg,      /* List of function arguments */
  صحیح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحیح target            /* Store function result in this register */
){
  صحیح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->n_عبارت;
  أكد( nFarg>0 );  /* All in-line functions have at least one argument */
  انتخاب( iFuncId ){
    حالت INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      صحیح endCoalesce = sqlite3VdbeMakeLabel(pParse);
      صحیح i;
      أكد( nFarg>=2 );
      sqlite3_عبارت_Code(pParse, pFarg->a[0].p_عبارت, target);
      برای(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_عبارت_Code(pParse, pFarg->a[i].p_عبارت, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      بشکن;
    }
    حالت INLINEFUNC_iif: {
      عبارت case_عبارت;
      memset(&case_عبارت, 0, اندازه(case_عبارت));
      case_عبارت.op = TK_CASE;
      case_عبارت.x.pList = pFarg;
      بازگشت sqlite3_عبارت_Code_Target(pParse, &case_عبارت, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    حالت INLINEFUNC_sqlite_offset: {
      عبارت *pArg = pFarg->a[0].p_عبارت;
      اگر( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }وگرنه{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      بشکن;
    }
#endif
    پیشفرض: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      أكد( nFarg==1 || nFarg==2 );
      target = sqlite3_عبارت_Code_Target(pParse, pFarg->a[0].p_عبارت, target);
      بشکن;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#اگر !defined(SQLITE_UNTESTABLE)
    حالت INLINEFUNC_عبارت_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_عبارت_Compare(0,pFarg->a[0].p_عبارت, pFarg->a[1].p_عبارت,-1),
         target);
      بشکن;
    }

    حالت INLINEFUNC_عبارت_implies_عبارت: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_عبارت_Implies_عبارت(pParse,pFarg->a[0].p_عبارت, pFarg->a[1].p_عبارت,-1),
         target);
      بشکن;
    }

    حالت INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      عبارت *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].p_عبارت;
      اگر( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_عبارت_Implies_Non_Null_Row(pFarg->a[0].p_عبارت,pA1->iTable,1),
           target);
      }وگرنه{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      بشکن;
    }

    حالت INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ثابت نویسه *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      نویسه aff;
      أكد( nFarg==1 );
      aff = sqlite3_عبارت_Affinity(pFarg->a[0].p_عبارت);
      أكد( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      بشکن;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  بازگشت target;
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
ایستا صحیح عبارت_گره_Can_Return_Subtype(Walker *pWalker, عبارت *p_عبارت){
  صحیح n;
  FuncDef *pDef;
  sqlite3 *db;
  اگر( p_عبارت->op==TK_CASE || p_عبارت->op==TK_UPLUS 
   || p_عبارت->op==TK_COLLATE || p_عبارت->op==TK_CAST 
  ){
    بازگشت WRC_Continue;
  }
  اگر( p_عبارت->op!=TK_FUNCTION && p_عبارت->op!=TK_AGG_FUNCTION ){
    بازگشت WRC_Prune;
  }
  أكد( عبارت_Use_XList(p_عبارت) );
  db = pWalker->pParse->db;
  n = p_عبارت->x.pList ? p_عبارت->x.pList->n_عبارت : 0;
  pDef = sqlite3FindFunction(db, p_عبارت->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  اگر( pDef==0 ) بازگشت WRC_Continue;
#وگرنه
  أكد( pDef!=0 );
#endif
  اگر( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    بازگشت WRC_Abort;
  }
  بازگشت WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
صحیح sqlite3_عبارت_Can_Return_Subtype(Parse *pParse, عبارت *p_عبارت){
  Walker w;
  memset(&w, 0, اندازه(w));
  w.pParse = pParse;
  w.x_عبارت_Callback = عبارت_گره_Can_Return_Subtype;
  sqlite3_Walk_عبارت(&w, p_عبارت);
  بازگشت w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ایستا SQLITE_NOINLINE صحیح sqlite3_Indexed_عبارت_Lookup(
  Parse *pParse,   /* The parsing context */
  عبارت *p_عبارت,     /* The expression to potentially bypass */
  صحیح target       /* Where to store the result of the expression */
){
  Indexed_عبارت *p;
  Vdbe *v;
  برای(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 عبارت_Aff;
    صحیح iDataCur = p->iDataCur;
    اگر( iDataCur<0 ) ادامه;
    اگر( pParse->iSelfTab ){
      اگر( p->iDataCur!=pParse->iSelfTab-1 ) ادامه;
      iDataCur = -1;
    }
    اگر( sqlite3_عبارت_Compare(0, p_عبارت, p->p_عبارت, iDataCur)!=0 ) ادامه;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    عبارت_Aff = sqlite3_عبارت_Affinity(p_عبارت);
    اگر( (عبارت_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (عبارت_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (عبارت_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ادامه;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    اگر( عبارت_Has_Property(p_عبارت, EP_SubtArg)
     && sqlite3_عبارت_Can_Return_Subtype(pParse, p_عبارت) 
    ){
      ادامه;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    اگر( p->bMaybeNullRow || sqlite3_عبارت_Is_Constant(pParse, p_عبارت) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      صحیح addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_عبارت_Code(pParse, p_عبارت, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }وگرنه{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    بازگشت target;
  }
  بازگشت -1;  /* Not found */
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
ایستا صحیح عبارت_Partidx_عبارت_Lookup(Parse *pParse, عبارت *p_عبارت, صحیح iTarget){
  Indexed_عبارت *p;
  برای(p=pParse->p_Idx_Part_عبارت; p; p=p->pIENext){
    اگر( p_عبارت->iColumn==p->iIdxCol && p_عبارت->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      صحیح addr = 0;
      صحیح ret;

      اگر( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_عبارت_Code_Target(pParse, p->p_عبارت, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت نویسه*)&p->aff, 1);
      اگر( sqlite3_عبارت_Can_Return_Subtype(pParse, p->p_عبارت) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      اگر( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      بازگشت ret;
    }
  }
  بازگشت 0;
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
ایستا SQLITE_NOINLINE صحیح عبارت_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  عبارت *p_عبارت,       /* AND or OR expression to be coded */
  صحیح target,        /* Put result in this register, guaranteed */
  صحیح *pTmpReg       /* Write a temporary register here */
){
  صحیح op;            /* The opcode.  TK_AND or TK_OR */
  صحیح skipOp;        /* Opcode for the branch that skips one operand */
  صحیح addrSkip;      /* Branch instruction that skips one of the operands */
  صحیح regSS = 0;     /* Register holding computed operand when other omitted */
  صحیح r1, r2;        /* Registers for left and right operands, respectively */
  عبارت *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  أكد( p_عبارت!=0 );
  op = p_عبارت->op;
  أكد( op==TK_AND || op==TK_OR );
  أكد( TK_AND==OP_And );            testcase( op==TK_AND );
  أكد( TK_OR==OP_Or );              testcase( op==TK_OR );
  أكد( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
  اگر( pAlt!=p_عبارت ){
    r1 = sqlite3_عبارت_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    بازگشت target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  اگر( عبارت_Eval_Rhs_First(p_عبارت) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_عبارت_Code_Target(pParse, p_عبارت->p_راست, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, pTmpReg);
  }وگرنه{
    /* Compute the left operand first */
    r1 = sqlite3_عبارت_Code_Target(pParse, p_عبارت->p_چپ, target);
    اگر( عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }وگرنه{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  اگر( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  بازگشت target;
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
صحیح sqlite3_عبارت_Code_Target(Parse *pParse, عبارت *p_عبارت, صحیح target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحیح op;                   /* The opcode being coded */
  صحیح inReg = target;       /* Results stored in register inReg */
  صحیح regFree1 = 0;         /* If non-zero free this temporary register */
  صحیح regFree2 = 0;         /* If non-zero free this temporary register */
  صحیح r1, r2;               /* Various register numbers */
  عبارت tempX;               /* Temporary expression node */
  صحیح p5 = 0;

  أكد( target>0 && target<=pParse->nMem );
  أكد( v!=0 );

عبارت_code_doover:
  اگر( p_عبارت==0 ){
    op = TK_NULL;
  }وگرنه اگر( pParse->pIdxEpr!=0
   && !عبارت_Has_Property(p_عبارت, EP_Leaf)
   && (r1 = sqlite3_Indexed_عبارت_Lookup(pParse, p_عبارت, target))>=0
  ){
    بازگشت r1;
  }وگرنه{
    أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
    op = p_عبارت->op;
  }
  أكد( op!=TK_ORDER );
  انتخاب( op ){
    حالت TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_عبارت->pAggInfo;
      ساختار AggInfo_col *pCol;
      أكد( pAggInfo!=0 );
      أكد( p_عبارت->iAgg>=0 );
      اگر( p_عبارت->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        بشکن;
      }
      pCol = &pAggInfo->aCol[p_عبارت->iAgg];
      اگر( !pAggInfo->directMode ){
        بازگشت AggInfoColumnReg(pAggInfo, p_عبارت->iAgg);
      }وگرنه اگر( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        اگر( pTab==0 ){
          /* No comment added */
        }وگرنه اگر( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }وگرنه{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          اگر( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        بازگشت target;
      }وگرنه اگر( p_عبارت->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_عبارت->iTable, p_عبارت->iColumn, target);
        بازگشت target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالت TK_COLUMN: {
      صحیح iTab = p_عبارت->iTable;
      صحیح iReg;
      اگر( عبارت_Has_Property(p_عبارت, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحیح aff;
        iReg = sqlite3_عبارت_Code_Target(pParse, p_عبارت->p_چپ,target);
        أكد( عبارت_Use_YTab(p_عبارت) );
        أكد( p_عبارت->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_عبارت->y.pTab, p_عبارت->iColumn);
        اگر( aff>SQLITE_AFF_BLOB ){
          ایستا ثابت نویسه zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        بازگشت iReg;
      }
      اگر( iTab<0 ){
        اگر( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          صحیح iSrc;
          صحیح iCol = p_عبارت->iColumn;
          أكد( عبارت_Use_YTab(p_عبارت) );
          pTab = p_عبارت->y.pTab;
          أكد( pTab!=0 );
          أكد( iCol>=XN_ROWID );
          أكد( iCol<pTab->nCol );
          اگر( iCol<0 ){
            بازگشت -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          اگر( pCol->colFlags & COLFLAG_GENERATED ){
            اگر( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_پیام(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              بازگشت 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            اگر( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_عبارت_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            بازگشت iSrc;
          }وگرنه
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          اگر( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            بازگشت target;
          }وگرنه{
            بازگشت iSrc;
          }
        }وگرنه{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      وگرنه اگر( pParse->p_Idx_Part_عبارت 
       && 0!=(r1 = عبارت_Partidx_عبارت_Lookup(pParse, p_عبارت, target))
      ){
        بازگشت r1;
      }
      أكد( عبارت_Use_YTab(p_عبارت) );
      أكد( p_عبارت->y.pTab!=0 );
      iReg = sqlite3_عبارت_Code_Get_Column(pParse, p_عبارت->y.pTab,
                               p_عبارت->iColumn, iTab, target,
                               p_عبارت->op2);
      بازگشت iReg;
    }
    حالت TK_INTEGER: {
      codeInteger(pParse, p_عبارت, 0, target);
      بازگشت target;
    }
    حالت TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_عبارت_Truth_مقدار(p_عبارت), target);
      بازگشت target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالت TK_FLOAT: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      codeReal(v, p_عبارت->u.zToken, 0, target);
      بازگشت target;
    }
#endif
    حالت TK_STRING: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3VdbeLoadString(v, target, p_عبارت->u.zToken);
      بازگشت target;
    }
    حالت TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_عبارت->y.nReg - 1);
      بازگشت target;
    }
    پیشفرض: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      بازگشت target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالت TK_BLOB: {
      صحیح n;
      ثابت نویسه *z;
      نویسه *zBlob;
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      أكد( p_عبارت->u.zToken[0]=='x' || p_عبارت->u.zToken[0]=='X' );
      أكد( p_عبارت->u.zToken[1]=='\'' );
      z = &p_عبارت->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      بازگشت target;
    }
#endif
    حالت TK_VARIABLE: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      أكد( p_عبارت->u.zToken!=0 );
      أكد( p_عبارت->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_عبارت->iColumn, target);
      بازگشت target;
    }
    حالت TK_REGISTER: {
      بازگشت p_عبارت->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    حالت TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_عبارت_Code(pParse, p_عبارت->p_چپ, target);
      أكد( inReg==target );
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_عبارت->u.zToken, 0));
      بازگشت inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    حالت TK_IS:
    حالت TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE:
    حالت TK_NE:
    حالت TK_EQ: {
      عبارت *p_چپ = p_عبارت->p_چپ;
      صحیح addrIsNull = 0;
      اگر( sqlite3_عبارت_Is_Vector(p_چپ) ){
        codeVectorCompare(pParse, p_عبارت, target, op, p5);
      }وگرنه{
        اگر( عبارت_Has_Property(p_عبارت, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                     &r1, &r2, &regFree1, &regFree2);
        }وگرنه{
          r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
          r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_چپ, p_عبارت->p_راست, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            عبارت_Has_Property(p_عبارت,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        اگر( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }وگرنه{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          اگر( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      بشکن;
    }
    حالت TK_AND:
    حالت TK_OR: {
      inReg = عبارت_Code_Target_And_Or(pParse, p_عبارت, target, &regFree1);
      بشکن;
    }
    حالت TK_PLUS:
    حالت TK_STAR:
    حالت TK_MINUS:
    حالت TK_REM:
    حالت TK_BITAND:
    حالت TK_BITOR:
    حالت TK_SLASH:
    حالت TK_LSHIFT:
    حالت TK_RSHIFT:
    حالت TK_CONCAT: {
      صحیح addrIsNull;
      أكد( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      أكد( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      أكد( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      أكد( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      أكد( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      أكد( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      أكد( TK_LSHIFT==OP_Shift_چپ );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_Shift_راست );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      اگر( عبارت_Has_Property(p_عبارت, EP_Subquery) ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      اگر( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      بشکن;
    }
    حالت TK_UMINUS: {
      عبارت *p_چپ = p_عبارت->p_چپ;
      أكد( p_چپ );
      اگر( p_چپ->op==TK_INTEGER ){
        codeInteger(pParse, p_چپ, 1, target);
        بازگشت target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }وگرنه اگر( p_چپ->op==TK_FLOAT ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        codeReal(v, p_چپ->u.zToken, 1, target);
        بازگشت target;
#endif
      }وگرنه{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_مقدار|EP_TokenOnly;
        tempX.u.i_مقدار = 0;
        عبارت_Clear_VVAProperties(&tempX);
        r1 = sqlite3_عبارت_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      بشکن;
    }
    حالت TK_BITNOT:
    حالت TK_NOT: {
      أكد( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      بشکن;
    }
    حالت TK_TRUTH: {
      صحیح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحیح bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      bNormal = p_عبارت->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      بشکن;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      صحیح addr;
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      بشکن;
    }
    حالت TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_عبارت->pAggInfo;
      اگر( pInfo==0
       || NEVER(p_عبارت->iAgg<0)
       || NEVER(p_عبارت->iAgg>=pInfo->nFunc)
      ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        sqlite3_Error_پیام(pParse, "misuse of aggregate: %#T()", p_عبارت);
      }وگرنه{
        بازگشت AggInfoFuncReg(pInfo, p_عبارت->iAgg);
      }
      بشکن;
    }
    حالت TK_FUNCTION: {
      عبارت_List *pFarg;       /* List of function arguments */
      صحیح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت نویسه *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      صحیح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      اگر( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
        بازگشت p_عبارت->y.pWin->regResult;
      }
#endif

      اگر( ConstFactorOk(pParse)
       && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        بازگشت sqlite3_عبارت_Code_Run_Just_Once(pParse, p_عبارت, -1);
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly) );
      أكد( عبارت_Use_XList(p_عبارت) );
      pFarg = p_عبارت->x.pList;
      nFarg = pFarg ? pFarg->n_عبارت : 0;
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      zId = p_عبارت->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      اگر( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      اگر( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_پیام(pParse, "unknown function: %#T()", p_عبارت);
        بشکن;
      }
      اگر( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        بازگشت عبارت_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }وگرنه اگر( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_عبارت_Function_Usable(pParse, p_عبارت, pDef);
      }

      برای(i=0; i<nFarg; i++){
        اگر( i<32 && sqlite3_عبارت_Is_Constant(pParse, pFarg->a[i].p_عبارت) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        اگر( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_عبارت_Coll_Seq(pParse, pFarg->a[i].p_عبارت);
        }
      }
      اگر( pFarg ){
        اگر( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }وگرنه{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        اگر( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 عبارت_Op;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].p_عبارت!=0 );
          عبارت_Op = pFarg->a[0].p_عبارت->op;
          اگر( عبارت_Op==TK_COLUMN || عبارت_Op==TK_AGG_COLUMN ){
            أكد( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_عبارت->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_عبارت_Code_عبارت_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }وگرنه{
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
      اگر( nFarg>=2 && عبارت_Has_Property(p_عبارت, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_عبارت);
      }وگرنه اگر( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_عبارت);
      }
#endif
      اگر( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        اگر( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (نویسه *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_عبارت->op2);
      اگر( nFarg ){
        اگر( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }وگرنه{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      بازگشت target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالت TK_EXISTS:
    حالت TK_SELECT: {
      صحیح nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      اگر( pParse->db->mallocFailed ){
        بازگشت 0;
      }وگرنه اگر( op==TK_SELECT
             && ALWAYS( عبارت_Use_XSelect(p_عبارت) )
             && (nCol = p_عبارت->x.pSelect->pEList->n_عبارت)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }وگرنه{
        بازگشت sqlite3CodeSubselect(pParse, p_عبارت);
      }
      بشکن;
    }
    حالت TK_SELECT_COLUMN: {
      صحیح n;
      عبارت *p_چپ = p_عبارت->p_چپ;
      اگر( p_چپ->iTable==0 || pParse->withinRJSubrtn > p_چپ->op2 ){
        p_چپ->iTable = sqlite3CodeSubselect(pParse, p_چپ);
        p_چپ->op2 = pParse->withinRJSubrtn;
      }
      أكد( p_چپ->op==TK_SELECT || p_چپ->op==TK_ERROR );
      n = sqlite3_عبارت_Vector_Size(p_چپ);
      اگر( p_عبارت->iTable!=n ){
        sqlite3_Error_پیام(pParse, "%d columns assigned %d values",
                                p_عبارت->iTable, n);
      }
      بازگشت p_چپ->iTable + p_عبارت->iColumn;
    }
    حالت TK_IN: {
      صحیح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحیح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_عبارت_Code_IN(pParse, p_عبارت, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      بازگشت target;
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
    حالت TK_BETWEEN: {
      عبارت_Code_Between(pParse, p_عبارت, target, 0, 0);
      بازگشت target;
    }
    حالت TK_COLLATE: {
      اگر( !عبارت_Has_Property(p_عبارت, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( p_عبارت->p_چپ );
        sqlite3_عبارت_Code(pParse, p_عبارت->p_چپ, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        بازگشت target;
      }وگرنه{
        p_عبارت = p_عبارت->p_چپ;
        برو عبارت_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالت TK_SPAN:
    حالت TK_UPLUS: {
      p_عبارت = p_عبارت->p_چپ;
      برو عبارت_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    حالت TK_TRIGGER: {
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
      صحیح iCol;
      صحیح p1;

      أكد( عبارت_Use_YTab(p_عبارت) );
      pTab = p_عبارت->y.pTab;
      iCol = p_عبارت->iColumn;
      p1 = p_عبارت->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      أكد( p_عبارت->iTable==0 || p_عبارت->iTable==1 );
      أكد( iCol>=-1 && iCol<pTab->nCol );
      أكد( pTab->iPKey<0 || iCol!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_عبارت->iTable ? "new" : "old"),
        (p_عبارت->iColumn<0 ? "rowid" : p_عبارت->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      اگر( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      بشکن;
    }

    حالت TK_VECTOR: {
      sqlite3_Error_پیام(pParse, "row value misused");
      بشکن;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    حالت TK_IF_NULL_ROW: {
      صحیح addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_عبارت->pAggInfo;
      اگر( pAggInfo ){
        أكد( p_عبارت->iAgg>=0 && p_عبارت->iAgg<pAggInfo->nColumn );
        اگر( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_عبارت->iAgg);
          بشکن;
        }
        اگر( p_عبارت->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_عبارت->iAgg].iSorterColumn,
                            target);
          inReg = target;
          بشکن;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_عبارت->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_عبارت_Code(pParse, p_عبارت->p_چپ, target);
      أكد( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      بشکن;
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
    حالت TK_CASE: {
      صحیح endLabel;                     /* GOTO label for end of CASE stmt */
      صحیح nextCase;                     /* GOTO label for next WHEN clause */
      صحیح n_عبارت;                        /* 2x number of WHEN terms */
      صحیح i;                            /* Loop counter */
      عبارت_List *pEList;                 /* List of WHEN terms */
      ساختار عبارت_List_item *aListelem;  /* Array of WHEN terms */
      عبارت opCompare;                   /* The X==Ei expression */
      عبارت *pX;                         /* The X expression */
      عبارت *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      عبارت *pDel = 0;
      sqlite3 *db = pParse->db;

      أكد( عبارت_Use_XList(p_عبارت) && p_عبارت->x.pList!=0 );
      أكد(p_عبارت->x.pList->n_عبارت > 0);
      pEList = p_عبارت->x.pList;
      aListelem = pEList->a;
      n_عبارت = pEList->n_عبارت;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      اگر( (pX = p_عبارت->p_چپ)!=0 ){
        pDel = sqlite3_عبارت_Dup(db, pX, 0);
        اگر( db->mallocFailed ){
          sqlite3_عبارت_Delete(db, pDel);
          بشکن;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_عبارت_To_Register(pDel, عبارت_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, اندازه(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_چپ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      برای(i=0; i<n_عبارت-1; i=i+2){
        اگر( pX ){
          أكد( pTest!=0 );
          opCompare.p_راست = aListelem[i].p_عبارت;
        }وگرنه{
          pTest = aListelem[i].p_عبارت;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_عبارت_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_عبارت->op==TK_COLUMN );
        sqlite3_عبارت_Code(pParse, aListelem[i+1].p_عبارت, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      اگر( (n_عبارت&1)!=0 ){
        sqlite3_عبارت_Code(pParse, pEList->a[n_عبارت-1].p_عبارت, target);
      }وگرنه{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_عبارت_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      بشکن;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالت TK_RAISE: {
      أكد( p_عبارت->aff_عبارت==OE_Rollback
           || p_عبارت->aff_عبارت==OE_Abort
           || p_عبارت->aff_عبارت==OE_Fail
           || p_عبارت->aff_عبارت==OE_Ignore
      );
      اگر( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_پیام(pParse,
                       "RAISE() may only be used within a trigger-program");
        بازگشت 0;
      }
      اگر( p_عبارت->aff_عبارت==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      اگر( p_عبارت->aff_عبارت==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_عبارت->aff_عبارت, r1);
      }
      بشکن;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  بازگشت inReg;
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
صحیح sqlite3_عبارت_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  عبارت *p_عبارت,      /* The expression to code when the VDBE initializes */
  صحیح regDest       /* Store the value in this register */
){
  عبارت_List *p;
  أكد( ConstFactorOk(pParse) );
  أكد( regDest!=0 );
  p = pParse->p_Const_عبارت;
  اگر( regDest<0 && p ){
    ساختار عبارت_List_item *pItem;
    صحیح i;
    برای(pItem=p->a, i=p->n_عبارت; i>0; pItem++, i--){
      اگر( pItem->fg.reusable
       && sqlite3_عبارت_Compare(0,pItem->p_عبارت,p_عبارت,-1)==0
      ){
        بازگشت pItem->u.i_Const_عبارت_Reg;
      }
    }
  }
  p_عبارت = sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0);
  اگر( p_عبارت!=0 && عبارت_Has_Property(p_عبارت, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحیح addr;
    أكد( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    اگر( !pParse->db->mallocFailed ){
      اگر( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_عبارت_Code(pParse, p_عبارت, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_عبارت_Delete(pParse->db, p_عبارت);
    sqlite3VdbeJumpHere(v, addr);
  }وگرنه{
    p = sqlite3_عبارت_List_Append(pParse, p, p_عبارت);
    اگر( p ){
       ساختار عبارت_List_item *pItem = &p->a[p->n_عبارت-1];
       pItem->fg.reusable = regDest<0;
       اگر( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_عبارت_Reg = regDest;
    }
    pParse->p_Const_عبارت = p;
  }
  بازگشت regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE تهی sqlite3_عبارت_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  صحیح iReg,        /* First register to set to NULL */
  صحیح nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  عبارت t;
  memset(&t, 0, اندازه(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_عبارت_Code_Run_Just_Once(pParse, &t, iReg);
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
صحیح sqlite3_عبارت_Code_Temp(Parse *pParse, عبارت *p_عبارت, صحیح *pReg){
  صحیح r2;
  p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  اگر( ConstFactorOk(pParse)
   && ALWAYS(p_عبارت!=0)
   && p_عبارت->op!=TK_REGISTER
   && sqlite3_عبارت_Is_Constant_Not_Join(pParse, p_عبارت)
  ){
    *pReg  = 0;
    r2 = sqlite3_عبارت_Code_Run_Just_Once(pParse, p_عبارت, -1);
  }وگرنه{
    صحیح r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_عبارت_Code_Target(pParse, p_عبارت, r1);
    اگر( r2==r1 ){
      *pReg = r1;
    }وگرنه{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  بازگشت r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
تهی sqlite3_عبارت_Code(Parse *pParse, عبارت *p_عبارت, صحیح target){
  صحیح inReg;

  أكد( p_عبارت==0 || !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  أكد( target>0 && target<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  اگر( pParse->pVdbe==0 ) بازگشت;
  inReg = sqlite3_عبارت_Code_Target(pParse, p_عبارت, target);
  اگر( inReg!=target ){
    u8 op;
    عبارت *pX = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
    testcase( pX!=p_عبارت );
    اگر( ALWAYS(pX)
     && (عبارت_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }وگرنه{
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
تهی sqlite3_عبارت_Code_Copy(Parse *pParse, عبارت *p_عبارت, صحیح target){
  sqlite3 *db = pParse->db;
  p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
  اگر( !db->mallocFailed ) sqlite3_عبارت_Code(pParse, p_عبارت, target);
  sqlite3_عبارت_Delete(db, p_عبارت);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
تهی sqlite3_عبارت_Code_Factorable(Parse *pParse, عبارت *p_عبارت, صحیح target){
  اگر( pParse->okConstFactor && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت) ){
    sqlite3_عبارت_Code_Run_Just_Once(pParse, p_عبارت, target);
  }وگرنه{
    sqlite3_عبارت_Code_Copy(pParse, p_عبارت, target);
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
صحیح sqlite3_عبارت_Code_عبارت_List(
  Parse *pParse,     /* Parsing context */
  عبارت_List *pList,   /* The expression list to be coded */
  صحیح target,        /* Where to write results */
  صحیح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ساختار عبارت_List_item *pItem;
  صحیح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( target>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_عبارت;
  اگر( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  برای(pItem=pList->a, i=0; i<n; i++, pItem++){
    عبارت *p_عبارت = pItem->p_عبارت;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    اگر( pItem->fg.bSorterRef ){
      i--;
      n--;
    }وگرنه
#endif
    اگر( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      اگر( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }وگرنه{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }وگرنه اگر( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت)
    ){
      sqlite3_عبارت_Code_Run_Just_Once(pParse, p_عبارت, target+i);
    }وگرنه{
      صحیح inReg = sqlite3_عبارت_Code_Target(pParse, p_عبارت, target+i);
      اگر( inReg!=target+i ){
        VdbeOp *pOp;
        اگر( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }وگرنه{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  بازگشت n;
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
ایستا تهی عبارت_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  عبارت *p_عبارت,      /* The BETWEEN expression */
  صحیح dest,         /* Jump destination or storage location */
  تهی (*xJump)(Parse*,عبارت*,صحیح,صحیح), /* Action to take */
  صحیح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  عبارت عبارت_And;     /* The AND operator in  x>=y AND x<=z  */
  عبارت comp_چپ;    /* The  x>=y  term */
  عبارت comp_راست;   /* The  x<=z  term */
  صحیح regFree1 = 0; /* Temporary use register */
  عبارت *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_چپ, 0, اندازه(عبارت));
  memset(&comp_راست, 0, اندازه(عبارت));
  memset(&عبارت_And, 0, اندازه(عبارت));

  أكد( عبارت_Use_XList(p_عبارت) );
  pDel = sqlite3_عبارت_Dup(db, p_عبارت->p_چپ, 0);
  اگر( db->mallocFailed==0 ){
    عبارت_And.op = TK_AND;
    عبارت_And.p_چپ = &comp_چپ;
    عبارت_And.p_راست = &comp_راست;
    comp_چپ.op = TK_GE;
    comp_چپ.p_چپ = pDel;
    comp_چپ.p_راست = p_عبارت->x.pList->a[0].p_عبارت;
    comp_راست.op = TK_LE;
    comp_راست.p_چپ = pDel;
    comp_راست.p_راست = p_عبارت->x.pList->a[1].p_عبارت;
    sqlite3_عبارت_To_Register(pDel, عبارت_Code_Vector(pParse, pDel, &regFree1));
    اگر( xJump ){
      xJump(pParse, &عبارت_And, dest, jumpIfNull);
    }وگرنه{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_عبارت_Code_Target(pParse, &عبارت_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_عبارت_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_عبارت_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_عبارت_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_عبارت_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_عبارت_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_عبارت_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_عبارت_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_عبارت_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_عبارت_If_False && jumpIfNull!=0 && regFree1!=0 );
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
تهی sqlite3_عبارت_If_True(Parse *pParse, عبارت *p_عبارت, صحیح dest, صحیح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحیح op = 0;
  صحیح regFree1 = 0;
  صحیح regFree2 = 0;
  صحیح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  اگر( NEVER(v==0) )     بازگشت;  /* Existence of VDBE checked by caller */
  اگر( NEVER(p_عبارت==0) ) بازگشت;  /* No way this can happen */
  أكد( !عبارت_Has_VVAProperty(p_عبارت, EP_Immutable) );
  op = p_عبارت->op;
  انتخاب( op ){
    حالت TK_AND:
    حالت TK_OR: {
      عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
      اگر( pAlt!=p_عبارت ){
        sqlite3_عبارت_If_True(pParse, pAlt, dest, jumpIfNull);
      }وگرنه{
        عبارت *pFirst, *pSecond;
        اگر( عبارت_Eval_Rhs_First(p_عبارت) ){
          pFirst = p_عبارت->p_راست;
          pSecond = p_عبارت->p_چپ;
        }وگرنه{
          pFirst = p_عبارت->p_چپ;
          pSecond = p_عبارت->p_راست;
        }
        اگر( op==TK_AND ){
          صحیح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_عبارت_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }وگرنه{
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_عبارت_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      بشکن;
    }
    حالت TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest, jumpIfNull);
      بشکن;
    }
    حالت TK_TRUTH: {
      صحیح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحیح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_عبارت->op2==TK_ISNOT;
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      اگر( isTrue ^ isNot ){
        sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }وگرنه{
        sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      بشکن;
    }
    حالت TK_IS:
    حالت TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE:
    حالت TK_NE:
    حالت TK_EQ: {
      صحیح addrIsNull;
      اگر( sqlite3_عبارت_Is_Vector(p_عبارت->p_چپ) ) برو default_عبارت;
      اگر( عبارت_Has_Property(p_عبارت, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_عبارت->p_چپ, p_عبارت->p_راست, op,
                  r1, r2, dest, jumpIfNull, عبارت_Has_Property(p_عبارت,EP_Commuted));
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
      اگر( addrIsNull ){
        اگر( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }وگرنه{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      بشکن;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      اگر( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      بشکن;
    }
    حالت TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      عبارت_Code_Between(pParse, p_عبارت, dest, sqlite3_عبارت_If_True, jumpIfNull);
      بشکن;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالت TK_IN: {
      صحیح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحیح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_عبارت_Code_IN(pParse, p_عبارت, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      بشکن;
    }
#endif
    پیشفرض: {
    default_عبارت:
      اگر( عبارت_Always_True(p_عبارت) ){
        sqlite3VdbeGoto(v, dest);
      }وگرنه اگر( عبارت_Always_False(p_عبارت) ){
        /* No-op */
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      بشکن;
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
تهی sqlite3_عبارت_If_False(Parse *pParse, عبارت *p_عبارت, صحیح dest, صحیح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحیح op = 0;
  صحیح regFree1 = 0;
  صحیح regFree2 = 0;
  صحیح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  اگر( NEVER(v==0) ) بازگشت; /* Existence of VDBE checked by caller */
  اگر( p_عبارت==0 )    بازگشت;
  أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );

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
  op = ((p_عبارت->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  أكد( p_عبارت->op!=TK_ISNULL || op==OP_NotNull );
  أكد( p_عبارت->op!=TK_NOTNULL || op==OP_IsNull );
  أكد( p_عبارت->op!=TK_NE || op==OP_Eq );
  أكد( p_عبارت->op!=TK_EQ || op==OP_Ne );
  أكد( p_عبارت->op!=TK_LT || op==OP_Ge );
  أكد( p_عبارت->op!=TK_LE || op==OP_Gt );
  أكد( p_عبارت->op!=TK_GT || op==OP_Le );
  أكد( p_عبارت->op!=TK_GE || op==OP_Lt );

  انتخاب( p_عبارت->op ){
    حالت TK_AND:
    حالت TK_OR: {
      عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
      اگر( pAlt!=p_عبارت ){
        sqlite3_عبارت_If_False(pParse, pAlt, dest, jumpIfNull);
      }وگرنه{
        عبارت *pFirst, *pSecond;
        اگر( عبارت_Eval_Rhs_First(p_عبارت) ){
          pFirst = p_عبارت->p_راست;
          pSecond = p_عبارت->p_چپ;
        }وگرنه{
          pFirst = p_عبارت->p_چپ;
          pSecond = p_عبارت->p_راست;
        }
        اگر( p_عبارت->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_عبارت_If_False(pParse, pSecond, dest, jumpIfNull);
        }وگرنه{
          صحیح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_عبارت_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      بشکن;
    }
    حالت TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest, jumpIfNull);
      بشکن;
    }
    حالت TK_TRUTH: {
      صحیح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحیح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_عبارت->op2==TK_ISNOT;
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      اگر( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }وگرنه{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      بشکن;
    }
    حالت TK_IS:
    حالت TK_ISNOT:
      testcase( p_عبارت->op==TK_IS );
      testcase( p_عبارت->op==TK_ISNOT );
      op = (p_عبارت->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE:
    حالت TK_NE:
    حالت TK_EQ: {
      صحیح addrIsNull;
      اگر( sqlite3_عبارت_Is_Vector(p_عبارت->p_چپ) ) برو default_عبارت;
      اگر( عبارت_Has_Property(p_عبارت, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_عبارت->p_چپ, p_عبارت->p_راست, op,
                  r1, r2, dest, jumpIfNull,عبارت_Has_Property(p_عبارت,EP_Commuted));
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
      اگر( addrIsNull ){
        اگر( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }وگرنه{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      بشکن;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت->p_چپ, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      اگر( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      بشکن;
    }
    حالت TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      عبارت_Code_Between(pParse, p_عبارت, dest, sqlite3_عبارت_If_False, jumpIfNull);
      بشکن;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالت TK_IN: {
      اگر( jumpIfNull ){
        sqlite3_عبارت_Code_IN(pParse, p_عبارت, dest, dest);
      }وگرنه{
        صحیح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_عبارت_Code_IN(pParse, p_عبارت, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      بشکن;
    }
#endif
    پیشفرض: {
    default_عبارت:
      اگر( عبارت_Always_False(p_عبارت) ){
        sqlite3VdbeGoto(v, dest);
      }وگرنه اگر( عبارت_Always_True(p_عبارت) ){
        /* no-op */
      }وگرنه{
        r1 = sqlite3_عبارت_Code_Temp(pParse, p_عبارت, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      بشکن;
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
تهی sqlite3_عبارت_If_False_Dup(Parse *pParse, عبارت *p_عبارت, صحیح dest,صحیح jumpIfNull){
  sqlite3 *db = pParse->db;
  عبارت *pCopy = sqlite3_عبارت_Dup(db, p_عبارت, 0);
  اگر( db->mallocFailed==0 ){
    sqlite3_عبارت_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_عبارت_Delete(db, pCopy);
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
ایستا SQLITE_NOINLINE صحیح عبارت_Compare_Variable(
  ثابت Parse *pParse,
  ثابت عبارت *pVar,
  ثابت عبارت *p_عبارت
){
  صحیح res = 2;
  صحیح iVar;
  sqlite3_مقدار *pL, *pR = 0;
 
  اگر( p_عبارت->op==TK_VARIABLE && pVar->iColumn==p_عبارت->iColumn ){
    بازگشت 0;
  }
  اگر( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) بازگشت 2;
  sqlite3_مقدار_From_عبارت(pParse->db, p_عبارت, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  اگر( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_مقدار(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    اگر( pL ){
      اگر( sqlite3_مقدار_type(pL)==SQLITE_TEXT ){
        sqlite3_مقدار_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_مقدار_Free(pR);
    sqlite3_مقدار_Free(pL);
  }
  بازگشت res;
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
صحیح sqlite3_عبارت_Compare(
  ثابت Parse *pParse,
  ثابت عبارت *pA,
  ثابت عبارت *pB,
  صحیح iTab
){
  u32 combinedFlags;
  اگر( pA==0 || pB==0 ){
    بازگشت pB==pA ? 0 : 2;
  }
  اگر( pParse && pA->op==TK_VARIABLE ){
    بازگشت عبارت_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  اگر( combinedFlags & EP_Int_مقدار ){
    اگر( (pA->flags&pB->flags&EP_Int_مقدار)!=0 && pA->u.i_مقدار==pB->u.i_مقدار ){
      بازگشت 0;
    }
    بازگشت 2;
  }
  اگر( pA->op!=pB->op || pA->op==TK_RAISE ){
    اگر( pA->op==TK_COLLATE && sqlite3_عبارت_Compare(pParse, pA->p_چپ,pB,iTab)<2 ){
      بازگشت 1;
    }
    اگر( pB->op==TK_COLLATE && sqlite3_عبارت_Compare(pParse, pA,pB->p_چپ,iTab)<2 ){
      بازگشت 1;
    }
    اگر( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }وگرنه{
      بازگشت 2;
    }
  }
  أكد( !عبارت_Has_Property(pA, EP_Int_مقدار) );
  أكد( !عبارت_Has_Property(pB, EP_Int_مقدار) );
  اگر( pA->u.zToken ){
    اگر( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      اگر( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) بازگشت 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      اگر( عبارت_Has_Property(pA,EP_WinFunc)!=عبارت_Has_Property(pB,EP_WinFunc) ){
        بازگشت 2;
      }
      اگر( عبارت_Has_Property(pA,EP_WinFunc) ){
        اگر( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          بازگشت 2;
        }
      }
#endif
    }وگرنه اگر( pA->op==TK_NULL ){
      بازگشت 0;
    }وگرنه اگر( pA->op==TK_COLLATE ){
      اگر( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) بازگشت 2;
    }وگرنه
    اگر( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      بازگشت 2;
    }
  }
  اگر( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) بازگشت 2;
  اگر( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    اگر( combinedFlags & EP_xIsSelect ) بازگشت 2;
    اگر( (combinedFlags & EP_FixedCol)==0
     && sqlite3_عبارت_Compare(pParse, pA->p_چپ, pB->p_چپ, iTab) ) بازگشت 2;
    اگر( sqlite3_عبارت_Compare(pParse, pA->p_راست, pB->p_راست, iTab) ) بازگشت 2;
    اگر( sqlite3_عبارت_List_Compare(pA->x.pList, pB->x.pList, iTab) ) بازگشت 2;
    اگر( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      اگر( pA->iColumn!=pB->iColumn ) بازگشت 2;
      اگر( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) بازگشت 2;
      اگر( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        بازگشت 2;
      }
    }
  }
  بازگشت 0;
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
صحیح sqlite3_عبارت_List_Compare(ثابت عبارت_List *pA, ثابت عبارت_List *pB, صحیح iTab){
  صحیح i;
  اگر( pA==0 && pB==0 ) بازگشت 0;
  اگر( pA==0 || pB==0 ) بازگشت 1;
  اگر( pA->n_عبارت!=pB->n_عبارت ) بازگشت 1;
  برای(i=0; i<pA->n_عبارت; i++){
    صحیح res;
    عبارت *p_عبارت_A = pA->a[i].p_عبارت;
    عبارت *p_عبارت_B = pB->a[i].p_عبارت;
    اگر( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) بازگشت 1;
    اگر( (res = sqlite3_عبارت_Compare(0, p_عبارت_A, p_عبارت_B, iTab)) ) بازگشت res;
  }
  بازگشت 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحیح sqlite3_عبارت_Compare_Skip(عبارت *pA,عبارت *pB, صحیح iTab){
  بازگشت sqlite3_عبارت_Compare(0,
             sqlite3_عبارت_Skip_Collate(pA),
             sqlite3_عبارت_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ایستا صحیح عبارت_Implies_Not_Null(
  ثابت Parse *pParse,/* Parsing context */
  ثابت عبارت *p,      /* The expression to be checked */
  ثابت عبارت *pNN,    /* The expression that is NOT NULL */
  صحیح iTab,           /* Table being evaluated */
  صحیح seenNot         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  اگر( sqlite3_عبارت_Compare(pParse, p, pNN, iTab)==0 ){
    بازگشت pNN->op!=TK_NULL;
  }
  انتخاب( p->op ){
    حالت TK_IN: {
      اگر( seenNot && عبارت_Has_Property(p, EP_xIsSelect) ) بازگشت 0;
      أكد( عبارت_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_عبارت>0) );
      بازگشت عبارت_Implies_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
    حالت TK_BETWEEN: {
      عبارت_List *pList;
      أكد( عبارت_Use_XList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->n_عبارت==2 );
      اگر( seenNot ) بازگشت 0;
      اگر( عبارت_Implies_Not_Null(pParse, pList->a[0].p_عبارت, pNN, iTab, 1)
       || عبارت_Implies_Not_Null(pParse, pList->a[1].p_عبارت, pNN, iTab, 1)
      ){
        بازگشت 1;
      }
      بازگشت عبارت_Implies_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
    حالت TK_EQ:
    حالت TK_NE:
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE:
    حالت TK_PLUS:
    حالت TK_MINUS:
    حالت TK_BITOR:
    حالت TK_LSHIFT:
    حالت TK_RSHIFT:
    حالت TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    حالت TK_STAR:
    حالت TK_REM:
    حالت TK_BITAND:
    حالت TK_SLASH: {
      اگر( عبارت_Implies_Not_Null(pParse, p->p_راست, pNN, iTab, seenNot) ) بازگشت 1;
      /* no break */ deliberate_fall_through
    }
    حالت TK_SPAN:
    حالت TK_COLLATE:
    حالت TK_UPLUS:
    حالت TK_UMINUS: {
      بازگشت عبارت_Implies_Not_Null(pParse, p->p_چپ, pNN, iTab, seenNot);
    }
    حالت TK_TRUTH: {
      اگر( seenNot ) بازگشت 0;
      اگر( p->op2!=TK_IS ) بازگشت 0;
      بازگشت عبارت_Implies_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
    حالت TK_BITNOT:
    حالت TK_NOT: {
      بازگشت عبارت_Implies_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
  }
  بازگشت 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ایستا صحیح sqlite3_عبارت_Is_Not_True(عبارت *p_عبارت){
  صحیح v;
  اگر( p_عبارت->op==TK_NULL ) بازگشت 1;
  اگر( p_عبارت->op==TK_TRUEFALSE && sqlite3_عبارت_Truth_مقدار(p_عبارت)==0 ) بازگشت 1;
  v = 1;
  اگر( sqlite3_عبارت_Is_Integer(p_عبارت, &v, 0, 0) && v==0 ) بازگشت 1;
  بازگشت 0;
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
ایستا صحیح sqlite3_عبارت_Is_IIF(sqlite3 *db, ثابت عبارت *p_عبارت){
  عبارت_List *pList;
  اگر( p_عبارت->op==TK_FUNCTION ){
    ثابت نویسه *z = p_عبارت->u.zToken;
    FuncDef *pDef;
    اگر( (z[0]!='i' && z[0]!='I') ) بازگشت 0;
    اگر( p_عبارت->x.pList==0 ) بازگشت 0;
    pDef = sqlite3FindFunction(db, z, p_عبارت->x.pList->n_عبارت, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    اگر( pDef==0 ) بازگشت 0;
#وگرنه
    اگر( NEVER(pDef==0) ) بازگشت 0;
#endif
    اگر( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) بازگشت 0;
    اگر( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) بازگشت 0;
  }وگرنه اگر( p_عبارت->op==TK_CASE ){
    اگر( p_عبارت->p_چپ!=0 ) بازگشت 0;
  }وگرنه{
    بازگشت 0;
  }
  pList = p_عبارت->x.pList;
  أكد( pList!=0 );
  اگر( pList->n_عبارت==2 ) بازگشت 1;
  اگر( pList->n_عبارت==3 && sqlite3_عبارت_Is_Not_True(pList->a[2].p_عبارت) ) بازگشت 1;
  بازگشت 0;
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
صحیح sqlite3_عبارت_Implies_عبارت(
  ثابت Parse *pParse,
  ثابت عبارت *pE1,
  ثابت عبارت *pE2,
  صحیح iTab
){
  اگر( sqlite3_عبارت_Compare(pParse, pE1, pE2, iTab)==0 ){
    بازگشت 1;
  }
  اگر( pE2->op==TK_OR
   && (sqlite3_عبارت_Implies_عبارت(pParse, pE1, pE2->p_چپ, iTab)
             || sqlite3_عبارت_Implies_عبارت(pParse, pE1, pE2->p_راست, iTab) )
  ){
    بازگشت 1;
  }
  اگر( pE2->op==TK_NOTNULL
   && عبارت_Implies_Not_Null(pParse, pE1, pE2->p_چپ, iTab, 0)
  ){
    بازگشت 1;
  }
  اگر( sqlite3_عبارت_Is_IIF(pParse->db, pE1) ){
    بازگشت sqlite3_عبارت_Implies_عبارت(pParse,pE1->x.pList->a[0].p_عبارت,pE2,iTab);
  }
  بازگشت 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ایستا تهی bothImplyNotNullRow(Walker *pWalker, عبارت *pE1, عبارت *pE2){
  اگر( pWalker->eCode==0 ){
    sqlite3_Walk_عبارت(pWalker, pE1);
    اگر( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_عبارت(pWalker, pE2);
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
ایستا صحیح impliesNotNullRow(Walker *pWalker, عبارت *p_عبارت){
  testcase( p_عبارت->op==TK_AGG_COLUMN );
  testcase( p_عبارت->op==TK_AGG_FUNCTION );
  اگر( عبارت_Has_Property(p_عبارت, EP_OuterON) ) بازگشت WRC_Prune;
  اگر( عبارت_Has_Property(p_عبارت, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    بازگشت WRC_Prune;
  }
  انتخاب( p_عبارت->op ){
    حالت TK_ISNOT:
    حالت TK_ISNULL:
    حالت TK_NOTNULL:
    حالت TK_IS:
    حالت TK_VECTOR:
    حالت TK_FUNCTION:
    حالت TK_TRUTH:
    حالت TK_CASE:
      testcase( p_عبارت->op==TK_ISNOT );
      testcase( p_عبارت->op==TK_ISNULL );
      testcase( p_عبارت->op==TK_NOTNULL );
      testcase( p_عبارت->op==TK_IS );
      testcase( p_عبارت->op==TK_VECTOR );
      testcase( p_عبارت->op==TK_FUNCTION );
      testcase( p_عبارت->op==TK_TRUTH );
      testcase( p_عبارت->op==TK_CASE );
      بازگشت WRC_Prune;

    حالت TK_COLUMN:
      اگر( pWalker->u.iCur==p_عبارت->iTable ){
        pWalker->eCode = 1;
        بازگشت WRC_Abort;
      }
      بازگشت WRC_Prune;

    حالت TK_OR:
    حالت TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_عبارت->op==TK_OR );
      testcase( p_عبارت->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_عبارت->p_چپ, p_عبارت->p_راست);
      بازگشت WRC_Prune;
       
    حالت TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      اگر( عبارت_Use_XList(p_عبارت) && ALWAYS(p_عبارت->x.pList->n_عبارت>0) ){
        sqlite3_Walk_عبارت(pWalker, p_عبارت->p_چپ);
      }
      بازگشت WRC_Prune;

    حالت TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( عبارت_Use_XList(p_عبارت) );
      أكد( p_عبارت->x.pList->n_عبارت==2 );
      sqlite3_Walk_عبارت(pWalker, p_عبارت->p_چپ);
      bothImplyNotNullRow(pWalker, p_عبارت->x.pList->a[0].p_عبارت,
                                   p_عبارت->x.pList->a[1].p_عبارت);
      بازگشت WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    حالت TK_EQ:
    حالت TK_NE:
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE: {
      عبارت *p_چپ = p_عبارت->p_چپ;
      عبارت *p_راست = p_عبارت->p_راست;
      testcase( p_عبارت->op==TK_EQ );
      testcase( p_عبارت->op==TK_NE );
      testcase( p_عبارت->op==TK_LT );
      testcase( p_عبارت->op==TK_LE );
      testcase( p_عبارت->op==TK_GT );
      testcase( p_عبارت->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      أكد( p_چپ->op!=TK_COLUMN || عبارت_Use_YTab(p_چپ) );
      أكد( p_راست->op!=TK_COLUMN || عبارت_Use_YTab(p_راست) );
      اگر( (p_چپ->op==TK_COLUMN
           && ALWAYS(p_چپ->y.pTab!=0)
           && IsVirtual(p_چپ->y.pTab))
       || (p_راست->op==TK_COLUMN
           && ALWAYS(p_راست->y.pTab!=0)
           && IsVirtual(p_راست->y.pTab))
      ){
        بازگشت WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    پیشفرض:
      بازگشت WRC_Continue;
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
صحیح sqlite3_عبارت_Implies_Non_Null_Row(عبارت *p, صحیح iTab, صحیح isRJ){
  Walker w;
  p = sqlite3_عبارت_Skip_Collate_And_Likely(p);
  اگر( p==0 ) بازگشت 0;
  اگر( p->op==TK_NOTNULL ){
    p = p->p_چپ;
  }وگرنه{
    وقتی( p->op==TK_AND ){
      اگر( sqlite3_عبارت_Implies_Non_Null_Row(p->p_چپ, iTab, isRJ) ) بازگشت 1;
      p = p->p_راست;
    }
  }
  w.x_عبارت_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_عبارت(&w, p);
  بازگشت w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ساختار IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  صحیح iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ایستا صحیح عبارت_Idx_Cover(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_COLUMN
   && p_عبارت->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_عبارت->iColumn)<0
  ){
    pWalker->eCode = 1;
    بازگشت WRC_Abort;
  }
  بازگشت WRC_Continue;
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
صحیح sqlite3_عبارت_Covered_By_Index(
  عبارت *p_عبارت,        /* The index to be tested */
  صحیح iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ساختار IdxCover xcov;
  memset(&w, 0, اندازه(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_عبارت_Callback = عبارت_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_عبارت(&w, p_عبارت);
  بازگشت !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ساختار RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  صحیح *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ایستا صحیح selectRefEnter(Walker *pWalker, Select *pSelect){
  ساختار RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  صحیح *piNew;
  اگر( pSrc->nSrc==0 ) بازگشت WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*اندازه(صحیح));
  اگر( piNew==0 ){
    p->nExclude = 0;
    بازگشت WRC_Abort;
  }وگرنه{
    p->aiExclude = piNew;
  }
  برای(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  بازگشت WRC_Continue;
}
ایستا تهی selectRefLeave(Walker *pWalker, Select *pSelect){
  ساختار RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  اگر( p->nExclude ){
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
ایستا صحیح عبارت_Ref_To_Src_List(Walker *pWalker, عبارت *p_عبارت){
  اگر( p_عبارت->op==TK_COLUMN
   || p_عبارت->op==TK_AGG_COLUMN
  ){
    صحیح i;
    ساختار RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحیح nSrc = pSrc ? pSrc->nSrc : 0;
    برای(i=0; i<nSrc; i++){
      اگر( p_عبارت->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        بازگشت WRC_Continue;
      }
    }
    برای(i=0; i<p->nExclude && p->aiExclude[i]!=p_عبارت->iTable; i++){}
    اگر( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  بازگشت WRC_Continue;
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
صحیح sqlite3ReferencesSrcList(Parse *pParse, عبارت *p_عبارت, SrcList *pSrcList){
  Walker w;
  ساختار RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, اندازه(w));
  memset(&x, 0, اندازه(x));
  w.x_عبارت_Callback = عبارت_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( p_عبارت->op==TK_AGG_FUNCTION );
  أكد( عبارت_Use_XList(p_عبارت) );
  sqlite3_Walk_عبارت_List(&w, p_عبارت->x.pList);
  اگر( p_عبارت->p_چپ ){
    أكد( p_عبارت->p_چپ->op==TK_ORDER );
    أكد( عبارت_Use_XList(p_عبارت->p_چپ) );
    أكد( p_عبارت->p_چپ->x.pList!=0 );
    sqlite3_Walk_عبارت_List(&w, p_عبارت->p_چپ->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  اگر( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
    sqlite3_Walk_عبارت(&w, p_عبارت->y.pWin->pFilter);
  }
#endif
  اگر( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  اگر( w.eCode & 0x01 ){
    بازگشت 1;
  }وگرنه اگر( w.eCode ){
    بازگشت 0;
  }وگرنه{
    بازگشت -1;
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
ایستا صحیح agginfo_Persist_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  اگر( ALWAYS(!عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced))
   && p_عبارت->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_عبارت->pAggInfo;
    صحیح iAgg = p_عبارت->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    اگر( p_عبارت->op!=TK_AGG_FUNCTION ){
      اگر( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_عبارت
      ){
        p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        اگر( p_عبارت && !sqlite3_عبارت_Deferred_Delete(pParse, p_عبارت) ){
          pAggInfo->aCol[iAgg].pCExpr = p_عبارت;
        }
      }
    }وگرنه{
      أكد( p_عبارت->op==TK_AGG_FUNCTION );
      اگر( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_عبارت
      ){
        p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        اگر( p_عبارت && !sqlite3_عبارت_Deferred_Delete(pParse, p_عبارت) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_عبارت;
        }
      }
    }
  }
  بازگشت WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
تهی sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, اندازه(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_عبارت_Callback = agginfo_Persist_عبارت_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ایستا صحیح addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  صحیح i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       اندازه(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  بازگشت i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ایستا صحیح addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  صحیح i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       اندازه(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  بازگشت i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ایستا تهی findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  عبارت *p_عبارت          /* Expr describing the column to find or insert */
){
  ساختار AggInfo_col *pCol;
  صحیح k;
  صحیح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  أكد( mxTerm <= SMXV(i16) );
  أكد( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  برای(k=0; k<pAggInfo->nColumn; k++, pCol++){
    اگر( pCol->pCExpr==p_عبارت ) بازگشت;
    اگر( pCol->iTable==p_عبارت->iTable
     && pCol->iColumn==p_عبارت->iColumn
     && p_عبارت->op!=TK_IF_NULL_ROW
    ){
      برو fix_up_عبارت;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  اگر( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    بازگشت;
  }
  اگر( k>mxTerm ){
    sqlite3_Error_پیام(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  أكد( عبارت_Use_YTab(p_عبارت) );
  pCol->pTab = p_عبارت->y.pTab;
  pCol->iTable = p_عبارت->iTable;
  pCol->iColumn = p_عبارت->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_عبارت;
  اگر( pAggInfo->pGroupBy && p_عبارت->op!=TK_IF_NULL_ROW ){
    صحیح j, n;
    عبارت_List *pGB = pAggInfo->pGroupBy;
    ساختار عبارت_List_item *pTerm = pGB->a;
    n = pGB->n_عبارت;
    برای(j=0; j<n; j++, pTerm++){
      عبارت *pE = pTerm->p_عبارت;
      اگر( pE->op==TK_COLUMN
       && pE->iTable==p_عبارت->iTable
       && pE->iColumn==p_عبارت->iColumn
      ){
        pCol->iSorterColumn = j;
        بشکن;
      }
    }
  }
  اگر( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_عبارت:
  عبارت_Set_VVAProperty(p_عبارت, EP_NoReduce);
  أكد( p_عبارت->pAggInfo==0 || p_عبارت->pAggInfo==pAggInfo );
  p_عبارت->pAggInfo = pAggInfo;
  اگر( p_عبارت->op==TK_COLUMN ){
    p_عبارت->op = TK_AGG_COLUMN;
  }
  أكد( k <= SMXV(p_عبارت->iAgg) );
  p_عبارت->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ایستا صحیح analyzeAggregate(Walker *pWalker, عبارت *p_عبارت){
  صحیح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->iFirstReg==0 );
  انتخاب( p_عبارت->op ){
    پیشفرض: {
      Indexed_عبارت *pIEpr;
      عبارت tmp;
      أكد( pParse->iSelfTab==0 );
      اگر( (pNC->ncFlags & NC_InAggFunc)==0 ) بشکن;
      اگر( pParse->pIdxEpr==0 ) بشکن;
      برای(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحیح iDataCur = pIEpr->iDataCur;
        اگر( iDataCur<0 ) ادامه;
        اگر( sqlite3_عبارت_Compare(0, p_عبارت, pIEpr->p_عبارت, iDataCur)==0 ) بشکن;
      }
      اگر( pIEpr==0 ) بشکن;
      اگر( NEVER(!عبارت_Use_YTab(p_عبارت)) ) بشکن;
      برای(i=0; i<pSrcList->nSrc; i++){
         اگر( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           بشکن;
         }
      }
      اگر( i>=pSrcList->nSrc ) بشکن;
      اگر( NEVER(p_عبارت->pAggInfo!=0) ) بشکن; /* Resolved by outer context */
      اگر( pParse->nErr ){ بازگشت WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, اندازه(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      اگر( pParse->nErr ){ بازگشت WRC_Abort; }
      أكد( pAggInfo->aCol!=0 );
      أكد( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_عبارت;
      p_عبارت->pAggInfo = pAggInfo;
      p_عبارت->iAgg = tmp.iAgg;
      بازگشت WRC_Prune;
    }
    حالت TK_IF_NULL_ROW:
    حالت TK_AGG_COLUMN:
    حالت TK_COLUMN: {
      testcase( p_عبارت->op==TK_AGG_COLUMN );
      testcase( p_عبارت->op==TK_COLUMN );
      testcase( p_عبارت->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      اگر( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        برای(i=0; i<pSrcList->nSrc; i++, pItem++){
          أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
          اگر( p_عبارت->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_عبارت);
            بشکن;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      بازگشت WRC_Continue;
    }
    حالت TK_AGG_FUNCTION: {
      اگر( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_عبارت->op2
       && p_عبارت->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ساختار AggInfo_func *pItem = pAggInfo->aFunc;
        صحیح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        أكد( mxTerm <= SMXV(i16) );
        برای(i=0; i<pAggInfo->nFunc; i++, pItem++){
          اگر( NEVER(pItem->pFExpr==p_عبارت) ) بشکن;
          اگر( sqlite3_عبارت_Compare(0, pItem->pFExpr, p_عبارت, -1)==0 ){
            بشکن;
          }
        }
        اگر( i>mxTerm ){
          sqlite3_Error_پیام(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          أكد( i<pAggInfo->nFunc );
        }وگرنه اگر( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          اگر( i>=0 ){
            صحیح nArg;
            أكد( !عبارت_Has_Property(p_عبارت, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_عبارت;
            أكد( عبارت_Use_UToken(p_عبارت) );
            nArg = p_عبارت->x.pList ? p_عبارت->x.pList->n_عبارت : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_عبارت->u.zToken, nArg, enc, 0);
            أكد( pItem->bOBUnique==0 );
            اگر( p_عبارت->p_چپ
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              عبارت_List *pOBList;
              أكد( nArg>0 );
              أكد( p_عبارت->p_چپ->op==TK_ORDER );
              أكد( عبارت_Use_XList(p_عبارت->p_چپ) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_عبارت->p_چپ->x.pList;
              أكد( pOBList->n_عبارت>0 );
              أكد( pItem->bOBUnique==0 );
              اگر( pOBList->n_عبارت==1
               && nArg==1
               && sqlite3_عبارت_Compare(0,pOBList->a[0].p_عبارت,
                               p_عبارت->x.pList->a[0].p_عبارت,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = عبارت_Has_Property(p_عبارت, EP_Distinct);
              }وگرنه{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }وگرنه{
              pItem->iOBTab = -1;
            }
            اگر( عبارت_Has_Property(p_عبارت, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }وگرنه{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
        عبارت_Set_VVAProperty(p_عبارت, EP_NoReduce);
        أكد( i <= SMXV(p_عبارت->iAgg) );
        p_عبارت->iAgg = (i16)i;
        p_عبارت->pAggInfo = pAggInfo;
        بازگشت WRC_Prune;
      }وگرنه{
        بازگشت WRC_Continue;
      }
    }
  }
  بازگشت WRC_Continue;
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
تهی sqlite3_عبارت_Analyze_Aggregates(NameContext *pNC, عبارت *p_عبارت){
  Walker w;
  w.x_عبارت_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  أكد( pNC->pSrcList!=0 );
  sqlite3_Walk_عبارت(&w, p_عبارت);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
تهی sqlite3_عبارت_Analyze_Agg_List(NameContext *pNC, عبارت_List *pList){
  ساختار عبارت_List_item *pItem;
  صحیح i;
  اگر( pList ){
    برای(pItem=pList->a, i=0; i<pList->n_عبارت; i++, pItem++){
      sqlite3_عبارت_Analyze_Aggregates(pNC, pItem->p_عبارت);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحیح sqlite3GetTempReg(Parse *pParse){
  اگر( pParse->nTempReg==0 ){
    بازگشت ++pParse->nMem;
  }
  بازگشت pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
تهی sqlite3ReleaseTempReg(Parse *pParse, صحیح iReg){
  اگر( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحیح kk;
    برای(kk=0; kk<pParse->nTempReg; kk++){
      أكد( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    اگر( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحیح sqlite3GetTempRange(Parse *pParse, صحیح nReg){
  صحیح i, n;
  اگر( nReg==1 ) بازگشت sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  اگر( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }وگرنه{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  بازگشت i;
}
تهی sqlite3ReleaseTempRange(Parse *pParse, صحیح iReg, صحیح nReg){
  اگر( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    بازگشت;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  اگر( nReg>pParse->nRangeReg ){
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
تهی sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
تهی sqlite3TouchRegister(Parse *pParse, صحیح iReg){
  اگر( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#اگر defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحیح sqlite3FirstAvailableRegister(Parse *pParse, صحیح iMin){
  ثابت عبارت_List *pList = pParse->p_Const_عبارت;
  اگر( pList ){
    صحیح i;
    برای(i=0; i<pList->n_عبارت; i++){
      اگر( pList->a[i].u.i_Const_عبارت_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_عبارت_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  بازگشت iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
صحیح sqlite3NoTempsInRange(Parse *pParse, صحیح iFirst, صحیح iLast){
  صحیح i;
  اگر( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     بازگشت 0;
  }
  برای(i=0; i<pParse->nTempReg; i++){
    اگر( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      بازگشت 0;
    }
  }
  اگر( pParse->p_Const_عبارت ){
    عبارت_List *pList = pParse->p_Const_عبارت;
    برای(i=0; i<pList->n_عبارت; i++){
      صحیح iReg = pList->a[i].u.i_Const_عبارت_Reg;
      اگر( iReg==0 ) ادامه;
      اگر( iReg>=iFirst && iReg<=iLast ) بازگشت 0;
    }
  }
  بازگشت 1;
}
#endif /* SQLITE_DEBUG */
