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
#అన్తర్భావ "sqliteInt.h"

/* Forward declarations */
స్థిర శూన్య సమాసం_సంకేతం_Between(Parse*,సమాసం*,పూర్ణాంకం,శూన్య(*)(Parse*,సమాసం*,పూర్ణాంకం,పూర్ణాంకం),పూర్ణాంకం);
స్థిర పూర్ణాంకం సమాసం_సంకేతం_సదిశ(Parse *pParse, సమాసం *p, పూర్ణాంకం *pi_గమ్యం_Free);

/*
** Return the affinity character for a single column of a table.
*/
అక్షరం sqlite3_పట్టిక_స్తంభం_Affinity(స్థిరరాశి పట్టిక *pTab, పూర్ణాంకం i_నిలువు){
  ఒకవేళ( i_నిలువు<0 || NEVER(i_నిలువు>=pTab->n_నిలువు) ) తిరిగి SQLITE_AFF_INTEGER;
  తిరిగి pTab->a_నిలువు[i_నిలువు].affinity;
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
అక్షరం sqlite3_సమాసం_Affinity(స్థిరరాశి సమాసం *p_సమాసం){
  పూర్ణాంకం op;
  op = p_సమాసం->op;
  అయితే( 1 /* exit-by-break */ ){
    ఒకవేళ( op==TK_స్తంభం || (op==TK_AGG_స్తంభం && p_సమాసం->y.pTab!=0) ){
      ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->y.pTab!=0 );
      తిరిగి sqlite3_పట్టిక_స్తంభం_Affinity(p_సమాసం->y.pTab, p_సమాసం->i_స్తంభం);
    }
    ఒకవేళ( op==TK_SELECT ){
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->x.pSelect!=0 );
      ప్రతిజ్ఞా( p_సమాసం->x.pSelect->pEList!=0 );
      ప్రతిజ్ఞా( p_సమాసం->x.pSelect->pEList->a[0].p_సమాసం!=0 );
      తిరిగి sqlite3_సమాసం_Affinity(p_సమాసం->x.pSelect->pEList->a[0].p_సమాసం);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
    ఒకవేళ( op==TK_CAST ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      తిరిగి sqlite3AffinityType(p_సమాసం->u.zToken, 0);
    }
#అయితే_ముగింపు
    ఒకవేళ( op==TK_SELECT_స్తంభం ){
      ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ!=0 && సమాసం_Use_XSelect(p_సమాసం->p_ఎడమ) );
      ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం < p_సమాసం->i_పట్టిక );
      ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం >= 0 );
      ప్రతిజ్ఞా( p_సమాసం->i_పట్టిక==p_సమాసం->p_ఎడమ->x.pSelect->pEList->n_సమాసం );
      తిరిగి sqlite3_సమాసం_Affinity(
          p_సమాసం->p_ఎడమ->x.pSelect->pEList->a[p_సమాసం->i_స్తంభం].p_సమాసం
      );
    }
    ఒకవేళ( op==TK_సదిశ
     || (op==TK_FUNCTION && p_సమాసం->aff_సమాసం==SQLITE_AFF_DEFER)
    ){
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      తిరిగి sqlite3_సమాసం_Affinity(p_సమాసం->x.pList->a[0].p_సమాసం);
    }
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Skip|EP_If_Null_అడ్డువరుస) ){
      ప్రతిజ్ఞా( p_సమాసం->op==TK_COLLATE
           || p_సమాసం->op==TK_IF_NULL_అడ్డువరుస
           || (p_సమాసం->op==TK_రిజిస్టరు && p_సమాసం->op2==TK_IF_NULL_అడ్డువరుస) );
      p_సమాసం = p_సమాసం->p_ఎడమ;
      op = p_సమాసం->op;
      కొనసాగు;
    }
    ఒకవేళ( op!=TK_రిజిస్టరు ) ఆపు;
    op = p_సమాసం->op2;
    ఒకవేళ( NEVER( op==TK_రిజిస్టరు ) ) ఆపు;
  }
  తిరిగి p_సమాసం->aff_సమాసం;
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
పూర్ణాంకం sqlite3_సమాసం_దత్తాంశం_Type(స్థిరరాశి సమాసం *p_సమాసం){
  అయితే( p_సమాసం ){
    ఎంపిక( p_సమాసం->op ){
      స్థితి TK_COLLATE:
      స్థితి TK_IF_NULL_అడ్డువరుస:
      స్థితి TK_UPLUS:  {
        p_సమాసం = p_సమాసం->p_ఎడమ;
        ఆపు;
      }
      స్థితి TK_NULL: {
        p_సమాసం = 0;
        ఆపు;
      }
      స్థితి TK_STRING: {
        తిరిగి 0x02;
      }
      స్థితి TK_BLOB: {
        తిరిగి 0x04;
      }
      స్థితి TK_CONCAT: {
        తిరిగి 0x06;
      }
      స్థితి TK_VARIABLE:
      స్థితి TK_AGG_FUNCTION:
      స్థితి TK_FUNCTION: {
        తిరిగి 0x07;
      }
      స్థితి TK_స్తంభం:
      స్థితి TK_AGG_స్తంభం:
      స్థితి TK_SELECT:
      స్థితి TK_CAST:
      స్థితి TK_SELECT_స్తంభం:
      స్థితి TK_సదిశ:  {
        పూర్ణాంకం aff = sqlite3_సమాసం_Affinity(p_సమాసం);
        ఒకవేళ( aff>=SQLITE_AFF_NUMERIC ) తిరిగి 0x05;
        ఒకవేళ( aff==SQLITE_AFF_పాఠ్యం )    తిరిగి 0x06;
        తిరిగి 0x07;
      }
      స్థితి TK_CASE: {
        పూర్ణాంకం res = 0;
        పూర్ణాంకం ii;
        సమాసం_List *pList = p_సమాసం->x.pList;
        ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) && pList!=0 );
        ప్రతిజ్ఞా( pList->n_సమాసం > 0);
        కొరకు(ii=1; ii<pList->n_సమాసం; ii+=2){
          res |= sqlite3_సమాసం_దత్తాంశం_Type(pList->a[ii].p_సమాసం);
        }
        ఒకవేళ( pList->n_సమాసం % 2 ){
          res |= sqlite3_సమాసం_దత్తాంశం_Type(pList->a[pList->n_సమాసం-1].p_సమాసం);
        }
        తిరిగి res;
      }
      అప్రమేయ: {
        తిరిగి 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  తిరిగి 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
సమాసం *sqlite3_సమాసం_కూడు_Collate_Token(
  Parse *pParse,           /* Parsing context */
  సమాసం *p_సమాసం,             /* Add the "COLLATE" clause to this expression */
  స్థిరరాశి Token *pCollName,  /* Name of collating sequence */
  పూర్ణాంకం dequote              /* True to dequote pCollName */
){
  ఒకవేళ( pCollName->n>0 && pParse->nErr==0 ){
    సమాసం *pNew = sqlite3_సమాసం_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ప్రతిజ్ఞా( p_సమాసం!=0 );
    ఒకవేళ( pNew ){
      pNew->p_ఎడమ = p_సమాసం;
      pNew->flags |= EP_Collate|EP_Skip;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
      pNew->n_ఎత్తు = p_సమాసం->n_ఎత్తు+1;
      sqlite3_సమాసం_Check_ఎత్తు(pParse, pNew->n_ఎత్తు);
      testcase( pParse->nErr>0 );
#అయితే_ముగింపు
      p_సమాసం = pNew;
    }
  }
  తిరిగి p_సమాసం;
}
సమాసం *sqlite3_సమాసం_కూడు_Collate_String(
  Parse *pParse,        /* Parsing context */
  సమాసం *p_సమాసం,          /* Add the "COLLATE" clause to this expression */
  స్థిరరాశి అక్షరం *zC        /* The collating sequence name */
){
  Token s;
  ప్రతిజ్ఞా( zC!=0 );
  sqlite3TokenInit(&s, (అక్షరం*)zC);
  తిరిగి sqlite3_సమాసం_కూడు_Collate_Token(pParse, p_సమాసం, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
సమాసం *sqlite3_సమాసం_Skip_Collate(సమాసం *p_సమాసం){
  అయితే( p_సమాసం && సమాసం_Has_Property(p_సమాసం, EP_Skip) ){
    ప్రతిజ్ఞా( p_సమాసం->op==TK_COLLATE );
    p_సమాసం = p_సమాసం->p_ఎడమ;
  }  
  తిరిగి p_సమాసం;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
సమాసం *sqlite3_సమాసం_Skip_Collate_And_Likely(సమాసం *p_సమాసం){
  అయితే( p_సమాసం && సమాసం_Has_Property(p_సమాసం, EP_Skip|EP_Unlikely) ){
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Unlikely) ){
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->x.pList->n_సమాసం>0 );
      ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
      p_సమాసం = p_సమాసం->x.pList->a[0].p_సమాసం;
    }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_COLLATE ){
      p_సమాసం = p_సమాసం->p_ఎడమ;
    }లేకపోతే{
      ఆపు;
    }
  }  
  తిరిగి p_సమాసం;
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
CollSeq *sqlite3_సమాసం_Coll_Seq(Parse *pParse, స్థిరరాశి సమాసం *p_సమాసం){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  స్థిరరాశి సమాసం *p = p_సమాసం;
  అయితే( p ){
    పూర్ణాంకం op = p->op;
    ఒకవేళ( op==TK_రిజిస్టరు ) op = p->op2;
    ఒకవేళ( (op==TK_AGG_స్తంభం && p->y.pTab!=0)
     || op==TK_స్తంభం || op==TK_TRIGGER
    ){
      పూర్ణాంకం j;
      ప్రతిజ్ఞా( సమాసం_Use_YTab(p) );
      ప్రతిజ్ఞా( p->y.pTab!=0 );
      ఒకవేళ( (j = p->i_స్తంభం)>=0 ){
        స్థిరరాశి అక్షరం *zColl = sqlite3_స్తంభం_Coll(&p->y.pTab->a_నిలువు[j]);
        pColl = sqlite3_కనుగొను_Coll_Seq(db, ENC(db), zColl, 0);
      }
      ఆపు;
    }
    ఒకవేళ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ఎడమ;
      కొనసాగు;
    }
    ఒకవేళ( op==TK_సదిశ
     || (op==TK_FUNCTION && p->aff_సమాసం==SQLITE_AFF_DEFER)
    ){
      ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
      p = p->x.pList->a[0].p_సమాసం;
      కొనసాగు;
    }
    ఒకవేళ( op==TK_COLLATE ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_Int_విలువ) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ఆపు;
    }
    ఒకవేళ( p->flags & EP_Collate ){
      ఒకవేళ( p->p_ఎడమ && (p->p_ఎడమ->flags & EP_Collate)!=0 ){
        p = p->p_ఎడమ;
      }లేకపోతే{
        సమాసం *p_తదుపరి  = p->p_కుడి;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ప్రతిజ్ఞా( !సమాసం_Use_XList(p) || p->x.pList==0 || p->p_కుడి==0 );
        ఒకవేళ( సమాసం_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          పూర్ణాంకం i;
          కొరకు(i=0; i<p->x.pList->n_సమాసం; i++){
            ఒకవేళ( సమాసం_Has_Property(p->x.pList->a[i].p_సమాసం, EP_Collate) ){
              p_తదుపరి = p->x.pList->a[i].p_సమాసం;
              ఆపు;
            }
          }
        }
        p = p_తదుపరి;
      }
    }లేకపోతే{
      ఆపు;
    }
  }
  ఒకవేళ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  తిరిగి pColl;
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
CollSeq *sqlite3_సమాసం_NNColl_Seq(Parse *pParse, స్థిరరాశి సమాసం *p_సమాసం){
  CollSeq *p = sqlite3_సమాసం_Coll_Seq(pParse, p_సమాసం);
  ఒకవేళ( p==0 ) p = pParse->db->pDfltColl;
  ప్రతిజ్ఞా( p!=0 );
  తిరిగి p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
పూర్ణాంకం sqlite3_సమాసం_Coll_Seq_సరిపోలిక(Parse *pParse, స్థిరరాశి సమాసం *pE1, స్థిరరాశి సమాసం *pE2){
  CollSeq *pColl1 = sqlite3_సమాసం_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_సమాసం_NNColl_Seq(pParse, pE2);
  ప్రతిజ్ఞా( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  తిరిగి pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
అక్షరం sqlite3CompareAffinity(స్థిరరాశి సమాసం *p_సమాసం, అక్షరం aff2){
  అక్షరం aff1 = sqlite3_సమాసం_Affinity(p_సమాసం);
  ఒకవేళ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ఒకవేళ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      తిరిగి SQLITE_AFF_NUMERIC;
    }లేకపోతే{
      తిరిగి SQLITE_AFF_BLOB;
    }
  }లేకపోతే{
    /* One side is a column, the other is not. Use the columns affinity. */
    ప్రతిజ్ఞా( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    తిరిగి (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
స్థిర అక్షరం comparisonAffinity(స్థిరరాశి సమాసం *p_సమాసం){
  అక్షరం aff;
  ప్రతిజ్ఞా( p_సమాసం->op==TK_EQ || p_సమాసం->op==TK_లోపల || p_సమాసం->op==TK_LT ||
          p_సమాసం->op==TK_GT || p_సమాసం->op==TK_GE || p_సమాసం->op==TK_LE ||
          p_సమాసం->op==TK_NE || p_సమాసం->op==TK_IS || p_సమాసం->op==TK_ISNOT );
  ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ );
  ప్రతిజ్ఞా( !సమాసం_Use_XSelect(p_సమాసం) );
  aff = sqlite3_సమాసం_Affinity(p_సమాసం->p_ఎడమ);
  ఒకవేళ( p_సమాసం->p_కుడి ){
    aff = sqlite3CompareAffinity(p_సమాసం->p_కుడి, aff);
  }లేకపోతే ఒకవేళ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  తిరిగి aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
పూర్ణాంకం sqlite3_సూచీ_Affinity_Ok(స్థిరరాశి సమాసం *p_సమాసం, అక్షరం idx_affinity){
  అక్షరం aff = comparisonAffinity(p_సమాసం);
  ఒకవేళ( aff<SQLITE_AFF_పాఠ్యం ){
    తిరిగి 1;
  }
  ఒకవేళ( aff==SQLITE_AFF_పాఠ్యం ){
    తిరిగి idx_affinity==SQLITE_AFF_పాఠ్యం;
  }
  తిరిగి sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
స్థిర u8 ద్విమాన_Compare_P5(
  స్థిరరాశి సమాసం *pExpr1,   /* Left operand */
  స్థిరరాశి సమాసం *pExpr2,   /* Right operand */
  పూర్ణాంకం jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (అక్షరం)sqlite3_సమాసం_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  తిరిగి aff;
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
CollSeq *sqlite3_ద్విమాన_Compare_Coll_Seq(
  Parse *pParse,
  స్థిరరాశి సమాసం *p_ఎడమ,
  స్థిరరాశి సమాసం *p_కుడి
){
  CollSeq *pColl;
  ప్రతిజ్ఞా( p_ఎడమ );
  ఒకవేళ( p_ఎడమ->flags & EP_Collate ){
    pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_ఎడమ);
  }లేకపోతే ఒకవేళ( p_కుడి && (p_కుడి->flags & EP_Collate)!=0 ){
    pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_కుడి);
  }లేకపోతే{
    pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_ఎడమ);
    ఒకవేళ( !pColl ){
      pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_కుడి);
    }
  }
  తిరిగి pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_సమాసం_Compare_Coll_Seq(Parse *pParse, స్థిరరాశి సమాసం *p){
  ఒకవేళ( సమాసం_Has_Property(p, EP_Commuted) ){
    తిరిగి sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p->p_కుడి, p->p_ఎడమ);
  }లేకపోతే{
    తిరిగి sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p->p_ఎడమ, p->p_కుడి);
  }
}

/*
** Generate code for a comparison operator.
*/
స్థిర పూర్ణాంకం సంకేతం_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  సమాసం *p_ఎడమ,      /* The left operand */
  సమాసం *p_కుడి,     /* The right operand */
  పూర్ణాంకం opcode,       /* The comparison opcode */
  పూర్ణాంకం in1, పూర్ణాంకం in2, /* Register holding operands */
  పూర్ణాంకం dest,         /* Jump here if true.  */
  పూర్ణాంకం jumpIfNull,   /* If true, jump if either operand is NULL */
  పూర్ణాంకం isCommuted    /* The comparison has been commuted */
){
  పూర్ణాంకం p5;
  పూర్ణాంకం addr;
  CollSeq *p4;

  ఒకవేళ( pParse->nErr ) తిరిగి 0;
  ఒకవేళ( isCommuted ){
    p4 = sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p_కుడి, p_ఎడమ);
  }లేకపోతే{
    p4 = sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p_ఎడమ, p_కుడి);
  }
  p5 = ద్విమాన_Compare_P5(p_ఎడమ, p_కుడి, jumpIfNull);
  addr = sqlite3_Vdbe_కూడు_Op4(pParse->pVdbe, opcode, in2, dest, in1,
                           (శూన్య*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  తిరిగి addr;
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
పూర్ణాంకం sqlite3_సమాసం_Is_సదిశ(స్థిరరాశి సమాసం *p_సమాసం){
  తిరిగి sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
పూర్ణాంకం sqlite3_సమాసం_సదిశ_కొలత(స్థిరరాశి సమాసం *p_సమాసం){
  u8 op = p_సమాసం->op;
  ఒకవేళ( op==TK_రిజిస్టరు ) op = p_సమాసం->op2;
  ఒకవేళ( op==TK_సదిశ ){
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    తిరిగి p_సమాసం->x.pList->n_సమాసం;
  }లేకపోతే ఒకవేళ( op==TK_SELECT ){
    ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
    తిరిగి p_సమాసం->x.pSelect->pEList->n_సమాసం;
  }లేకపోతే{
    తిరిగి 1;
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
సమాసం *sqlite3_సదిశ_క్షేత్రం_Subexpr(సమాసం *p_సదిశ, పూర్ణాంకం i){
  ప్రతిజ్ఞా( i<sqlite3_సమాసం_సదిశ_కొలత(p_సదిశ) || p_సదిశ->op==TK_ERROR );
  ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_సదిశ) ){
    ప్రతిజ్ఞా( p_సదిశ->op2==0 || p_సదిశ->op==TK_రిజిస్టరు );
    ఒకవేళ( p_సదిశ->op==TK_SELECT || p_సదిశ->op2==TK_SELECT ){
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సదిశ) );
      తిరిగి p_సదిశ->x.pSelect->pEList->a[i].p_సమాసం;
    }లేకపోతే{
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సదిశ) );
      తిరిగి p_సదిశ->x.pList->a[i].p_సమాసం;
    }
  }
  తిరిగి p_సదిశ;
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
సమాసం *sqlite3_సమాసం_For_సదిశ_క్షేత్రం(
  Parse *pParse,       /* Parsing context */
  సమాసం *p_సదిశ,       /* The vector.  List of expressions or a sub-SELECT */
  పూర్ణాంకం i_క్షేత్రం,          /* Which column of the vector to return */
  పూర్ణాంకం n_క్షేత్రం           /* Total number of columns in the vector */
){
  సమాసం *pRet;
  ఒకవేళ( p_సదిశ->op==TK_SELECT ){
    ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సదిశ) );
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_స్తంభం, 0, 0);
    ఒకవేళ( pRet ){
      సమాసం_సమితి_Property(pRet, EP_నిండిన_కొలత);
      pRet->i_పట్టిక = n_క్షేత్రం;
      pRet->i_స్తంభం = i_క్షేత్రం;
      pRet->p_ఎడమ = p_సదిశ;
    }
  }లేకపోతే{
    ఒకవేళ( p_సదిశ->op==TK_సదిశ ){
      సమాసం **pp_సదిశ;
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సదిశ) );
      pp_సదిశ = &p_సదిశ->x.pList->a[i_క్షేత్రం].p_సమాసం;
      p_సదిశ = *pp_సదిశ;
      ఒకవేళ( లోపల_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_సదిశ = 0;
        తిరిగి p_సదిశ;
      }
    }
    pRet = sqlite3_సమాసం_Dup(pParse->db, p_సదిశ, 0);
  }
  తిరిగి pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
స్థిర పూర్ణాంకం సమాసం_సంకేతం_Subselect(Parse *pParse, సమాసం *p_సమాసం){
  పూర్ణాంకం reg = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
  ఒకవేళ( p_సమాసం->op==TK_SELECT ){
    reg = sqlite3_సంకేతం_Subselect(pParse, p_సమాసం);
  }
#అయితే_ముగింపు
  తిరిగి reg;
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
స్థిర పూర్ణాంకం సమాసం_సదిశ_రిజిస్టరు(
  Parse *pParse,                  /* Parse context */
  సమాసం *p_సదిశ,                  /* Vector to extract element from */
  పూర్ణాంకం i_క్షేత్రం,                     /* Field to extract from pVector */
  పూర్ణాంకం regSelect,                  /* First in array of registers */
  సమాసం *pTmp,                     /* Temporary space */
  సమాసం **pp_సమాసం,                  /* OUT: Expression element */
  పూర్ణాంకం *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_సదిశ->op;
  ప్రతిజ్ఞా( op==TK_సదిశ || op==TK_రిజిస్టరు || op==TK_SELECT || op==TK_ERROR );
  ఒకవేళ( op==TK_రిజిస్టరు ){
    *pp_సమాసం = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_సదిశ, i_క్షేత్రం);
    తిరిగి p_సదిశ->i_పట్టిక+i_క్షేత్రం;
  }
  ఒకవేళ( op==TK_SELECT ){
    ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సదిశ) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, పరిమాణం(సమాసం));
    pTmp->op = TK_SELECT_స్తంభం;
    pTmp->p_ఎడమ = p_సదిశ;
    pTmp->i_స్తంభం = i_క్షేత్రం;
    pTmp->i_పట్టిక = p_సదిశ->x.pSelect->pEList->n_సమాసం;
    *pp_సమాసం = pTmp;
    తిరిగి regSelect+i_క్షేత్రం;
  }
  ఒకవేళ( op==TK_సదిశ ){
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సదిశ) );
    *pp_సమాసం = p_సదిశ->x.pList->a[i_క్షేత్రం].p_సమాసం;
    తిరిగి sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, *pp_సమాసం, pRegFree);
  }
  తిరిగి 0;
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
స్థిర శూన్య సంకేతం_సదిశ_Compare(
  Parse *pParse,        /* Code generator context */
  సమాసం *p_సమాసం,          /* The comparison operation */
  పూర్ణాంకం dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
  సమాసం *p_కుడి = p_సమాసం->p_కుడి;
  పూర్ణాంకం n_ఎడమ = sqlite3_సమాసం_సదిశ_కొలత(p_ఎడమ);
  పూర్ణాంకం i;
  పూర్ణాంకం reg_ఎడమ = 0;
  పూర్ణాంకం reg_కుడి = 0;
  u8 opx = op;
  పూర్ణాంకం addrCmp = 0;
  పూర్ణాంకం addr_పూర్తి = sqlite3VdbeMakeLabel(pParse);
  పూర్ణాంకం isCommuted = సమాసం_Has_Property(p_సమాసం,EP_Commuted);

  ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p_సమాసం,EP_Immutable) );
  ఒకవేళ( pParse->nErr ) తిరిగి;
  ఒకవేళ( n_ఎడమ!=sqlite3_సమాసం_సదిశ_కొలత(p_కుడి) ){
    sqlite3_Error_సందేశం(pParse, "row value misused");
    తిరిగి;
  }
  ప్రతిజ్ఞా( p_సమాసం->op==TK_EQ || p_సమాసం->op==TK_NE
       || p_సమాసం->op==TK_IS || p_సమాసం->op==TK_ISNOT
       || p_సమాసం->op==TK_LT || p_సమాసం->op==TK_GT
       || p_సమాసం->op==TK_LE || p_సమాసం->op==TK_GE
  );
  ప్రతిజ్ఞా( p_సమాసం->op==op || (p_సమాసం->op==TK_IS && op==TK_EQ)
            || (p_సమాసం->op==TK_ISNOT && op==TK_NE) );
  ప్రతిజ్ఞా( p5==0 || p_సమాసం->op!=op );
  ప్రతిజ్ఞా( p5==SQLITE_NULLEQ || p_సమాసం->op==op );

  ఒకవేళ( op==TK_LE ) opx = TK_LT;
  ఒకవేళ( op==TK_GE ) opx = TK_GT;
  ఒకవేళ( op==TK_NE ) opx = TK_EQ;

  reg_ఎడమ = సమాసం_సంకేతం_Subselect(pParse, p_ఎడమ);
  reg_కుడి = సమాసం_సంకేతం_Subselect(pParse, p_కుడి);

  sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, dest);
  కొరకు(i=0; 1 /*Loop exits by "break"*/; i++){
    పూర్ణాంకం regFree1 = 0, regFree2 = 0;
    సమాసం *pL = 0, *pR = 0;
    సమాసం tmp1, tmp2;
    పూర్ణాంకం r1, r2;
    ప్రతిజ్ఞా( i>=0 && i<n_ఎడమ );
    ఒకవేళ( addrCmp ) sqlite3_Vdbe_Jump_ఇక్కడ(v, addrCmp);
    r1 = సమాసం_సదిశ_రిజిస్టరు(pParse, p_ఎడమ, i, reg_ఎడమ, &tmp1, &pL, &regFree1);
    r2 = సమాసం_సదిశ_రిజిస్టరు(pParse, p_కుడి, i, reg_కుడి, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    సంకేతం_Compare(pParse, pL, pR, opx, r1, r2, addr_పూర్తి, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree1);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree2);
    ఒకవేళ( (opx==TK_LT || opx==TK_GT) && i<n_ఎడమ-1 ){
      addrCmp = sqlite3_Vdbe_కూడు_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ఒకవేళ( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, dest);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ఒకవేళ( i==n_ఎడమ-1 ){
      ఆపు;
    }
    ఒకవేళ( opx==TK_EQ ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_NotNull, dest, addr_పూర్తి); VdbeCoverage(v);
    }లేకపోతే{
      ప్రతిజ్ఞా( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, addr_పూర్తి);
      ఒకవేళ( i==n_ఎడమ-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_పూర్తి);
  ఒకవేళ( op==TK_NE ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Not, dest, dest);
  }
}

#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
పూర్ణాంకం sqlite3_సమాసం_Check_ఎత్తు(Parse *pParse, పూర్ణాంకం n_ఎత్తు){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం mx_ఎత్తు = pParse->db->a_హద్దు[SQLITE_హద్దు_సమాసం_లోతు];
  ఒకవేళ( n_ఎత్తు>mx_ఎత్తు ){
    sqlite3_Error_సందేశం(pParse,
       "Expression tree is too large (maximum depth %d)", mx_ఎత్తు
    );
    rc = SQLITE_ERROR;
  }
  తిరిగి rc;
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
స్థిర శూన్య ఎత్తు_యొక్క_సమాసం(స్థిరరాశి సమాసం *p, పూర్ణాంకం *pn_ఎత్తు){
  ఒకవేళ( p ){
    ఒకవేళ( p->n_ఎత్తు>*pn_ఎత్తు ){
      *pn_ఎత్తు = p->n_ఎత్తు;
    }
  }
}
స్థిర శూన్య ఎత్తు_యొక్క_సమాసం_List(స్థిరరాశి సమాసం_List *p, పూర్ణాంకం *pn_ఎత్తు){
  ఒకవేళ( p ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->n_సమాసం; i++){
      ఎత్తు_యొక్క_సమాసం(p->a[i].p_సమాసం, pn_ఎత్తు);
    }
  }
}
స్థిర శూన్య ఎత్తు_యొక్క_Select(స్థిరరాశి Select *pSelect, పూర్ణాంకం *pn_ఎత్తు){
  స్థిరరాశి Select *p;
  కొరకు(p=pSelect; p; p=p->p_పూర్వ){
    ఎత్తు_యొక్క_సమాసం(p->pWhere, pn_ఎత్తు);
    ఎత్తు_యొక్క_సమాసం(p->pHaving, pn_ఎత్తు);
    ఎత్తు_యొక్క_సమాసం(p->p_హద్దు, pn_ఎత్తు);
    ఎత్తు_యొక్క_సమాసం_List(p->pEList, pn_ఎత్తు);
    ఎత్తు_యొక్క_సమాసం_List(p->p_సమూహం_By, pn_ఎత్తు);
    ఎత్తు_యొక్క_సమాసం_List(p->p_క్రమం_By, pn_ఎత్తు);
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
స్థిర శూన్య సమాసం_సమితి_ఎత్తు(సమాసం *p){
  పూర్ణాంకం n_ఎత్తు = p->p_ఎడమ ? p->p_ఎడమ->n_ఎత్తు : 0;
  ఒకవేళ( NEVER(p->p_కుడి) && p->p_కుడి->n_ఎత్తు>n_ఎత్తు ){
    n_ఎత్తు = p->p_కుడి->n_ఎత్తు;
  }
  ఒకవేళ( సమాసం_Use_XSelect(p) ){
    ఎత్తు_యొక్క_Select(p->x.pSelect, &n_ఎత్తు);
  }లేకపోతే ఒకవేళ( p->x.pList ){
    ఎత్తు_యొక్క_సమాసం_List(p->x.pList, &n_ఎత్తు);
    p->flags |= EP_Propagate & sqlite3_సమాసం_List_Flags(p->x.pList);
  }
  p->n_ఎత్తు = n_ఎత్తు + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
శూన్య sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(Parse *pParse, సమాసం *p){
  ఒకవేళ( pParse->nErr ) తిరిగి;
  సమాసం_సమితి_ఎత్తు(p);
  sqlite3_సమాసం_Check_ఎత్తు(pParse, p->n_ఎత్తు);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
పూర్ణాంకం sqlite3_Select_సమాసం_ఎత్తు(స్థిరరాశి Select *p){
  పూర్ణాంకం n_ఎత్తు = 0;
  ఎత్తు_యొక్క_Select(p, &n_ఎత్తు);
  తిరిగి n_ఎత్తు;
}
#లేకపోతే /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
శూన్య sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(Parse *pParse, సమాసం *p){
  ఒకవేళ( pParse->nErr ) తిరిగి;
  ఒకవేళ( p && సమాసం_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_సమాసం_List_Flags(p->x.pList);
  }
}
#నిర్వచించు సమాసం_సమితి_ఎత్తు(y)
#అయితే_ముగింపు /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
శూన్య sqlite3_సమాసం_సమితి_Error_స్థానభ్రంశం(సమాసం *p_సమాసం, పూర్ణాంకం iOfst){
  ఒకవేళ( p_సమాసం==0 ) తిరిగి;
  ఒకవేళ( NEVER(సమాసం_Use_WJoin(p_సమాసం)) ) తిరిగి;
  p_సమాసం->w.iOfst = iOfst;
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
సమాసం *sqlite3_సమాసం_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  పూర్ణాంకం op,                 /* Expression opcode */
  స్థిరరాశి Token *pToken,    /* Token argument.  Might be NULL */
  పూర్ణాంకం dequote             /* True to dequote */
){
  సమాసం *pNew;
  పూర్ణాంకం nExtra = pToken ? pToken->n+1 : 0;

  ప్రతిజ్ఞా( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, పరిమాణం(సమాసం)+nExtra);
  ఒకవేళ( pNew ){
    memset(pNew, 0, పరిమాణం(సమాసం));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ఒకవేళ( nExtra ){
      ప్రతిజ్ఞా( pToken!=0 );
      pNew->u.zToken = (అక్షరం*)&pNew[1];
      ప్రతిజ్ఞా( pToken->z!=0 || pToken->n==0 );
      ఒకవేళ( pToken->n ) జ్ఞాపకం_నకలు(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ఒకవేళ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_సమాసం(pNew);
      }
    }
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    pNew->n_ఎత్తు = 1;
#అయితే_ముగింపు 
  }
  తిరిగి pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
సమాసం *sqlite3_సమాసం(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  పూర్ణాంకం op,                 /* Expression opcode */
  స్థిరరాశి అక్షరం *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  తిరిగి sqlite3_సమాసం_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
సమాసం *sqlite3_సమాసం_Int32(sqlite3 *db, పూర్ణాంకం iVal){
  సమాసం *pNew = sqlite3DbMallocRawNN(db, పరిమాణం(సమాసం));
  ఒకవేళ( pNew ){
    memset(pNew, 0, పరిమాణం(సమాసం));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_విలువ|EP_ఆకు|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_విలువ = iVal;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    pNew->n_ఎత్తు = 1;
#అయితే_ముగింపు 
  }
  తిరిగి pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
శూన్య sqlite3_సమాసం_Attach_Subtrees(
  sqlite3 *db,
  సమాసం *p_మూలం,
  సమాసం *p_ఎడమ,
  సమాసం *p_కుడి
){
  ఒకవేళ( p_మూలం==0 ){
    ప్రతిజ్ఞా( db->mallocFailed );
    sqlite3_సమాసం_Delete(db, p_ఎడమ);
    sqlite3_సమాసం_Delete(db, p_కుడి);
  }లేకపోతే{
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_మూలం) );
    ప్రతిజ్ఞా( p_మూలం->x.pSelect==0 );
    ఒకవేళ( p_కుడి ){
      p_మూలం->p_కుడి = p_కుడి;
      p_మూలం->flags |= EP_Propagate & p_కుడి->flags;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
      p_మూలం->n_ఎత్తు = p_కుడి->n_ఎత్తు+1;
    }లేకపోతే{
      p_మూలం->n_ఎత్తు = 1;
#అయితే_ముగింపు
    }
    ఒకవేళ( p_ఎడమ ){
      p_మూలం->p_ఎడమ = p_ఎడమ;
      p_మూలం->flags |= EP_Propagate & p_ఎడమ->flags;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
      ఒకవేళ( p_ఎడమ->n_ఎత్తు>=p_మూలం->n_ఎత్తు ){
        p_మూలం->n_ఎత్తు = p_ఎడమ->n_ఎత్తు+1;
      }
#అయితే_ముగింపు
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
సమాసం *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  పూర్ణాంకం op,                 /* Expression opcode */
  సమాసం *p_ఎడమ,            /* Left operand */
  సమాసం *p_కుడి            /* Right operand */
){
  సమాసం *p;
  p = sqlite3DbMallocRawNN(pParse->db, పరిమాణం(సమాసం));
  ఒకవేళ( p ){
    memset(p, 0, పరిమాణం(సమాసం));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_సమాసం_Attach_Subtrees(pParse->db, p, p_ఎడమ, p_కుడి);
    sqlite3_సమాసం_Check_ఎత్తు(pParse, p->n_ఎత్తు);
  }లేకపోతే{
    sqlite3_సమాసం_Delete(pParse->db, p_ఎడమ);
    sqlite3_సమాసం_Delete(pParse->db, p_కుడి);
  }
  తిరిగి p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
శూన్య sqlite3_PExpr_కూడు_Select(Parse *pParse, సమాసం *p_సమాసం, Select *pSelect){
  ఒకవేళ( p_సమాసం ){
    p_సమాసం->x.pSelect = pSelect;
    సమాసం_సమితి_Property(p_సమాసం, EP_xIsSelect|EP_Subquery);
    sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(pParse, p_సమాసం);
  }లేకపోతే{
    ప్రతిజ్ఞా( pParse->db->mallocFailed );
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
Select *sqlite3_సమాసం_List_గమ్యం_విలువలు(Parse *pParse, పూర్ణాంకం nElem, సమాసం_List *pEList){
  పూర్ణాంకం ii;
  Select *pRet = 0;
  ప్రతిజ్ఞా( nElem>1 );
  కొరకు(ii=0; ii<pEList->n_సమాసం; ii++){
    Select *pSel;
    సమాసం *p_సమాసం = pEList->a[ii].p_సమాసం;
    పూర్ణాంకం n_సమాసం_Elem;
    ఒకవేళ( p_సమాసం->op==TK_సదిశ ){
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      n_సమాసం_Elem = p_సమాసం->x.pList->n_సమాసం;
    }లేకపోతే{
      n_సమాసం_Elem = 1;
    }
    ఒకవేళ( n_సమాసం_Elem!=nElem ){
      sqlite3_Error_సందేశం(pParse, "IN(...) element has %d term%s - expected %d",
          n_సమాసం_Elem, n_సమాసం_Elem>1?"s":"", nElem
      );
      ఆపు;
    }
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    pSel = sqlite3SelectNew(pParse, p_సమాసం->x.pList, 0, 0, 0, 0, 0, SF_విలువలు,0);
    p_సమాసం->x.pList = 0;
    ఒకవేళ( pSel ){
      ఒకవేళ( pRet ){
        pSel->op = TK_ALL;
        pSel->p_పూర్వ = pRet;
      }
      pRet = pSel;
    }
  }

  ఒకవేళ( pRet && pRet->p_పూర్వ ){
    pRet->selFlags |= SF_Multi_విలువ;
  }
  sqlite3_సమాసం_List_Delete(pParse->db, pEList);
  తిరిగి pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
సమాసం *sqlite3_సమాసం_And(Parse *pParse, సమాసం *p_ఎడమ, సమాసం *p_కుడి){
  sqlite3 *db = pParse->db;
  ఒకవేళ( p_ఎడమ==0  ){
    తిరిగి p_కుడి;
  }లేకపోతే ఒకవేళ( p_కుడి==0 ){
    తిరిగి p_ఎడమ;
  }లేకపోతే{
    u32 f = p_ఎడమ->flags | p_కుడి->flags;
    ఒకవేళ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !లోపల_RENAME_OBJECT
    ){
      సమాసం *p;
      sqlite3_సమాసం_Deferred_Delete(pParse, p_ఎడమ);
      sqlite3_సమాసం_Deferred_Delete(pParse, p_కుడి);
      p = sqlite3_సమాసం_Int32(db, 0);
      ఒకవేళ( p ) p->op2 = 1;
      తిరిగి p;
    }లేకపోతే{
      తిరిగి sqlite3PExpr(pParse, TK_AND, p_ఎడమ, p_కుడి);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
సమాసం *sqlite3_సమాసం_Function(
  Parse *pParse,        /* Parsing context */
  సమాసం_List *pList,      /* Argument list */
  స్థిరరాశి Token *pToken,  /* Name of the function */
  పూర్ణాంకం eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  సమాసం *pNew;
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( pToken );
  pNew = sqlite3_సమాసం_Alloc(db, TK_FUNCTION, pToken, 1);
  ఒకవేళ( pNew==0 ){
    sqlite3_సమాసం_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( !సమాసం_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (పూర్ణాంకం)(pToken->z - pParse->z_తోక);
  ఒకవేళ( pList
   && pList->n_సమాసం > pParse->db->a_హద్దు[SQLITE_హద్దు_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_సందేశం(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  సమాసం_సమితి_Property(pNew, EP_HasFunc);
  ప్రతిజ్ఞా( సమాసం_Use_XList(pNew) );
  sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(pParse, pNew);
  ఒకవేళ( eDistinct==SF_Distinct ) సమాసం_సమితి_Property(pNew, EP_Distinct);
  తిరిగి pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
శూన్య sqlite3_సమాసం_క్రమం_By_Aggregate_Error(Parse *pParse, సమాసం *p){
  sqlite3_Error_సందేశం(pParse,
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
శూన్య sqlite3_సమాసం_కూడు_Function_క్రమం_By(
  Parse *pParse,        /* Parsing context */
  సమాసం *p_సమాసం,          /* The function call to which ORDER BY is to be added */
  సమాసం_List *p_క్రమం_By    /* The ORDER BY clause to add */
){
  సమాసం *pOB;
  sqlite3 *db = pParse->db;
  ఒకవేళ( NEVER(p_క్రమం_By==0) ){
    ప్రతిజ్ఞా( db->mallocFailed );
    తిరిగి;
  }
  ఒకవేళ( p_సమాసం==0 ){
    ప్రతిజ్ఞా( db->mallocFailed );
    sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
    తిరిగి;
  }
  ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
  ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ==0 );
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  ఒకవేళ( p_సమాసం->x.pList==0 || NEVER(p_సమాసం->x.pList->n_సమాసం==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3_సమాసం_List_Delete_Generic, p_క్రమం_By);
    తిరిగి;
  }
  ఒకవేళ( Is_కిటికీ_Func(p_సమాసం) ){
    sqlite3_సమాసం_క్రమం_By_Aggregate_Error(pParse, p_సమాసం);
    sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
    తిరిగి;
  }
  ఒకవేళ( p_క్రమం_By->n_సమాసం>db->a_హద్దు[SQLITE_హద్దు_స్తంభం] ){
    sqlite3_Error_సందేశం(pParse, "too many terms in ORDER BY clause");
    sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
    తిరిగి;
  }

  pOB = sqlite3_సమాసం_Alloc(db, TK_క్రమం, 0, 0);
  ఒకవేళ( pOB==0 ){
    sqlite3_సమాసం_List_Delete(db, p_క్రమం_By);
    తిరిగి;
  }
  pOB->x.pList = p_క్రమం_By;
  ప్రతిజ్ఞా( సమాసం_Use_XList(pOB) );
  p_సమాసం->p_ఎడమ = pOB;
  సమాసం_సమితి_Property(pOB, EP_నిండిన_కొలత);
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
శూన్య sqlite3_సమాసం_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  స్థిరరాశి సమాసం *p_సమాసం,     /* The function invocation */
  స్థిరరాశి FuncDef *pDef    /* The function being invoked */
){
  ప్రతిజ్ఞా( !లోపల_RENAME_OBJECT );
  ప్రతిజ్ఞా( (pDef->funcFlags & (SQLITE_FUNC_ప్రత్యక్ష|SQLITE_FUNC_UNSAFE))!=0 );
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_ప్రత్యక్ష)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_సందేశం(pParse, "unsafe use of %#T()", p_సమాసం);
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
శూన్య sqlite3_సమాసం_కేటాయించు_Var_సంఖ్య(Parse *pParse, సమాసం *p_సమాసం, u32 n){
  sqlite3 *db = pParse->db;
  స్థిరరాశి అక్షరం *z;
  ynVar x;

  ఒకవేళ( p_సమాసం==0 ) తిరిగి;
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ|EP_Reduced|EP_TokenOnly) );
  z = p_సమాసం->u.zToken;
  ప్రతిజ్ఞా( z!=0 );
  ప్రతిజ్ఞా( z[0]!=0 );
  ప్రతిజ్ఞా( n==(u32)sqlite3Strlen30(z) );
  ఒకవేళ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ప్రతిజ్ఞా( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }లేకపోతే{
    పూర్ణాంకం do_కూడు = 0;
    ఒకవేళ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      పూర్ణాంకం bOk;
      ఒకవేళ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }లేకపోతే{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->a_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య]-1 );
      testcase( i==db->a_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య] );
      ఒకవేళ( bOk==0 || i<1 || i>db->a_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య] ){
        sqlite3_Error_సందేశం(pParse, "variable number must be between ?1 and ?%d",
            db->a_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య]);
        sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, p_సమాసం);
        తిరిగి;
      }
      x = (ynVar)i;
      ఒకవేళ( x>pParse->nVar ){
        pParse->nVar = (పూర్ణాంకం)x;
        do_కూడు = 1;
      }లేకపోతే ఒకవేళ( x<SQLITE_VNBMC*64 ){
        do_కూడు = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }లేకపోతే ఒకవేళ( sqlite3_VList_అంకె_గమ్యం_Name(pParse->pVList, x)==0 ){
        do_కూడు = 1;
      }
    }లేకపోతే{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_గమ్యం_అంకె(pParse->pVList, z, n);
      ఒకవేళ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_కూడు = 1;
      }
    }
    ఒకవేళ( do_కూడు ){
      pParse->pVList = sqlite3_VList_కూడు(db, pParse->pVList, z, n, x);
      ఒకవేళ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_సమాసం->i_స్తంభం = x;
  ఒకవేళ( x>db->a_హద్దు[SQLITE_హద్దు_VARIABLE_సంఖ్య] ){
    sqlite3_Error_సందేశం(pParse, "too many SQL variables");
    sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, p_సమాసం);
  }
}

/*
** Recursively delete an expression tree.
*/
స్థిర SQLITE_NOINLINE శూన్య sqlite3_సమాసం_Delete_NN(sqlite3 *db, సమాసం *p){
  ప్రతిజ్ఞా( p!=0 );
  ప్రతిజ్ఞా( db!=0 );
సమాసం_Delete_Restart:
  ప్రతిజ్ఞా( !సమాసం_Use_UValue(p) || p->u.i_విలువ>=0 );
  ప్రతిజ్ఞా( !సమాసం_Use_YWin(p) || !సమాసం_Use_YSub(p) );
  ప్రతిజ్ఞా( !సమాసం_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  ప్రతిజ్ఞా( p->op!=TK_FUNCTION || !సమాసం_Use_YSub(p) );
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( సమాసం_Has_Property(p, EP_ఆకు) && !సమాసం_Has_Property(p, EP_TokenOnly) ){
    ప్రతిజ్ఞా( p->p_ఎడమ==0 );
    ప్రతిజ్ఞా( p->p_కుడి==0 );
    ప్రతిజ్ఞా( !సమాసం_Use_XSelect(p) || p->x.pSelect==0 );
    ప్రతిజ్ఞా( !సమాసం_Use_XList(p) || p->x.pList==0 );
  }
#అయితే_ముగింపు
  ఒకవేళ( !సమాసం_Has_Property(p, (EP_TokenOnly|EP_ఆకు)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ప్రతిజ్ఞా( (సమాసం_Use_XList(p) && p->x.pList==0) || p->p_కుడి==0 );
    ఒకవేళ( p->p_కుడి ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_WinFunc) );
      sqlite3_సమాసం_Delete_NN(db, p->p_కుడి);
    }లేకపోతే ఒకవేళ( సమాసం_Use_XSelect(p) ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }లేకపోతే{
      sqlite3_సమాసం_List_Delete(db, p->x.pList);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( సమాసం_Has_Property(p, EP_WinFunc) ){
        sqlite3_కిటికీ_Delete(db, p->y.pWin);
      }
#అయితే_ముగింపు
    }
    ఒకవేళ( p->p_ఎడమ && p->op!=TK_SELECT_స్తంభం ){
      సమాసం *p_ఎడమ = p->p_ఎడమ;
      ఒకవేళ( !సమాసం_Has_Property(p, EP_Static)
       && !సమాసం_Has_Property(p_ఎడమ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ఎడమ;
        వెళ్ళు సమాసం_Delete_Restart;
      }లేకపోతే{
        sqlite3_సమాసం_Delete_NN(db, p_ఎడమ);
      }
    }
  }
  ఒకవేళ( !సమాసం_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
శూన్య sqlite3_సమాసం_Delete(sqlite3 *db, సమాసం *p){
  ఒకవేళ( p ) sqlite3_సమాసం_Delete_NN(db, p);
}
శూన్య sqlite3_సమాసం_Delete_Generic(sqlite3 *db, శూన్య *p){
  ఒకవేళ( ALWAYS(p) ) sqlite3_సమాసం_Delete_NN(db, (సమాసం*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
శూన్య sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ఒకవేళ( p==0 ){
    /* Nothing to clear */
  }లేకపోతే ఒకవేళ( p->pOn ){
    sqlite3_సమాసం_Delete_NN(db, p->pOn);
  }లేకపోతే ఒకవేళ( p->pUsing ){
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
పూర్ణాంకం sqlite3_సమాసం_Deferred_Delete(Parse *pParse, సమాసం *p_సమాసం){
  తిరిగి 0==sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3_సమాసం_Delete_Generic, p_సమాసం);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
శూన్య sqlite3_సమాసం_Unmap_And_Delete(Parse *pParse, సమాసం *p){
  ఒకవేళ( p ){
    ఒకవేళ( లోపల_RENAME_OBJECT ){
      sqlite3_Rename_సమాసం_Unmap(pParse, p);
    }
    sqlite3_సమాసం_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
స్థిర పూర్ణాంకం సమాసం_Struct_కొలత(స్థిరరాశి సమాసం *p){
  ఒకవేళ( సమాసం_Has_Property(p, EP_TokenOnly) ) తిరిగి సమాసం_TOKENONLYSIZE;
  ఒకవేళ( సమాసం_Has_Property(p, EP_Reduced) ) తిరిగి సమాసం_REDUCEDSIZE;
  తిరిగి సమాసం_FULLSIZE;
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
స్థిర పూర్ణాంకం duped_సమాసం_Struct_కొలత(స్థిరరాశి సమాసం *p, పూర్ణాంకం flags){
  పూర్ణాంకం n_కొలత;
  ప్రతిజ్ఞా( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ప్రతిజ్ఞా( సమాసం_FULLSIZE<=0xfff );
  ప్రతిజ్ఞా( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ఒకవేళ( 0==flags || సమాసం_Has_Property(p, EP_నిండిన_కొలత) ){
    n_కొలత = సమాసం_FULLSIZE;
  }లేకపోతే{
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_OuterON) );
    ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p, EP_NoReduce) );
    ఒకవేళ( p->p_ఎడమ || p->x.pList ){
      n_కొలత = సమాసం_REDUCEDSIZE | EP_Reduced;
    }లేకపోతే{
      ప్రతిజ్ఞా( p->p_కుడి==0 );
      n_కొలత = సమాసం_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  తిరిగి n_కొలత;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
స్థిర పూర్ణాంకం duped_సమాసం_కణుపు_కొలత(స్థిరరాశి సమాసం *p, పూర్ణాంకం flags){
  పూర్ణాంకం nByte = duped_సమాసం_Struct_కొలత(p, flags) & 0xfff;
  ఒకవేళ( !సమాసం_Has_Property(p, EP_Int_విలువ) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  తిరిగి ROUND8(nByte);
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
స్థిర పూర్ణాంకం duped_సమాసం_కొలత(స్థిరరాశి సమాసం *p){
  పూర్ణాంకం nByte;
  ప్రతిజ్ఞా( p!=0 );
  nByte = duped_సమాసం_కణుపు_కొలత(p, EXPRDUP_REDUCE);
  ఒకవేళ( p->p_ఎడమ ) nByte += duped_సమాసం_కొలత(p->p_ఎడమ);
  ఒకవేళ( p->p_కుడి ) nByte += duped_సమాసం_కొలత(p->p_కుడి);
  ప్రతిజ్ఞా( nByte==ROUND8(nByte) );
  తిరిగి nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
రకనిర్వచనం నిర్మాణం EdupBuf EdupBuf;
నిర్మాణం EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#నిర్వచితమైతే SQLITE_DEBUG
  u8 *z_చివర;            /* First byte past the end of memory */
#అయితే_ముగింపు
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
స్థిర సమాసం *సమాసం_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  స్థిరరాశి సమాసం *p,        /* Expr tree to be duplicated */
  పూర్ణాంకం dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  సమాసం *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  పూర్ణాంకం nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ప్రతిజ్ఞా( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ఒకవేళ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#నిర్వచితమైతే SQLITE_DEBUG
    sEdupBuf.z_చివర = pEdupBuf->z_చివర;
#అయితే_ముగింపు
    staticFlag = EP_Static;
    ప్రతిజ్ఞా( sEdupBuf.zAlloc!=0 );
    ప్రతిజ్ఞా( dupFlags==EXPRDUP_REDUCE );
  }లేకపోతే{
    పూర్ణాంకం nAlloc;
    ఒకవేళ( dupFlags ){
      nAlloc = duped_సమాసం_కొలత(p);
    }లేకపోతే ఒకవేళ( !సమాసం_Has_Property(p, EP_Int_విలువ) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(సమాసం_FULLSIZE + nToken);
    }లేకపోతే{
      nToken = 0;
      nAlloc = ROUND8(సమాసం_FULLSIZE);
    }
    ప్రతిజ్ఞా( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#నిర్వచితమైతే SQLITE_DEBUG
    sEdupBuf.z_చివర = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#అయితే_ముగింపు
    
    staticFlag = 0;
  }
  pNew = (సమాసం *)sEdupBuf.zAlloc;
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pNew) );

  ఒకవేళ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    స్థిరరాశి గుర్తులేని n_Struct_కొలత = duped_సమాసం_Struct_కొలత(p, dupFlags);
    పూర్ణాంకం n_New_కొలత = n_Struct_కొలత & 0xfff;
    ఒకవేళ( nToken<0 ){
      ఒకవేళ( !సమాసం_Has_Property(p, EP_Int_విలువ) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }లేకపోతే{
        nToken = 0;
      }
    }
    ఒకవేళ( dupFlags ){
      ప్రతిజ్ఞా( (పూర్ణాంకం)(sEdupBuf.z_చివర - sEdupBuf.zAlloc) >= n_New_కొలత+nToken );
      ప్రతిజ్ఞా( సమాసం_Has_Property(p, EP_Reduced)==0 );
      జ్ఞాపకం_నకలు(sEdupBuf.zAlloc, p, n_New_కొలత);
    }లేకపోతే{
      u32 n_కొలత = (u32)సమాసం_Struct_కొలత(p);
      ప్రతిజ్ఞా( (పూర్ణాంకం)(sEdupBuf.z_చివర - sEdupBuf.zAlloc) >=
                                                   (పూర్ణాంకం)సమాసం_FULLSIZE+nToken );
      జ్ఞాపకం_నకలు(sEdupBuf.zAlloc, p, n_కొలత);
      ఒకవేళ( n_కొలత<సమాసం_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_కొలత], 0, సమాసం_FULLSIZE-n_కొలత);
      }
      n_New_కొలత = సమాసం_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_కొలత & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    సమాసం_Clear_VVAProperties(pNew);
    ఒకవేళ( dupFlags ){
      సమాసం_సమితి_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ప్రతిజ్ఞా( nToken>=0 );
    ఒకవేళ( nToken>0 ){
      అక్షరం *zToken = pNew->u.zToken = (అక్షరం*)&sEdupBuf.zAlloc[n_New_కొలత];
      జ్ఞాపకం_నకలు(zToken, p->u.zToken, nToken);
      n_New_కొలత += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_కొలత);

    ఒకవేళ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_ఆకు))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ఒకవేళ( సమాసం_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }లేకపోతే{
        pNew->x.pList = sqlite3_సమాసం_List_Dup(db, p->x.pList,
                           p->op!=TK_క్రమం ? dupFlags : 0);
      }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( సమాసం_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_కిటికీ_Dup(db, pNew, p->y.pWin);
        ప్రతిజ్ఞా( సమాసం_Has_Property(pNew, EP_WinFunc) );
      }
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ఒకవేళ( dupFlags ){
        ఒకవేళ( p->op==TK_SELECT_స్తంభం ){
          pNew->p_ఎడమ = p->p_ఎడమ;
          ప్రతిజ్ఞా( p->p_కుడి==0 
               || p->p_కుడి==p->p_ఎడమ
               || సమాసం_Has_Property(p->p_ఎడమ, EP_Subquery) );
        }లేకపోతే{
          pNew->p_ఎడమ = p->p_ఎడమ ?
                      సమాసం_Dup(db, p->p_ఎడమ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_కుడి = p->p_కుడి ?
                       సమాసం_Dup(db, p->p_కుడి, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }లేకపోతే{
        ఒకవేళ( p->op==TK_SELECT_స్తంభం ){
          pNew->p_ఎడమ = p->p_ఎడమ;
          ప్రతిజ్ఞా( p->p_కుడి==0 
               || p->p_కుడి==p->p_ఎడమ
               || సమాసం_Has_Property(p->p_ఎడమ, EP_Subquery) );
        }లేకపోతే{
          pNew->p_ఎడమ = sqlite3_సమాసం_Dup(db, p->p_ఎడమ, 0);
        }
        pNew->p_కుడి = sqlite3_సమాసం_Dup(db, p->p_కుడి, 0);
      }
    }
  }
  ఒకవేళ( pEdupBuf ) జ్ఞాపకం_నకలు(pEdupBuf, &sEdupBuf, పరిమాణం(sEdupBuf));
  ప్రతిజ్ఞా( sEdupBuf.zAlloc <= sEdupBuf.z_చివర );
  తిరిగి pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ఒకవేళ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ఒకవేళ( pRet ){
      పూర్ణాంకం i;
      pRet->nCte = p->nCte;
      కొరకు(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].p_నిలువుల_సంఖ్య = sqlite3_సమాసం_List_Dup(db, p->a[i].p_నిలువుల_సంఖ్య, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  తిరిగి pRet;
}
#లేకపోతే
# define sqlite3WithDup(x,y) 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
స్థిర పూర్ణాంకం gatherSelectWindowsCallback(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_FUNCTION && సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    కిటికీ *pWin = p_సమాసం->y.pWin;
    ప్రతిజ్ఞా( pWin );
    ప్రతిజ్ఞా( Is_కిటికీ_Func(p_సమాసం) );
    ప్రతిజ్ఞా( pWin->ppThis==0 );
    sqlite3_కిటికీ_Link(pSelect, pWin);
  }
  తిరిగి WRC_Continue;
}
స్థిర పూర్ణాంకం gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  తిరిగి p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
స్థిర శూన్య gatherSelectWindows(Select *p){
  Walker w;
  w.x_సమాసం_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#అయితే_ముగింపు


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
సమాసం *sqlite3_సమాసం_Dup(sqlite3 *db, స్థిరరాశి సమాసం *p, పూర్ణాంకం flags){
  ప్రతిజ్ఞా( flags==0 || flags==EXPRDUP_REDUCE );
  తిరిగి p ? సమాసం_Dup(db, p, flags, 0) : 0;
}
సమాసం_List *sqlite3_సమాసం_List_Dup(sqlite3 *db, స్థిరరాశి సమాసం_List *p, పూర్ణాంకం flags){
  సమాసం_List *pNew;
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  స్థిరరాశి నిర్మాణం సమాసం_List_అంశం *p_Old_అంశం;
  పూర్ణాంకం i;
  సమాసం *p_పూర్వ_Select_నిలువు_Old = 0;
  సమాసం *p_పూర్వ_Select_నిలువు_New = 0;
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p==0 ) తిరిగి 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_కొలత(db, p));
  ఒకవేళ( pNew==0 ) తిరిగి 0;
  pNew->n_సమాసం = p->n_సమాసం;
  pNew->nAlloc = p->nAlloc;
  p_అంశం = pNew->a;
  p_Old_అంశం = p->a;
  కొరకు(i=0; i<p->n_సమాసం; i++, p_అంశం++, p_Old_అంశం++){
    సమాసం *p_Old_సమాసం = p_Old_అంశం->p_సమాసం;
    సమాసం *p_New_సమాసం;
    p_అంశం->p_సమాసం = sqlite3_సమాసం_Dup(db, p_Old_సమాసం, flags);
    ఒకవేళ( p_Old_సమాసం
     && p_Old_సమాసం->op==TK_SELECT_స్తంభం
     && (p_New_సమాసం = p_అంశం->p_సమాసం)!=0
    ){
      ఒకవేళ( p_New_సమాసం->p_కుడి ){
        p_పూర్వ_Select_నిలువు_Old = p_Old_సమాసం->p_కుడి;
        p_పూర్వ_Select_నిలువు_New = p_New_సమాసం->p_కుడి;
        p_New_సమాసం->p_ఎడమ = p_New_సమాసం->p_కుడి;
      }లేకపోతే{
        ఒకవేళ( p_Old_సమాసం->p_ఎడమ!=p_పూర్వ_Select_నిలువు_Old ){
          p_పూర్వ_Select_నిలువు_Old = p_Old_సమాసం->p_ఎడమ;
          p_పూర్వ_Select_నిలువు_New = sqlite3_సమాసం_Dup(db, p_పూర్వ_Select_నిలువు_Old, flags);
          p_New_సమాసం->p_కుడి = p_పూర్వ_Select_నిలువు_New;
        }
        p_New_సమాసం->p_ఎడమ = p_పూర్వ_Select_నిలువు_New;
      }
    }
    p_అంశం->zEName = sqlite3DbStrDup(db, p_Old_అంశం->zEName);
    p_అంశం->fg = p_Old_అంశం->fg;
    p_అంశం->u = p_Old_అంశం->u;
  }
  తిరిగి pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, స్థిరరాశి SrcList *p, పూర్ణాంకం flags){
  SrcList *pNew;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p==0 ) తిరిగి 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ఒకవేళ( pNew==0 ) తిరిగి 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  కొరకు(i=0; i<p->nSrc; i++){
    Src_అంశం *p_New_అంశం = &pNew->a[i];
    స్థిరరాశి Src_అంశం *p_Old_అంశం = &p->a[i];
    పట్టిక *pTab;
    p_New_అంశం->fg = p_Old_అంశం->fg;
    ఒకవేళ( p_Old_అంశం->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, పరిమాణం(Subquery));
      ఒకవేళ( pNewSubq==0 ){
        ప్రతిజ్ఞా( db->mallocFailed );
        p_New_అంశం->fg.isSubquery = 0;
      }లేకపోతే{
        జ్ఞాపకం_నకలు(pNewSubq, p_Old_అంశం->u4.pSubq, పరిమాణం(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ఒకవేళ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_అంశం->fg.isSubquery = 0;
        }
      }
      p_New_అంశం->u4.pSubq = pNewSubq;
    }లేకపోతే ఒకవేళ( p_Old_అంశం->fg.fixedSchema ){
      p_New_అంశం->u4.pSchema = p_Old_అంశం->u4.pSchema;
    }లేకపోతే{
      p_New_అంశం->u4.zDatabase = sqlite3DbStrDup(db, p_Old_అంశం->u4.zDatabase);
    }
    p_New_అంశం->zName = sqlite3DbStrDup(db, p_Old_అంశం->zName);
    p_New_అంశం->zAlias = sqlite3DbStrDup(db, p_Old_అంశం->zAlias);
    p_New_అంశం->iCursor = p_Old_అంశం->iCursor;
    ఒకవేళ( p_New_అంశం->fg.is_సూచీకృత_By ){
      p_New_అంశం->u1.z_సూచీకృత_By = sqlite3DbStrDup(db, p_Old_అంశం->u1.z_సూచీకృత_By);
    }లేకపోతే ఒకవేళ( p_New_అంశం->fg.isTabFunc ){
      p_New_అంశం->u1.pFuncArg =
          sqlite3_సమాసం_List_Dup(db, p_Old_అంశం->u1.pFuncArg, flags);
    }లేకపోతే{
      p_New_అంశం->u1.n_అడ్డువరుస = p_Old_అంశం->u1.n_అడ్డువరుస;
    }
    p_New_అంశం->u2 = p_Old_అంశం->u2;
    ఒకవేళ( p_New_అంశం->fg.isCte ){
      p_New_అంశం->u2.pCteUse->nUse++;
    }
    pTab = p_New_అంశం->pSTab = p_Old_అంశం->pSTab;
    ఒకవేళ( pTab ){
      pTab->nTabRef++;
    }
    ఒకవేళ( p_Old_అంశం->fg.isUsing ){
      ప్రతిజ్ఞా( p_New_అంశం->fg.isUsing );
      p_New_అంశం->u3.pUsing = sqlite3IdListDup(db, p_Old_అంశం->u3.pUsing);
    }లేకపోతే{
      p_New_అంశం->u3.pOn = sqlite3_సమాసం_Dup(db, p_Old_అంశం->u3.pOn, flags);
    }
    p_New_అంశం->నిలువు_వాడిన = p_Old_అంశం->నిలువు_వాడిన;
  }
  తిరిగి pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, స్థిరరాశి IdList *p){
  IdList *pNew;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p==0 ) తిరిగి 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ఒకవేళ( pNew==0 ) తిరిగి 0;
  pNew->nId = p->nId;
  కొరకు(i=0; i<p->nId; i++){
    నిర్మాణం Id_List_అంశం *p_New_అంశం = &pNew->a[i];
    స్థిరరాశి నిర్మాణం Id_List_అంశం *p_Old_అంశం = &p->a[i];
    p_New_అంశం->zName = sqlite3DbStrDup(db, p_Old_అంశం->zName);
  }
  తిరిగి pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, స్థిరరాశి Select *pDup, పూర్ణాంకం flags){
  Select *pRet = 0;
  Select *p_తదుపరి = 0;
  Select **pp = &pRet;
  స్థిరరాశి Select *p;

  ప్రతిజ్ఞా( db!=0 );
  కొరకు(p=pDup; p; p=p->p_పూర్వ){
    Select *pNew = sqlite3DbMallocRawNN(db, పరిమాణం(*p) );
    ఒకవేళ( pNew==0 ) ఆపు;
    pNew->pEList = sqlite3_సమాసం_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_సమాసం_Dup(db, p->pWhere, flags);
    pNew->p_సమూహం_By = sqlite3_సమాసం_List_Dup(db, p->p_సమూహం_By, flags);
    pNew->pHaving = sqlite3_సమాసం_Dup(db, p->pHaving, flags);
    pNew->p_క్రమం_By = sqlite3_సమాసం_List_Dup(db, p->p_క్రమం_By, flags);
    pNew->op = p->op;
    pNew->p_తదుపరి = p_తదుపరి;
    pNew->p_పూర్వ = 0;
    pNew->p_హద్దు = sqlite3_సమాసం_Dup(db, p->p_హద్దు, flags);
    pNew->i_హద్దు = 0;
    pNew->i_స్థానభ్రంశం = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_అడ్డువరుస = p->n_Select_అడ్డువరుస;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_కిటికీ_List_Dup(db, p->pWinDefn);
    ఒకవేళ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#అయితే_ముగింపు
    pNew->selId = p->selId;
    ఒకవేళ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_తదుపరి = 0;
      sqlite3SelectDelete(db, pNew);
      ఆపు;
    }
    *pp = pNew;
    pp = &pNew->p_పూర్వ;
    p_తదుపరి = pNew;
  }
  తిరిగి pRet;
}
#లేకపోతే
Select *sqlite3SelectDup(sqlite3 *db, స్థిరరాశి Select *p, పూర్ణాంకం flags){
  ప్రతిజ్ఞా( p==0 );
  తిరిగి 0;
}
#అయితే_ముగింపు


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
స్థిర స్థిరరాశి నిర్మాణం సమాసం_List_అంశం zero_అంశం = {0};
SQLITE_NOINLINE సమాసం_List *sqlite3_సమాసం_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  సమాసం *p_సమాసం             /* Expression to be appended. Might be NULL */
){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  సమాసం_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ఒకవేళ( pList==0 ){
    sqlite3_సమాసం_Delete(db, p_సమాసం);
    తిరిగి 0;
  }
  pList->nAlloc = 4;
  pList->n_సమాసం = 1;
  p_అంశం = &pList->a[0];
  *p_అంశం = zero_అంశం;
  p_అంశం->p_సమాసం = p_సమాసం;
  తిరిగి pList;
}
SQLITE_NOINLINE సమాసం_List *sqlite3_సమాసం_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  సమాసం_List *pList,        /* List to which to append. Might be NULL */
  సమాసం *p_సమాసం             /* Expression to be appended. Might be NULL */
){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  సమాసం_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ఒకవేళ( pNew==0 ){
    sqlite3_సమాసం_List_Delete(db, pList);
    sqlite3_సమాసం_Delete(db, p_సమాసం);
    తిరిగి 0;
  }లేకపోతే{
    pList = pNew;
  }
  p_అంశం = &pList->a[pList->n_సమాసం++];
  *p_అంశం = zero_అంశం;
  p_అంశం->p_సమాసం = p_సమాసం;
  తిరిగి pList;
}
సమాసం_List *sqlite3_సమాసం_List_Append(
  Parse *pParse,          /* Parsing context */
  సమాసం_List *pList,        /* List to which to append. Might be NULL */
  సమాసం *p_సమాసం             /* Expression to be appended. Might be NULL */
){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  ఒకవేళ( pList==0 ){
    తిరిగి sqlite3_సమాసం_List_Append_New(pParse->db,p_సమాసం);
  }
  ఒకవేళ( pList->nAlloc<pList->n_సమాసం+1 ){
    తిరిగి sqlite3_సమాసం_List_Append_Grow(pParse->db,pList,p_సమాసం);
  }
  p_అంశం = &pList->a[pList->n_సమాసం++];
  *p_అంశం = zero_అంశం;
  p_అంశం->p_సమాసం = p_సమాసం;
  తిరిగి pList;
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
సమాసం_List *sqlite3_సమాసం_List_Append_సదిశ(
  Parse *pParse,         /* Parsing context */
  సమాసం_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  సమాసం *p_సమాసం            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  పూర్ణాంకం n;
  పూర్ణాంకం i;
  పూర్ణాంకం i_మొదటి = pList ? pList->n_సమాసం : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ఒకవేళ( NEVER(pColumns==0) ) వెళ్ళు సదిశ_append_error;
  ఒకవేళ( p_సమాసం==0 ) వెళ్ళు సదిశ_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ఒకవేళ( p_సమాసం->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం)) ){
    sqlite3_Error_సందేశం(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    వెళ్ళు సదిశ_append_error;
  }

  కొరకు(i=0; i<pColumns->nId; i++){
    సమాసం *p_Sub_సమాసం = sqlite3_సమాసం_For_సదిశ_క్షేత్రం(pParse, p_సమాసం, i, pColumns->nId);
    ప్రతిజ్ఞా( p_Sub_సమాసం!=0 || db->mallocFailed );
    ఒకవేళ( p_Sub_సమాసం==0 ) కొనసాగు;
    pList = sqlite3_సమాసం_List_Append(pParse, pList, p_Sub_సమాసం);
    ఒకవేళ( pList ){
      ప్రతిజ్ఞా( pList->n_సమాసం==i_మొదటి+i+1 );
      pList->a[pList->n_సమాసం-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ఒకవేళ( !db->mallocFailed && p_సమాసం->op==TK_SELECT && ALWAYS(pList!=0) ){
    సమాసం *p_మొదటి = pList->a[i_మొదటి].p_సమాసం;
    ప్రతిజ్ఞా( p_మొదటి!=0 );
    ప్రతిజ్ఞా( p_మొదటి->op==TK_SELECT_స్తంభం );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    p_మొదటి->p_కుడి = p_సమాసం;
    p_సమాసం = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    p_మొదటి->i_పట్టిక = pColumns->nId;
  }

సదిశ_append_error:
  sqlite3_సమాసం_Unmap_And_Delete(pParse, p_సమాసం);
  sqlite3IdListDelete(db, pColumns);
  తిరిగి pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
శూన్య sqlite3_సమాసం_List_సమితి_క్రమీకరించు_క్రమం(సమాసం_List *p, పూర్ణాంకం i_క్రమీకరించు_క్రమం, పూర్ణాంకం eNulls){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  ఒకవేళ( p==0 ) తిరిగి;
  ప్రతిజ్ఞా( p->n_సమాసం>0 );

  ప్రతిజ్ఞా( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ప్రతిజ్ఞా( i_క్రమీకరించు_క్రమం==SQLITE_SO_UNDEFINED
       || i_క్రమీకరించు_క్రమం==SQLITE_SO_ASC
       || i_క్రమీకరించు_క్రమం==SQLITE_SO_DESC
  );
  ప్రతిజ్ఞా( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_అంశం = &p->a[p->n_సమాసం-1];
  ప్రతిజ్ఞా( p_అంశం->fg.bNulls==0 );
  ఒకవేళ( i_క్రమీకరించు_క్రమం==SQLITE_SO_UNDEFINED ){
    i_క్రమీకరించు_క్రమం = SQLITE_SO_ASC;
  }
  p_అంశం->fg.క్రమీకరించు_Flags = (u8)i_క్రమీకరించు_క్రమం;

  ఒకవేళ( eNulls!=SQLITE_SO_UNDEFINED ){
    p_అంశం->fg.bNulls = 1;
    ఒకవేళ( i_క్రమీకరించు_క్రమం!=eNulls ){
      p_అంశం->fg.క్రమీకరించు_Flags |= KEYINFO_క్రమం_BIGNULL;
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
శూన్య sqlite3_సమాసం_List_సమితి_Name(
  Parse *pParse,          /* Parsing context */
  సమాసం_List *pList,        /* List to which to add the span. */
  స్థిరరాశి Token *pName,     /* Name to be added */
  పూర్ణాంకం dequote             /* True to cause the name to be dequoted */
){
  ప్రతిజ్ఞా( pList!=0 || pParse->db->mallocFailed!=0 );
  ప్రతిజ్ఞా( pParse->e_Parse_బహుళకం!=PARSE_బహుళకం_UNMAP || dequote==0 );
  ఒకవేళ( pList ){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం;
    ప్రతిజ్ఞా( pList->n_సమాసం>0 );
    p_అంశం = &pList->a[pList->n_సమాసం-1];
    ప్రతిజ్ఞా( p_అంశం->zEName==0 );
    ప్రతిజ్ఞా( p_అంశం->fg.eEName==ENAME_NAME );
    p_అంశం->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ఒకవేళ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_అంశం->zEName);
      ఒకవేళ( లోపల_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (స్థిరరాశి శూన్య*)p_అంశం->zEName, pName);
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
శూన్య sqlite3_సమాసం_List_సమితి_పరిధి(
  Parse *pParse,          /* Parsing context */
  సమాసం_List *pList,        /* List to which to add the span. */
  స్థిరరాశి అక్షరం *z_ప్రారంభం,     /* Start of the span */
  స్థిరరాశి అక్షరం *z_చివర        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ప్రతిజ్ఞా( pList!=0 || db->mallocFailed!=0 );
  ఒకవేళ( pList ){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం = &pList->a[pList->n_సమాసం-1];
    ప్రతిజ్ఞా( pList->n_సమాసం>0 );
    ఒకవేళ( p_అంశం->zEName==0 ){
      p_అంశం->zEName = sqlite3_Db_పరిధి_Dup(db, z_ప్రారంభం, z_చివర);
      p_అంశం->fg.eEName = ENAME_పరిధి;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
శూన్య sqlite3_సమాసం_List_Check_నిడివి(
  Parse *pParse,
  సమాసం_List *pEList,
  స్థిరరాశి అక్షరం *zObject
){
  పూర్ణాంకం mx = pParse->db->a_హద్దు[SQLITE_హద్దు_స్తంభం];
  testcase( pEList && pEList->n_సమాసం==mx );
  testcase( pEList && pEList->n_సమాసం==mx+1 );
  ఒకవేళ( pEList && pEList->n_సమాసం>mx ){
    sqlite3_Error_సందేశం(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
స్థిర SQLITE_NOINLINE శూన్య సమాసం_List_Delete_NN(sqlite3 *db, సమాసం_List *pList){
  పూర్ణాంకం i = pList->n_సమాసం;
  నిర్మాణం సమాసం_List_అంశం *p_అంశం =  pList->a;
  ప్రతిజ్ఞా( pList->n_సమాసం>0 );
  ప్రతిజ్ఞా( db!=0 );
  చేయి{
    sqlite3_సమాసం_Delete(db, p_అంశం->p_సమాసం);
    ఒకవేళ( p_అంశం->zEName ) sqlite3DbNNFreeNN(db, p_అంశం->zEName);
    p_అంశం++;
  }అయితే( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
శూన్య sqlite3_సమాసం_List_Delete(sqlite3 *db, సమాసం_List *pList){
  ఒకవేళ( pList ) సమాసం_List_Delete_NN(db, pList);
}
శూన్య sqlite3_సమాసం_List_Delete_Generic(sqlite3 *db, శూన్య *pList){
  ఒకవేళ( ALWAYS(pList) ) సమాసం_List_Delete_NN(db, (సమాసం_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_సమాసం_List_Flags(స్థిరరాశి సమాసం_List *pList){
  పూర్ణాంకం i;
  u32 m = 0;
  ప్రతిజ్ఞా( pList!=0 );
  కొరకు(i=0; i<pList->n_సమాసం; i++){
     సమాసం *p_సమాసం = pList->a[i].p_సమాసం;
     ప్రతిజ్ఞా( p_సమాసం!=0 );
     m |= p_సమాసం->flags;
  }
  తిరిగి m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
పూర్ణాంకం sqlite3SelectWalkFail(Walker *pWalker, Select *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  pWalker->e_సంకేతం = 0;
  తిరిగి WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(స్థిరరాశి అక్షరం *z_లోపల){
  ఒకవేళ( sqlite3StrICmp(z_లోపల, "true")==0  ) తిరిగి EP_IsTrue;
  ఒకవేళ( sqlite3StrICmp(z_లోపల, "false")==0 ) తిరిగి EP_IsFalse;
  తిరిగి 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
పూర్ణాంకం sqlite3_సమాసం_Id_గమ్యం_True_False(సమాసం *p_సమాసం){
  u32 v;
  ప్రతిజ్ఞా( p_సమాసం->op==TK_ID || p_సమాసం->op==TK_STRING );
  ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_Quoted|EP_Int_విలువ)
   && (v = sqlite3IsTrueOrFalse(p_సమాసం->u.zToken))!=0
  ){
    p_సమాసం->op = TK_TRUEFALSE;
    సమాసం_సమితి_Property(p_సమాసం, v);
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
పూర్ణాంకం sqlite3_సమాసం_Truth_విలువ(స్థిరరాశి సమాసం *p_సమాసం){
  p_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely((సమాసం*)p_సమాసం);
  ప్రతిజ్ఞా( p_సమాసం->op==TK_TRUEFALSE );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
  ప్రతిజ్ఞా( sqlite3StrICmp(p_సమాసం->u.zToken,"true")==0
       || sqlite3StrICmp(p_సమాసం->u.zToken,"false")==0 );
  తిరిగి p_సమాసం->u.zToken[4]==0;
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
సమాసం *sqlite3_సమాసం_Simplified_And_Or(సమాసం *p_సమాసం){
  ప్రతిజ్ఞా( p_సమాసం!=0 );
  ఒకవేళ( p_సమాసం->op==TK_AND || p_సమాసం->op==TK_OR ){
    సమాసం *p_కుడి = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం->p_కుడి);
    సమాసం *p_ఎడమ = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం->p_ఎడమ);
    ఒకవేళ( సమాసం_Always_True(p_ఎడమ) || సమాసం_Always_False(p_కుడి) ){
      p_సమాసం = p_సమాసం->op==TK_AND ? p_కుడి : p_ఎడమ;
    }లేకపోతే ఒకవేళ( సమాసం_Always_True(p_కుడి) || సమాసం_Always_False(p_ఎడమ) ){
      p_సమాసం = p_సమాసం->op==TK_AND ? p_ఎడమ : p_కుడి;
    }
  }
  తిరిగి p_సమాసం;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
స్థిర పూర్ణాంకం సమాసం_Eval_Rhs_మొదటి(సమాసం *p_సమాసం){
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం->p_ఎడమ, EP_Subquery)
   && !సమాసం_Has_Property(p_సమాసం->p_కుడి, EP_Subquery)
  ){
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి 0;
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
స్థిర పూర్ణాంకం సమాసం_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  సమాసం *p_సమాసం,       /* The comparison expression */
  పూర్ణాంకం *pR1,          /* OUT: Register holding the left operand */
  పూర్ణాంకం *pR2,          /* OUT: Register holding the right operand */
  పూర్ణాంకం *pFree1,       /* OUT: Temp register to free if not zero */
  పూర్ణాంకం *pFree2        /* OUT: Another temp register to free if not zero */
){
  పూర్ణాంకం addrIsNull;
  పూర్ణాంకం r1, r2;
  Vdbe *v = pParse->pVdbe;

  ప్రతిజ్ఞా( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ఒకవేళ( సమాసం_Eval_Rhs_మొదటి(p_సమాసం) && sqlite3_సమాసం_Can_Be_Null(p_సమాసం->p_కుడి) ){
    r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, pFree2);
    addrIsNull = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }లేకపోతే{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, pFree1);
  ఒకవేళ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం->p_కుడి, EP_Subquery)
     && sqlite3_సమాసం_Can_Be_Null(p_సమాసం->p_ఎడమ)
    ){
      addrIsNull = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  తిరిగి addrIsNull;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం సమాసం_కణుపు_Is_Constant_Function(
  Walker *pWalker,
  సమాసం *p_సమాసం
){
  పూర్ణాంకం n;             /* Number of arguments */
  సమాసం_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_TokenOnly)
   || (pList = p_సమాసం->x.pList)==0
  ){;
    n = 0;
  }లేకపోతే{
    n = pList->n_సమాసం;
    sqlite3_Walk_సమాసం_List(pWalker, pList);
    ఒకవేళ( pWalker->e_సంకేతం==0 ) తిరిగి WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_కనుగొను_Function(db, p_సమాసం->u.zToken, n, ENC(db), 0);
  ఒకవేళ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(సమాసం_Has_Property(p_సమాసం, EP_WinFunc))
  ){
    pWalker->e_సంకేతం = 0;
    తిరిగి WRC_Abort;
  }
  తిరిగి WRC_Prune;
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
స్థిర పూర్ణాంకం సమాసం_కణుపు_Is_Constant(Walker *pWalker, సమాసం *p_సమాసం){
  ప్రతిజ్ఞా( pWalker->e_సంకేతం>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ఒకవేళ( pWalker->e_సంకేతం==2 && సమాసం_Has_Property(p_సమాసం, EP_OuterON) ){
    pWalker->e_సంకేతం = 0;
    తిరిగి WRC_Abort;
  }

  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_FUNCTION:
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
        pWalker->e_సంకేతం = 0;  /* Window functions are never constant */
        తిరిగి WRC_Abort;
      }లేకపోతే ఒకవేళ( pWalker->e_సంకేతం>=4 && pWalker->e_సంకేతం<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ఒకవేళ( pWalker->e_సంకేతం==5 ) సమాసం_సమితి_Property(p_సమాసం, EP_FromDDL);
        తిరిగి WRC_Continue;
      }లేకపోతే ఒకవేళ( సమాసం_Has_Property(p_సమాసం,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        తిరిగి WRC_Continue;
      }లేకపోతే ఒకవేళ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        తిరిగి సమాసం_కణుపు_Is_Constant_Function(pWalker, p_సమాసం);
      }లేకపోతే{
        pWalker->e_సంకేతం = 0;  /* Function call is not constant */
        తిరిగి WRC_Abort;
      }
    స్థితి TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ఒకవేళ( sqlite3_సమాసం_Id_గమ్యం_True_False(p_సమాసం) ){
        తిరిగి WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    స్థితి TK_స్తంభం:
    స్థితి TK_AGG_FUNCTION:
    స్థితి TK_AGG_స్తంభం:
      testcase( p_సమాసం->op==TK_ID );
      testcase( p_సమాసం->op==TK_స్తంభం );
      testcase( p_సమాసం->op==TK_AGG_FUNCTION );
      testcase( p_సమాసం->op==TK_AGG_స్తంభం );
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు) && pWalker->e_సంకేతం!=2 ){
        తిరిగి WRC_Continue;
      }
      ఒకవేళ( pWalker->e_సంకేతం==3 && p_సమాసం->i_పట్టిక==pWalker->u.iCur ){
        తిరిగి WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    స్థితి TK_IF_NULL_అడ్డువరుస:
    స్థితి TK_రిజిస్టరు:
    స్థితి TK_DOT:
    స్థితి TK_RAISE:
      testcase( p_సమాసం->op==TK_రిజిస్టరు );
      testcase( p_సమాసం->op==TK_IF_NULL_అడ్డువరుస );
      testcase( p_సమాసం->op==TK_DOT );
      testcase( p_సమాసం->op==TK_RAISE );
      pWalker->e_సంకేతం = 0;
      తిరిగి WRC_Abort;
    స్థితి TK_IS:
    స్థితి TK_ISNOT:
      ఒకవేళ( pWalker->e_సంకేతం==6 ){
        pWalker->e_సంకేతం = 0;
        తిరిగి WRC_Abort;
      }
      తిరిగి WRC_Continue;
    స్థితి TK_VARIABLE:
      ఒకవేళ( pWalker->e_సంకేతం==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_సమాసం->op = TK_NULL;
      }లేకపోతే ఒకవేళ( pWalker->e_సంకేతం==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_సంకేతం = 0;
        తిరిగి WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    అప్రమేయ:
      testcase( p_సమాసం->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_సమాసం->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      తిరిగి WRC_Continue;
  }
}
స్థిర పూర్ణాంకం సమాసం_Is_Const(Parse *pParse, సమాసం *p, పూర్ణాంకం initFlag){
  Walker w;
  w.e_సంకేతం = initFlag;
  w.pParse = pParse;
  w.x_సమాసం_Callback = సమాసం_కణుపు_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#నిర్వచితమైతే SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#అయితే_ముగింపు
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం;
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
పూర్ణాంకం sqlite3_సమాసం_Is_Constant(Parse *pParse, సమాసం *p){
  తిరిగి సమాసం_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
పూర్ణాంకం sqlite3_సమాసం_List_Is_Constant(Parse *pParse, సమాసం_List *pList, పూర్ణాంకం bNoIs){
  స్థిరరాశి పూర్ణాంకం iInit = bNoIs ? 6 : 1;
  పూర్ణాంకం ii;
  కొరకు(ii=0; ii<pList->n_సమాసం; ii++){
    ఒకవేళ( 0==సమాసం_Is_Const(pParse, pList->a[ii].p_సమాసం, iInit) ) తిరిగి 0;
  }
  తిరిగి 1;
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
స్థిర పూర్ణాంకం sqlite3_సమాసం_Is_Constant_Not_Join(Parse *pParse, సమాసం *p){
  తిరిగి సమాసం_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
స్థిర పూర్ణాంకం సమాసం_Select_Walk_పట్టిక_Constant(Walker *pWalker, Select *pSelect){
  ప్రతిజ్ఞా( pSelect!=0 );
  ప్రతిజ్ఞా( pWalker->e_సంకేతం==3 || pWalker->e_సంకేతం==0 );
  ఒకవేళ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_సంకేతం = 0;
    తిరిగి WRC_Abort;
  }
  తిరిగి WRC_Prune;
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
పూర్ణాంకం sqlite3_సమాసం_Is_పట్టిక_Constant(సమాసం *p, పూర్ణాంకం iCur, పూర్ణాంకం bAllowSubq){
  Walker w;
  w.e_సంకేతం = 3;
  w.pParse = 0;
  w.x_సమాసం_Callback = సమాసం_కణుపు_Is_Constant;
  ఒకవేళ( bAllowSubq ){
    w.xSelectCallback = సమాసం_Select_Walk_పట్టిక_Constant;
  }లేకపోతే{
    w.xSelectCallback = sqlite3SelectWalkFail;
#నిర్వచితమైతే SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#అయితే_ముగింపు
  }
  w.u.iCur = iCur;
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం;
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
పూర్ణాంకం sqlite3_సమాసం_Is_Single_పట్టిక_Constraint(
  సమాసం *p_సమాసం,                 /* The constraint */
  స్థిరరాశి SrcList *pSrcList,     /* Complete FROM clause */
  పూర్ణాంకం iSrc,                    /* Which element of pSrcList to use */
  పూర్ణాంకం bAllowSubq               /* Allow non-correlated subqueries */
){
  స్థిరరాశి Src_అంశం *pSrc = &pSrcList->a[iSrc];
  ఒకవేళ( pSrc->fg.jointype & JT_LTORJ ){
    తిరిగి 0;  /* rule (3) */
  }
  ఒకవేళ( pSrc->fg.jointype & JT_ఎడమ ){
    ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_OuterON) ) తిరిగి 0;   /* rule (4a) */
    ఒకవేళ( p_సమాసం->w.iJoin!=pSrc->iCursor ) తిరిగి 0;         /* rule (4b) */
  }లేకపోతే{
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON) ) తిరిగి 0;    /* rule (5) */
  }
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    పూర్ణాంకం jj;
    కొరకు(jj=0; jj<iSrc; jj++){
      ఒకవేళ( p_సమాసం->w.iJoin==pSrcList->a[jj].iCursor ){
        ఒకవేళ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          తిరిగి 0;  /* restriction (6) */
        }
        ఆపు;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  తిరిగి sqlite3_సమాసం_Is_పట్టిక_Constant(p_సమాసం, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
స్థిర పూర్ణాంకం సమాసం_కణుపు_Is_Constant_Or_సమూహం_By(Walker *pWalker, సమాసం *p_సమాసం){
  సమాసం_List *p_సమూహం_By = pWalker->u.p_సమూహం_By;
  పూర్ణాంకం i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  కొరకు(i=0; i<p_సమూహం_By->n_సమాసం; i++){
    సమాసం *p = p_సమూహం_By->a[i].p_సమాసం;
    ఒకవేళ( sqlite3_సమాసం_Compare(0, p_సమాసం, p, -1)<2 ){
      CollSeq *pColl = sqlite3_సమాసం_NNColl_Seq(pWalker->pParse, p);
      ఒకవేళ( sqlite3_Is_ద్విమాన(pColl) ){
        తిరిగి WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
    pWalker->e_సంకేతం = 0;
    తిరిగి WRC_Abort;
  }

  తిరిగి సమాసం_కణుపు_Is_Constant(pWalker, p_సమాసం);
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
పూర్ణాంకం sqlite3_సమాసం_Is_Constant_Or_సమూహం_By(Parse *pParse, సమాసం *p, సమాసం_List *p_సమూహం_By){
  Walker w;
  w.e_సంకేతం = 1;
  w.x_సమాసం_Callback = సమాసం_కణుపు_Is_Constant_Or_సమూహం_By;
  w.xSelectCallback = 0;
  w.u.p_సమూహం_By = p_సమూహం_By;
  w.pParse = pParse;
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం;
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
పూర్ణాంకం sqlite3_సమాసం_Is_Constant_Or_Function(సమాసం *p, u8 isInit){
  ప్రతిజ్ఞా( isInit==0 || isInit==1 );
  తిరిగి సమాసం_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
పూర్ణాంకం sqlite3_సమాసం_కలిగియుంది_Subquery(సమాసం *p){
  Walker w;
  w.e_సంకేతం = 1;
  w.x_సమాసం_Callback = sqlite3_సమాసం_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#నిర్వచితమైతే SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#అయితే_ముగింపు
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం==0;
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
పూర్ణాంకం sqlite3_సమాసం_Is_Integer(స్థిరరాశి సమాసం *p, పూర్ణాంకం *p_విలువ, Parse *pParse, పూర్ణాంకం bRSI){
  పూర్ణాంకం iSign = 1;   /* Either +1 or -1. */
  అయితే( 1/*exit-by-break*/ ){
    ఒకవేళ( NEVER(p==0) ) తిరిగి 0;
    ఒకవేళ( సమాసం_Use_UValue(p) ){
      ప్రతిజ్ఞా( p->op==TK_INTEGER );
      *p_విలువ = p->u.i_విలువ*iSign;
      ఒకవేళ( bRSI && pParse==0 && p->op2 ){
        తిరిగి 0;
      }
      తిరిగి 1;
    }
    ఒకవేళ( p->op==TK_UPLUS ){
      p = p->p_ఎడమ;
      pParse = 0;
      కొనసాగు;
    }
    ఒకవేళ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ఎడమ;
      pParse = 0;
      కొనసాగు;
    }
    ఒకవేళ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_విలువ *pVal; /* The variable */
      పూర్ణాంకం rc = 0;          /* 1 if successful, 0 if failed */
      ప్రతిజ్ఞా( iSign==1 );
      ఒకవేళ( NEVER(pParse->pVdbe==0) ) ఆపు;
      ఒకవేళ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ఆపు;
      pVal = sqlite3_Vdbe_Get_Bound_విలువ(pParse->pReprepare, p->i_స్తంభం,
                                      SQLITE_AFF_BLOB);
      ఒకవేళ( pVal ){
        i64 vv;
        ఒకవేళ( sqlite3_విలువ_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_విలువ_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_విలువ = (పూర్ణాంకం)vv;
          rc = 1;       
        }
        sqlite3_విలువ_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_స్తంభం, bRSI);
      తిరిగి rc;
    }
    ఆపు;
  }
  తిరిగి 0;
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
పూర్ణాంకం sqlite3_సమాసం_Can_Be_Null(స్థిరరాశి సమాసం *p){
  u8 op;
  ప్రతిజ్ఞా( p!=0 );
  అయితే( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ఎడమ;
    ప్రతిజ్ఞా( p!=0 );
  }
  op = p->op;
  ఒకవేళ( op==TK_రిజిస్టరు ) op = p->op2;
  ఎంపిక( op ){
    స్థితి TK_INTEGER:
    స్థితి TK_STRING:
    స్థితి TK_FLOAT:
    స్థితి TK_BLOB:
      తిరిగి 0;
    స్థితి TK_స్తంభం:
      ప్రతిజ్ఞా( సమాసం_Use_YTab(p) );
      తిరిగి సమాసం_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
          || (p->i_స్తంభం==XN_ROWID && IsView(p->y.pTab))
#అయితే_ముగింపు
          || (p->i_స్తంభం>=0
              && p->y.pTab->a_నిలువు!=0 /* Possible due to prior error */
              && ALWAYS(p->i_స్తంభం<p->y.pTab->n_నిలువు)
              && p->y.pTab->a_నిలువు[p->i_స్తంభం].notNull==0);
    అప్రమేయ:
      తిరిగి 1;
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
పూర్ణాంకం sqlite3_సమాసం_Needs_No_Affinity_Change(స్థిరరాశి సమాసం *p, అక్షరం aff){
  u8 op;
  పూర్ణాంకం unaryMinus = 0;
  ఒకవేళ( aff==SQLITE_AFF_BLOB ) తిరిగి 1;
  అయితే( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ఒకవేళ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ఎడమ;
  }
  op = p->op;
  ఒకవేళ( op==TK_రిజిస్టరు ) op = p->op2;
  ఎంపిక( op ){
    స్థితి TK_INTEGER: {
      తిరిగి aff>=SQLITE_AFF_NUMERIC;
    }
    స్థితి TK_FLOAT: {
      తిరిగి aff>=SQLITE_AFF_NUMERIC;
    }
    స్థితి TK_STRING: {
      తిరిగి !unaryMinus && aff==SQLITE_AFF_పాఠ్యం;
    }
    స్థితి TK_BLOB: {
      తిరిగి !unaryMinus;
    }
    స్థితి TK_స్తంభం: {
      ప్రతిజ్ఞా( p->i_పట్టిక>=0 );  /* p cannot be part of a CHECK constraint */
      తిరిగి aff>=SQLITE_AFF_NUMERIC && p->i_స్తంభం<0;
    }
    అప్రమేయ: {
      తిరిగి 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
పూర్ణాంకం sqlite3IsRowid(స్థిరరాశి అక్షరం *z){
  ఒకవేళ( sqlite3StrICmp(z, "_ROWID_")==0 ) తిరిగి 1;
  ఒకవేళ( sqlite3StrICmp(z, "ROWID")==0 ) తిరిగి 1;
  ఒకవేళ( sqlite3StrICmp(z, "OID")==0 ) తిరిగి 1;
  తిరిగి 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
స్థిరరాశి అక్షరం *sqlite3RowidAlias(పట్టిక *pTab){
  స్థిరరాశి అక్షరం *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  పూర్ణాంకం ii;
  ప్రతిజ్ఞా( VisibleRowid(pTab) );
  కొరకు(ii=0; ii<Array_కొలత(azOpt); ii++){
    ఒకవేళ( sqlite3_స్తంభం_సూచీ(pTab, azOpt[ii])<0 ) తిరిగి azOpt[ii];
  }
  తిరిగి 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
స్థిర Select *is_Candidate_For_లోపల_Opt(స్థిరరాశి సమాసం *pX){
  Select *p;
  SrcList *pSrc;
  సమాసం_List *pEList;
  పట్టిక *pTab;
  పూర్ణాంకం i;
  ఒకవేళ( !సమాసం_Use_XSelect(pX) ) తిరిగి 0;                 /* Not a subquery */
  ఒకవేళ( సమాసం_Has_Property(pX, EP_VarSelect)  ) తిరిగి 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ఒకవేళ( p->p_పూర్వ ) తిరిగి 0;              /* Not a compound SELECT */
  ఒకవేళ( p->selFlags & SF_Aggregate ){
    తిరిగి 0; /* No GROUP BY keyword or aggregate functions */
  }
  ప్రతిజ్ఞా( p->p_సమూహం_By==0 );              /* Has no GROUP BY clause */
  ఒకవేళ( p->p_హద్దు ) తిరిగి 0;              /* Has no LIMIT clause */
  ఒకవేళ( p->pWhere ) తిరిగి 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ప్రతిజ్ఞా( pSrc!=0 );
  ఒకవేళ( pSrc->nSrc!=1 ) తిరిగి 0;          /* Single term in FROM clause */
  ఒకవేళ( pSrc->a[0].fg.isSubquery) తిరిగి 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( !IsView(pTab)  );              /* FROM clause is not a view */
  ఒకవేళ( IsVirtual(pTab) ) తిరిగి 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ప్రతిజ్ఞా( pEList!=0 );
  /* All SELECT results must be columns. */
  కొరకు(i=0; i<pEList->n_సమాసం; i++){
    సమాసం *pRes = pEList->a[i].p_సమాసం;
    ఒకవేళ( pRes->op!=TK_స్తంభం ) తిరిగి 0;
    ప్రతిజ్ఞా( pRes->i_పట్టిక==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  తిరిగి p;
}
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
స్థిర శూన్య sqlite3_సమితి_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  పూర్ణాంకం iCur,          /* Cursor for the index */
  పూర్ణాంకం regHasNull,    /* Register in which to store hasNull flag */
  పూర్ణాంకం e_క్రమీకరించు_క్రమం     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  పూర్ణాంకం addr1;
  పూర్ణాంకం op;
  sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regHasNull);
  ఒకవేళ( e_క్రమీకరించు_క్రమం==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }లేకపోతే{
    op = OP_చివరి;
  }
  addr1 = sqlite3_Vdbe_కూడు_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_చివరి?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
}
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
స్థిర పూర్ణాంకం sqlite3_లోపల_Rhs_Is_Constant(Parse *pParse, సమాసం *p_లోపల){
  సమాసం *pLHS;
  పూర్ణాంకం res;
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_లోపల, EP_xIsSelect) );
  pLHS = p_లోపల->p_ఎడమ;
  p_లోపల->p_ఎడమ = 0;
  res = sqlite3_సమాసం_Is_Constant(pParse, p_లోపల);
  p_లోపల->p_ఎడమ = pLHS;
  తిరిగి res;
}
#అయితే_ముగింపు

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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
పూర్ణాంకం sqlite3_కనుగొను_లోపల_సూచీ(
  Parse *pParse,             /* Parsing context */
  సమాసం *pX,                  /* The IN expression */
  u32 లోపల_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  పూర్ణాంకం *prRhsHasNull,         /* Register holding NULL status.  See notes */
  పూర్ణాంకం *aiMap,                /* Mapping from Index fields to RHS fields */
  పూర్ణాంకం *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  పూర్ణాంకం eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  పూర్ణాంకం iTab;                             /* Cursor of the RHS table */
  పూర్ణాంకం mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ప్రతిజ్ఞా( pX->op==TK_లోపల );
  mustBeUnique = (లోపల_Flags & లోపల_సూచీ_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ఒకవేళ( prRhsHasNull && సమాసం_Use_XSelect(pX) ){
    పూర్ణాంకం i;
    సమాసం_List *pEList = pX->x.pSelect->pEList;
    కొరకు(i=0; i<pEList->n_సమాసం; i++){
      ఒకవేళ( sqlite3_సమాసం_Can_Be_Null(pEList->a[i].p_సమాసం) ) ఆపు;
    }
    ఒకవేళ( i==pEList->n_సమాసం ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ఒకవేళ( pParse->nErr==0 && (p = is_Candidate_For_లోపల_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    పట్టిక *pTab;                           /* Table <table>. */
    పూర్ణాంకం iDb;                               /* Database idx for pTab */
    సమాసం_List *pEList = p->pEList;
    పూర్ణాంకం n_సమాసం = pEList->n_సమాసం;

    ప్రతిజ్ఞా( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ప్రతిజ్ఞా( p->pEList->a[0].p_సమాసం!=0 ); /* Because of isCandidateForInOpt(p) */
    ప్రతిజ్ఞా( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
    ప్రతిజ్ఞా( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
    sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ప్రతిజ్ఞా(v);  /* sqlite3GetVdbe() has always been previously called */
    ఒకవేళ( n_సమాసం==1 && pEList->a[0].p_సమాసం->i_స్తంభం<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      పూర్ణాంకం iAddr = sqlite3_Vdbe_కూడు_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_పట్టిక(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = లోపల_సూచీ_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_ఇక్కడ(v, iAddr);
    }లేకపోతే{
      సూచీ *pIdx;                         /* Iterator variable */
      పూర్ణాంకం affinity_ok = 1;
      పూర్ణాంకం i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      కొరకు(i=0; i<n_సమాసం && affinity_ok; i++){
        సమాసం *pLhs = sqlite3_సదిశ_క్షేత్రం_Subexpr(pX->p_ఎడమ, i);
        పూర్ణాంకం i_నిలువు = pEList->a[i].p_సమాసం->i_స్తంభం;
        అక్షరం idxaff = sqlite3_పట్టిక_స్తంభం_Affinity(pTab,i_నిలువు); /* RHS table */
        అక్షరం cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_పాఠ్యం );
        ఎంపిక( cmpaff ){
          స్థితి SQLITE_AFF_BLOB:
            ఆపు;
          స్థితి SQLITE_AFF_పాఠ్యం:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ప్రతిజ్ఞా( idxaff==SQLITE_AFF_పాఠ్యం );
            ఆపు;
          అప్రమేయ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ఒకవేళ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        కొరకు(pIdx=pTab->p_సూచీ; pIdx && eType==0; pIdx=pIdx->p_తదుపరి){
          Bitmask నిలువు_వాడిన;      /* Columns of the index used */
          Bitmask m_నిలువు;         /* Mask for the current column */
          ఒకవేళ( pIdx->n_స్తంభం<n_సమాసం ) కొనసాగు;
          ఒకవేళ( pIdx->pPartIdxWhere!=0 ) కొనసాగు;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_స్తంభం==BMS-2 );
          testcase( pIdx->n_స్తంభం==BMS-1 );
          ఒకవేళ( pIdx->n_స్తంభం>=BMS-1 ) కొనసాగు;
          ఒకవేళ( mustBeUnique ){
            ఒకవేళ( pIdx->n_కీలకం_నిలువు>n_సమాసం
             ||(pIdx->n_స్తంభం>n_సమాసం && !Is_Unique_సూచీ(pIdx))
            ){
              కొనసాగు;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          నిలువు_వాడిన = 0;   /* Columns of index used so far */
          కొరకు(i=0; i<n_సమాసం; i++){
            సమాసం *pLhs = sqlite3_సదిశ_క్షేత్రం_Subexpr(pX->p_ఎడమ, i);
            సమాసం *pRhs = pEList->a[i].p_సమాసం;
            CollSeq *pReq = sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, pLhs, pRhs);
            పూర్ణాంకం j;
 
            కొరకు(j=0; j<n_సమాసం; j++){
              ఒకవేళ( pIdx->ai_స్తంభం[j]!=pRhs->i_స్తంభం ) కొనసాగు;
              ప్రతిజ్ఞా( pIdx->azColl[j] );
              ఒకవేళ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                కొనసాగు;
              }
              ఆపు;
            }
            ఒకవేళ( j==n_సమాసం ) ఆపు;
            m_నిలువు = MASKBIT(j);
            ఒకవేళ( m_నిలువు & నిలువు_వాడిన ) ఆపు; /* Each column used only once */
            నిలువు_వాడిన |= m_నిలువు;
            ఒకవేళ( aiMap ) aiMap[i] = j;
          }
 
          ప్రతిజ్ఞా( n_సమాసం>0 && n_సమాసం<BMS );
          ప్రతిజ్ఞా( i==n_సమాసం || నిలువు_వాడిన!=(MASKBIT(n_సమాసం)-1) );
          ఒకవేళ( నిలువు_వాడిన==(MASKBIT(n_సమాసం)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            పూర్ణాంకం iAddr = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_కూడు_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ప్రతిజ్ఞా( లోపల_సూచీ_సూచీ_DESC == లోపల_సూచీ_సూచీ_ASC+1 );
            eType = లోపల_సూచీ_సూచీ_ASC + pIdx->a_క్రమీకరించు_క్రమం[0];
 
            ఒకవేళ( prRhsHasNull ){
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_వాడిన_ముసుగు
              u64 ముసుగు = (1<<n_సమాసం)-1;
              sqlite3_Vdbe_కూడు_Op3(v, OP_Columns_వాడిన,
                                iTab, LOWER32(ముసుగు), UPPER32(ముసుగు));
#అయితే_ముగింపు
              *prRhsHasNull = ++pParse->nMem;
              ఒకవేళ( n_సమాసం==1 ){
                sqlite3_సమితి_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_క్రమీకరించు_క్రమం[0]);
              }
            }
            sqlite3_Vdbe_Jump_ఇక్కడ(v, iAddr);
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
  ఒకవేళ( eType==0
   && (లోపల_Flags & లోపల_సూచీ_NOOP_OK)
   && సమాసం_Use_XList(pX)
   && (!sqlite3_లోపల_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_సమాసం<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = లోపల_సూచీ_NOOP;
  }

  ఒకవేళ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    పూర్ణాంకం rMayHaveNull = 0;
    పూర్ణాంకం bloomOk = (లోపల_Flags & లోపల_సూచీ_MEMBERSHIP)!=0;
    eType = లోపల_సూచీ_EPH;
    ఒకవేళ( లోపల_Flags & లోపల_సూచీ_LOOP ){
      pParse->nQueryLoop = 0;
    }లేకపోతే ఒకవేళ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ప్రతిజ్ఞా( pX->op==TK_లోపల );
    ఒకవేళ( !bloomOk
     && సమాసం_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_లోపల)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_సంకేతం_Rhs_యొక్క_లోపల(pParse, pX, iTab, bloomOk);
    ఒకవేళ( rMayHaveNull ){
      sqlite3_సమితి_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ఒకవేళ( aiMap && eType!=లోపల_సూచీ_సూచీ_ASC && eType!=లోపల_సూచీ_సూచీ_DESC ){
    పూర్ణాంకం i, n;
    n = sqlite3_సమాసం_సదిశ_కొలత(pX->p_ఎడమ);
    కొరకు(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  తిరిగి eType;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
స్థిర అక్షరం *సమాసం_INAffinity(Parse *pParse, స్థిరరాశి సమాసం *p_సమాసం){
  సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
  పూర్ణాంకం nVal = sqlite3_సమాసం_సదిశ_కొలత(p_ఎడమ);
  Select *pSelect = సమాసం_Use_XSelect(p_సమాసం) ? p_సమాసం->x.pSelect : 0;
  అక్షరం *zRet;

  ప్రతిజ్ఞా( p_సమాసం->op==TK_లోపల );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ఒకవేళ( zRet ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<nVal; i++){
      సమాసం *pA = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_ఎడమ, i);
      అక్షరం a = sqlite3_సమాసం_Affinity(pA);
      ఒకవేళ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_సమాసం, a);
      }లేకపోతే{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  తిరిగి zRet;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
శూన్య sqlite3SubselectError(Parse *pParse, పూర్ణాంకం nActual, పూర్ణాంకం nExpect){
  ఒకవేళ( pParse->nErr==0 ){
    స్థిరరాశి అక్షరం *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_సందేశం(pParse, zFmt, nActual, nExpect);
  }
}
#అయితే_ముగింపు

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
శూన్య sqlite3_సదిశ_Error_సందేశం(Parse *pParse, సమాసం *p_సమాసం){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
  ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
    sqlite3SubselectError(pParse, p_సమాసం->x.pSelect->pEList->n_సమాసం, 1);
  }లేకపోతే
#అయితే_ముగింపు
  {
    sqlite3_Error_సందేశం(pParse, "row value misused");
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
స్థిర పూర్ణాంకం కనుగొను_Compatible_లోపల_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  సమాసం *p_సమాసం,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_చివర;
  SubrtnSig *pSig;
  Vdbe *v;

  ఒకవేళ( pNewSig==0 ) తిరిగి 0;
  ఒకవేళ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) తిరిగి 0;
  ప్రతిజ్ఞా( p_సమాసం->op==TK_లోపల );
  ప్రతిజ్ఞా( !సమాసం_Use_YSub(p_సమాసం) );
  ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
  ప్రతిజ్ఞా( p_సమాసం->x.pSelect!=0 );
  ప్రతిజ్ఞా( (p_సమాసం->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_చివర = sqlite3_Vdbe_Get_చివరి_Op(v);
  కొరకు(; pOp<p_చివర; pOp++){
    ఒకవేళ( pOp->p4type!=P4_SUBRTNSIG ) కొనసాగు;
    ప్రతిజ్ఞా( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ప్రతిజ్ఞా( pSig!=0 );
    ఒకవేళ( !pSig->bComplete ) కొనసాగు;
    ఒకవేళ( pNewSig->selId!=pSig->selId ) కొనసాగు;
    ఒకవేళ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) కొనసాగు;
    p_సమాసం->y.sub.iAddr = pSig->iAddr;
    p_సమాసం->y.sub.regReturn = pSig->regReturn;
    p_సమాసం->i_పట్టిక = pSig->i_పట్టిక;
    సమాసం_సమితి_Property(p_సమాసం, EP_Subrtn);
    తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
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
శూన్య sqlite3_సంకేతం_Rhs_యొక్క_లోపల(
  Parse *pParse,          /* Parsing context */
  సమాసం *p_సమాసం,            /* The IN operator */
  పూర్ణాంకం iTab,               /* Use this cursor number */
  పూర్ణాంకం allowBloom          /* True to allow the use of a Bloom filter */
){
  పూర్ణాంకం addrOnce = 0;           /* Address of the OP_Once instruction at top */
  పూర్ణాంకం addr;                   /* Address of OP_OpenEphemeral instruction */
  సమాసం *p_ఎడమ;                /* the LHS of the IN operator */
  కీలకం_Info *p_కీలకం_Info = 0;      /* Key information */
  పూర్ణాంకం nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );

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
  ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ప్రతిజ్ఞా( !సమాసం_Use_XSelect(p_సమాసం) || p_సమాసం->x.pSelect!=0 );
    ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) && (p_సమాసం->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, పరిమాణం(pSig[0]));
      ఒకవేళ( pSig ){
        pSig->selId = p_సమాసం->x.pSelect->selId;
        pSig->zAff = సమాసం_INAffinity(pParse, p_సమాసం);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subrtn) 
     || కనుగొను_Compatible_లోపల_Rhs_Subrtn(pParse, p_సమాసం, pSig)
    ){
      addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
      ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_సమాసం->x.pSelect->selId));
      }
      ప్రతిజ్ఞా( సమాసం_Use_YSub(p_సమాసం) );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, p_సమాసం->y.sub.regReturn,
                        p_సమాసం->y.sub.iAddr);
      ప్రతిజ్ఞా( iTab!=p_సమాసం->i_పట్టిక );
      sqlite3_Vdbe_కూడు_Op2(v, OP_OpenDup, iTab, p_సమాసం->i_పట్టిక);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrOnce);
      ఒకవేళ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      తిరిగి;
    }

    /* Begin coding the subroutine */
    ప్రతిజ్ఞా( !సమాసం_Use_YWin(p_సమాసం) );
    సమాసం_సమితి_Property(p_సమాసం, EP_Subrtn);
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_TokenOnly|EP_Reduced) );
    p_సమాసం->y.sub.regReturn = ++pParse->nMem;
    p_సమాసం->y.sub.iAddr =
      sqlite3_Vdbe_కూడు_Op2(v, OP_BeginSubrtn, 0, p_సమాసం->y.sub.regReturn) + 1;
    ఒకవేళ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_సమాసం->y.sub.iAddr;
      pSig->regReturn = p_సమాసం->y.sub.regReturn;
      pSig->i_పట్టిక = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (స్థిరరాశి అక్షరం*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ఎడమ = p_సమాసం->p_ఎడమ;
  nVal = sqlite3_సమాసం_సదిశ_కొలత(p_ఎడమ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_సమాసం->i_పట్టిక = iTab;
  addr = sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, p_సమాసం->i_పట్టిక, nVal);
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
  ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
    VdbeComment((v, "Result of SELECT %u", p_సమాసం->x.pSelect->selId));
  }లేకపోతే{
    VdbeComment((v, "RHS of IN operator"));
  }
#అయితే_ముగింపు
  p_కీలకం_Info = sqlite3_కీలకం_Info_Alloc(pParse->db, nVal, 1);
  ప్రతిజ్ఞా( p_కీలకం_Info!=0 || pParse->nErr );
  ఒకవేళ( p_కీలకం_Info==0 ) తిరిగి;

  ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_సమాసం->x.pSelect;
    సమాసం_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    ఒకవేళ( ALWAYS(pEList->n_సమాసం==nVal) ){
      Select *pCopy;
      SelectDest dest;
      పూర్ణాంకం i;
      పూర్ణాంకం rc;
      పూర్ణాంకం addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_సమితి, iTab);
      dest.zAffSdst = సమాసం_INAffinity(pParse, p_సమాసం);
      pSelect->i_హద్దు = 0;
      ప్రతిజ్ఞా( pEList!=0 );
      ప్రతిజ్ఞా( pEList->n_సమాసం>0 );
      ప్రతిజ్ఞా( sqlite3_కీలకం_Info_Is_Writeable(p_కీలకం_Info) );
      కొరకు(i=0; i<nVal; i++){
        సమాసం *p = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_ఎడమ, i);
        CollSeq *pColl;
        p_కీలకం_Info->aColl[i] = pColl = sqlite3_ద్విమాన_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_సమాసం
        );
        ఒకవేళ( !sqlite3_Is_ద్విమాన(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ఒకవేళ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        పూర్ణాంకం regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_కూడు_Op2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (శూన్య *)p_కీలకం_Info, P4_KEYINFO);
        p_కీలకం_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      ఒకవేళ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ఒకవేళ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ఒకవేళ( rc ){
        sqlite3_కీలకం_Info_Unref(p_కీలకం_Info);
        తిరిగి;
      }
    }
  }లేకపోతే ఒకవేళ( ALWAYS(p_సమాసం->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    అక్షరం affinity;            /* Affinity of the LHS of the IN */
    పూర్ణాంకం i;
    సమాసం_List *pList = p_సమాసం->x.pList;
    నిర్మాణం సమాసం_List_అంశం *p_అంశం;
    పూర్ణాంకం r1, r2;
    affinity = sqlite3_సమాసం_Affinity(p_ఎడమ);
    ఒకవేళ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }లేకపోతే ఒకవేళ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ప్రతిజ్ఞా( p_కీలకం_Info!=0 );
    ప్రతిజ్ఞా( sqlite3_కీలకం_Info_Is_Writeable(p_కీలకం_Info) );
    p_కీలకం_Info->aColl[0] = sqlite3_సమాసం_Coll_Seq(pParse, p_సమాసం->p_ఎడమ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    r2 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    కొరకు(i=pList->n_సమాసం, p_అంశం=pList->a; i>0; i--, p_అంశం++){
      సమాసం *pE2 = p_అంశం->p_సమాసం;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ఒకవేళ( addrOnce && !sqlite3_సమాసం_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_గమ్యం_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_గమ్యం_Noop(v, addrOnce);
        సమాసం_Clear_Property(p_సమాసం, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_సమాసం_సంకేతం(pParse, pE2, r1);
      sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iTab, r2, r1, 1);
    }
    sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, r2);
  }
  ఒకవేళ( pSig ) pSig->bComplete = 1;
  ఒకవేళ( p_కీలకం_Info ){
    sqlite3VdbeChangeP4(v, addr, (శూన్య *)p_కీలకం_Info, P4_KEYINFO);
  }
  ఒకవేళ( addrOnce ){
    sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iTab);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addrOnce);
    /* Subroutine return */
    ప్రతిజ్ఞా( సమాసం_Use_YSub(p_సమాసం) );
    ప్రతిజ్ఞా( sqlite3VdbeGetOp(v,p_సమాసం->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3_Vdbe_కూడు_Op3(v, OP_Return, p_సమాసం->y.sub.regReturn,
                      p_సమాసం->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
పూర్ణాంకం sqlite3_సంకేతం_Subselect(Parse *pParse, సమాసం *p_సమాసం){
  పూర్ణాంకం addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  పూర్ణాంకం rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  పూర్ణాంకం nReg;                   /* Registers to allocate */
  సమాసం *p_హద్దు;               /* New limit expression */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం addrExplain;            /* Address of OP_Explain instruction */
#అయితే_ముగింపు

  Vdbe *v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  ఒకవేళ( pParse->nErr ) తిరిగి 0;
  testcase( p_సమాసం->op==TK_EXISTS );
  testcase( p_సమాసం->op==TK_SELECT );
  ప్రతిజ్ఞా( p_సమాసం->op==TK_EXISTS || p_సమాసం->op==TK_SELECT );
  ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
  pSel = p_సమాసం->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ప్రతిజ్ఞా( సమాసం_Use_YSub(p_సమాసం) );
    sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, p_సమాసం->y.sub.regReturn,
                      p_సమాసం->y.sub.iAddr);
    తిరిగి p_సమాసం->i_పట్టిక;
  }

  /* Begin coding the subroutine */
  ప్రతిజ్ఞా( !సమాసం_Use_YWin(p_సమాసం) );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Reduced|EP_TokenOnly) );
  సమాసం_సమితి_Property(p_సమాసం, EP_Subrtn);
  p_సమాసం->y.sub.regReturn = ++pParse->nMem;
  p_సమాసం->y.sub.iAddr =
    sqlite3_Vdbe_కూడు_Op2(v, OP_BeginSubrtn, 0, p_సమాసం->y.sub.regReturn) + 1;

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
  ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_VarSelect) ){
    addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
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
  sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_సమాసం->op==TK_SELECT ? pSel->pEList->n_సమాసం : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ఒకవేళ( p_సమాసం->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ఒకవేళ( (pSel->selFlags&SF_Distinct) && pSel->p_హద్దు && pSel->p_హద్దు->p_కుడి ){
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
    }లేకపోతే{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }లేకపోతే{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ఒకవేళ( pSel->p_హద్దు ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    సమాసం *p_ఎడమ = pSel->p_హద్దు->p_ఎడమ;
    ఒకవేళ( సమాసం_Has_Property(p_ఎడమ, EP_Int_విలువ)==0
     || (p_ఎడమ->u.i_విలువ!=1 && p_ఎడమ->u.i_విలువ!=0)
    ){
      sqlite3 *db = pParse->db;
      p_హద్దు = sqlite3_సమాసం_Int32(db, 0);
      ఒకవేళ( p_హద్దు ){
        p_హద్దు->aff_సమాసం = SQLITE_AFF_NUMERIC;
        p_హద్దు = sqlite3PExpr(pParse, TK_NE,
            sqlite3_సమాసం_Dup(db, p_ఎడమ, 0), p_హద్దు);
      }
      sqlite3_సమాసం_Deferred_Delete(pParse, p_ఎడమ);
      pSel->p_హద్దు->p_ఎడమ = p_హద్దు;
    }
  }లేకపోతే{
    /* If there is no pre-existing limit add a limit of 1 */
    p_హద్దు = sqlite3_సమాసం_Int32(pParse->db, 1);
    pSel->p_హద్దు = sqlite3PExpr(pParse, TK_హద్దు, p_హద్దు, 0);
  }
  pSel->i_హద్దు = 0;
  ఒకవేళ( sqlite3Select(pParse, pSel, &dest) ){
    p_సమాసం->op2 = p_సమాసం->op;
    p_సమాసం->op = TK_ERROR;
    తిరిగి 0;
  }
  p_సమాసం->i_పట్టిక = rReg = dest.iSDParm;
  సమాసం_సమితి_VVAProperty(p_సమాసం, EP_NoReduce);
  ఒకవేళ( addrOnce ){
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addrOnce);
  }
  sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, addrExplain, -1);

  ఒకవేళ( dest.eDest==SRT_Mem 
   && sqlite3_సమాసం_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_సమాసం) 
  ){
    sqlite3_Vdbe_కూడు_Op1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ప్రతిజ్ఞా( సమాసం_Use_YSub(p_సమాసం) );
  ప్రతిజ్ఞా( sqlite3VdbeGetOp(v,p_సమాసం->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3_Vdbe_కూడు_Op3(v, OP_Return, p_సమాసం->y.sub.regReturn,
                    p_సమాసం->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);
  తిరిగి rReg;
}
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
పూర్ణాంకం sqlite3_సమాసం_Check_లోపల(Parse *pParse, సమాసం *p_లోపల){
  పూర్ణాంకం n_సదిశ = sqlite3_సమాసం_సదిశ_కొలత(p_లోపల->p_ఎడమ);
  ఒకవేళ( సమాసం_Use_XSelect(p_లోపల) && !pParse->db->mallocFailed ){
    ఒకవేళ( n_సదిశ!=p_లోపల->x.pSelect->pEList->n_సమాసం ){
      sqlite3SubselectError(pParse, p_లోపల->x.pSelect->pEList->n_సమాసం, n_సదిశ);
      తిరిగి 1;
    }
  }లేకపోతే ఒకవేళ( n_సదిశ!=1 ){
    sqlite3_సదిశ_Error_సందేశం(pParse, p_లోపల->p_ఎడమ);
    తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
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
స్థిర శూన్య sqlite3_సమాసం_సంకేతం_లోపల(
  Parse *pParse,        /* Parsing and code generating context */
  సమాసం *p_సమాసం,          /* The IN expression */
  పూర్ణాంకం destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  పూర్ణాంకం destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  పూర్ణాంకం rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  పూర్ణాంకం eType;            /* Type of the RHS */
  పూర్ణాంకం rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  పూర్ణాంకం *aiMap = 0;       /* Map from vector field to index column */
  అక్షరం *zAff = 0;       /* Affinity string for comparisons */
  పూర్ణాంకం n_సదిశ;          /* Size of vectors for this IN operator */
  పూర్ణాంకం iDummy;           /* Dummy parameter to exprCodeVector() */
  సమాసం *p_ఎడమ;          /* The LHS of the IN operator */
  పూర్ణాంకం i;                /* loop counter */
  పూర్ణాంకం destStep2;        /* Where to jump when NULLs seen in step 2 */
  పూర్ణాంకం destStep6 = 0;    /* Start of code for Step 6 */
  పూర్ణాంకం addrTruthOp;      /* Address of opcode that determines the IN is true */
  పూర్ణాంకం destNotNull;      /* Jump here if a comparison is not true in step 6 */
  పూర్ణాంకం addr_శిఖరం;          /* Top of the step-6 loop */
  పూర్ణాంకం iTab = 0;         /* Index to use */
  u8 ok_Const_గుణకం = pParse->ok_Const_గుణకం;

  ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p_సమాసం,EP_Immutable) );
  p_ఎడమ = p_సమాసం->p_ఎడమ;
  ఒకవేళ( sqlite3_సమాసం_Check_లోపల(pParse, p_సమాసం) ) తిరిగి;
  zAff = సమాసం_INAffinity(pParse, p_సమాసం);
  n_సదిశ = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_ఎడమ);
  aiMap = (పూర్ణాంకం*)sqlite3DbMallocZero(pParse->db, n_సదిశ*పరిమాణం(పూర్ణాంకం));
  ఒకవేళ( pParse->db->mallocFailed ) వెళ్ళు sqlite3_సమాసం_సంకేతం_లోపల_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_కనుగొను_లోపల_సూచీ(pParse, p_సమాసం,
                             లోపల_సూచీ_MEMBERSHIP | లోపల_సూచీ_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ప్రతిజ్ఞా( pParse->nErr || n_సదిశ==1 || eType==లోపల_సూచీ_EPH
       || eType==లోపల_సూచీ_సూచీ_ASC || eType==లోపల_సూచీ_సూచీ_DESC
  );
#నిర్వచితమైతే SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  కొరకు(i=0; i<n_సదిశ; i++){
    పూర్ణాంకం j, cnt;
    కొరకు(cnt=j=0; j<n_సదిశ; j++) ఒకవేళ( aiMap[j]==i ) cnt++;
    ప్రతిజ్ఞా( cnt==1 );
  }
#అయితే_ముగింపు

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
  ప్రతిజ్ఞా( pParse->ok_Const_గుణకం==ok_Const_గుణకం );
  pParse->ok_Const_గుణకం = 0;
  rLhs = సమాసం_సంకేతం_సదిశ(pParse, p_ఎడమ, &iDummy);
  pParse->ok_Const_గుణకం = ok_Const_గుణకం;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ఒకవేళ( eType==లోపల_సూచీ_NOOP ){
    సమాసం_List *pList;
    CollSeq *pColl;
    పూర్ణాంకం labelOk = sqlite3VdbeMakeLabel(pParse);
    పూర్ణాంకం r2, reg_గమ్యం_Free;
    పూర్ణాంకం regCkNull = 0;
    పూర్ణాంకం ii;
    ప్రతిజ్ఞా( n_సదిశ==1 );
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    pList = p_సమాసం->x.pList;
    pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_సమాసం->p_ఎడమ);
    ఒకవేళ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op3(v, OP_బిట్టు_And, rLhs, rLhs, regCkNull);
    }
    కొరకు(ii=0; ii<pList->n_సమాసం; ii++){
      r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, pList->a[ii].p_సమాసం, &reg_గమ్యం_Free);
      ఒకవేళ( regCkNull && sqlite3_సమాసం_Can_Be_Null(pList->a[ii].p_సమాసం) ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_బిట్టు_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_గమ్యం_Free);
      ఒకవేళ( ii<pList->n_సమాసం-1 || destIfNull!=destIfFalse ){
        పూర్ణాంకం op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_కూడు_Op4(v, op, rLhs, labelOk, r2,
                          (శూన్య*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_సమాసం-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_సమాసం-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_సమాసం-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_సమాసం-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }లేకపోతే{
        పూర్ణాంకం op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ప్రతిజ్ఞా( destIfNull==destIfFalse );
        sqlite3_Vdbe_కూడు_Op4(v, op, rLhs, destIfFalse, r2,
                          (శూన్య*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ఒకవేళ( regCkNull ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, regCkNull);
    వెళ్ళు sqlite3_సమాసం_సంకేతం_లోపల_ముగిసిన;
  }

  ఒకవేళ( eType!=లోపల_సూచీ_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ఒకవేళ( n_సదిశ==1 ){
      అక్షరం aff = zAff[0];
      ఒకవేళ( aff>=SQLITE_AFF_పాఠ్యం && aff!=sqlite3_సమాసం_Affinity(p_ఎడమ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        పూర్ణాంకం rTmp = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, rLhs, n_సదిశ, 0, zAff, n_సదిశ);
    }

    కొరకు(i=0; i<n_సదిశ && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ఒకవేళ( i!=n_సదిశ ){
      /* Need to reorder the LHS fields according to aiMap */
      పూర్ణాంకం rLhsOrig = rLhs;
      rLhs = sqlite3_Get_తాత్కాలిక_Range(pParse, n_సదిశ);
      కొరకు(i=0; i<n_సదిశ; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ఒకవేళ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }లేకపోతే{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  కొరకు(i=0; i<n_సదిశ; i++){
    సమాసం *p = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_సమాసం->p_ఎడమ, i);
    ఒకవేళ( pParse->nErr ) వెళ్ళు sqlite3_సమాసం_సంకేతం_లోపల_oom_error;
    ఒకవేళ( sqlite3_సమాసం_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ఒకవేళ( eType==లోపల_సూచీ_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ప్రతిజ్ఞా( n_సదిశ==1 );
    sqlite3_Vdbe_కూడు_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);  /* Return True */
  }లేకపోతే{
    ఒకవేళ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subrtn) ){
        స్థిరరాశి VdbeOp *pOp = sqlite3VdbeGetOp(v, p_సమాసం->y.sub.iAddr);
        ప్రతిజ్ఞా( pOp->opcode==OP_Once || pParse->nErr );
        ఒకవేళ( pOp->p3>0 ){  /* tag-202407032019 */
          ప్రతిజ్ఞా( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_సదిశ); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది, iTab, destIfFalse,
                           rLhs, n_సదిశ); VdbeCoverage(v);
      వెళ్ళు sqlite3_సమాసం_సంకేతం_లోపల_ముగిసిన;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iTab, 0,
                                      rLhs, n_సదిశ); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ఒకవేళ( rRhsHasNull && n_సదిశ==1 ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ఒకవేళ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ఒకవేళ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ఒకవేళ( eType==లోపల_సూచీ_సూచీ_DESC ){
    addr_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_చివరి, iTab, destIfFalse);
  }లేకపోతే{
    testcase( eType==లోపల_సూచీ_EPH );
    testcase( eType==లోపల_సూచీ_ROWID );
    addr_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ఒకవేళ( n_సదిశ>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }లేకపోతే{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  కొరకు(i=0; i<n_సదిశ; i++){
    సమాసం *p;
    CollSeq *pColl;
    పూర్ణాంకం r3 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    p = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_ఎడమ, i);
    ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
      సమాసం *pRhs = p_సమాసం->x.pSelect->pEList->a[i].p_సమాసం;
      pColl = sqlite3_ద్విమాన_Compare_Coll_Seq(pParse, p, pRhs);
    }లేకపోతే{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_సమాసం_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iTab, aiMap[i], r3);
    sqlite3_Vdbe_కూడు_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (శూన్య*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, r3);
  }
  sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, destIfNull);
  ఒకవేళ( n_సదిశ>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iTab, addr_శిఖరం+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addrTruthOp);

sqlite3_సమాసం_సంకేతం_లోపల_ముగిసిన:
  VdbeComment((v, "end IN expr"));
sqlite3_సమాసం_సంకేతం_లోపల_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
స్థిర శూన్య సంకేతం_Real(Vdbe *v, స్థిరరాశి అక్షరం *z, పూర్ణాంకం negateFlag, పూర్ణాంకం iMem){
  ఒకవేళ( ALWAYS(z!=0) ){
    ద్విగుణ విలువ;
    sqlite3AtoF(z, &విలువ);
    ప్రతిజ్ఞా( !sqlite3IsNaN(విలువ) ); /* The new AtoF never returns NaN */
    ఒకవేళ( negateFlag ) విలువ = -విలువ;
    sqlite3_Vdbe_కూడు_Double(v, iMem, విలువ);
  }
}
#అయితే_ముగింపు


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
స్థిర శూన్య సంకేతం_Integer(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం negFlag, పూర్ణాంకం iMem){
  Vdbe *v = pParse->pVdbe;
  ఒకవేళ( p_సమాసం->flags & EP_Int_విలువ ){
    పూర్ణాంకం i = p_సమాసం->u.i_విలువ;
    ప్రతిజ్ఞా( i>=0 );
    ఒకవేళ( negFlag ) i = -i;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, i, iMem);
  }లేకపోతే{
    పూర్ణాంకం c;
    i64 విలువ;
    స్థిరరాశి అక్షరం *z = p_సమాసం->u.zToken;
    ప్రతిజ్ఞా( z!=0 );
    c = sqlite3_Dec_Or_Hex_గమ్యం_I64(z, &విలువ);
    ఒకవేళ( (c==3 && !negFlag) || (c==2) || (negFlag && విలువ==అతిచిన్న_INT64)){
#నిర్వచితమైతే SQLITE_OMIT_FLOATING_బిందువు
      sqlite3_Error_సందేశం(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_సమాసం);
#లేకపోతే
#నిర్వచితం_కాకపోతే SQLITE_OMIT_HEX_INTEGER
      ఒకవేళ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_సందేశం(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_సమాసం);
      }లేకపోతే
#అయితే_ముగింపు
      {
        సంకేతం_Real(v, z, negFlag, iMem);
      }
#అయితే_ముగింపు
    }లేకపోతే{
      ఒకవేళ( negFlag ){ విలువ = c==3 ? అతిచిన్న_INT64 : -విలువ; }
      sqlite3_Vdbe_కూడు_Int64(v, iMem, విలువ);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
శూన్య sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(
  Parse *pParse,  /* The parsing context */
  సూచీ *pIdx,    /* The index whose column is to be loaded */
  పూర్ణాంకం iTabCur,    /* Cursor pointing to a table row */
  పూర్ణాంకం i_Idx_నిలువు,    /* The column of the index to be loaded */
  పూర్ణాంకం reg_బహిర్గతం      /* Store the index column value in this register */
){
  i16 i_Tab_నిలువు = pIdx->ai_స్తంభం[i_Idx_నిలువు];
  ఒకవేళ( i_Tab_నిలువు==XN_సమాసం ){
    ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం );
    ప్రతిజ్ఞా( pIdx->a_నిలువు_సమాసం->n_సమాసం>i_Idx_నిలువు );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_సమాసం_సంకేతం_Copy(pParse, pIdx->a_నిలువు_సమాసం->a[i_Idx_నిలువు].p_సమాసం, reg_బహిర్గతం);
    pParse->iSelfTab = 0;
  }లేకపోతే{
    sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(pParse->pVdbe, pIdx->p_పట్టిక, iTabCur,
                                    i_Tab_నిలువు, reg_బహిర్గతం);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
శూన్య sqlite3_సమాసం_సంకేతం_Generated_స్తంభం(
  Parse *pParse,     /* Parsing context */
  పట్టిక *pTab,       /* Table containing the generated column */
  స్తంభం *p_నిలువు,      /* The generated column */
  పూర్ణాంకం reg_బహిర్గతం         /* Put the result in this register */
){
  పూర్ణాంకం iAddr;
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం nErr = pParse->nErr;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pParse->iSelfTab!=0 );
  ఒకవేళ( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_కూడు_Op3(v, OP_If_Null_అడ్డువరుస, pParse->iSelfTab-1, 0, reg_బహిర్గతం);
  }లేకపోతే{
    iAddr = 0;
  }
  sqlite3_సమాసం_సంకేతం_Copy(pParse, sqlite3_స్తంభం_సమాసం(pTab,p_నిలువు), reg_బహిర్గతం);
  ఒకవేళ( (p_నిలువు->నిలువు_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    పూర్ణాంకం p3 = 2+(పూర్ణాంకం)(p_నిలువు - pTab->a_నిలువు);
    sqlite3_Vdbe_కూడు_Op4(v, OP_TypeCheck, reg_బహిర్గతం, 1, p3, (అక్షరం*)pTab, P4_పట్టిక);
  }లేకపోతే ఒకవేళ( p_నిలువు->affinity>=SQLITE_AFF_పాఠ్యం ){
    sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, reg_బహిర్గతం, 1, 0, &p_నిలువు->affinity, 1);
  }
  ఒకవేళ( iAddr ) sqlite3_Vdbe_Jump_ఇక్కడ(v, iAddr);
  ఒకవేళ( pParse->nErr>nErr ) pParse->db->err_Byte_స్థానభ్రంశం = -1;
}
#అయితే_ముగింపు /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
శూన్య sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(
  Vdbe *v,        /* Parsing context */
  పట్టిక *pTab,    /* The table containing the value */
  పూర్ణాంకం iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  పూర్ణాంకం i_నిలువు,       /* Index of the column to extract */
  పూర్ణాంకం reg_బహిర్గతం      /* Extract the value into this register */
){
  స్తంభం *p_నిలువు;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( i_నిలువు!=XN_సమాసం );
  ఒకవేళ( i_నిలువు<0 || i_నిలువు==pTab->iPKey ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iTabCur, reg_బహిర్గతం);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }లేకపోతే{
    పూర్ణాంకం x;
    ఒకవేళ( IsVirtual(pTab) ){
      x = i_నిలువు;
      sqlite3_Vdbe_కూడు_Op3(v, OP_VColumn, iTabCur, x, reg_బహిర్గతం);
      తిరిగి;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
    }లేకపోతే ఒకవేళ( (p_నిలువు = &pTab->a_నిలువు[i_నిలువు])->నిలువు_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_BUSY ){
        sqlite3_Error_సందేశం(pParse, "generated column loop on \"%s\"",
                        p_నిలువు->zCnName);
      }లేకపోతే{
        పూర్ణాంకం savedSelfTab = pParse->iSelfTab;
        p_నిలువు->నిలువు_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_సమాసం_సంకేతం_Generated_స్తంభం(pParse, pTab, p_నిలువు, reg_బహిర్గతం);
        pParse->iSelfTab = savedSelfTab;
        p_నిలువు->నిలువు_Flags &= ~COLFLAG_BUSY;
      }
      తిరిగి;
#అయితే_ముగింపు
    }లేకపోతే ఒకవేళ( !HasRowid(pTab) ){
      testcase( i_నిలువు!=sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు) );
      x = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(sqlite3_Primary_కీలకం_సూచీ(pTab), i_నిలువు);
    }లేకపోతే{
      x = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab,i_నిలువు);
      testcase( x!=i_నిలువు );
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iTabCur, x, reg_బహిర్గతం);
    sqlite3_స్తంభం_Default(v, pTab, i_నిలువు, reg_బహిర్గతం);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_Get_స్తంభం(
  Parse *pParse,   /* Parsing and code generating context */
  పట్టిక *pTab,     /* Description of the table we are reading from */
  పూర్ణాంకం i_స్తంభం,     /* Index of the table column */
  పూర్ణాంకం i_పట్టిక,      /* The cursor pointing to the table */
  పూర్ణాంకం iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ప్రతిజ్ఞా( pParse->pVdbe!=0 );
  ప్రతిజ్ఞా( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ప్రతిజ్ఞా( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(pParse->pVdbe, pTab, i_పట్టిక, i_స్తంభం, iReg);
  ఒకవేళ( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_చివరి_Op(pParse->pVdbe);
    ఒకవేళ( pOp->opcode==OP_స్తంభం ) pOp->p5 = p5;
    ఒకవేళ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  తిరిగి iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
శూన్య sqlite3_సమాసం_సంకేతం_Move(Parse *pParse, పూర్ణాంకం iFrom, పూర్ణాంకం i_గమ్యం, పూర్ణాంకం nReg){
  sqlite3_Vdbe_కూడు_Op3(pParse->pVdbe, OP_Move, iFrom, i_గమ్యం, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
శూన్య sqlite3_సమాసం_గమ్యం_రిజిస్టరు(సమాసం *p_సమాసం, పూర్ణాంకం iReg){
  సమాసం *p = sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం);
  ఒకవేళ( NEVER(p==0) ) తిరిగి;
  ఒకవేళ( p->op==TK_రిజిస్టరు ){
    ప్రతిజ్ఞా( p->i_పట్టిక==iReg );
  }లేకపోతే{
    p->op2 = p->op;
    p->op = TK_రిజిస్టరు;
    p->i_పట్టిక = iReg;
    సమాసం_Clear_Property(p, EP_Skip);
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
స్థిర పూర్ణాంకం సమాసం_సంకేతం_సదిశ(Parse *pParse, సమాసం *p, పూర్ణాంకం *piFreeable){
  పూర్ణాంకం i_ఫలితం;
  పూర్ణాంకం n_ఫలితం = sqlite3_సమాసం_సదిశ_కొలత(p);
  ఒకవేళ( n_ఫలితం==1 ){
    i_ఫలితం = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p, piFreeable);
  }లేకపోతే{
    *piFreeable = 0;
    ఒకవేళ( p->op==TK_SELECT ){
#ఒకవేళ SQLITE_OMIT_SUBQUERY
      i_ఫలితం = 0;
#లేకపోతే
      i_ఫలితం = sqlite3_సంకేతం_Subselect(pParse, p);
#అయితే_ముగింపు
    }లేకపోతే{
      పూర్ణాంకం i;
      i_ఫలితం = pParse->nMem+1;
      pParse->nMem += n_ఫలితం;
      ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
      కొరకు(i=0; i<n_ఫలితం; i++){
        sqlite3_సమాసం_సంకేతం_Factorable(pParse, p->x.pList->a[i].p_సమాసం, i+i_ఫలితం);
      }
    }
  }
  తిరిగి i_ఫలితం;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
స్థిర శూన్య సమితి_Do_Not_విలీనం_Flag_On_Copy(Vdbe *v){
  ఒకవేళ( sqlite3_Vdbe_Get_చివరి_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
స్థిర పూర్ణాంకం సమాసం_సంకేతం_Inline_Function(
  Parse *pParse,        /* Parsing context */
  సమాసం_List *pFarg,      /* List of function arguments */
  పూర్ణాంకం iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  పూర్ణాంకం లక్ష్యం            /* Store function result in this register */
){
  పూర్ణాంకం nFarg;
  Vdbe *v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pFarg!=0 );
  nFarg = pFarg->n_సమాసం;
  ప్రతిజ్ఞా( nFarg>0 );  /* All in-line functions have at least one argument */
  ఎంపిక( iFuncId ){
    స్థితి INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      పూర్ణాంకం చివర_Coalesce = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం i;
      ప్రతిజ్ఞా( nFarg>=2 );
      sqlite3_సమాసం_సంకేతం(pParse, pFarg->a[0].p_సమాసం, లక్ష్యం);
      కొరకు(i=1; i<nFarg; i++){
        sqlite3_Vdbe_కూడు_Op2(v, OP_NotNull, లక్ష్యం, చివర_Coalesce);
        VdbeCoverage(v);
        sqlite3_సమాసం_సంకేతం(pParse, pFarg->a[i].p_సమాసం, లక్ష్యం);
      }
      సమితి_Do_Not_విలీనం_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, చివర_Coalesce);
      ఆపు;
    }
    స్థితి INLINEFUNC_iif: {
      సమాసం case_సమాసం;
      memset(&case_సమాసం, 0, పరిమాణం(case_సమాసం));
      case_సమాసం.op = TK_CASE;
      case_సమాసం.x.pList = pFarg;
      తిరిగి sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, &case_సమాసం, లక్ష్యం);
    }
#నిర్వచితమైతే SQLITE_ENABLE_స్థానభ్రంశం_SQL_FUNC
    స్థితి INLINEFUNC_sqlite_స్థానభ్రంశం: {
      సమాసం *pArg = pFarg->a[0].p_సమాసం;
      ఒకవేళ( pArg->op==TK_స్తంభం && pArg->i_పట్టిక>=0 ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్థానభ్రంశం, pArg->i_పట్టిక, pArg->i_స్తంభం, లక్ష్యం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
      }
      ఆపు;
    }
#అయితే_ముగింపు
    అప్రమేయ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ప్రతిజ్ఞా( nFarg==1 || nFarg==2 );
      లక్ష్యం = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, pFarg->a[0].p_సమాసం, లక్ష్యం);
      ఆపు;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ఒకవేళ !defined(SQLITE_UNTESTABLE)
    స్థితి INLINEFUNC_సమాసం_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ప్రతిజ్ఞా( nFarg==2 );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer,
         sqlite3_సమాసం_Compare(0,pFarg->a[0].p_సమాసం, pFarg->a[1].p_సమాసం,-1),
         లక్ష్యం);
      ఆపు;
    }

    స్థితి INLINEFUNC_సమాసం_సూచిస్తుంది_సమాసం: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ప్రతిజ్ఞా( nFarg==2 );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer,
         sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse,pFarg->a[0].p_సమాసం, pFarg->a[1].p_సమాసం,-1),
         లక్ష్యం);
      ఆపు;
    }

    స్థితి INLINEFUNC_సూచిస్తుంది_nonnull_అడ్డువరుస: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      సమాసం *pA1;
      ప్రతిజ్ఞా( nFarg==2 );
      pA1 = pFarg->a[1].p_సమాసం;
      ఒకవేళ( pA1->op==TK_స్తంభం ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer,
           sqlite3_సమాసం_సూచిస్తుంది_Non_Null_అడ్డువరుస(pFarg->a[0].p_సమాసం,pA1->i_పట్టిక,1),
           లక్ష్యం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
      }
      ఆపు;
    }

    స్థితి INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      స్థిరరాశి అక్షరం *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      అక్షరం aff;
      ప్రతిజ్ఞా( nFarg==1 );
      aff = sqlite3_సమాసం_Affinity(pFarg->a[0].p_సమాసం);
      ప్రతిజ్ఞా( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3_Vdbe_ఎక్కించు_String(v, లక్ష్యం,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ఆపు;
    }
#అయితే_ముగింపు /* !defined(SQLITE_UNTESTABLE) */
  }
  తిరిగి లక్ష్యం;
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
స్థిర పూర్ణాంకం సమాసం_కణుపు_Can_Return_Subtype(Walker *pWalker, సమాసం *p_సమాసం){
  పూర్ణాంకం n;
  FuncDef *pDef;
  sqlite3 *db;
  ఒకవేళ( p_సమాసం->op==TK_CASE || p_సమాసం->op==TK_UPLUS 
   || p_సమాసం->op==TK_COLLATE || p_సమాసం->op==TK_CAST 
  ){
    తిరిగి WRC_Continue;
  }
  ఒకవేళ( p_సమాసం->op!=TK_FUNCTION && p_సమాసం->op!=TK_AGG_FUNCTION ){
    తిరిగి WRC_Prune;
  }
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  db = pWalker->pParse->db;
  n = p_సమాసం->x.pList ? p_సమాసం->x.pList->n_సమాసం : 0;
  pDef = sqlite3_కనుగొను_Function(db, p_సమాసం->u.zToken, n, ENC(db), 0);
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ఒకవేళ( pDef==0 ) తిరిగి WRC_Continue;
#లేకపోతే
  ప్రతిజ్ఞా( pDef!=0 );
#అయితే_ముగింపు
  ఒకవేళ( (pDef->funcFlags & SQLITE_ఫలితం_SUBTYPE)!=0 ){
    pWalker->e_సంకేతం = 1;
    తిరిగి WRC_Abort;
  }
  తిరిగి WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
పూర్ణాంకం sqlite3_సమాసం_Can_Return_Subtype(Parse *pParse, సమాసం *p_సమాసం){
  Walker w;
  memset(&w, 0, పరిమాణం(w));
  w.pParse = pParse;
  w.x_సమాసం_Callback = సమాసం_కణుపు_Can_Return_Subtype;
  sqlite3_Walk_సమాసం(&w, p_సమాసం);
  తిరిగి w.e_సంకేతం;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం sqlite3_సూచీకృత_సమాసం_Lookup(
  Parse *pParse,   /* The parsing context */
  సమాసం *p_సమాసం,     /* The expression to potentially bypass */
  పూర్ణాంకం లక్ష్యం       /* Where to store the result of the expression */
){
  సూచీకృత_సమాసం *p;
  Vdbe *v;
  కొరకు(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 సమాసం_Aff;
    పూర్ణాంకం i_దత్తాంశం_Cur = p->i_దత్తాంశం_Cur;
    ఒకవేళ( i_దత్తాంశం_Cur<0 ) కొనసాగు;
    ఒకవేళ( pParse->iSelfTab ){
      ఒకవేళ( p->i_దత్తాంశం_Cur!=pParse->iSelfTab-1 ) కొనసాగు;
      i_దత్తాంశం_Cur = -1;
    }
    ఒకవేళ( sqlite3_సమాసం_Compare(0, p_సమాసం, p->p_సమాసం, i_దత్తాంశం_Cur)!=0 ) కొనసాగు;
    ప్రతిజ్ఞా( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    సమాసం_Aff = sqlite3_సమాసం_Affinity(p_సమాసం);
    ఒకవేళ( (సమాసం_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (సమాసం_Aff==SQLITE_AFF_పాఠ్యం && p->aff!=SQLITE_AFF_పాఠ్యం)
     || (సమాసం_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      కొనసాగు;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_SubtArg)
     && sqlite3_సమాసం_Can_Return_Subtype(pParse, p_సమాసం) 
    ){
      కొనసాగు;
    }

    v = pParse->pVdbe;
    ప్రతిజ్ఞా( v!=0 );
    ఒకవేళ( p->b_Maybe_Null_అడ్డువరుస || sqlite3_సమాసం_Is_Constant(pParse, p_సమాసం) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      పూర్ణాంకం addr = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_If_Null_అడ్డువరుస, p->iIdxCur, addr+3, లక్ష్యం);
      VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, p->iIdxCur, p->i_Idx_నిలువు, లక్ష్యం);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_నిలువు));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం, లక్ష్యం);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr+2);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, p->iIdxCur, p->i_Idx_నిలువు, లక్ష్యం);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_నిలువు));
    }
    తిరిగి లక్ష్యం;
  }
  తిరిగి -1;  /* Not found */
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
స్థిర పూర్ణాంకం సమాసం_Partidx_సమాసం_Lookup(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం i_లక్ష్యం){
  సూచీకృత_సమాసం *p;
  కొరకు(p=pParse->p_Idx_Part_సమాసం; p; p=p->pIENext){
    ఒకవేళ( p_సమాసం->i_స్తంభం==p->i_Idx_నిలువు && p_సమాసం->i_పట్టిక==p->i_దత్తాంశం_Cur ){
      Vdbe *v = pParse->pVdbe;
      పూర్ణాంకం addr = 0;
      పూర్ణాంకం ret;

      ఒకవేళ( p->b_Maybe_Null_అడ్డువరుస ){
        addr = sqlite3_Vdbe_కూడు_Op1(v, OP_If_Null_అడ్డువరుస, p->iIdxCur);
      }
      ret = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p->p_సమాసం, i_లక్ష్యం);
      sqlite3_Vdbe_కూడు_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (స్థిరరాశి అక్షరం*)&p->aff, 1);
      ఒకవేళ( sqlite3_సమాసం_Can_Return_Subtype(pParse, p->p_సమాసం) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_కూడు_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ఒకవేళ( addr ){
        sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      తిరిగి ret;
    }
  }
  తిరిగి 0;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం సమాసం_సంకేతం_లక్ష్యం_And_Or(
  Parse *pParse,     /* Parsing context */
  సమాసం *p_సమాసం,       /* AND or OR expression to be coded */
  పూర్ణాంకం లక్ష్యం,        /* Put result in this register, guaranteed */
  పూర్ణాంకం *pTmpReg       /* Write a temporary register here */
){
  పూర్ణాంకం op;            /* The opcode.  TK_AND or TK_OR */
  పూర్ణాంకం skipOp;        /* Opcode for the branch that skips one operand */
  పూర్ణాంకం addrSkip;      /* Branch instruction that skips one of the operands */
  పూర్ణాంకం regSS = 0;     /* Register holding computed operand when other omitted */
  పూర్ణాంకం r1, r2;        /* Registers for left and right operands, respectively */
  సమాసం *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  ప్రతిజ్ఞా( p_సమాసం!=0 );
  op = p_సమాసం->op;
  ప్రతిజ్ఞా( op==TK_AND || op==TK_OR );
  ప్రతిజ్ఞా( TK_AND==OP_And );            testcase( op==TK_AND );
  ప్రతిజ్ఞా( TK_OR==OP_Or );              testcase( op==TK_OR );
  ప్రతిజ్ఞా( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం);
  ఒకవేళ( pAlt!=p_సమాసం ){
    r1 = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, pAlt, లక్ష్యం);
    sqlite3_Vdbe_కూడు_Op3(v, OP_And, r1, r1, లక్ష్యం);
    తిరిగి లక్ష్యం;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ఒకవేళ( సమాసం_Eval_Rhs_మొదటి(p_సమాసం) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం->p_కుడి, లక్ష్యం);
    addrSkip = sqlite3_Vdbe_కూడు_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, pTmpReg);
  }లేకపోతే{
    /* Compute the left operand first */
    r1 = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం->p_ఎడమ, లక్ష్యం);
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం->p_కుడి, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_కూడు_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }లేకపోతే{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, pTmpReg);
  }
  sqlite3_Vdbe_కూడు_Op3(v, op, r2, r1, లక్ష్యం);
  testcase( (*pTmpReg)==0 );
  ఒకవేళ( addrSkip ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addrSkip);
    sqlite3_Vdbe_కూడు_Op3(v, OP_Or, regSS, regSS, లక్ష్యం);
    VdbeComment((v, "short-circut value"));
  }
  తిరిగి లక్ష్యం;
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
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_లక్ష్యం(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం లక్ష్యం){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  పూర్ణాంకం op;                   /* The opcode being coded */
  పూర్ణాంకం లోపల_Reg = లక్ష్యం;       /* Results stored in register inReg */
  పూర్ణాంకం regFree1 = 0;         /* If non-zero free this temporary register */
  పూర్ణాంకం regFree2 = 0;         /* If non-zero free this temporary register */
  పూర్ణాంకం r1, r2;               /* Various register numbers */
  సమాసం తాత్కాలిక_X;               /* Temporary expression node */
  పూర్ణాంకం p5 = 0;

  ప్రతిజ్ఞా( లక్ష్యం>0 && లక్ష్యం<=pParse->nMem );
  ప్రతిజ్ఞా( v!=0 );

సమాసం_సంకేతం_doover:
  ఒకవేళ( p_సమాసం==0 ){
    op = TK_NULL;
  }లేకపోతే ఒకవేళ( pParse->pIdxEpr!=0
   && !సమాసం_Has_Property(p_సమాసం, EP_ఆకు)
   && (r1 = sqlite3_సూచీకృత_సమాసం_Lookup(pParse, p_సమాసం, లక్ష్యం))>=0
  ){
    తిరిగి r1;
  }లేకపోతే{
    ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p_సమాసం,EP_Immutable) );
    op = p_సమాసం->op;
  }
  ప్రతిజ్ఞా( op!=TK_క్రమం );
  ఎంపిక( op ){
    స్థితి TK_AGG_స్తంభం: {
      AggInfo *pAggInfo = p_సమాసం->pAggInfo;
      నిర్మాణం Agg_Info_నిలువు *p_నిలువు;
      ప్రతిజ్ఞా( pAggInfo!=0 );
      ప్రతిజ్ఞా( p_సమాసం->iAgg>=0 );
      ఒకవేళ( p_సమాసం->iAgg>=pAggInfo->n_స్తంభం ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_కూడు_Op3(v, OP_NotNull, లక్ష్యం, 1, 20230325);
        Vdbe_Coverage_Never_తీసుకున్న(v);
#అయితే_ముగింపు
        ఆపు;
      }
      p_నిలువు = &pAggInfo->a_నిలువు[p_సమాసం->iAgg];
      ఒకవేళ( !pAggInfo->ప్రత్యక్ష_బహుళకం ){
        తిరిగి Agg_Info_స్తంభం_Reg(pAggInfo, p_సమాసం->iAgg);
      }లేకపోతే ఒకవేళ( pAggInfo->use_క్రమీకరణ_Idx ){
        పట్టిక *pTab = p_నిలువు->pTab;
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, pAggInfo->క్రమీకరణ_Idx_PTab,
                              p_నిలువు->i_Sorter_స్తంభం, లక్ష్యం);
        ఒకవేళ( pTab==0 ){
          /* No comment added */
        }లేకపోతే ఒకవేళ( p_నిలువు->i_స్తంభం<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }లేకపోతే{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_నిలువు[p_నిలువు->i_స్తంభం].zCnName));
          ఒకవేళ( pTab->a_నిలువు[p_నిలువు->i_స్తంభం].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, లక్ష్యం);
          }
        }
        తిరిగి లక్ష్యం;
      }లేకపోతే ఒకవేళ( p_సమాసం->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, p_సమాసం->i_పట్టిక, p_సమాసం->i_స్తంభం, లక్ష్యం);
        తిరిగి లక్ష్యం;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    స్థితి TK_స్తంభం: {
      పూర్ణాంకం iTab = p_సమాసం->i_పట్టిక;
      పూర్ణాంకం iReg;
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        పూర్ణాంకం aff;
        iReg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం->p_ఎడమ,లక్ష్యం);
        ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
        ప్రతిజ్ఞా( p_సమాసం->y.pTab!=0 );
        aff = sqlite3_పట్టిక_స్తంభం_Affinity(p_సమాసం->y.pTab, p_సమాసం->i_స్తంభం);
        ఒకవేళ( aff>SQLITE_AFF_BLOB ){
          స్థిర స్థిరరాశి అక్షరం zAff[] = "B\000C\000D\000E\000F";
          ప్రతిజ్ఞా( SQLITE_AFF_BLOB=='A' );
          ప్రతిజ్ఞా( SQLITE_AFF_పాఠ్యం=='B' );
          sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        తిరిగి iReg;
      }
      ఒకవేళ( iTab<0 ){
        ఒకవేళ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          స్తంభం *p_నిలువు;
          పట్టిక *pTab;
          పూర్ణాంకం iSrc;
          పూర్ణాంకం i_నిలువు = p_సమాసం->i_స్తంభం;
          ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
          pTab = p_సమాసం->y.pTab;
          ప్రతిజ్ఞా( pTab!=0 );
          ప్రతిజ్ఞా( i_నిలువు>=XN_ROWID );
          ప్రతిజ్ఞా( i_నిలువు<pTab->n_నిలువు );
          ఒకవేళ( i_నిలువు<0 ){
            తిరిగి -1-pParse->iSelfTab;
          }
          p_నిలువు = pTab->a_నిలువు + i_నిలువు;
          testcase( i_నిలువు!=sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab,i_నిలువు) );
          iSrc = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు) - pParse->iSelfTab;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
          ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_GENERATED ){
            ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_BUSY ){
              sqlite3_Error_సందేశం(pParse, "generated column loop on \"%s\"",
                              p_నిలువు->zCnName);
              తిరిగి 0;
            }
            p_నిలువు->నిలువు_Flags |= COLFLAG_BUSY;
            ఒకవేళ( p_నిలువు->నిలువు_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_సమాసం_సంకేతం_Generated_స్తంభం(pParse, pTab, p_నిలువు, iSrc);
            }
            p_నిలువు->నిలువు_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            తిరిగి iSrc;
          }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_GENERATED_COLUMNS */
          ఒకవేళ( p_నిలువు->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy, iSrc, లక్ష్యం);
            sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, లక్ష్యం);
            తిరిగి లక్ష్యం;
          }లేకపోతే{
            తిరిగి iSrc;
          }
        }లేకపోతే{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      లేకపోతే ఒకవేళ( pParse->p_Idx_Part_సమాసం 
       && 0!=(r1 = సమాసం_Partidx_సమాసం_Lookup(pParse, p_సమాసం, లక్ష్యం))
      ){
        తిరిగి r1;
      }
      ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->y.pTab!=0 );
      iReg = sqlite3_సమాసం_సంకేతం_Get_స్తంభం(pParse, p_సమాసం->y.pTab,
                               p_సమాసం->i_స్తంభం, iTab, లక్ష్యం,
                               p_సమాసం->op2);
      తిరిగి iReg;
    }
    స్థితి TK_INTEGER: {
      సంకేతం_Integer(pParse, p_సమాసం, 0, లక్ష్యం);
      తిరిగి లక్ష్యం;
    }
    స్థితి TK_TRUEFALSE: {
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, sqlite3_సమాసం_Truth_విలువ(p_సమాసం), లక్ష్యం);
      తిరిగి లక్ష్యం;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
    స్థితి TK_FLOAT: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      సంకేతం_Real(v, p_సమాసం->u.zToken, 0, లక్ష్యం);
      తిరిగి లక్ష్యం;
    }
#అయితే_ముగింపు
    స్థితి TK_STRING: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_Vdbe_ఎక్కించు_String(v, లక్ష్యం, p_సమాసం->u.zToken);
      తిరిగి లక్ష్యం;
    }
    స్థితి TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, లక్ష్యం, లక్ష్యం + p_సమాసం->y.nReg - 1);
      తిరిగి లక్ష్యం;
    }
    అప్రమేయ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ప్రతిజ్ఞా( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
      తిరిగి లక్ష్యం;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_BLOB_LITERAL
    స్థితి TK_BLOB: {
      పూర్ణాంకం n;
      స్థిరరాశి అక్షరం *z;
      అక్షరం *zBlob;
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      ప్రతిజ్ఞా( p_సమాసం->u.zToken[0]=='x' || p_సమాసం->u.zToken[0]=='X' );
      ప్రతిజ్ఞా( p_సమాసం->u.zToken[1]=='\'' );
      z = &p_సమాసం->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ప్రతిజ్ఞా( z[n]=='\'' );
      zBlob = sqlite3_Hex_గమ్యం_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_కూడు_Op4(v, OP_Blob, n/2, లక్ష్యం, 0, zBlob, P4_DYNAMIC);
      తిరిగి లక్ష్యం;
    }
#అయితే_ముగింపు
    స్థితి TK_VARIABLE: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      ప్రతిజ్ఞా( p_సమాసం->u.zToken!=0 );
      ప్రతిజ్ఞా( p_సమాసం->u.zToken[0]!=0 );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Variable, p_సమాసం->i_స్తంభం, లక్ష్యం);
      తిరిగి లక్ష్యం;
    }
    స్థితి TK_రిజిస్టరు: {
      తిరిగి p_సమాసం->i_పట్టిక;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
    స్థితి TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం->p_ఎడమ, లక్ష్యం);
      ప్రతిజ్ఞా( లోపల_Reg==లక్ష్యం );
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Cast, లక్ష్యం,
                        sqlite3AffinityType(p_సమాసం->u.zToken, 0));
      తిరిగి లోపల_Reg;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_CAST */
    స్థితి TK_IS:
    స్థితి TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE:
    స్థితి TK_NE:
    స్థితి TK_EQ: {
      సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
      పూర్ణాంకం addrIsNull = 0;
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_ఎడమ) ){
        సంకేతం_సదిశ_Compare(pParse, p_సమాసం, లక్ష్యం, op, p5);
      }లేకపోతే{
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = సమాసం_Compute_Operands(pParse, p_సమాసం,
                                     &r1, &r2, &regFree1, &regFree2);
        }లేకపోతే{
          r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
          r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, &regFree2);
        }
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, లోపల_Reg);
        సంకేతం_Compare(pParse, p_ఎడమ, p_సమాసం->p_కుడి, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            సమాసం_Has_Property(p_సమాసం,EP_Commuted));
        ప్రతిజ్ఞా(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ప్రతిజ్ఞా(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ప్రతిజ్ఞా(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ప్రతిజ్ఞా(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ప్రతిజ్ఞా(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ప్రతిజ్ఞా(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ఒకవేళ( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, లోపల_Reg);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op3(v, OP_ZeroOrNull, r1, లోపల_Reg, r2);
          ఒకవేళ( addrIsNull ){
            sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIsNull);
            sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లోపల_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ఆపు;
    }
    స్థితి TK_AND:
    స్థితి TK_OR: {
      లోపల_Reg = సమాసం_సంకేతం_లక్ష్యం_And_Or(pParse, p_సమాసం, లక్ష్యం, &regFree1);
      ఆపు;
    }
    స్థితి TK_PLUS:
    స్థితి TK_నక్షత్రం:
    స్థితి TK_MINUS:
    స్థితి TK_REM:
    స్థితి TK_BITAND:
    స్థితి TK_BITOR:
    స్థితి TK_SLASH:
    స్థితి TK_LSHIFT:
    స్థితి TK_RSHIFT:
    స్థితి TK_CONCAT: {
      పూర్ణాంకం addrIsNull;
      ప్రతిజ్ఞా( TK_PLUS==OP_కూడు );           testcase( op==TK_PLUS );
      ప్రతిజ్ఞా( TK_MINUS==OP_వ్యవకలనం );     testcase( op==TK_MINUS );
      ప్రతిజ్ఞా( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ప్రతిజ్ఞా( TK_BITAND==OP_బిట్టు_And );      testcase( op==TK_BITAND );
      ప్రతిజ్ఞా( TK_BITOR==OP_బిట్టు_Or );        testcase( op==TK_BITOR );
      ప్రతిజ్ఞా( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ప్రతిజ్ఞా( TK_LSHIFT==OP_జరుపు_ఎడమ );   testcase( op==TK_LSHIFT );
      ప్రతిజ్ఞా( TK_RSHIFT==OP_జరుపు_కుడి );  testcase( op==TK_RSHIFT );
      ప్రతిజ్ఞా( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subquery) ){
        addrIsNull = సమాసం_Compute_Operands(pParse, p_సమాసం,
                                   &r1, &r2, &regFree1, &regFree2);
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
        r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_కూడు_Op3(v, op, r2, r1, లక్ష్యం);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ఒకవేళ( addrIsNull ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIsNull);
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
        VdbeComment((v, "short-circut value"));
      }
      ఆపు;
    }
    స్థితి TK_UMINUS: {
      సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
      ప్రతిజ్ఞా( p_ఎడమ );
      ఒకవేళ( p_ఎడమ->op==TK_INTEGER ){
        సంకేతం_Integer(pParse, p_ఎడమ, 1, లక్ష్యం);
        తిరిగి లక్ష్యం;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
      }లేకపోతే ఒకవేళ( p_ఎడమ->op==TK_FLOAT ){
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
        సంకేతం_Real(v, p_ఎడమ->u.zToken, 1, లక్ష్యం);
        తిరిగి లక్ష్యం;
#అయితే_ముగింపు
      }లేకపోతే{
        తాత్కాలిక_X.op = TK_INTEGER;
        తాత్కాలిక_X.flags = EP_Int_విలువ|EP_TokenOnly;
        తాత్కాలిక_X.u.i_విలువ = 0;
        సమాసం_Clear_VVAProperties(&తాత్కాలిక_X);
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, &తాత్కాలిక_X, &regFree1);
        r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree2);
        sqlite3_Vdbe_కూడు_Op3(v, OP_వ్యవకలనం, r2, r1, లక్ష్యం);
        testcase( regFree2==0 );
      }
      ఆపు;
    }
    స్థితి TK_BITNOT:
    స్థితి TK_NOT: {
      ప్రతిజ్ఞా( TK_BITNOT==OP_బిట్టు_Not );   testcase( op==TK_BITNOT );
      ప్రతిజ్ఞా( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_కూడు_Op2(v, op, r1, లోపల_Reg);
      ఆపు;
    }
    స్థితి TK_TRUTH: {
      పూర్ణాంకం isTrue;    /* IS TRUE or IS NOT TRUE */
      పూర్ణాంకం b_సాధారణ;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_సమాసం_Truth_విలువ(p_సమాసం->p_కుడి);
      b_సాధారణ = p_సమాసం->op2==TK_IS;
      testcase( isTrue && b_సాధారణ);
      testcase( !isTrue && b_సాధారణ);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IsTrue, r1, లోపల_Reg, !isTrue, isTrue ^ b_సాధారణ);
      ఆపు;
    }
    స్థితి TK_ISNULL:
    స్థితి TK_NOTNULL: {
      పూర్ణాంకం addr;
      ప్రతిజ్ఞా( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ప్రతిజ్ఞా( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, లక్ష్యం);
      r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_కూడు_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, లక్ష్యం);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
      ఆపు;
    }
    స్థితి TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_సమాసం->pAggInfo;
      ఒకవేళ( pInfo==0
       || NEVER(p_సమాసం->iAgg<0)
       || NEVER(p_సమాసం->iAgg>=pInfo->nFunc)
      ){
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
        sqlite3_Error_సందేశం(pParse, "misuse of aggregate: %#T()", p_సమాసం);
      }లేకపోతే{
        తిరిగి AggInfoFuncReg(pInfo, p_సమాసం->iAgg);
      }
      ఆపు;
    }
    స్థితి TK_FUNCTION: {
      సమాసం_List *pFarg;       /* List of function arguments */
      పూర్ణాంకం nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      స్థిరరాశి అక్షరం *zId;       /* The function name */
      u32 const_ముసుగు = 0;     /* Mask of function arguments that are constant */
      పూర్ణాంకం i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
        తిరిగి p_సమాసం->y.pWin->reg_ఫలితం;
      }
#అయితే_ముగింపు

      ఒకవేళ( Const_గుణకం_Ok(pParse)
       && sqlite3_సమాసం_Is_Constant_Not_Join(pParse,p_సమాసం)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        తిరిగి sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(pParse, p_సమాసం, -1);
      }
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_TokenOnly) );
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      pFarg = p_సమాసం->x.pList;
      nFarg = pFarg ? pFarg->n_సమాసం : 0;
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      zId = p_సమాసం->u.zToken;
      pDef = sqlite3_కనుగొను_Function(db, zId, nFarg, enc, 0);
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ఒకవేళ( pDef==0 && pParse->explain ){
        pDef = sqlite3_కనుగొను_Function(db, "unknown", nFarg, enc, 0);
      }
#అయితే_ముగింపు
      ఒకవేళ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_అంతర్గత)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_అంతర్గత_Func)==0)
      ){
        sqlite3_Error_సందేశం(pParse, "unknown function: %#T()", p_సమాసం);
        ఆపు;
      }
      ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ప్రతిజ్ఞా( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ప్రతిజ్ఞా( (pDef->funcFlags & SQLITE_FUNC_ప్రత్యక్ష)==0 );
        తిరిగి సమాసం_సంకేతం_Inline_Function(pParse, pFarg,
             SQLITE_PTR_గమ్యం_INT(pDef->p_User_దత్తాంశం), లక్ష్యం);
      }లేకపోతే ఒకవేళ( pDef->funcFlags & (SQLITE_FUNC_ప్రత్యక్ష|SQLITE_FUNC_UNSAFE) ){
        sqlite3_సమాసం_Function_Usable(pParse, p_సమాసం, pDef);
      }

      కొరకు(i=0; i<nFarg; i++){
        ఒకవేళ( i<32 && sqlite3_సమాసం_Is_Constant(pParse, pFarg->a[i].p_సమాసం) ){
          testcase( i==31 );
          const_ముసుగు |= MASKBIT32(i);
        }
        ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_సమాసం_Coll_Seq(pParse, pFarg->a[i].p_సమాసం);
        }
      }
      ఒకవేళ( pFarg ){
        ఒకవేళ( const_ముసుగు ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }లేకపోతే{
          r1 = sqlite3_Get_తాత్కాలిక_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ఒకవేళ( (pDef->funcFlags & (SQLITE_FUNC_నిడివి|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 సమాసం_Op;
          ప్రతిజ్ఞా( nFarg==1 );
          ప్రతిజ్ఞా( pFarg->a[0].p_సమాసం!=0 );
          సమాసం_Op = pFarg->a[0].p_సమాసం->op;
          ఒకవేళ( సమాసం_Op==TK_స్తంభం || సమాసం_Op==TK_AGG_స్తంభం ){
            ప్రతిజ్ఞా( SQLITE_FUNC_నిడివి==OPFLAG_LENGTHARG );
            ప్రతిజ్ఞా( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ప్రతిజ్ఞా( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ప్రతిజ్ఞా( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_సమాసం->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pFarg, r1, 0, SQLITE_ECEL_గుణకం);
      }లేకపోతే{
        r1 = 0;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
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
      ఒకవేళ( nFarg>=2 && సమాసం_Has_Property(p_సమాసం, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_సమాసం);
      }లేకపోతే ఒకవేళ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_సమాసం);
      }
#అయితే_ముగింపు
      ఒకవేళ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ఒకవేళ( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_కూడు_Op4(v, OP_CollSeq, 0, 0, 0, (అక్షరం *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_కూడు_Function_Call(pParse, const_ముసుగు, r1, లక్ష్యం, nFarg,
                                 pDef, p_సమాసం->op2);
      ఒకవేళ( nFarg ){
        ఒకవేళ( const_ముసుగు==0 ){
          sqlite3_Release_తాత్కాలిక_Range(pParse, r1, nFarg);
        }లేకపోతే{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_ముసుగు, 1);
        }
      }
      తిరిగి లక్ష్యం;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_EXISTS:
    స్థితి TK_SELECT: {
      పూర్ణాంకం n_నిలువు;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ఒకవేళ( pParse->db->mallocFailed ){
        తిరిగి 0;
      }లేకపోతే ఒకవేళ( op==TK_SELECT
             && ALWAYS( సమాసం_Use_XSelect(p_సమాసం) )
             && (n_నిలువు = p_సమాసం->x.pSelect->pEList->n_సమాసం)!=1
      ){
        sqlite3SubselectError(pParse, n_నిలువు, 1);
      }లేకపోతే{
        తిరిగి sqlite3_సంకేతం_Subselect(pParse, p_సమాసం);
      }
      ఆపు;
    }
    స్థితి TK_SELECT_స్తంభం: {
      పూర్ణాంకం n;
      సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
      ఒకవేళ( p_ఎడమ->i_పట్టిక==0 || pParse->withinRJSubrtn > p_ఎడమ->op2 ){
        p_ఎడమ->i_పట్టిక = sqlite3_సంకేతం_Subselect(pParse, p_ఎడమ);
        p_ఎడమ->op2 = pParse->withinRJSubrtn;
      }
      ప్రతిజ్ఞా( p_ఎడమ->op==TK_SELECT || p_ఎడమ->op==TK_ERROR );
      n = sqlite3_సమాసం_సదిశ_కొలత(p_ఎడమ);
      ఒకవేళ( p_సమాసం->i_పట్టిక!=n ){
        sqlite3_Error_సందేశం(pParse, "%d columns assigned %d values",
                                p_సమాసం->i_పట్టిక, n);
      }
      తిరిగి p_ఎడమ->i_పట్టిక + p_సమాసం->i_స్తంభం;
    }
    స్థితి TK_లోపల: {
      పూర్ణాంకం destIfFalse = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
      sqlite3_సమాసం_సంకేతం_లోపల(pParse, p_సమాసం, destIfFalse, destIfNull);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, లక్ష్యం);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, లక్ష్యం, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      తిరిగి లక్ష్యం;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */


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
    స్థితి TK_BETWEEN: {
      సమాసం_సంకేతం_Between(pParse, p_సమాసం, లక్ష్యం, 0, 0);
      తిరిగి లక్ష్యం;
    }
    స్థితి TK_COLLATE: {
      ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ );
        sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం->p_ఎడమ, లక్ష్యం);
        sqlite3_Vdbe_కూడు_Op1(v, OP_ClrSubtype, లక్ష్యం);
        తిరిగి లక్ష్యం;
      }లేకపోతే{
        p_సమాసం = p_సమాసం->p_ఎడమ;
        వెళ్ళు సమాసం_సంకేతం_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    స్థితి TK_పరిధి:
    స్థితి TK_UPLUS: {
      p_సమాసం = p_సమాసం->p_ఎడమ;
      వెళ్ళు సమాసం_సంకేతం_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    స్థితి TK_TRIGGER: {
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
      పట్టిక *pTab;
      పూర్ణాంకం i_నిలువు;
      పూర్ణాంకం p1;

      ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
      pTab = p_సమాసం->y.pTab;
      i_నిలువు = p_సమాసం->i_స్తంభం;
      p1 = p_సమాసం->i_పట్టిక * (pTab->n_నిలువు+1) + 1
                     + sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు);

      ప్రతిజ్ఞా( p_సమాసం->i_పట్టిక==0 || p_సమాసం->i_పట్టిక==1 );
      ప్రతిజ్ఞా( i_నిలువు>=-1 && i_నిలువు<pTab->n_నిలువు );
      ప్రతిజ్ఞా( pTab->iPKey<0 || i_నిలువు!=pTab->iPKey );
      ప్రతిజ్ఞా( p1>=0 && p1<(pTab->n_నిలువు*2+2) );

      sqlite3_Vdbe_కూడు_Op2(v, OP_Param, p1, లక్ష్యం);
      VdbeComment((v, "r[%d]=%s.%s", లక్ష్యం,
        (p_సమాసం->i_పట్టిక ? "new" : "old"),
        (p_సమాసం->i_స్తంభం<0 ? "rowid" : p_సమాసం->y.pTab->a_నిలువు[i_నిలువు].zCnName)
      ));

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ఒకవేళ( i_నిలువు>=0 && pTab->a_నిలువు[i_నిలువు].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_కూడు_Op1(v, OP_RealAffinity, లక్ష్యం);
      }
#అయితే_ముగింపు
      ఆపు;
    }

    స్థితి TK_సదిశ: {
      sqlite3_Error_సందేశం(pParse, "row value misused");
      ఆపు;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    స్థితి TK_IF_NULL_అడ్డువరుస: {
      పూర్ణాంకం addrINR;
      u8 ok_Const_గుణకం = pParse->ok_Const_గుణకం;
      AggInfo *pAggInfo = p_సమాసం->pAggInfo;
      ఒకవేళ( pAggInfo ){
        ప్రతిజ్ఞా( p_సమాసం->iAgg>=0 && p_సమాసం->iAgg<pAggInfo->n_స్తంభం );
        ఒకవేళ( !pAggInfo->ప్రత్యక్ష_బహుళకం ){
          లోపల_Reg = Agg_Info_స్తంభం_Reg(pAggInfo, p_సమాసం->iAgg);
          ఆపు;
        }
        ఒకవేళ( p_సమాసం->pAggInfo->use_క్రమీకరణ_Idx ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, pAggInfo->క్రమీకరణ_Idx_PTab,
                            pAggInfo->a_నిలువు[p_సమాసం->iAgg].i_Sorter_స్తంభం,
                            లక్ష్యం);
          లోపల_Reg = లక్ష్యం;
          ఆపు;
        }
      }
      addrINR = sqlite3_Vdbe_కూడు_Op3(v, OP_If_Null_అడ్డువరుస, p_సమాసం->i_పట్టిక, 0, లక్ష్యం);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_గుణకం = 0;   /* note (1) above */
      sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం->p_ఎడమ, లక్ష్యం);
      ప్రతిజ్ఞా( లక్ష్యం==లోపల_Reg );
      pParse->ok_Const_గుణకం = ok_Const_గుణకం;
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrINR);
      ఆపు;
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
    స్థితి TK_CASE: {
      పూర్ణాంకం చివర_Label;                     /* GOTO label for end of CASE stmt */
      పూర్ణాంకం తదుపరి_Case;                     /* GOTO label for next WHEN clause */
      పూర్ణాంకం n_సమాసం;                        /* 2x number of WHEN terms */
      పూర్ణాంకం i;                            /* Loop counter */
      సమాసం_List *pEList;                 /* List of WHEN terms */
      నిర్మాణం సమాసం_List_అంశం *aListelem;  /* Array of WHEN terms */
      సమాసం opCompare;                   /* The X==Ei expression */
      సమాసం *pX;                         /* The X expression */
      సమాసం *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      సమాసం *pDel = 0;
      sqlite3 *db = pParse->db;

      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) && p_సమాసం->x.pList!=0 );
      ప్రతిజ్ఞా(p_సమాసం->x.pList->n_సమాసం > 0);
      pEList = p_సమాసం->x.pList;
      aListelem = pEList->a;
      n_సమాసం = pEList->n_సమాసం;
      చివర_Label = sqlite3VdbeMakeLabel(pParse);
      ఒకవేళ( (pX = p_సమాసం->p_ఎడమ)!=0 ){
        pDel = sqlite3_సమాసం_Dup(db, pX, 0);
        ఒకవేళ( db->mallocFailed ){
          sqlite3_సమాసం_Delete(db, pDel);
          ఆపు;
        }
        testcase( pX->op==TK_స్తంభం );
        sqlite3_సమాసం_గమ్యం_రిజిస్టరు(pDel, సమాసం_సంకేతం_సదిశ(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, పరిమాణం(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ఎడమ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      కొరకు(i=0; i<n_సమాసం-1; i=i+2){
        ఒకవేళ( pX ){
          ప్రతిజ్ఞా( pTest!=0 );
          opCompare.p_కుడి = aListelem[i].p_సమాసం;
        }లేకపోతే{
          pTest = aListelem[i].p_సమాసం;
        }
        తదుపరి_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_స్తంభం );
        sqlite3_సమాసం_If_False(pParse, pTest, తదుపరి_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_సమాసం->op==TK_స్తంభం );
        sqlite3_సమాసం_సంకేతం(pParse, aListelem[i+1].p_సమాసం, లక్ష్యం);
        sqlite3VdbeGoto(v, చివర_Label);
        sqlite3VdbeResolveLabel(v, తదుపరి_Case);
      }
      ఒకవేళ( (n_సమాసం&1)!=0 ){
        sqlite3_సమాసం_సంకేతం(pParse, pEList->a[n_సమాసం-1].p_సమాసం, లక్ష్యం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, లక్ష్యం);
      }
      sqlite3_సమాసం_Delete(db, pDel);
      సమితి_Do_Not_విలీనం_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, చివర_Label);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
    స్థితి TK_RAISE: {
      ప్రతిజ్ఞా( p_సమాసం->aff_సమాసం==OE_Rollback
           || p_సమాసం->aff_సమాసం==OE_Abort
           || p_సమాసం->aff_సమాసం==OE_Fail
           || p_సమాసం->aff_సమాసం==OE_Ignore
      );
      ఒకవేళ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_సందేశం(pParse,
                       "RAISE() may only be used within a trigger-program");
        తిరిగి 0;
      }
      ఒకవేళ( p_సమాసం->aff_సమాసం==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      ఒకవేళ( p_సమాసం->aff_సమాసం==OE_Ignore ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
        sqlite3_Vdbe_కూడు_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_సమాసం->aff_సమాసం, r1);
      }
      ఆపు;
    }
#అయితే_ముగింపు
  }
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree1);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree2);
  తిరిగి లోపల_Reg;
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
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(
  Parse *pParse,    /* Parsing context */
  సమాసం *p_సమాసం,      /* The expression to code when the VDBE initializes */
  పూర్ణాంకం regDest       /* Store the value in this register */
){
  సమాసం_List *p;
  ప్రతిజ్ఞా( Const_గుణకం_Ok(pParse) );
  ప్రతిజ్ఞా( regDest!=0 );
  p = pParse->p_Const_సమాసం;
  ఒకవేళ( regDest<0 && p ){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం;
    పూర్ణాంకం i;
    కొరకు(p_అంశం=p->a, i=p->n_సమాసం; i>0; p_అంశం++, i--){
      ఒకవేళ( p_అంశం->fg.reusable
       && sqlite3_సమాసం_Compare(0,p_అంశం->p_సమాసం,p_సమాసం,-1)==0
      ){
        తిరిగి p_అంశం->u.i_Const_సమాసం_Reg;
      }
    }
  }
  p_సమాసం = sqlite3_సమాసం_Dup(pParse->db, p_సమాసం, 0);
  ఒకవేళ( p_సమాసం!=0 && సమాసం_Has_Property(p_సమాసం, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    పూర్ణాంకం addr;
    ప్రతిజ్ఞా( v );
    addr = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_గుణకం = 0;
    ఒకవేళ( !pParse->db->mallocFailed ){
      ఒకవేళ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం, regDest);
    }
    pParse->ok_Const_గుణకం = 1;
    sqlite3_సమాసం_Delete(pParse->db, p_సమాసం);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
  }లేకపోతే{
    p = sqlite3_సమాసం_List_Append(pParse, p, p_సమాసం);
    ఒకవేళ( p ){
       నిర్మాణం సమాసం_List_అంశం *p_అంశం = &p->a[p->n_సమాసం-1];
       p_అంశం->fg.reusable = regDest<0;
       ఒకవేళ( regDest<0 ) regDest = ++pParse->nMem;
       p_అంశం->u.i_Const_సమాసం_Reg = regDest;
    }
    pParse->p_Const_సమాసం = p;
  }
  తిరిగి regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE శూన్య sqlite3_సమాసం_Null_రిజిస్టరు_Range(
  Parse *pParse,   /* Parsing context */
  పూర్ణాంకం iReg,        /* First register to set to NULL */
  పూర్ణాంకం nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_గుణకం = pParse->ok_Const_గుణకం;
  సమాసం t;
  memset(&t, 0, పరిమాణం(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_గుణకం = 1;
  sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_గుణకం = ok_Const_గుణకం;
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
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_తాత్కాలిక(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం *pReg){
  పూర్ణాంకం r2;
  p_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం);
  ఒకవేళ( Const_గుణకం_Ok(pParse)
   && ALWAYS(p_సమాసం!=0)
   && p_సమాసం->op!=TK_రిజిస్టరు
   && sqlite3_సమాసం_Is_Constant_Not_Join(pParse, p_సమాసం)
  ){
    *pReg  = 0;
    r2 = sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(pParse, p_సమాసం, -1);
  }లేకపోతే{
    పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    r2 = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం, r1);
    ఒకవేళ( r2==r1 ){
      *pReg = r1;
    }లేకపోతే{
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  తిరిగి r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
శూన్య sqlite3_సమాసం_సంకేతం(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం లక్ష్యం){
  పూర్ణాంకం లోపల_Reg;

  ప్రతిజ్ఞా( p_సమాసం==0 || !సమాసం_Has_VVAProperty(p_సమాసం,EP_Immutable) );
  ప్రతిజ్ఞా( లక్ష్యం>0 && లక్ష్యం<=pParse->nMem );
  ప్రతిజ్ఞా( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ఒకవేళ( pParse->pVdbe==0 ) తిరిగి;
  లోపల_Reg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం, లక్ష్యం);
  ఒకవేళ( లోపల_Reg!=లక్ష్యం ){
    u8 op;
    సమాసం *pX = sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం);
    testcase( pX!=p_సమాసం );
    ఒకవేళ( ALWAYS(pX)
     && (సమాసం_Has_Property(pX,EP_Subquery) || pX->op==TK_రిజిస్టరు)
    ){
      op = OP_Copy;
    }లేకపోతే{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_కూడు_Op2(pParse->pVdbe, op, లోపల_Reg, లక్ష్యం);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
శూన్య sqlite3_సమాసం_సంకేతం_Copy(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం లక్ష్యం){
  sqlite3 *db = pParse->db;
  p_సమాసం = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
  ఒకవేళ( !db->mallocFailed ) sqlite3_సమాసం_సంకేతం(pParse, p_సమాసం, లక్ష్యం);
  sqlite3_సమాసం_Delete(db, p_సమాసం);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
శూన్య sqlite3_సమాసం_సంకేతం_Factorable(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం లక్ష్యం){
  ఒకవేళ( pParse->ok_Const_గుణకం && sqlite3_సమాసం_Is_Constant_Not_Join(pParse,p_సమాసం) ){
    sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(pParse, p_సమాసం, లక్ష్యం);
  }లేకపోతే{
    sqlite3_సమాసం_సంకేతం_Copy(pParse, p_సమాసం, లక్ష్యం);
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
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_సమాసం_List(
  Parse *pParse,     /* Parsing context */
  సమాసం_List *pList,   /* The expression list to be coded */
  పూర్ణాంకం లక్ష్యం,        /* Where to write results */
  పూర్ణాంకం srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  పూర్ణాంకం i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ప్రతిజ్ఞా( pList!=0 );
  ప్రతిజ్ఞా( లక్ష్యం>0 );
  ప్రతిజ్ఞా( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_సమాసం;
  ఒకవేళ( !Const_గుణకం_Ok(pParse) ) flags &= ~SQLITE_ECEL_గుణకం;
  కొరకు(p_అంశం=pList->a, i=0; i<n; i++, p_అంశం++){
    సమాసం *p_సమాసం = p_అంశం->p_సమాసం;
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    ఒకవేళ( p_అంశం->fg.bSorterRef ){
      i--;
      n--;
    }లేకపోతే
#అయితే_ముగింపు
    ఒకవేళ( (flags & SQLITE_ECEL_REF)!=0 && (j = p_అంశం->u.x.i_క్రమం_By_నిలువు)>0 ){
      ఒకవేళ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, copyOp, j+srcReg-1, లక్ష్యం+i);
      }
    }లేకపోతే ఒకవేళ( (flags & SQLITE_ECEL_గుణకం)!=0
           && sqlite3_సమాసం_Is_Constant_Not_Join(pParse,p_సమాసం)
    ){
      sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(pParse, p_సమాసం, లక్ష్యం+i);
    }లేకపోతే{
      పూర్ణాంకం లోపల_Reg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, p_సమాసం, లక్ష్యం+i);
      ఒకవేళ( లోపల_Reg!=లక్ష్యం+i ){
        VdbeOp *pOp;
        ఒకవేళ( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_చివరి_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==లోపల_Reg
         && pOp->p2+pOp->p3+1==లక్ష్యం+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op2(v, copyOp, లోపల_Reg, లక్ష్యం+i);
        }
      }
    }
  }
  తిరిగి n;
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
స్థిర శూన్య సమాసం_సంకేతం_Between(
  Parse *pParse,    /* Parsing and code generating context */
  సమాసం *p_సమాసం,      /* The BETWEEN expression */
  పూర్ణాంకం dest,         /* Jump destination or storage location */
  శూన్య (*xJump)(Parse*,సమాసం*,పూర్ణాంకం,పూర్ణాంకం), /* Action to take */
  పూర్ణాంకం jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  సమాసం సమాసం_And;     /* The AND operator in  x>=y AND x<=z  */
  సమాసం comp_ఎడమ;    /* The  x>=y  term */
  సమాసం comp_కుడి;   /* The  x<=z  term */
  పూర్ణాంకం regFree1 = 0; /* Temporary use register */
  సమాసం *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ఎడమ, 0, పరిమాణం(సమాసం));
  memset(&comp_కుడి, 0, పరిమాణం(సమాసం));
  memset(&సమాసం_And, 0, పరిమాణం(సమాసం));

  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  pDel = sqlite3_సమాసం_Dup(db, p_సమాసం->p_ఎడమ, 0);
  ఒకవేళ( db->mallocFailed==0 ){
    సమాసం_And.op = TK_AND;
    సమాసం_And.p_ఎడమ = &comp_ఎడమ;
    సమాసం_And.p_కుడి = &comp_కుడి;
    comp_ఎడమ.op = TK_GE;
    comp_ఎడమ.p_ఎడమ = pDel;
    comp_ఎడమ.p_కుడి = p_సమాసం->x.pList->a[0].p_సమాసం;
    comp_కుడి.op = TK_LE;
    comp_కుడి.p_ఎడమ = pDel;
    comp_కుడి.p_కుడి = p_సమాసం->x.pList->a[1].p_సమాసం;
    sqlite3_సమాసం_గమ్యం_రిజిస్టరు(pDel, సమాసం_సంకేతం_సదిశ(pParse, pDel, &regFree1));
    ఒకవేళ( xJump ){
      xJump(pParse, &సమాసం_And, dest, jumpIfNull);
    }లేకపోతే{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, &సమాసం_And, dest);
    }
    sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree1);
  }
  sqlite3_సమాసం_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_సమాసం_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_సమాసం_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_సమాసం_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_సమాసం_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_సమాసం_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_సమాసం_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_సమాసం_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_సమాసం_If_False && jumpIfNull!=0 && regFree1!=0 );
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
శూన్య sqlite3_సమాసం_If_True(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం dest, పూర్ణాంకం jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం op = 0;
  పూర్ణాంకం regFree1 = 0;
  పూర్ణాంకం regFree2 = 0;
  పూర్ణాంకం r1, r2;

  ప్రతిజ్ఞా( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ఒకవేళ( NEVER(v==0) )     తిరిగి;  /* Existence of VDBE checked by caller */
  ఒకవేళ( NEVER(p_సమాసం==0) ) తిరిగి;  /* No way this can happen */
  ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p_సమాసం, EP_Immutable) );
  op = p_సమాసం->op;
  ఎంపిక( op ){
    స్థితి TK_AND:
    స్థితి TK_OR: {
      సమాసం *pAlt = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం);
      ఒకవేళ( pAlt!=p_సమాసం ){
        sqlite3_సమాసం_If_True(pParse, pAlt, dest, jumpIfNull);
      }లేకపోతే{
        సమాసం *p_మొదటి, *p_రెండవ;
        ఒకవేళ( సమాసం_Eval_Rhs_మొదటి(p_సమాసం) ){
          p_మొదటి = p_సమాసం->p_కుడి;
          p_రెండవ = p_సమాసం->p_ఎడమ;
        }లేకపోతే{
          p_మొదటి = p_సమాసం->p_ఎడమ;
          p_రెండవ = p_సమాసం->p_కుడి;
        }
        ఒకవేళ( op==TK_AND ){
          పూర్ణాంకం d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_సమాసం_If_False(pParse, p_మొదటి, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_సమాసం_If_True(pParse, p_రెండవ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }లేకపోతే{
          testcase( jumpIfNull==0 );
          sqlite3_సమాసం_If_True(pParse, p_మొదటి, dest, jumpIfNull);
          sqlite3_సమాసం_If_True(pParse, p_రెండవ, dest, jumpIfNull);
        }
      }
      ఆపు;
    }
    స్థితి TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_సమాసం_If_False(pParse, p_సమాసం->p_ఎడమ, dest, jumpIfNull);
      ఆపు;
    }
    స్థితి TK_TRUTH: {
      పూర్ణాంకం isNot;      /* IS NOT TRUE or IS NOT FALSE */
      పూర్ణాంకం isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_సమాసం->op2==TK_ISNOT;
      isTrue = sqlite3_సమాసం_Truth_విలువ(p_సమాసం->p_కుడి);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ఒకవేళ( isTrue ^ isNot ){
        sqlite3_సమాసం_If_True(pParse, p_సమాసం->p_ఎడమ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }లేకపోతే{
        sqlite3_సమాసం_If_False(pParse, p_సమాసం->p_ఎడమ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ఆపు;
    }
    స్థితి TK_IS:
    స్థితి TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE:
    స్థితి TK_NE:
    స్థితి TK_EQ: {
      పూర్ణాంకం addrIsNull;
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_సమాసం->p_ఎడమ) ) వెళ్ళు default_సమాసం;
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = సమాసం_Compute_Operands(pParse, p_సమాసం,
                                   &r1, &r2, &regFree1, &regFree2);
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
        r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, &regFree2);
        addrIsNull = 0;
      }
      సంకేతం_Compare(pParse, p_సమాసం->p_ఎడమ, p_సమాసం->p_కుడి, op,
                  r1, r2, dest, jumpIfNull, సమాసం_Has_Property(p_సమాసం,EP_Commuted));
      ప్రతిజ్ఞా(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ప్రతిజ్ఞా(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ప్రతిజ్ఞా(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ప్రతిజ్ఞా(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ప్రతిజ్ఞా(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      ప్రతిజ్ఞా(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ఒకవేళ( addrIsNull ){
        ఒకవేళ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }లేకపోతే{
          sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIsNull);
        }
      }
      ఆపు;
    }
    స్థితి TK_ISNULL:
    స్థితి TK_NOTNULL: {
      ప్రతిజ్ఞా( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ప్రతిజ్ఞా( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
      ప్రతిజ్ఞా( regFree1==0 || regFree1==r1 );
      ఒకవేళ( regFree1 ) sqlite3_Vdbe_Typeof_స్తంభం(v, r1);
      sqlite3_Vdbe_కూడు_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ఆపు;
    }
    స్థితి TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      సమాసం_సంకేతం_Between(pParse, p_సమాసం, dest, sqlite3_సమాసం_If_True, jumpIfNull);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_లోపల: {
      పూర్ణాంకం destIfFalse = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_సమాసం_సంకేతం_లోపల(pParse, p_సమాసం, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ఆపు;
    }
#అయితే_ముగింపు
    అప్రమేయ: {
    default_సమాసం:
      ఒకవేళ( సమాసం_Always_True(p_సమాసం) ){
        sqlite3VdbeGoto(v, dest);
      }లేకపోతే ఒకవేళ( సమాసం_Always_False(p_సమాసం) ){
        /* No-op */
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం, &regFree1);
        sqlite3_Vdbe_కూడు_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ఆపు;
    }
  }
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree1);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree2); 
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
శూన్య sqlite3_సమాసం_If_False(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం dest, పూర్ణాంకం jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం op = 0;
  పూర్ణాంకం regFree1 = 0;
  పూర్ణాంకం regFree2 = 0;
  పూర్ణాంకం r1, r2;

  ప్రతిజ్ఞా( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ఒకవేళ( NEVER(v==0) ) తిరిగి; /* Existence of VDBE checked by caller */
  ఒకవేళ( p_సమాసం==0 )    తిరిగి;
  ప్రతిజ్ఞా( !సమాసం_Has_VVAProperty(p_సమాసం,EP_Immutable) );

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
  op = ((p_సమాసం->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_ISNULL || op==OP_NotNull );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_NOTNULL || op==OP_IsNull );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_NE || op==OP_Eq );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_EQ || op==OP_Ne );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_LT || op==OP_Ge );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_LE || op==OP_Gt );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_GT || op==OP_Le );
  ప్రతిజ్ఞా( p_సమాసం->op!=TK_GE || op==OP_Lt );

  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_AND:
    స్థితి TK_OR: {
      సమాసం *pAlt = sqlite3_సమాసం_Simplified_And_Or(p_సమాసం);
      ఒకవేళ( pAlt!=p_సమాసం ){
        sqlite3_సమాసం_If_False(pParse, pAlt, dest, jumpIfNull);
      }లేకపోతే{
        సమాసం *p_మొదటి, *p_రెండవ;
        ఒకవేళ( సమాసం_Eval_Rhs_మొదటి(p_సమాసం) ){
          p_మొదటి = p_సమాసం->p_కుడి;
          p_రెండవ = p_సమాసం->p_ఎడమ;
        }లేకపోతే{
          p_మొదటి = p_సమాసం->p_ఎడమ;
          p_రెండవ = p_సమాసం->p_కుడి;
        }
        ఒకవేళ( p_సమాసం->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_సమాసం_If_False(pParse, p_మొదటి, dest, jumpIfNull);
          sqlite3_సమాసం_If_False(pParse, p_రెండవ, dest, jumpIfNull);
        }లేకపోతే{
          పూర్ణాంకం d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_సమాసం_If_True(pParse, p_మొదటి, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_సమాసం_If_False(pParse, p_రెండవ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ఆపు;
    }
    స్థితి TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_సమాసం_If_True(pParse, p_సమాసం->p_ఎడమ, dest, jumpIfNull);
      ఆపు;
    }
    స్థితి TK_TRUTH: {
      పూర్ణాంకం isNot;   /* IS NOT TRUE or IS NOT FALSE */
      పూర్ణాంకం isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_సమాసం->op2==TK_ISNOT;
      isTrue = sqlite3_సమాసం_Truth_విలువ(p_సమాసం->p_కుడి);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ఒకవేళ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_సమాసం_If_False(pParse, p_సమాసం->p_ఎడమ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }లేకపోతే{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_సమాసం_If_True(pParse, p_సమాసం->p_ఎడమ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ఆపు;
    }
    స్థితి TK_IS:
    స్థితి TK_ISNOT:
      testcase( p_సమాసం->op==TK_IS );
      testcase( p_సమాసం->op==TK_ISNOT );
      op = (p_సమాసం->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE:
    స్థితి TK_NE:
    స్థితి TK_EQ: {
      పూర్ణాంకం addrIsNull;
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_సమాసం->p_ఎడమ) ) వెళ్ళు default_సమాసం;
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = సమాసం_Compute_Operands(pParse, p_సమాసం,
                                   &r1, &r2, &regFree1, &regFree2);
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
        r2 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_కుడి, &regFree2);
        addrIsNull = 0;
      }
      సంకేతం_Compare(pParse, p_సమాసం->p_ఎడమ, p_సమాసం->p_కుడి, op,
                  r1, r2, dest, jumpIfNull,సమాసం_Has_Property(p_సమాసం,EP_Commuted));
      ప్రతిజ్ఞా(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ప్రతిజ్ఞా(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ప్రతిజ్ఞా(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ప్రతిజ్ఞా(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ప్రతిజ్ఞా(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      ప్రతిజ్ఞా(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ఒకవేళ( addrIsNull ){
        ఒకవేళ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }లేకపోతే{
          sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIsNull);
        }
      }
      ఆపు;
    }
    స్థితి TK_ISNULL:
    స్థితి TK_NOTNULL: {
      r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం->p_ఎడమ, &regFree1);
      ప్రతిజ్ఞా( regFree1==0 || regFree1==r1 );
      ఒకవేళ( regFree1 ) sqlite3_Vdbe_Typeof_స్తంభం(v, r1);
      sqlite3_Vdbe_కూడు_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ఆపు;
    }
    స్థితి TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      సమాసం_సంకేతం_Between(pParse, p_సమాసం, dest, sqlite3_సమాసం_If_False, jumpIfNull);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_లోపల: {
      ఒకవేళ( jumpIfNull ){
        sqlite3_సమాసం_సంకేతం_లోపల(pParse, p_సమాసం, dest, dest);
      }లేకపోతే{
        పూర్ణాంకం destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_సమాసం_సంకేతం_లోపల(pParse, p_సమాసం, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ఆపు;
    }
#అయితే_ముగింపు
    అప్రమేయ: {
    default_సమాసం:
      ఒకవేళ( సమాసం_Always_False(p_సమాసం) ){
        sqlite3VdbeGoto(v, dest);
      }లేకపోతే ఒకవేళ( సమాసం_Always_True(p_సమాసం) ){
        /* no-op */
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, p_సమాసం, &regFree1);
        sqlite3_Vdbe_కూడు_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ఆపు;
    }
  }
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree1);
  sqlite3_Release_తాత్కాలిక_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
శూన్య sqlite3_సమాసం_If_False_Dup(Parse *pParse, సమాసం *p_సమాసం, పూర్ణాంకం dest,పూర్ణాంకం jumpIfNull){
  sqlite3 *db = pParse->db;
  సమాసం *pCopy = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
  ఒకవేళ( db->mallocFailed==0 ){
    sqlite3_సమాసం_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_సమాసం_Delete(db, pCopy);
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం సమాసం_Compare_Variable(
  స్థిరరాశి Parse *pParse,
  స్థిరరాశి సమాసం *pVar,
  స్థిరరాశి సమాసం *p_సమాసం
){
  పూర్ణాంకం res = 2;
  పూర్ణాంకం iVar;
  sqlite3_విలువ *pL, *pR = 0;
 
  ఒకవేళ( p_సమాసం->op==TK_VARIABLE && pVar->i_స్తంభం==p_సమాసం->i_స్తంభం ){
    తిరిగి 0;
  }
  ఒకవేళ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) తిరిగి 2;
  sqlite3_విలువ_From_సమాసం(pParse->db, p_సమాసం, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ఒకవేళ( pR ){
    iVar = pVar->i_స్తంభం;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_విలువ(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ఒకవేళ( pL ){
      ఒకవేళ( sqlite3_విలువ_type(pL)==SQLITE_పాఠ్యం ){
        sqlite3_విలువ_పాఠ్యం(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_విలువ_Free(pR);
    sqlite3_విలువ_Free(pL);
  }
  తిరిగి res;
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
పూర్ణాంకం sqlite3_సమాసం_Compare(
  స్థిరరాశి Parse *pParse,
  స్థిరరాశి సమాసం *pA,
  స్థిరరాశి సమాసం *pB,
  పూర్ణాంకం iTab
){
  u32 combinedFlags;
  ఒకవేళ( pA==0 || pB==0 ){
    తిరిగి pB==pA ? 0 : 2;
  }
  ఒకవేళ( pParse && pA->op==TK_VARIABLE ){
    తిరిగి సమాసం_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ఒకవేళ( combinedFlags & EP_Int_విలువ ){
    ఒకవేళ( (pA->flags&pB->flags&EP_Int_విలువ)!=0 && pA->u.i_విలువ==pB->u.i_విలువ ){
      తిరిగి 0;
    }
    తిరిగి 2;
  }
  ఒకవేళ( pA->op!=pB->op || pA->op==TK_RAISE ){
    ఒకవేళ( pA->op==TK_COLLATE && sqlite3_సమాసం_Compare(pParse, pA->p_ఎడమ,pB,iTab)<2 ){
      తిరిగి 1;
    }
    ఒకవేళ( pB->op==TK_COLLATE && sqlite3_సమాసం_Compare(pParse, pA,pB->p_ఎడమ,iTab)<2 ){
      తిరిగి 1;
    }
    ఒకవేళ( pA->op==TK_AGG_స్తంభం && pB->op==TK_స్తంభం
     && pB->i_పట్టిక<0 && pA->i_పట్టిక==iTab
    ){
      /* fall through */
    }లేకపోతే{
      తిరిగి 2;
    }
  }
  ప్రతిజ్ఞా( !సమాసం_Has_Property(pA, EP_Int_విలువ) );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(pB, EP_Int_విలువ) );
  ఒకవేళ( pA->u.zToken ){
    ఒకవేళ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ఒకవేళ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) తిరిగి 2;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ప్రతిజ్ఞా( pA->op==pB->op );
      ఒకవేళ( సమాసం_Has_Property(pA,EP_WinFunc)!=సమాసం_Has_Property(pB,EP_WinFunc) ){
        తిరిగి 2;
      }
      ఒకవేళ( సమాసం_Has_Property(pA,EP_WinFunc) ){
        ఒకవేళ( sqlite3_కిటికీ_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          తిరిగి 2;
        }
      }
#అయితే_ముగింపు
    }లేకపోతే ఒకవేళ( pA->op==TK_NULL ){
      తిరిగి 0;
    }లేకపోతే ఒకవేళ( pA->op==TK_COLLATE ){
      ఒకవేళ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) తిరిగి 2;
    }లేకపోతే
    ఒకవేళ( pB->u.zToken!=0
     && pA->op!=TK_స్తంభం
     && pA->op!=TK_AGG_స్తంభం
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      తిరిగి 2;
    }
  }
  ఒకవేళ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) తిరిగి 2;
  ఒకవేళ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ఒకవేళ( combinedFlags & EP_xIsSelect ) తిరిగి 2;
    ఒకవేళ( (combinedFlags & EP_Fixed_నిలువు)==0
     && sqlite3_సమాసం_Compare(pParse, pA->p_ఎడమ, pB->p_ఎడమ, iTab) ) తిరిగి 2;
    ఒకవేళ( sqlite3_సమాసం_Compare(pParse, pA->p_కుడి, pB->p_కుడి, iTab) ) తిరిగి 2;
    ఒకవేళ( sqlite3_సమాసం_List_Compare(pA->x.pList, pB->x.pList, iTab) ) తిరిగి 2;
    ఒకవేళ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ఒకవేళ( pA->i_స్తంభం!=pB->i_స్తంభం ) తిరిగి 2;
      ఒకవేళ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) తిరిగి 2;
      ఒకవేళ( pA->op!=TK_లోపల && pA->i_పట్టిక!=pB->i_పట్టిక && pA->i_పట్టిక!=iTab ){
        తిరిగి 2;
      }
    }
  }
  తిరిగి 0;
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
పూర్ణాంకం sqlite3_సమాసం_List_Compare(స్థిరరాశి సమాసం_List *pA, స్థిరరాశి సమాసం_List *pB, పూర్ణాంకం iTab){
  పూర్ణాంకం i;
  ఒకవేళ( pA==0 && pB==0 ) తిరిగి 0;
  ఒకవేళ( pA==0 || pB==0 ) తిరిగి 1;
  ఒకవేళ( pA->n_సమాసం!=pB->n_సమాసం ) తిరిగి 1;
  కొరకు(i=0; i<pA->n_సమాసం; i++){
    పూర్ణాంకం res;
    సమాసం *p_సమాసం_A = pA->a[i].p_సమాసం;
    సమాసం *p_సమాసం_B = pB->a[i].p_సమాసం;
    ఒకవేళ( pA->a[i].fg.క్రమీకరించు_Flags!=pB->a[i].fg.క్రమీకరించు_Flags ) తిరిగి 1;
    ఒకవేళ( (res = sqlite3_సమాసం_Compare(0, p_సమాసం_A, p_సమాసం_B, iTab)) ) తిరిగి res;
  }
  తిరిగి 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
పూర్ణాంకం sqlite3_సమాసం_Compare_Skip(సమాసం *pA,సమాసం *pB, పూర్ణాంకం iTab){
  తిరిగి sqlite3_సమాసం_Compare(0,
             sqlite3_సమాసం_Skip_Collate(pA),
             sqlite3_సమాసం_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
స్థిర పూర్ణాంకం సమాసం_సూచిస్తుంది_Not_Null(
  స్థిరరాశి Parse *pParse,/* Parsing context */
  స్థిరరాశి సమాసం *p,      /* The expression to be checked */
  స్థిరరాశి సమాసం *pNN,    /* The expression that is NOT NULL */
  పూర్ణాంకం iTab,           /* Table being evaluated */
  పూర్ణాంకం చూసిన_Not         /* Return true only if p can be any non-NULL value */
){
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( pNN );
  ఒకవేళ( sqlite3_సమాసం_Compare(pParse, p, pNN, iTab)==0 ){
    తిరిగి pNN->op!=TK_NULL;
  }
  ఎంపిక( p->op ){
    స్థితి TK_లోపల: {
      ఒకవేళ( చూసిన_Not && సమాసం_Has_Property(p, EP_xIsSelect) ) తిరిగి 0;
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_సమాసం>0) );
      తిరిగి సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_ఎడమ, pNN, iTab, 1);
    }
    స్థితి TK_BETWEEN: {
      సమాసం_List *pList;
      ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
      pList = p->x.pList;
      ప్రతిజ్ఞా( pList!=0 );
      ప్రతిజ్ఞా( pList->n_సమాసం==2 );
      ఒకవేళ( చూసిన_Not ) తిరిగి 0;
      ఒకవేళ( సమాసం_సూచిస్తుంది_Not_Null(pParse, pList->a[0].p_సమాసం, pNN, iTab, 1)
       || సమాసం_సూచిస్తుంది_Not_Null(pParse, pList->a[1].p_సమాసం, pNN, iTab, 1)
      ){
        తిరిగి 1;
      }
      తిరిగి సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_ఎడమ, pNN, iTab, 1);
    }
    స్థితి TK_EQ:
    స్థితి TK_NE:
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE:
    స్థితి TK_PLUS:
    స్థితి TK_MINUS:
    స్థితి TK_BITOR:
    స్థితి TK_LSHIFT:
    స్థితి TK_RSHIFT:
    స్థితి TK_CONCAT:
      చూసిన_Not = 1;
      /* no break */ deliberate_fall_through
    స్థితి TK_నక్షత్రం:
    స్థితి TK_REM:
    స్థితి TK_BITAND:
    స్థితి TK_SLASH: {
      ఒకవేళ( సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_కుడి, pNN, iTab, చూసిన_Not) ) తిరిగి 1;
      /* no break */ deliberate_fall_through
    }
    స్థితి TK_పరిధి:
    స్థితి TK_COLLATE:
    స్థితి TK_UPLUS:
    స్థితి TK_UMINUS: {
      తిరిగి సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_ఎడమ, pNN, iTab, చూసిన_Not);
    }
    స్థితి TK_TRUTH: {
      ఒకవేళ( చూసిన_Not ) తిరిగి 0;
      ఒకవేళ( p->op2!=TK_IS ) తిరిగి 0;
      తిరిగి సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_ఎడమ, pNN, iTab, 1);
    }
    స్థితి TK_BITNOT:
    స్థితి TK_NOT: {
      తిరిగి సమాసం_సూచిస్తుంది_Not_Null(pParse, p->p_ఎడమ, pNN, iTab, 1);
    }
  }
  తిరిగి 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
స్థిర పూర్ణాంకం sqlite3_సమాసం_Is_Not_True(సమాసం *p_సమాసం){
  పూర్ణాంకం v;
  ఒకవేళ( p_సమాసం->op==TK_NULL ) తిరిగి 1;
  ఒకవేళ( p_సమాసం->op==TK_TRUEFALSE && sqlite3_సమాసం_Truth_విలువ(p_సమాసం)==0 ) తిరిగి 1;
  v = 1;
  ఒకవేళ( sqlite3_సమాసం_Is_Integer(p_సమాసం, &v, 0, 0) && v==0 ) తిరిగి 1;
  తిరిగి 0;
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
స్థిర పూర్ణాంకం sqlite3_సమాసం_Is_IIF(sqlite3 *db, స్థిరరాశి సమాసం *p_సమాసం){
  సమాసం_List *pList;
  ఒకవేళ( p_సమాసం->op==TK_FUNCTION ){
    స్థిరరాశి అక్షరం *z = p_సమాసం->u.zToken;
    FuncDef *pDef;
    ఒకవేళ( (z[0]!='i' && z[0]!='I') ) తిరిగి 0;
    ఒకవేళ( p_సమాసం->x.pList==0 ) తిరిగి 0;
    pDef = sqlite3_కనుగొను_Function(db, z, p_సమాసం->x.pList->n_సమాసం, ENC(db), 0);
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ఒకవేళ( pDef==0 ) తిరిగి 0;
#లేకపోతే
    ఒకవేళ( NEVER(pDef==0) ) తిరిగి 0;
#అయితే_ముగింపు
    ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) తిరిగి 0;
    ఒకవేళ( SQLITE_PTR_గమ్యం_INT(pDef->p_User_దత్తాంశం)!=INLINEFUNC_iif ) తిరిగి 0;
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_CASE ){
    ఒకవేళ( p_సమాసం->p_ఎడమ!=0 ) తిరిగి 0;
  }లేకపోతే{
    తిరిగి 0;
  }
  pList = p_సమాసం->x.pList;
  ప్రతిజ్ఞా( pList!=0 );
  ఒకవేళ( pList->n_సమాసం==2 ) తిరిగి 1;
  ఒకవేళ( pList->n_సమాసం==3 && sqlite3_సమాసం_Is_Not_True(pList->a[2].p_సమాసం) ) తిరిగి 1;
  తిరిగి 0;
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
పూర్ణాంకం sqlite3_సమాసం_సూచిస్తుంది_సమాసం(
  స్థిరరాశి Parse *pParse,
  స్థిరరాశి సమాసం *pE1,
  స్థిరరాశి సమాసం *pE2,
  పూర్ణాంకం iTab
){
  ఒకవేళ( sqlite3_సమాసం_Compare(pParse, pE1, pE2, iTab)==0 ){
    తిరిగి 1;
  }
  ఒకవేళ( pE2->op==TK_OR
   && (sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse, pE1, pE2->p_ఎడమ, iTab)
             || sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse, pE1, pE2->p_కుడి, iTab) )
  ){
    తిరిగి 1;
  }
  ఒకవేళ( pE2->op==TK_NOTNULL
   && సమాసం_సూచిస్తుంది_Not_Null(pParse, pE1, pE2->p_ఎడమ, iTab, 0)
  ){
    తిరిగి 1;
  }
  ఒకవేళ( sqlite3_సమాసం_Is_IIF(pParse->db, pE1) ){
    తిరిగి sqlite3_సమాసం_సూచిస్తుంది_సమాసం(pParse,pE1->x.pList->a[0].p_సమాసం,pE2,iTab);
  }
  తిరిగి 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
స్థిర శూన్య both_Imply_Not_Null_అడ్డువరుస(Walker *pWalker, సమాసం *pE1, సమాసం *pE2){
  ఒకవేళ( pWalker->e_సంకేతం==0 ){
    sqlite3_Walk_సమాసం(pWalker, pE1);
    ఒకవేళ( pWalker->e_సంకేతం ){
      pWalker->e_సంకేతం = 0;
      sqlite3_Walk_సమాసం(pWalker, pE2);
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
స్థిర పూర్ణాంకం సూచిస్తుంది_Not_Null_అడ్డువరుస(Walker *pWalker, సమాసం *p_సమాసం){
  testcase( p_సమాసం->op==TK_AGG_స్తంభం );
  testcase( p_సమాసం->op==TK_AGG_FUNCTION );
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON) ) తిరిగి WRC_Prune;
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    తిరిగి WRC_Prune;
  }
  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_ISNOT:
    స్థితి TK_ISNULL:
    స్థితి TK_NOTNULL:
    స్థితి TK_IS:
    స్థితి TK_సదిశ:
    స్థితి TK_FUNCTION:
    స్థితి TK_TRUTH:
    స్థితి TK_CASE:
      testcase( p_సమాసం->op==TK_ISNOT );
      testcase( p_సమాసం->op==TK_ISNULL );
      testcase( p_సమాసం->op==TK_NOTNULL );
      testcase( p_సమాసం->op==TK_IS );
      testcase( p_సమాసం->op==TK_సదిశ );
      testcase( p_సమాసం->op==TK_FUNCTION );
      testcase( p_సమాసం->op==TK_TRUTH );
      testcase( p_సమాసం->op==TK_CASE );
      తిరిగి WRC_Prune;

    స్థితి TK_స్తంభం:
      ఒకవేళ( pWalker->u.iCur==p_సమాసం->i_పట్టిక ){
        pWalker->e_సంకేతం = 1;
        తిరిగి WRC_Abort;
      }
      తిరిగి WRC_Prune;

    స్థితి TK_OR:
    స్థితి TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_సమాసం->op==TK_OR );
      testcase( p_సమాసం->op==TK_AND );
      both_Imply_Not_Null_అడ్డువరుస(pWalker, p_సమాసం->p_ఎడమ, p_సమాసం->p_కుడి);
      తిరిగి WRC_Prune;
       
    స్థితి TK_లోపల:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ఒకవేళ( సమాసం_Use_XList(p_సమాసం) && ALWAYS(p_సమాసం->x.pList->n_సమాసం>0) ){
        sqlite3_Walk_సమాసం(pWalker, p_సమాసం->p_ఎడమ);
      }
      తిరిగి WRC_Prune;

    స్థితి TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->x.pList->n_సమాసం==2 );
      sqlite3_Walk_సమాసం(pWalker, p_సమాసం->p_ఎడమ);
      both_Imply_Not_Null_అడ్డువరుస(pWalker, p_సమాసం->x.pList->a[0].p_సమాసం,
                                   p_సమాసం->x.pList->a[1].p_సమాసం);
      తిరిగి WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    స్థితి TK_EQ:
    స్థితి TK_NE:
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE: {
      సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
      సమాసం *p_కుడి = p_సమాసం->p_కుడి;
      testcase( p_సమాసం->op==TK_EQ );
      testcase( p_సమాసం->op==TK_NE );
      testcase( p_సమాసం->op==TK_LT );
      testcase( p_సమాసం->op==TK_LE );
      testcase( p_సమాసం->op==TK_GT );
      testcase( p_సమాసం->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      ప్రతిజ్ఞా( p_ఎడమ->op!=TK_స్తంభం || సమాసం_Use_YTab(p_ఎడమ) );
      ప్రతిజ్ఞా( p_కుడి->op!=TK_స్తంభం || సమాసం_Use_YTab(p_కుడి) );
      ఒకవేళ( (p_ఎడమ->op==TK_స్తంభం
           && ALWAYS(p_ఎడమ->y.pTab!=0)
           && IsVirtual(p_ఎడమ->y.pTab))
       || (p_కుడి->op==TK_స్తంభం
           && ALWAYS(p_కుడి->y.pTab!=0)
           && IsVirtual(p_కుడి->y.pTab))
      ){
        తిరిగి WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    అప్రమేయ:
      తిరిగి WRC_Continue;
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
పూర్ణాంకం sqlite3_సమాసం_సూచిస్తుంది_Non_Null_అడ్డువరుస(సమాసం *p, పూర్ణాంకం iTab, పూర్ణాంకం isRJ){
  Walker w;
  p = sqlite3_సమాసం_Skip_Collate_And_Likely(p);
  ఒకవేళ( p==0 ) తిరిగి 0;
  ఒకవేళ( p->op==TK_NOTNULL ){
    p = p->p_ఎడమ;
  }లేకపోతే{
    అయితే( p->op==TK_AND ){
      ఒకవేళ( sqlite3_సమాసం_సూచిస్తుంది_Non_Null_అడ్డువరుస(p->p_ఎడమ, iTab, isRJ) ) తిరిగి 1;
      p = p->p_కుడి;
    }
  }
  w.x_సమాసం_Callback = సూచిస్తుంది_Not_Null_అడ్డువరుస;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_సంకేతం = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_సమాసం(&w, p);
  తిరిగి w.e_సంకేతం;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
నిర్మాణం IdxCover {
  సూచీ *pIdx;     /* The index to be tested for coverage */
  పూర్ణాంకం iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
స్థిర పూర్ణాంకం సమాసం_Idx_Cover(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   && p_సమాసం->i_పట్టిక==pWalker->u.pIdxCover->iCur
   && sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pWalker->u.pIdxCover->pIdx, p_సమాసం->i_స్తంభం)<0
  ){
    pWalker->e_సంకేతం = 1;
    తిరిగి WRC_Abort;
  }
  తిరిగి WRC_Continue;
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
పూర్ణాంకం sqlite3_సమాసం_Covered_By_సూచీ(
  సమాసం *p_సమాసం,        /* The index to be tested */
  పూర్ణాంకం iCur,           /* The cursor number for the corresponding table */
  సూచీ *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  నిర్మాణం IdxCover xcov;
  memset(&w, 0, పరిమాణం(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_సమాసం_Callback = సమాసం_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_సమాసం(&w, p_సమాసం);
  తిరిగి !w.e_సంకేతం;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
నిర్మాణం RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  పూర్ణాంకం *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
స్థిర పూర్ణాంకం selectRefEnter(Walker *pWalker, Select *pSelect){
  నిర్మాణం RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  పూర్ణాంకం *piNew;
  ఒకవేళ( pSrc->nSrc==0 ) తిరిగి WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*పరిమాణం(పూర్ణాంకం));
  ఒకవేళ( piNew==0 ){
    p->nExclude = 0;
    తిరిగి WRC_Abort;
  }లేకపోతే{
    p->aiExclude = piNew;
  }
  కొరకు(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  తిరిగి WRC_Continue;
}
స్థిర శూన్య selectRefLeave(Walker *pWalker, Select *pSelect){
  నిర్మాణం RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ఒకవేళ( p->nExclude ){
    ప్రతిజ్ఞా( p->nExclude>=pSrc->nSrc );
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
స్థిర పూర్ణాంకం సమాసం_Ref_గమ్యం_Src_List(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   || p_సమాసం->op==TK_AGG_స్తంభం
  ){
    పూర్ణాంకం i;
    నిర్మాణం RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    పూర్ణాంకం nSrc = pSrc ? pSrc->nSrc : 0;
    కొరకు(i=0; i<nSrc; i++){
      ఒకవేళ( p_సమాసం->i_పట్టిక==pSrc->a[i].iCursor ){
        pWalker->e_సంకేతం |= 1;
        తిరిగి WRC_Continue;
      }
    }
    కొరకు(i=0; i<p->nExclude && p->aiExclude[i]!=p_సమాసం->i_పట్టిక; i++){}
    ఒకవేళ( i>=p->nExclude ){
      pWalker->e_సంకేతం |= 2;
    }
  }
  తిరిగి WRC_Continue;
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
పూర్ణాంకం sqlite3ReferencesSrcList(Parse *pParse, సమాసం *p_సమాసం, SrcList *pSrcList){
  Walker w;
  నిర్మాణం RefSrcList x;
  ప్రతిజ్ఞా( pParse->db!=0 );
  memset(&w, 0, పరిమాణం(w));
  memset(&x, 0, పరిమాణం(x));
  w.x_సమాసం_Callback = సమాసం_Ref_గమ్యం_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ప్రతిజ్ఞా( p_సమాసం->op==TK_AGG_FUNCTION );
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  sqlite3_Walk_సమాసం_List(&w, p_సమాసం->x.pList);
  ఒకవేళ( p_సమాసం->p_ఎడమ ){
    ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ->op==TK_క్రమం );
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం->p_ఎడమ) );
    ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ->x.pList!=0 );
    sqlite3_Walk_సమాసం_List(&w, p_సమాసం->p_ఎడమ->x.pList);
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
    sqlite3_Walk_సమాసం(&w, p_సమాసం->y.pWin->pFilter);
  }
#అయితే_ముగింపు
  ఒకవేళ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ఒకవేళ( w.e_సంకేతం & 0x01 ){
    తిరిగి 1;
  }లేకపోతే ఒకవేళ( w.e_సంకేతం ){
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి -1;
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
స్థిర పూర్ణాంకం agginfo_Persist_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( ALWAYS(!సమాసం_Has_Property(p_సమాసం, EP_TokenOnly|EP_Reduced))
   && p_సమాసం->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_సమాసం->pAggInfo;
    పూర్ణాంకం iAgg = p_సమాసం->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ప్రతిజ్ఞా( iAgg>=0 );
    ఒకవేళ( p_సమాసం->op!=TK_AGG_FUNCTION ){
      ఒకవేళ( iAgg<pAggInfo->n_స్తంభం
       && pAggInfo->a_నిలువు[iAgg].pCExpr==p_సమాసం
      ){
        p_సమాసం = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
        ఒకవేళ( p_సమాసం && !sqlite3_సమాసం_Deferred_Delete(pParse, p_సమాసం) ){
          pAggInfo->a_నిలువు[iAgg].pCExpr = p_సమాసం;
        }
      }
    }లేకపోతే{
      ప్రతిజ్ఞా( p_సమాసం->op==TK_AGG_FUNCTION );
      ఒకవేళ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_సమాసం
      ){
        p_సమాసం = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
        ఒకవేళ( p_సమాసం && !sqlite3_సమాసం_Deferred_Delete(pParse, p_సమాసం) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_సమాసం;
        }
      }
    }
  }
  తిరిగి WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
శూన్య sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, పరిమాణం(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_సమాసం_Callback = agginfo_Persist_సమాసం_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
స్థిర పూర్ణాంకం కూడు_Agg_Info_స్తంభం(sqlite3 *db, AggInfo *pInfo){
  పూర్ణాంకం i;
  pInfo->a_నిలువు = sqlite3ArrayAllocate(
       db,
       pInfo->a_నిలువు,
       పరిమాణం(pInfo->a_నిలువు[0]),
       &pInfo->n_స్తంభం,
       &i
  );
  తిరిగి i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
స్థిర పూర్ణాంకం కూడు_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
  పూర్ణాంకం i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       పరిమాణం(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  తిరిగి i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
స్థిర శూన్య కనుగొను_Or_Create_Agg_Info_స్తంభం(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  సమాసం *p_సమాసం          /* Expr describing the column to find or insert */
){
  నిర్మాణం Agg_Info_నిలువు *p_నిలువు;
  పూర్ణాంకం k;
  పూర్ణాంకం mx_పదం = pParse->db->a_హద్దు[SQLITE_హద్దు_స్తంభం];

  ప్రతిజ్ఞా( mx_పదం <= SMXV(i16) );
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg==0 );
  p_నిలువు = pAggInfo->a_నిలువు;
  కొరకు(k=0; k<pAggInfo->n_స్తంభం; k++, p_నిలువు++){
    ఒకవేళ( p_నిలువు->pCExpr==p_సమాసం ) తిరిగి;
    ఒకవేళ( p_నిలువు->i_పట్టిక==p_సమాసం->i_పట్టిక
     && p_నిలువు->i_స్తంభం==p_సమాసం->i_స్తంభం
     && p_సమాసం->op!=TK_IF_NULL_అడ్డువరుస
    ){
      వెళ్ళు fix_పైకి_సమాసం;
    }
  }
  k = కూడు_Agg_Info_స్తంభం(pParse->db, pAggInfo);
  ఒకవేళ( k<0 ){
    /* OOM on resize */
    ప్రతిజ్ఞా( pParse->db->mallocFailed );
    తిరిగి;
  }
  ఒకవేళ( k>mx_పదం ){
    sqlite3_Error_సందేశం(pParse, "more than %d aggregate terms", mx_పదం);
    k = mx_పదం;
  }
  p_నిలువు = &pAggInfo->a_నిలువు[k];
  ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
  p_నిలువు->pTab = p_సమాసం->y.pTab;
  p_నిలువు->i_పట్టిక = p_సమాసం->i_పట్టిక;
  p_నిలువు->i_స్తంభం = p_సమాసం->i_స్తంభం;
  p_నిలువు->i_Sorter_స్తంభం = -1;
  p_నిలువు->pCExpr = p_సమాసం;
  ఒకవేళ( pAggInfo->p_సమూహం_By && p_సమాసం->op!=TK_IF_NULL_అడ్డువరుస ){
    పూర్ణాంకం j, n;
    సమాసం_List *pGB = pAggInfo->p_సమూహం_By;
    నిర్మాణం సమాసం_List_అంశం *p_పదం = pGB->a;
    n = pGB->n_సమాసం;
    కొరకు(j=0; j<n; j++, p_పదం++){
      సమాసం *pE = p_పదం->p_సమాసం;
      ఒకవేళ( pE->op==TK_స్తంభం
       && pE->i_పట్టిక==p_సమాసం->i_పట్టిక
       && pE->i_స్తంభం==p_సమాసం->i_స్తంభం
      ){
        p_నిలువు->i_Sorter_స్తంభం = j;
        ఆపు;
      }
    }
  }
  ఒకవేళ( p_నిలువు->i_Sorter_స్తంభం<0 ){
    p_నిలువు->i_Sorter_స్తంభం = pAggInfo->n_క్రమీకరణ_స్తంభం++;
  }
fix_పైకి_సమాసం:
  సమాసం_సమితి_VVAProperty(p_సమాసం, EP_NoReduce);
  ప్రతిజ్ఞా( p_సమాసం->pAggInfo==0 || p_సమాసం->pAggInfo==pAggInfo );
  p_సమాసం->pAggInfo = pAggInfo;
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం ){
    p_సమాసం->op = TK_AGG_స్తంభం;
  }
  ప్రతిజ్ఞా( k <= SMXV(p_సమాసం->iAgg) );
  p_సమాసం->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
స్థిర పూర్ణాంకం analyzeAggregate(Walker *pWalker, సమాసం *p_సమాసం){
  పూర్ణాంకం i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ప్రతిజ్ఞా( pNC->ncFlags & NC_UAggInfo );
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg==0 );
  ఎంపిక( p_సమాసం->op ){
    అప్రమేయ: {
      సూచీకృత_సమాసం *pIEpr;
      సమాసం tmp;
      ప్రతిజ్ఞా( pParse->iSelfTab==0 );
      ఒకవేళ( (pNC->ncFlags & NC_లోపల_Agg_Func)==0 ) ఆపు;
      ఒకవేళ( pParse->pIdxEpr==0 ) ఆపు;
      కొరకు(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        పూర్ణాంకం i_దత్తాంశం_Cur = pIEpr->i_దత్తాంశం_Cur;
        ఒకవేళ( i_దత్తాంశం_Cur<0 ) కొనసాగు;
        ఒకవేళ( sqlite3_సమాసం_Compare(0, p_సమాసం, pIEpr->p_సమాసం, i_దత్తాంశం_Cur)==0 ) ఆపు;
      }
      ఒకవేళ( pIEpr==0 ) ఆపు;
      ఒకవేళ( NEVER(!సమాసం_Use_YTab(p_సమాసం)) ) ఆపు;
      కొరకు(i=0; i<pSrcList->nSrc; i++){
         ఒకవేళ( pSrcList->a[i].iCursor==pIEpr->i_దత్తాంశం_Cur ){
           testcase( i>0 );
           ఆపు;
         }
      }
      ఒకవేళ( i>=pSrcList->nSrc ) ఆపు;
      ఒకవేళ( NEVER(p_సమాసం->pAggInfo!=0) ) ఆపు; /* Resolved by outer context */
      ఒకవేళ( pParse->nErr ){ తిరిగి WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, పరిమాణం(tmp));
      tmp.op = TK_AGG_స్తంభం;
      tmp.i_పట్టిక = pIEpr->iIdxCur;
      tmp.i_స్తంభం = pIEpr->i_Idx_నిలువు;
      కనుగొను_Or_Create_Agg_Info_స్తంభం(pParse, pAggInfo, &tmp);
      ఒకవేళ( pParse->nErr ){ తిరిగి WRC_Abort; }
      ప్రతిజ్ఞా( pAggInfo->a_నిలువు!=0 );
      ప్రతిజ్ఞా( tmp.iAgg<pAggInfo->n_స్తంభం );
      pAggInfo->a_నిలువు[tmp.iAgg].pCExpr = p_సమాసం;
      p_సమాసం->pAggInfo = pAggInfo;
      p_సమాసం->iAgg = tmp.iAgg;
      తిరిగి WRC_Prune;
    }
    స్థితి TK_IF_NULL_అడ్డువరుస:
    స్థితి TK_AGG_స్తంభం:
    స్థితి TK_స్తంభం: {
      testcase( p_సమాసం->op==TK_AGG_స్తంభం );
      testcase( p_సమాసం->op==TK_స్తంభం );
      testcase( p_సమాసం->op==TK_IF_NULL_అడ్డువరుస );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ఒకవేళ( ALWAYS(pSrcList!=0) ){
        Src_అంశం *p_అంశం = pSrcList->a;
        కొరకు(i=0; i<pSrcList->nSrc; i++, p_అంశం++){
          ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_TokenOnly|EP_Reduced) );
          ఒకవేళ( p_సమాసం->i_పట్టిక==p_అంశం->iCursor ){
            కనుగొను_Or_Create_Agg_Info_స్తంభం(pParse, pAggInfo, p_సమాసం);
            ఆపు;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      తిరిగి WRC_Continue;
    }
    స్థితి TK_AGG_FUNCTION: {
      ఒకవేళ( (pNC->ncFlags & NC_లోపల_Agg_Func)==0
       && pWalker->walker_లోతు==p_సమాసం->op2
       && p_సమాసం->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        నిర్మాణం AggInfo_func *p_అంశం = pAggInfo->aFunc;
        పూర్ణాంకం mx_పదం = pParse->db->a_హద్దు[SQLITE_హద్దు_స్తంభం];
        ప్రతిజ్ఞా( mx_పదం <= SMXV(i16) );
        కొరకు(i=0; i<pAggInfo->nFunc; i++, p_అంశం++){
          ఒకవేళ( NEVER(p_అంశం->pFExpr==p_సమాసం) ) ఆపు;
          ఒకవేళ( sqlite3_సమాసం_Compare(0, p_అంశం->pFExpr, p_సమాసం, -1)==0 ){
            ఆపు;
          }
        }
        ఒకవేళ( i>mx_పదం ){
          sqlite3_Error_సందేశం(pParse, "more than %d aggregate terms", mx_పదం);
          i = mx_పదం;
          ప్రతిజ్ఞా( i<pAggInfo->nFunc );
        }లేకపోతే ఒకవేళ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = కూడు_Agg_Info_Func(pParse->db, pAggInfo);
          ఒకవేళ( i>=0 ){
            పూర్ణాంకం nArg;
            ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_xIsSelect) );
            p_అంశం = &pAggInfo->aFunc[i];
            p_అంశం->pFExpr = p_సమాసం;
            ప్రతిజ్ఞా( సమాసం_Use_UToken(p_సమాసం) );
            nArg = p_సమాసం->x.pList ? p_సమాసం->x.pList->n_సమాసం : 0;
            p_అంశం->pFunc = sqlite3_కనుగొను_Function(pParse->db,
                                         p_సమాసం->u.zToken, nArg, enc, 0);
            ప్రతిజ్ఞా( p_అంశం->bOBUnique==0 );
            ఒకవేళ( p_సమాసం->p_ఎడమ
             && (p_అంశం->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              సమాసం_List *pOBList;
              ప్రతిజ్ఞా( nArg>0 );
              ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ->op==TK_క్రమం );
              ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం->p_ఎడమ) );
              p_అంశం->iOBTab = pParse->nTab++;
              pOBList = p_సమాసం->p_ఎడమ->x.pList;
              ప్రతిజ్ఞా( pOBList->n_సమాసం>0 );
              ప్రతిజ్ఞా( p_అంశం->bOBUnique==0 );
              ఒకవేళ( pOBList->n_సమాసం==1
               && nArg==1
               && sqlite3_సమాసం_Compare(0,pOBList->a[0].p_సమాసం,
                               p_సమాసం->x.pList->a[0].p_సమాసం,-1)==0
              ){
                p_అంశం->bOBPayload = 0;
                p_అంశం->bOBUnique = సమాసం_Has_Property(p_సమాసం, EP_Distinct);
              }లేకపోతే{
                p_అంశం->bOBPayload = 1;
              }
              p_అంశం->bUseSubtype =
                    (p_అంశం->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }లేకపోతే{
              p_అంశం->iOBTab = -1;
            }
            ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Distinct) && !p_అంశం->bOBUnique ){
              p_అంశం->iDistinct = pParse->nTab++;
            }లేకపోతే{
              p_అంశం->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_TokenOnly|EP_Reduced) );
        సమాసం_సమితి_VVAProperty(p_సమాసం, EP_NoReduce);
        ప్రతిజ్ఞా( i <= SMXV(p_సమాసం->iAgg) );
        p_సమాసం->iAgg = (i16)i;
        p_సమాసం->pAggInfo = pAggInfo;
        తిరిగి WRC_Prune;
      }లేకపోతే{
        తిరిగి WRC_Continue;
      }
    }
  }
  తిరిగి WRC_Continue;
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
శూన్య sqlite3_సమాసం_Analyze_Aggregates(NameContext *pNC, సమాసం *p_సమాసం){
  Walker w;
  w.x_సమాసం_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_లోతు_Increase;
  w.xSelectCallback2 = sqlite3_Walker_లోతు_Decrease;
  w.walker_లోతు = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  ప్రతిజ్ఞా( pNC->pSrcList!=0 );
  sqlite3_Walk_సమాసం(&w, p_సమాసం);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
శూన్య sqlite3_సమాసం_Analyze_Agg_List(NameContext *pNC, సమాసం_List *pList){
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  పూర్ణాంకం i;
  ఒకవేళ( pList ){
    కొరకు(p_అంశం=pList->a, i=0; i<pList->n_సమాసం; i++, p_అంశం++){
      sqlite3_సమాసం_Analyze_Aggregates(pNC, p_అంశం->p_సమాసం);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
పూర్ణాంకం sqlite3_Get_తాత్కాలిక_Reg(Parse *pParse){
  ఒకవేళ( pParse->n_తాత్కాలిక_Reg==0 ){
    తిరిగి ++pParse->nMem;
  }
  తిరిగి pParse->a_తాత్కాలిక_Reg[--pParse->n_తాత్కాలిక_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
శూన్య sqlite3_Release_తాత్కాలిక_Reg(Parse *pParse, పూర్ణాంకం iReg){
  ఒకవేళ( iReg ){
#నిర్వచితమైతే SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    పూర్ణాంకం kk;
    కొరకు(kk=0; kk<pParse->n_తాత్కాలిక_Reg; kk++){
      ప్రతిజ్ఞా( pParse->a_తాత్కాలిక_Reg[kk]!=iReg );
    }
#అయితే_ముగింపు
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ఒకవేళ( pParse->n_తాత్కాలిక_Reg<Array_కొలత(pParse->a_తాత్కాలిక_Reg) ){
      pParse->a_తాత్కాలిక_Reg[pParse->n_తాత్కాలిక_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
పూర్ణాంకం sqlite3_Get_తాత్కాలిక_Range(Parse *pParse, పూర్ణాంకం nReg){
  పూర్ణాంకం i, n;
  ఒకవేళ( nReg==1 ) తిరిగి sqlite3_Get_తాత్కాలిక_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ఒకవేళ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }లేకపోతే{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  తిరిగి i;
}
శూన్య sqlite3_Release_తాత్కాలిక_Range(Parse *pParse, పూర్ణాంకం iReg, పూర్ణాంకం nReg){
  ఒకవేళ( nReg==1 ){
    sqlite3_Release_తాత్కాలిక_Reg(pParse, iReg);
    తిరిగి;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ఒకవేళ( nReg>pParse->nRangeReg ){
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
శూన్య sqlite3_Clear_తాత్కాలిక_Reg_Cache(Parse *pParse){
  pParse->n_తాత్కాలిక_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
శూన్య sqlite3_Touch_రిజిస్టరు(Parse *pParse, పూర్ణాంకం iReg){
  ఒకవేళ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ఒకవేళ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
పూర్ణాంకం sqlite3_మొదటి_అందుబాటు_రిజిస్టరు(Parse *pParse, పూర్ణాంకం iMin){
  స్థిరరాశి సమాసం_List *pList = pParse->p_Const_సమాసం;
  ఒకవేళ( pList ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pList->n_సమాసం; i++){
      ఒకవేళ( pList->a[i].u.i_Const_సమాసం_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_సమాసం_Reg + 1;
      }
    }
  }
  pParse->n_తాత్కాలిక_Reg = 0;
  pParse->nRangeReg = 0;
  తిరిగి iMin;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_No_Temps_లోపల_Range(Parse *pParse, పూర్ణాంకం i_మొదటి, పూర్ణాంకం i_చివరి){
  పూర్ణాంకం i;
  ఒకవేళ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_మొదటి
   && pParse->iRangeReg <= i_చివరి
  ){
     తిరిగి 0;
  }
  కొరకు(i=0; i<pParse->n_తాత్కాలిక_Reg; i++){
    ఒకవేళ( pParse->a_తాత్కాలిక_Reg[i]>=i_మొదటి && pParse->a_తాత్కాలిక_Reg[i]<=i_చివరి ){
      తిరిగి 0;
    }
  }
  ఒకవేళ( pParse->p_Const_సమాసం ){
    సమాసం_List *pList = pParse->p_Const_సమాసం;
    కొరకు(i=0; i<pList->n_సమాసం; i++){
      పూర్ణాంకం iReg = pList->a[i].u.i_Const_సమాసం_Reg;
      ఒకవేళ( iReg==0 ) కొనసాగు;
      ఒకవేళ( iReg>=i_మొదటి && iReg<=i_చివరి ) తిరిగి 0;
    }
  }
  తిరిగి 1;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */
