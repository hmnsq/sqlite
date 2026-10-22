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
#અન્તર્ભાવ "sqliteInt.h"

/* Forward declarations */
સ્થિર શૂન્ય પદાવલિ_Code_Between(Parse*,પદાવલિ*,પૂર્ણાંક,શૂન્ય(*)(Parse*,પદાવલિ*,પૂર્ણાંક,પૂર્ણાંક),પૂર્ણાંક);
સ્થિર પૂર્ણાંક પદાવલિ_Code_Vector(Parse *pParse, પદાવલિ *p, પૂર્ણાંક *piToFree);

/*
** Return the affinity character for a single column of a table.
*/
અક્ષર sqlite3TableColumnAffinity(અચલ Table *pTab, પૂર્ણાંક iCol){
  જો( iCol<0 || NEVER(iCol>=pTab->nCol) ) પાછા SQLITE_AFF_INTEGER;
  પાછા pTab->aCol[iCol].affinity;
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
અક્ષર sqlite3_પદાવલિ_Affinity(અચલ પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક op;
  op = p_પદાવલિ->op;
  જ્યારે( 1 /* exit-by-break */ ){
    જો( op==TK_COLUMN || (op==TK_AGG_COLUMN && p_પદાવલિ->y.pTab!=0) ){
      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->y.pTab!=0 );
      પાછા sqlite3TableColumnAffinity(p_પદાવલિ->y.pTab, p_પદાવલિ->iColumn);
    }
    જો( op==TK_SELECT ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect!=0 );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect->pEList!=0 );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect->pEList->a[0].p_પદાવલિ!=0 );
      પાછા sqlite3_પદાવલિ_Affinity(p_પદાવલિ->x.pSelect->pEList->a[0].p_પદાવલિ);
    }
#ifndef SQLITE_OMIT_CAST
    જો( op==TK_CAST ){
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      પાછા sqlite3AffinityType(p_પદાવલિ->u.zToken, 0);
    }
#endif
    જો( op==TK_SELECT_COLUMN ){
      પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું!=0 && પદાવલિ_Use_XSelect(p_પદાવલિ->p_ડાબું) );
      પ્રતિજ્ઞા( p_પદાવલિ->iColumn < p_પદાવલિ->iTable );
      પ્રતિજ્ઞા( p_પદાવલિ->iColumn >= 0 );
      પ્રતિજ્ઞા( p_પદાવલિ->iTable==p_પદાવલિ->p_ડાબું->x.pSelect->pEList->n_પદાવલિ );
      પાછા sqlite3_પદાવલિ_Affinity(
          p_પદાવલિ->p_ડાબું->x.pSelect->pEList->a[p_પદાવલિ->iColumn].p_પદાવલિ
      );
    }
    જો( op==TK_VECTOR
     || (op==TK_FUNCTION && p_પદાવલિ->aff_પદાવલિ==SQLITE_AFF_DEFER)
    ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      પાછા sqlite3_પદાવલિ_Affinity(p_પદાવલિ->x.pList->a[0].p_પદાવલિ);
    }
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Skip|EP_IfNullRow) ){
      પ્રતિજ્ઞા( p_પદાવલિ->op==TK_COLLATE
           || p_પદાવલિ->op==TK_IF_NULL_ROW
           || (p_પદાવલિ->op==TK_REGISTER && p_પદાવલિ->op2==TK_IF_NULL_ROW) );
      p_પદાવલિ = p_પદાવલિ->p_ડાબું;
      op = p_પદાવલિ->op;
      ચાલુ;
    }
    જો( op!=TK_REGISTER ) તોડો;
    op = p_પદાવલિ->op2;
    જો( NEVER( op==TK_REGISTER ) ) તોડો;
  }
  પાછા p_પદાવલિ->aff_પદાવલિ;
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
પૂર્ણાંક sqlite3_પદાવલિ_Data_Type(અચલ પદાવલિ *p_પદાવલિ){
  જ્યારે( p_પદાવલિ ){
    પસંદગી( p_પદાવલિ->op ){
      સ્થિતિ TK_COLLATE:
      સ્થિતિ TK_IF_NULL_ROW:
      સ્થિતિ TK_UPLUS:  {
        p_પદાવલિ = p_પદાવલિ->p_ડાબું;
        તોડો;
      }
      સ્થિતિ TK_NULL: {
        p_પદાવલિ = 0;
        તોડો;
      }
      સ્થિતિ TK_STRING: {
        પાછા 0x02;
      }
      સ્થિતિ TK_BLOB: {
        પાછા 0x04;
      }
      સ્થિતિ TK_CONCAT: {
        પાછા 0x06;
      }
      સ્થિતિ TK_VARIABLE:
      સ્થિતિ TK_AGG_FUNCTION:
      સ્થિતિ TK_FUNCTION: {
        પાછા 0x07;
      }
      સ્થિતિ TK_COLUMN:
      સ્થિતિ TK_AGG_COLUMN:
      સ્થિતિ TK_SELECT:
      સ્થિતિ TK_CAST:
      સ્થિતિ TK_SELECT_COLUMN:
      સ્થિતિ TK_VECTOR:  {
        પૂર્ણાંક aff = sqlite3_પદાવલિ_Affinity(p_પદાવલિ);
        જો( aff>=SQLITE_AFF_NUMERIC ) પાછા 0x05;
        જો( aff==SQLITE_AFF_TEXT )    પાછા 0x06;
        પાછા 0x07;
      }
      સ્થિતિ TK_CASE: {
        પૂર્ણાંક res = 0;
        પૂર્ણાંક ii;
        પદાવલિ_List *pList = p_પદાવલિ->x.pList;
        પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) && pList!=0 );
        પ્રતિજ્ઞા( pList->n_પદાવલિ > 0);
        માટે(ii=1; ii<pList->n_પદાવલિ; ii+=2){
          res |= sqlite3_પદાવલિ_Data_Type(pList->a[ii].p_પદાવલિ);
        }
        જો( pList->n_પદાવલિ % 2 ){
          res |= sqlite3_પદાવલિ_Data_Type(pList->a[pList->n_પદાવલિ-1].p_પદાવલિ);
        }
        પાછા res;
      }
      મૂળભૂત: {
        પાછા 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  પાછા 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
પદાવલિ *sqlite3_પદાવલિ_Add_Collate_Token(
  Parse *pParse,           /* Parsing context */
  પદાવલિ *p_પદાવલિ,             /* Add the "COLLATE" clause to this expression */
  અચલ Token *pCollName,  /* Name of collating sequence */
  પૂર્ણાંક dequote              /* True to dequote pCollName */
){
  જો( pCollName->n>0 && pParse->nErr==0 ){
    પદાવલિ *pNew = sqlite3_પદાવલિ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    પ્રતિજ્ઞા( p_પદાવલિ!=0 );
    જો( pNew ){
      pNew->p_ડાબું = p_પદાવલિ;
      pNew->flags |= EP_Collate|EP_Skip;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
      pNew->nHeight = p_પદાવલિ->nHeight+1;
      sqlite3_પદાવલિ_Check_Height(pParse, pNew->nHeight);
      testcase( pParse->nErr>0 );
#endif
      p_પદાવલિ = pNew;
    }
  }
  પાછા p_પદાવલિ;
}
પદાવલિ *sqlite3_પદાવલિ_Add_Collate_String(
  Parse *pParse,        /* Parsing context */
  પદાવલિ *p_પદાવલિ,          /* Add the "COLLATE" clause to this expression */
  અચલ અક્ષર *zC        /* The collating sequence name */
){
  Token s;
  પ્રતિજ્ઞા( zC!=0 );
  sqlite3TokenInit(&s, (અક્ષર*)zC);
  પાછા sqlite3_પદાવલિ_Add_Collate_Token(pParse, p_પદાવલિ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
પદાવલિ *sqlite3_પદાવલિ_Skip_Collate(પદાવલિ *p_પદાવલિ){
  જ્યારે( p_પદાવલિ && પદાવલિ_Has_Property(p_પદાવલિ, EP_Skip) ){
    પ્રતિજ્ઞા( p_પદાવલિ->op==TK_COLLATE );
    p_પદાવલિ = p_પદાવલિ->p_ડાબું;
  }  
  પાછા p_પદાવલિ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
પદાવલિ *sqlite3_પદાવલિ_Skip_Collate_And_Likely(પદાવલિ *p_પદાવલિ){
  જ્યારે( p_પદાવલિ && પદાવલિ_Has_Property(p_પદાવલિ, EP_Skip|EP_Unlikely) ){
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Unlikely) ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pList->n_પદાવલિ>0 );
      પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION );
      p_પદાવલિ = p_પદાવલિ->x.pList->a[0].p_પદાવલિ;
    }નહીંતર જો( p_પદાવલિ->op==TK_COLLATE ){
      p_પદાવલિ = p_પદાવલિ->p_ડાબું;
    }નહીંતર{
      તોડો;
    }
  }  
  પાછા p_પદાવલિ;
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
CollSeq *sqlite3_પદાવલિ_Coll_Seq(Parse *pParse, અચલ પદાવલિ *p_પદાવલિ){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  અચલ પદાવલિ *p = p_પદાવલિ;
  જ્યારે( p ){
    પૂર્ણાંક op = p->op;
    જો( op==TK_REGISTER ) op = p->op2;
    જો( (op==TK_AGG_COLUMN && p->y.pTab!=0)
     || op==TK_COLUMN || op==TK_TRIGGER
    ){
      પૂર્ણાંક j;
      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p) );
      પ્રતિજ્ઞા( p->y.pTab!=0 );
      જો( (j = p->iColumn)>=0 ){
        અચલ અક્ષર *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
      }
      તોડો;
    }
    જો( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ડાબું;
      ચાલુ;
    }
    જો( op==TK_VECTOR
     || (op==TK_FUNCTION && p->aff_પદાવલિ==SQLITE_AFF_DEFER)
    ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p) );
      p = p->x.pList->a[0].p_પદાવલિ;
      ચાલુ;
    }
    જો( op==TK_COLLATE ){
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_Int_કિંમત) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      તોડો;
    }
    જો( p->flags & EP_Collate ){
      જો( p->p_ડાબું && (p->p_ડાબું->flags & EP_Collate)!=0 ){
        p = p->p_ડાબું;
      }નહીંતર{
        પદાવલિ *pNext  = p->p_જમણું;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        પ્રતિજ્ઞા( !પદાવલિ_Use_XList(p) || p->x.pList==0 || p->p_જમણું==0 );
        જો( પદાવલિ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          પૂર્ણાંક i;
          માટે(i=0; i<p->x.pList->n_પદાવલિ; i++){
            જો( પદાવલિ_Has_Property(p->x.pList->a[i].p_પદાવલિ, EP_Collate) ){
              pNext = p->x.pList->a[i].p_પદાવલિ;
              તોડો;
            }
          }
        }
        p = pNext;
      }
    }નહીંતર{
      તોડો;
    }
  }
  જો( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  પાછા pColl;
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
CollSeq *sqlite3_પદાવલિ_NNColl_Seq(Parse *pParse, અચલ પદાવલિ *p_પદાવલિ){
  CollSeq *p = sqlite3_પદાવલિ_Coll_Seq(pParse, p_પદાવલિ);
  જો( p==0 ) p = pParse->db->pDfltColl;
  પ્રતિજ્ઞા( p!=0 );
  પાછા p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Coll_Seq_Match(Parse *pParse, અચલ પદાવલિ *pE1, અચલ પદાવલિ *pE2){
  CollSeq *pColl1 = sqlite3_પદાવલિ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_પદાવલિ_NNColl_Seq(pParse, pE2);
  પ્રતિજ્ઞા( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  પાછા pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
અક્ષર sqlite3CompareAffinity(અચલ પદાવલિ *p_પદાવલિ, અક્ષર aff2){
  અક્ષર aff1 = sqlite3_પદાવલિ_Affinity(p_પદાવલિ);
  જો( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    જો( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      પાછા SQLITE_AFF_NUMERIC;
    }નહીંતર{
      પાછા SQLITE_AFF_BLOB;
    }
  }નહીંતર{
    /* One side is a column, the other is not. Use the columns affinity. */
    પ્રતિજ્ઞા( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    પાછા (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
સ્થિર અક્ષર comparisonAffinity(અચલ પદાવલિ *p_પદાવલિ){
  અક્ષર aff;
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_EQ || p_પદાવલિ->op==TK_IN || p_પદાવલિ->op==TK_LT ||
          p_પદાવલિ->op==TK_GT || p_પદાવલિ->op==TK_GE || p_પદાવલિ->op==TK_LE ||
          p_પદાવલિ->op==TK_NE || p_પદાવલિ->op==TK_IS || p_પદાવલિ->op==TK_ISNOT );
  પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું );
  પ્રતિજ્ઞા( !પદાવલિ_Use_XSelect(p_પદાવલિ) );
  aff = sqlite3_પદાવલિ_Affinity(p_પદાવલિ->p_ડાબું);
  જો( p_પદાવલિ->p_જમણું ){
    aff = sqlite3CompareAffinity(p_પદાવલિ->p_જમણું, aff);
  }નહીંતર જો( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  પાછા aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
પૂર્ણાંક sqlite3IndexAffinityOk(અચલ પદાવલિ *p_પદાવલિ, અક્ષર idx_affinity){
  અક્ષર aff = comparisonAffinity(p_પદાવલિ);
  જો( aff<SQLITE_AFF_TEXT ){
    પાછા 1;
  }
  જો( aff==SQLITE_AFF_TEXT ){
    પાછા idx_affinity==SQLITE_AFF_TEXT;
  }
  પાછા sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
સ્થિર u8 દ્વિઅંકી_Compare_P5(
  અચલ પદાવલિ *pExpr1,   /* Left operand */
  અચલ પદાવલિ *pExpr2,   /* Right operand */
  પૂર્ણાંક jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (અક્ષર)sqlite3_પદાવલિ_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  પાછા aff;
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
CollSeq *sqlite3_દ્વિઅંકી_Compare_Coll_Seq(
  Parse *pParse,
  અચલ પદાવલિ *p_ડાબું,
  અચલ પદાવલિ *p_જમણું
){
  CollSeq *pColl;
  પ્રતિજ્ઞા( p_ડાબું );
  જો( p_ડાબું->flags & EP_Collate ){
    pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p_ડાબું);
  }નહીંતર જો( p_જમણું && (p_જમણું->flags & EP_Collate)!=0 ){
    pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p_જમણું);
  }નહીંતર{
    pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p_ડાબું);
    જો( !pColl ){
      pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p_જમણું);
    }
  }
  પાછા pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_પદાવલિ_Compare_Coll_Seq(Parse *pParse, અચલ પદાવલિ *p){
  જો( પદાવલિ_Has_Property(p, EP_Commuted) ){
    પાછા sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, p->p_જમણું, p->p_ડાબું);
  }નહીંતર{
    પાછા sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, p->p_ડાબું, p->p_જમણું);
  }
}

/*
** Generate code for a comparison operator.
*/
સ્થિર પૂર્ણાંક codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  પદાવલિ *p_ડાબું,      /* The left operand */
  પદાવલિ *p_જમણું,     /* The right operand */
  પૂર્ણાંક opcode,       /* The comparison opcode */
  પૂર્ણાંક in1, પૂર્ણાંક in2, /* Register holding operands */
  પૂર્ણાંક dest,         /* Jump here if true.  */
  પૂર્ણાંક jumpIfNull,   /* If true, jump if either operand is NULL */
  પૂર્ણાંક isCommuted    /* The comparison has been commuted */
){
  પૂર્ણાંક p5;
  પૂર્ણાંક addr;
  CollSeq *p4;

  જો( pParse->nErr ) પાછા 0;
  જો( isCommuted ){
    p4 = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, p_જમણું, p_ડાબું);
  }નહીંતર{
    p4 = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, p_ડાબું, p_જમણું);
  }
  p5 = દ્વિઅંકી_Compare_P5(p_ડાબું, p_જમણું, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (શૂન્ય*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  પાછા addr;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Vector(અચલ પદાવલિ *p_પદાવલિ){
  પાછા sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Vector_Size(અચલ પદાવલિ *p_પદાવલિ){
  u8 op = p_પદાવલિ->op;
  જો( op==TK_REGISTER ) op = p_પદાવલિ->op2;
  જો( op==TK_VECTOR ){
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
    પાછા p_પદાવલિ->x.pList->n_પદાવલિ;
  }નહીંતર જો( op==TK_SELECT ){
    પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
    પાછા p_પદાવલિ->x.pSelect->pEList->n_પદાવલિ;
  }નહીંતર{
    પાછા 1;
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
પદાવલિ *sqlite3VectorFieldSubexpr(પદાવલિ *pVector, પૂર્ણાંક i){
  પ્રતિજ્ઞા( i<sqlite3_પદાવલિ_Vector_Size(pVector) || pVector->op==TK_ERROR );
  જો( sqlite3_પદાવલિ_Is_Vector(pVector) ){
    પ્રતિજ્ઞા( pVector->op2==0 || pVector->op==TK_REGISTER );
    જો( pVector->op==TK_SELECT || pVector->op2==TK_SELECT ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(pVector) );
      પાછા pVector->x.pSelect->pEList->a[i].p_પદાવલિ;
    }નહીંતર{
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pVector) );
      પાછા pVector->x.pList->a[i].p_પદાવલિ;
    }
  }
  પાછા pVector;
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
પદાવલિ *sqlite3_પદાવલિ_For_Vector_Field(
  Parse *pParse,       /* Parsing context */
  પદાવલિ *pVector,       /* The vector.  List of expressions or a sub-SELECT */
  પૂર્ણાંક iField,          /* Which column of the vector to return */
  પૂર્ણાંક nField           /* Total number of columns in the vector */
){
  પદાવલિ *pRet;
  જો( pVector->op==TK_SELECT ){
    પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(pVector) );
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
    જો( pRet ){
      પદાવલિ_Set_Property(pRet, EP_FullSize);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->p_ડાબું = pVector;
    }
  }નહીંતર{
    જો( pVector->op==TK_VECTOR ){
      પદાવલિ **ppVector;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pVector) );
      ppVector = &pVector->x.pList->a[iField].p_પદાવલિ;
      pVector = *ppVector;
      જો( IN_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *ppVector = 0;
        પાછા pVector;
      }
    }
    pRet = sqlite3_પદાવલિ_Dup(pParse->db, pVector, 0);
  }
  પાછા pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Code_Subselect(Parse *pParse, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક reg = 0;
#ifndef SQLITE_OMIT_SUBQUERY
  જો( p_પદાવલિ->op==TK_SELECT ){
    reg = sqlite3CodeSubselect(pParse, p_પદાવલિ);
  }
#endif
  પાછા reg;
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
સ્થિર પૂર્ણાંક પદાવલિ_Vector_Register(
  Parse *pParse,                  /* Parse context */
  પદાવલિ *pVector,                  /* Vector to extract element from */
  પૂર્ણાંક iField,                     /* Field to extract from pVector */
  પૂર્ણાંક regSelect,                  /* First in array of registers */
  પદાવલિ *pTmp,                     /* Temporary space */
  પદાવલિ **pp_પદાવલિ,                  /* OUT: Expression element */
  પૂર્ણાંક *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = pVector->op;
  પ્રતિજ્ઞા( op==TK_VECTOR || op==TK_REGISTER || op==TK_SELECT || op==TK_ERROR );
  જો( op==TK_REGISTER ){
    *pp_પદાવલિ = sqlite3VectorFieldSubexpr(pVector, iField);
    પાછા pVector->iTable+iField;
  }
  જો( op==TK_SELECT ){
    પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(pVector) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, કદ(પદાવલિ));
    pTmp->op = TK_SELECT_COLUMN;
    pTmp->p_ડાબું = pVector;
    pTmp->iColumn = iField;
    pTmp->iTable = pVector->x.pSelect->pEList->n_પદાવલિ;
    *pp_પદાવલિ = pTmp;
    પાછા regSelect+iField;
  }
  જો( op==TK_VECTOR ){
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(pVector) );
    *pp_પદાવલિ = pVector->x.pList->a[iField].p_પદાવલિ;
    પાછા sqlite3_પદાવલિ_Code_Temp(pParse, *pp_પદાવલિ, pRegFree);
  }
  પાછા 0;
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
સ્થિર શૂન્ય codeVectorCompare(
  Parse *pParse,        /* Code generator context */
  પદાવલિ *p_પદાવલિ,          /* The comparison operation */
  પૂર્ણાંક dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
  પદાવલિ *p_જમણું = p_પદાવલિ->p_જમણું;
  પૂર્ણાંક n_ડાબું = sqlite3_પદાવલિ_Vector_Size(p_ડાબું);
  પૂર્ણાંક i;
  પૂર્ણાંક reg_ડાબું = 0;
  પૂર્ણાંક reg_જમણું = 0;
  u8 opx = op;
  પૂર્ણાંક addrCmp = 0;
  પૂર્ણાંક addrDone = sqlite3VdbeMakeLabel(pParse);
  પૂર્ણાંક isCommuted = પદાવલિ_Has_Property(p_પદાવલિ,EP_Commuted);

  પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p_પદાવલિ,EP_Immutable) );
  જો( pParse->nErr ) પાછા;
  જો( n_ડાબું!=sqlite3_પદાવલિ_Vector_Size(p_જમણું) ){
    sqlite3_Error_સંદેશ(pParse, "row value misused");
    પાછા;
  }
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_EQ || p_પદાવલિ->op==TK_NE
       || p_પદાવલિ->op==TK_IS || p_પદાવલિ->op==TK_ISNOT
       || p_પદાવલિ->op==TK_LT || p_પદાવલિ->op==TK_GT
       || p_પદાવલિ->op==TK_LE || p_પદાવલિ->op==TK_GE
  );
  પ્રતિજ્ઞા( p_પદાવલિ->op==op || (p_પદાવલિ->op==TK_IS && op==TK_EQ)
            || (p_પદાવલિ->op==TK_ISNOT && op==TK_NE) );
  પ્રતિજ્ઞા( p5==0 || p_પદાવલિ->op!=op );
  પ્રતિજ્ઞા( p5==SQLITE_NULLEQ || p_પદાવલિ->op==op );

  જો( op==TK_LE ) opx = TK_LT;
  જો( op==TK_GE ) opx = TK_GT;
  જો( op==TK_NE ) opx = TK_EQ;

  reg_ડાબું = પદાવલિ_Code_Subselect(pParse, p_ડાબું);
  reg_જમણું = પદાવલિ_Code_Subselect(pParse, p_જમણું);

  sqlite3VdbeAddOp2(v, OP_Integer, 1, dest);
  માટે(i=0; 1 /*Loop exits by "break"*/; i++){
    પૂર્ણાંક regFree1 = 0, regFree2 = 0;
    પદાવલિ *pL = 0, *pR = 0;
    પદાવલિ tmp1, tmp2;
    પૂર્ણાંક r1, r2;
    પ્રતિજ્ઞા( i>=0 && i<n_ડાબું );
    જો( addrCmp ) sqlite3VdbeJumpHere(v, addrCmp);
    r1 = પદાવલિ_Vector_Register(pParse, p_ડાબું, i, reg_ડાબું, &tmp1, &pL, &regFree1);
    r2 = પદાવલિ_Vector_Register(pParse, p_જમણું, i, reg_જમણું, &tmp2, &pR, &regFree2);
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
    જો( (opx==TK_LT || opx==TK_GT) && i<n_ડાબું-1 ){
      addrCmp = sqlite3VdbeAddOp0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    જો( p5==SQLITE_NULLEQ ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, dest);
    }નહીંતર{
      sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    જો( i==n_ડાબું-1 ){
      તોડો;
    }
    જો( opx==TK_EQ ){
      sqlite3VdbeAddOp2(v, OP_NotNull, dest, addrDone); VdbeCoverage(v);
    }નહીંતર{
      પ્રતિજ્ઞા( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3VdbeAddOp2(v, OP_Goto, 0, addrDone);
      જો( i==n_ડાબું-2 ) opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  જો( op==TK_NE ){
    sqlite3VdbeAddOp2(v, OP_Not, dest, dest);
  }
}

#જો SQLITE_MAX_પદાવલિ_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Check_Height(Parse *pParse, પૂર્ણાંક nHeight){
  પૂર્ણાંક rc = SQLITE_OK;
  પૂર્ણાંક mxHeight = pParse->db->aLimit[SQLITE_LIMIT_પદાવલિ_DEPTH];
  જો( nHeight>mxHeight ){
    sqlite3_Error_સંદેશ(pParse,
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  પાછા rc;
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
સ્થિર શૂન્ય height_Of_પદાવલિ(અચલ પદાવલિ *p, પૂર્ણાંક *pnHeight){
  જો( p ){
    જો( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
સ્થિર શૂન્ય height_Of_પદાવલિ_List(અચલ પદાવલિ_List *p, પૂર્ણાંક *pnHeight){
  જો( p ){
    પૂર્ણાંક i;
    માટે(i=0; i<p->n_પદાવલિ; i++){
      height_Of_પદાવલિ(p->a[i].p_પદાવલિ, pnHeight);
    }
  }
}
સ્થિર શૂન્ય heightOfSelect(અચલ Select *pSelect, પૂર્ણાંક *pnHeight){
  અચલ Select *p;
  માટે(p=pSelect; p; p=p->pPrior){
    height_Of_પદાવલિ(p->pWhere, pnHeight);
    height_Of_પદાવલિ(p->pHaving, pnHeight);
    height_Of_પદાવલિ(p->pLimit, pnHeight);
    height_Of_પદાવલિ_List(p->pEList, pnHeight);
    height_Of_પદાવલિ_List(p->pGroupBy, pnHeight);
    height_Of_પદાવલિ_List(p->pOrderBy, pnHeight);
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
સ્થિર શૂન્ય પદાવલિ_Set_Height(પદાવલિ *p){
  પૂર્ણાંક nHeight = p->p_ડાબું ? p->p_ડાબું->nHeight : 0;
  જો( NEVER(p->p_જમણું) && p->p_જમણું->nHeight>nHeight ){
    nHeight = p->p_જમણું->nHeight;
  }
  જો( પદાવલિ_Use_XSelect(p) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }નહીંતર જો( p->x.pList ){
    height_Of_પદાવલિ_List(p->x.pList, &nHeight);
    p->flags |= EP_Propagate & sqlite3_પદાવલિ_List_Flags(p->x.pList);
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
શૂન્ય sqlite3_પદાવલિ_Set_Height_And_Flags(Parse *pParse, પદાવલિ *p){
  જો( pParse->nErr ) પાછા;
  પદાવલિ_Set_Height(p);
  sqlite3_પદાવલિ_Check_Height(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
પૂર્ણાંક sqlite3_Select_પદાવલિ_Height(અચલ Select *p){
  પૂર્ણાંક nHeight = 0;
  heightOfSelect(p, &nHeight);
  પાછા nHeight;
}
#નહીંતર /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
શૂન્ય sqlite3_પદાવલિ_Set_Height_And_Flags(Parse *pParse, પદાવલિ *p){
  જો( pParse->nErr ) પાછા;
  જો( p && પદાવલિ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_પદાવલિ_List_Flags(p->x.pList);
  }
}
#define પદાવલિ_Set_Height(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
શૂન્ય sqlite3_પદાવલિ_Set_Error_Offset(પદાવલિ *p_પદાવલિ, પૂર્ણાંક iOfst){
  જો( p_પદાવલિ==0 ) પાછા;
  જો( NEVER(પદાવલિ_Use_WJoin(p_પદાવલિ)) ) પાછા;
  p_પદાવલિ->w.iOfst = iOfst;
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
પદાવલિ *sqlite3_પદાવલિ_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  પૂર્ણાંક op,                 /* Expression opcode */
  અચલ Token *pToken,    /* Token argument.  Might be NULL */
  પૂર્ણાંક dequote             /* True to dequote */
){
  પદાવલિ *pNew;
  પૂર્ણાંક nExtra = pToken ? pToken->n+1 : 0;

  પ્રતિજ્ઞા( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, કદ(પદાવલિ)+nExtra);
  જો( pNew ){
    memset(pNew, 0, કદ(પદાવલિ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    જો( nExtra ){
      પ્રતિજ્ઞા( pToken!=0 );
      pNew->u.zToken = (અક્ષર*)&pNew[1];
      પ્રતિજ્ઞા( pToken->z!=0 || pToken->n==0 );
      જો( pToken->n ) memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      જો( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_પદાવલિ(pNew);
      }
    }
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  પાછા pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
પદાવલિ *sqlite3_પદાવલિ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  પૂર્ણાંક op,                 /* Expression opcode */
  અચલ અક્ષર *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  પાછા sqlite3_પદાવલિ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
પદાવલિ *sqlite3_પદાવલિ_Int32(sqlite3 *db, પૂર્ણાંક iVal){
  પદાવલિ *pNew = sqlite3DbMallocRawNN(db, કદ(પદાવલિ));
  જો( pNew ){
    memset(pNew, 0, કદ(પદાવલિ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_કિંમત|EP_Leaf|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_કિંમત = iVal;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    pNew->nHeight = 1;
#endif 
  }
  પાછા pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
શૂન્ય sqlite3_પદાવલિ_Attach_Subtrees(
  sqlite3 *db,
  પદાવલિ *p_મૂળ,
  પદાવલિ *p_ડાબું,
  પદાવલિ *p_જમણું
){
  જો( p_મૂળ==0 ){
    પ્રતિજ્ઞા( db->mallocFailed );
    sqlite3_પદાવલિ_Delete(db, p_ડાબું);
    sqlite3_પદાવલિ_Delete(db, p_જમણું);
  }નહીંતર{
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_મૂળ) );
    પ્રતિજ્ઞા( p_મૂળ->x.pSelect==0 );
    જો( p_જમણું ){
      p_મૂળ->p_જમણું = p_જમણું;
      p_મૂળ->flags |= EP_Propagate & p_જમણું->flags;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
      p_મૂળ->nHeight = p_જમણું->nHeight+1;
    }નહીંતર{
      p_મૂળ->nHeight = 1;
#endif
    }
    જો( p_ડાબું ){
      p_મૂળ->p_ડાબું = p_ડાબું;
      p_મૂળ->flags |= EP_Propagate & p_ડાબું->flags;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
      જો( p_ડાબું->nHeight>=p_મૂળ->nHeight ){
        p_મૂળ->nHeight = p_ડાબું->nHeight+1;
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
પદાવલિ *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  પૂર્ણાંક op,                 /* Expression opcode */
  પદાવલિ *p_ડાબું,            /* Left operand */
  પદાવલિ *p_જમણું            /* Right operand */
){
  પદાવલિ *p;
  p = sqlite3DbMallocRawNN(pParse->db, કદ(પદાવલિ));
  જો( p ){
    memset(p, 0, કદ(પદાવલિ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_પદાવલિ_Attach_Subtrees(pParse->db, p, p_ડાબું, p_જમણું);
    sqlite3_પદાવલિ_Check_Height(pParse, p->nHeight);
  }નહીંતર{
    sqlite3_પદાવલિ_Delete(pParse->db, p_ડાબું);
    sqlite3_પદાવલિ_Delete(pParse->db, p_જમણું);
  }
  પાછા p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
શૂન્ય sqlite3PExprAddSelect(Parse *pParse, પદાવલિ *p_પદાવલિ, Select *pSelect){
  જો( p_પદાવલિ ){
    p_પદાવલિ->x.pSelect = pSelect;
    પદાવલિ_Set_Property(p_પદાવલિ, EP_xIsSelect|EP_Subquery);
    sqlite3_પદાવલિ_Set_Height_And_Flags(pParse, p_પદાવલિ);
  }નહીંતર{
    પ્રતિજ્ઞા( pParse->db->mallocFailed );
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
Select *sqlite3_પદાવલિ_List_To_કિંમતો(Parse *pParse, પૂર્ણાંક nElem, પદાવલિ_List *pEList){
  પૂર્ણાંક ii;
  Select *pRet = 0;
  પ્રતિજ્ઞા( nElem>1 );
  માટે(ii=0; ii<pEList->n_પદાવલિ; ii++){
    Select *pSel;
    પદાવલિ *p_પદાવલિ = pEList->a[ii].p_પદાવલિ;
    પૂર્ણાંક n_પદાવલિ_Elem;
    જો( p_પદાવલિ->op==TK_VECTOR ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      n_પદાવલિ_Elem = p_પદાવલિ->x.pList->n_પદાવલિ;
    }નહીંતર{
      n_પદાવલિ_Elem = 1;
    }
    જો( n_પદાવલિ_Elem!=nElem ){
      sqlite3_Error_સંદેશ(pParse, "IN(...) element has %d term%s - expected %d",
          n_પદાવલિ_Elem, n_પદાવલિ_Elem>1?"s":"", nElem
      );
      તોડો;
    }
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
    pSel = sqlite3SelectNew(pParse, p_પદાવલિ->x.pList, 0, 0, 0, 0, 0, SF_કિંમતો,0);
    p_પદાવલિ->x.pList = 0;
    જો( pSel ){
      જો( pRet ){
        pSel->op = TK_ALL;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  જો( pRet && pRet->pPrior ){
    pRet->selFlags |= SF_Multi_કિંમત;
  }
  sqlite3_પદાવલિ_List_Delete(pParse->db, pEList);
  પાછા pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
પદાવલિ *sqlite3_પદાવલિ_And(Parse *pParse, પદાવલિ *p_ડાબું, પદાવલિ *p_જમણું){
  sqlite3 *db = pParse->db;
  જો( p_ડાબું==0  ){
    પાછા p_જમણું;
  }નહીંતર જો( p_જમણું==0 ){
    પાછા p_ડાબું;
  }નહીંતર{
    u32 f = p_ડાબું->flags | p_જમણું->flags;
    જો( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !IN_RENAME_OBJECT
    ){
      પદાવલિ *p;
      sqlite3_પદાવલિ_Deferred_Delete(pParse, p_ડાબું);
      sqlite3_પદાવલિ_Deferred_Delete(pParse, p_જમણું);
      p = sqlite3_પદાવલિ_Int32(db, 0);
      જો( p ) p->op2 = 1;
      પાછા p;
    }નહીંતર{
      પાછા sqlite3PExpr(pParse, TK_AND, p_ડાબું, p_જમણું);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
પદાવલિ *sqlite3_પદાવલિ_Function(
  Parse *pParse,        /* Parsing context */
  પદાવલિ_List *pList,      /* Argument list */
  અચલ Token *pToken,  /* Name of the function */
  પૂર્ણાંક eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  પદાવલિ *pNew;
  sqlite3 *db = pParse->db;
  પ્રતિજ્ઞા( pToken );
  pNew = sqlite3_પદાવલિ_Alloc(db, TK_FUNCTION, pToken, 1);
  જો( pNew==0 ){
    sqlite3_પદાવલિ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    પાછા 0;
  }
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (પૂર્ણાંક)(pToken->z - pParse->zTail);
  જો( pList
   && pList->n_પદાવલિ > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_સંદેશ(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  પદાવલિ_Set_Property(pNew, EP_HasFunc);
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(pNew) );
  sqlite3_પદાવલિ_Set_Height_And_Flags(pParse, pNew);
  જો( eDistinct==SF_Distinct ) પદાવલિ_Set_Property(pNew, EP_Distinct);
  પાછા pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
શૂન્ય sqlite3_પદાવલિ_Order_By_Aggregate_Error(Parse *pParse, પદાવલિ *p){
  sqlite3_Error_સંદેશ(pParse,
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
શૂન્ય sqlite3_પદાવલિ_Add_Function_Order_By(
  Parse *pParse,        /* Parsing context */
  પદાવલિ *p_પદાવલિ,          /* The function call to which ORDER BY is to be added */
  પદાવલિ_List *pOrderBy    /* The ORDER BY clause to add */
){
  પદાવલિ *pOB;
  sqlite3 *db = pParse->db;
  જો( NEVER(pOrderBy==0) ){
    પ્રતિજ્ઞા( db->mallocFailed );
    પાછા;
  }
  જો( p_પદાવલિ==0 ){
    પ્રતિજ્ઞા( db->mallocFailed );
    sqlite3_પદાવલિ_List_Delete(db, pOrderBy);
    પાછા;
  }
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION );
  પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું==0 );
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  જો( p_પદાવલિ->x.pList==0 || NEVER(p_પદાવલિ->x.pList->n_પદાવલિ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3ParserAddCleanup(pParse, sqlite3_પદાવલિ_List_Delete_Generic, pOrderBy);
    પાછા;
  }
  જો( IsWindowFunc(p_પદાવલિ) ){
    sqlite3_પદાવલિ_Order_By_Aggregate_Error(pParse, p_પદાવલિ);
    sqlite3_પદાવલિ_List_Delete(db, pOrderBy);
    પાછા;
  }
  જો( pOrderBy->n_પદાવલિ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_સંદેશ(pParse, "too many terms in ORDER BY clause");
    sqlite3_પદાવલિ_List_Delete(db, pOrderBy);
    પાછા;
  }

  pOB = sqlite3_પદાવલિ_Alloc(db, TK_ORDER, 0, 0);
  જો( pOB==0 ){
    sqlite3_પદાવલિ_List_Delete(db, pOrderBy);
    પાછા;
  }
  pOB->x.pList = pOrderBy;
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(pOB) );
  p_પદાવલિ->p_ડાબું = pOB;
  પદાવલિ_Set_Property(pOB, EP_FullSize);
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
શૂન્ય sqlite3_પદાવલિ_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  અચલ પદાવલિ *p_પદાવલિ,     /* The function invocation */
  અચલ FuncDef *pDef    /* The function being invoked */
){
  પ્રતિજ્ઞા( !IN_RENAME_OBJECT );
  પ્રતિજ્ઞા( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0 );
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    જો( (pDef->funcFlags & SQLITE_FUNC_DIRECT)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_સંદેશ(pParse, "unsafe use of %#T()", p_પદાવલિ);
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
શૂન્ય sqlite3_પદાવલિ_Assign_Var_સંખ્યા(Parse *pParse, પદાવલિ *p_પદાવલિ, u32 n){
  sqlite3 *db = pParse->db;
  અચલ અક્ષર *z;
  ynVar x;

  જો( p_પદાવલિ==0 ) પાછા;
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત|EP_Reduced|EP_TokenOnly) );
  z = p_પદાવલિ->u.zToken;
  પ્રતિજ્ઞા( z!=0 );
  પ્રતિજ્ઞા( z[0]!=0 );
  પ્રતિજ્ઞા( n==(u32)sqlite3Strlen30(z) );
  જો( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    પ્રતિજ્ઞા( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }નહીંતર{
    પૂર્ણાંક doAdd = 0;
    જો( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      પૂર્ણાંક bOk;
      જો( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }નહીંતર{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_સંખ્યા]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_સંખ્યા] );
      જો( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_સંખ્યા] ){
        sqlite3_Error_સંદેશ(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_સંખ્યા]);
        sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, p_પદાવલિ);
        પાછા;
      }
      x = (ynVar)i;
      જો( x>pParse->nVar ){
        pParse->nVar = (પૂર્ણાંક)x;
        doAdd = 1;
      }નહીંતર જો( x<SQLITE_VNBMC*64 ){
        doAdd = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }નહીંતર જો( sqlite3_VList_અંક_To_Name(pParse->pVList, x)==0 ){
        doAdd = 1;
      }
    }નહીંતર{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_To_અંક(pParse->pVList, z, n);
      જો( x==0 ){
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    જો( doAdd ){
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
      જો( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_પદાવલિ->iColumn = x;
  જો( x>db->aLimit[SQLITE_LIMIT_VARIABLE_સંખ્યા] ){
    sqlite3_Error_સંદેશ(pParse, "too many SQL variables");
    sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, p_પદાવલિ);
  }
}

/*
** Recursively delete an expression tree.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય sqlite3_પદાવલિ_Delete_NN(sqlite3 *db, પદાવલિ *p){
  પ્રતિજ્ઞા( p!=0 );
  પ્રતિજ્ઞા( db!=0 );
પદાવલિ_Delete_Restart:
  પ્રતિજ્ઞા( !પદાવલિ_Use_UValue(p) || p->u.i_કિંમત>=0 );
  પ્રતિજ્ઞા( !પદાવલિ_Use_YWin(p) || !પદાવલિ_Use_YSub(p) );
  પ્રતિજ્ઞા( !પદાવલિ_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  પ્રતિજ્ઞા( p->op!=TK_FUNCTION || !પદાવલિ_Use_YSub(p) );
#ifdef SQLITE_DEBUG
  જો( પદાવલિ_Has_Property(p, EP_Leaf) && !પદાવલિ_Has_Property(p, EP_TokenOnly) ){
    પ્રતિજ્ઞા( p->p_ડાબું==0 );
    પ્રતિજ્ઞા( p->p_જમણું==0 );
    પ્રતિજ્ઞા( !પદાવલિ_Use_XSelect(p) || p->x.pSelect==0 );
    પ્રતિજ્ઞા( !પદાવલિ_Use_XList(p) || p->x.pList==0 );
  }
#endif
  જો( !પદાવલિ_Has_Property(p, (EP_TokenOnly|EP_Leaf)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    પ્રતિજ્ઞા( (પદાવલિ_Use_XList(p) && p->x.pList==0) || p->p_જમણું==0 );
    જો( p->p_જમણું ){
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_WinFunc) );
      sqlite3_પદાવલિ_Delete_NN(db, p->p_જમણું);
    }નહીંતર જો( પદાવલિ_Use_XSelect(p) ){
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }નહીંતર{
      sqlite3_પદાવલિ_List_Delete(db, p->x.pList);
#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( પદાવલિ_Has_Property(p, EP_WinFunc) ){
        sqlite3WindowDelete(db, p->y.pWin);
      }
#endif
    }
    જો( p->p_ડાબું && p->op!=TK_SELECT_COLUMN ){
      પદાવલિ *p_ડાબું = p->p_ડાબું;
      જો( !પદાવલિ_Has_Property(p, EP_Static)
       && !પદાવલિ_Has_Property(p_ડાબું, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ડાબું;
        જાઓ પદાવલિ_Delete_Restart;
      }નહીંતર{
        sqlite3_પદાવલિ_Delete_NN(db, p_ડાબું);
      }
    }
  }
  જો( !પદાવલિ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
શૂન્ય sqlite3_પદાવલિ_Delete(sqlite3 *db, પદાવલિ *p){
  જો( p ) sqlite3_પદાવલિ_Delete_NN(db, p);
}
શૂન્ય sqlite3_પદાવલિ_Delete_Generic(sqlite3 *db, શૂન્ય *p){
  જો( ALWAYS(p) ) sqlite3_પદાવલિ_Delete_NN(db, (પદાવલિ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
શૂન્ય sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  જો( p==0 ){
    /* Nothing to clear */
  }નહીંતર જો( p->pOn ){
    sqlite3_પદાવલિ_Delete_NN(db, p->pOn);
  }નહીંતર જો( p->pUsing ){
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
પૂર્ણાંક sqlite3_પદાવલિ_Deferred_Delete(Parse *pParse, પદાવલિ *p_પદાવલિ){
  પાછા 0==sqlite3ParserAddCleanup(pParse, sqlite3_પદાવલિ_Delete_Generic, p_પદાવલિ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
શૂન્ય sqlite3_પદાવલિ_Unmap_And_Delete(Parse *pParse, પદાવલિ *p){
  જો( p ){
    જો( IN_RENAME_OBJECT ){
      sqlite3_Rename_પદાવલિ_Unmap(pParse, p);
    }
    sqlite3_પદાવલિ_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Struct_Size(અચલ પદાવલિ *p){
  જો( પદાવલિ_Has_Property(p, EP_TokenOnly) ) પાછા પદાવલિ_TOKENONLYSIZE;
  જો( પદાવલિ_Has_Property(p, EP_Reduced) ) પાછા પદાવલિ_REDUCEDSIZE;
  પાછા પદાવલિ_FULLSIZE;
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
સ્થિર પૂર્ણાંક duped_પદાવલિ_Struct_Size(અચલ પદાવલિ *p, પૂર્ણાંક flags){
  પૂર્ણાંક nSize;
  પ્રતિજ્ઞા( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  પ્રતિજ્ઞા( પદાવલિ_FULLSIZE<=0xfff );
  પ્રતિજ્ઞા( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  જો( 0==flags || પદાવલિ_Has_Property(p, EP_FullSize) ){
    nSize = પદાવલિ_FULLSIZE;
  }નહીંતર{
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_OuterON) );
    પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p, EP_NoReduce) );
    જો( p->p_ડાબું || p->x.pList ){
      nSize = પદાવલિ_REDUCEDSIZE | EP_Reduced;
    }નહીંતર{
      પ્રતિજ્ઞા( p->p_જમણું==0 );
      nSize = પદાવલિ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  પાછા nSize;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
સ્થિર પૂર્ણાંક duped_પદાવલિ_ગાંઠ_Size(અચલ પદાવલિ *p, પૂર્ણાંક flags){
  પૂર્ણાંક nByte = duped_પદાવલિ_Struct_Size(p, flags) & 0xfff;
  જો( !પદાવલિ_Has_Property(p, EP_Int_કિંમત) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  પાછા ROUND8(nByte);
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
સ્થિર પૂર્ણાંક duped_પદાવલિ_Size(અચલ પદાવલિ *p){
  પૂર્ણાંક nByte;
  પ્રતિજ્ઞા( p!=0 );
  nByte = duped_પદાવલિ_ગાંઠ_Size(p, EXPRDUP_REDUCE);
  જો( p->p_ડાબું ) nByte += duped_પદાવલિ_Size(p->p_ડાબું);
  જો( p->p_જમણું ) nByte += duped_પદાવલિ_Size(p->p_જમણું);
  પ્રતિજ્ઞા( nByte==ROUND8(nByte) );
  પાછા nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
પ્રકારવ્યાખ્યા સંરચના EdupBuf EdupBuf;
સંરચના EdupBuf {
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
સ્થિર પદાવલિ *પદાવલિ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  અચલ પદાવલિ *p,        /* Expr tree to be duplicated */
  પૂર્ણાંક dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  પદાવલિ *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  પૂર્ણાંક nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  પ્રતિજ્ઞા( db!=0 );
  પ્રતિજ્ઞા( p );
  પ્રતિજ્ઞા( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  પ્રતિજ્ઞા( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  જો( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = pEdupBuf->zEnd;
#endif
    staticFlag = EP_Static;
    પ્રતિજ્ઞા( sEdupBuf.zAlloc!=0 );
    પ્રતિજ્ઞા( dupFlags==EXPRDUP_REDUCE );
  }નહીંતર{
    પૂર્ણાંક nAlloc;
    જો( dupFlags ){
      nAlloc = duped_પદાવલિ_Size(p);
    }નહીંતર જો( !પદાવલિ_Has_Property(p, EP_Int_કિંમત) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(પદાવલિ_FULLSIZE + nToken);
    }નહીંતર{
      nToken = 0;
      nAlloc = ROUND8(પદાવલિ_FULLSIZE);
    }
    પ્રતિજ્ઞા( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ifdef SQLITE_DEBUG
    sEdupBuf.zEnd = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#endif
    
    staticFlag = 0;
  }
  pNew = (પદાવલિ *)sEdupBuf.zAlloc;
  પ્રતિજ્ઞા( EIGHT_BYTE_ALIGNMENT(pNew) );

  જો( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    અચલ unsigned nStructSize = duped_પદાવલિ_Struct_Size(p, dupFlags);
    પૂર્ણાંક nNewSize = nStructSize & 0xfff;
    જો( nToken<0 ){
      જો( !પદાવલિ_Has_Property(p, EP_Int_કિંમત) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }નહીંતર{
        nToken = 0;
      }
    }
    જો( dupFlags ){
      પ્રતિજ્ઞા( (પૂર્ણાંક)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >= nNewSize+nToken );
      પ્રતિજ્ઞા( પદાવલિ_Has_Property(p, EP_Reduced)==0 );
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    }નહીંતર{
      u32 nSize = (u32)પદાવલિ_Struct_Size(p);
      પ્રતિજ્ઞા( (પૂર્ણાંક)(sEdupBuf.zEnd - sEdupBuf.zAlloc) >=
                                                   (પૂર્ણાંક)પદાવલિ_FULLSIZE+nToken );
      memcpy(sEdupBuf.zAlloc, p, nSize);
      જો( nSize<પદાવલિ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[nSize], 0, પદાવલિ_FULLSIZE-nSize);
      }
      nNewSize = પદાવલિ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    પદાવલિ_Clear_VVAProperties(pNew);
    જો( dupFlags ){
      પદાવલિ_Set_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    પ્રતિજ્ઞા( nToken>=0 );
    જો( nToken>0 ){
      અક્ષર *zToken = pNew->u.zToken = (અક્ષર*)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(nNewSize);

    જો( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_Leaf))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      જો( પદાવલિ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }નહીંતર{
        pNew->x.pList = sqlite3_પદાવલિ_List_Dup(db, p->x.pList,
                           p->op!=TK_ORDER ? dupFlags : 0);
      }

#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( પદાવલિ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
        પ્રતિજ્ઞા( પદાવલિ_Has_Property(pNew, EP_WinFunc) );
      }
#endif /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      જો( dupFlags ){
        જો( p->op==TK_SELECT_COLUMN ){
          pNew->p_ડાબું = p->p_ડાબું;
          પ્રતિજ્ઞા( p->p_જમણું==0 
               || p->p_જમણું==p->p_ડાબું
               || પદાવલિ_Has_Property(p->p_ડાબું, EP_Subquery) );
        }નહીંતર{
          pNew->p_ડાબું = p->p_ડાબું ?
                      પદાવલિ_Dup(db, p->p_ડાબું, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_જમણું = p->p_જમણું ?
                       પદાવલિ_Dup(db, p->p_જમણું, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }નહીંતર{
        જો( p->op==TK_SELECT_COLUMN ){
          pNew->p_ડાબું = p->p_ડાબું;
          પ્રતિજ્ઞા( p->p_જમણું==0 
               || p->p_જમણું==p->p_ડાબું
               || પદાવલિ_Has_Property(p->p_ડાબું, EP_Subquery) );
        }નહીંતર{
          pNew->p_ડાબું = sqlite3_પદાવલિ_Dup(db, p->p_ડાબું, 0);
        }
        pNew->p_જમણું = sqlite3_પદાવલિ_Dup(db, p->p_જમણું, 0);
      }
    }
  }
  જો( pEdupBuf ) memcpy(pEdupBuf, &sEdupBuf, કદ(sEdupBuf));
  પ્રતિજ્ઞા( sEdupBuf.zAlloc <= sEdupBuf.zEnd );
  પાછા pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ifndef SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  જો( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    જો( pRet ){
      પૂર્ણાંક i;
      pRet->nCte = p->nCte;
      માટે(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3_પદાવલિ_List_Dup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  પાછા pRet;
}
#નહીંતર
# define sqlite3WithDup(x,y) 0
#endif

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
સ્થિર પૂર્ણાંક gatherSelectWindowsCallback(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_FUNCTION && પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = p_પદાવલિ->y.pWin;
    પ્રતિજ્ઞા( pWin );
    પ્રતિજ્ઞા( IsWindowFunc(p_પદાવલિ) );
    પ્રતિજ્ઞા( pWin->ppThis==0 );
    sqlite3WindowLink(pSelect, pWin);
  }
  પાછા WRC_Continue;
}
સ્થિર પૂર્ણાંક gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  પાછા p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
સ્થિર શૂન્ય gatherSelectWindows(Select *p){
  Walker w;
  w.x_પદાવલિ_Callback = gatherSelectWindowsCallback;
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
પદાવલિ *sqlite3_પદાવલિ_Dup(sqlite3 *db, અચલ પદાવલિ *p, પૂર્ણાંક flags){
  પ્રતિજ્ઞા( flags==0 || flags==EXPRDUP_REDUCE );
  પાછા p ? પદાવલિ_Dup(db, p, flags, 0) : 0;
}
પદાવલિ_List *sqlite3_પદાવલિ_List_Dup(sqlite3 *db, અચલ પદાવલિ_List *p, પૂર્ણાંક flags){
  પદાવલિ_List *pNew;
  સંરચના પદાવલિ_List_item *pItem;
  અચલ સંરચના પદાવલિ_List_item *pOldItem;
  પૂર્ણાંક i;
  પદાવલિ *pPriorSelectColOld = 0;
  પદાવલિ *pPriorSelectColNew = 0;
  પ્રતિજ્ઞા( db!=0 );
  જો( p==0 ) પાછા 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  જો( pNew==0 ) પાછા 0;
  pNew->n_પદાવલિ = p->n_પદાવલિ;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  માટે(i=0; i<p->n_પદાવલિ; i++, pItem++, pOldItem++){
    પદાવલિ *p_Old_પદાવલિ = pOldItem->p_પદાવલિ;
    પદાવલિ *p_New_પદાવલિ;
    pItem->p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_Old_પદાવલિ, flags);
    જો( p_Old_પદાવલિ
     && p_Old_પદાવલિ->op==TK_SELECT_COLUMN
     && (p_New_પદાવલિ = pItem->p_પદાવલિ)!=0
    ){
      જો( p_New_પદાવલિ->p_જમણું ){
        pPriorSelectColOld = p_Old_પદાવલિ->p_જમણું;
        pPriorSelectColNew = p_New_પદાવલિ->p_જમણું;
        p_New_પદાવલિ->p_ડાબું = p_New_પદાવલિ->p_જમણું;
      }નહીંતર{
        જો( p_Old_પદાવલિ->p_ડાબું!=pPriorSelectColOld ){
          pPriorSelectColOld = p_Old_પદાવલિ->p_ડાબું;
          pPriorSelectColNew = sqlite3_પદાવલિ_Dup(db, pPriorSelectColOld, flags);
          p_New_પદાવલિ->p_જમણું = pPriorSelectColNew;
        }
        p_New_પદાવલિ->p_ડાબું = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  પાછા pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#જો !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, અચલ SrcList *p, પૂર્ણાંક flags){
  SrcList *pNew;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( db!=0 );
  જો( p==0 ) પાછા 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  જો( pNew==0 ) પાછા 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  માટે(i=0; i<p->nSrc; i++){
    SrcItem *pNewItem = &pNew->a[i];
    અચલ SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    જો( pOldItem->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, કદ(Subquery));
      જો( pNewSubq==0 ){
        પ્રતિજ્ઞા( db->mallocFailed );
        pNewItem->fg.isSubquery = 0;
      }નહીંતર{
        memcpy(pNewSubq, pOldItem->u4.pSubq, કદ(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        જો( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    }નહીંતર જો( pOldItem->fg.fixedSchema ){
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    }નહીંતર{
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    જો( pNewItem->fg.isIndexedBy ){
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    }નહીંતર જો( pNewItem->fg.isTabFunc ){
      pNewItem->u1.pFuncArg =
          sqlite3_પદાવલિ_List_Dup(db, pOldItem->u1.pFuncArg, flags);
    }નહીંતર{
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    જો( pNewItem->fg.isCte ){
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    જો( pTab ){
      pTab->nTabRef++;
    }
    જો( pOldItem->fg.isUsing ){
      પ્રતિજ્ઞા( pNewItem->fg.isUsing );
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    }નહીંતર{
      pNewItem->u3.pOn = sqlite3_પદાવલિ_Dup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  પાછા pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, અચલ IdList *p){
  IdList *pNew;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( db!=0 );
  જો( p==0 ) પાછા 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  જો( pNew==0 ) પાછા 0;
  pNew->nId = p->nId;
  માટે(i=0; i<p->nId; i++){
    સંરચના IdList_item *pNewItem = &pNew->a[i];
    અચલ સંરચના IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  પાછા pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, અચલ Select *pDup, પૂર્ણાંક flags){
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  અચલ Select *p;

  પ્રતિજ્ઞા( db!=0 );
  માટે(p=pDup; p; p=p->pPrior){
    Select *pNew = sqlite3DbMallocRawNN(db, કદ(*p) );
    જો( pNew==0 ) તોડો;
    pNew->pEList = sqlite3_પદાવલિ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_પદાવલિ_Dup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3_પદાવલિ_List_Dup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3_પદાવલિ_Dup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3_પદાવલિ_List_Dup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3_પદાવલિ_Dup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    જો( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#endif
    pNew->selId = p->selId;
    જો( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      તોડો;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  પાછા pRet;
}
#નહીંતર
Select *sqlite3SelectDup(sqlite3 *db, અચલ Select *p, પૂર્ણાંક flags){
  પ્રતિજ્ઞા( p==0 );
  પાછા 0;
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
સ્થિર અચલ સંરચના પદાવલિ_List_item zeroItem = {0};
SQLITE_NOINLINE પદાવલિ_List *sqlite3_પદાવલિ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  પદાવલિ *p_પદાવલિ             /* Expression to be appended. Might be NULL */
){
  સંરચના પદાવલિ_List_item *pItem;
  પદાવલિ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  જો( pList==0 ){
    sqlite3_પદાવલિ_Delete(db, p_પદાવલિ);
    પાછા 0;
  }
  pList->nAlloc = 4;
  pList->n_પદાવલિ = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->p_પદાવલિ = p_પદાવલિ;
  પાછા pList;
}
SQLITE_NOINLINE પદાવલિ_List *sqlite3_પદાવલિ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  પદાવલિ_List *pList,        /* List to which to append. Might be NULL */
  પદાવલિ *p_પદાવલિ             /* Expression to be appended. Might be NULL */
){
  સંરચના પદાવલિ_List_item *pItem;
  પદાવલિ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  જો( pNew==0 ){
    sqlite3_પદાવલિ_List_Delete(db, pList);
    sqlite3_પદાવલિ_Delete(db, p_પદાવલિ);
    પાછા 0;
  }નહીંતર{
    pList = pNew;
  }
  pItem = &pList->a[pList->n_પદાવલિ++];
  *pItem = zeroItem;
  pItem->p_પદાવલિ = p_પદાવલિ;
  પાછા pList;
}
પદાવલિ_List *sqlite3_પદાવલિ_List_Append(
  Parse *pParse,          /* Parsing context */
  પદાવલિ_List *pList,        /* List to which to append. Might be NULL */
  પદાવલિ *p_પદાવલિ             /* Expression to be appended. Might be NULL */
){
  સંરચના પદાવલિ_List_item *pItem;
  જો( pList==0 ){
    પાછા sqlite3_પદાવલિ_List_Append_New(pParse->db,p_પદાવલિ);
  }
  જો( pList->nAlloc<pList->n_પદાવલિ+1 ){
    પાછા sqlite3_પદાવલિ_List_Append_Grow(pParse->db,pList,p_પદાવલિ);
  }
  pItem = &pList->a[pList->n_પદાવલિ++];
  *pItem = zeroItem;
  pItem->p_પદાવલિ = p_પદાવલિ;
  પાછા pList;
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
પદાવલિ_List *sqlite3_પદાવલિ_List_Append_Vector(
  Parse *pParse,         /* Parsing context */
  પદાવલિ_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  પદાવલિ *p_પદાવલિ            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  પૂર્ણાંક n;
  પૂર્ણાંક i;
  પૂર્ણાંક iFirst = pList ? pList->n_પદાવલિ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  જો( NEVER(pColumns==0) ) જાઓ vector_append_error;
  જો( p_પદાવલિ==0 ) જાઓ vector_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  જો( p_પદાવલિ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ)) ){
    sqlite3_Error_સંદેશ(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    જાઓ vector_append_error;
  }

  માટે(i=0; i<pColumns->nId; i++){
    પદાવલિ *p_Sub_પદાવલિ = sqlite3_પદાવલિ_For_Vector_Field(pParse, p_પદાવલિ, i, pColumns->nId);
    પ્રતિજ્ઞા( p_Sub_પદાવલિ!=0 || db->mallocFailed );
    જો( p_Sub_પદાવલિ==0 ) ચાલુ;
    pList = sqlite3_પદાવલિ_List_Append(pParse, pList, p_Sub_પદાવલિ);
    જો( pList ){
      પ્રતિજ્ઞા( pList->n_પદાવલિ==iFirst+i+1 );
      pList->a[pList->n_પદાવલિ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  જો( !db->mallocFailed && p_પદાવલિ->op==TK_SELECT && ALWAYS(pList!=0) ){
    પદાવલિ *pFirst = pList->a[iFirst].p_પદાવલિ;
    પ્રતિજ્ઞા( pFirst!=0 );
    પ્રતિજ્ઞા( pFirst->op==TK_SELECT_COLUMN );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    pFirst->p_જમણું = p_પદાવલિ;
    p_પદાવલિ = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3_પદાવલિ_Unmap_And_Delete(pParse, p_પદાવલિ);
  sqlite3IdListDelete(db, pColumns);
  પાછા pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
શૂન્ય sqlite3_પદાવલિ_List_Set_Sort_Order(પદાવલિ_List *p, પૂર્ણાંક iSortOrder, પૂર્ણાંક eNulls){
  સંરચના પદાવલિ_List_item *pItem;
  જો( p==0 ) પાછા;
  પ્રતિજ્ઞા( p->n_પદાવલિ>0 );

  પ્રતિજ્ઞા( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  પ્રતિજ્ઞા( iSortOrder==SQLITE_SO_UNDEFINED
       || iSortOrder==SQLITE_SO_ASC
       || iSortOrder==SQLITE_SO_DESC
  );
  પ્રતિજ્ઞા( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  pItem = &p->a[p->n_પદાવલિ-1];
  પ્રતિજ્ઞા( pItem->fg.bNulls==0 );
  જો( iSortOrder==SQLITE_SO_UNDEFINED ){
    iSortOrder = SQLITE_SO_ASC;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  જો( eNulls!=SQLITE_SO_UNDEFINED ){
    pItem->fg.bNulls = 1;
    જો( iSortOrder!=eNulls ){
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
શૂન્ય sqlite3_પદાવલિ_List_Set_Name(
  Parse *pParse,          /* Parsing context */
  પદાવલિ_List *pList,        /* List to which to add the span. */
  અચલ Token *pName,     /* Name to be added */
  પૂર્ણાંક dequote             /* True to cause the name to be dequoted */
){
  પ્રતિજ્ઞા( pList!=0 || pParse->db->mallocFailed!=0 );
  પ્રતિજ્ઞા( pParse->eParseMode!=PARSE_MODE_UNMAP || dequote==0 );
  જો( pList ){
    સંરચના પદાવલિ_List_item *pItem;
    પ્રતિજ્ઞા( pList->n_પદાવલિ>0 );
    pItem = &pList->a[pList->n_પદાવલિ-1];
    પ્રતિજ્ઞા( pItem->zEName==0 );
    પ્રતિજ્ઞા( pItem->fg.eEName==ENAME_NAME );
    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    જો( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(pItem->zEName);
      જો( IN_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (અચલ શૂન્ય*)pItem->zEName, pName);
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
શૂન્ય sqlite3_પદાવલિ_List_Set_Span(
  Parse *pParse,          /* Parsing context */
  પદાવલિ_List *pList,        /* List to which to add the span. */
  અચલ અક્ષર *zStart,     /* Start of the span */
  અચલ અક્ષર *zEnd        /* End of the span */
){
  sqlite3 *db = pParse->db;
  પ્રતિજ્ઞા( pList!=0 || db->mallocFailed!=0 );
  જો( pList ){
    સંરચના પદાવલિ_List_item *pItem = &pList->a[pList->n_પદાવલિ-1];
    પ્રતિજ્ઞા( pList->n_પદાવલિ>0 );
    જો( pItem->zEName==0 ){
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = ENAME_SPAN;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
શૂન્ય sqlite3_પદાવલિ_List_Check_Length(
  Parse *pParse,
  પદાવલિ_List *pEList,
  અચલ અક્ષર *zObject
){
  પૂર્ણાંક mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->n_પદાવલિ==mx );
  testcase( pEList && pEList->n_પદાવલિ==mx+1 );
  જો( pEList && pEList->n_પદાવલિ>mx ){
    sqlite3_Error_સંદેશ(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય પદાવલિ_List_Delete_NN(sqlite3 *db, પદાવલિ_List *pList){
  પૂર્ણાંક i = pList->n_પદાવલિ;
  સંરચના પદાવલિ_List_item *pItem =  pList->a;
  પ્રતિજ્ઞા( pList->n_પદાવલિ>0 );
  પ્રતિજ્ઞા( db!=0 );
  કરો{
    sqlite3_પદાવલિ_Delete(db, pItem->p_પદાવલિ);
    જો( pItem->zEName ) sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  }જ્યારે( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
શૂન્ય sqlite3_પદાવલિ_List_Delete(sqlite3 *db, પદાવલિ_List *pList){
  જો( pList ) પદાવલિ_List_Delete_NN(db, pList);
}
શૂન્ય sqlite3_પદાવલિ_List_Delete_Generic(sqlite3 *db, શૂન્ય *pList){
  જો( ALWAYS(pList) ) પદાવલિ_List_Delete_NN(db, (પદાવલિ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_પદાવલિ_List_Flags(અચલ પદાવલિ_List *pList){
  પૂર્ણાંક i;
  u32 m = 0;
  પ્રતિજ્ઞા( pList!=0 );
  માટે(i=0; i<pList->n_પદાવલિ; i++){
     પદાવલિ *p_પદાવલિ = pList->a[i].p_પદાવલિ;
     પ્રતિજ્ઞા( p_પદાવલિ!=0 );
     m |= p_પદાવલિ->flags;
  }
  પાછા m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
પૂર્ણાંક sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->eCode = 0;
  પાછા WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(અચલ અક્ષર *zIn){
  જો( sqlite3StrICmp(zIn, "true")==0  ) પાછા EP_IsTrue;
  જો( sqlite3StrICmp(zIn, "false")==0 ) પાછા EP_IsFalse;
  પાછા 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Id_To_True_False(પદાવલિ *p_પદાવલિ){
  u32 v;
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_ID || p_પદાવલિ->op==TK_STRING );
  જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Quoted|EP_Int_કિંમત)
   && (v = sqlite3IsTrueOrFalse(p_પદાવલિ->u.zToken))!=0
  ){
    p_પદાવલિ->op = TK_TRUEFALSE;
    પદાવલિ_Set_Property(p_પદાવલિ, v);
    પાછા 1;
  }
  પાછા 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Truth_કિંમત(અચલ પદાવલિ *p_પદાવલિ){
  p_પદાવલિ = sqlite3_પદાવલિ_Skip_Collate_And_Likely((પદાવલિ*)p_પદાવલિ);
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_TRUEFALSE );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
  પ્રતિજ્ઞા( sqlite3StrICmp(p_પદાવલિ->u.zToken,"true")==0
       || sqlite3StrICmp(p_પદાવલિ->u.zToken,"false")==0 );
  પાછા p_પદાવલિ->u.zToken[4]==0;
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
પદાવલિ *sqlite3_પદાવલિ_Simplified_And_Or(પદાવલિ *p_પદાવલિ){
  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  જો( p_પદાવલિ->op==TK_AND || p_પદાવલિ->op==TK_OR ){
    પદાવલિ *p_જમણું = sqlite3_પદાવલિ_Simplified_And_Or(p_પદાવલિ->p_જમણું);
    પદાવલિ *p_ડાબું = sqlite3_પદાવલિ_Simplified_And_Or(p_પદાવલિ->p_ડાબું);
    જો( પદાવલિ_Always_True(p_ડાબું) || પદાવલિ_Always_False(p_જમણું) ){
      p_પદાવલિ = p_પદાવલિ->op==TK_AND ? p_જમણું : p_ડાબું;
    }નહીંતર જો( પદાવલિ_Always_True(p_જમણું) || પદાવલિ_Always_False(p_ડાબું) ){
      p_પદાવલિ = p_પદાવલિ->op==TK_AND ? p_ડાબું : p_જમણું;
    }
  }
  પાછા p_પદાવલિ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Eval_Rhs_First(પદાવલિ *p_પદાવલિ){
  જો( પદાવલિ_Has_Property(p_પદાવલિ->p_ડાબું, EP_Subquery)
   && !પદાવલિ_Has_Property(p_પદાવલિ->p_જમણું, EP_Subquery)
  ){
    પાછા 1;
  }નહીંતર{
    પાછા 0;
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
સ્થિર પૂર્ણાંક પદાવલિ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  પદાવલિ *p_પદાવલિ,       /* The comparison expression */
  પૂર્ણાંક *pR1,          /* OUT: Register holding the left operand */
  પૂર્ણાંક *pR2,          /* OUT: Register holding the right operand */
  પૂર્ણાંક *pFree1,       /* OUT: Temp register to free if not zero */
  પૂર્ણાંક *pFree2        /* OUT: Another temp register to free if not zero */
){
  પૂર્ણાંક addrIsNull;
  પૂર્ણાંક r1, r2;
  Vdbe *v = pParse->pVdbe;

  પ્રતિજ્ઞા( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  જો( પદાવલિ_Eval_Rhs_First(p_પદાવલિ) && sqlite3_પદાવલિ_Can_Be_Null(p_પદાવલિ->p_જમણું) ){
    r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }નહીંતર{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, pFree1);
  જો( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    જો( પદાવલિ_Has_Property(p_પદાવલિ->p_જમણું, EP_Subquery)
     && sqlite3_પદાવલિ_Can_Be_Null(p_પદાવલિ->p_ડાબું)
    ){
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  પાછા addrIsNull;
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
સ્થિર SQLITE_NOINLINE પૂર્ણાંક પદાવલિ_ગાંઠ_Is_Constant_Function(
  Walker *pWalker,
  પદાવલિ *p_પદાવલિ
){
  પૂર્ણાંક n;             /* Number of arguments */
  પદાવલિ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION );
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly)
   || (pList = p_પદાવલિ->x.pList)==0
  ){;
    n = 0;
  }નહીંતર{
    n = pList->n_પદાવલિ;
    sqlite3_Walk_પદાવલિ_List(pWalker, pList);
    જો( pWalker->eCode==0 ) પાછા WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, p_પદાવલિ->u.zToken, n, ENC(db), 0);
  જો( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc))
  ){
    pWalker->eCode = 0;
    પાછા WRC_Abort;
  }
  પાછા WRC_Prune;
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
સ્થિર પૂર્ણાંક પદાવલિ_ગાંઠ_Is_Constant(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પ્રતિજ્ઞા( pWalker->eCode>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  જો( pWalker->eCode==2 && પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) ){
    pWalker->eCode = 0;
    પાછા WRC_Abort;
  }

  પસંદગી( p_પદાવલિ->op ){
    સ્થિતિ TK_FUNCTION:
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
        pWalker->eCode = 0;  /* Window functions are never constant */
        પાછા WRC_Abort;
      }નહીંતર જો( pWalker->eCode>=4 && pWalker->eCode<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        જો( pWalker->eCode==5 ) પદાવલિ_Set_Property(p_પદાવલિ, EP_FromDDL);
        પાછા WRC_Continue;
      }નહીંતર જો( પદાવલિ_Has_Property(p_પદાવલિ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        પાછા WRC_Continue;
      }નહીંતર જો( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        પાછા પદાવલિ_ગાંઠ_Is_Constant_Function(pWalker, p_પદાવલિ);
      }નહીંતર{
        pWalker->eCode = 0;  /* Function call is not constant */
        પાછા WRC_Abort;
      }
    સ્થિતિ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      જો( sqlite3_પદાવલિ_Id_To_True_False(p_પદાવલિ) ){
        પાછા WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_COLUMN:
    સ્થિતિ TK_AGG_FUNCTION:
    સ્થિતિ TK_AGG_COLUMN:
      testcase( p_પદાવલિ->op==TK_ID );
      testcase( p_પદાવલિ->op==TK_COLUMN );
      testcase( p_પદાવલિ->op==TK_AGG_FUNCTION );
      testcase( p_પદાવલિ->op==TK_AGG_COLUMN );
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol) && pWalker->eCode!=2 ){
        પાછા WRC_Continue;
      }
      જો( pWalker->eCode==3 && p_પદાવલિ->iTable==pWalker->u.iCur ){
        પાછા WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_IF_NULL_ROW:
    સ્થિતિ TK_REGISTER:
    સ્થિતિ TK_DOT:
    સ્થિતિ TK_RAISE:
      testcase( p_પદાવલિ->op==TK_REGISTER );
      testcase( p_પદાવલિ->op==TK_IF_NULL_ROW );
      testcase( p_પદાવલિ->op==TK_DOT );
      testcase( p_પદાવલિ->op==TK_RAISE );
      pWalker->eCode = 0;
      પાછા WRC_Abort;
    સ્થિતિ TK_IS:
    સ્થિતિ TK_ISNOT:
      જો( pWalker->eCode==6 ){
        pWalker->eCode = 0;
        પાછા WRC_Abort;
      }
      પાછા WRC_Continue;
    સ્થિતિ TK_VARIABLE:
      જો( pWalker->eCode==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_પદાવલિ->op = TK_NULL;
      }નહીંતર જો( pWalker->eCode==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->eCode = 0;
        પાછા WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    મૂળભૂત:
      testcase( p_પદાવલિ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_પદાવલિ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      પાછા WRC_Continue;
  }
}
સ્થિર પૂર્ણાંક પદાવલિ_Is_Const(Parse *pParse, પદાવલિ *p, પૂર્ણાંક initFlag){
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Constant(Parse *pParse, પદાવલિ *p){
  પાછા પદાવલિ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
પૂર્ણાંક sqlite3_પદાવલિ_List_Is_Constant(Parse *pParse, પદાવલિ_List *pList, પૂર્ણાંક bNoIs){
  અચલ પૂર્ણાંક iInit = bNoIs ? 6 : 1;
  પૂર્ણાંક ii;
  માટે(ii=0; ii<pList->n_પદાવલિ; ii++){
    જો( 0==પદાવલિ_Is_Const(pParse, pList->a[ii].p_પદાવલિ, iInit) ) પાછા 0;
  }
  પાછા 1;
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
સ્થિર પૂર્ણાંક sqlite3_પદાવલિ_Is_Constant_Not_Join(Parse *pParse, પદાવલિ *p){
  પાછા પદાવલિ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Select_Walk_Table_Constant(Walker *pWalker, Select *pSelect){
  પ્રતિજ્ઞા( pSelect!=0 );
  પ્રતિજ્ઞા( pWalker->eCode==3 || pWalker->eCode==0 );
  જો( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->eCode = 0;
    પાછા WRC_Abort;
  }
  પાછા WRC_Prune;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Table_Constant(પદાવલિ *p, પૂર્ણાંક iCur, પૂર્ણાંક bAllowSubq){
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Is_Constant;
  જો( bAllowSubq ){
    w.xSelectCallback = પદાવલિ_Select_Walk_Table_Constant;
  }નહીંતર{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  }
  w.u.iCur = iCur;
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Single_Table_Constraint(
  પદાવલિ *p_પદાવલિ,                 /* The constraint */
  અચલ SrcList *pSrcList,     /* Complete FROM clause */
  પૂર્ણાંક iSrc,                    /* Which element of pSrcList to use */
  પૂર્ણાંક bAllowSubq               /* Allow non-correlated subqueries */
){
  અચલ SrcItem *pSrc = &pSrcList->a[iSrc];
  જો( pSrc->fg.jointype & JT_LTORJ ){
    પાછા 0;  /* rule (3) */
  }
  જો( pSrc->fg.jointype & JT_ડાબું ){
    જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) ) પાછા 0;   /* rule (4a) */
    જો( p_પદાવલિ->w.iJoin!=pSrc->iCursor ) પાછા 0;         /* rule (4b) */
  }નહીંતર{
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) ) પાછા 0;    /* rule (5) */
  }
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    પૂર્ણાંક jj;
    માટે(jj=0; jj<iSrc; jj++){
      જો( p_પદાવલિ->w.iJoin==pSrcList->a[jj].iCursor ){
        જો( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          પાછા 0;  /* restriction (6) */
        }
        તોડો;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  પાછા sqlite3_પદાવલિ_Is_Table_Constant(p_પદાવલિ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
સ્થિર પૂર્ણાંક પદાવલિ_ગાંઠ_Is_Constant_Or_Group_By(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પદાવલિ_List *pGroupBy = pWalker->u.pGroupBy;
  પૂર્ણાંક i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  માટે(i=0; i<pGroupBy->n_પદાવલિ; i++){
    પદાવલિ *p = pGroupBy->a[i].p_પદાવલિ;
    જો( sqlite3_પદાવલિ_Compare(0, p_પદાવલિ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_પદાવલિ_NNColl_Seq(pWalker->pParse, p);
      જો( sqlite3_Is_દ્વિઅંકી(pColl) ){
        પાછા WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
    pWalker->eCode = 0;
    પાછા WRC_Abort;
  }

  પાછા પદાવલિ_ગાંઠ_Is_Constant(pWalker, p_પદાવલિ);
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Constant_Or_Group_By(Parse *pParse, પદાવલિ *p, પદાવલિ_List *pGroupBy){
  Walker w;
  w.eCode = 1;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Is_Constant_Or_Group_By;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Constant_Or_Function(પદાવલિ *p, u8 isInit){
  પ્રતિજ્ઞા( isInit==0 || isInit==1 );
  પાછા પદાવલિ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Contains_Subquery(પદાવલિ *p){
  Walker w;
  w.eCode = 1;
  w.x_પદાવલિ_Callback = sqlite3_પદાવલિ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ifdef SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#endif
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode==0;
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
પૂર્ણાંક sqlite3_પદાવલિ_Is_Integer(અચલ પદાવલિ *p, પૂર્ણાંક *p_કિંમત, Parse *pParse, પૂર્ણાંક bRSI){
  પૂર્ણાંક iSign = 1;   /* Either +1 or -1. */
  જ્યારે( 1/*exit-by-break*/ ){
    જો( NEVER(p==0) ) પાછા 0;
    જો( પદાવલિ_Use_UValue(p) ){
      પ્રતિજ્ઞા( p->op==TK_INTEGER );
      *p_કિંમત = p->u.i_કિંમત*iSign;
      જો( bRSI && pParse==0 && p->op2 ){
        પાછા 0;
      }
      પાછા 1;
    }
    જો( p->op==TK_UPLUS ){
      p = p->p_ડાબું;
      pParse = 0;
      ચાલુ;
    }
    જો( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ડાબું;
      pParse = 0;
      ચાલુ;
    }
    જો( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_કિંમત *pVal; /* The variable */
      પૂર્ણાંક rc = 0;          /* 1 if successful, 0 if failed */
      પ્રતિજ્ઞા( iSign==1 );
      જો( NEVER(pParse->pVdbe==0) ) તોડો;
      જો( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) તોડો;
      pVal = sqlite3_Vdbe_Get_Bound_કિંમત(pParse->pReprepare, p->iColumn,
                                      SQLITE_AFF_BLOB);
      જો( pVal ){
        i64 vv;
        જો( sqlite3_કિંમત_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_કિંમત_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_કિંમત = (પૂર્ણાંક)vv;
          rc = 1;       
        }
        sqlite3_કિંમત_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->iColumn, bRSI);
      પાછા rc;
    }
    તોડો;
  }
  પાછા 0;
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
પૂર્ણાંક sqlite3_પદાવલિ_Can_Be_Null(અચલ પદાવલિ *p){
  u8 op;
  પ્રતિજ્ઞા( p!=0 );
  જ્યારે( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ડાબું;
    પ્રતિજ્ઞા( p!=0 );
  }
  op = p->op;
  જો( op==TK_REGISTER ) op = p->op2;
  પસંદગી( op ){
    સ્થિતિ TK_INTEGER:
    સ્થિતિ TK_STRING:
    સ્થિતિ TK_FLOAT:
    સ્થિતિ TK_BLOB:
      પાછા 0;
    સ્થિતિ TK_COLUMN:
      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p) );
      પાછા પદાવલિ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          || (p->iColumn==XN_ROWID && IsView(p->y.pTab))
#endif
          || (p->iColumn>=0
              && p->y.pTab->aCol!=0 /* Possible due to prior error */
              && ALWAYS(p->iColumn<p->y.pTab->nCol)
              && p->y.pTab->aCol[p->iColumn].notNull==0);
    મૂળભૂત:
      પાછા 1;
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
પૂર્ણાંક sqlite3_પદાવલિ_Needs_No_Affinity_Change(અચલ પદાવલિ *p, અક્ષર aff){
  u8 op;
  પૂર્ણાંક unaryMinus = 0;
  જો( aff==SQLITE_AFF_BLOB ) પાછા 1;
  જ્યારે( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    જો( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ડાબું;
  }
  op = p->op;
  જો( op==TK_REGISTER ) op = p->op2;
  પસંદગી( op ){
    સ્થિતિ TK_INTEGER: {
      પાછા aff>=SQLITE_AFF_NUMERIC;
    }
    સ્થિતિ TK_FLOAT: {
      પાછા aff>=SQLITE_AFF_NUMERIC;
    }
    સ્થિતિ TK_STRING: {
      પાછા !unaryMinus && aff==SQLITE_AFF_TEXT;
    }
    સ્થિતિ TK_BLOB: {
      પાછા !unaryMinus;
    }
    સ્થિતિ TK_COLUMN: {
      પ્રતિજ્ઞા( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      પાછા aff>=SQLITE_AFF_NUMERIC && p->iColumn<0;
    }
    મૂળભૂત: {
      પાછા 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
પૂર્ણાંક sqlite3IsRowid(અચલ અક્ષર *z){
  જો( sqlite3StrICmp(z, "_ROWID_")==0 ) પાછા 1;
  જો( sqlite3StrICmp(z, "ROWID")==0 ) પાછા 1;
  જો( sqlite3StrICmp(z, "OID")==0 ) પાછા 1;
  પાછા 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
અચલ અક્ષર *sqlite3RowidAlias(Table *pTab){
  અચલ અક્ષર *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  પૂર્ણાંક ii;
  પ્રતિજ્ઞા( VisibleRowid(pTab) );
  માટે(ii=0; ii<ArraySize(azOpt); ii++){
    જો( sqlite3ColumnIndex(pTab, azOpt[ii])<0 ) પાછા azOpt[ii];
  }
  પાછા 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ifndef SQLITE_OMIT_SUBQUERY
સ્થિર Select *isCandidateForInOpt(અચલ પદાવલિ *pX){
  Select *p;
  SrcList *pSrc;
  પદાવલિ_List *pEList;
  Table *pTab;
  પૂર્ણાંક i;
  જો( !પદાવલિ_Use_XSelect(pX) ) પાછા 0;                 /* Not a subquery */
  જો( પદાવલિ_Has_Property(pX, EP_VarSelect)  ) પાછા 0;  /* Correlated subq */
  p = pX->x.pSelect;
  જો( p->pPrior ) પાછા 0;              /* Not a compound SELECT */
  જો( p->selFlags & SF_Aggregate ){
    પાછા 0; /* No GROUP BY keyword or aggregate functions */
  }
  પ્રતિજ્ઞા( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  જો( p->pLimit ) પાછા 0;              /* Has no LIMIT clause */
  જો( p->pWhere ) પાછા 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  પ્રતિજ્ઞા( pSrc!=0 );
  જો( pSrc->nSrc!=1 ) પાછા 0;          /* Single term in FROM clause */
  જો( pSrc->a[0].fg.isSubquery) પાછા 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( !IsView(pTab)  );              /* FROM clause is not a view */
  જો( IsVirtual(pTab) ) પાછા 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  પ્રતિજ્ઞા( pEList!=0 );
  /* All SELECT results must be columns. */
  માટે(i=0; i<pEList->n_પદાવલિ; i++){
    પદાવલિ *pRes = pEList->a[i].p_પદાવલિ;
    જો( pRes->op!=TK_COLUMN ) પાછા 0;
    પ્રતિજ્ઞા( pRes->iTable==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  પાછા p;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
સ્થિર શૂન્ય sqlite3SetHasNullFlag(
  Vdbe *v,           /* Write new code into this statement under construction */
  પૂર્ણાંક iCur,          /* Cursor for the index */
  પૂર્ણાંક regHasNull,    /* Register in which to store hasNull flag */
  પૂર્ણાંક eSortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  પૂર્ણાંક addr1;
  પૂર્ણાંક op;
  sqlite3VdbeAddOp2(v, OP_Integer, 0, regHasNull);
  જો( eSortOrder==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }નહીંતર{
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
સ્થિર પૂર્ણાંક sqlite3InRhsIsConstant(Parse *pParse, પદાવલિ *pIn){
  પદાવલિ *pLHS;
  પૂર્ણાંક res;
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pIn, EP_xIsSelect) );
  pLHS = pIn->p_ડાબું;
  pIn->p_ડાબું = 0;
  res = sqlite3_પદાવલિ_Is_Constant(pParse, pIn);
  pIn->p_ડાબું = pLHS;
  પાછા res;
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
પૂર્ણાંક sqlite3FindInIndex(
  Parse *pParse,             /* Parsing context */
  પદાવલિ *pX,                  /* The IN expression */
  u32 inFlags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  પૂર્ણાંક *prRhsHasNull,         /* Register holding NULL status.  See notes */
  પૂર્ણાંક *aiMap,                /* Mapping from Index fields to RHS fields */
  પૂર્ણાંક *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  પૂર્ણાંક eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  પૂર્ણાંક iTab;                             /* Cursor of the RHS table */
  પૂર્ણાંક mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  પ્રતિજ્ઞા( pX->op==TK_IN );
  mustBeUnique = (inFlags & IN_INDEX_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  જો( prRhsHasNull && પદાવલિ_Use_XSelect(pX) ){
    પૂર્ણાંક i;
    પદાવલિ_List *pEList = pX->x.pSelect->pEList;
    માટે(i=0; i<pEList->n_પદાવલિ; i++){
      જો( sqlite3_પદાવલિ_Can_Be_Null(pEList->a[i].p_પદાવલિ) ) તોડો;
    }
    જો( i==pEList->n_પદાવલિ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  જો( pParse->nErr==0 && (p = isCandidateForInOpt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Table *pTab;                           /* Table <table>. */
    પૂર્ણાંક iDb;                               /* Database idx for pTab */
    પદાવલિ_List *pEList = p->pEList;
    પૂર્ણાંક n_પદાવલિ = pEList->n_પદાવલિ;

    પ્રતિજ્ઞા( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    પ્રતિજ્ઞા( p->pEList->a[0].p_પદાવલિ!=0 ); /* Because of isCandidateForInOpt(p) */
    પ્રતિજ્ઞા( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    પ્રતિજ્ઞા( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    પ્રતિજ્ઞા(v);  /* sqlite3GetVdbe() has always been previously called */
    જો( n_પદાવલિ==1 && pEList->a[0].p_પદાવલિ->iColumn<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      પૂર્ણાંક iAddr = sqlite3VdbeAddOp0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3VdbeJumpHere(v, iAddr);
    }નહીંતર{
      Index *pIdx;                         /* Iterator variable */
      પૂર્ણાંક affinity_ok = 1;
      પૂર્ણાંક i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      માટે(i=0; i<n_પદાવલિ && affinity_ok; i++){
        પદાવલિ *pLhs = sqlite3VectorFieldSubexpr(pX->p_ડાબું, i);
        પૂર્ણાંક iCol = pEList->a[i].p_પદાવલિ->iColumn;
        અક્ષર idxaff = sqlite3TableColumnAffinity(pTab,iCol); /* RHS table */
        અક્ષર cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_TEXT );
        પસંદગી( cmpaff ){
          સ્થિતિ SQLITE_AFF_BLOB:
            તોડો;
          સ્થિતિ SQLITE_AFF_TEXT:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            પ્રતિજ્ઞા( idxaff==SQLITE_AFF_TEXT );
            તોડો;
          મૂળભૂત:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      જો( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        માટે(pIdx=pTab->pIndex; pIdx && eType==0; pIdx=pIdx->pNext){
          Bitmask colUsed;      /* Columns of the index used */
          Bitmask mCol;         /* Mask for the current column */
          જો( pIdx->nColumn<n_પદાવલિ ) ચાલુ;
          જો( pIdx->pPartIdxWhere!=0 ) ચાલુ;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->nColumn==BMS-2 );
          testcase( pIdx->nColumn==BMS-1 );
          જો( pIdx->nColumn>=BMS-1 ) ચાલુ;
          જો( mustBeUnique ){
            જો( pIdx->n_ચાવી_Col>n_પદાવલિ
             ||(pIdx->nColumn>n_પદાવલિ && !IsUniqueIndex(pIdx))
            ){
              ચાલુ;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          colUsed = 0;   /* Columns of index used so far */
          માટે(i=0; i<n_પદાવલિ; i++){
            પદાવલિ *pLhs = sqlite3VectorFieldSubexpr(pX->p_ડાબું, i);
            પદાવલિ *pRhs = pEList->a[i].p_પદાવલિ;
            CollSeq *pReq = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, pLhs, pRhs);
            પૂર્ણાંક j;
 
            માટે(j=0; j<n_પદાવલિ; j++){
              જો( pIdx->aiColumn[j]!=pRhs->iColumn ) ચાલુ;
              પ્રતિજ્ઞા( pIdx->azColl[j] );
              જો( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ચાલુ;
              }
              તોડો;
            }
            જો( j==n_પદાવલિ ) તોડો;
            mCol = MASKBIT(j);
            જો( mCol & colUsed ) તોડો; /* Each column used only once */
            colUsed |= mCol;
            જો( aiMap ) aiMap[i] = j;
          }
 
          પ્રતિજ્ઞા( n_પદાવલિ>0 && n_પદાવલિ<BMS );
          પ્રતિજ્ઞા( i==n_પદાવલિ || colUsed!=(MASKBIT(n_પદાવલિ)-1) );
          જો( colUsed==(MASKBIT(n_પદાવલિ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            પૂર્ણાંક iAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3VdbeAddOp3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            પ્રતિજ્ઞા( IN_INDEX_INDEX_DESC == IN_INDEX_INDEX_ASC+1 );
            eType = IN_INDEX_INDEX_ASC + pIdx->aSortOrder[0];
 
            જો( prRhsHasNull ){
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
              u64 mask = (1<<n_પદાવલિ)-1;
              sqlite3VdbeAddOp3(v, OP_ColumnsUsed,
                                iTab, LOWER32(mask), UPPER32(mask));
#endif
              *prRhsHasNull = ++pParse->nMem;
              જો( n_પદાવલિ==1 ){
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
  જો( eType==0
   && (inFlags & IN_INDEX_NOOP_OK)
   && પદાવલિ_Use_XList(pX)
   && (!sqlite3InRhsIsConstant(pParse,pX) || pX->x.pList->n_પદાવલિ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = IN_INDEX_NOOP;
  }

  જો( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    પૂર્ણાંક rMayHaveNull = 0;
    પૂર્ણાંક bloomOk = (inFlags & IN_INDEX_MEMBERSHIP)!=0;
    eType = IN_INDEX_EPH;
    જો( inFlags & IN_INDEX_LOOP ){
      pParse->nQueryLoop = 0;
    }નહીંતર જો( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    પ્રતિજ્ઞા( pX->op==TK_IN );
    જો( !bloomOk
     && પદાવલિ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_ClonedRhsIn)!=0
    ){
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    જો( rMayHaveNull ){
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  જો( aiMap && eType!=IN_INDEX_INDEX_ASC && eType!=IN_INDEX_INDEX_DESC ){
    પૂર્ણાંક i, n;
    n = sqlite3_પદાવલિ_Vector_Size(pX->p_ડાબું);
    માટે(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  પાછા eType;
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
સ્થિર અક્ષર *પદાવલિ_INAffinity(Parse *pParse, અચલ પદાવલિ *p_પદાવલિ){
  પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
  પૂર્ણાંક nVal = sqlite3_પદાવલિ_Vector_Size(p_ડાબું);
  Select *pSelect = પદાવલિ_Use_XSelect(p_પદાવલિ) ? p_પદાવલિ->x.pSelect : 0;
  અક્ષર *zRet;

  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_IN );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  જો( zRet ){
    પૂર્ણાંક i;
    માટે(i=0; i<nVal; i++){
      પદાવલિ *pA = sqlite3VectorFieldSubexpr(p_ડાબું, i);
      અક્ષર a = sqlite3_પદાવલિ_Affinity(pA);
      જો( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_પદાવલિ, a);
      }નહીંતર{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  પાછા zRet;
}
#endif

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
શૂન્ય sqlite3SubselectError(Parse *pParse, પૂર્ણાંક nActual, પૂર્ણાંક nExpect){
  જો( pParse->nErr==0 ){
    અચલ અક્ષર *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_સંદેશ(pParse, zFmt, nActual, nExpect);
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
શૂન્ય sqlite3_Vector_Error_સંદેશ(Parse *pParse, પદાવલિ *p_પદાવલિ){
#ifndef SQLITE_OMIT_SUBQUERY
  જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
    sqlite3SubselectError(pParse, p_પદાવલિ->x.pSelect->pEList->n_પદાવલિ, 1);
  }નહીંતર
#endif
  {
    sqlite3_Error_સંદેશ(pParse, "row value misused");
  }
}

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
સ્થિર પૂર્ણાંક findCompatibleInRhsSubrtn(
  Parse *pParse,          /* Parsing context */
  પદાવલિ *p_પદાવલિ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  જો( pNewSig==0 ) પાછા 0;
  જો( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) પાછા 0;
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_IN );
  પ્રતિજ્ઞા( !પદાવલિ_Use_YSub(p_પદાવલિ) );
  પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
  પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect!=0 );
  પ્રતિજ્ઞા( (p_પદાવલિ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  માટે(; pOp<pEnd; pOp++){
    જો( pOp->p4type!=P4_SUBRTNSIG ) ચાલુ;
    પ્રતિજ્ઞા( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    પ્રતિજ્ઞા( pSig!=0 );
    જો( !pSig->bComplete ) ચાલુ;
    જો( pNewSig->selId!=pSig->selId ) ચાલુ;
    જો( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ચાલુ;
    p_પદાવલિ->y.sub.iAddr = pSig->iAddr;
    p_પદાવલિ->y.sub.regReturn = pSig->regReturn;
    p_પદાવલિ->iTable = pSig->iTable;
    પદાવલિ_Set_Property(p_પદાવલિ, EP_Subrtn);
    પાછા 1;
  }
  પાછા 0;
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
શૂન્ય sqlite3CodeRhsOfIN(
  Parse *pParse,          /* Parsing context */
  પદાવલિ *p_પદાવલિ,            /* The IN operator */
  પૂર્ણાંક iTab,               /* Use this cursor number */
  પૂર્ણાંક allowBloom          /* True to allow the use of a Bloom filter */
){
  પૂર્ણાંક addrOnce = 0;           /* Address of the OP_Once instruction at top */
  પૂર્ણાંક addr;                   /* Address of OP_OpenEphemeral instruction */
  પદાવલિ *p_ડાબું;                /* the LHS of the IN operator */
  ચાવી_Info *p_ચાવી_Info = 0;      /* Key information */
  પૂર્ણાંક nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );

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
  જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    પ્રતિજ્ઞા( !પદાવલિ_Use_XSelect(p_પદાવલિ) || p_પદાવલિ->x.pSelect!=0 );
    જો( પદાવલિ_Use_XSelect(p_પદાવલિ) && (p_પદાવલિ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, કદ(pSig[0]));
      જો( pSig ){
        pSig->selId = p_પદાવલિ->x.pSelect->selId;
        pSig->zAff = પદાવલિ_INAffinity(pParse, p_પદાવલિ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subrtn) 
     || findCompatibleInRhsSubrtn(pParse, p_પદાવલિ, pSig)
    ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
      જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_પદાવલિ->x.pSelect->selId));
      }
      પ્રતિજ્ઞા( પદાવલિ_Use_YSub(p_પદાવલિ) );
      sqlite3VdbeAddOp2(v, OP_Gosub, p_પદાવલિ->y.sub.regReturn,
                        p_પદાવલિ->y.sub.iAddr);
      પ્રતિજ્ઞા( iTab!=p_પદાવલિ->iTable );
      sqlite3VdbeAddOp2(v, OP_OpenDup, iTab, p_પદાવલિ->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      જો( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      પાછા;
    }

    /* Begin coding the subroutine */
    પ્રતિજ્ઞા( !પદાવલિ_Use_YWin(p_પદાવલિ) );
    પદાવલિ_Set_Property(p_પદાવલિ, EP_Subrtn);
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly|EP_Reduced) );
    p_પદાવલિ->y.sub.regReturn = ++pParse->nMem;
    p_પદાવલિ->y.sub.iAddr =
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_પદાવલિ->y.sub.regReturn) + 1;
    જો( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_પદાવલિ->y.sub.iAddr;
      pSig->regReturn = p_પદાવલિ->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (અચલ અક્ષર*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ડાબું = p_પદાવલિ->p_ડાબું;
  nVal = sqlite3_પદાવલિ_Vector_Size(p_ડાબું);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_પદાવલિ->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, p_પદાવલિ->iTable, nVal);
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
    VdbeComment((v, "Result of SELECT %u", p_પદાવલિ->x.pSelect->selId));
  }નહીંતર{
    VdbeComment((v, "RHS of IN operator"));
  }
#endif
  p_ચાવી_Info = sqlite3_ચાવી_Info_Alloc(pParse->db, nVal, 1);
  પ્રતિજ્ઞા( p_ચાવી_Info!=0 || pParse->nErr );
  જો( p_ચાવી_Info==0 ) પાછા;

  જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_પદાવલિ->x.pSelect;
    પદાવલિ_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    જો( ALWAYS(pEList->n_પદાવલિ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      પૂર્ણાંક i;
      પૂર્ણાંક rc;
      પૂર્ણાંક addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_Set, iTab);
      dest.zAffSdst = પદાવલિ_INAffinity(pParse, p_પદાવલિ);
      pSelect->iLimit = 0;
      પ્રતિજ્ઞા( pEList!=0 );
      પ્રતિજ્ઞા( pEList->n_પદાવલિ>0 );
      પ્રતિજ્ઞા( sqlite3_ચાવી_Info_Is_Writeable(p_ચાવી_Info) );
      માટે(i=0; i<nVal; i++){
        પદાવલિ *p = sqlite3VectorFieldSubexpr(p_ડાબું, i);
        CollSeq *pColl;
        p_ચાવી_Info->aColl[i] = pColl = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_પદાવલિ
        );
        જો( !sqlite3_Is_દ્વિઅંકી(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      જો( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        પૂર્ણાંક regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (શૂન્ય *)p_ચાવી_Info, P4_KEYINFO);
        p_ચાવી_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      જો( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        જો( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      જો( rc ){
        sqlite3_ચાવી_Info_Unref(p_ચાવી_Info);
        પાછા;
      }
    }
  }નહીંતર જો( ALWAYS(p_પદાવલિ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    અક્ષર affinity;            /* Affinity of the LHS of the IN */
    પૂર્ણાંક i;
    પદાવલિ_List *pList = p_પદાવલિ->x.pList;
    સંરચના પદાવલિ_List_item *pItem;
    પૂર્ણાંક r1, r2;
    affinity = sqlite3_પદાવલિ_Affinity(p_ડાબું);
    જો( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }નહીંતર જો( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    પ્રતિજ્ઞા( p_ચાવી_Info!=0 );
    પ્રતિજ્ઞા( sqlite3_ચાવી_Info_Is_Writeable(p_ચાવી_Info) );
    p_ચાવી_Info->aColl[0] = sqlite3_પદાવલિ_Coll_Seq(pParse, p_પદાવલિ->p_ડાબું);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    માટે(i=pList->n_પદાવલિ, pItem=pList->a; i>0; i--, pItem++){
      પદાવલિ *pE2 = pItem->p_પદાવલિ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      જો( addrOnce && !sqlite3_પદાવલિ_Is_Constant(pParse, pE2) ){
        sqlite3VdbeChangeToNoop(v, addrOnce-1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        પદાવલિ_Clear_Property(p_પદાવલિ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_પદાવલિ_Code(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  જો( pSig ) pSig->bComplete = 1;
  જો( p_ચાવી_Info ){
    sqlite3VdbeChangeP4(v, addr, (શૂન્ય *)p_ચાવી_Info, P4_KEYINFO);
  }
  જો( addrOnce ){
    sqlite3VdbeAddOp1(v, OP_NullRow, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);
    /* Subroutine return */
    પ્રતિજ્ઞા( પદાવલિ_Use_YSub(p_પદાવલિ) );
    પ્રતિજ્ઞા( sqlite3VdbeGetOp(v,p_પદાવલિ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3VdbeAddOp3(v, OP_Return, p_પદાવલિ->y.sub.regReturn,
                      p_પદાવલિ->y.sub.iAddr, 1);
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
પૂર્ણાંક sqlite3CodeSubselect(Parse *pParse, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  પૂર્ણાંક rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  પૂર્ણાંક nReg;                   /* Registers to allocate */
  પદાવલિ *pLimit;               /* New limit expression */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  પૂર્ણાંક addrExplain;            /* Address of OP_Explain instruction */
#endif

  Vdbe *v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );
  જો( pParse->nErr ) પાછા 0;
  testcase( p_પદાવલિ->op==TK_EXISTS );
  testcase( p_પદાવલિ->op==TK_SELECT );
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_EXISTS || p_પદાવલિ->op==TK_SELECT );
  પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
  pSel = p_પદાવલિ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    પ્રતિજ્ઞા( પદાવલિ_Use_YSub(p_પદાવલિ) );
    sqlite3VdbeAddOp2(v, OP_Gosub, p_પદાવલિ->y.sub.regReturn,
                      p_પદાવલિ->y.sub.iAddr);
    પાછા p_પદાવલિ->iTable;
  }

  /* Begin coding the subroutine */
  પ્રતિજ્ઞા( !પદાવલિ_Use_YWin(p_પદાવલિ) );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Reduced|EP_TokenOnly) );
  પદાવલિ_Set_Property(p_પદાવલિ, EP_Subrtn);
  p_પદાવલિ->y.sub.regReturn = ++pParse->nMem;
  p_પદાવલિ->y.sub.iAddr =
    sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, p_પદાવલિ->y.sub.regReturn) + 1;

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
  જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_VarSelect) ){
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
  nReg = p_પદાવલિ->op==TK_SELECT ? pSel->pEList->n_પદાવલિ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  જો( p_પદાવલિ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    જો( (pSel->selFlags&SF_Distinct) && pSel->pLimit && pSel->pLimit->p_જમણું ){
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
    }નહીંતર{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }નહીંતર{
    dest.eDest = SRT_Exists;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  જો( pSel->pLimit ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    પદાવલિ *p_ડાબું = pSel->pLimit->p_ડાબું;
    જો( પદાવલિ_Has_Property(p_ડાબું, EP_Int_કિંમત)==0
     || (p_ડાબું->u.i_કિંમત!=1 && p_ડાબું->u.i_કિંમત!=0)
    ){
      sqlite3 *db = pParse->db;
      pLimit = sqlite3_પદાવલિ_Int32(db, 0);
      જો( pLimit ){
        pLimit->aff_પદાવલિ = SQLITE_AFF_NUMERIC;
        pLimit = sqlite3PExpr(pParse, TK_NE,
            sqlite3_પદાવલિ_Dup(db, p_ડાબું, 0), pLimit);
      }
      sqlite3_પદાવલિ_Deferred_Delete(pParse, p_ડાબું);
      pSel->pLimit->p_ડાબું = pLimit;
    }
  }નહીંતર{
    /* If there is no pre-existing limit add a limit of 1 */
    pLimit = sqlite3_પદાવલિ_Int32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, TK_LIMIT, pLimit, 0);
  }
  pSel->iLimit = 0;
  જો( sqlite3Select(pParse, pSel, &dest) ){
    p_પદાવલિ->op2 = p_પદાવલિ->op;
    p_પદાવલિ->op = TK_ERROR;
    પાછા 0;
  }
  p_પદાવલિ->iTable = rReg = dest.iSDParm;
  પદાવલિ_Set_VVAProperty(p_પદાવલિ, EP_NoReduce);
  જો( addrOnce ){
    sqlite3VdbeJumpHere(v, addrOnce);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);

  જો( dest.eDest==SRT_Mem 
   && sqlite3_પદાવલિ_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_પદાવલિ) 
  ){
    sqlite3VdbeAddOp1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  પ્રતિજ્ઞા( પદાવલિ_Use_YSub(p_પદાવલિ) );
  પ્રતિજ્ઞા( sqlite3VdbeGetOp(v,p_પદાવલિ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3VdbeAddOp3(v, OP_Return, p_પદાવલિ->y.sub.regReturn,
                    p_પદાવલિ->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3ClearTempRegCache(pParse);
  પાછા rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Check_IN(Parse *pParse, પદાવલિ *pIn){
  પૂર્ણાંક nVector = sqlite3_પદાવલિ_Vector_Size(pIn->p_ડાબું);
  જો( પદાવલિ_Use_XSelect(pIn) && !pParse->db->mallocFailed ){
    જો( nVector!=pIn->x.pSelect->pEList->n_પદાવલિ ){
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->n_પદાવલિ, nVector);
      પાછા 1;
    }
  }નહીંતર જો( nVector!=1 ){
    sqlite3_Vector_Error_સંદેશ(pParse, pIn->p_ડાબું);
    પાછા 1;
  }
  પાછા 0;
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
સ્થિર શૂન્ય sqlite3_પદાવલિ_Code_IN(
  Parse *pParse,        /* Parsing and code generating context */
  પદાવલિ *p_પદાવલિ,          /* The IN expression */
  પૂર્ણાંક destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  પૂર્ણાંક destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  પૂર્ણાંક rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  પૂર્ણાંક eType;            /* Type of the RHS */
  પૂર્ણાંક rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  પૂર્ણાંક *aiMap = 0;       /* Map from vector field to index column */
  અક્ષર *zAff = 0;       /* Affinity string for comparisons */
  પૂર્ણાંક nVector;          /* Size of vectors for this IN operator */
  પૂર્ણાંક iDummy;           /* Dummy parameter to exprCodeVector() */
  પદાવલિ *p_ડાબું;          /* The LHS of the IN operator */
  પૂર્ણાંક i;                /* loop counter */
  પૂર્ણાંક destStep2;        /* Where to jump when NULLs seen in step 2 */
  પૂર્ણાંક destStep6 = 0;    /* Start of code for Step 6 */
  પૂર્ણાંક addrTruthOp;      /* Address of opcode that determines the IN is true */
  પૂર્ણાંક destNotNull;      /* Jump here if a comparison is not true in step 6 */
  પૂર્ણાંક addr_ટોચ;          /* Top of the step-6 loop */
  પૂર્ણાંક iTab = 0;         /* Index to use */
  u8 okConstFactor = pParse->okConstFactor;

  પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p_પદાવલિ,EP_Immutable) );
  p_ડાબું = p_પદાવલિ->p_ડાબું;
  જો( sqlite3_પદાવલિ_Check_IN(pParse, p_પદાવલિ) ) પાછા;
  zAff = પદાવલિ_INAffinity(pParse, p_પદાવલિ);
  nVector = sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ->p_ડાબું);
  aiMap = (પૂર્ણાંક*)sqlite3DbMallocZero(pParse->db, nVector*કદ(પૂર્ણાંક));
  જો( pParse->db->mallocFailed ) જાઓ sqlite3_પદાવલિ_Code_IN_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, p_પદાવલિ,
                             IN_INDEX_MEMBERSHIP | IN_INDEX_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  પ્રતિજ્ઞા( pParse->nErr || nVector==1 || eType==IN_INDEX_EPH
       || eType==IN_INDEX_INDEX_ASC || eType==IN_INDEX_INDEX_DESC
  );
#ifdef SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  માટે(i=0; i<nVector; i++){
    પૂર્ણાંક j, cnt;
    માટે(cnt=j=0; j<nVector; j++) જો( aiMap[j]==i ) cnt++;
    પ્રતિજ્ઞા( cnt==1 );
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
  પ્રતિજ્ઞા( pParse->okConstFactor==okConstFactor );
  pParse->okConstFactor = 0;
  rLhs = પદાવલિ_Code_Vector(pParse, p_ડાબું, &iDummy);
  pParse->okConstFactor = okConstFactor;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  જો( eType==IN_INDEX_NOOP ){
    પદાવલિ_List *pList;
    CollSeq *pColl;
    પૂર્ણાંક labelOk = sqlite3VdbeMakeLabel(pParse);
    પૂર્ણાંક r2, regToFree;
    પૂર્ણાંક regCkNull = 0;
    પૂર્ણાંક ii;
    પ્રતિજ્ઞા( nVector==1 );
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
    pList = p_પદાવલિ->x.pList;
    pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p_પદાવલિ->p_ડાબું);
    જો( destIfNull!=destIfFalse ){
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_BitAnd, rLhs, rLhs, regCkNull);
    }
    માટે(ii=0; ii<pList->n_પદાવલિ; ii++){
      r2 = sqlite3_પદાવલિ_Code_Temp(pParse, pList->a[ii].p_પદાવલિ, &regToFree);
      જો( regCkNull && sqlite3_પદાવલિ_Can_Be_Null(pList->a[ii].p_પદાવલિ) ){
        sqlite3VdbeAddOp3(v, OP_BitAnd, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      જો( ii<pList->n_પદાવલિ-1 || destIfNull!=destIfFalse ){
        પૂર્ણાંક op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2,
                          (શૂન્ય*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_પદાવલિ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_પદાવલિ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_પદાવલિ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_પદાવલિ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }નહીંતર{
        પૂર્ણાંક op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        પ્રતિજ્ઞા( destIfNull==destIfFalse );
        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2,
                          (શૂન્ય*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    જો( regCkNull ){
      sqlite3VdbeAddOp2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    જાઓ sqlite3_પદાવલિ_Code_IN_finished;
  }

  જો( eType!=IN_INDEX_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    જો( nVector==1 ){
      અક્ષર aff = zAff[0];
      જો( aff>=SQLITE_AFF_TEXT && aff!=sqlite3_પદાવલિ_Affinity(p_ડાબું) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        પૂર્ણાંક rTmp = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }નહીંતર{
      sqlite3VdbeAddOp4(v, OP_Affinity, rLhs, nVector, 0, zAff, nVector);
    }

    માટે(i=0; i<nVector && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    જો( i!=nVector ){
      /* Need to reorder the LHS fields according to aiMap */
      પૂર્ણાંક rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      માટે(i=0; i<nVector; i++){
        testcase( aiMap[i]!=i );
        sqlite3VdbeAddOp3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  જો( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }નહીંતર{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  માટે(i=0; i<nVector; i++){
    પદાવલિ *p = sqlite3VectorFieldSubexpr(p_પદાવલિ->p_ડાબું, i);
    જો( pParse->nErr ) જાઓ sqlite3_પદાવલિ_Code_IN_oom_error;
    જો( sqlite3_પદાવલિ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3VdbeAddOp2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  જો( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    પ્રતિજ્ઞા( nVector==1 );
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3VdbeAddOp0(v, OP_Goto);  /* Return True */
  }નહીંતર{
    જો( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subrtn) ){
        અચલ VdbeOp *pOp = sqlite3VdbeGetOp(v, p_પદાવલિ->y.sub.iAddr);
        પ્રતિજ્ઞા( pOp->opcode==OP_Once || pParse->nErr );
        જો( pOp->p3>0 ){  /* tag-202407032019 */
          પ્રતિજ્ઞા( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3VdbeAddOp4Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, nVector); VdbeCoverage(v);
        }
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iTab, destIfFalse,
                           rLhs, nVector); VdbeCoverage(v);
      જાઓ sqlite3_પદાવલિ_Code_IN_finished;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3VdbeAddOp4Int(v, OP_Found, iTab, 0,
                                      rLhs, nVector); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  જો( rRhsHasNull && nVector==1 ){
    sqlite3VdbeAddOp2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  જો( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  જો( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  જો( eType==IN_INDEX_INDEX_DESC ){
    addr_ટોચ = sqlite3VdbeAddOp2(v, OP_Last, iTab, destIfFalse);
  }નહીંતર{
    testcase( eType==IN_INDEX_EPH );
    testcase( eType==IN_INDEX_ROWID );
    addr_ટોચ = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  જો( nVector>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }નહીંતર{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  માટે(i=0; i<nVector; i++){
    પદાવલિ *p;
    CollSeq *pColl;
    પૂર્ણાંક r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(p_ડાબું, i);
    જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
      પદાવલિ *pRhs = p_પદાવલિ->x.pSelect->pEList->a[i].p_પદાવલિ;
      pColl = sqlite3_દ્વિઅંકી_Compare_Coll_Seq(pParse, p, pRhs);
    }નહીંતર{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3VdbeAddOp3(v, OP_Column, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (શૂન્ય*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
  જો( nVector>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr_ટોચ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3_પદાવલિ_Code_IN_finished:
  VdbeComment((v, "end IN expr"));
sqlite3_પદાવલિ_Code_IN_oom_error:
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
સ્થિર શૂન્ય codeReal(Vdbe *v, અચલ અક્ષર *z, પૂર્ણાંક negateFlag, પૂર્ણાંક iMem){
  જો( ALWAYS(z!=0) ){
    દ્વિગુણ કિંમત;
    sqlite3AtoF(z, &કિંમત);
    પ્રતિજ્ઞા( !sqlite3IsNaN(કિંમત) ); /* The new AtoF never returns NaN */
    જો( negateFlag ) કિંમત = -કિંમત;
    sqlite3VdbeAddDouble(v, iMem, કિંમત);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
સ્થિર શૂન્ય codeInteger(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક negFlag, પૂર્ણાંક iMem){
  Vdbe *v = pParse->pVdbe;
  જો( p_પદાવલિ->flags & EP_Int_કિંમત ){
    પૂર્ણાંક i = p_પદાવલિ->u.i_કિંમત;
    પ્રતિજ્ઞા( i>=0 );
    જો( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }નહીંતર{
    પૂર્ણાંક c;
    i64 કિંમત;
    અચલ અક્ષર *z = p_પદાવલિ->u.zToken;
    પ્રતિજ્ઞા( z!=0 );
    c = sqlite3DecOrHexToI64(z, &કિંમત);
    જો( (c==3 && !negFlag) || (c==2) || (negFlag && કિંમત==SMALLEST_INT64)){
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3_Error_સંદેશ(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_પદાવલિ);
#નહીંતર
#ifndef SQLITE_OMIT_HEX_INTEGER
      જો( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_સંદેશ(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_પદાવલિ);
      }નહીંતર
#endif
      {
        codeReal(v, z, negFlag, iMem);
      }
#endif
    }નહીંતર{
      જો( negFlag ){ કિંમત = c==3 ? SMALLEST_INT64 : -કિંમત; }
      sqlite3VdbeAddInt64(v, iMem, કિંમત);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
શૂન્ય sqlite3_પદાવલિ_Code_Load_Index_Column(
  Parse *pParse,  /* The parsing context */
  Index *pIdx,    /* The index whose column is to be loaded */
  પૂર્ણાંક iTabCur,    /* Cursor pointing to a table row */
  પૂર્ણાંક iIdxCol,    /* The column of the index to be loaded */
  પૂર્ણાંક regOut      /* Store the index column value in this register */
){
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  જો( iTabCol==XN_પદાવલિ ){
    પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ );
    પ્રતિજ્ઞા( pIdx->a_Col_પદાવલિ->n_પદાવલિ>iIdxCol );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_પદાવલિ_Code_Copy(pParse, pIdx->a_Col_પદાવલિ->a[iIdxCol].p_પદાવલિ, regOut);
    pParse->iSelfTab = 0;
  }નહીંતર{
    sqlite3_પદાવલિ_Code_Get_Column_Of_Table(pParse->pVdbe, pIdx->pTable, iTabCur,
                                    iTabCol, regOut);
  }
}

#ifndef SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
શૂન્ય sqlite3_પદાવલિ_Code_Generated_Column(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the generated column */
  Column *pCol,      /* The generated column */
  પૂર્ણાંક regOut         /* Put the result in this register */
){
  પૂર્ણાંક iAddr;
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક nErr = pParse->nErr;
  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pParse->iSelfTab!=0 );
  જો( pParse->iSelfTab>0 ){
    iAddr = sqlite3VdbeAddOp3(v, OP_IfNullRow, pParse->iSelfTab-1, 0, regOut);
  }નહીંતર{
    iAddr = 0;
  }
  sqlite3_પદાવલિ_Code_Copy(pParse, sqlite3_Column_પદાવલિ(pTab,pCol), regOut);
  જો( (pCol->colFlags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    પૂર્ણાંક p3 = 2+(પૂર્ણાંક)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, OP_TypeCheck, regOut, 1, p3, (અક્ષર*)pTab, P4_TABLE);
  }નહીંતર જો( pCol->affinity>=SQLITE_AFF_TEXT ){
    sqlite3VdbeAddOp4(v, OP_Affinity, regOut, 1, 0, &pCol->affinity, 1);
  }
  જો( iAddr ) sqlite3VdbeJumpHere(v, iAddr);
  જો( pParse->nErr>nErr ) pParse->db->errByteOffset = -1;
}
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
શૂન્ય sqlite3_પદાવલિ_Code_Get_Column_Of_Table(
  Vdbe *v,        /* Parsing context */
  Table *pTab,    /* The table containing the value */
  પૂર્ણાંક iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  પૂર્ણાંક iCol,       /* Index of the column to extract */
  પૂર્ણાંક regOut      /* Extract the value into this register */
){
  Column *pCol;
  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( iCol!=XN_પદાવલિ );
  જો( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }નહીંતર{
    પૂર્ણાંક x;
    જો( IsVirtual(pTab) ){
      x = iCol;
      sqlite3VdbeAddOp3(v, OP_VColumn, iTabCur, x, regOut);
      પાછા;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
    }નહીંતર જો( (pCol = &pTab->aCol[iCol])->colFlags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      જો( pCol->colFlags & COLFLAG_BUSY ){
        sqlite3_Error_સંદેશ(pParse, "generated column loop on \"%s\"",
                        pCol->zCnName);
      }નહીંતર{
        પૂર્ણાંક savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_પદાવલિ_Code_Generated_Column(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~COLFLAG_BUSY;
      }
      પાછા;
#endif
    }નહીંતર જો( !HasRowid(pTab) ){
      testcase( iCol!=sqlite3TableColumnToStorage(pTab, iCol) );
      x = sqlite3TableColumnToIndex(sqlite3_Primary_ચાવી_Index(pTab), iCol);
    }નહીંતર{
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
પૂર્ણાંક sqlite3_પદાવલિ_Code_Get_Column(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  પૂર્ણાંક iColumn,     /* Index of the table column */
  પૂર્ણાંક iTable,      /* The cursor pointing to the table */
  પૂર્ણાંક iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  પ્રતિજ્ઞા( pParse->pVdbe!=0 );
  પ્રતિજ્ઞા( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  પ્રતિજ્ઞા( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_પદાવલિ_Code_Get_Column_Of_Table(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  જો( p5 ){
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    જો( pOp->opcode==OP_Column ) pOp->p5 = p5;
    જો( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  પાછા iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
શૂન્ય sqlite3_પદાવલિ_Code_Move(Parse *pParse, પૂર્ણાંક iFrom, પૂર્ણાંક iTo, પૂર્ણાંક nReg){
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
શૂન્ય sqlite3_પદાવલિ_To_Register(પદાવલિ *p_પદાવલિ, પૂર્ણાંક iReg){
  પદાવલિ *p = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p_પદાવલિ);
  જો( NEVER(p==0) ) પાછા;
  જો( p->op==TK_REGISTER ){
    પ્રતિજ્ઞા( p->iTable==iReg );
  }નહીંતર{
    p->op2 = p->op;
    p->op = TK_REGISTER;
    p->iTable = iReg;
    પદાવલિ_Clear_Property(p, EP_Skip);
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
સ્થિર પૂર્ણાંક પદાવલિ_Code_Vector(Parse *pParse, પદાવલિ *p, પૂર્ણાંક *piFreeable){
  પૂર્ણાંક iResult;
  પૂર્ણાંક nResult = sqlite3_પદાવલિ_Vector_Size(p);
  જો( nResult==1 ){
    iResult = sqlite3_પદાવલિ_Code_Temp(pParse, p, piFreeable);
  }નહીંતર{
    *piFreeable = 0;
    જો( p->op==TK_SELECT ){
#જો SQLITE_OMIT_SUBQUERY
      iResult = 0;
#નહીંતર
      iResult = sqlite3CodeSubselect(pParse, p);
#endif
    }નહીંતર{
      પૂર્ણાંક i;
      iResult = pParse->nMem+1;
      pParse->nMem += nResult;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p) );
      માટે(i=0; i<nResult; i++){
        sqlite3_પદાવલિ_Code_Factorable(pParse, p->x.pList->a[i].p_પદાવલિ, i+iResult);
      }
    }
  }
  પાછા iResult;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
સ્થિર શૂન્ય setDoNotMergeFlagOnCopy(Vdbe *v){
  જો( sqlite3VdbeGetLastOp(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Code_Inline_Function(
  Parse *pParse,        /* Parsing context */
  પદાવલિ_List *pFarg,      /* List of function arguments */
  પૂર્ણાંક iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  પૂર્ણાંક target            /* Store function result in this register */
){
  પૂર્ણાંક nFarg;
  Vdbe *v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pFarg!=0 );
  nFarg = pFarg->n_પદાવલિ;
  પ્રતિજ્ઞા( nFarg>0 );  /* All in-line functions have at least one argument */
  પસંદગી( iFuncId ){
    સ્થિતિ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      પૂર્ણાંક endCoalesce = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક i;
      પ્રતિજ્ઞા( nFarg>=2 );
      sqlite3_પદાવલિ_Code(pParse, pFarg->a[0].p_પદાવલિ, target);
      માટે(i=1; i<nFarg; i++){
        sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
        VdbeCoverage(v);
        sqlite3_પદાવલિ_Code(pParse, pFarg->a[i].p_પદાવલિ, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      તોડો;
    }
    સ્થિતિ INLINEFUNC_iif: {
      પદાવલિ case_પદાવલિ;
      memset(&case_પદાવલિ, 0, કદ(case_પદાવલિ));
      case_પદાવલિ.op = TK_CASE;
      case_પદાવલિ.x.pList = pFarg;
      પાછા sqlite3_પદાવલિ_Code_Target(pParse, &case_પદાવલિ, target);
    }
#ifdef SQLITE_ENABLE_OFFSET_SQL_FUNC
    સ્થિતિ INLINEFUNC_sqlite_offset: {
      પદાવલિ *pArg = pFarg->a[0].p_પદાવલિ;
      જો( pArg->op==TK_COLUMN && pArg->iTable>=0 ){
        sqlite3VdbeAddOp3(v, OP_Offset, pArg->iTable, pArg->iColumn, target);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      તોડો;
    }
#endif
    મૂળભૂત: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      પ્રતિજ્ઞા( nFarg==1 || nFarg==2 );
      target = sqlite3_પદાવલિ_Code_Target(pParse, pFarg->a[0].p_પદાવલિ, target);
      તોડો;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#જો !defined(SQLITE_UNTESTABLE)
    સ્થિતિ INLINEFUNC_પદાવલિ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      પ્રતિજ્ઞા( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_પદાવલિ_Compare(0,pFarg->a[0].p_પદાવલિ, pFarg->a[1].p_પદાવલિ,-1),
         target);
      તોડો;
    }

    સ્થિતિ INLINEFUNC_પદાવલિ_implies_પદાવલિ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      પ્રતિજ્ઞા( nFarg==2 );
      sqlite3VdbeAddOp2(v, OP_Integer,
         sqlite3_પદાવલિ_Implies_પદાવલિ(pParse,pFarg->a[0].p_પદાવલિ, pFarg->a[1].p_પદાવલિ,-1),
         target);
      તોડો;
    }

    સ્થિતિ INLINEFUNC_implies_nonnull_row: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      પદાવલિ *pA1;
      પ્રતિજ્ઞા( nFarg==2 );
      pA1 = pFarg->a[1].p_પદાવલિ;
      જો( pA1->op==TK_COLUMN ){
        sqlite3VdbeAddOp2(v, OP_Integer,
           sqlite3_પદાવલિ_Implies_Non_Null_Row(pFarg->a[0].p_પદાવલિ,pA1->iTable,1),
           target);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      તોડો;
    }

    સ્થિતિ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      અચલ અક્ષર *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      અક્ષર aff;
      પ્રતિજ્ઞા( nFarg==1 );
      aff = sqlite3_પદાવલિ_Affinity(pFarg->a[0].p_પદાવલિ);
      પ્રતિજ્ઞા( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3VdbeLoadString(v, target,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      તોડો;
    }
#endif /* !defined(SQLITE_UNTESTABLE) */
  }
  પાછા target;
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
સ્થિર પૂર્ણાંક પદાવલિ_ગાંઠ_Can_Return_Subtype(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક n;
  FuncDef *pDef;
  sqlite3 *db;
  જો( p_પદાવલિ->op==TK_CASE || p_પદાવલિ->op==TK_UPLUS 
   || p_પદાવલિ->op==TK_COLLATE || p_પદાવલિ->op==TK_CAST 
  ){
    પાછા WRC_Continue;
  }
  જો( p_પદાવલિ->op!=TK_FUNCTION && p_પદાવલિ->op!=TK_AGG_FUNCTION ){
    પાછા WRC_Prune;
  }
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  db = pWalker->pParse->db;
  n = p_પદાવલિ->x.pList ? p_પદાવલિ->x.pList->n_પદાવલિ : 0;
  pDef = sqlite3FindFunction(db, p_પદાવલિ->u.zToken, n, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  જો( pDef==0 ) પાછા WRC_Continue;
#નહીંતર
  પ્રતિજ્ઞા( pDef!=0 );
#endif
  જો( (pDef->funcFlags & SQLITE_RESULT_SUBTYPE)!=0 ){
    pWalker->eCode = 1;
    પાછા WRC_Abort;
  }
  પાછા WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Can_Return_Subtype(Parse *pParse, પદાવલિ *p_પદાવલિ){
  Walker w;
  memset(&w, 0, કદ(w));
  w.pParse = pParse;
  w.x_પદાવલિ_Callback = પદાવલિ_ગાંઠ_Can_Return_Subtype;
  sqlite3_Walk_પદાવલિ(&w, p_પદાવલિ);
  પાછા w.eCode;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક sqlite3_Indexed_પદાવલિ_Lookup(
  Parse *pParse,   /* The parsing context */
  પદાવલિ *p_પદાવલિ,     /* The expression to potentially bypass */
  પૂર્ણાંક target       /* Where to store the result of the expression */
){
  Indexed_પદાવલિ *p;
  Vdbe *v;
  માટે(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 પદાવલિ_Aff;
    પૂર્ણાંક iDataCur = p->iDataCur;
    જો( iDataCur<0 ) ચાલુ;
    જો( pParse->iSelfTab ){
      જો( p->iDataCur!=pParse->iSelfTab-1 ) ચાલુ;
      iDataCur = -1;
    }
    જો( sqlite3_પદાવલિ_Compare(0, p_પદાવલિ, p->p_પદાવલિ, iDataCur)!=0 ) ચાલુ;
    પ્રતિજ્ઞા( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    પદાવલિ_Aff = sqlite3_પદાવલિ_Affinity(p_પદાવલિ);
    જો( (પદાવલિ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (પદાવલિ_Aff==SQLITE_AFF_TEXT && p->aff!=SQLITE_AFF_TEXT)
     || (પદાવલિ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ચાલુ;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_SubtArg)
     && sqlite3_પદાવલિ_Can_Return_Subtype(pParse, p_પદાવલિ) 
    ){
      ચાલુ;
    }

    v = pParse->pVdbe;
    પ્રતિજ્ઞા( v!=0 );
    જો( p->bMaybeNullRow || sqlite3_પદાવલિ_Is_Constant(pParse, p_પદાવલિ) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      પૂર્ણાંક addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_IfNullRow, p->iIdxCur, addr+3, target);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr+2);
    }નહીંતર{
      sqlite3VdbeAddOp3(v, OP_Column, p->iIdxCur, p->iIdxCol, target);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->iIdxCol));
    }
    પાછા target;
  }
  પાછા -1;  /* Not found */
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
સ્થિર પૂર્ણાંક પદાવલિ_Partidx_પદાવલિ_Lookup(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક iTarget){
  Indexed_પદાવલિ *p;
  માટે(p=pParse->p_Idx_Part_પદાવલિ; p; p=p->pIENext){
    જો( p_પદાવલિ->iColumn==p->iIdxCol && p_પદાવલિ->iTable==p->iDataCur ){
      Vdbe *v = pParse->pVdbe;
      પૂર્ણાંક addr = 0;
      પૂર્ણાંક ret;

      જો( p->bMaybeNullRow ){
        addr = sqlite3VdbeAddOp1(v, OP_IfNullRow, p->iIdxCur);
      }
      ret = sqlite3_પદાવલિ_Code_Target(pParse, p->p_પદાવલિ, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (અચલ અક્ષર*)&p->aff, 1);
      જો( sqlite3_પદાવલિ_Can_Return_Subtype(pParse, p->p_પદાવલિ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3VdbeAddOp1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      જો( addr ){
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      પાછા ret;
    }
  }
  પાછા 0;
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
સ્થિર SQLITE_NOINLINE પૂર્ણાંક પદાવલિ_Code_Target_And_Or(
  Parse *pParse,     /* Parsing context */
  પદાવલિ *p_પદાવલિ,       /* AND or OR expression to be coded */
  પૂર્ણાંક target,        /* Put result in this register, guaranteed */
  પૂર્ણાંક *pTmpReg       /* Write a temporary register here */
){
  પૂર્ણાંક op;            /* The opcode.  TK_AND or TK_OR */
  પૂર્ણાંક skipOp;        /* Opcode for the branch that skips one operand */
  પૂર્ણાંક addrSkip;      /* Branch instruction that skips one of the operands */
  પૂર્ણાંક regSS = 0;     /* Register holding computed operand when other omitted */
  પૂર્ણાંક r1, r2;        /* Registers for left and right operands, respectively */
  પદાવલિ *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  op = p_પદાવલિ->op;
  પ્રતિજ્ઞા( op==TK_AND || op==TK_OR );
  પ્રતિજ્ઞા( TK_AND==OP_And );            testcase( op==TK_AND );
  પ્રતિજ્ઞા( TK_OR==OP_Or );              testcase( op==TK_OR );
  પ્રતિજ્ઞા( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_પદાવલિ_Simplified_And_Or(p_પદાવલિ);
  જો( pAlt!=p_પદાવલિ ){
    r1 = sqlite3_પદાવલિ_Code_Target(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, OP_And, r1, r1, target);
    પાછા target;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  જો( પદાવલિ_Eval_Rhs_First(p_પદાવલિ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ->p_જમણું, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, pTmpReg);
  }નહીંતર{
    /* Compute the left operand first */
    r1 = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ->p_ડાબું, target);
    જો( પદાવલિ_Has_Property(p_પદાવલિ->p_જમણું, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }નહીંતર{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  testcase( (*pTmpReg)==0 );
  જો( addrSkip ){
    sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, OP_Or, regSS, regSS, target);
    VdbeComment((v, "short-circut value"));
  }
  પાછા target;
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
પૂર્ણાંક sqlite3_પદાવલિ_Code_Target(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  પૂર્ણાંક op;                   /* The opcode being coded */
  પૂર્ણાંક inReg = target;       /* Results stored in register inReg */
  પૂર્ણાંક regFree1 = 0;         /* If non-zero free this temporary register */
  પૂર્ણાંક regFree2 = 0;         /* If non-zero free this temporary register */
  પૂર્ણાંક r1, r2;               /* Various register numbers */
  પદાવલિ tempX;               /* Temporary expression node */
  પૂર્ણાંક p5 = 0;

  પ્રતિજ્ઞા( target>0 && target<=pParse->nMem );
  પ્રતિજ્ઞા( v!=0 );

પદાવલિ_code_doover:
  જો( p_પદાવલિ==0 ){
    op = TK_NULL;
  }નહીંતર જો( pParse->pIdxEpr!=0
   && !પદાવલિ_Has_Property(p_પદાવલિ, EP_Leaf)
   && (r1 = sqlite3_Indexed_પદાવલિ_Lookup(pParse, p_પદાવલિ, target))>=0
  ){
    પાછા r1;
  }નહીંતર{
    પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p_પદાવલિ,EP_Immutable) );
    op = p_પદાવલિ->op;
  }
  પ્રતિજ્ઞા( op!=TK_ORDER );
  પસંદગી( op ){
    સ્થિતિ TK_AGG_COLUMN: {
      AggInfo *pAggInfo = p_પદાવલિ->pAggInfo;
      સંરચના AggInfo_col *pCol;
      પ્રતિજ્ઞા( pAggInfo!=0 );
      પ્રતિજ્ઞા( p_પદાવલિ->iAgg>=0 );
      જો( p_પદાવલિ->iAgg>=pAggInfo->nColumn ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
#ifdef SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3VdbeAddOp3(v, OP_NotNull, target, 1, 20230325);
        VdbeCoverageNeverTaken(v);
#endif
        તોડો;
      }
      pCol = &pAggInfo->aCol[p_પદાવલિ->iAgg];
      જો( !pAggInfo->directMode ){
        પાછા AggInfoColumnReg(pAggInfo, p_પદાવલિ->iAgg);
      }નહીંતર જો( pAggInfo->useSortingIdx ){
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                              pCol->iSorterColumn, target);
        જો( pTab==0 ){
          /* No comment added */
        }નહીંતર જો( pCol->iColumn<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }નહીંતર{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->aCol[pCol->iColumn].zCnName));
          જો( pTab->aCol[pCol->iColumn].affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
          }
        }
        પાછા target;
      }નહીંતર જો( p_પદાવલિ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3VdbeAddOp3(v, OP_Column, p_પદાવલિ->iTable, p_પદાવલિ->iColumn, target);
        પાછા target;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    સ્થિતિ TK_COLUMN: {
      પૂર્ણાંક iTab = p_પદાવલિ->iTable;
      પૂર્ણાંક iReg;
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        પૂર્ણાંક aff;
        iReg = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ->p_ડાબું,target);
        પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
        પ્રતિજ્ઞા( p_પદાવલિ->y.pTab!=0 );
        aff = sqlite3TableColumnAffinity(p_પદાવલિ->y.pTab, p_પદાવલિ->iColumn);
        જો( aff>SQLITE_AFF_BLOB ){
          સ્થિર અચલ અક્ષર zAff[] = "B\000C\000D\000E\000F";
          પ્રતિજ્ઞા( SQLITE_AFF_BLOB=='A' );
          પ્રતિજ્ઞા( SQLITE_AFF_TEXT=='B' );
          sqlite3VdbeAddOp4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        પાછા iReg;
      }
      જો( iTab<0 ){
        જો( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          Column *pCol;
          Table *pTab;
          પૂર્ણાંક iSrc;
          પૂર્ણાંક iCol = p_પદાવલિ->iColumn;
          પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
          pTab = p_પદાવલિ->y.pTab;
          પ્રતિજ્ઞા( pTab!=0 );
          પ્રતિજ્ઞા( iCol>=XN_ROWID );
          પ્રતિજ્ઞા( iCol<pTab->nCol );
          જો( iCol<0 ){
            પાછા -1-pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          testcase( iCol!=sqlite3TableColumnToStorage(pTab,iCol) );
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
          જો( pCol->colFlags & COLFLAG_GENERATED ){
            જો( pCol->colFlags & COLFLAG_BUSY ){
              sqlite3_Error_સંદેશ(pParse, "generated column loop on \"%s\"",
                              pCol->zCnName);
              પાછા 0;
            }
            pCol->colFlags |= COLFLAG_BUSY;
            જો( pCol->colFlags & COLFLAG_NOTAVAIL ){
              sqlite3_પદાવલિ_Code_Generated_Column(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            પાછા iSrc;
          }નહીંતર
#endif /* SQLITE_OMIT_GENERATED_COLUMNS */
          જો( pCol->affinity==SQLITE_AFF_REAL ){
            sqlite3VdbeAddOp2(v, OP_SCopy, iSrc, target);
            sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
            પાછા target;
          }નહીંતર{
            પાછા iSrc;
          }
        }નહીંતર{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      નહીંતર જો( pParse->p_Idx_Part_પદાવલિ 
       && 0!=(r1 = પદાવલિ_Partidx_પદાવલિ_Lookup(pParse, p_પદાવલિ, target))
      ){
        પાછા r1;
      }
      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->y.pTab!=0 );
      iReg = sqlite3_પદાવલિ_Code_Get_Column(pParse, p_પદાવલિ->y.pTab,
                               p_પદાવલિ->iColumn, iTab, target,
                               p_પદાવલિ->op2);
      પાછા iReg;
    }
    સ્થિતિ TK_INTEGER: {
      codeInteger(pParse, p_પદાવલિ, 0, target);
      પાછા target;
    }
    સ્થિતિ TK_TRUEFALSE: {
      sqlite3VdbeAddOp2(v, OP_Integer, sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ), target);
      પાછા target;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    સ્થિતિ TK_FLOAT: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      codeReal(v, p_પદાવલિ->u.zToken, 0, target);
      પાછા target;
    }
#endif
    સ્થિતિ TK_STRING: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3VdbeLoadString(v, target, p_પદાવલિ->u.zToken);
      પાછા target;
    }
    સ્થિતિ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3VdbeAddOp3(v, OP_Null, 0, target, target + p_પદાવલિ->y.nReg - 1);
      પાછા target;
    }
    મૂળભૂત: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      પ્રતિજ્ઞા( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      પાછા target;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    સ્થિતિ TK_BLOB: {
      પૂર્ણાંક n;
      અચલ અક્ષર *z;
      અક્ષર *zBlob;
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      પ્રતિજ્ઞા( p_પદાવલિ->u.zToken[0]=='x' || p_પદાવલિ->u.zToken[0]=='X' );
      પ્રતિજ્ઞા( p_પદાવલિ->u.zToken[1]=='\'' );
      z = &p_પદાવલિ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      પ્રતિજ્ઞા( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      પાછા target;
    }
#endif
    સ્થિતિ TK_VARIABLE: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      પ્રતિજ્ઞા( p_પદાવલિ->u.zToken!=0 );
      પ્રતિજ્ઞા( p_પદાવલિ->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, p_પદાવલિ->iColumn, target);
      પાછા target;
    }
    સ્થિતિ TK_REGISTER: {
      પાછા p_પદાવલિ->iTable;
    }
#ifndef SQLITE_OMIT_CAST
    સ્થિતિ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ->p_ડાબું, target);
      પ્રતિજ્ઞા( inReg==target );
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3VdbeAddOp2(v, OP_Cast, target,
                        sqlite3AffinityType(p_પદાવલિ->u.zToken, 0));
      પાછા inReg;
    }
#endif /* SQLITE_OMIT_CAST */
    સ્થિતિ TK_IS:
    સ્થિતિ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_EQ: {
      પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
      પૂર્ણાંક addrIsNull = 0;
      જો( sqlite3_પદાવલિ_Is_Vector(p_ડાબું) ){
        codeVectorCompare(pParse, p_પદાવલિ, target, op, p5);
      }નહીંતર{
        જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = પદાવલિ_Compute_Operands(pParse, p_પદાવલિ,
                                     &r1, &r2, &regFree1, &regFree2);
        }નહીંતર{
          r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
          r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, &regFree2);
        }
        sqlite3VdbeAddOp2(v, OP_Integer, 1, inReg);
        codeCompare(pParse, p_ડાબું, p_પદાવલિ->p_જમણું, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            પદાવલિ_Has_Property(p_પદાવલિ,EP_Commuted));
        પ્રતિજ્ઞા(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        પ્રતિજ્ઞા(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        પ્રતિજ્ઞા(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        પ્રતિજ્ઞા(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        પ્રતિજ્ઞા(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        પ્રતિજ્ઞા(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        જો( p5==SQLITE_NULLEQ ){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, inReg);
        }નહીંતર{
          sqlite3VdbeAddOp3(v, OP_ZeroOrNull, r1, inReg, r2);
          જો( addrIsNull ){
            sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, OP_Null, 0, inReg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      તોડો;
    }
    સ્થિતિ TK_AND:
    સ્થિતિ TK_OR: {
      inReg = પદાવલિ_Code_Target_And_Or(pParse, p_પદાવલિ, target, &regFree1);
      તોડો;
    }
    સ્થિતિ TK_PLUS:
    સ્થિતિ TK_STAR:
    સ્થિતિ TK_MINUS:
    સ્થિતિ TK_REM:
    સ્થિતિ TK_BITAND:
    સ્થિતિ TK_BITOR:
    સ્થિતિ TK_SLASH:
    સ્થિતિ TK_LSHIFT:
    સ્થિતિ TK_RSHIFT:
    સ્થિતિ TK_CONCAT: {
      પૂર્ણાંક addrIsNull;
      પ્રતિજ્ઞા( TK_PLUS==OP_Add );           testcase( op==TK_PLUS );
      પ્રતિજ્ઞા( TK_MINUS==OP_Subtract );     testcase( op==TK_MINUS );
      પ્રતિજ્ઞા( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      પ્રતિજ્ઞા( TK_BITAND==OP_BitAnd );      testcase( op==TK_BITAND );
      પ્રતિજ્ઞા( TK_BITOR==OP_BitOr );        testcase( op==TK_BITOR );
      પ્રતિજ્ઞા( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      પ્રતિજ્ઞા( TK_LSHIFT==OP_Shift_ડાબું );   testcase( op==TK_LSHIFT );
      પ્રતિજ્ઞા( TK_RSHIFT==OP_Shift_જમણું );  testcase( op==TK_RSHIFT );
      પ્રતિજ્ઞા( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subquery) ){
        addrIsNull = પદાવલિ_Compute_Operands(pParse, p_પદાવલિ,
                                   &r1, &r2, &regFree1, &regFree2);
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
        r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      જો( addrIsNull ){
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
        VdbeComment((v, "short-circut value"));
      }
      તોડો;
    }
    સ્થિતિ TK_UMINUS: {
      પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
      પ્રતિજ્ઞા( p_ડાબું );
      જો( p_ડાબું->op==TK_INTEGER ){
        codeInteger(pParse, p_ડાબું, 1, target);
        પાછા target;
#ifndef SQLITE_OMIT_FLOATING_POINT
      }નહીંતર જો( p_ડાબું->op==TK_FLOAT ){
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
        codeReal(v, p_ડાબું->u.zToken, 1, target);
        પાછા target;
#endif
      }નહીંતર{
        tempX.op = TK_INTEGER;
        tempX.flags = EP_Int_કિંમત|EP_TokenOnly;
        tempX.u.i_કિંમત = 0;
        પદાવલિ_Clear_VVAProperties(&tempX);
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, &tempX, &regFree1);
        r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      તોડો;
    }
    સ્થિતિ TK_BITNOT:
    સ્થિતિ TK_NOT: {
      પ્રતિજ્ઞા( TK_BITNOT==OP_BitNot );   testcase( op==TK_BITNOT );
      પ્રતિજ્ઞા( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
      testcase( regFree1==0 );
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      તોડો;
    }
    સ્થિતિ TK_TRUTH: {
      પૂર્ણાંક isTrue;    /* IS TRUE or IS NOT TRUE */
      પૂર્ણાંક bNormal;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ->p_જમણું);
      bNormal = p_પદાવલિ->op2==TK_IS;
      testcase( isTrue && bNormal);
      testcase( !isTrue && bNormal);
      sqlite3VdbeAddOp4Int(v, OP_IsTrue, r1, inReg, !isTrue, isTrue ^ bNormal);
      તોડો;
    }
    સ્થિતિ TK_ISNULL:
    સ્થિતિ TK_NOTNULL: {
      પૂર્ણાંક addr;
      પ્રતિજ્ઞા( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      પ્રતિજ્ઞા( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      તોડો;
    }
    સ્થિતિ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_પદાવલિ->pAggInfo;
      જો( pInfo==0
       || NEVER(p_પદાવલિ->iAgg<0)
       || NEVER(p_પદાવલિ->iAgg>=pInfo->nFunc)
      ){
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
        sqlite3_Error_સંદેશ(pParse, "misuse of aggregate: %#T()", p_પદાવલિ);
      }નહીંતર{
        પાછા AggInfoFuncReg(pInfo, p_પદાવલિ->iAgg);
      }
      તોડો;
    }
    સ્થિતિ TK_FUNCTION: {
      પદાવલિ_List *pFarg;       /* List of function arguments */
      પૂર્ણાંક nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      અચલ અક્ષર *zId;       /* The function name */
      u32 constMask = 0;     /* Mask of function arguments that are constant */
      પૂર્ણાંક i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
        પાછા p_પદાવલિ->y.pWin->regResult;
      }
#endif

      જો( ConstFactorOk(pParse)
       && sqlite3_પદાવલિ_Is_Constant_Not_Join(pParse,p_પદાવલિ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        પાછા sqlite3_પદાવલિ_Code_Run_Just_Once(pParse, p_પદાવલિ, -1);
      }
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly) );
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      pFarg = p_પદાવલિ->x.pList;
      nFarg = pFarg ? pFarg->n_પદાવલિ : 0;
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      zId = p_પદાવલિ->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      જો( pDef==0 && pParse->explain ){
        pDef = sqlite3FindFunction(db, "unknown", nFarg, enc, 0);
      }
#endif
      જો( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_InternalFunc)==0)
      ){
        sqlite3_Error_સંદેશ(pParse, "unknown function: %#T()", p_પદાવલિ);
        તોડો;
      }
      જો( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        પ્રતિજ્ઞા( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        પ્રતિજ્ઞા( (pDef->funcFlags & SQLITE_FUNC_DIRECT)==0 );
        પાછા પદાવલિ_Code_Inline_Function(pParse, pFarg,
             SQLITE_PTR_TO_INT(pDef->pUserData), target);
      }નહીંતર જો( pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE) ){
        sqlite3_પદાવલિ_Function_Usable(pParse, p_પદાવલિ, pDef);
      }

      માટે(i=0; i<nFarg; i++){
        જો( i<32 && sqlite3_પદાવલિ_Is_Constant(pParse, pFarg->a[i].p_પદાવલિ) ){
          testcase( i==31 );
          constMask |= MASKBIT32(i);
        }
        જો( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, pFarg->a[i].p_પદાવલિ);
        }
      }
      જો( pFarg ){
        જો( constMask ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }નહીંતર{
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        જો( (pDef->funcFlags & (SQLITE_FUNC_LENGTH|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 પદાવલિ_Op;
          પ્રતિજ્ઞા( nFarg==1 );
          પ્રતિજ્ઞા( pFarg->a[0].p_પદાવલિ!=0 );
          પદાવલિ_Op = pFarg->a[0].p_પદાવલિ->op;
          જો( પદાવલિ_Op==TK_COLUMN || પદાવલિ_Op==TK_AGG_COLUMN ){
            પ્રતિજ્ઞા( SQLITE_FUNC_LENGTH==OPFLAG_LENGTHARG );
            પ્રતિજ્ઞા( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            પ્રતિજ્ઞા( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            પ્રતિજ્ઞા( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_પદાવલિ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_FACTOR);
      }નહીંતર{
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
      જો( nFarg>=2 && પદાવલિ_Has_Property(p_પદાવલિ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_પદાવલિ);
      }નહીંતર જો( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_પદાવલિ);
      }
#endif
      જો( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        જો( !pColl ) pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (અક્ષર *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg,
                                 pDef, p_પદાવલિ->op2);
      જો( nFarg ){
        જો( constMask==0 ){
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        }નહીંતર{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, constMask, 1);
        }
      }
      પાછા target;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_EXISTS:
    સ્થિતિ TK_SELECT: {
      પૂર્ણાંક nCol;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      જો( pParse->db->mallocFailed ){
        પાછા 0;
      }નહીંતર જો( op==TK_SELECT
             && ALWAYS( પદાવલિ_Use_XSelect(p_પદાવલિ) )
             && (nCol = p_પદાવલિ->x.pSelect->pEList->n_પદાવલિ)!=1
      ){
        sqlite3SubselectError(pParse, nCol, 1);
      }નહીંતર{
        પાછા sqlite3CodeSubselect(pParse, p_પદાવલિ);
      }
      તોડો;
    }
    સ્થિતિ TK_SELECT_COLUMN: {
      પૂર્ણાંક n;
      પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
      જો( p_ડાબું->iTable==0 || pParse->withinRJSubrtn > p_ડાબું->op2 ){
        p_ડાબું->iTable = sqlite3CodeSubselect(pParse, p_ડાબું);
        p_ડાબું->op2 = pParse->withinRJSubrtn;
      }
      પ્રતિજ્ઞા( p_ડાબું->op==TK_SELECT || p_ડાબું->op==TK_ERROR );
      n = sqlite3_પદાવલિ_Vector_Size(p_ડાબું);
      જો( p_પદાવલિ->iTable!=n ){
        sqlite3_Error_સંદેશ(pParse, "%d columns assigned %d values",
                                p_પદાવલિ->iTable, n);
      }
      પાછા p_ડાબું->iTable + p_પદાવલિ->iColumn;
    }
    સ્થિતિ TK_IN: {
      પૂર્ણાંક destIfFalse = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3_પદાવલિ_Code_IN(pParse, p_પદાવલિ, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      પાછા target;
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
    સ્થિતિ TK_BETWEEN: {
      પદાવલિ_Code_Between(pParse, p_પદાવલિ, target, 0, 0);
      પાછા target;
    }
    સ્થિતિ TK_COLLATE: {
      જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું );
        sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ->p_ડાબું, target);
        sqlite3VdbeAddOp1(v, OP_ClrSubtype, target);
        પાછા target;
      }નહીંતર{
        p_પદાવલિ = p_પદાવલિ->p_ડાબું;
        જાઓ પદાવલિ_code_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    સ્થિતિ TK_SPAN:
    સ્થિતિ TK_UPLUS: {
      p_પદાવલિ = p_પદાવલિ->p_ડાબું;
      જાઓ પદાવલિ_code_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    સ્થિતિ TK_TRIGGER: {
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
      પૂર્ણાંક iCol;
      પૂર્ણાંક p1;

      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
      pTab = p_પદાવલિ->y.pTab;
      iCol = p_પદાવલિ->iColumn;
      p1 = p_પદાવલિ->iTable * (pTab->nCol+1) + 1
                     + sqlite3TableColumnToStorage(pTab, iCol);

      પ્રતિજ્ઞા( p_પદાવલિ->iTable==0 || p_પદાવલિ->iTable==1 );
      પ્રતિજ્ઞા( iCol>=-1 && iCol<pTab->nCol );
      પ્રતિજ્ઞા( pTab->iPKey<0 || iCol!=pTab->iPKey );
      પ્રતિજ્ઞા( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "r[%d]=%s.%s", target,
        (p_પદાવલિ->iTable ? "new" : "old"),
        (p_પદાવલિ->iColumn<0 ? "rowid" : p_પદાવલિ->y.pTab->aCol[iCol].zCnName)
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      જો( iCol>=0 && pTab->aCol[iCol].affinity==SQLITE_AFF_REAL ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      તોડો;
    }

    સ્થિતિ TK_VECTOR: {
      sqlite3_Error_સંદેશ(pParse, "row value misused");
      તોડો;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    સ્થિતિ TK_IF_NULL_ROW: {
      પૂર્ણાંક addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = p_પદાવલિ->pAggInfo;
      જો( pAggInfo ){
        પ્રતિજ્ઞા( p_પદાવલિ->iAgg>=0 && p_પદાવલિ->iAgg<pAggInfo->nColumn );
        જો( !pAggInfo->directMode ){
          inReg = AggInfoColumnReg(pAggInfo, p_પદાવલિ->iAgg);
          તોડો;
        }
        જો( p_પદાવલિ->pAggInfo->useSortingIdx ){
          sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdxPTab,
                            pAggInfo->aCol[p_પદાવલિ->iAgg].iSorterColumn,
                            target);
          inReg = target;
          તોડો;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, OP_IfNullRow, p_પદાવલિ->iTable, 0, target);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->okConstFactor = 0;   /* note (1) above */
      sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ->p_ડાબું, target);
      પ્રતિજ્ઞા( target==inReg );
      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      તોડો;
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
    સ્થિતિ TK_CASE: {
      પૂર્ણાંક endLabel;                     /* GOTO label for end of CASE stmt */
      પૂર્ણાંક nextCase;                     /* GOTO label for next WHEN clause */
      પૂર્ણાંક n_પદાવલિ;                        /* 2x number of WHEN terms */
      પૂર્ણાંક i;                            /* Loop counter */
      પદાવલિ_List *pEList;                 /* List of WHEN terms */
      સંરચના પદાવલિ_List_item *aListelem;  /* Array of WHEN terms */
      પદાવલિ opCompare;                   /* The X==Ei expression */
      પદાવલિ *pX;                         /* The X expression */
      પદાવલિ *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      પદાવલિ *pDel = 0;
      sqlite3 *db = pParse->db;

      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) && p_પદાવલિ->x.pList!=0 );
      પ્રતિજ્ઞા(p_પદાવલિ->x.pList->n_પદાવલિ > 0);
      pEList = p_પદાવલિ->x.pList;
      aListelem = pEList->a;
      n_પદાવલિ = pEList->n_પદાવલિ;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      જો( (pX = p_પદાવલિ->p_ડાબું)!=0 ){
        pDel = sqlite3_પદાવલિ_Dup(db, pX, 0);
        જો( db->mallocFailed ){
          sqlite3_પદાવલિ_Delete(db, pDel);
          તોડો;
        }
        testcase( pX->op==TK_COLUMN );
        sqlite3_પદાવલિ_To_Register(pDel, પદાવલિ_Code_Vector(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, કદ(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ડાબું = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      માટે(i=0; i<n_પદાવલિ-1; i=i+2){
        જો( pX ){
          પ્રતિજ્ઞા( pTest!=0 );
          opCompare.p_જમણું = aListelem[i].p_પદાવલિ;
        }નહીંતર{
          pTest = aListelem[i].p_પદાવલિ;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_COLUMN );
        sqlite3_પદાવલિ_If_False(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_પદાવલિ->op==TK_COLUMN );
        sqlite3_પદાવલિ_Code(pParse, aListelem[i+1].p_પદાવલિ, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      જો( (n_પદાવલિ&1)!=0 ){
        sqlite3_પદાવલિ_Code(pParse, pEList->a[n_પદાવલિ-1].p_પદાવલિ, target);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      sqlite3_પદાવલિ_Delete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      તોડો;
    }
#ifndef SQLITE_OMIT_TRIGGER
    સ્થિતિ TK_RAISE: {
      પ્રતિજ્ઞા( p_પદાવલિ->aff_પદાવલિ==OE_Rollback
           || p_પદાવલિ->aff_પદાવલિ==OE_Abort
           || p_પદાવલિ->aff_પદાવલિ==OE_Fail
           || p_પદાવલિ->aff_પદાવલિ==OE_Ignore
      );
      જો( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_સંદેશ(pParse,
                       "RAISE() may only be used within a trigger-program");
        પાછા 0;
      }
      જો( p_પદાવલિ->aff_પદાવલિ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      જો( p_પદાવલિ->aff_પદાવલિ==OE_Ignore ){
        sqlite3VdbeAddOp2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
        sqlite3VdbeAddOp3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_પદાવલિ->aff_પદાવલિ, r1);
      }
      તોડો;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  પાછા inReg;
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
પૂર્ણાંક sqlite3_પદાવલિ_Code_Run_Just_Once(
  Parse *pParse,    /* Parsing context */
  પદાવલિ *p_પદાવલિ,      /* The expression to code when the VDBE initializes */
  પૂર્ણાંક regDest       /* Store the value in this register */
){
  પદાવલિ_List *p;
  પ્રતિજ્ઞા( ConstFactorOk(pParse) );
  પ્રતિજ્ઞા( regDest!=0 );
  p = pParse->p_Const_પદાવલિ;
  જો( regDest<0 && p ){
    સંરચના પદાવલિ_List_item *pItem;
    પૂર્ણાંક i;
    માટે(pItem=p->a, i=p->n_પદાવલિ; i>0; pItem++, i--){
      જો( pItem->fg.reusable
       && sqlite3_પદાવલિ_Compare(0,pItem->p_પદાવલિ,p_પદાવલિ,-1)==0
      ){
        પાછા pItem->u.i_Const_પદાવલિ_Reg;
      }
    }
  }
  p_પદાવલિ = sqlite3_પદાવલિ_Dup(pParse->db, p_પદાવલિ, 0);
  જો( p_પદાવલિ!=0 && પદાવલિ_Has_Property(p_પદાવલિ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    પૂર્ણાંક addr;
    પ્રતિજ્ઞા( v );
    addr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    pParse->okConstFactor = 0;
    જો( !pParse->db->mallocFailed ){
      જો( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3_પદાવલિ_Delete(pParse->db, p_પદાવલિ);
    sqlite3VdbeJumpHere(v, addr);
  }નહીંતર{
    p = sqlite3_પદાવલિ_List_Append(pParse, p, p_પદાવલિ);
    જો( p ){
       સંરચના પદાવલિ_List_item *pItem = &p->a[p->n_પદાવલિ-1];
       pItem->fg.reusable = regDest<0;
       જો( regDest<0 ) regDest = ++pParse->nMem;
       pItem->u.i_Const_પદાવલિ_Reg = regDest;
    }
    pParse->p_Const_પદાવલિ = p;
  }
  પાછા regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE શૂન્ય sqlite3_પદાવલિ_Null_Register_Range(
  Parse *pParse,   /* Parsing context */
  પૂર્ણાંક iReg,        /* First register to set to NULL */
  પૂર્ણાંક nReg         /* Number of sequential registers to NULL out */
){
  u8 okConstFactor = pParse->okConstFactor;
  પદાવલિ t;
  memset(&t, 0, કદ(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3_પદાવલિ_Code_Run_Just_Once(pParse, &t, iReg);
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
પૂર્ણાંક sqlite3_પદાવલિ_Code_Temp(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક *pReg){
  પૂર્ણાંક r2;
  p_પદાવલિ = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p_પદાવલિ);
  જો( ConstFactorOk(pParse)
   && ALWAYS(p_પદાવલિ!=0)
   && p_પદાવલિ->op!=TK_REGISTER
   && sqlite3_પદાવલિ_Is_Constant_Not_Join(pParse, p_પદાવલિ)
  ){
    *pReg  = 0;
    r2 = sqlite3_પદાવલિ_Code_Run_Just_Once(pParse, p_પદાવલિ, -1);
  }નહીંતર{
    પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ, r1);
    જો( r2==r1 ){
      *pReg = r1;
    }નહીંતર{
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  પાછા r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
શૂન્ય sqlite3_પદાવલિ_Code(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક target){
  પૂર્ણાંક inReg;

  પ્રતિજ્ઞા( p_પદાવલિ==0 || !પદાવલિ_Has_VVAProperty(p_પદાવલિ,EP_Immutable) );
  પ્રતિજ્ઞા( target>0 && target<=pParse->nMem );
  પ્રતિજ્ઞા( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  જો( pParse->pVdbe==0 ) પાછા;
  inReg = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ, target);
  જો( inReg!=target ){
    u8 op;
    પદાવલિ *pX = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p_પદાવલિ);
    testcase( pX!=p_પદાવલિ );
    જો( ALWAYS(pX)
     && (પદાવલિ_Has_Property(pX,EP_Subquery) || pX->op==TK_REGISTER)
    ){
      op = OP_Copy;
    }નહીંતર{
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
શૂન્ય sqlite3_પદાવલિ_Code_Copy(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક target){
  sqlite3 *db = pParse->db;
  p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
  જો( !db->mallocFailed ) sqlite3_પદાવલિ_Code(pParse, p_પદાવલિ, target);
  sqlite3_પદાવલિ_Delete(db, p_પદાવલિ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
શૂન્ય sqlite3_પદાવલિ_Code_Factorable(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક target){
  જો( pParse->okConstFactor && sqlite3_પદાવલિ_Is_Constant_Not_Join(pParse,p_પદાવલિ) ){
    sqlite3_પદાવલિ_Code_Run_Just_Once(pParse, p_પદાવલિ, target);
  }નહીંતર{
    sqlite3_પદાવલિ_Code_Copy(pParse, p_પદાવલિ, target);
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
પૂર્ણાંક sqlite3_પદાવલિ_Code_પદાવલિ_List(
  Parse *pParse,     /* Parsing context */
  પદાવલિ_List *pList,   /* The expression list to be coded */
  પૂર્ણાંક target,        /* Where to write results */
  પૂર્ણાંક srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  સંરચના પદાવલિ_List_item *pItem;
  પૂર્ણાંક i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  પ્રતિજ્ઞા( pList!=0 );
  પ્રતિજ્ઞા( target>0 );
  પ્રતિજ્ઞા( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_પદાવલિ;
  જો( !ConstFactorOk(pParse) ) flags &= ~SQLITE_ECEL_FACTOR;
  માટે(pItem=pList->a, i=0; i<n; i++, pItem++){
    પદાવલિ *p_પદાવલિ = pItem->p_પદાવલિ;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    જો( pItem->fg.bSorterRef ){
      i--;
      n--;
    }નહીંતર
#endif
    જો( (flags & SQLITE_ECEL_REF)!=0 && (j = pItem->u.x.iOrderByCol)>0 ){
      જો( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }નહીંતર{
        sqlite3VdbeAddOp2(v, copyOp, j+srcReg-1, target+i);
      }
    }નહીંતર જો( (flags & SQLITE_ECEL_FACTOR)!=0
           && sqlite3_પદાવલિ_Is_Constant_Not_Join(pParse,p_પદાવલિ)
    ){
      sqlite3_પદાવલિ_Code_Run_Just_Once(pParse, p_પદાવલિ, target+i);
    }નહીંતર{
      પૂર્ણાંક inReg = sqlite3_પદાવલિ_Code_Target(pParse, p_પદાવલિ, target+i);
      જો( inReg!=target+i ){
        VdbeOp *pOp;
        જો( copyOp==OP_Copy
         && (pOp=sqlite3VdbeGetLastOp(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==inReg
         && pOp->p2+pOp->p3+1==target+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }નહીંતર{
          sqlite3VdbeAddOp2(v, copyOp, inReg, target+i);
        }
      }
    }
  }
  પાછા n;
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
સ્થિર શૂન્ય પદાવલિ_Code_Between(
  Parse *pParse,    /* Parsing and code generating context */
  પદાવલિ *p_પદાવલિ,      /* The BETWEEN expression */
  પૂર્ણાંક dest,         /* Jump destination or storage location */
  શૂન્ય (*xJump)(Parse*,પદાવલિ*,પૂર્ણાંક,પૂર્ણાંક), /* Action to take */
  પૂર્ણાંક jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  પદાવલિ પદાવલિ_And;     /* The AND operator in  x>=y AND x<=z  */
  પદાવલિ comp_ડાબું;    /* The  x>=y  term */
  પદાવલિ comp_જમણું;   /* The  x<=z  term */
  પૂર્ણાંક regFree1 = 0; /* Temporary use register */
  પદાવલિ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ડાબું, 0, કદ(પદાવલિ));
  memset(&comp_જમણું, 0, કદ(પદાવલિ));
  memset(&પદાવલિ_And, 0, કદ(પદાવલિ));

  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  pDel = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ->p_ડાબું, 0);
  જો( db->mallocFailed==0 ){
    પદાવલિ_And.op = TK_AND;
    પદાવલિ_And.p_ડાબું = &comp_ડાબું;
    પદાવલિ_And.p_જમણું = &comp_જમણું;
    comp_ડાબું.op = TK_GE;
    comp_ડાબું.p_ડાબું = pDel;
    comp_ડાબું.p_જમણું = p_પદાવલિ->x.pList->a[0].p_પદાવલિ;
    comp_જમણું.op = TK_LE;
    comp_જમણું.p_ડાબું = pDel;
    comp_જમણું.p_જમણું = p_પદાવલિ->x.pList->a[1].p_પદાવલિ;
    sqlite3_પદાવલિ_To_Register(pDel, પદાવલિ_Code_Vector(pParse, pDel, &regFree1));
    જો( xJump ){
      xJump(pParse, &પદાવલિ_And, dest, jumpIfNull);
    }નહીંતર{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_પદાવલિ_Code_Target(pParse, &પદાવલિ_And, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3_પદાવલિ_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_પદાવલિ_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_પદાવલિ_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_પદાવલિ_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_પદાવલિ_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_પદાવલિ_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_પદાવલિ_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_પદાવલિ_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_પદાવલિ_If_False && jumpIfNull!=0 && regFree1!=0 );
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
શૂન્ય sqlite3_પદાવલિ_If_True(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક dest, પૂર્ણાંક jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક op = 0;
  પૂર્ણાંક regFree1 = 0;
  પૂર્ણાંક regFree2 = 0;
  પૂર્ણાંક r1, r2;

  પ્રતિજ્ઞા( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  જો( NEVER(v==0) )     પાછા;  /* Existence of VDBE checked by caller */
  જો( NEVER(p_પદાવલિ==0) ) પાછા;  /* No way this can happen */
  પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p_પદાવલિ, EP_Immutable) );
  op = p_પદાવલિ->op;
  પસંદગી( op ){
    સ્થિતિ TK_AND:
    સ્થિતિ TK_OR: {
      પદાવલિ *pAlt = sqlite3_પદાવલિ_Simplified_And_Or(p_પદાવલિ);
      જો( pAlt!=p_પદાવલિ ){
        sqlite3_પદાવલિ_If_True(pParse, pAlt, dest, jumpIfNull);
      }નહીંતર{
        પદાવલિ *pFirst, *pSecond;
        જો( પદાવલિ_Eval_Rhs_First(p_પદાવલિ) ){
          pFirst = p_પદાવલિ->p_જમણું;
          pSecond = p_પદાવલિ->p_ડાબું;
        }નહીંતર{
          pFirst = p_પદાવલિ->p_ડાબું;
          pSecond = p_પદાવલિ->p_જમણું;
        }
        જો( op==TK_AND ){
          પૂર્ણાંક d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_પદાવલિ_If_False(pParse, pFirst, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_પદાવલિ_If_True(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }નહીંતર{
          testcase( jumpIfNull==0 );
          sqlite3_પદાવલિ_If_True(pParse, pFirst, dest, jumpIfNull);
          sqlite3_પદાવલિ_If_True(pParse, pSecond, dest, jumpIfNull);
        }
      }
      તોડો;
    }
    સ્થિતિ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_પદાવલિ_If_False(pParse, p_પદાવલિ->p_ડાબું, dest, jumpIfNull);
      તોડો;
    }
    સ્થિતિ TK_TRUTH: {
      પૂર્ણાંક isNot;      /* IS NOT TRUE or IS NOT FALSE */
      પૂર્ણાંક isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_પદાવલિ->op2==TK_ISNOT;
      isTrue = sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ->p_જમણું);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      જો( isTrue ^ isNot ){
        sqlite3_પદાવલિ_If_True(pParse, p_પદાવલિ->p_ડાબું, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }નહીંતર{
        sqlite3_પદાવલિ_If_False(pParse, p_પદાવલિ->p_ડાબું, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      તોડો;
    }
    સ્થિતિ TK_IS:
    સ્થિતિ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_EQ: {
      પૂર્ણાંક addrIsNull;
      જો( sqlite3_પદાવલિ_Is_Vector(p_પદાવલિ->p_ડાબું) ) જાઓ default_પદાવલિ;
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = પદાવલિ_Compute_Operands(pParse, p_પદાવલિ,
                                   &r1, &r2, &regFree1, &regFree2);
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
        r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_પદાવલિ->p_ડાબું, p_પદાવલિ->p_જમણું, op,
                  r1, r2, dest, jumpIfNull, પદાવલિ_Has_Property(p_પદાવલિ,EP_Commuted));
      પ્રતિજ્ઞા(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      પ્રતિજ્ઞા(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      પ્રતિજ્ઞા(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      પ્રતિજ્ઞા(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      પ્રતિજ્ઞા(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      પ્રતિજ્ઞા(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      જો( addrIsNull ){
        જો( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }નહીંતર{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      તોડો;
    }
    સ્થિતિ TK_ISNULL:
    સ્થિતિ TK_NOTNULL: {
      પ્રતિજ્ઞા( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      પ્રતિજ્ઞા( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
      પ્રતિજ્ઞા( regFree1==0 || regFree1==r1 );
      જો( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      તોડો;
    }
    સ્થિતિ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      પદાવલિ_Code_Between(pParse, p_પદાવલિ, dest, sqlite3_પદાવલિ_If_True, jumpIfNull);
      તોડો;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_IN: {
      પૂર્ણાંક destIfFalse = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_પદાવલિ_Code_IN(pParse, p_પદાવલિ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      તોડો;
    }
#endif
    મૂળભૂત: {
    default_પદાવલિ:
      જો( પદાવલિ_Always_True(p_પદાવલિ) ){
        sqlite3VdbeGoto(v, dest);
      }નહીંતર જો( પદાવલિ_Always_False(p_પદાવલિ) ){
        /* No-op */
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      તોડો;
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
શૂન્ય sqlite3_પદાવલિ_If_False(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક dest, પૂર્ણાંક jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક op = 0;
  પૂર્ણાંક regFree1 = 0;
  પૂર્ણાંક regFree2 = 0;
  પૂર્ણાંક r1, r2;

  પ્રતિજ્ઞા( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  જો( NEVER(v==0) ) પાછા; /* Existence of VDBE checked by caller */
  જો( p_પદાવલિ==0 )    પાછા;
  પ્રતિજ્ઞા( !પદાવલિ_Has_VVAProperty(p_પદાવલિ,EP_Immutable) );

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
  op = ((p_પદાવલિ->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_ISNULL || op==OP_NotNull );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_NOTNULL || op==OP_IsNull );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_NE || op==OP_Eq );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_EQ || op==OP_Ne );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_LT || op==OP_Ge );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_LE || op==OP_Gt );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_GT || op==OP_Le );
  પ્રતિજ્ઞા( p_પદાવલિ->op!=TK_GE || op==OP_Lt );

  પસંદગી( p_પદાવલિ->op ){
    સ્થિતિ TK_AND:
    સ્થિતિ TK_OR: {
      પદાવલિ *pAlt = sqlite3_પદાવલિ_Simplified_And_Or(p_પદાવલિ);
      જો( pAlt!=p_પદાવલિ ){
        sqlite3_પદાવલિ_If_False(pParse, pAlt, dest, jumpIfNull);
      }નહીંતર{
        પદાવલિ *pFirst, *pSecond;
        જો( પદાવલિ_Eval_Rhs_First(p_પદાવલિ) ){
          pFirst = p_પદાવલિ->p_જમણું;
          pSecond = p_પદાવલિ->p_ડાબું;
        }નહીંતર{
          pFirst = p_પદાવલિ->p_ડાબું;
          pSecond = p_પદાવલિ->p_જમણું;
        }
        જો( p_પદાવલિ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_પદાવલિ_If_False(pParse, pFirst, dest, jumpIfNull);
          sqlite3_પદાવલિ_If_False(pParse, pSecond, dest, jumpIfNull);
        }નહીંતર{
          પૂર્ણાંક d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_પદાવલિ_If_True(pParse, pFirst, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_પદાવલિ_If_False(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      તોડો;
    }
    સ્થિતિ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_પદાવલિ_If_True(pParse, p_પદાવલિ->p_ડાબું, dest, jumpIfNull);
      તોડો;
    }
    સ્થિતિ TK_TRUTH: {
      પૂર્ણાંક isNot;   /* IS NOT TRUE or IS NOT FALSE */
      પૂર્ણાંક isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_પદાવલિ->op2==TK_ISNOT;
      isTrue = sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ->p_જમણું);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      જો( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_પદાવલિ_If_False(pParse, p_પદાવલિ->p_ડાબું, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }નહીંતર{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_પદાવલિ_If_True(pParse, p_પદાવલિ->p_ડાબું, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      તોડો;
    }
    સ્થિતિ TK_IS:
    સ્થિતિ TK_ISNOT:
      testcase( p_પદાવલિ->op==TK_IS );
      testcase( p_પદાવલિ->op==TK_ISNOT );
      op = (p_પદાવલિ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_EQ: {
      પૂર્ણાંક addrIsNull;
      જો( sqlite3_પદાવલિ_Is_Vector(p_પદાવલિ->p_ડાબું) ) જાઓ default_પદાવલિ;
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = પદાવલિ_Compute_Operands(pParse, p_પદાવલિ,
                                   &r1, &r2, &regFree1, &regFree2);
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
        r2 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_જમણું, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, p_પદાવલિ->p_ડાબું, p_પદાવલિ->p_જમણું, op,
                  r1, r2, dest, jumpIfNull,પદાવલિ_Has_Property(p_પદાવલિ,EP_Commuted));
      પ્રતિજ્ઞા(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      પ્રતિજ્ઞા(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      પ્રતિજ્ઞા(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      પ્રતિજ્ઞા(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      પ્રતિજ્ઞા(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      પ્રતિજ્ઞા(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      જો( addrIsNull ){
        જો( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }નહીંતર{
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      તોડો;
    }
    સ્થિતિ TK_ISNULL:
    સ્થિતિ TK_NOTNULL: {
      r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ->p_ડાબું, &regFree1);
      પ્રતિજ્ઞા( regFree1==0 || regFree1==r1 );
      જો( regFree1 ) sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      તોડો;
    }
    સ્થિતિ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      પદાવલિ_Code_Between(pParse, p_પદાવલિ, dest, sqlite3_પદાવલિ_If_False, jumpIfNull);
      તોડો;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_IN: {
      જો( jumpIfNull ){
        sqlite3_પદાવલિ_Code_IN(pParse, p_પદાવલિ, dest, dest);
      }નહીંતર{
        પૂર્ણાંક destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_પદાવલિ_Code_IN(pParse, p_પદાવલિ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      તોડો;
    }
#endif
    મૂળભૂત: {
    default_પદાવલિ:
      જો( પદાવલિ_Always_False(p_પદાવલિ) ){
        sqlite3VdbeGoto(v, dest);
      }નહીંતર જો( પદાવલિ_Always_True(p_પદાવલિ) ){
        /* no-op */
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, p_પદાવલિ, &regFree1);
        sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      તોડો;
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
શૂન્ય sqlite3_પદાવલિ_If_False_Dup(Parse *pParse, પદાવલિ *p_પદાવલિ, પૂર્ણાંક dest,પૂર્ણાંક jumpIfNull){
  sqlite3 *db = pParse->db;
  પદાવલિ *pCopy = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
  જો( db->mallocFailed==0 ){
    sqlite3_પદાવલિ_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_પદાવલિ_Delete(db, pCopy);
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
સ્થિર SQLITE_NOINLINE પૂર્ણાંક પદાવલિ_Compare_Variable(
  અચલ Parse *pParse,
  અચલ પદાવલિ *pVar,
  અચલ પદાવલિ *p_પદાવલિ
){
  પૂર્ણાંક res = 2;
  પૂર્ણાંક iVar;
  sqlite3_કિંમત *pL, *pR = 0;
 
  જો( p_પદાવલિ->op==TK_VARIABLE && pVar->iColumn==p_પદાવલિ->iColumn ){
    પાછા 0;
  }
  જો( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) પાછા 2;
  sqlite3_કિંમત_From_પદાવલિ(pParse->db, p_પદાવલિ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  જો( pR ){
    iVar = pVar->iColumn;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_કિંમત(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    જો( pL ){
      જો( sqlite3_કિંમત_type(pL)==SQLITE_TEXT ){
        sqlite3_કિંમત_text(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_કિંમત_Free(pR);
    sqlite3_કિંમત_Free(pL);
  }
  પાછા res;
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
પૂર્ણાંક sqlite3_પદાવલિ_Compare(
  અચલ Parse *pParse,
  અચલ પદાવલિ *pA,
  અચલ પદાવલિ *pB,
  પૂર્ણાંક iTab
){
  u32 combinedFlags;
  જો( pA==0 || pB==0 ){
    પાછા pB==pA ? 0 : 2;
  }
  જો( pParse && pA->op==TK_VARIABLE ){
    પાછા પદાવલિ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  જો( combinedFlags & EP_Int_કિંમત ){
    જો( (pA->flags&pB->flags&EP_Int_કિંમત)!=0 && pA->u.i_કિંમત==pB->u.i_કિંમત ){
      પાછા 0;
    }
    પાછા 2;
  }
  જો( pA->op!=pB->op || pA->op==TK_RAISE ){
    જો( pA->op==TK_COLLATE && sqlite3_પદાવલિ_Compare(pParse, pA->p_ડાબું,pB,iTab)<2 ){
      પાછા 1;
    }
    જો( pB->op==TK_COLLATE && sqlite3_પદાવલિ_Compare(pParse, pA,pB->p_ડાબું,iTab)<2 ){
      પાછા 1;
    }
    જો( pA->op==TK_AGG_COLUMN && pB->op==TK_COLUMN
     && pB->iTable<0 && pA->iTable==iTab
    ){
      /* fall through */
    }નહીંતર{
      પાછા 2;
    }
  }
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pA, EP_Int_કિંમત) );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pB, EP_Int_કિંમત) );
  જો( pA->u.zToken ){
    જો( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      જો( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) પાછા 2;
#ifndef SQLITE_OMIT_WINDOWFUNC
      પ્રતિજ્ઞા( pA->op==pB->op );
      જો( પદાવલિ_Has_Property(pA,EP_WinFunc)!=પદાવલિ_Has_Property(pB,EP_WinFunc) ){
        પાછા 2;
      }
      જો( પદાવલિ_Has_Property(pA,EP_WinFunc) ){
        જો( sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          પાછા 2;
        }
      }
#endif
    }નહીંતર જો( pA->op==TK_NULL ){
      પાછા 0;
    }નહીંતર જો( pA->op==TK_COLLATE ){
      જો( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) પાછા 2;
    }નહીંતર
    જો( pB->u.zToken!=0
     && pA->op!=TK_COLUMN
     && pA->op!=TK_AGG_COLUMN
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      પાછા 2;
    }
  }
  જો( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) પાછા 2;
  જો( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    જો( combinedFlags & EP_xIsSelect ) પાછા 2;
    જો( (combinedFlags & EP_FixedCol)==0
     && sqlite3_પદાવલિ_Compare(pParse, pA->p_ડાબું, pB->p_ડાબું, iTab) ) પાછા 2;
    જો( sqlite3_પદાવલિ_Compare(pParse, pA->p_જમણું, pB->p_જમણું, iTab) ) પાછા 2;
    જો( sqlite3_પદાવલિ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) પાછા 2;
    જો( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      જો( pA->iColumn!=pB->iColumn ) પાછા 2;
      જો( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) પાછા 2;
      જો( pA->op!=TK_IN && pA->iTable!=pB->iTable && pA->iTable!=iTab ){
        પાછા 2;
      }
    }
  }
  પાછા 0;
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
પૂર્ણાંક sqlite3_પદાવલિ_List_Compare(અચલ પદાવલિ_List *pA, અચલ પદાવલિ_List *pB, પૂર્ણાંક iTab){
  પૂર્ણાંક i;
  જો( pA==0 && pB==0 ) પાછા 0;
  જો( pA==0 || pB==0 ) પાછા 1;
  જો( pA->n_પદાવલિ!=pB->n_પદાવલિ ) પાછા 1;
  માટે(i=0; i<pA->n_પદાવલિ; i++){
    પૂર્ણાંક res;
    પદાવલિ *p_પદાવલિ_A = pA->a[i].p_પદાવલિ;
    પદાવલિ *p_પદાવલિ_B = pB->a[i].p_પદાવલિ;
    જો( pA->a[i].fg.sortFlags!=pB->a[i].fg.sortFlags ) પાછા 1;
    જો( (res = sqlite3_પદાવલિ_Compare(0, p_પદાવલિ_A, p_પદાવલિ_B, iTab)) ) પાછા res;
  }
  પાછા 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
પૂર્ણાંક sqlite3_પદાવલિ_Compare_Skip(પદાવલિ *pA,પદાવલિ *pB, પૂર્ણાંક iTab){
  પાછા sqlite3_પદાવલિ_Compare(0,
             sqlite3_પદાવલિ_Skip_Collate(pA),
             sqlite3_પદાવલિ_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
સ્થિર પૂર્ણાંક પદાવલિ_Implies_Not_Null(
  અચલ Parse *pParse,/* Parsing context */
  અચલ પદાવલિ *p,      /* The expression to be checked */
  અચલ પદાવલિ *pNN,    /* The expression that is NOT NULL */
  પૂર્ણાંક iTab,           /* Table being evaluated */
  પૂર્ણાંક seenNot         /* Return true only if p can be any non-NULL value */
){
  પ્રતિજ્ઞા( p );
  પ્રતિજ્ઞા( pNN );
  જો( sqlite3_પદાવલિ_Compare(pParse, p, pNN, iTab)==0 ){
    પાછા pNN->op!=TK_NULL;
  }
  પસંદગી( p->op ){
    સ્થિતિ TK_IN: {
      જો( seenNot && પદાવલિ_Has_Property(p, EP_xIsSelect) ) પાછા 0;
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_પદાવલિ>0) );
      પાછા પદાવલિ_Implies_Not_Null(pParse, p->p_ડાબું, pNN, iTab, 1);
    }
    સ્થિતિ TK_BETWEEN: {
      પદાવલિ_List *pList;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p) );
      pList = p->x.pList;
      પ્રતિજ્ઞા( pList!=0 );
      પ્રતિજ્ઞા( pList->n_પદાવલિ==2 );
      જો( seenNot ) પાછા 0;
      જો( પદાવલિ_Implies_Not_Null(pParse, pList->a[0].p_પદાવલિ, pNN, iTab, 1)
       || પદાવલિ_Implies_Not_Null(pParse, pList->a[1].p_પદાવલિ, pNN, iTab, 1)
      ){
        પાછા 1;
      }
      પાછા પદાવલિ_Implies_Not_Null(pParse, p->p_ડાબું, pNN, iTab, 1);
    }
    સ્થિતિ TK_EQ:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE:
    સ્થિતિ TK_PLUS:
    સ્થિતિ TK_MINUS:
    સ્થિતિ TK_BITOR:
    સ્થિતિ TK_LSHIFT:
    સ્થિતિ TK_RSHIFT:
    સ્થિતિ TK_CONCAT:
      seenNot = 1;
      /* no break */ deliberate_fall_through
    સ્થિતિ TK_STAR:
    સ્થિતિ TK_REM:
    સ્થિતિ TK_BITAND:
    સ્થિતિ TK_SLASH: {
      જો( પદાવલિ_Implies_Not_Null(pParse, p->p_જમણું, pNN, iTab, seenNot) ) પાછા 1;
      /* no break */ deliberate_fall_through
    }
    સ્થિતિ TK_SPAN:
    સ્થિતિ TK_COLLATE:
    સ્થિતિ TK_UPLUS:
    સ્થિતિ TK_UMINUS: {
      પાછા પદાવલિ_Implies_Not_Null(pParse, p->p_ડાબું, pNN, iTab, seenNot);
    }
    સ્થિતિ TK_TRUTH: {
      જો( seenNot ) પાછા 0;
      જો( p->op2!=TK_IS ) પાછા 0;
      પાછા પદાવલિ_Implies_Not_Null(pParse, p->p_ડાબું, pNN, iTab, 1);
    }
    સ્થિતિ TK_BITNOT:
    સ્થિતિ TK_NOT: {
      પાછા પદાવલિ_Implies_Not_Null(pParse, p->p_ડાબું, pNN, iTab, 1);
    }
  }
  પાછા 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
સ્થિર પૂર્ણાંક sqlite3_પદાવલિ_Is_Not_True(પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક v;
  જો( p_પદાવલિ->op==TK_NULL ) પાછા 1;
  જો( p_પદાવલિ->op==TK_TRUEFALSE && sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ)==0 ) પાછા 1;
  v = 1;
  જો( sqlite3_પદાવલિ_Is_Integer(p_પદાવલિ, &v, 0, 0) && v==0 ) પાછા 1;
  પાછા 0;
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
સ્થિર પૂર્ણાંક sqlite3_પદાવલિ_Is_IIF(sqlite3 *db, અચલ પદાવલિ *p_પદાવલિ){
  પદાવલિ_List *pList;
  જો( p_પદાવલિ->op==TK_FUNCTION ){
    અચલ અક્ષર *z = p_પદાવલિ->u.zToken;
    FuncDef *pDef;
    જો( (z[0]!='i' && z[0]!='I') ) પાછા 0;
    જો( p_પદાવલિ->x.pList==0 ) પાછા 0;
    pDef = sqlite3FindFunction(db, z, p_પદાવલિ->x.pList->n_પદાવલિ, ENC(db), 0);
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    જો( pDef==0 ) પાછા 0;
#નહીંતર
    જો( NEVER(pDef==0) ) પાછા 0;
#endif
    જો( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) પાછા 0;
    જો( SQLITE_PTR_TO_INT(pDef->pUserData)!=INLINEFUNC_iif ) પાછા 0;
  }નહીંતર જો( p_પદાવલિ->op==TK_CASE ){
    જો( p_પદાવલિ->p_ડાબું!=0 ) પાછા 0;
  }નહીંતર{
    પાછા 0;
  }
  pList = p_પદાવલિ->x.pList;
  પ્રતિજ્ઞા( pList!=0 );
  જો( pList->n_પદાવલિ==2 ) પાછા 1;
  જો( pList->n_પદાવલિ==3 && sqlite3_પદાવલિ_Is_Not_True(pList->a[2].p_પદાવલિ) ) પાછા 1;
  પાછા 0;
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
પૂર્ણાંક sqlite3_પદાવલિ_Implies_પદાવલિ(
  અચલ Parse *pParse,
  અચલ પદાવલિ *pE1,
  અચલ પદાવલિ *pE2,
  પૂર્ણાંક iTab
){
  જો( sqlite3_પદાવલિ_Compare(pParse, pE1, pE2, iTab)==0 ){
    પાછા 1;
  }
  જો( pE2->op==TK_OR
   && (sqlite3_પદાવલિ_Implies_પદાવલિ(pParse, pE1, pE2->p_ડાબું, iTab)
             || sqlite3_પદાવલિ_Implies_પદાવલિ(pParse, pE1, pE2->p_જમણું, iTab) )
  ){
    પાછા 1;
  }
  જો( pE2->op==TK_NOTNULL
   && પદાવલિ_Implies_Not_Null(pParse, pE1, pE2->p_ડાબું, iTab, 0)
  ){
    પાછા 1;
  }
  જો( sqlite3_પદાવલિ_Is_IIF(pParse->db, pE1) ){
    પાછા sqlite3_પદાવલિ_Implies_પદાવલિ(pParse,pE1->x.pList->a[0].p_પદાવલિ,pE2,iTab);
  }
  પાછા 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
સ્થિર શૂન્ય bothImplyNotNullRow(Walker *pWalker, પદાવલિ *pE1, પદાવલિ *pE2){
  જો( pWalker->eCode==0 ){
    sqlite3_Walk_પદાવલિ(pWalker, pE1);
    જો( pWalker->eCode ){
      pWalker->eCode = 0;
      sqlite3_Walk_પદાવલિ(pWalker, pE2);
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
સ્થિર પૂર્ણાંક impliesNotNullRow(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  testcase( p_પદાવલિ->op==TK_AGG_COLUMN );
  testcase( p_પદાવલિ->op==TK_AGG_FUNCTION );
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) ) પાછા WRC_Prune;
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    પાછા WRC_Prune;
  }
  પસંદગી( p_પદાવલિ->op ){
    સ્થિતિ TK_ISNOT:
    સ્થિતિ TK_ISNULL:
    સ્થિતિ TK_NOTNULL:
    સ્થિતિ TK_IS:
    સ્થિતિ TK_VECTOR:
    સ્થિતિ TK_FUNCTION:
    સ્થિતિ TK_TRUTH:
    સ્થિતિ TK_CASE:
      testcase( p_પદાવલિ->op==TK_ISNOT );
      testcase( p_પદાવલિ->op==TK_ISNULL );
      testcase( p_પદાવલિ->op==TK_NOTNULL );
      testcase( p_પદાવલિ->op==TK_IS );
      testcase( p_પદાવલિ->op==TK_VECTOR );
      testcase( p_પદાવલિ->op==TK_FUNCTION );
      testcase( p_પદાવલિ->op==TK_TRUTH );
      testcase( p_પદાવલિ->op==TK_CASE );
      પાછા WRC_Prune;

    સ્થિતિ TK_COLUMN:
      જો( pWalker->u.iCur==p_પદાવલિ->iTable ){
        pWalker->eCode = 1;
        પાછા WRC_Abort;
      }
      પાછા WRC_Prune;

    સ્થિતિ TK_OR:
    સ્થિતિ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_પદાવલિ->op==TK_OR );
      testcase( p_પદાવલિ->op==TK_AND );
      bothImplyNotNullRow(pWalker, p_પદાવલિ->p_ડાબું, p_પદાવલિ->p_જમણું);
      પાછા WRC_Prune;
       
    સ્થિતિ TK_IN:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      જો( પદાવલિ_Use_XList(p_પદાવલિ) && ALWAYS(p_પદાવલિ->x.pList->n_પદાવલિ>0) ){
        sqlite3_Walk_પદાવલિ(pWalker, p_પદાવલિ->p_ડાબું);
      }
      પાછા WRC_Prune;

    સ્થિતિ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pList->n_પદાવલિ==2 );
      sqlite3_Walk_પદાવલિ(pWalker, p_પદાવલિ->p_ડાબું);
      bothImplyNotNullRow(pWalker, p_પદાવલિ->x.pList->a[0].p_પદાવલિ,
                                   p_પદાવલિ->x.pList->a[1].p_પદાવલિ);
      પાછા WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    સ્થિતિ TK_EQ:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE: {
      પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
      પદાવલિ *p_જમણું = p_પદાવલિ->p_જમણું;
      testcase( p_પદાવલિ->op==TK_EQ );
      testcase( p_પદાવલિ->op==TK_NE );
      testcase( p_પદાવલિ->op==TK_LT );
      testcase( p_પદાવલિ->op==TK_LE );
      testcase( p_પદાવલિ->op==TK_GT );
      testcase( p_પદાવલિ->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      પ્રતિજ્ઞા( p_ડાબું->op!=TK_COLUMN || પદાવલિ_Use_YTab(p_ડાબું) );
      પ્રતિજ્ઞા( p_જમણું->op!=TK_COLUMN || પદાવલિ_Use_YTab(p_જમણું) );
      જો( (p_ડાબું->op==TK_COLUMN
           && ALWAYS(p_ડાબું->y.pTab!=0)
           && IsVirtual(p_ડાબું->y.pTab))
       || (p_જમણું->op==TK_COLUMN
           && ALWAYS(p_જમણું->y.pTab!=0)
           && IsVirtual(p_જમણું->y.pTab))
      ){
        પાછા WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    મૂળભૂત:
      પાછા WRC_Continue;
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
પૂર્ણાંક sqlite3_પદાવલિ_Implies_Non_Null_Row(પદાવલિ *p, પૂર્ણાંક iTab, પૂર્ણાંક isRJ){
  Walker w;
  p = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p);
  જો( p==0 ) પાછા 0;
  જો( p->op==TK_NOTNULL ){
    p = p->p_ડાબું;
  }નહીંતર{
    જ્યારે( p->op==TK_AND ){
      જો( sqlite3_પદાવલિ_Implies_Non_Null_Row(p->p_ડાબું, iTab, isRJ) ) પાછા 1;
      p = p->p_જમણું;
    }
  }
  w.x_પદાવલિ_Callback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_પદાવલિ(&w, p);
  પાછા w.eCode;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
સંરચના IdxCover {
  Index *pIdx;     /* The index to be tested for coverage */
  પૂર્ણાંક iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Idx_Cover(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_COLUMN
   && p_પદાવલિ->iTable==pWalker->u.pIdxCover->iCur
   && sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, p_પદાવલિ->iColumn)<0
  ){
    pWalker->eCode = 1;
    પાછા WRC_Abort;
  }
  પાછા WRC_Continue;
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
પૂર્ણાંક sqlite3_પદાવલિ_Covered_By_Index(
  પદાવલિ *p_પદાવલિ,        /* The index to be tested */
  પૂર્ણાંક iCur,           /* The cursor number for the corresponding table */
  Index *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  સંરચના IdxCover xcov;
  memset(&w, 0, કદ(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_પદાવલિ_Callback = પદાવલિ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_પદાવલિ(&w, p_પદાવલિ);
  પાછા !w.eCode;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
સંરચના RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  પૂર્ણાંક *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
સ્થિર પૂર્ણાંક selectRefEnter(Walker *pWalker, Select *pSelect){
  સંરચના RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  પૂર્ણાંક *piNew;
  જો( pSrc->nSrc==0 ) પાછા WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*કદ(પૂર્ણાંક));
  જો( piNew==0 ){
    p->nExclude = 0;
    પાછા WRC_Abort;
  }નહીંતર{
    p->aiExclude = piNew;
  }
  માટે(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  પાછા WRC_Continue;
}
સ્થિર શૂન્ય selectRefLeave(Walker *pWalker, Select *pSelect){
  સંરચના RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  જો( p->nExclude ){
    પ્રતિજ્ઞા( p->nExclude>=pSrc->nSrc );
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
સ્થિર પૂર્ણાંક પદાવલિ_Ref_To_Src_List(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_COLUMN
   || p_પદાવલિ->op==TK_AGG_COLUMN
  ){
    પૂર્ણાંક i;
    સંરચના RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    પૂર્ણાંક nSrc = pSrc ? pSrc->nSrc : 0;
    માટે(i=0; i<nSrc; i++){
      જો( p_પદાવલિ->iTable==pSrc->a[i].iCursor ){
        pWalker->eCode |= 1;
        પાછા WRC_Continue;
      }
    }
    માટે(i=0; i<p->nExclude && p->aiExclude[i]!=p_પદાવલિ->iTable; i++){}
    જો( i>=p->nExclude ){
      pWalker->eCode |= 2;
    }
  }
  પાછા WRC_Continue;
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
પૂર્ણાંક sqlite3ReferencesSrcList(Parse *pParse, પદાવલિ *p_પદાવલિ, SrcList *pSrcList){
  Walker w;
  સંરચના RefSrcList x;
  પ્રતિજ્ઞા( pParse->db!=0 );
  memset(&w, 0, કદ(w));
  memset(&x, 0, કદ(x));
  w.x_પદાવલિ_Callback = પદાવલિ_Ref_To_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  પ્રતિજ્ઞા( p_પદાવલિ->op==TK_AGG_FUNCTION );
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  sqlite3_Walk_પદાવલિ_List(&w, p_પદાવલિ->x.pList);
  જો( p_પદાવલિ->p_ડાબું ){
    પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું->op==TK_ORDER );
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ->p_ડાબું) );
    પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું->x.pList!=0 );
    sqlite3_Walk_પદાવલિ_List(&w, p_પદાવલિ->p_ડાબું->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
    sqlite3_Walk_પદાવલિ(&w, p_પદાવલિ->y.pWin->pFilter);
  }
#endif
  જો( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  જો( w.eCode & 0x01 ){
    પાછા 1;
  }નહીંતર જો( w.eCode ){
    પાછા 0;
  }નહીંતર{
    પાછા -1;
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
સ્થિર પૂર્ણાંક agginfo_Persist_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( ALWAYS(!પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly|EP_Reduced))
   && p_પદાવલિ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_પદાવલિ->pAggInfo;
    પૂર્ણાંક iAgg = p_પદાવલિ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    પ્રતિજ્ઞા( iAgg>=0 );
    જો( p_પદાવલિ->op!=TK_AGG_FUNCTION ){
      જો( iAgg<pAggInfo->nColumn
       && pAggInfo->aCol[iAgg].pCExpr==p_પદાવલિ
      ){
        p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
        જો( p_પદાવલિ && !sqlite3_પદાવલિ_Deferred_Delete(pParse, p_પદાવલિ) ){
          pAggInfo->aCol[iAgg].pCExpr = p_પદાવલિ;
        }
      }
    }નહીંતર{
      પ્રતિજ્ઞા( p_પદાવલિ->op==TK_AGG_FUNCTION );
      જો( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_પદાવલિ
      ){
        p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
        જો( p_પદાવલિ && !sqlite3_પદાવલિ_Deferred_Delete(pParse, p_પદાવલિ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_પદાવલિ;
        }
      }
    }
  }
  પાછા WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
શૂન્ય sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, કદ(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_પદાવલિ_Callback = agginfo_Persist_પદાવલિ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
સ્થિર પૂર્ણાંક addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  પૂર્ણાંક i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       કદ(pInfo->aCol[0]),
       &pInfo->nColumn,
       &i
  );
  પાછા i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
સ્થિર પૂર્ણાંક addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  પૂર્ણાંક i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       કદ(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  પાછા i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
સ્થિર શૂન્ય findOrCreateAggInfoColumn(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  પદાવલિ *p_પદાવલિ          /* Expr describing the column to find or insert */
){
  સંરચના AggInfo_col *pCol;
  પૂર્ણાંક k;
  પૂર્ણાંક mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  પ્રતિજ્ઞા( mxTerm <= SMXV(i16) );
  પ્રતિજ્ઞા( pAggInfo->iFirstReg==0 );
  pCol = pAggInfo->aCol;
  માટે(k=0; k<pAggInfo->nColumn; k++, pCol++){
    જો( pCol->pCExpr==p_પદાવલિ ) પાછા;
    જો( pCol->iTable==p_પદાવલિ->iTable
     && pCol->iColumn==p_પદાવલિ->iColumn
     && p_પદાવલિ->op!=TK_IF_NULL_ROW
    ){
      જાઓ fix_up_પદાવલિ;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  જો( k<0 ){
    /* OOM on resize */
    પ્રતિજ્ઞા( pParse->db->mallocFailed );
    પાછા;
  }
  જો( k>mxTerm ){
    sqlite3_Error_સંદેશ(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];
  પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
  pCol->pTab = p_પદાવલિ->y.pTab;
  pCol->iTable = p_પદાવલિ->iTable;
  pCol->iColumn = p_પદાવલિ->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = p_પદાવલિ;
  જો( pAggInfo->pGroupBy && p_પદાવલિ->op!=TK_IF_NULL_ROW ){
    પૂર્ણાંક j, n;
    પદાવલિ_List *pGB = pAggInfo->pGroupBy;
    સંરચના પદાવલિ_List_item *pTerm = pGB->a;
    n = pGB->n_પદાવલિ;
    માટે(j=0; j<n; j++, pTerm++){
      પદાવલિ *pE = pTerm->p_પદાવલિ;
      જો( pE->op==TK_COLUMN
       && pE->iTable==p_પદાવલિ->iTable
       && pE->iColumn==p_પદાવલિ->iColumn
      ){
        pCol->iSorterColumn = j;
        તોડો;
      }
    }
  }
  જો( pCol->iSorterColumn<0 ){
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_પદાવલિ:
  પદાવલિ_Set_VVAProperty(p_પદાવલિ, EP_NoReduce);
  પ્રતિજ્ઞા( p_પદાવલિ->pAggInfo==0 || p_પદાવલિ->pAggInfo==pAggInfo );
  p_પદાવલિ->pAggInfo = pAggInfo;
  જો( p_પદાવલિ->op==TK_COLUMN ){
    p_પદાવલિ->op = TK_AGG_COLUMN;
  }
  પ્રતિજ્ઞા( k <= SMXV(p_પદાવલિ->iAgg) );
  p_પદાવલિ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
સ્થિર પૂર્ણાંક analyzeAggregate(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  પ્રતિજ્ઞા( pNC->ncFlags & NC_UAggInfo );
  પ્રતિજ્ઞા( pAggInfo->iFirstReg==0 );
  પસંદગી( p_પદાવલિ->op ){
    મૂળભૂત: {
      Indexed_પદાવલિ *pIEpr;
      પદાવલિ tmp;
      પ્રતિજ્ઞા( pParse->iSelfTab==0 );
      જો( (pNC->ncFlags & NC_InAggFunc)==0 ) તોડો;
      જો( pParse->pIdxEpr==0 ) તોડો;
      માટે(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        પૂર્ણાંક iDataCur = pIEpr->iDataCur;
        જો( iDataCur<0 ) ચાલુ;
        જો( sqlite3_પદાવલિ_Compare(0, p_પદાવલિ, pIEpr->p_પદાવલિ, iDataCur)==0 ) તોડો;
      }
      જો( pIEpr==0 ) તોડો;
      જો( NEVER(!પદાવલિ_Use_YTab(p_પદાવલિ)) ) તોડો;
      માટે(i=0; i<pSrcList->nSrc; i++){
         જો( pSrcList->a[i].iCursor==pIEpr->iDataCur ){
           testcase( i>0 );
           તોડો;
         }
      }
      જો( i>=pSrcList->nSrc ) તોડો;
      જો( NEVER(p_પદાવલિ->pAggInfo!=0) ) તોડો; /* Resolved by outer context */
      જો( pParse->nErr ){ પાછા WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, કદ(tmp));
      tmp.op = TK_AGG_COLUMN;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      જો( pParse->nErr ){ પાછા WRC_Abort; }
      પ્રતિજ્ઞા( pAggInfo->aCol!=0 );
      પ્રતિજ્ઞા( tmp.iAgg<pAggInfo->nColumn );
      pAggInfo->aCol[tmp.iAgg].pCExpr = p_પદાવલિ;
      p_પદાવલિ->pAggInfo = pAggInfo;
      p_પદાવલિ->iAgg = tmp.iAgg;
      પાછા WRC_Prune;
    }
    સ્થિતિ TK_IF_NULL_ROW:
    સ્થિતિ TK_AGG_COLUMN:
    સ્થિતિ TK_COLUMN: {
      testcase( p_પદાવલિ->op==TK_AGG_COLUMN );
      testcase( p_પદાવલિ->op==TK_COLUMN );
      testcase( p_પદાવલિ->op==TK_IF_NULL_ROW );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      જો( ALWAYS(pSrcList!=0) ){
        SrcItem *pItem = pSrcList->a;
        માટે(i=0; i<pSrcList->nSrc; i++, pItem++){
          પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly|EP_Reduced) );
          જો( p_પદાવલિ->iTable==pItem->iCursor ){
            findOrCreateAggInfoColumn(pParse, pAggInfo, p_પદાવલિ);
            તોડો;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      પાછા WRC_Continue;
    }
    સ્થિતિ TK_AGG_FUNCTION: {
      જો( (pNC->ncFlags & NC_InAggFunc)==0
       && pWalker->walkerDepth==p_પદાવલિ->op2
       && p_પદાવલિ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        સંરચના AggInfo_func *pItem = pAggInfo->aFunc;
        પૂર્ણાંક mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
        પ્રતિજ્ઞા( mxTerm <= SMXV(i16) );
        માટે(i=0; i<pAggInfo->nFunc; i++, pItem++){
          જો( NEVER(pItem->pFExpr==p_પદાવલિ) ) તોડો;
          જો( sqlite3_પદાવલિ_Compare(0, pItem->pFExpr, p_પદાવલિ, -1)==0 ){
            તોડો;
          }
        }
        જો( i>mxTerm ){
          sqlite3_Error_સંદેશ(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;
          પ્રતિજ્ઞા( i<pAggInfo->nFunc );
        }નહીંતર જો( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          જો( i>=0 ){
            પૂર્ણાંક nArg;
            પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = p_પદાવલિ;
            પ્રતિજ્ઞા( પદાવલિ_Use_UToken(p_પદાવલિ) );
            nArg = p_પદાવલિ->x.pList ? p_પદાવલિ->x.pList->n_પદાવલિ : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                                         p_પદાવલિ->u.zToken, nArg, enc, 0);
            પ્રતિજ્ઞા( pItem->bOBUnique==0 );
            જો( p_પદાવલિ->p_ડાબું
             && (pItem->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              પદાવલિ_List *pOBList;
              પ્રતિજ્ઞા( nArg>0 );
              પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું->op==TK_ORDER );
              પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ->p_ડાબું) );
              pItem->iOBTab = pParse->nTab++;
              pOBList = p_પદાવલિ->p_ડાબું->x.pList;
              પ્રતિજ્ઞા( pOBList->n_પદાવલિ>0 );
              પ્રતિજ્ઞા( pItem->bOBUnique==0 );
              જો( pOBList->n_પદાવલિ==1
               && nArg==1
               && sqlite3_પદાવલિ_Compare(0,pOBList->a[0].p_પદાવલિ,
                               p_પદાવલિ->x.pList->a[0].p_પદાવલિ,-1)==0
              ){
                pItem->bOBPayload = 0;
                pItem->bOBUnique = પદાવલિ_Has_Property(p_પદાવલિ, EP_Distinct);
              }નહીંતર{
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype =
                    (pItem->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }નહીંતર{
              pItem->iOBTab = -1;
            }
            જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Distinct) && !pItem->bOBUnique ){
              pItem->iDistinct = pParse->nTab++;
            }નહીંતર{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly|EP_Reduced) );
        પદાવલિ_Set_VVAProperty(p_પદાવલિ, EP_NoReduce);
        પ્રતિજ્ઞા( i <= SMXV(p_પદાવલિ->iAgg) );
        p_પદાવલિ->iAgg = (i16)i;
        p_પદાવલિ->pAggInfo = pAggInfo;
        પાછા WRC_Prune;
      }નહીંતર{
        પાછા WRC_Continue;
      }
    }
  }
  પાછા WRC_Continue;
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
શૂન્ય sqlite3_પદાવલિ_Analyze_Aggregates(NameContext *pNC, પદાવલિ *p_પદાવલિ){
  Walker w;
  w.x_પદાવલિ_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  પ્રતિજ્ઞા( pNC->pSrcList!=0 );
  sqlite3_Walk_પદાવલિ(&w, p_પદાવલિ);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
શૂન્ય sqlite3_પદાવલિ_Analyze_Agg_List(NameContext *pNC, પદાવલિ_List *pList){
  સંરચના પદાવલિ_List_item *pItem;
  પૂર્ણાંક i;
  જો( pList ){
    માટે(pItem=pList->a, i=0; i<pList->n_પદાવલિ; i++, pItem++){
      sqlite3_પદાવલિ_Analyze_Aggregates(pNC, pItem->p_પદાવલિ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
પૂર્ણાંક sqlite3GetTempReg(Parse *pParse){
  જો( pParse->nTempReg==0 ){
    પાછા ++pParse->nMem;
  }
  પાછા pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
શૂન્ય sqlite3ReleaseTempReg(Parse *pParse, પૂર્ણાંક iReg){
  જો( iReg ){
#ifdef SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    પૂર્ણાંક kk;
    માટે(kk=0; kk<pParse->nTempReg; kk++){
      પ્રતિજ્ઞા( pParse->aTempReg[kk]!=iReg );
    }
#endif
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    જો( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
પૂર્ણાંક sqlite3GetTempRange(Parse *pParse, પૂર્ણાંક nReg){
  પૂર્ણાંક i, n;
  જો( nReg==1 ) પાછા sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  જો( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }નહીંતર{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  પાછા i;
}
શૂન્ય sqlite3ReleaseTempRange(Parse *pParse, પૂર્ણાંક iReg, પૂર્ણાંક nReg){
  જો( nReg==1 ){
    sqlite3ReleaseTempReg(pParse, iReg);
    પાછા;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  જો( nReg>pParse->nRangeReg ){
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
શૂન્ય sqlite3ClearTempRegCache(Parse *pParse){
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
શૂન્ય sqlite3TouchRegister(Parse *pParse, પૂર્ણાંક iReg){
  જો( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#જો defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
પૂર્ણાંક sqlite3FirstAvailableRegister(Parse *pParse, પૂર્ણાંક iMin){
  અચલ પદાવલિ_List *pList = pParse->p_Const_પદાવલિ;
  જો( pList ){
    પૂર્ણાંક i;
    માટે(i=0; i<pList->n_પદાવલિ; i++){
      જો( pList->a[i].u.i_Const_પદાવલિ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_પદાવલિ_Reg + 1;
      }
    }
  }
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
  પાછા iMin;
}
#endif /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ifdef SQLITE_DEBUG
પૂર્ણાંક sqlite3NoTempsInRange(Parse *pParse, પૂર્ણાંક iFirst, પૂર્ણાંક iLast){
  પૂર્ણાંક i;
  જો( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > iFirst
   && pParse->iRangeReg <= iLast
  ){
     પાછા 0;
  }
  માટે(i=0; i<pParse->nTempReg; i++){
    જો( pParse->aTempReg[i]>=iFirst && pParse->aTempReg[i]<=iLast ){
      પાછા 0;
    }
  }
  જો( pParse->p_Const_પદાવલિ ){
    પદાવલિ_List *pList = pParse->p_Const_પદાવલિ;
    માટે(i=0; i<pList->n_પદાવલિ; i++){
      પૂર્ણાંક iReg = pList->a[i].u.i_Const_પદાવલિ_Reg;
      જો( iReg==0 ) ચાલુ;
      જો( iReg>=iFirst && iReg<=iLast ) પાછા 0;
    }
  }
  પાછા 1;
}
#endif /* SQLITE_DEBUG */
