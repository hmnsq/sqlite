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
ساکن خالی اظہاریہ_رمز_Between(Parse*,اظہاریہ*,صحیح,خالی(*)(Parse*,اظہاریہ*,صحیح,صحیح),صحیح);
ساکن صحیح اظہاریہ_رمز_سمتیہ(Parse *pParse, اظہاریہ *p, صحیح *pi_مقصد_Free);

/*
** Return the affinity character for a single column of a table.
*/
حرف sqlite3_جدول_عمود_Affinity(ثابت جدول *pTab, صحیح i_ستون){
  اگر( i_ستون<0 || NEVER(i_ستون>=pTab->n_ستون) ) واپس SQLITE_AFF_INTEGER;
  واپس pTab->a_ستون[i_ستون].affinity;
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
  صحیح op;
  op = p_اظہاریہ->op;
  جب( 1 /* exit-by-break */ ){
    اگر( op==TK_عمود || (op==TK_AGG_عمود && p_اظہاریہ->y.pTab!=0) ){
      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      أكد( p_اظہاریہ->y.pTab!=0 );
      واپس sqlite3_جدول_عمود_Affinity(p_اظہاریہ->y.pTab, p_اظہاریہ->i_عمود);
    }
    اگر( op==TK_SELECT ){
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pSelect!=0 );
      أكد( p_اظہاریہ->x.pSelect->pEList!=0 );
      أكد( p_اظہاریہ->x.pSelect->pEList->a[0].p_اظہاریہ!=0 );
      واپس sqlite3_اظہاریہ_Affinity(p_اظہاریہ->x.pSelect->pEList->a[0].p_اظہاریہ);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CAST
    اگر( op==TK_CAST ){
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      واپس sqlite3AffinityType(p_اظہاریہ->u.zToken, 0);
    }
#اگر_ختم
    اگر( op==TK_SELECT_عمود ){
      أكد( p_اظہاریہ->p_بایاں!=0 && اظہاریہ_Use_XSelect(p_اظہاریہ->p_بایاں) );
      أكد( p_اظہاریہ->i_عمود < p_اظہاریہ->i_جدول );
      أكد( p_اظہاریہ->i_عمود >= 0 );
      أكد( p_اظہاریہ->i_جدول==p_اظہاریہ->p_بایاں->x.pSelect->pEList->n_اظہاریہ );
      واپس sqlite3_اظہاریہ_Affinity(
          p_اظہاریہ->p_بایاں->x.pSelect->pEList->a[p_اظہاریہ->i_عمود].p_اظہاریہ
      );
    }
    اگر( op==TK_سمتیہ
     || (op==TK_FUNCTION && p_اظہاریہ->aff_اظہاریہ==SQLITE_AFF_DEFER)
    ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      واپس sqlite3_اظہاریہ_Affinity(p_اظہاریہ->x.pList->a[0].p_اظہاریہ);
    }
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip|EP_If_Null_صف) ){
      أكد( p_اظہاریہ->op==TK_COLLATE
           || p_اظہاریہ->op==TK_IF_NULL_صف
           || (p_اظہاریہ->op==TK_رجسٹر && p_اظہاریہ->op2==TK_IF_NULL_صف) );
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
      op = p_اظہاریہ->op;
      جاری;
    }
    اگر( op!=TK_رجسٹر ) توڑو;
    op = p_اظہاریہ->op2;
    اگر( NEVER( op==TK_رجسٹر ) ) توڑو;
  }
  واپس p_اظہاریہ->aff_اظہاریہ;
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
صحیح sqlite3_اظہاریہ_ڈیٹا_Type(ثابت اظہاریہ *p_اظہاریہ){
  جب( p_اظہاریہ ){
    انتخاب( p_اظہاریہ->op ){
      حالت TK_COLLATE:
      حالت TK_IF_NULL_صف:
      حالت TK_UPLUS:  {
        p_اظہاریہ = p_اظہاریہ->p_بایاں;
        توڑو;
      }
      حالت TK_NULL: {
        p_اظہاریہ = 0;
        توڑو;
      }
      حالت TK_STRING: {
        واپس 0x02;
      }
      حالت TK_BLOB: {
        واپس 0x04;
      }
      حالت TK_CONCAT: {
        واپس 0x06;
      }
      حالت TK_VARIABLE:
      حالت TK_AGG_FUNCTION:
      حالت TK_FUNCTION: {
        واپس 0x07;
      }
      حالت TK_عمود:
      حالت TK_AGG_عمود:
      حالت TK_SELECT:
      حالت TK_CAST:
      حالت TK_SELECT_عمود:
      حالت TK_سمتیہ:  {
        صحیح aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
        اگر( aff>=SQLITE_AFF_NUMERIC ) واپس 0x05;
        اگر( aff==SQLITE_AFF_متن )    واپس 0x06;
        واپس 0x07;
      }
      حالت TK_CASE: {
        صحیح res = 0;
        صحیح ii;
        اظہاریہ_List *pList = p_اظہاریہ->x.pList;
        أكد( اظہاریہ_Use_XList(p_اظہاریہ) && pList!=0 );
        أكد( pList->n_اظہاریہ > 0);
        برائے(ii=1; ii<pList->n_اظہاریہ; ii+=2){
          res |= sqlite3_اظہاریہ_ڈیٹا_Type(pList->a[ii].p_اظہاریہ);
        }
        اگر( pList->n_اظہاریہ % 2 ){
          res |= sqlite3_اظہاریہ_ڈیٹا_Type(pList->a[pList->n_اظہاریہ-1].p_اظہاریہ);
        }
        واپس res;
      }
      طےشدہ: {
        واپس 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  واپس 0x00;
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
  صحیح dequote              /* True to dequote pCollName */
){
  اگر( pCollName->n>0 && pParse->nErr==0 ){
    اظہاریہ *pNew = sqlite3_اظہاریہ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    أكد( p_اظہاریہ!=0 );
    اگر( pNew ){
      pNew->p_بایاں = p_اظہاریہ;
      pNew->flags |= EP_Collate|EP_Skip;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
      pNew->n_اونچائی = p_اظہاریہ->n_اونچائی+1;
      sqlite3_اظہاریہ_Check_اونچائی(pParse, pNew->n_اونچائی);
      testcase( pParse->nErr>0 );
#اگر_ختم
      p_اظہاریہ = pNew;
    }
  }
  واپس p_اظہاریہ;
}
اظہاریہ *sqlite3_اظہاریہ_جوڑو_Collate_String(
  Parse *pParse,        /* Parsing context */
  اظہاریہ *p_اظہاریہ,          /* Add the "COLLATE" clause to this expression */
  ثابت حرف *zC        /* The collating sequence name */
){
  Token s;
  أكد( zC!=0 );
  sqlite3TokenInit(&s, (حرف*)zC);
  واپس sqlite3_اظہاریہ_جوڑو_Collate_Token(pParse, p_اظہاریہ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
اظہاریہ *sqlite3_اظہاریہ_Skip_Collate(اظہاریہ *p_اظہاریہ){
  جب( p_اظہاریہ && اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip) ){
    أكد( p_اظہاریہ->op==TK_COLLATE );
    p_اظہاریہ = p_اظہاریہ->p_بایاں;
  }  
  واپس p_اظہاریہ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
اظہاریہ *sqlite3_اظہاریہ_Skip_Collate_And_Likely(اظہاریہ *p_اظہاریہ){
  جب( p_اظہاریہ && اظہاریہ_Has_Property(p_اظہاریہ, EP_Skip|EP_Unlikely) ){
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Unlikely) ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pList->n_اظہاریہ>0 );
      أكد( p_اظہاریہ->op==TK_FUNCTION );
      p_اظہاریہ = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
    }ورنہ اگر( p_اظہاریہ->op==TK_COLLATE ){
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
    }ورنہ{
      توڑو;
    }
  }  
  واپس p_اظہاریہ;
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
  جب( p ){
    صحیح op = p->op;
    اگر( op==TK_رجسٹر ) op = p->op2;
    اگر( (op==TK_AGG_عمود && p->y.pTab!=0)
     || op==TK_عمود || op==TK_TRIGGER
    ){
      صحیح j;
      أكد( اظہاریہ_Use_YTab(p) );
      أكد( p->y.pTab!=0 );
      اگر( (j = p->i_عمود)>=0 ){
        ثابت حرف *zColl = sqlite3_عمود_Coll(&p->y.pTab->a_ستون[j]);
        pColl = sqlite3_ڈھونڈو_Coll_Seq(db, ENC(db), zColl, 0);
      }
      توڑو;
    }
    اگر( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_بایاں;
      جاری;
    }
    اگر( op==TK_سمتیہ
     || (op==TK_FUNCTION && p->aff_اظہاریہ==SQLITE_AFF_DEFER)
    ){
      أكد( اظہاریہ_Use_XList(p) );
      p = p->x.pList->a[0].p_اظہاریہ;
      جاری;
    }
    اگر( op==TK_COLLATE ){
      أكد( !اظہاریہ_Has_Property(p, EP_Int_قدر) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      توڑو;
    }
    اگر( p->flags & EP_Collate ){
      اگر( p->p_بایاں && (p->p_بایاں->flags & EP_Collate)!=0 ){
        p = p->p_بایاں;
      }ورنہ{
        اظہاریہ *p_اگلا  = p->p_دایاں;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        أكد( !اظہاریہ_Use_XList(p) || p->x.pList==0 || p->p_دایاں==0 );
        اگر( اظہاریہ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          صحیح i;
          برائے(i=0; i<p->x.pList->n_اظہاریہ; i++){
            اگر( اظہاریہ_Has_Property(p->x.pList->a[i].p_اظہاریہ, EP_Collate) ){
              p_اگلا = p->x.pList->a[i].p_اظہاریہ;
              توڑو;
            }
          }
        }
        p = p_اگلا;
      }
    }ورنہ{
      توڑو;
    }
  }
  اگر( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  واپس pColl;
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
  اگر( p==0 ) p = pParse->db->pDfltColl;
  أكد( p!=0 );
  واپس p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
صحیح sqlite3_اظہاریہ_Coll_Seq_مطابقت(Parse *pParse, ثابت اظہاریہ *pE1, ثابت اظہاریہ *pE2){
  CollSeq *pColl1 = sqlite3_اظہاریہ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_اظہاریہ_NNColl_Seq(pParse, pE2);
  أكد( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  واپس pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
حرف sqlite3CompareAffinity(ثابت اظہاریہ *p_اظہاریہ, حرف aff2){
  حرف aff1 = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
  اگر( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    اگر( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      واپس SQLITE_AFF_NUMERIC;
    }ورنہ{
      واپس SQLITE_AFF_BLOB;
    }
  }ورنہ{
    /* One side is a column, the other is not. Use the columns affinity. */
    أكد( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    واپس (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ساکن حرف comparisonAffinity(ثابت اظہاریہ *p_اظہاریہ){
  حرف aff;
  أكد( p_اظہاریہ->op==TK_EQ || p_اظہاریہ->op==TK_اندر || p_اظہاریہ->op==TK_LT ||
          p_اظہاریہ->op==TK_GT || p_اظہاریہ->op==TK_GE || p_اظہاریہ->op==TK_LE ||
          p_اظہاریہ->op==TK_NE || p_اظہاریہ->op==TK_IS || p_اظہاریہ->op==TK_ISNOT );
  أكد( p_اظہاریہ->p_بایاں );
  أكد( !اظہاریہ_Use_XSelect(p_اظہاریہ) );
  aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_بایاں);
  اگر( p_اظہاریہ->p_دایاں ){
    aff = sqlite3CompareAffinity(p_اظہاریہ->p_دایاں, aff);
  }ورنہ اگر( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  واپس aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
صحیح sqlite3_اشاریہ_Affinity_Ok(ثابت اظہاریہ *p_اظہاریہ, حرف idx_affinity){
  حرف aff = comparisonAffinity(p_اظہاریہ);
  اگر( aff<SQLITE_AFF_متن ){
    واپس 1;
  }
  اگر( aff==SQLITE_AFF_متن ){
    واپس idx_affinity==SQLITE_AFF_متن;
  }
  واپس sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ساکن u8 ثنائی_Compare_P5(
  ثابت اظہاریہ *pExpr1,   /* Left operand */
  ثابت اظہاریہ *pExpr2,   /* Right operand */
  صحیح jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (حرف)sqlite3_اظہاریہ_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  واپس aff;
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
  اگر( p_بایاں->flags & EP_Collate ){
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_بایاں);
  }ورنہ اگر( p_دایاں && (p_دایاں->flags & EP_Collate)!=0 ){
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_دایاں);
  }ورنہ{
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_بایاں);
    اگر( !pColl ){
      pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_دایاں);
    }
  }
  واپس pColl;
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
  اگر( اظہاریہ_Has_Property(p, EP_Commuted) ){
    واپس sqlite3_ثنائی_Compare_Coll_Seq(pParse, p->p_دایاں, p->p_بایاں);
  }ورنہ{
    واپس sqlite3_ثنائی_Compare_Coll_Seq(pParse, p->p_بایاں, p->p_دایاں);
  }
}

/*
** Generate code for a comparison operator.
*/
ساکن صحیح رمز_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  اظہاریہ *p_بایاں,      /* The left operand */
  اظہاریہ *p_دایاں,     /* The right operand */
  صحیح opcode,       /* The comparison opcode */
  صحیح in1, صحیح in2, /* Register holding operands */
  صحیح dest,         /* Jump here if true.  */
  صحیح jumpIfNull,   /* If true, jump if either operand is NULL */
  صحیح isCommuted    /* The comparison has been commuted */
){
  صحیح p5;
  صحیح addr;
  CollSeq *p4;

  اگر( pParse->nErr ) واپس 0;
  اگر( isCommuted ){
    p4 = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_دایاں, p_بایاں);
  }ورنہ{
    p4 = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p_بایاں, p_دایاں);
  }
  p5 = ثنائی_Compare_P5(p_بایاں, p_دایاں, jumpIfNull);
  addr = sqlite3_Vdbe_جوڑو_Op4(pParse->pVdbe, opcode, in2, dest, in1,
                           (خالی*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  واپس addr;
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
صحیح sqlite3_اظہاریہ_Is_سمتیہ(ثابت اظہاریہ *p_اظہاریہ){
  واپس sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
صحیح sqlite3_اظہاریہ_سمتیہ_سائز(ثابت اظہاریہ *p_اظہاریہ){
  u8 op = p_اظہاریہ->op;
  اگر( op==TK_رجسٹر ) op = p_اظہاریہ->op2;
  اگر( op==TK_سمتیہ ){
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    واپس p_اظہاریہ->x.pList->n_اظہاریہ;
  }ورنہ اگر( op==TK_SELECT ){
    أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
    واپس p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ;
  }ورنہ{
    واپس 1;
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
اظہاریہ *sqlite3_سمتیہ_میدان_Subexpr(اظہاریہ *p_سمتیہ, صحیح i){
  أكد( i<sqlite3_اظہاریہ_سمتیہ_سائز(p_سمتیہ) || p_سمتیہ->op==TK_ERROR );
  اگر( sqlite3_اظہاریہ_Is_سمتیہ(p_سمتیہ) ){
    أكد( p_سمتیہ->op2==0 || p_سمتیہ->op==TK_رجسٹر );
    اگر( p_سمتیہ->op==TK_SELECT || p_سمتیہ->op2==TK_SELECT ){
      أكد( اظہاریہ_Use_XSelect(p_سمتیہ) );
      واپس p_سمتیہ->x.pSelect->pEList->a[i].p_اظہاریہ;
    }ورنہ{
      أكد( اظہاریہ_Use_XList(p_سمتیہ) );
      واپس p_سمتیہ->x.pList->a[i].p_اظہاریہ;
    }
  }
  واپس p_سمتیہ;
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
  صحیح i_میدان,          /* Which column of the vector to return */
  صحیح n_میدان           /* Total number of columns in the vector */
){
  اظہاریہ *pRet;
  اگر( p_سمتیہ->op==TK_SELECT ){
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_عمود, 0, 0);
    اگر( pRet ){
      اظہاریہ_سیٹ_Property(pRet, EP_بھرا_سائز);
      pRet->i_جدول = n_میدان;
      pRet->i_عمود = i_میدان;
      pRet->p_بایاں = p_سمتیہ;
    }
  }ورنہ{
    اگر( p_سمتیہ->op==TK_سمتیہ ){
      اظہاریہ **pp_سمتیہ;
      أكد( اظہاریہ_Use_XList(p_سمتیہ) );
      pp_سمتیہ = &p_سمتیہ->x.pList->a[i_میدان].p_اظہاریہ;
      p_سمتیہ = *pp_سمتیہ;
      اگر( اندر_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_سمتیہ = 0;
        واپس p_سمتیہ;
      }
    }
    pRet = sqlite3_اظہاریہ_Dup(pParse->db, p_سمتیہ, 0);
  }
  واپس pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ساکن صحیح اظہاریہ_رمز_Subselect(Parse *pParse, اظہاریہ *p_اظہاریہ){
  صحیح reg = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
  اگر( p_اظہاریہ->op==TK_SELECT ){
    reg = sqlite3_رمز_Subselect(pParse, p_اظہاریہ);
  }
#اگر_ختم
  واپس reg;
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
ساکن صحیح اظہاریہ_سمتیہ_رجسٹر(
  Parse *pParse,                  /* Parse context */
  اظہاریہ *p_سمتیہ,                  /* Vector to extract element from */
  صحیح i_میدان,                     /* Field to extract from pVector */
  صحیح regSelect,                  /* First in array of registers */
  اظہاریہ *pTmp,                     /* Temporary space */
  اظہاریہ **pp_اظہاریہ,                  /* OUT: Expression element */
  صحیح *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_سمتیہ->op;
  أكد( op==TK_سمتیہ || op==TK_رجسٹر || op==TK_SELECT || op==TK_ERROR );
  اگر( op==TK_رجسٹر ){
    *pp_اظہاریہ = sqlite3_سمتیہ_میدان_Subexpr(p_سمتیہ, i_میدان);
    واپس p_سمتیہ->i_جدول+i_میدان;
  }
  اگر( op==TK_SELECT ){
    أكد( اظہاریہ_Use_XSelect(p_سمتیہ) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, حجم(اظہاریہ));
    pTmp->op = TK_SELECT_عمود;
    pTmp->p_بایاں = p_سمتیہ;
    pTmp->i_عمود = i_میدان;
    pTmp->i_جدول = p_سمتیہ->x.pSelect->pEList->n_اظہاریہ;
    *pp_اظہاریہ = pTmp;
    واپس regSelect+i_میدان;
  }
  اگر( op==TK_سمتیہ ){
    أكد( اظہاریہ_Use_XList(p_سمتیہ) );
    *pp_اظہاریہ = p_سمتیہ->x.pList->a[i_میدان].p_اظہاریہ;
    واپس sqlite3_اظہاریہ_رمز_عارضی(pParse, *pp_اظہاریہ, pRegFree);
  }
  واپس 0;
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
ساکن خالی رمز_سمتیہ_Compare(
  Parse *pParse,        /* Code generator context */
  اظہاریہ *p_اظہاریہ,          /* The comparison operation */
  صحیح dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
  اظہاریہ *p_دایاں = p_اظہاریہ->p_دایاں;
  صحیح n_بایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_بایاں);
  صحیح i;
  صحیح reg_بایاں = 0;
  صحیح reg_دایاں = 0;
  u8 opx = op;
  صحیح addrCmp = 0;
  صحیح addr_مکمل = sqlite3VdbeMakeLabel(pParse);
  صحیح isCommuted = اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted);

  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  اگر( pParse->nErr ) واپس;
  اگر( n_بایاں!=sqlite3_اظہاریہ_سمتیہ_سائز(p_دایاں) ){
    sqlite3_Error_پیغام(pParse, "row value misused");
    واپس;
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

  اگر( op==TK_LE ) opx = TK_LT;
  اگر( op==TK_GE ) opx = TK_GT;
  اگر( op==TK_NE ) opx = TK_EQ;

  reg_بایاں = اظہاریہ_رمز_Subselect(pParse, p_بایاں);
  reg_دایاں = اظہاریہ_رمز_Subselect(pParse, p_دایاں);

  sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, dest);
  برائے(i=0; 1 /*Loop exits by "break"*/; i++){
    صحیح regFree1 = 0, regFree2 = 0;
    اظہاریہ *pL = 0, *pR = 0;
    اظہاریہ tmp1, tmp2;
    صحیح r1, r2;
    أكد( i>=0 && i<n_بایاں );
    اگر( addrCmp ) sqlite3_Vdbe_Jump_یہاں(v, addrCmp);
    r1 = اظہاریہ_سمتیہ_رجسٹر(pParse, p_بایاں, i, reg_بایاں, &tmp1, &pL, &regFree1);
    r2 = اظہاریہ_سمتیہ_رجسٹر(pParse, p_دایاں, i, reg_دایاں, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    رمز_Compare(pParse, pL, pR, opx, r1, r2, addr_مکمل, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_عارضی_Reg(pParse, regFree1);
    sqlite3_Release_عارضی_Reg(pParse, regFree2);
    اگر( (opx==TK_LT || opx==TK_GT) && i<n_بایاں-1 ){
      addrCmp = sqlite3_Vdbe_جوڑو_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    اگر( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, dest);
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    اگر( i==n_بایاں-1 ){
      توڑو;
    }
    اگر( opx==TK_EQ ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, dest, addr_مکمل); VdbeCoverage(v);
    }ورنہ{
      أكد( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, addr_مکمل);
      اگر( i==n_بایاں-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_یہاں(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_مکمل);
  اگر( op==TK_NE ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Not, dest, dest);
  }
}

#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
صحیح sqlite3_اظہاریہ_Check_اونچائی(Parse *pParse, صحیح n_اونچائی){
  صحیح rc = SQLITE_OK;
  صحیح mx_اونچائی = pParse->db->a_حد_بندی[SQLITE_حد_بندی_اظہاریہ_گہرائی];
  اگر( n_اونچائی>mx_اونچائی ){
    sqlite3_Error_پیغام(pParse,
       "Expression tree is too large (maximum depth %d)", mx_اونچائی
    );
    rc = SQLITE_ERROR;
  }
  واپس rc;
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
ساکن خالی اونچائی_کا_اظہاریہ(ثابت اظہاریہ *p, صحیح *pn_اونچائی){
  اگر( p ){
    اگر( p->n_اونچائی>*pn_اونچائی ){
      *pn_اونچائی = p->n_اونچائی;
    }
  }
}
ساکن خالی اونچائی_کا_اظہاریہ_List(ثابت اظہاریہ_List *p, صحیح *pn_اونچائی){
  اگر( p ){
    صحیح i;
    برائے(i=0; i<p->n_اظہاریہ; i++){
      اونچائی_کا_اظہاریہ(p->a[i].p_اظہاریہ, pn_اونچائی);
    }
  }
}
ساکن خالی اونچائی_کا_Select(ثابت Select *pSelect, صحیح *pn_اونچائی){
  ثابت Select *p;
  برائے(p=pSelect; p; p=p->p_پیشگی){
    اونچائی_کا_اظہاریہ(p->pWhere, pn_اونچائی);
    اونچائی_کا_اظہاریہ(p->pHaving, pn_اونچائی);
    اونچائی_کا_اظہاریہ(p->p_حد_بندی, pn_اونچائی);
    اونچائی_کا_اظہاریہ_List(p->pEList, pn_اونچائی);
    اونچائی_کا_اظہاریہ_List(p->p_گروہ_By, pn_اونچائی);
    اونچائی_کا_اظہاریہ_List(p->p_ترتیب_By, pn_اونچائی);
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
ساکن خالی اظہاریہ_سیٹ_اونچائی(اظہاریہ *p){
  صحیح n_اونچائی = p->p_بایاں ? p->p_بایاں->n_اونچائی : 0;
  اگر( NEVER(p->p_دایاں) && p->p_دایاں->n_اونچائی>n_اونچائی ){
    n_اونچائی = p->p_دایاں->n_اونچائی;
  }
  اگر( اظہاریہ_Use_XSelect(p) ){
    اونچائی_کا_Select(p->x.pSelect, &n_اونچائی);
  }ورنہ اگر( p->x.pList ){
    اونچائی_کا_اظہاریہ_List(p->x.pList, &n_اونچائی);
    p->flags |= EP_Propagate & sqlite3_اظہاریہ_List_Flags(p->x.pList);
  }
  p->n_اونچائی = n_اونچائی + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
خالی sqlite3_اظہاریہ_سیٹ_اونچائی_And_Flags(Parse *pParse, اظہاریہ *p){
  اگر( pParse->nErr ) واپس;
  اظہاریہ_سیٹ_اونچائی(p);
  sqlite3_اظہاریہ_Check_اونچائی(pParse, p->n_اونچائی);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
صحیح sqlite3_Select_اظہاریہ_اونچائی(ثابت Select *p){
  صحیح n_اونچائی = 0;
  اونچائی_کا_Select(p, &n_اونچائی);
  واپس n_اونچائی;
}
#ورنہ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
خالی sqlite3_اظہاریہ_سیٹ_اونچائی_And_Flags(Parse *pParse, اظہاریہ *p){
  اگر( pParse->nErr ) واپس;
  اگر( p && اظہاریہ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_اظہاریہ_List_Flags(p->x.pList);
  }
}
#تعریف_کرو اظہاریہ_سیٹ_اونچائی(y)
#اگر_ختم /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
خالی sqlite3_اظہاریہ_سیٹ_Error_انحراف(اظہاریہ *p_اظہاریہ, صحیح iOfst){
  اگر( p_اظہاریہ==0 ) واپس;
  اگر( NEVER(اظہاریہ_Use_WJoin(p_اظہاریہ)) ) واپس;
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
  صحیح op,                 /* Expression opcode */
  ثابت Token *pToken,    /* Token argument.  Might be NULL */
  صحیح dequote             /* True to dequote */
){
  اظہاریہ *pNew;
  صحیح nExtra = pToken ? pToken->n+1 : 0;

  أكد( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, حجم(اظہاریہ)+nExtra);
  اگر( pNew ){
    memset(pNew, 0, حجم(اظہاریہ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    اگر( nExtra ){
      أكد( pToken!=0 );
      pNew->u.zToken = (حرف*)&pNew[1];
      أكد( pToken->z!=0 || pToken->n==0 );
      اگر( pToken->n ) حافظہ_نقل(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      اگر( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_اظہاریہ(pNew);
      }
    }
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    pNew->n_اونچائی = 1;
#اگر_ختم 
  }
  واپس pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
اظہاریہ *sqlite3_اظہاریہ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  صحیح op,                 /* Expression opcode */
  ثابت حرف *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  واپس sqlite3_اظہاریہ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
اظہاریہ *sqlite3_اظہاریہ_Int32(sqlite3 *db, صحیح iVal){
  اظہاریہ *pNew = sqlite3DbMallocRawNN(db, حجم(اظہاریہ));
  اگر( pNew ){
    memset(pNew, 0, حجم(اظہاریہ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_قدر|EP_برگ|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_قدر = iVal;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    pNew->n_اونچائی = 1;
#اگر_ختم 
  }
  واپس pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
خالی sqlite3_اظہاریہ_Attach_Subtrees(
  sqlite3 *db,
  اظہاریہ *p_جڑ,
  اظہاریہ *p_بایاں,
  اظہاریہ *p_دایاں
){
  اگر( p_جڑ==0 ){
    أكد( db->mallocFailed );
    sqlite3_اظہاریہ_Delete(db, p_بایاں);
    sqlite3_اظہاریہ_Delete(db, p_دایاں);
  }ورنہ{
    أكد( اظہاریہ_Use_XList(p_جڑ) );
    أكد( p_جڑ->x.pSelect==0 );
    اگر( p_دایاں ){
      p_جڑ->p_دایاں = p_دایاں;
      p_جڑ->flags |= EP_Propagate & p_دایاں->flags;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
      p_جڑ->n_اونچائی = p_دایاں->n_اونچائی+1;
    }ورنہ{
      p_جڑ->n_اونچائی = 1;
#اگر_ختم
    }
    اگر( p_بایاں ){
      p_جڑ->p_بایاں = p_بایاں;
      p_جڑ->flags |= EP_Propagate & p_بایاں->flags;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
      اگر( p_بایاں->n_اونچائی>=p_جڑ->n_اونچائی ){
        p_جڑ->n_اونچائی = p_بایاں->n_اونچائی+1;
      }
#اگر_ختم
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
  صحیح op,                 /* Expression opcode */
  اظہاریہ *p_بایاں,            /* Left operand */
  اظہاریہ *p_دایاں            /* Right operand */
){
  اظہاریہ *p;
  p = sqlite3DbMallocRawNN(pParse->db, حجم(اظہاریہ));
  اگر( p ){
    memset(p, 0, حجم(اظہاریہ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_اظہاریہ_Attach_Subtrees(pParse->db, p, p_بایاں, p_دایاں);
    sqlite3_اظہاریہ_Check_اونچائی(pParse, p->n_اونچائی);
  }ورنہ{
    sqlite3_اظہاریہ_Delete(pParse->db, p_بایاں);
    sqlite3_اظہاریہ_Delete(pParse->db, p_دایاں);
  }
  واپس p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
خالی sqlite3_PExpr_جوڑو_Select(Parse *pParse, اظہاریہ *p_اظہاریہ, Select *pSelect){
  اگر( p_اظہاریہ ){
    p_اظہاریہ->x.pSelect = pSelect;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_xIsSelect|EP_Subquery);
    sqlite3_اظہاریہ_سیٹ_اونچائی_And_Flags(pParse, p_اظہاریہ);
  }ورنہ{
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
Select *sqlite3_اظہاریہ_List_مقصد_اقدار(Parse *pParse, صحیح nElem, اظہاریہ_List *pEList){
  صحیح ii;
  Select *pRet = 0;
  أكد( nElem>1 );
  برائے(ii=0; ii<pEList->n_اظہاریہ; ii++){
    Select *pSel;
    اظہاریہ *p_اظہاریہ = pEList->a[ii].p_اظہاریہ;
    صحیح n_اظہاریہ_Elem;
    اگر( p_اظہاریہ->op==TK_سمتیہ ){
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      n_اظہاریہ_Elem = p_اظہاریہ->x.pList->n_اظہاریہ;
    }ورنہ{
      n_اظہاریہ_Elem = 1;
    }
    اگر( n_اظہاریہ_Elem!=nElem ){
      sqlite3_Error_پیغام(pParse, "IN(...) element has %d term%s - expected %d",
          n_اظہاریہ_Elem, n_اظہاریہ_Elem>1?"s":"", nElem
      );
      توڑو;
    }
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pSel = sqlite3SelectNew(pParse, p_اظہاریہ->x.pList, 0, 0, 0, 0, 0, SF_اقدار,0);
    p_اظہاریہ->x.pList = 0;
    اگر( pSel ){
      اگر( pRet ){
        pSel->op = TK_ALL;
        pSel->p_پیشگی = pRet;
      }
      pRet = pSel;
    }
  }

  اگر( pRet && pRet->p_پیشگی ){
    pRet->selFlags |= SF_Multi_قدر;
  }
  sqlite3_اظہاریہ_List_Delete(pParse->db, pEList);
  واپس pRet;
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
  اگر( p_بایاں==0  ){
    واپس p_دایاں;
  }ورنہ اگر( p_دایاں==0 ){
    واپس p_بایاں;
  }ورنہ{
    u32 f = p_بایاں->flags | p_دایاں->flags;
    اگر( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !اندر_RENAME_OBJECT
    ){
      اظہاریہ *p;
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_بایاں);
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_دایاں);
      p = sqlite3_اظہاریہ_Int32(db, 0);
      اگر( p ) p->op2 = 1;
      واپس p;
    }ورنہ{
      واپس sqlite3PExpr(pParse, TK_AND, p_بایاں, p_دایاں);
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
  صحیح eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  اظہاریہ *pNew;
  sqlite3 *db = pParse->db;
  أكد( pToken );
  pNew = sqlite3_اظہاریہ_Alloc(db, TK_FUNCTION, pToken, 1);
  اگر( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    واپس 0;
  }
  أكد( !اظہاریہ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (صحیح)(pToken->z - pParse->z_دم);
  اگر( pList
   && pList->n_اظہاریہ > pParse->db->a_حد_بندی[SQLITE_حد_بندی_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_پیغام(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  اظہاریہ_سیٹ_Property(pNew, EP_HasFunc);
  أكد( اظہاریہ_Use_XList(pNew) );
  sqlite3_اظہاریہ_سیٹ_اونچائی_And_Flags(pParse, pNew);
  اگر( eDistinct==SF_Distinct ) اظہاریہ_سیٹ_Property(pNew, EP_Distinct);
  واپس pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
خالی sqlite3_اظہاریہ_ترتیب_By_Aggregate_Error(Parse *pParse, اظہاریہ *p){
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
خالی sqlite3_اظہاریہ_جوڑو_Function_ترتیب_By(
  Parse *pParse,        /* Parsing context */
  اظہاریہ *p_اظہاریہ,          /* The function call to which ORDER BY is to be added */
  اظہاریہ_List *p_ترتیب_By    /* The ORDER BY clause to add */
){
  اظہاریہ *pOB;
  sqlite3 *db = pParse->db;
  اگر( NEVER(p_ترتیب_By==0) ){
    أكد( db->mallocFailed );
    واپس;
  }
  اگر( p_اظہاریہ==0 ){
    أكد( db->mallocFailed );
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    واپس;
  }
  أكد( p_اظہاریہ->op==TK_FUNCTION );
  أكد( p_اظہاریہ->p_بایاں==0 );
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  اگر( p_اظہاریہ->x.pList==0 || NEVER(p_اظہاریہ->x.pList->n_اظہاریہ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_List_Delete_Generic, p_ترتیب_By);
    واپس;
  }
  اگر( Is_کھڑکی_Func(p_اظہاریہ) ){
    sqlite3_اظہاریہ_ترتیب_By_Aggregate_Error(pParse, p_اظہاریہ);
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    واپس;
  }
  اگر( p_ترتیب_By->n_اظہاریہ>db->a_حد_بندی[SQLITE_حد_بندی_عمود] ){
    sqlite3_Error_پیغام(pParse, "too many terms in ORDER BY clause");
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    واپس;
  }

  pOB = sqlite3_اظہاریہ_Alloc(db, TK_ترتیب, 0, 0);
  اگر( pOB==0 ){
    sqlite3_اظہاریہ_List_Delete(db, p_ترتیب_By);
    واپس;
  }
  pOB->x.pList = p_ترتیب_By;
  أكد( اظہاریہ_Use_XList(pOB) );
  p_اظہاریہ->p_بایاں = pOB;
  اظہاریہ_سیٹ_Property(pOB, EP_بھرا_سائز);
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
خالی sqlite3_اظہاریہ_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ثابت اظہاریہ *p_اظہاریہ,     /* The function invocation */
  ثابت FuncDef *pDef    /* The function being invoked */
){
  أكد( !اندر_RENAME_OBJECT );
  أكد( (pDef->funcFlags & (SQLITE_FUNC_براہ_راست|SQLITE_FUNC_UNSAFE))!=0 );
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    اگر( (pDef->funcFlags & SQLITE_FUNC_براہ_راست)!=0
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
خالی sqlite3_اظہاریہ_تفویض_Var_نمبر(Parse *pParse, اظہاریہ *p_اظہاریہ, u32 n){
  sqlite3 *db = pParse->db;
  ثابت حرف *z;
  ynVar x;

  اگر( p_اظہاریہ==0 ) واپس;
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر|EP_Reduced|EP_TokenOnly) );
  z = p_اظہاریہ->u.zToken;
  أكد( z!=0 );
  أكد( z[0]!=0 );
  أكد( n==(u32)sqlite3Strlen30(z) );
  اگر( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    أكد( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ورنہ{
    صحیح do_جوڑو = 0;
    اگر( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      صحیح bOk;
      اگر( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ورنہ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر]-1 );
      testcase( i==db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] );
      اگر( bOk==0 || i<1 || i>db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] ){
        sqlite3_Error_پیغام(pParse, "variable number must be between ?1 and ?%d",
            db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر]);
        sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, p_اظہاریہ);
        واپس;
      }
      x = (ynVar)i;
      اگر( x>pParse->nVar ){
        pParse->nVar = (صحیح)x;
        do_جوڑو = 1;
      }ورنہ اگر( x<SQLITE_VNBMC*64 ){
        do_جوڑو = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ورنہ اگر( sqlite3_VList_عدد_مقصد_Name(pParse->pVList, x)==0 ){
        do_جوڑو = 1;
      }
    }ورنہ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_مقصد_عدد(pParse->pVList, z, n);
      اگر( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_جوڑو = 1;
      }
    }
    اگر( do_جوڑو ){
      pParse->pVList = sqlite3_VList_جوڑو(db, pParse->pVList, z, n, x);
      اگر( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_اظہاریہ->i_عمود = x;
  اگر( x>db->a_حد_بندی[SQLITE_حد_بندی_VARIABLE_نمبر] ){
    sqlite3_Error_پیغام(pParse, "too many SQL variables");
    sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, p_اظہاریہ);
  }
}

/*
** Recursively delete an expression tree.
*/
ساکن SQLITE_NOINLINE خالی sqlite3_اظہاریہ_Delete_NN(sqlite3 *db, اظہاریہ *p){
  أكد( p!=0 );
  أكد( db!=0 );
اظہاریہ_Delete_Restart:
  أكد( !اظہاریہ_Use_UValue(p) || p->u.i_قدر>=0 );
  أكد( !اظہاریہ_Use_YWin(p) || !اظہاریہ_Use_YSub(p) );
  أكد( !اظہاریہ_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  أكد( p->op!=TK_FUNCTION || !اظہاریہ_Use_YSub(p) );
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( اظہاریہ_Has_Property(p, EP_برگ) && !اظہاریہ_Has_Property(p, EP_TokenOnly) ){
    أكد( p->p_بایاں==0 );
    أكد( p->p_دایاں==0 );
    أكد( !اظہاریہ_Use_XSelect(p) || p->x.pSelect==0 );
    أكد( !اظہاریہ_Use_XList(p) || p->x.pList==0 );
  }
#اگر_ختم
  اگر( !اظہاریہ_Has_Property(p, (EP_TokenOnly|EP_برگ)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    أكد( (اظہاریہ_Use_XList(p) && p->x.pList==0) || p->p_دایاں==0 );
    اگر( p->p_دایاں ){
      أكد( !اظہاریہ_Has_Property(p, EP_WinFunc) );
      sqlite3_اظہاریہ_Delete_NN(db, p->p_دایاں);
    }ورنہ اگر( اظہاریہ_Use_XSelect(p) ){
      أكد( !اظہاریہ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ورنہ{
      sqlite3_اظہاریہ_List_Delete(db, p->x.pList);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( اظہاریہ_Has_Property(p, EP_WinFunc) ){
        sqlite3_کھڑکی_Delete(db, p->y.pWin);
      }
#اگر_ختم
    }
    اگر( p->p_بایاں && p->op!=TK_SELECT_عمود ){
      اظہاریہ *p_بایاں = p->p_بایاں;
      اگر( !اظہاریہ_Has_Property(p, EP_Static)
       && !اظہاریہ_Has_Property(p_بایاں, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_بایاں;
        جاؤ اظہاریہ_Delete_Restart;
      }ورنہ{
        sqlite3_اظہاریہ_Delete_NN(db, p_بایاں);
      }
    }
  }
  اگر( !اظہاریہ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
خالی sqlite3_اظہاریہ_Delete(sqlite3 *db, اظہاریہ *p){
  اگر( p ) sqlite3_اظہاریہ_Delete_NN(db, p);
}
خالی sqlite3_اظہاریہ_Delete_Generic(sqlite3 *db, خالی *p){
  اگر( ALWAYS(p) ) sqlite3_اظہاریہ_Delete_NN(db, (اظہاریہ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
خالی sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  اگر( p==0 ){
    /* Nothing to clear */
  }ورنہ اگر( p->pOn ){
    sqlite3_اظہاریہ_Delete_NN(db, p->pOn);
  }ورنہ اگر( p->pUsing ){
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
صحیح sqlite3_اظہاریہ_Deferred_Delete(Parse *pParse, اظہاریہ *p_اظہاریہ){
  واپس 0==sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_Delete_Generic, p_اظہاریہ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
خالی sqlite3_اظہاریہ_Unmap_And_Delete(Parse *pParse, اظہاریہ *p){
  اگر( p ){
    اگر( اندر_RENAME_OBJECT ){
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
ساکن صحیح اظہاریہ_Struct_سائز(ثابت اظہاریہ *p){
  اگر( اظہاریہ_Has_Property(p, EP_TokenOnly) ) واپس اظہاریہ_TOKENONLYSIZE;
  اگر( اظہاریہ_Has_Property(p, EP_Reduced) ) واپس اظہاریہ_REDUCEDSIZE;
  واپس اظہاریہ_FULLSIZE;
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
ساکن صحیح duped_اظہاریہ_Struct_سائز(ثابت اظہاریہ *p, صحیح flags){
  صحیح n_سائز;
  أكد( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  أكد( اظہاریہ_FULLSIZE<=0xfff );
  أكد( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  اگر( 0==flags || اظہاریہ_Has_Property(p, EP_بھرا_سائز) ){
    n_سائز = اظہاریہ_FULLSIZE;
  }ورنہ{
    أكد( !اظہاریہ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    أكد( !اظہاریہ_Has_Property(p, EP_OuterON) );
    أكد( !اظہاریہ_Has_VVAProperty(p, EP_NoReduce) );
    اگر( p->p_بایاں || p->x.pList ){
      n_سائز = اظہاریہ_REDUCEDSIZE | EP_Reduced;
    }ورنہ{
      أكد( p->p_دایاں==0 );
      n_سائز = اظہاریہ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  واپس n_سائز;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ساکن صحیح duped_اظہاریہ_گرہ_سائز(ثابت اظہاریہ *p, صحیح flags){
  صحیح nByte = duped_اظہاریہ_Struct_سائز(p, flags) & 0xfff;
  اگر( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  واپس ROUND8(nByte);
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
ساکن صحیح duped_اظہاریہ_سائز(ثابت اظہاریہ *p){
  صحیح nByte;
  أكد( p!=0 );
  nByte = duped_اظہاریہ_گرہ_سائز(p, EXPRDUP_REDUCE);
  اگر( p->p_بایاں ) nByte += duped_اظہاریہ_سائز(p->p_بایاں);
  اگر( p->p_دایاں ) nByte += duped_اظہاریہ_سائز(p->p_دایاں);
  أكد( nByte==ROUND8(nByte) );
  واپس nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
قسمتعریف ساخت EdupBuf EdupBuf;
ساخت EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#اگر_تعریف_شدہ SQLITE_DEBUG
  u8 *z_انتہا;            /* First byte past the end of memory */
#اگر_ختم
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ساکن اظہاریہ *اظہاریہ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ثابت اظہاریہ *p,        /* Expr tree to be duplicated */
  صحیح dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  اظہاریہ *pNew;           /* Value to return */
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
#اگر_تعریف_شدہ SQLITE_DEBUG
    sEdupBuf.z_انتہا = pEdupBuf->z_انتہا;
#اگر_ختم
    staticFlag = EP_Static;
    أكد( sEdupBuf.zAlloc!=0 );
    أكد( dupFlags==EXPRDUP_REDUCE );
  }ورنہ{
    صحیح nAlloc;
    اگر( dupFlags ){
      nAlloc = duped_اظہاریہ_سائز(p);
    }ورنہ اگر( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(اظہاریہ_FULLSIZE + nToken);
    }ورنہ{
      nToken = 0;
      nAlloc = ROUND8(اظہاریہ_FULLSIZE);
    }
    أكد( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#اگر_تعریف_شدہ SQLITE_DEBUG
    sEdupBuf.z_انتہا = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#اگر_ختم
    
    staticFlag = 0;
  }
  pNew = (اظہاریہ *)sEdupBuf.zAlloc;
  أكد( EIGHT_BYTE_ALIGNMENT(pNew) );

  اگر( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ثابت غیرعلامتی n_Struct_سائز = duped_اظہاریہ_Struct_سائز(p, dupFlags);
    صحیح n_New_سائز = n_Struct_سائز & 0xfff;
    اگر( nToken<0 ){
      اگر( !اظہاریہ_Has_Property(p, EP_Int_قدر) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ورنہ{
        nToken = 0;
      }
    }
    اگر( dupFlags ){
      أكد( (صحیح)(sEdupBuf.z_انتہا - sEdupBuf.zAlloc) >= n_New_سائز+nToken );
      أكد( اظہاریہ_Has_Property(p, EP_Reduced)==0 );
      حافظہ_نقل(sEdupBuf.zAlloc, p, n_New_سائز);
    }ورنہ{
      u32 n_سائز = (u32)اظہاریہ_Struct_سائز(p);
      أكد( (صحیح)(sEdupBuf.z_انتہا - sEdupBuf.zAlloc) >=
                                                   (صحیح)اظہاریہ_FULLSIZE+nToken );
      حافظہ_نقل(sEdupBuf.zAlloc, p, n_سائز);
      اگر( n_سائز<اظہاریہ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_سائز], 0, اظہاریہ_FULLSIZE-n_سائز);
      }
      n_New_سائز = اظہاریہ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_سائز & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    اظہاریہ_Clear_VVAProperties(pNew);
    اگر( dupFlags ){
      اظہاریہ_سیٹ_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    أكد( nToken>=0 );
    اگر( nToken>0 ){
      حرف *zToken = pNew->u.zToken = (حرف*)&sEdupBuf.zAlloc[n_New_سائز];
      حافظہ_نقل(zToken, p->u.zToken, nToken);
      n_New_سائز += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_سائز);

    اگر( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_برگ))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      اگر( اظہاریہ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ورنہ{
        pNew->x.pList = sqlite3_اظہاریہ_List_Dup(db, p->x.pList,
                           p->op!=TK_ترتیب ? dupFlags : 0);
      }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( اظہاریہ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_کھڑکی_Dup(db, pNew, p->y.pWin);
        أكد( اظہاریہ_Has_Property(pNew, EP_WinFunc) );
      }
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      اگر( dupFlags ){
        اگر( p->op==TK_SELECT_عمود ){
          pNew->p_بایاں = p->p_بایاں;
          أكد( p->p_دایاں==0 
               || p->p_دایاں==p->p_بایاں
               || اظہاریہ_Has_Property(p->p_بایاں, EP_Subquery) );
        }ورنہ{
          pNew->p_بایاں = p->p_بایاں ?
                      اظہاریہ_Dup(db, p->p_بایاں, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_دایاں = p->p_دایاں ?
                       اظہاریہ_Dup(db, p->p_دایاں, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ورنہ{
        اگر( p->op==TK_SELECT_عمود ){
          pNew->p_بایاں = p->p_بایاں;
          أكد( p->p_دایاں==0 
               || p->p_دایاں==p->p_بایاں
               || اظہاریہ_Has_Property(p->p_بایاں, EP_Subquery) );
        }ورنہ{
          pNew->p_بایاں = sqlite3_اظہاریہ_Dup(db, p->p_بایاں, 0);
        }
        pNew->p_دایاں = sqlite3_اظہاریہ_Dup(db, p->p_دایاں, 0);
      }
    }
  }
  اگر( pEdupBuf ) حافظہ_نقل(pEdupBuf, &sEdupBuf, حجم(sEdupBuf));
  أكد( sEdupBuf.zAlloc <= sEdupBuf.z_انتہا );
  واپس pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  اگر( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    اگر( pRet ){
      صحیح i;
      pRet->nCte = p->nCte;
      برائے(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].p_ستون_تعداد = sqlite3_اظہاریہ_List_Dup(db, p->a[i].p_ستون_تعداد, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  واپس pRet;
}
#ورنہ
# define sqlite3WithDup(x,y) 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ساکن صحیح gatherSelectWindowsCallback(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_FUNCTION && اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    کھڑکی *pWin = p_اظہاریہ->y.pWin;
    أكد( pWin );
    أكد( Is_کھڑکی_Func(p_اظہاریہ) );
    أكد( pWin->ppThis==0 );
    sqlite3_کھڑکی_Link(pSelect, pWin);
  }
  واپس WRC_Continue;
}
ساکن صحیح gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  واپس p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ساکن خالی gatherSelectWindows(Select *p){
  Walker w;
  w.x_اظہاریہ_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#اگر_ختم


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
اظہاریہ *sqlite3_اظہاریہ_Dup(sqlite3 *db, ثابت اظہاریہ *p, صحیح flags){
  أكد( flags==0 || flags==EXPRDUP_REDUCE );
  واپس p ? اظہاریہ_Dup(db, p, flags, 0) : 0;
}
اظہاریہ_List *sqlite3_اظہاریہ_List_Dup(sqlite3 *db, ثابت اظہاریہ_List *p, صحیح flags){
  اظہاریہ_List *pNew;
  ساخت اظہاریہ_List_شے *p_شے;
  ثابت ساخت اظہاریہ_List_شے *p_Old_شے;
  صحیح i;
  اظہاریہ *p_پیشگی_Select_ستون_Old = 0;
  اظہاریہ *p_پیشگی_Select_ستون_New = 0;
  أكد( db!=0 );
  اگر( p==0 ) واپس 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_سائز(db, p));
  اگر( pNew==0 ) واپس 0;
  pNew->n_اظہاریہ = p->n_اظہاریہ;
  pNew->nAlloc = p->nAlloc;
  p_شے = pNew->a;
  p_Old_شے = p->a;
  برائے(i=0; i<p->n_اظہاریہ; i++, p_شے++, p_Old_شے++){
    اظہاریہ *p_Old_اظہاریہ = p_Old_شے->p_اظہاریہ;
    اظہاریہ *p_New_اظہاریہ;
    p_شے->p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_Old_اظہاریہ, flags);
    اگر( p_Old_اظہاریہ
     && p_Old_اظہاریہ->op==TK_SELECT_عمود
     && (p_New_اظہاریہ = p_شے->p_اظہاریہ)!=0
    ){
      اگر( p_New_اظہاریہ->p_دایاں ){
        p_پیشگی_Select_ستون_Old = p_Old_اظہاریہ->p_دایاں;
        p_پیشگی_Select_ستون_New = p_New_اظہاریہ->p_دایاں;
        p_New_اظہاریہ->p_بایاں = p_New_اظہاریہ->p_دایاں;
      }ورنہ{
        اگر( p_Old_اظہاریہ->p_بایاں!=p_پیشگی_Select_ستون_Old ){
          p_پیشگی_Select_ستون_Old = p_Old_اظہاریہ->p_بایاں;
          p_پیشگی_Select_ستون_New = sqlite3_اظہاریہ_Dup(db, p_پیشگی_Select_ستون_Old, flags);
          p_New_اظہاریہ->p_دایاں = p_پیشگی_Select_ستون_New;
        }
        p_New_اظہاریہ->p_بایاں = p_پیشگی_Select_ستون_New;
      }
    }
    p_شے->zEName = sqlite3DbStrDup(db, p_Old_شے->zEName);
    p_شے->fg = p_Old_شے->fg;
    p_شے->u = p_Old_شے->u;
  }
  واپس pNew;
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
  اگر( p==0 ) واپس 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  اگر( pNew==0 ) واپس 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  برائے(i=0; i<p->nSrc; i++){
    Src_شے *p_New_شے = &pNew->a[i];
    ثابت Src_شے *p_Old_شے = &p->a[i];
    جدول *pTab;
    p_New_شے->fg = p_Old_شے->fg;
    اگر( p_Old_شے->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, حجم(Subquery));
      اگر( pNewSubq==0 ){
        أكد( db->mallocFailed );
        p_New_شے->fg.isSubquery = 0;
      }ورنہ{
        حافظہ_نقل(pNewSubq, p_Old_شے->u4.pSubq, حجم(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        اگر( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_شے->fg.isSubquery = 0;
        }
      }
      p_New_شے->u4.pSubq = pNewSubq;
    }ورنہ اگر( p_Old_شے->fg.fixedSchema ){
      p_New_شے->u4.pSchema = p_Old_شے->u4.pSchema;
    }ورنہ{
      p_New_شے->u4.zDatabase = sqlite3DbStrDup(db, p_Old_شے->u4.zDatabase);
    }
    p_New_شے->zName = sqlite3DbStrDup(db, p_Old_شے->zName);
    p_New_شے->zAlias = sqlite3DbStrDup(db, p_Old_شے->zAlias);
    p_New_شے->iCursor = p_Old_شے->iCursor;
    اگر( p_New_شے->fg.is_اشاریہ_دار_By ){
      p_New_شے->u1.z_اشاریہ_دار_By = sqlite3DbStrDup(db, p_Old_شے->u1.z_اشاریہ_دار_By);
    }ورنہ اگر( p_New_شے->fg.isTabFunc ){
      p_New_شے->u1.pFuncArg =
          sqlite3_اظہاریہ_List_Dup(db, p_Old_شے->u1.pFuncArg, flags);
    }ورنہ{
      p_New_شے->u1.n_صف = p_Old_شے->u1.n_صف;
    }
    p_New_شے->u2 = p_Old_شے->u2;
    اگر( p_New_شے->fg.isCte ){
      p_New_شے->u2.pCteUse->nUse++;
    }
    pTab = p_New_شے->pSTab = p_Old_شے->pSTab;
    اگر( pTab ){
      pTab->nTabRef++;
    }
    اگر( p_Old_شے->fg.isUsing ){
      أكد( p_New_شے->fg.isUsing );
      p_New_شے->u3.pUsing = sqlite3IdListDup(db, p_Old_شے->u3.pUsing);
    }ورنہ{
      p_New_شے->u3.pOn = sqlite3_اظہاریہ_Dup(db, p_Old_شے->u3.pOn, flags);
    }
    p_New_شے->ستون_مستعمل = p_Old_شے->ستون_مستعمل;
  }
  واپس pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ثابت IdList *p){
  IdList *pNew;
  صحیح i;
  أكد( db!=0 );
  اگر( p==0 ) واپس 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  اگر( pNew==0 ) واپس 0;
  pNew->nId = p->nId;
  برائے(i=0; i<p->nId; i++){
    ساخت Id_List_شے *p_New_شے = &pNew->a[i];
    ثابت ساخت Id_List_شے *p_Old_شے = &p->a[i];
    p_New_شے->zName = sqlite3DbStrDup(db, p_Old_شے->zName);
  }
  واپس pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *pDup, صحیح flags){
  Select *pRet = 0;
  Select *p_اگلا = 0;
  Select **pp = &pRet;
  ثابت Select *p;

  أكد( db!=0 );
  برائے(p=pDup; p; p=p->p_پیشگی){
    Select *pNew = sqlite3DbMallocRawNN(db, حجم(*p) );
    اگر( pNew==0 ) توڑو;
    pNew->pEList = sqlite3_اظہاریہ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_اظہاریہ_Dup(db, p->pWhere, flags);
    pNew->p_گروہ_By = sqlite3_اظہاریہ_List_Dup(db, p->p_گروہ_By, flags);
    pNew->pHaving = sqlite3_اظہاریہ_Dup(db, p->pHaving, flags);
    pNew->p_ترتیب_By = sqlite3_اظہاریہ_List_Dup(db, p->p_ترتیب_By, flags);
    pNew->op = p->op;
    pNew->p_اگلا = p_اگلا;
    pNew->p_پیشگی = 0;
    pNew->p_حد_بندی = sqlite3_اظہاریہ_Dup(db, p->p_حد_بندی, flags);
    pNew->i_حد_بندی = 0;
    pNew->i_انحراف = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_صف = p->n_Select_صف;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_کھڑکی_List_Dup(db, p->pWinDefn);
    اگر( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#اگر_ختم
    pNew->selId = p->selId;
    اگر( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_اگلا = 0;
      sqlite3SelectDelete(db, pNew);
      توڑو;
    }
    *pp = pNew;
    pp = &pNew->p_پیشگی;
    p_اگلا = pNew;
  }
  واپس pRet;
}
#ورنہ
Select *sqlite3SelectDup(sqlite3 *db, ثابت Select *p, صحیح flags){
  أكد( p==0 );
  واپس 0;
}
#اگر_ختم


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
ساکن ثابت ساخت اظہاریہ_List_شے zero_شے = {0};
SQLITE_NOINLINE اظہاریہ_List *sqlite3_اظہاریہ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  ساخت اظہاریہ_List_شے *p_شے;
  اظہاریہ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  اگر( pList==0 ){
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
    واپس 0;
  }
  pList->nAlloc = 4;
  pList->n_اظہاریہ = 1;
  p_شے = &pList->a[0];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
  واپس pList;
}
SQLITE_NOINLINE اظہاریہ_List *sqlite3_اظہاریہ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  اظہاریہ_List *pList,        /* List to which to append. Might be NULL */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  ساخت اظہاریہ_List_شے *p_شے;
  اظہاریہ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  اگر( pNew==0 ){
    sqlite3_اظہاریہ_List_Delete(db, pList);
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
    واپس 0;
  }ورنہ{
    pList = pNew;
  }
  p_شے = &pList->a[pList->n_اظہاریہ++];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
  واپس pList;
}
اظہاریہ_List *sqlite3_اظہاریہ_List_Append(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to append. Might be NULL */
  اظہاریہ *p_اظہاریہ             /* Expression to be appended. Might be NULL */
){
  ساخت اظہاریہ_List_شے *p_شے;
  اگر( pList==0 ){
    واپس sqlite3_اظہاریہ_List_Append_New(pParse->db,p_اظہاریہ);
  }
  اگر( pList->nAlloc<pList->n_اظہاریہ+1 ){
    واپس sqlite3_اظہاریہ_List_Append_Grow(pParse->db,pList,p_اظہاریہ);
  }
  p_شے = &pList->a[pList->n_اظہاریہ++];
  *p_شے = zero_شے;
  p_شے->p_اظہاریہ = p_اظہاریہ;
  واپس pList;
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
  صحیح n;
  صحیح i;
  صحیح i_پہلا = pList ? pList->n_اظہاریہ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  اگر( NEVER(pColumns==0) ) جاؤ سمتیہ_append_error;
  اگر( p_اظہاریہ==0 ) جاؤ سمتیہ_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  اگر( p_اظہاریہ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ)) ){
    sqlite3_Error_پیغام(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    جاؤ سمتیہ_append_error;
  }

  برائے(i=0; i<pColumns->nId; i++){
    اظہاریہ *p_Sub_اظہاریہ = sqlite3_اظہاریہ_For_سمتیہ_میدان(pParse, p_اظہاریہ, i, pColumns->nId);
    أكد( p_Sub_اظہاریہ!=0 || db->mallocFailed );
    اگر( p_Sub_اظہاریہ==0 ) جاری;
    pList = sqlite3_اظہاریہ_List_Append(pParse, pList, p_Sub_اظہاریہ);
    اگر( pList ){
      أكد( pList->n_اظہاریہ==i_پہلا+i+1 );
      pList->a[pList->n_اظہاریہ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  اگر( !db->mallocFailed && p_اظہاریہ->op==TK_SELECT && ALWAYS(pList!=0) ){
    اظہاریہ *p_پہلا = pList->a[i_پہلا].p_اظہاریہ;
    أكد( p_پہلا!=0 );
    أكد( p_پہلا->op==TK_SELECT_عمود );
    
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
  واپس pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
خالی sqlite3_اظہاریہ_List_سیٹ_ترتیب_دو_ترتیب(اظہاریہ_List *p, صحیح i_ترتیب_دو_ترتیب, صحیح eNulls){
  ساخت اظہاریہ_List_شے *p_شے;
  اگر( p==0 ) واپس;
  أكد( p->n_اظہاریہ>0 );

  أكد( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  أكد( i_ترتیب_دو_ترتیب==SQLITE_SO_UNDEFINED
       || i_ترتیب_دو_ترتیب==SQLITE_SO_ASC
       || i_ترتیب_دو_ترتیب==SQLITE_SO_DESC
  );
  أكد( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_شے = &p->a[p->n_اظہاریہ-1];
  أكد( p_شے->fg.bNulls==0 );
  اگر( i_ترتیب_دو_ترتیب==SQLITE_SO_UNDEFINED ){
    i_ترتیب_دو_ترتیب = SQLITE_SO_ASC;
  }
  p_شے->fg.ترتیب_دو_Flags = (u8)i_ترتیب_دو_ترتیب;

  اگر( eNulls!=SQLITE_SO_UNDEFINED ){
    p_شے->fg.bNulls = 1;
    اگر( i_ترتیب_دو_ترتیب!=eNulls ){
      p_شے->fg.ترتیب_دو_Flags |= KEYINFO_ترتیب_BIGNULL;
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
خالی sqlite3_اظہاریہ_List_سیٹ_Name(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to add the span. */
  ثابت Token *pName,     /* Name to be added */
  صحیح dequote             /* True to cause the name to be dequoted */
){
  أكد( pList!=0 || pParse->db->mallocFailed!=0 );
  أكد( pParse->e_Parse_منوال!=PARSE_منوال_UNMAP || dequote==0 );
  اگر( pList ){
    ساخت اظہاریہ_List_شے *p_شے;
    أكد( pList->n_اظہاریہ>0 );
    p_شے = &pList->a[pList->n_اظہاریہ-1];
    أكد( p_شے->zEName==0 );
    أكد( p_شے->fg.eEName==ENAME_NAME );
    p_شے->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    اگر( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_شے->zEName);
      اگر( اندر_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ثابت خالی*)p_شے->zEName, pName);
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
خالی sqlite3_اظہاریہ_List_سیٹ_وسعت(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pList,        /* List to which to add the span. */
  ثابت حرف *z_آغاز,     /* Start of the span */
  ثابت حرف *z_انتہا        /* End of the span */
){
  sqlite3 *db = pParse->db;
  أكد( pList!=0 || db->mallocFailed!=0 );
  اگر( pList ){
    ساخت اظہاریہ_List_شے *p_شے = &pList->a[pList->n_اظہاریہ-1];
    أكد( pList->n_اظہاریہ>0 );
    اگر( p_شے->zEName==0 ){
      p_شے->zEName = sqlite3_Db_وسعت_Dup(db, z_آغاز, z_انتہا);
      p_شے->fg.eEName = ENAME_وسعت;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
خالی sqlite3_اظہاریہ_List_Check_لمبائی(
  Parse *pParse,
  اظہاریہ_List *pEList,
  ثابت حرف *zObject
){
  صحیح mx = pParse->db->a_حد_بندی[SQLITE_حد_بندی_عمود];
  testcase( pEList && pEList->n_اظہاریہ==mx );
  testcase( pEList && pEList->n_اظہاریہ==mx+1 );
  اگر( pEList && pEList->n_اظہاریہ>mx ){
    sqlite3_Error_پیغام(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ساکن SQLITE_NOINLINE خالی اظہاریہ_List_Delete_NN(sqlite3 *db, اظہاریہ_List *pList){
  صحیح i = pList->n_اظہاریہ;
  ساخت اظہاریہ_List_شے *p_شے =  pList->a;
  أكد( pList->n_اظہاریہ>0 );
  أكد( db!=0 );
  کرو{
    sqlite3_اظہاریہ_Delete(db, p_شے->p_اظہاریہ);
    اگر( p_شے->zEName ) sqlite3DbNNFreeNN(db, p_شے->zEName);
    p_شے++;
  }جب( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
خالی sqlite3_اظہاریہ_List_Delete(sqlite3 *db, اظہاریہ_List *pList){
  اگر( pList ) اظہاریہ_List_Delete_NN(db, pList);
}
خالی sqlite3_اظہاریہ_List_Delete_Generic(sqlite3 *db, خالی *pList){
  اگر( ALWAYS(pList) ) اظہاریہ_List_Delete_NN(db, (اظہاریہ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_اظہاریہ_List_Flags(ثابت اظہاریہ_List *pList){
  صحیح i;
  u32 m = 0;
  أكد( pList!=0 );
  برائے(i=0; i<pList->n_اظہاریہ; i++){
     اظہاریہ *p_اظہاریہ = pList->a[i].p_اظہاریہ;
     أكد( p_اظہاریہ!=0 );
     m |= p_اظہاریہ->flags;
  }
  واپس m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
صحیح sqlite3SelectWalkFail(Walker *pWalker, Select *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  pWalker->e_رمز = 0;
  واپس WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ثابت حرف *z_اندر){
  اگر( sqlite3StrICmp(z_اندر, "true")==0  ) واپس EP_IsTrue;
  اگر( sqlite3StrICmp(z_اندر, "false")==0 ) واپس EP_IsFalse;
  واپس 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
صحیح sqlite3_اظہاریہ_Id_مقصد_True_False(اظہاریہ *p_اظہاریہ){
  u32 v;
  أكد( p_اظہاریہ->op==TK_ID || p_اظہاریہ->op==TK_STRING );
  اگر( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Quoted|EP_Int_قدر)
   && (v = sqlite3IsTrueOrFalse(p_اظہاریہ->u.zToken))!=0
  ){
    p_اظہاریہ->op = TK_TRUEFALSE;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, v);
    واپس 1;
  }
  واپس 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
صحیح sqlite3_اظہاریہ_Truth_قدر(ثابت اظہاریہ *p_اظہاریہ){
  p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely((اظہاریہ*)p_اظہاریہ);
  أكد( p_اظہاریہ->op==TK_TRUEFALSE );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
  أكد( sqlite3StrICmp(p_اظہاریہ->u.zToken,"true")==0
       || sqlite3StrICmp(p_اظہاریہ->u.zToken,"false")==0 );
  واپس p_اظہاریہ->u.zToken[4]==0;
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
  اگر( p_اظہاریہ->op==TK_AND || p_اظہاریہ->op==TK_OR ){
    اظہاریہ *p_دایاں = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ->p_دایاں);
    اظہاریہ *p_بایاں = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ->p_بایاں);
    اگر( اظہاریہ_Always_True(p_بایاں) || اظہاریہ_Always_False(p_دایاں) ){
      p_اظہاریہ = p_اظہاریہ->op==TK_AND ? p_دایاں : p_بایاں;
    }ورنہ اگر( اظہاریہ_Always_True(p_دایاں) || اظہاریہ_Always_False(p_بایاں) ){
      p_اظہاریہ = p_اظہاریہ->op==TK_AND ? p_بایاں : p_دایاں;
    }
  }
  واپس p_اظہاریہ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ساکن صحیح اظہاریہ_Eval_Rhs_پہلا(اظہاریہ *p_اظہاریہ){
  اگر( اظہاریہ_Has_Property(p_اظہاریہ->p_بایاں, EP_Subquery)
   && !اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery)
  ){
    واپس 1;
  }ورنہ{
    واپس 0;
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
ساکن صحیح اظہاریہ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  اظہاریہ *p_اظہاریہ,       /* The comparison expression */
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
  اگر( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) && sqlite3_اظہاریہ_Can_Be_Null(p_اظہاریہ->p_دایاں) ){
    r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, pFree2);
    addrIsNull = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ورنہ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, pFree1);
  اگر( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    اگر( اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery)
     && sqlite3_اظہاریہ_Can_Be_Null(p_اظہاریہ->p_بایاں)
    ){
      addrIsNull = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  واپس addrIsNull;
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
ساکن SQLITE_NOINLINE صحیح اظہاریہ_گرہ_Is_Constant_Function(
  Walker *pWalker,
  اظہاریہ *p_اظہاریہ
){
  صحیح n;             /* Number of arguments */
  اظہاریہ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  أكد( p_اظہاریہ->op==TK_FUNCTION );
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly)
   || (pList = p_اظہاریہ->x.pList)==0
  ){;
    n = 0;
  }ورنہ{
    n = pList->n_اظہاریہ;
    sqlite3_Walk_اظہاریہ_List(pWalker, pList);
    اگر( pWalker->e_رمز==0 ) واپس WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_ڈھونڈو_Function(db, p_اظہاریہ->u.zToken, n, ENC(db), 0);
  اگر( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc))
  ){
    pWalker->e_رمز = 0;
    واپس WRC_Abort;
  }
  واپس WRC_Prune;
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
ساکن صحیح اظہاریہ_گرہ_Is_Constant(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  أكد( pWalker->e_رمز>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  اگر( pWalker->e_رمز==2 && اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ){
    pWalker->e_رمز = 0;
    واپس WRC_Abort;
  }

  انتخاب( p_اظہاریہ->op ){
    حالت TK_FUNCTION:
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
        pWalker->e_رمز = 0;  /* Window functions are never constant */
        واپس WRC_Abort;
      }ورنہ اگر( pWalker->e_رمز>=4 && pWalker->e_رمز<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        اگر( pWalker->e_رمز==5 ) اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_FromDDL);
        واپس WRC_Continue;
      }ورنہ اگر( اظہاریہ_Has_Property(p_اظہاریہ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        واپس WRC_Continue;
      }ورنہ اگر( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        واپس اظہاریہ_گرہ_Is_Constant_Function(pWalker, p_اظہاریہ);
      }ورنہ{
        pWalker->e_رمز = 0;  /* Function call is not constant */
        واپس WRC_Abort;
      }
    حالت TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      اگر( sqlite3_اظہاریہ_Id_مقصد_True_False(p_اظہاریہ) ){
        واپس WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    حالت TK_عمود:
    حالت TK_AGG_FUNCTION:
    حالت TK_AGG_عمود:
      testcase( p_اظہاریہ->op==TK_ID );
      testcase( p_اظہاریہ->op==TK_عمود );
      testcase( p_اظہاریہ->op==TK_AGG_FUNCTION );
      testcase( p_اظہاریہ->op==TK_AGG_عمود );
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون) && pWalker->e_رمز!=2 ){
        واپس WRC_Continue;
      }
      اگر( pWalker->e_رمز==3 && p_اظہاریہ->i_جدول==pWalker->u.iCur ){
        واپس WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    حالت TK_IF_NULL_صف:
    حالت TK_رجسٹر:
    حالت TK_DOT:
    حالت TK_RAISE:
      testcase( p_اظہاریہ->op==TK_رجسٹر );
      testcase( p_اظہاریہ->op==TK_IF_NULL_صف );
      testcase( p_اظہاریہ->op==TK_DOT );
      testcase( p_اظہاریہ->op==TK_RAISE );
      pWalker->e_رمز = 0;
      واپس WRC_Abort;
    حالت TK_IS:
    حالت TK_ISNOT:
      اگر( pWalker->e_رمز==6 ){
        pWalker->e_رمز = 0;
        واپس WRC_Abort;
      }
      واپس WRC_Continue;
    حالت TK_VARIABLE:
      اگر( pWalker->e_رمز==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_اظہاریہ->op = TK_NULL;
      }ورنہ اگر( pWalker->e_رمز==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_رمز = 0;
        واپس WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    طےشدہ:
      testcase( p_اظہاریہ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_اظہاریہ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      واپس WRC_Continue;
  }
}
ساکن صحیح اظہاریہ_Is_Const(Parse *pParse, اظہاریہ *p, صحیح initFlag){
  Walker w;
  w.e_رمز = initFlag;
  w.pParse = pParse;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شدہ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#اگر_ختم
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز;
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
صحیح sqlite3_اظہاریہ_Is_Constant(Parse *pParse, اظہاریہ *p){
  واپس اظہاریہ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
صحیح sqlite3_اظہاریہ_List_Is_Constant(Parse *pParse, اظہاریہ_List *pList, صحیح bNoIs){
  ثابت صحیح iInit = bNoIs ? 6 : 1;
  صحیح ii;
  برائے(ii=0; ii<pList->n_اظہاریہ; ii++){
    اگر( 0==اظہاریہ_Is_Const(pParse, pList->a[ii].p_اظہاریہ, iInit) ) واپس 0;
  }
  واپس 1;
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
ساکن صحیح sqlite3_اظہاریہ_Is_Constant_Not_Join(Parse *pParse, اظہاریہ *p){
  واپس اظہاریہ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ساکن صحیح اظہاریہ_Select_Walk_جدول_Constant(Walker *pWalker, Select *pSelect){
  أكد( pSelect!=0 );
  أكد( pWalker->e_رمز==3 || pWalker->e_رمز==0 );
  اگر( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_رمز = 0;
    واپس WRC_Abort;
  }
  واپس WRC_Prune;
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
صحیح sqlite3_اظہاریہ_Is_جدول_Constant(اظہاریہ *p, صحیح iCur, صحیح bAllowSubq){
  Walker w;
  w.e_رمز = 3;
  w.pParse = 0;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant;
  اگر( bAllowSubq ){
    w.xSelectCallback = اظہاریہ_Select_Walk_جدول_Constant;
  }ورنہ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شدہ SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#اگر_ختم
  }
  w.u.iCur = iCur;
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز;
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
صحیح sqlite3_اظہاریہ_Is_Single_جدول_Constraint(
  اظہاریہ *p_اظہاریہ,                 /* The constraint */
  ثابت SrcList *pSrcList,     /* Complete FROM clause */
  صحیح iSrc,                    /* Which element of pSrcList to use */
  صحیح bAllowSubq               /* Allow non-correlated subqueries */
){
  ثابت Src_شے *pSrc = &pSrcList->a[iSrc];
  اگر( pSrc->fg.jointype & JT_LTORJ ){
    واپس 0;  /* rule (3) */
  }
  اگر( pSrc->fg.jointype & JT_بایاں ){
    اگر( !اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) واپس 0;   /* rule (4a) */
    اگر( p_اظہاریہ->w.iJoin!=pSrc->iCursor ) واپس 0;         /* rule (4b) */
  }ورنہ{
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) واپس 0;    /* rule (5) */
  }
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    صحیح jj;
    برائے(jj=0; jj<iSrc; jj++){
      اگر( p_اظہاریہ->w.iJoin==pSrcList->a[jj].iCursor ){
        اگر( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          واپس 0;  /* restriction (6) */
        }
        توڑو;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  واپس sqlite3_اظہاریہ_Is_جدول_Constant(p_اظہاریہ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ساکن صحیح اظہاریہ_گرہ_Is_Constant_Or_گروہ_By(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اظہاریہ_List *p_گروہ_By = pWalker->u.p_گروہ_By;
  صحیح i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  برائے(i=0; i<p_گروہ_By->n_اظہاریہ; i++){
    اظہاریہ *p = p_گروہ_By->a[i].p_اظہاریہ;
    اگر( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_اظہاریہ_NNColl_Seq(pWalker->pParse, p);
      اگر( sqlite3_Is_ثنائی(pColl) ){
        واپس WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    pWalker->e_رمز = 0;
    واپس WRC_Abort;
  }

  واپس اظہاریہ_گرہ_Is_Constant(pWalker, p_اظہاریہ);
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
صحیح sqlite3_اظہاریہ_Is_Constant_Or_گروہ_By(Parse *pParse, اظہاریہ *p, اظہاریہ_List *p_گروہ_By){
  Walker w;
  w.e_رمز = 1;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Is_Constant_Or_گروہ_By;
  w.xSelectCallback = 0;
  w.u.p_گروہ_By = p_گروہ_By;
  w.pParse = pParse;
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز;
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
صحیح sqlite3_اظہاریہ_Is_Constant_Or_Function(اظہاریہ *p, u8 isInit){
  أكد( isInit==0 || isInit==1 );
  واپس اظہاریہ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
صحیح sqlite3_اظہاریہ_شامل_ہے_Subquery(اظہاریہ *p){
  Walker w;
  w.e_رمز = 1;
  w.x_اظہاریہ_Callback = sqlite3_اظہاریہ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#اگر_تعریف_شدہ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#اگر_ختم
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز==0;
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
صحیح sqlite3_اظہاریہ_Is_Integer(ثابت اظہاریہ *p, صحیح *p_قدر, Parse *pParse, صحیح bRSI){
  صحیح iSign = 1;   /* Either +1 or -1. */
  جب( 1/*exit-by-break*/ ){
    اگر( NEVER(p==0) ) واپس 0;
    اگر( اظہاریہ_Use_UValue(p) ){
      أكد( p->op==TK_INTEGER );
      *p_قدر = p->u.i_قدر*iSign;
      اگر( bRSI && pParse==0 && p->op2 ){
        واپس 0;
      }
      واپس 1;
    }
    اگر( p->op==TK_UPLUS ){
      p = p->p_بایاں;
      pParse = 0;
      جاری;
    }
    اگر( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_بایاں;
      pParse = 0;
      جاری;
    }
    اگر( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_قدر *pVal; /* The variable */
      صحیح rc = 0;          /* 1 if successful, 0 if failed */
      أكد( iSign==1 );
      اگر( NEVER(pParse->pVdbe==0) ) توڑو;
      اگر( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) توڑو;
      pVal = sqlite3_Vdbe_Get_Bound_قدر(pParse->pReprepare, p->i_عمود,
                                      SQLITE_AFF_BLOB);
      اگر( pVal ){
        i64 vv;
        اگر( sqlite3_قدر_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_قدر_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_قدر = (صحیح)vv;
          rc = 1;       
        }
        sqlite3_قدر_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_عمود, bRSI);
      واپس rc;
    }
    توڑو;
  }
  واپس 0;
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
صحیح sqlite3_اظہاریہ_Can_Be_Null(ثابت اظہاریہ *p){
  u8 op;
  أكد( p!=0 );
  جب( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_بایاں;
    أكد( p!=0 );
  }
  op = p->op;
  اگر( op==TK_رجسٹر ) op = p->op2;
  انتخاب( op ){
    حالت TK_INTEGER:
    حالت TK_STRING:
    حالت TK_FLOAT:
    حالت TK_BLOB:
      واپس 0;
    حالت TK_عمود:
      أكد( اظہاریہ_Use_YTab(p) );
      واپس اظہاریہ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
          || (p->i_عمود==XN_ROWID && IsView(p->y.pTab))
#اگر_ختم
          || (p->i_عمود>=0
              && p->y.pTab->a_ستون!=0 /* Possible due to prior error */
              && ALWAYS(p->i_عمود<p->y.pTab->n_ستون)
              && p->y.pTab->a_ستون[p->i_عمود].notNull==0);
    طےشدہ:
      واپس 1;
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
صحیح sqlite3_اظہاریہ_Needs_No_Affinity_Change(ثابت اظہاریہ *p, حرف aff){
  u8 op;
  صحیح unaryMinus = 0;
  اگر( aff==SQLITE_AFF_BLOB ) واپس 1;
  جب( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    اگر( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_بایاں;
  }
  op = p->op;
  اگر( op==TK_رجسٹر ) op = p->op2;
  انتخاب( op ){
    حالت TK_INTEGER: {
      واپس aff>=SQLITE_AFF_NUMERIC;
    }
    حالت TK_FLOAT: {
      واپس aff>=SQLITE_AFF_NUMERIC;
    }
    حالت TK_STRING: {
      واپس !unaryMinus && aff==SQLITE_AFF_متن;
    }
    حالت TK_BLOB: {
      واپس !unaryMinus;
    }
    حالت TK_عمود: {
      أكد( p->i_جدول>=0 );  /* p cannot be part of a CHECK constraint */
      واپس aff>=SQLITE_AFF_NUMERIC && p->i_عمود<0;
    }
    طےشدہ: {
      واپس 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
صحیح sqlite3IsRowid(ثابت حرف *z){
  اگر( sqlite3StrICmp(z, "_ROWID_")==0 ) واپس 1;
  اگر( sqlite3StrICmp(z, "ROWID")==0 ) واپس 1;
  اگر( sqlite3StrICmp(z, "OID")==0 ) واپس 1;
  واپس 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ثابت حرف *sqlite3RowidAlias(جدول *pTab){
  ثابت حرف *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  صحیح ii;
  أكد( VisibleRowid(pTab) );
  برائے(ii=0; ii<Array_سائز(azOpt); ii++){
    اگر( sqlite3_عمود_اشاریہ(pTab, azOpt[ii])<0 ) واپس azOpt[ii];
  }
  واپس 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
ساکن Select *is_Candidate_For_اندر_Opt(ثابت اظہاریہ *pX){
  Select *p;
  SrcList *pSrc;
  اظہاریہ_List *pEList;
  جدول *pTab;
  صحیح i;
  اگر( !اظہاریہ_Use_XSelect(pX) ) واپس 0;                 /* Not a subquery */
  اگر( اظہاریہ_Has_Property(pX, EP_VarSelect)  ) واپس 0;  /* Correlated subq */
  p = pX->x.pSelect;
  اگر( p->p_پیشگی ) واپس 0;              /* Not a compound SELECT */
  اگر( p->selFlags & SF_Aggregate ){
    واپس 0; /* No GROUP BY keyword or aggregate functions */
  }
  أكد( p->p_گروہ_By==0 );              /* Has no GROUP BY clause */
  اگر( p->p_حد_بندی ) واپس 0;              /* Has no LIMIT clause */
  اگر( p->pWhere ) واپس 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  أكد( pSrc!=0 );
  اگر( pSrc->nSrc!=1 ) واپس 0;          /* Single term in FROM clause */
  اگر( pSrc->a[0].fg.isSubquery) واپس 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab)  );              /* FROM clause is not a view */
  اگر( IsVirtual(pTab) ) واپس 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  أكد( pEList!=0 );
  /* All SELECT results must be columns. */
  برائے(i=0; i<pEList->n_اظہاریہ; i++){
    اظہاریہ *pRes = pEList->a[i].p_اظہاریہ;
    اگر( pRes->op!=TK_عمود ) واپس 0;
    أكد( pRes->i_جدول==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  واپس p;
}
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ساکن خالی sqlite3_سیٹ_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  صحیح iCur,          /* Cursor for the index */
  صحیح regHasNull,    /* Register in which to store hasNull flag */
  صحیح e_ترتیب_دو_ترتیب     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  صحیح addr1;
  صحیح op;
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, regHasNull);
  اگر( e_ترتیب_دو_ترتیب==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ورنہ{
    op = OP_آخری;
  }
  addr1 = sqlite3_Vdbe_جوڑو_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_آخری?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_یہاں(v, addr1);
}
#اگر_ختم


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ساکن صحیح sqlite3_اندر_Rhs_Is_Constant(Parse *pParse, اظہاریہ *p_اندر){
  اظہاریہ *pLHS;
  صحیح res;
  أكد( !اظہاریہ_Has_Property(p_اندر, EP_xIsSelect) );
  pLHS = p_اندر->p_بایاں;
  p_اندر->p_بایاں = 0;
  res = sqlite3_اظہاریہ_Is_Constant(pParse, p_اندر);
  p_اندر->p_بایاں = pLHS;
  واپس res;
}
#اگر_ختم

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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
صحیح sqlite3_ڈھونڈو_اندر_اشاریہ(
  Parse *pParse,             /* Parsing context */
  اظہاریہ *pX,                  /* The IN expression */
  u32 اندر_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  صحیح *prRhsHasNull,         /* Register holding NULL status.  See notes */
  صحیح *aiMap,                /* Mapping from Index fields to RHS fields */
  صحیح *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  صحیح eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  صحیح iTab;                             /* Cursor of the RHS table */
  صحیح mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  أكد( pX->op==TK_اندر );
  mustBeUnique = (اندر_Flags & اندر_اشاریہ_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  اگر( prRhsHasNull && اظہاریہ_Use_XSelect(pX) ){
    صحیح i;
    اظہاریہ_List *pEList = pX->x.pSelect->pEList;
    برائے(i=0; i<pEList->n_اظہاریہ; i++){
      اگر( sqlite3_اظہاریہ_Can_Be_Null(pEList->a[i].p_اظہاریہ) ) توڑو;
    }
    اگر( i==pEList->n_اظہاریہ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  اگر( pParse->nErr==0 && (p = is_Candidate_For_اندر_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    جدول *pTab;                           /* Table <table>. */
    صحیح iDb;                               /* Database idx for pTab */
    اظہاریہ_List *pEList = p->pEList;
    صحیح n_اظہاریہ = pEList->n_اظہاریہ;

    أكد( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    أكد( p->pEList->a[0].p_اظہاریہ!=0 ); /* Because of isCandidateForInOpt(p) */
    أكد( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
    أكد( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_رمز_Verify_Schema(pParse, iDb);
    sqlite3_جدول_تالا(pParse, iDb, pTab->tnum, 0, pTab->zName);

    أكد(v);  /* sqlite3GetVdbe() has always been previously called */
    اگر( n_اظہاریہ==1 && pEList->a[0].p_اظہاریہ->i_عمود<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      صحیح iAddr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_جدول(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = اندر_اشاریہ_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_یہاں(v, iAddr);
    }ورنہ{
      اشاریہ *pIdx;                         /* Iterator variable */
      صحیح affinity_ok = 1;
      صحیح i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      برائے(i=0; i<n_اظہاریہ && affinity_ok; i++){
        اظہاریہ *pLhs = sqlite3_سمتیہ_میدان_Subexpr(pX->p_بایاں, i);
        صحیح i_ستون = pEList->a[i].p_اظہاریہ->i_عمود;
        حرف idxaff = sqlite3_جدول_عمود_Affinity(pTab,i_ستون); /* RHS table */
        حرف cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_متن );
        انتخاب( cmpaff ){
          حالت SQLITE_AFF_BLOB:
            توڑو;
          حالت SQLITE_AFF_متن:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            أكد( idxaff==SQLITE_AFF_متن );
            توڑو;
          طےشدہ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      اگر( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        برائے(pIdx=pTab->p_اشاریہ; pIdx && eType==0; pIdx=pIdx->p_اگلا){
          Bitmask ستون_مستعمل;      /* Columns of the index used */
          Bitmask m_ستون;         /* Mask for the current column */
          اگر( pIdx->n_عمود<n_اظہاریہ ) جاری;
          اگر( pIdx->pPartIdxWhere!=0 ) جاری;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_عمود==BMS-2 );
          testcase( pIdx->n_عمود==BMS-1 );
          اگر( pIdx->n_عمود>=BMS-1 ) جاری;
          اگر( mustBeUnique ){
            اگر( pIdx->n_کلید_ستون>n_اظہاریہ
             ||(pIdx->n_عمود>n_اظہاریہ && !Is_Unique_اشاریہ(pIdx))
            ){
              جاری;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          ستون_مستعمل = 0;   /* Columns of index used so far */
          برائے(i=0; i<n_اظہاریہ; i++){
            اظہاریہ *pLhs = sqlite3_سمتیہ_میدان_Subexpr(pX->p_بایاں, i);
            اظہاریہ *pRhs = pEList->a[i].p_اظہاریہ;
            CollSeq *pReq = sqlite3_ثنائی_Compare_Coll_Seq(pParse, pLhs, pRhs);
            صحیح j;
 
            برائے(j=0; j<n_اظہاریہ; j++){
              اگر( pIdx->ai_عمود[j]!=pRhs->i_عمود ) جاری;
              أكد( pIdx->azColl[j] );
              اگر( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                جاری;
              }
              توڑو;
            }
            اگر( j==n_اظہاریہ ) توڑو;
            m_ستون = MASKBIT(j);
            اگر( m_ستون & ستون_مستعمل ) توڑو; /* Each column used only once */
            ستون_مستعمل |= m_ستون;
            اگر( aiMap ) aiMap[i] = j;
          }
 
          أكد( n_اظہاریہ>0 && n_اظہاریہ<BMS );
          أكد( i==n_اظہاریہ || ستون_مستعمل!=(MASKBIT(n_اظہاریہ)-1) );
          اگر( ستون_مستعمل==(MASKBIT(n_اظہاریہ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            صحیح iAddr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            أكد( اندر_اشاریہ_اشاریہ_DESC == اندر_اشاریہ_اشاریہ_ASC+1 );
            eType = اندر_اشاریہ_اشاریہ_ASC + pIdx->a_ترتیب_دو_ترتیب[0];
 
            اگر( prRhsHasNull ){
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_مستعمل_نقاب
              u64 نقاب = (1<<n_اظہاریہ)-1;
              sqlite3_Vdbe_جوڑو_Op3(v, OP_Columns_مستعمل,
                                iTab, LOWER32(نقاب), UPPER32(نقاب));
#اگر_ختم
              *prRhsHasNull = ++pParse->nMem;
              اگر( n_اظہاریہ==1 ){
                sqlite3_سیٹ_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_ترتیب_دو_ترتیب[0]);
              }
            }
            sqlite3_Vdbe_Jump_یہاں(v, iAddr);
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
   && (اندر_Flags & اندر_اشاریہ_NOOP_OK)
   && اظہاریہ_Use_XList(pX)
   && (!sqlite3_اندر_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_اظہاریہ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = اندر_اشاریہ_NOOP;
  }

  اگر( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    صحیح rMayHaveNull = 0;
    صحیح bloomOk = (اندر_Flags & اندر_اشاریہ_MEMBERSHIP)!=0;
    eType = اندر_اشاریہ_EPH;
    اگر( اندر_Flags & اندر_اشاریہ_LOOP ){
      pParse->nQueryLoop = 0;
    }ورنہ اگر( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    أكد( pX->op==TK_اندر );
    اگر( !bloomOk
     && اظہاریہ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_اندر)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_رمز_Rhs_کا_اندر(pParse, pX, iTab, bloomOk);
    اگر( rMayHaveNull ){
      sqlite3_سیٹ_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  اگر( aiMap && eType!=اندر_اشاریہ_اشاریہ_ASC && eType!=اندر_اشاریہ_اشاریہ_DESC ){
    صحیح i, n;
    n = sqlite3_اظہاریہ_سمتیہ_سائز(pX->p_بایاں);
    برائے(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  واپس eType;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ساکن حرف *اظہاریہ_INAffinity(Parse *pParse, ثابت اظہاریہ *p_اظہاریہ){
  اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
  صحیح nVal = sqlite3_اظہاریہ_سمتیہ_سائز(p_بایاں);
  Select *pSelect = اظہاریہ_Use_XSelect(p_اظہاریہ) ? p_اظہاریہ->x.pSelect : 0;
  حرف *zRet;

  أكد( p_اظہاریہ->op==TK_اندر );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  اگر( zRet ){
    صحیح i;
    برائے(i=0; i<nVal; i++){
      اظہاریہ *pA = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
      حرف a = sqlite3_اظہاریہ_Affinity(pA);
      اگر( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_اظہاریہ, a);
      }ورنہ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  واپس zRet;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
خالی sqlite3SubselectError(Parse *pParse, صحیح nActual, صحیح nExpect){
  اگر( pParse->nErr==0 ){
    ثابت حرف *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_پیغام(pParse, zFmt, nActual, nExpect);
  }
}
#اگر_ختم

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
خالی sqlite3_سمتیہ_Error_پیغام(Parse *pParse, اظہاریہ *p_اظہاریہ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
  اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    sqlite3SubselectError(pParse, p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ, 1);
  }ورنہ
#اگر_ختم
  {
    sqlite3_Error_پیغام(pParse, "row value misused");
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ساکن صحیح ڈھونڈو_Compatible_اندر_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  اظہاریہ *p_اظہاریہ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_انتہا;
  SubrtnSig *pSig;
  Vdbe *v;

  اگر( pNewSig==0 ) واپس 0;
  اگر( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) واپس 0;
  أكد( p_اظہاریہ->op==TK_اندر );
  أكد( !اظہاریہ_Use_YSub(p_اظہاریہ) );
  أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
  أكد( p_اظہاریہ->x.pSelect!=0 );
  أكد( (p_اظہاریہ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  أكد( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_انتہا = sqlite3_Vdbe_Get_آخری_Op(v);
  برائے(; pOp<p_انتہا; pOp++){
    اگر( pOp->p4type!=P4_SUBRTNSIG ) جاری;
    أكد( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    أكد( pSig!=0 );
    اگر( !pSig->bComplete ) جاری;
    اگر( pNewSig->selId!=pSig->selId ) جاری;
    اگر( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) جاری;
    p_اظہاریہ->y.sub.iAddr = pSig->iAddr;
    p_اظہاریہ->y.sub.regReturn = pSig->regReturn;
    p_اظہاریہ->i_جدول = pSig->i_جدول;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Subrtn);
    واپس 1;
  }
  واپس 0;
}
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
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
خالی sqlite3_رمز_Rhs_کا_اندر(
  Parse *pParse,          /* Parsing context */
  اظہاریہ *p_اظہاریہ,            /* The IN operator */
  صحیح iTab,               /* Use this cursor number */
  صحیح allowBloom          /* True to allow the use of a Bloom filter */
){
  صحیح addrOnce = 0;           /* Address of the OP_Once instruction at top */
  صحیح addr;                   /* Address of OP_OpenEphemeral instruction */
  اظہاریہ *p_بایاں;                /* the LHS of the IN operator */
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
  اگر( !اظہاریہ_Has_Property(p_اظہاریہ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    أكد( !اظہاریہ_Use_XSelect(p_اظہاریہ) || p_اظہاریہ->x.pSelect!=0 );
    اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) && (p_اظہاریہ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, حجم(pSig[0]));
      اگر( pSig ){
        pSig->selId = p_اظہاریہ->x.pSelect->selId;
        pSig->zAff = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) 
     || ڈھونڈو_Compatible_اندر_Rhs_Subrtn(pParse, p_اظہاریہ, pSig)
    ){
      addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
      اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_اظہاریہ->x.pSelect->selId));
      }
      أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_اظہاریہ->y.sub.regReturn,
                        p_اظہاریہ->y.sub.iAddr);
      أكد( iTab!=p_اظہاریہ->i_جدول );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenDup, iTab, p_اظہاریہ->i_جدول);
      sqlite3_Vdbe_Jump_یہاں(v, addrOnce);
      اگر( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      واپس;
    }

    /* Begin coding the subroutine */
    أكد( !اظہاریہ_Use_YWin(p_اظہاریہ) );
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Subrtn);
    أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );
    p_اظہاریہ->y.sub.regReturn = ++pParse->nMem;
    p_اظہاریہ->y.sub.iAddr =
      sqlite3_Vdbe_جوڑو_Op2(v, OP_BeginSubrtn, 0, p_اظہاریہ->y.sub.regReturn) + 1;
    اگر( pSig ){
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
  nVal = sqlite3_اظہاریہ_سمتیہ_سائز(p_بایاں);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_اظہاریہ->i_جدول = iTab;
  addr = sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, p_اظہاریہ->i_جدول, nVal);
#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
  اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
    VdbeComment((v, "Result of SELECT %u", p_اظہاریہ->x.pSelect->selId));
  }ورنہ{
    VdbeComment((v, "RHS of IN operator"));
  }
#اگر_ختم
  p_کلید_Info = sqlite3_کلید_Info_Alloc(pParse->db, nVal, 1);
  أكد( p_کلید_Info!=0 || pParse->nErr );
  اگر( p_کلید_Info==0 ) واپس;

  اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
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
    اگر( ALWAYS(pEList->n_اظہاریہ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      صحیح i;
      صحیح rc;
      صحیح addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_سیٹ, iTab);
      dest.zAffSdst = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
      pSelect->i_حد_بندی = 0;
      أكد( pEList!=0 );
      أكد( pEList->n_اظہاریہ>0 );
      أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
      برائے(i=0; i<nVal; i++){
        اظہاریہ *p = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
        CollSeq *pColl;
        p_کلید_Info->aColl[i] = pColl = sqlite3_ثنائی_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_اظہاریہ
        );
        اگر( !sqlite3_Is_ثنائی(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      اگر( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        صحیح regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_جوڑو_Op2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (خالی *)p_کلید_Info, P4_KEYINFO);
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
        واپس;
      }
    }
  }ورنہ اگر( ALWAYS(p_اظہاریہ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    حرف affinity;            /* Affinity of the LHS of the IN */
    صحیح i;
    اظہاریہ_List *pList = p_اظہاریہ->x.pList;
    ساخت اظہاریہ_List_شے *p_شے;
    صحیح r1, r2;
    affinity = sqlite3_اظہاریہ_Affinity(p_بایاں);
    اگر( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ورنہ اگر( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    أكد( p_کلید_Info!=0 );
    أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Info) );
    p_کلید_Info->aColl[0] = sqlite3_اظہاریہ_Coll_Seq(pParse, p_اظہاریہ->p_بایاں);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_عارضی_Reg(pParse);
    r2 = sqlite3_Get_عارضی_Reg(pParse);
    برائے(i=pList->n_اظہاریہ, p_شے=pList->a; i>0; i--, p_شے++){
      اظہاریہ *pE2 = p_شے->p_اظہاریہ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      اگر( addrOnce && !sqlite3_اظہاریہ_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_مقصد_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_مقصد_Noop(v, addrOnce);
        اظہاریہ_Clear_Property(p_اظہاریہ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_اظہاریہ_رمز(pParse, pE2, r1);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iTab, r2, r1, 1);
    }
    sqlite3_Release_عارضی_Reg(pParse, r1);
    sqlite3_Release_عارضی_Reg(pParse, r2);
  }
  اگر( pSig ) pSig->bComplete = 1;
  اگر( p_کلید_Info ){
    sqlite3VdbeChangeP4(v, addr, (خالی *)p_کلید_Info, P4_KEYINFO);
  }
  اگر( addrOnce ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, iTab);
    sqlite3_Vdbe_Jump_یہاں(v, addrOnce);
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
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
صحیح sqlite3_رمز_Subselect(Parse *pParse, اظہاریہ *p_اظہاریہ){
  صحیح addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  صحیح rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  صحیح nReg;                   /* Registers to allocate */
  اظہاریہ *p_حد_بندی;               /* New limit expression */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addrExplain;            /* Address of OP_Explain instruction */
#اگر_ختم

  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  اگر( pParse->nErr ) واپس 0;
  testcase( p_اظہاریہ->op==TK_EXISTS );
  testcase( p_اظہاریہ->op==TK_SELECT );
  أكد( p_اظہاریہ->op==TK_EXISTS || p_اظہاریہ->op==TK_SELECT );
  أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
  pSel = p_اظہاریہ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    أكد( اظہاریہ_Use_YSub(p_اظہاریہ) );
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_اظہاریہ->y.sub.regReturn,
                      p_اظہاریہ->y.sub.iAddr);
    واپس p_اظہاریہ->i_جدول;
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
  اگر( !اظہاریہ_Has_Property(p_اظہاریہ, EP_VarSelect) ){
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
  sqlite3_Vdbe_جائزہ_حالت_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_اظہاریہ->op==TK_SELECT ? pSel->pEList->n_اظہاریہ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  اگر( p_اظہاریہ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    اگر( (pSel->selFlags&SF_Distinct) && pSel->p_حد_بندی && pSel->p_حد_بندی->p_دایاں ){
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
    }ورنہ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ورنہ{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  اگر( pSel->p_حد_بندی ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    اظہاریہ *p_بایاں = pSel->p_حد_بندی->p_بایاں;
    اگر( اظہاریہ_Has_Property(p_بایاں, EP_Int_قدر)==0
     || (p_بایاں->u.i_قدر!=1 && p_بایاں->u.i_قدر!=0)
    ){
      sqlite3 *db = pParse->db;
      p_حد_بندی = sqlite3_اظہاریہ_Int32(db, 0);
      اگر( p_حد_بندی ){
        p_حد_بندی->aff_اظہاریہ = SQLITE_AFF_NUMERIC;
        p_حد_بندی = sqlite3PExpr(pParse, TK_NE,
            sqlite3_اظہاریہ_Dup(db, p_بایاں, 0), p_حد_بندی);
      }
      sqlite3_اظہاریہ_Deferred_Delete(pParse, p_بایاں);
      pSel->p_حد_بندی->p_بایاں = p_حد_بندی;
    }
  }ورنہ{
    /* If there is no pre-existing limit add a limit of 1 */
    p_حد_بندی = sqlite3_اظہاریہ_Int32(pParse->db, 1);
    pSel->p_حد_بندی = sqlite3PExpr(pParse, TK_حد_بندی, p_حد_بندی, 0);
  }
  pSel->i_حد_بندی = 0;
  اگر( sqlite3Select(pParse, pSel, &dest) ){
    p_اظہاریہ->op2 = p_اظہاریہ->op;
    p_اظہاریہ->op = TK_ERROR;
    واپس 0;
  }
  p_اظہاریہ->i_جدول = rReg = dest.iSDParm;
  اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);
  اگر( addrOnce ){
    sqlite3_Vdbe_Jump_یہاں(v, addrOnce);
  }
  sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExplain, addrExplain, -1);

  اگر( dest.eDest==SRT_Mem 
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
  واپس rReg;
}
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
صحیح sqlite3_اظہاریہ_Check_اندر(Parse *pParse, اظہاریہ *p_اندر){
  صحیح n_سمتیہ = sqlite3_اظہاریہ_سمتیہ_سائز(p_اندر->p_بایاں);
  اگر( اظہاریہ_Use_XSelect(p_اندر) && !pParse->db->mallocFailed ){
    اگر( n_سمتیہ!=p_اندر->x.pSelect->pEList->n_اظہاریہ ){
      sqlite3SubselectError(pParse, p_اندر->x.pSelect->pEList->n_اظہاریہ, n_سمتیہ);
      واپس 1;
    }
  }ورنہ اگر( n_سمتیہ!=1 ){
    sqlite3_سمتیہ_Error_پیغام(pParse, p_اندر->p_بایاں);
    واپس 1;
  }
  واپس 0;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
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
ساکن خالی sqlite3_اظہاریہ_رمز_اندر(
  Parse *pParse,        /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,          /* The IN expression */
  صحیح destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  صحیح destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  صحیح rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  صحیح eType;            /* Type of the RHS */
  صحیح rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  صحیح *aiMap = 0;       /* Map from vector field to index column */
  حرف *zAff = 0;       /* Affinity string for comparisons */
  صحیح n_سمتیہ;          /* Size of vectors for this IN operator */
  صحیح iDummy;           /* Dummy parameter to exprCodeVector() */
  اظہاریہ *p_بایاں;          /* The LHS of the IN operator */
  صحیح i;                /* loop counter */
  صحیح destStep2;        /* Where to jump when NULLs seen in step 2 */
  صحیح destStep6 = 0;    /* Start of code for Step 6 */
  صحیح addrTruthOp;      /* Address of opcode that determines the IN is true */
  صحیح destNotNull;      /* Jump here if a comparison is not true in step 6 */
  صحیح addr_چوٹی;          /* Top of the step-6 loop */
  صحیح iTab = 0;         /* Index to use */
  u8 ok_Const_ضربی_عدد = pParse->ok_Const_ضربی_عدد;

  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  p_بایاں = p_اظہاریہ->p_بایاں;
  اگر( sqlite3_اظہاریہ_Check_اندر(pParse, p_اظہاریہ) ) واپس;
  zAff = اظہاریہ_INAffinity(pParse, p_اظہاریہ);
  n_سمتیہ = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_بایاں);
  aiMap = (صحیح*)sqlite3DbMallocZero(pParse->db, n_سمتیہ*حجم(صحیح));
  اگر( pParse->db->mallocFailed ) جاؤ sqlite3_اظہاریہ_رمز_اندر_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  أكد( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_ڈھونڈو_اندر_اشاریہ(pParse, p_اظہاریہ,
                             اندر_اشاریہ_MEMBERSHIP | اندر_اشاریہ_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  أكد( pParse->nErr || n_سمتیہ==1 || eType==اندر_اشاریہ_EPH
       || eType==اندر_اشاریہ_اشاریہ_ASC || eType==اندر_اشاریہ_اشاریہ_DESC
  );
#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  برائے(i=0; i<n_سمتیہ; i++){
    صحیح j, cnt;
    برائے(cnt=j=0; j<n_سمتیہ; j++) اگر( aiMap[j]==i ) cnt++;
    أكد( cnt==1 );
  }
#اگر_ختم

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
  أكد( pParse->ok_Const_ضربی_عدد==ok_Const_ضربی_عدد );
  pParse->ok_Const_ضربی_عدد = 0;
  rLhs = اظہاریہ_رمز_سمتیہ(pParse, p_بایاں, &iDummy);
  pParse->ok_Const_ضربی_عدد = ok_Const_ضربی_عدد;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  اگر( eType==اندر_اشاریہ_NOOP ){
    اظہاریہ_List *pList;
    CollSeq *pColl;
    صحیح labelOk = sqlite3VdbeMakeLabel(pParse);
    صحیح r2, reg_مقصد_Free;
    صحیح regCkNull = 0;
    صحیح ii;
    أكد( n_سمتیہ==1 );
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    pList = p_اظہاریہ->x.pList;
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_اظہاریہ->p_بایاں);
    اگر( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_ثنائی_ہندسہ_And, rLhs, rLhs, regCkNull);
    }
    برائے(ii=0; ii<pList->n_اظہاریہ; ii++){
      r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, pList->a[ii].p_اظہاریہ, &reg_مقصد_Free);
      اگر( regCkNull && sqlite3_اظہاریہ_Can_Be_Null(pList->a[ii].p_اظہاریہ) ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_ثنائی_ہندسہ_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_عارضی_Reg(pParse, reg_مقصد_Free);
      اگر( ii<pList->n_اظہاریہ-1 || destIfNull!=destIfFalse ){
        صحیح op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_جوڑو_Op4(v, op, rLhs, labelOk, r2,
                          (خالی*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_اظہاریہ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_اظہاریہ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_اظہاریہ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_اظہاریہ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ورنہ{
        صحیح op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        أكد( destIfNull==destIfFalse );
        sqlite3_Vdbe_جوڑو_Op4(v, op, rLhs, destIfFalse, r2,
                          (خالی*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    اگر( regCkNull ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_عارضی_Reg(pParse, regCkNull);
    جاؤ sqlite3_اظہاریہ_رمز_اندر_ختم_شدہ;
  }

  اگر( eType!=اندر_اشاریہ_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    اگر( n_سمتیہ==1 ){
      حرف aff = zAff[0];
      اگر( aff>=SQLITE_AFF_متن && aff!=sqlite3_اظہاریہ_Affinity(p_بایاں) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        صحیح rTmp = sqlite3_Get_عارضی_Reg(pParse);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, rLhs, n_سمتیہ, 0, zAff, n_سمتیہ);
    }

    برائے(i=0; i<n_سمتیہ && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    اگر( i!=n_سمتیہ ){
      /* Need to reorder the LHS fields according to aiMap */
      صحیح rLhsOrig = rLhs;
      rLhs = sqlite3_Get_عارضی_Range(pParse, n_سمتیہ);
      برائے(i=0; i<n_سمتیہ; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  اگر( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ورنہ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  برائے(i=0; i<n_سمتیہ; i++){
    اظہاریہ *p = sqlite3_سمتیہ_میدان_Subexpr(p_اظہاریہ->p_بایاں, i);
    اگر( pParse->nErr ) جاؤ sqlite3_اظہاریہ_رمز_اندر_oom_error;
    اگر( sqlite3_اظہاریہ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  اگر( eType==اندر_اشاریہ_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    أكد( n_سمتیہ==1 );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);  /* Return True */
  }ورنہ{
    اگر( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) ){
        ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, p_اظہاریہ->y.sub.iAddr);
        أكد( pOp->opcode==OP_Once || pParse->nErr );
        اگر( pOp->p3>0 ){  /* tag-202407032019 */
          أكد( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_سمتیہ); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Not_ملا, iTab, destIfFalse,
                           rLhs, n_سمتیہ); VdbeCoverage(v);
      جاؤ sqlite3_اظہاریہ_رمز_اندر_ختم_شدہ;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_ملا, iTab, 0,
                                      rLhs, n_سمتیہ); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  اگر( rRhsHasNull && n_سمتیہ==1 ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
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
  اگر( eType==اندر_اشاریہ_اشاریہ_DESC ){
    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_آخری, iTab, destIfFalse);
  }ورنہ{
    testcase( eType==اندر_اشاریہ_EPH );
    testcase( eType==اندر_اشاریہ_ROWID );
    addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  اگر( n_سمتیہ>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ورنہ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  برائے(i=0; i<n_سمتیہ; i++){
    اظہاریہ *p;
    CollSeq *pColl;
    صحیح r3 = sqlite3_Get_عارضی_Reg(pParse);
    p = sqlite3_سمتیہ_میدان_Subexpr(p_بایاں, i);
    اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
      اظہاریہ *pRhs = p_اظہاریہ->x.pSelect->pEList->a[i].p_اظہاریہ;
      pColl = sqlite3_ثنائی_Compare_Coll_Seq(pParse, p, pRhs);
    }ورنہ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iTab, aiMap[i], r3);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (خالی*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_عارضی_Reg(pParse, r3);
  }
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, destIfNull);
  اگر( n_سمتیہ>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iTab, addr_چوٹی+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_یہاں(v, addrTruthOp);

sqlite3_اظہاریہ_رمز_اندر_ختم_شدہ:
  VdbeComment((v, "end IN expr"));
sqlite3_اظہاریہ_رمز_اندر_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ساکن خالی رمز_Real(Vdbe *v, ثابت حرف *z, صحیح negateFlag, صحیح iMem){
  اگر( ALWAYS(z!=0) ){
    دوہرا قدر;
    sqlite3AtoF(z, &قدر);
    أكد( !sqlite3IsNaN(قدر) ); /* The new AtoF never returns NaN */
    اگر( negateFlag ) قدر = -قدر;
    sqlite3_Vdbe_جوڑو_Double(v, iMem, قدر);
  }
}
#اگر_ختم


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ساکن خالی رمز_Integer(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح negFlag, صحیح iMem){
  Vdbe *v = pParse->pVdbe;
  اگر( p_اظہاریہ->flags & EP_Int_قدر ){
    صحیح i = p_اظہاریہ->u.i_قدر;
    أكد( i>=0 );
    اگر( negFlag ) i = -i;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, i, iMem);
  }ورنہ{
    صحیح c;
    i64 قدر;
    ثابت حرف *z = p_اظہاریہ->u.zToken;
    أكد( z!=0 );
    c = sqlite3_Dec_Or_Hex_مقصد_I64(z, &قدر);
    اگر( (c==3 && !negFlag) || (c==2) || (negFlag && قدر==سب_سے_چھوٹا_INT64)){
#اگر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
      sqlite3_Error_پیغام(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_اظہاریہ);
#ورنہ
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_HEX_INTEGER
      اگر( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_پیغام(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_اظہاریہ);
      }ورنہ
#اگر_ختم
      {
        رمز_Real(v, z, negFlag, iMem);
      }
#اگر_ختم
    }ورنہ{
      اگر( negFlag ){ قدر = c==3 ? سب_سے_چھوٹا_INT64 : -قدر; }
      sqlite3_Vdbe_جوڑو_Int64(v, iMem, قدر);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
خالی sqlite3_اظہاریہ_رمز_بھرو_اشاریہ_عمود(
  Parse *pParse,  /* The parsing context */
  اشاریہ *pIdx,    /* The index whose column is to be loaded */
  صحیح iTabCur,    /* Cursor pointing to a table row */
  صحیح i_Idx_ستون,    /* The column of the index to be loaded */
  صحیح reg_برآمد      /* Store the index column value in this register */
){
  i16 i_Tab_ستون = pIdx->ai_عمود[i_Idx_ستون];
  اگر( i_Tab_ستون==XN_اظہاریہ ){
    أكد( pIdx->a_ستون_اظہاریہ );
    أكد( pIdx->a_ستون_اظہاریہ->n_اظہاریہ>i_Idx_ستون );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_اظہاریہ_رمز_Copy(pParse, pIdx->a_ستون_اظہاریہ->a[i_Idx_ستون].p_اظہاریہ, reg_برآمد);
    pParse->iSelfTab = 0;
  }ورنہ{
    sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(pParse->pVdbe, pIdx->p_جدول, iTabCur,
                                    i_Tab_ستون, reg_برآمد);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
خالی sqlite3_اظہاریہ_رمز_Generated_عمود(
  Parse *pParse,     /* Parsing context */
  جدول *pTab,       /* Table containing the generated column */
  عمود *p_ستون,      /* The generated column */
  صحیح reg_برآمد         /* Put the result in this register */
){
  صحیح iAddr;
  Vdbe *v = pParse->pVdbe;
  صحیح nErr = pParse->nErr;
  أكد( v!=0 );
  أكد( pParse->iSelfTab!=0 );
  اگر( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, pParse->iSelfTab-1, 0, reg_برآمد);
  }ورنہ{
    iAddr = 0;
  }
  sqlite3_اظہاریہ_رمز_Copy(pParse, sqlite3_عمود_اظہاریہ(pTab,p_ستون), reg_برآمد);
  اگر( (p_ستون->ستون_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    صحیح p3 = 2+(صحیح)(p_ستون - pTab->a_ستون);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_TypeCheck, reg_برآمد, 1, p3, (حرف*)pTab, P4_جدول);
  }ورنہ اگر( p_ستون->affinity>=SQLITE_AFF_متن ){
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, reg_برآمد, 1, 0, &p_ستون->affinity, 1);
  }
  اگر( iAddr ) sqlite3_Vdbe_Jump_یہاں(v, iAddr);
  اگر( pParse->nErr>nErr ) pParse->db->err_Byte_انحراف = -1;
}
#اگر_ختم /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
خالی sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(
  Vdbe *v,        /* Parsing context */
  جدول *pTab,    /* The table containing the value */
  صحیح iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  صحیح i_ستون,       /* Index of the column to extract */
  صحیح reg_برآمد      /* Extract the value into this register */
){
  عمود *p_ستون;
  أكد( v!=0 );
  أكد( pTab!=0 );
  أكد( i_ستون!=XN_اظہاریہ );
  اگر( i_ستون<0 || i_ستون==pTab->iPKey ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Rowid, iTabCur, reg_برآمد);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ورنہ{
    صحیح x;
    اگر( IsVirtual(pTab) ){
      x = i_ستون;
      sqlite3_Vdbe_جوڑو_Op3(v, OP_VColumn, iTabCur, x, reg_برآمد);
      واپس;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
    }ورنہ اگر( (p_ستون = &pTab->a_ستون[i_ستون])->ستون_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      اگر( p_ستون->ستون_Flags & COLFLAG_BUSY ){
        sqlite3_Error_پیغام(pParse, "generated column loop on \"%s\"",
                        p_ستون->zCnName);
      }ورنہ{
        صحیح savedSelfTab = pParse->iSelfTab;
        p_ستون->ستون_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_اظہاریہ_رمز_Generated_عمود(pParse, pTab, p_ستون, reg_برآمد);
        pParse->iSelfTab = savedSelfTab;
        p_ستون->ستون_Flags &= ~COLFLAG_BUSY;
      }
      واپس;
#اگر_ختم
    }ورنہ اگر( !HasRowid(pTab) ){
      testcase( i_ستون!=sqlite3_جدول_عمود_مقصد_Storage(pTab, i_ستون) );
      x = sqlite3_جدول_عمود_مقصد_اشاریہ(sqlite3_Primary_کلید_اشاریہ(pTab), i_ستون);
    }ورنہ{
      x = sqlite3_جدول_عمود_مقصد_Storage(pTab,i_ستون);
      testcase( x!=i_ستون );
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iTabCur, x, reg_برآمد);
    sqlite3_عمود_Default(v, pTab, i_ستون, reg_برآمد);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
صحیح sqlite3_اظہاریہ_رمز_Get_عمود(
  Parse *pParse,   /* Parsing and code generating context */
  جدول *pTab,     /* Description of the table we are reading from */
  صحیح i_عمود,     /* Index of the table column */
  صحیح i_جدول,      /* The cursor pointing to the table */
  صحیح iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  أكد( pParse->pVdbe!=0 );
  أكد( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  أكد( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_اظہاریہ_رمز_Get_عمود_کا_جدول(pParse->pVdbe, pTab, i_جدول, i_عمود, iReg);
  اگر( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_آخری_Op(pParse->pVdbe);
    اگر( pOp->opcode==OP_عمود ) pOp->p5 = p5;
    اگر( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  واپس iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
خالی sqlite3_اظہاریہ_رمز_Move(Parse *pParse, صحیح iFrom, صحیح i_مقصد, صحیح nReg){
  sqlite3_Vdbe_جوڑو_Op3(pParse->pVdbe, OP_Move, iFrom, i_مقصد, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
خالی sqlite3_اظہاریہ_مقصد_رجسٹر(اظہاریہ *p_اظہاریہ, صحیح iReg){
  اظہاریہ *p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  اگر( NEVER(p==0) ) واپس;
  اگر( p->op==TK_رجسٹر ){
    أكد( p->i_جدول==iReg );
  }ورنہ{
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
ساکن صحیح اظہاریہ_رمز_سمتیہ(Parse *pParse, اظہاریہ *p, صحیح *piFreeable){
  صحیح i_نتیجہ;
  صحیح n_نتیجہ = sqlite3_اظہاریہ_سمتیہ_سائز(p);
  اگر( n_نتیجہ==1 ){
    i_نتیجہ = sqlite3_اظہاریہ_رمز_عارضی(pParse, p, piFreeable);
  }ورنہ{
    *piFreeable = 0;
    اگر( p->op==TK_SELECT ){
#اگر SQLITE_OMIT_SUBQUERY
      i_نتیجہ = 0;
#ورنہ
      i_نتیجہ = sqlite3_رمز_Subselect(pParse, p);
#اگر_ختم
    }ورنہ{
      صحیح i;
      i_نتیجہ = pParse->nMem+1;
      pParse->nMem += n_نتیجہ;
      أكد( اظہاریہ_Use_XList(p) );
      برائے(i=0; i<n_نتیجہ; i++){
        sqlite3_اظہاریہ_رمز_Factorable(pParse, p->x.pList->a[i].p_اظہاریہ, i+i_نتیجہ);
      }
    }
  }
  واپس i_نتیجہ;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ساکن خالی سیٹ_Do_Not_ملاؤ_Flag_On_Copy(Vdbe *v){
  اگر( sqlite3_Vdbe_Get_آخری_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ساکن صحیح اظہاریہ_رمز_Inline_Function(
  Parse *pParse,        /* Parsing context */
  اظہاریہ_List *pFarg,      /* List of function arguments */
  صحیح iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  صحیح ہدف            /* Store function result in this register */
){
  صحیح nFarg;
  Vdbe *v = pParse->pVdbe;
  أكد( v!=0 );
  أكد( pFarg!=0 );
  nFarg = pFarg->n_اظہاریہ;
  أكد( nFarg>0 );  /* All in-line functions have at least one argument */
  انتخاب( iFuncId ){
    حالت INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      صحیح انتہا_Coalesce = sqlite3VdbeMakeLabel(pParse);
      صحیح i;
      أكد( nFarg>=2 );
      sqlite3_اظہاریہ_رمز(pParse, pFarg->a[0].p_اظہاریہ, ہدف);
      برائے(i=1; i<nFarg; i++){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, ہدف, انتہا_Coalesce);
        VdbeCoverage(v);
        sqlite3_اظہاریہ_رمز(pParse, pFarg->a[i].p_اظہاریہ, ہدف);
      }
      سیٹ_Do_Not_ملاؤ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, انتہا_Coalesce);
      توڑو;
    }
    حالت INLINEFUNC_iif: {
      اظہاریہ case_اظہاریہ;
      memset(&case_اظہاریہ, 0, حجم(case_اظہاریہ));
      case_اظہاریہ.op = TK_CASE;
      case_اظہاریہ.x.pList = pFarg;
      واپس sqlite3_اظہاریہ_رمز_ہدف(pParse, &case_اظہاریہ, ہدف);
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_انحراف_SQL_FUNC
    حالت INLINEFUNC_sqlite_انحراف: {
      اظہاریہ *pArg = pFarg->a[0].p_اظہاریہ;
      اگر( pArg->op==TK_عمود && pArg->i_جدول>=0 ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_انحراف, pArg->i_جدول, pArg->i_عمود, ہدف);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
      }
      توڑو;
    }
#اگر_ختم
    طےشدہ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      أكد( nFarg==1 || nFarg==2 );
      ہدف = sqlite3_اظہاریہ_رمز_ہدف(pParse, pFarg->a[0].p_اظہاریہ, ہدف);
      توڑو;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#اگر !defined(SQLITE_UNTESTABLE)
    حالت INLINEFUNC_اظہاریہ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
         sqlite3_اظہاریہ_Compare(0,pFarg->a[0].p_اظہاریہ, pFarg->a[1].p_اظہاریہ,-1),
         ہدف);
      توڑو;
    }

    حالت INLINEFUNC_اظہاریہ_مستلزم_اظہاریہ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      أكد( nFarg==2 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
         sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse,pFarg->a[0].p_اظہاریہ, pFarg->a[1].p_اظہاریہ,-1),
         ہدف);
      توڑو;
    }

    حالت INLINEFUNC_مستلزم_nonnull_صف: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      اظہاریہ *pA1;
      أكد( nFarg==2 );
      pA1 = pFarg->a[1].p_اظہاریہ;
      اگر( pA1->op==TK_عمود ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer,
           sqlite3_اظہاریہ_مستلزم_Non_Null_صف(pFarg->a[0].p_اظہاریہ,pA1->i_جدول,1),
           ہدف);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
      }
      توڑو;
    }

    حالت INLINEFUNC_affinity: {
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
      sqlite3_Vdbe_بھرو_String(v, ہدف,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      توڑو;
    }
#اگر_ختم /* !defined(SQLITE_UNTESTABLE) */
  }
  واپس ہدف;
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
ساکن صحیح اظہاریہ_گرہ_Can_Return_Subtype(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحیح n;
  FuncDef *pDef;
  sqlite3 *db;
  اگر( p_اظہاریہ->op==TK_CASE || p_اظہاریہ->op==TK_UPLUS 
   || p_اظہاریہ->op==TK_COLLATE || p_اظہاریہ->op==TK_CAST 
  ){
    واپس WRC_Continue;
  }
  اگر( p_اظہاریہ->op!=TK_FUNCTION && p_اظہاریہ->op!=TK_AGG_FUNCTION ){
    واپس WRC_Prune;
  }
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  db = pWalker->pParse->db;
  n = p_اظہاریہ->x.pList ? p_اظہاریہ->x.pList->n_اظہاریہ : 0;
  pDef = sqlite3_ڈھونڈو_Function(db, p_اظہاریہ->u.zToken, n, ENC(db), 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  اگر( pDef==0 ) واپس WRC_Continue;
#ورنہ
  أكد( pDef!=0 );
#اگر_ختم
  اگر( (pDef->funcFlags & SQLITE_نتیجہ_SUBTYPE)!=0 ){
    pWalker->e_رمز = 1;
    واپس WRC_Abort;
  }
  واپس WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
صحیح sqlite3_اظہاریہ_Can_Return_Subtype(Parse *pParse, اظہاریہ *p_اظہاریہ){
  Walker w;
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_اظہاریہ_Callback = اظہاریہ_گرہ_Can_Return_Subtype;
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  واپس w.e_رمز;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ساکن SQLITE_NOINLINE صحیح sqlite3_اشاریہ_دار_اظہاریہ_Lookup(
  Parse *pParse,   /* The parsing context */
  اظہاریہ *p_اظہاریہ,     /* The expression to potentially bypass */
  صحیح ہدف       /* Where to store the result of the expression */
){
  اشاریہ_دار_اظہاریہ *p;
  Vdbe *v;
  برائے(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 اظہاریہ_Aff;
    صحیح i_ڈیٹا_Cur = p->i_ڈیٹا_Cur;
    اگر( i_ڈیٹا_Cur<0 ) جاری;
    اگر( pParse->iSelfTab ){
      اگر( p->i_ڈیٹا_Cur!=pParse->iSelfTab-1 ) جاری;
      i_ڈیٹا_Cur = -1;
    }
    اگر( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, p->p_اظہاریہ, i_ڈیٹا_Cur)!=0 ) جاری;
    أكد( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    اظہاریہ_Aff = sqlite3_اظہاریہ_Affinity(p_اظہاریہ);
    اگر( (اظہاریہ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (اظہاریہ_Aff==SQLITE_AFF_متن && p->aff!=SQLITE_AFF_متن)
     || (اظہاریہ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      جاری;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_SubtArg)
     && sqlite3_اظہاریہ_Can_Return_Subtype(pParse, p_اظہاریہ) 
    ){
      جاری;
    }

    v = pParse->pVdbe;
    أكد( v!=0 );
    اگر( p->b_Maybe_Null_صف || sqlite3_اظہاریہ_Is_Constant(pParse, p_اظہاریہ) ){
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
      sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, p->iIdxCur, addr+3, ہدف);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, p->iIdxCur, p->i_Idx_ستون, ہدف);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ستون));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ, ہدف);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_یہاں(v, addr+2);
    }ورنہ{
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, p->iIdxCur, p->i_Idx_ستون, ہدف);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ستون));
    }
    واپس ہدف;
  }
  واپس -1;  /* Not found */
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
ساکن صحیح اظہاریہ_Partidx_اظہاریہ_Lookup(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح i_ہدف){
  اشاریہ_دار_اظہاریہ *p;
  برائے(p=pParse->p_Idx_Part_اظہاریہ; p; p=p->pIENext){
    اگر( p_اظہاریہ->i_عمود==p->i_Idx_ستون && p_اظہاریہ->i_جدول==p->i_ڈیٹا_Cur ){
      Vdbe *v = pParse->pVdbe;
      صحیح addr = 0;
      صحیح ret;

      اگر( p->b_Maybe_Null_صف ){
        addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_If_Null_صف, p->iIdxCur);
      }
      ret = sqlite3_اظہاریہ_رمز_ہدف(pParse, p->p_اظہاریہ, i_ہدف);
      sqlite3_Vdbe_جوڑو_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ثابت حرف*)&p->aff, 1);
      اگر( sqlite3_اظہاریہ_Can_Return_Subtype(pParse, p->p_اظہاریہ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_جوڑو_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      اگر( addr ){
        sqlite3_Vdbe_Jump_یہاں(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      واپس ret;
    }
  }
  واپس 0;
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
ساکن SQLITE_NOINLINE صحیح اظہاریہ_رمز_ہدف_And_Or(
  Parse *pParse,     /* Parsing context */
  اظہاریہ *p_اظہاریہ,       /* AND or OR expression to be coded */
  صحیح ہدف,        /* Put result in this register, guaranteed */
  صحیح *pTmpReg       /* Write a temporary register here */
){
  صحیح op;            /* The opcode.  TK_AND or TK_OR */
  صحیح skipOp;        /* Opcode for the branch that skips one operand */
  صحیح addrSkip;      /* Branch instruction that skips one of the operands */
  صحیح regSS = 0;     /* Register holding computed operand when other omitted */
  صحیح r1, r2;        /* Registers for left and right operands, respectively */
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
  اگر( pAlt!=p_اظہاریہ ){
    r1 = sqlite3_اظہاریہ_رمز_ہدف(pParse, pAlt, ہدف);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_And, r1, r1, ہدف);
    واپس ہدف;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  اگر( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ->p_دایاں, ہدف);
    addrSkip = sqlite3_Vdbe_جوڑو_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, pTmpReg);
  }ورنہ{
    /* Compute the left operand first */
    r1 = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ->p_بایاں, ہدف);
    اگر( اظہاریہ_Has_Property(p_اظہاریہ->p_دایاں, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_جوڑو_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ورنہ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, pTmpReg);
  }
  sqlite3_Vdbe_جوڑو_Op3(v, op, r2, r1, ہدف);
  testcase( (*pTmpReg)==0 );
  اگر( addrSkip ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_یہاں(v, addrSkip);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Or, regSS, regSS, ہدف);
    VdbeComment((v, "short-circut value"));
  }
  واپس ہدف;
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
صحیح sqlite3_اظہاریہ_رمز_ہدف(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح ہدف){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  صحیح op;                   /* The opcode being coded */
  صحیح اندر_Reg = ہدف;       /* Results stored in register inReg */
  صحیح regFree1 = 0;         /* If non-zero free this temporary register */
  صحیح regFree2 = 0;         /* If non-zero free this temporary register */
  صحیح r1, r2;               /* Various register numbers */
  اظہاریہ عارضی_X;               /* Temporary expression node */
  صحیح p5 = 0;

  أكد( ہدف>0 && ہدف<=pParse->nMem );
  أكد( v!=0 );

اظہاریہ_رمز_doover:
  اگر( p_اظہاریہ==0 ){
    op = TK_NULL;
  }ورنہ اگر( pParse->pIdxEpr!=0
   && !اظہاریہ_Has_Property(p_اظہاریہ, EP_برگ)
   && (r1 = sqlite3_اشاریہ_دار_اظہاریہ_Lookup(pParse, p_اظہاریہ, ہدف))>=0
  ){
    واپس r1;
  }ورنہ{
    أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
    op = p_اظہاریہ->op;
  }
  أكد( op!=TK_ترتیب );
  انتخاب( op ){
    حالت TK_AGG_عمود: {
      AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
      ساخت Agg_Info_ستون *p_ستون;
      أكد( pAggInfo!=0 );
      أكد( p_اظہاریہ->iAgg>=0 );
      اگر( p_اظہاریہ->iAgg>=pAggInfo->n_عمود ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_جوڑو_Op3(v, OP_NotNull, ہدف, 1, 20230325);
        Vdbe_Coverage_Never_لیا_گیا(v);
#اگر_ختم
        توڑو;
      }
      p_ستون = &pAggInfo->a_ستون[p_اظہاریہ->iAgg];
      اگر( !pAggInfo->براہ_راست_منوال ){
        واپس Agg_Info_عمود_Reg(pAggInfo, p_اظہاریہ->iAgg);
      }ورنہ اگر( pAggInfo->use_ترتیب_کاری_Idx ){
        جدول *pTab = p_ستون->pTab;
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, pAggInfo->ترتیب_کاری_Idx_PTab,
                              p_ستون->i_Sorter_عمود, ہدف);
        اگر( pTab==0 ){
          /* No comment added */
        }ورنہ اگر( p_ستون->i_عمود<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ورنہ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_ستون[p_ستون->i_عمود].zCnName));
          اگر( pTab->a_ستون[p_ستون->i_عمود].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, ہدف);
          }
        }
        واپس ہدف;
      }ورنہ اگر( p_اظہاریہ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, p_اظہاریہ->i_جدول, p_اظہاریہ->i_عمود, ہدف);
        واپس ہدف;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    حالت TK_عمود: {
      صحیح iTab = p_اظہاریہ->i_جدول;
      صحیح iReg;
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        صحیح aff;
        iReg = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ->p_بایاں,ہدف);
        أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
        أكد( p_اظہاریہ->y.pTab!=0 );
        aff = sqlite3_جدول_عمود_Affinity(p_اظہاریہ->y.pTab, p_اظہاریہ->i_عمود);
        اگر( aff>SQLITE_AFF_BLOB ){
          ساکن ثابت حرف zAff[] = "B\000C\000D\000E\000F";
          أكد( SQLITE_AFF_BLOB=='A' );
          أكد( SQLITE_AFF_متن=='B' );
          sqlite3_Vdbe_جوڑو_Op4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        واپس iReg;
      }
      اگر( iTab<0 ){
        اگر( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          عمود *p_ستون;
          جدول *pTab;
          صحیح iSrc;
          صحیح i_ستون = p_اظہاریہ->i_عمود;
          أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
          pTab = p_اظہاریہ->y.pTab;
          أكد( pTab!=0 );
          أكد( i_ستون>=XN_ROWID );
          أكد( i_ستون<pTab->n_ستون );
          اگر( i_ستون<0 ){
            واپس -1-pParse->iSelfTab;
          }
          p_ستون = pTab->a_ستون + i_ستون;
          testcase( i_ستون!=sqlite3_جدول_عمود_مقصد_Storage(pTab,i_ستون) );
          iSrc = sqlite3_جدول_عمود_مقصد_Storage(pTab, i_ستون) - pParse->iSelfTab;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
          اگر( p_ستون->ستون_Flags & COLFLAG_GENERATED ){
            اگر( p_ستون->ستون_Flags & COLFLAG_BUSY ){
              sqlite3_Error_پیغام(pParse, "generated column loop on \"%s\"",
                              p_ستون->zCnName);
              واپس 0;
            }
            p_ستون->ستون_Flags |= COLFLAG_BUSY;
            اگر( p_ستون->ستون_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_اظہاریہ_رمز_Generated_عمود(pParse, pTab, p_ستون, iSrc);
            }
            p_ستون->ستون_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            واپس iSrc;
          }ورنہ
#اگر_ختم /* SQLITE_OMIT_GENERATED_COLUMNS */
          اگر( p_ستون->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy, iSrc, ہدف);
            sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, ہدف);
            واپس ہدف;
          }ورنہ{
            واپس iSrc;
          }
        }ورنہ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ورنہ اگر( pParse->p_Idx_Part_اظہاریہ 
       && 0!=(r1 = اظہاریہ_Partidx_اظہاریہ_Lookup(pParse, p_اظہاریہ, ہدف))
      ){
        واپس r1;
      }
      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      أكد( p_اظہاریہ->y.pTab!=0 );
      iReg = sqlite3_اظہاریہ_رمز_Get_عمود(pParse, p_اظہاریہ->y.pTab,
                               p_اظہاریہ->i_عمود, iTab, ہدف,
                               p_اظہاریہ->op2);
      واپس iReg;
    }
    حالت TK_INTEGER: {
      رمز_Integer(pParse, p_اظہاریہ, 0, ہدف);
      واپس ہدف;
    }
    حالت TK_TRUEFALSE: {
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ), ہدف);
      واپس ہدف;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
    حالت TK_FLOAT: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      رمز_Real(v, p_اظہاریہ->u.zToken, 0, ہدف);
      واپس ہدف;
    }
#اگر_ختم
    حالت TK_STRING: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_Vdbe_بھرو_String(v, ہدف, p_اظہاریہ->u.zToken);
      واپس ہدف;
    }
    حالت TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, ہدف, ہدف + p_اظہاریہ->y.nReg - 1);
      واپس ہدف;
    }
    طےشدہ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      أكد( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
      واپس ہدف;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_BLOB_LITERAL
    حالت TK_BLOB: {
      صحیح n;
      ثابت حرف *z;
      حرف *zBlob;
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      أكد( p_اظہاریہ->u.zToken[0]=='x' || p_اظہاریہ->u.zToken[0]=='X' );
      أكد( p_اظہاریہ->u.zToken[1]=='\'' );
      z = &p_اظہاریہ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      أكد( z[n]=='\'' );
      zBlob = sqlite3_Hex_مقصد_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Blob, n/2, ہدف, 0, zBlob, P4_DYNAMIC);
      واپس ہدف;
    }
#اگر_ختم
    حالت TK_VARIABLE: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      أكد( p_اظہاریہ->u.zToken!=0 );
      أكد( p_اظہاریہ->u.zToken[0]!=0 );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Variable, p_اظہاریہ->i_عمود, ہدف);
      واپس ہدف;
    }
    حالت TK_رجسٹر: {
      واپس p_اظہاریہ->i_جدول;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CAST
    حالت TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ->p_بایاں, ہدف);
      أكد( اندر_Reg==ہدف );
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Cast, ہدف,
                        sqlite3AffinityType(p_اظہاریہ->u.zToken, 0));
      واپس اندر_Reg;
    }
#اگر_ختم /* SQLITE_OMIT_CAST */
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
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      صحیح addrIsNull = 0;
      اگر( sqlite3_اظہاریہ_Is_سمتیہ(p_بایاں) ){
        رمز_سمتیہ_Compare(pParse, p_اظہاریہ, ہدف, op, p5);
      }ورنہ{
        اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                     &r1, &r2, &regFree1, &regFree2);
        }ورنہ{
          r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
          r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        }
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, اندر_Reg);
        رمز_Compare(pParse, p_بایاں, p_اظہاریہ->p_دایاں, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            اظہاریہ_Has_Property(p_اظہاریہ,EP_Commuted));
        أكد(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        أكد(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        أكد(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        أكد(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        أكد(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        أكد(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        اگر( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, اندر_Reg);
        }ورنہ{
          sqlite3_Vdbe_جوڑو_Op3(v, OP_ZeroOrNull, r1, اندر_Reg, r2);
          اگر( addrIsNull ){
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_یہاں(v, addrIsNull);
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, اندر_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      توڑو;
    }
    حالت TK_AND:
    حالت TK_OR: {
      اندر_Reg = اظہاریہ_رمز_ہدف_And_Or(pParse, p_اظہاریہ, ہدف, &regFree1);
      توڑو;
    }
    حالت TK_PLUS:
    حالت TK_ستارہ:
    حالت TK_MINUS:
    حالت TK_REM:
    حالت TK_BITAND:
    حالت TK_BITOR:
    حالت TK_SLASH:
    حالت TK_LSHIFT:
    حالت TK_RSHIFT:
    حالت TK_CONCAT: {
      صحیح addrIsNull;
      أكد( TK_PLUS==OP_جوڑو );           testcase( op==TK_PLUS );
      أكد( TK_MINUS==OP_تفریق );     testcase( op==TK_MINUS );
      أكد( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      أكد( TK_BITAND==OP_ثنائی_ہندسہ_And );      testcase( op==TK_BITAND );
      أكد( TK_BITOR==OP_ثنائی_ہندسہ_Or );        testcase( op==TK_BITOR );
      أكد( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      أكد( TK_LSHIFT==OP_سرکاؤ_بایاں );   testcase( op==TK_LSHIFT );
      أكد( TK_RSHIFT==OP_سرکاؤ_دایاں );  testcase( op==TK_RSHIFT );
      أكد( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_جوڑو_Op3(v, op, r2, r1, ہدف);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      اگر( addrIsNull ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_یہاں(v, addrIsNull);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
        VdbeComment((v, "short-circut value"));
      }
      توڑو;
    }
    حالت TK_UMINUS: {
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      أكد( p_بایاں );
      اگر( p_بایاں->op==TK_INTEGER ){
        رمز_Integer(pParse, p_بایاں, 1, ہدف);
        واپس ہدف;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
      }ورنہ اگر( p_بایاں->op==TK_FLOAT ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        رمز_Real(v, p_بایاں->u.zToken, 1, ہدف);
        واپس ہدف;
#اگر_ختم
      }ورنہ{
        عارضی_X.op = TK_INTEGER;
        عارضی_X.flags = EP_Int_قدر|EP_TokenOnly;
        عارضی_X.u.i_قدر = 0;
        اظہاریہ_Clear_VVAProperties(&عارضی_X);
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, &عارضی_X, &regFree1);
        r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree2);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_تفریق, r2, r1, ہدف);
        testcase( regFree2==0 );
      }
      توڑو;
    }
    حالت TK_BITNOT:
    حالت TK_NOT: {
      أكد( TK_BITNOT==OP_ثنائی_ہندسہ_Not );   testcase( op==TK_BITNOT );
      أكد( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, اندر_Reg);
      توڑو;
    }
    حالت TK_TRUTH: {
      صحیح isTrue;    /* IS TRUE or IS NOT TRUE */
      صحیح b_معمول;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      b_معمول = p_اظہاریہ->op2==TK_IS;
      testcase( isTrue && b_معمول);
      testcase( !isTrue && b_معمول);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IsTrue, r1, اندر_Reg, !isTrue, isTrue ^ b_معمول);
      توڑو;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      صحیح addr;
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, ہدف);
      r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_جوڑو_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, ہدف);
      sqlite3_Vdbe_Jump_یہاں(v, addr);
      توڑو;
    }
    حالت TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_اظہاریہ->pAggInfo;
      اگر( pInfo==0
       || NEVER(p_اظہاریہ->iAgg<0)
       || NEVER(p_اظہاریہ->iAgg>=pInfo->nFunc)
      ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        sqlite3_Error_پیغام(pParse, "misuse of aggregate: %#T()", p_اظہاریہ);
      }ورنہ{
        واپس AggInfoFuncReg(pInfo, p_اظہاریہ->iAgg);
      }
      توڑو;
    }
    حالت TK_FUNCTION: {
      اظہاریہ_List *pFarg;       /* List of function arguments */
      صحیح nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ثابت حرف *zId;       /* The function name */
      u32 const_نقاب = 0;     /* Mask of function arguments that are constant */
      صحیح i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
        واپس p_اظہاریہ->y.pWin->reg_نتیجہ;
      }
#اگر_ختم

      اگر( Const_ضربی_عدد_Ok(pParse)
       && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        واپس sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(pParse, p_اظہاریہ, -1);
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly) );
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      pFarg = p_اظہاریہ->x.pList;
      nFarg = pFarg ? pFarg->n_اظہاریہ : 0;
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      zId = p_اظہاریہ->u.zToken;
      pDef = sqlite3_ڈھونڈو_Function(db, zId, nFarg, enc, 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      اگر( pDef==0 && pParse->explain ){
        pDef = sqlite3_ڈھونڈو_Function(db, "unknown", nFarg, enc, 0);
      }
#اگر_ختم
      اگر( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_اندرونی)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_اندرونی_Func)==0)
      ){
        sqlite3_Error_پیغام(pParse, "unknown function: %#T()", p_اظہاریہ);
        توڑو;
      }
      اگر( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        أكد( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        أكد( (pDef->funcFlags & SQLITE_FUNC_براہ_راست)==0 );
        واپس اظہاریہ_رمز_Inline_Function(pParse, pFarg,
             SQLITE_PTR_مقصد_INT(pDef->p_User_ڈیٹا), ہدف);
      }ورنہ اگر( pDef->funcFlags & (SQLITE_FUNC_براہ_راست|SQLITE_FUNC_UNSAFE) ){
        sqlite3_اظہاریہ_Function_Usable(pParse, p_اظہاریہ, pDef);
      }

      برائے(i=0; i<nFarg; i++){
        اگر( i<32 && sqlite3_اظہاریہ_Is_Constant(pParse, pFarg->a[i].p_اظہاریہ) ){
          testcase( i==31 );
          const_نقاب |= MASKBIT32(i);
        }
        اگر( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, pFarg->a[i].p_اظہاریہ);
        }
      }
      اگر( pFarg ){
        اگر( const_نقاب ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ورنہ{
          r1 = sqlite3_Get_عارضی_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        اگر( (pDef->funcFlags & (SQLITE_FUNC_لمبائی|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 اظہاریہ_Op;
          أكد( nFarg==1 );
          أكد( pFarg->a[0].p_اظہاریہ!=0 );
          اظہاریہ_Op = pFarg->a[0].p_اظہاریہ->op;
          اگر( اظہاریہ_Op==TK_عمود || اظہاریہ_Op==TK_AGG_عمود ){
            أكد( SQLITE_FUNC_لمبائی==OPFLAG_LENGTHARG );
            أكد( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            أكد( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            أكد( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_اظہاریہ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_ضربی_عدد);
      }ورنہ{
        r1 = 0;
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
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
      اگر( nFarg>=2 && اظہاریہ_Has_Property(p_اظہاریہ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_اظہاریہ);
      }ورنہ اگر( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_اظہاریہ);
      }
#اگر_ختم
      اگر( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        اگر( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_جوڑو_Op4(v, OP_CollSeq, 0, 0, 0, (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جوڑو_Function_Call(pParse, const_نقاب, r1, ہدف, nFarg,
                                 pDef, p_اظہاریہ->op2);
      اگر( nFarg ){
        اگر( const_نقاب==0 ){
          sqlite3_Release_عارضی_Range(pParse, r1, nFarg);
        }ورنہ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_نقاب, 1);
        }
      }
      واپس ہدف;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_EXISTS:
    حالت TK_SELECT: {
      صحیح n_ستون;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      اگر( pParse->db->mallocFailed ){
        واپس 0;
      }ورنہ اگر( op==TK_SELECT
             && ALWAYS( اظہاریہ_Use_XSelect(p_اظہاریہ) )
             && (n_ستون = p_اظہاریہ->x.pSelect->pEList->n_اظہاریہ)!=1
      ){
        sqlite3SubselectError(pParse, n_ستون, 1);
      }ورنہ{
        واپس sqlite3_رمز_Subselect(pParse, p_اظہاریہ);
      }
      توڑو;
    }
    حالت TK_SELECT_عمود: {
      صحیح n;
      اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
      اگر( p_بایاں->i_جدول==0 || pParse->withinRJSubrtn > p_بایاں->op2 ){
        p_بایاں->i_جدول = sqlite3_رمز_Subselect(pParse, p_بایاں);
        p_بایاں->op2 = pParse->withinRJSubrtn;
      }
      أكد( p_بایاں->op==TK_SELECT || p_بایاں->op==TK_ERROR );
      n = sqlite3_اظہاریہ_سمتیہ_سائز(p_بایاں);
      اگر( p_اظہاریہ->i_جدول!=n ){
        sqlite3_Error_پیغام(pParse, "%d columns assigned %d values",
                                p_اظہاریہ->i_جدول, n);
      }
      واپس p_بایاں->i_جدول + p_اظہاریہ->i_عمود;
    }
    حالت TK_اندر: {
      صحیح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحیح destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
      sqlite3_اظہاریہ_رمز_اندر(pParse, p_اظہاریہ, destIfFalse, destIfNull);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, ہدف);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, ہدف, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      واپس ہدف;
    }
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */


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
      اظہاریہ_رمز_Between(pParse, p_اظہاریہ, ہدف, 0, 0);
      واپس ہدف;
    }
    حالت TK_COLLATE: {
      اگر( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        أكد( p_اظہاریہ->p_بایاں );
        sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ->p_بایاں, ہدف);
        sqlite3_Vdbe_جوڑو_Op1(v, OP_ClrSubtype, ہدف);
        واپس ہدف;
      }ورنہ{
        p_اظہاریہ = p_اظہاریہ->p_بایاں;
        جاؤ اظہاریہ_رمز_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    حالت TK_وسعت:
    حالت TK_UPLUS: {
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
      جاؤ اظہاریہ_رمز_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
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
      جدول *pTab;
      صحیح i_ستون;
      صحیح p1;

      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      pTab = p_اظہاریہ->y.pTab;
      i_ستون = p_اظہاریہ->i_عمود;
      p1 = p_اظہاریہ->i_جدول * (pTab->n_ستون+1) + 1
                     + sqlite3_جدول_عمود_مقصد_Storage(pTab, i_ستون);

      أكد( p_اظہاریہ->i_جدول==0 || p_اظہاریہ->i_جدول==1 );
      أكد( i_ستون>=-1 && i_ستون<pTab->n_ستون );
      أكد( pTab->iPKey<0 || i_ستون!=pTab->iPKey );
      أكد( p1>=0 && p1<(pTab->n_ستون*2+2) );

      sqlite3_Vdbe_جوڑو_Op2(v, OP_Param, p1, ہدف);
      VdbeComment((v, "r[%d]=%s.%s", ہدف,
        (p_اظہاریہ->i_جدول ? "new" : "old"),
        (p_اظہاریہ->i_عمود<0 ? "rowid" : p_اظہاریہ->y.pTab->a_ستون[i_ستون].zCnName)
      ));

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      اگر( i_ستون>=0 && pTab->a_ستون[i_ستون].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_جوڑو_Op1(v, OP_RealAffinity, ہدف);
      }
#اگر_ختم
      توڑو;
    }

    حالت TK_سمتیہ: {
      sqlite3_Error_پیغام(pParse, "row value misused");
      توڑو;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    حالت TK_IF_NULL_صف: {
      صحیح addrINR;
      u8 ok_Const_ضربی_عدد = pParse->ok_Const_ضربی_عدد;
      AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
      اگر( pAggInfo ){
        أكد( p_اظہاریہ->iAgg>=0 && p_اظہاریہ->iAgg<pAggInfo->n_عمود );
        اگر( !pAggInfo->براہ_راست_منوال ){
          اندر_Reg = Agg_Info_عمود_Reg(pAggInfo, p_اظہاریہ->iAgg);
          توڑو;
        }
        اگر( p_اظہاریہ->pAggInfo->use_ترتیب_کاری_Idx ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, pAggInfo->ترتیب_کاری_Idx_PTab,
                            pAggInfo->a_ستون[p_اظہاریہ->iAgg].i_Sorter_عمود,
                            ہدف);
          اندر_Reg = ہدف;
          توڑو;
        }
      }
      addrINR = sqlite3_Vdbe_جوڑو_Op3(v, OP_If_Null_صف, p_اظہاریہ->i_جدول, 0, ہدف);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_ضربی_عدد = 0;   /* note (1) above */
      sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ->p_بایاں, ہدف);
      أكد( ہدف==اندر_Reg );
      pParse->ok_Const_ضربی_عدد = ok_Const_ضربی_عدد;
      sqlite3_Vdbe_Jump_یہاں(v, addrINR);
      توڑو;
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
      صحیح انتہا_Label;                     /* GOTO label for end of CASE stmt */
      صحیح اگلا_Case;                     /* GOTO label for next WHEN clause */
      صحیح n_اظہاریہ;                        /* 2x number of WHEN terms */
      صحیح i;                            /* Loop counter */
      اظہاریہ_List *pEList;                 /* List of WHEN terms */
      ساخت اظہاریہ_List_شے *aListelem;  /* Array of WHEN terms */
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
      انتہا_Label = sqlite3VdbeMakeLabel(pParse);
      اگر( (pX = p_اظہاریہ->p_بایاں)!=0 ){
        pDel = sqlite3_اظہاریہ_Dup(db, pX, 0);
        اگر( db->mallocFailed ){
          sqlite3_اظہاریہ_Delete(db, pDel);
          توڑو;
        }
        testcase( pX->op==TK_عمود );
        sqlite3_اظہاریہ_مقصد_رجسٹر(pDel, اظہاریہ_رمز_سمتیہ(pParse, pDel, &regFree1));
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
      برائے(i=0; i<n_اظہاریہ-1; i=i+2){
        اگر( pX ){
          أكد( pTest!=0 );
          opCompare.p_دایاں = aListelem[i].p_اظہاریہ;
        }ورنہ{
          pTest = aListelem[i].p_اظہاریہ;
        }
        اگلا_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_عمود );
        sqlite3_اظہاریہ_If_False(pParse, pTest, اگلا_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_اظہاریہ->op==TK_عمود );
        sqlite3_اظہاریہ_رمز(pParse, aListelem[i+1].p_اظہاریہ, ہدف);
        sqlite3VdbeGoto(v, انتہا_Label);
        sqlite3VdbeResolveLabel(v, اگلا_Case);
      }
      اگر( (n_اظہاریہ&1)!=0 ){
        sqlite3_اظہاریہ_رمز(pParse, pEList->a[n_اظہاریہ-1].p_اظہاریہ, ہدف);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, ہدف);
      }
      sqlite3_اظہاریہ_Delete(db, pDel);
      سیٹ_Do_Not_ملاؤ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, انتہا_Label);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
    حالت TK_RAISE: {
      أكد( p_اظہاریہ->aff_اظہاریہ==OE_Rollback
           || p_اظہاریہ->aff_اظہاریہ==OE_Abort
           || p_اظہاریہ->aff_اظہاریہ==OE_Fail
           || p_اظہاریہ->aff_اظہاریہ==OE_Ignore
      );
      اگر( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_پیغام(pParse,
                       "RAISE() may only be used within a trigger-program");
        واپس 0;
      }
      اگر( p_اظہاریہ->aff_اظہاریہ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      اگر( p_اظہاریہ->aff_اظہاریہ==OE_Ignore ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_اظہاریہ->aff_اظہاریہ, r1);
      }
      توڑو;
    }
#اگر_ختم
  }
  sqlite3_Release_عارضی_Reg(pParse, regFree1);
  sqlite3_Release_عارضی_Reg(pParse, regFree2);
  واپس اندر_Reg;
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
صحیح sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(
  Parse *pParse,    /* Parsing context */
  اظہاریہ *p_اظہاریہ,      /* The expression to code when the VDBE initializes */
  صحیح regDest       /* Store the value in this register */
){
  اظہاریہ_List *p;
  أكد( Const_ضربی_عدد_Ok(pParse) );
  أكد( regDest!=0 );
  p = pParse->p_Const_اظہاریہ;
  اگر( regDest<0 && p ){
    ساخت اظہاریہ_List_شے *p_شے;
    صحیح i;
    برائے(p_شے=p->a, i=p->n_اظہاریہ; i>0; p_شے++, i--){
      اگر( p_شے->fg.reusable
       && sqlite3_اظہاریہ_Compare(0,p_شے->p_اظہاریہ,p_اظہاریہ,-1)==0
      ){
        واپس p_شے->u.i_Const_اظہاریہ_Reg;
      }
    }
  }
  p_اظہاریہ = sqlite3_اظہاریہ_Dup(pParse->db, p_اظہاریہ, 0);
  اگر( p_اظہاریہ!=0 && اظہاریہ_Has_Property(p_اظہاریہ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    صحیح addr;
    أكد( v );
    addr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_ضربی_عدد = 0;
    اگر( !pParse->db->mallocFailed ){
      اگر( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ, regDest);
    }
    pParse->ok_Const_ضربی_عدد = 1;
    sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ);
    sqlite3_Vdbe_Jump_یہاں(v, addr);
  }ورنہ{
    p = sqlite3_اظہاریہ_List_Append(pParse, p, p_اظہاریہ);
    اگر( p ){
       ساخت اظہاریہ_List_شے *p_شے = &p->a[p->n_اظہاریہ-1];
       p_شے->fg.reusable = regDest<0;
       اگر( regDest<0 ) regDest = ++pParse->nMem;
       p_شے->u.i_Const_اظہاریہ_Reg = regDest;
    }
    pParse->p_Const_اظہاریہ = p;
  }
  واپس regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE خالی sqlite3_اظہاریہ_Null_رجسٹر_Range(
  Parse *pParse,   /* Parsing context */
  صحیح iReg,        /* First register to set to NULL */
  صحیح nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_ضربی_عدد = pParse->ok_Const_ضربی_عدد;
  اظہاریہ t;
  memset(&t, 0, حجم(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_ضربی_عدد = 1;
  sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_ضربی_عدد = ok_Const_ضربی_عدد;
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
صحیح sqlite3_اظہاریہ_رمز_عارضی(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح *pReg){
  صحیح r2;
  p_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
  اگر( Const_ضربی_عدد_Ok(pParse)
   && ALWAYS(p_اظہاریہ!=0)
   && p_اظہاریہ->op!=TK_رجسٹر
   && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse, p_اظہاریہ)
  ){
    *pReg  = 0;
    r2 = sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(pParse, p_اظہاریہ, -1);
  }ورنہ{
    صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
    r2 = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ, r1);
    اگر( r2==r1 ){
      *pReg = r1;
    }ورنہ{
      sqlite3_Release_عارضی_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  واپس r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
خالی sqlite3_اظہاریہ_رمز(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح ہدف){
  صحیح اندر_Reg;

  أكد( p_اظہاریہ==0 || !اظہاریہ_Has_VVAProperty(p_اظہاریہ,EP_Immutable) );
  أكد( ہدف>0 && ہدف<=pParse->nMem );
  أكد( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  اگر( pParse->pVdbe==0 ) واپس;
  اندر_Reg = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ, ہدف);
  اگر( اندر_Reg!=ہدف ){
    u8 op;
    اظہاریہ *pX = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ);
    testcase( pX!=p_اظہاریہ );
    اگر( ALWAYS(pX)
     && (اظہاریہ_Has_Property(pX,EP_Subquery) || pX->op==TK_رجسٹر)
    ){
      op = OP_Copy;
    }ورنہ{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_جوڑو_Op2(pParse->pVdbe, op, اندر_Reg, ہدف);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
خالی sqlite3_اظہاریہ_رمز_Copy(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح ہدف){
  sqlite3 *db = pParse->db;
  p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
  اگر( !db->mallocFailed ) sqlite3_اظہاریہ_رمز(pParse, p_اظہاریہ, ہدف);
  sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
خالی sqlite3_اظہاریہ_رمز_Factorable(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح ہدف){
  اگر( pParse->ok_Const_ضربی_عدد && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ) ){
    sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(pParse, p_اظہاریہ, ہدف);
  }ورنہ{
    sqlite3_اظہاریہ_رمز_Copy(pParse, p_اظہاریہ, ہدف);
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
صحیح sqlite3_اظہاریہ_رمز_اظہاریہ_List(
  Parse *pParse,     /* Parsing context */
  اظہاریہ_List *pList,   /* The expression list to be coded */
  صحیح ہدف,        /* Where to write results */
  صحیح srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ساخت اظہاریہ_List_شے *p_شے;
  صحیح i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  أكد( pList!=0 );
  أكد( ہدف>0 );
  أكد( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_اظہاریہ;
  اگر( !Const_ضربی_عدد_Ok(pParse) ) flags &= ~SQLITE_ECEL_ضربی_عدد;
  برائے(p_شے=pList->a, i=0; i<n; i++, p_شے++){
    اظہاریہ *p_اظہاریہ = p_شے->p_اظہاریہ;
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    اگر( p_شے->fg.bSorterRef ){
      i--;
      n--;
    }ورنہ
#اگر_ختم
    اگر( (flags & SQLITE_ECEL_REF)!=0 && (j = p_شے->u.x.i_ترتیب_By_ستون)>0 ){
      اگر( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, copyOp, j+srcReg-1, ہدف+i);
      }
    }ورنہ اگر( (flags & SQLITE_ECEL_ضربی_عدد)!=0
           && sqlite3_اظہاریہ_Is_Constant_Not_Join(pParse,p_اظہاریہ)
    ){
      sqlite3_اظہاریہ_رمز_تسلسل_Just_Once(pParse, p_اظہاریہ, ہدف+i);
    }ورنہ{
      صحیح اندر_Reg = sqlite3_اظہاریہ_رمز_ہدف(pParse, p_اظہاریہ, ہدف+i);
      اگر( اندر_Reg!=ہدف+i ){
        VdbeOp *pOp;
        اگر( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_آخری_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==اندر_Reg
         && pOp->p2+pOp->p3+1==ہدف+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ورنہ{
          sqlite3_Vdbe_جوڑو_Op2(v, copyOp, اندر_Reg, ہدف+i);
        }
      }
    }
  }
  واپس n;
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
ساکن خالی اظہاریہ_رمز_Between(
  Parse *pParse,    /* Parsing and code generating context */
  اظہاریہ *p_اظہاریہ,      /* The BETWEEN expression */
  صحیح dest,         /* Jump destination or storage location */
  خالی (*xJump)(Parse*,اظہاریہ*,صحیح,صحیح), /* Action to take */
  صحیح jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  اظہاریہ اظہاریہ_And;     /* The AND operator in  x>=y AND x<=z  */
  اظہاریہ comp_بایاں;    /* The  x>=y  term */
  اظہاریہ comp_دایاں;   /* The  x<=z  term */
  صحیح regFree1 = 0; /* Temporary use register */
  اظہاریہ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_بایاں, 0, حجم(اظہاریہ));
  memset(&comp_دایاں, 0, حجم(اظہاریہ));
  memset(&اظہاریہ_And, 0, حجم(اظہاریہ));

  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  pDel = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ->p_بایاں, 0);
  اگر( db->mallocFailed==0 ){
    اظہاریہ_And.op = TK_AND;
    اظہاریہ_And.p_بایاں = &comp_بایاں;
    اظہاریہ_And.p_دایاں = &comp_دایاں;
    comp_بایاں.op = TK_GE;
    comp_بایاں.p_بایاں = pDel;
    comp_بایاں.p_دایاں = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
    comp_دایاں.op = TK_LE;
    comp_دایاں.p_بایاں = pDel;
    comp_دایاں.p_دایاں = p_اظہاریہ->x.pList->a[1].p_اظہاریہ;
    sqlite3_اظہاریہ_مقصد_رجسٹر(pDel, اظہاریہ_رمز_سمتیہ(pParse, pDel, &regFree1));
    اگر( xJump ){
      xJump(pParse, &اظہاریہ_And, dest, jumpIfNull);
    }ورنہ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_اظہاریہ_رمز_ہدف(pParse, &اظہاریہ_And, dest);
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
خالی sqlite3_اظہاریہ_If_True(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح dest, صحیح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحیح op = 0;
  صحیح regFree1 = 0;
  صحیح regFree2 = 0;
  صحیح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  اگر( NEVER(v==0) )     واپس;  /* Existence of VDBE checked by caller */
  اگر( NEVER(p_اظہاریہ==0) ) واپس;  /* No way this can happen */
  أكد( !اظہاریہ_Has_VVAProperty(p_اظہاریہ, EP_Immutable) );
  op = p_اظہاریہ->op;
  انتخاب( op ){
    حالت TK_AND:
    حالت TK_OR: {
      اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
      اگر( pAlt!=p_اظہاریہ ){
        sqlite3_اظہاریہ_If_True(pParse, pAlt, dest, jumpIfNull);
      }ورنہ{
        اظہاریہ *p_پہلا, *p_دوسرا;
        اگر( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
          p_پہلا = p_اظہاریہ->p_دایاں;
          p_دوسرا = p_اظہاریہ->p_بایاں;
        }ورنہ{
          p_پہلا = p_اظہاریہ->p_بایاں;
          p_دوسرا = p_اظہاریہ->p_دایاں;
        }
        اگر( op==TK_AND ){
          صحیح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_False(pParse, p_پہلا, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اظہاریہ_If_True(pParse, p_دوسرا, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ورنہ{
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_True(pParse, p_پہلا, dest, jumpIfNull);
          sqlite3_اظہاریہ_If_True(pParse, p_دوسرا, dest, jumpIfNull);
        }
      }
      توڑو;
    }
    حالت TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest, jumpIfNull);
      توڑو;
    }
    حالت TK_TRUTH: {
      صحیح isNot;      /* IS NOT TRUE or IS NOT FALSE */
      صحیح isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اظہاریہ->op2==TK_ISNOT;
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      اگر( isTrue ^ isNot ){
        sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ورنہ{
        sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      توڑو;
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
      اگر( sqlite3_اظہاریہ_Is_سمتیہ(p_اظہاریہ->p_بایاں) ) جاؤ default_اظہاریہ;
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      رمز_Compare(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں, op,
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
      اگر( addrIsNull ){
        اگر( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ورنہ{
          sqlite3_Vdbe_Jump_یہاں(v, addrIsNull);
        }
      }
      توڑو;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      أكد( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      أكد( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      اگر( regFree1 ) sqlite3_Vdbe_Typeof_عمود(v, r1);
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      توڑو;
    }
    حالت TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اظہاریہ_رمز_Between(pParse, p_اظہاریہ, dest, sqlite3_اظہاریہ_If_True, jumpIfNull);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_اندر: {
      صحیح destIfFalse = sqlite3VdbeMakeLabel(pParse);
      صحیح destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_اظہاریہ_رمز_اندر(pParse, p_اظہاریہ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      توڑو;
    }
#اگر_ختم
    طےشدہ: {
    default_اظہاریہ:
      اگر( اظہاریہ_Always_True(p_اظہاریہ) ){
        sqlite3VdbeGoto(v, dest);
      }ورنہ اگر( اظہاریہ_Always_False(p_اظہاریہ) ){
        /* No-op */
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      توڑو;
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
خالی sqlite3_اظہاریہ_If_False(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح dest, صحیح jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  صحیح op = 0;
  صحیح regFree1 = 0;
  صحیح regFree2 = 0;
  صحیح r1, r2;

  أكد( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  اگر( NEVER(v==0) ) واپس; /* Existence of VDBE checked by caller */
  اگر( p_اظہاریہ==0 )    واپس;
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

  انتخاب( p_اظہاریہ->op ){
    حالت TK_AND:
    حالت TK_OR: {
      اظہاریہ *pAlt = sqlite3_اظہاریہ_Simplified_And_Or(p_اظہاریہ);
      اگر( pAlt!=p_اظہاریہ ){
        sqlite3_اظہاریہ_If_False(pParse, pAlt, dest, jumpIfNull);
      }ورنہ{
        اظہاریہ *p_پہلا, *p_دوسرا;
        اگر( اظہاریہ_Eval_Rhs_پہلا(p_اظہاریہ) ){
          p_پہلا = p_اظہاریہ->p_دایاں;
          p_دوسرا = p_اظہاریہ->p_بایاں;
        }ورنہ{
          p_پہلا = p_اظہاریہ->p_بایاں;
          p_دوسرا = p_اظہاریہ->p_دایاں;
        }
        اگر( p_اظہاریہ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_False(pParse, p_پہلا, dest, jumpIfNull);
          sqlite3_اظہاریہ_If_False(pParse, p_دوسرا, dest, jumpIfNull);
        }ورنہ{
          صحیح d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_اظہاریہ_If_True(pParse, p_پہلا, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_اظہاریہ_If_False(pParse, p_دوسرا, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      توڑو;
    }
    حالت TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest, jumpIfNull);
      توڑو;
    }
    حالت TK_TRUTH: {
      صحیح isNot;   /* IS NOT TRUE or IS NOT FALSE */
      صحیح isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_اظہاریہ->op2==TK_ISNOT;
      isTrue = sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      اگر( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_اظہاریہ_If_False(pParse, p_اظہاریہ->p_بایاں, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ورنہ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_اظہاریہ_If_True(pParse, p_اظہاریہ->p_بایاں, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      توڑو;
    }
    حالت TK_IS:
    حالت TK_ISNOT:
      testcase( p_اظہاریہ->op==TK_IS );
      testcase( p_اظہاریہ->op==TK_ISNOT );
      op = (p_اظہاریہ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE:
    حالت TK_NE:
    حالت TK_EQ: {
      صحیح addrIsNull;
      اگر( sqlite3_اظہاریہ_Is_سمتیہ(p_اظہاریہ->p_بایاں) ) جاؤ default_اظہاریہ;
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = اظہاریہ_Compute_Operands(pParse, p_اظہاریہ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
        r2 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_دایاں, &regFree2);
        addrIsNull = 0;
      }
      رمز_Compare(pParse, p_اظہاریہ->p_بایاں, p_اظہاریہ->p_دایاں, op,
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
      اگر( addrIsNull ){
        اگر( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ورنہ{
          sqlite3_Vdbe_Jump_یہاں(v, addrIsNull);
        }
      }
      توڑو;
    }
    حالت TK_ISNULL:
    حالت TK_NOTNULL: {
      r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ->p_بایاں, &regFree1);
      أكد( regFree1==0 || regFree1==r1 );
      اگر( regFree1 ) sqlite3_Vdbe_Typeof_عمود(v, r1);
      sqlite3_Vdbe_جوڑو_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      توڑو;
    }
    حالت TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      اظہاریہ_رمز_Between(pParse, p_اظہاریہ, dest, sqlite3_اظہاریہ_If_False, jumpIfNull);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_اندر: {
      اگر( jumpIfNull ){
        sqlite3_اظہاریہ_رمز_اندر(pParse, p_اظہاریہ, dest, dest);
      }ورنہ{
        صحیح destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_اظہاریہ_رمز_اندر(pParse, p_اظہاریہ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      توڑو;
    }
#اگر_ختم
    طےشدہ: {
    default_اظہاریہ:
      اگر( اظہاریہ_Always_False(p_اظہاریہ) ){
        sqlite3VdbeGoto(v, dest);
      }ورنہ اگر( اظہاریہ_Always_True(p_اظہاریہ) ){
        /* no-op */
      }ورنہ{
        r1 = sqlite3_اظہاریہ_رمز_عارضی(pParse, p_اظہاریہ, &regFree1);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      توڑو;
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
خالی sqlite3_اظہاریہ_If_False_Dup(Parse *pParse, اظہاریہ *p_اظہاریہ, صحیح dest,صحیح jumpIfNull){
  sqlite3 *db = pParse->db;
  اظہاریہ *pCopy = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
  اگر( db->mallocFailed==0 ){
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
ساکن SQLITE_NOINLINE صحیح اظہاریہ_Compare_Variable(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pVar,
  ثابت اظہاریہ *p_اظہاریہ
){
  صحیح res = 2;
  صحیح iVar;
  sqlite3_قدر *pL, *pR = 0;
 
  اگر( p_اظہاریہ->op==TK_VARIABLE && pVar->i_عمود==p_اظہاریہ->i_عمود ){
    واپس 0;
  }
  اگر( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) واپس 2;
  sqlite3_قدر_From_اظہاریہ(pParse->db, p_اظہاریہ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  اگر( pR ){
    iVar = pVar->i_عمود;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_قدر(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    اگر( pL ){
      اگر( sqlite3_قدر_type(pL)==SQLITE_متن ){
        sqlite3_قدر_متن(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_قدر_Free(pR);
    sqlite3_قدر_Free(pL);
  }
  واپس res;
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
صحیح sqlite3_اظہاریہ_Compare(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pA,
  ثابت اظہاریہ *pB,
  صحیح iTab
){
  u32 combinedFlags;
  اگر( pA==0 || pB==0 ){
    واپس pB==pA ? 0 : 2;
  }
  اگر( pParse && pA->op==TK_VARIABLE ){
    واپس اظہاریہ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  اگر( combinedFlags & EP_Int_قدر ){
    اگر( (pA->flags&pB->flags&EP_Int_قدر)!=0 && pA->u.i_قدر==pB->u.i_قدر ){
      واپس 0;
    }
    واپس 2;
  }
  اگر( pA->op!=pB->op || pA->op==TK_RAISE ){
    اگر( pA->op==TK_COLLATE && sqlite3_اظہاریہ_Compare(pParse, pA->p_بایاں,pB,iTab)<2 ){
      واپس 1;
    }
    اگر( pB->op==TK_COLLATE && sqlite3_اظہاریہ_Compare(pParse, pA,pB->p_بایاں,iTab)<2 ){
      واپس 1;
    }
    اگر( pA->op==TK_AGG_عمود && pB->op==TK_عمود
     && pB->i_جدول<0 && pA->i_جدول==iTab
    ){
      /* fall through */
    }ورنہ{
      واپس 2;
    }
  }
  أكد( !اظہاریہ_Has_Property(pA, EP_Int_قدر) );
  أكد( !اظہاریہ_Has_Property(pB, EP_Int_قدر) );
  اگر( pA->u.zToken ){
    اگر( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      اگر( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) واپس 2;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      أكد( pA->op==pB->op );
      اگر( اظہاریہ_Has_Property(pA,EP_WinFunc)!=اظہاریہ_Has_Property(pB,EP_WinFunc) ){
        واپس 2;
      }
      اگر( اظہاریہ_Has_Property(pA,EP_WinFunc) ){
        اگر( sqlite3_کھڑکی_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          واپس 2;
        }
      }
#اگر_ختم
    }ورنہ اگر( pA->op==TK_NULL ){
      واپس 0;
    }ورنہ اگر( pA->op==TK_COLLATE ){
      اگر( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) واپس 2;
    }ورنہ
    اگر( pB->u.zToken!=0
     && pA->op!=TK_عمود
     && pA->op!=TK_AGG_عمود
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      واپس 2;
    }
  }
  اگر( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) واپس 2;
  اگر( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    اگر( combinedFlags & EP_xIsSelect ) واپس 2;
    اگر( (combinedFlags & EP_Fixed_ستون)==0
     && sqlite3_اظہاریہ_Compare(pParse, pA->p_بایاں, pB->p_بایاں, iTab) ) واپس 2;
    اگر( sqlite3_اظہاریہ_Compare(pParse, pA->p_دایاں, pB->p_دایاں, iTab) ) واپس 2;
    اگر( sqlite3_اظہاریہ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) واپس 2;
    اگر( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      اگر( pA->i_عمود!=pB->i_عمود ) واپس 2;
      اگر( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) واپس 2;
      اگر( pA->op!=TK_اندر && pA->i_جدول!=pB->i_جدول && pA->i_جدول!=iTab ){
        واپس 2;
      }
    }
  }
  واپس 0;
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
صحیح sqlite3_اظہاریہ_List_Compare(ثابت اظہاریہ_List *pA, ثابت اظہاریہ_List *pB, صحیح iTab){
  صحیح i;
  اگر( pA==0 && pB==0 ) واپس 0;
  اگر( pA==0 || pB==0 ) واپس 1;
  اگر( pA->n_اظہاریہ!=pB->n_اظہاریہ ) واپس 1;
  برائے(i=0; i<pA->n_اظہاریہ; i++){
    صحیح res;
    اظہاریہ *p_اظہاریہ_A = pA->a[i].p_اظہاریہ;
    اظہاریہ *p_اظہاریہ_B = pB->a[i].p_اظہاریہ;
    اگر( pA->a[i].fg.ترتیب_دو_Flags!=pB->a[i].fg.ترتیب_دو_Flags ) واپس 1;
    اگر( (res = sqlite3_اظہاریہ_Compare(0, p_اظہاریہ_A, p_اظہاریہ_B, iTab)) ) واپس res;
  }
  واپس 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
صحیح sqlite3_اظہاریہ_Compare_Skip(اظہاریہ *pA,اظہاریہ *pB, صحیح iTab){
  واپس sqlite3_اظہاریہ_Compare(0,
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
ساکن صحیح اظہاریہ_مستلزم_Not_Null(
  ثابت Parse *pParse,/* Parsing context */
  ثابت اظہاریہ *p,      /* The expression to be checked */
  ثابت اظہاریہ *pNN,    /* The expression that is NOT NULL */
  صحیح iTab,           /* Table being evaluated */
  صحیح نظر_آیا_Not         /* Return true only if p can be any non-NULL value */
){
  أكد( p );
  أكد( pNN );
  اگر( sqlite3_اظہاریہ_Compare(pParse, p, pNN, iTab)==0 ){
    واپس pNN->op!=TK_NULL;
  }
  انتخاب( p->op ){
    حالت TK_اندر: {
      اگر( نظر_آیا_Not && اظہاریہ_Has_Property(p, EP_xIsSelect) ) واپس 0;
      أكد( اظہاریہ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_اظہاریہ>0) );
      واپس اظہاریہ_مستلزم_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
    حالت TK_BETWEEN: {
      اظہاریہ_List *pList;
      أكد( اظہاریہ_Use_XList(p) );
      pList = p->x.pList;
      أكد( pList!=0 );
      أكد( pList->n_اظہاریہ==2 );
      اگر( نظر_آیا_Not ) واپس 0;
      اگر( اظہاریہ_مستلزم_Not_Null(pParse, pList->a[0].p_اظہاریہ, pNN, iTab, 1)
       || اظہاریہ_مستلزم_Not_Null(pParse, pList->a[1].p_اظہاریہ, pNN, iTab, 1)
      ){
        واپس 1;
      }
      واپس اظہاریہ_مستلزم_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
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
      نظر_آیا_Not = 1;
      /* no break */ deliberate_fall_through
    حالت TK_ستارہ:
    حالت TK_REM:
    حالت TK_BITAND:
    حالت TK_SLASH: {
      اگر( اظہاریہ_مستلزم_Not_Null(pParse, p->p_دایاں, pNN, iTab, نظر_آیا_Not) ) واپس 1;
      /* no break */ deliberate_fall_through
    }
    حالت TK_وسعت:
    حالت TK_COLLATE:
    حالت TK_UPLUS:
    حالت TK_UMINUS: {
      واپس اظہاریہ_مستلزم_Not_Null(pParse, p->p_بایاں, pNN, iTab, نظر_آیا_Not);
    }
    حالت TK_TRUTH: {
      اگر( نظر_آیا_Not ) واپس 0;
      اگر( p->op2!=TK_IS ) واپس 0;
      واپس اظہاریہ_مستلزم_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
    حالت TK_BITNOT:
    حالت TK_NOT: {
      واپس اظہاریہ_مستلزم_Not_Null(pParse, p->p_بایاں, pNN, iTab, 1);
    }
  }
  واپس 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ساکن صحیح sqlite3_اظہاریہ_Is_Not_True(اظہاریہ *p_اظہاریہ){
  صحیح v;
  اگر( p_اظہاریہ->op==TK_NULL ) واپس 1;
  اگر( p_اظہاریہ->op==TK_TRUEFALSE && sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ)==0 ) واپس 1;
  v = 1;
  اگر( sqlite3_اظہاریہ_Is_Integer(p_اظہاریہ, &v, 0, 0) && v==0 ) واپس 1;
  واپس 0;
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
ساکن صحیح sqlite3_اظہاریہ_Is_IIF(sqlite3 *db, ثابت اظہاریہ *p_اظہاریہ){
  اظہاریہ_List *pList;
  اگر( p_اظہاریہ->op==TK_FUNCTION ){
    ثابت حرف *z = p_اظہاریہ->u.zToken;
    FuncDef *pDef;
    اگر( (z[0]!='i' && z[0]!='I') ) واپس 0;
    اگر( p_اظہاریہ->x.pList==0 ) واپس 0;
    pDef = sqlite3_ڈھونڈو_Function(db, z, p_اظہاریہ->x.pList->n_اظہاریہ, ENC(db), 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    اگر( pDef==0 ) واپس 0;
#ورنہ
    اگر( NEVER(pDef==0) ) واپس 0;
#اگر_ختم
    اگر( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) واپس 0;
    اگر( SQLITE_PTR_مقصد_INT(pDef->p_User_ڈیٹا)!=INLINEFUNC_iif ) واپس 0;
  }ورنہ اگر( p_اظہاریہ->op==TK_CASE ){
    اگر( p_اظہاریہ->p_بایاں!=0 ) واپس 0;
  }ورنہ{
    واپس 0;
  }
  pList = p_اظہاریہ->x.pList;
  أكد( pList!=0 );
  اگر( pList->n_اظہاریہ==2 ) واپس 1;
  اگر( pList->n_اظہاریہ==3 && sqlite3_اظہاریہ_Is_Not_True(pList->a[2].p_اظہاریہ) ) واپس 1;
  واپس 0;
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
صحیح sqlite3_اظہاریہ_مستلزم_اظہاریہ(
  ثابت Parse *pParse,
  ثابت اظہاریہ *pE1,
  ثابت اظہاریہ *pE2,
  صحیح iTab
){
  اگر( sqlite3_اظہاریہ_Compare(pParse, pE1, pE2, iTab)==0 ){
    واپس 1;
  }
  اگر( pE2->op==TK_OR
   && (sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse, pE1, pE2->p_بایاں, iTab)
             || sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse, pE1, pE2->p_دایاں, iTab) )
  ){
    واپس 1;
  }
  اگر( pE2->op==TK_NOTNULL
   && اظہاریہ_مستلزم_Not_Null(pParse, pE1, pE2->p_بایاں, iTab, 0)
  ){
    واپس 1;
  }
  اگر( sqlite3_اظہاریہ_Is_IIF(pParse->db, pE1) ){
    واپس sqlite3_اظہاریہ_مستلزم_اظہاریہ(pParse,pE1->x.pList->a[0].p_اظہاریہ,pE2,iTab);
  }
  واپس 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ساکن خالی both_Imply_Not_Null_صف(Walker *pWalker, اظہاریہ *pE1, اظہاریہ *pE2){
  اگر( pWalker->e_رمز==0 ){
    sqlite3_Walk_اظہاریہ(pWalker, pE1);
    اگر( pWalker->e_رمز ){
      pWalker->e_رمز = 0;
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
ساکن صحیح مستلزم_Not_Null_صف(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  testcase( p_اظہاریہ->op==TK_AGG_عمود );
  testcase( p_اظہاریہ->op==TK_AGG_FUNCTION );
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ) واپس WRC_Prune;
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    واپس WRC_Prune;
  }
  انتخاب( p_اظہاریہ->op ){
    حالت TK_ISNOT:
    حالت TK_ISNULL:
    حالت TK_NOTNULL:
    حالت TK_IS:
    حالت TK_سمتیہ:
    حالت TK_FUNCTION:
    حالت TK_TRUTH:
    حالت TK_CASE:
      testcase( p_اظہاریہ->op==TK_ISNOT );
      testcase( p_اظہاریہ->op==TK_ISNULL );
      testcase( p_اظہاریہ->op==TK_NOTNULL );
      testcase( p_اظہاریہ->op==TK_IS );
      testcase( p_اظہاریہ->op==TK_سمتیہ );
      testcase( p_اظہاریہ->op==TK_FUNCTION );
      testcase( p_اظہاریہ->op==TK_TRUTH );
      testcase( p_اظہاریہ->op==TK_CASE );
      واپس WRC_Prune;

    حالت TK_عمود:
      اگر( pWalker->u.iCur==p_اظہاریہ->i_جدول ){
        pWalker->e_رمز = 1;
        واپس WRC_Abort;
      }
      واپس WRC_Prune;

    حالت TK_OR:
    حالت TK_AND:
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
      واپس WRC_Prune;
       
    حالت TK_اندر:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      اگر( اظہاریہ_Use_XList(p_اظہاریہ) && ALWAYS(p_اظہاریہ->x.pList->n_اظہاریہ>0) ){
        sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->p_بایاں);
      }
      واپس WRC_Prune;

    حالت TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pList->n_اظہاریہ==2 );
      sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->p_بایاں);
      both_Imply_Not_Null_صف(pWalker, p_اظہاریہ->x.pList->a[0].p_اظہاریہ,
                                   p_اظہاریہ->x.pList->a[1].p_اظہاریہ);
      واپس WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    حالت TK_EQ:
    حالت TK_NE:
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE: {
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
      أكد( p_بایاں->op!=TK_عمود || اظہاریہ_Use_YTab(p_بایاں) );
      أكد( p_دایاں->op!=TK_عمود || اظہاریہ_Use_YTab(p_دایاں) );
      اگر( (p_بایاں->op==TK_عمود
           && ALWAYS(p_بایاں->y.pTab!=0)
           && IsVirtual(p_بایاں->y.pTab))
       || (p_دایاں->op==TK_عمود
           && ALWAYS(p_دایاں->y.pTab!=0)
           && IsVirtual(p_دایاں->y.pTab))
      ){
        واپس WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    طےشدہ:
      واپس WRC_Continue;
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
صحیح sqlite3_اظہاریہ_مستلزم_Non_Null_صف(اظہاریہ *p, صحیح iTab, صحیح isRJ){
  Walker w;
  p = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p);
  اگر( p==0 ) واپس 0;
  اگر( p->op==TK_NOTNULL ){
    p = p->p_بایاں;
  }ورنہ{
    جب( p->op==TK_AND ){
      اگر( sqlite3_اظہاریہ_مستلزم_Non_Null_صف(p->p_بایاں, iTab, isRJ) ) واپس 1;
      p = p->p_دایاں;
    }
  }
  w.x_اظہاریہ_Callback = مستلزم_Not_Null_صف;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_رمز = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_اظہاریہ(&w, p);
  واپس w.e_رمز;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ساخت IdxCover {
  اشاریہ *pIdx;     /* The index to be tested for coverage */
  صحیح iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ساکن صحیح اظہاریہ_Idx_Cover(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_عمود
   && p_اظہاریہ->i_جدول==pWalker->u.pIdxCover->iCur
   && sqlite3_جدول_عمود_مقصد_اشاریہ(pWalker->u.pIdxCover->pIdx, p_اظہاریہ->i_عمود)<0
  ){
    pWalker->e_رمز = 1;
    واپس WRC_Abort;
  }
  واپس WRC_Continue;
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
صحیح sqlite3_اظہاریہ_Covered_By_اشاریہ(
  اظہاریہ *p_اظہاریہ,        /* The index to be tested */
  صحیح iCur,           /* The cursor number for the corresponding table */
  اشاریہ *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ساخت IdxCover xcov;
  memset(&w, 0, حجم(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_اظہاریہ_Callback = اظہاریہ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  واپس !w.e_رمز;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ساخت RefSrcList {
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
ساکن صحیح selectRefEnter(Walker *pWalker, Select *pSelect){
  ساخت RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  صحیح *piNew;
  اگر( pSrc->nSrc==0 ) واپس WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*حجم(صحیح));
  اگر( piNew==0 ){
    p->nExclude = 0;
    واپس WRC_Abort;
  }ورنہ{
    p->aiExclude = piNew;
  }
  برائے(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  واپس WRC_Continue;
}
ساکن خالی selectRefLeave(Walker *pWalker, Select *pSelect){
  ساخت RefSrcList *p = pWalker->u.pRefSrcList;
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
ساکن صحیح اظہاریہ_Ref_مقصد_Src_List(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_عمود
   || p_اظہاریہ->op==TK_AGG_عمود
  ){
    صحیح i;
    ساخت RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    صحیح nSrc = pSrc ? pSrc->nSrc : 0;
    برائے(i=0; i<nSrc; i++){
      اگر( p_اظہاریہ->i_جدول==pSrc->a[i].iCursor ){
        pWalker->e_رمز |= 1;
        واپس WRC_Continue;
      }
    }
    برائے(i=0; i<p->nExclude && p->aiExclude[i]!=p_اظہاریہ->i_جدول; i++){}
    اگر( i>=p->nExclude ){
      pWalker->e_رمز |= 2;
    }
  }
  واپس WRC_Continue;
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
صحیح sqlite3ReferencesSrcList(Parse *pParse, اظہاریہ *p_اظہاریہ, SrcList *pSrcList){
  Walker w;
  ساخت RefSrcList x;
  أكد( pParse->db!=0 );
  memset(&w, 0, حجم(w));
  memset(&x, 0, حجم(x));
  w.x_اظہاریہ_Callback = اظہاریہ_Ref_مقصد_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  أكد( p_اظہاریہ->op==TK_AGG_FUNCTION );
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  sqlite3_Walk_اظہاریہ_List(&w, p_اظہاریہ->x.pList);
  اگر( p_اظہاریہ->p_بایاں ){
    أكد( p_اظہاریہ->p_بایاں->op==TK_ترتیب );
    أكد( اظہاریہ_Use_XList(p_اظہاریہ->p_بایاں) );
    أكد( p_اظہاریہ->p_بایاں->x.pList!=0 );
    sqlite3_Walk_اظہاریہ_List(&w, p_اظہاریہ->p_بایاں->x.pList);
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
    sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ->y.pWin->pFilter);
  }
#اگر_ختم
  اگر( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  اگر( w.e_رمز & 0x01 ){
    واپس 1;
  }ورنہ اگر( w.e_رمز ){
    واپس 0;
  }ورنہ{
    واپس -1;
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
ساکن صحیح agginfo_Persist_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( ALWAYS(!اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced))
   && p_اظہاریہ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_اظہاریہ->pAggInfo;
    صحیح iAgg = p_اظہاریہ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    أكد( iAgg>=0 );
    اگر( p_اظہاریہ->op!=TK_AGG_FUNCTION ){
      اگر( iAgg<pAggInfo->n_عمود
       && pAggInfo->a_ستون[iAgg].pCExpr==p_اظہاریہ
      ){
        p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        اگر( p_اظہاریہ && !sqlite3_اظہاریہ_Deferred_Delete(pParse, p_اظہاریہ) ){
          pAggInfo->a_ستون[iAgg].pCExpr = p_اظہاریہ;
        }
      }
    }ورنہ{
      أكد( p_اظہاریہ->op==TK_AGG_FUNCTION );
      اگر( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_اظہاریہ
      ){
        p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        اگر( p_اظہاریہ && !sqlite3_اظہاریہ_Deferred_Delete(pParse, p_اظہاریہ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_اظہاریہ;
        }
      }
    }
  }
  واپس WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
خالی sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, حجم(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_اظہاریہ_Callback = agginfo_Persist_اظہاریہ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساکن صحیح جوڑو_Agg_Info_عمود(sqlite3 *db, AggInfo *pInfo){
  صحیح i;
  pInfo->a_ستون = sqlite3ArrayAllocate(
       db,
       pInfo->a_ستون,
       حجم(pInfo->a_ستون[0]),
       &pInfo->n_عمود,
       &i
  );
  واپس i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ساکن صحیح جوڑو_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
  صحیح i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       حجم(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  واپس i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ساکن خالی ڈھونڈو_Or_Create_Agg_Info_عمود(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  اظہاریہ *p_اظہاریہ          /* Expr describing the column to find or insert */
){
  ساخت Agg_Info_ستون *p_ستون;
  صحیح k;
  صحیح mx_رکن = pParse->db->a_حد_بندی[SQLITE_حد_بندی_عمود];

  أكد( mx_رکن <= SMXV(i16) );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  p_ستون = pAggInfo->a_ستون;
  برائے(k=0; k<pAggInfo->n_عمود; k++, p_ستون++){
    اگر( p_ستون->pCExpr==p_اظہاریہ ) واپس;
    اگر( p_ستون->i_جدول==p_اظہاریہ->i_جدول
     && p_ستون->i_عمود==p_اظہاریہ->i_عمود
     && p_اظہاریہ->op!=TK_IF_NULL_صف
    ){
      جاؤ fix_اوپر_اظہاریہ;
    }
  }
  k = جوڑو_Agg_Info_عمود(pParse->db, pAggInfo);
  اگر( k<0 ){
    /* OOM on resize */
    أكد( pParse->db->mallocFailed );
    واپس;
  }
  اگر( k>mx_رکن ){
    sqlite3_Error_پیغام(pParse, "more than %d aggregate terms", mx_رکن);
    k = mx_رکن;
  }
  p_ستون = &pAggInfo->a_ستون[k];
  أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
  p_ستون->pTab = p_اظہاریہ->y.pTab;
  p_ستون->i_جدول = p_اظہاریہ->i_جدول;
  p_ستون->i_عمود = p_اظہاریہ->i_عمود;
  p_ستون->i_Sorter_عمود = -1;
  p_ستون->pCExpr = p_اظہاریہ;
  اگر( pAggInfo->p_گروہ_By && p_اظہاریہ->op!=TK_IF_NULL_صف ){
    صحیح j, n;
    اظہاریہ_List *pGB = pAggInfo->p_گروہ_By;
    ساخت اظہاریہ_List_شے *p_رکن = pGB->a;
    n = pGB->n_اظہاریہ;
    برائے(j=0; j<n; j++, p_رکن++){
      اظہاریہ *pE = p_رکن->p_اظہاریہ;
      اگر( pE->op==TK_عمود
       && pE->i_جدول==p_اظہاریہ->i_جدول
       && pE->i_عمود==p_اظہاریہ->i_عمود
      ){
        p_ستون->i_Sorter_عمود = j;
        توڑو;
      }
    }
  }
  اگر( p_ستون->i_Sorter_عمود<0 ){
    p_ستون->i_Sorter_عمود = pAggInfo->n_ترتیب_کاری_عمود++;
  }
fix_اوپر_اظہاریہ:
  اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);
  أكد( p_اظہاریہ->pAggInfo==0 || p_اظہاریہ->pAggInfo==pAggInfo );
  p_اظہاریہ->pAggInfo = pAggInfo;
  اگر( p_اظہاریہ->op==TK_عمود ){
    p_اظہاریہ->op = TK_AGG_عمود;
  }
  أكد( k <= SMXV(p_اظہاریہ->iAgg) );
  p_اظہاریہ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ساکن صحیح analyzeAggregate(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحیح i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  أكد( pNC->ncFlags & NC_UAggInfo );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  انتخاب( p_اظہاریہ->op ){
    طےشدہ: {
      اشاریہ_دار_اظہاریہ *pIEpr;
      اظہاریہ tmp;
      أكد( pParse->iSelfTab==0 );
      اگر( (pNC->ncFlags & NC_اندر_Agg_Func)==0 ) توڑو;
      اگر( pParse->pIdxEpr==0 ) توڑو;
      برائے(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        صحیح i_ڈیٹا_Cur = pIEpr->i_ڈیٹا_Cur;
        اگر( i_ڈیٹا_Cur<0 ) جاری;
        اگر( sqlite3_اظہاریہ_Compare(0, p_اظہاریہ, pIEpr->p_اظہاریہ, i_ڈیٹا_Cur)==0 ) توڑو;
      }
      اگر( pIEpr==0 ) توڑو;
      اگر( NEVER(!اظہاریہ_Use_YTab(p_اظہاریہ)) ) توڑو;
      برائے(i=0; i<pSrcList->nSrc; i++){
         اگر( pSrcList->a[i].iCursor==pIEpr->i_ڈیٹا_Cur ){
           testcase( i>0 );
           توڑو;
         }
      }
      اگر( i>=pSrcList->nSrc ) توڑو;
      اگر( NEVER(p_اظہاریہ->pAggInfo!=0) ) توڑو; /* Resolved by outer context */
      اگر( pParse->nErr ){ واپس WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, حجم(tmp));
      tmp.op = TK_AGG_عمود;
      tmp.i_جدول = pIEpr->iIdxCur;
      tmp.i_عمود = pIEpr->i_Idx_ستون;
      ڈھونڈو_Or_Create_Agg_Info_عمود(pParse, pAggInfo, &tmp);
      اگر( pParse->nErr ){ واپس WRC_Abort; }
      أكد( pAggInfo->a_ستون!=0 );
      أكد( tmp.iAgg<pAggInfo->n_عمود );
      pAggInfo->a_ستون[tmp.iAgg].pCExpr = p_اظہاریہ;
      p_اظہاریہ->pAggInfo = pAggInfo;
      p_اظہاریہ->iAgg = tmp.iAgg;
      واپس WRC_Prune;
    }
    حالت TK_IF_NULL_صف:
    حالت TK_AGG_عمود:
    حالت TK_عمود: {
      testcase( p_اظہاریہ->op==TK_AGG_عمود );
      testcase( p_اظہاریہ->op==TK_عمود );
      testcase( p_اظہاریہ->op==TK_IF_NULL_صف );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      اگر( ALWAYS(pSrcList!=0) ){
        Src_شے *p_شے = pSrcList->a;
        برائے(i=0; i<pSrcList->nSrc; i++, p_شے++){
          أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );
          اگر( p_اظہاریہ->i_جدول==p_شے->iCursor ){
            ڈھونڈو_Or_Create_Agg_Info_عمود(pParse, pAggInfo, p_اظہاریہ);
            توڑو;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      واپس WRC_Continue;
    }
    حالت TK_AGG_FUNCTION: {
      اگر( (pNC->ncFlags & NC_اندر_Agg_Func)==0
       && pWalker->walker_گہرائی==p_اظہاریہ->op2
       && p_اظہاریہ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ساخت AggInfo_func *p_شے = pAggInfo->aFunc;
        صحیح mx_رکن = pParse->db->a_حد_بندی[SQLITE_حد_بندی_عمود];
        أكد( mx_رکن <= SMXV(i16) );
        برائے(i=0; i<pAggInfo->nFunc; i++, p_شے++){
          اگر( NEVER(p_شے->pFExpr==p_اظہاریہ) ) توڑو;
          اگر( sqlite3_اظہاریہ_Compare(0, p_شے->pFExpr, p_اظہاریہ, -1)==0 ){
            توڑو;
          }
        }
        اگر( i>mx_رکن ){
          sqlite3_Error_پیغام(pParse, "more than %d aggregate terms", mx_رکن);
          i = mx_رکن;
          أكد( i<pAggInfo->nFunc );
        }ورنہ اگر( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = جوڑو_Agg_Info_Func(pParse->db, pAggInfo);
          اگر( i>=0 ){
            صحیح nArg;
            أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect) );
            p_شے = &pAggInfo->aFunc[i];
            p_شے->pFExpr = p_اظہاریہ;
            أكد( اظہاریہ_Use_UToken(p_اظہاریہ) );
            nArg = p_اظہاریہ->x.pList ? p_اظہاریہ->x.pList->n_اظہاریہ : 0;
            p_شے->pFunc = sqlite3_ڈھونڈو_Function(pParse->db,
                                         p_اظہاریہ->u.zToken, nArg, enc, 0);
            أكد( p_شے->bOBUnique==0 );
            اگر( p_اظہاریہ->p_بایاں
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
              اگر( pOBList->n_اظہاریہ==1
               && nArg==1
               && sqlite3_اظہاریہ_Compare(0,pOBList->a[0].p_اظہاریہ,
                               p_اظہاریہ->x.pList->a[0].p_اظہاریہ,-1)==0
              ){
                p_شے->bOBPayload = 0;
                p_شے->bOBUnique = اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct);
              }ورنہ{
                p_شے->bOBPayload = 1;
              }
              p_شے->bUseSubtype =
                    (p_شے->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ورنہ{
              p_شے->iOBTab = -1;
            }
            اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct) && !p_شے->bOBUnique ){
              p_شے->iDistinct = pParse->nTab++;
            }ورنہ{
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
        واپس WRC_Prune;
      }ورنہ{
        واپس WRC_Continue;
      }
    }
  }
  واپس WRC_Continue;
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
خالی sqlite3_اظہاریہ_Analyze_Aggregates(NameContext *pNC, اظہاریہ *p_اظہاریہ){
  Walker w;
  w.x_اظہاریہ_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_گہرائی_Increase;
  w.xSelectCallback2 = sqlite3_Walker_گہرائی_Decrease;
  w.walker_گہرائی = 0;
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
خالی sqlite3_اظہاریہ_Analyze_Agg_List(NameContext *pNC, اظہاریہ_List *pList){
  ساخت اظہاریہ_List_شے *p_شے;
  صحیح i;
  اگر( pList ){
    برائے(p_شے=pList->a, i=0; i<pList->n_اظہاریہ; i++, p_شے++){
      sqlite3_اظہاریہ_Analyze_Aggregates(pNC, p_شے->p_اظہاریہ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
صحیح sqlite3_Get_عارضی_Reg(Parse *pParse){
  اگر( pParse->n_عارضی_Reg==0 ){
    واپس ++pParse->nMem;
  }
  واپس pParse->a_عارضی_Reg[--pParse->n_عارضی_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
خالی sqlite3_Release_عارضی_Reg(Parse *pParse, صحیح iReg){
  اگر( iReg ){
#اگر_تعریف_شدہ SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    صحیح kk;
    برائے(kk=0; kk<pParse->n_عارضی_Reg; kk++){
      أكد( pParse->a_عارضی_Reg[kk]!=iReg );
    }
#اگر_ختم
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    اگر( pParse->n_عارضی_Reg<Array_سائز(pParse->a_عارضی_Reg) ){
      pParse->a_عارضی_Reg[pParse->n_عارضی_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
صحیح sqlite3_Get_عارضی_Range(Parse *pParse, صحیح nReg){
  صحیح i, n;
  اگر( nReg==1 ) واپس sqlite3_Get_عارضی_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  اگر( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ورنہ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  واپس i;
}
خالی sqlite3_Release_عارضی_Range(Parse *pParse, صحیح iReg, صحیح nReg){
  اگر( nReg==1 ){
    sqlite3_Release_عارضی_Reg(pParse, iReg);
    واپس;
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
خالی sqlite3_Clear_عارضی_Reg_Cache(Parse *pParse){
  pParse->n_عارضی_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
خالی sqlite3_Touch_رجسٹر(Parse *pParse, صحیح iReg){
  اگر( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#اگر defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
صحیح sqlite3_پہلا_دستیاب_رجسٹر(Parse *pParse, صحیح iMin){
  ثابت اظہاریہ_List *pList = pParse->p_Const_اظہاریہ;
  اگر( pList ){
    صحیح i;
    برائے(i=0; i<pList->n_اظہاریہ; i++){
      اگر( pList->a[i].u.i_Const_اظہاریہ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_اظہاریہ_Reg + 1;
      }
    }
  }
  pParse->n_عارضی_Reg = 0;
  pParse->nRangeReg = 0;
  واپس iMin;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح sqlite3_No_Temps_اندر_Range(Parse *pParse, صحیح i_پہلا, صحیح i_آخری){
  صحیح i;
  اگر( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_پہلا
   && pParse->iRangeReg <= i_آخری
  ){
     واپس 0;
  }
  برائے(i=0; i<pParse->n_عارضی_Reg; i++){
    اگر( pParse->a_عارضی_Reg[i]>=i_پہلا && pParse->a_عارضی_Reg[i]<=i_آخری ){
      واپس 0;
    }
  }
  اگر( pParse->p_Const_اظہاریہ ){
    اظہاریہ_List *pList = pParse->p_Const_اظہاریہ;
    برائے(i=0; i<pList->n_اظہاریہ; i++){
      صحیح iReg = pList->a[i].u.i_Const_اظہاریہ_Reg;
      اگر( iReg==0 ) جاری;
      اگر( iReg>=i_پہلا && iReg<=i_آخری ) واپس 0;
    }
  }
  واپس 1;
}
#اگر_ختم /* SQLITE_DEBUG */
