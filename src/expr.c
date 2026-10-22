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
ساكن فراغ اظہاریہ_کوڈ_Between(Parse*,اظہاریہ*,صحيح,فراغ(*)(Parse*,اظہاریہ*,صحيح,صحيح),صحيح);
ساكن صحيح اظہاریہ_کوڈ_سمتیہ(Parse *pParse, اظہاریہ *p, صحيح *pi_منزل_Free);

/*
** Return the affinity character for a single column of a table.
*/
حرف sqlite3_جدول_کالم_Affinity(ثابت جدول *pTab, صحيح i_خانہ){
  إذا( i_خانہ<0 || NEVER(i_خانہ>=pTab->n_خانہ) ) أرجع SQLITE_AFF_INTEGER;
  أرجع pTab->a_خانہ[i_خانہ].affinity;
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
حرف sqlite3_اظہاریہ_Affinity(ثابت اظہاریہ *p_اظہاریہ){
  صحيح op;
  op = p_اظہاریہ->op;
  طالما( 1 /* exit-by-break */ ){
    إذا( op==TK_کالم || (op==TK_AGG_کالم && p_اظہاریہ->y.pTab!=0) ){
      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      أكد( p_اظہاریہ->y.pTab!=0 );
      أرجع sqlite3_جدول_کالم_Affinity(p_اظہاریہ->y.pTab, p_اظہاریہ->i_کالم);
    }
    إذا( op==TK_SELECT ){
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pSelect!=0 );
      أكد( p_اظہاریہ->x.pSelect->pEList!=0 );
      أكد( p_اظہاریہ->x.pSelect->pEList->a[0].p_اظہاریہ!=0 );
      أرجع sqlite3_اظہاریہ_Affinity(p_اظہاریہ->x.pSelect->pEList->a[0].p_اظہاریہ);
    }
#جے_بنا_تعریف SQLITE_OMIT_CAST
    إذا( op==TK_CAST ){
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      أرجع sqlite3AffinityType(p_اظہاریہ->u.zToken, 0);
    }
#جے_انت
    إذا( op==TK_SELECT_کالم ){
      أكد( p_اظہاریہ->p_بایاں!=0 && اظہاریہ_Use_XSelect(p_اظہاریہ->p_بایاں) );
      أكد( p_اظہاریہ->i_کالم < p_اظہاریہ->i_جدول );
      أكد( p_اظہاریہ->i_کالم >= 0 );
      أكد( p_اظہاریہ->i_جدول==p_اظہاریہ->p_بایاں->x.pSelect->pEList->n_اظہاریہ );
      أرجع sqlite3_اظہاریہ_Affinity(
          p_اظہاریہ->p_بایاں->x.pSelect->pEList->a[p_اظہاریہ->i_کالم].p_اظہاریہ
      );
    }
    إذا( op==TK_سمتیہ
     || (op==TK_FUNCTION && p_اظہاریہ->aff_اظہاریہ==SQLITE_AFF_DEFER)
    ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أرجع sqlite3_اظہاریہ_Affinity(p_اظہاریہ->x.pList->a[0].p_اظہاریہ);
    }
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip|EP_If_Null_صف) ){
      أكد( p_اظہاریہ->op==TK_COLLATE
           || p_اظہاریہ->op==TK_IF_NULL_صف
           || (p_اظہاریہ->op==TK_رجسٹر && p_اظہاریہ->op2==TK_IF_NULL_صف) );
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
      op = p_اظہاریہ->op;
      استمر;
    }
    إذا( op!=TK_رجسٹر ) اكسر;
    op = p_اظہاریہ->op2;
    إذا( NEVER( op==TK_رجسٹر ) ) اكسر;
  }
  أرجع p_اظہاریہ->aff_اظہاریہ;
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
صحيح sqlite3_اظہاریہ_انکڑے_Type(ثابت اظہاریہ *p_اظہاریہ){
  طالما( p_اظہاریہ ){
    بدل( p_اظہاریہ->op ){
      حالة TK_COLLATE:
      حالة TK_IF_NULL_صف:
      حالة TK_UPLUS:  {
        p_اظہاریہ = p_اظہاریہ->p_بایاں;
        اكسر;
      }
      حالة TK_NULL: {
        p_اظہاریہ = 0;
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
      حالة TK_کالم:
      حالة TK_AGG_کالم:
      حالة TK_SELECT:
      حالة TK_CAST:
      حالة TK_SELECT_کالم:
      حالة TK_سمتیہ:  {
        صحيح aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
        إذا( aff>=SQLITE_AFF_NUMERIC ) أرجع 0x05;
        إذا( aff==SQLITE_AFF_متن )    أرجع 0x06;
        أرجع 0x07;
      }
      حالة TK_CASE: {
        صحيح res = 0;
        صحيح ii;
        اظہاریہ_List *pList = p_اظہاریہ->x.pList;
        أكد( اظہاریہ_Use_XList(p_اظہاریہ) && pList!=0 );
        أكد( pList->n_اظہاریہ > 0);
        لكل(ii=1; ii<pList->n_اظہاریہ; ii+=2){
          res |= sqlite3_اظہاریہ_انکڑے_Type(pList->a[ii].p_اظہاریہ);
        }
        إذا( pList->n_اظہاریہ % 2 ){
          res |= sqlite3_اظہاریہ_انکڑے_Type(pList->a[pList->n_اظہاریہ-1].p_اظہاریہ);
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
اظہاریہ *sqlite3_اظہاریہ_جوڑو_Collate_Token(
  Parse *pParse,           /* Parsing context */
  اظہاریہ *p_اظہاریہ,             /* Add the "COLLATE" clause to this expression */
  ثابت Token *pCollName,  /* Name of collating sequence */
  صحيح dequote              /* True to dequote pCollName */
){
  إذا( pCollName->n>0 && pParse->nErr==0 ){
    اظہاریہ *pNew = sqlite3_اظہاریہ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( p_اظہاریہ!=0 );
    إذا( pNew ){
      pNew->p_بایاں = p_اظہاریہ;
      pNew->flags |= EP_Collate|EP_Skip;
#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
      pNew->n_اچائی = p_اظہاریہ->n_اچائی+1;
      sqlite3_اظہاریہ_Check_اچائی(pParse, pNew->n_اچائی);
      testcase( pParse->nErr>0 );
#جے_انت
      p_اظہاریہ = pNew;
    }
  }
  أرجع p_اظہاریہ;
}
اظہاریہ *sqlite3_اظہاریہ_جوڑو_Collate_String(
  Parse *pParse,        /* Parsing context */
  اظہاریہ *p_اظہاریہ,          /* Add the "COLLATE" clause to this expression */
  ثابت حرف *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (حرف*)zC);
  أرجع sqlite3_اظہاریہ_جوڑو_Collate_Token(pParse, p_اظہاریہ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
اظہاریہ *sqlite3_اظہاریہ_Skip_Collate(اظہاریہ *p_اظہاریہ){
  طالما( p_اظہاریہ && اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip) ){
    أكد( p_اظہاریہ->op==TK_COLLATE );
    p_اظہاریہ = p_اظہاریہ->p_بایاں;
  }  
  أرجع p_اظہاریہ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
اظہاریہ *sqlite3_اظہاریہ_Skip_Collate_And_Likely(اظہاریہ *p_اظہاریہ){
  طالما( p_اظہاریہ && اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip|EP_Unlikely) ){
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Unlikely) ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pList->n_اظہاریہ>0 );
      أكد( p_اظہاریہ->op==TK_FUNCTION );
      p_اظہاریہ = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
    }وإلا إذا( p_اظہاریہ->op==TK_COLLATE ){
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
    }وإلا{
      اكسر;
    }
  }  
  أرجع p_اظہاریہ;
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
CollSeq *sqlite3_اظہاریہ_Coll_Seq(Parse *pParse, ثابت اظہاریہ *p_اظہاریہ){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ثابت اظہاریہ *p = p_اظہاریہ;
  طالما( p ){
    صحيح op = p->op;
    إذا( op==TK_رجسٹر ) op = p->op2;
    إذا( (op==TK_AGG_کالم && p->y.pTab!=0)
     || op==TK_کالم || op==TK_TRIGGER
    ){
      صحيح j;
      أكد( اظہاریہ_Use_YTab(p) );
      أكد( p->y.pTab!=0 );
      إذا( (j = p->i_کالم)>=0 ){
        ثابت حرف *zColl = sqlite3_کالم_Coll(&p->y.pTab->a_خانہ[j]);
        pColl = sqlite3_لبھو_Coll_Seq(db, ENC(db), zColl, 0);
      }
      اكسر;
    }
    إذا( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_بایاں;
      استمر;
    }
    إذا( op==TK_سمتیہ
     || (op==TK_FUNCTION && p->aff_اظہاریہ==SQLITE_AFF_DEFER)
    ){
      أكد( اظہاریہ_Use_XList(p) );
      p = p->x.pList->a[0].p_اظہاریہ;
      استمر;
    }
    إذا( op==TK_COLLATE ){
      أكد( !اظہاریہ_Has_Property(p, EP_Int_قدر) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      اكسر;
    }
    إذا( p->flags & EP_Collate ){
      إذا( p->p_بایاں && (p->p_بایاں->flags & EP_Collate)!=0 ){
        p = p->p_بایاں;
      }وإلا{
        اظہاریہ *p_اگلا  = p->p_دایاں;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !اظہاریہ_Use_XList(p) || p->x.pList==0 || p->p_دایاں==0 );
        إذا( اظہاریہ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحيح i;
          لكل(i=0; i<p->x.pList->n_اظہاریہ; i++){
            إذا( اظہاریہ_Has_Property(p->x.pList->a[i].p_اظہاریہ, EP_Collate) ){
              p_اگلا = p->x.pList->a[i].p_اظہاریہ;
              اكسر;
            }
          }
        }
        p = p_اگلا;
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
CollSeq *sqlite3_اظہاریہ_NNColl_Seq(Parse *pParse, ثابت اظہاریہ *p_اظہاریہ){
  CollSeq *p = sqlite3_اظہاریہ_Coll_Seq(pParse, p_اظہاریہ);
  إذا( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  أرجع p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحيح sqlite3_اظہاریہ_Coll_Seq_میل(Parse *pParse, ثابت اظہاریہ *pE1, ثابت اظہاریہ *pE2){
  CollSeq *pColl1 = sqlite3_اظہاریہ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_اظہاریہ_NNColl_Seq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  أرجع pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
حرف sqlite3CompareAffinity(ثابت اظہاریہ *p_اظہاریہ, حرف aff2){
  حرف aff1 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
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
ساكن حرف comparisonAffinity(ثابت اظہاریہ *p_اظہاریہ){
  حرف aff;
  أكد( p_اظہاریہ->op==TK_EQ || p_اظہاریہ->op==TK_وچ || p_اظہاریہ->op==TK_LT ||
          p_اظہاریہ->op==TK_GT || p_اظہاریہ->op==TK_GE || p_اظہاریہ->op==TK_LE ||
          p_اظہاریہ->op==TK_NE || p_اظہاریہ->op==TK_IS || p_اظہاریہ->op==TK_ISNOT );
  أكد( p_اظہاریہ->p_بایاں );
  أكد( !اظہاریہ_Use_XSelect(p_اظہاریہ) );
  aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_بایاں);
  إذا( p_اظہاریہ->p_دایاں ){
    aff = sqlite3CompareAffinity(p_اظہاریہ->p_دایاں, aff);
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
صحيح sqlite3_اشاریہ_Affinity_Ok(ثابت اظہاریہ *p_اظہاریہ, حرف idx_affinity){
  حرف aff = comparisonAffinity(p_اظہاریہ);
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
ساكن u8 ثنائی_Compare_P5(
  ثابت اظہاریہ *pExpr1,   /* Left operand */
  ثابت اظہاریہ *pExpr2,   /* Right operand */
  صحيح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (حرف)sqlite3_اظہاریہ_Affinity(pExpr2);
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
CollSeq *sqlite3_ثنائی_Compare_Coll_Seq(
  Parse *pParse,
  ثابت اظہاریہ *p_بایاں,
  ثابت اظہاریہ *p_دایاں
){
  CollSeq *pColl;
  أكد( p_بایاں );
  إذا( p_بایاں->flags & EP_Collate ){
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_بایاں);
  }وإلا إذا( p_دایاں && (p_دایاں->flags & EP_Collate)!=0 ){
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_دایاں);
  }وإلا{
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_بایاں);
    إذا( !pColl ){
      pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_دایاں);
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
CollSeq *sqlite3_اظہاریہ_Compare_Coll_Seq(Parse *pParse, ثابت اظہاریہ *p){
  إذا( اظہاریہ_Has_Property(p, EP_Commuted) ){
    أرجع sqlite3_ثنائی_Compare_Coll_Seq(pParse, p->p_دایاں, p->p_بایاں);
  }وإلا{
    أرجع sqlite3_ثنائی_Compare_Coll_Seq(pParse, p->p_بایاں, p->p_دایاں);
  }
}

/*
** Generate code for a comparison operator.
*/
ساكن صحيح کوڈ_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  اظہاریہ *p_بایاں,      /* The left operand */
  اظہاریہ *p_دایاں,     /* The right operand */
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
    p4 = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_دایاں, p_بایاں);
  }وإلا{
    p4 = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_بایاں, p_دایاں);
  }
  p5 = ثنائی_Compare_P5(p_بایاں, p_دایاں, jumpIfNull);
  addr = sqlite3_Vdbe_جوڑو_Op4(pParse->pVdbe, opcode, in2, dest, in1,
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
صحيح sqlite3_اظہاریہ_Is_سمتیہ(ثابت اظہاریہ *p_اظہاریہ){
  أرجع sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحيح sqlite3_اظہاریہ_سمتیہ_ناپ(ثابت اظہاریہ *p_اظہاریہ){
  u8 op = p_اظہاریہ->op;
  إذا( op==TK_رجسٹر ) op = p_اظہاریہ->op2;
  إذا( op==TK_سمتیہ ){
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    أرجع p_اظہاریہ->x.pList->n_اظہاریہ;
  }وإلا إذا( op==TK_SELECT ){
    أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
    أرجع p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ;
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
اظہاریہ *sqlite3_سمتیہ_میدان_Subexpr(اظہاریہ *p_سمتیہ, صحيح i){
  أكد( i<sqlite3_اظہاریہ_سمتیہ_ناپ(p_سمتیہ) || p_سمتیہ->op==TK_ERROR );
  إذا( sqlite3_اظہاریہ_Is_سمتیہ(p_سمتیہ) ){
    أكد( p_سمتیہ->op2==0 || p_سمتیہ->op==TK_رجسٹر );
    إذا( p_سمتیہ->op==TK_SELECT || p_سمتیہ->op2==TK_SELECT ){
      أكد( اظہاریہ_Use_XSelect(p_سمتیہ) );
      أرجع p_سمتیہ->x.pSelect->pEList->a[i].p_اظہاریہ;
    }وإلا{
      أكد( اظہاریہ_Use_XList(p_سمتیہ) );
      أرجع p_سمتیہ->x.pList->a[i].p_اظہاریہ;
    }
  }
  أرجع p_سمتیہ;
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
اظہاریہ *sqlite3_اظہاریہ_For_سمتیہ_میدان(
  Parse *pParse,       /* Parsing context */
  اظہاریہ *p_سمتیہ,       /* The vector.  List of expressions or a sub-SELECT */
  صحيح i_میدان,          /* Which column of the vector to return */
  صحيح n_میدان           /* Total number of columns in the vector */
){
  اظہاریہ *pRet;
  إذا( p_سمتیہ->op==TK_SELECT ){
    أكد( اظہاریہ_Use_XSelect(p_سمتیہ) );
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_کالم, 0, 0);
    إذا( pRet ){
      اظہاریہ_سیٹ_Property(pRet, EP_بھریا_ناپ);
      pRet->i_جدول = n_میدان;
      pRet->i_کالم = i_میدان;
      pRet->p_بایاں = p_سمتیہ;
    }
  }وإلا{
    إذا( p_سمتیہ->op==TK_سمتیہ ){
      اظہاریہ **pp_سمتیہ;
      أكد( اظہاریہ_Use_XList(p_سمتیہ) );
      pp_سمتیہ = &p_سمتیہ->x.pList->a[i_میدان].p_اظہاریہ;
      p_سمتیہ = *pp_سمتیہ;
      إذا( وچ_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_سمتیہ = 0;
        أرجع p_سمتیہ;
      }
    }
    pRet = sqlite3_اظہاریہ_Dup(pParse->db, p_سمتیہ, 0);
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
ساكن صحيح اظہاریہ_کوڈ_Subselect(Parse *pParse, اظہاریہ *p_اظہاریہ){
  صحيح reg = 0;
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
  إذا( p_اظہاریہ->op==TK_SELECT ){
    reg = sqlite3_کوڈ_Subselect(pParse, p_اظہاریہ);
  }
#جے_انت
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
ساكن صحيح اظہاریہ_سمتیہ_رجسٹر(
  Parse *pParse,                  /* Parse context */
  اظہاریہ *p_سمتیہ,                  /* Vector to extract element from */
  صحيح i_میدان,                     /* Field to extract from pVector */
  صحيح regSelect,                  /* First in array of registers */
  اظہاریہ *pTmp,                     /* Temporary space */
  اظہاریہ **pp_اظہاریہ,                  /* OUT: Expression element */
  صحيح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_سمتیہ->op;
  أكد( op==TK_سمتیہ || op==TK_رجسٹر || op==TK_SELECT || op==TK_ERROR );
  إذا( op==TK_رجسٹر ){
    *pp_اظہاریہ = sqlite3_سمتیہ_میدان_Subexpr(p_سمتیہ, i_میدان);
    أرجع p_سمتیہ->i_جدول+i_میدان;
  }
  إذا( op==TK_SELECT ){
    أكد( اظہاریہ_Use_XSelect(p_سمتیہ) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, حجم(اظہاریہ));
    pTmp->op = TK_SELECT_کالم;
    pTmp->p_بایاں = p_سمتیہ;
    pTmp->i_کالم = i_میدان;
    pTmp->i_جدول = p_سمتیہ->x.pSelect->pEList->n_اظہاریہ;
    *pp_اظہاریہ = pTmp;
    أرجع regSelect+i_میدان;
  }
  إذا( op==TK_سمتیہ ){
    أكد( اظہاریہ_Use_XList(p_سمتیہ) );
    *pp_اظہاریہ = p_سمتیہ->x.pList->a[i_میدان].p_اظہاریہ;
    أرجع sqlite3_اظہاریہ_کوڈ_عارضی(pParse, *pp_اظہاریہ, pRegFree);
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
ساكن فراغ کوڈ_سمتیہ_Compare(
  Parse *pParse,        /* Code generator context */
  اظہاریہ *p_اظہاریہ,          /* The comparison operation */
  صحيح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
  اظہاریہ *p_دایاں = p_اظہاریہ->p_دایاں;
  صحيح n_بایاں = sqlite3_اظہاریہ_سمتیہ_ناپ(p_بایاں);
  صحيح i;
  صحيح reg_بایاں = 0;
  صحيح reg_دایاں = 0;
  u8 opx = op;
  صحيح addrCmp = 0;
  صحيح addr_پورا = sqlite3VdbeMakeLabel(pParse);
  صحيح isCommuted = اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted);

  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  إذا( pParse->nErr ) أرجع;
  إذا( n_بایاں!=sqlite3_اظہاریہ_سمتیہ_ناپ(p_دایاں) ){
    sqlite3_Error_پیغام(pParse, "row value misused");
    أرجع;
  }
  أكد( p_اظہاریہ->op==TK_EQ || p_اظہاریہ->op==TK_NE
       || p_اظہاریہ->op==TK_IS || p_اظہاریہ->op==TK_ISNOT
       || p_اظہاریہ->op==TK_LT || p_اظہاریہ->op==TK_GT
       || p_اظہاریہ->op==TK_LE || p_اظہاریہ->op==TK_GE
  );
  أكد( p_اظہاریہ->op==op || (p_اظہاریہ->op==TK_IS && op==TK_EQ)
            || (p_اظہاریہ->op==TK_ISNOT && op==TK_NE) );
  أكد( p5==0 || p_اظہاریہ->op!=op );
  أكد( p5==SQLITE_NULLEQ || p_اظہاریہ->op==op );

  إذا( op==TK_LE ) opx = TK_LT;
  إذا( op==TK_GE ) opx = TK_GT;
  إذا( op==TK_NE ) opx = TK_EQ;

  reg_بایاں = اظہاریہ_کوڈ_Subselect(pParse, p_بایاں);
  reg_دایاں = اظہاریہ_کوڈ_Subselect(pParse, p_دایاں);

  sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, dest);
  لكل(i=0; 1 /*Loop exits by "break"*/; i++){
    صحيح regFree1 = 0, regFree2 = 0;
    اظہاریہ *pL = 0, *pR = 0;
    اظہاریہ tmp1, tmp2;
    صحيح r1, r2;
    أكد( i>=0 && i<n_بایاں );
    إذا( addrCmp ) sqlite3_Vdbe_Jump_ایتھے(v, addrCmp);
    r1 = اظہاریہ_سمتیہ_رجسٹر(pParse, p_بایاں, i, reg_بایاں, &tmp1, &pL, &regFree1);
    r2 = اظہاریہ_سمتیہ_رجسٹر(pParse, p_دایاں, i, reg_دایاں, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    کوڈ_Compare(pParse, pL, pR, opx, r1, r2, addr_پورا, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_عارضی_Reg(pParse, regFree1);
    sqlite3_Release_عارضی_Reg(pParse, regFree2);
    إذا( (opx==TK_LT || opx==TK_GT) && i<n_بایاں-1 ){
      addrCmp = sqlite3_Vdbe_جوڑو_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    إذا( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, dest);
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    إذا( i==n_بایاں-1 ){
      اكسر;
    }
    إذا( opx==TK_EQ ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, dest, addr_پورا); VdbeCoverage(v);
    }وإلا{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, addr_پورا);
      إذا( i==n_بایاں-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_ایتھے(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_پورا);
  إذا( op==TK_NE ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Not, dest, dest);
  }
}

#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحيح sqlite3_اظہاریہ_Check_اچائی(Parse *pParse, صحيح n_اچائی){
  صحيح rc = SQLITE_OK;
  صحيح mx_اچائی = pParse->db->a_حد_بندی[SQLITE_حد_بندی_اظہاریہ_ڈونگھائی];
  إذا( n_اچائی>mx_اچائی ){
    sqlite3_Error_پیغام(pParse,
       "Expression tree is too large (maximum depth %d)", mx_اچائی
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
ساكن فراغ اچائی_دا_اظہاریہ(ثابت اظہاریہ *p, صحيح *pn_اچائی){
  إذا( p ){
    إذا( p->n_اچائی>*pn_اچائی ){
      *pn_اچائی = p->n_اچائی;
    }
  }
}
ساكن فراغ اچائی_دا_اظہاریہ_List(ثابت اظہاریہ_List *p, صحيح *pn_اچائی){
  إذا( p ){
    صحيح i;
    لكل(i=0; i<p->n_اظہاریہ; i++){
      اچائی_دا_اظہاریہ(p->a[i].p_اظہاریہ, pn_اچائی);
    }
  }
}
ساكن فراغ اچائی_دا_Select(ثابت Select *pSelect, صحيح *pn_اچائی){
  ثابت Select *p;
  لكل(p=pSelect; p; p=p->p_پہلوں){
    اچائی_دا_اظہاریہ(p->pWhere, pn_اچائی);
    اچائی_دا_اظہاریہ(p->pHaving, pn_اچائی);
    اچائی_دا_اظہاریہ(p->p_حد_بندی, pn_اچائی);
    اچائی_دا_اظہاریہ_List(p->pEList, pn_اچائی);
    اچائی_دا_اظہاریہ_List(p->p_گروہ_By, pn_اچائی);
    اچائی_دا_اظہاریہ_List(p->p_ترتیب_By, pn_اچائی);
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
ساكن فراغ اظہاریہ_سیٹ_اچائی(اظہاریہ *p){
  صحيح n_اچائی = p->p_بایاں ? p->p_بایاں->n_اچائی : 0;
  إذا( NEVER(p->p_دایاں) && p->p_دایاں->n_اچائی>n_اچائی ){
    n_اچائی = p->p_دایاں->n_اچائی;
  }
  إذا( اظہاریہ_Use_XSelect(p) ){
    اچائی_دا_Select(p->x.pSelect, &n_اچائی);
  }وإلا إذا( p->x.pList ){
    اچائی_دا_اظہاریہ_List(p->x.pList, &n_اچائی);
    p->flags |= EP_Propagate & sqlite3_اظہاریہ_List_Flags(p->x.pList);
  }
  p->n_اچائی = n_اچائی + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3_اظہاریہ_سیٹ_اچائی_And_Flags(Parse *pParse, اظہاریہ *p){
  إذا( pParse->nErr ) أرجع;
  اظہاریہ_سیٹ_اچائی(p);
  sqlite3_اظہاریہ_Check_اچائی(pParse, p->n_اچائی);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحيح sqlite3_Select_اظہاریہ_اچائی(ثابت Select *p){
  صحيح n_اچائی = 0;
  اچائی_دا_Select(p, &n_اچائی);
  أرجع n_اچائی;
}
#وإلا /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
فراغ sqlite3_اظہاریہ_سیٹ_اچائی_And_Flags(Parse *pParse, اظہاریہ *p){
  إذا( pParse->nErr ) أرجع;
  إذا( p && اظہاریہ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_اظہاریہ_List_Flags(p->x.pList);
  }
}
#تعریف_کرو اظہاریہ_سیٹ_اچائی(y)
#جے_انت /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
فراغ sqlite3_اظہاریہ_سیٹ_Error_وتھ(اظہاریہ *p_اظہاریہ, صحيح iOfst){
  إذا( p_اظہاریہ==0 ) أرجع;
  إذا( NEVER(اظہاریہ_Use_WJoin(p_اظہاریہ)) ) أرجع;
  p_اظہاریہ->w.iOfst = iOfst;
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
اظہاریہ *sqlite3_اظہاریہ_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  صحيح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحيح dequote             /* True to dequote */
){
  اظہاریہ *pNew;
  صحيح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, حجم(اظہاریہ)+nExtra);
  إذا( pNew ){
    memset(pNew, 0, حجم(اظہاریہ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    إذا( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (حرف*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      إذا( pToken->n ) حافظہ_نقل(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      إذا( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_اظہاریہ(pNew);
      }
    }
#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
    pNew->n_اچائی = 1;
#جے_انت 
  }
  أرجع pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
اظہاریہ *sqlite3_اظہاریہ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحيح op,                 /* Expression opcode */
  ثابت حرف *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  أرجع sqlite3_اظہاریہ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
اظہاریہ *sqlite3_اظہاریہ_Int32(sqlite3 *db, صحيح iVal){
  اظہاریہ *pNew = sqlite3DbMallocRawNN(db, حجم(اظہاریہ));
  إذا( pNew ){
    memset(pNew, 0, حجم(اظہاریہ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_قدر|EP_پتی|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_قدر = iVal;
#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
    pNew->n_اچائی = 1;
#جے_انت 
  }
  أرجع pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
فراغ sqlite3_اظہاریہ_Attach_Subtrees(
  sqlite3 *db,
  اظہاریہ *p_جڑ,
  اظہاریہ *p_بایاں,
  اظہاریہ *p_دایاں
){
  إذا( p_جڑ==0 ){
    أكد( db->mallocFailed );
    sqlite3_اظہاریہ_Delete(db, p_بایاں);
    sqlite3_اظہاریہ_Delete(db, p_دایاں);
  }وإلا{
    أكد( اظہاریہ_Use_XList(p_جڑ) );
    أكد( p_جڑ->x.pSelect==0 );
    إذا( p_دایاں ){
      p_جڑ->p_دایاں = p_دایاں;
      p_جڑ->flags |= EP_Propagate & p_دایاں->flags;
#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
      p_جڑ->n_اچائی = p_دایاں->n_اچائی+1;
    }وإلا{
      p_جڑ->n_اچائی = 1;
#جے_انت
    }
    إذا( p_بایاں ){
      p_جڑ->p_بایاں = p_بایاں;
      p_جڑ->flags |= EP_Propagate & p_بایاں->flags;
#إذا SQLITE_MAX_اظہاریہ_ڈونگھائی>0
      إذا( p_بایاں->n_اچائی>=p_جڑ->n_اچائی ){
        p_جڑ->n_اچائی = p_بایاں->n_اچائی+1;
      }
#جے_انت
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
اظہاریہ *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  صحيح op,                 /* Expression opcode */
  اظہاریہ *p_بایاں,            /* Left operand */
  اظہاریہ *p_دایاں            /* Right operand */
){
  اظہاریہ *p;
  p = sqlite3DbMallocRawNN(pParse->db, حجم(اظہاریہ));
  إذا( p ){
    memset(p, 0, حجم(اظہاریہ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_اظہاریہ_Attach_Subtrees(pParse->db, p, p_بایاں, p_دایاں);
    sqlite3_اظہاریہ_Check_اچائی(pParse, p->n_اچائی);
  }وإلا{
    sqlite3_اظہاریہ_Delete(pParse->db, p_بایاں);
    sqlite3_اظہاریہ_Delete(pParse->db, p_دایاں);
  }
  أرجع p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
فراغ sqlite3_PExpr_جوڑو_Select(Parse *pParse, اظہاریہ *p_اظہاریہ, Select *pSelect){
  إذا( p_اظہاریہ ){
    p_اظہاریہ->x.pSelect = pSelect;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_xIsSelect|EP_Subquery);
    sqlite3_اظہاریہ_سیٹ_اچائی_And_Flags(pParse, p_اظہاریہ);
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
Select *sqlite3_اظہاریہ_List_منزل_اقدار(Parse *pParse, صحيح nElem, اظہاریہ_List *pEList){
  صحيح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  لكل(ii=0; ii<pEList->n_اظہاریہ; ii++){
    Select *pSel;
    اظہاریہ *p_اظہاریہ = pEList->a[ii].p_اظہاریہ;
    صحيح n_اظہاریہ_Elem;
    إذا( p_اظہاریہ->op==TK_سمتیہ ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      n_اظہاریہ_Elem = p_اظہاریہ->x.pList->n_اظہاریہ;
    }وإلا{
      n_اظہاریہ_Elem = 1;
    }
    إذا( n_اظہاریہ_Elem!=nElem ){
      sqlite3_Error_پیغام(pParse, "IN(...) element has %d term%s - expected %d",
          n_اظہاریہ_Elem, n_اظہاریہ_Elem>1?"s":"", nElem
      );
      اكسر;
    }
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pSel = sqlite3SelectNew(pParse, p_اظہاریہ->x.pList, 0, 0, 0, 0, 0, SF_اقدار,0);
    p_اظہاریہ->x.pList = 0;
    إذا( pSel ){
      إذا( pRet ){
        pSel->op = TK_ALL;
        pSel->p_پہلوں = pRet;
      }
      pRet = pSel;
    }
  }

  إذا( pRet && pRet->p_پہلوں ){
    pRet->selFlags |= SF_Multi_قدر;
  }
  sqlite3_اظہاریہ_List_Delete(pParse->db, pEList);
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
اظہاریہ *sqlite3_اظہاریہ_And(Parse *pParse, اظہاریہ *p_بایاں, اظہاریہ *p_دایاں){
  sqlite3 *db = pParse->db;
  إذا( p_بایاں==0  ){
    أرجع p_دایاں;
  }وإلا إذا( p_دایاں==0 ){
    أرجع p_بایاں;
  }وإلا{
    u32 f = p_بایاں->flags | p_دایاں->flags;
    إذا( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !وچ_RENAME_OBJECT
    ){
      اظہاریہ *p;
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_بایاں);
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_دایاں);
      p = sqlite3_اظہاریہ_Int32(db, 0);
      إذا( p ) p->op2 = 1;
      أرجع p;
    }وإلا{
      أرجع sqlite3PExpr(pParse, TK_AND, p_بایاں, p_دایاں);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
اظہاریہ *sqlite3_اظہاریہ_Function(
  Parse *pParse,        /* Parsing context */
  اظہاریہ_List *pList,      /* Argument list */
  ثابت Token *pToken,  /* Name of the function */
  صحيح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  اظہاریہ *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3_اظہاریہ_Alloc(db, TK_FUNCTION, pToken, 1);
  إذا( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    أرجع 0;
  }
  أكد( !اظہاریہ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحيح)(pToken->z - pParse->z_پوچھ);
  إذا( pList
   && pList->n_اظہاریہ > pParse->db->a_حد_بندی[SQLITE_حد_بندی_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_پیغام(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  اظہاریہ_سیٹ_Property(pNew, EP_HasFunc);
  أكد( اظہاریہ_Use_XList(pNew) );
  sqlite3_اظہاریہ_سیٹ_اچائی_And_Flags(pParse, pNew);
  إذا( eDistinct==SF_Distinct ) اظہاریہ_سیٹ_Property(pNew, EP_Distinct);
  أرجع pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
فراغ sqlite3_اظہاریہ_ترتیب_By_Aggregate_Error(Parse *pParse, اظہاریہ *p){
  sqlite3_Error_پیغام(pParse,
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
فراغ sqlite3_اظہاریہ_جوڑو_Function_ترتیب_By(
  Parse *pParse,        /* Parsing context */
  اظہاریہ *p_اظہاریہ,          /* The function call to which ORDER BY is to be added */
  اظہاریہ_List *p_ترتیب_By    /* The ORDER BY clause to add */
){
  اظہاریہ *pOB;
  sqlite3 *db = pParse->db;
  إذا( NEVER(p_ترتیب_By==0) ){
    أكد( db->mallocFailed );
    أرجع;
  }
  إذا( p_اظہاریہ==0 ){
    أكد( db->mallocFailed );
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  أكد( p_اظہاریہ->op==TK_FUNCTION );
  أكد( p_اظہاریہ->p_بایاں==0 );
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  إذا( p_اظہاریہ->x.pList==0 || NEVER(p_اظہاریہ->x.pList->n_اظہاریہ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_List_Delete_Generic, p_ترتیب_By);
    أرجع;
  }
  إذا( Is_کھڑکی_Func(p_اظہاریہ) ){
    sqlite3_اظہاریہ_ترتیب_By_Aggregate_Error(pParse, p_اظہاریہ);
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  إذا( p_ترتیب_By->n_اظہاریہ>db->a_حد_بندی[SQLITE_حد_بندی_کالم] ){
    sqlite3_Error_پیغام(pParse, "too many terms in ORDER BY clause");
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    أرجع;
  }

  pOB = sqlite3_اظہاریہ_Alloc(db, TK_ترتیب, 0, 0);
  إذا( pOB==0 ){
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    أرجع;
  }
  pOB->x.pList = p_ترتیب_By;
  أكد( اظہاریہ_Use_XList(pOB) );
  p_اظہاریہ->p_بایاں = pOB;
  اظہاریہ_سیٹ_Property(pOB, EP_بھریا_ناپ);
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
فراغ sqlite3_اظہاریہ_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت اظہاریہ *p_اظہاریہ,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !وچ_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_سدھا|SQLITE_FUNC_UNSAFE))!=0 );
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    إذا( (pDef->funcFlags & SQLITE_FUNC_سدھا)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_پیغام(pParse, "unsafe use of %#T()", p_اظہاریہ);
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
فراغ sqlite3_اظہاریہ_سونپو_Var_نمبر(Parse *pParse, اظہاریہ *p_اظہاریہ, u32 n){
  sqlite3 *db = pParse->db;
  ثابت حرف *z;
  ynVar x;

  إذا( p_اظہاریہ==0 ) أرجع;
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر|EP_Reduced|EP_TokenOnly) );
  z = p_اظہاریہ->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  إذا( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }وإلا{
    صحيح do_جوڑو = 0;
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
      testcase( i==db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر]-1 );
      testcase( i==db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] );
      إذا( bOk==0 || i<1 || i>db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] ){
        sqlite3_Error_پیغام(pParse, "variable number must be between ?1 and ?%d",
            db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر]);
        sqlite3_ریکارڈ_Error_وتھ_دا_اظہاریہ(pParse->db, p_اظہاریہ);
        أرجع;
      }
      x = (ynVar)i;
      إذا( x>pParse->nVar ){
        pParse->nVar = (صحيح)x;
        do_جوڑو = 1;
      }وإلا إذا( x<SQLITE_VNBMC*64 ){
        do_جوڑو = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }وإلا إذا( sqlite3_VList_عدد_منزل_Name(pParse->pVList, x)==0 ){
        do_جوڑو = 1;
      }
    }وإلا{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_منزل_عدد(pParse->pVList, z, n);
      إذا( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_جوڑو = 1;
      }
    }
    إذا( do_جوڑو ){
      pParse->pVList = sqlite3_VList_جوڑو(db, pParse->pVList, z, n, x);
      إذا( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_اظہاریہ->i_کالم = x;
  إذا( x>db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] ){
    sqlite3_Error_پیغام(pParse, "too many SQL variables");
    sqlite3_ریکارڈ_Error_وتھ_دا_اظہاریہ(pParse->db, p_اظہاریہ);
  }
}

/*
** Recursively delete an expression tree.
*/
ساكن SQLITE_NOINLINE فراغ sqlite3_اظہاریہ_Delete_NN(sqlite3 *db, اظہاریہ *p){
  أكد( p!=0 );
  أكد( db!=0 );
اظہاریہ_Delete_Restart:
  أكد( !اظہاریہ_Use_UValue(p) || p->u.i_قدر>=0 );
  أكد( !اظہاریہ_Use_YWin(p) || !اظہاریہ_Use_YSub(p) );
  أكد( !اظہاریہ_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !اظہاریہ_Use_YSub(p) );
#جے_تعریف_شدہ SQLITE_DEBUG
  إذا( اظہاریہ_Has_Property(p, EP_پتی) && !اظہاریہ_Has_Property(p, EP_TokenOnly) ){
    أكد( p->p_بایاں==0 );
    أكد( p->p_دایاں==0 );
    أكد( !اظہاریہ_Use_XSelect(p) || p->x.pSelect==0 );
    أكد( !اظہاریہ_Use_XList(p) || p->x.pList==0 );
  }
#جے_انت
  إذا( !اظہاریہ_Has_Property(p, (EP_TokenOnly|EP_پتی)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (اظہاریہ_Use_XList(p) && p->x.pList==0) || p->p_دایاں==0 );
    إذا( p->p_دایاں ){
      أكد( !اظہاریہ_Has_Property(p, EP_WinFunc) );
      sqlite3_اظہاریہ_Delete_NN(db, p->p_دایاں);
    }وإلا إذا( اظہاریہ_Use_XSelect(p) ){
      أكد( !اظہاریہ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }وإلا{
      sqlite3_اظہاریہ_List_Delete(db, p->x.pList);
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
      إذا( اظہاریہ_Has_Property(p, EP_WinFunc) ){
        sqlite3_کھڑکی_Delete(db, p->y.pWin);
      }
#جے_انت
    }
    إذا( p->p_بایاں && p->op!=TK_SELECT_کالم ){
      اظہاریہ *p_بایاں = p->p_بایاں;
      إذا( !اظہاریہ_Has_Property(p, EP_Static)
       && !اظہاریہ_Has_Property(p_بایاں, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_بایاں;
        اذهب اظہاریہ_Delete_Restart;
      }وإلا{
        sqlite3_اظہاریہ_Delete_NN(db, p_بایاں);
      }
    }
  }
  إذا( !اظہاریہ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
فراغ sqlite3_اظہاریہ_Delete(sqlite3 *db, اظہاریہ *p){
  إذا( p ) sqlite3_اظہاریہ_Delete_NN(db, p);
}
فراغ sqlite3_اظہاریہ_Delete_Generic(sqlite3 *db, فراغ *p){
  إذا( ALWAYS(p) ) sqlite3_اظہاریہ_Delete_NN(db, (اظہاریہ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
فراغ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  إذا( p==0 ){
    /* Nothing to clear */
  }وإلا إذا( p->pOn ){
    sqlite3_اظہاریہ_Delete_NN(db, p->pOn);
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
صحيح sqlite3_اظہاریہ_Deferred_Delete(Parse *pParse, اظہاریہ *p_اظہاریہ){
  أرجع 0==sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_Delete_Generic, p_اظہاریہ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
فراغ sqlite3_اظہاریہ_Unmap_And_Delete(Parse *pParse, اظہاریہ *p){
  إذا( p ){
    إذا( وچ_RENAME_OBJECT ){
      sqlite3_Rename_اظہاریہ_Unmap(pParse, p);
    }
    sqlite3_اظہاریہ_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ساكن صحيح اظہاریہ_Struct_ناپ(ثابت اظہاریہ *p){
  إذا( اظہاریہ_Has_Property(p, EP_TokenOnly) ) أرجع اظہاریہ_TOKENONLYSIZE;
  إذا( اظہاریہ_Has_Property(p, EP_Reduced) ) أرجع اظہاریہ_REDUCEDSIZE;
  أرجع اظہاریہ_FULLSIZE;
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
ساكن صحيح duped_اظہاریہ_Struct_ناپ(ثابت اظہاریہ *p, صحيح flags){
  صحيح n_ناپ;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( اظہاریہ_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  إذا( 0==flags || اظہاریہ_Has_Property(p, EP_بھریا_ناپ) ){
    n_ناپ = اظہاریہ_FULLSIZE;
  }وإلا{
    أكد( !اظہاریہ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    أكد( !اظہاریہ_Has_Property(p, EP_OuterON) );
    أكد( !اظہاریہ_Has_VVAProperty(p, EP_NoReduce) );
    إذا( p->p_بایاں || p->x.pList ){
      n_ناپ = اظہاریہ_REDUCEDSIZE | EP_Reduced;
    }وإلا{
      أكد( p->p_دایاں==0 );
      n_ناپ = اظہاریہ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  أرجع n_ناپ;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ساكن صحيح duped_اظہاریہ_گرہ_ناپ(ثابت اظہاریہ *p, صحيح flags){
  صحيح nByte = duped_اظہاریہ_Struct_ناپ(p, flags) & 0xfff;
  إذا( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
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
ساكن صحيح duped_اظہاریہ_ناپ(ثابت اظہاریہ *p){
  صحيح nByte;
  أكد( p!=0 );
  nByte = duped_اظہاریہ_گرہ_ناپ(p, EXPRDUP_REDUCE);
  إذا( p->p_بایاں ) nByte += duped_اظہاریہ_ناپ(p->p_بایاں);
  إذا( p->p_دایاں ) nByte += duped_اظہاریہ_ناپ(p->p_دایاں);
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
#جے_تعریف_شدہ SQLITE_DEBUG
  u8 *z_سرا;            /* First byte past the end of memory */
#جے_انت
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ساكن اظہاریہ *اظہاریہ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت اظہاریہ *p,        /* Expr tree to be duplicated */
  صحيح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  اظہاریہ *pNew;           /* Value to return */
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
#جے_تعریف_شدہ SQLITE_DEBUG
    sEdupBuf.z_سرا = pEdupBuf->z_سرا;
#جے_انت
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }وإلا{
    صحيح nAlloc;
    إذا( dupFlags ){
      nAlloc = duped_اظہاریہ_ناپ(p);
    }وإلا إذا( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(اظہاریہ_FULLSIZE + nToken);
    }وإلا{
      nToken = 0;
      nAlloc = ROUND8(اظہاریہ_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#جے_تعریف_شدہ SQLITE_DEBUG
    sEdupBuf.z_سرا = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#جے_انت
    
    staticFlag = 0;
  }
  pNew = (اظہاریہ *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  إذا( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت بنانشانی n_Struct_ناپ = duped_اظہاریہ_Struct_ناپ(p, dupFlags);
    صحيح n_New_ناپ = n_Struct_ناپ & 0xfff;
    إذا( nToken<0 ){
      إذا( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }وإلا{
        nToken = 0;
      }
    }
    إذا( dupFlags ){
      أكد( (صحيح)(sEdupBuf.z_سرا - sEdupBuf.zAlloc) >= n_New_ناپ+nToken );
      أكد( اظہاریہ_Has_Property(p, EP_Reduced)==0 );
      حافظہ_نقل(sEdupBuf.zAlloc, p, n_New_ناپ);
    }وإلا{
      u32 n_ناپ = (u32)اظہاریہ_Struct_ناپ(p);
      أكد( (صحيح)(sEdupBuf.z_سرا - sEdupBuf.zAlloc) >=
                                                   (صحيح)اظہاریہ_FULLSIZE+nToken );
      حافظہ_نقل(sEdupBuf.zAlloc, p, n_ناپ);
      إذا( n_ناپ<اظہاریہ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_ناپ], 0, اظہاریہ_FULLSIZE-n_ناپ);
      }
      n_New_ناپ = اظہاریہ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_ناپ & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    اظہاریہ_Clear_VVAProperties(pNew);
    إذا( dupFlags ){
      اظہاریہ_سیٹ_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    إذا( nToken>0 ){
      حرف *zToken = pNew->u.zToken = (حرف*)&sEdupBuf.zAlloc[n_New_ناپ];
      حافظہ_نقل(zToken, p->u.zToken, nToken);
      n_New_ناپ += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_ناپ);

    إذا( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_پتی))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      إذا( اظہاریہ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }وإلا{
        pNew->x.pList = sqlite3_اظہاریہ_List_Dup(db, p->x.pList,
                           p->op!=TK_ترتیب ? dupFlags : 0);
      }

#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
      إذا( اظہاریہ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_کھڑکی_Dup(db, pNew, p->y.pWin);
        أكد( اظہاریہ_Has_Property(pNew, EP_WinFunc) );
      }
#جے_انت /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      إذا( dupFlags ){
        إذا( p->op==TK_SELECT_کالم ){
          pNew->p_بایاں = p->p_بایاں;
          أكد( p->p_دایاں==0 
               || p->p_دایاں==p->p_بایاں
               || اظہاریہ_Has_Property(p->p_بایاں, EP_Subquery) );
        }وإلا{
          pNew->p_بایاں = p->p_بایاں ?
                      اظہاریہ_Dup(db, p->p_بایاں, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_دایاں = p->p_دایاں ?
                       اظہاریہ_Dup(db, p->p_دایاں, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }وإلا{
        إذا( p->op==TK_SELECT_کالم ){
          pNew->p_بایاں = p->p_بایاں;
          أكد( p->p_دایاں==0 
               || p->p_دایاں==p->p_بایاں
               || اظہاریہ_Has_Property(p->p_بایاں, EP_Subquery) );
        }وإلا{
          pNew->p_بایاں = sqlite3_اظہاریہ_Dup(db, p->p_بایاں, 0);
        }
        pNew->p_دایاں = sqlite3_اظہاریہ_Dup(db, p->p_دایاں, 0);
      }
    }
  }
  إذا( pEdupBuf ) حافظہ_نقل(pEdupBuf, &sEdupBuf, حجم(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.z_سرا );
  أرجع pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#جے_بنا_تعریف SQLITE_OMIT_CTE
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
        pRet->a[i].p_خانے_گنتی = sqlite3_اظہاریہ_List_Dup(db, p->a[i].p_خانے_گنتی, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  أرجع pRet;
}
#وإلا
# define sqlite3WithDup(x,y) 0
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ساكن صحيح gatherSelectWindowsCallback(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_FUNCTION && اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    کھڑکی *pWin = p_اظہاریہ->y.pWin;
    أكد( pWin );
    أكد( Is_کھڑکی_Func(p_اظہاریہ) );
    أكد( pWin->ppThis==0 );
    sqlite3_کھڑکی_Link(pSelect, pWin);
  }
  أرجع WRC_Continue;
}
ساكن صحيح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  أرجع p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ساكن فراغ gatherSelectWindows(Select *p){
  Walker w;
  w.x_اظہاریہ_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#جے_انت


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
اظہاریہ *sqlite3_اظہاریہ_Dup(sqlite3 *db, ثابت اظہاریہ *p, صحيح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  أرجع p ? اظہاریہ_Dup(db, p, flags, 0) : 0;
}
اظہاریہ_List *sqlite3_اظہاریہ_List_Dup(sqlite3 *db, ثابت اظہاریہ_List *p, صحيح flags){
  اظہاریہ_List *pNew;
  بنية اظہاریہ_List_شے *p_شے;
  ثابت بنية اظہاریہ_List_شے *p_Old_شے;
  صحيح i;
  اظہاریہ *p_پہلوں_Select_خانہ_Old = 0;
  اظہاریہ *p_پہلوں_Select_خانہ_New = 0;
  أكد( db!=0 );
  إذا( p==0 ) أرجع 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_ناپ(db, p));
  إذا( pNew==0 ) أرجع 0;
  pNew->n_اظہاریہ = p->n_اظہاریہ;
  pNew->nAlloc = p->nAlloc;
  p_شے = pNew->a;
  p_Old_شے = p->a;
  لكل(i=0; i<p->n_اظہاریہ; i++, p_شے++, p_Old_شے++){
    اظہاریہ *p_Old_اظہاریہ = p_Old_شے->p_اظہاریہ;
    اظہاریہ *p_New_اظہاریہ;
    p_شے->p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_Old_اظہاریہ, flags);
    إذا( p_Old_اظہاریہ
     && p_Old_اظہاریہ->op==TK_SELECT_کالم
     && (p_New_اظہاریہ = p_شے->p_اظہاریہ)!=0
    ){
      إذا( p_New_اظہاریہ->p_دایاں ){
        p_پہلوں_Select_خانہ_Old = p_Old_اظہاریہ->p_دایاں;
        p_پہلوں_Select_خانہ_New = p_New_اظہاریہ->p_دایاں;
        p_New_اظہاریہ->p_بایاں = p_New_اظہاریہ->p_دایاں;
      }وإلا{
        إذا( p_Old_اظہاریہ->p_بایاں!=p_پہلوں_Select_خانہ_Old ){
          p_پہلوں_Select_خانہ_Old = p_Old_اظہاریہ->p_بایاں;
          p_پہلوں_Select_خانہ_New = sqlite3_اظہاریہ_Dup(db, p_پہلوں_Select_خانہ_Old, flags);
          p_New_اظہاریہ->p_دایاں = p_پہلوں_Select_خانہ_New;
        }
        p_New_اظہاریہ->p_بایاں = p_پہلوں_Select_خانہ_New;
      }
    }
    p_شے->zEName = sqlite3DbStrDup(db, p_Old_شے->zEName);
    p_شے->fg = p_Old_شے->fg;
    p_شے->u = p_Old_شے->u;
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
    Src_شے *p_New_شے = &pNew->a[i];
    ثابت Src_شے *p_Old_شے = &p->a[i];
    جدول *pTab;
    p_New_شے->fg = p_Old_شے->fg;
    إذا( p_Old_شے->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, حجم(Subquery));
      إذا( pNewSubq==0 ){
        أكد( db->mallocFailed );
        p_New_شے->fg.isSubquery = 0;
      }وإلا{
        حافظہ_نقل(pNewSubq, p_Old_شے->u4.pSubq, حجم(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        إذا( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_شے->fg.isSubquery = 0;
        }
      }
      p_New_شے->u4.pSubq = pNewSubq;
    }وإلا إذا( p_Old_شے->fg.fixedSchema ){
      p_New_شے->u4.pSchema = p_Old_شے->u4.pSchema;
    }وإلا{
      p_New_شے->u4.zDatabase = sqlite3DbStrDup(db, p_Old_شے->u4.zDatabase);
    }
    p_New_شے->zName = sqlite3DbStrDup(db, p_Old_شے->zName);
    p_New_شے->zAlias = sqlite3DbStrDup(db, p_Old_شے->zAlias);
    p_New_شے->iCursor = p_Old_شے->iCursor;
    إذا( p_New_شے->fg.is_اشاریہ_دار_By ){
      p_New_شے->u1.z_اشاریہ_دار_By = sqlite3DbStrDup(db, p_Old_شے->u1.z_اشاریہ_دار_By);
    }وإلا إذا( p_New_شے->fg.isTabFunc ){
      p_New_شے->u1.pFuncArg =
          sqlite3_اظہاریہ_List_Dup(db, p_Old_شے->u1.pFuncArg, flags);
    }وإلا{
      p_New_شے->u1.n_صف = p_Old_شے->u1.n_صف;
    }
    p_New_شے->u2 = p_Old_شے->u2;
    إذا( p_New_شے->fg.isCte ){
      p_New_شے->u2.pCteUse->nUse++;
    }
    pTab = p_New_شے->pSTab = p_Old_شے->pSTab;
    إذا( pTab ){
      pTab->nTabRef++;
    }
    إذا( p_Old_شے->fg.isUsing ){
      أكد( p_New_شے->fg.isUsing );
      p_New_شے->u3.pUsing = sqlite3IdListDup(db, p_Old_شے->u3.pUsing);
    }وإلا{
      p_New_شے->u3.pOn = sqlite3_اظہاریہ_Dup(db, p_Old_شے->u3.pOn, flags);
    }
    p_New_شے->خانہ_ورتیا = p_Old_شے->خانہ_ورتیا;
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
    بنية Id_List_شے *p_New_شے = &pNew->a[i];
    ثابت بنية Id_List_شے *p_Old_شے = &p->a[i];
    p_New_شے->zName = sqlite3DbStrDup(db, p_Old_شے->zName);
  }
  أرجع pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحيح flags){
  Select *pRet = 0;
  Select *p_اگلا = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  لكل(p=pDup; p; p=p->p_پہلوں){
    Select *pNew = sqlite3DbMallocRawNN(db, حجم(*p) );
    إذا( pNew==0 ) اكسر;
    pNew->pEList = sqlite3_اظہاریہ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_اظہاریہ_Dup(db, p->pWhere, flags);
    pNew->p_گروہ_By = sqlite3_اظہاریہ_List_Dup(db, p->p_گروہ_By, flags);
    pNew->pHaving = sqlite3_اظہاریہ_Dup(db, p->pHaving, flags);
    pNew->p_ترتیب_By = sqlite3_اظہاریہ_List_Dup(db, p->p_ترتیب_By, flags);
    pNew->op = p->op;
    pNew->p_اگلا = p_اگلا;
    pNew->p_پہلوں = 0;
    pNew->p_حد_بندی = sqlite3_اظہاریہ_Dup(db, p->p_حد_بندی, flags);
    pNew->i_حد_بندی = 0;
    pNew->i_وتھ = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_صف = p->n_Select_صف;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_کھڑکی_List_Dup(db, p->pWinDefn);
    إذا( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#جے_انت
    pNew->selId = p->selId;
    إذا( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_اگلا = 0;
      sqlite3SelectDelete(db, pNew);
      اكسر;
    }
    *pp = pNew;
    pp = &pNew->p_پہلوں;
    p_اگلا = pNew;
  }
  أرجع pRet;
}
#وإلا
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحيح flags){
  أكد( p==0 );
  أرجع 0;
}
#جے_انت


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
ساكن ثابت بنية اظہاریہ_List_شے zero_شے = {0};
SQLITE_NOINLINE اظہاریہ_List *sqlite3_اظہاریہ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  بنية اظہاریہ_List_شے *p_شے;
  اظہاریہ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  إذا( pList==0 ){
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
    أرجع 0;
  }
  pList->nAlloc = 4;
  pList->n_اظہاریہ = 1;
  p_شے = &pList->a[0];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
  أرجع pList;
}
SQLITE_NOINLINE اظہاریہ_List *sqlite3_اظہاریہ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اظہاریہ_List *pList,        /* List to which to append. Might be NULL */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  بنية اظہاریہ_List_شے *p_شے;
  اظہاریہ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  إذا( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, pList);
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
    أرجع 0;
  }وإلا{
    pList = pNew;
  }
  p_شے = &pList->a[pList->n_اظہاریہ++];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
  أرجع pList;
}
اظہاریہ_List *sqlite3_اظہاریہ_List_Append(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to append. Might be NULL */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  بنية اظہاریہ_List_شے *p_شے;
  إذا( pList==0 ){
    أرجع sqlite3_اظہاریہ_List_Append_New(pParse->db,p_اظہاریہ);
  }
  إذا( pList->nAlloc<pList->n_اظہاریہ+1 ){
    أرجع sqlite3_اظہاریہ_List_Append_Grow(pParse->db,pList,p_اظہاریہ);
  }
  p_شے = &pList->a[pList->n_اظہاریہ++];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
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
اظہاریہ_List *sqlite3_اظہاریہ_List_Append_سمتیہ(
  Parse *pParse,         /* Parsing context */
  اظہاریہ_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  اظہاریہ *p_اظہاریہ            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  صحيح n;
  صحيح i;
  صحيح i_پہلا = pList ? pList->n_اظہاریہ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  إذا( NEVER(pColumns==0) ) اذهب سمتیہ_append_error;
  إذا( p_اظہاریہ==0 ) اذهب سمتیہ_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  إذا( p_اظہاریہ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ)) ){
    sqlite3_Error_پیغام(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    اذهب سمتیہ_append_error;
  }

  لكل(i=0; i<pColumns->nId; i++){
    اظہاریہ *p_Sub_اظہاریہ = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ, i, pColumns->nId);
    أكد( p_Sub_اظہاریہ!=0 || db->mallocFailed );
    إذا( p_Sub_اظہاریہ==0 ) استمر;
    pList = sqlite3_اظہاریہ_List_Append(pParse, pList, p_Sub_اظہاریہ);
    إذا( pList ){
      أكد( pList->n_اظہاریہ==i_پہلا+i+1 );
      pList->a[pList->n_اظہاریہ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  إذا( !db->mallocFailed && p_اظہاریہ->op==TK_SELECT && ALWAYS(pList!=0) ){
    اظہاریہ *p_پہلا = pList->a[i_پہلا].p_اظہاریہ;
    أكد( p_پہلا!=0 );
    أكد( p_پہلا->op==TK_SELECT_کالم );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    p_پہلا->p_دایاں = p_اظہاریہ;
    p_اظہاریہ = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    p_پہلا->i_جدول = pColumns->nId;
  }

سمتیہ_append_error:
  sqlite3_اظہاریہ_Unmap_And_Delete(pParse, p_اظہاریہ);
  sqlite3IdListDelete(db, pColumns);
  أرجع pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
فراغ sqlite3_اظہاریہ_List_سیٹ_ترتیب_دیو_ترتیب(اظہاریہ_List *p, صحيح i_ترتیب_دیو_ترتیب, صحيح eNulls){
  بنية اظہاریہ_List_شے *p_شے;
  إذا( p==0 ) أرجع;
  أكد( p->n_اظہاریہ>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( i_ترتیب_دیو_ترتیب==SQLITE_SO_UNDEFINED
       || i_ترتیب_دیو_ترتیب==SQLITE_SO_ASC
       || i_ترتیب_دیو_ترتیب==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_شے = &p->a[p->n_اظہاریہ-1];
  أكد( p_شے->fg.bNulls==0 );
  إذا( i_ترتیب_دیو_ترتیب==SQLITE_SO_UNDEFINED ){
    i_ترتیب_دیو_ترتیب = SQLITE_SO_ASC;
  }
  p_شے->fg.ترتیب_دیو_Flags = (u8)i_ترتیب_دیو_ترتیب;

  إذا( eNulls!=SQLITE_SO_UNDEFINED ){
    p_شے->fg.bNulls = 1;
    إذا( i_ترتیب_دیو_ترتیب!=eNulls ){
      p_شے->fg.ترتیب_دیو_Flags |= KEYINFO_ترتیب_BIGNULL;
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
فراغ sqlite3_اظہاریہ_List_سیٹ_Name(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحيح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->e_Parse_منوال!=PARSE_منوال_UNMAP || dequote==0 );
  إذا( pList ){
    بنية اظہاریہ_List_شے *p_شے;
    أكد( pList->n_اظہاریہ>0 );
    p_شے = &pList->a[pList->n_اظہاریہ-1];
    أكد( p_شے->zEName==0 );
    أكد( p_شے->fg.eEName==ENAME_NAME );
    p_شے->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    إذا( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_شے->zEName);
      إذا( وچ_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ثابت فراغ*)p_شے->zEName, pName);
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
فراغ sqlite3_اظہاریہ_List_سیٹ_وسعت(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to add the span. */
  ثابت حرف *z_شروع,     /* Start of the span */
  ثابت حرف *z_سرا        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  إذا( pList ){
    بنية اظہاریہ_List_شے *p_شے = &pList->a[pList->n_اظہاریہ-1];
    أكد( pList->n_اظہاریہ>0 );
    إذا( p_شے->zEName==0 ){
      p_شے->zEName = sqlite3_Db_وسعت_Dup(db, z_شروع, z_سرا);
      p_شے->fg.eEName = ENAME_وسعت;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
فراغ sqlite3_اظہاریہ_List_Check_لمائی(
  Parse *pParse,
  اظہاریہ_List *pEList,
  ثابت حرف *zObject
){
  صحيح mx = pParse->db->a_حد_بندی[SQLITE_حد_بندی_کالم];
  testcase( pEList && pEList->n_اظہاریہ==mx );
  testcase( pEList && pEList->n_اظہاریہ==mx+1 );
  إذا( pEList && pEList->n_اظہاریہ>mx ){
    sqlite3_Error_پیغام(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ساكن SQLITE_NOINLINE فراغ اظہاریہ_List_Delete_NN(sqlite3 *db, اظہاریہ_List *pList){
  صحيح i = pList->n_اظہاریہ;
  بنية اظہاریہ_List_شے *p_شے =  pList->a;
  أكد( pList->n_اظہاریہ>0 );
  أكد( db!=0 );
  افعل{
    sqlite3_اظہاریہ_Delete(db, p_شے->p_اظہاریہ);
    إذا( p_شے->zEName ) sqlite3DbNNFreeNN(db, p_شے->zEName);
    p_شے++;
  }طالما( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
فراغ sqlite3_اظہاریہ_List_Delete(sqlite3 *db, اظہاریہ_List *pList){
  إذا( pList ) اظہاریہ_List_Delete_NN(db, pList);
}
فراغ sqlite3_اظہاریہ_List_Delete_Generic(sqlite3 *db, فراغ *pList){
  إذا( ALWAYS(pList) ) اظہاریہ_List_Delete_NN(db, (اظہاریہ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_اظہاریہ_List_Flags(ثابت اظہاریہ_List *pList){
  صحيح i;
  u32 m = 0;
  أكد( pList!=0 );
  لكل(i=0; i<pList->n_اظہاریہ; i++){
     اظہاریہ *p_اظہاریہ = pList->a[i].p_اظہاریہ;
     أكد( p_اظہاریہ!=0 );
     m |= p_اظہاریہ->flags;
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
صحيح sqlite3SelectWalkFail(Walker *pWalker, Select *Not_ورتیا){
  بن_ورتیا_PARAMETER(Not_ورتیا);
  pWalker->e_کوڈ = 0;
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
u32 sqlite3IsTrueOrFalse(ثابت حرف *z_وچ){
  إذا( sqlite3StrICmp(z_وچ, "true")==0  ) أرجع EP_IsTrue;
  إذا( sqlite3StrICmp(z_وچ, "false")==0 ) أرجع EP_IsFalse;
  أرجع 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحيح sqlite3_اظہاریہ_Id_منزل_True_False(اظہاریہ *p_اظہاریہ){
  u32 v;
  أكد( p_اظہاریہ->op==TK_ID || p_اظہاریہ->op==TK_STRING );
  إذا( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Quoted|EP_Int_قدر)
   && (v = sqlite3IsTrueOrFalse(p_اظہاریہ->u.zToken))!=0
  ){
    p_اظہاریہ->op = TK_TRUEFALSE;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, v);
    أرجع 1;
  }
  أرجع 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحيح sqlite3_اظہاریہ_Truth_قدر(ثابت اظہاریہ *p_اظہاریہ){
  p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely((اظہاریہ*)p_اظہاریہ);
  أكد( p_اظہاریہ->op==TK_TRUEFALSE );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
  أكد( sqlite3StrICmp(p_اظہاریہ->u.zToken,"true")==0
       || sqlite3StrICmp(p_اظہاریہ->u.zToken,"false")==0 );
  أرجع p_اظہاریہ->u.zToken[4]==0;
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
اظہاریہ *sqlite3_اظہاریہ_Simplified_And_Or(اظہاریہ *p_اظہاریہ){
  أكد( p_اظہاریہ!=0 );
  إذا( p_اظہاریہ->op==TK_AND || p_اظہاریہ->op==TK_OR ){
    اظہاریہ *p_دایاں = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ->p_دایاں);
    اظہاریہ *p_بایاں = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ->p_بایاں);
    إذا( اظہاریہ_Always_True(p_بایاں) || اظہاریہ_Always_False(p_دایاں) ){
      p_اظہاریہ = p_اظہاریہ->op==TK_AND ? p_دایاں : p_بایاں;
    }وإلا إذا( اظہاریہ_Always_True(p_دایاں) || اظہاریہ_Always_False(p_بایاں) ){
      p_اظہاریہ = p_اظہاریہ->op==TK_AND ? p_بایاں : p_دایاں;
    }
  }
  أرجع p_اظہاریہ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ساكن صحيح اظہاریہ_Eval_Rhs_پہلا(اظہاریہ *p_اظہاریہ){
  إذا( اظہاریہ_Has_Property(p_اظہاریہ->p_بایاں, EP_Subquery)
   && !اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery)
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
ساكن صحيح اظہاریہ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  اظہاریہ *p_اظہاریہ,       /* The comparison expression */
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
  إذا( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) && sqlite3_اظہاریہ_Can_Be_Null(p_اظہاریہ->p_دایاں) ){
    r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, pFree2);
    addrIsNull = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }وإلا{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, pFree1);
  إذا( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    إذا( اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery)
     && sqlite3_اظہاریہ_Can_Be_Null(p_اظہاریہ->p_بایاں)
    ){
      addrIsNull = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, pFree2);
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
ساكن SQLITE_NOINLINE صحيح اظہاریہ_گرہ_Is_Constant_Function(
  Walker *pWalker,
  اظہاریہ *p_اظہاریہ
){
  صحيح n;             /* Number of arguments */
  اظہاریہ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( p_اظہاریہ->op==TK_FUNCTION );
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly)
   || (pList = p_اظہاریہ->x.pList)==0
  ){;
    n = 0;
  }وإلا{
    n = pList->n_اظہاریہ;
    sqlite3_Walk_اظہاریہ_List(pWalker, pList);
    إذا( pWalker->e_کوڈ==0 ) أرجع WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_لبھو_Function(db, p_اظہاریہ->u.zToken, n, ENC(db), 0);
  إذا( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc))
  ){
    pWalker->e_کوڈ = 0;
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
ساكن صحيح اظہاریہ_گرہ_Is_Constant(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  أكد( pWalker->e_کوڈ>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  إذا( pWalker->e_کوڈ==2 && اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ){
    pWalker->e_کوڈ = 0;
    أرجع WRC_Abort;
  }

  بدل( p_اظہاریہ->op ){
    حالة TK_FUNCTION:
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
        pWalker->e_کوڈ = 0;  /* Window functions are never constant */
        أرجع WRC_Abort;
      }وإلا إذا( pWalker->e_کوڈ>=4 && pWalker->e_کوڈ<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        إذا( pWalker->e_کوڈ==5 ) اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_FromDDL);
        أرجع WRC_Continue;
      }وإلا إذا( اظہاریہ_Has_Property(p_اظہاریہ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        أرجع WRC_Continue;
      }وإلا إذا( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        أرجع اظہاریہ_گرہ_Is_Constant_Function(pWalker, p_اظہاریہ);
      }وإلا{
        pWalker->e_کوڈ = 0;  /* Function call is not constant */
        أرجع WRC_Abort;
      }
    حالة TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      إذا( sqlite3_اظہاریہ_Id_منزل_True_False(p_اظہاریہ) ){
        أرجع WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالة TK_کالم:
    حالة TK_AGG_FUNCTION:
    حالة TK_AGG_کالم:
      testcase( p_اظہاریہ->op==TK_ID );
      testcase( p_اظہاریہ->op==TK_کالم );
      testcase( p_اظہاریہ->op==TK_AGG_FUNCTION );
      testcase( p_اظہاریہ->op==TK_AGG_کالم );
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_خانہ) && pWalker->e_کوڈ!=2 ){
        أرجع WRC_Continue;
      }
      إذا( pWalker->e_کوڈ==3 && p_اظہاریہ->i_جدول==pWalker->u.iCur ){
        أرجع WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالة TK_IF_NULL_صف:
    حالة TK_رجسٹر:
    حالة TK_DOT:
    حالة TK_RAISE:
      testcase( p_اظہاریہ->op==TK_رجسٹر );
      testcase( p_اظہاریہ->op==TK_IF_NULL_صف );
      testcase( p_اظہاریہ->op==TK_DOT );
      testcase( p_اظہاریہ->op==TK_RAISE );
      pWalker->e_کوڈ = 0;
      أرجع WRC_Abort;
    حالة TK_IS:
    حالة TK_ISNOT:
      إذا( pWalker->e_کوڈ==6 ){
        pWalker->e_کوڈ = 0;
        أرجع WRC_Abort;
      }
      أرجع WRC_Continue;
    حالة TK_VARIABLE:
      إذا( pWalker->e_کوڈ==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_اظہاریہ->op = TK_NULL;
      }وإلا إذا( pWalker->e_کوڈ==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_کوڈ = 0;
        أرجع WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    افتراضي:
      testcase( p_اظہاریہ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_اظہاریہ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      أرجع WRC_Continue;
  }
}
ساكن صحيح اظہاریہ_Is_Const(Parse *pParse, اظہاریہ *p, صحيح initFlag){
  Walker w;
  w.e_کوڈ = initFlag;
  w.pParse = pParse;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#جے_تعریف_شدہ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#جے_انت
  sqlite3_Walk_اظہاریہ(&w, p);
  أرجع w.e_کوڈ;
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
صحيح sqlite3_اظہاریہ_Is_Constant(Parse *pParse, اظہاریہ *p){
  أرجع اظہاریہ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحيح sqlite3_اظہاریہ_List_Is_Constant(Parse *pParse, اظہاریہ_List *pList, صحيح bNoIs){
  ثابت صحيح iInit = bNoIs ? 6 : 1;
  صحيح ii;
  لكل(ii=0; ii<pList->n_اظہاریہ; ii++){
    إذا( 0==اظہاریہ_Is_Const(pParse, pList->a[ii].p_اظہاریہ, iInit) ) أرجع 0;
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
ساكن صحيح sqlite3_اظہاریہ_Is_Constant_Not_Join(Parse *pParse, اظہاریہ *p){
  أرجع اظہاریہ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ساكن صحيح اظہاریہ_Select_Walk_جدول_Constant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->e_کوڈ==3 || pWalker->e_کوڈ==0 );
  إذا( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_کوڈ = 0;
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
صحيح sqlite3_اظہاریہ_Is_جدول_Constant(اظہاریہ *p, صحيح iCur, صحيح bAllowSubq){
  Walker w;
  w.e_کوڈ = 3;
  w.pParse = 0;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant;
  إذا( bAllowSubq ){
    w.xSelectCallback = اظہاریہ_Select_Walk_جدول_Constant;
  }وإلا{
    w.xSelectCallback = sqlite3SelectWalkFail;
#جے_تعریف_شدہ SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#جے_انت
  }
  w.u.iCur = iCur;
  sqlite3_Walk_اظہاریہ(&w, p);
  أرجع w.e_کوڈ;
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
صحيح sqlite3_اظہاریہ_Is_Single_جدول_Constraint(
  اظہاریہ *p_اظہاریہ,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحيح iSrc,                    /* Which element of pSrcList to use */
  صحيح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت Src_شے *pSrc = &pSrcList->a[iSrc];
  إذا( pSrc->fg.jointype & JT_LTORJ ){
    أرجع 0;  /* rule (3) */
  }
  إذا( pSrc->fg.jointype & JT_بایاں ){
    إذا( !اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) أرجع 0;   /* rule (4a) */
    إذا( p_اظہاریہ->w.iJoin!=pSrc->iCursor ) أرجع 0;         /* rule (4b) */
  }وإلا{
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) أرجع 0;    /* rule (5) */
  }
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      إذا( p_اظہاریہ->w.iJoin==pSrcList->a[jj].iCursor ){
        إذا( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          أرجع 0;  /* restriction (6) */
        }
        اكسر;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  أرجع sqlite3_اظہاریہ_Is_جدول_Constant(p_اظہاریہ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ساكن صحيح اظہاریہ_گرہ_Is_Constant_Or_گروہ_By(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اظہاریہ_List *p_گروہ_By = pWalker->u.p_گروہ_By;
  صحيح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  لكل(i=0; i<p_گروہ_By->n_اظہاریہ; i++){
    اظہاریہ *p = p_گروہ_By->a[i].p_اظہاریہ;
    إذا( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_اظہاریہ_NNColl_Seq(pWalker->pParse, p);
      إذا( sqlite3_Is_ثنائی(pColl) ){
        أرجع WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    pWalker->e_کوڈ = 0;
    أرجع WRC_Abort;
  }

  أرجع اظہاریہ_گرہ_Is_Constant(pWalker, p_اظہاریہ);
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
صحيح sqlite3_اظہاریہ_Is_Constant_Or_گروہ_By(Parse *pParse, اظہاریہ *p, اظہاریہ_List *p_گروہ_By){
  Walker w;
  w.e_کوڈ = 1;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant_Or_گروہ_By;
  w.xSelectCallback = 0;
  w.u.p_گروہ_By = p_گروہ_By;
  w.pParse = pParse;
  sqlite3_Walk_اظہاریہ(&w, p);
  أرجع w.e_کوڈ;
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
صحيح sqlite3_اظہاریہ_Is_Constant_Or_Function(اظہاریہ *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  أرجع اظہاریہ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحيح sqlite3_اظہاریہ_شامل_اے_Subquery(اظہاریہ *p){
  Walker w;
  w.e_کوڈ = 1;
  w.x_اظہاریہ_Callback = sqlite3_اظہاریہ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#جے_تعریف_شدہ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#جے_انت
  sqlite3_Walk_اظہاریہ(&w, p);
  أرجع w.e_کوڈ==0;
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
صحيح sqlite3_اظہاریہ_Is_Integer(ثابت اظہاریہ *p, صحيح *p_قدر, Parse *pParse, صحيح bRSI){
  صحيح iSign = 1;   /* Either +1 or -1. */
  طالما( 1/*exit-by-break*/ ){
    إذا( NEVER(p==0) ) أرجع 0;
    إذا( اظہاریہ_Use_UValue(p) ){
      أكد( p->op==TK_INTEGER );
      *p_قدر = p->u.i_قدر*iSign;
      إذا( bRSI && pParse==0 && p->op2 ){
        أرجع 0;
      }
      أرجع 1;
    }
    إذا( p->op==TK_UPLUS ){
      p = p->p_بایاں;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_بایاں;
      pParse = 0;
      استمر;
    }
    إذا( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_قدر *pVal; /* The variable */
      صحيح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      إذا( NEVER(pParse->pVdbe==0) ) اكسر;
      إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) اكسر;
      pVal = sqlite3_Vdbe_Get_Bound_قدر(pParse->pReprepare, p->i_کالم,
                                      SQLITE_AFF_BLOB);
      إذا( pVal ){
        i64 vv;
        إذا( sqlite3_قدر_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_قدر_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_قدر = (صحيح)vv;
          rc = 1;       
        }
        sqlite3_قدر_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_کالم, bRSI);
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
صحيح sqlite3_اظہاریہ_Can_Be_Null(ثابت اظہاریہ *p){
  u8 op;
  أكد( p!=0 );
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_بایاں;
    أكد( p!=0 );
  }
  op = p->op;
  إذا( op==TK_رجسٹر ) op = p->op2;
  بدل( op ){
    حالة TK_INTEGER:
    حالة TK_STRING:
    حالة TK_FLOAT:
    حالة TK_BLOB:
      أرجع 0;
    حالة TK_کالم:
      أكد( اظہاریہ_Use_YTab(p) );
      أرجع اظہاریہ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#جے_تعریف_شدہ SQLITE_ALLOW_ROWID_وچ_VIEW
          || (p->i_کالم==XN_ROWID && IsView(p->y.pTab))
#جے_انت
          || (p->i_کالم>=0
              && p->y.pTab->a_خانہ!=0 /* Possible due to prior error */
              && ALWAYS(p->i_کالم<p->y.pTab->n_خانہ)
              && p->y.pTab->a_خانہ[p->i_کالم].notNull==0);
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
صحيح sqlite3_اظہاریہ_Needs_No_Affinity_Change(ثابت اظہاریہ *p, حرف aff){
  u8 op;
  صحيح unaryMinus = 0;
  إذا( aff==SQLITE_AFF_BLOB ) أرجع 1;
  طالما( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    إذا( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_بایاں;
  }
  op = p->op;
  إذا( op==TK_رجسٹر ) op = p->op2;
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
    حالة TK_کالم: {
      أكد( p->i_جدول>=0 );  /* p cannot be part of a CHECK constraint */
      أرجع aff>=SQLITE_AFF_NUMERIC && p->i_کالم<0;
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
  لكل(ii=0; ii<Array_ناپ(azOpt); ii++){
    إذا( sqlite3_کالم_اشاریہ(pTab, azOpt[ii])<0 ) أرجع azOpt[ii];
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
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
ساكن Select *is_Candidate_For_وچ_Opt(ثابت اظہاریہ *pX){
  Select *p;
  SrcList *pSrc;
  اظہاریہ_List *pEList;
  جدول *pTab;
  صحيح i;
  إذا( !اظہاریہ_Use_XSelect(pX) ) أرجع 0;                 /* Not a subquery */
  إذا( اظہاریہ_Has_Property(pX, EP_VarSelect)  ) أرجع 0;  /* Correlated subq */
  p = pX->x.pSelect;
  إذا( p->p_پہلوں ) أرجع 0;              /* Not a compound SELECT */
  إذا( p->selFlags & SF_Aggregate ){
    أرجع 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->p_گروہ_By==0 );              /* Has no GROUP BY clause */
  إذا( p->p_حد_بندی ) أرجع 0;              /* Has no LIMIT clause */
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
  لكل(i=0; i<pEList->n_اظہاریہ; i++){
    اظہاریہ *pRes = pEList->a[i].p_اظہاریہ;
    إذا( pRes->op!=TK_کالم ) أرجع 0;
    أكد( pRes->i_جدول==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  أرجع p;
}
#جے_انت /* SQLITE_OMIT_SUBQUERY */

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ساكن فراغ sqlite3_سیٹ_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  صحيح iCur,          /* Cursor for the index */
  صحيح regHasNull,    /* Register in which to store hasNull flag */
  صحيح e_ترتیب_دیو_ترتیب     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  صحيح addr1;
  صحيح op;
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, regHasNull);
  إذا( e_ترتیب_دیو_ترتیب==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }وإلا{
    op = OP_آخری;
  }
  addr1 = sqlite3_Vdbe_جوڑو_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_آخری?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_ایتھے(v, addr1);
}
#جے_انت


#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ساكن صحيح sqlite3_وچ_Rhs_Is_Constant(Parse *pParse, اظہاریہ *p_وچ){
  اظہاریہ *pLHS;
  صحيح res;
  أكد( !اظہاریہ_Has_Property(p_وچ, EP_xIsSelect) );
  pLHS = p_وچ->p_بایاں;
  p_وچ->p_بایاں = 0;
  res = sqlite3_اظہاریہ_Is_Constant(pParse, p_وچ);
  p_وچ->p_بایاں = pLHS;
  أرجع res;
}
#جے_انت

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
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
صحيح sqlite3_لبھو_وچ_اشاریہ(
  Parse *pParse,             /* Parsing context */
  اظہاریہ *pX,                  /* The IN expression */
  u32 وچ_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  صحيح *prRhsHasNull,         /* Register holding NULL status.  See notes */
  صحيح *aiMap,                /* Mapping from Index fields to RHS fields */
  صحيح *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  صحيح eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  صحيح iTab;                             /* Cursor of the RHS table */
  صحيح mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  أكد( pX->op==TK_وچ );
  mustBeUnique = (وچ_Flags & وچ_اشاریہ_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  إذا( prRhsHasNull && اظہاریہ_Use_XSelect(pX) ){
    صحيح i;
    اظہاریہ_List *pEList = pX->x.pSelect->pEList;
    لكل(i=0; i<pEList->n_اظہاریہ; i++){
      إذا( sqlite3_اظہاریہ_Can_Be_Null(pEList->a[i].p_اظہاریہ) ) اكسر;
    }
    إذا( i==pEList->n_اظہاریہ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  إذا( pParse->nErr==0 && (p = is_Candidate_For_وچ_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    جدول *pTab;                           /* Table <table>. */
    صحيح iDb;                               /* Database idx for pTab */
    اظہاریہ_List *pEList = p->pEList;
    صحيح n_اظہاریہ = pEList->n_اظہاریہ;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].p_اظہاریہ!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_کوڈ_Verify_Schema(pParse, iDb);
    sqlite3_جدول_جندرا(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    إذا( n_اظہاریہ==1 && pEList->a[0].p_اظہاریہ->i_کالم<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحيح iAddr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_جدول(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = وچ_اشاریہ_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_ایتھے(v, iAddr);
    }وإلا{
      اشاریہ *pIdx;                         /* Iterator variable */
      صحيح affinity_ok = 1;
      صحيح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      لكل(i=0; i<n_اظہاریہ && affinity_ok; i++){
        اظہاریہ *pLhs = sqlite3_سمتیہ_میدان_Subexpr(pX->p_بایاں, i);
        صحيح i_خانہ = pEList->a[i].p_اظہاریہ->i_کالم;
        حرف idxaff = sqlite3_جدول_کالم_Affinity(pTab,i_خانہ); /* RHS table */
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
        لكل(pIdx=pTab->p_اشاریہ; pIdx && eType==0; pIdx=pIdx->p_اگلا){
          Bitmask خانہ_ورتیا;      /* Columns of the index used */
          Bitmask m_خانہ;         /* Mask for the current column */
          إذا( pIdx->n_کالم<n_اظہاریہ ) استمر;
          إذا( pIdx->pPartIdxWhere!=0 ) استمر;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_کالم==BMS-2 );
          testcase( pIdx->n_کالم==BMS-1 );
          إذا( pIdx->n_کالم>=BMS-1 ) استمر;
          إذا( mustBeUnique ){
            إذا( pIdx->n_کلید_خانہ>n_اظہاریہ
             ||(pIdx->n_کالم>n_اظہاریہ && !Is_Unique_اشاریہ(pIdx))
            ){
              استمر;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          خانہ_ورتیا = 0;   /* Columns of index used so far */
          لكل(i=0; i<n_اظہاریہ; i++){
            اظہاریہ *pLhs = sqlite3_سمتیہ_میدان_Subexpr(pX->p_بایاں, i);
            اظہاریہ *pRhs = pEList->a[i].p_اظہاریہ;
            CollSeq *pReq = sqlite3_ثنائی_Compare_Coll_Seq(pParse, pLhs, pRhs);
            صحيح j;
 
            لكل(j=0; j<n_اظہاریہ; j++){
              إذا( pIdx->ai_کالم[j]!=pRhs->i_کالم ) استمر;
              أكد( pIdx->azColl[j] );
              إذا( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                استمر;
              }
              اكسر;
            }
            إذا( j==n_اظہاریہ ) اكسر;
            m_خانہ = MASKBIT(j);
            إذا( m_خانہ & خانہ_ورتیا ) اكسر; /* Each column used only once */
            خانہ_ورتیا |= m_خانہ;
            إذا( aiMap ) aiMap[i] = j;
          }
 
          أكد( n_اظہاریہ>0 && n_اظہاریہ<BMS );
          أكد( i==n_اظہاریہ || خانہ_ورتیا!=(MASKBIT(n_اظہاریہ)-1) );
          إذا( خانہ_ورتیا==(MASKBIT(n_اظہاریہ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحيح iAddr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( وچ_اشاریہ_اشاریہ_DESC == وچ_اشاریہ_اشاریہ_ASC+1 );
            eType = وچ_اشاریہ_اشاریہ_ASC + pIdx->a_ترتیب_دیو_ترتیب[0];
 
            إذا( prRhsHasNull ){
#جے_تعریف_شدہ SQLITE_ENABLE_کالم_ورتیا_نقاب
              u64 نقاب = (1<<n_اظہاریہ)-1;
              sqlite3_Vdbe_جوڑو_Op3(v, OP_Columns_ورتیا,
                                iTab, LOWER32(نقاب), UPPER32(نقاب));
#جے_انت
              *prRhsHasNull = ++pParse->nMem;
              إذا( n_اظہاریہ==1 ){
                sqlite3_سیٹ_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_ترتیب_دیو_ترتیب[0]);
              }
            }
            sqlite3_Vdbe_Jump_ایتھے(v, iAddr);
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
   && (وچ_Flags & وچ_اشاریہ_NOOP_OK)
   && اظہاریہ_Use_XList(pX)
   && (!sqlite3_وچ_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_اظہاریہ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = وچ_اشاریہ_NOOP;
  }

  إذا( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحيح rMayHaveNull = 0;
    صحيح bloomOk = (وچ_Flags & وچ_اشاریہ_MEMBERSHIP)!=0;
    eType = وچ_اشاریہ_EPH;
    إذا( وچ_Flags & وچ_اشاریہ_LOOP ){
      pParse->nQueryLoop = 0;
    }وإلا إذا( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_وچ );
    إذا( !bloomOk
     && اظہاریہ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_وچ)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_کوڈ_Rhs_دا_وچ(pParse, pX, iTab, bloomOk);
    إذا( rMayHaveNull ){
      sqlite3_سیٹ_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  إذا( aiMap && eType!=وچ_اشاریہ_اشاریہ_ASC && eType!=وچ_اشاریہ_اشاریہ_DESC ){
    صحيح i, n;
    n = sqlite3_اظہاریہ_سمتیہ_ناپ(pX->p_بایاں);
    لكل(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  أرجع eType;
}
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ساكن حرف *اظہاریہ_INAffinity(Parse *pParse, ثابت اظہاریہ *p_اظہاریہ){
  اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
  صحيح nVal = sqlite3_اظہاریہ_سمتیہ_ناپ(p_بایاں);
  Select *pSelect = اظہاریہ_Use_XSelect(p_اظہاریہ) ? p_اظہاریہ->x.pSelect : 0;
  حرف *zRet;

  أكد( p_اظہاریہ->op==TK_وچ );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  إذا( zRet ){
    صحيح i;
    لكل(i=0; i<nVal; i++){
      اظہاریہ *pA = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
      حرف a = sqlite3_اظہاریہ_Affinity(pA);
      إذا( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_اظہاریہ, a);
      }وإلا{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  أرجع zRet;
}
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
فراغ sqlite3SubselectError(Parse *pParse, صحيح nActual, صحيح nExpect){
  إذا( pParse->nErr==0 ){
    ثابت حرف *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_پیغام(pParse, zFmt, nActual, nExpect);
  }
}
#جے_انت

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
فراغ sqlite3_سمتیہ_Error_پیغام(Parse *pParse, اظہاریہ *p_اظہاریہ){
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
  إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    sqlite3SubselectError(pParse, p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ, 1);
  }وإلا
#جے_انت
  {
    sqlite3_Error_پیغام(pParse, "row value misused");
  }
}

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ساكن صحيح لبھو_Compatible_وچ_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  اظہاریہ *p_اظہاریہ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_سرا;
  SubrtnSig *pSig;
  Vdbe *v;

  إذا( pNewSig==0 ) أرجع 0;
  إذا( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) أرجع 0;
  أكد( p_اظہاریہ->op==TK_وچ );
  أكد( !اظہاریہ_Use_YSub(p_اظہاریہ) );
  أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
  أكد( p_اظہاریہ->x.pSelect!=0 );
  أكد( (p_اظہاریہ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_سرا = sqlite3_Vdbe_Get_آخری_Op(v);
  لكل(; pOp<p_سرا; pOp++){
    إذا( pOp->p4type!=P4_SUBRTNSIG ) استمر;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    إذا( !pSig->bComplete ) استمر;
    إذا( pNewSig->selId!=pSig->selId ) استمر;
    إذا( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) استمر;
    p_اظہاریہ->y.sub.iAddr = pSig->iAddr;
    p_اظہاریہ->y.sub.regReturn = pSig->regReturn;
    p_اظہاریہ->i_جدول = pSig->i_جدول;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Subrtn);
    أرجع 1;
  }
  أرجع 0;
}
#جے_انت /* SQLITE_OMIT_SUBQUERY */

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
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
فراغ sqlite3_کوڈ_Rhs_دا_وچ(
  Parse *pParse,          /* Parsing context */
  اظہاریہ *p_اظہاریہ,            /* The IN operator */
  صحيح iTab,               /* Use this cursor number */
  صحيح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحيح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحيح addr;                   /* Address of OP_OpenEphemeral instruction */
  اظہاریہ *p_بایاں;                /* the LHS of the IN operator */
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
  إذا( !اظہاریہ_Has_Property(p_اظہاریہ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !اظہاریہ_Use_XSelect(p_اظہاریہ) || p_اظہاریہ->x.pSelect!=0 );
    إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) && (p_اظہاریہ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, حجم(pSig[0]));
      إذا( pSig ){
        pSig->selId = p_اظہاریہ->x.pSelect->selId;
        pSig->zAff = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) 
     || لبھو_Compatible_وچ_Rhs_Subrtn(pParse, p_اظہاریہ, pSig)
    ){
      addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
      إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_اظہاریہ->x.pSelect->selId));
      }
      أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_اظہاریہ->y.sub.regReturn,
                        p_اظہاریہ->y.sub.iAddr);
      أكد( iTab!=p_اظہاریہ->i_جدول );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenDup, iTab, p_اظہاریہ->i_جدول);
      sqlite3_Vdbe_Jump_ایتھے(v, addrOnce);
      إذا( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      أرجع;
    }

    /* Begin coding the subroutine */
    أكد( !اظہاریہ_Use_YWin(p_اظہاریہ) );
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Subrtn);
    أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );
    p_اظہاریہ->y.sub.regReturn = ++pParse->nMem;
    p_اظہاریہ->y.sub.iAddr =
      sqlite3_Vdbe_جوڑو_Op2(v, OP_BeginSubrtn, 0, p_اظہاریہ->y.sub.regReturn) + 1;
    إذا( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_اظہاریہ->y.sub.iAddr;
      pSig->regReturn = p_اظہاریہ->y.sub.regReturn;
      pSig->i_جدول = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ثابت حرف*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_بایاں = p_اظہاریہ->p_بایاں;
  nVal = sqlite3_اظہاریہ_سمتیہ_ناپ(p_بایاں);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_اظہاریہ->i_جدول = iTab;
  addr = sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, p_اظہاریہ->i_جدول, nVal);
#جے_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
  إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    VdbeComment((v, "Result of SELECT %u", p_اظہاریہ->x.pSelect->selId));
  }وإلا{
    VdbeComment((v, "RHS of IN operator"));
  }
#جے_انت
  p_کلید_Info = sqlite3_کلید_Info_Alloc(pParse->db, nVal, 1);
  أكد( p_کلید_Info!=0 || pParse->nErr );
  إذا( p_کلید_Info==0 ) أرجع;

  إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_اظہاریہ->x.pSelect;
    اظہاریہ_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    إذا( ALWAYS(pEList->n_اظہاریہ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحيح i;
      صحيح rc;
      صحيح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_سیٹ, iTab);
      dest.zAffSdst = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
      pSelect->i_حد_بندی = 0;
      أكد( pEList!=0 );
      أكد( pEList->n_اظہاریہ>0 );
      أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
      لكل(i=0; i<nVal; i++){
        اظہاریہ *p = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
        CollSeq *pColl;
        p_کلید_Info->aColl[i] = pColl = sqlite3_ثنائی_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_اظہاریہ
        );
        إذا( !sqlite3_Is_ثنائی(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      إذا( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحيح regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_جوڑو_Op2(v, OP_Blob, 10000, regBloom);
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
  }وإلا إذا( ALWAYS(p_اظہاریہ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    حرف affinity;            /* Affinity of the LHS of the IN */
    صحيح i;
    اظہاریہ_List *pList = p_اظہاریہ->x.pList;
    بنية اظہاریہ_List_شے *p_شے;
    صحيح r1, r2;
    affinity = sqlite3_اظہاریہ_Affinity(p_بایاں);
    إذا( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }وإلا إذا( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( p_کلید_Info!=0 );
    أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
    p_کلید_Info->aColl[0] = sqlite3_اظہاریہ_Coll_Seq(pParse, p_اظہاریہ->p_بایاں);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_عارضی_Reg(pParse);
    r2 = sqlite3_Get_عارضی_Reg(pParse);
    لكل(i=pList->n_اظہاریہ, p_شے=pList->a; i>0; i--, p_شے++){
      اظہاریہ *pE2 = p_شے->p_اظہاریہ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      إذا( addrOnce && !sqlite3_اظہاریہ_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_منزل_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_منزل_Noop(v, addrOnce);
        اظہاریہ_Clear_Property(p_اظہاریہ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_اظہاریہ_کوڈ(pParse, pE2, r1);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iTab, r2, r1, 1);
    }
    sqlite3_Release_عارضی_Reg(pParse, r1);
    sqlite3_Release_عارضی_Reg(pParse, r2);
  }
  إذا( pSig ) pSig->bComplete = 1;
  إذا( p_کلید_Info ){
    sqlite3VdbeChangeP4(v, addr, (فراغ *)p_کلید_Info, P4_KEYINFO);
  }
  إذا( addrOnce ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, iTab);
    sqlite3_Vdbe_Jump_ایتھے(v, addrOnce);
    /* Subroutine return */
    أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
    أكد( sqlite3VdbeGetOp(v,p_اظہاریہ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Return, p_اظہاریہ->y.sub.regReturn,
                      p_اظہاریہ->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_عارضی_Reg_Cache(pParse);
  }
}
#جے_انت /* SQLITE_OMIT_SUBQUERY */

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
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
صحيح sqlite3_کوڈ_Subselect(Parse *pParse, اظہاریہ *p_اظہاریہ){
  صحيح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحيح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحيح nReg;                   /* Registers to allocate */
  اظہاریہ *p_حد_بندی;               /* New limit expression */
#جے_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;            /* Address of OP_Explain instruction */
#جے_انت

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  إذا( pParse->nErr ) أرجع 0;
  testcase( p_اظہاریہ->op==TK_EXISTS );
  testcase( p_اظہاریہ->op==TK_SELECT );
  أكد( p_اظہاریہ->op==TK_EXISTS || p_اظہاریہ->op==TK_SELECT );
  أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
  pSel = p_اظہاریہ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_اظہاریہ->y.sub.regReturn,
                      p_اظہاریہ->y.sub.iAddr);
    أرجع p_اظہاریہ->i_جدول;
  }

  /* Begin coding the subroutine */
  أكد( !اظہاریہ_Use_YWin(p_اظہاریہ) );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced|EP_TokenOnly) );
  اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Subrtn);
  p_اظہاریہ->y.sub.regReturn = ++pParse->nMem;
  p_اظہاریہ->y.sub.iAddr =
    sqlite3_Vdbe_جوڑو_Op2(v, OP_BeginSubrtn, 0, p_اظہاریہ->y.sub.regReturn) + 1;

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
  إذا( !اظہاریہ_Has_Property(p_اظہاریہ, EP_VarSelect) ){
    addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
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
  sqlite3_Vdbe_جانچ_حالت_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_اظہاریہ->op==TK_SELECT ? pSel->pEList->n_اظہاریہ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  إذا( p_اظہاریہ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    إذا( (pSel->selFlags&SF_Distinct) && pSel->p_حد_بندی && pSel->p_حد_بندی->p_دایاں ){
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
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }وإلا{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  إذا( pSel->p_حد_بندی ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    اظہاریہ *p_بایاں = pSel->p_حد_بندی->p_بایاں;
    إذا( اظہاریہ_Has_Property(p_بایاں, EP_Int_قدر)==0
     || (p_بایاں->u.i_قدر!=1 && p_بایاں->u.i_قدر!=0)
    ){
      sqlite3 *db = pParse->db;
      p_حد_بندی = sqlite3_اظہاریہ_Int32(db, 0);
      إذا( p_حد_بندی ){
        p_حد_بندی->aff_اظہاریہ = SQLITE_AFF_NUMERIC;
        p_حد_بندی = sqlite3PExpr(pParse, TK_NE,
            sqlite3_اظہاریہ_Dup(db, p_بایاں, 0), p_حد_بندی);
      }
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_بایاں);
      pSel->p_حد_بندی->p_بایاں = p_حد_بندی;
    }
  }وإلا{
    /* If there is no pre-existing limit add a limit of 1 */
    p_حد_بندی = sqlite3_اظہاریہ_Int32(pParse->db, 1);
    pSel->p_حد_بندی = sqlite3PExpr(pParse, TK_حد_بندی, p_حد_بندی, 0);
  }
  pSel->i_حد_بندی = 0;
  إذا( sqlite3Select(pParse, pSel, &dest) ){
    p_اظہاریہ->op2 = p_اظہاریہ->op;
    p_اظہاریہ->op = TK_ERROR;
    أرجع 0;
  }
  p_اظہاریہ->i_جدول = rReg = dest.iSDParm;
  اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);
  إذا( addrOnce ){
    sqlite3_Vdbe_Jump_ایتھے(v, addrOnce);
  }
  sqlite3_Vdbe_جانچ_حالت_Range(v, addrExplain, addrExplain, -1);

  إذا( dest.eDest==SRT_Mem 
   && sqlite3_اظہاریہ_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_اظہاریہ) 
  ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
  أكد( sqlite3VdbeGetOp(v,p_اظہاریہ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3_Vdbe_جوڑو_Op3(v, OP_Return, p_اظہاریہ->y.sub.regReturn,
                    p_اظہاریہ->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_عارضی_Reg_Cache(pParse);
  أرجع rReg;
}
#جے_انت /* SQLITE_OMIT_SUBQUERY */

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحيح sqlite3_اظہاریہ_Check_وچ(Parse *pParse, اظہاریہ *p_وچ){
  صحيح n_سمتیہ = sqlite3_اظہاریہ_سمتیہ_ناپ(p_وچ->p_بایاں);
  إذا( اظہاریہ_Use_XSelect(p_وچ) && !pParse->db->mallocFailed ){
    إذا( n_سمتیہ!=p_وچ->x.pSelect->pEList->n_اظہاریہ ){
      sqlite3SubselectError(pParse, p_وچ->x.pSelect->pEList->n_اظہاریہ, n_سمتیہ);
      أرجع 1;
    }
  }وإلا إذا( n_سمتیہ!=1 ){
    sqlite3_سمتیہ_Error_پیغام(pParse, p_وچ->p_بایاں);
    أرجع 1;
  }
  أرجع 0;
}
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
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
ساكن فراغ sqlite3_اظہاریہ_کوڈ_وچ(
  Parse *pParse,        /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,          /* The IN expression */
  صحيح destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  صحيح destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  صحيح rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  صحيح eType;            /* Type of the RHS */
  صحيح rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  صحيح *aiMap = 0;       /* Map from vector field to index column */
  حرف *zAff = 0;       /* Affinity string for comparisons */
  صحيح n_سمتیہ;          /* Size of vectors for this IN operator */
  صحيح iDummy;           /* Dummy parameter to exprCodeVector() */
  اظہاریہ *p_بایاں;          /* The LHS of the IN operator */
  صحيح i;                /* loop counter */
  صحيح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحيح destStep6 = 0;    /* Start of code for Step 6 */
  صحيح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحيح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحيح addr_چوٹی;          /* Top of the step-6 loop */
  صحيح iTab = 0;         /* Index to use */
  u8 ok_Const_ضارب = pParse->ok_Const_ضارب;

  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  p_بایاں = p_اظہاریہ->p_بایاں;
  إذا( sqlite3_اظہاریہ_Check_وچ(pParse, p_اظہاریہ) ) أرجع;
  zAff = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
  n_سمتیہ = sqlite3_اظہاریہ_سمتیہ_ناپ(p_اظہاریہ->p_بایاں);
  aiMap = (صحيح*)sqlite3DbMallocZero(pParse->db, n_سمتیہ*حجم(صحيح));
  إذا( pParse->db->mallocFailed ) اذهب sqlite3_اظہاریہ_کوڈ_وچ_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_لبھو_وچ_اشاریہ(pParse, p_اظہاریہ,
                             وچ_اشاریہ_MEMBERSHIP | وچ_اشاریہ_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  أكد( pParse->nErr || n_سمتیہ==1 || eType==وچ_اشاریہ_EPH
       || eType==وچ_اشاریہ_اشاریہ_ASC || eType==وچ_اشاریہ_اشاریہ_DESC
  );
#جے_تعریف_شدہ SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  لكل(i=0; i<n_سمتیہ; i++){
    صحيح j, cnt;
    لكل(cnt=j=0; j<n_سمتیہ; j++) إذا( aiMap[j]==i ) cnt++;
    أكد( cnt==1 );
  }
#جے_انت

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
  أكد( pParse->ok_Const_ضارب==ok_Const_ضارب );
  pParse->ok_Const_ضارب = 0;
  rLhs = اظہاریہ_کوڈ_سمتیہ(pParse, p_بایاں, &iDummy);
  pParse->ok_Const_ضارب = ok_Const_ضارب;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  إذا( eType==وچ_اشاریہ_NOOP ){
    اظہاریہ_List *pList;
    CollSeq *pColl;
    صحيح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحيح r2, reg_منزل_Free;
    صحيح regCkNull = 0;
    صحيح ii;
    أكد( n_سمتیہ==1 );
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_اظہاریہ->p_بایاں);
    إذا( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_ثنائی_ہندسہ_And, rLhs, rLhs, regCkNull);
    }
    لكل(ii=0; ii<pList->n_اظہاریہ; ii++){
      r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, pList->a[ii].p_اظہاریہ, &reg_منزل_Free);
      إذا( regCkNull && sqlite3_اظہاریہ_Can_Be_Null(pList->a[ii].p_اظہاریہ) ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_ثنائی_ہندسہ_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_عارضی_Reg(pParse, reg_منزل_Free);
      إذا( ii<pList->n_اظہاریہ-1 || destIfNull!=destIfFalse ){
        صحيح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_جوڑو_Op4(v, op, rLhs, labelOk, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_اظہاریہ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_اظہاریہ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_اظہاریہ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_اظہاریہ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }وإلا{
        صحيح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3_Vdbe_جوڑو_Op4(v, op, rLhs, destIfFalse, r2,
                          (فراغ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    إذا( regCkNull ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_عارضی_Reg(pParse, regCkNull);
    اذهب sqlite3_اظہاریہ_کوڈ_وچ_مکمل;
  }

  إذا( eType!=وچ_اشاریہ_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    إذا( n_سمتیہ==1 ){
      حرف aff = zAff[0];
      إذا( aff>=SQLITE_AFF_متن && aff!=sqlite3_اظہاریہ_Affinity(p_بایاں) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحيح rTmp = sqlite3_Get_عارضی_Reg(pParse);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, rLhs, n_سمتیہ, 0, zAff, n_سمتیہ);
    }

    لكل(i=0; i<n_سمتیہ && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    إذا( i!=n_سمتیہ ){
      /* Need to reorder the LHS fields according to aiMap */
      صحيح rLhsOrig = rLhs;
      rLhs = sqlite3_Get_عارضی_Range(pParse, n_سمتیہ);
      لكل(i=0; i<n_سمتیہ; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
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
  لكل(i=0; i<n_سمتیہ; i++){
    اظہاریہ *p = sqlite3_سمتیہ_میدان_Subexpr(p_اظہاریہ->p_بایاں, i);
    إذا( pParse->nErr ) اذهب sqlite3_اظہاریہ_کوڈ_وچ_oom_error;
    إذا( sqlite3_اظہاریہ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  إذا( eType==وچ_اشاریہ_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( n_سمتیہ==1 );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);  /* Return True */
  }وإلا{
    إذا( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, p_اظہاریہ->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        إذا( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_سمتیہ); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_لبھیا, iTab, destIfFalse,
                           rLhs, n_سمتیہ); VdbeCoverage(v);
      اذهب sqlite3_اظہاریہ_کوڈ_وچ_مکمل;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_لبھیا, iTab, 0,
                                      rLhs, n_سمتیہ); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  إذا( rRhsHasNull && n_سمتیہ==1 ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
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
  إذا( eType==وچ_اشاریہ_اشاریہ_DESC ){
    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_آخری, iTab, destIfFalse);
  }وإلا{
    testcase( eType==وچ_اشاریہ_EPH );
    testcase( eType==وچ_اشاریہ_ROWID );
    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  إذا( n_سمتیہ>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }وإلا{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  لكل(i=0; i<n_سمتیہ; i++){
    اظہاریہ *p;
    CollSeq *pColl;
    صحيح r3 = sqlite3_Get_عارضی_Reg(pParse);
    p = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
    إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
      اظہاریہ *pRhs = p_اظہاریہ->x.pSelect->pEList->a[i].p_اظہاریہ;
      pColl = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p, pRhs);
    }وإلا{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iTab, aiMap[i], r3);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (فراغ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_عارضی_Reg(pParse, r3);
  }
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, destIfNull);
  إذا( n_سمتیہ>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iTab, addr_چوٹی+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_ایتھے(v, addrTruthOp);

sqlite3_اظہاریہ_کوڈ_وچ_مکمل:
  VdbeComment((v, "end IN expr"));
sqlite3_اظہاریہ_کوڈ_وچ_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#جے_انت /* SQLITE_OMIT_SUBQUERY */

#جے_بنا_تعریف SQLITE_OMIT_FLOATING_نقطہ
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ساكن فراغ کوڈ_Real(Vdbe *v, ثابت حرف *z, صحيح negateFlag, صحيح iMem){
  إذا( ALWAYS(z!=0) ){
    مزدوج قدر;
    sqlite3AtoF(z, &قدر);
    أكد( !sqlite3IsNaN(قدر) ); /* The new AtoF never returns NaN */
    إذا( negateFlag ) قدر = -قدر;
    sqlite3_Vdbe_جوڑو_Double(v, iMem, قدر);
  }
}
#جے_انت


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ساكن فراغ کوڈ_Integer(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح negFlag, صحيح iMem){
  Vdbe *v = pParse->pVdbe;
  إذا( p_اظہاریہ->flags & EP_Int_قدر ){
    صحيح i = p_اظہاریہ->u.i_قدر;
    أكد( i>=0 );
    إذا( negFlag ) i = -i;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, i, iMem);
  }وإلا{
    صحيح c;
    i64 قدر;
    ثابت حرف *z = p_اظہاریہ->u.zToken;
    أكد( z!=0 );
    c = sqlite3_Dec_Or_Hex_منزل_I64(z, &قدر);
    إذا( (c==3 && !negFlag) || (c==2) || (negFlag && قدر==سبھ_توں_چھوٹا_INT64)){
#جے_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
      sqlite3_Error_پیغام(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_اظہاریہ);
#وإلا
#جے_بنا_تعریف SQLITE_OMIT_HEX_INTEGER
      إذا( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_پیغام(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_اظہاریہ);
      }وإلا
#جے_انت
      {
        کوڈ_Real(v, z, negFlag, iMem);
      }
#جے_انت
    }وإلا{
      إذا( negFlag ){ قدر = c==3 ? سبھ_توں_چھوٹا_INT64 : -قدر; }
      sqlite3_Vdbe_جوڑو_Int64(v, iMem, قدر);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
فراغ sqlite3_اظہاریہ_کوڈ_لدو_اشاریہ_کالم(
  Parse *pParse,  /* The parsing context */
  اشاریہ *pIdx,    /* The index whose column is to be loaded */
  صحيح iTabCur,    /* Cursor pointing to a table row */
  صحيح i_Idx_خانہ,    /* The column of the index to be loaded */
  صحيح reg_باہرلا      /* Store the index column value in this register */
){
  i16 i_Tab_خانہ = pIdx->ai_کالم[i_Idx_خانہ];
  إذا( i_Tab_خانہ==XN_اظہاریہ ){
    أكد( pIdx->a_خانہ_اظہاریہ );
    أكد( pIdx->a_خانہ_اظہاریہ->n_اظہاریہ>i_Idx_خانہ );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_اظہاریہ_کوڈ_Copy(pParse, pIdx->a_خانہ_اظہاریہ->a[i_Idx_خانہ].p_اظہاریہ, reg_باہرلا);
    pParse->iSelfTab = 0;
  }وإلا{
    sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(pParse->pVdbe, pIdx->p_جدول, iTabCur,
                                    i_Tab_خانہ, reg_باہرلا);
  }
}

#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
فراغ sqlite3_اظہاریہ_کوڈ_Generated_کالم(
  Parse *pParse,     /* Parsing context */
  جدول *pTab,       /* Table containing the generated column */
  کالم *p_خانہ,      /* The generated column */
  صحيح reg_باہرلا         /* Put the result in this register */
){
  صحيح iAddr;
  Vdbe *v = pParse->pVdbe;
  صحيح nErr = pParse->nErr;
  أكد( v!=0 );
  أكد( pParse->iSelfTab!=0 );
  إذا( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, pParse->iSelfTab-1, 0, reg_باہرلا);
  }وإلا{
    iAddr = 0;
  }
  sqlite3_اظہاریہ_کوڈ_Copy(pParse, sqlite3_کالم_اظہاریہ(pTab,p_خانہ), reg_باہرلا);
  إذا( (p_خانہ->خانہ_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحيح p3 = 2+(صحيح)(p_خانہ - pTab->a_خانہ);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_TypeCheck, reg_باہرلا, 1, p3, (حرف*)pTab, P4_جدول);
  }وإلا إذا( p_خانہ->affinity>=SQLITE_AFF_متن ){
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, reg_باہرلا, 1, 0, &p_خانہ->affinity, 1);
  }
  إذا( iAddr ) sqlite3_Vdbe_Jump_ایتھے(v, iAddr);
  إذا( pParse->nErr>nErr ) pParse->db->err_Byte_وتھ = -1;
}
#جے_انت /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
فراغ sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(
  Vdbe *v,        /* Parsing context */
  جدول *pTab,    /* The table containing the value */
  صحيح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحيح i_خانہ,       /* Index of the column to extract */
  صحيح reg_باہرلا      /* Extract the value into this register */
){
  کالم *p_خانہ;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( i_خانہ!=XN_اظہاریہ );
  إذا( i_خانہ<0 || i_خانہ==pTab->iPKey ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iTabCur, reg_باہرلا);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }وإلا{
    صحيح x;
    إذا( IsVirtual(pTab) ){
      x = i_خانہ;
      sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iTabCur, x, reg_باہرلا);
      أرجع;
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
    }وإلا إذا( (p_خانہ = &pTab->a_خانہ[i_خانہ])->خانہ_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      إذا( p_خانہ->خانہ_Flags & COLFLAG_BUSY ){
        sqlite3_Error_پیغام(pParse, "generated column loop on \"%s\"",
                        p_خانہ->zCnName);
      }وإلا{
        صحيح savedSelfTab = pParse->iSelfTab;
        p_خانہ->خانہ_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_اظہاریہ_کوڈ_Generated_کالم(pParse, pTab, p_خانہ, reg_باہرلا);
        pParse->iSelfTab = savedSelfTab;
        p_خانہ->خانہ_Flags &= ~COLFLAG_BUSY;
      }
      أرجع;
#جے_انت
    }وإلا إذا( !HasRowid(pTab) ){
      testcase( i_خانہ!=sqlite3_جدول_کالم_منزل_Storage(pTab, i_خانہ) );
      x = sqlite3_جدول_کالم_منزل_اشاریہ(sqlite3_Primary_کلید_اشاریہ(pTab), i_خانہ);
    }وإلا{
      x = sqlite3_جدول_کالم_منزل_Storage(pTab,i_خانہ);
      testcase( x!=i_خانہ );
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, iTabCur, x, reg_باہرلا);
    sqlite3_کالم_Default(v, pTab, i_خانہ, reg_باہرلا);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
صحيح sqlite3_اظہاریہ_کوڈ_Get_کالم(
  Parse *pParse,   /* Parsing and code generating context */
  جدول *pTab,     /* Description of the table we are reading from */
  صحيح i_کالم,     /* Index of the table column */
  صحيح i_جدول,      /* The cursor pointing to the table */
  صحيح iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  أكد( pParse->pVdbe!=0 );
  أكد( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  أكد( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_اظہاریہ_کوڈ_Get_کالم_دا_جدول(pParse->pVdbe, pTab, i_جدول, i_کالم, iReg);
  إذا( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_آخری_Op(pParse->pVdbe);
    إذا( pOp->opcode==OP_کالم ) pOp->p5 = p5;
    إذا( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  أرجع iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
فراغ sqlite3_اظہاریہ_کوڈ_Move(Parse *pParse, صحيح iFrom, صحيح i_منزل, صحيح nReg){
  sqlite3_Vdbe_جوڑو_Op3(pParse->pVdbe, OP_Move, iFrom, i_منزل, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
فراغ sqlite3_اظہاریہ_منزل_رجسٹر(اظہاریہ *p_اظہاریہ, صحيح iReg){
  اظہاریہ *p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  إذا( NEVER(p==0) ) أرجع;
  إذا( p->op==TK_رجسٹر ){
    أكد( p->i_جدول==iReg );
  }وإلا{
    p->op2 = p->op;
    p->op = TK_رجسٹر;
    p->i_جدول = iReg;
    اظہاریہ_Clear_Property(p, EP_Skip);
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
ساكن صحيح اظہاریہ_کوڈ_سمتیہ(Parse *pParse, اظہاریہ *p, صحيح *piFreeable){
  صحيح i_نتیجہ;
  صحيح n_نتیجہ = sqlite3_اظہاریہ_سمتیہ_ناپ(p);
  إذا( n_نتیجہ==1 ){
    i_نتیجہ = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p, piFreeable);
  }وإلا{
    *piFreeable = 0;
    إذا( p->op==TK_SELECT ){
#إذا SQLITE_OMIT_SUBQUERY
      i_نتیجہ = 0;
#وإلا
      i_نتیجہ = sqlite3_کوڈ_Subselect(pParse, p);
#جے_انت
    }وإلا{
      صحيح i;
      i_نتیجہ = pParse->nMem+1;
      pParse->nMem += n_نتیجہ;
      أكد( اظہاریہ_Use_XList(p) );
      لكل(i=0; i<n_نتیجہ; i++){
        sqlite3_اظہاریہ_کوڈ_Factorable(pParse, p->x.pList->a[i].p_اظہاریہ, i+i_نتیجہ);
      }
    }
  }
  أرجع i_نتیجہ;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ساكن فراغ سیٹ_Do_Not_ملاؤ_Flag_On_Copy(Vdbe *v){
  إذا( sqlite3_Vdbe_Get_آخری_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ساكن صحيح اظہاریہ_کوڈ_Inline_Function(
  Parse *pParse,        /* Parsing context */
  اظہاریہ_List *pFarg,      /* List of function arguments */
  صحيح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحيح نشانہ            /* Store function result in this register */
){
  صحيح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->n_اظہاریہ;
  أكد( nFarg>0 );  /* All in-line functions have at least one argument */
  بدل( iFuncId ){
    حالة INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      صحيح سرا_Coalesce = sqlite3VdbeMakeLabel(pParse);
      صحيح i;
      أكد( nFarg>=2 );
      sqlite3_اظہاریہ_کوڈ(pParse, pFarg->a[0].p_اظہاریہ, نشانہ);
      لكل(i=1; i<nFarg; i++){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, نشانہ, سرا_Coalesce);
        VdbeCoverage(v);
        sqlite3_اظہاریہ_کوڈ(pParse, pFarg->a[i].p_اظہاریہ, نشانہ);
      }
      سیٹ_Do_Not_ملاؤ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, سرا_Coalesce);
      اكسر;
    }
    حالة INLINEFUNC_iif: {
      اظہاریہ case_اظہاریہ;
      memset(&case_اظہاریہ, 0, حجم(case_اظہاریہ));
      case_اظہاریہ.op = TK_CASE;
      case_اظہاریہ.x.pList = pFarg;
      أرجع sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, &case_اظہاریہ, نشانہ);
    }
#جے_تعریف_شدہ SQLITE_ENABLE_وتھ_SQL_FUNC
    حالة INLINEFUNC_sqlite_وتھ: {
      اظہاریہ *pArg = pFarg->a[0].p_اظہاریہ;
      إذا( pArg->op==TK_کالم && pArg->i_جدول>=0 ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_وتھ, pArg->i_جدول, pArg->i_کالم, نشانہ);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
      }
      اكسر;
    }
#جے_انت
    افتراضي: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      أكد( nFarg==1 || nFarg==2 );
      نشانہ = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, pFarg->a[0].p_اظہاریہ, نشانہ);
      اكسر;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#إذا !defined(SQLITE_UNTESTABLE)
    حالة INLINEFUNC_اظہاریہ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
         sqlite3_اظہاریہ_Compare(0,pFarg->a[0].p_اظہاریہ, pFarg->a[1].p_اظہاریہ,-1),
         نشانہ);
      اكسر;
    }

    حالة INLINEFUNC_اظہاریہ_سٹا_دیندا_اظہاریہ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
         sqlite3_اظہاریہ_سٹا_دیندا_اظہاریہ(pParse,pFarg->a[0].p_اظہاریہ, pFarg->a[1].p_اظہاریہ,-1),
         نشانہ);
      اكسر;
    }

    حالة INLINEFUNC_سٹا_دیندا_nonnull_صف: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      اظہاریہ *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].p_اظہاریہ;
      إذا( pA1->op==TK_کالم ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
           sqlite3_اظہاریہ_سٹا_دیندا_Non_Null_صف(pFarg->a[0].p_اظہاریہ,pA1->i_جدول,1),
           نشانہ);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
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
      aff = sqlite3_اظہاریہ_Affinity(pFarg->a[0].p_اظہاریہ);
      أكد( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3_Vdbe_لدو_String(v, نشانہ,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      اكسر;
    }
#جے_انت /* !defined(SQLITE_UNTESTABLE) */
  }
  أرجع نشانہ;
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
ساكن صحيح اظہاریہ_گرہ_Can_Return_Subtype(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحيح n;
  FuncDef *pDef;
  sqlite3 *db;
  إذا( p_اظہاریہ->op==TK_CASE || p_اظہاریہ->op==TK_UPLUS 
   || p_اظہاریہ->op==TK_COLLATE || p_اظہاریہ->op==TK_CAST 
  ){
    أرجع WRC_Continue;
  }
  إذا( p_اظہاریہ->op!=TK_FUNCTION && p_اظہاریہ->op!=TK_AGG_FUNCTION ){
    أرجع WRC_Prune;
  }
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  db = pWalker->pParse->db;
  n = p_اظہاریہ->x.pList ? p_اظہاریہ->x.pList->n_اظہاریہ : 0;
  pDef = sqlite3_لبھو_Function(db, p_اظہاریہ->u.zToken, n, ENC(db), 0);
#جے_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  إذا( pDef==0 ) أرجع WRC_Continue;
#وإلا
  أكد( pDef!=0 );
#جے_انت
  إذا( (pDef->funcFlags & SQLITE_نتیجہ_SUBTYPE)!=0 ){
    pWalker->e_کوڈ = 1;
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
صحيح sqlite3_اظہاریہ_Can_Return_Subtype(Parse *pParse, اظہاریہ *p_اظہاریہ){
  Walker w;
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Can_Return_Subtype;
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  أرجع w.e_کوڈ;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ساكن SQLITE_NOINLINE صحيح sqlite3_اشاریہ_دار_اظہاریہ_Lookup(
  Parse *pParse,   /* The parsing context */
  اظہاریہ *p_اظہاریہ,     /* The expression to potentially bypass */
  صحيح نشانہ       /* Where to store the result of the expression */
){
  اشاریہ_دار_اظہاریہ *p;
  Vdbe *v;
  لكل(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 اظہاریہ_Aff;
    صحيح i_انکڑے_Cur = p->i_انکڑے_Cur;
    إذا( i_انکڑے_Cur<0 ) استمر;
    إذا( pParse->iSelfTab ){
      إذا( p->i_انکڑے_Cur!=pParse->iSelfTab-1 ) استمر;
      i_انکڑے_Cur = -1;
    }
    إذا( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, p->p_اظہاریہ, i_انکڑے_Cur)!=0 ) استمر;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    اظہاریہ_Aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
    إذا( (اظہاریہ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (اظہاریہ_Aff==SQLITE_AFF_متن && p->aff!=SQLITE_AFF_متن)
     || (اظہاریہ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      استمر;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_SubtArg)
     && sqlite3_اظہاریہ_Can_Return_Subtype(pParse, p_اظہاریہ) 
    ){
      استمر;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    إذا( p->b_Maybe_Null_صف || sqlite3_اظہاریہ_Is_Constant(pParse, p_اظہاریہ) ){
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
      sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, p->iIdxCur, addr+3, نشانہ);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, p->iIdxCur, p->i_Idx_خانہ, نشانہ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_خانہ));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ, نشانہ);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_ایتھے(v, addr+2);
    }وإلا{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, p->iIdxCur, p->i_Idx_خانہ, نشانہ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_خانہ));
    }
    أرجع نشانہ;
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
ساكن صحيح اظہاریہ_Partidx_اظہاریہ_Lookup(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح i_نشانہ){
  اشاریہ_دار_اظہاریہ *p;
  لكل(p=pParse->p_Idx_Part_اظہاریہ; p; p=p->pIENext){
    إذا( p_اظہاریہ->i_کالم==p->i_Idx_خانہ && p_اظہاریہ->i_جدول==p->i_انکڑے_Cur ){
      Vdbe *v = pParse->pVdbe;
      صحيح addr = 0;
      صحيح ret;

      إذا( p->b_Maybe_Null_صف ){
        addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_If_Null_صف, p->iIdxCur);
      }
      ret = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p->p_اظہاریہ, i_نشانہ);
      sqlite3_Vdbe_جوڑو_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت حرف*)&p->aff, 1);
      إذا( sqlite3_اظہاریہ_Can_Return_Subtype(pParse, p->p_اظہاریہ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_جوڑو_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      إذا( addr ){
        sqlite3_Vdbe_Jump_ایتھے(v, addr);
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
ساكن SQLITE_NOINLINE صحيح اظہاریہ_کوڈ_نشانہ_And_Or(
  Parse *pParse,     /* Parsing context */
  اظہاریہ *p_اظہاریہ,       /* AND or OR expression to be coded */
  صحيح نشانہ,        /* Put result in this register, guaranteed */
  صحيح *pTmpReg       /* Write a temporary register here */
){
  صحيح op;            /* The opcode.  TK_AND or TK_OR */
  صحيح skipOp;        /* Opcode for the branch that skips one operand */
  صحيح addrSkip;      /* Branch instruction that skips one of the operands */
  صحيح regSS = 0;     /* Register holding computed operand when other omitted */
  صحيح r1, r2;        /* Registers for left and right operands, respectively */
  اظہاریہ *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  أكد( p_اظہاریہ!=0 );
  op = p_اظہاریہ->op;
  أكد( op==TK_AND || op==TK_OR );
  أكد( TK_AND==OP_And );            testcase( op==TK_AND );
  أكد( TK_OR==OP_Or );              testcase( op==TK_OR );
  أكد( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
  إذا( pAlt!=p_اظہاریہ ){
    r1 = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, pAlt, نشانہ);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_And, r1, r1, نشانہ);
    أرجع نشانہ;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  إذا( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ->p_دایاں, نشانہ);
    addrSkip = sqlite3_Vdbe_جوڑو_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, pTmpReg);
  }وإلا{
    /* Compute the left operand first */
    r1 = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ->p_بایاں, نشانہ);
    إذا( اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_جوڑو_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }وإلا{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, pTmpReg);
  }
  sqlite3_Vdbe_جوڑو_Op3(v, op, r2, r1, نشانہ);
  testcase( (*pTmpReg)==0 );
  إذا( addrSkip ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_ایتھے(v, addrSkip);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Or, regSS, regSS, نشانہ);
    VdbeComment((v, "short-circut value"));
  }
  أرجع نشانہ;
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
صحيح sqlite3_اظہاریہ_کوڈ_نشانہ(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح نشانہ){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحيح op;                   /* The opcode being coded */
  صحيح وچ_Reg = نشانہ;       /* Results stored in register inReg */
  صحيح regFree1 = 0;         /* If non-zero free this temporary register */
  صحيح regFree2 = 0;         /* If non-zero free this temporary register */
  صحيح r1, r2;               /* Various register numbers */
  اظہاریہ عارضی_X;               /* Temporary expression node */
  صحيح p5 = 0;

  أكد( نشانہ>0 && نشانہ<=pParse->nMem );
  أكد( v!=0 );

اظہاریہ_کوڈ_doover:
  إذا( p_اظہاریہ==0 ){
    op = TK_NULL;
  }وإلا إذا( pParse->pIdxEpr!=0
   && !اظہاریہ_Has_Property(p_اظہاریہ, EP_پتی)
   && (r1 = sqlite3_اشاریہ_دار_اظہاریہ_Lookup(pParse, p_اظہاریہ, نشانہ))>=0
  ){
    أرجع r1;
  }وإلا{
    أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
    op = p_اظہاریہ->op;
  }
  أكد( op!=TK_ترتیب );
  بدل( op ){
    حالة TK_AGG_کالم: {
      AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
      بنية Agg_Info_خانہ *p_خانہ;
      أكد( pAggInfo!=0 );
      أكد( p_اظہاریہ->iAgg>=0 );
      إذا( p_اظہاریہ->iAgg>=pAggInfo->n_کالم ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
#جے_تعریف_شدہ SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotNull, نشانہ, 1, 20230325);
        Vdbe_Coverage_Never_لیا(v);
#جے_انت
        اكسر;
      }
      p_خانہ = &pAggInfo->a_خانہ[p_اظہاریہ->iAgg];
      إذا( !pAggInfo->سدھا_منوال ){
        أرجع Agg_Info_کالم_Reg(pAggInfo, p_اظہاریہ->iAgg);
      }وإلا إذا( pAggInfo->use_چھانٹی_Idx ){
        جدول *pTab = p_خانہ->pTab;
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, pAggInfo->چھانٹی_Idx_PTab,
                              p_خانہ->i_Sorter_کالم, نشانہ);
        إذا( pTab==0 ){
          /* No comment added */
        }وإلا إذا( p_خانہ->i_کالم<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }وإلا{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_خانہ[p_خانہ->i_کالم].zCnName));
          إذا( pTab->a_خانہ[p_خانہ->i_کالم].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, نشانہ);
          }
        }
        أرجع نشانہ;
      }وإلا إذا( p_اظہاریہ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, p_اظہاریہ->i_جدول, p_اظہاریہ->i_کالم, نشانہ);
        أرجع نشانہ;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالة TK_کالم: {
      صحيح iTab = p_اظہاریہ->i_جدول;
      صحيح iReg;
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_خانہ) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحيح aff;
        iReg = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ->p_بایاں,نشانہ);
        أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
        أكد( p_اظہاریہ->y.pTab!=0 );
        aff = sqlite3_جدول_کالم_Affinity(p_اظہاریہ->y.pTab, p_اظہاریہ->i_کالم);
        إذا( aff>SQLITE_AFF_BLOB ){
          ساكن ثابت حرف zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_متن=='B' );
          sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, iReg, 1, 0,
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
          کالم *p_خانہ;
          جدول *pTab;
          صحيح iSrc;
          صحيح i_خانہ = p_اظہاریہ->i_کالم;
          أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
          pTab = p_اظہاریہ->y.pTab;
          أكد( pTab!=0 );
          أكد( i_خانہ>=XN_ROWID );
          أكد( i_خانہ<pTab->n_خانہ );
          إذا( i_خانہ<0 ){
            أرجع -1-pParse->iSelfTab;
          }
          p_خانہ = pTab->a_خانہ + i_خانہ;
          testcase( i_خانہ!=sqlite3_جدول_کالم_منزل_Storage(pTab,i_خانہ) );
          iSrc = sqlite3_جدول_کالم_منزل_Storage(pTab, i_خانہ) - pParse->iSelfTab;
#جے_بنا_تعریف SQLITE_OMIT_GENERATED_COLUMNS
          إذا( p_خانہ->خانہ_Flags & COLFLAG_GENERATED ){
            إذا( p_خانہ->خانہ_Flags & COLFLAG_BUSY ){
              sqlite3_Error_پیغام(pParse, "generated column loop on \"%s\"",
                              p_خانہ->zCnName);
              أرجع 0;
            }
            p_خانہ->خانہ_Flags |= COLFLAG_BUSY;
            إذا( p_خانہ->خانہ_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_اظہاریہ_کوڈ_Generated_کالم(pParse, pTab, p_خانہ, iSrc);
            }
            p_خانہ->خانہ_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            أرجع iSrc;
          }وإلا
#جے_انت /* SQLITE_OMIT_GENERATED_COLUMNS */
          إذا( p_خانہ->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, iSrc, نشانہ);
            sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, نشانہ);
            أرجع نشانہ;
          }وإلا{
            أرجع iSrc;
          }
        }وإلا{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      وإلا إذا( pParse->p_Idx_Part_اظہاریہ 
       && 0!=(r1 = اظہاریہ_Partidx_اظہاریہ_Lookup(pParse, p_اظہاریہ, نشانہ))
      ){
        أرجع r1;
      }
      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      أكد( p_اظہاریہ->y.pTab!=0 );
      iReg = sqlite3_اظہاریہ_کوڈ_Get_کالم(pParse, p_اظہاریہ->y.pTab,
                               p_اظہاریہ->i_کالم, iTab, نشانہ,
                               p_اظہاریہ->op2);
      أرجع iReg;
    }
    حالة TK_INTEGER: {
      کوڈ_Integer(pParse, p_اظہاریہ, 0, نشانہ);
      أرجع نشانہ;
    }
    حالة TK_TRUEFALSE: {
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ), نشانہ);
      أرجع نشانہ;
    }
#جے_بنا_تعریف SQLITE_OMIT_FLOATING_نقطہ
    حالة TK_FLOAT: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      کوڈ_Real(v, p_اظہاریہ->u.zToken, 0, نشانہ);
      أرجع نشانہ;
    }
#جے_انت
    حالة TK_STRING: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_Vdbe_لدو_String(v, نشانہ, p_اظہاریہ->u.zToken);
      أرجع نشانہ;
    }
    حالة TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, نشانہ, نشانہ + p_اظہاریہ->y.nReg - 1);
      أرجع نشانہ;
    }
    افتراضي: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
      أرجع نشانہ;
    }
#جے_بنا_تعریف SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      صحيح n;
      ثابت حرف *z;
      حرف *zBlob;
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      أكد( p_اظہاریہ->u.zToken[0]=='x' || p_اظہاریہ->u.zToken[0]=='X' );
      أكد( p_اظہاریہ->u.zToken[1]=='\'' );
      z = &p_اظہاریہ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3_Hex_منزل_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Blob, n/2, نشانہ, 0, zBlob, P4_DYNAMIC);
      أرجع نشانہ;
    }
#جے_انت
    حالة TK_VARIABLE: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      أكد( p_اظہاریہ->u.zToken!=0 );
      أكد( p_اظہاریہ->u.zToken[0]!=0 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Variable, p_اظہاریہ->i_کالم, نشانہ);
      أرجع نشانہ;
    }
    حالة TK_رجسٹر: {
      أرجع p_اظہاریہ->i_جدول;
    }
#جے_بنا_تعریف SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ->p_بایاں, نشانہ);
      أكد( وچ_Reg==نشانہ );
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Cast, نشانہ,
                        sqlite3AffinityType(p_اظہاریہ->u.zToken, 0));
      أرجع وچ_Reg;
    }
#جے_انت /* SQLITE_OMIT_CAST */
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
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      صحيح addrIsNull = 0;
      إذا( sqlite3_اظہاریہ_Is_سمتیہ(p_بایاں) ){
        کوڈ_سمتیہ_Compare(pParse, p_اظہاریہ, نشانہ, op, p5);
      }وإلا{
        إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                     &r1, &r2, &regFree1, &regFree2);
        }وإلا{
          r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
          r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        }
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, وچ_Reg);
        کوڈ_Compare(pParse, p_بایاں, p_اظہاریہ->p_دایاں, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        إذا( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, وچ_Reg);
        }وإلا{
          sqlite3_Vdbe_جوڑو_Op3(v, OP_ZeroOrNull, r1, وچ_Reg, r2);
          إذا( addrIsNull ){
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_ایتھے(v, addrIsNull);
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, وچ_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_AND:
    حالة TK_OR: {
      وچ_Reg = اظہاریہ_کوڈ_نشانہ_And_Or(pParse, p_اظہاریہ, نشانہ, &regFree1);
      اكسر;
    }
    حالة TK_PLUS:
    حالة TK_تارا:
    حالة TK_MINUS:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_BITOR:
    حالة TK_SLASH:
    حالة TK_LSHIFT:
    حالة TK_RSHIFT:
    حالة TK_CONCAT: {
      صحيح addrIsNull;
      أكد( TK_PLUS==OP_جوڑو );           testcase( op==TK_PLUS );
      أكد( TK_MINUS==OP_گھٹاؤ );     testcase( op==TK_MINUS );
      أكد( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      أكد( TK_BITAND==OP_ثنائی_ہندسہ_And );      testcase( op==TK_BITAND );
      أكد( TK_BITOR==OP_ثنائی_ہندسہ_Or );        testcase( op==TK_BITOR );
      أكد( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      أكد( TK_LSHIFT==OP_کھسکاؤ_بایاں );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_کھسکاؤ_دایاں );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_جوڑو_Op3(v, op, r2, r1, نشانہ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      إذا( addrIsNull ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_ایتھے(v, addrIsNull);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
        VdbeComment((v, "short-circut value"));
      }
      اكسر;
    }
    حالة TK_UMINUS: {
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      أكد( p_بایاں );
      إذا( p_بایاں->op==TK_INTEGER ){
        کوڈ_Integer(pParse, p_بایاں, 1, نشانہ);
        أرجع نشانہ;
#جے_بنا_تعریف SQLITE_OMIT_FLOATING_نقطہ
      }وإلا إذا( p_بایاں->op==TK_FLOAT ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        کوڈ_Real(v, p_بایاں->u.zToken, 1, نشانہ);
        أرجع نشانہ;
#جے_انت
      }وإلا{
        عارضی_X.op = TK_INTEGER;
        عارضی_X.flags = EP_Int_قدر|EP_TokenOnly;
        عارضی_X.u.i_قدر = 0;
        اظہاریہ_Clear_VVAProperties(&عارضی_X);
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, &عارضی_X, &regFree1);
        r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree2);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_گھٹاؤ, r2, r1, نشانہ);
        testcase( regFree2==0 );
      }
      اكسر;
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أكد( TK_BITNOT==OP_ثنائی_ہندسہ_Not );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, وچ_Reg);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحيح b_عام;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      b_عام = p_اظہاریہ->op2==TK_IS;
      testcase( isTrue && b_عام);
      testcase( !isTrue && b_عام);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsTrue, r1, وچ_Reg, !isTrue, isTrue ^ b_عام);
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      صحيح addr;
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, نشانہ);
      r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_جوڑو_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, نشانہ);
      sqlite3_Vdbe_Jump_ایتھے(v, addr);
      اكسر;
    }
    حالة TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_اظہاریہ->pAggInfo;
      إذا( pInfo==0
       || NEVER(p_اظہاریہ->iAgg<0)
       || NEVER(p_اظہاریہ->iAgg>=pInfo->nFunc)
      ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        sqlite3_Error_پیغام(pParse, "misuse of aggregate: %#T()", p_اظہاریہ);
      }وإلا{
        أرجع AggInfoFuncReg(pInfo, p_اظہاریہ->iAgg);
      }
      اكسر;
    }
    حالة TK_FUNCTION: {
      اظہاریہ_List *pFarg;       /* List of function arguments */
      صحيح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت حرف *zId;       /* The function name */
      u32 const_نقاب = 0;     /* Mask of function arguments that are constant */
      صحيح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
        أرجع p_اظہاریہ->y.pWin->reg_نتیجہ;
      }
#جے_انت

      إذا( Const_ضارب_Ok(pParse)
       && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        أرجع sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(pParse, p_اظہاریہ, -1);
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly) );
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      pFarg = p_اظہاریہ->x.pList;
      nFarg = pFarg ? pFarg->n_اظہاریہ : 0;
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      zId = p_اظہاریہ->u.zToken;
      pDef = sqlite3_لبھو_Function(db, zId, nFarg, enc, 0);
#جے_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      إذا( pDef==0 && pParse->explain ){
        pDef = sqlite3_لبھو_Function(db, "unknown", nFarg, enc, 0);
      }
#جے_انت
      إذا( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_اندرونی)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_اندرونی_Func)==0)
      ){
        sqlite3_Error_پیغام(pParse, "unknown function: %#T()", p_اظہاریہ);
        اكسر;
      }
      إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_سدھا)==0 );
        أرجع اظہاریہ_کوڈ_Inline_Function(pParse, pFarg,
             SQLITE_PTR_منزل_INT(pDef->p_User_انکڑے), نشانہ);
      }وإلا إذا( pDef->funcFlags & (SQLITE_FUNC_سدھا|SQLITE_FUNC_UNSAFE) ){
        sqlite3_اظہاریہ_Function_Usable(pParse, p_اظہاریہ, pDef);
      }

      لكل(i=0; i<nFarg; i++){
        إذا( i<32 && sqlite3_اظہاریہ_Is_Constant(pParse, pFarg->a[i].p_اظہاریہ) ){
          testcase( i==31 );
          const_نقاب |= MASKBIT32(i);
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, pFarg->a[i].p_اظہاریہ);
        }
      }
      إذا( pFarg ){
        إذا( const_نقاب ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }وإلا{
          r1 = sqlite3_Get_عارضی_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        إذا( (pDef->funcFlags & (SQLITE_FUNC_لمائی|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 اظہاریہ_Op;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].p_اظہاریہ!=0 );
          اظہاریہ_Op = pFarg->a[0].p_اظہاریہ->op;
          إذا( اظہاریہ_Op==TK_کالم || اظہاریہ_Op==TK_AGG_کالم ){
            أكد( SQLITE_FUNC_لمائی==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_اظہاریہ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_اظہاریہ_کوڈ_اظہاریہ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_ضارب);
      }وإلا{
        r1 = 0;
      }
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
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
      إذا( nFarg>=2 && اظہاریہ_Has_Property(p_اظہاریہ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_اظہاریہ);
      }وإلا إذا( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_اظہاریہ);
      }
#جے_انت
      إذا( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        إذا( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_جوڑو_Op4(v, OP_CollSeq, 0, 0, 0, (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جوڑو_Function_Call(pParse, const_نقاب, r1, نشانہ, nFarg,
                                 pDef, p_اظہاریہ->op2);
      إذا( nFarg ){
        إذا( const_نقاب==0 ){
          sqlite3_Release_عارضی_Range(pParse, r1, nFarg);
        }وإلا{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_نقاب, 1);
        }
      }
      أرجع نشانہ;
    }
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT: {
      صحيح n_خانہ;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      إذا( pParse->db->mallocFailed ){
        أرجع 0;
      }وإلا إذا( op==TK_SELECT
             && ALWAYS( اظہاریہ_Use_XSelect(p_اظہاریہ) )
             && (n_خانہ = p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ)!=1
      ){
        sqlite3SubselectError(pParse, n_خانہ, 1);
      }وإلا{
        أرجع sqlite3_کوڈ_Subselect(pParse, p_اظہاریہ);
      }
      اكسر;
    }
    حالة TK_SELECT_کالم: {
      صحيح n;
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      إذا( p_بایاں->i_جدول==0 || pParse->withinRJSubrtn > p_بایاں->op2 ){
        p_بایاں->i_جدول = sqlite3_کوڈ_Subselect(pParse, p_بایاں);
        p_بایاں->op2 = pParse->withinRJSubrtn;
      }
      أكد( p_بایاں->op==TK_SELECT || p_بایاں->op==TK_ERROR );
      n = sqlite3_اظہاریہ_سمتیہ_ناپ(p_بایاں);
      إذا( p_اظہاریہ->i_جدول!=n ){
        sqlite3_Error_پیغام(pParse, "%d columns assigned %d values",
                                p_اظہاریہ->i_جدول, n);
      }
      أرجع p_بایاں->i_جدول + p_اظہاریہ->i_کالم;
    }
    حالة TK_وچ: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
      sqlite3_اظہاریہ_کوڈ_وچ(pParse, p_اظہاریہ, destIfFalse, destIfNull);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, نشانہ);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, نشانہ, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      أرجع نشانہ;
    }
#جے_انت /* SQLITE_OMIT_SUBQUERY */


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
      اظہاریہ_کوڈ_Between(pParse, p_اظہاریہ, نشانہ, 0, 0);
      أرجع نشانہ;
    }
    حالة TK_COLLATE: {
      إذا( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( p_اظہاریہ->p_بایاں );
        sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ->p_بایاں, نشانہ);
        sqlite3_Vdbe_جوڑو_Op1(v, OP_ClrSubtype, نشانہ);
        أرجع نشانہ;
      }وإلا{
        p_اظہاریہ = p_اظہاریہ->p_بایاں;
        اذهب اظہاریہ_کوڈ_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالة TK_وسعت:
    حالة TK_UPLUS: {
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
      اذهب اظہاریہ_کوڈ_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
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
      صحيح i_خانہ;
      صحيح p1;

      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      pTab = p_اظہاریہ->y.pTab;
      i_خانہ = p_اظہاریہ->i_کالم;
      p1 = p_اظہاریہ->i_جدول * (pTab->n_خانہ+1) + 1
                     + sqlite3_جدول_کالم_منزل_Storage(pTab, i_خانہ);

      أكد( p_اظہاریہ->i_جدول==0 || p_اظہاریہ->i_جدول==1 );
      أكد( i_خانہ>=-1 && i_خانہ<pTab->n_خانہ );
      أكد( pTab->iPKey<0 || i_خانہ!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->n_خانہ*2+2) );

      sqlite3_Vdbe_جوڑو_Op2(v, OP_Param, p1, نشانہ);
      VdbeComment((v, "r[%d]=%s.%s", نشانہ,
        (p_اظہاریہ->i_جدول ? "new" : "old"),
        (p_اظہاریہ->i_کالم<0 ? "rowid" : p_اظہاریہ->y.pTab->a_خانہ[i_خانہ].zCnName)
      ));

#جے_بنا_تعریف SQLITE_OMIT_FLOATING_نقطہ
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      إذا( i_خانہ>=0 && pTab->a_خانہ[i_خانہ].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, نشانہ);
      }
#جے_انت
      اكسر;
    }

    حالة TK_سمتیہ: {
      sqlite3_Error_پیغام(pParse, "row value misused");
      اكسر;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    حالة TK_IF_NULL_صف: {
      صحيح addrINR;
      u8 ok_Const_ضارب = pParse->ok_Const_ضارب;
      AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
      إذا( pAggInfo ){
        أكد( p_اظہاریہ->iAgg>=0 && p_اظہاریہ->iAgg<pAggInfo->n_کالم );
        إذا( !pAggInfo->سدھا_منوال ){
          وچ_Reg = Agg_Info_کالم_Reg(pAggInfo, p_اظہاریہ->iAgg);
          اكسر;
        }
        إذا( p_اظہاریہ->pAggInfo->use_چھانٹی_Idx ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_کالم, pAggInfo->چھانٹی_Idx_PTab,
                            pAggInfo->a_خانہ[p_اظہاریہ->iAgg].i_Sorter_کالم,
                            نشانہ);
          وچ_Reg = نشانہ;
          اكسر;
        }
      }
      addrINR = sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, p_اظہاریہ->i_جدول, 0, نشانہ);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_ضارب = 0;   /* note (1) above */
      sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ->p_بایاں, نشانہ);
      أكد( نشانہ==وچ_Reg );
      pParse->ok_Const_ضارب = ok_Const_ضارب;
      sqlite3_Vdbe_Jump_ایتھے(v, addrINR);
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
      صحيح سرا_Label;                     /* GOTO label for end of CASE stmt */
      صحيح اگلا_Case;                     /* GOTO label for next WHEN clause */
      صحيح n_اظہاریہ;                        /* 2x number of WHEN terms */
      صحيح i;                            /* Loop counter */
      اظہاریہ_List *pEList;                 /* List of WHEN terms */
      بنية اظہاریہ_List_شے *aListelem;  /* Array of WHEN terms */
      اظہاریہ opCompare;                   /* The X==Ei expression */
      اظہاریہ *pX;                         /* The X expression */
      اظہاریہ *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      اظہاریہ *pDel = 0;
      sqlite3 *db = pParse->db;

      أكد( اظہاریہ_Use_XList(p_اظہاریہ) && p_اظہاریہ->x.pList!=0 );
      أكد(p_اظہاریہ->x.pList->n_اظہاریہ > 0);
      pEList = p_اظہاریہ->x.pList;
      aListelem = pEList->a;
      n_اظہاریہ = pEList->n_اظہاریہ;
      سرا_Label = sqlite3VdbeMakeLabel(pParse);
      إذا( (pX = p_اظہاریہ->p_بایاں)!=0 ){
        pDel = sqlite3_اظہاریہ_Dup(db, pX, 0);
        إذا( db->mallocFailed ){
          sqlite3_اظہاریہ_Delete(db, pDel);
          اكسر;
        }
        testcase( pX->op==TK_کالم );
        sqlite3_اظہاریہ_منزل_رجسٹر(pDel, اظہاریہ_کوڈ_سمتیہ(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, حجم(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_بایاں = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      لكل(i=0; i<n_اظہاریہ-1; i=i+2){
        إذا( pX ){
          أكد( pTest!=0 );
          opCompare.p_دایاں = aListelem[i].p_اظہاریہ;
        }وإلا{
          pTest = aListelem[i].p_اظہاریہ;
        }
        اگلا_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_کالم );
        sqlite3_اظہاریہ_If_False(pParse, pTest, اگلا_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_اظہاریہ->op==TK_کالم );
        sqlite3_اظہاریہ_کوڈ(pParse, aListelem[i+1].p_اظہاریہ, نشانہ);
        sqlite3VdbeGoto(v, سرا_Label);
        sqlite3VdbeResolveLabel(v, اگلا_Case);
      }
      إذا( (n_اظہاریہ&1)!=0 ){
        sqlite3_اظہاریہ_کوڈ(pParse, pEList->a[n_اظہاریہ-1].p_اظہاریہ, نشانہ);
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, نشانہ);
      }
      sqlite3_اظہاریہ_Delete(db, pDel);
      سیٹ_Do_Not_ملاؤ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, سرا_Label);
      اكسر;
    }
#جے_بنا_تعریف SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      أكد( p_اظہاریہ->aff_اظہاریہ==OE_Rollback
           || p_اظہاریہ->aff_اظہاریہ==OE_Abort
           || p_اظہاریہ->aff_اظہاریہ==OE_Fail
           || p_اظہاریہ->aff_اظہاریہ==OE_Ignore
      );
      إذا( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_پیغام(pParse,
                       "RAISE() may only be used within a trigger-program");
        أرجع 0;
      }
      إذا( p_اظہاریہ->aff_اظہاریہ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      إذا( p_اظہاریہ->aff_اظہاریہ==OE_Ignore ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_اظہاریہ->aff_اظہاریہ, r1);
      }
      اكسر;
    }
#جے_انت
  }
  sqlite3_Release_عارضی_Reg(pParse, regFree1);
  sqlite3_Release_عارضی_Reg(pParse, regFree2);
  أرجع وچ_Reg;
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
صحيح sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(
  Parse *pParse,    /* Parsing context */
  اظہاریہ *p_اظہاریہ,      /* The expression to code when the VDBE initializes */
  صحيح regDest       /* Store the value in this register */
){
  اظہاریہ_List *p;
  أكد( Const_ضارب_Ok(pParse) );
  أكد( regDest!=0 );
  p = pParse->p_Const_اظہاریہ;
  إذا( regDest<0 && p ){
    بنية اظہاریہ_List_شے *p_شے;
    صحيح i;
    لكل(p_شے=p->a, i=p->n_اظہاریہ; i>0; p_شے++, i--){
      إذا( p_شے->fg.reusable
       && sqlite3_اظہاریہ_Compare(0,p_شے->p_اظہاریہ,p_اظہاریہ,-1)==0
      ){
        أرجع p_شے->u.i_Const_اظہاریہ_Reg;
      }
    }
  }
  p_اظہاریہ = sqlite3_اظہاریہ_Dup(pParse->db, p_اظہاریہ, 0);
  إذا( p_اظہاریہ!=0 && اظہاریہ_Has_Property(p_اظہاریہ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحيح addr;
    أكد( v );
    addr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_ضارب = 0;
    إذا( !pParse->db->mallocFailed ){
      إذا( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ, regDest);
    }
    pParse->ok_Const_ضارب = 1;
    sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
    sqlite3_Vdbe_Jump_ایتھے(v, addr);
  }وإلا{
    p = sqlite3_اظہاریہ_List_Append(pParse, p, p_اظہاریہ);
    إذا( p ){
       بنية اظہاریہ_List_شے *p_شے = &p->a[p->n_اظہاریہ-1];
       p_شے->fg.reusable = regDest<0;
       إذا( regDest<0 ) regDest = ++pParse->nMem;
       p_شے->u.i_Const_اظہاریہ_Reg = regDest;
    }
    pParse->p_Const_اظہاریہ = p;
  }
  أرجع regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE فراغ sqlite3_اظہاریہ_Null_رجسٹر_Range(
  Parse *pParse,   /* Parsing context */
  صحيح iReg,        /* First register to set to NULL */
  صحيح nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_ضارب = pParse->ok_Const_ضارب;
  اظہاریہ t;
  memset(&t, 0, حجم(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_ضارب = 1;
  sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_ضارب = ok_Const_ضارب;
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
صحيح sqlite3_اظہاریہ_کوڈ_عارضی(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح *pReg){
  صحيح r2;
  p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  إذا( Const_ضارب_Ok(pParse)
   && ALWAYS(p_اظہاریہ!=0)
   && p_اظہاریہ->op!=TK_رجسٹر
   && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse, p_اظہاریہ)
  ){
    *pReg  = 0;
    r2 = sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(pParse, p_اظہاریہ, -1);
  }وإلا{
    صحيح r1 = sqlite3_Get_عارضی_Reg(pParse);
    r2 = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ, r1);
    إذا( r2==r1 ){
      *pReg = r1;
    }وإلا{
      sqlite3_Release_عارضی_Reg(pParse, r1);
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
فراغ sqlite3_اظہاریہ_کوڈ(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح نشانہ){
  صحيح وچ_Reg;

  أكد( p_اظہاریہ==0 || !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  أكد( نشانہ>0 && نشانہ<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  إذا( pParse->pVdbe==0 ) أرجع;
  وچ_Reg = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ, نشانہ);
  إذا( وچ_Reg!=نشانہ ){
    u8 op;
    اظہاریہ *pX = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
    testcase( pX!=p_اظہاریہ );
    إذا( ALWAYS(pX)
     && (اظہاریہ_Has_Property(pX,EP_Subquery) || pX->op==TK_رجسٹر)
    ){
      op = OP_Copy;
    }وإلا{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_جوڑو_Op2(pParse->pVdbe, op, وچ_Reg, نشانہ);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
فراغ sqlite3_اظہاریہ_کوڈ_Copy(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح نشانہ){
  sqlite3 *db = pParse->db;
  p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
  إذا( !db->mallocFailed ) sqlite3_اظہاریہ_کوڈ(pParse, p_اظہاریہ, نشانہ);
  sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
فراغ sqlite3_اظہاریہ_کوڈ_Factorable(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح نشانہ){
  إذا( pParse->ok_Const_ضارب && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ) ){
    sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(pParse, p_اظہاریہ, نشانہ);
  }وإلا{
    sqlite3_اظہاریہ_کوڈ_Copy(pParse, p_اظہاریہ, نشانہ);
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
صحيح sqlite3_اظہاریہ_کوڈ_اظہاریہ_List(
  Parse *pParse,     /* Parsing context */
  اظہاریہ_List *pList,   /* The expression list to be coded */
  صحيح نشانہ,        /* Where to write results */
  صحيح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  بنية اظہاریہ_List_شے *p_شے;
  صحيح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( نشانہ>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_اظہاریہ;
  إذا( !Const_ضارب_Ok(pParse) ) flags &= ~SQLITE_ECEL_ضارب;
  لكل(p_شے=pList->a, i=0; i<n; i++, p_شے++){
    اظہاریہ *p_اظہاریہ = p_شے->p_اظہاریہ;
#جے_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    إذا( p_شے->fg.bSorterRef ){
      i--;
      n--;
    }وإلا
#جے_انت
    إذا( (flags & SQLITE_ECEL_REF)!=0 && (j = p_شے->u.x.i_ترتیب_By_خانہ)>0 ){
      إذا( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }وإلا{
        sqlite3_Vdbe_جوڑو_Op2(v, copyOp, j+srcReg-1, نشانہ+i);
      }
    }وإلا إذا( (flags & SQLITE_ECEL_ضارب)!=0
           && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ)
    ){
      sqlite3_اظہاریہ_کوڈ_لگاتار_Just_Once(pParse, p_اظہاریہ, نشانہ+i);
    }وإلا{
      صحيح وچ_Reg = sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, p_اظہاریہ, نشانہ+i);
      إذا( وچ_Reg!=نشانہ+i ){
        VdbeOp *pOp;
        إذا( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_آخری_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==وچ_Reg
         && pOp->p2+pOp->p3+1==نشانہ+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }وإلا{
          sqlite3_Vdbe_جوڑو_Op2(v, copyOp, وچ_Reg, نشانہ+i);
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
ساكن فراغ اظہاریہ_کوڈ_Between(
  Parse *pParse,    /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,      /* The BETWEEN expression */
  صحيح dest,         /* Jump destination or storage location */
  فراغ (*xJump)(Parse*,اظہاریہ*,صحيح,صحيح), /* Action to take */
  صحيح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  اظہاریہ اظہاریہ_And;     /* The AND operator in  x>=y AND x<=z  */
  اظہاریہ comp_بایاں;    /* The  x>=y  term */
  اظہاریہ comp_دایاں;   /* The  x<=z  term */
  صحيح regFree1 = 0; /* Temporary use register */
  اظہاریہ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_بایاں, 0, حجم(اظہاریہ));
  memset(&comp_دایاں, 0, حجم(اظہاریہ));
  memset(&اظہاریہ_And, 0, حجم(اظہاریہ));

  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  pDel = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ->p_بایاں, 0);
  إذا( db->mallocFailed==0 ){
    اظہاریہ_And.op = TK_AND;
    اظہاریہ_And.p_بایاں = &comp_بایاں;
    اظہاریہ_And.p_دایاں = &comp_دایاں;
    comp_بایاں.op = TK_GE;
    comp_بایاں.p_بایاں = pDel;
    comp_بایاں.p_دایاں = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
    comp_دایاں.op = TK_LE;
    comp_دایاں.p_بایاں = pDel;
    comp_دایاں.p_دایاں = p_اظہاریہ->x.pList->a[1].p_اظہاریہ;
    sqlite3_اظہاریہ_منزل_رجسٹر(pDel, اظہاریہ_کوڈ_سمتیہ(pParse, pDel, &regFree1));
    إذا( xJump ){
      xJump(pParse, &اظہاریہ_And, dest, jumpIfNull);
    }وإلا{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_اظہاریہ_کوڈ_نشانہ(pParse, &اظہاریہ_And, dest);
    }
    sqlite3_Release_عارضی_Reg(pParse, regFree1);
  }
  sqlite3_اظہاریہ_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_اظہاریہ_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_اظہاریہ_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_اظہاریہ_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_اظہاریہ_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_اظہاریہ_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_اظہاریہ_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_اظہاریہ_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_اظہاریہ_If_False && jumpIfNull!=0 && regFree1!=0 );
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
فراغ sqlite3_اظہاریہ_If_True(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) )     أرجع;  /* Existence of VDBE checked by caller */
  إذا( NEVER(p_اظہاریہ==0) ) أرجع;  /* No way this can happen */
  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ, EP_Immutable) );
  op = p_اظہاریہ->op;
  بدل( op ){
    حالة TK_AND:
    حالة TK_OR: {
      اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
      إذا( pAlt!=p_اظہاریہ ){
        sqlite3_اظہاریہ_If_True(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        اظہاریہ *p_پہلا, *p_دوجا;
        إذا( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
          p_پہلا = p_اظہاریہ->p_دایاں;
          p_دوجا = p_اظہاریہ->p_بایاں;
        }وإلا{
          p_پہلا = p_اظہاریہ->p_بایاں;
          p_دوجا = p_اظہاریہ->p_دایاں;
        }
        إذا( op==TK_AND ){
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_False(pParse, p_پہلا, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اظہاریہ_If_True(pParse, p_دوجا, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }وإلا{
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_True(pParse, p_پہلا, dest, jumpIfNull);
          sqlite3_اظہاریہ_If_True(pParse, p_دوجا, dest, jumpIfNull);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اظہاریہ->op2==TK_ISNOT;
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }وإلا{
        sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest,
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
      إذا( sqlite3_اظہاریہ_Is_سمتیہ(p_اظہاریہ->p_بایاں) ) اذهب default_اظہاریہ;
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      کوڈ_Compare(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں, op,
                  r1, r2, dest, jumpIfNull, اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted));
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
          sqlite3_Vdbe_Jump_ایتھے(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3_Vdbe_Typeof_کالم(v, r1);
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اظہاریہ_کوڈ_Between(pParse, p_اظہاریہ, dest, sqlite3_اظہاریہ_If_True, jumpIfNull);
      اكسر;
    }
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
    حالة TK_وچ: {
      صحيح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحيح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_اظہاریہ_کوڈ_وچ(pParse, p_اظہاریہ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      اكسر;
    }
#جے_انت
    افتراضي: {
    default_اظہاریہ:
      إذا( اظہاریہ_Always_True(p_اظہاریہ) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( اظہاریہ_Always_False(p_اظہاریہ) ){
        /* No-op */
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3_Release_عارضی_Reg(pParse, regFree1);
  sqlite3_Release_عارضی_Reg(pParse, regFree2); 
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
فراغ sqlite3_اظہاریہ_If_False(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح dest, صحيح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحيح op = 0;
  صحيح regFree1 = 0;
  صحيح regFree2 = 0;
  صحيح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  إذا( NEVER(v==0) ) أرجع; /* Existence of VDBE checked by caller */
  إذا( p_اظہاریہ==0 )    أرجع;
  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );

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
  op = ((p_اظہاریہ->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  أكد( p_اظہاریہ->op!=TK_ISNULL || op==OP_NotNull );
  أكد( p_اظہاریہ->op!=TK_NOTNULL || op==OP_IsNull );
  أكد( p_اظہاریہ->op!=TK_NE || op==OP_Eq );
  أكد( p_اظہاریہ->op!=TK_EQ || op==OP_Ne );
  أكد( p_اظہاریہ->op!=TK_LT || op==OP_Ge );
  أكد( p_اظہاریہ->op!=TK_LE || op==OP_Gt );
  أكد( p_اظہاریہ->op!=TK_GT || op==OP_Le );
  أكد( p_اظہاریہ->op!=TK_GE || op==OP_Lt );

  بدل( p_اظہاریہ->op ){
    حالة TK_AND:
    حالة TK_OR: {
      اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
      إذا( pAlt!=p_اظہاریہ ){
        sqlite3_اظہاریہ_If_False(pParse, pAlt, dest, jumpIfNull);
      }وإلا{
        اظہاریہ *p_پہلا, *p_دوجا;
        إذا( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
          p_پہلا = p_اظہاریہ->p_دایاں;
          p_دوجا = p_اظہاریہ->p_بایاں;
        }وإلا{
          p_پہلا = p_اظہاریہ->p_بایاں;
          p_دوجا = p_اظہاریہ->p_دایاں;
        }
        إذا( p_اظہاریہ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_False(pParse, p_پہلا, dest, jumpIfNull);
          sqlite3_اظہاریہ_If_False(pParse, p_دوجا, dest, jumpIfNull);
        }وإلا{
          صحيح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_True(pParse, p_پہلا, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اظہاریہ_If_False(pParse, p_دوجا, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      اكسر;
    }
    حالة TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest, jumpIfNull);
      اكسر;
    }
    حالة TK_TRUTH: {
      صحيح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحيح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اظہاریہ->op2==TK_ISNOT;
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      إذا( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }وإلا{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT:
      testcase( p_اظہاریہ->op==TK_IS );
      testcase( p_اظہاریہ->op==TK_ISNOT );
      op = (p_اظہاریہ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE:
    حالة TK_NE:
    حالة TK_EQ: {
      صحيح addrIsNull;
      إذا( sqlite3_اظہاریہ_Is_سمتیہ(p_اظہاریہ->p_بایاں) ) اذهب default_اظہاریہ;
      إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      کوڈ_Compare(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں, op,
                  r1, r2, dest, jumpIfNull,اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted));
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
          sqlite3_Vdbe_Jump_ایتھے(v, addrIsNull);
        }
      }
      اكسر;
    }
    حالة TK_ISNULL:
    حالة TK_NOTNULL: {
      r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      إذا( regFree1 ) sqlite3_Vdbe_Typeof_کالم(v, r1);
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      اكسر;
    }
    حالة TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اظہاریہ_کوڈ_Between(pParse, p_اظہاریہ, dest, sqlite3_اظہاریہ_If_False, jumpIfNull);
      اكسر;
    }
#جے_بنا_تعریف SQLITE_OMIT_SUBQUERY
    حالة TK_وچ: {
      إذا( jumpIfNull ){
        sqlite3_اظہاریہ_کوڈ_وچ(pParse, p_اظہاریہ, dest, dest);
      }وإلا{
        صحيح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_اظہاریہ_کوڈ_وچ(pParse, p_اظہاریہ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      اكسر;
    }
#جے_انت
    افتراضي: {
    default_اظہاریہ:
      إذا( اظہاریہ_Always_False(p_اظہاریہ) ){
        sqlite3VdbeGoto(v, dest);
      }وإلا إذا( اظہاریہ_Always_True(p_اظہاریہ) ){
        /* no-op */
      }وإلا{
        r1 = sqlite3_اظہاریہ_کوڈ_عارضی(pParse, p_اظہاریہ, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      اكسر;
    }
  }
  sqlite3_Release_عارضی_Reg(pParse, regFree1);
  sqlite3_Release_عارضی_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
فراغ sqlite3_اظہاریہ_If_False_Dup(Parse *pParse, اظہاریہ *p_اظہاریہ, صحيح dest,صحيح jumpIfNull){
  sqlite3 *db = pParse->db;
  اظہاریہ *pCopy = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
  إذا( db->mallocFailed==0 ){
    sqlite3_اظہاریہ_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_اظہاریہ_Delete(db, pCopy);
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
ساكن SQLITE_NOINLINE صحيح اظہاریہ_Compare_Variable(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pVar,
  ثابت اظہاریہ *p_اظہاریہ
){
  صحيح res = 2;
  صحيح iVar;
  sqlite3_قدر *pL, *pR = 0;
 
  إذا( p_اظہاریہ->op==TK_VARIABLE && pVar->i_کالم==p_اظہاریہ->i_کالم ){
    أرجع 0;
  }
  إذا( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) أرجع 2;
  sqlite3_قدر_From_اظہاریہ(pParse->db, p_اظہاریہ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  إذا( pR ){
    iVar = pVar->i_کالم;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_قدر(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    إذا( pL ){
      إذا( sqlite3_قدر_type(pL)==SQLITE_متن ){
        sqlite3_قدر_متن(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_قدر_Free(pR);
    sqlite3_قدر_Free(pL);
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
صحيح sqlite3_اظہاریہ_Compare(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pA,
  ثابت اظہاریہ *pB,
  صحيح iTab
){
  u32 combinedFlags;
  إذا( pA==0 || pB==0 ){
    أرجع pB==pA ? 0 : 2;
  }
  إذا( pParse && pA->op==TK_VARIABLE ){
    أرجع اظہاریہ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  إذا( combinedFlags & EP_Int_قدر ){
    إذا( (pA->flags&pB->flags&EP_Int_قدر)!=0 && pA->u.i_قدر==pB->u.i_قدر ){
      أرجع 0;
    }
    أرجع 2;
  }
  إذا( pA->op!=pB->op || pA->op==TK_RAISE ){
    إذا( pA->op==TK_COLLATE && sqlite3_اظہاریہ_Compare(pParse, pA->p_بایاں,pB,iTab)<2 ){
      أرجع 1;
    }
    إذا( pB->op==TK_COLLATE && sqlite3_اظہاریہ_Compare(pParse, pA,pB->p_بایاں,iTab)<2 ){
      أرجع 1;
    }
    إذا( pA->op==TK_AGG_کالم && pB->op==TK_کالم
     && pB->i_جدول<0 && pA->i_جدول==iTab
    ){
      /* fall through */
    }وإلا{
      أرجع 2;
    }
  }
  أكد( !اظہاریہ_Has_Property(pA, EP_Int_قدر) );
  أكد( !اظہاریہ_Has_Property(pB, EP_Int_قدر) );
  إذا( pA->u.zToken ){
    إذا( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      إذا( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      إذا( اظہاریہ_Has_Property(pA,EP_WinFunc)!=اظہاریہ_Has_Property(pB,EP_WinFunc) ){
        أرجع 2;
      }
      إذا( اظہاریہ_Has_Property(pA,EP_WinFunc) ){
        إذا( sqlite3_کھڑکی_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          أرجع 2;
        }
      }
#جے_انت
    }وإلا إذا( pA->op==TK_NULL ){
      أرجع 0;
    }وإلا إذا( pA->op==TK_COLLATE ){
      إذا( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) أرجع 2;
    }وإلا
    إذا( pB->u.zToken!=0
     && pA->op!=TK_کالم
     && pA->op!=TK_AGG_کالم
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      أرجع 2;
    }
  }
  إذا( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) أرجع 2;
  إذا( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    إذا( combinedFlags & EP_xIsSelect ) أرجع 2;
    إذا( (combinedFlags & EP_Fixed_خانہ)==0
     && sqlite3_اظہاریہ_Compare(pParse, pA->p_بایاں, pB->p_بایاں, iTab) ) أرجع 2;
    إذا( sqlite3_اظہاریہ_Compare(pParse, pA->p_دایاں, pB->p_دایاں, iTab) ) أرجع 2;
    إذا( sqlite3_اظہاریہ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) أرجع 2;
    إذا( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      إذا( pA->i_کالم!=pB->i_کالم ) أرجع 2;
      إذا( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) أرجع 2;
      إذا( pA->op!=TK_وچ && pA->i_جدول!=pB->i_جدول && pA->i_جدول!=iTab ){
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
صحيح sqlite3_اظہاریہ_List_Compare(ثابت اظہاریہ_List *pA, ثابت اظہاریہ_List *pB, صحيح iTab){
  صحيح i;
  إذا( pA==0 && pB==0 ) أرجع 0;
  إذا( pA==0 || pB==0 ) أرجع 1;
  إذا( pA->n_اظہاریہ!=pB->n_اظہاریہ ) أرجع 1;
  لكل(i=0; i<pA->n_اظہاریہ; i++){
    صحيح res;
    اظہاریہ *p_اظہاریہ_A = pA->a[i].p_اظہاریہ;
    اظہاریہ *p_اظہاریہ_B = pB->a[i].p_اظہاریہ;
    إذا( pA->a[i].fg.ترتیب_دیو_Flags!=pB->a[i].fg.ترتیب_دیو_Flags ) أرجع 1;
    إذا( (res = sqlite3_اظہاریہ_Compare(0, p_اظہاریہ_A, p_اظہاریہ_B, iTab)) ) أرجع res;
  }
  أرجع 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحيح sqlite3_اظہاریہ_Compare_Skip(اظہاریہ *pA,اظہاریہ *pB, صحيح iTab){
  أرجع sqlite3_اظہاریہ_Compare(0,
             sqlite3_اظہاریہ_Skip_Collate(pA),
             sqlite3_اظہاریہ_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ساكن صحيح اظہاریہ_سٹا_دیندا_Not_Null(
  ثابت Parse *pParse,/* Parsing context */
  ثابت اظہاریہ *p,      /* The expression to be checked */
  ثابت اظہاریہ *pNN,    /* The expression that is NOT NULL */
  صحيح iTab,           /* Table being evaluated */
  صحيح تکیا_Not         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  إذا( sqlite3_اظہاریہ_Compare(pParse, p, pNN, iTab)==0 ){
    أرجع pNN->op!=TK_NULL;
  }
  بدل( p->op ){
    حالة TK_وچ: {
      إذا( تکیا_Not && اظہاریہ_Has_Property(p, EP_xIsSelect) ) أرجع 0;
      أكد( اظہاریہ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_اظہاریہ>0) );
      أرجع اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
    حالة TK_BETWEEN: {
      اظہاریہ_List *pList;
      أكد( اظہاریہ_Use_XList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->n_اظہاریہ==2 );
      إذا( تکیا_Not ) أرجع 0;
      إذا( اظہاریہ_سٹا_دیندا_Not_Null(pParse, pList->a[0].p_اظہاریہ, pNN, iTab, 1)
       || اظہاریہ_سٹا_دیندا_Not_Null(pParse, pList->a[1].p_اظہاریہ, pNN, iTab, 1)
      ){
        أرجع 1;
      }
      أرجع اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
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
      تکیا_Not = 1;
      /* no break */ deliberate_fall_through
    حالة TK_تارا:
    حالة TK_REM:
    حالة TK_BITAND:
    حالة TK_SLASH: {
      إذا( اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_دایاں, pNN, iTab, تکیا_Not) ) أرجع 1;
      /* no break */ deliberate_fall_through
    }
    حالة TK_وسعت:
    حالة TK_COLLATE:
    حالة TK_UPLUS:
    حالة TK_UMINUS: {
      أرجع اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_بایاں, pNN, iTab, تکیا_Not);
    }
    حالة TK_TRUTH: {
      إذا( تکیا_Not ) أرجع 0;
      إذا( p->op2!=TK_IS ) أرجع 0;
      أرجع اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
    حالة TK_BITNOT:
    حالة TK_NOT: {
      أرجع اظہاریہ_سٹا_دیندا_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
  }
  أرجع 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ساكن صحيح sqlite3_اظہاریہ_Is_Not_True(اظہاریہ *p_اظہاریہ){
  صحيح v;
  إذا( p_اظہاریہ->op==TK_NULL ) أرجع 1;
  إذا( p_اظہاریہ->op==TK_TRUEFALSE && sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ)==0 ) أرجع 1;
  v = 1;
  إذا( sqlite3_اظہاریہ_Is_Integer(p_اظہاریہ, &v, 0, 0) && v==0 ) أرجع 1;
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
ساكن صحيح sqlite3_اظہاریہ_Is_IIF(sqlite3 *db, ثابت اظہاریہ *p_اظہاریہ){
  اظہاریہ_List *pList;
  إذا( p_اظہاریہ->op==TK_FUNCTION ){
    ثابت حرف *z = p_اظہاریہ->u.zToken;
    FuncDef *pDef;
    إذا( (z[0]!='i' && z[0]!='I') ) أرجع 0;
    إذا( p_اظہاریہ->x.pList==0 ) أرجع 0;
    pDef = sqlite3_لبھو_Function(db, z, p_اظہاریہ->x.pList->n_اظہاریہ, ENC(db), 0);
#جے_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    إذا( pDef==0 ) أرجع 0;
#وإلا
    إذا( NEVER(pDef==0) ) أرجع 0;
#جے_انت
    إذا( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) أرجع 0;
    إذا( SQLITE_PTR_منزل_INT(pDef->p_User_انکڑے)!=INLINEFUNC_iif ) أرجع 0;
  }وإلا إذا( p_اظہاریہ->op==TK_CASE ){
    إذا( p_اظہاریہ->p_بایاں!=0 ) أرجع 0;
  }وإلا{
    أرجع 0;
  }
  pList = p_اظہاریہ->x.pList;
  أكد( pList!=0 );
  إذا( pList->n_اظہاریہ==2 ) أرجع 1;
  إذا( pList->n_اظہاریہ==3 && sqlite3_اظہاریہ_Is_Not_True(pList->a[2].p_اظہاریہ) ) أرجع 1;
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
صحيح sqlite3_اظہاریہ_سٹا_دیندا_اظہاریہ(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pE1,
  ثابت اظہاریہ *pE2,
  صحيح iTab
){
  إذا( sqlite3_اظہاریہ_Compare(pParse, pE1, pE2, iTab)==0 ){
    أرجع 1;
  }
  إذا( pE2->op==TK_OR
   && (sqlite3_اظہاریہ_سٹا_دیندا_اظہاریہ(pParse, pE1, pE2->p_بایاں, iTab)
             || sqlite3_اظہاریہ_سٹا_دیندا_اظہاریہ(pParse, pE1, pE2->p_دایاں, iTab) )
  ){
    أرجع 1;
  }
  إذا( pE2->op==TK_NOTNULL
   && اظہاریہ_سٹا_دیندا_Not_Null(pParse, pE1, pE2->p_بایاں, iTab, 0)
  ){
    أرجع 1;
  }
  إذا( sqlite3_اظہاریہ_Is_IIF(pParse->db, pE1) ){
    أرجع sqlite3_اظہاریہ_سٹا_دیندا_اظہاریہ(pParse,pE1->x.pList->a[0].p_اظہاریہ,pE2,iTab);
  }
  أرجع 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ساكن فراغ both_Imply_Not_Null_صف(Walker *pWalker, اظہاریہ *pE1, اظہاریہ *pE2){
  إذا( pWalker->e_کوڈ==0 ){
    sqlite3_Walk_اظہاریہ(pWalker, pE1);
    إذا( pWalker->e_کوڈ ){
      pWalker->e_کوڈ = 0;
      sqlite3_Walk_اظہاریہ(pWalker, pE2);
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
ساكن صحيح سٹا_دیندا_Not_Null_صف(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  testcase( p_اظہاریہ->op==TK_AGG_کالم );
  testcase( p_اظہاریہ->op==TK_AGG_FUNCTION );
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) أرجع WRC_Prune;
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    أرجع WRC_Prune;
  }
  بدل( p_اظہاریہ->op ){
    حالة TK_ISNOT:
    حالة TK_ISNULL:
    حالة TK_NOTNULL:
    حالة TK_IS:
    حالة TK_سمتیہ:
    حالة TK_FUNCTION:
    حالة TK_TRUTH:
    حالة TK_CASE:
      testcase( p_اظہاریہ->op==TK_ISNOT );
      testcase( p_اظہاریہ->op==TK_ISNULL );
      testcase( p_اظہاریہ->op==TK_NOTNULL );
      testcase( p_اظہاریہ->op==TK_IS );
      testcase( p_اظہاریہ->op==TK_سمتیہ );
      testcase( p_اظہاریہ->op==TK_FUNCTION );
      testcase( p_اظہاریہ->op==TK_TRUTH );
      testcase( p_اظہاریہ->op==TK_CASE );
      أرجع WRC_Prune;

    حالة TK_کالم:
      إذا( pWalker->u.iCur==p_اظہاریہ->i_جدول ){
        pWalker->e_کوڈ = 1;
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
      testcase( p_اظہاریہ->op==TK_OR );
      testcase( p_اظہاریہ->op==TK_AND );
      both_Imply_Not_Null_صف(pWalker, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں);
      أرجع WRC_Prune;
       
    حالة TK_وچ:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      إذا( اظہاریہ_Use_XList(p_اظہاریہ) && ALWAYS(p_اظہاریہ->x.pList->n_اظہاریہ>0) ){
        sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->p_بایاں);
      }
      أرجع WRC_Prune;

    حالة TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pList->n_اظہاریہ==2 );
      sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->p_بایاں);
      both_Imply_Not_Null_صف(pWalker, p_اظہاریہ->x.pList->a[0].p_اظہاریہ,
                                   p_اظہاریہ->x.pList->a[1].p_اظہاریہ);
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
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      اظہاریہ *p_دایاں = p_اظہاریہ->p_دایاں;
      testcase( p_اظہاریہ->op==TK_EQ );
      testcase( p_اظہاریہ->op==TK_NE );
      testcase( p_اظہاریہ->op==TK_LT );
      testcase( p_اظہاریہ->op==TK_LE );
      testcase( p_اظہاریہ->op==TK_GT );
      testcase( p_اظہاریہ->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      أكد( p_بایاں->op!=TK_کالم || اظہاریہ_Use_YTab(p_بایاں) );
      أكد( p_دایاں->op!=TK_کالم || اظہاریہ_Use_YTab(p_دایاں) );
      إذا( (p_بایاں->op==TK_کالم
           && ALWAYS(p_بایاں->y.pTab!=0)
           && IsVirtual(p_بایاں->y.pTab))
       || (p_دایاں->op==TK_کالم
           && ALWAYS(p_دایاں->y.pTab!=0)
           && IsVirtual(p_دایاں->y.pTab))
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
صحيح sqlite3_اظہاریہ_سٹا_دیندا_Non_Null_صف(اظہاریہ *p, صحيح iTab, صحيح isRJ){
  Walker w;
  p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p);
  إذا( p==0 ) أرجع 0;
  إذا( p->op==TK_NOTNULL ){
    p = p->p_بایاں;
  }وإلا{
    طالما( p->op==TK_AND ){
      إذا( sqlite3_اظہاریہ_سٹا_دیندا_Non_Null_صف(p->p_بایاں, iTab, isRJ) ) أرجع 1;
      p = p->p_دایاں;
    }
  }
  w.x_اظہاریہ_Callback = سٹا_دیندا_Not_Null_صف;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_کوڈ = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_اظہاریہ(&w, p);
  أرجع w.e_کوڈ;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
بنية IdxCover {
  اشاریہ *pIdx;     /* The index to be tested for coverage */
  صحيح iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ساكن صحيح اظہاریہ_Idx_Cover(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_کالم
   && p_اظہاریہ->i_جدول==pWalker->u.pIdxCover->iCur
   && sqlite3_جدول_کالم_منزل_اشاریہ(pWalker->u.pIdxCover->pIdx, p_اظہاریہ->i_کالم)<0
  ){
    pWalker->e_کوڈ = 1;
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
صحيح sqlite3_اظہاریہ_Covered_By_اشاریہ(
  اظہاریہ *p_اظہاریہ,        /* The index to be tested */
  صحيح iCur,           /* The cursor number for the corresponding table */
  اشاریہ *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  بنية IdxCover xcov;
  memset(&w, 0, حجم(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_اظہاریہ_Callback = اظہاریہ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  أرجع !w.e_کوڈ;
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
ساكن صحيح اظہاریہ_Ref_منزل_Src_List(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( p_اظہاریہ->op==TK_کالم
   || p_اظہاریہ->op==TK_AGG_کالم
  ){
    صحيح i;
    بنية RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحيح nSrc = pSrc ? pSrc->nSrc : 0;
    لكل(i=0; i<nSrc; i++){
      إذا( p_اظہاریہ->i_جدول==pSrc->a[i].iCursor ){
        pWalker->e_کوڈ |= 1;
        أرجع WRC_Continue;
      }
    }
    لكل(i=0; i<p->nExclude && p->aiExclude[i]!=p_اظہاریہ->i_جدول; i++){}
    إذا( i>=p->nExclude ){
      pWalker->e_کوڈ |= 2;
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
صحيح sqlite3ReferencesSrcList(Parse *pParse, اظہاریہ *p_اظہاریہ, SrcList *pSrcList){
  Walker w;
  بنية RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, حجم(w));
  memset(&x, 0, حجم(x));
  w.x_اظہاریہ_Callback = اظہاریہ_Ref_منزل_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( p_اظہاریہ->op==TK_AGG_FUNCTION );
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  sqlite3_Walk_اظہاریہ_List(&w, p_اظہاریہ->x.pList);
  إذا( p_اظہاریہ->p_بایاں ){
    أكد( p_اظہاریہ->p_بایاں->op==TK_ترتیب );
    أكد( اظہاریہ_Use_XList(p_اظہاریہ->p_بایاں) );
    أكد( p_اظہاریہ->p_بایاں->x.pList!=0 );
    sqlite3_Walk_اظہاریہ_List(&w, p_اظہاریہ->p_بایاں->x.pList);
  }
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
    sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ->y.pWin->pFilter);
  }
#جے_انت
  إذا( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  إذا( w.e_کوڈ & 0x01 ){
    أرجع 1;
  }وإلا إذا( w.e_کوڈ ){
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
ساكن صحيح agginfo_Persist_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  إذا( ALWAYS(!اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced))
   && p_اظہاریہ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
    صحيح iAgg = p_اظہاریہ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    إذا( p_اظہاریہ->op!=TK_AGG_FUNCTION ){
      إذا( iAgg<pAggInfo->n_کالم
       && pAggInfo->a_خانہ[iAgg].pCExpr==p_اظہاریہ
      ){
        p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        إذا( p_اظہاریہ && !sqlite3_اظہاریہ_Deferred_Delete(pParse, p_اظہاریہ) ){
          pAggInfo->a_خانہ[iAgg].pCExpr = p_اظہاریہ;
        }
      }
    }وإلا{
      أكد( p_اظہاریہ->op==TK_AGG_FUNCTION );
      إذا( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_اظہاریہ
      ){
        p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        إذا( p_اظہاریہ && !sqlite3_اظہاریہ_Deferred_Delete(pParse, p_اظہاریہ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_اظہاریہ;
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
  pWalker->x_اظہاریہ_Callback = agginfo_Persist_اظہاریہ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح جوڑو_Agg_Info_کالم(sqlite3 *db, AggInfo *pInfo){
  صحيح i;
  pInfo->a_خانہ = sqlite3ArrayAllocate(
       db,
       pInfo->a_خانہ,
       حجم(pInfo->a_خانہ[0]),
       &pInfo->n_کالم,
       &i
  );
  أرجع i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساكن صحيح جوڑو_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
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
ساكن فراغ لبھو_Or_Create_Agg_Info_کالم(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  اظہاریہ *p_اظہاریہ          /* Expr describing the column to find or insert */
){
  بنية Agg_Info_خانہ *p_خانہ;
  صحيح k;
  صحيح mx_رکن = pParse->db->a_حد_بندی[SQLITE_حد_بندی_کالم];

  أكد( mx_رکن <= SMXV(i16) );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  p_خانہ = pAggInfo->a_خانہ;
  لكل(k=0; k<pAggInfo->n_کالم; k++, p_خانہ++){
    إذا( p_خانہ->pCExpr==p_اظہاریہ ) أرجع;
    إذا( p_خانہ->i_جدول==p_اظہاریہ->i_جدول
     && p_خانہ->i_کالم==p_اظہاریہ->i_کالم
     && p_اظہاریہ->op!=TK_IF_NULL_صف
    ){
      اذهب fix_اتے_اظہاریہ;
    }
  }
  k = جوڑو_Agg_Info_کالم(pParse->db, pAggInfo);
  إذا( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    أرجع;
  }
  إذا( k>mx_رکن ){
    sqlite3_Error_پیغام(pParse, "more than %d aggregate terms", mx_رکن);
    k = mx_رکن;
  }
  p_خانہ = &pAggInfo->a_خانہ[k];
  أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
  p_خانہ->pTab = p_اظہاریہ->y.pTab;
  p_خانہ->i_جدول = p_اظہاریہ->i_جدول;
  p_خانہ->i_کالم = p_اظہاریہ->i_کالم;
  p_خانہ->i_Sorter_کالم = -1;
  p_خانہ->pCExpr = p_اظہاریہ;
  إذا( pAggInfo->p_گروہ_By && p_اظہاریہ->op!=TK_IF_NULL_صف ){
    صحيح j, n;
    اظہاریہ_List *pGB = pAggInfo->p_گروہ_By;
    بنية اظہاریہ_List_شے *p_رکن = pGB->a;
    n = pGB->n_اظہاریہ;
    لكل(j=0; j<n; j++, p_رکن++){
      اظہاریہ *pE = p_رکن->p_اظہاریہ;
      إذا( pE->op==TK_کالم
       && pE->i_جدول==p_اظہاریہ->i_جدول
       && pE->i_کالم==p_اظہاریہ->i_کالم
      ){
        p_خانہ->i_Sorter_کالم = j;
        اكسر;
      }
    }
  }
  إذا( p_خانہ->i_Sorter_کالم<0 ){
    p_خانہ->i_Sorter_کالم = pAggInfo->n_چھانٹی_کالم++;
  }
fix_اتے_اظہاریہ:
  اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);
  أكد( p_اظہاریہ->pAggInfo==0 || p_اظہاریہ->pAggInfo==pAggInfo );
  p_اظہاریہ->pAggInfo = pAggInfo;
  إذا( p_اظہاریہ->op==TK_کالم ){
    p_اظہاریہ->op = TK_AGG_کالم;
  }
  أكد( k <= SMXV(p_اظہاریہ->iAgg) );
  p_اظہاریہ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ساكن صحيح analyzeAggregate(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحيح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  بدل( p_اظہاریہ->op ){
    افتراضي: {
      اشاریہ_دار_اظہاریہ *pIEpr;
      اظہاریہ tmp;
      أكد( pParse->iSelfTab==0 );
      إذا( (pNC->ncFlags & NC_وچ_Agg_Func)==0 ) اكسر;
      إذا( pParse->pIdxEpr==0 ) اكسر;
      لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحيح i_انکڑے_Cur = pIEpr->i_انکڑے_Cur;
        إذا( i_انکڑے_Cur<0 ) استمر;
        إذا( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, pIEpr->p_اظہاریہ, i_انکڑے_Cur)==0 ) اكسر;
      }
      إذا( pIEpr==0 ) اكسر;
      إذا( NEVER(!اظہاریہ_Use_YTab(p_اظہاریہ)) ) اكسر;
      لكل(i=0; i<pSrcList->nSrc; i++){
         إذا( pSrcList->a[i].iCursor==pIEpr->i_انکڑے_Cur ){
           testcase( i>0 );
           اكسر;
         }
      }
      إذا( i>=pSrcList->nSrc ) اكسر;
      إذا( NEVER(p_اظہاریہ->pAggInfo!=0) ) اكسر; /* Resolved by outer context */
      إذا( pParse->nErr ){ أرجع WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, حجم(tmp));
      tmp.op = TK_AGG_کالم;
      tmp.i_جدول = pIEpr->iIdxCur;
      tmp.i_کالم = pIEpr->i_Idx_خانہ;
      لبھو_Or_Create_Agg_Info_کالم(pParse, pAggInfo, &tmp);
      إذا( pParse->nErr ){ أرجع WRC_Abort; }
      أكد( pAggInfo->a_خانہ!=0 );
      أكد( tmp.iAgg<pAggInfo->n_کالم );
      pAggInfo->a_خانہ[tmp.iAgg].pCExpr = p_اظہاریہ;
      p_اظہاریہ->pAggInfo = pAggInfo;
      p_اظہاریہ->iAgg = tmp.iAgg;
      أرجع WRC_Prune;
    }
    حالة TK_IF_NULL_صف:
    حالة TK_AGG_کالم:
    حالة TK_کالم: {
      testcase( p_اظہاریہ->op==TK_AGG_کالم );
      testcase( p_اظہاریہ->op==TK_کالم );
      testcase( p_اظہاریہ->op==TK_IF_NULL_صف );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      إذا( ALWAYS(pSrcList!=0) ){
        Src_شے *p_شے = pSrcList->a;
        لكل(i=0; i<pSrcList->nSrc; i++, p_شے++){
          أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );
          إذا( p_اظہاریہ->i_جدول==p_شے->iCursor ){
            لبھو_Or_Create_Agg_Info_کالم(pParse, pAggInfo, p_اظہاریہ);
            اكسر;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      أرجع WRC_Continue;
    }
    حالة TK_AGG_FUNCTION: {
      إذا( (pNC->ncFlags & NC_وچ_Agg_Func)==0
       && pWalker->walker_ڈونگھائی==p_اظہاریہ->op2
       && p_اظہاریہ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        بنية AggInfo_func *p_شے = pAggInfo->aFunc;
        صحيح mx_رکن = pParse->db->a_حد_بندی[SQLITE_حد_بندی_کالم];
        أكد( mx_رکن <= SMXV(i16) );
        لكل(i=0; i<pAggInfo->nFunc; i++, p_شے++){
          إذا( NEVER(p_شے->pFExpr==p_اظہاریہ) ) اكسر;
          إذا( sqlite3_اظہاریہ_Compare(0, p_شے->pFExpr, p_اظہاریہ, -1)==0 ){
            اكسر;
          }
        }
        إذا( i>mx_رکن ){
          sqlite3_Error_پیغام(pParse, "more than %d aggregate terms", mx_رکن);
          i = mx_رکن;
          أكد( i<pAggInfo->nFunc );
        }وإلا إذا( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = جوڑو_Agg_Info_Func(pParse->db, pAggInfo);
          إذا( i>=0 ){
            صحيح nArg;
            أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect) );
            p_شے = &pAggInfo->aFunc[i];
            p_شے->pFExpr = p_اظہاریہ;
            أكد( اظہاریہ_Use_UToken(p_اظہاریہ) );
            nArg = p_اظہاریہ->x.pList ? p_اظہاریہ->x.pList->n_اظہاریہ : 0;
            p_شے->pFunc = sqlite3_لبھو_Function(pParse->db,
                                         p_اظہاریہ->u.zToken, nArg, enc, 0);
            أكد( p_شے->bOBUnique==0 );
            إذا( p_اظہاریہ->p_بایاں
             && (p_شے->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              اظہاریہ_List *pOBList;
              أكد( nArg>0 );
              أكد( p_اظہاریہ->p_بایاں->op==TK_ترتیب );
              أكد( اظہاریہ_Use_XList(p_اظہاریہ->p_بایاں) );
              p_شے->iOBTab = pParse->nTab++;
              pOBList = p_اظہاریہ->p_بایاں->x.pList;
              أكد( pOBList->n_اظہاریہ>0 );
              أكد( p_شے->bOBUnique==0 );
              إذا( pOBList->n_اظہاریہ==1
               && nArg==1
               && sqlite3_اظہاریہ_Compare(0,pOBList->a[0].p_اظہاریہ,
                               p_اظہاریہ->x.pList->a[0].p_اظہاریہ,-1)==0
              ){
                p_شے->bOBPayload = 0;
                p_شے->bOBUnique = اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct);
              }وإلا{
                p_شے->bOBPayload = 1;
              }
              p_شے->bUseSubtype =
                    (p_شے->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }وإلا{
              p_شے->iOBTab = -1;
            }
            إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct) && !p_شے->bOBUnique ){
              p_شے->iDistinct = pParse->nTab++;
            }وإلا{
              p_شے->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );
        اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);
        أكد( i <= SMXV(p_اظہاریہ->iAgg) );
        p_اظہاریہ->iAgg = (i16)i;
        p_اظہاریہ->pAggInfo = pAggInfo;
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
فراغ sqlite3_اظہاریہ_Analyze_Aggregates(NameContext *pNC, اظہاریہ *p_اظہاریہ){
  Walker w;
  w.x_اظہاریہ_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_ڈونگھائی_Increase;
  w.xSelectCallback2 = sqlite3_Walker_ڈونگھائی_Decrease;
  w.walker_ڈونگھائی = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  أكد( pNC->pSrcList!=0 );
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
فراغ sqlite3_اظہاریہ_Analyze_Agg_List(NameContext *pNC, اظہاریہ_List *pList){
  بنية اظہاریہ_List_شے *p_شے;
  صحيح i;
  إذا( pList ){
    لكل(p_شے=pList->a, i=0; i<pList->n_اظہاریہ; i++, p_شے++){
      sqlite3_اظہاریہ_Analyze_Aggregates(pNC, p_شے->p_اظہاریہ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحيح sqlite3_Get_عارضی_Reg(Parse *pParse){
  إذا( pParse->n_عارضی_Reg==0 ){
    أرجع ++pParse->nMem;
  }
  أرجع pParse->a_عارضی_Reg[--pParse->n_عارضی_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
فراغ sqlite3_Release_عارضی_Reg(Parse *pParse, صحيح iReg){
  إذا( iReg ){
#جے_تعریف_شدہ SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحيح kk;
    لكل(kk=0; kk<pParse->n_عارضی_Reg; kk++){
      أكد( pParse->a_عارضی_Reg[kk]!=iReg );
    }
#جے_انت
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    إذا( pParse->n_عارضی_Reg<Array_ناپ(pParse->a_عارضی_Reg) ){
      pParse->a_عارضی_Reg[pParse->n_عارضی_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحيح sqlite3_Get_عارضی_Range(Parse *pParse, صحيح nReg){
  صحيح i, n;
  إذا( nReg==1 ) أرجع sqlite3_Get_عارضی_Reg(pParse);
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
فراغ sqlite3_Release_عارضی_Range(Parse *pParse, صحيح iReg, صحيح nReg){
  إذا( nReg==1 ){
    sqlite3_Release_عارضی_Reg(pParse, iReg);
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
فراغ sqlite3_Clear_عارضی_Reg_Cache(Parse *pParse){
  pParse->n_عارضی_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
فراغ sqlite3_Touch_رجسٹر(Parse *pParse, صحيح iReg){
  إذا( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#إذا defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحيح sqlite3_پہلا_دستیاب_رجسٹر(Parse *pParse, صحيح iMin){
  ثابت اظہاریہ_List *pList = pParse->p_Const_اظہاریہ;
  إذا( pList ){
    صحيح i;
    لكل(i=0; i<pList->n_اظہاریہ; i++){
      إذا( pList->a[i].u.i_Const_اظہاریہ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_اظہاریہ_Reg + 1;
      }
    }
  }
  pParse->n_عارضی_Reg = 0;
  pParse->nRangeReg = 0;
  أرجع iMin;
}
#جے_انت /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#جے_تعریف_شدہ SQLITE_DEBUG
صحيح sqlite3_No_Temps_وچ_Range(Parse *pParse, صحيح i_پہلا, صحيح i_آخری){
  صحيح i;
  إذا( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_پہلا
   && pParse->iRangeReg <= i_آخری
  ){
     أرجع 0;
  }
  لكل(i=0; i<pParse->n_عارضی_Reg; i++){
    إذا( pParse->a_عارضی_Reg[i]>=i_پہلا && pParse->a_عارضی_Reg[i]<=i_آخری ){
      أرجع 0;
    }
  }
  إذا( pParse->p_Const_اظہاریہ ){
    اظہاریہ_List *pList = pParse->p_Const_اظہاریہ;
    لكل(i=0; i<pList->n_اظہاریہ; i++){
      صحيح iReg = pList->a[i].u.i_Const_اظہاریہ_Reg;
      إذا( iReg==0 ) استمر;
      إذا( iReg>=i_پہلا && iReg<=i_آخری ) أرجع 0;
    }
  }
  أرجع 1;
}
#جے_انت /* SQLITE_DEBUG */
