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
ساكن فراغ اوڠکاڤن_Code_Between(Parse*,اوڠکاڤن*,صحيح,فراغ(*)(Parse*,اوڠکاڤن*,صحيح,صحيح),صحيح);
ساكن صحيح اوڠکاڤن_Code_Vector(Parse *pParse, اوڠکاڤن *p, صحيح *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
حرف sqlite3TableColumnAffinity(ثابت Table *pTab, صحيح iCol){
  إذا( iCol<0 || NEVER(iCol>=pTab->nCol) ) أرجع SQLITE_AFF_INTEGER;
  أرجع pTab->aCol[iCol].affinity;
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
حرف sqlite3_اوڠکاڤن_Affinity(ثابت اوڠکاڤن *p_اوڠکاڤن){
  صحيح op;
  op = p_اوڠکاڤن->op;
  طالما( 1 /* exit-by-break */ ){
    إذا( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_اوڠکاڤن->y.pTab!=0) ){
      أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
      أكد( p_اوڠکاڤن->y.pTab!=0 );
      أرجع sqlite3TableColumnAffinity(p_اوڠکاڤن->y.pTab, p_اوڠکاڤن->iColumn);
    }
    إذا( op==TK_SELECT ){
      أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
      أكد( p_اوڠکاڤن->x.pSelect!=0 );
      أكد( p_اوڠکاڤن->x.pSelect->pEList!=0 );
      أكد( p_اوڠکاڤن->x.pSelect->pEList->a[0].p_اوڠکاڤن!=0 );
      أرجع sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن->x.pSelect->pEList->a[0].p_اوڠکاڤن);
    }
#ifndef SQLITE_OMIT_CAST
    إذا( op==TK_CAST ){
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      أرجع sqlite3AffinityType(p_اوڠکاڤن->u.zToken, 0);
    }
#endif
    إذا( op==TK_SELECT_COLUMN ){
      أكد( p_اوڠکاڤن->p_کيري!=0 && اوڠکاڤن_Use_XSelect(p_اوڠکاڤن->p_کيري) );
      أكد( p_اوڠکاڤن->iColumn < p_اوڠکاڤن->iTable );
      أكد( p_اوڠکاڤن->iColumn >= 0 );
      أكد( p_اوڠکاڤن->iTable==p_اوڠکاڤن->p_کيري->x.pSelect->pEList->n_اوڠکاڤن );
      أرجع sqlite3_اوڠکاڤن_Affinity(
          p_اوڠکاڤن->p_کيري->x.pSelect->pEList->a[p_اوڠکاڤن->iColumn].p_اوڠکاڤن
      );
    }
    إذا( op==TK_VECTOR
     || (op==TK_FUNCTION && p_اوڠکاڤن->aff_اوڠکاڤن==SQLITE_AFF_DEFER)
    ){
      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
      أرجع sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن);
    }
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Skip|EP_IfNullRow) ){
      أكد( p_اوڠکاڤن->op==TK_COLLATE
           || p_اوڠکاڤن->op==TK_IF_NULL_ROW
           || (p_اوڠکاڤن->op==TK_REGISTER && p_اوڠکاڤن->op2==TK_IF_NULL_ROW) );
      p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
      op = p_اوڠکاڤن->op;
      استمر;
    }
    إذا( op!=TK_REGISTER ) اكسر;
    op = p_اوڠکاڤن->op2;
    إذا( NEVER( op==TK_REGISTER ) ) اكسر;
  }
  أرجع p_اوڠکاڤن->aff_اوڠکاڤن;
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
صحيح sqlite3_اوڠکاڤن_Data_Type(ثابت اوڠکاڤن *p_اوڠکاڤن){
  طالما( p_اوڠکاڤن ){
    بدل( p_اوڠکاڤن->op ){
      حالة TK_COLLATE:
      حالة TK_IF_NULL_ROW:
      حالة TK_UPLUS:  {
        p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
        اكسر;
      }
      حالة TK_NULL: {
        p_اوڠکاڤن = 0;
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
      حالة TK_COLUMN:
      حالة TK_AGG_COLUMN:
      حالة TK_SELECT:
      حالة TK_CAST:
      حالة TK_SELECT_COLUMN:
      حالة TK_VECTOR:  {
        صحيح aff = sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن);
        إذا( aff>=SQLITE_AFF_NUMERIC ) أرجع 0x05;
        إذا( aff==SQLITE_AFF_TEXT )    أرجع 0x06;
        أرجع 0x07;
      }
      حالة TK_CASE: {
        صحيح res = 0;
        صحيح ii;
        اوڠکاڤن_List *pList = p_اوڠکاڤن->x.pList;
        أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) && pList!=0 );
        أكد( pList->n_اوڠکاڤن > 0);
        لكل(ii=1; ii<pList->n_اوڠکاڤن; ii+=2){
          res |= sqlite3_اوڠکاڤن_Data_Type(pList->a[ii].p_اوڠکاڤن);
        }
        إذا( pList->n_اوڠکاڤن % 2 ){
          res |= sqlite3_اوڠکاڤن_Data_Type(pList->a[pList->n_اوڠکاڤن-1].p_اوڠکاڤن);
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
اوڠکاڤن *sqlite3_اوڠکاڤن_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,             /* Add the "COLLATE" clause to this expression */
  ثابت Token *pCollName,  /* Name of collating sequence */
  صحيح dequote              /* True to dequote pCollName */
){
  إذا( pCollName->n>0 && pParse->nErr==0 ){
    اوڠکاڤن *pNew = sqlite3_اوڠکاڤن_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( p_اوڠکاڤن!=0 );
    إذا( pNew ){
      pNew->p_کيري = p_اوڠکاڤن;
      pNew->flags |= EP_Collate|EP_Skip;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
      pNew->nHeight = p_اوڠکاڤن->nHeight+1;
      sqlite3_اوڠکاڤن_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_اوڠکاڤن = pNew;
    }
  }
  أرجع p_اوڠکاڤن;
}
اوڠکاڤن *sqlite3_اوڠکاڤن_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,          /* Add the "COLLATE" clause to this expression */
  ثابت حرف *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (حرف*)zC);
  أرجع sqlite3_اوڠکاڤن_Add_Collate_Token(pParse, p_اوڠکاڤن, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
اوڠکاڤن *sqlite3_اوڠکاڤن_Skip_Collate(اوڠکاڤن *p_اوڠکاڤن){
  طالما( p_اوڠکاڤن && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Skip) ){
    أكد( p_اوڠکاڤن->op==TK_COLLATE );
    p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
  }  
  أرجع p_اوڠکاڤن;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
اوڠکاڤن *sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(اوڠکاڤن *p_اوڠکاڤن){
  طالما( p_اوڠکاڤن && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Skip|EP_Unlikely) ){
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Unlikely) ){
      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
      أكد( p_اوڠکاڤن->x.pList->n_اوڠکاڤن>0 );
      أكد( p_اوڠکاڤن->op==TK_FUNCTION );
      p_اوڠکاڤن = p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن;
    }وإلا إذا( p_اوڠکاڤن->op==TK_COLLATE ){
      p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
    }وإلا{
      اكسر;
    }
  }  
  أرجع p_اوڠکاڤن;
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
CollSeq *sqlite3_اوڠکاڤن_Coll_Seq(Parse *pParse, ثابت اوڠکاڤن *p_اوڠکاڤن){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ثابت اوڠکاڤن *p = p_اوڠکاڤن;
  طالما( p ){
    صحيح op = p->op;
    إذا( op==TK_REGISTER ) op = p->op2;
    إذا( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      صحيح j;
      أكد( اوڠکاڤن_Use_YTab(p) );
      أكد( p->y.pTab!=0 );
      إذا( (j = p->iColumn)>=0 ){
        ثابت حرف *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      اكسر;
    }
    إذا( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_کيري;
      استمر;
    }
    إذا( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_اوڠکاڤن==SQLITE_AFF_DEFER)
    ){
      أكد( اوڠکاڤن_Use_XList(p) );
      p = p->x.pList->a[0].p_اوڠکاڤن;
      استمر;
    }
    إذا( op==TK_COLLATE ){
      أكد( !اوڠکاڤن_Has_Property(p, EP_Int_نيلاي) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      اكسر;
    }
    إذا( p->flags & EP_Collate ){
      إذا( p->p_کيري && (p->p_کيري->flags & EP_Collate)!=0 ){
        p = p->p_کيري;
      }وإلا{
        اوڠکاڤن *pNext  = p->p_کانن;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !اوڠکاڤن_Use_XList(p) || p->x.pList==0 || p->p_کانن==0 );
        إذا( اوڠکاڤن_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحيح i;
          لكل(i=0; i<p->x.pList->n_اوڠکاڤن; i++){
            إذا( اوڠکاڤن_Has_Property(p->x.pList->a[i].p_اوڠکاڤن, EP_Collate) ){
              pNext = p->x.pList->a[i].p_اوڠکاڤن;
              اكسر;
            }
          }
        }
        p = pNext;
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
CollSeq *sqlite3_اوڠکاڤن_NNColl_Seq(Parse *pParse, ثابت اوڠکاڤن *p_اوڠکاڤن){
  CollSeq *p = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_اوڠکاڤن);
  إذا( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  أرجع p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحيح sqlite3_اوڠکاڤن_Coll_Seq_Match(Parse *pParse, ثابت اوڠکاڤن *pE1, ثابت اوڠکاڤن *pE2){
  CollSeq *pColl1 = sqlite3_اوڠکاڤن_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_اوڠکاڤن_NNColl_Seq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  أرجع pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
حرف sqlite3CompareAffinity(ثابت اوڠکاڤن *p_اوڠکاڤن, حرف aff2){
  حرف aff1 = sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن);
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
ساكن حرف comparisonAffinity(ثابت اوڠکاڤن *p_اوڠکاڤن){
  حرف aff;
  أكد( p_اوڠکاڤن->op==TK_EQ || p_اوڠکاڤن->op==TK_IN || p_اوڠکاڤن->op==TK_LT ||
          p_اوڠکاڤن->op==TK_GT || p_اوڠکاڤن->op==TK_GE || p_اوڠکاڤن->op==TK_LE ||
          p_اوڠکاڤن->op==TK_NE || p_اوڠکاڤن->op==TK_IS || p_اوڠکاڤن->op==TK_ISNOT );
  أكد( p_اوڠکاڤن->p_کيري );
  أكد( !اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
  aff = sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن->p_کيري);
  إذا( p_اوڠکاڤن->p_کانن ){
    aff = sqlite3CompareAffinity(p_اوڠکاڤن->p_کانن, aff);
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
صحيح sqlite3IndexAffinityOk(ثابت اوڠکاڤن *p_اوڠکاڤن, حرف idx_affinity){
  حرف aff = comparisonAffinity(p_اوڠکاڤن);
  إذا( aff<SQLITE_AFF_TEXT ){
    أرجع 1;
  }
  إذا( aff==SQLITE_AFF_TEXT ){
    أرجع idx_affinity==SQLITE_AFF_TEXT;
  }
  أرجع sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ساكن u8 بيناري_Compare_P5(
  ثابت اوڠکاڤن *pExpr1,   /* Left operand */
  ثابت اوڠکاڤن *pExpr2,   /* Right operand */
  صحيح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (حرف)sqlite3_اوڠکاڤن_Affinity(pExpr2);
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
CollSeq *sqlite3_بيناري_Compare_Coll_Seq(
  Parse *pParse,
  ثابت اوڠکاڤن *p_کيري,
  ثابت اوڠکاڤن *p_کانن
){
  CollSeq *pColl;
  أكد( p_کيري );
  إذا( p_کيري->flags & EP_Collate ){
    pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_کيري);
  }وإلا إذا( p_کانن && (p_کانن->flags & EP_Collate)!=0 ){
    pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_کانن);
  }وإلا{
    pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_کيري);
    إذا( !pColl ){
      pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_کانن);
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
CollSeq *sqlite3_اوڠکاڤن_Compare_Coll_Seq(Parse *pParse, ثابت اوڠکاڤن *p){
  إذا( اوڠکاڤن_Has_Property(p, EP_Commuted) ){
    أرجع sqlite3_بيناري_Compare_Coll_Seq(pParse, p->p_کانن, p->p_کيري);
  }وإلا{
    أرجع sqlite3_بيناري_Compare_Coll_Seq(pParse, p->p_کيري, p->p_کانن);
  }
}

/*
** Generate code for a comparison operator.
*/
ساكن صحيح codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  اوڠکاڤن *p_کيري,      /* The left operand */
  اوڠکاڤن *p_کانن,     /* The right operand */
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
    p4 = sqlite3_بيناري_Compare_Coll_Seq(pParse, p_کانن, p_کيري);
  }وإلا{
    p4 = sqlite3_بيناري_Compare_Coll_Seq(pParse, p_کيري, p_کانن);
  }
  p5 = بيناري_Compare_P5(p_کيري, p_کانن, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
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
صحيح sqlite3_اوڠکاڤن_Is_Vector(ثابت اوڠکاڤن *p_اوڠکاڤن){
  أرجع sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحيح sqlite3_اوڠکاڤن_Vector_Size(ثابت اوڠکاڤن *p_اوڠکاڤن){
  u8 op = p_اوڠکاڤن->op;
  إذا( op==TK_REGISTER ) op = p_اوڠکاڤن->op2;
  إذا( op==TK_VECTOR ){
    أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
    أرجع p_اوڠکاڤن->x.pList->n_اوڠکاڤن;
  }وإلا إذا( op==TK_SELECT ){
    أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
    أرجع p_اوڠکاڤن->x.pSelect->pEList->n_اوڠکاڤن;
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
اوڠکاڤن *sqlite3VectorFieldSubexpr(اوڠکاڤن *pVector, صحيح i){
  أكد( i<sqlite3_اوڠکاڤن_Vector_Size(pVector) || pVector->op==TK_ERROR );
  إذا( sqlite3_اوڠکاڤن_Is_Vector(pVector) ){
    أكد( pVector->op2==0 || pVector->op==TK_REGISTER );
    إذا( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      أكد( اوڠکاڤن_Use_XSelect(pVector) );
      أرجع pVector->x.pSelect->pEList->a[i].p_اوڠکاڤن;
    }وإلا{
      أكد( اوڠکاڤن_Use_XList(pVector) );
      أرجع pVector->x.pList->a[i].p_اوڠکاڤن;
    }
  }
  أرجع pVector;
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
اوڠکاڤن *sqlite3_اوڠکاڤن_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  اوڠکاڤن *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  صحيح iField,          /* Which column of the vector to return */
  صحيح nField           /* Total number of columns in the vector */
){
  اوڠکاڤن *pRet;
  إذا( pVector->op==TK_SELECT ){
    أكد( اوڠکاڤن_Use_XSelect(pVector) );
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
    إذا( pRet ){
      اوڠکاڤن_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_کيري = pVector;
    }
  }وإلا{
    إذا( pVector->op==TK_VECTOR ){
      اوڠکاڤن **ppVector;
      أكد( اوڠکاڤن_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_اوڠکاڤن;
      pVector = *ppVector;
      إذا( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        أرجع pVector;
      }
    }
    pRet = sqlite3_اوڠکاڤن_Dup(pParse->db, pVector, 0);
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
ساكن صحيح اوڠکاڤن_Code_Subselect(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن){
  صحيح reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  إذا( p_اوڠکاڤن->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_اوڠکاڤن);
  }
#endif
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
ساكن صحيح اوڠکاڤن_Vector_Register(
  Parse *pParse,                  /* Parse context */
  اوڠکاڤن *pVector,                  /* Vector to extract element from */
  صحيح iField,                     /* Field to extract from pVector */
  صحيح regSelect,                  /* First in array of registers */
  اوڠکاڤن *pTmp,                     /* Temporary space */
  اوڠکاڤن **pp_اوڠکاڤن,                  /* OUT: Expression element */
  صحيح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  أكد( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  إذا( op==TK_REGISTER ){
    *pp_اوڠکاڤن = sqlite3VectorFieldSubexpr(pVector, iField);
    أرجع pVector->iTable+iField;
  }
  إذا( op==TK_SELECT ){
    أكد( اوڠکاڤن_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, حجم(اوڠکاڤن));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_کيري = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_اوڠکاڤن;
    *pp_اوڠکاڤن = pTmp;
    أرجع regSelect+iField;
  }
  إذا( op==TK_VECTOR ){
    أكد( اوڠکاڤن_Use_XList(pVector) );
    *pp_اوڠکاڤن = pVector->x.pList->a[iField].p_اوڠکاڤن;
    أرجع sqlite3_اوڠکاڤن_Code_Temp(pParse, *pp_اوڠکاڤن, pRegFree);
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
ساكن فراغ codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  اوڠکاڤن *p_اوڠکاڤن,          /* The comparison operation */
  صحيح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
  اوڠکاڤن *p_کانن = p_اوڠکاڤن->p_کانن;
  صحيح n_کيري = sqlite3_اوڠکاڤن_Vector_Size(p_کيري);
  صحيح i;
  صحيح reg_کيري = 0;
  صحيح reg_کانن = 0;
  u8 opx = op;
  صحيح addrCmp = 0;
  صحيح addrDone = sqlite3VdbeMakeLabel(pParse);
  صحيح isCommuted = اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_Commuted);

  أكد( !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن,EP_Immutable) );
  إذا( pParse->nErr ) أرجع;
  إذا( n_کيري!=sqlite3_اوڠکاڤن_Vector_Size(p_کانن) ){
    sqlite3_Error_ڤسنن(pParse, "row value misused");
    أرجع;
  }
  أكد( p_اوڠکاڤن->op==TK_EQ || p_اوڠکاڤن->op==TK_NE
       || p_اوڠکاڤن->op==TK_IS || p_اوڠکاڤن->op==TK_ISNOT
       || p_اوڠکاڤن->op==TK_LT || p_اوڠکاڤن->op==TK_GT
       || p_اوڠکاڤن->op==TK_LE || p_اوڠکاڤن->op==TK_GE
  );
  أكد( p_اوڠکاڤن->op==op || (p_اوڠکاڤن->op==TK_IS && op==TK_EQ)
            || (p_اوڠکاڤن->op==TK_ISNOT && op==TK_NE) );
  أكد( p5==0 || p_اوڠکاڤن->op!=op );
  أكد( p5==SQLITE_NULLEQ || p_اوڠکاڤن->op==op );

  إذا( op==TK_LE ) opx = TK_LT;
  إذا( op==TK_GE ) opx = TK_GT;
  إذا( op==TK_NE ) opx = TK_EQ;

  reg_کيري = اوڠکاڤن_Code_Subselect(pParse, p_کيري);
  reg_کانن = اوڠکاڤن_Code_Subselect(pParse, p_کانن);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  لكل(i=0; 1 /*Loop exits by "break"*/; i++){
    صحيح regFree1 = 0, regFree2 = 0;
    اوڠکاڤن *pL = 0, *pR = 0;
    اوڠکاڤن tmp1, tmp2;
    صحيح r1, r2;
    أكد( i>=0 && i<n_کيري );
    إذا( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = اوڠکاڤن_Vector_Register(pParse, p_کيري, i, reg_کيري, &tmp1, &pL, &regFree1);
    r2 = اوڠکاڤن_Vector_Register(pParse, p_کانن, i, reg_کانن, &tmp2, &pR, &regFree2);
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
    إذا( (opx==TK_LT || opx==TK_GT) && i<n_کيري-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    إذا( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }وإلا{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    إذا( i==n_کيري-1 ){
      اكسر;
    }
    إذا( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }وإلا{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      إذا( i==n_کيري-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  إذا( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحيح sqlite3_اوڠکاڤن_Check_Height(Parse *pParse, صحيح nHeight){
  صحيح rc = SQLITE_OK;
  صحيح mxHeight = pParse->db->aLimit[SQLITE_LIMIT_اوڠکاڤن_DEPTH];
  إذا( nHeight>mxHeight ){
    sqlite3_Error_ڤسنن(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
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
ساكن فراغ height_Of_اوڠکاڤن(ثابت اوڠکاڤن *p, صحيح *pnHeight){
  إذا( p ){
    إذا( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
ساكن فراغ height_Of_اوڠکاڤن_List(ثابت اوڠکاڤن_List *p, صحيح *pnHeight){
  إذا( p ){
    صحيح i;
    لكل(i=0; i<p->n_اوڠکاڤن; i++){
      height_Of_اوڠکاڤن(p->a[i].p_اوڠکاڤن, pnHeight);
    }
  }
}
ساكن فراغ heightOfSelect(ثابت Select *pSelect, صحيح *pnHeight){
  ثابت Select *p;
  لكل(p=pSelect; p; p=p->pPrior){
    height_Of_اوڠکاڤن(p->pWhere, pnHeight);
    height_Of_اوڠکاڤن(p->pHaving, pnHeight);
    height_Of_اوڠکاڤن(p->pLimit, pnHeight);
    height_Of_اوڠکاڤن_List(p->pEList, pnHeight);
    height_Of_اوڠکاڤن_List(p->pGroupBy, pnHeight);
    height_Of_اوڠکاڤن_List(p->pOrderBy, pnHeight);
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
ساكن فراغ اوڠکاڤن_Set_Height(اوڠکاڤن *p){
  صحيح nHeight = p->p_کيري ? p->p_کيري->nHeight : 0;
  إذا( NEVER(p->p_کانن) && p->p_کانن->nHeight>nHeight ){
    nHeight = p->p_کانن->nHeight;
  }
  إذا( اوڠکاڤن_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }وإلا إذا( p->x.pList ){
    height_Of_اوڠکاڤن_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_اوڠکاڤن_List_Flags(p->x.pList);
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
فراغ sqlite3_اوڠکاڤن_Set_Height_And_Flags(Parse *pParse, اوڠکاڤن *p){
  إذا( pParse->nErr ) أرجع;
  اوڠکاڤن_Set_Height(p);
  sqlite3_اوڠکاڤن_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحيح sqlite3_Select_اوڠکاڤن_Height(ثابت Select *p){
  صحيح nHeight = 0;
  heightOfSelect(p, &nHeight);
  أرجع nHeight;
}
#وإلا /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3_اوڠکاڤن_Set_Height_And_Flags(Parse *pParse, اوڠکاڤن *p){
  إذا( pParse->nErr ) أرجع;
  إذا( p && اوڠکاڤن_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_اوڠکاڤن_List_Flags(p->x.pList);
  }
}
#define اوڠکاڤن_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
فراغ sqlite3_اوڠکاڤن_Set_Error_Offset(اوڠکاڤن *p_اوڠکاڤن, صحيح iOfst){
  إذا( p_اوڠکاڤن==0 ) أرجع;
  إذا( NEVER(اوڠکاڤن_Use_WJoin(p_اوڠکاڤن)) ) أرجع;
  p_اوڠکاڤن->w.iOfst = iOfst;
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
اوڠکاڤن *sqlite3_اوڠکاڤن_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  صحيح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحيح dequote             /* True to dequote */
){
  اوڠکاڤن *pNew;
  صحيح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, حجم(اوڠکاڤن)+nExtra);
  إذا( pNew ){
    memset(pNew, 0, حجم(اوڠکاڤن));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    إذا( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (حرف*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      إذا( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      إذا( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_اوڠکاڤن(pNew);
      }
    }
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  أرجع pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
اوڠکاڤن *sqlite3_اوڠکاڤن(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحيح op,                 /* Expression opcode */
  ثابت حرف *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  أرجع sqlite3_اوڠکاڤن_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
اوڠکاڤن *sqlite3_اوڠکاڤن_Int32(sqlite3 *db, صحيح iVal){
  اوڠکاڤن *pNew = sqlite3DbMallocRawNN(db, حجم(اوڠکاڤن));
  إذا( pNew ){
    memset(pNew, 0, حجم(اوڠکاڤن));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_نيلاي|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_نيلاي = iVal;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  أرجع pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
فراغ sqlite3_اوڠکاڤن_Attach_Subtrees(
  sqlite3 *db,
  اوڠکاڤن *p_اکر,
  اوڠکاڤن *p_کيري,
  اوڠکاڤن *p_کانن
){
  إذا( p_اکر==0 ){
    أكد( db->mallocFailed );
    sqlite3_اوڠکاڤن_Delete(db, p_کيري);
    sqlite3_اوڠکاڤن_Delete(db, p_کانن);
  }وإلا{
    أكد( اوڠکاڤن_Use_XList(p_اکر) );
    أكد( p_اکر->x.pSelect==0 );
    إذا( p_کانن ){
      p_اکر->p_کانن = p_کانن;
      p_اکر->flags |= EP_Propagate & p_کانن->flags;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
      p_اکر->nHeight = p_کانن->nHeight+1;
    }وإلا{
      p_اکر->nHeight = 1;
#endif
    }
    إذا( p_کيري ){
      p_اکر->p_کيري = p_کيري;
      p_اکر->flags |= EP_Propagate & p_کيري->flags;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
      إذا( p_کيري->nHeight>=p_اکر->nHeight ){
        p_اکر->nHeight = p_کيري->nHeight+1;
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
اوڠکاڤن *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  صحيح op,                 /* Expression opcode */
  اوڠکاڤن *p_کيري,            /* Left operand */
  اوڠکاڤن *p_کانن            /* Right operand */
){
  اوڠکاڤن *p;
  p = sqlite3DbMallocRawNN(pParse->db, حجم(اوڠکاڤن));
  إذا( p ){
    memset(p, 0, حجم(اوڠکاڤن));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_اوڠکاڤن_Attach_Subtrees(pParse->db, p, p_کيري, p_کانن);
    sqlite3_اوڠکاڤن_Check_Height(pParse, p->nHeight);
  }وإلا{
    sqlite3_اوڠکاڤن_Delete(pParse->db, p_کيري);
    sqlite3_اوڠکاڤن_Delete(pParse->db, p_کانن);
  }
  أرجع p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
فراغ sqlite3PExprAddSelect(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, Select *pSelect){
  إذا( p_اوڠکاڤن ){
    p_اوڠکاڤن->x.pSelect = pSelect;
    اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_xIsSelect|EP_Subquery);
    sqlite3_اوڠکاڤن_Set_Height_And_Flags(pParse, p_اوڠکاڤن);
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
Select *sqlite3_اوڠکاڤن_List_To_نيلاي_سموا(Parse *pParse, صحيح nElem, اوڠکاڤن_List *pEList){
  صحيح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  لكل(ii=0; ii<pEList->n_اوڠکاڤن; ii++){
    Select *pSel;
    اوڠکاڤن *p_اوڠکاڤن = pEList->a[ii].p_اوڠکاڤن;
    صحيح n_اوڠکاڤن_Elem;
    إذا( p_اوڠکاڤن->op==TK_VECTOR ){
      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
      n_اوڠکاڤن_Elem = p_اوڠکاڤن->x.pList->n_اوڠکاڤن;
    }وإلا{
      n_اوڠکاڤن_Elem = 1;
    }
    إذا( n_اوڠکاڤن_Elem!=nElem ){
      sqlite3_Error_ڤسنن(pParse, "IN(...) element has %d term%s - expected %d",
          n_اوڠکاڤن_Elem, n_اوڠکاڤن_Elem>1?"s":"", nElem
      );
      اكسر;
    }
    أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
    pSel = sqlite3SelectNew(pParse, p_اوڠکاڤن->x.pList, 0, 0, 0, 0, 0, SF_نيلاي_سموا,0);
    p_اوڠکاڤن->x.pList = 0;
    إذا( pSel ){
      إذا( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  إذا( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_نيلاي;
  }
  sqlite3_اوڠکاڤن_List_Delete(pParse->db, pEList);
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
اوڠکاڤن *sqlite3_اوڠکاڤن_And(Parse *pParse, اوڠکاڤن *p_کيري, اوڠکاڤن *p_کانن){
  sqlite3 *db = pParse->db;
  إذا( p_کيري==0  ){
    أرجع p_کانن;
  }وإلا إذا( p_کانن==0 ){
    أرجع p_کيري;
  }وإلا{
    u32 f = p_کيري->flags | p_کانن->flags;
    إذا( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      اوڠکاڤن *p;
      sqlite3_اوڠکاڤن_Deferred_Delete(pParse, p_کيري);
      sqlite3_اوڠکاڤن_Deferred_Delete(pParse, p_کانن);
      p = sqlite3_اوڠکاڤن_Int32(db, 0);
      إذا( p ) p->op2 = 1;
      أرجع p;
    }وإلا{
      أرجع sqlite3PExpr(pParse, TK_AND, p_کيري, p_کانن);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
اوڠکاڤن *sqlite3_اوڠکاڤن_Function(
  Parse *pParse,        /* Parsing context */
  اوڠکاڤن_List *pList,      /* Argument list */
  ثابت Token *pToken,  /* Name of the function */
  صحيح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  اوڠکاڤن *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3_اوڠکاڤن_Alloc(db, TK_FUNCTION, pToken, 1);
  إذا( pNew==0 ){
    sqlite3_اوڠکاڤن_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    أرجع 0;
  }
  أكد( !اوڠکاڤن_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحيح)(pToken->z - pParse->zTail);
  إذا( pList
   && pList->n_اوڠکاڤن > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_ڤسنن(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  اوڠکاڤن_Set_Property(pNew, EP_HasFunc);
  أكد( اوڠکاڤن_Use_XList(pNew) );
  sqlite3_اوڠکاڤن_Set_Height_And_Flags(pParse, pNew);
  إذا( eDistinct==SF_Distinct ) اوڠکاڤن_Set_Property(pNew, EP_Distinct);
  أرجع pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
فراغ sqlite3_اوڠکاڤن_Order_By_Aggregate_Error(Parse *pParse, اوڠکاڤن *p){
  sqlite3_Error_ڤسنن(pParse,
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
فراغ sqlite3_اوڠکاڤن_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,          /* The function call to which ORDER BY is to be added */
  اوڠکاڤن_List *pOrderBy    /* The ORDER BY clause to add */
){
  اوڠکاڤن *pOB;
  sqlite3 *db = pParse->db;
  إذا( NEVER(pOrderBy==0) ){
    أكد( db->mallocFailed );
    أرجع;
  }
  إذا( p_اوڠکاڤن==0 ){
    أكد( db->mallocFailed );
    sqlite3_اوڠکاڤن_List_Delete(db, pOrderBy);
    أرجع;
  }
  أكد( p_اوڠکاڤن->op==TK_FUNCTION );
  أكد( p_اوڠکاڤن->p_کيري==0 );
  أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
  إذا( p_اوڠکاڤن->x.pList==0 || NEVER(p_اوڠکاڤن->x.pList->n_اوڠکاڤن==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_اوڠکاڤن_List_Delete_Generic, pOrderBy);
    أرجع;
  }
  إذا( IsWindowFunc(p_اوڠکاڤن) ){
    sqlite3_اوڠکاڤن_Order_By_Aggregate_Error(pParse, p_اوڠکاڤن);
    sqlite3_اوڠکاڤن_List_Delete(db, pOrderBy);
    أرجع;
  }
  إذا( pOrderBy->n_اوڠکاڤن>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ڤسنن(pParse, "too many terms in ORDER BY clause");
    sqlite3_اوڠکاڤن_List_Delete(db, pOrderBy);
    أرجع;
  }

  pOB = sqlite3_اوڠکاڤن_Alloc(db, TK_ORDER, 0, 0);
  إذا( pOB==0 ){
    sqlite3_اوڠکاڤن_List_Delete(db, pOrderBy);
    أرجع;
  }
  pOB->x.pList = pOrderBy;
  أكد( اوڠکاڤن_Use_XList(pOB) );
  p_اوڠکاڤن->p_کيري = pOB;
  اوڠکاڤن_Set_Property(pOB, EP_FullSize);
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
فراغ sqlite3_اوڠکاڤن_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت اوڠکاڤن *p_اوڠکاڤن,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !IN_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    إذا( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_ڤسنن(pParse, "unsafe use of %#T()", p_اوڠکاڤن);
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
فراغ sqlite3_اوڠکاڤن_Assign_Var_نومبور(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, u32 n){
  sqlite3 *db = pParse->db;
  ثابت حرف *z;
  ynVar x;

  إذا( p_اوڠکاڤن==0 ) أرجع;
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي|EP_Reduced|EP_TokenOnly) );
  z = p_اوڠکاڤن->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  إذا( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }وإلا{
    صحيح doAdd = 0;
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
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_نومبور]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_نومبور] );
      إذا( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_نومبور] ){
        sqlite3_Error_ڤسنن(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_نومبور]);
        sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, p_اوڠکاڤن);
        أرجع;
      }
      x = (ynVar)i;
      إذا( x>pParse->nVar ){
        pParse->nVar = (صحيح)x;
        doAdd = 1;
      }وإلا إذا( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }وإلا إذا( sqlite3_VList_اڠک_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }وإلا{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_اڠک(pParse->pVList, z, n);
      إذا( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    إذا( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      إذا( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_اوڠکاڤن->iColumn = x;
  إذا( x>db->aLimit[SQLITE_LIMIT_VARIABLE_نومبور] ){
    sqlite3_Error_ڤسنن(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, p_اوڠکاڤن);
  }
}

/*
** Recursively delete an expression tree.
*/
ساكن SQLITE_NOINLINE فراغ sqlite3_اوڠکاڤن_Delete_NN(sqlite3 *db, اوڠکاڤن *p){
  أكد( p!=0 );
  أكد( db!=0 );
اوڠکاڤن_Delete_Restart:
  أكد( !اوڠکاڤن_Use_UValue(p) || p->u.i_نيلاي>=0 );
  أكد( !اوڠکاڤن_Use_YWin(p) || !اوڠکاڤن_Use_YSub(p) );
  أكد( !اوڠکاڤن_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !اوڠکاڤن_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  إذا( اوڠکاڤن_Has_Property(p, EP_Leaf) && !اوڠکاڤن_Has_Property(p, EP_TokenOnly) ){
    أكد( p->p_کيري==0 );
    أكد( p->p_کانن==0 );
    أكد( !اوڠکاڤن_Use_XSelect(p) || p->x.pSelect==0 );
    أكد( !اوڠکاڤن_Use_XList(p) || p->x.pList==0 );
  }
#endif
  إذا( !اوڠکاڤن_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (اوڠکاڤن_Use_XList(p) && p->x.pList==0) || p->p_کانن==0 );
    إذا( p->p_کانن ){
      أكد( !اوڠکاڤن_Has_Property(p, EP_WinFunc) );
      sqlite3_اوڠکاڤن_Delete_NN(db, p->p_کانن);
    }وإلا إذا( اوڠکاڤن_Use_XSelect(p) ){
      أكد( !اوڠکاڤن_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }وإلا{
      sqlite3_اوڠکاڤن_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      إذا( اوڠکاڤن_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    إذا( p->p_کيري && p->op!=TK_SELECT_COLUMN ){
      اوڠکاڤن *p_کيري = p->p_کيري;
      إذا( !اوڠکاڤن_Has_Property(p, EP_Static)
       && !اوڠکاڤن_Has_Property(p_کيري, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_کيري;
        اذهب اوڠکاڤن_Delete_Restart;
      }وإلا{
        sqlite3_اوڠکاڤن_Delete_NN(db, p_کيري);
      }
    }
  }
  إذا( !اوڠکاڤن_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
فراغ sqlite3_اوڠکاڤن_Delete(sqlite3 *db, اوڠکاڤن *p){
  إذا( p ) sqlite3_اوڠکاڤن_Delete_NN(db, p);
}
فراغ sqlite3_اوڠکاڤن_Delete_Generic(sqlite3 *db, فراغ *p){
  إذا( ALWAYS(p) ) sqlite3_اوڠکاڤن_Delete_NN(db, (اوڠکاڤن*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
فراغ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  إذا( p==0 ){
    /* Nothing to clear */
  }وإلا إذا( p->pOn ){
    sqlite3_اوڠکاڤن_Delete_NN(db, p->pOn);
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
صحيح sqlite3_اوڠکاڤن_Deferred_Delete(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن){
  أرجع 0==sqlite3ParserAddCleanup(pParse, sqlite3_اوڠکاڤن_Delete_Generic, p_اوڠکاڤن);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
فراغ sqlite3_اوڠکاڤن_Unmap_And_Delete(Parse *pParse, اوڠکاڤن *p){
  إذا( p ){
    إذا( IN_RENAME_OBJECT ){
      sqlite3_Rename_اوڠکاڤن_Unmap(pParse, p);
    }
    sqlite3_اوڠکاڤن_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ساكن صحيح اوڠکاڤن_Struct_Size(ثابت اوڠکاڤن *p){
  إذا( اوڠکاڤن_Has_Property(p, EP_TokenOnly) ) أرجع اوڠکاڤن_TOKENONLYSIZE;
  إذا( اوڠکاڤن_Has_Property(p, EP_Reduced) ) أرجع اوڠکاڤن_REDUCEDSIZE;
  أرجع اوڠکاڤن_FULLSIZE;
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
ساكن صحيح duped_اوڠکاڤن_Struct_Size(ثابت اوڠکاڤن *p, صحيح flags){
  صحيح nSize;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( اوڠکاڤن_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  إذا( 0==flags || اوڠکاڤن_Has_Property(p, EP_FullSize) ){
    nSize = اوڠکاڤن_FULLSIZE;
  }وإلا{
    أكد( !اوڠکاڤن_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    أكد( !اوڠکاڤن_Has_Property(p, EP_OuterON) );
    أكد( !اوڠکاڤن_Has_VVAProperty(p, EP_NoReduce) );
    إذا( p->p_کيري || p->x.pList ){
      nSize = اوڠکاڤن_REDUCEDSIZE | EP_Reduced;
    }وإلا{
      أكد( p->p_کانن==0 );
      nSize = اوڠکاڤن_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  أرجع nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ساكن صحيح duped_اوڠکاڤن_نود_Size(ثابت اوڠکاڤن *p, صحيح flags){
  صحيح nByte = duped_اوڠکاڤن_Struct_Size(p, flags) & 0xfff;
  إذا( !اوڠکاڤن_Has_Property(p, EP_Int_نيلاي) && p->u.zToken ){
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
ساكن صحيح duped_اوڠکاڤن_Size(ثابت اوڠکاڤن *p){
  صحيح nByte;
  أكد( p!=0 );
  nByte = duped_اوڠکاڤن_نود_Size(p, EXPRDUP_REDUCE);
  إذا( p->p_کيري ) nByte += duped_اوڠکاڤن_Size(p->p_کيري);
  إذا( p->p_کانن ) nByte += duped_اوڠکاڤن_Size(p->p_کانن);
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
ساكن اوڠکاڤن *اوڠکاڤن_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت اوڠکاڤن *p,        /* Expr tree to be duplicated */
  صحيح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  اوڠکاڤن *pNew;           /* Value to return */
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
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }وإلا{
    صحيح nAlloc;
    إذا( dupFlags ){
      nAlloc = duped_اوڠکاڤن_Size(p);
    }وإلا إذا( !اوڠکاڤن_Has_Property(p, EP_Int_نيلاي) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(اوڠکاڤن_FULLSIZE + nToken);
    }وإلا{
      nToken = 0;
      nAlloc = ROUND8(اوڠکاڤن_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (اوڠکاڤن *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  إذا( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت unsigned nStructSize = duped_اوڠکاڤن_Struct_Size(p, dupFlags);
    صحيح nNewSize = nStructSize & 0xfff;
    إذا( nToken<0 ){
      إذا( !اوڠکاڤن_Has_Property(p, EP_Int_نيلاي) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }وإلا{
        nToken = 0;
      }
    }
    إذا( dupFlags ){
      أكد( (صحيح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      أكد( اوڠکاڤن_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }وإلا{
      u32 nSize = (u32)اوڠکاڤن_Struct_Size(p);
      أكد( (صحيح)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (صحيح)اوڠکاڤن_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      إذا( nSize<اوڠکاڤن_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, اوڠکاڤن_FULLSIZE-nSize);
      }
      nNewSize = اوڠکاڤن_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    اوڠکاڤن_Clear_VVAProperties(pNew);
    إذا( dupFlags ){
      اوڠکاڤن_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    إذا( nToken>0 ){
      حرف *zToken = pNew->u.zToken = (حرف*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    إذا( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      إذا( اوڠکاڤن_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }وإلا{
        pNew->x.pList = sqlite3_اوڠکاڤن_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      إذا( اوڠکاڤن_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        أكد( اوڠکاڤن_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      إذا( dupFlags ){
        إذا( p->op==TK_SELECT_COLUMN ){
          pNew->p_کيري = p->p_کيري;
          أكد( p->p_کانن==0 
               || p->p_کانن==p->p_کيري
               || اوڠکاڤن_Has_Property(p->p_کيري, EP_Subquery) );
        }وإلا{
          pNew->p_کيري = p->p_کيري ?
                      اوڠکاڤن_Dup(db, p->p_کيري, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_کانن = p->p_کانن ?
                       اوڠکاڤن_Dup(db, p->p_کانن, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }وإلا{
        إذا( p->op==TK_SELECT_COLUMN ){
          pNew->p_کيري = p->p_کيري;
          أكد( p->p_کانن==0 
               || p->p_کانن==p->p_کيري
               || اوڠکاڤن_Has_Property(p->p_کيري, EP_Subquery) );
        }وإلا{
          pNew->p_کيري = sqlite3_اوڠکاڤن_Dup(db, p->p_کيري, 0);
        }
        pNew->p_کانن = sqlite3_اوڠکاڤن_Dup(db, p->p_کانن, 0);
      }
    }
  }
  إذا( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, حجم(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  أرجع pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
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
        pRet->a[i].pCols = sqlite3_اوڠکاڤن_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  أرجع pRet;
}
#وإلا
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ساكن صحيح gatherSelectWindowsCallback(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  إذا( p_اوڠکاڤن->op==TK_FUNCTION && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_اوڠکاڤن->y.pWin;
    أكد( pWin );
    أكد( IsWindowFunc(p_اوڠکاڤن) );
    أكد( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  أرجع WRC_Continue;
}
ساكن صحيح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  أرجع p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ساكن فراغ gatherSelectWindows(Select *p){
  Walker w;
  w.x_اوڠکاڤن_Callback = gatherSelectWindowsCallback;
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
اوڠکاڤن *sqlite3_اوڠکاڤن_Dup(sqlite3 *db, ثابت اوڠکاڤن *p, صحيح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  أرجع p ? اوڠکاڤن_Dup(db, p, flags, 0) : 0;
}
اوڠکاڤن_List *sqlite3_اوڠکاڤن_List_Dup(sqlite3 *db, ثابت اوڠکاڤن_List *p, صحيح flags){
  اوڠکاڤن_List *pNew;
  بنية اوڠکاڤن_List_item *pItem;
  ثابت بنية اوڠکاڤن_List_item *pOldItem;
  صحيح i;
  اوڠکاڤن *pPriorSelectColOld = 0;
  اوڠکاڤن *pPriorSelectColNew = 0;
  أكد( db!=0 );
  إذا( p==0 ) أرجع 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  إذا( pNew==0 ) أرجع 0;
  pNew->n_اوڠکاڤن = p->n_اوڠکاڤن;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  لكل(i=0; i<p->n_اوڠکاڤن; i++, pItem++, pOldItem++){
    اوڠکاڤن *p_Old_اوڠکاڤن = pOldItem->p_اوڠکاڤن;
    اوڠکاڤن *p_New_اوڠکاڤن;
    pItem->p_اوڠکاڤن = sqlite3_اوڠکاڤن_Dup(db, p_Old_اوڠکاڤن, flags);
    إذا( p_Old_اوڠکاڤن
     && p_Old_اوڠکاڤن->op==TK_SELECT_COLUMN
     && (p_New_اوڠکاڤن = pItem->p_اوڠکاڤن)!=0
    ){
      إذا( p_New_اوڠکاڤن->p_کانن ){
        pPriorSelectColOld = p_Old_اوڠکاڤن->p_کانن;
        pPriorSelectColNew = p_New_اوڠکاڤن->p_کانن;
        p_New_اوڠکاڤن->p_کيري = p_New_اوڠکاڤن->p_کانن;
      }وإلا{
        إذا( p_Old_اوڠکاڤن->p_کيري!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_اوڠکاڤن->p_کيري;
          pPriorSelectColNew = sqlite3_اوڠکاڤن_Dup(db, pPriorSelectColOld, flags);
          p_New_اوڠکاڤن->p_کانن = pPriorSelectColNew;
        }
        p_New_اوڠکاڤن->p_کيري = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
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
    SrcItem *pNewItem = &pNew->a[i];
    ثابت SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    إذا( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, حجم(Subquery));
      إذا( pNewSubq==0 ){
        أكد( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }وإلا{
        memcpy(pNewSubq, pOldItem->u4.pSubq, حجم(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        إذا( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }وإلا إذا( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }وإلا{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    إذا( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }وإلا إذا( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_اوڠکاڤن_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }وإلا{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    إذا( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    إذا( pTab ){
      pTab->nTabRef++;
    }
    إذا( pOldItem->fg.isUsing ){
      أكد( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }وإلا{
      pNewItem->u3.pOn = sqlite3_اوڠکاڤن_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
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
    بنية IdList_item *pNewItem = &pNew->a[i];
    ثابت بنية IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  أرجع pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحيح flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  لكل(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, حجم(*p) );
    إذا( pNew==0 ) اكسر;
    pNew->pEList = sqlite3_اوڠکاڤن_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_اوڠکاڤن_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_اوڠکاڤن_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_اوڠکاڤن_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_اوڠکاڤن_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_اوڠکاڤن_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    إذا( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    إذا( db->mallocFailed ){
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
  أرجع pRet;
}
#وإلا
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحيح flags){
  أكد( p==0 );
  أرجع 0;
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
ساكن ثابت بنية اوڠکاڤن_List_item zeroItem = {0};
SQLITE_NOINLINE اوڠکاڤن_List *sqlite3_اوڠکاڤن_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اوڠکاڤن *p_اوڠکاڤن             /* Expression to be appended. Might be NULL */
){
  بنية اوڠکاڤن_List_item *pItem;
  اوڠکاڤن_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  إذا( pList==0 ){
    sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن);
    أرجع 0;
  }
  pList->nAlloc = 4;
  pList->n_اوڠکاڤن = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_اوڠکاڤن = p_اوڠکاڤن;
  أرجع pList;
}
SQLITE_NOINLINE اوڠکاڤن_List *sqlite3_اوڠکاڤن_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اوڠکاڤن_List *pList,        /* List to which to append. Might be NULL */
  اوڠکاڤن *p_اوڠکاڤن             /* Expression to be appended. Might be NULL */
){
  بنية اوڠکاڤن_List_item *pItem;
  اوڠکاڤن_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  إذا( pNew==0 ){
    sqlite3_اوڠکاڤن_List_Delete(db, pList);
    sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن);
    أرجع 0;
  }وإلا{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_اوڠکاڤن++];
  *pItem = zeroItem;
  pItem->p_اوڠکاڤن = p_اوڠکاڤن;
  أرجع pList;
}
اوڠکاڤن_List *sqlite3_اوڠکاڤن_List_Append(
  Parse *pParse,          /* Parsing context */
  اوڠکاڤن_List *pList,        /* List to which to append. Might be NULL */
  اوڠکاڤن *p_اوڠکاڤن             /* Expression to be appended. Might be NULL */
){
  بنية اوڠکاڤن_List_item *pItem;
  إذا( pList==0 ){
    أرجع sqlite3_اوڠکاڤن_List_Append_New(pParse->db,p_اوڠکاڤن);
  }
  إذا( pList->nAlloc<pList->n_اوڠکاڤن+1 ){
    أرجع sqlite3_اوڠکاڤن_List_Append_Grow(pParse->db,pList,p_اوڠکاڤن);
  }
  pItem = &pList->a[pList->n_اوڠکاڤن++];
  *pItem = zeroItem;
  pItem->p_اوڠکاڤن = p_اوڠکاڤن;
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
اوڠکاڤن_List *sqlite3_اوڠکاڤن_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  اوڠکاڤن_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  اوڠکاڤن *p_اوڠکاڤن            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  صحيح n;
  صحيح i;
  صحيح iFirst = pList ? pList->n_اوڠکاڤن : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  إذا( NEVER(pColumns==0) ) اذهب vector_append_error;
  إذا( p_اوڠکاڤن==0 ) اذهب vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  إذا( p_اوڠکاڤن->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن)) ){
    sqlite3_Error_ڤسنن(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    اذهب vector_append_error;
  }

  لكل(i=0; i<pColumns->nId; i++){
    اوڠکاڤن *p_Sub_اوڠکاڤن = sqlite3_اوڠکاڤن_For_Vector_Field(pParse, p_اوڠکاڤن, i, pColumns->nId);
    أكد( p_Sub_اوڠکاڤن!=0 || db->mallocFailed );
    إذا( p_Sub_اوڠکاڤن==0 ) استمر;
    pList = sqlite3_اوڠکاڤن_List_Append(pParse, pList, p_Sub_اوڠکاڤن);
    إذا( pList ){
      أكد( pList->n_اوڠکاڤن==iFirst+i+1 );
      pList->a[pList->n_اوڠکاڤن-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  إذا( !db->mallocFailed && p_اوڠکاڤن->op==TK_SELECT && ALWAYS(pList!=0) ){
    اوڠکاڤن *pFirst = pList->a[iFirst].p_اوڠکاڤن;
    أكد( pFirst!=0 );
    أكد( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_کانن = p_اوڠکاڤن;
    p_اوڠکاڤن = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_اوڠکاڤن_Unmap_And_Delete(pParse, p_اوڠکاڤن);
  sqlite3IdListDelete(db, pColumns);
  أرجع pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
فراغ sqlite3_اوڠکاڤن_List_Set_Sort_Order(اوڠکاڤن_List *p, صحيح iSortOrder, صحيح eNulls){
  بنية اوڠکاڤن_List_item *pItem;
  إذا( p==0 ) أرجع;
  أكد( p->n_اوڠکاڤن>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_اوڠکاڤن-1];
  أكد( pItem->fg.bNulls==0 );
  إذا( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  إذا( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    إذا( iSortOrder!=eNulls ){
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
فراغ sqlite3_اوڠکاڤن_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  اوڠکاڤن_List *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحيح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  إذا( pList ){
    بنية اوڠکاڤن_List_item *pItem;
    أكد( pList->n_اوڠکاڤن>0 );
    pItem = &pList->a[pList->n_اوڠکاڤن-1];
    أكد( pItem->zEName==0 );
    أكد( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    إذا( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      إذا( IN_RENAME_OBJECT ){
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
فراغ sqlite3_اوڠکاڤن_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  اوڠکاڤن_List *pList,        /* List to which to add the span. */
  ثابت حرف *zStart,     /* Start of the span */
  ثابت حرف *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  إذا( pList ){
    بنية اوڠکاڤن_List_item *pItem = &pList->a[pList->n_اوڠکاڤن-1];
    أكد( pList->n_اوڠکاڤن>0 );
    إذا( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
فراغ sqlite3_اوڠکاڤن_List_Check_Length(
  Parse *pParse,
  اوڠکاڤن_List *pEList,
  ثابت حرف *zObject
){
  صحيح mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_اوڠکاڤن==mx );
  testcase( pEList && pEList->n_اوڠکاڤن==mx+1 );
  إذا( pEList && pEList->n_اوڠکاڤن>mx ){
    sqlite3_Error_ڤسنن(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ساكن SQLITE_NOINLINE فراغ اوڠکاڤن_List_Delete_NN(sqlite3 *db, اوڠکاڤن_List *pList){
  صحيح i = pList->n_اوڠکاڤن;
  بنية اوڠکاڤن_List_item *pItem =  pList->a;
  أكد( pList->n_اوڠکاڤن>0 );
  أكد( db!=0 );
  افعل{
    sqlite3_اوڠکاڤن_Delete(db, pItem->p_اوڠکاڤن);
    إذا( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }طالما( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
فراغ sqlite3_اوڠکاڤن_List_Delete(sqlite3 *db, اوڠکاڤن_List *pList){
  إذا( pList ) اوڠکاڤن_List_Delete_NN(db, pList);
}
فراغ sqlite3_اوڠکاڤن_List_Delete_Generic(sqlite3 *db, فراغ *pList){
  إذا( ALWAYS(pList) ) اوڠکاڤن_List_Delete_NN(db, (اوڠکاڤن_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_اوڠکاڤن_List_Flags(ثابت اوڠکاڤن_List *pList){
  صحيح i;
  u32 m = 0;
  أكد( pList!=0 );
  لكل(i=0; i<pList->n_اوڠکاڤن; i++){
     اوڠکاڤن *p_اوڠکاڤن = pList->a[i].p_اوڠکاڤن;
     أكد( p_اوڠکاڤن!=0 );
     m |= p_اوڠکاڤن->flags;
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
صحيح sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
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
u32 sqlite3IsTrueOrFalse(ثابت حرف *zIn){
  إذا( sqlite3StrICmp(zIn, "true")==0  ) أرجع EP_IsTrue;
  إذا( sqlite3StrICmp(zIn, "false")==0 ) أرجع EP_IsFalse;
  أرجع 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحيح sqlite3_اوڠکاڤن_Id_To_True_False(اوڠکاڤن *p_اوڠکاڤن){
  u32 v;
  أكد( p_اوڠکاڤن->op==TK_ID || p_اوڠکاڤن->op==TK_STRING );
  إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Quoted|EP_Int_نيلاي)
   && (v = sqlite3IsTrueOrFalse(p_اوڠکاڤن->u.zToken))!=0
  ){
    p_اوڠکاڤن->op = TK_TRUEFALSE;
    اوڠکاڤن_Set_Property(p_اوڠکاڤن, v);
    أرجع 1;
  }
  أرجع 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحيح sqlite3_اوڠکاڤن_Truth_نيلاي(ثابت اوڠکاڤن *p_اوڠکاڤن){
  p_اوڠکاڤن = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely((اوڠکاڤن*)p_اوڠکاڤن);
  أكد( p_اوڠکاڤن->op==TK_TRUEFALSE );
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
  أكد( sqlite3StrICmp(p_اوڠکاڤن->u.zToken,"true")==0
       || sqlite3StrICmp(p_اوڠکاڤن->u.zToken,"false")==0 );
  أرجع p_اوڠکاڤن->u.zToken[4]==0;
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
اوڠکاڤن *sqlite3_اوڠکاڤن_Simplified_And_Or(اوڠکاڤن *p_اوڠکاڤن){
  أكد( p_اوڠکاڤن!=0 );
  إذا( p_اوڠکاڤن->op==TK_AND || p_اوڠکاڤن->op==TK_OR ){
    اوڠکاڤن *p_کانن = sqlite3_اوڠکاڤن_Simplified_And_Or(p_اوڠکاڤن->p_کانن);
    اوڠکاڤن *p_کيري = sqlite3_اوڠکاڤن_Simplified_And_Or(p_اوڠکاڤن->p_کيري);
    إذا( اوڠکاڤن_Always_True(p_کيري) || اوڠکاڤن_Always_False(p_کانن) ){
      p_اوڠکاڤن = p_اوڠکاڤن->op==TK_AND ? p_کانن : p_کيري;
    }وإلا إذا( اوڠکاڤن_Always_True(p_کانن) || اوڠکاڤن_Always_False(p_کيري) ){
      p_اوڠکاڤن = p_اوڠکاڤن->op==TK_AND ? p_کيري : p_کانن;
    }
  }
  أرجع p_اوڠکاڤن;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ساكن صحيح اوڠکاڤن_Eval_Rhs_First(اوڠکاڤن *p_اوڠکاڤن){
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن->p_کيري, EP_Subquery)
   && !اوڠکاڤن_Has_Property(p_اوڠکاڤن->p_کانن, EP_Subquery)
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
ساكن صحيح اوڠکاڤن_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,       /* The comparison expression */
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
  إذا( اوڠکاڤن_Eval_Rhs_First(p_اوڠکاڤن) && sqlite3_اوڠکاڤن_Can_Be_Null(p_اوڠکاڤن->p_کانن) ){
    r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }وإلا{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, pFree1);
  إذا( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن->p_کانن, EP_Subquery)
     && sqlite3_اوڠکاڤن_Can_Be_Null(p_اوڠکاڤن->p_کيري)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, pFree2);
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
ساكن SQLITE_NOINLINE صحيح اوڠکاڤن_نود_Is_Constant_Function(
  Walker *pWalker,
  اوڠکاڤن *p_اوڠکاڤن
){
  صحيح n;             /* Number of arguments */
  اوڠکاڤن_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( p_اوڠکاڤن->op==TK_FUNCTION );
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly)
   || (pList = p_اوڠکاڤن->x.pList)==0
  ){;
    n = 0;
  }وإلا{
    n = pList->n_اوڠکاڤن;
    sqlite3_Walk_اوڠکاڤن_List(pWalker, pList);
    إذا( pWalker->eCode==0 ) أرجع WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_اوڠکاڤن->u.zToken, n, ENC(db), 0);
  إذا( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc))
  ){
    pWalker->eCode = 0;
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
ساكن صحيح اوڠکاڤن_نود_Is_Constant(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  أكد( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  إذا( pWalker->eCode==2 && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON) ){
    pWalker->eCode = 0;
    أرجع WRC_Abort;
  }

  بدل( p_اوڠکاڤن->op ){
    حالة TK_FUNCTION:
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        أرجع WRC_Abort;
      }وإلا إذا( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        إذا( pWalker->eCode==5 ) اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_FromDDL);
        أرجع WRC_Continue;
      }وإلا إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        أرجع WRC_Continue;
      }وإلا إذا( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        أرجع اوڠکاڤن_نود_Is_Constant_Function(pWalker, p_اوڠکاڤن);
      }وإلا{
        pWalker->eCode = 0;  /* Function call is not constant */
        أرجع WRC_Abort;
      }
    حالة TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      إذا( sqlite3_اوڠکاڤن_Id_To_True_False(p_اوڠکاڤن) ){
        أرجع WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالة TK_COLUMN:
    حالة TK_AGG_FUNCTION:
    حالة TK_AGG_COLUMN:
      testcase( p_اوڠکاڤن->op==TK_ID );
      testcase( p_اوڠکاڤن->op==TK_COLUMN );
      testcase( p_اوڠکاڤن->op==TK_AGG_FUNCTION );
      testcase( p_اوڠکاڤن->op==TK_AGG_COLUMN );
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_FixedCol) && pWalker->eCode!=2 ){
        أرجع WRC_Continue;
      }
      إذا( pWalker->eCode==3 && p_اوڠکاڤن->iTable==pWalker->u.iCur ){
        أرجع WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالة TK_IF_NULL_ROW:
    حالة TK_REGISTER:
    حالة TK_DOT:
    حالة TK_RAISE:
      testcase( p_اوڠکاڤن->op==TK_REGISTER );
      testcase( p_اوڠکاڤن->op==TK_IF_NULL_ROW );
      testcase( p_اوڠکاڤن->op==TK_DOT );
      testcase( p_اوڠکاڤن->op==TK_RAISE );
      pWalker->eCode = 0;
      أرجع WRC_Abort;
    حالة TK_IS:
    حالة TK_ISNOT:
      إذا( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        أرجع WRC_Abort;
      }
      أرجع WRC_Continue;
    حالة TK_VARIABLE:
      إذا( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_اوڠکاڤن->op = TK_NULL;
      }وإلا إذا( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        أرجع WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    افتراضي:
      testcase( p_اوڠکاڤن->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_اوڠکاڤن->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      أرجع WRC_Continue;
  }
}
ساكن صحيح اوڠکاڤن_Is_Const(Parse *pParse, اوڠکاڤن *p, صحيح initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_نود_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_اوڠکاڤن(&w, p);
  أرجع w.eCode;
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
صحيح sqlite3_اوڠکاڤن_Is_Constant(Parse *pParse, اوڠکاڤن *p){
  أرجع اوڠکاڤن_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحيح sqlite3_اوڠکاڤن_List_Is_Constant(Parse *pParse, اوڠکاڤن_List *pList, صحيح bNoIs){
  ثابت صحيح iInit = bNoIs ? 6 : 1;
  صحيح ii;
  لكل(ii=0; ii<pList->n_اوڠکاڤن; ii++){
    إذا( 0==اوڠکاڤن_Is_Const(pParse, pList->a[ii].p_اوڠکاڤن, iInit) ) أرجع 0;
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
ساكن صحيح sqlite3_اوڠکاڤن_Is_Constant_Not_Join(Parse *pParse, اوڠکاڤن *p){
  أرجع اوڠکاڤن_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ساكن صحيح اوڠکاڤن_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->eCode==3 || pWalker->eCode==0 );
  إذا( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
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
صحيح sqlite3_اوڠکاڤن_Is_Table_Constant(اوڠکاڤن *p, صحيح iCur, صحيح bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_نود_Is_Constant;
  إذا( bAllowSubq ){
    w.xSelectCallback = اوڠکاڤن_Select_Walk_Table_Constant;
  }وإلا{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_اوڠکاڤن(&w, p);
  أرجع w.eCode;
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
صحيح sqlite3_اوڠکاڤن_Is_Single_Table_Constraint(
  اوڠکاڤن *p_اوڠکاڤن,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحيح iSrc,                    /* Which element of pSrcList to use */
  صحيح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت SrcItem *pSrc = &pSrcList->a[iSrc];
  إذا( pSrc->fg.jointype & JT_LTORJ ){
    أرجع 0;  /* rule (3) */
  }
  إذا( pSrc->fg.jointype & JT_کيري ){
    إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON) ) أرجع 0;   /* rule (4a) */
    إذا( p_اوڠکاڤن->w.iJoin!=pSrc->iCursor ) أرجع 0;         /* rule (4b) */
  }وإلا{
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON) ) أرجع 0;    /* rule (5) */
  }
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      إذا( p_اوڠکاڤن->w.iJoin==pSrcList->a[jj].iCursor ){
        إذا( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          أرجع 0;  /* restriction (6) */
        }
        اكسر;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  أرجع sqlite3_اوڠکاڤن_Is_Table_Constant(p_اوڠکاڤن, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ساكن صحيح اوڠکاڤن_نود_Is_Constant_Or_Group_By(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  اوڠکاڤن_List *pGroupBy = pWalker->u.pGroupBy;
  صحيح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  لكل(i=0; i<pGroupBy->n_اوڠکاڤن; i++){
    اوڠکاڤن *p = pGroupBy->a[i].p_اوڠکاڤن;
    إذا( sqlite3_اوڠکاڤن_Compare(0, p_اوڠکاڤن, p, -1)<2 ){
      CollSeq *pColl = sqlite3_اوڠکاڤن_NNColl_Seq(pWalker->pParse, p);
      إذا( sqlite3_Is_بيناري(pColl) ){
        أرجع WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
    pWalker->eCode = 0;
    أرجع WRC_Abort;
  }

  أرجع اوڠکاڤن_نود_Is_Constant(pWalker, p_اوڠکاڤن);
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
صحيح sqlite3_اوڠکاڤن_Is_Constant_Or_Group_By(Parse *pParse, اوڠکاڤن *p, اوڠکاڤن_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_نود_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_اوڠکاڤن(&w, p);
  أرجع w.eCode;
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
صحيح sqlite3_اوڠکاڤن_Is_Constant_Or_Function(اوڠکاڤن *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  أرجع اوڠکاڤن_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحيح sqlite3_اوڠکاڤن_Contains_Subquery(اوڠکاڤن *p){
  Walker w;
  w.eCode = 1;
  w.x_اوڠکاڤن_Callback = sqlite3_اوڠکاڤن_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_اوڠکاڤن(&w, p);
  أرجع w.eCode==0;
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
صحيح sqlite3_اوڠکاڤن_Is_Integer(ثابت اوڠکاڤن *p, صحيح *p_نيلاي, Parse *pParse, صحيح bRSI){
  صحيح iSign = 1;   /* Either +1 or -1. */
  طالما( 1/*exit-by-break*/ ){
    إذا( NEVER(p==0) ) أرجع 0;
    إذا( اوڠکاڤن_Use_UValue(p) ){
      أكد( p->op==TK_INTEGER );
      *p_نيلاي = p->u.i_نيلاي*iSign;
      إذا( bRSI && pParse==0 && p->op2 ){
        أرجع 0;
      }
      أرجع 1;
    }
    إذا( p->op==TK_UPLUS ){
      p = p->p_کيري;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_کيري;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_نيلاي *pVal; /* The variable */
      صحيح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      إذا( NEVER(pParse->pVdbe==0) ) اكسر;
      إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) اكسر;
      pVal = sqlite3_Vdbe_Get_Bound_نيلاي(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      إذا( pVal ){
        i64 vv;
        إذا( sqlite3_نيلاي_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_نيلاي_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_نيلاي = (صحيح)vv;
          rc = 1;       
        }
        sqlite3_نيلاي_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
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
صحيح sqlite3_اوڠکاڤن_Can_Be_Null(ثابت اوڠکاڤن *p){
  u8 op;
  أكد( p!=0 );
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_کيري;
    أكد( p!=0 );
  }
  op = p->op;
  إذا( op==TK_REGISTER ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER:
    حالة TK_STRING:
    حالة TK_FLOAT:
    حالة TK_BLOB:
      أرجع 0;
    حالة TK_COLUMN:
      أكد( اوڠکاڤن_Use_YTab(p) );
      أرجع اوڠکاڤن_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
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
صحيح sqlite3_اوڠکاڤن_Needs_No_Affinity_Change(ثابت اوڠکاڤن *p, حرف aff){
  u8 op;
  صحيح unaryMinus = 0;
  إذا( aff==SQLITE_AFF_BLOB ) أرجع 1;
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    إذا( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_کيري;
  }
  op = p->op;
  إذا( op==TK_REGISTER ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER: {
      أرجع aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_FLOAT: {
      أرجع aff>=SQLITE_AFF_NUMERIC;
    }
    حالة TK_STRING: {
      أرجع !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    حالة TK_BLOB: {
      أرجع !unaryMinus;
    }
    حالة TK_COLUMN: {
      أكد( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      أرجع aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
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
ثابت حرف *sqlite3RowidAlias(Table *pTab){
  ثابت حرف *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  صحيح ii;
  أكد( VisibleRowid(pTab) );
  لكل(ii=0; ii<ArraySize(azOpt); ii++){
    إذا( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) أرجع azOpt[ii];
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
#ifndef SQLITE_OMIT_SUBQUERY
ساكن Select *isCandidateForInOpt(ثابت اوڠکاڤن *pX){
  Select *p;
  SrcList *pSrc;
  اوڠکاڤن_List *pEList;
  Table *pTab;
  صحيح i;
  إذا( !اوڠکاڤن_Use_XSelect(pX) ) أرجع 0;                 /* Not a subquery */
  إذا( اوڠکاڤن_Has_Property(pX, EP_VarSelect)  ) أرجع 0;  /* Correlated subq */
  p = pX->x.pSelect;
  إذا( p->pPrior ) أرجع 0;              /* Not a compound SELECT */
  إذا( p->selFlags & SF_Aggregate ){
    أرجع 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  إذا( p->pLimit ) أرجع 0;              /* Has no LIMIT clause */
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
  لكل(i=0; i<pEList->n_اوڠکاڤن; i++){
    اوڠکاڤن *pRes = pEList->a[i].p_اوڠکاڤن;
    إذا( pRes->op!=TK_COLUMN ) أرجع 0;
    أكد( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  أرجع p;
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
  إذا( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }وإلا{
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
ساكن صحيح sqlite3InRhsIsConstant(Parse *pParse, اوڠکاڤن *pIn){
  اوڠکاڤن *pLHS;
  صحيح res;
  أكد( !اوڠکاڤن_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_کيري;
  pIn->p_کيري = 0;
  res = sqlite3_اوڠکاڤن_Is_Constant(pParse, pIn);
  pIn->p_کيري = pLHS;
  أرجع res;
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
  اوڠکاڤن *pX,                  /* The IN expression */
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
  إذا( prRhsHasNull && اوڠکاڤن_Use_XSelect(pX) ){
    صحيح i;
    اوڠکاڤن_List *pEList = pX->x.pSelect->pEList;
    لكل(i=0; i<pEList->n_اوڠکاڤن; i++){
      إذا( sqlite3_اوڠکاڤن_Can_Be_Null(pEList->a[i].p_اوڠکاڤن) ) اكسر;
    }
    إذا( i==pEList->n_اوڠکاڤن ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  إذا( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    صحيح iDb;                               /* Database idx for pTab */
    اوڠکاڤن_List *pEList = p->pEList;
    صحيح n_اوڠکاڤن = pEList->n_اوڠکاڤن;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].p_اوڠکاڤن!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    إذا( n_اوڠکاڤن==1 && pEList->a[0].p_اوڠکاڤن->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحيح iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }وإلا{
      Index *pIdx;                         /* Iterator variable */
      صحيح affinity_ok = 1;
      صحيح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      لكل(i=0; i<n_اوڠکاڤن && affinity_ok; i++){
        اوڠکاڤن *pLhs = sqlite3VectorFieldSubexpr(pX->p_کيري, i);
        صحيح iCol = pEList->a[i].p_اوڠکاڤن->iColumn;
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

      إذا( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        لكل(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          إذا( pIdx->nColumn<n_اوڠکاڤن ) استمر;
          إذا( pIdx->pPartIdxWhere!=0 ) استمر;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          إذا( pIdx->nColumn>=BMS-1 ) استمر;
          إذا( mustBeUnique ){
            إذا( pIdx->n_کونچي_Col>n_اوڠکاڤن
             ||(pIdx->nColumn>n_اوڠکاڤن && !IsUniqueIndex(pIdx))
            ){
              استمر;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          لكل(i=0; i<n_اوڠکاڤن; i++){
            اوڠکاڤن *pLhs = sqlite3VectorFieldSubexpr(pX->p_کيري, i);
            اوڠکاڤن *pRhs = pEList->a[i].p_اوڠکاڤن;
            CollSeq *pReq = sqlite3_بيناري_Compare_Coll_Seq(pParse, pLhs, pRhs);
            صحيح j;
 
            لكل(j=0; j<n_اوڠکاڤن; j++){
              إذا( pIdx->aiColumn[j]!=pRhs->iColumn ) استمر;
              أكد( pIdx->azColl[j] );
              إذا( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                استمر;
              }
              اكسر;
            }
            إذا( j==n_اوڠکاڤن ) اكسر;
            mCol = MASKBIT(j);
            إذا( mCol & colUsed ) اكسر; /* Each column used only once */
            colUsed |= mCol;
            إذا( aiMap ) aiMap[i] = j;
          }
 
          أكد( n_اوڠکاڤن>0 && n_اوڠکاڤن<BMS );
          أكد( i==n_اوڠکاڤن || colUsed!=(MASKBIT(n_اوڠکاڤن)-1) );
          إذا( colUsed==(MASKBIT(n_اوڠکاڤن)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحيح iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_کونچي_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            إذا( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_اوڠکاڤن)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              إذا( n_اوڠکاڤن==1 ){
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
  إذا( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && اوڠکاڤن_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_اوڠکاڤن<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  إذا( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحيح rMayHaveNull = 0;
    صحيح bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    إذا( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }وإلا إذا( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_IN );
    إذا( !bloomOk
     && اوڠکاڤن_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    إذا( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  إذا( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    صحيح i, n;
    n = sqlite3_اوڠکاڤن_Vector_Size(pX->p_کيري);
    لكل(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  أرجع eType;
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
ساكن حرف *اوڠکاڤن_INAffinity(Parse *pParse, ثابت اوڠکاڤن *p_اوڠکاڤن){
  اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
  صحيح nVal = sqlite3_اوڠکاڤن_Vector_Size(p_کيري);
  Select *pSelect = اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ? p_اوڠکاڤن->x.pSelect : 0;
  حرف *zRet;

  أكد( p_اوڠکاڤن->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  إذا( zRet ){
    صحيح i;
    لكل(i=0; i<nVal; i++){
      اوڠکاڤن *pA = sqlite3VectorFieldSubexpr(p_کيري, i);
      حرف a = sqlite3_اوڠکاڤن_Affinity(pA);
      إذا( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_اوڠکاڤن, a);
      }وإلا{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  أرجع zRet;
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
  إذا( pParse->nErr==0 ){
    ثابت حرف *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_ڤسنن(pParse, zFmt, nActual, nExpect);
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
فراغ sqlite3_Vector_Error_ڤسنن(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن){
#ifndef SQLITE_OMIT_SUBQUERY
  إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
    sqlite3SubselectError(pParse, p_اوڠکاڤن->x.pSelect->pEList->n_اوڠکاڤن, 1);
  }وإلا
#endif
  {
    sqlite3_Error_ڤسنن(pParse, "row value misused");
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
  اوڠکاڤن *p_اوڠکاڤن,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  إذا( pNewSig==0 ) أرجع 0;
  إذا( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) أرجع 0;
  أكد( p_اوڠکاڤن->op==TK_IN );
  أكد( !اوڠکاڤن_Use_YSub(p_اوڠکاڤن) );
  أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
  أكد( p_اوڠکاڤن->x.pSelect!=0 );
  أكد( (p_اوڠکاڤن->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  لكل(; pOp<pEnd; pOp++){
    إذا( pOp->p4type!=P4_SUBRTNSIG ) استمر;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    إذا( !pSig->bComplete ) استمر;
    إذا( pNewSig->selId!=pSig->selId ) استمر;
    إذا( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) استمر;
    p_اوڠکاڤن->y.sub.iAddr = pSig->iAddr;
    p_اوڠکاڤن->y.sub.regReturn = pSig->regReturn;
    p_اوڠکاڤن->iTable = pSig->iTable;
    اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_Subrtn);
    أرجع 1;
  }
  أرجع 0;
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
  اوڠکاڤن *p_اوڠکاڤن,            /* The IN operator */
  صحيح iTab,               /* Use this cursor number */
  صحيح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحيح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحيح addr;                   /* Address of OP_OpenEphemeral instruction */
  اوڠکاڤن *p_کيري;                /* the LHS of the IN operator */
  کونچي_Info *p_کونچي_Info = 0;      /* Key information */
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
  إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) || p_اوڠکاڤن->x.pSelect!=0 );
    إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) && (p_اوڠکاڤن->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, حجم(pSig[0]));
      إذا( pSig ){
        pSig->selId = p_اوڠکاڤن->x.pSelect->selId;
        pSig->zAff = اوڠکاڤن_INAffinity(pParse, p_اوڠکاڤن);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_اوڠکاڤن, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_اوڠکاڤن->x.pSelect->selId));
      }
      أكد( اوڠکاڤن_Use_YSub(p_اوڠکاڤن) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_اوڠکاڤن->y.sub.regReturn,
                        p_اوڠکاڤن->y.sub.iAddr);
      أكد( iTab!=p_اوڠکاڤن->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_اوڠکاڤن->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      إذا( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      أرجع;
    }

    /* Begin coding the subroutine */
    أكد( !اوڠکاڤن_Use_YWin(p_اوڠکاڤن) );
    اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_Subrtn);
    أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly|EP_Reduced) );
    p_اوڠکاڤن->y.sub.regReturn = ++pParse->nMem;
    p_اوڠکاڤن->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_اوڠکاڤن->y.sub.regReturn) + 1;
    إذا( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_اوڠکاڤن->y.sub.iAddr;
      pSig->regReturn = p_اوڠکاڤن->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_کيري = p_اوڠکاڤن->p_کيري;
  nVal = sqlite3_اوڠکاڤن_Vector_Size(p_کيري);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_اوڠکاڤن->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_اوڠکاڤن->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
    VdbeComment((v, "Result of SELECT %u", p_اوڠکاڤن->x.pSelect->selId));
  }وإلا{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_کونچي_Info = sqlite3_کونچي_Info_Alloc(pParse->db, nVal, 1);
  أكد( p_کونچي_Info!=0 || pParse->nErr );
  إذا( p_کونچي_Info==0 ) أرجع;

  إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_اوڠکاڤن->x.pSelect;
    اوڠکاڤن_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    إذا( ALWAYS(pEList->n_اوڠکاڤن==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحيح i;
      صحيح rc;
      صحيح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = اوڠکاڤن_INAffinity(pParse, p_اوڠکاڤن);
      pSelect->iLimit = 0;
      أكد( pEList!=0 );
      أكد( pEList->n_اوڠکاڤن>0 );
      أكد( sqlite3_کونچي_Info_Is_Writeable(p_کونچي_Info) );
      لكل(i=0; i<nVal; i++){
        اوڠکاڤن *p = sqlite3VectorFieldSubexpr(p_کيري, i);
        CollSeq *pColl;
        p_کونچي_Info->aColl[i] = pColl = sqlite3_بيناري_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_اوڠکاڤن
        );
        إذا( !sqlite3_Is_بيناري(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      إذا( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحيح regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (فراغ *)p_کونچي_Info, P4_KEYINFO);
        p_کونچي_Info = 0;
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
        sqlite3_کونچي_Info_Unref(p_کونچي_Info);
        أرجع;
      }
    }
  }وإلا إذا( ALWAYS(p_اوڠکاڤن->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    حرف affinity;            /* Affinity of the LHS of the IN */
    صحيح i;
    اوڠکاڤن_List *pList = p_اوڠکاڤن->x.pList;
    بنية اوڠکاڤن_List_item *pItem;
    صحيح r1, r2;
    affinity = sqlite3_اوڠکاڤن_Affinity(p_کيري);
    إذا( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }وإلا إذا( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( p_کونچي_Info!=0 );
    أكد( sqlite3_کونچي_Info_Is_Writeable(p_کونچي_Info) );
    p_کونچي_Info->aColl[0] = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_اوڠکاڤن->p_کيري);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    لكل(i=pList->n_اوڠکاڤن, pItem=pList->a; i>0; i--, pItem++){
      اوڠکاڤن *pE2 = pItem->p_اوڠکاڤن;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      إذا( addrOnce && !sqlite3_اوڠکاڤن_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        اوڠکاڤن_Clear_Property(p_اوڠکاڤن, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_اوڠکاڤن_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_سيسيڤ, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  إذا( pSig ) pSig->bComplete = 1;
  إذا( p_کونچي_Info ){
    sqlite3VdbeChangeP4(v, addr, (فراغ *)p_کونچي_Info, P4_KEYINFO);
  }
  إذا( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    أكد( اوڠکاڤن_Use_YSub(p_اوڠکاڤن) );
    أكد( sqlite3VdbeGetOp(v,p_اوڠکاڤن->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_اوڠکاڤن->y.sub.regReturn,
                      p_اوڠکاڤن->y.sub.iAddr, 1);
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
صحيح sqlite3CodeSubselect(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن){
  صحيح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحيح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحيح nReg;                   /* Registers to allocate */
  اوڠکاڤن *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  إذا( pParse->nErr ) أرجع 0;
  testcase( p_اوڠکاڤن->op==TK_EXISTS );
  testcase( p_اوڠکاڤن->op==TK_SELECT );
  أكد( p_اوڠکاڤن->op==TK_EXISTS || p_اوڠکاڤن->op==TK_SELECT );
  أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
  pSel = p_اوڠکاڤن->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( اوڠکاڤن_Use_YSub(p_اوڠکاڤن) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_اوڠکاڤن->y.sub.regReturn,
                      p_اوڠکاڤن->y.sub.iAddr);
    أرجع p_اوڠکاڤن->iTable;
  }

  /* Begin coding the subroutine */
  أكد( !اوڠکاڤن_Use_YWin(p_اوڠکاڤن) );
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Reduced|EP_TokenOnly) );
  اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_Subrtn);
  p_اوڠکاڤن->y.sub.regReturn = ++pParse->nMem;
  p_اوڠکاڤن->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_اوڠکاڤن->y.sub.regReturn) + 1;

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
  إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_VarSelect) ){
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
  nReg = p_اوڠکاڤن->op==TK_SELECT ? pSel->pEList->n_اوڠکاڤن : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  إذا( p_اوڠکاڤن->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    إذا( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_کانن ){
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
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }وإلا{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  إذا( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    اوڠکاڤن *p_کيري = pSel->pLimit->p_کيري;
    إذا( اوڠکاڤن_Has_Property(p_کيري, EP_Int_نيلاي)==0
     || (p_کيري->u.i_نيلاي!=1 && p_کيري->u.i_نيلاي!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_اوڠکاڤن_Int32(db, 0);
      إذا( pLimit ){
        pLimit->aff_اوڠکاڤن = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_اوڠکاڤن_Dup(db, p_کيري, 0), pLimit);
      }
      sqlite3_اوڠکاڤن_Deferred_Delete(pParse, p_کيري);
      pSel->pLimit->p_کيري = pLimit;
    }
  }وإلا{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_اوڠکاڤن_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  إذا( sqlite3Select(pParse, pSel, &dest) ){
    p_اوڠکاڤن->op2 = p_اوڠکاڤن->op;
    p_اوڠکاڤن->op = TK_ERROR;
    أرجع 0;
  }
  p_اوڠکاڤن->iTable = rReg = dest.iSDParm;
  اوڠکاڤن_Set_VVAProperty(p_اوڠکاڤن, EP_NoReduce);
  إذا( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  إذا( dest.eDest==SRT_Mem 
   && sqlite3_اوڠکاڤن_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_اوڠکاڤن) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  أكد( اوڠکاڤن_Use_YSub(p_اوڠکاڤن) );
  أكد( sqlite3VdbeGetOp(v,p_اوڠکاڤن->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_اوڠکاڤن->y.sub.regReturn,
                    p_اوڠکاڤن->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  أرجع rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحيح sqlite3_اوڠکاڤن_Check_IN(Parse *pParse, اوڠکاڤن *pIn){
  صحيح nVector = sqlite3_اوڠکاڤن_Vector_Size(pIn->p_کيري);
  إذا( اوڠکاڤن_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    إذا( nVector!=pIn->x.pSelect->pEList->n_اوڠکاڤن ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_اوڠکاڤن, nVector);
      أرجع 1;
    }
  }وإلا إذا( nVector!=1 ){
    sqlite3_Vector_Error_ڤسنن(pParse, pIn->p_کيري);
    أرجع 1;
  }
  أرجع 0;
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
ساكن فراغ sqlite3_اوڠکاڤن_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  اوڠکاڤن *p_اوڠکاڤن,          /* The IN expression */
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
  اوڠکاڤن *p_کيري;          /* The LHS of the IN operator */
  صحيح i;                /* loop counter */
  صحيح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحيح destStep6 = 0;    /* Start of code for Step 6 */
  صحيح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحيح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحيح addr_اتس;          /* Top of the step-6 loop */
  صحيح iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  أكد( !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن,EP_Immutable) );
  p_کيري = p_اوڠکاڤن->p_کيري;
  إذا( sqlite3_اوڠکاڤن_Check_IN(pParse, p_اوڠکاڤن) ) أرجع;
  zAff = اوڠکاڤن_INAffinity(pParse, p_اوڠکاڤن);
  nVector = sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن->p_کيري);
  aiMap = (صحيح*)sqlite3DbMallocZero(pParse->db, nVector*حجم(صحيح));
  إذا( pParse->db->mallocFailed ) اذهب sqlite3_اوڠکاڤن_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_اوڠکاڤن,
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
    لكل(cnt=j=0; j<nVector; j++) إذا( aiMap[j]==i ) cnt++;
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
  rLhs = اوڠکاڤن_Code_Vector(pParse, p_کيري, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  إذا( eType==IN_INDEX_NOOP ){
    اوڠکاڤن_List *pList;
    CollSeq *pColl;
    صحيح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحيح r2, regToFree;
    صحيح regCkNull = 0;
    صحيح ii;
    أكد( nVector==1 );
    أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
    pList = p_اوڠکاڤن->x.pList;
    pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p_اوڠکاڤن->p_کيري);
    إذا( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    لكل(ii=0; ii<pList->n_اوڠکاڤن; ii++){
      r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, pList->a[ii].p_اوڠکاڤن, &regToFree);
      إذا( regCkNull && sqlite3_اوڠکاڤن_Can_Be_Null(pList->a[ii].p_اوڠکاڤن) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      إذا( ii<pList->n_اوڠکاڤن-1 || destIfNull!=destIfFalse ){
        صحيح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_اوڠکاڤن-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_اوڠکاڤن-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_اوڠکاڤن-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_اوڠکاڤن-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }وإلا{
        صحيح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    إذا( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    اذهب sqlite3_اوڠکاڤن_Code_IN_finished;
  }

  إذا( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    إذا( nVector==1 ){
      حرف aff = zAff[0];
      إذا( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_اوڠکاڤن_Affinity(p_کيري) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحيح rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }وإلا{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    لكل(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    إذا( i!=nVector ){
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
  إذا( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }وإلا{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  لكل(i=0; i<nVector; i++){
    اوڠکاڤن *p = sqlite3VectorFieldSubexpr(p_اوڠکاڤن->p_کيري, i);
    إذا( pParse->nErr ) اذهب sqlite3_اوڠکاڤن_Code_IN_oom_error;
    إذا( sqlite3_اوڠکاڤن_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  إذا( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }وإلا{
    إذا( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, p_اوڠکاڤن->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        إذا( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      اذهب sqlite3_اوڠکاڤن_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  إذا( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
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
  إذا( eType==IN_INDEX_INDEX_DESC ){
    addr_اتس = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }وإلا{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_اتس = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  إذا( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }وإلا{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  لكل(i=0; i<nVector; i++){
    اوڠکاڤن *p;
    CollSeq *pColl;
    صحيح r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_کيري, i);
    إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
      اوڠکاڤن *pRhs = p_اوڠکاڤن->x.pSelect->pEList->a[i].p_اوڠکاڤن;
      pColl = sqlite3_بيناري_Compare_Coll_Seq(pParse, p, pRhs);
    }وإلا{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (فراغ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  إذا( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_اتس+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_اوڠکاڤن_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_اوڠکاڤن_Code_IN_oom_error:
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
  إذا( ALWAYS(z!=0) ){
    مزدوج نيلاي;
    sqlite3AtoF(z, &نيلاي);
    أكد( !sqlite3IsNaN(نيلاي) ); /* The new AtoF never returns NaN */
    إذا( negateFlag ) نيلاي = -نيلاي;
    sqlite3VdbeAddDouble(v, iMem, نيلاي);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ساكن فراغ codeInteger(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح negFlag, صحيح iMem){
  Vdbe *v = pParse->pVdbe;
  إذا( p_اوڠکاڤن->flags & EP_Int_نيلاي ){
    صحيح i = p_اوڠکاڤن->u.i_نيلاي;
    أكد( i>=0 );
    إذا( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }وإلا{
    صحيح c;
    i64 نيلاي;
    ثابت حرف *z = p_اوڠکاڤن->u.zToken;
    أكد( z!=0 );
    c = sqlite3DecOrHexToI64(z, &نيلاي);
    إذا( (c==3 && !negFlag) || (c==2) || (negFlag && نيلاي==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_ڤسنن(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_اوڠکاڤن);
#وإلا
#ifndef SQLITE_OMIT_HEX_INTEGER
      إذا( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_ڤسنن(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_اوڠکاڤن);
      }وإلا
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }وإلا{
      إذا( negFlag ){ نيلاي = c==3 ? SMALLEST_INT64 : -نيلاي; }
      sqlite3VdbeAddInt64(v, iMem, نيلاي);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
فراغ sqlite3_اوڠکاڤن_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  صحيح iTabCur,    /* Cursor pointing to a table row */
  صحيح iIdxCol,    /* The column of the index to be loaded */
  صحيح regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  إذا( iTabCol==XN_اوڠکاڤن ){
    أكد( pIdx->a_Col_اوڠکاڤن );
    أكد( pIdx->a_Col_اوڠکاڤن->n_اوڠکاڤن>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_اوڠکاڤن_Code_Copy(pParse, pIdx->a_Col_اوڠکاڤن->a[iIdxCol].p_اوڠکاڤن, regOut);
    pParse->iSelfTab = 0;
  }وإلا{
    sqlite3_اوڠکاڤن_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
فراغ sqlite3_اوڠکاڤن_Code_Generated_Column(
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
  إذا( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }وإلا{
    iAddr = 0;
  }
  sqlite3_اوڠکاڤن_Code_Copy(pParse, sqlite3_Column_اوڠکاڤن(pTab,pCol), regOut);
  إذا( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحيح p3 = 2+(صحيح)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (حرف*)pTab, P4_TABLE);
  }وإلا إذا( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  إذا( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  إذا( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
فراغ sqlite3_اوڠکاڤن_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  صحيح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحيح iCol,       /* Index of the column to extract */
  صحيح regOut      /* Extract the value into this register */
){
  Column *pCol;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( iCol!=XN_اوڠکاڤن );
  إذا( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }وإلا{
    صحيح x;
    إذا( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      أرجع;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }وإلا إذا( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      إذا( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_ڤسنن(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }وإلا{
        صحيح savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_اوڠکاڤن_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      أرجع;
#endif
    }وإلا إذا( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_کونچي_Index(pTab), iCol);
    }وإلا{
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
صحيح sqlite3_اوڠکاڤن_Code_Get_Column(
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
  sqlite3_اوڠکاڤن_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  إذا( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    إذا( pOp->opcode==OP_Column ) pOp->p5 = p5;
    إذا( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  أرجع iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
فراغ sqlite3_اوڠکاڤن_Code_Move(Parse *pParse, صحيح iFrom, صحيح iTo, صحيح nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
فراغ sqlite3_اوڠکاڤن_To_Register(اوڠکاڤن *p_اوڠکاڤن, صحيح iReg){
  اوڠکاڤن *p = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(p_اوڠکاڤن);
  إذا( NEVER(p==0) ) أرجع;
  إذا( p->op==TK_REGISTER ){
    أكد( p->iTable==iReg );
  }وإلا{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    اوڠکاڤن_Clear_Property(p, EP_Skip);
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
ساكن صحيح اوڠکاڤن_Code_Vector(Parse *pParse, اوڠکاڤن *p, صحيح *piFreeable){
  صحيح iResult;
  صحيح nResult = sqlite3_اوڠکاڤن_Vector_Size(p);
  إذا( nResult==1 ){
    iResult = sqlite3_اوڠکاڤن_Code_Temp(pParse, p, piFreeable);
  }وإلا{
    *piFreeable = 0;
    إذا( p->op==TK_SELECT ){
#إذا SQLITE_OMIT_SUBQUERY
      iResult = 0;
#وإلا
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }وإلا{
      صحيح i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      أكد( اوڠکاڤن_Use_XList(p) );
      لكل(i=0; i<nResult; i++){
        sqlite3_اوڠکاڤن_Code_Factorable(pParse, p->x.pList->a[i].p_اوڠکاڤن, i+iResult);
      }
    }
  }
  أرجع iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ساكن فراغ setDoNotMergeFlagOnCopy(Vdbe *v){
  إذا( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ساكن صحيح اوڠکاڤن_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  اوڠکاڤن_List *pFarg,      /* List of function arguments */
  صحيح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحيح target            /* Store function result in this register */
){
  صحيح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->n_اوڠکاڤن;
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
      sqlite3_اوڠکاڤن_Code(pParse, pFarg->a[0].p_اوڠکاڤن, target);
      لكل(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_اوڠکاڤن_Code(pParse, pFarg->a[i].p_اوڠکاڤن, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      اكسر;
    }
    حالة INLINEFUNC_iif: {
      اوڠکاڤن case_اوڠکاڤن;
      memset(&case_اوڠکاڤن, 0, حجم(case_اوڠکاڤن));
      case_اوڠکاڤن.op = TK_CASE;
      case_اوڠکاڤن.x.pList = pFarg;
      أرجع sqlite3_اوڠکاڤن_Code_Target(pParse, &case_اوڠکاڤن, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    حالة INLINEFUNC_sqlite_offset: {
      اوڠکاڤن *pArg = pFarg->a[0].p_اوڠکاڤن;
      إذا( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }وإلا{
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
      target = sqlite3_اوڠکاڤن_Code_Target(pParse, pFarg->a[0].p_اوڠکاڤن, target);
      اكسر;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#إذا !defined(SQLITE_UNTESTABLE)
    حالة INLINEFUNC_اوڠکاڤن_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_اوڠکاڤن_Compare(0,pFarg->a[0].p_اوڠکاڤن, pFarg->a[1].p_اوڠکاڤن,-1),
         target);
      اكسر;
    }

    حالة INLINEFUNC_اوڠکاڤن_implies_اوڠکاڤن: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_اوڠکاڤن_Implies_اوڠکاڤن(pParse,pFarg->a[0].p_اوڠکاڤن, pFarg->a[1].p_اوڠکاڤن,-1),
         target);
      اكسر;
    }

    حالة INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      اوڠکاڤن *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].p_اوڠکاڤن;
      إذا( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_اوڠکاڤن_Implies_Non_Null_Row(pFarg->a[0].p_اوڠکاڤن,pA1->iTable,1),
           target);
      }وإلا{
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
      aff = sqlite3_اوڠکاڤن_Affinity(pFarg->a[0].p_اوڠکاڤن);
      أكد( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      اكسر;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  أرجع target;
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
ساكن صحيح اوڠکاڤن_نود_Can_Return_Subtype(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  صحيح n;
  FuncDef *pDef;
  sqlite3 *db;
  إذا( p_اوڠکاڤن->op==TK_CASE || p_اوڠکاڤن->op==TK_UPLUS 
   || p_اوڠکاڤن->op==TK_COLLATE || p_اوڠکاڤن->op==TK_CAST 
  ){
    أرجع WRC_Continue;
  }
  إذا( p_اوڠکاڤن->op!=TK_FUNCTION && p_اوڠکاڤن->op!=TK_AGG_FUNCTION ){
    أرجع WRC_Prune;
  }
  أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
  db = pWalker->pParse->db;
  n = p_اوڠکاڤن->x.pList ? p_اوڠکاڤن->x.pList->n_اوڠکاڤن : 0;
  pDef = sqlite3FindFunction(db, p_اوڠکاڤن->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  إذا( pDef==0 ) أرجع WRC_Continue;
#وإلا
  أكد( pDef!=0 );
#endif
  إذا( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
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
صحيح sqlite3_اوڠکاڤن_Can_Return_Subtype(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن){
  Walker w;
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_نود_Can_Return_Subtype;
  sqlite3_Walk_اوڠکاڤن(&w, p_اوڠکاڤن);
  أرجع w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ساكن SQLITE_NOINLINE صحيح sqlite3_Indexed_اوڠکاڤن_Lookup(
  Parse *pParse,   /* The parsing context */
  اوڠکاڤن *p_اوڠکاڤن,     /* The expression to potentially bypass */
  صحيح target       /* Where to store the result of the expression */
){
  Indexed_اوڠکاڤن *p;
  Vdbe *v;
  لكل(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 اوڠکاڤن_Aff;
    صحيح iDataCur = p->iDataCur;
    إذا( iDataCur<0 ) استمر;
    إذا( pParse->iSelfTab ){
      إذا( p->iDataCur!=pParse->iSelfTab-1 ) استمر;
      iDataCur = -1;
    }
    إذا( sqlite3_اوڠکاڤن_Compare(0, p_اوڠکاڤن, p->p_اوڠکاڤن, iDataCur)!=0 ) استمر;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    اوڠکاڤن_Aff = sqlite3_اوڠکاڤن_Affinity(p_اوڠکاڤن);
    إذا( (اوڠکاڤن_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (اوڠکاڤن_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (اوڠکاڤن_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      استمر;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_SubtArg)
     && sqlite3_اوڠکاڤن_Can_Return_Subtype(pParse, p_اوڠکاڤن) 
    ){
      استمر;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    إذا( p->bMaybeNullRow || sqlite3_اوڠکاڤن_Is_Constant(pParse, p_اوڠکاڤن) ){
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
      sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }وإلا{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    أرجع target;
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
ساكن صحيح اوڠکاڤن_Partidx_اوڠکاڤن_Lookup(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح iTarget){
  Indexed_اوڠکاڤن *p;
  لكل(p=pParse->p_Idx_Part_اوڠکاڤن; p; p=p->pIENext){
    إذا( p_اوڠکاڤن->iColumn==p->iIdxCol && p_اوڠکاڤن->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      صحيح addr = 0;
      صحيح ret;

      إذا( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_اوڠکاڤن_Code_Target(pParse, p->p_اوڠکاڤن, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت حرف*)&p->aff, 1);
      إذا( sqlite3_اوڠکاڤن_Can_Return_Subtype(pParse, p->p_اوڠکاڤن) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      إذا( addr ){
        sqlite3VdbeJumpHere(v, addr);
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
ساكن SQLITE_NOINLINE صحيح اوڠکاڤن_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,       /* AND or OR expression to be coded */
  صحيح target,        /* Put result in this register, guaranteed */
  صحيح *pTmpReg       /* Write a temporary register here */
){
  صحيح op;            /* The opcode.  TK_AND or TK_OR */
  صحيح skipOp;        /* Opcode for the branch that skips one operand */
  صحيح addrSkip;      /* Branch instruction that skips one of the operands */
  صحيح regSS = 0;     /* Register holding computed operand when other omitted */
  صحيح r1, r2;        /* Registers for left and right operands, respectively */
  اوڠکاڤن *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  أكد( p_اوڠکاڤن!=0 );
  op = p_اوڠکاڤن->op;
  أكد( op==TK_AND || op==TK_OR );
  أكد( TK_AND==OP_And );            testcase( op==TK_AND );
  أكد( TK_OR==OP_Or );              testcase( op==TK_OR );
  أكد( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_اوڠکاڤن_Simplified_And_Or(p_اوڠکاڤن);
  إذا( pAlt!=p_اوڠکاڤن ){
    r1 = sqlite3_اوڠکاڤن_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    أرجع target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  إذا( اوڠکاڤن_Eval_Rhs_First(p_اوڠکاڤن) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن->p_کانن, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, pTmpReg);
  }وإلا{
    /* Compute the left operand first */
    r1 = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن->p_کيري, target);
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن->p_کانن, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }وإلا{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  إذا( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  أرجع target;
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
صحيح sqlite3_اوڠکاڤن_Code_Target(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحيح op;                   /* The opcode being coded */
  صحيح inReg = target;       /* Results stored in register inReg */
  صحيح regFree1 = 0;         /* If non-zero free this temporary register */
  صحيح regFree2 = 0;         /* If non-zero free this temporary register */
  صحيح r1, r2;               /* Various register numbers */
  اوڠکاڤن tempX;               /* Temporary expression node */
  صحيح p5 = 0;

  أكد( target>0 && target<=pParse->nMem );
  أكد( v!=0 );

اوڠکاڤن_code_doover:
  إذا( p_اوڠکاڤن==0 ){
    op = TK_NULL;
  }وإلا إذا( pParse->pIdxEpr!=0
   && !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Leaf)
   && (r1 = sqlite3_Indexed_اوڠکاڤن_Lookup(pParse, p_اوڠکاڤن, target))>=0
  ){
    أرجع r1;
  }وإلا{
    أكد( !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن,EP_Immutable) );
    op = p_اوڠکاڤن->op;
  }
  أكد( op!=TK_ORDER );
  بدل( op ){
    حالة TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_اوڠکاڤن->pAggInfo;
      بنية AggInfo_col *pCol;
      أكد( pAggInfo!=0 );
      أكد( p_اوڠکاڤن->iAgg>=0 );
      إذا( p_اوڠکاڤن->iAgg>=pAggInfo->nColumn ){
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
      pCol = &pAggInfo->aCol[p_اوڠکاڤن->iAgg];
      إذا( !pAggInfo->directMode ){
        أرجع AggInfoColumnReg(pAggInfo, p_اوڠکاڤن->iAgg);
      }وإلا إذا( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        إذا( pTab==0 ){
          /* No comment added */
        }وإلا إذا( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }وإلا{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          إذا( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        أرجع target;
      }وإلا إذا( p_اوڠکاڤن->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_اوڠکاڤن->iTable, p_اوڠکاڤن->iColumn, target);
        أرجع target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالة TK_COLUMN: {
      صحيح iTab = p_اوڠکاڤن->iTable;
      صحيح iReg;
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحيح aff;
        iReg = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن->p_کيري,target);
        أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
        أكد( p_اوڠکاڤن->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_اوڠکاڤن->y.pTab, p_اوڠکاڤن->iColumn);
        إذا( aff>SQLITE_AFF_BLOB ){
          ساكن ثابت حرف zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
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
          Column *pCol;
          Table *pTab;
          صحيح iSrc;
          صحيح iCol = p_اوڠکاڤن->iColumn;
          أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
          pTab = p_اوڠکاڤن->y.pTab;
          أكد( pTab!=0 );
          أكد( iCol>=XN_ROWID );
          أكد( iCol<pTab->nCol );
          إذا( iCol<0 ){
            أرجع -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          إذا( pCol->colFlags & COLFLAG_GENERATED ){
            إذا( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_ڤسنن(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              أرجع 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            إذا( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_اوڠکاڤن_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            أرجع iSrc;
          }وإلا
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          إذا( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            أرجع target;
          }وإلا{
            أرجع iSrc;
          }
        }وإلا{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      وإلا إذا( pParse->p_Idx_Part_اوڠکاڤن 
       && 0!=(r1 = اوڠکاڤن_Partidx_اوڠکاڤن_Lookup(pParse, p_اوڠکاڤن, target))
      ){
        أرجع r1;
      }
      أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
      أكد( p_اوڠکاڤن->y.pTab!=0 );
      iReg = sqlite3_اوڠکاڤن_Code_Get_Column(pParse, p_اوڠکاڤن->y.pTab,
                               p_اوڠکاڤن->iColumn, iTab, target,
                               p_اوڠکاڤن->op2);
      أرجع iReg;
    }
    حالة TK_INTEGER: {
      codeInteger(pParse, p_اوڠکاڤن, 0, target);
      أرجع target;
    }
    حالة TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_اوڠکاڤن_Truth_نيلاي(p_اوڠکاڤن), target);
      أرجع target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالة TK_FLOAT: {
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      codeReal(v, p_اوڠکاڤن->u.zToken, 0, target);
      أرجع target;
    }
#endif
    حالة TK_STRING: {
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      sqlite3VdbeLoadString(v, target, p_اوڠکاڤن->u.zToken);
      أرجع target;
    }
    حالة TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_اوڠکاڤن->y.nReg - 1);
      أرجع target;
    }
    افتراضي: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      أرجع target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      صحيح n;
      ثابت حرف *z;
      حرف *zBlob;
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      أكد( p_اوڠکاڤن->u.zToken[0]=='x' || p_اوڠکاڤن->u.zToken[0]=='X' );
      أكد( p_اوڠکاڤن->u.zToken[1]=='\'' );
      z = &p_اوڠکاڤن->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      أرجع target;
    }
#endif
    حالة TK_VARIABLE: {
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      أكد( p_اوڠکاڤن->u.zToken!=0 );
      أكد( p_اوڠکاڤن->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_اوڠکاڤن->iColumn, target);
      أرجع target;
    }
    حالة TK_REGISTER: {
      أرجع p_اوڠکاڤن->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن->p_کيري, target);
      أكد( inReg==target );
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_اوڠکاڤن->u.zToken, 0));
      أرجع inReg;
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
      اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
      صحيح addrIsNull = 0;
      إذا( sqlite3_اوڠکاڤن_Is_Vector(p_کيري) ){
        codeVectorCompare(pParse, p_اوڠکاڤن, target, op, p5);
      }وإلا{
        إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = اوڠکاڤن_Compute_Operands(pParse, p_اوڠکاڤن,
                                     &r1, &r2, &regFree1, &regFree2);
        }وإلا{
          r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
          r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_کيري, p_اوڠکاڤن->p_کانن, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        إذا( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }وإلا{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          إذا( addrIsNull ){
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
      inReg = اوڠکاڤن_Code_Target_And_Or(pParse, p_اوڠکاڤن, target, &regFree1);
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
      أكد( TK_LSHIFT==OP_Shift_کيري );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_Shift_کانن );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subquery) ){
        addrIsNull = اوڠکاڤن_Compute_Operands(pParse, p_اوڠکاڤن,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
        r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      إذا( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      اكسر;
    }
    حالة TK_UMINUS: {
      اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
      أكد( p_کيري );
      إذا( p_کيري->op==TK_INTEGER ){
        codeInteger(pParse, p_کيري, 1, target);
        أرجع target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }وإلا إذا( p_کيري->op==TK_FLOAT ){
        أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
        codeReal(v, p_کيري->u.zToken, 1, target);
        أرجع target;
#endif
      }وإلا{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_نيلاي|EP_TokenOnly;
        tempX.u.i_نيلاي = 0;
        اوڠکاڤن_Clear_VVAProperties(&tempX);
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أكد( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحيح bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_اوڠکاڤن_Truth_نيلاي(p_اوڠکاڤن->p_کانن);
      bNormal = p_اوڠکاڤن->op2==TK_IS;
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
      r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      اكسر;
    }
    حالة TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_اوڠکاڤن->pAggInfo;
      إذا( pInfo==0
       || NEVER(p_اوڠکاڤن->iAgg<0)
       || NEVER(p_اوڠکاڤن->iAgg>=pInfo->nFunc)
      ){
        أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
        sqlite3_Error_ڤسنن(pParse, "misuse of aggregate: %#T()", p_اوڠکاڤن);
      }وإلا{
        أرجع AggInfoFuncReg(pInfo, p_اوڠکاڤن->iAgg);
      }
      اكسر;
    }
    حالة TK_FUNCTION: {
      اوڠکاڤن_List *pFarg;       /* List of function arguments */
      صحيح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت حرف *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      صحيح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
        أرجع p_اوڠکاڤن->y.pWin->regResult;
      }
#endif

      إذا( ConstFactorOk(pParse)
       && sqlite3_اوڠکاڤن_Is_Constant_Not_Join(pParse,p_اوڠکاڤن)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        أرجع sqlite3_اوڠکاڤن_Code_Run_Just_Once(pParse, p_اوڠکاڤن, -1);
      }
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly) );
      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
      pFarg = p_اوڠکاڤن->x.pList;
      nFarg = pFarg ? pFarg->n_اوڠکاڤن : 0;
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      zId = p_اوڠکاڤن->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      إذا( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      إذا( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_ڤسنن(pParse, "unknown function: %#T()", p_اوڠکاڤن);
        اكسر;
      }
      إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        أرجع اوڠکاڤن_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }وإلا إذا( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_اوڠکاڤن_Function_Usable(pParse, p_اوڠکاڤن, pDef);
      }

      لكل(i=0; i<nFarg; i++){
        إذا( i<32 && sqlite3_اوڠکاڤن_Is_Constant(pParse, pFarg->a[i].p_اوڠکاڤن) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_اوڠکاڤن_Coll_Seq(pParse, pFarg->a[i].p_اوڠکاڤن);
        }
      }
      إذا( pFarg ){
        إذا( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }وإلا{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        إذا( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 اوڠکاڤن_Op;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].p_اوڠکاڤن!=0 );
          اوڠکاڤن_Op = pFarg->a[0].p_اوڠکاڤن->op;
          إذا( اوڠکاڤن_Op==TK_COLUMN || اوڠکاڤن_Op==TK_AGG_COLUMN ){
            أكد( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_اوڠکاڤن->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_اوڠکاڤن_Code_اوڠکاڤن_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }وإلا{
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
      إذا( nFarg>=2 && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_اوڠکاڤن);
      }وإلا إذا( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_اوڠکاڤن);
      }
#endif
      إذا( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        إذا( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_اوڠکاڤن->op2);
      إذا( nFarg ){
        إذا( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }وإلا{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      أرجع target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT: {
      صحيح nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      إذا( pParse->db->mallocFailed ){
        أرجع 0;
      }وإلا إذا( op==TK_SELECT
             && ALWAYS( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) )
             && (nCol = p_اوڠکاڤن->x.pSelect->pEList->n_اوڠکاڤن)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }وإلا{
        أرجع sqlite3CodeSubselect(pParse, p_اوڠکاڤن);
      }
      اكسر;
    }
    حالة TK_SELECT_COLUMN: {
      صحيح n;
      اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
      إذا( p_کيري->iTable==0 || pParse->withinRJSubrtn > p_کيري->op2 ){
        p_کيري->iTable = sqlite3CodeSubselect(pParse, p_کيري);
        p_کيري->op2 = pParse->withinRJSubrtn;
      }
      أكد( p_کيري->op==TK_SELECT || p_کيري->op==TK_ERROR );
      n = sqlite3_اوڠکاڤن_Vector_Size(p_کيري);
      إذا( p_اوڠکاڤن->iTable!=n ){
        sqlite3_Error_ڤسنن(pParse, "%d columns assigned %d values",
                                p_اوڠکاڤن->iTable, n);
      }
      أرجع p_کيري->iTable + p_اوڠکاڤن->iColumn;
    }
    حالة TK_IN: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_اوڠکاڤن_Code_IN(pParse, p_اوڠکاڤن, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      أرجع target;
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
      اوڠکاڤن_Code_Between(pParse, p_اوڠکاڤن, target, 0, 0);
      أرجع target;
    }
    حالة TK_COLLATE: {
      إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( p_اوڠکاڤن->p_کيري );
        sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن->p_کيري, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        أرجع target;
      }وإلا{
        p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
        اذهب اوڠکاڤن_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالة TK_SPAN:
    حالة TK_UPLUS: {
      p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
      اذهب اوڠکاڤن_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
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

      أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
      pTab = p_اوڠکاڤن->y.pTab;
      iCol = p_اوڠکاڤن->iColumn;
      p1 = p_اوڠکاڤن->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      أكد( p_اوڠکاڤن->iTable==0 || p_اوڠکاڤن->iTable==1 );
      أكد( iCol>=-1 && iCol<pTab->nCol );
      أكد( pTab->iPKey<0 || iCol!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_اوڠکاڤن->iTable ? "new" : "old"),
        (p_اوڠکاڤن->iColumn<0 ? "rowid" : p_اوڠکاڤن->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      إذا( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      اكسر;
    }

    حالة TK_VECTOR: {
      sqlite3_Error_ڤسنن(pParse, "row value misused");
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
      AggInfo *pAggInfo = p_اوڠکاڤن->pAggInfo;
      إذا( pAggInfo ){
        أكد( p_اوڠکاڤن->iAgg>=0 && p_اوڠکاڤن->iAgg<pAggInfo->nColumn );
        إذا( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_اوڠکاڤن->iAgg);
          اكسر;
        }
        إذا( p_اوڠکاڤن->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_اوڠکاڤن->iAgg].iSorterColumn,
                            target);
          inReg = target;
          اكسر;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_اوڠکاڤن->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن->p_کيري, target);
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
      صحيح n_اوڠکاڤن;                        /* 2x number of WHEN terms */
      صحيح i;                            /* Loop counter */
      اوڠکاڤن_List *pEList;                 /* List of WHEN terms */
      بنية اوڠکاڤن_List_item *aListelem;  /* Array of WHEN terms */
      اوڠکاڤن opCompare;                   /* The X==Ei expression */
      اوڠکاڤن *pX;                         /* The X expression */
      اوڠکاڤن *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      اوڠکاڤن *pDel = 0;
      sqlite3 *db = pParse->db;

      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) && p_اوڠکاڤن->x.pList!=0 );
      أكد(p_اوڠکاڤن->x.pList->n_اوڠکاڤن > 0);
      pEList = p_اوڠکاڤن->x.pList;
      aListelem = pEList->a;
      n_اوڠکاڤن = pEList->n_اوڠکاڤن;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      إذا( (pX = p_اوڠکاڤن->p_کيري)!=0 ){
        pDel = sqlite3_اوڠکاڤن_Dup(db, pX, 0);
        إذا( db->mallocFailed ){
          sqlite3_اوڠکاڤن_Delete(db, pDel);
          اكسر;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_اوڠکاڤن_To_Register(pDel, اوڠکاڤن_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, حجم(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_کيري = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      لكل(i=0; i<n_اوڠکاڤن-1; i=i+2){
        إذا( pX ){
          أكد( pTest!=0 );
          opCompare.p_کانن = aListelem[i].p_اوڠکاڤن;
        }وإلا{
          pTest = aListelem[i].p_اوڠکاڤن;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_اوڠکاڤن_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_اوڠکاڤن->op==TK_COLUMN );
        sqlite3_اوڠکاڤن_Code(pParse, aListelem[i+1].p_اوڠکاڤن, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      إذا( (n_اوڠکاڤن&1)!=0 ){
        sqlite3_اوڠکاڤن_Code(pParse, pEList->a[n_اوڠکاڤن-1].p_اوڠکاڤن, target);
      }وإلا{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_اوڠکاڤن_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      اكسر;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      أكد( p_اوڠکاڤن->aff_اوڠکاڤن==OE_Rollback
           || p_اوڠکاڤن->aff_اوڠکاڤن==OE_Abort
           || p_اوڠکاڤن->aff_اوڠکاڤن==OE_Fail
           || p_اوڠکاڤن->aff_اوڠکاڤن==OE_Ignore
      );
      إذا( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_ڤسنن(pParse,
                       "RAISE() may only be used within a trigger-program");
        أرجع 0;
      }
      إذا( p_اوڠکاڤن->aff_اوڠکاڤن==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      إذا( p_اوڠکاڤن->aff_اوڠکاڤن==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_اوڠکاڤن->aff_اوڠکاڤن, r1);
      }
      اكسر;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  أرجع inReg;
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
صحيح sqlite3_اوڠکاڤن_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  اوڠکاڤن *p_اوڠکاڤن,      /* The expression to code when the VDBE initializes */
  صحيح regDest       /* Store the value in this register */
){
  اوڠکاڤن_List *p;
  أكد( ConstFactorOk(pParse) );
  أكد( regDest!=0 );
  p = pParse->p_Const_اوڠکاڤن;
  إذا( regDest<0 && p ){
    بنية اوڠکاڤن_List_item *pItem;
    صحيح i;
    لكل(pItem=p->a, i=p->n_اوڠکاڤن; i>0; pItem++, i--){
      إذا( pItem->fg.reusable
       && sqlite3_اوڠکاڤن_Compare(0,pItem->p_اوڠکاڤن,p_اوڠکاڤن,-1)==0
      ){
        أرجع pItem->u.i_Const_اوڠکاڤن_Reg;
      }
    }
  }
  p_اوڠکاڤن = sqlite3_اوڠکاڤن_Dup(pParse->db, p_اوڠکاڤن, 0);
  إذا( p_اوڠکاڤن!=0 && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحيح addr;
    أكد( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    إذا( !pParse->db->mallocFailed ){
      إذا( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_اوڠکاڤن_Delete(pParse->db, p_اوڠکاڤن);
    sqlite3VdbeJumpHere(v, addr);
  }وإلا{
    p = sqlite3_اوڠکاڤن_List_Append(pParse, p, p_اوڠکاڤن);
    إذا( p ){
       بنية اوڠکاڤن_List_item *pItem = &p->a[p->n_اوڠکاڤن-1];
       pItem->fg.reusable = regDest<0;
       إذا( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_اوڠکاڤن_Reg = regDest;
    }
    pParse->p_Const_اوڠکاڤن = p;
  }
  أرجع regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE فراغ sqlite3_اوڠکاڤن_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  صحيح iReg,        /* First register to set to NULL */
  صحيح nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  اوڠکاڤن t;
  memset(&t, 0, حجم(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_اوڠکاڤن_Code_Run_Just_Once(pParse, &t, iReg);
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
صحيح sqlite3_اوڠکاڤن_Code_Temp(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح *pReg){
  صحيح r2;
  p_اوڠکاڤن = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(p_اوڠکاڤن);
  إذا( ConstFactorOk(pParse)
   && ALWAYS(p_اوڠکاڤن!=0)
   && p_اوڠکاڤن->op!=TK_REGISTER
   && sqlite3_اوڠکاڤن_Is_Constant_Not_Join(pParse, p_اوڠکاڤن)
  ){
    *pReg  = 0;
    r2 = sqlite3_اوڠکاڤن_Code_Run_Just_Once(pParse, p_اوڠکاڤن, -1);
  }وإلا{
    صحيح r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن, r1);
    إذا( r2==r1 ){
      *pReg = r1;
    }وإلا{
      sqlite3ReleaseTempReg(pParse, r1);
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
فراغ sqlite3_اوڠکاڤن_Code(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح target){
  صحيح inReg;

  أكد( p_اوڠکاڤن==0 || !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن,EP_Immutable) );
  أكد( target>0 && target<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  إذا( pParse->pVdbe==0 ) أرجع;
  inReg = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن, target);
  إذا( inReg!=target ){
    u8 op;
    اوڠکاڤن *pX = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(p_اوڠکاڤن);
    testcase( pX!=p_اوڠکاڤن );
    إذا( ALWAYS(pX)
     && (اوڠکاڤن_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }وإلا{
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
فراغ sqlite3_اوڠکاڤن_Code_Copy(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح target){
  sqlite3 *db = pParse->db;
  p_اوڠکاڤن = sqlite3_اوڠکاڤن_Dup(db, p_اوڠکاڤن, 0);
  إذا( !db->mallocFailed ) sqlite3_اوڠکاڤن_Code(pParse, p_اوڠکاڤن, target);
  sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
فراغ sqlite3_اوڠکاڤن_Code_Factorable(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح target){
  إذا( pParse->okConstFactor && sqlite3_اوڠکاڤن_Is_Constant_Not_Join(pParse,p_اوڠکاڤن) ){
    sqlite3_اوڠکاڤن_Code_Run_Just_Once(pParse, p_اوڠکاڤن, target);
  }وإلا{
    sqlite3_اوڠکاڤن_Code_Copy(pParse, p_اوڠکاڤن, target);
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
صحيح sqlite3_اوڠکاڤن_Code_اوڠکاڤن_List(
  Parse *pParse,     /* Parsing context */
  اوڠکاڤن_List *pList,   /* The expression list to be coded */
  صحيح target,        /* Where to write results */
  صحيح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  بنية اوڠکاڤن_List_item *pItem;
  صحيح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( target>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_اوڠکاڤن;
  إذا( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  لكل(pItem=pList->a, i=0; i<n; i++, pItem++){
    اوڠکاڤن *p_اوڠکاڤن = pItem->p_اوڠکاڤن;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    إذا( pItem->fg.bSorterRef ){
      i--;
      n--;
    }وإلا
#endif
    إذا( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      إذا( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }وإلا{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }وإلا إذا( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_اوڠکاڤن_Is_Constant_Not_Join(pParse,p_اوڠکاڤن)
    ){
      sqlite3_اوڠکاڤن_Code_Run_Just_Once(pParse, p_اوڠکاڤن, target+i);
    }وإلا{
      صحيح inReg = sqlite3_اوڠکاڤن_Code_Target(pParse, p_اوڠکاڤن, target+i);
      إذا( inReg!=target+i ){
        VdbeOp *pOp;
        إذا( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }وإلا{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
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
ساكن فراغ اوڠکاڤن_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  اوڠکاڤن *p_اوڠکاڤن,      /* The BETWEEN expression */
  صحيح dest,         /* Jump destination or storage location */
  فراغ (*xJump)(Parse*,اوڠکاڤن*,صحيح,صحيح), /* Action to take */
  صحيح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  اوڠکاڤن اوڠکاڤن_And;     /* The AND operator in  x>=y AND x<=z  */
  اوڠکاڤن comp_کيري;    /* The  x>=y  term */
  اوڠکاڤن comp_کانن;   /* The  x<=z  term */
  صحيح regFree1 = 0; /* Temporary use register */
  اوڠکاڤن *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_کيري, 0, حجم(اوڠکاڤن));
  memset(&comp_کانن, 0, حجم(اوڠکاڤن));
  memset(&اوڠکاڤن_And, 0, حجم(اوڠکاڤن));

  أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
  pDel = sqlite3_اوڠکاڤن_Dup(db, p_اوڠکاڤن->p_کيري, 0);
  إذا( db->mallocFailed==0 ){
    اوڠکاڤن_And.op = TK_AND;
    اوڠکاڤن_And.p_کيري = &comp_کيري;
    اوڠکاڤن_And.p_کانن = &comp_کانن;
    comp_کيري.op = TK_GE;
    comp_کيري.p_کيري = pDel;
    comp_کيري.p_کانن = p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن;
    comp_کانن.op = TK_LE;
    comp_کانن.p_کيري = pDel;
    comp_کانن.p_کانن = p_اوڠکاڤن->x.pList->a[1].p_اوڠکاڤن;
    sqlite3_اوڠکاڤن_To_Register(pDel, اوڠکاڤن_Code_Vector(pParse, pDel, &regFree1));
    إذا( xJump ){
      xJump(pParse, &اوڠکاڤن_And, dest, jumpIfNull);
    }وإلا{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_اوڠکاڤن_Code_Target(pParse, &اوڠکاڤن_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_اوڠکاڤن_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_اوڠکاڤن_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_اوڠکاڤن_If_False && jumpIfNull!=0 && regFree1!=0 );
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
فراغ sqlite3_اوڠکاڤن_If_True(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) )     أرجع;  /* Existence of VDBE checked by caller */
  إذا( NEVER(p_اوڠکاڤن==0) ) أرجع;  /* No way this can happen */
  أكد( !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن, EP_Immutable) );
  op = p_اوڠکاڤن->op;
  بدل( op ){
    حالة TK_AND:
    حالة TK_OR: {
      اوڠکاڤن *pAlt = sqlite3_اوڠکاڤن_Simplified_And_Or(p_اوڠکاڤن);
      إذا( pAlt!=p_اوڠکاڤن ){
        sqlite3_اوڠکاڤن_If_True(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        اوڠکاڤن *pFirst, *pSecond;
        إذا( اوڠکاڤن_Eval_Rhs_First(p_اوڠکاڤن) ){
          pFirst = p_اوڠکاڤن->p_کانن;
          pSecond = p_اوڠکاڤن->p_کيري;
        }وإلا{
          pFirst = p_اوڠکاڤن->p_کيري;
          pSecond = p_اوڠکاڤن->p_کانن;
        }
        إذا( op==TK_AND ){
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اوڠکاڤن_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اوڠکاڤن_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }وإلا{
          testcase( jumpIfNull==0 );
          sqlite3_اوڠکاڤن_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_اوڠکاڤن_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اوڠکاڤن_If_False(pParse, p_اوڠکاڤن->p_کيري, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اوڠکاڤن->op2==TK_ISNOT;
      isTrue = sqlite3_اوڠکاڤن_Truth_نيلاي(p_اوڠکاڤن->p_کانن);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        sqlite3_اوڠکاڤن_If_True(pParse, p_اوڠکاڤن->p_کيري, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }وإلا{
        sqlite3_اوڠکاڤن_If_False(pParse, p_اوڠکاڤن->p_کيري, dest,
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
      إذا( sqlite3_اوڠکاڤن_Is_Vector(p_اوڠکاڤن->p_کيري) ) اذهب default_اوڠکاڤن;
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اوڠکاڤن_Compute_Operands(pParse, p_اوڠکاڤن,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
        r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_اوڠکاڤن->p_کيري, p_اوڠکاڤن->p_کانن, op,
                  r1, r2, dest, jumpIfNull, اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_Commuted));
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
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اوڠکاڤن_Code_Between(pParse, p_اوڠکاڤن, dest, sqlite3_اوڠکاڤن_If_True, jumpIfNull);
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_IN: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_اوڠکاڤن_Code_IN(pParse, p_اوڠکاڤن, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      اكسر;
    }
#endif
    افتراضي: {
    default_اوڠکاڤن:
      إذا( اوڠکاڤن_Always_True(p_اوڠکاڤن) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( اوڠکاڤن_Always_False(p_اوڠکاڤن) ){
        /* No-op */
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن, &regFree1);
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
فراغ sqlite3_اوڠکاڤن_If_False(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) ) أرجع; /* Existence of VDBE checked by caller */
  إذا( p_اوڠکاڤن==0 )    أرجع;
  أكد( !اوڠکاڤن_Has_VVAProperty(p_اوڠکاڤن,EP_Immutable) );

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
  op = ((p_اوڠکاڤن->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  أكد( p_اوڠکاڤن->op!=TK_ISNULL || op==OP_NotNull );
  أكد( p_اوڠکاڤن->op!=TK_NOTNULL || op==OP_IsNull );
  أكد( p_اوڠکاڤن->op!=TK_NE || op==OP_Eq );
  أكد( p_اوڠکاڤن->op!=TK_EQ || op==OP_Ne );
  أكد( p_اوڠکاڤن->op!=TK_LT || op==OP_Ge );
  أكد( p_اوڠکاڤن->op!=TK_LE || op==OP_Gt );
  أكد( p_اوڠکاڤن->op!=TK_GT || op==OP_Le );
  أكد( p_اوڠکاڤن->op!=TK_GE || op==OP_Lt );

  بدل( p_اوڠکاڤن->op ){
    حالة TK_AND:
    حالة TK_OR: {
      اوڠکاڤن *pAlt = sqlite3_اوڠکاڤن_Simplified_And_Or(p_اوڠکاڤن);
      إذا( pAlt!=p_اوڠکاڤن ){
        sqlite3_اوڠکاڤن_If_False(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        اوڠکاڤن *pFirst, *pSecond;
        إذا( اوڠکاڤن_Eval_Rhs_First(p_اوڠکاڤن) ){
          pFirst = p_اوڠکاڤن->p_کانن;
          pSecond = p_اوڠکاڤن->p_کيري;
        }وإلا{
          pFirst = p_اوڠکاڤن->p_کيري;
          pSecond = p_اوڠکاڤن->p_کانن;
        }
        إذا( p_اوڠکاڤن->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_اوڠکاڤن_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_اوڠکاڤن_If_False(pParse, pSecond, dest, jumpIfNull);
        }وإلا{
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اوڠکاڤن_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اوڠکاڤن_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اوڠکاڤن_If_True(pParse, p_اوڠکاڤن->p_کيري, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اوڠکاڤن->op2==TK_ISNOT;
      isTrue = sqlite3_اوڠکاڤن_Truth_نيلاي(p_اوڠکاڤن->p_کانن);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_اوڠکاڤن_If_False(pParse, p_اوڠکاڤن->p_کيري, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }وإلا{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_اوڠکاڤن_If_True(pParse, p_اوڠکاڤن->p_کيري, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT:
      testcase( p_اوڠکاڤن->op==TK_IS );
      testcase( p_اوڠکاڤن->op==TK_ISNOT );
      op = (p_اوڠکاڤن->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      صحيح addrIsNull;
      إذا( sqlite3_اوڠکاڤن_Is_Vector(p_اوڠکاڤن->p_کيري) ) اذهب default_اوڠکاڤن;
      إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اوڠکاڤن_Compute_Operands(pParse, p_اوڠکاڤن,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
        r2 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کانن, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_اوڠکاڤن->p_کيري, p_اوڠکاڤن->p_کانن, op,
                  r1, r2, dest, jumpIfNull,اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_Commuted));
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
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن->p_کيري, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اوڠکاڤن_Code_Between(pParse, p_اوڠکاڤن, dest, sqlite3_اوڠکاڤن_If_False, jumpIfNull);
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_IN: {
      إذا( jumpIfNull ){
        sqlite3_اوڠکاڤن_Code_IN(pParse, p_اوڠکاڤن, dest, dest);
      }وإلا{
        صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_اوڠکاڤن_Code_IN(pParse, p_اوڠکاڤن, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      اكسر;
    }
#endif
    افتراضي: {
    default_اوڠکاڤن:
      إذا( اوڠکاڤن_Always_False(p_اوڠکاڤن) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( اوڠکاڤن_Always_True(p_اوڠکاڤن) ){
        /* no-op */
      }وإلا{
        r1 = sqlite3_اوڠکاڤن_Code_Temp(pParse, p_اوڠکاڤن, &regFree1);
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
فراغ sqlite3_اوڠکاڤن_If_False_Dup(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, صحيح dest,صحيح jumpIfNull){
  sqlite3 *db = pParse->db;
  اوڠکاڤن *pCopy = sqlite3_اوڠکاڤن_Dup(db, p_اوڠکاڤن, 0);
  إذا( db->mallocFailed==0 ){
    sqlite3_اوڠکاڤن_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_اوڠکاڤن_Delete(db, pCopy);
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
ساكن SQLITE_NOINLINE صحيح اوڠکاڤن_Compare_Variable(
  ثابت Parse *pParse,
  ثابت اوڠکاڤن *pVar,
  ثابت اوڠکاڤن *p_اوڠکاڤن
){
  صحيح res = 2;
  صحيح iVar;
  sqlite3_نيلاي *pL, *pR = 0;
 
  إذا( p_اوڠکاڤن->op==TK_VARIABLE && pVar->iColumn==p_اوڠکاڤن->iColumn ){
    أرجع 0;
  }
  إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) أرجع 2;
  sqlite3_نيلاي_From_اوڠکاڤن(pParse->db, p_اوڠکاڤن, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  إذا( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_نيلاي(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    إذا( pL ){
      إذا( sqlite3_نيلاي_type(pL)==SQLITE_TEXT ){
        sqlite3_نيلاي_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_نيلاي_Free(pR);
    sqlite3_نيلاي_Free(pL);
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
صحيح sqlite3_اوڠکاڤن_Compare(
  ثابت Parse *pParse,
  ثابت اوڠکاڤن *pA,
  ثابت اوڠکاڤن *pB,
  صحيح iTab
){
  u32 combinedFlags;
  إذا( pA==0 || pB==0 ){
    أرجع pB==pA ? 0 : 2;
  }
  إذا( pParse && pA->op==TK_VARIABLE ){
    أرجع اوڠکاڤن_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  إذا( combinedFlags & EP_Int_نيلاي ){
    إذا( (pA->flags&pB->flags&EP_Int_نيلاي)!=0 && pA->u.i_نيلاي==pB->u.i_نيلاي ){
      أرجع 0;
    }
    أرجع 2;
  }
  إذا( pA->op!=pB->op || pA->op==TK_RAISE ){
    إذا( pA->op==TK_COLLATE && sqlite3_اوڠکاڤن_Compare(pParse, pA->p_کيري,pB,iTab)<2 ){
      أرجع 1;
    }
    إذا( pB->op==TK_COLLATE && sqlite3_اوڠکاڤن_Compare(pParse, pA,pB->p_کيري,iTab)<2 ){
      أرجع 1;
    }
    إذا( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }وإلا{
      أرجع 2;
    }
  }
  أكد( !اوڠکاڤن_Has_Property(pA, EP_Int_نيلاي) );
  أكد( !اوڠکاڤن_Has_Property(pB, EP_Int_نيلاي) );
  إذا( pA->u.zToken ){
    إذا( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      إذا( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      إذا( اوڠکاڤن_Has_Property(pA,EP_WinFunc)!=اوڠکاڤن_Has_Property(pB,EP_WinFunc) ){
        أرجع 2;
      }
      إذا( اوڠکاڤن_Has_Property(pA,EP_WinFunc) ){
        إذا( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          أرجع 2;
        }
      }
#endif
    }وإلا إذا( pA->op==TK_NULL ){
      أرجع 0;
    }وإلا إذا( pA->op==TK_COLLATE ){
      إذا( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
    }وإلا
    إذا( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      أرجع 2;
    }
  }
  إذا( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) أرجع 2;
  إذا( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    إذا( combinedFlags & EP_xIsSelect ) أرجع 2;
    إذا( (combinedFlags & EP_FixedCol)==0
     && sqlite3_اوڠکاڤن_Compare(pParse, pA->p_کيري, pB->p_کيري, iTab) ) أرجع 2;
    إذا( sqlite3_اوڠکاڤن_Compare(pParse, pA->p_کانن, pB->p_کانن, iTab) ) أرجع 2;
    إذا( sqlite3_اوڠکاڤن_List_Compare(pA->x.pList, pB->x.pList, iTab) ) أرجع 2;
    إذا( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      إذا( pA->iColumn!=pB->iColumn ) أرجع 2;
      إذا( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) أرجع 2;
      إذا( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
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
صحيح sqlite3_اوڠکاڤن_List_Compare(ثابت اوڠکاڤن_List *pA, ثابت اوڠکاڤن_List *pB, صحيح iTab){
  صحيح i;
  إذا( pA==0 && pB==0 ) أرجع 0;
  إذا( pA==0 || pB==0 ) أرجع 1;
  إذا( pA->n_اوڠکاڤن!=pB->n_اوڠکاڤن ) أرجع 1;
  لكل(i=0; i<pA->n_اوڠکاڤن; i++){
    صحيح res;
    اوڠکاڤن *p_اوڠکاڤن_A = pA->a[i].p_اوڠکاڤن;
    اوڠکاڤن *p_اوڠکاڤن_B = pB->a[i].p_اوڠکاڤن;
    إذا( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) أرجع 1;
    إذا( (res = sqlite3_اوڠکاڤن_Compare(0, p_اوڠکاڤن_A, p_اوڠکاڤن_B, iTab)) ) أرجع res;
  }
  أرجع 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحيح sqlite3_اوڠکاڤن_Compare_Skip(اوڠکاڤن *pA,اوڠکاڤن *pB, صحيح iTab){
  أرجع sqlite3_اوڠکاڤن_Compare(0,
             sqlite3_اوڠکاڤن_Skip_Collate(pA),
             sqlite3_اوڠکاڤن_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ساكن صحيح اوڠکاڤن_Implies_Not_Null(
  ثابت Parse *pParse,/* Parsing context */
  ثابت اوڠکاڤن *p,      /* The expression to be checked */
  ثابت اوڠکاڤن *pNN,    /* The expression that is NOT NULL */
  صحيح iTab,           /* Table being evaluated */
  صحيح seenNot         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  إذا( sqlite3_اوڠکاڤن_Compare(pParse, p, pNN, iTab)==0 ){
    أرجع pNN->op!=TK_NULL;
  }
  بدل( p->op ){
    حالة TK_IN: {
      إذا( seenNot && اوڠکاڤن_Has_Property(p, EP_xIsSelect) ) أرجع 0;
      أكد( اوڠکاڤن_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_اوڠکاڤن>0) );
      أرجع اوڠکاڤن_Implies_Not_Null(pParse, p->p_کيري, pNN, iTab, 1);
    }
    حالة TK_BETWEEN: {
      اوڠکاڤن_List *pList;
      أكد( اوڠکاڤن_Use_XList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->n_اوڠکاڤن==2 );
      إذا( seenNot ) أرجع 0;
      إذا( اوڠکاڤن_Implies_Not_Null(pParse, pList->a[0].p_اوڠکاڤن, pNN, iTab, 1)
       || اوڠکاڤن_Implies_Not_Null(pParse, pList->a[1].p_اوڠکاڤن, pNN, iTab, 1)
      ){
        أرجع 1;
      }
      أرجع اوڠکاڤن_Implies_Not_Null(pParse, p->p_کيري, pNN, iTab, 1);
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
      إذا( اوڠکاڤن_Implies_Not_Null(pParse, p->p_کانن, pNN, iTab, seenNot) ) أرجع 1;
      /* no break */ deliberate_fall_through
    }
    حالة TK_SPAN:
    حالة TK_COLLATE:
    حالة TK_UPLUS:
    حالة TK_UMINUS: {
      أرجع اوڠکاڤن_Implies_Not_Null(pParse, p->p_کيري, pNN, iTab, seenNot);
    }
    حالة TK_TRUTH: {
      إذا( seenNot ) أرجع 0;
      إذا( p->op2!=TK_IS ) أرجع 0;
      أرجع اوڠکاڤن_Implies_Not_Null(pParse, p->p_کيري, pNN, iTab, 1);
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أرجع اوڠکاڤن_Implies_Not_Null(pParse, p->p_کيري, pNN, iTab, 1);
    }
  }
  أرجع 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ساكن صحيح sqlite3_اوڠکاڤن_Is_Not_True(اوڠکاڤن *p_اوڠکاڤن){
  صحيح v;
  إذا( p_اوڠکاڤن->op==TK_NULL ) أرجع 1;
  إذا( p_اوڠکاڤن->op==TK_TRUEFALSE && sqlite3_اوڠکاڤن_Truth_نيلاي(p_اوڠکاڤن)==0 ) أرجع 1;
  v = 1;
  إذا( sqlite3_اوڠکاڤن_Is_Integer(p_اوڠکاڤن, &v, 0, 0) && v==0 ) أرجع 1;
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
ساكن صحيح sqlite3_اوڠکاڤن_Is_IIF(sqlite3 *db, ثابت اوڠکاڤن *p_اوڠکاڤن){
  اوڠکاڤن_List *pList;
  إذا( p_اوڠکاڤن->op==TK_FUNCTION ){
    ثابت حرف *z = p_اوڠکاڤن->u.zToken;
    FuncDef *pDef;
    إذا( (z[0]!='i' && z[0]!='I') ) أرجع 0;
    إذا( p_اوڠکاڤن->x.pList==0 ) أرجع 0;
    pDef = sqlite3FindFunction(db, z, p_اوڠکاڤن->x.pList->n_اوڠکاڤن, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    إذا( pDef==0 ) أرجع 0;
#وإلا
    إذا( NEVER(pDef==0) ) أرجع 0;
#endif
    إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) أرجع 0;
    إذا( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) أرجع 0;
  }وإلا إذا( p_اوڠکاڤن->op==TK_CASE ){
    إذا( p_اوڠکاڤن->p_کيري!=0 ) أرجع 0;
  }وإلا{
    أرجع 0;
  }
  pList = p_اوڠکاڤن->x.pList;
  أكد( pList!=0 );
  إذا( pList->n_اوڠکاڤن==2 ) أرجع 1;
  إذا( pList->n_اوڠکاڤن==3 && sqlite3_اوڠکاڤن_Is_Not_True(pList->a[2].p_اوڠکاڤن) ) أرجع 1;
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
صحيح sqlite3_اوڠکاڤن_Implies_اوڠکاڤن(
  ثابت Parse *pParse,
  ثابت اوڠکاڤن *pE1,
  ثابت اوڠکاڤن *pE2,
  صحيح iTab
){
  إذا( sqlite3_اوڠکاڤن_Compare(pParse, pE1, pE2, iTab)==0 ){
    أرجع 1;
  }
  إذا( pE2->op==TK_OR
   && (sqlite3_اوڠکاڤن_Implies_اوڠکاڤن(pParse, pE1, pE2->p_کيري, iTab)
             || sqlite3_اوڠکاڤن_Implies_اوڠکاڤن(pParse, pE1, pE2->p_کانن, iTab) )
  ){
    أرجع 1;
  }
  إذا( pE2->op==TK_NOTNULL
   && اوڠکاڤن_Implies_Not_Null(pParse, pE1, pE2->p_کيري, iTab, 0)
  ){
    أرجع 1;
  }
  إذا( sqlite3_اوڠکاڤن_Is_IIF(pParse->db, pE1) ){
    أرجع sqlite3_اوڠکاڤن_Implies_اوڠکاڤن(pParse,pE1->x.pList->a[0].p_اوڠکاڤن,pE2,iTab);
  }
  أرجع 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ساكن فراغ bothImplyNotNullRow(Walker *pWalker, اوڠکاڤن *pE1, اوڠکاڤن *pE2){
  إذا( pWalker->eCode==0 ){
    sqlite3_Walk_اوڠکاڤن(pWalker, pE1);
    إذا( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_اوڠکاڤن(pWalker, pE2);
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
ساكن صحيح impliesNotNullRow(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  testcase( p_اوڠکاڤن->op==TK_AGG_COLUMN );
  testcase( p_اوڠکاڤن->op==TK_AGG_FUNCTION );
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON) ) أرجع WRC_Prune;
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    أرجع WRC_Prune;
  }
  بدل( p_اوڠکاڤن->op ){
    حالة TK_ISNOT:
    حالة TK_ISNULL:
    حالة TK_NOTNULL:
    حالة TK_IS:
    حالة TK_VECTOR:
    حالة TK_FUNCTION:
    حالة TK_TRUTH:
    حالة TK_CASE:
      testcase( p_اوڠکاڤن->op==TK_ISNOT );
      testcase( p_اوڠکاڤن->op==TK_ISNULL );
      testcase( p_اوڠکاڤن->op==TK_NOTNULL );
      testcase( p_اوڠکاڤن->op==TK_IS );
      testcase( p_اوڠکاڤن->op==TK_VECTOR );
      testcase( p_اوڠکاڤن->op==TK_FUNCTION );
      testcase( p_اوڠکاڤن->op==TK_TRUTH );
      testcase( p_اوڠکاڤن->op==TK_CASE );
      أرجع WRC_Prune;

    حالة TK_COLUMN:
      إذا( pWalker->u.iCur==p_اوڠکاڤن->iTable ){
        pWalker->eCode = 1;
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
      testcase( p_اوڠکاڤن->op==TK_OR );
      testcase( p_اوڠکاڤن->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_اوڠکاڤن->p_کيري, p_اوڠکاڤن->p_کانن);
      أرجع WRC_Prune;
       
    حالة TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      إذا( اوڠکاڤن_Use_XList(p_اوڠکاڤن) && ALWAYS(p_اوڠکاڤن->x.pList->n_اوڠکاڤن>0) ){
        sqlite3_Walk_اوڠکاڤن(pWalker, p_اوڠکاڤن->p_کيري);
      }
      أرجع WRC_Prune;

    حالة TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
      أكد( p_اوڠکاڤن->x.pList->n_اوڠکاڤن==2 );
      sqlite3_Walk_اوڠکاڤن(pWalker, p_اوڠکاڤن->p_کيري);
      bothImplyNotNullRow(pWalker, p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن,
                                   p_اوڠکاڤن->x.pList->a[1].p_اوڠکاڤن);
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
      اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
      اوڠکاڤن *p_کانن = p_اوڠکاڤن->p_کانن;
      testcase( p_اوڠکاڤن->op==TK_EQ );
      testcase( p_اوڠکاڤن->op==TK_NE );
      testcase( p_اوڠکاڤن->op==TK_LT );
      testcase( p_اوڠکاڤن->op==TK_LE );
      testcase( p_اوڠکاڤن->op==TK_GT );
      testcase( p_اوڠکاڤن->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      أكد( p_کيري->op!=TK_COLUMN || اوڠکاڤن_Use_YTab(p_کيري) );
      أكد( p_کانن->op!=TK_COLUMN || اوڠکاڤن_Use_YTab(p_کانن) );
      إذا( (p_کيري->op==TK_COLUMN
           && ALWAYS(p_کيري->y.pTab!=0)
           && IsVirtual(p_کيري->y.pTab))
       || (p_کانن->op==TK_COLUMN
           && ALWAYS(p_کانن->y.pTab!=0)
           && IsVirtual(p_کانن->y.pTab))
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
صحيح sqlite3_اوڠکاڤن_Implies_Non_Null_Row(اوڠکاڤن *p, صحيح iTab, صحيح isRJ){
  Walker w;
  p = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(p);
  إذا( p==0 ) أرجع 0;
  إذا( p->op==TK_NOTNULL ){
    p = p->p_کيري;
  }وإلا{
    طالما( p->op==TK_AND ){
      إذا( sqlite3_اوڠکاڤن_Implies_Non_Null_Row(p->p_کيري, iTab, isRJ) ) أرجع 1;
      p = p->p_کانن;
    }
  }
  w.x_اوڠکاڤن_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_اوڠکاڤن(&w, p);
  أرجع w.eCode;
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
ساكن صحيح اوڠکاڤن_Idx_Cover(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  إذا( p_اوڠکاڤن->op==TK_COLUMN
   && p_اوڠکاڤن->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_اوڠکاڤن->iColumn)<0
  ){
    pWalker->eCode = 1;
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
صحيح sqlite3_اوڠکاڤن_Covered_By_Index(
  اوڠکاڤن *p_اوڠکاڤن,        /* The index to be tested */
  صحيح iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  بنية IdxCover xcov;
  memset(&w, 0, حجم(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_اوڠکاڤن(&w, p_اوڠکاڤن);
  أرجع !w.eCode;
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
ساكن صحيح اوڠکاڤن_Ref_To_Src_List(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  إذا( p_اوڠکاڤن->op==TK_COLUMN
   || p_اوڠکاڤن->op==TK_AGG_COLUMN
  ){
    صحيح i;
    بنية RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحيح nSrc = pSrc ? pSrc->nSrc : 0;
    لكل(i=0; i<nSrc; i++){
      إذا( p_اوڠکاڤن->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        أرجع WRC_Continue;
      }
    }
    لكل(i=0; i<p->nExclude && p->aiExclude[i]!=p_اوڠکاڤن->iTable; i++){}
    إذا( i>=p->nExclude ){
      pWalker->eCode |= 2;
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
صحيح sqlite3ReferencesSrcList(Parse *pParse, اوڠکاڤن *p_اوڠکاڤن, SrcList *pSrcList){
  Walker w;
  بنية RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, حجم(w));
  memset(&x, 0, حجم(x));
  w.x_اوڠکاڤن_Callback = اوڠکاڤن_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( p_اوڠکاڤن->op==TK_AGG_FUNCTION );
  أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
  sqlite3_Walk_اوڠکاڤن_List(&w, p_اوڠکاڤن->x.pList);
  إذا( p_اوڠکاڤن->p_کيري ){
    أكد( p_اوڠکاڤن->p_کيري->op==TK_ORDER );
    أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن->p_کيري) );
    أكد( p_اوڠکاڤن->p_کيري->x.pList!=0 );
    sqlite3_Walk_اوڠکاڤن_List(&w, p_اوڠکاڤن->p_کيري->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
    sqlite3_Walk_اوڠکاڤن(&w, p_اوڠکاڤن->y.pWin->pFilter);
  }
#endif
  إذا( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  إذا( w.eCode & 0x01 ){
    أرجع 1;
  }وإلا إذا( w.eCode ){
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
ساكن صحيح agginfo_Persist_اوڠکاڤن_Cb(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  إذا( ALWAYS(!اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly|EP_Reduced))
   && p_اوڠکاڤن->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_اوڠکاڤن->pAggInfo;
    صحيح iAgg = p_اوڠکاڤن->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    إذا( p_اوڠکاڤن->op!=TK_AGG_FUNCTION ){
      إذا( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_اوڠکاڤن
      ){
        p_اوڠکاڤن = sqlite3_اوڠکاڤن_Dup(db, p_اوڠکاڤن, 0);
        إذا( p_اوڠکاڤن && !sqlite3_اوڠکاڤن_Deferred_Delete(pParse, p_اوڠکاڤن) ){
          pAggInfo->aCol[iAgg].pCExpr = p_اوڠکاڤن;
        }
      }
    }وإلا{
      أكد( p_اوڠکاڤن->op==TK_AGG_FUNCTION );
      إذا( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_اوڠکاڤن
      ){
        p_اوڠکاڤن = sqlite3_اوڠکاڤن_Dup(db, p_اوڠکاڤن, 0);
        إذا( p_اوڠکاڤن && !sqlite3_اوڠکاڤن_Deferred_Delete(pParse, p_اوڠکاڤن) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_اوڠکاڤن;
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
  pWalker->x_اوڠکاڤن_Callback = agginfo_Persist_اوڠکاڤن_Cb;
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
  أرجع i;
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
  أرجع i;
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
  اوڠکاڤن *p_اوڠکاڤن          /* Expr describing the column to find or insert */
){
  بنية AggInfo_col *pCol;
  صحيح k;
  صحيح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  أكد( mxTerm <= SMXV(i16) );
  أكد( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  لكل(k=0; k<pAggInfo->nColumn; k++, pCol++){
    إذا( pCol->pCExpr==p_اوڠکاڤن ) أرجع;
    إذا( pCol->iTable==p_اوڠکاڤن->iTable
     && pCol->iColumn==p_اوڠکاڤن->iColumn
     && p_اوڠکاڤن->op!=TK_IF_NULL_ROW
    ){
      اذهب fix_up_اوڠکاڤن;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  إذا( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    أرجع;
  }
  إذا( k>mxTerm ){
    sqlite3_Error_ڤسنن(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
  pCol->pTab = p_اوڠکاڤن->y.pTab;
  pCol->iTable = p_اوڠکاڤن->iTable;
  pCol->iColumn = p_اوڠکاڤن->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_اوڠکاڤن;
  إذا( pAggInfo->pGroupBy && p_اوڠکاڤن->op!=TK_IF_NULL_ROW ){
    صحيح j, n;
    اوڠکاڤن_List *pGB = pAggInfo->pGroupBy;
    بنية اوڠکاڤن_List_item *pTerm = pGB->a;
    n = pGB->n_اوڠکاڤن;
    لكل(j=0; j<n; j++, pTerm++){
      اوڠکاڤن *pE = pTerm->p_اوڠکاڤن;
      إذا( pE->op==TK_COLUMN
       && pE->iTable==p_اوڠکاڤن->iTable
       && pE->iColumn==p_اوڠکاڤن->iColumn
      ){
        pCol->iSorterColumn = j;
        اكسر;
      }
    }
  }
  إذا( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_اوڠکاڤن:
  اوڠکاڤن_Set_VVAProperty(p_اوڠکاڤن, EP_NoReduce);
  أكد( p_اوڠکاڤن->pAggInfo==0 || p_اوڠکاڤن->pAggInfo==pAggInfo );
  p_اوڠکاڤن->pAggInfo = pAggInfo;
  إذا( p_اوڠکاڤن->op==TK_COLUMN ){
    p_اوڠکاڤن->op = TK_AGG_COLUMN;
  }
  أكد( k <= SMXV(p_اوڠکاڤن->iAgg) );
  p_اوڠکاڤن->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ساكن صحيح analyzeAggregate(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  صحيح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->iFirstReg==0 );
  بدل( p_اوڠکاڤن->op ){
    افتراضي: {
      Indexed_اوڠکاڤن *pIEpr;
      اوڠکاڤن tmp;
      أكد( pParse->iSelfTab==0 );
      إذا( (pNC->ncFlags & NC_InAggFunc)==0 ) اكسر;
      إذا( pParse->pIdxEpr==0 ) اكسر;
      لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحيح iDataCur = pIEpr->iDataCur;
        إذا( iDataCur<0 ) استمر;
        إذا( sqlite3_اوڠکاڤن_Compare(0, p_اوڠکاڤن, pIEpr->p_اوڠکاڤن, iDataCur)==0 ) اكسر;
      }
      إذا( pIEpr==0 ) اكسر;
      إذا( NEVER(!اوڠکاڤن_Use_YTab(p_اوڠکاڤن)) ) اكسر;
      لكل(i=0; i<pSrcList->nSrc; i++){
         إذا( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           اكسر;
         }
      }
      إذا( i>=pSrcList->nSrc ) اكسر;
      إذا( NEVER(p_اوڠکاڤن->pAggInfo!=0) ) اكسر; /* Resolved by outer context */
      إذا( pParse->nErr ){ أرجع WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, حجم(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      إذا( pParse->nErr ){ أرجع WRC_Abort; }
      أكد( pAggInfo->aCol!=0 );
      أكد( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_اوڠکاڤن;
      p_اوڠکاڤن->pAggInfo = pAggInfo;
      p_اوڠکاڤن->iAgg = tmp.iAgg;
      أرجع WRC_Prune;
    }
    حالة TK_IF_NULL_ROW:
    حالة TK_AGG_COLUMN:
    حالة TK_COLUMN: {
      testcase( p_اوڠکاڤن->op==TK_AGG_COLUMN );
      testcase( p_اوڠکاڤن->op==TK_COLUMN );
      testcase( p_اوڠکاڤن->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      إذا( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        لكل(i=0; i<pSrcList->nSrc; i++, pItem++){
          أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly|EP_Reduced) );
          إذا( p_اوڠکاڤن->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_اوڠکاڤن);
            اكسر;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      أرجع WRC_Continue;
    }
    حالة TK_AGG_FUNCTION: {
      إذا( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_اوڠکاڤن->op2
       && p_اوڠکاڤن->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        بنية AggInfo_func *pItem = pAggInfo->aFunc;
        صحيح mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        أكد( mxTerm <= SMXV(i16) );
        لكل(i=0; i<pAggInfo->nFunc; i++, pItem++){
          إذا( NEVER(pItem->pFExpr==p_اوڠکاڤن) ) اكسر;
          إذا( sqlite3_اوڠکاڤن_Compare(0, pItem->pFExpr, p_اوڠکاڤن, -1)==0 ){
            اكسر;
          }
        }
        إذا( i>mxTerm ){
          sqlite3_Error_ڤسنن(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          أكد( i<pAggInfo->nFunc );
        }وإلا إذا( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          إذا( i>=0 ){
            صحيح nArg;
            أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_اوڠکاڤن;
            أكد( اوڠکاڤن_Use_UToken(p_اوڠکاڤن) );
            nArg = p_اوڠکاڤن->x.pList ? p_اوڠکاڤن->x.pList->n_اوڠکاڤن : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_اوڠکاڤن->u.zToken, nArg, enc, 0);
            أكد( pItem->bOBUnique==0 );
            إذا( p_اوڠکاڤن->p_کيري
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              اوڠکاڤن_List *pOBList;
              أكد( nArg>0 );
              أكد( p_اوڠکاڤن->p_کيري->op==TK_ORDER );
              أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن->p_کيري) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_اوڠکاڤن->p_کيري->x.pList;
              أكد( pOBList->n_اوڠکاڤن>0 );
              أكد( pItem->bOBUnique==0 );
              إذا( pOBList->n_اوڠکاڤن==1
               && nArg==1
               && sqlite3_اوڠکاڤن_Compare(0,pOBList->a[0].p_اوڠکاڤن,
                               p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Distinct);
              }وإلا{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }وإلا{
              pItem->iOBTab = -1;
            }
            إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }وإلا{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly|EP_Reduced) );
        اوڠکاڤن_Set_VVAProperty(p_اوڠکاڤن, EP_NoReduce);
        أكد( i <= SMXV(p_اوڠکاڤن->iAgg) );
        p_اوڠکاڤن->iAgg = (i16)i;
        p_اوڠکاڤن->pAggInfo = pAggInfo;
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
فراغ sqlite3_اوڠکاڤن_Analyze_Aggregates(NameContext *pNC, اوڠکاڤن *p_اوڠکاڤن){
  Walker w;
  w.x_اوڠکاڤن_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  أكد( pNC->pSrcList!=0 );
  sqlite3_Walk_اوڠکاڤن(&w, p_اوڠکاڤن);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
فراغ sqlite3_اوڠکاڤن_Analyze_Agg_List(NameContext *pNC, اوڠکاڤن_List *pList){
  بنية اوڠکاڤن_List_item *pItem;
  صحيح i;
  إذا( pList ){
    لكل(pItem=pList->a, i=0; i<pList->n_اوڠکاڤن; i++, pItem++){
      sqlite3_اوڠکاڤن_Analyze_Aggregates(pNC, pItem->p_اوڠکاڤن);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحيح sqlite3GetTempReg(Parse *pParse){
  إذا( pParse->nTempReg==0 ){
    أرجع ++pParse->nMem;
  }
  أرجع pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
فراغ sqlite3ReleaseTempReg(Parse *pParse, صحيح iReg){
  إذا( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحيح kk;
    لكل(kk=0; kk<pParse->nTempReg; kk++){
      أكد( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    إذا( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحيح sqlite3GetTempRange(Parse *pParse, صحيح nReg){
  صحيح i, n;
  إذا( nReg==1 ) أرجع sqlite3GetTempReg(pParse);
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
فراغ sqlite3ReleaseTempRange(Parse *pParse, صحيح iReg, صحيح nReg){
  إذا( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
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
فراغ sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
فراغ sqlite3TouchRegister(Parse *pParse, صحيح iReg){
  إذا( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#إذا defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحيح sqlite3FirstAvailableRegister(Parse *pParse, صحيح iMin){
  ثابت اوڠکاڤن_List *pList = pParse->p_Const_اوڠکاڤن;
  إذا( pList ){
    صحيح i;
    لكل(i=0; i<pList->n_اوڠکاڤن; i++){
      إذا( pList->a[i].u.i_Const_اوڠکاڤن_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_اوڠکاڤن_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  أرجع iMin;
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
  إذا( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     أرجع 0;
  }
  لكل(i=0; i<pParse->nTempReg; i++){
    إذا( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      أرجع 0;
    }
  }
  إذا( pParse->p_Const_اوڠکاڤن ){
    اوڠکاڤن_List *pList = pParse->p_Const_اوڠکاڤن;
    لكل(i=0; i<pList->n_اوڠکاڤن; i++){
      صحيح iReg = pList->a[i].u.i_Const_اوڠکاڤن_Reg;
      إذا( iReg==0 ) استمر;
      إذا( iReg>=iFirst && iReg<=iLast ) أرجع 0;
    }
  }
  أرجع 1;
}
#endif /* SQLITE_DEBUG */
