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
#অন্তর্ভাব "sqliteInt.h"

/* Forward declarations */
স্থির শূন্য রাশিমালা_Code_Between(Parse*,রাশিমালা*,পূর্ণাঙ্ক,শূন্য(*)(Parse*,রাশিমালা*,পূর্ণাঙ্ক,পূর্ণাঙ্ক),পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক রাশিমালা_Code_Vector(Parse *pParse, রাশিমালা *p, পূর্ণাঙ্ক *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
বর্ণ sqlite3TableColumnAffinity(ধ্রুব Table *pTab, পূর্ণাঙ্ক iCol){
  যদি( iCol<0 || NEVER(iCol>=pTab->nCol) ) প্রত্যাগম SQLITE_AFF_INTEGER;
  প্রত্যাগম pTab->aCol[iCol].affinity;
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
বর্ণ sqlite3_রাশিমালা_Affinity(ধ্রুব রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক op;
  op = p_রাশিমালা->op;
  যাবত্( 1 /* exit-by-break */ ){
    যদি( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_রাশিমালা->y.pTab!=0) ){
      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->y.pTab!=0 );
      প্রত্যাগম sqlite3TableColumnAffinity(p_রাশিমালা->y.pTab, p_রাশিমালা->iColumn);
    }
    যদি( op==TK_SELECT ){
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->x.pSelect!=0 );
      প্রতিজ্ঞা( p_রাশিমালা->x.pSelect->pEList!=0 );
      প্রতিজ্ঞা( p_রাশিমালা->x.pSelect->pEList->a[0].p_রাশিমালা!=0 );
      প্রত্যাগম sqlite3_রাশিমালা_Affinity(p_রাশিমালা->x.pSelect->pEList->a[0].p_রাশিমালা);
    }
#ifndef SQLITE_OMIT_CAST
    যদি( op==TK_CAST ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      প্রত্যাগম sqlite3AffinityType(p_রাশিমালা->u.zToken, 0);
    }
#endif
    যদি( op==TK_SELECT_COLUMN ){
      প্রতিজ্ঞা( p_রাশিমালা->p_বাম!=0 && রাশিমালা_Use_XSelect(p_রাশিমালা->p_বাম) );
      প্রতিজ্ঞা( p_রাশিমালা->iColumn < p_রাশিমালা->iTable );
      প্রতিজ্ঞা( p_রাশিমালা->iColumn >= 0 );
      প্রতিজ্ঞা( p_রাশিমালা->iTable==p_রাশিমালা->p_বাম->x.pSelect->pEList->n_রাশিমালা );
      প্রত্যাগম sqlite3_রাশিমালা_Affinity(
          p_রাশিমালা->p_বাম->x.pSelect->pEList->a[p_রাশিমালা->iColumn].p_রাশিমালা
      );
    }
    যদি( op==TK_VECTOR
     || (op==TK_FUNCTION && p_রাশিমালা->aff_রাশিমালা==SQLITE_AFF_DEFER)
    ){
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      প্রত্যাগম sqlite3_রাশিমালা_Affinity(p_রাশিমালা->x.pList->a[0].p_রাশিমালা);
    }
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Skip|EP_IfNullRow) ){
      প্রতিজ্ঞা( p_রাশিমালা->op==TK_COLLATE
           || p_রাশিমালা->op==TK_IF_NULL_ROW
           || (p_রাশিমালা->op==TK_REGISTER && p_রাশিমালা->op2==TK_IF_NULL_ROW) );
      p_রাশিমালা = p_রাশিমালা->p_বাম;
      op = p_রাশিমালা->op;
      অনুবর্ত;
    }
    যদি( op!=TK_REGISTER ) ভঙ্গ;
    op = p_রাশিমালা->op2;
    যদি( NEVER( op==TK_REGISTER ) ) ভঙ্গ;
  }
  প্রত্যাগম p_রাশিমালা->aff_রাশিমালা;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Data_Type(ধ্রুব রাশিমালা *p_রাশিমালা){
  যাবত্( p_রাশিমালা ){
    বিকল্প( p_রাশিমালা->op ){
      পক্ষ TK_COLLATE:
      পক্ষ TK_IF_NULL_ROW:
      পক্ষ TK_UPLUS:  {
        p_রাশিমালা = p_রাশিমালা->p_বাম;
        ভঙ্গ;
      }
      পক্ষ TK_NULL: {
        p_রাশিমালা = 0;
        ভঙ্গ;
      }
      পক্ষ TK_STRING: {
        প্রত্যাগম 0x02;
      }
      পক্ষ TK_BLOB: {
        প্রত্যাগম 0x04;
      }
      পক্ষ TK_CONCAT: {
        প্রত্যাগম 0x06;
      }
      পক্ষ TK_VARIABLE:
      পক্ষ TK_AGG_FUNCTION:
      পক্ষ TK_FUNCTION: {
        প্রত্যাগম 0x07;
      }
      পক্ষ TK_COLUMN:
      পক্ষ TK_AGG_COLUMN:
      পক্ষ TK_SELECT:
      পক্ষ TK_CAST:
      পক্ষ TK_SELECT_COLUMN:
      পক্ষ TK_VECTOR:  {
        পূর্ণাঙ্ক aff = sqlite3_রাশিমালা_Affinity(p_রাশিমালা);
        যদি( aff>=SQLITE_AFF_NUMERIC ) প্রত্যাগম 0x05;
        যদি( aff==SQLITE_AFF_TEXT )    প্রত্যাগম 0x06;
        প্রত্যাগম 0x07;
      }
      পক্ষ TK_CASE: {
        পূর্ণাঙ্ক res = 0;
        পূর্ণাঙ্ক ii;
        রাশিমালা_List *pList = p_রাশিমালা->x.pList;
        প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) && pList!=0 );
        প্রতিজ্ঞা( pList->n_রাশিমালা > 0);
        প্রতি(ii=1; ii<pList->n_রাশিমালা; ii+=2){
          res |= sqlite3_রাশিমালা_Data_Type(pList->a[ii].p_রাশিমালা);
        }
        যদি( pList->n_রাশিমালা % 2 ){
          res |= sqlite3_রাশিমালা_Data_Type(pList->a[pList->n_রাশিমালা-1].p_রাশিমালা);
        }
        প্রত্যাগম res;
      }
      স্বভাব: {
        প্রত্যাগম 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  প্রত্যাগম 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
রাশিমালা *sqlite3_রাশিমালা_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  রাশিমালা *p_রাশিমালা,             /* Add the "COLLATE" clause to this expression */
  ধ্রুব Token *pCollName,  /* Name of collating sequence */
  পূর্ণাঙ্ক dequote              /* True to dequote pCollName */
){
  যদি( pCollName->n>0 && pParse->nErr==0 ){
    রাশিমালা *pNew = sqlite3_রাশিমালা_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    প্রতিজ্ঞা( p_রাশিমালা!=0 );
    যদি( pNew ){
      pNew->p_বাম = p_রাশিমালা;
      pNew->flags |= EP_Collate|EP_Skip;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
      pNew->nHeight = p_রাশিমালা->nHeight+1;
      sqlite3_রাশিমালা_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_রাশিমালা = pNew;
    }
  }
  প্রত্যাগম p_রাশিমালা;
}
রাশিমালা *sqlite3_রাশিমালা_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  রাশিমালা *p_রাশিমালা,          /* Add the "COLLATE" clause to this expression */
  ধ্রুব বর্ণ *zC        /* The collating sequence name */
){
  Token s;
  প্রতিজ্ঞা( zC!=0 );
  sqlite3TokenInit(&s, (বর্ণ*)zC);
  প্রত্যাগম sqlite3_রাশিমালা_Add_Collate_Token(pParse, p_রাশিমালা, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
রাশিমালা *sqlite3_রাশিমালা_Skip_Collate(রাশিমালা *p_রাশিমালা){
  যাবত্( p_রাশিমালা && রাশিমালা_Has_Property(p_রাশিমালা, EP_Skip) ){
    প্রতিজ্ঞা( p_রাশিমালা->op==TK_COLLATE );
    p_রাশিমালা = p_রাশিমালা->p_বাম;
  }  
  প্রত্যাগম p_রাশিমালা;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
রাশিমালা *sqlite3_রাশিমালা_Skip_Collate_And_Likely(রাশিমালা *p_রাশিমালা){
  যাবত্( p_রাশিমালা && রাশিমালা_Has_Property(p_রাশিমালা, EP_Skip|EP_Unlikely) ){
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Unlikely) ){
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->x.pList->n_রাশিমালা>0 );
      প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
      p_রাশিমালা = p_রাশিমালা->x.pList->a[0].p_রাশিমালা;
    }অন্যথা যদি( p_রাশিমালা->op==TK_COLLATE ){
      p_রাশিমালা = p_রাশিমালা->p_বাম;
    }অন্যথা{
      ভঙ্গ;
    }
  }  
  প্রত্যাগম p_রাশিমালা;
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
CollSeq *sqlite3_রাশিমালা_Coll_Seq(Parse *pParse, ধ্রুব রাশিমালা *p_রাশিমালা){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ধ্রুব রাশিমালা *p = p_রাশিমালা;
  যাবত্( p ){
    পূর্ণাঙ্ক op = p->op;
    যদি( op==TK_REGISTER ) op = p->op2;
    যদি( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      পূর্ণাঙ্ক j;
      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p) );
      প্রতিজ্ঞা( p->y.pTab!=0 );
      যদি( (j = p->iColumn)>=0 ){
        ধ্রুব বর্ণ *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      ভঙ্গ;
    }
    যদি( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_বাম;
      অনুবর্ত;
    }
    যদি( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_রাশিমালা==SQLITE_AFF_DEFER)
    ){
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p) );
      p = p->x.pList->a[0].p_রাশিমালা;
      অনুবর্ত;
    }
    যদি( op==TK_COLLATE ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_Int_মান) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ভঙ্গ;
    }
    যদি( p->flags & EP_Collate ){
      যদি( p->p_বাম && (p->p_বাম->flags & EP_Collate)!=0 ){
        p = p->p_বাম;
      }অন্যথা{
        রাশিমালা *pNext  = p->p_ডান;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        প্রতিজ্ঞা( !রাশিমালা_Use_XList(p) || p->x.pList==0 || p->p_ডান==0 );
        যদি( রাশিমালা_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          পূর্ণাঙ্ক i;
          প্রতি(i=0; i<p->x.pList->n_রাশিমালা; i++){
            যদি( রাশিমালা_Has_Property(p->x.pList->a[i].p_রাশিমালা, EP_Collate) ){
              pNext = p->x.pList->a[i].p_রাশিমালা;
              ভঙ্গ;
            }
          }
        }
        p = pNext;
      }
    }অন্যথা{
      ভঙ্গ;
    }
  }
  যদি( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  প্রত্যাগম pColl;
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
CollSeq *sqlite3_রাশিমালা_NNColl_Seq(Parse *pParse, ধ্রুব রাশিমালা *p_রাশিমালা){
  CollSeq *p = sqlite3_রাশিমালা_Coll_Seq(pParse, p_রাশিমালা);
  যদি( p==0 ) p = pParse->db->pDfltColl;
  প্রতিজ্ঞা( p!=0 );
  প্রত্যাগম p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Coll_Seq_Match(Parse *pParse, ধ্রুব রাশিমালা *pE1, ধ্রুব রাশিমালা *pE2){
  CollSeq *pColl1 = sqlite3_রাশিমালা_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_রাশিমালা_NNColl_Seq(pParse, pE2);
  প্রতিজ্ঞা( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  প্রত্যাগম pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
বর্ণ sqlite3CompareAffinity(ধ্রুব রাশিমালা *p_রাশিমালা, বর্ণ aff2){
  বর্ণ aff1 = sqlite3_রাশিমালা_Affinity(p_রাশিমালা);
  যদি( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    যদি( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      প্রত্যাগম SQLITE_AFF_NUMERIC;
    }অন্যথা{
      প্রত্যাগম SQLITE_AFF_BLOB;
    }
  }অন্যথা{
    /* One side is a column, the other is not. Use the columns affinity. */
    প্রতিজ্ঞা( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    প্রত্যাগম (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
স্থির বর্ণ comparisonAffinity(ধ্রুব রাশিমালা *p_রাশিমালা){
  বর্ণ aff;
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_EQ || p_রাশিমালা->op==TK_IN || p_রাশিমালা->op==TK_LT ||
          p_রাশিমালা->op==TK_GT || p_রাশিমালা->op==TK_GE || p_রাশিমালা->op==TK_LE ||
          p_রাশিমালা->op==TK_NE || p_রাশিমালা->op==TK_IS || p_রাশিমালা->op==TK_ISNOT );
  প্রতিজ্ঞা( p_রাশিমালা->p_বাম );
  প্রতিজ্ঞা( !রাশিমালা_Use_XSelect(p_রাশিমালা) );
  aff = sqlite3_রাশিমালা_Affinity(p_রাশিমালা->p_বাম);
  যদি( p_রাশিমালা->p_ডান ){
    aff = sqlite3CompareAffinity(p_রাশিমালা->p_ডান, aff);
  }অন্যথা যদি( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  প্রত্যাগম aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
পূর্ণাঙ্ক sqlite3IndexAffinityOk(ধ্রুব রাশিমালা *p_রাশিমালা, বর্ণ idx_affinity){
  বর্ণ aff = comparisonAffinity(p_রাশিমালা);
  যদি( aff<SQLITE_AFF_TEXT ){
    প্রত্যাগম 1;
  }
  যদি( aff==SQLITE_AFF_TEXT ){
    প্রত্যাগম idx_affinity==SQLITE_AFF_TEXT;
  }
  প্রত্যাগম sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
স্থির u8 দ্বিমিক_Compare_P5(
  ধ্রুব রাশিমালা *pExpr1,   /* Left operand */
  ধ্রুব রাশিমালা *pExpr2,   /* Right operand */
  পূর্ণাঙ্ক jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (বর্ণ)sqlite3_রাশিমালা_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  প্রত্যাগম aff;
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
CollSeq *sqlite3_দ্বিমিক_Compare_Coll_Seq(
  Parse *pParse,
  ধ্রুব রাশিমালা *p_বাম,
  ধ্রুব রাশিমালা *p_ডান
){
  CollSeq *pColl;
  প্রতিজ্ঞা( p_বাম );
  যদি( p_বাম->flags & EP_Collate ){
    pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p_বাম);
  }অন্যথা যদি( p_ডান && (p_ডান->flags & EP_Collate)!=0 ){
    pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p_ডান);
  }অন্যথা{
    pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p_বাম);
    যদি( !pColl ){
      pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p_ডান);
    }
  }
  প্রত্যাগম pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_রাশিমালা_Compare_Coll_Seq(Parse *pParse, ধ্রুব রাশিমালা *p){
  যদি( রাশিমালা_Has_Property(p, EP_Commuted) ){
    প্রত্যাগম sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p->p_ডান, p->p_বাম);
  }অন্যথা{
    প্রত্যাগম sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p->p_বাম, p->p_ডান);
  }
}

/*
** Generate code for a comparison operator.
*/
স্থির পূর্ণাঙ্ক codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  রাশিমালা *p_বাম,      /* The left operand */
  রাশিমালা *p_ডান,     /* The right operand */
  পূর্ণাঙ্ক opcode,       /* The comparison opcode */
  পূর্ণাঙ্ক in1, পূর্ণাঙ্ক in2, /* Register holding operands */
  পূর্ণাঙ্ক dest,         /* Jump here if true.  */
  পূর্ণাঙ্ক jumpIfNull,   /* If true, jump if either operand is NULL */
  পূর্ণাঙ্ক isCommuted    /* The comparison has been commuted */
){
  পূর্ণাঙ্ক p5;
  পূর্ণাঙ্ক addr;
  CollSeq *p4;

  যদি( pParse->nErr ) প্রত্যাগম 0;
  যদি( isCommuted ){
    p4 = sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p_ডান, p_বাম);
  }অন্যথা{
    p4 = sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p_বাম, p_ডান);
  }
  p5 = দ্বিমিক_Compare_P5(p_বাম, p_ডান, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (শূন্য*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  প্রত্যাগম addr;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Vector(ধ্রুব রাশিমালা *p_রাশিমালা){
  প্রত্যাগম sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Vector_Size(ধ্রুব রাশিমালা *p_রাশিমালা){
  u8 op = p_রাশিমালা->op;
  যদি( op==TK_REGISTER ) op = p_রাশিমালা->op2;
  যদি( op==TK_VECTOR ){
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    প্রত্যাগম p_রাশিমালা->x.pList->n_রাশিমালা;
  }অন্যথা যদি( op==TK_SELECT ){
    প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
    প্রত্যাগম p_রাশিমালা->x.pSelect->pEList->n_রাশিমালা;
  }অন্যথা{
    প্রত্যাগম 1;
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
রাশিমালা *sqlite3VectorFieldSubexpr(রাশিমালা *pVector, পূর্ণাঙ্ক i){
  প্রতিজ্ঞা( i<sqlite3_রাশিমালা_Vector_Size(pVector) || pVector->op==TK_ERROR );
  যদি( sqlite3_রাশিমালা_Is_Vector(pVector) ){
    প্রতিজ্ঞা( pVector->op2==0 || pVector->op==TK_REGISTER );
    যদি( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(pVector) );
      প্রত্যাগম pVector->x.pSelect->pEList->a[i].p_রাশিমালা;
    }অন্যথা{
      প্রতিজ্ঞা( রাশিমালা_Use_XList(pVector) );
      প্রত্যাগম pVector->x.pList->a[i].p_রাশিমালা;
    }
  }
  প্রত্যাগম pVector;
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
রাশিমালা *sqlite3_রাশিমালা_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  রাশিমালা *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  পূর্ণাঙ্ক iField,          /* Which column of the vector to return */
  পূর্ণাঙ্ক nField           /* Total number of columns in the vector */
){
  রাশিমালা *pRet;
  যদি( pVector->op==TK_SELECT ){
    প্রতিজ্ঞা( রাশিমালা_Use_XSelect(pVector) );
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
    যদি( pRet ){
      রাশিমালা_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_বাম = pVector;
    }
  }অন্যথা{
    যদি( pVector->op==TK_VECTOR ){
      রাশিমালা **ppVector;
      প্রতিজ্ঞা( রাশিমালা_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_রাশিমালা;
      pVector = *ppVector;
      যদি( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        প্রত্যাগম pVector;
      }
    }
    pRet = sqlite3_রাশিমালা_Dup(pParse->db, pVector, 0);
  }
  প্রত্যাগম pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Code_Subselect(Parse *pParse, রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  যদি( p_রাশিমালা->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_রাশিমালা);
  }
#endif
  প্রত্যাগম reg;
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
স্থির পূর্ণাঙ্ক রাশিমালা_Vector_Register(
  Parse *pParse,                  /* Parse context */
  রাশিমালা *pVector,                  /* Vector to extract element from */
  পূর্ণাঙ্ক iField,                     /* Field to extract from pVector */
  পূর্ণাঙ্ক regSelect,                  /* First in array of registers */
  রাশিমালা *pTmp,                     /* Temporary space */
  রাশিমালা **pp_রাশিমালা,                  /* OUT: Expression element */
  পূর্ণাঙ্ক *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  প্রতিজ্ঞা( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  যদি( op==TK_REGISTER ){
    *pp_রাশিমালা = sqlite3VectorFieldSubexpr(pVector, iField);
    প্রত্যাগম pVector->iTable+iField;
  }
  যদি( op==TK_SELECT ){
    প্রতিজ্ঞা( রাশিমালা_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, পরিমাণ(রাশিমালা));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_বাম = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_রাশিমালা;
    *pp_রাশিমালা = pTmp;
    প্রত্যাগম regSelect+iField;
  }
  যদি( op==TK_VECTOR ){
    প্রতিজ্ঞা( রাশিমালা_Use_XList(pVector) );
    *pp_রাশিমালা = pVector->x.pList->a[iField].p_রাশিমালা;
    প্রত্যাগম sqlite3_রাশিমালা_Code_Temp(pParse, *pp_রাশিমালা, pRegFree);
  }
  প্রত্যাগম 0;
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
স্থির শূন্য codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  রাশিমালা *p_রাশিমালা,          /* The comparison operation */
  পূর্ণাঙ্ক dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
  রাশিমালা *p_ডান = p_রাশিমালা->p_ডান;
  পূর্ণাঙ্ক n_বাম = sqlite3_রাশিমালা_Vector_Size(p_বাম);
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক reg_বাম = 0;
  পূর্ণাঙ্ক reg_ডান = 0;
  u8 opx = op;
  পূর্ণাঙ্ক addrCmp = 0;
  পূর্ণাঙ্ক addrDone = sqlite3VdbeMakeLabel(pParse);
  পূর্ণাঙ্ক isCommuted = রাশিমালা_Has_Property(p_রাশিমালা,EP_Commuted);

  প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p_রাশিমালা,EP_Immutable) );
  যদি( pParse->nErr ) প্রত্যাগম;
  যদি( n_বাম!=sqlite3_রাশিমালা_Vector_Size(p_ডান) ){
    sqlite3_Error_বার্তা(pParse, "row value misused");
    প্রত্যাগম;
  }
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_EQ || p_রাশিমালা->op==TK_NE
       || p_রাশিমালা->op==TK_IS || p_রাশিমালা->op==TK_ISNOT
       || p_রাশিমালা->op==TK_LT || p_রাশিমালা->op==TK_GT
       || p_রাশিমালা->op==TK_LE || p_রাশিমালা->op==TK_GE
  );
  প্রতিজ্ঞা( p_রাশিমালা->op==op || (p_রাশিমালা->op==TK_IS && op==TK_EQ)
            || (p_রাশিমালা->op==TK_ISNOT && op==TK_NE) );
  প্রতিজ্ঞা( p5==0 || p_রাশিমালা->op!=op );
  প্রতিজ্ঞা( p5==SQLITE_NULLEQ || p_রাশিমালা->op==op );

  যদি( op==TK_LE ) opx = TK_LT;
  যদি( op==TK_GE ) opx = TK_GT;
  যদি( op==TK_NE ) opx = TK_EQ;

  reg_বাম = রাশিমালা_Code_Subselect(pParse, p_বাম);
  reg_ডান = রাশিমালা_Code_Subselect(pParse, p_ডান);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  প্রতি(i=0; 1 /*Loop exits by "break"*/; i++){
    পূর্ণাঙ্ক regFree1 = 0, regFree2 = 0;
    রাশিমালা *pL = 0, *pR = 0;
    রাশিমালা tmp1, tmp2;
    পূর্ণাঙ্ক r1, r2;
    প্রতিজ্ঞা( i>=0 && i<n_বাম );
    যদি( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = রাশিমালা_Vector_Register(pParse, p_বাম, i, reg_বাম, &tmp1, &pL, &regFree1);
    r2 = রাশিমালা_Vector_Register(pParse, p_ডান, i, reg_ডান, &tmp2, &pR, &regFree2);
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
    যদি( (opx==TK_LT || opx==TK_GT) && i<n_বাম-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    যদি( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }অন্যথা{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    যদি( i==n_বাম-1 ){
      ভঙ্গ;
    }
    যদি( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }অন্যথা{
      প্রতিজ্ঞা( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      যদি( i==n_বাম-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  যদি( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Check_Height(Parse *pParse, পূর্ণাঙ্ক nHeight){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক mxHeight = pParse->db->aLimit[SQLITE_LIMIT_রাশিমালা_DEPTH];
  যদি( nHeight>mxHeight ){
    sqlite3_Error_বার্তা(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  প্রত্যাগম rc;
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
স্থির শূন্য height_Of_রাশিমালা(ধ্রুব রাশিমালা *p, পূর্ণাঙ্ক *pnHeight){
  যদি( p ){
    যদি( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
স্থির শূন্য height_Of_রাশিমালা_List(ধ্রুব রাশিমালা_List *p, পূর্ণাঙ্ক *pnHeight){
  যদি( p ){
    পূর্ণাঙ্ক i;
    প্রতি(i=0; i<p->n_রাশিমালা; i++){
      height_Of_রাশিমালা(p->a[i].p_রাশিমালা, pnHeight);
    }
  }
}
স্থির শূন্য heightOfSelect(ধ্রুব Select *pSelect, পূর্ণাঙ্ক *pnHeight){
  ধ্রুব Select *p;
  প্রতি(p=pSelect; p; p=p->pPrior){
    height_Of_রাশিমালা(p->pWhere, pnHeight);
    height_Of_রাশিমালা(p->pHaving, pnHeight);
    height_Of_রাশিমালা(p->pLimit, pnHeight);
    height_Of_রাশিমালা_List(p->pEList, pnHeight);
    height_Of_রাশিমালা_List(p->pGroupBy, pnHeight);
    height_Of_রাশিমালা_List(p->pOrderBy, pnHeight);
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
স্থির শূন্য রাশিমালা_Set_Height(রাশিমালা *p){
  পূর্ণাঙ্ক nHeight = p->p_বাম ? p->p_বাম->nHeight : 0;
  যদি( NEVER(p->p_ডান) && p->p_ডান->nHeight>nHeight ){
    nHeight = p->p_ডান->nHeight;
  }
  যদি( রাশিমালা_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }অন্যথা যদি( p->x.pList ){
    height_Of_রাশিমালা_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_রাশিমালা_List_Flags(p->x.pList);
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
শূন্য sqlite3_রাশিমালা_Set_Height_And_Flags(Parse *pParse, রাশিমালা *p){
  যদি( pParse->nErr ) প্রত্যাগম;
  রাশিমালা_Set_Height(p);
  sqlite3_রাশিমালা_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
পূর্ণাঙ্ক sqlite3_Select_রাশিমালা_Height(ধ্রুব Select *p){
  পূর্ণাঙ্ক nHeight = 0;
  heightOfSelect(p, &nHeight);
  প্রত্যাগম nHeight;
}
#অন্যথা /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
শূন্য sqlite3_রাশিমালা_Set_Height_And_Flags(Parse *pParse, রাশিমালা *p){
  যদি( pParse->nErr ) প্রত্যাগম;
  যদি( p && রাশিমালা_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_রাশিমালা_List_Flags(p->x.pList);
  }
}
#define রাশিমালা_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
শূন্য sqlite3_রাশিমালা_Set_Error_Offset(রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক iOfst){
  যদি( p_রাশিমালা==0 ) প্রত্যাগম;
  যদি( NEVER(রাশিমালা_Use_WJoin(p_রাশিমালা)) ) প্রত্যাগম;
  p_রাশিমালা->w.iOfst = iOfst;
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
রাশিমালা *sqlite3_রাশিমালা_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  পূর্ণাঙ্ক op,                 /* Expression opcode */
  ধ্রুব Token *pToken,    /* Token argument.  Might be NULL */
  পূর্ণাঙ্ক dequote             /* True to dequote */
){
  রাশিমালা *pNew;
  পূর্ণাঙ্ক nExtra = pToken ? pToken->n+1 : 0;

  প্রতিজ্ঞা( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, পরিমাণ(রাশিমালা)+nExtra);
  যদি( pNew ){
    memset(pNew, 0, পরিমাণ(রাশিমালা));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    যদি( nExtra ){
      প্রতিজ্ঞা( pToken!=0 );
      pNew->u.zToken = (বর্ণ*)&pNew[1];
      প্রতিজ্ঞা( pToken->z!=0 || pToken->n==0 );
      যদি( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      যদি( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_রাশিমালা(pNew);
      }
    }
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  প্রত্যাগম pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
রাশিমালা *sqlite3_রাশিমালা(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  পূর্ণাঙ্ক op,                 /* Expression opcode */
  ধ্রুব বর্ণ *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  প্রত্যাগম sqlite3_রাশিমালা_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
রাশিমালা *sqlite3_রাশিমালা_Int32(sqlite3 *db, পূর্ণাঙ্ক iVal){
  রাশিমালা *pNew = sqlite3DbMallocRawNN(db, পরিমাণ(রাশিমালা));
  যদি( pNew ){
    memset(pNew, 0, পরিমাণ(রাশিমালা));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_মান|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_মান = iVal;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  প্রত্যাগম pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
শূন্য sqlite3_রাশিমালা_Attach_Subtrees(
  sqlite3 *db,
  রাশিমালা *p_মূল,
  রাশিমালা *p_বাম,
  রাশিমালা *p_ডান
){
  যদি( p_মূল==0 ){
    প্রতিজ্ঞা( db->mallocFailed );
    sqlite3_রাশিমালা_Delete(db, p_বাম);
    sqlite3_রাশিমালা_Delete(db, p_ডান);
  }অন্যথা{
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_মূল) );
    প্রতিজ্ঞা( p_মূল->x.pSelect==0 );
    যদি( p_ডান ){
      p_মূল->p_ডান = p_ডান;
      p_মূল->flags |= EP_Propagate & p_ডান->flags;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
      p_মূল->nHeight = p_ডান->nHeight+1;
    }অন্যথা{
      p_মূল->nHeight = 1;
#endif
    }
    যদি( p_বাম ){
      p_মূল->p_বাম = p_বাম;
      p_মূল->flags |= EP_Propagate & p_বাম->flags;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
      যদি( p_বাম->nHeight>=p_মূল->nHeight ){
        p_মূল->nHeight = p_বাম->nHeight+1;
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
রাশিমালা *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  পূর্ণাঙ্ক op,                 /* Expression opcode */
  রাশিমালা *p_বাম,            /* Left operand */
  রাশিমালা *p_ডান            /* Right operand */
){
  রাশিমালা *p;
  p = sqlite3DbMallocRawNN(pParse->db, পরিমাণ(রাশিমালা));
  যদি( p ){
    memset(p, 0, পরিমাণ(রাশিমালা));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_রাশিমালা_Attach_Subtrees(pParse->db, p, p_বাম, p_ডান);
    sqlite3_রাশিমালা_Check_Height(pParse, p->nHeight);
  }অন্যথা{
    sqlite3_রাশিমালা_Delete(pParse->db, p_বাম);
    sqlite3_রাশিমালা_Delete(pParse->db, p_ডান);
  }
  প্রত্যাগম p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
শূন্য sqlite3PExprAddSelect(Parse *pParse, রাশিমালা *p_রাশিমালা, Select *pSelect){
  যদি( p_রাশিমালা ){
    p_রাশিমালা->x.pSelect = pSelect;
    রাশিমালা_Set_Property(p_রাশিমালা, EP_xIsSelect|EP_Subquery);
    sqlite3_রাশিমালা_Set_Height_And_Flags(pParse, p_রাশিমালা);
  }অন্যথা{
    প্রতিজ্ঞা( pParse->db->mallocFailed );
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
Select *sqlite3_রাশিমালা_List_To_মানসমূহ(Parse *pParse, পূর্ণাঙ্ক nElem, রাশিমালা_List *pEList){
  পূর্ণাঙ্ক ii;
  Select *pRet = 0;
  প্রতিজ্ঞা( nElem>1 );
  প্রতি(ii=0; ii<pEList->n_রাশিমালা; ii++){
    Select *pSel;
    রাশিমালা *p_রাশিমালা = pEList->a[ii].p_রাশিমালা;
    পূর্ণাঙ্ক n_রাশিমালা_Elem;
    যদি( p_রাশিমালা->op==TK_VECTOR ){
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      n_রাশিমালা_Elem = p_রাশিমালা->x.pList->n_রাশিমালা;
    }অন্যথা{
      n_রাশিমালা_Elem = 1;
    }
    যদি( n_রাশিমালা_Elem!=nElem ){
      sqlite3_Error_বার্তা(pParse, "IN(...) element has %d term%s - expected %d",
          n_রাশিমালা_Elem, n_রাশিমালা_Elem>1?"s":"", nElem
      );
      ভঙ্গ;
    }
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    pSel = sqlite3SelectNew(pParse, p_রাশিমালা->x.pList, 0, 0, 0, 0, 0, SF_মানসমূহ,0);
    p_রাশিমালা->x.pList = 0;
    যদি( pSel ){
      যদি( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  যদি( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_মান;
  }
  sqlite3_রাশিমালা_List_Delete(pParse->db, pEList);
  প্রত্যাগম pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
রাশিমালা *sqlite3_রাশিমালা_And(Parse *pParse, রাশিমালা *p_বাম, রাশিমালা *p_ডান){
  sqlite3 *db = pParse->db;
  যদি( p_বাম==0  ){
    প্রত্যাগম p_ডান;
  }অন্যথা যদি( p_ডান==0 ){
    প্রত্যাগম p_বাম;
  }অন্যথা{
    u32 f = p_বাম->flags | p_ডান->flags;
    যদি( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      রাশিমালা *p;
      sqlite3_রাশিমালা_Deferred_Delete(pParse, p_বাম);
      sqlite3_রাশিমালা_Deferred_Delete(pParse, p_ডান);
      p = sqlite3_রাশিমালা_Int32(db, 0);
      যদি( p ) p->op2 = 1;
      প্রত্যাগম p;
    }অন্যথা{
      প্রত্যাগম sqlite3PExpr(pParse, TK_AND, p_বাম, p_ডান);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
রাশিমালা *sqlite3_রাশিমালা_Function(
  Parse *pParse,        /* Parsing context */
  রাশিমালা_List *pList,      /* Argument list */
  ধ্রুব Token *pToken,  /* Name of the function */
  পূর্ণাঙ্ক eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  রাশিমালা *pNew;
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( pToken );
  pNew = sqlite3_রাশিমালা_Alloc(db, TK_FUNCTION, pToken, 1);
  যদি( pNew==0 ){
    sqlite3_রাশিমালা_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    প্রত্যাগম 0;
  }
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (পূর্ণাঙ্ক)(pToken->z - pParse->zTail);
  যদি( pList
   && pList->n_রাশিমালা > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_বার্তা(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  রাশিমালা_Set_Property(pNew, EP_HasFunc);
  প্রতিজ্ঞা( রাশিমালা_Use_XList(pNew) );
  sqlite3_রাশিমালা_Set_Height_And_Flags(pParse, pNew);
  যদি( eDistinct==SF_Distinct ) রাশিমালা_Set_Property(pNew, EP_Distinct);
  প্রত্যাগম pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
শূন্য sqlite3_রাশিমালা_Order_By_Aggregate_Error(Parse *pParse, রাশিমালা *p){
  sqlite3_Error_বার্তা(pParse,
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
শূন্য sqlite3_রাশিমালা_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  রাশিমালা *p_রাশিমালা,          /* The function call to which ORDER BY is to be added */
  রাশিমালা_List *pOrderBy    /* The ORDER BY clause to add */
){
  রাশিমালা *pOB;
  sqlite3 *db = pParse->db;
  যদি( NEVER(pOrderBy==0) ){
    প্রতিজ্ঞা( db->mallocFailed );
    প্রত্যাগম;
  }
  যদি( p_রাশিমালা==0 ){
    প্রতিজ্ঞা( db->mallocFailed );
    sqlite3_রাশিমালা_List_Delete(db, pOrderBy);
    প্রত্যাগম;
  }
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
  প্রতিজ্ঞা( p_রাশিমালা->p_বাম==0 );
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  যদি( p_রাশিমালা->x.pList==0 || NEVER(p_রাশিমালা->x.pList->n_রাশিমালা==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_রাশিমালা_List_Delete_Generic, pOrderBy);
    প্রত্যাগম;
  }
  যদি( IsWindowFunc(p_রাশিমালা) ){
    sqlite3_রাশিমালা_Order_By_Aggregate_Error(pParse, p_রাশিমালা);
    sqlite3_রাশিমালা_List_Delete(db, pOrderBy);
    প্রত্যাগম;
  }
  যদি( pOrderBy->n_রাশিমালা>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_বার্তা(pParse, "too many terms in ORDER BY clause");
    sqlite3_রাশিমালা_List_Delete(db, pOrderBy);
    প্রত্যাগম;
  }

  pOB = sqlite3_রাশিমালা_Alloc(db, TK_ORDER, 0, 0);
  যদি( pOB==0 ){
    sqlite3_রাশিমালা_List_Delete(db, pOrderBy);
    প্রত্যাগম;
  }
  pOB->x.pList = pOrderBy;
  প্রতিজ্ঞা( রাশিমালা_Use_XList(pOB) );
  p_রাশিমালা->p_বাম = pOB;
  রাশিমালা_Set_Property(pOB, EP_FullSize);
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
শূন্য sqlite3_রাশিমালা_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ধ্রুব রাশিমালা *p_রাশিমালা,     /* The function invocation */
  ধ্রুব FuncDef *pDef    /* The function being invoked */
){
  প্রতিজ্ঞা( !IN_RENAME_OBJECT );
  প্রতিজ্ঞা( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    যদি( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_বার্তা(pParse, "unsafe use of %#T()", p_রাশিমালা);
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
শূন্য sqlite3_রাশিমালা_Assign_Var_সংখ্যা(Parse *pParse, রাশিমালা *p_রাশিমালা, u32 n){
  sqlite3 *db = pParse->db;
  ধ্রুব বর্ণ *z;
  ynVar x;

  যদি( p_রাশিমালা==0 ) প্রত্যাগম;
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান|EP_Reduced|EP_TokenOnly) );
  z = p_রাশিমালা->u.zToken;
  প্রতিজ্ঞা( z!=0 );
  প্রতিজ্ঞা( z[0]!=0 );
  প্রতিজ্ঞা( n==(u32)sqlite3Strlen30(z) );
  যদি( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    প্রতিজ্ঞা( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }অন্যথা{
    পূর্ণাঙ্ক doAdd = 0;
    যদি( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      পূর্ণাঙ্ক bOk;
      যদি( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }অন্যথা{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_সংখ্যা]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_সংখ্যা] );
      যদি( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_সংখ্যা] ){
        sqlite3_Error_বার্তা(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_সংখ্যা]);
        sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, p_রাশিমালা);
        প্রত্যাগম;
      }
      x = (ynVar)i;
      যদি( x>pParse->nVar ){
        pParse->nVar = (পূর্ণাঙ্ক)x;
        doAdd = 1;
      }অন্যথা যদি( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }অন্যথা যদি( sqlite3_VList_অঙ্ক_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }অন্যথা{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_অঙ্ক(pParse->pVList, z, n);
      যদি( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    যদি( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      যদি( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_রাশিমালা->iColumn = x;
  যদি( x>db->aLimit[SQLITE_LIMIT_VARIABLE_সংখ্যা] ){
    sqlite3_Error_বার্তা(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, p_রাশিমালা);
  }
}

/*
** Recursively delete an expression tree.
*/
স্থির SQLITE_NOINLINE শূন্য sqlite3_রাশিমালা_Delete_NN(sqlite3 *db, রাশিমালা *p){
  প্রতিজ্ঞা( p!=0 );
  প্রতিজ্ঞা( db!=0 );
রাশিমালা_Delete_Restart:
  প্রতিজ্ঞা( !রাশিমালা_Use_UValue(p) || p->u.i_মান>=0 );
  প্রতিজ্ঞা( !রাশিমালা_Use_YWin(p) || !রাশিমালা_Use_YSub(p) );
  প্রতিজ্ঞা( !রাশিমালা_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  প্রতিজ্ঞা( p->op!=TK_FUNCTION || !রাশিমালা_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  যদি( রাশিমালা_Has_Property(p, EP_Leaf) && !রাশিমালা_Has_Property(p, EP_TokenOnly) ){
    প্রতিজ্ঞা( p->p_বাম==0 );
    প্রতিজ্ঞা( p->p_ডান==0 );
    প্রতিজ্ঞা( !রাশিমালা_Use_XSelect(p) || p->x.pSelect==0 );
    প্রতিজ্ঞা( !রাশিমালা_Use_XList(p) || p->x.pList==0 );
  }
#endif
  যদি( !রাশিমালা_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    প্রতিজ্ঞা( (রাশিমালা_Use_XList(p) && p->x.pList==0) || p->p_ডান==0 );
    যদি( p->p_ডান ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_WinFunc) );
      sqlite3_রাশিমালা_Delete_NN(db, p->p_ডান);
    }অন্যথা যদি( রাশিমালা_Use_XSelect(p) ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }অন্যথা{
      sqlite3_রাশিমালা_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( রাশিমালা_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    যদি( p->p_বাম && p->op!=TK_SELECT_COLUMN ){
      রাশিমালা *p_বাম = p->p_বাম;
      যদি( !রাশিমালা_Has_Property(p, EP_Static)
       && !রাশিমালা_Has_Property(p_বাম, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_বাম;
        গচ্ছ রাশিমালা_Delete_Restart;
      }অন্যথা{
        sqlite3_রাশিমালা_Delete_NN(db, p_বাম);
      }
    }
  }
  যদি( !রাশিমালা_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
শূন্য sqlite3_রাশিমালা_Delete(sqlite3 *db, রাশিমালা *p){
  যদি( p ) sqlite3_রাশিমালা_Delete_NN(db, p);
}
শূন্য sqlite3_রাশিমালা_Delete_Generic(sqlite3 *db, শূন্য *p){
  যদি( ALWAYS(p) ) sqlite3_রাশিমালা_Delete_NN(db, (রাশিমালা*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
শূন্য sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  যদি( p==0 ){
    /* Nothing to clear */
  }অন্যথা যদি( p->pOn ){
    sqlite3_রাশিমালা_Delete_NN(db, p->pOn);
  }অন্যথা যদি( p->pUsing ){
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Deferred_Delete(Parse *pParse, রাশিমালা *p_রাশিমালা){
  প্রত্যাগম 0==sqlite3ParserAddCleanup(pParse, sqlite3_রাশিমালা_Delete_Generic, p_রাশিমালা);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
শূন্য sqlite3_রাশিমালা_Unmap_And_Delete(Parse *pParse, রাশিমালা *p){
  যদি( p ){
    যদি( IN_RENAME_OBJECT ){
      sqlite3_Rename_রাশিমালা_Unmap(pParse, p);
    }
    sqlite3_রাশিমালা_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Struct_Size(ধ্রুব রাশিমালা *p){
  যদি( রাশিমালা_Has_Property(p, EP_TokenOnly) ) প্রত্যাগম রাশিমালা_TOKENONLYSIZE;
  যদি( রাশিমালা_Has_Property(p, EP_Reduced) ) প্রত্যাগম রাশিমালা_REDUCEDSIZE;
  প্রত্যাগম রাশিমালা_FULLSIZE;
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
স্থির পূর্ণাঙ্ক duped_রাশিমালা_Struct_Size(ধ্রুব রাশিমালা *p, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক nSize;
  প্রতিজ্ঞা( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  প্রতিজ্ঞা( রাশিমালা_FULLSIZE<=0xfff );
  প্রতিজ্ঞা( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  যদি( 0==flags || রাশিমালা_Has_Property(p, EP_FullSize) ){
    nSize = রাশিমালা_FULLSIZE;
  }অন্যথা{
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_OuterON) );
    প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p, EP_NoReduce) );
    যদি( p->p_বাম || p->x.pList ){
      nSize = রাশিমালা_REDUCEDSIZE | EP_Reduced;
    }অন্যথা{
      প্রতিজ্ঞা( p->p_ডান==0 );
      nSize = রাশিমালা_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  প্রত্যাগম nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
স্থির পূর্ণাঙ্ক duped_রাশিমালা_গ্রন্থি_Size(ধ্রুব রাশিমালা *p, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক nByte = duped_রাশিমালা_Struct_Size(p, flags) & 0xfff;
  যদি( !রাশিমালা_Has_Property(p, EP_Int_মান) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  প্রত্যাগম ROUND8(nByte);
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
স্থির পূর্ণাঙ্ক duped_রাশিমালা_Size(ধ্রুব রাশিমালা *p){
  পূর্ণাঙ্ক nByte;
  প্রতিজ্ঞা( p!=0 );
  nByte = duped_রাশিমালা_গ্রন্থি_Size(p, EXPRDUP_REDUCE);
  যদি( p->p_বাম ) nByte += duped_রাশিমালা_Size(p->p_বাম);
  যদি( p->p_ডান ) nByte += duped_রাশিমালা_Size(p->p_ডান);
  প্রতিজ্ঞা( nByte==ROUND8(nByte) );
  প্রত্যাগম nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
প্রকারনির্দেশ সংরচনা EdupBuf EdupBuf;
সংরচনা EdupBuf {
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
স্থির রাশিমালা *রাশিমালা_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ধ্রুব রাশিমালা *p,        /* Expr tree to be duplicated */
  পূর্ণাঙ্ক dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  রাশিমালা *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  পূর্ণাঙ্ক nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  প্রতিজ্ঞা( db!=0 );
  প্রতিজ্ঞা( p );
  প্রতিজ্ঞা( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  প্রতিজ্ঞা( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  যদি( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    প্রতিজ্ঞা( sEdupBuf.zAlloc!=0 );
    প্রতিজ্ঞা( dupFlags==EXPRDUP_REDUCE );
  }অন্যথা{
    পূর্ণাঙ্ক nAlloc;
    যদি( dupFlags ){
      nAlloc = duped_রাশিমালা_Size(p);
    }অন্যথা যদি( !রাশিমালা_Has_Property(p, EP_Int_মান) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(রাশিমালা_FULLSIZE + nToken);
    }অন্যথা{
      nToken = 0;
      nAlloc = ROUND8(রাশিমালা_FULLSIZE);
    }
    প্রতিজ্ঞা( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (রাশিমালা *)sEdupBuf.zAlloc;
  প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT(pNew) );

  যদি( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ধ্রুব unsigned nStructSize = duped_রাশিমালা_Struct_Size(p, dupFlags);
    পূর্ণাঙ্ক nNewSize = nStructSize & 0xfff;
    যদি( nToken<0 ){
      যদি( !রাশিমালা_Has_Property(p, EP_Int_মান) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }অন্যথা{
        nToken = 0;
      }
    }
    যদি( dupFlags ){
      প্রতিজ্ঞা( (পূর্ণাঙ্ক)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      প্রতিজ্ঞা( রাশিমালা_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }অন্যথা{
      u32 nSize = (u32)রাশিমালা_Struct_Size(p);
      প্রতিজ্ঞা( (পূর্ণাঙ্ক)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (পূর্ণাঙ্ক)রাশিমালা_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      যদি( nSize<রাশিমালা_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, রাশিমালা_FULLSIZE-nSize);
      }
      nNewSize = রাশিমালা_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    রাশিমালা_Clear_VVAProperties(pNew);
    যদি( dupFlags ){
      রাশিমালা_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    প্রতিজ্ঞা( nToken>=0 );
    যদি( nToken>0 ){
      বর্ণ *zToken = pNew->u.zToken = (বর্ণ*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    যদি( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      যদি( রাশিমালা_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }অন্যথা{
        pNew->x.pList = sqlite3_রাশিমালা_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( রাশিমালা_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        প্রতিজ্ঞা( রাশিমালা_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      যদি( dupFlags ){
        যদি( p->op==TK_SELECT_COLUMN ){
          pNew->p_বাম = p->p_বাম;
          প্রতিজ্ঞা( p->p_ডান==0 
               || p->p_ডান==p->p_বাম
               || রাশিমালা_Has_Property(p->p_বাম, EP_Subquery) );
        }অন্যথা{
          pNew->p_বাম = p->p_বাম ?
                      রাশিমালা_Dup(db, p->p_বাম, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_ডান = p->p_ডান ?
                       রাশিমালা_Dup(db, p->p_ডান, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }অন্যথা{
        যদি( p->op==TK_SELECT_COLUMN ){
          pNew->p_বাম = p->p_বাম;
          প্রতিজ্ঞা( p->p_ডান==0 
               || p->p_ডান==p->p_বাম
               || রাশিমালা_Has_Property(p->p_বাম, EP_Subquery) );
        }অন্যথা{
          pNew->p_বাম = sqlite3_রাশিমালা_Dup(db, p->p_বাম, 0);
        }
        pNew->p_ডান = sqlite3_রাশিমালা_Dup(db, p->p_ডান, 0);
      }
    }
  }
  যদি( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, পরিমাণ(sEdupBuf));
  প্রতিজ্ঞা( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  প্রত্যাগম pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  যদি( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    যদি( pRet ){
      পূর্ণাঙ্ক i;
      pRet->nCte = p->nCte;
      প্রতি(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_রাশিমালা_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  প্রত্যাগম pRet;
}
#অন্যথা
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
স্থির পূর্ণাঙ্ক gatherSelectWindowsCallback(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_FUNCTION && রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_রাশিমালা->y.pWin;
    প্রতিজ্ঞা( pWin );
    প্রতিজ্ঞা( IsWindowFunc(p_রাশিমালা) );
    প্রতিজ্ঞা( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  প্রত্যাগম WRC_Continue;
}
স্থির পূর্ণাঙ্ক gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  প্রত্যাগম p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
স্থির শূন্য gatherSelectWindows(Select *p){
  Walker w;
  w.x_রাশিমালা_Callback = gatherSelectWindowsCallback;
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
রাশিমালা *sqlite3_রাশিমালা_Dup(sqlite3 *db, ধ্রুব রাশিমালা *p, পূর্ণাঙ্ক flags){
  প্রতিজ্ঞা( flags==0 || flags==EXPRDUP_REDUCE );
  প্রত্যাগম p ? রাশিমালা_Dup(db, p, flags, 0) : 0;
}
রাশিমালা_List *sqlite3_রাশিমালা_List_Dup(sqlite3 *db, ধ্রুব রাশিমালা_List *p, পূর্ণাঙ্ক flags){
  রাশিমালা_List *pNew;
  সংরচনা রাশিমালা_List_item *pItem;
  ধ্রুব সংরচনা রাশিমালা_List_item *pOldItem;
  পূর্ণাঙ্ক i;
  রাশিমালা *pPriorSelectColOld = 0;
  রাশিমালা *pPriorSelectColNew = 0;
  প্রতিজ্ঞা( db!=0 );
  যদি( p==0 ) প্রত্যাগম 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  যদি( pNew==0 ) প্রত্যাগম 0;
  pNew->n_রাশিমালা = p->n_রাশিমালা;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  প্রতি(i=0; i<p->n_রাশিমালা; i++, pItem++, pOldItem++){
    রাশিমালা *p_Old_রাশিমালা = pOldItem->p_রাশিমালা;
    রাশিমালা *p_New_রাশিমালা;
    pItem->p_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_Old_রাশিমালা, flags);
    যদি( p_Old_রাশিমালা
     && p_Old_রাশিমালা->op==TK_SELECT_COLUMN
     && (p_New_রাশিমালা = pItem->p_রাশিমালা)!=0
    ){
      যদি( p_New_রাশিমালা->p_ডান ){
        pPriorSelectColOld = p_Old_রাশিমালা->p_ডান;
        pPriorSelectColNew = p_New_রাশিমালা->p_ডান;
        p_New_রাশিমালা->p_বাম = p_New_রাশিমালা->p_ডান;
      }অন্যথা{
        যদি( p_Old_রাশিমালা->p_বাম!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_রাশিমালা->p_বাম;
          pPriorSelectColNew = sqlite3_রাশিমালা_Dup(db, pPriorSelectColOld, flags);
          p_New_রাশিমালা->p_ডান = pPriorSelectColNew;
        }
        p_New_রাশিমালা->p_বাম = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  প্রত্যাগম pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#যদি !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ধ্রুব SrcList *p, পূর্ণাঙ্ক flags){
  SrcList *pNew;
  পূর্ণাঙ্ক i;
  প্রতিজ্ঞা( db!=0 );
  যদি( p==0 ) প্রত্যাগম 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  যদি( pNew==0 ) প্রত্যাগম 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  প্রতি(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    ধ্রুব SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    যদি( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, পরিমাণ(Subquery));
      যদি( pNewSubq==0 ){
        প্রতিজ্ঞা( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }অন্যথা{
        memcpy(pNewSubq, pOldItem->u4.pSubq, পরিমাণ(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        যদি( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }অন্যথা যদি( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }অন্যথা{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    যদি( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }অন্যথা যদি( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_রাশিমালা_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }অন্যথা{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    যদি( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    যদি( pTab ){
      pTab->nTabRef++;
    }
    যদি( pOldItem->fg.isUsing ){
      প্রতিজ্ঞা( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }অন্যথা{
      pNewItem->u3.pOn = sqlite3_রাশিমালা_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  প্রত্যাগম pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ধ্রুব IdList *p){
  IdList *pNew;
  পূর্ণাঙ্ক i;
  প্রতিজ্ঞা( db!=0 );
  যদি( p==0 ) প্রত্যাগম 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  যদি( pNew==0 ) প্রত্যাগম 0;
  pNew->nId = p->nId;
  প্রতি(i=0; i<p->nId; i++){
    সংরচনা IdList_item *pNewItem = &pNew->a[i];
    ধ্রুব সংরচনা IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  প্রত্যাগম pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ধ্রুব Select *pDup, পূর্ণাঙ্ক flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  ধ্রুব Select *p;

  প্রতিজ্ঞা( db!=0 );
  প্রতি(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, পরিমাণ(*p) );
    যদি( pNew==0 ) ভঙ্গ;
    pNew->pEList = sqlite3_রাশিমালা_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_রাশিমালা_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_রাশিমালা_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_রাশিমালা_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_রাশিমালা_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_রাশিমালা_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    যদি( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    যদি( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      ভঙ্গ;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  প্রত্যাগম pRet;
}
#অন্যথা
Select *sqlite3SelectDup(sqlite3 *db, ধ্রুব Select *p, পূর্ণাঙ্ক flags){
  প্রতিজ্ঞা( p==0 );
  প্রত্যাগম 0;
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
স্থির ধ্রুব সংরচনা রাশিমালা_List_item zeroItem = {0};
SQLITE_NOINLINE রাশিমালা_List *sqlite3_রাশিমালা_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  রাশিমালা *p_রাশিমালা             /* Expression to be appended. Might be NULL */
){
  সংরচনা রাশিমালা_List_item *pItem;
  রাশিমালা_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  যদি( pList==0 ){
    sqlite3_রাশিমালা_Delete(db, p_রাশিমালা);
    প্রত্যাগম 0;
  }
  pList->nAlloc = 4;
  pList->n_রাশিমালা = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_রাশিমালা = p_রাশিমালা;
  প্রত্যাগম pList;
}
SQLITE_NOINLINE রাশিমালা_List *sqlite3_রাশিমালা_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  রাশিমালা_List *pList,        /* List to which to append. Might be NULL */
  রাশিমালা *p_রাশিমালা             /* Expression to be appended. Might be NULL */
){
  সংরচনা রাশিমালা_List_item *pItem;
  রাশিমালা_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  যদি( pNew==0 ){
    sqlite3_রাশিমালা_List_Delete(db, pList);
    sqlite3_রাশিমালা_Delete(db, p_রাশিমালা);
    প্রত্যাগম 0;
  }অন্যথা{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_রাশিমালা++];
  *pItem = zeroItem;
  pItem->p_রাশিমালা = p_রাশিমালা;
  প্রত্যাগম pList;
}
রাশিমালা_List *sqlite3_রাশিমালা_List_Append(
  Parse *pParse,          /* Parsing context */
  রাশিমালা_List *pList,        /* List to which to append. Might be NULL */
  রাশিমালা *p_রাশিমালা             /* Expression to be appended. Might be NULL */
){
  সংরচনা রাশিমালা_List_item *pItem;
  যদি( pList==0 ){
    প্রত্যাগম sqlite3_রাশিমালা_List_Append_New(pParse->db,p_রাশিমালা);
  }
  যদি( pList->nAlloc<pList->n_রাশিমালা+1 ){
    প্রত্যাগম sqlite3_রাশিমালা_List_Append_Grow(pParse->db,pList,p_রাশিমালা);
  }
  pItem = &pList->a[pList->n_রাশিমালা++];
  *pItem = zeroItem;
  pItem->p_রাশিমালা = p_রাশিমালা;
  প্রত্যাগম pList;
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
রাশিমালা_List *sqlite3_রাশিমালা_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  রাশিমালা_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  রাশিমালা *p_রাশিমালা            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  পূর্ণাঙ্ক n;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক iFirst = pList ? pList->n_রাশিমালা : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  যদি( NEVER(pColumns==0) ) গচ্ছ vector_append_error;
  যদি( p_রাশিমালা==0 ) গচ্ছ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  যদি( p_রাশিমালা->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা)) ){
    sqlite3_Error_বার্তা(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    গচ্ছ vector_append_error;
  }

  প্রতি(i=0; i<pColumns->nId; i++){
    রাশিমালা *p_Sub_রাশিমালা = sqlite3_রাশিমালা_For_Vector_Field(pParse, p_রাশিমালা, i, pColumns->nId);
    প্রতিজ্ঞা( p_Sub_রাশিমালা!=0 || db->mallocFailed );
    যদি( p_Sub_রাশিমালা==0 ) অনুবর্ত;
    pList = sqlite3_রাশিমালা_List_Append(pParse, pList, p_Sub_রাশিমালা);
    যদি( pList ){
      প্রতিজ্ঞা( pList->n_রাশিমালা==iFirst+i+1 );
      pList->a[pList->n_রাশিমালা-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  যদি( !db->mallocFailed && p_রাশিমালা->op==TK_SELECT && ALWAYS(pList!=0) ){
    রাশিমালা *pFirst = pList->a[iFirst].p_রাশিমালা;
    প্রতিজ্ঞা( pFirst!=0 );
    প্রতিজ্ঞা( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_ডান = p_রাশিমালা;
    p_রাশিমালা = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_রাশিমালা_Unmap_And_Delete(pParse, p_রাশিমালা);
  sqlite3IdListDelete(db, pColumns);
  প্রত্যাগম pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
শূন্য sqlite3_রাশিমালা_List_Set_Sort_Order(রাশিমালা_List *p, পূর্ণাঙ্ক iSortOrder, পূর্ণাঙ্ক eNulls){
  সংরচনা রাশিমালা_List_item *pItem;
  যদি( p==0 ) প্রত্যাগম;
  প্রতিজ্ঞা( p->n_রাশিমালা>0 );

  প্রতিজ্ঞা( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  প্রতিজ্ঞা( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  প্রতিজ্ঞা( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_রাশিমালা-1];
  প্রতিজ্ঞা( pItem->fg.bNulls==0 );
  যদি( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  যদি( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    যদি( iSortOrder!=eNulls ){
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
শূন্য sqlite3_রাশিমালা_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  রাশিমালা_List *pList,        /* List to which to add the span. */
  ধ্রুব Token *pName,     /* Name to be added */
  পূর্ণাঙ্ক dequote             /* True to cause the name to be dequoted */
){
  প্রতিজ্ঞা( pList!=0 || pParse->db->mallocFailed!=0 );
  প্রতিজ্ঞা( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  যদি( pList ){
    সংরচনা রাশিমালা_List_item *pItem;
    প্রতিজ্ঞা( pList->n_রাশিমালা>0 );
    pItem = &pList->a[pList->n_রাশিমালা-1];
    প্রতিজ্ঞা( pItem->zEName==0 );
    প্রতিজ্ঞা( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    যদি( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      যদি( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ধ্রুব শূন্য*)pItem->zEName, pName);
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
শূন্য sqlite3_রাশিমালা_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  রাশিমালা_List *pList,        /* List to which to add the span. */
  ধ্রুব বর্ণ *zStart,     /* Start of the span */
  ধ্রুব বর্ণ *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  প্রতিজ্ঞা( pList!=0 || db->mallocFailed!=0 );
  যদি( pList ){
    সংরচনা রাশিমালা_List_item *pItem = &pList->a[pList->n_রাশিমালা-1];
    প্রতিজ্ঞা( pList->n_রাশিমালা>0 );
    যদি( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
শূন্য sqlite3_রাশিমালা_List_Check_Length(
  Parse *pParse,
  রাশিমালা_List *pEList,
  ধ্রুব বর্ণ *zObject
){
  পূর্ণাঙ্ক mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_রাশিমালা==mx );
  testcase( pEList && pEList->n_রাশিমালা==mx+1 );
  যদি( pEList && pEList->n_রাশিমালা>mx ){
    sqlite3_Error_বার্তা(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
স্থির SQLITE_NOINLINE শূন্য রাশিমালা_List_Delete_NN(sqlite3 *db, রাশিমালা_List *pList){
  পূর্ণাঙ্ক i = pList->n_রাশিমালা;
  সংরচনা রাশিমালা_List_item *pItem =  pList->a;
  প্রতিজ্ঞা( pList->n_রাশিমালা>0 );
  প্রতিজ্ঞা( db!=0 );
  কুরু{
    sqlite3_রাশিমালা_Delete(db, pItem->p_রাশিমালা);
    যদি( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }যাবত্( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
শূন্য sqlite3_রাশিমালা_List_Delete(sqlite3 *db, রাশিমালা_List *pList){
  যদি( pList ) রাশিমালা_List_Delete_NN(db, pList);
}
শূন্য sqlite3_রাশিমালা_List_Delete_Generic(sqlite3 *db, শূন্য *pList){
  যদি( ALWAYS(pList) ) রাশিমালা_List_Delete_NN(db, (রাশিমালা_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_রাশিমালা_List_Flags(ধ্রুব রাশিমালা_List *pList){
  পূর্ণাঙ্ক i;
  u32 m = 0;
  প্রতিজ্ঞা( pList!=0 );
  প্রতি(i=0; i<pList->n_রাশিমালা; i++){
     রাশিমালা *p_রাশিমালা = pList->a[i].p_রাশিমালা;
     প্রতিজ্ঞা( p_রাশিমালা!=0 );
     m |= p_রাশিমালা->flags;
  }
  প্রত্যাগম m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
পূর্ণাঙ্ক sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  প্রত্যাগম WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ধ্রুব বর্ণ *zIn){
  যদি( sqlite3StrICmp(zIn, "true")==0  ) প্রত্যাগম EP_IsTrue;
  যদি( sqlite3StrICmp(zIn, "false")==0 ) প্রত্যাগম EP_IsFalse;
  প্রত্যাগম 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Id_To_True_False(রাশিমালা *p_রাশিমালা){
  u32 v;
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_ID || p_রাশিমালা->op==TK_STRING );
  যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Quoted|EP_Int_মান)
   && (v = sqlite3IsTrueOrFalse(p_রাশিমালা->u.zToken))!=0
  ){
    p_রাশিমালা->op = TK_TRUEFALSE;
    রাশিমালা_Set_Property(p_রাশিমালা, v);
    প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Truth_মান(ধ্রুব রাশিমালা *p_রাশিমালা){
  p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate_And_Likely((রাশিমালা*)p_রাশিমালা);
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_TRUEFALSE );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
  প্রতিজ্ঞা( sqlite3StrICmp(p_রাশিমালা->u.zToken,"true")==0
       || sqlite3StrICmp(p_রাশিমালা->u.zToken,"false")==0 );
  প্রত্যাগম p_রাশিমালা->u.zToken[4]==0;
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
রাশিমালা *sqlite3_রাশিমালা_Simplified_And_Or(রাশিমালা *p_রাশিমালা){
  প্রতিজ্ঞা( p_রাশিমালা!=0 );
  যদি( p_রাশিমালা->op==TK_AND || p_রাশিমালা->op==TK_OR ){
    রাশিমালা *p_ডান = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা->p_ডান);
    রাশিমালা *p_বাম = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা->p_বাম);
    যদি( রাশিমালা_Always_True(p_বাম) || রাশিমালা_Always_False(p_ডান) ){
      p_রাশিমালা = p_রাশিমালা->op==TK_AND ? p_ডান : p_বাম;
    }অন্যথা যদি( রাশিমালা_Always_True(p_ডান) || রাশিমালা_Always_False(p_বাম) ){
      p_রাশিমালা = p_রাশিমালা->op==TK_AND ? p_বাম : p_ডান;
    }
  }
  প্রত্যাগম p_রাশিমালা;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Eval_Rhs_First(রাশিমালা *p_রাশিমালা){
  যদি( রাশিমালা_Has_Property(p_রাশিমালা->p_বাম, EP_Subquery)
   && !রাশিমালা_Has_Property(p_রাশিমালা->p_ডান, EP_Subquery)
  ){
    প্রত্যাগম 1;
  }অন্যথা{
    প্রত্যাগম 0;
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
স্থির পূর্ণাঙ্ক রাশিমালা_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  রাশিমালা *p_রাশিমালা,       /* The comparison expression */
  পূর্ণাঙ্ক *pR1,          /* OUT: Register holding the left operand */
  পূর্ণাঙ্ক *pR2,          /* OUT: Register holding the right operand */
  পূর্ণাঙ্ক *pFree1,       /* OUT: Temp register to free if not zero */
  পূর্ণাঙ্ক *pFree2        /* OUT: Another temp register to free if not zero */
){
  পূর্ণাঙ্ক addrIsNull;
  পূর্ণাঙ্ক r1, r2;
  Vdbe *v = pParse->pVdbe;

  প্রতিজ্ঞা( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  যদি( রাশিমালা_Eval_Rhs_First(p_রাশিমালা) && sqlite3_রাশিমালা_Can_Be_Null(p_রাশিমালা->p_ডান) ){
    r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }অন্যথা{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, pFree1);
  যদি( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    যদি( রাশিমালা_Has_Property(p_রাশিমালা->p_ডান, EP_Subquery)
     && sqlite3_রাশিমালা_Can_Be_Null(p_রাশিমালা->p_বাম)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  প্রত্যাগম addrIsNull;
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক রাশিমালা_গ্রন্থি_Is_Constant_Function(
  Walker *pWalker,
  রাশিমালা *p_রাশিমালা
){
  পূর্ণাঙ্ক n;             /* Number of arguments */
  রাশিমালা_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly)
   || (pList = p_রাশিমালা->x.pList)==0
  ){;
    n = 0;
  }অন্যথা{
    n = pList->n_রাশিমালা;
    sqlite3_Walk_রাশিমালা_List(pWalker, pList);
    যদি( pWalker->eCode==0 ) প্রত্যাগম WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_রাশিমালা->u.zToken, n, ENC(db), 0);
  যদি( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    প্রত্যাগম WRC_Abort;
  }
  প্রত্যাগম WRC_Prune;
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
স্থির পূর্ণাঙ্ক রাশিমালা_গ্রন্থি_Is_Constant(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  প্রতিজ্ঞা( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  যদি( pWalker->eCode==2 && রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ){
    pWalker->eCode = 0;
    প্রত্যাগম WRC_Abort;
  }

  বিকল্প( p_রাশিমালা->op ){
    পক্ষ TK_FUNCTION:
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        প্রত্যাগম WRC_Abort;
      }অন্যথা যদি( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        যদি( pWalker->eCode==5 ) রাশিমালা_Set_Property(p_রাশিমালা, EP_FromDDL);
        প্রত্যাগম WRC_Continue;
      }অন্যথা যদি( রাশিমালা_Has_Property(p_রাশিমালা,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        প্রত্যাগম WRC_Continue;
      }অন্যথা যদি( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        প্রত্যাগম রাশিমালা_গ্রন্থি_Is_Constant_Function(pWalker, p_রাশিমালা);
      }অন্যথা{
        pWalker->eCode = 0;  /* Function call is not constant */
        প্রত্যাগম WRC_Abort;
      }
    পক্ষ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      যদি( sqlite3_রাশিমালা_Id_To_True_False(p_রাশিমালা) ){
        প্রত্যাগম WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    পক্ষ TK_COLUMN:
    পক্ষ TK_AGG_FUNCTION:
    পক্ষ TK_AGG_COLUMN:
      testcase( p_রাশিমালা->op==TK_ID );
      testcase( p_রাশিমালা->op==TK_COLUMN );
      testcase( p_রাশিমালা->op==TK_AGG_FUNCTION );
      testcase( p_রাশিমালা->op==TK_AGG_COLUMN );
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FixedCol) && pWalker->eCode!=2 ){
        প্রত্যাগম WRC_Continue;
      }
      যদি( pWalker->eCode==3 && p_রাশিমালা->iTable==pWalker->u.iCur ){
        প্রত্যাগম WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    পক্ষ TK_IF_NULL_ROW:
    পক্ষ TK_REGISTER:
    পক্ষ TK_DOT:
    পক্ষ TK_RAISE:
      testcase( p_রাশিমালা->op==TK_REGISTER );
      testcase( p_রাশিমালা->op==TK_IF_NULL_ROW );
      testcase( p_রাশিমালা->op==TK_DOT );
      testcase( p_রাশিমালা->op==TK_RAISE );
      pWalker->eCode = 0;
      প্রত্যাগম WRC_Abort;
    পক্ষ TK_IS:
    পক্ষ TK_ISNOT:
      যদি( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        প্রত্যাগম WRC_Abort;
      }
      প্রত্যাগম WRC_Continue;
    পক্ষ TK_VARIABLE:
      যদি( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_রাশিমালা->op = TK_NULL;
      }অন্যথা যদি( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        প্রত্যাগম WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    স্বভাব:
      testcase( p_রাশিমালা->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_রাশিমালা->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      প্রত্যাগম WRC_Continue;
  }
}
স্থির পূর্ণাঙ্ক রাশিমালা_Is_Const(Parse *pParse, রাশিমালা *p, পূর্ণাঙ্ক initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_রাশিমালা(&w, p);
  প্রত্যাগম w.eCode;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Constant(Parse *pParse, রাশিমালা *p){
  প্রত্যাগম রাশিমালা_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_List_Is_Constant(Parse *pParse, রাশিমালা_List *pList, পূর্ণাঙ্ক bNoIs){
  ধ্রুব পূর্ণাঙ্ক iInit = bNoIs ? 6 : 1;
  পূর্ণাঙ্ক ii;
  প্রতি(ii=0; ii<pList->n_রাশিমালা; ii++){
    যদি( 0==রাশিমালা_Is_Const(pParse, pList->a[ii].p_রাশিমালা, iInit) ) প্রত্যাগম 0;
  }
  প্রত্যাগম 1;
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
স্থির পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Constant_Not_Join(Parse *pParse, রাশিমালা *p){
  প্রত্যাগম রাশিমালা_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  প্রতিজ্ঞা( pSelect!=0 );
  প্রতিজ্ঞা( pWalker->eCode==3 || pWalker->eCode==0 );
  যদি( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    প্রত্যাগম WRC_Abort;
  }
  প্রত্যাগম WRC_Prune;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Table_Constant(রাশিমালা *p, পূর্ণাঙ্ক iCur, পূর্ণাঙ্ক bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Is_Constant;
  যদি( bAllowSubq ){
    w.xSelectCallback = রাশিমালা_Select_Walk_Table_Constant;
  }অন্যথা{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_রাশিমালা(&w, p);
  প্রত্যাগম w.eCode;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Single_Table_Constraint(
  রাশিমালা *p_রাশিমালা,                 /* The constraint */
  ধ্রুব SrcList *pSrcList,     /* Complete FROM clause */
  পূর্ণাঙ্ক iSrc,                    /* Which element of pSrcList to use */
  পূর্ণাঙ্ক bAllowSubq               /* Allow non-correlated subqueries */
){
  ধ্রুব SrcItem *pSrc = &pSrcList->a[iSrc];
  যদি( pSrc->fg.jointype & JT_LTORJ ){
    প্রত্যাগম 0;  /* rule (3) */
  }
  যদি( pSrc->fg.jointype & JT_বাম ){
    যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ) প্রত্যাগম 0;   /* rule (4a) */
    যদি( p_রাশিমালা->w.iJoin!=pSrc->iCursor ) প্রত্যাগম 0;         /* rule (4b) */
  }অন্যথা{
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ) প্রত্যাগম 0;    /* rule (5) */
  }
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    পূর্ণাঙ্ক jj;
    প্রতি(jj=0; jj<iSrc; jj++){
      যদি( p_রাশিমালা->w.iJoin==pSrcList->a[jj].iCursor ){
        যদি( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          প্রত্যাগম 0;  /* restriction (6) */
        }
        ভঙ্গ;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  প্রত্যাগম sqlite3_রাশিমালা_Is_Table_Constant(p_রাশিমালা, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
স্থির পূর্ণাঙ্ক রাশিমালা_গ্রন্থি_Is_Constant_Or_Group_By(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  রাশিমালা_List *pGroupBy = pWalker->u.pGroupBy;
  পূর্ণাঙ্ক i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  প্রতি(i=0; i<pGroupBy->n_রাশিমালা; i++){
    রাশিমালা *p = pGroupBy->a[i].p_রাশিমালা;
    যদি( sqlite3_রাশিমালা_Compare(0, p_রাশিমালা, p, -1)<2 ){
      CollSeq *pColl = sqlite3_রাশিমালা_NNColl_Seq(pWalker->pParse, p);
      যদি( sqlite3_Is_দ্বিমিক(pColl) ){
        প্রত্যাগম WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
    pWalker->eCode = 0;
    প্রত্যাগম WRC_Abort;
  }

  প্রত্যাগম রাশিমালা_গ্রন্থি_Is_Constant(pWalker, p_রাশিমালা);
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Constant_Or_Group_By(Parse *pParse, রাশিমালা *p, রাশিমালা_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_রাশিমালা(&w, p);
  প্রত্যাগম w.eCode;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Constant_Or_Function(রাশিমালা *p, u8 isInit){
  প্রতিজ্ঞা( isInit==0 || isInit==1 );
  প্রত্যাগম রাশিমালা_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Contains_Subquery(রাশিমালা *p){
  Walker w;
  w.eCode = 1;
  w.x_রাশিমালা_Callback = sqlite3_রাশিমালা_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_রাশিমালা(&w, p);
  প্রত্যাগম w.eCode==0;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Integer(ধ্রুব রাশিমালা *p, পূর্ণাঙ্ক *p_মান, Parse *pParse, পূর্ণাঙ্ক bRSI){
  পূর্ণাঙ্ক iSign = 1;   /* Either +1 or -1. */
  যাবত্( 1/*exit-by-break*/ ){
    যদি( NEVER(p==0) ) প্রত্যাগম 0;
    যদি( রাশিমালা_Use_UValue(p) ){
      প্রতিজ্ঞা( p->op==TK_INTEGER );
      *p_মান = p->u.i_মান*iSign;
      যদি( bRSI && pParse==0 && p->op2 ){
        প্রত্যাগম 0;
      }
      প্রত্যাগম 1;
    }
    যদি( p->op==TK_UPLUS ){
      p = p->p_বাম;
      pParse = 0;
      অনুবর্ত;
    }
    যদি( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_বাম;
      pParse = 0;
      অনুবর্ত;
    }
    যদি( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_মান *pVal; /* The variable */
      পূর্ণাঙ্ক rc = 0;          /* 1 if successful, 0 if failed */
      প্রতিজ্ঞা( iSign==1 );
      যদি( NEVER(pParse->pVdbe==0) ) ভঙ্গ;
      যদি( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ভঙ্গ;
      pVal = sqlite3_Vdbe_Get_Bound_মান(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      যদি( pVal ){
        i64 vv;
        যদি( sqlite3_মান_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_মান_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_মান = (পূর্ণাঙ্ক)vv;
          rc = 1;       
        }
        sqlite3_মান_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      প্রত্যাগম rc;
    }
    ভঙ্গ;
  }
  প্রত্যাগম 0;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Can_Be_Null(ধ্রুব রাশিমালা *p){
  u8 op;
  প্রতিজ্ঞা( p!=0 );
  যাবত্( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_বাম;
    প্রতিজ্ঞা( p!=0 );
  }
  op = p->op;
  যদি( op==TK_REGISTER ) op = p->op2;
  বিকল্প( op ){
    পক্ষ TK_INTEGER:
    পক্ষ TK_STRING:
    পক্ষ TK_FLOAT:
    পক্ষ TK_BLOB:
      প্রত্যাগম 0;
    পক্ষ TK_COLUMN:
      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p) );
      প্রত্যাগম রাশিমালা_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    স্বভাব:
      প্রত্যাগম 1;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Needs_No_Affinity_Change(ধ্রুব রাশিমালা *p, বর্ণ aff){
  u8 op;
  পূর্ণাঙ্ক unaryMinus = 0;
  যদি( aff==SQLITE_AFF_BLOB ) প্রত্যাগম 1;
  যাবত্( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    যদি( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_বাম;
  }
  op = p->op;
  যদি( op==TK_REGISTER ) op = p->op2;
  বিকল্প( op ){
    পক্ষ TK_INTEGER: {
      প্রত্যাগম aff>=SQLITE_AFF_NUMERIC;
    }
    পক্ষ TK_FLOAT: {
      প্রত্যাগম aff>=SQLITE_AFF_NUMERIC;
    }
    পক্ষ TK_STRING: {
      প্রত্যাগম !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    পক্ষ TK_BLOB: {
      প্রত্যাগম !unaryMinus;
    }
    পক্ষ TK_COLUMN: {
      প্রতিজ্ঞা( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      প্রত্যাগম aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    স্বভাব: {
      প্রত্যাগম 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
পূর্ণাঙ্ক sqlite3IsRowid(ধ্রুব বর্ণ *z){
  যদি( sqlite3StrICmp(z, "_ROWID_")==0 ) প্রত্যাগম 1;
  যদি( sqlite3StrICmp(z, "ROWID")==0 ) প্রত্যাগম 1;
  যদি( sqlite3StrICmp(z, "OID")==0 ) প্রত্যাগম 1;
  প্রত্যাগম 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ধ্রুব বর্ণ *sqlite3RowidAlias(Table *pTab){
  ধ্রুব বর্ণ *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  পূর্ণাঙ্ক ii;
  প্রতিজ্ঞা( VisibleRowid(pTab) );
  প্রতি(ii=0; ii<ArraySize(azOpt); ii++){
    যদি( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) প্রত্যাগম azOpt[ii];
  }
  প্রত্যাগম 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
স্থির Select *isCandidateForInOpt(ধ্রুব রাশিমালা *pX){
  Select *p;
  SrcList *pSrc;
  রাশিমালা_List *pEList;
  Table *pTab;
  পূর্ণাঙ্ক i;
  যদি( !রাশিমালা_Use_XSelect(pX) ) প্রত্যাগম 0;                 /* Not a subquery */
  যদি( রাশিমালা_Has_Property(pX, EP_VarSelect)  ) প্রত্যাগম 0;  /* Correlated subq */
  p = pX->x.pSelect;
  যদি( p->pPrior ) প্রত্যাগম 0;              /* Not a compound SELECT */
  যদি( p->selFlags & SF_Aggregate ){
    প্রত্যাগম 0; /* No GROUP BY keyword or aggregate functions */
  }
  প্রতিজ্ঞা( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  যদি( p->pLimit ) প্রত্যাগম 0;              /* Has no LIMIT clause */
  যদি( p->pWhere ) প্রত্যাগম 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  প্রতিজ্ঞা( pSrc!=0 );
  যদি( pSrc->nSrc!=1 ) প্রত্যাগম 0;          /* Single term in FROM clause */
  যদি( pSrc->a[0].fg.isSubquery) প্রত্যাগম 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( !IsView(pTab)  );              /* FROM clause is not a view */
  যদি( IsVirtual(pTab) ) প্রত্যাগম 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  প্রতিজ্ঞা( pEList!=0 );
  /* All SELECT results must be columns. */
  প্রতি(i=0; i<pEList->n_রাশিমালা; i++){
    রাশিমালা *pRes = pEList->a[i].p_রাশিমালা;
    যদি( pRes->op!=TK_COLUMN ) প্রত্যাগম 0;
    প্রতিজ্ঞা( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  প্রত্যাগম p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
স্থির শূন্য sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  পূর্ণাঙ্ক iCur,          /* Cursor for the index */
  পূর্ণাঙ্ক regHasNull,    /* Register in which to store hasNull flag */
  পূর্ণাঙ্ক eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  পূর্ণাঙ্ক addr1;
  পূর্ণাঙ্ক op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  যদি( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }অন্যথা{
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
স্থির পূর্ণাঙ্ক sqlite3InRhsIsConstant(Parse *pParse, রাশিমালা *pIn){
  রাশিমালা *pLHS;
  পূর্ণাঙ্ক res;
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_বাম;
  pIn->p_বাম = 0;
  res = sqlite3_রাশিমালা_Is_Constant(pParse, pIn);
  pIn->p_বাম = pLHS;
  প্রত্যাগম res;
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
পূর্ণাঙ্ক sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  রাশিমালা *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  পূর্ণাঙ্ক *prRhsHasNull,         /* Register holding NULL status.  See notes */
  পূর্ণাঙ্ক *aiMap,                /* Mapping from Index fields to RHS fields */
  পূর্ণাঙ্ক *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  পূর্ণাঙ্ক eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  পূর্ণাঙ্ক iTab;                             /* Cursor of the RHS table */
  পূর্ণাঙ্ক mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  প্রতিজ্ঞা( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  যদি( prRhsHasNull && রাশিমালা_Use_XSelect(pX) ){
    পূর্ণাঙ্ক i;
    রাশিমালা_List *pEList = pX->x.pSelect->pEList;
    প্রতি(i=0; i<pEList->n_রাশিমালা; i++){
      যদি( sqlite3_রাশিমালা_Can_Be_Null(pEList->a[i].p_রাশিমালা) ) ভঙ্গ;
    }
    যদি( i==pEList->n_রাশিমালা ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  যদি( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    পূর্ণাঙ্ক iDb;                               /* Database idx for pTab */
    রাশিমালা_List *pEList = p->pEList;
    পূর্ণাঙ্ক n_রাশিমালা = pEList->n_রাশিমালা;

    প্রতিজ্ঞা( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    প্রতিজ্ঞা( p->pEList->a[0].p_রাশিমালা!=0 ); /* Because of isCandidateForInOpt(p) */
    প্রতিজ্ঞা( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    প্রতিজ্ঞা( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    প্রতিজ্ঞা(v);  /* sqlite3GetVdbe() has always been previously called */
    যদি( n_রাশিমালা==1 && pEList->a[0].p_রাশিমালা->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      পূর্ণাঙ্ক iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }অন্যথা{
      Index *pIdx;                         /* Iterator variable */
      পূর্ণাঙ্ক affinity_ok = 1;
      পূর্ণাঙ্ক i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      প্রতি(i=0; i<n_রাশিমালা && affinity_ok; i++){
        রাশিমালা *pLhs = sqlite3VectorFieldSubexpr(pX->p_বাম, i);
        পূর্ণাঙ্ক iCol = pEList->a[i].p_রাশিমালা->iColumn;
        বর্ণ idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        বর্ণ cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        বিকল্প( cmpaff ){
          পক্ষ SQLITE_AFF_BLOB:
            ভঙ্গ;
          পক্ষ SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            প্রতিজ্ঞা( idxaff==SQLITE_AFF_TEXT );
            ভঙ্গ;
          স্বভাব:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      যদি( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        প্রতি(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          যদি( pIdx->nColumn<n_রাশিমালা ) অনুবর্ত;
          যদি( pIdx->pPartIdxWhere!=0 ) অনুবর্ত;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          যদি( pIdx->nColumn>=BMS-1 ) অনুবর্ত;
          যদি( mustBeUnique ){
            যদি( pIdx->n_চাবি_Col>n_রাশিমালা
             ||(pIdx->nColumn>n_রাশিমালা && !IsUniqueIndex(pIdx))
            ){
              অনুবর্ত;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          প্রতি(i=0; i<n_রাশিমালা; i++){
            রাশিমালা *pLhs = sqlite3VectorFieldSubexpr(pX->p_বাম, i);
            রাশিমালা *pRhs = pEList->a[i].p_রাশিমালা;
            CollSeq *pReq = sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, pLhs, pRhs);
            পূর্ণাঙ্ক j;
 
            প্রতি(j=0; j<n_রাশিমালা; j++){
              যদি( pIdx->aiColumn[j]!=pRhs->iColumn ) অনুবর্ত;
              প্রতিজ্ঞা( pIdx->azColl[j] );
              যদি( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                অনুবর্ত;
              }
              ভঙ্গ;
            }
            যদি( j==n_রাশিমালা ) ভঙ্গ;
            mCol = MASKBIT(j);
            যদি( mCol & colUsed ) ভঙ্গ; /* Each column used only once */
            colUsed |= mCol;
            যদি( aiMap ) aiMap[i] = j;
          }
 
          প্রতিজ্ঞা( n_রাশিমালা>0 && n_রাশিমালা<BMS );
          প্রতিজ্ঞা( i==n_রাশিমালা || colUsed!=(MASKBIT(n_রাশিমালা)-1) );
          যদি( colUsed==(MASKBIT(n_রাশিমালা)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            পূর্ণাঙ্ক iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            প্রতিজ্ঞা( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            যদি( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_রাশিমালা)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              যদি( n_রাশিমালা==1 ){
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
  যদি( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && রাশিমালা_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_রাশিমালা<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  যদি( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    পূর্ণাঙ্ক rMayHaveNull = 0;
    পূর্ণাঙ্ক bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    যদি( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }অন্যথা যদি( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    প্রতিজ্ঞা( pX->op==TK_IN );
    যদি( !bloomOk
     && রাশিমালা_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    যদি( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  যদি( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    পূর্ণাঙ্ক i, n;
    n = sqlite3_রাশিমালা_Vector_Size(pX->p_বাম);
    প্রতি(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  প্রত্যাগম eType;
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
স্থির বর্ণ *রাশিমালা_INAffinity(Parse *pParse, ধ্রুব রাশিমালা *p_রাশিমালা){
  রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
  পূর্ণাঙ্ক nVal = sqlite3_রাশিমালা_Vector_Size(p_বাম);
  Select *pSelect = রাশিমালা_Use_XSelect(p_রাশিমালা) ? p_রাশিমালা->x.pSelect : 0;
  বর্ণ *zRet;

  প্রতিজ্ঞা( p_রাশিমালা->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  যদি( zRet ){
    পূর্ণাঙ্ক i;
    প্রতি(i=0; i<nVal; i++){
      রাশিমালা *pA = sqlite3VectorFieldSubexpr(p_বাম, i);
      বর্ণ a = sqlite3_রাশিমালা_Affinity(pA);
      যদি( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_রাশিমালা, a);
      }অন্যথা{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  প্রত্যাগম zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
শূন্য sqlite3SubselectError(Parse *pParse, পূর্ণাঙ্ক nActual, পূর্ণাঙ্ক nExpect){
  যদি( pParse->nErr==0 ){
    ধ্রুব বর্ণ *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_বার্তা(pParse, zFmt, nActual, nExpect);
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
শূন্য sqlite3_Vector_Error_বার্তা(Parse *pParse, রাশিমালা *p_রাশিমালা){
#ifndef SQLITE_OMIT_SUBQUERY
  যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
    sqlite3SubselectError(pParse, p_রাশিমালা->x.pSelect->pEList->n_রাশিমালা, 1);
  }অন্যথা
#endif
  {
    sqlite3_Error_বার্তা(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
স্থির পূর্ণাঙ্ক findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  রাশিমালা *p_রাশিমালা,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  যদি( pNewSig==0 ) প্রত্যাগম 0;
  যদি( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) প্রত্যাগম 0;
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_IN );
  প্রতিজ্ঞা( !রাশিমালা_Use_YSub(p_রাশিমালা) );
  প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
  প্রতিজ্ঞা( p_রাশিমালা->x.pSelect!=0 );
  প্রতিজ্ঞা( (p_রাশিমালা->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  প্রতি(; pOp<pEnd; pOp++){
    যদি( pOp->p4type!=P4_SUBRTNSIG ) অনুবর্ত;
    প্রতিজ্ঞা( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    প্রতিজ্ঞা( pSig!=0 );
    যদি( !pSig->bComplete ) অনুবর্ত;
    যদি( pNewSig->selId!=pSig->selId ) অনুবর্ত;
    যদি( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) অনুবর্ত;
    p_রাশিমালা->y.sub.iAddr = pSig->iAddr;
    p_রাশিমালা->y.sub.regReturn = pSig->regReturn;
    p_রাশিমালা->iTable = pSig->iTable;
    রাশিমালা_Set_Property(p_রাশিমালা, EP_Subrtn);
    প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
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
শূন্য sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  রাশিমালা *p_রাশিমালা,            /* The IN operator */
  পূর্ণাঙ্ক iTab,               /* Use this cursor number */
  পূর্ণাঙ্ক allowBloom          /* True to allow the use of a Bloom filter */
){
  পূর্ণাঙ্ক addrOnce = 0;           /* Address of the OP_Once instruction at top */
  পূর্ণাঙ্ক addr;                   /* Address of OP_OpenEphemeral instruction */
  রাশিমালা *p_বাম;                /* the LHS of the IN operator */
  চাবি_Info *p_চাবি_Info = 0;      /* Key information */
  পূর্ণাঙ্ক nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );

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
  যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    প্রতিজ্ঞা( !রাশিমালা_Use_XSelect(p_রাশিমালা) || p_রাশিমালা->x.pSelect!=0 );
    যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) && (p_রাশিমালা->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, পরিমাণ(pSig[0]));
      যদি( pSig ){
        pSig->selId = p_রাশিমালা->x.pSelect->selId;
        pSig->zAff = রাশিমালা_INAffinity(pParse, p_রাশিমালা);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_রাশিমালা, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_রাশিমালা->x.pSelect->selId));
      }
      প্রতিজ্ঞা( রাশিমালা_Use_YSub(p_রাশিমালা) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_রাশিমালা->y.sub.regReturn,
                        p_রাশিমালা->y.sub.iAddr);
      প্রতিজ্ঞা( iTab!=p_রাশিমালা->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_রাশিমালা->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      যদি( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      প্রত্যাগম;
    }

    /* Begin coding the subroutine */
    প্রতিজ্ঞা( !রাশিমালা_Use_YWin(p_রাশিমালা) );
    রাশিমালা_Set_Property(p_রাশিমালা, EP_Subrtn);
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly|EP_Reduced) );
    p_রাশিমালা->y.sub.regReturn = ++pParse->nMem;
    p_রাশিমালা->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_রাশিমালা->y.sub.regReturn) + 1;
    যদি( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_রাশিমালা->y.sub.iAddr;
      pSig->regReturn = p_রাশিমালা->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ধ্রুব বর্ণ*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_বাম = p_রাশিমালা->p_বাম;
  nVal = sqlite3_রাশিমালা_Vector_Size(p_বাম);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_রাশিমালা->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_রাশিমালা->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
    VdbeComment((v, "Result of SELECT %u", p_রাশিমালা->x.pSelect->selId));
  }অন্যথা{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_চাবি_Info = sqlite3_চাবি_Info_Alloc(pParse->db, nVal, 1);
  প্রতিজ্ঞা( p_চাবি_Info!=0 || pParse->nErr );
  যদি( p_চাবি_Info==0 ) প্রত্যাগম;

  যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_রাশিমালা->x.pSelect;
    রাশিমালা_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    যদি( ALWAYS(pEList->n_রাশিমালা==nVal) ){
      Select *pCopy;
      SelectDest dest;
      পূর্ণাঙ্ক i;
      পূর্ণাঙ্ক rc;
      পূর্ণাঙ্ক addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = রাশিমালা_INAffinity(pParse, p_রাশিমালা);
      pSelect->iLimit = 0;
      প্রতিজ্ঞা( pEList!=0 );
      প্রতিজ্ঞা( pEList->n_রাশিমালা>0 );
      প্রতিজ্ঞা( sqlite3_চাবি_Info_Is_Writeable(p_চাবি_Info) );
      প্রতি(i=0; i<nVal; i++){
        রাশিমালা *p = sqlite3VectorFieldSubexpr(p_বাম, i);
        CollSeq *pColl;
        p_চাবি_Info->aColl[i] = pColl = sqlite3_দ্বিমিক_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_রাশিমালা
        );
        যদি( !sqlite3_Is_দ্বিমিক(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      যদি( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        পূর্ণাঙ্ক regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (শূন্য *)p_চাবি_Info, P4_KEYINFO);
        p_চাবি_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      যদি( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        যদি( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      যদি( rc ){
        sqlite3_চাবি_Info_Unref(p_চাবি_Info);
        প্রত্যাগম;
      }
    }
  }অন্যথা যদি( ALWAYS(p_রাশিমালা->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    বর্ণ affinity;            /* Affinity of the LHS of the IN */
    পূর্ণাঙ্ক i;
    রাশিমালা_List *pList = p_রাশিমালা->x.pList;
    সংরচনা রাশিমালা_List_item *pItem;
    পূর্ণাঙ্ক r1, r2;
    affinity = sqlite3_রাশিমালা_Affinity(p_বাম);
    যদি( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }অন্যথা যদি( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    প্রতিজ্ঞা( p_চাবি_Info!=0 );
    প্রতিজ্ঞা( sqlite3_চাবি_Info_Is_Writeable(p_চাবি_Info) );
    p_চাবি_Info->aColl[0] = sqlite3_রাশিমালা_Coll_Seq(pParse, p_রাশিমালা->p_বাম);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    প্রতি(i=pList->n_রাশিমালা, pItem=pList->a; i>0; i--, pItem++){
      রাশিমালা *pE2 = pItem->p_রাশিমালা;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      যদি( addrOnce && !sqlite3_রাশিমালা_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        রাশিমালা_Clear_Property(p_রাশিমালা, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_রাশিমালা_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ঢোকাও, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  যদি( pSig ) pSig->bComplete = 1;
  যদি( p_চাবি_Info ){
    sqlite3VdbeChangeP4(v, addr, (শূন্য *)p_চাবি_Info, P4_KEYINFO);
  }
  যদি( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    প্রতিজ্ঞা( রাশিমালা_Use_YSub(p_রাশিমালা) );
    প্রতিজ্ঞা( sqlite3VdbeGetOp(v,p_রাশিমালা->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_রাশিমালা->y.sub.regReturn,
                      p_রাশিমালা->y.sub.iAddr, 1);
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
পূর্ণাঙ্ক sqlite3CodeSubselect(Parse *pParse, রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  পূর্ণাঙ্ক rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  পূর্ণাঙ্ক nReg;                   /* Registers to allocate */
  রাশিমালা *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  পূর্ণাঙ্ক addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );
  যদি( pParse->nErr ) প্রত্যাগম 0;
  testcase( p_রাশিমালা->op==TK_EXISTS );
  testcase( p_রাশিমালা->op==TK_SELECT );
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_EXISTS || p_রাশিমালা->op==TK_SELECT );
  প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
  pSel = p_রাশিমালা->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    প্রতিজ্ঞা( রাশিমালা_Use_YSub(p_রাশিমালা) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_রাশিমালা->y.sub.regReturn,
                      p_রাশিমালা->y.sub.iAddr);
    প্রত্যাগম p_রাশিমালা->iTable;
  }

  /* Begin coding the subroutine */
  প্রতিজ্ঞা( !রাশিমালা_Use_YWin(p_রাশিমালা) );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Reduced|EP_TokenOnly) );
  রাশিমালা_Set_Property(p_রাশিমালা, EP_Subrtn);
  p_রাশিমালা->y.sub.regReturn = ++pParse->nMem;
  p_রাশিমালা->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_রাশিমালা->y.sub.regReturn) + 1;

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
  যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_VarSelect) ){
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
  nReg = p_রাশিমালা->op==TK_SELECT ? pSel->pEList->n_রাশিমালা : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  যদি( p_রাশিমালা->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    যদি( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_ডান ){
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
    }অন্যথা{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }অন্যথা{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  যদি( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    রাশিমালা *p_বাম = pSel->pLimit->p_বাম;
    যদি( রাশিমালা_Has_Property(p_বাম, EP_Int_মান)==0
     || (p_বাম->u.i_মান!=1 && p_বাম->u.i_মান!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_রাশিমালা_Int32(db, 0);
      যদি( pLimit ){
        pLimit->aff_রাশিমালা = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_রাশিমালা_Dup(db, p_বাম, 0), pLimit);
      }
      sqlite3_রাশিমালা_Deferred_Delete(pParse, p_বাম);
      pSel->pLimit->p_বাম = pLimit;
    }
  }অন্যথা{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_রাশিমালা_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  যদি( sqlite3Select(pParse, pSel, &dest) ){
    p_রাশিমালা->op2 = p_রাশিমালা->op;
    p_রাশিমালা->op = TK_ERROR;
    প্রত্যাগম 0;
  }
  p_রাশিমালা->iTable = rReg = dest.iSDParm;
  রাশিমালা_Set_VVAProperty(p_রাশিমালা, EP_NoReduce);
  যদি( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  যদি( dest.eDest==SRT_Mem 
   && sqlite3_রাশিমালা_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_রাশিমালা) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  প্রতিজ্ঞা( রাশিমালা_Use_YSub(p_রাশিমালা) );
  প্রতিজ্ঞা( sqlite3VdbeGetOp(v,p_রাশিমালা->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_রাশিমালা->y.sub.regReturn,
                    p_রাশিমালা->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  প্রত্যাগম rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Check_IN(Parse *pParse, রাশিমালা *pIn){
  পূর্ণাঙ্ক nVector = sqlite3_রাশিমালা_Vector_Size(pIn->p_বাম);
  যদি( রাশিমালা_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    যদি( nVector!=pIn->x.pSelect->pEList->n_রাশিমালা ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_রাশিমালা, nVector);
      প্রত্যাগম 1;
    }
  }অন্যথা যদি( nVector!=1 ){
    sqlite3_Vector_Error_বার্তা(pParse, pIn->p_বাম);
    প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
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
স্থির শূন্য sqlite3_রাশিমালা_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  রাশিমালা *p_রাশিমালা,          /* The IN expression */
  পূর্ণাঙ্ক destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  পূর্ণাঙ্ক destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  পূর্ণাঙ্ক rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  পূর্ণাঙ্ক eType;            /* Type of the RHS */
  পূর্ণাঙ্ক rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  পূর্ণাঙ্ক *aiMap = 0;       /* Map from vector field to index column */
  বর্ণ *zAff = 0;       /* Affinity string for comparisons */
  পূর্ণাঙ্ক nVector;          /* Size of vectors for this IN operator */
  পূর্ণাঙ্ক iDummy;           /* Dummy parameter to exprCodeVector() */
  রাশিমালা *p_বাম;          /* The LHS of the IN operator */
  পূর্ণাঙ্ক i;                /* loop counter */
  পূর্ণাঙ্ক destStep2;        /* Where to jump when NULLs seen in step 2 */
  পূর্ণাঙ্ক destStep6 = 0;    /* Start of code for Step 6 */
  পূর্ণাঙ্ক addrTruthOp;      /* Address of opcode that determines the IN is true */
  পূর্ণাঙ্ক destNotNull;      /* Jump here if a comparison is not true in step 6 */
  পূর্ণাঙ্ক addr_শীর্ষ;          /* Top of the step-6 loop */
  পূর্ণাঙ্ক iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p_রাশিমালা,EP_Immutable) );
  p_বাম = p_রাশিমালা->p_বাম;
  যদি( sqlite3_রাশিমালা_Check_IN(pParse, p_রাশিমালা) ) প্রত্যাগম;
  zAff = রাশিমালা_INAffinity(pParse, p_রাশিমালা);
  nVector = sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা->p_বাম);
  aiMap = (পূর্ণাঙ্ক*)sqlite3DbMallocZero(pParse->db, nVector*পরিমাণ(পূর্ণাঙ্ক));
  যদি( pParse->db->mallocFailed ) গচ্ছ sqlite3_রাশিমালা_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_রাশিমালা,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  প্রতিজ্ঞা( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  প্রতি(i=0; i<nVector; i++){
    পূর্ণাঙ্ক j, cnt;
    প্রতি(cnt=j=0; j<nVector; j++) যদি( aiMap[j]==i ) cnt++;
    প্রতিজ্ঞা( cnt==1 );
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
  প্রতিজ্ঞা( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = রাশিমালা_Code_Vector(pParse, p_বাম, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  যদি( eType==IN_INDEX_NOOP ){
    রাশিমালা_List *pList;
    CollSeq *pColl;
    পূর্ণাঙ্ক labelOk = sqlite3VdbeMakeLabel(pParse);
    পূর্ণাঙ্ক r2, regToFree;
    পূর্ণাঙ্ক regCkNull = 0;
    পূর্ণাঙ্ক ii;
    প্রতিজ্ঞা( nVector==1 );
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    pList = p_রাশিমালা->x.pList;
    pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p_রাশিমালা->p_বাম);
    যদি( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    প্রতি(ii=0; ii<pList->n_রাশিমালা; ii++){
      r2 = sqlite3_রাশিমালা_Code_Temp(pParse, pList->a[ii].p_রাশিমালা, &regToFree);
      যদি( regCkNull && sqlite3_রাশিমালা_Can_Be_Null(pList->a[ii].p_রাশিমালা) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      যদি( ii<pList->n_রাশিমালা-1 || destIfNull!=destIfFalse ){
        পূর্ণাঙ্ক op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (শূন্য*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_রাশিমালা-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_রাশিমালা-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_রাশিমালা-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_রাশিমালা-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }অন্যথা{
        পূর্ণাঙ্ক op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        প্রতিজ্ঞা( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (শূন্য*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    যদি( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    গচ্ছ sqlite3_রাশিমালা_Code_IN_finished;
  }

  যদি( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    যদি( nVector==1 ){
      বর্ণ aff = zAff[0];
      যদি( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_রাশিমালা_Affinity(p_বাম) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        পূর্ণাঙ্ক rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }অন্যথা{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    প্রতি(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    যদি( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      পূর্ণাঙ্ক rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      প্রতি(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  যদি( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }অন্যথা{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  প্রতি(i=0; i<nVector; i++){
    রাশিমালা *p = sqlite3VectorFieldSubexpr(p_রাশিমালা->p_বাম, i);
    যদি( pParse->nErr ) গচ্ছ sqlite3_রাশিমালা_Code_IN_oom_error;
    যদি( sqlite3_রাশিমালা_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  যদি( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    প্রতিজ্ঞা( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }অন্যথা{
    যদি( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subrtn) ){
        ধ্রুব VdbeOp *pOp = sqlite3VdbeGetOp(v, p_রাশিমালা->y.sub.iAddr);
        প্রতিজ্ঞা( pOp->opcode==OP_Once || pParse->nErr );
        যদি( pOp->p3>0 ){  /* tag-202407032019 */
          প্রতিজ্ঞা( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      গচ্ছ sqlite3_রাশিমালা_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  যদি( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  যদি( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  যদি( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  যদি( eType==IN_INDEX_INDEX_DESC ){
    addr_শীর্ষ = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }অন্যথা{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_শীর্ষ = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  যদি( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }অন্যথা{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  প্রতি(i=0; i<nVector; i++){
    রাশিমালা *p;
    CollSeq *pColl;
    পূর্ণাঙ্ক r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_বাম, i);
    যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
      রাশিমালা *pRhs = p_রাশিমালা->x.pSelect->pEList->a[i].p_রাশিমালা;
      pColl = sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p, pRhs);
    }অন্যথা{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (শূন্য*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  যদি( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_শীর্ষ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_রাশিমালা_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_রাশিমালা_Code_IN_oom_error:
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
স্থির শূন্য codeReal(Vdbe *v, ধ্রুব বর্ণ *z, পূর্ণাঙ্ক negateFlag, পূর্ণাঙ্ক iMem){
  যদি( ALWAYS(z!=0) ){
    দ্বিগুণ মান;
    sqlite3AtoF(z, &মান);
    প্রতিজ্ঞা( !sqlite3IsNaN(মান) ); /* The new AtoF never returns NaN */
    যদি( negateFlag ) মান = -মান;
    sqlite3VdbeAddDouble(v, iMem, মান);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
স্থির শূন্য codeInteger(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক negFlag, পূর্ণাঙ্ক iMem){
  Vdbe *v = pParse->pVdbe;
  যদি( p_রাশিমালা->flags & EP_Int_মান ){
    পূর্ণাঙ্ক i = p_রাশিমালা->u.i_মান;
    প্রতিজ্ঞা( i>=0 );
    যদি( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }অন্যথা{
    পূর্ণাঙ্ক c;
    i64 মান;
    ধ্রুব বর্ণ *z = p_রাশিমালা->u.zToken;
    প্রতিজ্ঞা( z!=0 );
    c = sqlite3DecOrHexToI64(z, &মান);
    যদি( (c==3 && !negFlag) || (c==2) || (negFlag && মান==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_বার্তা(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_রাশিমালা);
#অন্যথা
#ifndef SQLITE_OMIT_HEX_INTEGER
      যদি( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_বার্তা(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_রাশিমালা);
      }অন্যথা
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }অন্যথা{
      যদি( negFlag ){ মান = c==3 ? SMALLEST_INT64 : -মান; }
      sqlite3VdbeAddInt64(v, iMem, মান);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
শূন্য sqlite3_রাশিমালা_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  পূর্ণাঙ্ক iTabCur,    /* Cursor pointing to a table row */
  পূর্ণাঙ্ক iIdxCol,    /* The column of the index to be loaded */
  পূর্ণাঙ্ক regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  যদি( iTabCol==XN_রাশিমালা ){
    প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা );
    প্রতিজ্ঞা( pIdx->a_Col_রাশিমালা->n_রাশিমালা>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_রাশিমালা_Code_Copy(pParse, pIdx->a_Col_রাশিমালা->a[iIdxCol].p_রাশিমালা, regOut);
    pParse->iSelfTab = 0;
  }অন্যথা{
    sqlite3_রাশিমালা_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
শূন্য sqlite3_রাশিমালা_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  পূর্ণাঙ্ক regOut         /* Put the result in this register */
){
  পূর্ণাঙ্ক iAddr;
  Vdbe *v = pParse->pVdbe;
  পূর্ণাঙ্ক nErr = pParse->nErr;
  প্রতিজ্ঞা( v!=0 );
  প্রতিজ্ঞা( pParse->iSelfTab!=0 );
  যদি( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }অন্যথা{
    iAddr = 0;
  }
  sqlite3_রাশিমালা_Code_Copy(pParse, sqlite3_Column_রাশিমালা(pTab,pCol), regOut);
  যদি( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    পূর্ণাঙ্ক p3 = 2+(পূর্ণাঙ্ক)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (বর্ণ*)pTab, P4_TABLE);
  }অন্যথা যদি( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  যদি( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  যদি( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
শূন্য sqlite3_রাশিমালা_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  পূর্ণাঙ্ক iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  পূর্ণাঙ্ক iCol,       /* Index of the column to extract */
  পূর্ণাঙ্ক regOut      /* Extract the value into this register */
){
  Column *pCol;
  প্রতিজ্ঞা( v!=0 );
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( iCol!=XN_রাশিমালা );
  যদি( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }অন্যথা{
    পূর্ণাঙ্ক x;
    যদি( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      প্রত্যাগম;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }অন্যথা যদি( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      যদি( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_বার্তা(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }অন্যথা{
        পূর্ণাঙ্ক savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_রাশিমালা_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      প্রত্যাগম;
#endif
    }অন্যথা যদি( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_চাবি_Index(pTab), iCol);
    }অন্যথা{
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  পূর্ণাঙ্ক iColumn,     /* Index of the table column */
  পূর্ণাঙ্ক iTable,      /* The cursor pointing to the table */
  পূর্ণাঙ্ক iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  প্রতিজ্ঞা( pParse->pVdbe!=0 );
  প্রতিজ্ঞা( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  প্রতিজ্ঞা( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_রাশিমালা_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  যদি( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    যদি( pOp->opcode==OP_Column ) pOp->p5 = p5;
    যদি( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  প্রত্যাগম iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
শূন্য sqlite3_রাশিমালা_Code_Move(Parse *pParse, পূর্ণাঙ্ক iFrom, পূর্ণাঙ্ক iTo, পূর্ণাঙ্ক nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
শূন্য sqlite3_রাশিমালা_To_Register(রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক iReg){
  রাশিমালা *p = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা);
  যদি( NEVER(p==0) ) প্রত্যাগম;
  যদি( p->op==TK_REGISTER ){
    প্রতিজ্ঞা( p->iTable==iReg );
  }অন্যথা{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    রাশিমালা_Clear_Property(p, EP_Skip);
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
স্থির পূর্ণাঙ্ক রাশিমালা_Code_Vector(Parse *pParse, রাশিমালা *p, পূর্ণাঙ্ক *piFreeable){
  পূর্ণাঙ্ক iResult;
  পূর্ণাঙ্ক nResult = sqlite3_রাশিমালা_Vector_Size(p);
  যদি( nResult==1 ){
    iResult = sqlite3_রাশিমালা_Code_Temp(pParse, p, piFreeable);
  }অন্যথা{
    *piFreeable = 0;
    যদি( p->op==TK_SELECT ){
#যদি SQLITE_OMIT_SUBQUERY
      iResult = 0;
#অন্যথা
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }অন্যথা{
      পূর্ণাঙ্ক i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p) );
      প্রতি(i=0; i<nResult; i++){
        sqlite3_রাশিমালা_Code_Factorable(pParse, p->x.pList->a[i].p_রাশিমালা, i+iResult);
      }
    }
  }
  প্রত্যাগম iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
স্থির শূন্য setDoNotMergeFlagOnCopy(Vdbe *v){
  যদি( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  রাশিমালা_List *pFarg,      /* List of function arguments */
  পূর্ণাঙ্ক iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  পূর্ণাঙ্ক target            /* Store function result in this register */
){
  পূর্ণাঙ্ক nFarg;
  Vdbe *v = pParse->pVdbe;
  প্রতিজ্ঞা( v!=0 );
  প্রতিজ্ঞা( pFarg!=0 );
  nFarg = pFarg->n_রাশিমালা;
  প্রতিজ্ঞা( nFarg>0 );  /* All in-line functions have at least one argument */
  বিকল্প( iFuncId ){
    পক্ষ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      পূর্ণাঙ্ক endCoalesce = sqlite3VdbeMakeLabel(pParse);
      পূর্ণাঙ্ক i;
      প্রতিজ্ঞা( nFarg>=2 );
      sqlite3_রাশিমালা_Code(pParse, pFarg->a[0].p_রাশিমালা, target);
      প্রতি(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_রাশিমালা_Code(pParse, pFarg->a[i].p_রাশিমালা, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      ভঙ্গ;
    }
    পক্ষ INLINEFUNC_iif: {
      রাশিমালা case_রাশিমালা;
      memset(&case_রাশিমালা, 0, পরিমাণ(case_রাশিমালা));
      case_রাশিমালা.op = TK_CASE;
      case_রাশিমালা.x.pList = pFarg;
      প্রত্যাগম sqlite3_রাশিমালা_Code_Target(pParse, &case_রাশিমালা, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    পক্ষ INLINEFUNC_sqlite_offset: {
      রাশিমালা *pArg = pFarg->a[0].p_রাশিমালা;
      যদি( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ভঙ্গ;
    }
#endif
    স্বভাব: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      প্রতিজ্ঞা( nFarg==1 || nFarg==2 );
      target = sqlite3_রাশিমালা_Code_Target(pParse, pFarg->a[0].p_রাশিমালা, target);
      ভঙ্গ;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#যদি !defined(SQLITE_UNTESTABLE)
    পক্ষ INLINEFUNC_রাশিমালা_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      প্রতিজ্ঞা( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_রাশিমালা_Compare(0,pFarg->a[0].p_রাশিমালা, pFarg->a[1].p_রাশিমালা,-1),
         target);
      ভঙ্গ;
    }

    পক্ষ INLINEFUNC_রাশিমালা_implies_রাশিমালা: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      প্রতিজ্ঞা( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_রাশিমালা_Implies_রাশিমালা(pParse,pFarg->a[0].p_রাশিমালা, pFarg->a[1].p_রাশিমালা,-1),
         target);
      ভঙ্গ;
    }

    পক্ষ INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      রাশিমালা *pA1;
      প্রতিজ্ঞা( nFarg==2 );
      pA1 = pFarg->a[1].p_রাশিমালা;
      যদি( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_রাশিমালা_Implies_Non_Null_Row(pFarg->a[0].p_রাশিমালা,pA1->iTable,1),
           target);
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      ভঙ্গ;
    }

    পক্ষ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ধ্রুব বর্ণ *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      বর্ণ aff;
      প্রতিজ্ঞা( nFarg==1 );
      aff = sqlite3_রাশিমালা_Affinity(pFarg->a[0].p_রাশিমালা);
      প্রতিজ্ঞা( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ভঙ্গ;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  প্রত্যাগম target;
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
স্থির পূর্ণাঙ্ক রাশিমালা_গ্রন্থি_Can_Return_Subtype(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক n;
  FuncDef *pDef;
  sqlite3 *db;
  যদি( p_রাশিমালা->op==TK_CASE || p_রাশিমালা->op==TK_UPLUS 
   || p_রাশিমালা->op==TK_COLLATE || p_রাশিমালা->op==TK_CAST 
  ){
    প্রত্যাগম WRC_Continue;
  }
  যদি( p_রাশিমালা->op!=TK_FUNCTION && p_রাশিমালা->op!=TK_AGG_FUNCTION ){
    প্রত্যাগম WRC_Prune;
  }
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  db = pWalker->pParse->db;
  n = p_রাশিমালা->x.pList ? p_রাশিমালা->x.pList->n_রাশিমালা : 0;
  pDef = sqlite3FindFunction(db, p_রাশিমালা->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  যদি( pDef==0 ) প্রত্যাগম WRC_Continue;
#অন্যথা
  প্রতিজ্ঞা( pDef!=0 );
#endif
  যদি( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    প্রত্যাগম WRC_Abort;
  }
  প্রত্যাগম WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Can_Return_Subtype(Parse *pParse, রাশিমালা *p_রাশিমালা){
  Walker w;
  memset(&w, 0, পরিমাণ(w));
  w.pParse = pParse;
  w.x_রাশিমালা_Callback = রাশিমালা_গ্রন্থি_Can_Return_Subtype;
  sqlite3_Walk_রাশিমালা(&w, p_রাশিমালা);
  প্রত্যাগম w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক sqlite3_Indexed_রাশিমালা_Lookup(
  Parse *pParse,   /* The parsing context */
  রাশিমালা *p_রাশিমালা,     /* The expression to potentially bypass */
  পূর্ণাঙ্ক target       /* Where to store the result of the expression */
){
  Indexed_রাশিমালা *p;
  Vdbe *v;
  প্রতি(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 রাশিমালা_Aff;
    পূর্ণাঙ্ক iDataCur = p->iDataCur;
    যদি( iDataCur<0 ) অনুবর্ত;
    যদি( pParse->iSelfTab ){
      যদি( p->iDataCur!=pParse->iSelfTab-1 ) অনুবর্ত;
      iDataCur = -1;
    }
    যদি( sqlite3_রাশিমালা_Compare(0, p_রাশিমালা, p->p_রাশিমালা, iDataCur)!=0 ) অনুবর্ত;
    প্রতিজ্ঞা( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    রাশিমালা_Aff = sqlite3_রাশিমালা_Affinity(p_রাশিমালা);
    যদি( (রাশিমালা_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (রাশিমালা_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (রাশিমালা_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      অনুবর্ত;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_SubtArg)
     && sqlite3_রাশিমালা_Can_Return_Subtype(pParse, p_রাশিমালা) 
    ){
      অনুবর্ত;
    }

    v = pParse->pVdbe;
    প্রতিজ্ঞা( v!=0 );
    যদি( p->bMaybeNullRow || sqlite3_রাশিমালা_Is_Constant(pParse, p_রাশিমালা) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      পূর্ণাঙ্ক addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }অন্যথা{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    প্রত্যাগম target;
  }
  প্রত্যাগম -1;  /* Not found */
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
স্থির পূর্ণাঙ্ক রাশিমালা_Partidx_রাশিমালা_Lookup(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক iTarget){
  Indexed_রাশিমালা *p;
  প্রতি(p=pParse->p_Idx_Part_রাশিমালা; p; p=p->pIENext){
    যদি( p_রাশিমালা->iColumn==p->iIdxCol && p_রাশিমালা->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      পূর্ণাঙ্ক addr = 0;
      পূর্ণাঙ্ক ret;

      যদি( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_রাশিমালা_Code_Target(pParse, p->p_রাশিমালা, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ধ্রুব বর্ণ*)&p->aff, 1);
      যদি( sqlite3_রাশিমালা_Can_Return_Subtype(pParse, p->p_রাশিমালা) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      যদি( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      প্রত্যাগম ret;
    }
  }
  প্রত্যাগম 0;
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক রাশিমালা_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  রাশিমালা *p_রাশিমালা,       /* AND or OR expression to be coded */
  পূর্ণাঙ্ক target,        /* Put result in this register, guaranteed */
  পূর্ণাঙ্ক *pTmpReg       /* Write a temporary register here */
){
  পূর্ণাঙ্ক op;            /* The opcode.  TK_AND or TK_OR */
  পূর্ণাঙ্ক skipOp;        /* Opcode for the branch that skips one operand */
  পূর্ণাঙ্ক addrSkip;      /* Branch instruction that skips one of the operands */
  পূর্ণাঙ্ক regSS = 0;     /* Register holding computed operand when other omitted */
  পূর্ণাঙ্ক r1, r2;        /* Registers for left and right operands, respectively */
  রাশিমালা *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  প্রতিজ্ঞা( p_রাশিমালা!=0 );
  op = p_রাশিমালা->op;
  প্রতিজ্ঞা( op==TK_AND || op==TK_OR );
  প্রতিজ্ঞা( TK_AND==OP_And );            testcase( op==TK_AND );
  প্রতিজ্ঞা( TK_OR==OP_Or );              testcase( op==TK_OR );
  প্রতিজ্ঞা( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা);
  যদি( pAlt!=p_রাশিমালা ){
    r1 = sqlite3_রাশিমালা_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    প্রত্যাগম target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  যদি( রাশিমালা_Eval_Rhs_First(p_রাশিমালা) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা->p_ডান, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, pTmpReg);
  }অন্যথা{
    /* Compute the left operand first */
    r1 = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা->p_বাম, target);
    যদি( রাশিমালা_Has_Property(p_রাশিমালা->p_ডান, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }অন্যথা{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  যদি( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  প্রত্যাগম target;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Code_Target(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  পূর্ণাঙ্ক op;                   /* The opcode being coded */
  পূর্ণাঙ্ক inReg = target;       /* Results stored in register inReg */
  পূর্ণাঙ্ক regFree1 = 0;         /* If non-zero free this temporary register */
  পূর্ণাঙ্ক regFree2 = 0;         /* If non-zero free this temporary register */
  পূর্ণাঙ্ক r1, r2;               /* Various register numbers */
  রাশিমালা tempX;               /* Temporary expression node */
  পূর্ণাঙ্ক p5 = 0;

  প্রতিজ্ঞা( target>0 && target<=pParse->nMem );
  প্রতিজ্ঞা( v!=0 );

রাশিমালা_code_doover:
  যদি( p_রাশিমালা==0 ){
    op = TK_NULL;
  }অন্যথা যদি( pParse->pIdxEpr!=0
   && !রাশিমালা_Has_Property(p_রাশিমালা, EP_Leaf)
   && (r1 = sqlite3_Indexed_রাশিমালা_Lookup(pParse, p_রাশিমালা, target))>=0
  ){
    প্রত্যাগম r1;
  }অন্যথা{
    প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p_রাশিমালা,EP_Immutable) );
    op = p_রাশিমালা->op;
  }
  প্রতিজ্ঞা( op!=TK_ORDER );
  বিকল্প( op ){
    পক্ষ TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_রাশিমালা->pAggInfo;
      সংরচনা AggInfo_col *pCol;
      প্রতিজ্ঞা( pAggInfo!=0 );
      প্রতিজ্ঞা( p_রাশিমালা->iAgg>=0 );
      যদি( p_রাশিমালা->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        ভঙ্গ;
      }
      pCol = &pAggInfo->aCol[p_রাশিমালা->iAgg];
      যদি( !pAggInfo->directMode ){
        প্রত্যাগম AggInfoColumnReg(pAggInfo, p_রাশিমালা->iAgg);
      }অন্যথা যদি( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        যদি( pTab==0 ){
          /* No comment added */
        }অন্যথা যদি( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }অন্যথা{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          যদি( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        প্রত্যাগম target;
      }অন্যথা যদি( p_রাশিমালা->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_রাশিমালা->iTable, p_রাশিমালা->iColumn, target);
        প্রত্যাগম target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    পক্ষ TK_COLUMN: {
      পূর্ণাঙ্ক iTab = p_রাশিমালা->iTable;
      পূর্ণাঙ্ক iReg;
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        পূর্ণাঙ্ক aff;
        iReg = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা->p_বাম,target);
        প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
        প্রতিজ্ঞা( p_রাশিমালা->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_রাশিমালা->y.pTab, p_রাশিমালা->iColumn);
        যদি( aff>SQLITE_AFF_BLOB ){
          স্থির ধ্রুব বর্ণ zAff[] = "B\000C\000D\000E\000F";
          প্রতিজ্ঞা( SQLITE_AFF_BLOB=='A' );
          প্রতিজ্ঞা( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        প্রত্যাগম iReg;
      }
      যদি( iTab<0 ){
        যদি( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          পূর্ণাঙ্ক iSrc;
          পূর্ণাঙ্ক iCol = p_রাশিমালা->iColumn;
          প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
          pTab = p_রাশিমালা->y.pTab;
          প্রতিজ্ঞা( pTab!=0 );
          প্রতিজ্ঞা( iCol>=XN_ROWID );
          প্রতিজ্ঞা( iCol<pTab->nCol );
          যদি( iCol<0 ){
            প্রত্যাগম -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          যদি( pCol->colFlags & COLFLAG_GENERATED ){
            যদি( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_বার্তা(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              প্রত্যাগম 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            যদি( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_রাশিমালা_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            প্রত্যাগম iSrc;
          }অন্যথা
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          যদি( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            প্রত্যাগম target;
          }অন্যথা{
            প্রত্যাগম iSrc;
          }
        }অন্যথা{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      অন্যথা যদি( pParse->p_Idx_Part_রাশিমালা 
       && 0!=(r1 = রাশিমালা_Partidx_রাশিমালা_Lookup(pParse, p_রাশিমালা, target))
      ){
        প্রত্যাগম r1;
      }
      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->y.pTab!=0 );
      iReg = sqlite3_রাশিমালা_Code_Get_Column(pParse, p_রাশিমালা->y.pTab,
                               p_রাশিমালা->iColumn, iTab, target,
                               p_রাশিমালা->op2);
      প্রত্যাগম iReg;
    }
    পক্ষ TK_INTEGER: {
      codeInteger(pParse, p_রাশিমালা, 0, target);
      প্রত্যাগম target;
    }
    পক্ষ TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা), target);
      প্রত্যাগম target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    পক্ষ TK_FLOAT: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      codeReal(v, p_রাশিমালা->u.zToken, 0, target);
      প্রত্যাগম target;
    }
#endif
    পক্ষ TK_STRING: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3VdbeLoadString(v, target, p_রাশিমালা->u.zToken);
      প্রত্যাগম target;
    }
    পক্ষ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_রাশিমালা->y.nReg - 1);
      প্রত্যাগম target;
    }
    স্বভাব: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      প্রতিজ্ঞা( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      প্রত্যাগম target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    পক্ষ TK_BLOB: {
      পূর্ণাঙ্ক n;
      ধ্রুব বর্ণ *z;
      বর্ণ *zBlob;
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      প্রতিজ্ঞা( p_রাশিমালা->u.zToken[0]=='x' || p_রাশিমালা->u.zToken[0]=='X' );
      প্রতিজ্ঞা( p_রাশিমালা->u.zToken[1]=='\'' );
      z = &p_রাশিমালা->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      প্রতিজ্ঞা( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      প্রত্যাগম target;
    }
#endif
    পক্ষ TK_VARIABLE: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      প্রতিজ্ঞা( p_রাশিমালা->u.zToken!=0 );
      প্রতিজ্ঞা( p_রাশিমালা->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_রাশিমালা->iColumn, target);
      প্রত্যাগম target;
    }
    পক্ষ TK_REGISTER: {
      প্রত্যাগম p_রাশিমালা->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    পক্ষ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা->p_বাম, target);
      প্রতিজ্ঞা( inReg==target );
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_রাশিমালা->u.zToken, 0));
      প্রত্যাগম inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    পক্ষ TK_IS:
    পক্ষ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    পক্ষ TK_LT:
    পক্ষ TK_LE:
    পক্ষ TK_GT:
    পক্ষ TK_GE:
    পক্ষ TK_NE:
    পক্ষ TK_EQ: {
      রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
      পূর্ণাঙ্ক addrIsNull = 0;
      যদি( sqlite3_রাশিমালা_Is_Vector(p_বাম) ){
        codeVectorCompare(pParse, p_রাশিমালা, target, op, p5);
      }অন্যথা{
        যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = রাশিমালা_Compute_Operands(pParse, p_রাশিমালা,
                                     &r1, &r2, &regFree1, &regFree2);
        }অন্যথা{
          r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
          r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_বাম, p_রাশিমালা->p_ডান, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            রাশিমালা_Has_Property(p_রাশিমালা,EP_Commuted));
        প্রতিজ্ঞা(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        প্রতিজ্ঞা(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        প্রতিজ্ঞা(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        প্রতিজ্ঞা(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        প্রতিজ্ঞা(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        প্রতিজ্ঞা(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        যদি( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }অন্যথা{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          যদি( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ভঙ্গ;
    }
    পক্ষ TK_AND:
    পক্ষ TK_OR: {
      inReg = রাশিমালা_Code_Target_And_Or(pParse, p_রাশিমালা, target, &regFree1);
      ভঙ্গ;
    }
    পক্ষ TK_PLUS:
    পক্ষ TK_STAR:
    পক্ষ TK_MINUS:
    পক্ষ TK_REM:
    পক্ষ TK_BITAND:
    পক্ষ TK_BITOR:
    পক্ষ TK_SLASH:
    পক্ষ TK_LSHIFT:
    পক্ষ TK_RSHIFT:
    পক্ষ TK_CONCAT: {
      পূর্ণাঙ্ক addrIsNull;
      প্রতিজ্ঞা( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      প্রতিজ্ঞা( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      প্রতিজ্ঞা( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      প্রতিজ্ঞা( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      প্রতিজ্ঞা( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      প্রতিজ্ঞা( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      প্রতিজ্ঞা( TK_LSHIFT==OP_Shift_বাম );   testcase( op==TK_LSHIFT );
      প্রতিজ্ঞা( TK_RSHIFT==OP_Shift_ডান );  testcase( op==TK_RSHIFT );
      প্রতিজ্ঞা( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subquery) ){
        addrIsNull = রাশিমালা_Compute_Operands(pParse, p_রাশিমালা,
                                   &r1, &r2, &regFree1, &regFree2);
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
        r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      যদি( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      ভঙ্গ;
    }
    পক্ষ TK_UMINUS: {
      রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
      প্রতিজ্ঞা( p_বাম );
      যদি( p_বাম->op==TK_INTEGER ){
        codeInteger(pParse, p_বাম, 1, target);
        প্রত্যাগম target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }অন্যথা যদি( p_বাম->op==TK_FLOAT ){
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
        codeReal(v, p_বাম->u.zToken, 1, target);
        প্রত্যাগম target;
#endif
      }অন্যথা{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_মান|EP_TokenOnly;
        tempX.u.i_মান = 0;
        রাশিমালা_Clear_VVAProperties(&tempX);
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      ভঙ্গ;
    }
    পক্ষ TK_BITNOT:
    পক্ষ TK_NOT: {
      প্রতিজ্ঞা( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      প্রতিজ্ঞা( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      ভঙ্গ;
    }
    পক্ষ TK_TRUTH: {
      পূর্ণাঙ্ক isTrue;    /* IS TRUE or IS NOT TRUE */
      পূর্ণাঙ্ক bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা->p_ডান);
      bNormal = p_রাশিমালা->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      ভঙ্গ;
    }
    পক্ষ TK_ISNULL:
    পক্ষ TK_NOTNULL: {
      পূর্ণাঙ্ক addr;
      প্রতিজ্ঞা( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      প্রতিজ্ঞা( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      ভঙ্গ;
    }
    পক্ষ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_রাশিমালা->pAggInfo;
      যদি( pInfo==0
       || NEVER(p_রাশিমালা->iAgg<0)
       || NEVER(p_রাশিমালা->iAgg>=pInfo->nFunc)
      ){
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
        sqlite3_Error_বার্তা(pParse, "misuse of aggregate: %#T()", p_রাশিমালা);
      }অন্যথা{
        প্রত্যাগম AggInfoFuncReg(pInfo, p_রাশিমালা->iAgg);
      }
      ভঙ্গ;
    }
    পক্ষ TK_FUNCTION: {
      রাশিমালা_List *pFarg;       /* List of function arguments */
      পূর্ণাঙ্ক nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ধ্রুব বর্ণ *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      পূর্ণাঙ্ক i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
        প্রত্যাগম p_রাশিমালা->y.pWin->regResult;
      }
#endif

      যদি( ConstFactorOk(pParse)
       && sqlite3_রাশিমালা_Is_Constant_Not_Join(pParse,p_রাশিমালা)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        প্রত্যাগম sqlite3_রাশিমালা_Code_Run_Just_Once(pParse, p_রাশিমালা, -1);
      }
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly) );
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      pFarg = p_রাশিমালা->x.pList;
      nFarg = pFarg ? pFarg->n_রাশিমালা : 0;
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      zId = p_রাশিমালা->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      যদি( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      যদি( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_বার্তা(pParse, "unknown function: %#T()", p_রাশিমালা);
        ভঙ্গ;
      }
      যদি( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        প্রতিজ্ঞা( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        প্রতিজ্ঞা( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        প্রত্যাগম রাশিমালা_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }অন্যথা যদি( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_রাশিমালা_Function_Usable(pParse, p_রাশিমালা, pDef);
      }

      প্রতি(i=0; i<nFarg; i++){
        যদি( i<32 && sqlite3_রাশিমালা_Is_Constant(pParse, pFarg->a[i].p_রাশিমালা) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        যদি( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_রাশিমালা_Coll_Seq(pParse, pFarg->a[i].p_রাশিমালা);
        }
      }
      যদি( pFarg ){
        যদি( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }অন্যথা{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        যদি( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 রাশিমালা_Op;
          প্রতিজ্ঞা( nFarg==1 );
          প্রতিজ্ঞা( pFarg->a[0].p_রাশিমালা!=0 );
          রাশিমালা_Op = pFarg->a[0].p_রাশিমালা->op;
          যদি( রাশিমালা_Op==TK_COLUMN || রাশিমালা_Op==TK_AGG_COLUMN ){
            প্রতিজ্ঞা( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            প্রতিজ্ঞা( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            প্রতিজ্ঞা( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            প্রতিজ্ঞা( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_রাশিমালা->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_রাশিমালা_Code_রাশিমালা_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }অন্যথা{
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
      যদি( nFarg>=2 && রাশিমালা_Has_Property(p_রাশিমালা, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_রাশিমালা);
      }অন্যথা যদি( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_রাশিমালা);
      }
#endif
      যদি( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        যদি( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (বর্ণ *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_রাশিমালা->op2);
      যদি( nFarg ){
        যদি( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }অন্যথা{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      প্রত্যাগম target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    পক্ষ TK_EXISTS:
    পক্ষ TK_SELECT: {
      পূর্ণাঙ্ক nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      যদি( pParse->db->mallocFailed ){
        প্রত্যাগম 0;
      }অন্যথা যদি( op==TK_SELECT
             && ALWAYS( রাশিমালা_Use_XSelect(p_রাশিমালা) )
             && (nCol = p_রাশিমালা->x.pSelect->pEList->n_রাশিমালা)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }অন্যথা{
        প্রত্যাগম sqlite3CodeSubselect(pParse, p_রাশিমালা);
      }
      ভঙ্গ;
    }
    পক্ষ TK_SELECT_COLUMN: {
      পূর্ণাঙ্ক n;
      রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
      যদি( p_বাম->iTable==0 || pParse->withinRJSubrtn > p_বাম->op2 ){
        p_বাম->iTable = sqlite3CodeSubselect(pParse, p_বাম);
        p_বাম->op2 = pParse->withinRJSubrtn;
      }
      প্রতিজ্ঞা( p_বাম->op==TK_SELECT || p_বাম->op==TK_ERROR );
      n = sqlite3_রাশিমালা_Vector_Size(p_বাম);
      যদি( p_রাশিমালা->iTable!=n ){
        sqlite3_Error_বার্তা(pParse, "%d columns assigned %d values",
                                p_রাশিমালা->iTable, n);
      }
      প্রত্যাগম p_বাম->iTable + p_রাশিমালা->iColumn;
    }
    পক্ষ TK_IN: {
      পূর্ণাঙ্ক destIfFalse = sqlite3VdbeMakeLabel(pParse);
      পূর্ণাঙ্ক destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_রাশিমালা_Code_IN(pParse, p_রাশিমালা, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      প্রত্যাগম target;
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
    পক্ষ TK_BETWEEN: {
      রাশিমালা_Code_Between(pParse, p_রাশিমালা, target, 0, 0);
      প্রত্যাগম target;
    }
    পক্ষ TK_COLLATE: {
      যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        প্রতিজ্ঞা( p_রাশিমালা->p_বাম );
        sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা->p_বাম, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        প্রত্যাগম target;
      }অন্যথা{
        p_রাশিমালা = p_রাশিমালা->p_বাম;
        গচ্ছ রাশিমালা_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    পক্ষ TK_SPAN:
    পক্ষ TK_UPLUS: {
      p_রাশিমালা = p_রাশিমালা->p_বাম;
      গচ্ছ রাশিমালা_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    পক্ষ TK_TRIGGER: {
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
      পূর্ণাঙ্ক iCol;
      পূর্ণাঙ্ক p1;

      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
      pTab = p_রাশিমালা->y.pTab;
      iCol = p_রাশিমালা->iColumn;
      p1 = p_রাশিমালা->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      প্রতিজ্ঞা( p_রাশিমালা->iTable==0 || p_রাশিমালা->iTable==1 );
      প্রতিজ্ঞা( iCol>=-1 && iCol<pTab->nCol );
      প্রতিজ্ঞা( pTab->iPKey<0 || iCol!=pTab->iPKey );
      প্রতিজ্ঞা( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_রাশিমালা->iTable ? "new" : "old"),
        (p_রাশিমালা->iColumn<0 ? "rowid" : p_রাশিমালা->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      যদি( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      ভঙ্গ;
    }

    পক্ষ TK_VECTOR: {
      sqlite3_Error_বার্তা(pParse, "row value misused");
      ভঙ্গ;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    পক্ষ TK_IF_NULL_ROW: {
      পূর্ণাঙ্ক addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_রাশিমালা->pAggInfo;
      যদি( pAggInfo ){
        প্রতিজ্ঞা( p_রাশিমালা->iAgg>=0 && p_রাশিমালা->iAgg<pAggInfo->nColumn );
        যদি( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_রাশিমালা->iAgg);
          ভঙ্গ;
        }
        যদি( p_রাশিমালা->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_রাশিমালা->iAgg].iSorterColumn,
                            target);
          inReg = target;
          ভঙ্গ;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_রাশিমালা->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা->p_বাম, target);
      প্রতিজ্ঞা( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      ভঙ্গ;
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
    পক্ষ TK_CASE: {
      পূর্ণাঙ্ক endLabel;                     /* GOTO label for end of CASE stmt */
      পূর্ণাঙ্ক nextCase;                     /* GOTO label for next WHEN clause */
      পূর্ণাঙ্ক n_রাশিমালা;                        /* 2x number of WHEN terms */
      পূর্ণাঙ্ক i;                            /* Loop counter */
      রাশিমালা_List *pEList;                 /* List of WHEN terms */
      সংরচনা রাশিমালা_List_item *aListelem;  /* Array of WHEN terms */
      রাশিমালা opCompare;                   /* The X==Ei expression */
      রাশিমালা *pX;                         /* The X expression */
      রাশিমালা *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      রাশিমালা *pDel = 0;
      sqlite3 *db = pParse->db;

      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) && p_রাশিমালা->x.pList!=0 );
      প্রতিজ্ঞা(p_রাশিমালা->x.pList->n_রাশিমালা > 0);
      pEList = p_রাশিমালা->x.pList;
      aListelem = pEList->a;
      n_রাশিমালা = pEList->n_রাশিমালা;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      যদি( (pX = p_রাশিমালা->p_বাম)!=0 ){
        pDel = sqlite3_রাশিমালা_Dup(db, pX, 0);
        যদি( db->mallocFailed ){
          sqlite3_রাশিমালা_Delete(db, pDel);
          ভঙ্গ;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_রাশিমালা_To_Register(pDel, রাশিমালা_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, পরিমাণ(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_বাম = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      প্রতি(i=0; i<n_রাশিমালা-1; i=i+2){
        যদি( pX ){
          প্রতিজ্ঞা( pTest!=0 );
          opCompare.p_ডান = aListelem[i].p_রাশিমালা;
        }অন্যথা{
          pTest = aListelem[i].p_রাশিমালা;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_রাশিমালা_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_রাশিমালা->op==TK_COLUMN );
        sqlite3_রাশিমালা_Code(pParse, aListelem[i+1].p_রাশিমালা, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      যদি( (n_রাশিমালা&1)!=0 ){
        sqlite3_রাশিমালা_Code(pParse, pEList->a[n_রাশিমালা-1].p_রাশিমালা, target);
      }অন্যথা{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_রাশিমালা_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    পক্ষ TK_RAISE: {
      প্রতিজ্ঞা( p_রাশিমালা->aff_রাশিমালা==OE_Rollback
           || p_রাশিমালা->aff_রাশিমালা==OE_Abort
           || p_রাশিমালা->aff_রাশিমালা==OE_Fail
           || p_রাশিমালা->aff_রাশিমালা==OE_Ignore
      );
      যদি( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_বার্তা(pParse,
                       "RAISE() may only be used within a trigger-program");
        প্রত্যাগম 0;
      }
      যদি( p_রাশিমালা->aff_রাশিমালা==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      যদি( p_রাশিমালা->aff_রাশিমালা==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_রাশিমালা->aff_রাশিমালা, r1);
      }
      ভঙ্গ;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  প্রত্যাগম inReg;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  রাশিমালা *p_রাশিমালা,      /* The expression to code when the VDBE initializes */
  পূর্ণাঙ্ক regDest       /* Store the value in this register */
){
  রাশিমালা_List *p;
  প্রতিজ্ঞা( ConstFactorOk(pParse) );
  প্রতিজ্ঞা( regDest!=0 );
  p = pParse->p_Const_রাশিমালা;
  যদি( regDest<0 && p ){
    সংরচনা রাশিমালা_List_item *pItem;
    পূর্ণাঙ্ক i;
    প্রতি(pItem=p->a, i=p->n_রাশিমালা; i>0; pItem++, i--){
      যদি( pItem->fg.reusable
       && sqlite3_রাশিমালা_Compare(0,pItem->p_রাশিমালা,p_রাশিমালা,-1)==0
      ){
        প্রত্যাগম pItem->u.i_Const_রাশিমালা_Reg;
      }
    }
  }
  p_রাশিমালা = sqlite3_রাশিমালা_Dup(pParse->db, p_রাশিমালা, 0);
  যদি( p_রাশিমালা!=0 && রাশিমালা_Has_Property(p_রাশিমালা, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    পূর্ণাঙ্ক addr;
    প্রতিজ্ঞা( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    যদি( !pParse->db->mallocFailed ){
      যদি( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_রাশিমালা_Delete(pParse->db, p_রাশিমালা);
    sqlite3VdbeJumpHere(v, addr);
  }অন্যথা{
    p = sqlite3_রাশিমালা_List_Append(pParse, p, p_রাশিমালা);
    যদি( p ){
       সংরচনা রাশিমালা_List_item *pItem = &p->a[p->n_রাশিমালা-1];
       pItem->fg.reusable = regDest<0;
       যদি( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_রাশিমালা_Reg = regDest;
    }
    pParse->p_Const_রাশিমালা = p;
  }
  প্রত্যাগম regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE শূন্য sqlite3_রাশিমালা_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  পূর্ণাঙ্ক iReg,        /* First register to set to NULL */
  পূর্ণাঙ্ক nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  রাশিমালা t;
  memset(&t, 0, পরিমাণ(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_রাশিমালা_Code_Run_Just_Once(pParse, &t, iReg);
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Code_Temp(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক *pReg){
  পূর্ণাঙ্ক r2;
  p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা);
  যদি( ConstFactorOk(pParse)
   && ALWAYS(p_রাশিমালা!=0)
   && p_রাশিমালা->op!=TK_REGISTER
   && sqlite3_রাশিমালা_Is_Constant_Not_Join(pParse, p_রাশিমালা)
  ){
    *pReg  = 0;
    r2 = sqlite3_রাশিমালা_Code_Run_Just_Once(pParse, p_রাশিমালা, -1);
  }অন্যথা{
    পূর্ণাঙ্ক r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা, r1);
    যদি( r2==r1 ){
      *pReg = r1;
    }অন্যথা{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  প্রত্যাগম r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
শূন্য sqlite3_রাশিমালা_Code(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক target){
  পূর্ণাঙ্ক inReg;

  প্রতিজ্ঞা( p_রাশিমালা==0 || !রাশিমালা_Has_VVAProperty(p_রাশিমালা,EP_Immutable) );
  প্রতিজ্ঞা( target>0 && target<=pParse->nMem );
  প্রতিজ্ঞা( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  যদি( pParse->pVdbe==0 ) প্রত্যাগম;
  inReg = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা, target);
  যদি( inReg!=target ){
    u8 op;
    রাশিমালা *pX = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা);
    testcase( pX!=p_রাশিমালা );
    যদি( ALWAYS(pX)
     && (রাশিমালা_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }অন্যথা{
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
শূন্য sqlite3_রাশিমালা_Code_Copy(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক target){
  sqlite3 *db = pParse->db;
  p_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
  যদি( !db->mallocFailed ) sqlite3_রাশিমালা_Code(pParse, p_রাশিমালা, target);
  sqlite3_রাশিমালা_Delete(db, p_রাশিমালা);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
শূন্য sqlite3_রাশিমালা_Code_Factorable(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক target){
  যদি( pParse->okConstFactor && sqlite3_রাশিমালা_Is_Constant_Not_Join(pParse,p_রাশিমালা) ){
    sqlite3_রাশিমালা_Code_Run_Just_Once(pParse, p_রাশিমালা, target);
  }অন্যথা{
    sqlite3_রাশিমালা_Code_Copy(pParse, p_রাশিমালা, target);
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Code_রাশিমালা_List(
  Parse *pParse,     /* Parsing context */
  রাশিমালা_List *pList,   /* The expression list to be coded */
  পূর্ণাঙ্ক target,        /* Where to write results */
  পূর্ণাঙ্ক srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  সংরচনা রাশিমালা_List_item *pItem;
  পূর্ণাঙ্ক i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  প্রতিজ্ঞা( pList!=0 );
  প্রতিজ্ঞা( target>0 );
  প্রতিজ্ঞা( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_রাশিমালা;
  যদি( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  প্রতি(pItem=pList->a, i=0; i<n; i++, pItem++){
    রাশিমালা *p_রাশিমালা = pItem->p_রাশিমালা;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    যদি( pItem->fg.bSorterRef ){
      i--;
      n--;
    }অন্যথা
#endif
    যদি( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      যদি( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }অন্যথা{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }অন্যথা যদি( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_রাশিমালা_Is_Constant_Not_Join(pParse,p_রাশিমালা)
    ){
      sqlite3_রাশিমালা_Code_Run_Just_Once(pParse, p_রাশিমালা, target+i);
    }অন্যথা{
      পূর্ণাঙ্ক inReg = sqlite3_রাশিমালা_Code_Target(pParse, p_রাশিমালা, target+i);
      যদি( inReg!=target+i ){
        VdbeOp *pOp;
        যদি( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }অন্যথা{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  প্রত্যাগম n;
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
স্থির শূন্য রাশিমালা_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  রাশিমালা *p_রাশিমালা,      /* The BETWEEN expression */
  পূর্ণাঙ্ক dest,         /* Jump destination or storage location */
  শূন্য (*xJump)(Parse*,রাশিমালা*,পূর্ণাঙ্ক,পূর্ণাঙ্ক), /* Action to take */
  পূর্ণাঙ্ক jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  রাশিমালা রাশিমালা_And;     /* The AND operator in  x>=y AND x<=z  */
  রাশিমালা comp_বাম;    /* The  x>=y  term */
  রাশিমালা comp_ডান;   /* The  x<=z  term */
  পূর্ণাঙ্ক regFree1 = 0; /* Temporary use register */
  রাশিমালা *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_বাম, 0, পরিমাণ(রাশিমালা));
  memset(&comp_ডান, 0, পরিমাণ(রাশিমালা));
  memset(&রাশিমালা_And, 0, পরিমাণ(রাশিমালা));

  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  pDel = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা->p_বাম, 0);
  যদি( db->mallocFailed==0 ){
    রাশিমালা_And.op = TK_AND;
    রাশিমালা_And.p_বাম = &comp_বাম;
    রাশিমালা_And.p_ডান = &comp_ডান;
    comp_বাম.op = TK_GE;
    comp_বাম.p_বাম = pDel;
    comp_বাম.p_ডান = p_রাশিমালা->x.pList->a[0].p_রাশিমালা;
    comp_ডান.op = TK_LE;
    comp_ডান.p_বাম = pDel;
    comp_ডান.p_ডান = p_রাশিমালা->x.pList->a[1].p_রাশিমালা;
    sqlite3_রাশিমালা_To_Register(pDel, রাশিমালা_Code_Vector(pParse, pDel, &regFree1));
    যদি( xJump ){
      xJump(pParse, &রাশিমালা_And, dest, jumpIfNull);
    }অন্যথা{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_রাশিমালা_Code_Target(pParse, &রাশিমালা_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_রাশিমালা_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_রাশিমালা_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_রাশিমালা_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_রাশিমালা_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_রাশিমালা_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_রাশিমালা_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_রাশিমালা_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_রাশিমালা_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_রাশিমালা_If_False && jumpIfNull!=0 && regFree1!=0 );
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
শূন্য sqlite3_রাশিমালা_If_True(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক dest, পূর্ণাঙ্ক jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  পূর্ণাঙ্ক op = 0;
  পূর্ণাঙ্ক regFree1 = 0;
  পূর্ণাঙ্ক regFree2 = 0;
  পূর্ণাঙ্ক r1, r2;

  প্রতিজ্ঞা( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  যদি( NEVER(v==0) )     প্রত্যাগম;  /* Existence of VDBE checked by caller */
  যদি( NEVER(p_রাশিমালা==0) ) প্রত্যাগম;  /* No way this can happen */
  প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p_রাশিমালা, EP_Immutable) );
  op = p_রাশিমালা->op;
  বিকল্প( op ){
    পক্ষ TK_AND:
    পক্ষ TK_OR: {
      রাশিমালা *pAlt = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা);
      যদি( pAlt!=p_রাশিমালা ){
        sqlite3_রাশিমালা_If_True(pParse, pAlt, dest, jumpIfNull);
      }অন্যথা{
        রাশিমালা *pFirst, *pSecond;
        যদি( রাশিমালা_Eval_Rhs_First(p_রাশিমালা) ){
          pFirst = p_রাশিমালা->p_ডান;
          pSecond = p_রাশিমালা->p_বাম;
        }অন্যথা{
          pFirst = p_রাশিমালা->p_বাম;
          pSecond = p_রাশিমালা->p_ডান;
        }
        যদি( op==TK_AND ){
          পূর্ণাঙ্ক d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_রাশিমালা_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_রাশিমালা_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }অন্যথা{
          testcase( jumpIfNull==0 );
          sqlite3_রাশিমালা_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_রাশিমালা_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      ভঙ্গ;
    }
    পক্ষ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_রাশিমালা_If_False(pParse, p_রাশিমালা->p_বাম, dest, jumpIfNull);
      ভঙ্গ;
    }
    পক্ষ TK_TRUTH: {
      পূর্ণাঙ্ক isNot;      /* IS NOT TRUE or IS NOT FALSE */
      পূর্ণাঙ্ক isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_রাশিমালা->op2==TK_ISNOT;
      isTrue = sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা->p_ডান);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      যদি( isTrue ^ isNot ){
        sqlite3_রাশিমালা_If_True(pParse, p_রাশিমালা->p_বাম, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }অন্যথা{
        sqlite3_রাশিমালা_If_False(pParse, p_রাশিমালা->p_বাম, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ভঙ্গ;
    }
    পক্ষ TK_IS:
    পক্ষ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    পক্ষ TK_LT:
    পক্ষ TK_LE:
    পক্ষ TK_GT:
    পক্ষ TK_GE:
    পক্ষ TK_NE:
    পক্ষ TK_EQ: {
      পূর্ণাঙ্ক addrIsNull;
      যদি( sqlite3_রাশিমালা_Is_Vector(p_রাশিমালা->p_বাম) ) গচ্ছ default_রাশিমালা;
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = রাশিমালা_Compute_Operands(pParse, p_রাশিমালা,
                                   &r1, &r2, &regFree1, &regFree2);
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
        r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_রাশিমালা->p_বাম, p_রাশিমালা->p_ডান, op,
                  r1, r2, dest, jumpIfNull, রাশিমালা_Has_Property(p_রাশিমালা,EP_Commuted));
      প্রতিজ্ঞা(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      প্রতিজ্ঞা(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      প্রতিজ্ঞা(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      প্রতিজ্ঞা(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      প্রতিজ্ঞা(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      প্রতিজ্ঞা(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      যদি( addrIsNull ){
        যদি( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }অন্যথা{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ভঙ্গ;
    }
    পক্ষ TK_ISNULL:
    পক্ষ TK_NOTNULL: {
      প্রতিজ্ঞা( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      প্রতিজ্ঞা( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
      প্রতিজ্ঞা( regFree1==0 || regFree1==r1 );
      যদি( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ভঙ্গ;
    }
    পক্ষ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      রাশিমালা_Code_Between(pParse, p_রাশিমালা, dest, sqlite3_রাশিমালা_If_True, jumpIfNull);
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    পক্ষ TK_IN: {
      পূর্ণাঙ্ক destIfFalse = sqlite3VdbeMakeLabel(pParse);
      পূর্ণাঙ্ক destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_রাশিমালা_Code_IN(pParse, p_রাশিমালা, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ভঙ্গ;
    }
#endif
    স্বভাব: {
    default_রাশিমালা:
      যদি( রাশিমালা_Always_True(p_রাশিমালা) ){
        sqlite3VdbeGoto(v, dest);
      }অন্যথা যদি( রাশিমালা_Always_False(p_রাশিমালা) ){
        /* No-op */
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ভঙ্গ;
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
শূন্য sqlite3_রাশিমালা_If_False(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক dest, পূর্ণাঙ্ক jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  পূর্ণাঙ্ক op = 0;
  পূর্ণাঙ্ক regFree1 = 0;
  পূর্ণাঙ্ক regFree2 = 0;
  পূর্ণাঙ্ক r1, r2;

  প্রতিজ্ঞা( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  যদি( NEVER(v==0) ) প্রত্যাগম; /* Existence of VDBE checked by caller */
  যদি( p_রাশিমালা==0 )    প্রত্যাগম;
  প্রতিজ্ঞা( !রাশিমালা_Has_VVAProperty(p_রাশিমালা,EP_Immutable) );

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
  op = ((p_রাশিমালা->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_ISNULL || op==OP_NotNull );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_NOTNULL || op==OP_IsNull );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_NE || op==OP_Eq );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_EQ || op==OP_Ne );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_LT || op==OP_Ge );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_LE || op==OP_Gt );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_GT || op==OP_Le );
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_GE || op==OP_Lt );

  বিকল্প( p_রাশিমালা->op ){
    পক্ষ TK_AND:
    পক্ষ TK_OR: {
      রাশিমালা *pAlt = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা);
      যদি( pAlt!=p_রাশিমালা ){
        sqlite3_রাশিমালা_If_False(pParse, pAlt, dest, jumpIfNull);
      }অন্যথা{
        রাশিমালা *pFirst, *pSecond;
        যদি( রাশিমালা_Eval_Rhs_First(p_রাশিমালা) ){
          pFirst = p_রাশিমালা->p_ডান;
          pSecond = p_রাশিমালা->p_বাম;
        }অন্যথা{
          pFirst = p_রাশিমালা->p_বাম;
          pSecond = p_রাশিমালা->p_ডান;
        }
        যদি( p_রাশিমালা->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_রাশিমালা_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_রাশিমালা_If_False(pParse, pSecond, dest, jumpIfNull);
        }অন্যথা{
          পূর্ণাঙ্ক d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_রাশিমালা_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_রাশিমালা_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ভঙ্গ;
    }
    পক্ষ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_রাশিমালা_If_True(pParse, p_রাশিমালা->p_বাম, dest, jumpIfNull);
      ভঙ্গ;
    }
    পক্ষ TK_TRUTH: {
      পূর্ণাঙ্ক isNot;   /* IS NOT TRUE or IS NOT FALSE */
      পূর্ণাঙ্ক isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_রাশিমালা->op2==TK_ISNOT;
      isTrue = sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা->p_ডান);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      যদি( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_রাশিমালা_If_False(pParse, p_রাশিমালা->p_বাম, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }অন্যথা{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_রাশিমালা_If_True(pParse, p_রাশিমালা->p_বাম, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ভঙ্গ;
    }
    পক্ষ TK_IS:
    পক্ষ TK_ISNOT:
      testcase( p_রাশিমালা->op==TK_IS );
      testcase( p_রাশিমালা->op==TK_ISNOT );
      op = (p_রাশিমালা->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    পক্ষ TK_LT:
    পক্ষ TK_LE:
    পক্ষ TK_GT:
    পক্ষ TK_GE:
    পক্ষ TK_NE:
    পক্ষ TK_EQ: {
      পূর্ণাঙ্ক addrIsNull;
      যদি( sqlite3_রাশিমালা_Is_Vector(p_রাশিমালা->p_বাম) ) গচ্ছ default_রাশিমালা;
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = রাশিমালা_Compute_Operands(pParse, p_রাশিমালা,
                                   &r1, &r2, &regFree1, &regFree2);
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
        r2 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_ডান, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_রাশিমালা->p_বাম, p_রাশিমালা->p_ডান, op,
                  r1, r2, dest, jumpIfNull,রাশিমালা_Has_Property(p_রাশিমালা,EP_Commuted));
      প্রতিজ্ঞা(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      প্রতিজ্ঞা(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      প্রতিজ্ঞা(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      প্রতিজ্ঞা(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      প্রতিজ্ঞা(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      প্রতিজ্ঞা(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      যদি( addrIsNull ){
        যদি( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }অন্যথা{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      ভঙ্গ;
    }
    পক্ষ TK_ISNULL:
    পক্ষ TK_NOTNULL: {
      r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা->p_বাম, &regFree1);
      প্রতিজ্ঞা( regFree1==0 || regFree1==r1 );
      যদি( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ভঙ্গ;
    }
    পক্ষ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      রাশিমালা_Code_Between(pParse, p_রাশিমালা, dest, sqlite3_রাশিমালা_If_False, jumpIfNull);
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    পক্ষ TK_IN: {
      যদি( jumpIfNull ){
        sqlite3_রাশিমালা_Code_IN(pParse, p_রাশিমালা, dest, dest);
      }অন্যথা{
        পূর্ণাঙ্ক destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_রাশিমালা_Code_IN(pParse, p_রাশিমালা, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ভঙ্গ;
    }
#endif
    স্বভাব: {
    default_রাশিমালা:
      যদি( রাশিমালা_Always_False(p_রাশিমালা) ){
        sqlite3VdbeGoto(v, dest);
      }অন্যথা যদি( রাশিমালা_Always_True(p_রাশিমালা) ){
        /* no-op */
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_Temp(pParse, p_রাশিমালা, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ভঙ্গ;
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
শূন্য sqlite3_রাশিমালা_If_False_Dup(Parse *pParse, রাশিমালা *p_রাশিমালা, পূর্ণাঙ্ক dest,পূর্ণাঙ্ক jumpIfNull){
  sqlite3 *db = pParse->db;
  রাশিমালা *pCopy = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
  যদি( db->mallocFailed==0 ){
    sqlite3_রাশিমালা_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_রাশিমালা_Delete(db, pCopy);
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক রাশিমালা_Compare_Variable(
  ধ্রুব Parse *pParse,
  ধ্রুব রাশিমালা *pVar,
  ধ্রুব রাশিমালা *p_রাশিমালা
){
  পূর্ণাঙ্ক res = 2;
  পূর্ণাঙ্ক iVar;
  sqlite3_মান *pL, *pR = 0;
 
  যদি( p_রাশিমালা->op==TK_VARIABLE && pVar->iColumn==p_রাশিমালা->iColumn ){
    প্রত্যাগম 0;
  }
  যদি( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) প্রত্যাগম 2;
  sqlite3_মান_From_রাশিমালা(pParse->db, p_রাশিমালা, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  যদি( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_মান(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    যদি( pL ){
      যদি( sqlite3_মান_type(pL)==SQLITE_TEXT ){
        sqlite3_মান_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_মান_Free(pR);
    sqlite3_মান_Free(pL);
  }
  প্রত্যাগম res;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Compare(
  ধ্রুব Parse *pParse,
  ধ্রুব রাশিমালা *pA,
  ধ্রুব রাশিমালা *pB,
  পূর্ণাঙ্ক iTab
){
  u32 combinedFlags;
  যদি( pA==0 || pB==0 ){
    প্রত্যাগম pB==pA ? 0 : 2;
  }
  যদি( pParse && pA->op==TK_VARIABLE ){
    প্রত্যাগম রাশিমালা_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  যদি( combinedFlags & EP_Int_মান ){
    যদি( (pA->flags&pB->flags&EP_Int_মান)!=0 && pA->u.i_মান==pB->u.i_মান ){
      প্রত্যাগম 0;
    }
    প্রত্যাগম 2;
  }
  যদি( pA->op!=pB->op || pA->op==TK_RAISE ){
    যদি( pA->op==TK_COLLATE && sqlite3_রাশিমালা_Compare(pParse, pA->p_বাম,pB,iTab)<2 ){
      প্রত্যাগম 1;
    }
    যদি( pB->op==TK_COLLATE && sqlite3_রাশিমালা_Compare(pParse, pA,pB->p_বাম,iTab)<2 ){
      প্রত্যাগম 1;
    }
    যদি( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }অন্যথা{
      প্রত্যাগম 2;
    }
  }
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(pA, EP_Int_মান) );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(pB, EP_Int_মান) );
  যদি( pA->u.zToken ){
    যদি( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      যদি( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) প্রত্যাগম 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      প্রতিজ্ঞা( pA->op==pB->op );
      যদি( রাশিমালা_Has_Property(pA,EP_WinFunc)!=রাশিমালা_Has_Property(pB,EP_WinFunc) ){
        প্রত্যাগম 2;
      }
      যদি( রাশিমালা_Has_Property(pA,EP_WinFunc) ){
        যদি( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          প্রত্যাগম 2;
        }
      }
#endif
    }অন্যথা যদি( pA->op==TK_NULL ){
      প্রত্যাগম 0;
    }অন্যথা যদি( pA->op==TK_COLLATE ){
      যদি( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) প্রত্যাগম 2;
    }অন্যথা
    যদি( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      প্রত্যাগম 2;
    }
  }
  যদি( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) প্রত্যাগম 2;
  যদি( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    যদি( combinedFlags & EP_xIsSelect ) প্রত্যাগম 2;
    যদি( (combinedFlags & EP_FixedCol)==0
     && sqlite3_রাশিমালা_Compare(pParse, pA->p_বাম, pB->p_বাম, iTab) ) প্রত্যাগম 2;
    যদি( sqlite3_রাশিমালা_Compare(pParse, pA->p_ডান, pB->p_ডান, iTab) ) প্রত্যাগম 2;
    যদি( sqlite3_রাশিমালা_List_Compare(pA->x.pList, pB->x.pList, iTab) ) প্রত্যাগম 2;
    যদি( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      যদি( pA->iColumn!=pB->iColumn ) প্রত্যাগম 2;
      যদি( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) প্রত্যাগম 2;
      যদি( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        প্রত্যাগম 2;
      }
    }
  }
  প্রত্যাগম 0;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_List_Compare(ধ্রুব রাশিমালা_List *pA, ধ্রুব রাশিমালা_List *pB, পূর্ণাঙ্ক iTab){
  পূর্ণাঙ্ক i;
  যদি( pA==0 && pB==0 ) প্রত্যাগম 0;
  যদি( pA==0 || pB==0 ) প্রত্যাগম 1;
  যদি( pA->n_রাশিমালা!=pB->n_রাশিমালা ) প্রত্যাগম 1;
  প্রতি(i=0; i<pA->n_রাশিমালা; i++){
    পূর্ণাঙ্ক res;
    রাশিমালা *p_রাশিমালা_A = pA->a[i].p_রাশিমালা;
    রাশিমালা *p_রাশিমালা_B = pB->a[i].p_রাশিমালা;
    যদি( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) প্রত্যাগম 1;
    যদি( (res = sqlite3_রাশিমালা_Compare(0, p_রাশিমালা_A, p_রাশিমালা_B, iTab)) ) প্রত্যাগম res;
  }
  প্রত্যাগম 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Compare_Skip(রাশিমালা *pA,রাশিমালা *pB, পূর্ণাঙ্ক iTab){
  প্রত্যাগম sqlite3_রাশিমালা_Compare(0,
             sqlite3_রাশিমালা_Skip_Collate(pA),
             sqlite3_রাশিমালা_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Implies_Not_Null(
  ধ্রুব Parse *pParse,/* Parsing context */
  ধ্রুব রাশিমালা *p,      /* The expression to be checked */
  ধ্রুব রাশিমালা *pNN,    /* The expression that is NOT NULL */
  পূর্ণাঙ্ক iTab,           /* Table being evaluated */
  পূর্ণাঙ্ক seenNot         /* Return true only if p can be any non-NULL value */
){
  প্রতিজ্ঞা( p );
  প্রতিজ্ঞা( pNN );
  যদি( sqlite3_রাশিমালা_Compare(pParse, p, pNN, iTab)==0 ){
    প্রত্যাগম pNN->op!=TK_NULL;
  }
  বিকল্প( p->op ){
    পক্ষ TK_IN: {
      যদি( seenNot && রাশিমালা_Has_Property(p, EP_xIsSelect) ) প্রত্যাগম 0;
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_রাশিমালা>0) );
      প্রত্যাগম রাশিমালা_Implies_Not_Null(pParse, p->p_বাম, pNN, iTab, 1);
    }
    পক্ষ TK_BETWEEN: {
      রাশিমালা_List *pList;
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p) );
      pList = p->x.pList;
      প্রতিজ্ঞা( pList!=0 );
      প্রতিজ্ঞা( pList->n_রাশিমালা==2 );
      যদি( seenNot ) প্রত্যাগম 0;
      যদি( রাশিমালা_Implies_Not_Null(pParse, pList->a[0].p_রাশিমালা, pNN, iTab, 1)
       || রাশিমালা_Implies_Not_Null(pParse, pList->a[1].p_রাশিমালা, pNN, iTab, 1)
      ){
        প্রত্যাগম 1;
      }
      প্রত্যাগম রাশিমালা_Implies_Not_Null(pParse, p->p_বাম, pNN, iTab, 1);
    }
    পক্ষ TK_EQ:
    পক্ষ TK_NE:
    পক্ষ TK_LT:
    পক্ষ TK_LE:
    পক্ষ TK_GT:
    পক্ষ TK_GE:
    পক্ষ TK_PLUS:
    পক্ষ TK_MINUS:
    পক্ষ TK_BITOR:
    পক্ষ TK_LSHIFT:
    পক্ষ TK_RSHIFT:
    পক্ষ TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    পক্ষ TK_STAR:
    পক্ষ TK_REM:
    পক্ষ TK_BITAND:
    পক্ষ TK_SLASH: {
      যদি( রাশিমালা_Implies_Not_Null(pParse, p->p_ডান, pNN, iTab, seenNot) ) প্রত্যাগম 1;
      /* no break */ deliberate_fall_through
    }
    পক্ষ TK_SPAN:
    পক্ষ TK_COLLATE:
    পক্ষ TK_UPLUS:
    পক্ষ TK_UMINUS: {
      প্রত্যাগম রাশিমালা_Implies_Not_Null(pParse, p->p_বাম, pNN, iTab, seenNot);
    }
    পক্ষ TK_TRUTH: {
      যদি( seenNot ) প্রত্যাগম 0;
      যদি( p->op2!=TK_IS ) প্রত্যাগম 0;
      প্রত্যাগম রাশিমালা_Implies_Not_Null(pParse, p->p_বাম, pNN, iTab, 1);
    }
    পক্ষ TK_BITNOT:
    পক্ষ TK_NOT: {
      প্রত্যাগম রাশিমালা_Implies_Not_Null(pParse, p->p_বাম, pNN, iTab, 1);
    }
  }
  প্রত্যাগম 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
স্থির পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Not_True(রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক v;
  যদি( p_রাশিমালা->op==TK_NULL ) প্রত্যাগম 1;
  যদি( p_রাশিমালা->op==TK_TRUEFALSE && sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা)==0 ) প্রত্যাগম 1;
  v = 1;
  যদি( sqlite3_রাশিমালা_Is_Integer(p_রাশিমালা, &v, 0, 0) && v==0 ) প্রত্যাগম 1;
  প্রত্যাগম 0;
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
স্থির পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_IIF(sqlite3 *db, ধ্রুব রাশিমালা *p_রাশিমালা){
  রাশিমালা_List *pList;
  যদি( p_রাশিমালা->op==TK_FUNCTION ){
    ধ্রুব বর্ণ *z = p_রাশিমালা->u.zToken;
    FuncDef *pDef;
    যদি( (z[0]!='i' && z[0]!='I') ) প্রত্যাগম 0;
    যদি( p_রাশিমালা->x.pList==0 ) প্রত্যাগম 0;
    pDef = sqlite3FindFunction(db, z, p_রাশিমালা->x.pList->n_রাশিমালা, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    যদি( pDef==0 ) প্রত্যাগম 0;
#অন্যথা
    যদি( NEVER(pDef==0) ) প্রত্যাগম 0;
#endif
    যদি( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) প্রত্যাগম 0;
    যদি( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) প্রত্যাগম 0;
  }অন্যথা যদি( p_রাশিমালা->op==TK_CASE ){
    যদি( p_রাশিমালা->p_বাম!=0 ) প্রত্যাগম 0;
  }অন্যথা{
    প্রত্যাগম 0;
  }
  pList = p_রাশিমালা->x.pList;
  প্রতিজ্ঞা( pList!=0 );
  যদি( pList->n_রাশিমালা==2 ) প্রত্যাগম 1;
  যদি( pList->n_রাশিমালা==3 && sqlite3_রাশিমালা_Is_Not_True(pList->a[2].p_রাশিমালা) ) প্রত্যাগম 1;
  প্রত্যাগম 0;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Implies_রাশিমালা(
  ধ্রুব Parse *pParse,
  ধ্রুব রাশিমালা *pE1,
  ধ্রুব রাশিমালা *pE2,
  পূর্ণাঙ্ক iTab
){
  যদি( sqlite3_রাশিমালা_Compare(pParse, pE1, pE2, iTab)==0 ){
    প্রত্যাগম 1;
  }
  যদি( pE2->op==TK_OR
   && (sqlite3_রাশিমালা_Implies_রাশিমালা(pParse, pE1, pE2->p_বাম, iTab)
             || sqlite3_রাশিমালা_Implies_রাশিমালা(pParse, pE1, pE2->p_ডান, iTab) )
  ){
    প্রত্যাগম 1;
  }
  যদি( pE2->op==TK_NOTNULL
   && রাশিমালা_Implies_Not_Null(pParse, pE1, pE2->p_বাম, iTab, 0)
  ){
    প্রত্যাগম 1;
  }
  যদি( sqlite3_রাশিমালা_Is_IIF(pParse->db, pE1) ){
    প্রত্যাগম sqlite3_রাশিমালা_Implies_রাশিমালা(pParse,pE1->x.pList->a[0].p_রাশিমালা,pE2,iTab);
  }
  প্রত্যাগম 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
স্থির শূন্য bothImplyNotNullRow(Walker *pWalker, রাশিমালা *pE1, রাশিমালা *pE2){
  যদি( pWalker->eCode==0 ){
    sqlite3_Walk_রাশিমালা(pWalker, pE1);
    যদি( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_রাশিমালা(pWalker, pE2);
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
স্থির পূর্ণাঙ্ক impliesNotNullRow(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  testcase( p_রাশিমালা->op==TK_AGG_COLUMN );
  testcase( p_রাশিমালা->op==TK_AGG_FUNCTION );
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ) প্রত্যাগম WRC_Prune;
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    প্রত্যাগম WRC_Prune;
  }
  বিকল্প( p_রাশিমালা->op ){
    পক্ষ TK_ISNOT:
    পক্ষ TK_ISNULL:
    পক্ষ TK_NOTNULL:
    পক্ষ TK_IS:
    পক্ষ TK_VECTOR:
    পক্ষ TK_FUNCTION:
    পক্ষ TK_TRUTH:
    পক্ষ TK_CASE:
      testcase( p_রাশিমালা->op==TK_ISNOT );
      testcase( p_রাশিমালা->op==TK_ISNULL );
      testcase( p_রাশিমালা->op==TK_NOTNULL );
      testcase( p_রাশিমালা->op==TK_IS );
      testcase( p_রাশিমালা->op==TK_VECTOR );
      testcase( p_রাশিমালা->op==TK_FUNCTION );
      testcase( p_রাশিমালা->op==TK_TRUTH );
      testcase( p_রাশিমালা->op==TK_CASE );
      প্রত্যাগম WRC_Prune;

    পক্ষ TK_COLUMN:
      যদি( pWalker->u.iCur==p_রাশিমালা->iTable ){
        pWalker->eCode = 1;
        প্রত্যাগম WRC_Abort;
      }
      প্রত্যাগম WRC_Prune;

    পক্ষ TK_OR:
    পক্ষ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_রাশিমালা->op==TK_OR );
      testcase( p_রাশিমালা->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_রাশিমালা->p_বাম, p_রাশিমালা->p_ডান);
      প্রত্যাগম WRC_Prune;
       
    পক্ষ TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      যদি( রাশিমালা_Use_XList(p_রাশিমালা) && ALWAYS(p_রাশিমালা->x.pList->n_রাশিমালা>0) ){
        sqlite3_Walk_রাশিমালা(pWalker, p_রাশিমালা->p_বাম);
      }
      প্রত্যাগম WRC_Prune;

    পক্ষ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->x.pList->n_রাশিমালা==2 );
      sqlite3_Walk_রাশিমালা(pWalker, p_রাশিমালা->p_বাম);
      bothImplyNotNullRow(pWalker, p_রাশিমালা->x.pList->a[0].p_রাশিমালা,
                                   p_রাশিমালা->x.pList->a[1].p_রাশিমালা);
      প্রত্যাগম WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    পক্ষ TK_EQ:
    পক্ষ TK_NE:
    পক্ষ TK_LT:
    পক্ষ TK_LE:
    পক্ষ TK_GT:
    পক্ষ TK_GE: {
      রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
      রাশিমালা *p_ডান = p_রাশিমালা->p_ডান;
      testcase( p_রাশিমালা->op==TK_EQ );
      testcase( p_রাশিমালা->op==TK_NE );
      testcase( p_রাশিমালা->op==TK_LT );
      testcase( p_রাশিমালা->op==TK_LE );
      testcase( p_রাশিমালা->op==TK_GT );
      testcase( p_রাশিমালা->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      প্রতিজ্ঞা( p_বাম->op!=TK_COLUMN || রাশিমালা_Use_YTab(p_বাম) );
      প্রতিজ্ঞা( p_ডান->op!=TK_COLUMN || রাশিমালা_Use_YTab(p_ডান) );
      যদি( (p_বাম->op==TK_COLUMN
           && ALWAYS(p_বাম->y.pTab!=0)
           && IsVirtual(p_বাম->y.pTab))
       || (p_ডান->op==TK_COLUMN
           && ALWAYS(p_ডান->y.pTab!=0)
           && IsVirtual(p_ডান->y.pTab))
      ){
        প্রত্যাগম WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    স্বভাব:
      প্রত্যাগম WRC_Continue;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Implies_Non_Null_Row(রাশিমালা *p, পূর্ণাঙ্ক iTab, পূর্ণাঙ্ক isRJ){
  Walker w;
  p = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p);
  যদি( p==0 ) প্রত্যাগম 0;
  যদি( p->op==TK_NOTNULL ){
    p = p->p_বাম;
  }অন্যথা{
    যাবত্( p->op==TK_AND ){
      যদি( sqlite3_রাশিমালা_Implies_Non_Null_Row(p->p_বাম, iTab, isRJ) ) প্রত্যাগম 1;
      p = p->p_ডান;
    }
  }
  w.x_রাশিমালা_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_রাশিমালা(&w, p);
  প্রত্যাগম w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
সংরচনা IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  পূর্ণাঙ্ক iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
স্থির পূর্ণাঙ্ক রাশিমালা_Idx_Cover(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_COLUMN
   && p_রাশিমালা->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_রাশিমালা->iColumn)<0
  ){
    pWalker->eCode = 1;
    প্রত্যাগম WRC_Abort;
  }
  প্রত্যাগম WRC_Continue;
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
পূর্ণাঙ্ক sqlite3_রাশিমালা_Covered_By_Index(
  রাশিমালা *p_রাশিমালা,        /* The index to be tested */
  পূর্ণাঙ্ক iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  সংরচনা IdxCover xcov;
  memset(&w, 0, পরিমাণ(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_রাশিমালা_Callback = রাশিমালা_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_রাশিমালা(&w, p_রাশিমালা);
  প্রত্যাগম !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
সংরচনা RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  পূর্ণাঙ্ক *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
স্থির পূর্ণাঙ্ক selectRefEnter(Walker *pWalker, Select *pSelect){
  সংরচনা RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  পূর্ণাঙ্ক *piNew;
  যদি( pSrc->nSrc==0 ) প্রত্যাগম WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*পরিমাণ(পূর্ণাঙ্ক));
  যদি( piNew==0 ){
    p->nExclude = 0;
    প্রত্যাগম WRC_Abort;
  }অন্যথা{
    p->aiExclude = piNew;
  }
  প্রতি(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  প্রত্যাগম WRC_Continue;
}
স্থির শূন্য selectRefLeave(Walker *pWalker, Select *pSelect){
  সংরচনা RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  যদি( p->nExclude ){
    প্রতিজ্ঞা( p->nExclude>=pSrc->nSrc );
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
স্থির পূর্ণাঙ্ক রাশিমালা_Ref_To_Src_List(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_COLUMN
   || p_রাশিমালা->op==TK_AGG_COLUMN
  ){
    পূর্ণাঙ্ক i;
    সংরচনা RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    পূর্ণাঙ্ক nSrc = pSrc ? pSrc->nSrc : 0;
    প্রতি(i=0; i<nSrc; i++){
      যদি( p_রাশিমালা->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        প্রত্যাগম WRC_Continue;
      }
    }
    প্রতি(i=0; i<p->nExclude && p->aiExclude[i]!=p_রাশিমালা->iTable; i++){}
    যদি( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  প্রত্যাগম WRC_Continue;
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
পূর্ণাঙ্ক sqlite3ReferencesSrcList(Parse *pParse, রাশিমালা *p_রাশিমালা, SrcList *pSrcList){
  Walker w;
  সংরচনা RefSrcList x;
  প্রতিজ্ঞা( pParse->db!=0 );
  memset(&w, 0, পরিমাণ(w));
  memset(&x, 0, পরিমাণ(x));
  w.x_রাশিমালা_Callback = রাশিমালা_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_AGG_FUNCTION );
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  sqlite3_Walk_রাশিমালা_List(&w, p_রাশিমালা->x.pList);
  যদি( p_রাশিমালা->p_বাম ){
    প্রতিজ্ঞা( p_রাশিমালা->p_বাম->op==TK_ORDER );
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা->p_বাম) );
    প্রতিজ্ঞা( p_রাশিমালা->p_বাম->x.pList!=0 );
    sqlite3_Walk_রাশিমালা_List(&w, p_রাশিমালা->p_বাম->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
    sqlite3_Walk_রাশিমালা(&w, p_রাশিমালা->y.pWin->pFilter);
  }
#endif
  যদি( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  যদি( w.eCode & 0x01 ){
    প্রত্যাগম 1;
  }অন্যথা যদি( w.eCode ){
    প্রত্যাগম 0;
  }অন্যথা{
    প্রত্যাগম -1;
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
স্থির পূর্ণাঙ্ক agginfo_Persist_রাশিমালা_Cb(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( ALWAYS(!রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly|EP_Reduced))
   && p_রাশিমালা->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_রাশিমালা->pAggInfo;
    পূর্ণাঙ্ক iAgg = p_রাশিমালা->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    প্রতিজ্ঞা( iAgg>=0 );
    যদি( p_রাশিমালা->op!=TK_AGG_FUNCTION ){
      যদি( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_রাশিমালা
      ){
        p_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
        যদি( p_রাশিমালা && !sqlite3_রাশিমালা_Deferred_Delete(pParse, p_রাশিমালা) ){
          pAggInfo->aCol[iAgg].pCExpr = p_রাশিমালা;
        }
      }
    }অন্যথা{
      প্রতিজ্ঞা( p_রাশিমালা->op==TK_AGG_FUNCTION );
      যদি( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_রাশিমালা
      ){
        p_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
        যদি( p_রাশিমালা && !sqlite3_রাশিমালা_Deferred_Delete(pParse, p_রাশিমালা) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_রাশিমালা;
        }
      }
    }
  }
  প্রত্যাগম WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
শূন্য sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, পরিমাণ(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_রাশিমালা_Callback = agginfo_Persist_রাশিমালা_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
স্থির পূর্ণাঙ্ক addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  পূর্ণাঙ্ক i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       পরিমাণ(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  প্রত্যাগম i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
স্থির পূর্ণাঙ্ক addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  পূর্ণাঙ্ক i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       পরিমাণ(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  প্রত্যাগম i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
স্থির শূন্য findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  রাশিমালা *p_রাশিমালা          /* Expr describing the column to find or insert */
){
  সংরচনা AggInfo_col *pCol;
  পূর্ণাঙ্ক k;
  পূর্ণাঙ্ক mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  প্রতিজ্ঞা( mxTerm <= SMXV(i16) );
  প্রতিজ্ঞা( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  প্রতি(k=0; k<pAggInfo->nColumn; k++, pCol++){
    যদি( pCol->pCExpr==p_রাশিমালা ) প্রত্যাগম;
    যদি( pCol->iTable==p_রাশিমালা->iTable
     && pCol->iColumn==p_রাশিমালা->iColumn
     && p_রাশিমালা->op!=TK_IF_NULL_ROW
    ){
      গচ্ছ fix_up_রাশিমালা;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  যদি( k<0 ){
    /* OOM on resize */
    প্রতিজ্ঞা( pParse->db->mallocFailed );
    প্রত্যাগম;
  }
  যদি( k>mxTerm ){
    sqlite3_Error_বার্তা(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
  pCol->pTab = p_রাশিমালা->y.pTab;
  pCol->iTable = p_রাশিমালা->iTable;
  pCol->iColumn = p_রাশিমালা->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_রাশিমালা;
  যদি( pAggInfo->pGroupBy && p_রাশিমালা->op!=TK_IF_NULL_ROW ){
    পূর্ণাঙ্ক j, n;
    রাশিমালা_List *pGB = pAggInfo->pGroupBy;
    সংরচনা রাশিমালা_List_item *pTerm = pGB->a;
    n = pGB->n_রাশিমালা;
    প্রতি(j=0; j<n; j++, pTerm++){
      রাশিমালা *pE = pTerm->p_রাশিমালা;
      যদি( pE->op==TK_COLUMN
       && pE->iTable==p_রাশিমালা->iTable
       && pE->iColumn==p_রাশিমালা->iColumn
      ){
        pCol->iSorterColumn = j;
        ভঙ্গ;
      }
    }
  }
  যদি( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_রাশিমালা:
  রাশিমালা_Set_VVAProperty(p_রাশিমালা, EP_NoReduce);
  প্রতিজ্ঞা( p_রাশিমালা->pAggInfo==0 || p_রাশিমালা->pAggInfo==pAggInfo );
  p_রাশিমালা->pAggInfo = pAggInfo;
  যদি( p_রাশিমালা->op==TK_COLUMN ){
    p_রাশিমালা->op = TK_AGG_COLUMN;
  }
  প্রতিজ্ঞা( k <= SMXV(p_রাশিমালা->iAgg) );
  p_রাশিমালা->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
স্থির পূর্ণাঙ্ক analyzeAggregate(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  পূর্ণাঙ্ক i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  প্রতিজ্ঞা( pNC->ncFlags & NC_UAggInfo );
  প্রতিজ্ঞা( pAggInfo->iFirstReg==0 );
  বিকল্প( p_রাশিমালা->op ){
    স্বভাব: {
      Indexed_রাশিমালা *pIEpr;
      রাশিমালা tmp;
      প্রতিজ্ঞা( pParse->iSelfTab==0 );
      যদি( (pNC->ncFlags & NC_InAggFunc)==0 ) ভঙ্গ;
      যদি( pParse->pIdxEpr==0 ) ভঙ্গ;
      প্রতি(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        পূর্ণাঙ্ক iDataCur = pIEpr->iDataCur;
        যদি( iDataCur<0 ) অনুবর্ত;
        যদি( sqlite3_রাশিমালা_Compare(0, p_রাশিমালা, pIEpr->p_রাশিমালা, iDataCur)==0 ) ভঙ্গ;
      }
      যদি( pIEpr==0 ) ভঙ্গ;
      যদি( NEVER(!রাশিমালা_Use_YTab(p_রাশিমালা)) ) ভঙ্গ;
      প্রতি(i=0; i<pSrcList->nSrc; i++){
         যদি( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           ভঙ্গ;
         }
      }
      যদি( i>=pSrcList->nSrc ) ভঙ্গ;
      যদি( NEVER(p_রাশিমালা->pAggInfo!=0) ) ভঙ্গ; /* Resolved by outer context */
      যদি( pParse->nErr ){ প্রত্যাগম WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, পরিমাণ(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      যদি( pParse->nErr ){ প্রত্যাগম WRC_Abort; }
      প্রতিজ্ঞা( pAggInfo->aCol!=0 );
      প্রতিজ্ঞা( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_রাশিমালা;
      p_রাশিমালা->pAggInfo = pAggInfo;
      p_রাশিমালা->iAgg = tmp.iAgg;
      প্রত্যাগম WRC_Prune;
    }
    পক্ষ TK_IF_NULL_ROW:
    পক্ষ TK_AGG_COLUMN:
    পক্ষ TK_COLUMN: {
      testcase( p_রাশিমালা->op==TK_AGG_COLUMN );
      testcase( p_রাশিমালা->op==TK_COLUMN );
      testcase( p_রাশিমালা->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      যদি( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        প্রতি(i=0; i<pSrcList->nSrc; i++, pItem++){
          প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly|EP_Reduced) );
          যদি( p_রাশিমালা->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_রাশিমালা);
            ভঙ্গ;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      প্রত্যাগম WRC_Continue;
    }
    পক্ষ TK_AGG_FUNCTION: {
      যদি( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_রাশিমালা->op2
       && p_রাশিমালা->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        সংরচনা AggInfo_func *pItem = pAggInfo->aFunc;
        পূর্ণাঙ্ক mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        প্রতিজ্ঞা( mxTerm <= SMXV(i16) );
        প্রতি(i=0; i<pAggInfo->nFunc; i++, pItem++){
          যদি( NEVER(pItem->pFExpr==p_রাশিমালা) ) ভঙ্গ;
          যদি( sqlite3_রাশিমালা_Compare(0, pItem->pFExpr, p_রাশিমালা, -1)==0 ){
            ভঙ্গ;
          }
        }
        যদি( i>mxTerm ){
          sqlite3_Error_বার্তা(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          প্রতিজ্ঞা( i<pAggInfo->nFunc );
        }অন্যথা যদি( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          যদি( i>=0 ){
            পূর্ণাঙ্ক nArg;
            প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_রাশিমালা;
            প্রতিজ্ঞা( রাশিমালা_Use_UToken(p_রাশিমালা) );
            nArg = p_রাশিমালা->x.pList ? p_রাশিমালা->x.pList->n_রাশিমালা : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_রাশিমালা->u.zToken, nArg, enc, 0);
            প্রতিজ্ঞা( pItem->bOBUnique==0 );
            যদি( p_রাশিমালা->p_বাম
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              রাশিমালা_List *pOBList;
              প্রতিজ্ঞা( nArg>0 );
              প্রতিজ্ঞা( p_রাশিমালা->p_বাম->op==TK_ORDER );
              প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা->p_বাম) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_রাশিমালা->p_বাম->x.pList;
              প্রতিজ্ঞা( pOBList->n_রাশিমালা>0 );
              প্রতিজ্ঞা( pItem->bOBUnique==0 );
              যদি( pOBList->n_রাশিমালা==1
               && nArg==1
               && sqlite3_রাশিমালা_Compare(0,pOBList->a[0].p_রাশিমালা,
                               p_রাশিমালা->x.pList->a[0].p_রাশিমালা,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = রাশিমালা_Has_Property(p_রাশিমালা, EP_Distinct);
              }অন্যথা{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }অন্যথা{
              pItem->iOBTab = -1;
            }
            যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }অন্যথা{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly|EP_Reduced) );
        রাশিমালা_Set_VVAProperty(p_রাশিমালা, EP_NoReduce);
        প্রতিজ্ঞা( i <= SMXV(p_রাশিমালা->iAgg) );
        p_রাশিমালা->iAgg = (i16)i;
        p_রাশিমালা->pAggInfo = pAggInfo;
        প্রত্যাগম WRC_Prune;
      }অন্যথা{
        প্রত্যাগম WRC_Continue;
      }
    }
  }
  প্রত্যাগম WRC_Continue;
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
শূন্য sqlite3_রাশিমালা_Analyze_Aggregates(NameContext *pNC, রাশিমালা *p_রাশিমালা){
  Walker w;
  w.x_রাশিমালা_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  প্রতিজ্ঞা( pNC->pSrcList!=0 );
  sqlite3_Walk_রাশিমালা(&w, p_রাশিমালা);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
শূন্য sqlite3_রাশিমালা_Analyze_Agg_List(NameContext *pNC, রাশিমালা_List *pList){
  সংরচনা রাশিমালা_List_item *pItem;
  পূর্ণাঙ্ক i;
  যদি( pList ){
    প্রতি(pItem=pList->a, i=0; i<pList->n_রাশিমালা; i++, pItem++){
      sqlite3_রাশিমালা_Analyze_Aggregates(pNC, pItem->p_রাশিমালা);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
পূর্ণাঙ্ক sqlite3GetTempReg(Parse *pParse){
  যদি( pParse->nTempReg==0 ){
    প্রত্যাগম ++pParse->nMem;
  }
  প্রত্যাগম pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
শূন্য sqlite3ReleaseTempReg(Parse *pParse, পূর্ণাঙ্ক iReg){
  যদি( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    পূর্ণাঙ্ক kk;
    প্রতি(kk=0; kk<pParse->nTempReg; kk++){
      প্রতিজ্ঞা( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    যদি( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
পূর্ণাঙ্ক sqlite3GetTempRange(Parse *pParse, পূর্ণাঙ্ক nReg){
  পূর্ণাঙ্ক i, n;
  যদি( nReg==1 ) প্রত্যাগম sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  যদি( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }অন্যথা{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  প্রত্যাগম i;
}
শূন্য sqlite3ReleaseTempRange(Parse *pParse, পূর্ণাঙ্ক iReg, পূর্ণাঙ্ক nReg){
  যদি( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    প্রত্যাগম;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  যদি( nReg>pParse->nRangeReg ){
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
শূন্য sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
শূন্য sqlite3TouchRegister(Parse *pParse, পূর্ণাঙ্ক iReg){
  যদি( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#যদি defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
পূর্ণাঙ্ক sqlite3FirstAvailableRegister(Parse *pParse, পূর্ণাঙ্ক iMin){
  ধ্রুব রাশিমালা_List *pList = pParse->p_Const_রাশিমালা;
  যদি( pList ){
    পূর্ণাঙ্ক i;
    প্রতি(i=0; i<pList->n_রাশিমালা; i++){
      যদি( pList->a[i].u.i_Const_রাশিমালা_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_রাশিমালা_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  প্রত্যাগম iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
পূর্ণাঙ্ক sqlite3NoTempsInRange(Parse *pParse, পূর্ণাঙ্ক iFirst, পূর্ণাঙ্ক iLast){
  পূর্ণাঙ্ক i;
  যদি( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     প্রত্যাগম 0;
  }
  প্রতি(i=0; i<pParse->nTempReg; i++){
    যদি( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      প্রত্যাগম 0;
    }
  }
  যদি( pParse->p_Const_রাশিমালা ){
    রাশিমালা_List *pList = pParse->p_Const_রাশিমালা;
    প্রতি(i=0; i<pList->n_রাশিমালা; i++){
      পূর্ণাঙ্ক iReg = pList->a[i].u.i_Const_রাশিমালা_Reg;
      যদি( iReg==0 ) অনুবর্ত;
      যদি( iReg>=iFirst && iReg<=iLast ) প্রত্যাগম 0;
    }
  }
  প্রত্যাগম 1;
}
#endif /* SQLITE_DEBUG */
