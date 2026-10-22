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
ساكن فراغ عبارت_کد_Between(Parse*,عبارت*,صحيح,فراغ(*)(Parse*,عبارت*,صحيح,صحيح),صحيح);
ساكن صحيح عبارت_کد_وکتور(Parse *pParse, عبارت *p, صحيح *pi_مقصد_Free);

/*
** Return the affinity character for a single column of a table.
*/
حرف sqlite3_جدول_ستونی_Affinity(ثابت جدول *pTab, صحيح i_ستون){
  إذا( i_ستون<0 || NEVER(i_ستون>=pTab->n_ستون) ) أرجع SQLITE_AFF_INTEGER;
  أرجع pTab->a_ستون[i_ستون].affinity;
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
حرف sqlite3_عبارت_Affinity(ثابت عبارت *p_عبارت){
  صحيح op;
  op = p_عبارت->op;
  طالما( 1 /* exit-by-break */ ){
    إذا( op==TK_ستونی || (op==TK_AGG_ستونی && p_عبارت->y.pTab!=0) ){
      أكد( عبارت_Use_YTab(p_عبارت) );
      أكد( p_عبارت->y.pTab!=0 );
      أرجع sqlite3_جدول_ستونی_Affinity(p_عبارت->y.pTab, p_عبارت->i_ستونی);
    }
    إذا( op==TK_SELECT ){
      أكد( عبارت_Use_XSelect(p_عبارت) );
      أكد( p_عبارت->x.pSelect!=0 );
      أكد( p_عبارت->x.pSelect->pEList!=0 );
      أكد( p_عبارت->x.pSelect->pEList->a[0].p_عبارت!=0 );
      أرجع sqlite3_عبارت_Affinity(p_عبارت->x.pSelect->pEList->a[0].p_عبارت);
    }
#اگر_تعریف_نشده SQLITE_OMIT_CAST
    إذا( op==TK_CAST ){
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      أرجع sqlite3AffinityType(p_عبارت->u.zToken, 0);
    }
#پایان_اگر
    إذا( op==TK_SELECT_ستونی ){
      أكد( p_عبارت->p_چپ!=0 && عبارت_Use_XSelect(p_عبارت->p_چپ) );
      أكد( p_عبارت->i_ستونی < p_عبارت->i_جدول );
      أكد( p_عبارت->i_ستونی >= 0 );
      أكد( p_عبارت->i_جدول==p_عبارت->p_چپ->x.pSelect->pEList->n_عبارت );
      أرجع sqlite3_عبارت_Affinity(
          p_عبارت->p_چپ->x.pSelect->pEList->a[p_عبارت->i_ستونی].p_عبارت
      );
    }
    إذا( op==TK_وکتور
     || (op==TK_FUNCTION && p_عبارت->aff_عبارت==SQLITE_AFF_DEFER)
    ){
      أكد( عبارت_Use_XList(p_عبارت) );
      أرجع sqlite3_عبارت_Affinity(p_عبارت->x.pList->a[0].p_عبارت);
    }
    إذا( عبارت_Has_Property(p_عبارت, EP_Skip|EP_If_Null_ردیف) ){
      أكد( p_عبارت->op==TK_COLLATE
           || p_عبارت->op==TK_IF_NULL_ردیف
           || (p_عبارت->op==TK_ثبات && p_عبارت->op2==TK_IF_NULL_ردیف) );
      p_عبارت = p_عبارت->p_چپ;
      op = p_عبارت->op;
      استمر;
    }
    إذا( op!=TK_ثبات ) اكسر;
    op = p_عبارت->op2;
    إذا( NEVER( op==TK_ثبات ) ) اكسر;
  }
  أرجع p_عبارت->aff_عبارت;
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
صحيح sqlite3_عبارت_داده_Type(ثابت عبارت *p_عبارت){
  طالما( p_عبارت ){
    بدل( p_عبارت->op ){
      حالة TK_COLLATE:
      حالة TK_IF_NULL_ردیف:
      حالة TK_UPLUS:  {
        p_عبارت = p_عبارت->p_چپ;
        اكسر;
      }
      حالة TK_NULL: {
        p_عبارت = 0;
        اكسر;
      }
      حالة TK_STRING: {
        أرجع 0x02;
      }
      حالة TK_BLOB: {
        أرجع 0x04;
      }
      حالة TK_CONCAT: {
        أرجع 0x06;
      }
      حالة TK_VARIABLE:
      حالة TK_AGG_FUNCTION:
      حالة TK_FUNCTION: {
        أرجع 0x07;
      }
      حالة TK_ستونی:
      حالة TK_AGG_ستونی:
      حالة TK_SELECT:
      حالة TK_CAST:
      حالة TK_SELECT_ستونی:
      حالة TK_وکتور:  {
        صحيح aff = sqlite3_عبارت_Affinity(p_عبارت);
        إذا( aff>=SQLITE_AFF_NUMERIC ) أرجع 0x05;
        إذا( aff==SQLITE_AFF_متن )    أرجع 0x06;
        أرجع 0x07;
      }
      حالة TK_CASE: {
        صحيح res = 0;
        صحيح ii;
        عبارت_List *pList = p_عبارت->x.pList;
        أكد( عبارت_Use_XList(p_عبارت) && pList!=0 );
        أكد( pList->n_عبارت > 0);
        لكل(ii=1; ii<pList->n_عبارت; ii+=2){
          res |= sqlite3_عبارت_داده_Type(pList->a[ii].p_عبارت);
        }
        إذا( pList->n_عبارت % 2 ){
          res |= sqlite3_عبارت_داده_Type(pList->a[pList->n_عبارت-1].p_عبارت);
        }
        أرجع res;
      }
      افتراضي: {
        أرجع 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  أرجع 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
عبارت *sqlite3_عبارت_جمع_کن_Collate_Token(
  Parse *pParse,           /* Parsing context */
  عبارت *p_عبارت,             /* Add the "COLLATE" clause to this expression */
  ثابت Token *pCollName,  /* Name of collating sequence */
  صحيح dequote              /* True to dequote pCollName */
){
  إذا( pCollName->n>0 && pParse->nErr==0 ){
    عبارت *pNew = sqlite3_عبارت_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( p_عبارت!=0 );
    إذا( pNew ){
      pNew->p_چپ = p_عبارت;
      pNew->flags |= EP_Collate|EP_Skip;
#إذا SQLITE_MAX_عبارت_عمق>0
      pNew->n_ارتفاع = p_عبارت->n_ارتفاع+1;
      sqlite3_عبارت_Check_ارتفاع(pParse, pNew->n_ارتفاع);
      testcase( pParse->nErr>0 );
#پایان_اگر
      p_عبارت = pNew;
    }
  }
  أرجع p_عبارت;
}
عبارت *sqlite3_عبارت_جمع_کن_Collate_String(
  Parse *pParse,        /* Parsing context */
  عبارت *p_عبارت,          /* Add the "COLLATE" clause to this expression */
  ثابت حرف *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (حرف*)zC);
  أرجع sqlite3_عبارت_جمع_کن_Collate_Token(pParse, p_عبارت, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
عبارت *sqlite3_عبارت_Skip_Collate(عبارت *p_عبارت){
  طالما( p_عبارت && عبارت_Has_Property(p_عبارت, EP_Skip) ){
    أكد( p_عبارت->op==TK_COLLATE );
    p_عبارت = p_عبارت->p_چپ;
  }  
  أرجع p_عبارت;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
عبارت *sqlite3_عبارت_Skip_Collate_And_Likely(عبارت *p_عبارت){
  طالما( p_عبارت && عبارت_Has_Property(p_عبارت, EP_Skip|EP_Unlikely) ){
    إذا( عبارت_Has_Property(p_عبارت, EP_Unlikely) ){
      أكد( عبارت_Use_XList(p_عبارت) );
      أكد( p_عبارت->x.pList->n_عبارت>0 );
      أكد( p_عبارت->op==TK_FUNCTION );
      p_عبارت = p_عبارت->x.pList->a[0].p_عبارت;
    }وإلا إذا( p_عبارت->op==TK_COLLATE ){
      p_عبارت = p_عبارت->p_چپ;
    }وإلا{
      اكسر;
    }
  }  
  أرجع p_عبارت;
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
  طالما( p ){
    صحيح op = p->op;
    إذا( op==TK_ثبات ) op = p->op2;
    إذا( (op==TK_AGG_ستونی && p->y.pTab!=0)
     || op==TK_ستونی || op==TK_TRIGGER
    ){
      صحيح j;
      أكد( عبارت_Use_YTab(p) );
      أكد( p->y.pTab!=0 );
      إذا( (j = p->i_ستونی)>=0 ){
        ثابت حرف *zColl = sqlite3_ستونی_Coll(&p->y.pTab->a_ستون[j]);
        pColl = sqlite3_پیدا_کن_Coll_Seq(db, ENC(db), zColl, 0);
      }
      اكسر;
    }
    إذا( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_چپ;
      استمر;
    }
    إذا( op==TK_وکتور
     || (op==TK_FUNCTION && p->aff_عبارت==SQLITE_AFF_DEFER)
    ){
      أكد( عبارت_Use_XList(p) );
      p = p->x.pList->a[0].p_عبارت;
      استمر;
    }
    إذا( op==TK_COLLATE ){
      أكد( !عبارت_Has_Property(p, EP_Int_مقدار) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      اكسر;
    }
    إذا( p->flags & EP_Collate ){
      إذا( p->p_چپ && (p->p_چپ->flags & EP_Collate)!=0 ){
        p = p->p_چپ;
      }وإلا{
        عبارت *p_بعدی  = p->p_راست;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !عبارت_Use_XList(p) || p->x.pList==0 || p->p_راست==0 );
        إذا( عبارت_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحيح i;
          لكل(i=0; i<p->x.pList->n_عبارت; i++){
            إذا( عبارت_Has_Property(p->x.pList->a[i].p_عبارت, EP_Collate) ){
              p_بعدی = p->x.pList->a[i].p_عبارت;
              اكسر;
            }
          }
        }
        p = p_بعدی;
      }
    }وإلا{
      اكسر;
    }
  }
  إذا( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  أرجع pColl;
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
  إذا( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  أرجع p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحيح sqlite3_عبارت_Coll_Seq_تطابق(Parse *pParse, ثابت عبارت *pE1, ثابت عبارت *pE2){
  CollSeq *pColl1 = sqlite3_عبارت_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_عبارت_NNColl_Seq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  أرجع pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
حرف sqlite3CompareAffinity(ثابت عبارت *p_عبارت, حرف aff2){
  حرف aff1 = sqlite3_عبارت_Affinity(p_عبارت);
  إذا( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    إذا( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      أرجع SQLITE_AFF_NUMERIC;
    }وإلا{
      أرجع SQLITE_AFF_BLOB;
    }
  }وإلا{
    /* One side is a column, the other is not. Use the columns affinity. */
    أكد( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    أرجع (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ساكن حرف comparisonAffinity(ثابت عبارت *p_عبارت){
  حرف aff;
  أكد( p_عبارت->op==TK_EQ || p_عبارت->op==TK_درون || p_عبارت->op==TK_LT ||
          p_عبارت->op==TK_GT || p_عبارت->op==TK_GE || p_عبارت->op==TK_LE ||
          p_عبارت->op==TK_NE || p_عبارت->op==TK_IS || p_عبارت->op==TK_ISNOT );
  أكد( p_عبارت->p_چپ );
  أكد( !عبارت_Use_XSelect(p_عبارت) );
  aff = sqlite3_عبارت_Affinity(p_عبارت->p_چپ);
  إذا( p_عبارت->p_راست ){
    aff = sqlite3CompareAffinity(p_عبارت->p_راست, aff);
  }وإلا إذا( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  أرجع aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
صحيح sqlite3_شاخص_Affinity_Ok(ثابت عبارت *p_عبارت, حرف idx_affinity){
  حرف aff = comparisonAffinity(p_عبارت);
  إذا( aff<SQLITE_AFF_متن ){
    أرجع 1;
  }
  إذا( aff==SQLITE_AFF_متن ){
    أرجع idx_affinity==SQLITE_AFF_متن;
  }
  أرجع sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ساكن u8 دودویی_Compare_P5(
  ثابت عبارت *pExpr1,   /* Left operand */
  ثابت عبارت *pExpr2,   /* Right operand */
  صحيح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (حرف)sqlite3_عبارت_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  أرجع aff;
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
  إذا( p_چپ->flags & EP_Collate ){
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_چپ);
  }وإلا إذا( p_راست && (p_راست->flags & EP_Collate)!=0 ){
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_راست);
  }وإلا{
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_چپ);
    إذا( !pColl ){
      pColl = sqlite3_عبارت_Coll_Seq(pParse, p_راست);
    }
  }
  أرجع pColl;
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
  إذا( عبارت_Has_Property(p, EP_Commuted) ){
    أرجع sqlite3_دودویی_Compare_Coll_Seq(pParse, p->p_راست, p->p_چپ);
  }وإلا{
    أرجع sqlite3_دودویی_Compare_Coll_Seq(pParse, p->p_چپ, p->p_راست);
  }
}

/*
** Generate code for a comparison operator.
*/
ساكن صحيح کد_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  عبارت *p_چپ,      /* The left operand */
  عبارت *p_راست,     /* The right operand */
  صحيح opcode,       /* The comparison opcode */
  صحيح in1, صحيح in2, /* Register holding operands */
  صحيح dest,         /* Jump here if true.  */
  صحيح jumpIfNull,   /* If true, jump if either operand is NULL */
  صحيح isCommuted    /* The comparison has been commuted */
){
  صحيح p5;
  صحيح addr;
  CollSeq *p4;

  إذا( pParse->nErr ) أرجع 0;
  إذا( isCommuted ){
    p4 = sqlite3_دودویی_Compare_Coll_Seq(pParse, p_راست, p_چپ);
  }وإلا{
    p4 = sqlite3_دودویی_Compare_Coll_Seq(pParse, p_چپ, p_راست);
  }
  p5 = دودویی_Compare_P5(p_چپ, p_راست, jumpIfNull);
  addr = sqlite3_Vdbe_جمع_کن_Op4(pParse->pVdbe, opcode, in2, dest, in1,
                           (فراغ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  أرجع addr;
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
صحيح sqlite3_عبارت_Is_وکتور(ثابت عبارت *p_عبارت){
  أرجع sqlite3_عبارت_وکتور_اندازه(p_عبارت)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحيح sqlite3_عبارت_وکتور_اندازه(ثابت عبارت *p_عبارت){
  u8 op = p_عبارت->op;
  إذا( op==TK_ثبات ) op = p_عبارت->op2;
  إذا( op==TK_وکتور ){
    أكد( عبارت_Use_XList(p_عبارت) );
    أرجع p_عبارت->x.pList->n_عبارت;
  }وإلا إذا( op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(p_عبارت) );
    أرجع p_عبارت->x.pSelect->pEList->n_عبارت;
  }وإلا{
    أرجع 1;
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
عبارت *sqlite3_وکتور_فیلد_Subexpr(عبارت *p_وکتور, صحيح i){
  أكد( i<sqlite3_عبارت_وکتور_اندازه(p_وکتور) || p_وکتور->op==TK_ERROR );
  إذا( sqlite3_عبارت_Is_وکتور(p_وکتور) ){
    أكد( p_وکتور->op2==0 || p_وکتور->op==TK_ثبات );
    إذا( p_وکتور->op==TK_SELECT || p_وکتور->op2==TK_SELECT ){
      أكد( عبارت_Use_XSelect(p_وکتور) );
      أرجع p_وکتور->x.pSelect->pEList->a[i].p_عبارت;
    }وإلا{
      أكد( عبارت_Use_XList(p_وکتور) );
      أرجع p_وکتور->x.pList->a[i].p_عبارت;
    }
  }
  أرجع p_وکتور;
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
عبارت *sqlite3_عبارت_For_وکتور_فیلد(
  Parse *pParse,       /* Parsing context */
  عبارت *p_وکتور,       /* The vector.  List of expressions or a sub-SELECT */
  صحيح i_فیلد,          /* Which column of the vector to return */
  صحيح n_فیلد           /* Total number of columns in the vector */
){
  عبارت *pRet;
  إذا( p_وکتور->op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(p_وکتور) );
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_ستونی, 0, 0);
    إذا( pRet ){
      عبارت_مجموعه_Property(pRet, EP_پر_اندازه);
      pRet->i_جدول = n_فیلد;
      pRet->i_ستونی = i_فیلد;
      pRet->p_چپ = p_وکتور;
    }
  }وإلا{
    إذا( p_وکتور->op==TK_وکتور ){
      عبارت **pp_وکتور;
      أكد( عبارت_Use_XList(p_وکتور) );
      pp_وکتور = &p_وکتور->x.pList->a[i_فیلد].p_عبارت;
      p_وکتور = *pp_وکتور;
      إذا( درون_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_وکتور = 0;
        أرجع p_وکتور;
      }
    }
    pRet = sqlite3_عبارت_Dup(pParse->db, p_وکتور, 0);
  }
  أرجع pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ساكن صحيح عبارت_کد_Subselect(Parse *pParse, عبارت *p_عبارت){
  صحيح reg = 0;
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
  إذا( p_عبارت->op==TK_SELECT ){
    reg = sqlite3_کد_Subselect(pParse, p_عبارت);
  }
#پایان_اگر
  أرجع reg;
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
ساكن صحيح عبارت_وکتور_ثبات(
  Parse *pParse,                  /* Parse context */
  عبارت *p_وکتور,                  /* Vector to extract element from */
  صحيح i_فیلد,                     /* Field to extract from pVector */
  صحيح regSelect,                  /* First in array of registers */
  عبارت *pTmp,                     /* Temporary space */
  عبارت **pp_عبارت,                  /* OUT: Expression element */
  صحيح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_وکتور->op;
  أكد( op==TK_وکتور || op==TK_ثبات || op==TK_SELECT || op==TK_ERROR );
  إذا( op==TK_ثبات ){
    *pp_عبارت = sqlite3_وکتور_فیلد_Subexpr(p_وکتور, i_فیلد);
    أرجع p_وکتور->i_جدول+i_فیلد;
  }
  إذا( op==TK_SELECT ){
    أكد( عبارت_Use_XSelect(p_وکتور) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, حجم(عبارت));
    pTmp->op = TK_SELECT_ستونی;
    pTmp->p_چپ = p_وکتور;
    pTmp->i_ستونی = i_فیلد;
    pTmp->i_جدول = p_وکتور->x.pSelect->pEList->n_عبارت;
    *pp_عبارت = pTmp;
    أرجع regSelect+i_فیلد;
  }
  إذا( op==TK_وکتور ){
    أكد( عبارت_Use_XList(p_وکتور) );
    *pp_عبارت = p_وکتور->x.pList->a[i_فیلد].p_عبارت;
    أرجع sqlite3_عبارت_کد_موقت(pParse, *pp_عبارت, pRegFree);
  }
  أرجع 0;
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
ساكن فراغ کد_وکتور_Compare(
  Parse *pParse,        /* Code generator context */
  عبارت *p_عبارت,          /* The comparison operation */
  صحيح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  عبارت *p_چپ = p_عبارت->p_چپ;
  عبارت *p_راست = p_عبارت->p_راست;
  صحيح n_چپ = sqlite3_عبارت_وکتور_اندازه(p_چپ);
  صحيح i;
  صحيح reg_چپ = 0;
  صحيح reg_راست = 0;
  u8 opx = op;
  صحيح addrCmp = 0;
  صحيح addr_انجامشده = sqlite3VdbeMakeLabel(pParse);
  صحيح isCommuted = عبارت_Has_Property(p_عبارت,EP_Commuted);

  أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  إذا( pParse->nErr ) أرجع;
  إذا( n_چپ!=sqlite3_عبارت_وکتور_اندازه(p_راست) ){
    sqlite3_Error_پیام(pParse, "row value misused");
    أرجع;
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

  إذا( op==TK_LE ) opx = TK_LT;
  إذا( op==TK_GE ) opx = TK_GT;
  إذا( op==TK_NE ) opx = TK_EQ;

  reg_چپ = عبارت_کد_Subselect(pParse, p_چپ);
  reg_راست = عبارت_کد_Subselect(pParse, p_راست);

  sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, dest);
  لكل(i=0; 1 /*Loop exits by "break"*/; i++){
    صحيح regFree1 = 0, regFree2 = 0;
    عبارت *pL = 0, *pR = 0;
    عبارت tmp1, tmp2;
    صحيح r1, r2;
    أكد( i>=0 && i<n_چپ );
    إذا( addrCmp ) sqlite3_Vdbe_Jump_اینجا(v, addrCmp);
    r1 = عبارت_وکتور_ثبات(pParse, p_چپ, i, reg_چپ, &tmp1, &pL, &regFree1);
    r2 = عبارت_وکتور_ثبات(pParse, p_راست, i, reg_راست, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    کد_Compare(pParse, pL, pR, opx, r1, r2, addr_انجامشده, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_موقت_Reg(pParse, regFree1);
    sqlite3_Release_موقت_Reg(pParse, regFree2);
    إذا( (opx==TK_LT || opx==TK_GT) && i<n_چپ-1 ){
      addrCmp = sqlite3_Vdbe_جمع_کن_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    إذا( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, dest);
    }وإلا{
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    إذا( i==n_چپ-1 ){
      اكسر;
    }
    إذا( opx==TK_EQ ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_NotNull, dest, addr_انجامشده); VdbeCoverage(v);
    }وإلا{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, addr_انجامشده);
      إذا( i==n_چپ-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_اینجا(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_انجامشده);
  إذا( op==TK_NE ){
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Not, dest, dest);
  }
}

#إذا SQLITE_MAX_عبارت_عمق>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحيح sqlite3_عبارت_Check_ارتفاع(Parse *pParse, صحيح n_ارتفاع){
  صحيح rc = SQLITE_OK;
  صحيح mx_ارتفاع = pParse->db->a_حد[SQLITE_حد_عبارت_عمق];
  إذا( n_ارتفاع>mx_ارتفاع ){
    sqlite3_Error_پیام(pParse,
       "Expression tree is too large (maximum depth %d)", mx_ارتفاع
    );
    rc = SQLITE_ERROR;
  }
  أرجع rc;
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
ساكن فراغ ارتفاع_از_عبارت(ثابت عبارت *p, صحيح *pn_ارتفاع){
  إذا( p ){
    إذا( p->n_ارتفاع>*pn_ارتفاع ){
      *pn_ارتفاع = p->n_ارتفاع;
    }
  }
}
ساكن فراغ ارتفاع_از_عبارت_List(ثابت عبارت_List *p, صحيح *pn_ارتفاع){
  إذا( p ){
    صحيح i;
    لكل(i=0; i<p->n_عبارت; i++){
      ارتفاع_از_عبارت(p->a[i].p_عبارت, pn_ارتفاع);
    }
  }
}
ساكن فراغ ارتفاع_از_Select(ثابت Select *pSelect, صحيح *pn_ارتفاع){
  ثابت Select *p;
  لكل(p=pSelect; p; p=p->p_پیشین){
    ارتفاع_از_عبارت(p->pWhere, pn_ارتفاع);
    ارتفاع_از_عبارت(p->pHaving, pn_ارتفاع);
    ارتفاع_از_عبارت(p->p_حد, pn_ارتفاع);
    ارتفاع_از_عبارت_List(p->pEList, pn_ارتفاع);
    ارتفاع_از_عبارت_List(p->p_گروه_By, pn_ارتفاع);
    ارتفاع_از_عبارت_List(p->p_ترتیب_By, pn_ارتفاع);
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
ساكن فراغ عبارت_مجموعه_ارتفاع(عبارت *p){
  صحيح n_ارتفاع = p->p_چپ ? p->p_چپ->n_ارتفاع : 0;
  إذا( NEVER(p->p_راست) && p->p_راست->n_ارتفاع>n_ارتفاع ){
    n_ارتفاع = p->p_راست->n_ارتفاع;
  }
  إذا( عبارت_Use_XSelect(p) ){
    ارتفاع_از_Select(p->x.pSelect, &n_ارتفاع);
  }وإلا إذا( p->x.pList ){
    ارتفاع_از_عبارت_List(p->x.pList, &n_ارتفاع);
    p->flags |= EP_Propagate & sqlite3_عبارت_List_Flags(p->x.pList);
  }
  p->n_ارتفاع = n_ارتفاع + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3_عبارت_مجموعه_ارتفاع_And_Flags(Parse *pParse, عبارت *p){
  إذا( pParse->nErr ) أرجع;
  عبارت_مجموعه_ارتفاع(p);
  sqlite3_عبارت_Check_ارتفاع(pParse, p->n_ارتفاع);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحيح sqlite3_Select_عبارت_ارتفاع(ثابت Select *p){
  صحيح n_ارتفاع = 0;
  ارتفاع_از_Select(p, &n_ارتفاع);
  أرجع n_ارتفاع;
}
#وإلا /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3_عبارت_مجموعه_ارتفاع_And_Flags(Parse *pParse, عبارت *p){
  إذا( pParse->nErr ) أرجع;
  إذا( p && عبارت_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_عبارت_List_Flags(p->x.pList);
  }
}
#تعریف_کن عبارت_مجموعه_ارتفاع(y)
#پایان_اگر /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
فراغ sqlite3_عبارت_مجموعه_Error_جابه_جایی(عبارت *p_عبارت, صحيح iOfst){
  إذا( p_عبارت==0 ) أرجع;
  إذا( NEVER(عبارت_Use_WJoin(p_عبارت)) ) أرجع;
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
  صحيح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحيح dequote             /* True to dequote */
){
  عبارت *pNew;
  صحيح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, حجم(عبارت)+nExtra);
  إذا( pNew ){
    memset(pNew, 0, حجم(عبارت));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    إذا( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (حرف*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      إذا( pToken->n ) کپی_حافظه(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      إذا( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_عبارت(pNew);
      }
    }
#إذا SQLITE_MAX_عبارت_عمق>0
    pNew->n_ارتفاع = 1;
#پایان_اگر 
  }
  أرجع pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
عبارت *sqlite3_عبارت(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحيح op,                 /* Expression opcode */
  ثابت حرف *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  أرجع sqlite3_عبارت_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
عبارت *sqlite3_عبارت_Int32(sqlite3 *db, صحيح iVal){
  عبارت *pNew = sqlite3DbMallocRawNN(db, حجم(عبارت));
  إذا( pNew ){
    memset(pNew, 0, حجم(عبارت));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_مقدار|EP_برگ|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_مقدار = iVal;
#إذا SQLITE_MAX_عبارت_عمق>0
    pNew->n_ارتفاع = 1;
#پایان_اگر 
  }
  أرجع pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
فراغ sqlite3_عبارت_Attach_Subtrees(
  sqlite3 *db,
  عبارت *p_ریشه,
  عبارت *p_چپ,
  عبارت *p_راست
){
  إذا( p_ریشه==0 ){
    أكد( db->mallocFailed );
    sqlite3_عبارت_Delete(db, p_چپ);
    sqlite3_عبارت_Delete(db, p_راست);
  }وإلا{
    أكد( عبارت_Use_XList(p_ریشه) );
    أكد( p_ریشه->x.pSelect==0 );
    إذا( p_راست ){
      p_ریشه->p_راست = p_راست;
      p_ریشه->flags |= EP_Propagate & p_راست->flags;
#إذا SQLITE_MAX_عبارت_عمق>0
      p_ریشه->n_ارتفاع = p_راست->n_ارتفاع+1;
    }وإلا{
      p_ریشه->n_ارتفاع = 1;
#پایان_اگر
    }
    إذا( p_چپ ){
      p_ریشه->p_چپ = p_چپ;
      p_ریشه->flags |= EP_Propagate & p_چپ->flags;
#إذا SQLITE_MAX_عبارت_عمق>0
      إذا( p_چپ->n_ارتفاع>=p_ریشه->n_ارتفاع ){
        p_ریشه->n_ارتفاع = p_چپ->n_ارتفاع+1;
      }
#پایان_اگر
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
  صحيح op,                 /* Expression opcode */
  عبارت *p_چپ,            /* Left operand */
  عبارت *p_راست            /* Right operand */
){
  عبارت *p;
  p = sqlite3DbMallocRawNN(pParse->db, حجم(عبارت));
  إذا( p ){
    memset(p, 0, حجم(عبارت));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_عبارت_Attach_Subtrees(pParse->db, p, p_چپ, p_راست);
    sqlite3_عبارت_Check_ارتفاع(pParse, p->n_ارتفاع);
  }وإلا{
    sqlite3_عبارت_Delete(pParse->db, p_چپ);
    sqlite3_عبارت_Delete(pParse->db, p_راست);
  }
  أرجع p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
فراغ sqlite3_PExpr_جمع_کن_Select(Parse *pParse, عبارت *p_عبارت, Select *pSelect){
  إذا( p_عبارت ){
    p_عبارت->x.pSelect = pSelect;
    عبارت_مجموعه_Property(p_عبارت, EP_xIsSelect|EP_Subquery);
    sqlite3_عبارت_مجموعه_ارتفاع_And_Flags(pParse, p_عبارت);
  }وإلا{
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
Select *sqlite3_عبارت_List_مقصد_مقادیر(Parse *pParse, صحيح nElem, عبارت_List *pEList){
  صحيح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  لكل(ii=0; ii<pEList->n_عبارت; ii++){
    Select *pSel;
    عبارت *p_عبارت = pEList->a[ii].p_عبارت;
    صحيح n_عبارت_Elem;
    إذا( p_عبارت->op==TK_وکتور ){
      أكد( عبارت_Use_XList(p_عبارت) );
      n_عبارت_Elem = p_عبارت->x.pList->n_عبارت;
    }وإلا{
      n_عبارت_Elem = 1;
    }
    إذا( n_عبارت_Elem!=nElem ){
      sqlite3_Error_پیام(pParse, "IN(...) element has %d term%s - expected %d",
          n_عبارت_Elem, n_عبارت_Elem>1?"s":"", nElem
      );
      اكسر;
    }
    أكد( عبارت_Use_XList(p_عبارت) );
    pSel = sqlite3SelectNew(pParse, p_عبارت->x.pList, 0, 0, 0, 0, 0, SF_مقادیر,0);
    p_عبارت->x.pList = 0;
    إذا( pSel ){
      إذا( pRet ){
        pSel->op = TK_ALL;
        pSel->p_پیشین = pRet;
      }
      pRet = pSel;
    }
  }

  إذا( pRet && pRet->p_پیشین ){
    pRet->selFlags |= SF_Multi_مقدار;
  }
  sqlite3_عبارت_List_Delete(pParse->db, pEList);
  أرجع pRet;
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
  إذا( p_چپ==0  ){
    أرجع p_راست;
  }وإلا إذا( p_راست==0 ){
    أرجع p_چپ;
  }وإلا{
    u32 f = p_چپ->flags | p_راست->flags;
    إذا( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !درون_RENAME_OBJECT
    ){
      عبارت *p;
      sqlite3_عبارت_Deferred_Delete(pParse, p_چپ);
      sqlite3_عبارت_Deferred_Delete(pParse, p_راست);
      p = sqlite3_عبارت_Int32(db, 0);
      إذا( p ) p->op2 = 1;
      أرجع p;
    }وإلا{
      أرجع sqlite3PExpr(pParse, TK_AND, p_چپ, p_راست);
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
  صحيح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  عبارت *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3_عبارت_Alloc(db, TK_FUNCTION, pToken, 1);
  إذا( pNew==0 ){
    sqlite3_عبارت_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    أرجع 0;
  }
  أكد( !عبارت_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحيح)(pToken->z - pParse->z_دم);
  إذا( pList
   && pList->n_عبارت > pParse->db->a_حد[SQLITE_حد_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_پیام(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  عبارت_مجموعه_Property(pNew, EP_HasFunc);
  أكد( عبارت_Use_XList(pNew) );
  sqlite3_عبارت_مجموعه_ارتفاع_And_Flags(pParse, pNew);
  إذا( eDistinct==SF_Distinct ) عبارت_مجموعه_Property(pNew, EP_Distinct);
  أرجع pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
فراغ sqlite3_عبارت_ترتیب_By_Aggregate_Error(Parse *pParse, عبارت *p){
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
فراغ sqlite3_عبارت_جمع_کن_Function_ترتیب_By(
  Parse *pParse,        /* Parsing context */
  عبارت *p_عبارت,          /* The function call to which ORDER BY is to be added */
  عبارت_List *p_ترتیب_By    /* The ORDER BY clause to add */
){
  عبارت *pOB;
  sqlite3 *db = pParse->db;
  إذا( NEVER(p_ترتیب_By==0) ){
    أكد( db->mallocFailed );
    أرجع;
  }
  إذا( p_عبارت==0 ){
    أكد( db->mallocFailed );
    sqlite3_عبارت_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( p_عبارت->p_چپ==0 );
  أكد( عبارت_Use_XList(p_عبارت) );
  إذا( p_عبارت->x.pList==0 || NEVER(p_عبارت->x.pList->n_عبارت==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_جمع_کن_Cleanup(pParse, sqlite3_عبارت_List_Delete_Generic, p_ترتیب_By);
    أرجع;
  }
  إذا( Is_پنجره_Func(p_عبارت) ){
    sqlite3_عبارت_ترتیب_By_Aggregate_Error(pParse, p_عبارت);
    sqlite3_عبارت_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  إذا( p_ترتیب_By->n_عبارت>db->a_حد[SQLITE_حد_ستونی] ){
    sqlite3_Error_پیام(pParse, "too many terms in ORDER BY clause");
    sqlite3_عبارت_List_Delete(db, p_ترتیب_By);
    أرجع;
  }

  pOB = sqlite3_عبارت_Alloc(db, TK_ترتیب, 0, 0);
  إذا( pOB==0 ){
    sqlite3_عبارت_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  pOB->x.pList = p_ترتیب_By;
  أكد( عبارت_Use_XList(pOB) );
  p_عبارت->p_چپ = pOB;
  عبارت_مجموعه_Property(pOB, EP_پر_اندازه);
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
فراغ sqlite3_عبارت_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت عبارت *p_عبارت,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !درون_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_مستقیم|SQLITE_FUNC_UNSAFE))!=0 );
  إذا( عبارت_Has_Property(p_عبارت, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    إذا( (pDef->funcFlags & SQLITE_FUNC_مستقیم)!=0
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
فراغ sqlite3_عبارت_تخصیص_بده_Var_عدد(Parse *pParse, عبارت *p_عبارت, u32 n){
  sqlite3 *db = pParse->db;
  ثابت حرف *z;
  ynVar x;

  إذا( p_عبارت==0 ) أرجع;
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار|EP_Reduced|EP_TokenOnly) );
  z = p_عبارت->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  إذا( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }وإلا{
    صحيح do_جمع_کن = 0;
    إذا( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      صحيح bOk;
      إذا( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }وإلا{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->a_حد[SQLITE_حد_VARIABLE_عدد]-1 );
      testcase( i==db->a_حد[SQLITE_حد_VARIABLE_عدد] );
      إذا( bOk==0 || i<1 || i>db->a_حد[SQLITE_حد_VARIABLE_عدد] ){
        sqlite3_Error_پیام(pParse, "variable number must be between ?1 and ?%d",
            db->a_حد[SQLITE_حد_VARIABLE_عدد]);
        sqlite3_رکورد_Error_جابه_جایی_از_عبارت(pParse->db, p_عبارت);
        أرجع;
      }
      x = (ynVar)i;
      إذا( x>pParse->nVar ){
        pParse->nVar = (صحيح)x;
        do_جمع_کن = 1;
      }وإلا إذا( x<SQLITE_VNBMC*64 ){
        do_جمع_کن = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }وإلا إذا( sqlite3_VList_رقم_مقصد_Name(pParse->pVList, x)==0 ){
        do_جمع_کن = 1;
      }
    }وإلا{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_مقصد_رقم(pParse->pVList, z, n);
      إذا( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_جمع_کن = 1;
      }
    }
    إذا( do_جمع_کن ){
      pParse->pVList = sqlite3_VList_جمع_کن(db, pParse->pVList, z, n, x);
      إذا( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_عبارت->i_ستونی = x;
  إذا( x>db->a_حد[SQLITE_حد_VARIABLE_عدد] ){
    sqlite3_Error_پیام(pParse, "too many SQL variables");
    sqlite3_رکورد_Error_جابه_جایی_از_عبارت(pParse->db, p_عبارت);
  }
}

/*
** Recursively delete an expression tree.
*/
ساكن SQLITE_NOINLINE فراغ sqlite3_عبارت_Delete_NN(sqlite3 *db, عبارت *p){
  أكد( p!=0 );
  أكد( db!=0 );
عبارت_Delete_Restart:
  أكد( !عبارت_Use_UValue(p) || p->u.i_مقدار>=0 );
  أكد( !عبارت_Use_YWin(p) || !عبارت_Use_YSub(p) );
  أكد( !عبارت_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !عبارت_Use_YSub(p) );
#اگر_تعریف_شده SQLITE_DEBUG
  إذا( عبارت_Has_Property(p, EP_برگ) && !عبارت_Has_Property(p, EP_TokenOnly) ){
    أكد( p->p_چپ==0 );
    أكد( p->p_راست==0 );
    أكد( !عبارت_Use_XSelect(p) || p->x.pSelect==0 );
    أكد( !عبارت_Use_XList(p) || p->x.pList==0 );
  }
#پایان_اگر
  إذا( !عبارت_Has_Property(p, (EP_TokenOnly|EP_برگ)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (عبارت_Use_XList(p) && p->x.pList==0) || p->p_راست==0 );
    إذا( p->p_راست ){
      أكد( !عبارت_Has_Property(p, EP_WinFunc) );
      sqlite3_عبارت_Delete_NN(db, p->p_راست);
    }وإلا إذا( عبارت_Use_XSelect(p) ){
      أكد( !عبارت_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }وإلا{
      sqlite3_عبارت_List_Delete(db, p->x.pList);
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      إذا( عبارت_Has_Property(p, EP_WinFunc) ){
        sqlite3_پنجره_Delete(db, p->y.pWin);
      }
#پایان_اگر
    }
    إذا( p->p_چپ && p->op!=TK_SELECT_ستونی ){
      عبارت *p_چپ = p->p_چپ;
      إذا( !عبارت_Has_Property(p, EP_Static)
       && !عبارت_Has_Property(p_چپ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_چپ;
        اذهب عبارت_Delete_Restart;
      }وإلا{
        sqlite3_عبارت_Delete_NN(db, p_چپ);
      }
    }
  }
  إذا( !عبارت_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
فراغ sqlite3_عبارت_Delete(sqlite3 *db, عبارت *p){
  إذا( p ) sqlite3_عبارت_Delete_NN(db, p);
}
فراغ sqlite3_عبارت_Delete_Generic(sqlite3 *db, فراغ *p){
  إذا( ALWAYS(p) ) sqlite3_عبارت_Delete_NN(db, (عبارت*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
فراغ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  إذا( p==0 ){
    /* Nothing to clear */
  }وإلا إذا( p->pOn ){
    sqlite3_عبارت_Delete_NN(db, p->pOn);
  }وإلا إذا( p->pUsing ){
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
صحيح sqlite3_عبارت_Deferred_Delete(Parse *pParse, عبارت *p_عبارت){
  أرجع 0==sqlite3_Parser_جمع_کن_Cleanup(pParse, sqlite3_عبارت_Delete_Generic, p_عبارت);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
فراغ sqlite3_عبارت_Unmap_And_Delete(Parse *pParse, عبارت *p){
  إذا( p ){
    إذا( درون_RENAME_OBJECT ){
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
ساكن صحيح عبارت_Struct_اندازه(ثابت عبارت *p){
  إذا( عبارت_Has_Property(p, EP_TokenOnly) ) أرجع عبارت_TOKENONLYSIZE;
  إذا( عبارت_Has_Property(p, EP_Reduced) ) أرجع عبارت_REDUCEDSIZE;
  أرجع عبارت_FULLSIZE;
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
ساكن صحيح duped_عبارت_Struct_اندازه(ثابت عبارت *p, صحيح flags){
  صحيح n_اندازه;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( عبارت_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  إذا( 0==flags || عبارت_Has_Property(p, EP_پر_اندازه) ){
    n_اندازه = عبارت_FULLSIZE;
  }وإلا{
    أكد( !عبارت_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    أكد( !عبارت_Has_Property(p, EP_OuterON) );
    أكد( !عبارت_Has_VVAProperty(p, EP_NoReduce) );
    إذا( p->p_چپ || p->x.pList ){
      n_اندازه = عبارت_REDUCEDSIZE | EP_Reduced;
    }وإلا{
      أكد( p->p_راست==0 );
      n_اندازه = عبارت_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  أرجع n_اندازه;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ساكن صحيح duped_عبارت_گره_اندازه(ثابت عبارت *p, صحيح flags){
  صحيح nByte = duped_عبارت_Struct_اندازه(p, flags) & 0xfff;
  إذا( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  أرجع ROUND8(nByte);
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
ساكن صحيح duped_عبارت_اندازه(ثابت عبارت *p){
  صحيح nByte;
  أكد( p!=0 );
  nByte = duped_عبارت_گره_اندازه(p, EXPRDUP_REDUCE);
  إذا( p->p_چپ ) nByte += duped_عبارت_اندازه(p->p_چپ);
  إذا( p->p_راست ) nByte += duped_عبارت_اندازه(p->p_راست);
  أكد( nByte==ROUND8(nByte) );
  أرجع nByte;
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
#اگر_تعریف_شده SQLITE_DEBUG
  u8 *z_انتها;            /* First byte past the end of memory */
#پایان_اگر
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ساكن عبارت *عبارت_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت عبارت *p,        /* Expr tree to be duplicated */
  صحيح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  عبارت *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  صحيح nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  أكد( db!=0 );
  أكد( p );
  أكد( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  أكد( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  إذا( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#اگر_تعریف_شده SQLITE_DEBUG
    sEdupBuf.z_انتها = pEdupBuf->z_انتها;
#پایان_اگر
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }وإلا{
    صحيح nAlloc;
    إذا( dupFlags ){
      nAlloc = duped_عبارت_اندازه(p);
    }وإلا إذا( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(عبارت_FULLSIZE + nToken);
    }وإلا{
      nToken = 0;
      nAlloc = ROUND8(عبارت_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#اگر_تعریف_شده SQLITE_DEBUG
    sEdupBuf.z_انتها = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#پایان_اگر
    
    staticFlag = 0;
  }
  pNew = (عبارت *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  إذا( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت بیعلامت n_Struct_اندازه = duped_عبارت_Struct_اندازه(p, dupFlags);
    صحيح n_New_اندازه = n_Struct_اندازه & 0xfff;
    إذا( nToken<0 ){
      إذا( !عبارت_Has_Property(p, EP_Int_مقدار) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }وإلا{
        nToken = 0;
      }
    }
    إذا( dupFlags ){
      أكد( (صحيح)(sEdupBuf.z_انتها - sEdupBuf.zAlloc) >= n_New_اندازه+nToken );
      أكد( عبارت_Has_Property(p, EP_Reduced)==0 );
      کپی_حافظه(sEdupBuf.zAlloc, p, n_New_اندازه);
    }وإلا{
      u32 n_اندازه = (u32)عبارت_Struct_اندازه(p);
      أكد( (صحيح)(sEdupBuf.z_انتها - sEdupBuf.zAlloc) >=
                                                   (صحيح)عبارت_FULLSIZE+nToken );
      کپی_حافظه(sEdupBuf.zAlloc, p, n_اندازه);
      إذا( n_اندازه<عبارت_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_اندازه], 0, عبارت_FULLSIZE-n_اندازه);
      }
      n_New_اندازه = عبارت_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_اندازه & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    عبارت_Clear_VVAProperties(pNew);
    إذا( dupFlags ){
      عبارت_مجموعه_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    إذا( nToken>0 ){
      حرف *zToken = pNew->u.zToken = (حرف*)&sEdupBuf.zAlloc[n_New_اندازه];
      کپی_حافظه(zToken, p->u.zToken, nToken);
      n_New_اندازه += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_اندازه);

    إذا( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_برگ))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      إذا( عبارت_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }وإلا{
        pNew->x.pList = sqlite3_عبارت_List_Dup(db, p->x.pList,
                           p->op!=TK_ترتیب ? dupFlags : 0);
      }

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      إذا( عبارت_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_پنجره_Dup(db, pNew, p->y.pWin);
        أكد( عبارت_Has_Property(pNew, EP_WinFunc) );
      }
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      إذا( dupFlags ){
        إذا( p->op==TK_SELECT_ستونی ){
          pNew->p_چپ = p->p_چپ;
          أكد( p->p_راست==0 
               || p->p_راست==p->p_چپ
               || عبارت_Has_Property(p->p_چپ, EP_Subquery) );
        }وإلا{
          pNew->p_چپ = p->p_چپ ?
                      عبارت_Dup(db, p->p_چپ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_راست = p->p_راست ?
                       عبارت_Dup(db, p->p_راست, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }وإلا{
        إذا( p->op==TK_SELECT_ستونی ){
          pNew->p_چپ = p->p_چپ;
          أكد( p->p_راست==0 
               || p->p_راست==p->p_چپ
               || عبارت_Has_Property(p->p_چپ, EP_Subquery) );
        }وإلا{
          pNew->p_چپ = sqlite3_عبارت_Dup(db, p->p_چپ, 0);
        }
        pNew->p_راست = sqlite3_عبارت_Dup(db, p->p_راست, 0);
      }
    }
  }
  إذا( pEdupBuf ) کپی_حافظه(pEdupBuf, &sEdupBuf, حجم(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.z_انتها );
  أرجع pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#اگر_تعریف_نشده SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  إذا( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    إذا( pRet ){
      صحيح i;
      pRet->nCte = p->nCte;
      لكل(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].p_تعداد_ستون = sqlite3_عبارت_List_Dup(db, p->a[i].p_تعداد_ستون, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  أرجع pRet;
}
#وإلا
# define sqlite3WithDup(x,y) 0
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ساكن صحيح gatherSelectWindowsCallback(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_FUNCTION && عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    پنجره *pWin = p_عبارت->y.pWin;
    أكد( pWin );
    أكد( Is_پنجره_Func(p_عبارت) );
    أكد( pWin->ppThis==0 );
    sqlite3_پنجره_Link(pSelect, pWin);
  }
  أرجع WRC_Continue;
}
ساكن صحيح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  أرجع p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ساكن فراغ gatherSelectWindows(Select *p){
  Walker w;
  w.x_عبارت_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#پایان_اگر


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
عبارت *sqlite3_عبارت_Dup(sqlite3 *db, ثابت عبارت *p, صحيح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  أرجع p ? عبارت_Dup(db, p, flags, 0) : 0;
}
عبارت_List *sqlite3_عبارت_List_Dup(sqlite3 *db, ثابت عبارت_List *p, صحيح flags){
  عبارت_List *pNew;
  بنية عبارت_List_قلم *p_قلم;
  ثابت بنية عبارت_List_قلم *p_Old_قلم;
  صحيح i;
  عبارت *p_پیشین_Select_ستون_Old = 0;
  عبارت *p_پیشین_Select_ستون_New = 0;
  أكد( db!=0 );
  إذا( p==0 ) أرجع 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_اندازه(db, p));
  إذا( pNew==0 ) أرجع 0;
  pNew->n_عبارت = p->n_عبارت;
  pNew->nAlloc = p->nAlloc;
  p_قلم = pNew->a;
  p_Old_قلم = p->a;
  لكل(i=0; i<p->n_عبارت; i++, p_قلم++, p_Old_قلم++){
    عبارت *p_Old_عبارت = p_Old_قلم->p_عبارت;
    عبارت *p_New_عبارت;
    p_قلم->p_عبارت = sqlite3_عبارت_Dup(db, p_Old_عبارت, flags);
    إذا( p_Old_عبارت
     && p_Old_عبارت->op==TK_SELECT_ستونی
     && (p_New_عبارت = p_قلم->p_عبارت)!=0
    ){
      إذا( p_New_عبارت->p_راست ){
        p_پیشین_Select_ستون_Old = p_Old_عبارت->p_راست;
        p_پیشین_Select_ستون_New = p_New_عبارت->p_راست;
        p_New_عبارت->p_چپ = p_New_عبارت->p_راست;
      }وإلا{
        إذا( p_Old_عبارت->p_چپ!=p_پیشین_Select_ستون_Old ){
          p_پیشین_Select_ستون_Old = p_Old_عبارت->p_چپ;
          p_پیشین_Select_ستون_New = sqlite3_عبارت_Dup(db, p_پیشین_Select_ستون_Old, flags);
          p_New_عبارت->p_راست = p_پیشین_Select_ستون_New;
        }
        p_New_عبارت->p_چپ = p_پیشین_Select_ستون_New;
      }
    }
    p_قلم->zEName = sqlite3DbStrDup(db, p_Old_قلم->zEName);
    p_قلم->fg = p_Old_قلم->fg;
    p_قلم->u = p_Old_قلم->u;
  }
  أرجع pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ثابت SrcList *p, صحيح flags){
  SrcList *pNew;
  صحيح i;
  أكد( db!=0 );
  إذا( p==0 ) أرجع 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  إذا( pNew==0 ) أرجع 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  لكل(i=0; i<p->nSrc; i++){
    Src_قلم *p_New_قلم = &pNew->a[i];
    ثابت Src_قلم *p_Old_قلم = &p->a[i];
    جدول *pTab;
    p_New_قلم->fg = p_Old_قلم->fg;
    إذا( p_Old_قلم->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, حجم(Subquery));
      إذا( pNewSubq==0 ){
        أكد( db->mallocFailed );
        p_New_قلم->fg.isSubquery = 0;
      }وإلا{
        کپی_حافظه(pNewSubq, p_Old_قلم->u4.pSubq, حجم(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        إذا( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_قلم->fg.isSubquery = 0;
        }
      }
      p_New_قلم->u4.pSubq = pNewSubq;
    }وإلا إذا( p_Old_قلم->fg.fixedSchema ){
      p_New_قلم->u4.pSchema = p_Old_قلم->u4.pSchema;
    }وإلا{
      p_New_قلم->u4.zDatabase = sqlite3DbStrDup(db, p_Old_قلم->u4.zDatabase);
    }
    p_New_قلم->zName = sqlite3DbStrDup(db, p_Old_قلم->zName);
    p_New_قلم->zAlias = sqlite3DbStrDup(db, p_Old_قلم->zAlias);
    p_New_قلم->iCursor = p_Old_قلم->iCursor;
    إذا( p_New_قلم->fg.is_نمایهای_By ){
      p_New_قلم->u1.z_نمایهای_By = sqlite3DbStrDup(db, p_Old_قلم->u1.z_نمایهای_By);
    }وإلا إذا( p_New_قلم->fg.isTabFunc ){
      p_New_قلم->u1.pFuncArg =
          sqlite3_عبارت_List_Dup(db, p_Old_قلم->u1.pFuncArg, flags);
    }وإلا{
      p_New_قلم->u1.n_ردیف = p_Old_قلم->u1.n_ردیف;
    }
    p_New_قلم->u2 = p_Old_قلم->u2;
    إذا( p_New_قلم->fg.isCte ){
      p_New_قلم->u2.pCteUse->nUse++;
    }
    pTab = p_New_قلم->pSTab = p_Old_قلم->pSTab;
    إذا( pTab ){
      pTab->nTabRef++;
    }
    إذا( p_Old_قلم->fg.isUsing ){
      أكد( p_New_قلم->fg.isUsing );
      p_New_قلم->u3.pUsing = sqlite3IdListDup(db, p_Old_قلم->u3.pUsing);
    }وإلا{
      p_New_قلم->u3.pOn = sqlite3_عبارت_Dup(db, p_Old_قلم->u3.pOn, flags);
    }
    p_New_قلم->ستون_استفاده_شده = p_Old_قلم->ستون_استفاده_شده;
  }
  أرجع pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ثابت IdList *p){
  IdList *pNew;
  صحيح i;
  أكد( db!=0 );
  إذا( p==0 ) أرجع 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  إذا( pNew==0 ) أرجع 0;
  pNew->nId = p->nId;
  لكل(i=0; i<p->nId; i++){
    بنية Id_List_قلم *p_New_قلم = &pNew->a[i];
    ثابت بنية Id_List_قلم *p_Old_قلم = &p->a[i];
    p_New_قلم->zName = sqlite3DbStrDup(db, p_Old_قلم->zName);
  }
  أرجع pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحيح flags){
  Select *pRet = 0;
  Select *p_بعدی = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  لكل(p=pDup; p; p=p->p_پیشین){
    Select *pNew = sqlite3DbMallocRawNN(db, حجم(*p) );
    إذا( pNew==0 ) اكسر;
    pNew->pEList = sqlite3_عبارت_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_عبارت_Dup(db, p->pWhere, flags);
    pNew->p_گروه_By = sqlite3_عبارت_List_Dup(db, p->p_گروه_By, flags);
    pNew->pHaving = sqlite3_عبارت_Dup(db, p->pHaving, flags);
    pNew->p_ترتیب_By = sqlite3_عبارت_List_Dup(db, p->p_ترتیب_By, flags);
    pNew->op = p->op;
    pNew->p_بعدی = p_بعدی;
    pNew->p_پیشین = 0;
    pNew->p_حد = sqlite3_عبارت_Dup(db, p->p_حد, flags);
    pNew->i_حد = 0;
    pNew->i_جابه_جایی = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_ردیف = p->n_Select_ردیف;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_پنجره_List_Dup(db, p->pWinDefn);
    إذا( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#پایان_اگر
    pNew->selId = p->selId;
    إذا( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_بعدی = 0;
      sqlite3SelectDelete(db, pNew);
      اكسر;
    }
    *pp = pNew;
    pp = &pNew->p_پیشین;
    p_بعدی = pNew;
  }
  أرجع pRet;
}
#وإلا
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحيح flags){
  أكد( p==0 );
  أرجع 0;
}
#پایان_اگر


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
ساكن ثابت بنية عبارت_List_قلم zero_قلم = {0};
SQLITE_NOINLINE عبارت_List *sqlite3_عبارت_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  بنية عبارت_List_قلم *p_قلم;
  عبارت_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  إذا( pList==0 ){
    sqlite3_عبارت_Delete(db, p_عبارت);
    أرجع 0;
  }
  pList->nAlloc = 4;
  pList->n_عبارت = 1;
  p_قلم = &pList->a[0];
  *p_قلم = zero_قلم;
  p_قلم->p_عبارت = p_عبارت;
  أرجع pList;
}
SQLITE_NOINLINE عبارت_List *sqlite3_عبارت_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  عبارت_List *pList,        /* List to which to append. Might be NULL */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  بنية عبارت_List_قلم *p_قلم;
  عبارت_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  إذا( pNew==0 ){
    sqlite3_عبارت_List_Delete(db, pList);
    sqlite3_عبارت_Delete(db, p_عبارت);
    أرجع 0;
  }وإلا{
    pList = pNew;
  }
  p_قلم = &pList->a[pList->n_عبارت++];
  *p_قلم = zero_قلم;
  p_قلم->p_عبارت = p_عبارت;
  أرجع pList;
}
عبارت_List *sqlite3_عبارت_List_Append(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to append. Might be NULL */
  عبارت *p_عبارت             /* Expression to be appended. Might be NULL */
){
  بنية عبارت_List_قلم *p_قلم;
  إذا( pList==0 ){
    أرجع sqlite3_عبارت_List_Append_New(pParse->db,p_عبارت);
  }
  إذا( pList->nAlloc<pList->n_عبارت+1 ){
    أرجع sqlite3_عبارت_List_Append_Grow(pParse->db,pList,p_عبارت);
  }
  p_قلم = &pList->a[pList->n_عبارت++];
  *p_قلم = zero_قلم;
  p_قلم->p_عبارت = p_عبارت;
  أرجع pList;
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
عبارت_List *sqlite3_عبارت_List_Append_وکتور(
  Parse *pParse,         /* Parsing context */
  عبارت_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  عبارت *p_عبارت            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  صحيح n;
  صحيح i;
  صحيح i_اولی = pList ? pList->n_عبارت : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  إذا( NEVER(pColumns==0) ) اذهب وکتور_append_error;
  إذا( p_عبارت==0 ) اذهب وکتور_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  إذا( p_عبارت->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_عبارت_وکتور_اندازه(p_عبارت)) ){
    sqlite3_Error_پیام(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    اذهب وکتور_append_error;
  }

  لكل(i=0; i<pColumns->nId; i++){
    عبارت *p_Sub_عبارت = sqlite3_عبارت_For_وکتور_فیلد(pParse, p_عبارت, i, pColumns->nId);
    أكد( p_Sub_عبارت!=0 || db->mallocFailed );
    إذا( p_Sub_عبارت==0 ) استمر;
    pList = sqlite3_عبارت_List_Append(pParse, pList, p_Sub_عبارت);
    إذا( pList ){
      أكد( pList->n_عبارت==i_اولی+i+1 );
      pList->a[pList->n_عبارت-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  إذا( !db->mallocFailed && p_عبارت->op==TK_SELECT && ALWAYS(pList!=0) ){
    عبارت *p_اولی = pList->a[i_اولی].p_عبارت;
    أكد( p_اولی!=0 );
    أكد( p_اولی->op==TK_SELECT_ستونی );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    p_اولی->p_راست = p_عبارت;
    p_عبارت = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    p_اولی->i_جدول = pColumns->nId;
  }

وکتور_append_error:
  sqlite3_عبارت_Unmap_And_Delete(pParse, p_عبارت);
  sqlite3IdListDelete(db, pColumns);
  أرجع pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
فراغ sqlite3_عبارت_List_مجموعه_مرتب_سازی_ترتیب(عبارت_List *p, صحيح i_مرتب_سازی_ترتیب, صحيح eNulls){
  بنية عبارت_List_قلم *p_قلم;
  إذا( p==0 ) أرجع;
  أكد( p->n_عبارت>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( i_مرتب_سازی_ترتیب==SQLITE_SO_UNDEFINED
       || i_مرتب_سازی_ترتیب==SQLITE_SO_ASC
       || i_مرتب_سازی_ترتیب==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_قلم = &p->a[p->n_عبارت-1];
  أكد( p_قلم->fg.bNulls==0 );
  إذا( i_مرتب_سازی_ترتیب==SQLITE_SO_UNDEFINED ){
    i_مرتب_سازی_ترتیب = SQLITE_SO_ASC;
  }
  p_قلم->fg.مرتب_سازی_Flags = (u8)i_مرتب_سازی_ترتیب;

  إذا( eNulls!=SQLITE_SO_UNDEFINED ){
    p_قلم->fg.bNulls = 1;
    إذا( i_مرتب_سازی_ترتیب!=eNulls ){
      p_قلم->fg.مرتب_سازی_Flags |= KEYINFO_ترتیب_BIGNULL;
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
فراغ sqlite3_عبارت_List_مجموعه_Name(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحيح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->e_Parse_نما!=PARSE_نما_UNMAP || dequote==0 );
  إذا( pList ){
    بنية عبارت_List_قلم *p_قلم;
    أكد( pList->n_عبارت>0 );
    p_قلم = &pList->a[pList->n_عبارت-1];
    أكد( p_قلم->zEName==0 );
    أكد( p_قلم->fg.eEName==ENAME_NAME );
    p_قلم->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    إذا( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_قلم->zEName);
      إذا( درون_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ثابت فراغ*)p_قلم->zEName, pName);
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
فراغ sqlite3_عبارت_List_مجموعه_بازه(
  Parse *pParse,          /* Parsing context */
  عبارت_List *pList,        /* List to which to add the span. */
  ثابت حرف *z_شروع,     /* Start of the span */
  ثابت حرف *z_انتها        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  إذا( pList ){
    بنية عبارت_List_قلم *p_قلم = &pList->a[pList->n_عبارت-1];
    أكد( pList->n_عبارت>0 );
    إذا( p_قلم->zEName==0 ){
      p_قلم->zEName = sqlite3_Db_بازه_Dup(db, z_شروع, z_انتها);
      p_قلم->fg.eEName = ENAME_بازه;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
فراغ sqlite3_عبارت_List_Check_درازی(
  Parse *pParse,
  عبارت_List *pEList,
  ثابت حرف *zObject
){
  صحيح mx = pParse->db->a_حد[SQLITE_حد_ستونی];
  testcase( pEList && pEList->n_عبارت==mx );
  testcase( pEList && pEList->n_عبارت==mx+1 );
  إذا( pEList && pEList->n_عبارت>mx ){
    sqlite3_Error_پیام(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ساكن SQLITE_NOINLINE فراغ عبارت_List_Delete_NN(sqlite3 *db, عبارت_List *pList){
  صحيح i = pList->n_عبارت;
  بنية عبارت_List_قلم *p_قلم =  pList->a;
  أكد( pList->n_عبارت>0 );
  أكد( db!=0 );
  افعل{
    sqlite3_عبارت_Delete(db, p_قلم->p_عبارت);
    إذا( p_قلم->zEName ) sqlite3DbNNFreeNN(db, p_قلم->zEName);
    p_قلم++;
  }طالما( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
فراغ sqlite3_عبارت_List_Delete(sqlite3 *db, عبارت_List *pList){
  إذا( pList ) عبارت_List_Delete_NN(db, pList);
}
فراغ sqlite3_عبارت_List_Delete_Generic(sqlite3 *db, فراغ *pList){
  إذا( ALWAYS(pList) ) عبارت_List_Delete_NN(db, (عبارت_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_عبارت_List_Flags(ثابت عبارت_List *pList){
  صحيح i;
  u32 m = 0;
  أكد( pList!=0 );
  لكل(i=0; i<pList->n_عبارت; i++){
     عبارت *p_عبارت = pList->a[i].p_عبارت;
     أكد( p_عبارت!=0 );
     m |= p_عبارت->flags;
  }
  أرجع m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
صحيح sqlite3SelectWalkFail(Walker *pWalker, Select *Not_استفاده_شده){
  استفادهنشده_PARAMETER(Not_استفاده_شده);
  pWalker->e_کد = 0;
  أرجع WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ثابت حرف *z_درون){
  إذا( sqlite3StrICmp(z_درون, "true")==0  ) أرجع EP_IsTrue;
  إذا( sqlite3StrICmp(z_درون, "false")==0 ) أرجع EP_IsFalse;
  أرجع 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحيح sqlite3_عبارت_Id_مقصد_True_False(عبارت *p_عبارت){
  u32 v;
  أكد( p_عبارت->op==TK_ID || p_عبارت->op==TK_STRING );
  إذا( !عبارت_Has_Property(p_عبارت, EP_Quoted|EP_Int_مقدار)
   && (v = sqlite3IsTrueOrFalse(p_عبارت->u.zToken))!=0
  ){
    p_عبارت->op = TK_TRUEFALSE;
    عبارت_مجموعه_Property(p_عبارت, v);
    أرجع 1;
  }
  أرجع 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحيح sqlite3_عبارت_Truth_مقدار(ثابت عبارت *p_عبارت){
  p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely((عبارت*)p_عبارت);
  أكد( p_عبارت->op==TK_TRUEFALSE );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  أكد( sqlite3StrICmp(p_عبارت->u.zToken,"true")==0
       || sqlite3StrICmp(p_عبارت->u.zToken,"false")==0 );
  أرجع p_عبارت->u.zToken[4]==0;
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
  إذا( p_عبارت->op==TK_AND || p_عبارت->op==TK_OR ){
    عبارت *p_راست = sqlite3_عبارت_Simplified_And_Or(p_عبارت->p_راست);
    عبارت *p_چپ = sqlite3_عبارت_Simplified_And_Or(p_عبارت->p_چپ);
    إذا( عبارت_Always_True(p_چپ) || عبارت_Always_False(p_راست) ){
      p_عبارت = p_عبارت->op==TK_AND ? p_راست : p_چپ;
    }وإلا إذا( عبارت_Always_True(p_راست) || عبارت_Always_False(p_چپ) ){
      p_عبارت = p_عبارت->op==TK_AND ? p_چپ : p_راست;
    }
  }
  أرجع p_عبارت;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ساكن صحيح عبارت_Eval_Rhs_اولی(عبارت *p_عبارت){
  إذا( عبارت_Has_Property(p_عبارت->p_چپ, EP_Subquery)
   && !عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery)
  ){
    أرجع 1;
  }وإلا{
    أرجع 0;
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
ساكن صحيح عبارت_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  عبارت *p_عبارت,       /* The comparison expression */
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
  إذا( عبارت_Eval_Rhs_اولی(p_عبارت) && sqlite3_عبارت_Can_Be_Null(p_عبارت->p_راست) ){
    r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, pFree2);
    addrIsNull = sqlite3_Vdbe_جمع_کن_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }وإلا{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, pFree1);
  إذا( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    إذا( عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery)
     && sqlite3_عبارت_Can_Be_Null(p_عبارت->p_چپ)
    ){
      addrIsNull = sqlite3_Vdbe_جمع_کن_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  أرجع addrIsNull;
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
ساكن SQLITE_NOINLINE صحيح عبارت_گره_Is_Constant_Function(
  Walker *pWalker,
  عبارت *p_عبارت
){
  صحيح n;             /* Number of arguments */
  عبارت_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( p_عبارت->op==TK_FUNCTION );
  إذا( عبارت_Has_Property(p_عبارت, EP_TokenOnly)
   || (pList = p_عبارت->x.pList)==0
  ){;
    n = 0;
  }وإلا{
    n = pList->n_عبارت;
    sqlite3_Walk_عبارت_List(pWalker, pList);
    إذا( pWalker->e_کد==0 ) أرجع WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_پیدا_کن_Function(db, p_عبارت->u.zToken, n, ENC(db), 0);
  إذا( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(عبارت_Has_Property(p_عبارت, EP_WinFunc))
  ){
    pWalker->e_کد = 0;
    أرجع WRC_Abort;
  }
  أرجع WRC_Prune;
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
ساكن صحيح عبارت_گره_Is_Constant(Walker *pWalker, عبارت *p_عبارت){
  أكد( pWalker->e_کد>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  إذا( pWalker->e_کد==2 && عبارت_Has_Property(p_عبارت, EP_OuterON) ){
    pWalker->e_کد = 0;
    أرجع WRC_Abort;
  }

  بدل( p_عبارت->op ){
    حالة TK_FUNCTION:
      إذا( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
        pWalker->e_کد = 0;  /* Window functions are never constant */
        أرجع WRC_Abort;
      }وإلا إذا( pWalker->e_کد>=4 && pWalker->e_کد<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        إذا( pWalker->e_کد==5 ) عبارت_مجموعه_Property(p_عبارت, EP_FromDDL);
        أرجع WRC_Continue;
      }وإلا إذا( عبارت_Has_Property(p_عبارت,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        أرجع WRC_Continue;
      }وإلا إذا( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        أرجع عبارت_گره_Is_Constant_Function(pWalker, p_عبارت);
      }وإلا{
        pWalker->e_کد = 0;  /* Function call is not constant */
        أرجع WRC_Abort;
      }
    حالة TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      إذا( sqlite3_عبارت_Id_مقصد_True_False(p_عبارت) ){
        أرجع WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالة TK_ستونی:
    حالة TK_AGG_FUNCTION:
    حالة TK_AGG_ستونی:
      testcase( p_عبارت->op==TK_ID );
      testcase( p_عبارت->op==TK_ستونی );
      testcase( p_عبارت->op==TK_AGG_FUNCTION );
      testcase( p_عبارت->op==TK_AGG_ستونی );
      إذا( عبارت_Has_Property(p_عبارت, EP_Fixed_ستون) && pWalker->e_کد!=2 ){
        أرجع WRC_Continue;
      }
      إذا( pWalker->e_کد==3 && p_عبارت->i_جدول==pWalker->u.iCur ){
        أرجع WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالة TK_IF_NULL_ردیف:
    حالة TK_ثبات:
    حالة TK_DOT:
    حالة TK_RAISE:
      testcase( p_عبارت->op==TK_ثبات );
      testcase( p_عبارت->op==TK_IF_NULL_ردیف );
      testcase( p_عبارت->op==TK_DOT );
      testcase( p_عبارت->op==TK_RAISE );
      pWalker->e_کد = 0;
      أرجع WRC_Abort;
    حالة TK_IS:
    حالة TK_ISNOT:
      إذا( pWalker->e_کد==6 ){
        pWalker->e_کد = 0;
        أرجع WRC_Abort;
      }
      أرجع WRC_Continue;
    حالة TK_VARIABLE:
      إذا( pWalker->e_کد==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_عبارت->op = TK_NULL;
      }وإلا إذا( pWalker->e_کد==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_کد = 0;
        أرجع WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    افتراضي:
      testcase( p_عبارت->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_عبارت->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      أرجع WRC_Continue;
  }
}
ساكن صحيح عبارت_Is_Const(Parse *pParse, عبارت *p, صحيح initFlag){
  Walker w;
  w.e_کد = initFlag;
  w.pParse = pParse;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شده SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#پایان_اگر
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد;
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
صحيح sqlite3_عبارت_Is_Constant(Parse *pParse, عبارت *p){
  أرجع عبارت_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحيح sqlite3_عبارت_List_Is_Constant(Parse *pParse, عبارت_List *pList, صحيح bNoIs){
  ثابت صحيح iInit = bNoIs ? 6 : 1;
  صحيح ii;
  لكل(ii=0; ii<pList->n_عبارت; ii++){
    إذا( 0==عبارت_Is_Const(pParse, pList->a[ii].p_عبارت, iInit) ) أرجع 0;
  }
  أرجع 1;
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
ساكن صحيح sqlite3_عبارت_Is_Constant_Not_Join(Parse *pParse, عبارت *p){
  أرجع عبارت_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ساكن صحيح عبارت_Select_Walk_جدول_Constant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->e_کد==3 || pWalker->e_کد==0 );
  إذا( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_کد = 0;
    أرجع WRC_Abort;
  }
  أرجع WRC_Prune;
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
صحيح sqlite3_عبارت_Is_جدول_Constant(عبارت *p, صحيح iCur, صحيح bAllowSubq){
  Walker w;
  w.e_کد = 3;
  w.pParse = 0;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant;
  إذا( bAllowSubq ){
    w.xSelectCallback = عبارت_Select_Walk_جدول_Constant;
  }وإلا{
    w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شده SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#پایان_اگر
  }
  w.u.iCur = iCur;
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد;
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
صحيح sqlite3_عبارت_Is_Single_جدول_Constraint(
  عبارت *p_عبارت,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحيح iSrc,                    /* Which element of pSrcList to use */
  صحيح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت Src_قلم *pSrc = &pSrcList->a[iSrc];
  إذا( pSrc->fg.jointype & JT_LTORJ ){
    أرجع 0;  /* rule (3) */
  }
  إذا( pSrc->fg.jointype & JT_چپ ){
    إذا( !عبارت_Has_Property(p_عبارت, EP_OuterON) ) أرجع 0;   /* rule (4a) */
    إذا( p_عبارت->w.iJoin!=pSrc->iCursor ) أرجع 0;         /* rule (4b) */
  }وإلا{
    إذا( عبارت_Has_Property(p_عبارت, EP_OuterON) ) أرجع 0;    /* rule (5) */
  }
  إذا( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      إذا( p_عبارت->w.iJoin==pSrcList->a[jj].iCursor ){
        إذا( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          أرجع 0;  /* restriction (6) */
        }
        اكسر;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  أرجع sqlite3_عبارت_Is_جدول_Constant(p_عبارت, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ساكن صحيح عبارت_گره_Is_Constant_Or_گروه_By(Walker *pWalker, عبارت *p_عبارت){
  عبارت_List *p_گروه_By = pWalker->u.p_گروه_By;
  صحيح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  لكل(i=0; i<p_گروه_By->n_عبارت; i++){
    عبارت *p = p_گروه_By->a[i].p_عبارت;
    إذا( sqlite3_عبارت_Compare(0, p_عبارت, p, -1)<2 ){
      CollSeq *pColl = sqlite3_عبارت_NNColl_Seq(pWalker->pParse, p);
      إذا( sqlite3_Is_دودویی(pColl) ){
        أرجع WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  إذا( عبارت_Use_XSelect(p_عبارت) ){
    pWalker->e_کد = 0;
    أرجع WRC_Abort;
  }

  أرجع عبارت_گره_Is_Constant(pWalker, p_عبارت);
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
صحيح sqlite3_عبارت_Is_Constant_Or_گروه_By(Parse *pParse, عبارت *p, عبارت_List *p_گروه_By){
  Walker w;
  w.e_کد = 1;
  w.x_عبارت_Callback = عبارت_گره_Is_Constant_Or_گروه_By;
  w.xSelectCallback = 0;
  w.u.p_گروه_By = p_گروه_By;
  w.pParse = pParse;
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد;
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
صحيح sqlite3_عبارت_Is_Constant_Or_Function(عبارت *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  أرجع عبارت_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحيح sqlite3_عبارت_شامل_است_Subquery(عبارت *p){
  Walker w;
  w.e_کد = 1;
  w.x_عبارت_Callback = sqlite3_عبارت_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شده SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#پایان_اگر
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد==0;
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
صحيح sqlite3_عبارت_Is_Integer(ثابت عبارت *p, صحيح *p_مقدار, Parse *pParse, صحيح bRSI){
  صحيح iSign = 1;   /* Either +1 or -1. */
  طالما( 1/*exit-by-break*/ ){
    إذا( NEVER(p==0) ) أرجع 0;
    إذا( عبارت_Use_UValue(p) ){
      أكد( p->op==TK_INTEGER );
      *p_مقدار = p->u.i_مقدار*iSign;
      إذا( bRSI && pParse==0 && p->op2 ){
        أرجع 0;
      }
      أرجع 1;
    }
    إذا( p->op==TK_UPLUS ){
      p = p->p_چپ;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_چپ;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_مقدار *pVal; /* The variable */
      صحيح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      إذا( NEVER(pParse->pVdbe==0) ) اكسر;
      إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) اكسر;
      pVal = sqlite3_Vdbe_Get_Bound_مقدار(pParse->pReprepare, p->i_ستونی,
                                      SQLITE_AFF_BLOB);
      إذا( pVal ){
        i64 vv;
        إذا( sqlite3_مقدار_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_مقدار_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_مقدار = (صحيح)vv;
          rc = 1;       
        }
        sqlite3_مقدار_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_ستونی, bRSI);
      أرجع rc;
    }
    اكسر;
  }
  أرجع 0;
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
صحيح sqlite3_عبارت_Can_Be_Null(ثابت عبارت *p){
  u8 op;
  أكد( p!=0 );
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_چپ;
    أكد( p!=0 );
  }
  op = p->op;
  إذا( op==TK_ثبات ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER:
    حالة TK_STRING:
    حالة TK_FLOAT:
    حالة TK_BLOB:
      أرجع 0;
    حالة TK_ستونی:
      أكد( عبارت_Use_YTab(p) );
      أرجع عبارت_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#اگر_تعریف_شده SQLITE_ALLOW_ROWID_درون_VIEW
          || (p->i_ستونی==XN_ROWID && IsView(p->y.pTab))
#پایان_اگر
          || (p->i_ستونی>=0
              && p->y.pTab->a_ستون!=0 /* Possible due to prior error */
              && ALWAYS(p->i_ستونی<p->y.pTab->n_ستون)
              && p->y.pTab->a_ستون[p->i_ستونی].notNull==0);
    افتراضي:
      أرجع 1;
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
صحيح sqlite3_عبارت_Needs_No_Affinity_Change(ثابت عبارت *p, حرف aff){
  u8 op;
  صحيح unaryMinus = 0;
  إذا( aff==SQLITE_AFF_BLOB ) أرجع 1;
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    إذا( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_چپ;
  }
  op = p->op;
  إذا( op==TK_ثبات ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER: {
      أرجع aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_FLOAT: {
      أرجع aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_STRING: {
      أرجع !unaryMinus && aff==SQLITE_AFF_متن;
    }
    حالة TK_BLOB: {
      أرجع !unaryMinus;
    }
    حالة TK_ستونی: {
      أكد( p->i_جدول>=0 );  /* p cannot be part of a CHECK constraint */
      أرجع aff>=SQLITE_AFF_NUMERIC && p->i_ستونی<0;
    }
    افتراضي: {
      أرجع 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
صحيح sqlite3IsRowid(ثابت حرف *z){
  إذا( sqlite3StrICmp(z, "_ROWID_")==0 ) أرجع 1;
  إذا( sqlite3StrICmp(z, "ROWID")==0 ) أرجع 1;
  إذا( sqlite3StrICmp(z, "OID")==0 ) أرجع 1;
  أرجع 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ثابت حرف *sqlite3RowidAlias(جدول *pTab){
  ثابت حرف *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  صحيح ii;
  أكد( VisibleRowid(pTab) );
  لكل(ii=0; ii<Array_اندازه(azOpt); ii++){
    إذا( sqlite3_ستونی_شاخص(pTab, azOpt[ii])<0 ) أرجع azOpt[ii];
  }
  أرجع 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
ساكن Select *is_Candidate_For_درون_Opt(ثابت عبارت *pX){
  Select *p;
  SrcList *pSrc;
  عبارت_List *pEList;
  جدول *pTab;
  صحيح i;
  إذا( !عبارت_Use_XSelect(pX) ) أرجع 0;                 /* Not a subquery */
  إذا( عبارت_Has_Property(pX, EP_VarSelect)  ) أرجع 0;  /* Correlated subq */
  p = pX->x.pSelect;
  إذا( p->p_پیشین ) أرجع 0;              /* Not a compound SELECT */
  إذا( p->selFlags & SF_Aggregate ){
    أرجع 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->p_گروه_By==0 );              /* Has no GROUP BY clause */
  إذا( p->p_حد ) أرجع 0;              /* Has no LIMIT clause */
  إذا( p->pWhere ) أرجع 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  أكد( pSrc!=0 );
  إذا( pSrc->nSrc!=1 ) أرجع 0;          /* Single term in FROM clause */
  إذا( pSrc->a[0].fg.isSubquery) أرجع 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab)  );              /* FROM clause is not a view */
  إذا( IsVirtual(pTab) ) أرجع 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  أكد( pEList!=0 );
  /* All SELECT results must be columns. */
  لكل(i=0; i<pEList->n_عبارت; i++){
    عبارت *pRes = pEList->a[i].p_عبارت;
    إذا( pRes->op!=TK_ستونی ) أرجع 0;
    أكد( pRes->i_جدول==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  أرجع p;
}
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ساكن فراغ sqlite3_مجموعه_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  صحيح iCur,          /* Cursor for the index */
  صحيح regHasNull,    /* Register in which to store hasNull flag */
  صحيح e_مرتب_سازی_ترتیب     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  صحيح addr1;
  صحيح op;
  sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, regHasNull);
  إذا( e_مرتب_سازی_ترتیب==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }وإلا{
    op = OP_آخرین;
  }
  addr1 = sqlite3_Vdbe_جمع_کن_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_آخرین?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_اینجا(v, addr1);
}
#پایان_اگر


#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ساكن صحيح sqlite3_درون_Rhs_Is_Constant(Parse *pParse, عبارت *p_درون){
  عبارت *pLHS;
  صحيح res;
  أكد( !عبارت_Has_Property(p_درون, EP_xIsSelect) );
  pLHS = p_درون->p_چپ;
  p_درون->p_چپ = 0;
  res = sqlite3_عبارت_Is_Constant(pParse, p_درون);
  p_درون->p_چپ = pLHS;
  أرجع res;
}
#پایان_اگر

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
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
صحيح sqlite3_پیدا_کن_درون_شاخص(
  Parse *pParse,             /* Parsing context */
  عبارت *pX,                  /* The IN expression */
  u32 درون_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  صحيح *prRhsHasNull,         /* Register holding NULL status.  See notes */
  صحيح *aiMap,                /* Mapping from Index fields to RHS fields */
  صحيح *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  صحيح eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  صحيح iTab;                             /* Cursor of the RHS table */
  صحيح mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  أكد( pX->op==TK_درون );
  mustBeUnique = (درون_Flags & درون_شاخص_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  إذا( prRhsHasNull && عبارت_Use_XSelect(pX) ){
    صحيح i;
    عبارت_List *pEList = pX->x.pSelect->pEList;
    لكل(i=0; i<pEList->n_عبارت; i++){
      إذا( sqlite3_عبارت_Can_Be_Null(pEList->a[i].p_عبارت) ) اكسر;
    }
    إذا( i==pEList->n_عبارت ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  إذا( pParse->nErr==0 && (p = is_Candidate_For_درون_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    جدول *pTab;                           /* Table <table>. */
    صحيح iDb;                               /* Database idx for pTab */
    عبارت_List *pEList = p->pEList;
    صحيح n_عبارت = pEList->n_عبارت;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].p_عبارت!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_مقصد_شاخص(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_کد_Verify_Schema(pParse, iDb);
    sqlite3_جدول_قفل(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    إذا( n_عبارت==1 && pEList->a[0].p_عبارت->i_ستونی<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحيح iAddr = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_جدول(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = درون_شاخص_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_اینجا(v, iAddr);
    }وإلا{
      شاخص *pIdx;                         /* Iterator variable */
      صحيح affinity_ok = 1;
      صحيح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      لكل(i=0; i<n_عبارت && affinity_ok; i++){
        عبارت *pLhs = sqlite3_وکتور_فیلد_Subexpr(pX->p_چپ, i);
        صحيح i_ستون = pEList->a[i].p_عبارت->i_ستونی;
        حرف idxaff = sqlite3_جدول_ستونی_Affinity(pTab,i_ستون); /* RHS table */
        حرف cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_متن );
        بدل( cmpaff ){
          حالة SQLITE_AFF_BLOB:
            اكسر;
          حالة SQLITE_AFF_متن:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            أكد( idxaff==SQLITE_AFF_متن );
            اكسر;
          افتراضي:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      إذا( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        لكل(pIdx=pTab->p_شاخص; pIdx && eType==0; pIdx=pIdx->p_بعدی){
          Bitmask ستون_استفاده_شده;      /* Columns of the index used */
          Bitmask m_ستون;         /* Mask for the current column */
          إذا( pIdx->n_ستونی<n_عبارت ) استمر;
          إذا( pIdx->pPartIdxWhere!=0 ) استمر;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_ستونی==BMS-2 );
          testcase( pIdx->n_ستونی==BMS-1 );
          إذا( pIdx->n_ستونی>=BMS-1 ) استمر;
          إذا( mustBeUnique ){
            إذا( pIdx->n_کلید_ستون>n_عبارت
             ||(pIdx->n_ستونی>n_عبارت && !Is_Unique_شاخص(pIdx))
            ){
              استمر;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          ستون_استفاده_شده = 0;   /* Columns of index used so far */
          لكل(i=0; i<n_عبارت; i++){
            عبارت *pLhs = sqlite3_وکتور_فیلد_Subexpr(pX->p_چپ, i);
            عبارت *pRhs = pEList->a[i].p_عبارت;
            CollSeq *pReq = sqlite3_دودویی_Compare_Coll_Seq(pParse, pLhs, pRhs);
            صحيح j;
 
            لكل(j=0; j<n_عبارت; j++){
              إذا( pIdx->ai_ستونی[j]!=pRhs->i_ستونی ) استمر;
              أكد( pIdx->azColl[j] );
              إذا( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                استمر;
              }
              اكسر;
            }
            إذا( j==n_عبارت ) اكسر;
            m_ستون = MASKBIT(j);
            إذا( m_ستون & ستون_استفاده_شده ) اكسر; /* Each column used only once */
            ستون_استفاده_شده |= m_ستون;
            إذا( aiMap ) aiMap[i] = j;
          }
 
          أكد( n_عبارت>0 && n_عبارت<BMS );
          أكد( i==n_عبارت || ستون_استفاده_شده!=(MASKBIT(n_عبارت)-1) );
          إذا( ستون_استفاده_شده==(MASKBIT(n_عبارت)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحيح iAddr = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_جمع_کن_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( درون_شاخص_شاخص_DESC == درون_شاخص_شاخص_ASC+1 );
            eType = درون_شاخص_شاخص_ASC + pIdx->a_مرتب_سازی_ترتیب[0];
 
            إذا( prRhsHasNull ){
#اگر_تعریف_شده SQLITE_ENABLE_ستونی_استفاده_شده_ماسک
              u64 ماسک = (1<<n_عبارت)-1;
              sqlite3_Vdbe_جمع_کن_Op3(v, OP_Columns_استفاده_شده,
                                iTab, LOWER32(ماسک), UPPER32(ماسک));
#پایان_اگر
              *prRhsHasNull = ++pParse->nMem;
              إذا( n_عبارت==1 ){
                sqlite3_مجموعه_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_مرتب_سازی_ترتیب[0]);
              }
            }
            sqlite3_Vdbe_Jump_اینجا(v, iAddr);
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
  إذا( eType==0
   && (درون_Flags & درون_شاخص_NOOP_OK)
   && عبارت_Use_XList(pX)
   && (!sqlite3_درون_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_عبارت<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = درون_شاخص_NOOP;
  }

  إذا( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحيح rMayHaveNull = 0;
    صحيح bloomOk = (درون_Flags & درون_شاخص_MEMBERSHIP)!=0;
    eType = درون_شاخص_EPH;
    إذا( درون_Flags & درون_شاخص_LOOP ){
      pParse->nQueryLoop = 0;
    }وإلا إذا( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_درون );
    إذا( !bloomOk
     && عبارت_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_درون)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_کد_Rhs_از_درون(pParse, pX, iTab, bloomOk);
    إذا( rMayHaveNull ){
      sqlite3_مجموعه_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  إذا( aiMap && eType!=درون_شاخص_شاخص_ASC && eType!=درون_شاخص_شاخص_DESC ){
    صحيح i, n;
    n = sqlite3_عبارت_وکتور_اندازه(pX->p_چپ);
    لكل(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  أرجع eType;
}
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ساكن حرف *عبارت_INAffinity(Parse *pParse, ثابت عبارت *p_عبارت){
  عبارت *p_چپ = p_عبارت->p_چپ;
  صحيح nVal = sqlite3_عبارت_وکتور_اندازه(p_چپ);
  Select *pSelect = عبارت_Use_XSelect(p_عبارت) ? p_عبارت->x.pSelect : 0;
  حرف *zRet;

  أكد( p_عبارت->op==TK_درون );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  إذا( zRet ){
    صحيح i;
    لكل(i=0; i<nVal; i++){
      عبارت *pA = sqlite3_وکتور_فیلد_Subexpr(p_چپ, i);
      حرف a = sqlite3_عبارت_Affinity(pA);
      إذا( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_عبارت, a);
      }وإلا{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  أرجع zRet;
}
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
فراغ sqlite3SubselectError(Parse *pParse, صحيح nActual, صحيح nExpect){
  إذا( pParse->nErr==0 ){
    ثابت حرف *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_پیام(pParse, zFmt, nActual, nExpect);
  }
}
#پایان_اگر

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
فراغ sqlite3_وکتور_Error_پیام(Parse *pParse, عبارت *p_عبارت){
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
  إذا( عبارت_Use_XSelect(p_عبارت) ){
    sqlite3SubselectError(pParse, p_عبارت->x.pSelect->pEList->n_عبارت, 1);
  }وإلا
#پایان_اگر
  {
    sqlite3_Error_پیام(pParse, "row value misused");
  }
}

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ساكن صحيح پیدا_کن_Compatible_درون_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  عبارت *p_عبارت,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_انتها;
  SubrtnSig *pSig;
  Vdbe *v;

  إذا( pNewSig==0 ) أرجع 0;
  إذا( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) أرجع 0;
  أكد( p_عبارت->op==TK_درون );
  أكد( !عبارت_Use_YSub(p_عبارت) );
  أكد( عبارت_Use_XSelect(p_عبارت) );
  أكد( p_عبارت->x.pSelect!=0 );
  أكد( (p_عبارت->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_انتها = sqlite3_Vdbe_Get_آخرین_Op(v);
  لكل(; pOp<p_انتها; pOp++){
    إذا( pOp->p4type!=P4_SUBRTNSIG ) استمر;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    إذا( !pSig->bComplete ) استمر;
    إذا( pNewSig->selId!=pSig->selId ) استمر;
    إذا( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) استمر;
    p_عبارت->y.sub.iAddr = pSig->iAddr;
    p_عبارت->y.sub.regReturn = pSig->regReturn;
    p_عبارت->i_جدول = pSig->i_جدول;
    عبارت_مجموعه_Property(p_عبارت, EP_Subrtn);
    أرجع 1;
  }
  أرجع 0;
}
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
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
فراغ sqlite3_کد_Rhs_از_درون(
  Parse *pParse,          /* Parsing context */
  عبارت *p_عبارت,            /* The IN operator */
  صحيح iTab,               /* Use this cursor number */
  صحيح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحيح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحيح addr;                   /* Address of OP_OpenEphemeral instruction */
  عبارت *p_چپ;                /* the LHS of the IN operator */
  کلید_Info *p_کلید_Info = 0;      /* Key information */
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
  إذا( !عبارت_Has_Property(p_عبارت, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !عبارت_Use_XSelect(p_عبارت) || p_عبارت->x.pSelect!=0 );
    إذا( عبارت_Use_XSelect(p_عبارت) && (p_عبارت->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, حجم(pSig[0]));
      إذا( pSig ){
        pSig->selId = p_عبارت->x.pSelect->selId;
        pSig->zAff = عبارت_INAffinity(pParse, p_عبارت);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    إذا( عبارت_Has_Property(p_عبارت, EP_Subrtn) 
     || پیدا_کن_Compatible_درون_Rhs_Subrtn(pParse, p_عبارت, pSig)
    ){
      addrOnce = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
      إذا( عبارت_Use_XSelect(p_عبارت) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_عبارت->x.pSelect->selId));
      }
      أكد( عبارت_Use_YSub(p_عبارت) );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, p_عبارت->y.sub.regReturn,
                        p_عبارت->y.sub.iAddr);
      أكد( iTab!=p_عبارت->i_جدول );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenDup, iTab, p_عبارت->i_جدول);
      sqlite3_Vdbe_Jump_اینجا(v, addrOnce);
      إذا( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      أرجع;
    }

    /* Begin coding the subroutine */
    أكد( !عبارت_Use_YWin(p_عبارت) );
    عبارت_مجموعه_Property(p_عبارت, EP_Subrtn);
    أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
    p_عبارت->y.sub.regReturn = ++pParse->nMem;
    p_عبارت->y.sub.iAddr =
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_BeginSubrtn, 0, p_عبارت->y.sub.regReturn) + 1;
    إذا( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_عبارت->y.sub.iAddr;
      pSig->regReturn = p_عبارت->y.sub.regReturn;
      pSig->i_جدول = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_چپ = p_عبارت->p_چپ;
  nVal = sqlite3_عبارت_وکتور_اندازه(p_چپ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_عبارت->i_جدول = iTab;
  addr = sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenEphemeral, p_عبارت->i_جدول, nVal);
#اگر_تعریف_شده SQLITE_ENABLE_EXPLAIN_COMMENTS
  إذا( عبارت_Use_XSelect(p_عبارت) ){
    VdbeComment((v, "Result of SELECT %u", p_عبارت->x.pSelect->selId));
  }وإلا{
    VdbeComment((v, "RHS of IN operator"));
  }
#پایان_اگر
  p_کلید_Info = sqlite3_کلید_Info_Alloc(pParse->db, nVal, 1);
  أكد( p_کلید_Info!=0 || pParse->nErr );
  إذا( p_کلید_Info==0 ) أرجع;

  إذا( عبارت_Use_XSelect(p_عبارت) ){
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
    إذا( ALWAYS(pEList->n_عبارت==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحيح i;
      صحيح rc;
      صحيح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_مجموعه, iTab);
      dest.zAffSdst = عبارت_INAffinity(pParse, p_عبارت);
      pSelect->i_حد = 0;
      أكد( pEList!=0 );
      أكد( pEList->n_عبارت>0 );
      أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
      لكل(i=0; i<nVal; i++){
        عبارت *p = sqlite3_وکتور_فیلد_Subexpr(p_چپ, i);
        CollSeq *pColl;
        p_کلید_Info->aColl[i] = pColl = sqlite3_دودویی_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_عبارت
        );
        إذا( !sqlite3_Is_دودویی(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      إذا( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحيح regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (فراغ *)p_کلید_Info, P4_KEYINFO);
        p_کلید_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      إذا( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        إذا( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      إذا( rc ){
        sqlite3_کلید_Info_Unref(p_کلید_Info);
        أرجع;
      }
    }
  }وإلا إذا( ALWAYS(p_عبارت->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    حرف affinity;            /* Affinity of the LHS of the IN */
    صحيح i;
    عبارت_List *pList = p_عبارت->x.pList;
    بنية عبارت_List_قلم *p_قلم;
    صحيح r1, r2;
    affinity = sqlite3_عبارت_Affinity(p_چپ);
    إذا( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }وإلا إذا( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( p_کلید_Info!=0 );
    أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
    p_کلید_Info->aColl[0] = sqlite3_عبارت_Coll_Seq(pParse, p_عبارت->p_چپ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_موقت_Reg(pParse);
    r2 = sqlite3_Get_موقت_Reg(pParse);
    لكل(i=pList->n_عبارت, p_قلم=pList->a; i>0; i--, p_قلم++){
      عبارت *pE2 = p_قلم->p_عبارت;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      إذا( addrOnce && !sqlite3_عبارت_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_مقصد_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_مقصد_Noop(v, addrOnce);
        عبارت_Clear_Property(p_عبارت, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_عبارت_کد(pParse, pE2, r1);
      sqlite3_Vdbe_جمع_کن_Op4(v, OP_Make_رکورد, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Idx_درج, iTab, r2, r1, 1);
    }
    sqlite3_Release_موقت_Reg(pParse, r1);
    sqlite3_Release_موقت_Reg(pParse, r2);
  }
  إذا( pSig ) pSig->bComplete = 1;
  إذا( p_کلید_Info ){
    sqlite3VdbeChangeP4(v, addr, (فراغ *)p_کلید_Info, P4_KEYINFO);
  }
  إذا( addrOnce ){
    sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, iTab);
    sqlite3_Vdbe_Jump_اینجا(v, addrOnce);
    /* Subroutine return */
    أكد( عبارت_Use_YSub(p_عبارت) );
    أكد( sqlite3VdbeGetOp(v,p_عبارت->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Return, p_عبارت->y.sub.regReturn,
                      p_عبارت->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_موقت_Reg_Cache(pParse);
  }
}
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

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
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
صحيح sqlite3_کد_Subselect(Parse *pParse, عبارت *p_عبارت){
  صحيح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحيح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحيح nReg;                   /* Registers to allocate */
  عبارت *p_حد;               /* New limit expression */
#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;            /* Address of OP_Explain instruction */
#پایان_اگر

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  إذا( pParse->nErr ) أرجع 0;
  testcase( p_عبارت->op==TK_EXISTS );
  testcase( p_عبارت->op==TK_SELECT );
  أكد( p_عبارت->op==TK_EXISTS || p_عبارت->op==TK_SELECT );
  أكد( عبارت_Use_XSelect(p_عبارت) );
  pSel = p_عبارت->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  إذا( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( عبارت_Use_YSub(p_عبارت) );
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, p_عبارت->y.sub.regReturn,
                      p_عبارت->y.sub.iAddr);
    أرجع p_عبارت->i_جدول;
  }

  /* Begin coding the subroutine */
  أكد( !عبارت_Use_YWin(p_عبارت) );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Reduced|EP_TokenOnly) );
  عبارت_مجموعه_Property(p_عبارت, EP_Subrtn);
  p_عبارت->y.sub.regReturn = ++pParse->nMem;
  p_عبارت->y.sub.iAddr =
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_BeginSubrtn, 0, p_عبارت->y.sub.regReturn) + 1;

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
  إذا( !عبارت_Has_Property(p_عبارت, EP_VarSelect) ){
    addrOnce = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
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
  sqlite3_Vdbe_پیمایش_وضعیت_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_عبارت->op==TK_SELECT ? pSel->pEList->n_عبارت : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  إذا( p_عبارت->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    إذا( (pSel->selFlags&SF_Distinct) && pSel->p_حد && pSel->p_حد->p_راست ){
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
    }وإلا{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }وإلا{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  إذا( pSel->p_حد ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    عبارت *p_چپ = pSel->p_حد->p_چپ;
    إذا( عبارت_Has_Property(p_چپ, EP_Int_مقدار)==0
     || (p_چپ->u.i_مقدار!=1 && p_چپ->u.i_مقدار!=0)
    ){
      sqlite3 *db = pParse->db;
      p_حد = sqlite3_عبارت_Int32(db, 0);
      إذا( p_حد ){
        p_حد->aff_عبارت = SQLITE_AFF_NUMERIC;
        p_حد = sqlite3PExpr(pParse, TK_NE,
            sqlite3_عبارت_Dup(db, p_چپ, 0), p_حد);
      }
      sqlite3_عبارت_Deferred_Delete(pParse, p_چپ);
      pSel->p_حد->p_چپ = p_حد;
    }
  }وإلا{
    /* If there is no pre-existing limit add a limit of 1 */
    p_حد = sqlite3_عبارت_Int32(pParse->db, 1);
    pSel->p_حد = sqlite3PExpr(pParse, TK_حد, p_حد, 0);
  }
  pSel->i_حد = 0;
  إذا( sqlite3Select(pParse, pSel, &dest) ){
    p_عبارت->op2 = p_عبارت->op;
    p_عبارت->op = TK_ERROR;
    أرجع 0;
  }
  p_عبارت->i_جدول = rReg = dest.iSDParm;
  عبارت_مجموعه_VVAProperty(p_عبارت, EP_NoReduce);
  إذا( addrOnce ){
    sqlite3_Vdbe_Jump_اینجا(v, addrOnce);
  }
  sqlite3_Vdbe_پیمایش_وضعیت_Range(v, addrExplain, addrExplain, -1);

  إذا( dest.eDest==SRT_Mem 
   && sqlite3_عبارت_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_عبارت) 
  ){
    sqlite3_Vdbe_جمع_کن_Op1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  أكد( عبارت_Use_YSub(p_عبارت) );
  أكد( sqlite3VdbeGetOp(v,p_عبارت->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3_Vdbe_جمع_کن_Op3(v, OP_Return, p_عبارت->y.sub.regReturn,
                    p_عبارت->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_موقت_Reg_Cache(pParse);
  أرجع rReg;
}
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحيح sqlite3_عبارت_Check_درون(Parse *pParse, عبارت *p_درون){
  صحيح n_وکتور = sqlite3_عبارت_وکتور_اندازه(p_درون->p_چپ);
  إذا( عبارت_Use_XSelect(p_درون) && !pParse->db->mallocFailed ){
    إذا( n_وکتور!=p_درون->x.pSelect->pEList->n_عبارت ){
      sqlite3SubselectError(pParse, p_درون->x.pSelect->pEList->n_عبارت, n_وکتور);
      أرجع 1;
    }
  }وإلا إذا( n_وکتور!=1 ){
    sqlite3_وکتور_Error_پیام(pParse, p_درون->p_چپ);
    أرجع 1;
  }
  أرجع 0;
}
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
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
ساكن فراغ sqlite3_عبارت_کد_درون(
  Parse *pParse,        /* Parsing and code generating context */
  عبارت *p_عبارت,          /* The IN expression */
  صحيح destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  صحيح destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  صحيح rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  صحيح eType;            /* Type of the RHS */
  صحيح rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  صحيح *aiMap = 0;       /* Map from vector field to index column */
  حرف *zAff = 0;       /* Affinity string for comparisons */
  صحيح n_وکتور;          /* Size of vectors for this IN operator */
  صحيح iDummy;           /* Dummy parameter to exprCodeVector() */
  عبارت *p_چپ;          /* The LHS of the IN operator */
  صحيح i;                /* loop counter */
  صحيح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحيح destStep6 = 0;    /* Start of code for Step 6 */
  صحيح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحيح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحيح addr_بالاترین;          /* Top of the step-6 loop */
  صحيح iTab = 0;         /* Index to use */
  u8 ok_Const_ضریب = pParse->ok_Const_ضریب;

  أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  p_چپ = p_عبارت->p_چپ;
  إذا( sqlite3_عبارت_Check_درون(pParse, p_عبارت) ) أرجع;
  zAff = عبارت_INAffinity(pParse, p_عبارت);
  n_وکتور = sqlite3_عبارت_وکتور_اندازه(p_عبارت->p_چپ);
  aiMap = (صحيح*)sqlite3DbMallocZero(pParse->db, n_وکتور*حجم(صحيح));
  إذا( pParse->db->mallocFailed ) اذهب sqlite3_عبارت_کد_درون_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_پیدا_کن_درون_شاخص(pParse, p_عبارت,
                             درون_شاخص_MEMBERSHIP | درون_شاخص_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  أكد( pParse->nErr || n_وکتور==1 || eType==درون_شاخص_EPH
       || eType==درون_شاخص_شاخص_ASC || eType==درون_شاخص_شاخص_DESC
  );
#اگر_تعریف_شده SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  لكل(i=0; i<n_وکتور; i++){
    صحيح j, cnt;
    لكل(cnt=j=0; j<n_وکتور; j++) إذا( aiMap[j]==i ) cnt++;
    أكد( cnt==1 );
  }
#پایان_اگر

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
  أكد( pParse->ok_Const_ضریب==ok_Const_ضریب );
  pParse->ok_Const_ضریب = 0;
  rLhs = عبارت_کد_وکتور(pParse, p_چپ, &iDummy);
  pParse->ok_Const_ضریب = ok_Const_ضریب;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  إذا( eType==درون_شاخص_NOOP ){
    عبارت_List *pList;
    CollSeq *pColl;
    صحيح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحيح r2, reg_مقصد_Free;
    صحيح regCkNull = 0;
    صحيح ii;
    أكد( n_وکتور==1 );
    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    pColl = sqlite3_عبارت_Coll_Seq(pParse, p_عبارت->p_چپ);
    إذا( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_موقت_Reg(pParse);
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_بیت_And, rLhs, rLhs, regCkNull);
    }
    لكل(ii=0; ii<pList->n_عبارت; ii++){
      r2 = sqlite3_عبارت_کد_موقت(pParse, pList->a[ii].p_عبارت, &reg_مقصد_Free);
      إذا( regCkNull && sqlite3_عبارت_Can_Be_Null(pList->a[ii].p_عبارت) ){
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_بیت_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_موقت_Reg(pParse, reg_مقصد_Free);
      إذا( ii<pList->n_عبارت-1 || destIfNull!=destIfFalse ){
        صحيح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_جمع_کن_Op4(v, op, rLhs, labelOk, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_عبارت-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_عبارت-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_عبارت-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_عبارت-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }وإلا{
        صحيح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3_Vdbe_جمع_کن_Op4(v, op, rLhs, destIfFalse, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    إذا( regCkNull ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_موقت_Reg(pParse, regCkNull);
    اذهب sqlite3_عبارت_کد_درون_پایانیافته;
  }

  إذا( eType!=درون_شاخص_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    إذا( n_وکتور==1 ){
      حرف aff = zAff[0];
      إذا( aff>=SQLITE_AFF_متن && aff!=sqlite3_عبارت_Affinity(p_چپ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحيح rTmp = sqlite3_Get_موقت_Reg(pParse);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_جمع_کن_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }وإلا{
      sqlite3_Vdbe_جمع_کن_Op4(v, OP_Affinity, rLhs, n_وکتور, 0, zAff, n_وکتور);
    }

    لكل(i=0; i<n_وکتور && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    إذا( i!=n_وکتور ){
      /* Need to reorder the LHS fields according to aiMap */
      صحيح rLhsOrig = rLhs;
      rLhs = sqlite3_Get_موقت_Range(pParse, n_وکتور);
      لكل(i=0; i<n_وکتور; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  إذا( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }وإلا{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  لكل(i=0; i<n_وکتور; i++){
    عبارت *p = sqlite3_وکتور_فیلد_Subexpr(p_عبارت->p_چپ, i);
    إذا( pParse->nErr ) اذهب sqlite3_عبارت_کد_درون_oom_error;
    إذا( sqlite3_عبارت_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  إذا( eType==درون_شاخص_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( n_وکتور==1 );
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Goto);  /* Return True */
  }وإلا{
    إذا( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      إذا( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, p_عبارت->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        إذا( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_وکتور); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Not_یافت, iTab, destIfFalse,
                           rLhs, n_وکتور); VdbeCoverage(v);
      اذهب sqlite3_عبارت_کد_درون_پایانیافته;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_یافت, iTab, 0,
                                      rLhs, n_وکتور); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  إذا( rRhsHasNull && n_وکتور==1 ){
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  إذا( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  إذا( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  إذا( eType==درون_شاخص_شاخص_DESC ){
    addr_بالاترین = sqlite3_Vdbe_جمع_کن_Op2(v, OP_آخرین, iTab, destIfFalse);
  }وإلا{
    testcase( eType==درون_شاخص_EPH );
    testcase( eType==درون_شاخص_ROWID );
    addr_بالاترین = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  إذا( n_وکتور>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }وإلا{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  لكل(i=0; i<n_وکتور; i++){
    عبارت *p;
    CollSeq *pColl;
    صحيح r3 = sqlite3_Get_موقت_Reg(pParse);
    p = sqlite3_وکتور_فیلد_Subexpr(p_چپ, i);
    إذا( عبارت_Use_XSelect(p_عبارت) ){
      عبارت *pRhs = p_عبارت->x.pSelect->pEList->a[i].p_عبارت;
      pColl = sqlite3_دودویی_Compare_Coll_Seq(pParse, p, pRhs);
    }وإلا{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_عبارت_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, iTab, aiMap[i], r3);
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (فراغ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_موقت_Reg(pParse, r3);
  }
  sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, destIfNull);
  إذا( n_وکتور>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_بعدی, iTab, addr_بالاترین+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_اینجا(v, addrTruthOp);

sqlite3_عبارت_کد_درون_پایانیافته:
  VdbeComment((v, "end IN expr"));
sqlite3_عبارت_کد_درون_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ساكن فراغ کد_Real(Vdbe *v, ثابت حرف *z, صحيح negateFlag, صحيح iMem){
  إذا( ALWAYS(z!=0) ){
    مزدوج مقدار;
    sqlite3AtoF(z, &مقدار);
    أكد( !sqlite3IsNaN(مقدار) ); /* The new AtoF never returns NaN */
    إذا( negateFlag ) مقدار = -مقدار;
    sqlite3_Vdbe_جمع_کن_Double(v, iMem, مقدار);
  }
}
#پایان_اگر


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ساكن فراغ کد_Integer(Parse *pParse, عبارت *p_عبارت, صحيح negFlag, صحيح iMem){
  Vdbe *v = pParse->pVdbe;
  إذا( p_عبارت->flags & EP_Int_مقدار ){
    صحيح i = p_عبارت->u.i_مقدار;
    أكد( i>=0 );
    إذا( negFlag ) i = -i;
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, i, iMem);
  }وإلا{
    صحيح c;
    i64 مقدار;
    ثابت حرف *z = p_عبارت->u.zToken;
    أكد( z!=0 );
    c = sqlite3_Dec_Or_Hex_مقصد_I64(z, &مقدار);
    إذا( (c==3 && !negFlag) || (c==2) || (negFlag && مقدار==کوچکترین_INT64)){
#اگر_تعریف_شده SQLITE_OMIT_FLOATING_نقطه
      sqlite3_Error_پیام(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_عبارت);
#وإلا
#اگر_تعریف_نشده SQLITE_OMIT_HEX_INTEGER
      إذا( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_پیام(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_عبارت);
      }وإلا
#پایان_اگر
      {
        کد_Real(v, z, negFlag, iMem);
      }
#پایان_اگر
    }وإلا{
      إذا( negFlag ){ مقدار = c==3 ? کوچکترین_INT64 : -مقدار; }
      sqlite3_Vdbe_جمع_کن_Int64(v, iMem, مقدار);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
فراغ sqlite3_عبارت_کد_بارگیری_شاخص_ستونی(
  Parse *pParse,  /* The parsing context */
  شاخص *pIdx,    /* The index whose column is to be loaded */
  صحيح iTabCur,    /* Cursor pointing to a table row */
  صحيح i_Idx_ستون,    /* The column of the index to be loaded */
  صحيح reg_بیرون      /* Store the index column value in this register */
){
  i16 i_Tab_ستون = pIdx->ai_ستونی[i_Idx_ستون];
  إذا( i_Tab_ستون==XN_عبارت ){
    أكد( pIdx->a_ستون_عبارت );
    أكد( pIdx->a_ستون_عبارت->n_عبارت>i_Idx_ستون );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_عبارت_کد_Copy(pParse, pIdx->a_ستون_عبارت->a[i_Idx_ستون].p_عبارت, reg_بیرون);
    pParse->iSelfTab = 0;
  }وإلا{
    sqlite3_عبارت_کد_Get_ستونی_از_جدول(pParse->pVdbe, pIdx->p_جدول, iTabCur,
                                    i_Tab_ستون, reg_بیرون);
  }
}

#اگر_تعریف_نشده SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
فراغ sqlite3_عبارت_کد_Generated_ستونی(
  Parse *pParse,     /* Parsing context */
  جدول *pTab,       /* Table containing the generated column */
  ستونی *p_ستون,      /* The generated column */
  صحيح reg_بیرون         /* Put the result in this register */
){
  صحيح iAddr;
  Vdbe *v = pParse->pVdbe;
  صحيح nErr = pParse->nErr;
  أكد( v!=0 );
  أكد( pParse->iSelfTab!=0 );
  إذا( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_جمع_کن_Op3(v, OP_If_Null_ردیف, pParse->iSelfTab-1, 0, reg_بیرون);
  }وإلا{
    iAddr = 0;
  }
  sqlite3_عبارت_کد_Copy(pParse, sqlite3_ستونی_عبارت(pTab,p_ستون), reg_بیرون);
  إذا( (p_ستون->ستون_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحيح p3 = 2+(صحيح)(p_ستون - pTab->a_ستون);
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_TypeCheck, reg_بیرون, 1, p3, (حرف*)pTab, P4_جدول);
  }وإلا إذا( p_ستون->affinity>=SQLITE_AFF_متن ){
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_Affinity, reg_بیرون, 1, 0, &p_ستون->affinity, 1);
  }
  إذا( iAddr ) sqlite3_Vdbe_Jump_اینجا(v, iAddr);
  إذا( pParse->nErr>nErr ) pParse->db->err_Byte_جابه_جایی = -1;
}
#پایان_اگر /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
فراغ sqlite3_عبارت_کد_Get_ستونی_از_جدول(
  Vdbe *v,        /* Parsing context */
  جدول *pTab,    /* The table containing the value */
  صحيح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحيح i_ستون,       /* Index of the column to extract */
  صحيح reg_بیرون      /* Extract the value into this register */
){
  ستونی *p_ستون;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( i_ستون!=XN_عبارت );
  إذا( i_ستون<0 || i_ستون==pTab->iPKey ){
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rowid, iTabCur, reg_بیرون);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }وإلا{
    صحيح x;
    إذا( IsVirtual(pTab) ){
      x = i_ستون;
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_VColumn, iTabCur, x, reg_بیرون);
      أرجع;
#اگر_تعریف_نشده SQLITE_OMIT_GENERATED_COLUMNS
    }وإلا إذا( (p_ستون = &pTab->a_ستون[i_ستون])->ستون_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      إذا( p_ستون->ستون_Flags & COLFLAG_BUSY ){
        sqlite3_Error_پیام(pParse, "generated column loop on \"%s\"",
                        p_ستون->zCnName);
      }وإلا{
        صحيح savedSelfTab = pParse->iSelfTab;
        p_ستون->ستون_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_عبارت_کد_Generated_ستونی(pParse, pTab, p_ستون, reg_بیرون);
        pParse->iSelfTab = savedSelfTab;
        p_ستون->ستون_Flags &= ~COLFLAG_BUSY;
      }
      أرجع;
#پایان_اگر
    }وإلا إذا( !HasRowid(pTab) ){
      testcase( i_ستون!=sqlite3_جدول_ستونی_مقصد_Storage(pTab, i_ستون) );
      x = sqlite3_جدول_ستونی_مقصد_شاخص(sqlite3_Primary_کلید_شاخص(pTab), i_ستون);
    }وإلا{
      x = sqlite3_جدول_ستونی_مقصد_Storage(pTab,i_ستون);
      testcase( x!=i_ستون );
    }
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, iTabCur, x, reg_بیرون);
    sqlite3_ستونی_Default(v, pTab, i_ستون, reg_بیرون);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
صحيح sqlite3_عبارت_کد_Get_ستونی(
  Parse *pParse,   /* Parsing and code generating context */
  جدول *pTab,     /* Description of the table we are reading from */
  صحيح i_ستونی,     /* Index of the table column */
  صحيح i_جدول,      /* The cursor pointing to the table */
  صحيح iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  أكد( pParse->pVdbe!=0 );
  أكد( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  أكد( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_عبارت_کد_Get_ستونی_از_جدول(pParse->pVdbe, pTab, i_جدول, i_ستونی, iReg);
  إذا( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_آخرین_Op(pParse->pVdbe);
    إذا( pOp->opcode==OP_ستونی ) pOp->p5 = p5;
    إذا( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  أرجع iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
فراغ sqlite3_عبارت_کد_Move(Parse *pParse, صحيح iFrom, صحيح i_مقصد, صحيح nReg){
  sqlite3_Vdbe_جمع_کن_Op3(pParse->pVdbe, OP_Move, iFrom, i_مقصد, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
فراغ sqlite3_عبارت_مقصد_ثبات(عبارت *p_عبارت, صحيح iReg){
  عبارت *p = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  إذا( NEVER(p==0) ) أرجع;
  إذا( p->op==TK_ثبات ){
    أكد( p->i_جدول==iReg );
  }وإلا{
    p->op2 = p->op;
    p->op = TK_ثبات;
    p->i_جدول = iReg;
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
ساكن صحيح عبارت_کد_وکتور(Parse *pParse, عبارت *p, صحيح *piFreeable){
  صحيح i_نتیجه;
  صحيح n_نتیجه = sqlite3_عبارت_وکتور_اندازه(p);
  إذا( n_نتیجه==1 ){
    i_نتیجه = sqlite3_عبارت_کد_موقت(pParse, p, piFreeable);
  }وإلا{
    *piFreeable = 0;
    إذا( p->op==TK_SELECT ){
#إذا SQLITE_OMIT_SUBQUERY
      i_نتیجه = 0;
#وإلا
      i_نتیجه = sqlite3_کد_Subselect(pParse, p);
#پایان_اگر
    }وإلا{
      صحيح i;
      i_نتیجه = pParse->nMem+1;
      pParse->nMem += n_نتیجه;
      أكد( عبارت_Use_XList(p) );
      لكل(i=0; i<n_نتیجه; i++){
        sqlite3_عبارت_کد_Factorable(pParse, p->x.pList->a[i].p_عبارت, i+i_نتیجه);
      }
    }
  }
  أرجع i_نتیجه;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ساكن فراغ مجموعه_Do_Not_ادغام_Flag_On_Copy(Vdbe *v){
  إذا( sqlite3_Vdbe_Get_آخرین_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ساكن صحيح عبارت_کد_Inline_Function(
  Parse *pParse,        /* Parsing context */
  عبارت_List *pFarg,      /* List of function arguments */
  صحيح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحيح هدف            /* Store function result in this register */
){
  صحيح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->n_عبارت;
  أكد( nFarg>0 );  /* All in-line functions have at least one argument */
  بدل( iFuncId ){
    حالة INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      صحيح انتها_Coalesce = sqlite3VdbeMakeLabel(pParse);
      صحيح i;
      أكد( nFarg>=2 );
      sqlite3_عبارت_کد(pParse, pFarg->a[0].p_عبارت, هدف);
      لكل(i=1; i<nFarg; i++){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_NotNull, هدف, انتها_Coalesce);
        VdbeCoverage(v);
        sqlite3_عبارت_کد(pParse, pFarg->a[i].p_عبارت, هدف);
      }
      مجموعه_Do_Not_ادغام_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, انتها_Coalesce);
      اكسر;
    }
    حالة INLINEFUNC_iif: {
      عبارت case_عبارت;
      memset(&case_عبارت, 0, حجم(case_عبارت));
      case_عبارت.op = TK_CASE;
      case_عبارت.x.pList = pFarg;
      أرجع sqlite3_عبارت_کد_هدف(pParse, &case_عبارت, هدف);
    }
#اگر_تعریف_شده SQLITE_ENABLE_جابه_جایی_SQL_FUNC
    حالة INLINEFUNC_sqlite_جابه_جایی: {
      عبارت *pArg = pFarg->a[0].p_عبارت;
      إذا( pArg->op==TK_ستونی && pArg->i_جدول>=0 ){
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_جابه_جایی, pArg->i_جدول, pArg->i_ستونی, هدف);
      }وإلا{
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
      }
      اكسر;
    }
#پایان_اگر
    افتراضي: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      أكد( nFarg==1 || nFarg==2 );
      هدف = sqlite3_عبارت_کد_هدف(pParse, pFarg->a[0].p_عبارت, هدف);
      اكسر;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#إذا !defined(SQLITE_UNTESTABLE)
    حالة INLINEFUNC_عبارت_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer,
         sqlite3_عبارت_Compare(0,pFarg->a[0].p_عبارت, pFarg->a[1].p_عبارت,-1),
         هدف);
      اكسر;
    }

    حالة INLINEFUNC_عبارت_نتیجه_میدهد_عبارت: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer,
         sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse,pFarg->a[0].p_عبارت, pFarg->a[1].p_عبارت,-1),
         هدف);
      اكسر;
    }

    حالة INLINEFUNC_نتیجه_میدهد_nonnull_ردیف: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      عبارت *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].p_عبارت;
      إذا( pA1->op==TK_ستونی ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer,
           sqlite3_عبارت_نتیجه_میدهد_Non_Null_ردیف(pFarg->a[0].p_عبارت,pA1->i_جدول,1),
           هدف);
      }وإلا{
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
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
      aff = sqlite3_عبارت_Affinity(pFarg->a[0].p_عبارت);
      أكد( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3_Vdbe_بارگیری_String(v, هدف,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      اكسر;
    }
#پایان_اگر /* !defined(SQLITE_UNTESTABLE) */
  }
  أرجع هدف;
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
ساكن صحيح عبارت_گره_Can_Return_Subtype(Walker *pWalker, عبارت *p_عبارت){
  صحيح n;
  FuncDef *pDef;
  sqlite3 *db;
  إذا( p_عبارت->op==TK_CASE || p_عبارت->op==TK_UPLUS 
   || p_عبارت->op==TK_COLLATE || p_عبارت->op==TK_CAST 
  ){
    أرجع WRC_Continue;
  }
  إذا( p_عبارت->op!=TK_FUNCTION && p_عبارت->op!=TK_AGG_FUNCTION ){
    أرجع WRC_Prune;
  }
  أكد( عبارت_Use_XList(p_عبارت) );
  db = pWalker->pParse->db;
  n = p_عبارت->x.pList ? p_عبارت->x.pList->n_عبارت : 0;
  pDef = sqlite3_پیدا_کن_Function(db, p_عبارت->u.zToken, n, ENC(db), 0);
#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  إذا( pDef==0 ) أرجع WRC_Continue;
#وإلا
  أكد( pDef!=0 );
#پایان_اگر
  إذا( (pDef->funcFlags & SQLITE_نتیجه_SUBTYPE)!=0 ){
    pWalker->e_کد = 1;
    أرجع WRC_Abort;
  }
  أرجع WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
صحيح sqlite3_عبارت_Can_Return_Subtype(Parse *pParse, عبارت *p_عبارت){
  Walker w;
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_عبارت_Callback = عبارت_گره_Can_Return_Subtype;
  sqlite3_Walk_عبارت(&w, p_عبارت);
  أرجع w.e_کد;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ساكن SQLITE_NOINLINE صحيح sqlite3_نمایهای_عبارت_Lookup(
  Parse *pParse,   /* The parsing context */
  عبارت *p_عبارت,     /* The expression to potentially bypass */
  صحيح هدف       /* Where to store the result of the expression */
){
  نمایهای_عبارت *p;
  Vdbe *v;
  لكل(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 عبارت_Aff;
    صحيح i_داده_Cur = p->i_داده_Cur;
    إذا( i_داده_Cur<0 ) استمر;
    إذا( pParse->iSelfTab ){
      إذا( p->i_داده_Cur!=pParse->iSelfTab-1 ) استمر;
      i_داده_Cur = -1;
    }
    إذا( sqlite3_عبارت_Compare(0, p_عبارت, p->p_عبارت, i_داده_Cur)!=0 ) استمر;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    عبارت_Aff = sqlite3_عبارت_Affinity(p_عبارت);
    إذا( (عبارت_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (عبارت_Aff==SQLITE_AFF_متن && p->aff!=SQLITE_AFF_متن)
     || (عبارت_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      استمر;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    إذا( عبارت_Has_Property(p_عبارت, EP_SubtArg)
     && sqlite3_عبارت_Can_Return_Subtype(pParse, p_عبارت) 
    ){
      استمر;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    إذا( p->b_Maybe_Null_ردیف || sqlite3_عبارت_Is_Constant(pParse, p_عبارت) ){
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
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_If_Null_ردیف, p->iIdxCur, addr+3, هدف);
      VdbeCoverage(v);
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, p->iIdxCur, p->i_Idx_ستون, هدف);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ستون));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_عبارت_کد(pParse, p_عبارت, هدف);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_اینجا(v, addr+2);
    }وإلا{
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, p->iIdxCur, p->i_Idx_ستون, هدف);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ستون));
    }
    أرجع هدف;
  }
  أرجع -1;  /* Not found */
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
ساكن صحيح عبارت_Partidx_عبارت_Lookup(Parse *pParse, عبارت *p_عبارت, صحيح i_هدف){
  نمایهای_عبارت *p;
  لكل(p=pParse->p_Idx_Part_عبارت; p; p=p->pIENext){
    إذا( p_عبارت->i_ستونی==p->i_Idx_ستون && p_عبارت->i_جدول==p->i_داده_Cur ){
      Vdbe *v = pParse->pVdbe;
      صحيح addr = 0;
      صحيح ret;

      إذا( p->b_Maybe_Null_ردیف ){
        addr = sqlite3_Vdbe_جمع_کن_Op1(v, OP_If_Null_ردیف, p->iIdxCur);
      }
      ret = sqlite3_عبارت_کد_هدف(pParse, p->p_عبارت, i_هدف);
      sqlite3_Vdbe_جمع_کن_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت حرف*)&p->aff, 1);
      إذا( sqlite3_عبارت_Can_Return_Subtype(pParse, p->p_عبارت) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_جمع_کن_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      إذا( addr ){
        sqlite3_Vdbe_Jump_اینجا(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      أرجع ret;
    }
  }
  أرجع 0;
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
ساكن SQLITE_NOINLINE صحيح عبارت_کد_هدف_And_Or(
  Parse *pParse,     /* Parsing context */
  عبارت *p_عبارت,       /* AND or OR expression to be coded */
  صحيح هدف,        /* Put result in this register, guaranteed */
  صحيح *pTmpReg       /* Write a temporary register here */
){
  صحيح op;            /* The opcode.  TK_AND or TK_OR */
  صحيح skipOp;        /* Opcode for the branch that skips one operand */
  صحيح addrSkip;      /* Branch instruction that skips one of the operands */
  صحيح regSS = 0;     /* Register holding computed operand when other omitted */
  صحيح r1, r2;        /* Registers for left and right operands, respectively */
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
  إذا( pAlt!=p_عبارت ){
    r1 = sqlite3_عبارت_کد_هدف(pParse, pAlt, هدف);
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_And, r1, r1, هدف);
    أرجع هدف;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  إذا( عبارت_Eval_Rhs_اولی(p_عبارت) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_عبارت_کد_هدف(pParse, p_عبارت->p_راست, هدف);
    addrSkip = sqlite3_Vdbe_جمع_کن_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, pTmpReg);
  }وإلا{
    /* Compute the left operand first */
    r1 = sqlite3_عبارت_کد_هدف(pParse, p_عبارت->p_چپ, هدف);
    إذا( عبارت_Has_Property(p_عبارت->p_راست, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_جمع_کن_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }وإلا{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, pTmpReg);
  }
  sqlite3_Vdbe_جمع_کن_Op3(v, op, r2, r1, هدف);
  testcase( (*pTmpReg)==0 );
  إذا( addrSkip ){
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_اینجا(v, addrSkip);
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Or, regSS, regSS, هدف);
    VdbeComment((v, "short-circut value"));
  }
  أرجع هدف;
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
صحيح sqlite3_عبارت_کد_هدف(Parse *pParse, عبارت *p_عبارت, صحيح هدف){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحيح op;                   /* The opcode being coded */
  صحيح درون_Reg = هدف;       /* Results stored in register inReg */
  صحيح regFree1 = 0;         /* If non-zero free this temporary register */
  صحيح regFree2 = 0;         /* If non-zero free this temporary register */
  صحيح r1, r2;               /* Various register numbers */
  عبارت موقت_X;               /* Temporary expression node */
  صحيح p5 = 0;

  أكد( هدف>0 && هدف<=pParse->nMem );
  أكد( v!=0 );

عبارت_کد_doover:
  إذا( p_عبارت==0 ){
    op = TK_NULL;
  }وإلا إذا( pParse->pIdxEpr!=0
   && !عبارت_Has_Property(p_عبارت, EP_برگ)
   && (r1 = sqlite3_نمایهای_عبارت_Lookup(pParse, p_عبارت, هدف))>=0
  ){
    أرجع r1;
  }وإلا{
    أكد( !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
    op = p_عبارت->op;
  }
  أكد( op!=TK_ترتیب );
  بدل( op ){
    حالة TK_AGG_ستونی: {
      AggInfo *pAggInfo = p_عبارت->pAggInfo;
      بنية Agg_Info_ستون *p_ستون;
      أكد( pAggInfo!=0 );
      أكد( p_عبارت->iAgg>=0 );
      إذا( p_عبارت->iAgg>=pAggInfo->n_ستونی ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
#اگر_تعریف_شده SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_NotNull, هدف, 1, 20230325);
        Vdbe_Coverage_Never_گرفته(v);
#پایان_اگر
        اكسر;
      }
      p_ستون = &pAggInfo->a_ستون[p_عبارت->iAgg];
      إذا( !pAggInfo->مستقیم_نما ){
        أرجع Agg_Info_ستونی_Reg(pAggInfo, p_عبارت->iAgg);
      }وإلا إذا( pAggInfo->use_ترتیبدهی_Idx ){
        جدول *pTab = p_ستون->pTab;
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, pAggInfo->ترتیبدهی_Idx_PTab,
                              p_ستون->i_Sorter_ستونی, هدف);
        إذا( pTab==0 ){
          /* No comment added */
        }وإلا إذا( p_ستون->i_ستونی<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }وإلا{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_ستون[p_ستون->i_ستونی].zCnName));
          إذا( pTab->a_ستون[p_ستون->i_ستونی].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جمع_کن_Op1(v, OP_RealAffinity, هدف);
          }
        }
        أرجع هدف;
      }وإلا إذا( p_عبارت->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, p_عبارت->i_جدول, p_عبارت->i_ستونی, هدف);
        أرجع هدف;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالة TK_ستونی: {
      صحيح iTab = p_عبارت->i_جدول;
      صحيح iReg;
      إذا( عبارت_Has_Property(p_عبارت, EP_Fixed_ستون) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحيح aff;
        iReg = sqlite3_عبارت_کد_هدف(pParse, p_عبارت->p_چپ,هدف);
        أكد( عبارت_Use_YTab(p_عبارت) );
        أكد( p_عبارت->y.pTab!=0 );
        aff = sqlite3_جدول_ستونی_Affinity(p_عبارت->y.pTab, p_عبارت->i_ستونی);
        إذا( aff>SQLITE_AFF_BLOB ){
          ساكن ثابت حرف zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_متن=='B' );
          sqlite3_Vdbe_جمع_کن_Op4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        أرجع iReg;
      }
      إذا( iTab<0 ){
        إذا( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          ستونی *p_ستون;
          جدول *pTab;
          صحيح iSrc;
          صحيح i_ستون = p_عبارت->i_ستونی;
          أكد( عبارت_Use_YTab(p_عبارت) );
          pTab = p_عبارت->y.pTab;
          أكد( pTab!=0 );
          أكد( i_ستون>=XN_ROWID );
          أكد( i_ستون<pTab->n_ستون );
          إذا( i_ستون<0 ){
            أرجع -1-pParse->iSelfTab;
          }
          p_ستون = pTab->a_ستون + i_ستون;
          testcase( i_ستون!=sqlite3_جدول_ستونی_مقصد_Storage(pTab,i_ستون) );
          iSrc = sqlite3_جدول_ستونی_مقصد_Storage(pTab, i_ستون) - pParse->iSelfTab;
#اگر_تعریف_نشده SQLITE_OMIT_GENERATED_COLUMNS
          إذا( p_ستون->ستون_Flags & COLFLAG_GENERATED ){
            إذا( p_ستون->ستون_Flags & COLFLAG_BUSY ){
              sqlite3_Error_پیام(pParse, "generated column loop on \"%s\"",
                              p_ستون->zCnName);
              أرجع 0;
            }
            p_ستون->ستون_Flags |= COLFLAG_BUSY;
            إذا( p_ستون->ستون_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_عبارت_کد_Generated_ستونی(pParse, pTab, p_ستون, iSrc);
            }
            p_ستون->ستون_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            أرجع iSrc;
          }وإلا
#پایان_اگر /* SQLITE_OMIT_GENERATED_COLUMNS */
          إذا( p_ستون->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جمع_کن_Op2(v, OP_SCopy, iSrc, هدف);
            sqlite3_Vdbe_جمع_کن_Op1(v, OP_RealAffinity, هدف);
            أرجع هدف;
          }وإلا{
            أرجع iSrc;
          }
        }وإلا{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      وإلا إذا( pParse->p_Idx_Part_عبارت 
       && 0!=(r1 = عبارت_Partidx_عبارت_Lookup(pParse, p_عبارت, هدف))
      ){
        أرجع r1;
      }
      أكد( عبارت_Use_YTab(p_عبارت) );
      أكد( p_عبارت->y.pTab!=0 );
      iReg = sqlite3_عبارت_کد_Get_ستونی(pParse, p_عبارت->y.pTab,
                               p_عبارت->i_ستونی, iTab, هدف,
                               p_عبارت->op2);
      أرجع iReg;
    }
    حالة TK_INTEGER: {
      کد_Integer(pParse, p_عبارت, 0, هدف);
      أرجع هدف;
    }
    حالة TK_TRUEFALSE: {
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, sqlite3_عبارت_Truth_مقدار(p_عبارت), هدف);
      أرجع هدف;
    }
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
    حالة TK_FLOAT: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      کد_Real(v, p_عبارت->u.zToken, 0, هدف);
      أرجع هدف;
    }
#پایان_اگر
    حالة TK_STRING: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_Vdbe_بارگیری_String(v, هدف, p_عبارت->u.zToken);
      أرجع هدف;
    }
    حالة TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Null, 0, هدف, هدف + p_عبارت->y.nReg - 1);
      أرجع هدف;
    }
    افتراضي: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
      أرجع هدف;
    }
#اگر_تعریف_نشده SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      صحيح n;
      ثابت حرف *z;
      حرف *zBlob;
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      أكد( p_عبارت->u.zToken[0]=='x' || p_عبارت->u.zToken[0]=='X' );
      أكد( p_عبارت->u.zToken[1]=='\'' );
      z = &p_عبارت->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3_Hex_مقصد_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_جمع_کن_Op4(v, OP_Blob, n/2, هدف, 0, zBlob, P4_DYNAMIC);
      أرجع هدف;
    }
#پایان_اگر
    حالة TK_VARIABLE: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      أكد( p_عبارت->u.zToken!=0 );
      أكد( p_عبارت->u.zToken[0]!=0 );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Variable, p_عبارت->i_ستونی, هدف);
      أرجع هدف;
    }
    حالة TK_ثبات: {
      أرجع p_عبارت->i_جدول;
    }
#اگر_تعریف_نشده SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_عبارت_کد(pParse, p_عبارت->p_چپ, هدف);
      أكد( درون_Reg==هدف );
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Cast, هدف,
                        sqlite3AffinityType(p_عبارت->u.zToken, 0));
      أرجع درون_Reg;
    }
#پایان_اگر /* SQLITE_OMIT_CAST */
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
      عبارت *p_چپ = p_عبارت->p_چپ;
      صحيح addrIsNull = 0;
      إذا( sqlite3_عبارت_Is_وکتور(p_چپ) ){
        کد_وکتور_Compare(pParse, p_عبارت, هدف, op, p5);
      }وإلا{
        إذا( عبارت_Has_Property(p_عبارت, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                     &r1, &r2, &regFree1, &regFree2);
        }وإلا{
          r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
          r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, &regFree2);
        }
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, درون_Reg);
        کد_Compare(pParse, p_چپ, p_عبارت->p_راست, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            عبارت_Has_Property(p_عبارت,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        إذا( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, درون_Reg);
        }وإلا{
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_ZeroOrNull, r1, درون_Reg, r2);
          إذا( addrIsNull ){
            sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_اینجا(v, addrIsNull);
            sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, درون_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_AND:
    حالة TK_OR: {
      درون_Reg = عبارت_کد_هدف_And_Or(pParse, p_عبارت, هدف, &regFree1);
      اكسر;
    }
    حالة TK_PLUS:
    حالة TK_ستاره:
    حالة TK_MINUS:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_BITOR:
    حالة TK_SLASH:
    حالة TK_LSHIFT:
    حالة TK_RSHIFT:
    حالة TK_CONCAT: {
      صحيح addrIsNull;
      أكد( TK_PLUS==OP_جمع_کن );           testcase( op==TK_PLUS );
      أكد( TK_MINUS==OP_تفریق );     testcase( op==TK_MINUS );
      أكد( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      أكد( TK_BITAND==OP_بیت_And );      testcase( op==TK_BITAND );
      أكد( TK_BITOR==OP_بیت_Or );        testcase( op==TK_BITOR );
      أكد( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      أكد( TK_LSHIFT==OP_لغزش_چپ );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_لغزش_راست );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      إذا( عبارت_Has_Property(p_عبارت, EP_Subquery) ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_جمع_کن_Op3(v, op, r2, r1, هدف);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      إذا( addrIsNull ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_اینجا(v, addrIsNull);
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
        VdbeComment((v, "short-circut value"));
      }
      اكسر;
    }
    حالة TK_UMINUS: {
      عبارت *p_چپ = p_عبارت->p_چپ;
      أكد( p_چپ );
      إذا( p_چپ->op==TK_INTEGER ){
        کد_Integer(pParse, p_چپ, 1, هدف);
        أرجع هدف;
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
      }وإلا إذا( p_چپ->op==TK_FLOAT ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        کد_Real(v, p_چپ->u.zToken, 1, هدف);
        أرجع هدف;
#پایان_اگر
      }وإلا{
        موقت_X.op = TK_INTEGER;
        موقت_X.flags = EP_Int_مقدار|EP_TokenOnly;
        موقت_X.u.i_مقدار = 0;
        عبارت_Clear_VVAProperties(&موقت_X);
        r1 = sqlite3_عبارت_کد_موقت(pParse, &موقت_X, &regFree1);
        r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree2);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_تفریق, r2, r1, هدف);
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أكد( TK_BITNOT==OP_بیت_Not );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_جمع_کن_Op2(v, op, r1, درون_Reg);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحيح b_نرمال;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      b_نرمال = p_عبارت->op2==TK_IS;
      testcase( isTrue && b_نرمال);
      testcase( !isTrue && b_نرمال);
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_IsTrue, r1, درون_Reg, !isTrue, isTrue ^ b_نرمال);
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      صحيح addr;
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, هدف);
      r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_جمع_کن_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, هدف);
      sqlite3_Vdbe_Jump_اینجا(v, addr);
      اكسر;
    }
    حالة TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_عبارت->pAggInfo;
      إذا( pInfo==0
       || NEVER(p_عبارت->iAgg<0)
       || NEVER(p_عبارت->iAgg>=pInfo->nFunc)
      ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
        sqlite3_Error_پیام(pParse, "misuse of aggregate: %#T()", p_عبارت);
      }وإلا{
        أرجع AggInfoFuncReg(pInfo, p_عبارت->iAgg);
      }
      اكسر;
    }
    حالة TK_FUNCTION: {
      عبارت_List *pFarg;       /* List of function arguments */
      صحيح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت حرف *zId;       /* The function name */
      u32 const_ماسک = 0;     /* Mask of function arguments that are constant */
      صحيح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      إذا( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
        أرجع p_عبارت->y.pWin->reg_نتیجه;
      }
#پایان_اگر

      إذا( Const_ضریب_Ok(pParse)
       && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        أرجع sqlite3_عبارت_کد_پیاپی_Just_Once(pParse, p_عبارت, -1);
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly) );
      أكد( عبارت_Use_XList(p_عبارت) );
      pFarg = p_عبارت->x.pList;
      nFarg = pFarg ? pFarg->n_عبارت : 0;
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      zId = p_عبارت->u.zToken;
      pDef = sqlite3_پیدا_کن_Function(db, zId, nFarg, enc, 0);
#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      إذا( pDef==0 && pParse->explain ){
        pDef = sqlite3_پیدا_کن_Function(db, "unknown", nFarg, enc, 0);
      }
#پایان_اگر
      إذا( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_داخلی)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_داخلی_Func)==0)
      ){
        sqlite3_Error_پیام(pParse, "unknown function: %#T()", p_عبارت);
        اكسر;
      }
      إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_مستقیم)==0 );
        أرجع عبارت_کد_Inline_Function(pParse, pFarg,
             SQLITE_PTR_مقصد_INT(pDef->p_User_داده), هدف);
      }وإلا إذا( pDef->funcFlags & (SQLITE_FUNC_مستقیم|SQLITE_FUNC_UNSAFE) ){
        sqlite3_عبارت_Function_Usable(pParse, p_عبارت, pDef);
      }

      لكل(i=0; i<nFarg; i++){
        إذا( i<32 && sqlite3_عبارت_Is_Constant(pParse, pFarg->a[i].p_عبارت) ){
          testcase( i==31 );
          const_ماسک |= MASKBIT32(i);
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_عبارت_Coll_Seq(pParse, pFarg->a[i].p_عبارت);
        }
      }
      إذا( pFarg ){
        إذا( const_ماسک ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }وإلا{
          r1 = sqlite3_Get_موقت_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        إذا( (pDef->funcFlags & (SQLITE_FUNC_درازی|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 عبارت_Op;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].p_عبارت!=0 );
          عبارت_Op = pFarg->a[0].p_عبارت->op;
          إذا( عبارت_Op==TK_ستونی || عبارت_Op==TK_AGG_ستونی ){
            أكد( SQLITE_FUNC_درازی==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_عبارت->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_عبارت_کد_عبارت_List(pParse, pFarg, r1, 0, SQLITE_ECEL_ضریب);
      }وإلا{
        r1 = 0;
      }
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
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
      إذا( nFarg>=2 && عبارت_Has_Property(p_عبارت, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_عبارت);
      }وإلا إذا( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_عبارت);
      }
#پایان_اگر
      إذا( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        إذا( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_جمع_کن_Op4(v, OP_CollSeq, 0, 0, 0, (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جمع_کن_Function_Call(pParse, const_ماسک, r1, هدف, nFarg,
                                 pDef, p_عبارت->op2);
      إذا( nFarg ){
        إذا( const_ماسک==0 ){
          sqlite3_Release_موقت_Range(pParse, r1, nFarg);
        }وإلا{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_ماسک, 1);
        }
      }
      أرجع هدف;
    }
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT: {
      صحيح n_ستون;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      إذا( pParse->db->mallocFailed ){
        أرجع 0;
      }وإلا إذا( op==TK_SELECT
             && ALWAYS( عبارت_Use_XSelect(p_عبارت) )
             && (n_ستون = p_عبارت->x.pSelect->pEList->n_عبارت)!=1
      ){
        sqlite3SubselectError(pParse, n_ستون, 1);
      }وإلا{
        أرجع sqlite3_کد_Subselect(pParse, p_عبارت);
      }
      اكسر;
    }
    حالة TK_SELECT_ستونی: {
      صحيح n;
      عبارت *p_چپ = p_عبارت->p_چپ;
      إذا( p_چپ->i_جدول==0 || pParse->withinRJSubrtn > p_چپ->op2 ){
        p_چپ->i_جدول = sqlite3_کد_Subselect(pParse, p_چپ);
        p_چپ->op2 = pParse->withinRJSubrtn;
      }
      أكد( p_چپ->op==TK_SELECT || p_چپ->op==TK_ERROR );
      n = sqlite3_عبارت_وکتور_اندازه(p_چپ);
      إذا( p_عبارت->i_جدول!=n ){
        sqlite3_Error_پیام(pParse, "%d columns assigned %d values",
                                p_عبارت->i_جدول, n);
      }
      أرجع p_چپ->i_جدول + p_عبارت->i_ستونی;
    }
    حالة TK_درون: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
      sqlite3_عبارت_کد_درون(pParse, p_عبارت, destIfFalse, destIfNull);
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, هدف);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_جمع_کن_Imm, هدف, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      أرجع هدف;
    }
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */


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
      عبارت_کد_Between(pParse, p_عبارت, هدف, 0, 0);
      أرجع هدف;
    }
    حالة TK_COLLATE: {
      إذا( !عبارت_Has_Property(p_عبارت, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( p_عبارت->p_چپ );
        sqlite3_عبارت_کد(pParse, p_عبارت->p_چپ, هدف);
        sqlite3_Vdbe_جمع_کن_Op1(v, OP_ClrSubtype, هدف);
        أرجع هدف;
      }وإلا{
        p_عبارت = p_عبارت->p_چپ;
        اذهب عبارت_کد_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالة TK_بازه:
    حالة TK_UPLUS: {
      p_عبارت = p_عبارت->p_چپ;
      اذهب عبارت_کد_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
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
      جدول *pTab;
      صحيح i_ستون;
      صحيح p1;

      أكد( عبارت_Use_YTab(p_عبارت) );
      pTab = p_عبارت->y.pTab;
      i_ستون = p_عبارت->i_ستونی;
      p1 = p_عبارت->i_جدول * (pTab->n_ستون+1) + 1
                     + sqlite3_جدول_ستونی_مقصد_Storage(pTab, i_ستون);

      أكد( p_عبارت->i_جدول==0 || p_عبارت->i_جدول==1 );
      أكد( i_ستون>=-1 && i_ستون<pTab->n_ستون );
      أكد( pTab->iPKey<0 || i_ستون!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->n_ستون*2+2) );

      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Param, p1, هدف);
      VdbeComment((v, "r[%d]=%s.%s", هدف,
        (p_عبارت->i_جدول ? "new" : "old"),
        (p_عبارت->i_ستونی<0 ? "rowid" : p_عبارت->y.pTab->a_ستون[i_ستون].zCnName)
      ));

#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      إذا( i_ستون>=0 && pTab->a_ستون[i_ستون].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_جمع_کن_Op1(v, OP_RealAffinity, هدف);
      }
#پایان_اگر
      اكسر;
    }

    حالة TK_وکتور: {
      sqlite3_Error_پیام(pParse, "row value misused");
      اكسر;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    حالة TK_IF_NULL_ردیف: {
      صحيح addrINR;
      u8 ok_Const_ضریب = pParse->ok_Const_ضریب;
      AggInfo *pAggInfo = p_عبارت->pAggInfo;
      إذا( pAggInfo ){
        أكد( p_عبارت->iAgg>=0 && p_عبارت->iAgg<pAggInfo->n_ستونی );
        إذا( !pAggInfo->مستقیم_نما ){
          درون_Reg = Agg_Info_ستونی_Reg(pAggInfo, p_عبارت->iAgg);
          اكسر;
        }
        إذا( p_عبارت->pAggInfo->use_ترتیبدهی_Idx ){
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, pAggInfo->ترتیبدهی_Idx_PTab,
                            pAggInfo->a_ستون[p_عبارت->iAgg].i_Sorter_ستونی,
                            هدف);
          درون_Reg = هدف;
          اكسر;
        }
      }
      addrINR = sqlite3_Vdbe_جمع_کن_Op3(v, OP_If_Null_ردیف, p_عبارت->i_جدول, 0, هدف);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_ضریب = 0;   /* note (1) above */
      sqlite3_عبارت_کد(pParse, p_عبارت->p_چپ, هدف);
      أكد( هدف==درون_Reg );
      pParse->ok_Const_ضریب = ok_Const_ضریب;
      sqlite3_Vdbe_Jump_اینجا(v, addrINR);
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
      صحيح انتها_Label;                     /* GOTO label for end of CASE stmt */
      صحيح بعدی_Case;                     /* GOTO label for next WHEN clause */
      صحيح n_عبارت;                        /* 2x number of WHEN terms */
      صحيح i;                            /* Loop counter */
      عبارت_List *pEList;                 /* List of WHEN terms */
      بنية عبارت_List_قلم *aListelem;  /* Array of WHEN terms */
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
      انتها_Label = sqlite3VdbeMakeLabel(pParse);
      إذا( (pX = p_عبارت->p_چپ)!=0 ){
        pDel = sqlite3_عبارت_Dup(db, pX, 0);
        إذا( db->mallocFailed ){
          sqlite3_عبارت_Delete(db, pDel);
          اكسر;
        }
        testcase( pX->op==TK_ستونی );
        sqlite3_عبارت_مقصد_ثبات(pDel, عبارت_کد_وکتور(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, حجم(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_چپ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      لكل(i=0; i<n_عبارت-1; i=i+2){
        إذا( pX ){
          أكد( pTest!=0 );
          opCompare.p_راست = aListelem[i].p_عبارت;
        }وإلا{
          pTest = aListelem[i].p_عبارت;
        }
        بعدی_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_ستونی );
        sqlite3_عبارت_If_False(pParse, pTest, بعدی_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_عبارت->op==TK_ستونی );
        sqlite3_عبارت_کد(pParse, aListelem[i+1].p_عبارت, هدف);
        sqlite3VdbeGoto(v, انتها_Label);
        sqlite3VdbeResolveLabel(v, بعدی_Case);
      }
      إذا( (n_عبارت&1)!=0 ){
        sqlite3_عبارت_کد(pParse, pEList->a[n_عبارت-1].p_عبارت, هدف);
      }وإلا{
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, هدف);
      }
      sqlite3_عبارت_Delete(db, pDel);
      مجموعه_Do_Not_ادغام_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, انتها_Label);
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      أكد( p_عبارت->aff_عبارت==OE_Rollback
           || p_عبارت->aff_عبارت==OE_Abort
           || p_عبارت->aff_عبارت==OE_Fail
           || p_عبارت->aff_عبارت==OE_Ignore
      );
      إذا( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_پیام(pParse,
                       "RAISE() may only be used within a trigger-program");
        أرجع 0;
      }
      إذا( p_عبارت->aff_عبارت==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      إذا( p_عبارت->aff_عبارت==OE_Ignore ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_عبارت->aff_عبارت, r1);
      }
      اكسر;
    }
#پایان_اگر
  }
  sqlite3_Release_موقت_Reg(pParse, regFree1);
  sqlite3_Release_موقت_Reg(pParse, regFree2);
  أرجع درون_Reg;
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
صحيح sqlite3_عبارت_کد_پیاپی_Just_Once(
  Parse *pParse,    /* Parsing context */
  عبارت *p_عبارت,      /* The expression to code when the VDBE initializes */
  صحيح regDest       /* Store the value in this register */
){
  عبارت_List *p;
  أكد( Const_ضریب_Ok(pParse) );
  أكد( regDest!=0 );
  p = pParse->p_Const_عبارت;
  إذا( regDest<0 && p ){
    بنية عبارت_List_قلم *p_قلم;
    صحيح i;
    لكل(p_قلم=p->a, i=p->n_عبارت; i>0; p_قلم++, i--){
      إذا( p_قلم->fg.reusable
       && sqlite3_عبارت_Compare(0,p_قلم->p_عبارت,p_عبارت,-1)==0
      ){
        أرجع p_قلم->u.i_Const_عبارت_Reg;
      }
    }
  }
  p_عبارت = sqlite3_عبارت_Dup(pParse->db, p_عبارت, 0);
  إذا( p_عبارت!=0 && عبارت_Has_Property(p_عبارت, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحيح addr;
    أكد( v );
    addr = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_ضریب = 0;
    إذا( !pParse->db->mallocFailed ){
      إذا( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_عبارت_کد(pParse, p_عبارت, regDest);
    }
    pParse->ok_Const_ضریب = 1;
    sqlite3_عبارت_Delete(pParse->db, p_عبارت);
    sqlite3_Vdbe_Jump_اینجا(v, addr);
  }وإلا{
    p = sqlite3_عبارت_List_Append(pParse, p, p_عبارت);
    إذا( p ){
       بنية عبارت_List_قلم *p_قلم = &p->a[p->n_عبارت-1];
       p_قلم->fg.reusable = regDest<0;
       إذا( regDest<0 ) regDest = ++pParse->nMem;
       p_قلم->u.i_Const_عبارت_Reg = regDest;
    }
    pParse->p_Const_عبارت = p;
  }
  أرجع regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE فراغ sqlite3_عبارت_Null_ثبات_Range(
  Parse *pParse,   /* Parsing context */
  صحيح iReg,        /* First register to set to NULL */
  صحيح nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_ضریب = pParse->ok_Const_ضریب;
  عبارت t;
  memset(&t, 0, حجم(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_ضریب = 1;
  sqlite3_عبارت_کد_پیاپی_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_ضریب = ok_Const_ضریب;
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
صحيح sqlite3_عبارت_کد_موقت(Parse *pParse, عبارت *p_عبارت, صحيح *pReg){
  صحيح r2;
  p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  إذا( Const_ضریب_Ok(pParse)
   && ALWAYS(p_عبارت!=0)
   && p_عبارت->op!=TK_ثبات
   && sqlite3_عبارت_Is_Constant_Not_Join(pParse, p_عبارت)
  ){
    *pReg  = 0;
    r2 = sqlite3_عبارت_کد_پیاپی_Just_Once(pParse, p_عبارت, -1);
  }وإلا{
    صحيح r1 = sqlite3_Get_موقت_Reg(pParse);
    r2 = sqlite3_عبارت_کد_هدف(pParse, p_عبارت, r1);
    إذا( r2==r1 ){
      *pReg = r1;
    }وإلا{
      sqlite3_Release_موقت_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  أرجع r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
فراغ sqlite3_عبارت_کد(Parse *pParse, عبارت *p_عبارت, صحيح هدف){
  صحيح درون_Reg;

  أكد( p_عبارت==0 || !عبارت_Has_VVAProperty(p_عبارت,EP_Immutable) );
  أكد( هدف>0 && هدف<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  إذا( pParse->pVdbe==0 ) أرجع;
  درون_Reg = sqlite3_عبارت_کد_هدف(pParse, p_عبارت, هدف);
  إذا( درون_Reg!=هدف ){
    u8 op;
    عبارت *pX = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
    testcase( pX!=p_عبارت );
    إذا( ALWAYS(pX)
     && (عبارت_Has_Property(pX,EP_Subquery) || pX->op==TK_ثبات)
    ){
      op = OP_Copy;
    }وإلا{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_جمع_کن_Op2(pParse->pVdbe, op, درون_Reg, هدف);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
فراغ sqlite3_عبارت_کد_Copy(Parse *pParse, عبارت *p_عبارت, صحيح هدف){
  sqlite3 *db = pParse->db;
  p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
  إذا( !db->mallocFailed ) sqlite3_عبارت_کد(pParse, p_عبارت, هدف);
  sqlite3_عبارت_Delete(db, p_عبارت);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
فراغ sqlite3_عبارت_کد_Factorable(Parse *pParse, عبارت *p_عبارت, صحيح هدف){
  إذا( pParse->ok_Const_ضریب && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت) ){
    sqlite3_عبارت_کد_پیاپی_Just_Once(pParse, p_عبارت, هدف);
  }وإلا{
    sqlite3_عبارت_کد_Copy(pParse, p_عبارت, هدف);
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
صحيح sqlite3_عبارت_کد_عبارت_List(
  Parse *pParse,     /* Parsing context */
  عبارت_List *pList,   /* The expression list to be coded */
  صحيح هدف,        /* Where to write results */
  صحيح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  بنية عبارت_List_قلم *p_قلم;
  صحيح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( هدف>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_عبارت;
  إذا( !Const_ضریب_Ok(pParse) ) flags &= ~SQLITE_ECEL_ضریب;
  لكل(p_قلم=pList->a, i=0; i<n; i++, p_قلم++){
    عبارت *p_عبارت = p_قلم->p_عبارت;
#اگر_تعریف_شده SQLITE_ENABLE_SORTER_REFERENCES
    إذا( p_قلم->fg.bSorterRef ){
      i--;
      n--;
    }وإلا
#پایان_اگر
    إذا( (flags & SQLITE_ECEL_REF)!=0 && (j = p_قلم->u.x.i_ترتیب_By_ستون)>0 ){
      إذا( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }وإلا{
        sqlite3_Vdbe_جمع_کن_Op2(v, copyOp, j+srcReg-1, هدف+i);
      }
    }وإلا إذا( (flags & SQLITE_ECEL_ضریب)!=0
           && sqlite3_عبارت_Is_Constant_Not_Join(pParse,p_عبارت)
    ){
      sqlite3_عبارت_کد_پیاپی_Just_Once(pParse, p_عبارت, هدف+i);
    }وإلا{
      صحيح درون_Reg = sqlite3_عبارت_کد_هدف(pParse, p_عبارت, هدف+i);
      إذا( درون_Reg!=هدف+i ){
        VdbeOp *pOp;
        إذا( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_آخرین_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==درون_Reg
         && pOp->p2+pOp->p3+1==هدف+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }وإلا{
          sqlite3_Vdbe_جمع_کن_Op2(v, copyOp, درون_Reg, هدف+i);
        }
      }
    }
  }
  أرجع n;
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
ساكن فراغ عبارت_کد_Between(
  Parse *pParse,    /* Parsing and code generating context */
  عبارت *p_عبارت,      /* The BETWEEN expression */
  صحيح dest,         /* Jump destination or storage location */
  فراغ (*xJump)(Parse*,عبارت*,صحيح,صحيح), /* Action to take */
  صحيح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  عبارت عبارت_And;     /* The AND operator in  x>=y AND x<=z  */
  عبارت comp_چپ;    /* The  x>=y  term */
  عبارت comp_راست;   /* The  x<=z  term */
  صحيح regFree1 = 0; /* Temporary use register */
  عبارت *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_چپ, 0, حجم(عبارت));
  memset(&comp_راست, 0, حجم(عبارت));
  memset(&عبارت_And, 0, حجم(عبارت));

  أكد( عبارت_Use_XList(p_عبارت) );
  pDel = sqlite3_عبارت_Dup(db, p_عبارت->p_چپ, 0);
  إذا( db->mallocFailed==0 ){
    عبارت_And.op = TK_AND;
    عبارت_And.p_چپ = &comp_چپ;
    عبارت_And.p_راست = &comp_راست;
    comp_چپ.op = TK_GE;
    comp_چپ.p_چپ = pDel;
    comp_چپ.p_راست = p_عبارت->x.pList->a[0].p_عبارت;
    comp_راست.op = TK_LE;
    comp_راست.p_چپ = pDel;
    comp_راست.p_راست = p_عبارت->x.pList->a[1].p_عبارت;
    sqlite3_عبارت_مقصد_ثبات(pDel, عبارت_کد_وکتور(pParse, pDel, &regFree1));
    إذا( xJump ){
      xJump(pParse, &عبارت_And, dest, jumpIfNull);
    }وإلا{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_عبارت_کد_هدف(pParse, &عبارت_And, dest);
    }
    sqlite3_Release_موقت_Reg(pParse, regFree1);
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
فراغ sqlite3_عبارت_If_True(Parse *pParse, عبارت *p_عبارت, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) )     أرجع;  /* Existence of VDBE checked by caller */
  إذا( NEVER(p_عبارت==0) ) أرجع;  /* No way this can happen */
  أكد( !عبارت_Has_VVAProperty(p_عبارت, EP_Immutable) );
  op = p_عبارت->op;
  بدل( op ){
    حالة TK_AND:
    حالة TK_OR: {
      عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
      إذا( pAlt!=p_عبارت ){
        sqlite3_عبارت_If_True(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        عبارت *p_اولی, *p_دومی;
        إذا( عبارت_Eval_Rhs_اولی(p_عبارت) ){
          p_اولی = p_عبارت->p_راست;
          p_دومی = p_عبارت->p_چپ;
        }وإلا{
          p_اولی = p_عبارت->p_چپ;
          p_دومی = p_عبارت->p_راست;
        }
        إذا( op==TK_AND ){
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_False(pParse, p_اولی, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_عبارت_If_True(pParse, p_دومی, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }وإلا{
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_True(pParse, p_اولی, dest, jumpIfNull);
          sqlite3_عبارت_If_True(pParse, p_دومی, dest, jumpIfNull);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_عبارت->op2==TK_ISNOT;
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }وإلا{
        sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest,
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
      إذا( sqlite3_عبارت_Is_وکتور(p_عبارت->p_چپ) ) اذهب default_عبارت;
      إذا( عبارت_Has_Property(p_عبارت, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      کد_Compare(pParse, p_عبارت->p_چپ, p_عبارت->p_راست, op,
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
      إذا( addrIsNull ){
        إذا( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }وإلا{
          sqlite3_Vdbe_Jump_اینجا(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3_Vdbe_Typeof_ستونی(v, r1);
      sqlite3_Vdbe_جمع_کن_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      عبارت_کد_Between(pParse, p_عبارت, dest, sqlite3_عبارت_If_True, jumpIfNull);
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
    حالة TK_درون: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_عبارت_کد_درون(pParse, p_عبارت, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      اكسر;
    }
#پایان_اگر
    افتراضي: {
    default_عبارت:
      إذا( عبارت_Always_True(p_عبارت) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( عبارت_Always_False(p_عبارت) ){
        /* No-op */
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت, &regFree1);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3_Release_موقت_Reg(pParse, regFree1);
  sqlite3_Release_موقت_Reg(pParse, regFree2); 
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
فراغ sqlite3_عبارت_If_False(Parse *pParse, عبارت *p_عبارت, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) ) أرجع; /* Existence of VDBE checked by caller */
  إذا( p_عبارت==0 )    أرجع;
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

  بدل( p_عبارت->op ){
    حالة TK_AND:
    حالة TK_OR: {
      عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
      إذا( pAlt!=p_عبارت ){
        sqlite3_عبارت_If_False(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        عبارت *p_اولی, *p_دومی;
        إذا( عبارت_Eval_Rhs_اولی(p_عبارت) ){
          p_اولی = p_عبارت->p_راست;
          p_دومی = p_عبارت->p_چپ;
        }وإلا{
          p_اولی = p_عبارت->p_چپ;
          p_دومی = p_عبارت->p_راست;
        }
        إذا( p_عبارت->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_False(pParse, p_اولی, dest, jumpIfNull);
          sqlite3_عبارت_If_False(pParse, p_دومی, dest, jumpIfNull);
        }وإلا{
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_عبارت_If_True(pParse, p_اولی, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_عبارت_If_False(pParse, p_دومی, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_عبارت->op2==TK_ISNOT;
      isTrue = sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_عبارت_If_False(pParse, p_عبارت->p_چپ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }وإلا{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_عبارت_If_True(pParse, p_عبارت->p_چپ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT:
      testcase( p_عبارت->op==TK_IS );
      testcase( p_عبارت->op==TK_ISNOT );
      op = (p_عبارت->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      صحيح addrIsNull;
      إذا( sqlite3_عبارت_Is_وکتور(p_عبارت->p_چپ) ) اذهب default_عبارت;
      إذا( عبارت_Has_Property(p_عبارت, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = عبارت_Compute_Operands(pParse, p_عبارت,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
        r2 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_راست, &regFree2);
        addrIsNull = 0;
      }
      کد_Compare(pParse, p_عبارت->p_چپ, p_عبارت->p_راست, op,
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
      إذا( addrIsNull ){
        إذا( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }وإلا{
          sqlite3_Vdbe_Jump_اینجا(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت->p_چپ, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3_Vdbe_Typeof_ستونی(v, r1);
      sqlite3_Vdbe_جمع_کن_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      عبارت_کد_Between(pParse, p_عبارت, dest, sqlite3_عبارت_If_False, jumpIfNull);
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
    حالة TK_درون: {
      إذا( jumpIfNull ){
        sqlite3_عبارت_کد_درون(pParse, p_عبارت, dest, dest);
      }وإلا{
        صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_عبارت_کد_درون(pParse, p_عبارت, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      اكسر;
    }
#پایان_اگر
    افتراضي: {
    default_عبارت:
      إذا( عبارت_Always_False(p_عبارت) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( عبارت_Always_True(p_عبارت) ){
        /* no-op */
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, p_عبارت, &regFree1);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3_Release_موقت_Reg(pParse, regFree1);
  sqlite3_Release_موقت_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
فراغ sqlite3_عبارت_If_False_Dup(Parse *pParse, عبارت *p_عبارت, صحيح dest,صحيح jumpIfNull){
  sqlite3 *db = pParse->db;
  عبارت *pCopy = sqlite3_عبارت_Dup(db, p_عبارت, 0);
  إذا( db->mallocFailed==0 ){
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
ساكن SQLITE_NOINLINE صحيح عبارت_Compare_Variable(
  ثابت Parse *pParse,
  ثابت عبارت *pVar,
  ثابت عبارت *p_عبارت
){
  صحيح res = 2;
  صحيح iVar;
  sqlite3_مقدار *pL, *pR = 0;
 
  إذا( p_عبارت->op==TK_VARIABLE && pVar->i_ستونی==p_عبارت->i_ستونی ){
    أرجع 0;
  }
  إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) أرجع 2;
  sqlite3_مقدار_From_عبارت(pParse->db, p_عبارت, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  إذا( pR ){
    iVar = pVar->i_ستونی;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_مقدار(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    إذا( pL ){
      إذا( sqlite3_مقدار_type(pL)==SQLITE_متن ){
        sqlite3_مقدار_متن(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_مقدار_Free(pR);
    sqlite3_مقدار_Free(pL);
  }
  أرجع res;
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
صحيح sqlite3_عبارت_Compare(
  ثابت Parse *pParse,
  ثابت عبارت *pA,
  ثابت عبارت *pB,
  صحيح iTab
){
  u32 combinedFlags;
  إذا( pA==0 || pB==0 ){
    أرجع pB==pA ? 0 : 2;
  }
  إذا( pParse && pA->op==TK_VARIABLE ){
    أرجع عبارت_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  إذا( combinedFlags & EP_Int_مقدار ){
    إذا( (pA->flags&pB->flags&EP_Int_مقدار)!=0 && pA->u.i_مقدار==pB->u.i_مقدار ){
      أرجع 0;
    }
    أرجع 2;
  }
  إذا( pA->op!=pB->op || pA->op==TK_RAISE ){
    إذا( pA->op==TK_COLLATE && sqlite3_عبارت_Compare(pParse, pA->p_چپ,pB,iTab)<2 ){
      أرجع 1;
    }
    إذا( pB->op==TK_COLLATE && sqlite3_عبارت_Compare(pParse, pA,pB->p_چپ,iTab)<2 ){
      أرجع 1;
    }
    إذا( pA->op==TK_AGG_ستونی && pB->op==TK_ستونی
     && pB->i_جدول<0 && pA->i_جدول==iTab
    ){
      /* fall through */
    }وإلا{
      أرجع 2;
    }
  }
  أكد( !عبارت_Has_Property(pA, EP_Int_مقدار) );
  أكد( !عبارت_Has_Property(pB, EP_Int_مقدار) );
  إذا( pA->u.zToken ){
    إذا( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      إذا( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      إذا( عبارت_Has_Property(pA,EP_WinFunc)!=عبارت_Has_Property(pB,EP_WinFunc) ){
        أرجع 2;
      }
      إذا( عبارت_Has_Property(pA,EP_WinFunc) ){
        إذا( sqlite3_پنجره_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          أرجع 2;
        }
      }
#پایان_اگر
    }وإلا إذا( pA->op==TK_NULL ){
      أرجع 0;
    }وإلا إذا( pA->op==TK_COLLATE ){
      إذا( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
    }وإلا
    إذا( pB->u.zToken!=0
     && pA->op!=TK_ستونی
     && pA->op!=TK_AGG_ستونی
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      أرجع 2;
    }
  }
  إذا( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) أرجع 2;
  إذا( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    إذا( combinedFlags & EP_xIsSelect ) أرجع 2;
    إذا( (combinedFlags & EP_Fixed_ستون)==0
     && sqlite3_عبارت_Compare(pParse, pA->p_چپ, pB->p_چپ, iTab) ) أرجع 2;
    إذا( sqlite3_عبارت_Compare(pParse, pA->p_راست, pB->p_راست, iTab) ) أرجع 2;
    إذا( sqlite3_عبارت_List_Compare(pA->x.pList, pB->x.pList, iTab) ) أرجع 2;
    إذا( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      إذا( pA->i_ستونی!=pB->i_ستونی ) أرجع 2;
      إذا( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) أرجع 2;
      إذا( pA->op!=TK_درون && pA->i_جدول!=pB->i_جدول && pA->i_جدول!=iTab ){
        أرجع 2;
      }
    }
  }
  أرجع 0;
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
صحيح sqlite3_عبارت_List_Compare(ثابت عبارت_List *pA, ثابت عبارت_List *pB, صحيح iTab){
  صحيح i;
  إذا( pA==0 && pB==0 ) أرجع 0;
  إذا( pA==0 || pB==0 ) أرجع 1;
  إذا( pA->n_عبارت!=pB->n_عبارت ) أرجع 1;
  لكل(i=0; i<pA->n_عبارت; i++){
    صحيح res;
    عبارت *p_عبارت_A = pA->a[i].p_عبارت;
    عبارت *p_عبارت_B = pB->a[i].p_عبارت;
    إذا( pA->a[i].fg.مرتب_سازی_Flags!=pB->a[i].fg.مرتب_سازی_Flags ) أرجع 1;
    إذا( (res = sqlite3_عبارت_Compare(0, p_عبارت_A, p_عبارت_B, iTab)) ) أرجع res;
  }
  أرجع 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحيح sqlite3_عبارت_Compare_Skip(عبارت *pA,عبارت *pB, صحيح iTab){
  أرجع sqlite3_عبارت_Compare(0,
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
ساكن صحيح عبارت_نتیجه_میدهد_Not_Null(
  ثابت Parse *pParse,/* Parsing context */
  ثابت عبارت *p,      /* The expression to be checked */
  ثابت عبارت *pNN,    /* The expression that is NOT NULL */
  صحيح iTab,           /* Table being evaluated */
  صحيح دیده_Not         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  إذا( sqlite3_عبارت_Compare(pParse, p, pNN, iTab)==0 ){
    أرجع pNN->op!=TK_NULL;
  }
  بدل( p->op ){
    حالة TK_درون: {
      إذا( دیده_Not && عبارت_Has_Property(p, EP_xIsSelect) ) أرجع 0;
      أكد( عبارت_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_عبارت>0) );
      أرجع عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
    حالة TK_BETWEEN: {
      عبارت_List *pList;
      أكد( عبارت_Use_XList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->n_عبارت==2 );
      إذا( دیده_Not ) أرجع 0;
      إذا( عبارت_نتیجه_میدهد_Not_Null(pParse, pList->a[0].p_عبارت, pNN, iTab, 1)
       || عبارت_نتیجه_میدهد_Not_Null(pParse, pList->a[1].p_عبارت, pNN, iTab, 1)
      ){
        أرجع 1;
      }
      أرجع عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
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
      دیده_Not = 1;
      /* no break */ deliberate_fall_through
    حالة TK_ستاره:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_SLASH: {
      إذا( عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_راست, pNN, iTab, دیده_Not) ) أرجع 1;
      /* no break */ deliberate_fall_through
    }
    حالة TK_بازه:
    حالة TK_COLLATE:
    حالة TK_UPLUS:
    حالة TK_UMINUS: {
      أرجع عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_چپ, pNN, iTab, دیده_Not);
    }
    حالة TK_TRUTH: {
      إذا( دیده_Not ) أرجع 0;
      إذا( p->op2!=TK_IS ) أرجع 0;
      أرجع عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أرجع عبارت_نتیجه_میدهد_Not_Null(pParse, p->p_چپ, pNN, iTab, 1);
    }
  }
  أرجع 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ساكن صحيح sqlite3_عبارت_Is_Not_True(عبارت *p_عبارت){
  صحيح v;
  إذا( p_عبارت->op==TK_NULL ) أرجع 1;
  إذا( p_عبارت->op==TK_TRUEFALSE && sqlite3_عبارت_Truth_مقدار(p_عبارت)==0 ) أرجع 1;
  v = 1;
  إذا( sqlite3_عبارت_Is_Integer(p_عبارت, &v, 0, 0) && v==0 ) أرجع 1;
  أرجع 0;
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
ساكن صحيح sqlite3_عبارت_Is_IIF(sqlite3 *db, ثابت عبارت *p_عبارت){
  عبارت_List *pList;
  إذا( p_عبارت->op==TK_FUNCTION ){
    ثابت حرف *z = p_عبارت->u.zToken;
    FuncDef *pDef;
    إذا( (z[0]!='i' && z[0]!='I') ) أرجع 0;
    إذا( p_عبارت->x.pList==0 ) أرجع 0;
    pDef = sqlite3_پیدا_کن_Function(db, z, p_عبارت->x.pList->n_عبارت, ENC(db), 0);
#اگر_تعریف_شده SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    إذا( pDef==0 ) أرجع 0;
#وإلا
    إذا( NEVER(pDef==0) ) أرجع 0;
#پایان_اگر
    إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) أرجع 0;
    إذا( SQLITE_PTR_مقصد_INT(pDef->p_User_داده)!=INLINEFUNC_iif ) أرجع 0;
  }وإلا إذا( p_عبارت->op==TK_CASE ){
    إذا( p_عبارت->p_چپ!=0 ) أرجع 0;
  }وإلا{
    أرجع 0;
  }
  pList = p_عبارت->x.pList;
  أكد( pList!=0 );
  إذا( pList->n_عبارت==2 ) أرجع 1;
  إذا( pList->n_عبارت==3 && sqlite3_عبارت_Is_Not_True(pList->a[2].p_عبارت) ) أرجع 1;
  أرجع 0;
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
صحيح sqlite3_عبارت_نتیجه_میدهد_عبارت(
  ثابت Parse *pParse,
  ثابت عبارت *pE1,
  ثابت عبارت *pE2,
  صحيح iTab
){
  إذا( sqlite3_عبارت_Compare(pParse, pE1, pE2, iTab)==0 ){
    أرجع 1;
  }
  إذا( pE2->op==TK_OR
   && (sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse, pE1, pE2->p_چپ, iTab)
             || sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse, pE1, pE2->p_راست, iTab) )
  ){
    أرجع 1;
  }
  إذا( pE2->op==TK_NOTNULL
   && عبارت_نتیجه_میدهد_Not_Null(pParse, pE1, pE2->p_چپ, iTab, 0)
  ){
    أرجع 1;
  }
  إذا( sqlite3_عبارت_Is_IIF(pParse->db, pE1) ){
    أرجع sqlite3_عبارت_نتیجه_میدهد_عبارت(pParse,pE1->x.pList->a[0].p_عبارت,pE2,iTab);
  }
  أرجع 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ساكن فراغ both_Imply_Not_Null_ردیف(Walker *pWalker, عبارت *pE1, عبارت *pE2){
  إذا( pWalker->e_کد==0 ){
    sqlite3_Walk_عبارت(pWalker, pE1);
    إذا( pWalker->e_کد ){
      pWalker->e_کد = 0;
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
ساكن صحيح نتیجه_میدهد_Not_Null_ردیف(Walker *pWalker, عبارت *p_عبارت){
  testcase( p_عبارت->op==TK_AGG_ستونی );
  testcase( p_عبارت->op==TK_AGG_FUNCTION );
  إذا( عبارت_Has_Property(p_عبارت, EP_OuterON) ) أرجع WRC_Prune;
  إذا( عبارت_Has_Property(p_عبارت, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    أرجع WRC_Prune;
  }
  بدل( p_عبارت->op ){
    حالة TK_ISNOT:
    حالة TK_ISNULL:
    حالة TK_NOTNULL:
    حالة TK_IS:
    حالة TK_وکتور:
    حالة TK_FUNCTION:
    حالة TK_TRUTH:
    حالة TK_CASE:
      testcase( p_عبارت->op==TK_ISNOT );
      testcase( p_عبارت->op==TK_ISNULL );
      testcase( p_عبارت->op==TK_NOTNULL );
      testcase( p_عبارت->op==TK_IS );
      testcase( p_عبارت->op==TK_وکتور );
      testcase( p_عبارت->op==TK_FUNCTION );
      testcase( p_عبارت->op==TK_TRUTH );
      testcase( p_عبارت->op==TK_CASE );
      أرجع WRC_Prune;

    حالة TK_ستونی:
      إذا( pWalker->u.iCur==p_عبارت->i_جدول ){
        pWalker->e_کد = 1;
        أرجع WRC_Abort;
      }
      أرجع WRC_Prune;

    حالة TK_OR:
    حالة TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_عبارت->op==TK_OR );
      testcase( p_عبارت->op==TK_AND );
      both_Imply_Not_Null_ردیف(pWalker, p_عبارت->p_چپ, p_عبارت->p_راست);
      أرجع WRC_Prune;
       
    حالة TK_درون:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      إذا( عبارت_Use_XList(p_عبارت) && ALWAYS(p_عبارت->x.pList->n_عبارت>0) ){
        sqlite3_Walk_عبارت(pWalker, p_عبارت->p_چپ);
      }
      أرجع WRC_Prune;

    حالة TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( عبارت_Use_XList(p_عبارت) );
      أكد( p_عبارت->x.pList->n_عبارت==2 );
      sqlite3_Walk_عبارت(pWalker, p_عبارت->p_چپ);
      both_Imply_Not_Null_ردیف(pWalker, p_عبارت->x.pList->a[0].p_عبارت,
                                   p_عبارت->x.pList->a[1].p_عبارت);
      أرجع WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    حالة TK_EQ:
    حالة TK_NE:
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE: {
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
      أكد( p_چپ->op!=TK_ستونی || عبارت_Use_YTab(p_چپ) );
      أكد( p_راست->op!=TK_ستونی || عبارت_Use_YTab(p_راست) );
      إذا( (p_چپ->op==TK_ستونی
           && ALWAYS(p_چپ->y.pTab!=0)
           && IsVirtual(p_چپ->y.pTab))
       || (p_راست->op==TK_ستونی
           && ALWAYS(p_راست->y.pTab!=0)
           && IsVirtual(p_راست->y.pTab))
      ){
        أرجع WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    افتراضي:
      أرجع WRC_Continue;
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
صحيح sqlite3_عبارت_نتیجه_میدهد_Non_Null_ردیف(عبارت *p, صحيح iTab, صحيح isRJ){
  Walker w;
  p = sqlite3_عبارت_Skip_Collate_And_Likely(p);
  إذا( p==0 ) أرجع 0;
  إذا( p->op==TK_NOTNULL ){
    p = p->p_چپ;
  }وإلا{
    طالما( p->op==TK_AND ){
      إذا( sqlite3_عبارت_نتیجه_میدهد_Non_Null_ردیف(p->p_چپ, iTab, isRJ) ) أرجع 1;
      p = p->p_راست;
    }
  }
  w.x_عبارت_Callback = نتیجه_میدهد_Not_Null_ردیف;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_کد = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_عبارت(&w, p);
  أرجع w.e_کد;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
بنية IdxCover {
  شاخص *pIdx;     /* The index to be tested for coverage */
  صحيح iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ساكن صحيح عبارت_Idx_Cover(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_ستونی
   && p_عبارت->i_جدول==pWalker->u.pIdxCover->iCur
   && sqlite3_جدول_ستونی_مقصد_شاخص(pWalker->u.pIdxCover->pIdx, p_عبارت->i_ستونی)<0
  ){
    pWalker->e_کد = 1;
    أرجع WRC_Abort;
  }
  أرجع WRC_Continue;
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
صحيح sqlite3_عبارت_Covered_By_شاخص(
  عبارت *p_عبارت,        /* The index to be tested */
  صحيح iCur,           /* The cursor number for the corresponding table */
  شاخص *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  بنية IdxCover xcov;
  memset(&w, 0, حجم(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_عبارت_Callback = عبارت_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_عبارت(&w, p_عبارت);
  أرجع !w.e_کد;
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
  إذا( pSrc->nSrc==0 ) أرجع WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*حجم(صحيح));
  إذا( piNew==0 ){
    p->nExclude = 0;
    أرجع WRC_Abort;
  }وإلا{
    p->aiExclude = piNew;
  }
  لكل(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  أرجع WRC_Continue;
}
ساكن فراغ selectRefLeave(Walker *pWalker, Select *pSelect){
  بنية RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  إذا( p->nExclude ){
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
ساكن صحيح عبارت_Ref_مقصد_Src_List(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_ستونی
   || p_عبارت->op==TK_AGG_ستونی
  ){
    صحيح i;
    بنية RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحيح nSrc = pSrc ? pSrc->nSrc : 0;
    لكل(i=0; i<nSrc; i++){
      إذا( p_عبارت->i_جدول==pSrc->a[i].iCursor ){
        pWalker->e_کد |= 1;
        أرجع WRC_Continue;
      }
    }
    لكل(i=0; i<p->nExclude && p->aiExclude[i]!=p_عبارت->i_جدول; i++){}
    إذا( i>=p->nExclude ){
      pWalker->e_کد |= 2;
    }
  }
  أرجع WRC_Continue;
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
صحيح sqlite3ReferencesSrcList(Parse *pParse, عبارت *p_عبارت, SrcList *pSrcList){
  Walker w;
  بنية RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, حجم(w));
  memset(&x, 0, حجم(x));
  w.x_عبارت_Callback = عبارت_Ref_مقصد_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( p_عبارت->op==TK_AGG_FUNCTION );
  أكد( عبارت_Use_XList(p_عبارت) );
  sqlite3_Walk_عبارت_List(&w, p_عبارت->x.pList);
  إذا( p_عبارت->p_چپ ){
    أكد( p_عبارت->p_چپ->op==TK_ترتیب );
    أكد( عبارت_Use_XList(p_عبارت->p_چپ) );
    أكد( p_عبارت->p_چپ->x.pList!=0 );
    sqlite3_Walk_عبارت_List(&w, p_عبارت->p_چپ->x.pList);
  }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
  إذا( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
    sqlite3_Walk_عبارت(&w, p_عبارت->y.pWin->pFilter);
  }
#پایان_اگر
  إذا( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  إذا( w.e_کد & 0x01 ){
    أرجع 1;
  }وإلا إذا( w.e_کد ){
    أرجع 0;
  }وإلا{
    أرجع -1;
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
ساكن صحيح agginfo_Persist_عبارت_Cb(Walker *pWalker, عبارت *p_عبارت){
  إذا( ALWAYS(!عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced))
   && p_عبارت->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_عبارت->pAggInfo;
    صحيح iAgg = p_عبارت->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    إذا( p_عبارت->op!=TK_AGG_FUNCTION ){
      إذا( iAgg<pAggInfo->n_ستونی
       && pAggInfo->a_ستون[iAgg].pCExpr==p_عبارت
      ){
        p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        إذا( p_عبارت && !sqlite3_عبارت_Deferred_Delete(pParse, p_عبارت) ){
          pAggInfo->a_ستون[iAgg].pCExpr = p_عبارت;
        }
      }
    }وإلا{
      أكد( p_عبارت->op==TK_AGG_FUNCTION );
      إذا( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_عبارت
      ){
        p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        إذا( p_عبارت && !sqlite3_عبارت_Deferred_Delete(pParse, p_عبارت) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_عبارت;
        }
      }
    }
  }
  أرجع WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
فراغ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, حجم(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_عبارت_Callback = agginfo_Persist_عبارت_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح جمع_کن_Agg_Info_ستونی(sqlite3 *db, AggInfo *pInfo){
  صحيح i;
  pInfo->a_ستون = sqlite3ArrayAllocate(
       db,
       pInfo->a_ستون,
       حجم(pInfo->a_ستون[0]),
       &pInfo->n_ستونی,
       &i
  );
  أرجع i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح جمع_کن_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
  صحيح i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       حجم(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  أرجع i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ساكن فراغ پیدا_کن_Or_Create_Agg_Info_ستونی(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  عبارت *p_عبارت          /* Expr describing the column to find or insert */
){
  بنية Agg_Info_ستون *p_ستون;
  صحيح k;
  صحيح mx_جمله = pParse->db->a_حد[SQLITE_حد_ستونی];

  أكد( mx_جمله <= SMXV(i16) );
  أكد( pAggInfo->i_اولی_Reg==0 );
  p_ستون = pAggInfo->a_ستون;
  لكل(k=0; k<pAggInfo->n_ستونی; k++, p_ستون++){
    إذا( p_ستون->pCExpr==p_عبارت ) أرجع;
    إذا( p_ستون->i_جدول==p_عبارت->i_جدول
     && p_ستون->i_ستونی==p_عبارت->i_ستونی
     && p_عبارت->op!=TK_IF_NULL_ردیف
    ){
      اذهب fix_به_بالا_عبارت;
    }
  }
  k = جمع_کن_Agg_Info_ستونی(pParse->db, pAggInfo);
  إذا( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    أرجع;
  }
  إذا( k>mx_جمله ){
    sqlite3_Error_پیام(pParse, "more than %d aggregate terms", mx_جمله);
    k = mx_جمله;
  }
  p_ستون = &pAggInfo->a_ستون[k];
  أكد( عبارت_Use_YTab(p_عبارت) );
  p_ستون->pTab = p_عبارت->y.pTab;
  p_ستون->i_جدول = p_عبارت->i_جدول;
  p_ستون->i_ستونی = p_عبارت->i_ستونی;
  p_ستون->i_Sorter_ستونی = -1;
  p_ستون->pCExpr = p_عبارت;
  إذا( pAggInfo->p_گروه_By && p_عبارت->op!=TK_IF_NULL_ردیف ){
    صحيح j, n;
    عبارت_List *pGB = pAggInfo->p_گروه_By;
    بنية عبارت_List_قلم *p_جمله = pGB->a;
    n = pGB->n_عبارت;
    لكل(j=0; j<n; j++, p_جمله++){
      عبارت *pE = p_جمله->p_عبارت;
      إذا( pE->op==TK_ستونی
       && pE->i_جدول==p_عبارت->i_جدول
       && pE->i_ستونی==p_عبارت->i_ستونی
      ){
        p_ستون->i_Sorter_ستونی = j;
        اكسر;
      }
    }
  }
  إذا( p_ستون->i_Sorter_ستونی<0 ){
    p_ستون->i_Sorter_ستونی = pAggInfo->n_ترتیبدهی_ستونی++;
  }
fix_به_بالا_عبارت:
  عبارت_مجموعه_VVAProperty(p_عبارت, EP_NoReduce);
  أكد( p_عبارت->pAggInfo==0 || p_عبارت->pAggInfo==pAggInfo );
  p_عبارت->pAggInfo = pAggInfo;
  إذا( p_عبارت->op==TK_ستونی ){
    p_عبارت->op = TK_AGG_ستونی;
  }
  أكد( k <= SMXV(p_عبارت->iAgg) );
  p_عبارت->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ساكن صحيح analyzeAggregate(Walker *pWalker, عبارت *p_عبارت){
  صحيح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->i_اولی_Reg==0 );
  بدل( p_عبارت->op ){
    افتراضي: {
      نمایهای_عبارت *pIEpr;
      عبارت tmp;
      أكد( pParse->iSelfTab==0 );
      إذا( (pNC->ncFlags & NC_درون_Agg_Func)==0 ) اكسر;
      إذا( pParse->pIdxEpr==0 ) اكسر;
      لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحيح i_داده_Cur = pIEpr->i_داده_Cur;
        إذا( i_داده_Cur<0 ) استمر;
        إذا( sqlite3_عبارت_Compare(0, p_عبارت, pIEpr->p_عبارت, i_داده_Cur)==0 ) اكسر;
      }
      إذا( pIEpr==0 ) اكسر;
      إذا( NEVER(!عبارت_Use_YTab(p_عبارت)) ) اكسر;
      لكل(i=0; i<pSrcList->nSrc; i++){
         إذا( pSrcList->a[i].iCursor==pIEpr->i_داده_Cur ){
           testcase( i>0 );
           اكسر;
         }
      }
      إذا( i>=pSrcList->nSrc ) اكسر;
      إذا( NEVER(p_عبارت->pAggInfo!=0) ) اكسر; /* Resolved by outer context */
      إذا( pParse->nErr ){ أرجع WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, حجم(tmp));
      tmp.op = TK_AGG_ستونی;
      tmp.i_جدول = pIEpr->iIdxCur;
      tmp.i_ستونی = pIEpr->i_Idx_ستون;
      پیدا_کن_Or_Create_Agg_Info_ستونی(pParse, pAggInfo, &tmp);
      إذا( pParse->nErr ){ أرجع WRC_Abort; }
      أكد( pAggInfo->a_ستون!=0 );
      أكد( tmp.iAgg<pAggInfo->n_ستونی );
      pAggInfo->a_ستون[tmp.iAgg].pCExpr = p_عبارت;
      p_عبارت->pAggInfo = pAggInfo;
      p_عبارت->iAgg = tmp.iAgg;
      أرجع WRC_Prune;
    }
    حالة TK_IF_NULL_ردیف:
    حالة TK_AGG_ستونی:
    حالة TK_ستونی: {
      testcase( p_عبارت->op==TK_AGG_ستونی );
      testcase( p_عبارت->op==TK_ستونی );
      testcase( p_عبارت->op==TK_IF_NULL_ردیف );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      إذا( ALWAYS(pSrcList!=0) ){
        Src_قلم *p_قلم = pSrcList->a;
        لكل(i=0; i<pSrcList->nSrc; i++, p_قلم++){
          أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
          إذا( p_عبارت->i_جدول==p_قلم->iCursor ){
            پیدا_کن_Or_Create_Agg_Info_ستونی(pParse, pAggInfo, p_عبارت);
            اكسر;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      أرجع WRC_Continue;
    }
    حالة TK_AGG_FUNCTION: {
      إذا( (pNC->ncFlags & NC_درون_Agg_Func)==0
       && pWalker->walker_عمق==p_عبارت->op2
       && p_عبارت->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        بنية AggInfo_func *p_قلم = pAggInfo->aFunc;
        صحيح mx_جمله = pParse->db->a_حد[SQLITE_حد_ستونی];
        أكد( mx_جمله <= SMXV(i16) );
        لكل(i=0; i<pAggInfo->nFunc; i++, p_قلم++){
          إذا( NEVER(p_قلم->pFExpr==p_عبارت) ) اكسر;
          إذا( sqlite3_عبارت_Compare(0, p_قلم->pFExpr, p_عبارت, -1)==0 ){
            اكسر;
          }
        }
        إذا( i>mx_جمله ){
          sqlite3_Error_پیام(pParse, "more than %d aggregate terms", mx_جمله);
          i = mx_جمله;
          أكد( i<pAggInfo->nFunc );
        }وإلا إذا( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = جمع_کن_Agg_Info_Func(pParse->db, pAggInfo);
          إذا( i>=0 ){
            صحيح nArg;
            أكد( !عبارت_Has_Property(p_عبارت, EP_xIsSelect) );
            p_قلم = &pAggInfo->aFunc[i];
            p_قلم->pFExpr = p_عبارت;
            أكد( عبارت_Use_UToken(p_عبارت) );
            nArg = p_عبارت->x.pList ? p_عبارت->x.pList->n_عبارت : 0;
            p_قلم->pFunc = sqlite3_پیدا_کن_Function(pParse->db,
                                         p_عبارت->u.zToken, nArg, enc, 0);
            أكد( p_قلم->bOBUnique==0 );
            إذا( p_عبارت->p_چپ
             && (p_قلم->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              عبارت_List *pOBList;
              أكد( nArg>0 );
              أكد( p_عبارت->p_چپ->op==TK_ترتیب );
              أكد( عبارت_Use_XList(p_عبارت->p_چپ) );
              p_قلم->iOBTab = pParse->nTab++;
              pOBList = p_عبارت->p_چپ->x.pList;
              أكد( pOBList->n_عبارت>0 );
              أكد( p_قلم->bOBUnique==0 );
              إذا( pOBList->n_عبارت==1
               && nArg==1
               && sqlite3_عبارت_Compare(0,pOBList->a[0].p_عبارت,
                               p_عبارت->x.pList->a[0].p_عبارت,-1)==0
              ){
                p_قلم->bOBPayload = 0;
                p_قلم->bOBUnique = عبارت_Has_Property(p_عبارت, EP_Distinct);
              }وإلا{
                p_قلم->bOBPayload = 1;
              }
              p_قلم->bUseSubtype =
                    (p_قلم->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }وإلا{
              p_قلم->iOBTab = -1;
            }
            إذا( عبارت_Has_Property(p_عبارت, EP_Distinct) && !p_قلم->bOBUnique ){
              p_قلم->iDistinct = pParse->nTab++;
            }وإلا{
              p_قلم->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        أكد( !عبارت_Has_Property(p_عبارت, EP_TokenOnly|EP_Reduced) );
        عبارت_مجموعه_VVAProperty(p_عبارت, EP_NoReduce);
        أكد( i <= SMXV(p_عبارت->iAgg) );
        p_عبارت->iAgg = (i16)i;
        p_عبارت->pAggInfo = pAggInfo;
        أرجع WRC_Prune;
      }وإلا{
        أرجع WRC_Continue;
      }
    }
  }
  أرجع WRC_Continue;
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
فراغ sqlite3_عبارت_Analyze_Aggregates(NameContext *pNC, عبارت *p_عبارت){
  Walker w;
  w.x_عبارت_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_عمق_Increase;
  w.xSelectCallback2 = sqlite3_Walker_عمق_Decrease;
  w.walker_عمق = 0;
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
فراغ sqlite3_عبارت_Analyze_Agg_List(NameContext *pNC, عبارت_List *pList){
  بنية عبارت_List_قلم *p_قلم;
  صحيح i;
  إذا( pList ){
    لكل(p_قلم=pList->a, i=0; i<pList->n_عبارت; i++, p_قلم++){
      sqlite3_عبارت_Analyze_Aggregates(pNC, p_قلم->p_عبارت);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحيح sqlite3_Get_موقت_Reg(Parse *pParse){
  إذا( pParse->n_موقت_Reg==0 ){
    أرجع ++pParse->nMem;
  }
  أرجع pParse->a_موقت_Reg[--pParse->n_موقت_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
فراغ sqlite3_Release_موقت_Reg(Parse *pParse, صحيح iReg){
  إذا( iReg ){
#اگر_تعریف_شده SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحيح kk;
    لكل(kk=0; kk<pParse->n_موقت_Reg; kk++){
      أكد( pParse->a_موقت_Reg[kk]!=iReg );
    }
#پایان_اگر
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    إذا( pParse->n_موقت_Reg<Array_اندازه(pParse->a_موقت_Reg) ){
      pParse->a_موقت_Reg[pParse->n_موقت_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحيح sqlite3_Get_موقت_Range(Parse *pParse, صحيح nReg){
  صحيح i, n;
  إذا( nReg==1 ) أرجع sqlite3_Get_موقت_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  إذا( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }وإلا{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  أرجع i;
}
فراغ sqlite3_Release_موقت_Range(Parse *pParse, صحيح iReg, صحيح nReg){
  إذا( nReg==1 ){
    sqlite3_Release_موقت_Reg(pParse, iReg);
    أرجع;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  إذا( nReg>pParse->nRangeReg ){
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
فراغ sqlite3_Clear_موقت_Reg_Cache(Parse *pParse){
  pParse->n_موقت_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
فراغ sqlite3_Touch_ثبات(Parse *pParse, صحيح iReg){
  إذا( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#إذا defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحيح sqlite3_اولی_موجود_ثبات(Parse *pParse, صحيح iMin){
  ثابت عبارت_List *pList = pParse->p_Const_عبارت;
  إذا( pList ){
    صحيح i;
    لكل(i=0; i<pList->n_عبارت; i++){
      إذا( pList->a[i].u.i_Const_عبارت_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_عبارت_Reg + 1;
      }
    }
  }
  pParse->n_موقت_Reg = 0;
  pParse->nRangeReg = 0;
  أرجع iMin;
}
#پایان_اگر /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#اگر_تعریف_شده SQLITE_DEBUG
صحيح sqlite3_No_Temps_درون_Range(Parse *pParse, صحيح i_اولی, صحيح i_آخرین){
  صحيح i;
  إذا( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_اولی
   && pParse->iRangeReg <= i_آخرین
  ){
     أرجع 0;
  }
  لكل(i=0; i<pParse->n_موقت_Reg; i++){
    إذا( pParse->a_موقت_Reg[i]>=i_اولی && pParse->a_موقت_Reg[i]<=i_آخرین ){
      أرجع 0;
    }
  }
  إذا( pParse->p_Const_عبارت ){
    عبارت_List *pList = pParse->p_Const_عبارت;
    لكل(i=0; i<pList->n_عبارت; i++){
      صحيح iReg = pList->a[i].u.i_Const_عبارت_Reg;
      إذا( iReg==0 ) استمر;
      إذا( iReg>=i_اولی && iReg<=i_آخرین ) أرجع 0;
    }
  }
  أرجع 1;
}
#پایان_اگر /* SQLITE_DEBUG */
