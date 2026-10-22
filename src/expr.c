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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* Forward declarations */
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Between(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ),ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸದಿಶ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ *pi_ಗಮ್ಯ_Free);

/*
** Return the affinity character for a single column of a table.
*/
ಅಕ್ಷರ sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_Affinity(ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pTab, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 || NEVER(i_ಲಂಬಸಾಲು>=pTab->n_ಲಂಬಸಾಲು) ) ಹಿಂತಿರುಗಿ SQLITE_AFF_INTEGER;
  ಹಿಂತಿರುಗಿ pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].affinity;
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
ಅಕ್ಷರ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ op;
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಆಗ( 1 /* exit-by-break */ ){
    ಆದರೆ( op==TK_ಸ್ತಂಭ || (op==TK_AGG_ಸ್ತಂಭ && p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0) ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
      ಹಿಂತಿರುಗಿ sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
    }
    ಆದರೆ( op==TK_SELECT ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
    ಆದರೆ( op==TK_CAST ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಹಿಂತಿರುಗಿ sqlite3AffinityType(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( op==TK_SELECT_ಸ್ತಂಭ ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ < p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ >= 0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect->pEList->a[p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ
      );
    }
    ಆದರೆ( op==TK_ಸದಿಶ
     || (op==TK_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==SQLITE_AFF_DEFER)
    ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_If_Null_ಅಡ್ಡಸಾಲು) ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE
           || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು
           || (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ರಿಜಿಸ್ಟರ್ && p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IF_NULL_ಅಡ್ಡಸಾಲು) );
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( op!=TK_ರಿಜಿಸ್ಟರ್ ) ನಿಲ್ಲಿಸು;
    op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
    ಆದರೆ( NEVER( op==TK_ರಿಜಿಸ್ಟರ್ ) ) ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆಗ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
      ಸ್ಥಿತಿ TK_COLLATE:
      ಸ್ಥಿತಿ TK_IF_NULL_ಅಡ್ಡಸಾಲು:
      ಸ್ಥಿತಿ TK_UPLUS:  {
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_NULL: {
        p_ಅಭಿವ್ಯಕ್ತಿ = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_STRING: {
        ಹಿಂತಿರುಗಿ 0x02;
      }
      ಸ್ಥಿತಿ TK_BLOB: {
        ಹಿಂತಿರುಗಿ 0x04;
      }
      ಸ್ಥಿತಿ TK_CONCAT: {
        ಹಿಂತಿರುಗಿ 0x06;
      }
      ಸ್ಥಿತಿ TK_VARIABLE:
      ಸ್ಥಿತಿ TK_AGG_FUNCTION:
      ಸ್ಥಿತಿ TK_FUNCTION: {
        ಹಿಂತಿರುಗಿ 0x07;
      }
      ಸ್ಥಿತಿ TK_ಸ್ತಂಭ:
      ಸ್ಥಿತಿ TK_AGG_ಸ್ತಂಭ:
      ಸ್ಥಿತಿ TK_SELECT:
      ಸ್ಥಿತಿ TK_CAST:
      ಸ್ಥಿತಿ TK_SELECT_ಸ್ತಂಭ:
      ಸ್ಥಿತಿ TK_ಸದಿಶ:  {
        ಪೂರ್ಣಾಂಕ aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( aff>=SQLITE_AFF_NUMERIC ) ಹಿಂತಿರುಗಿ 0x05;
        ಆದರೆ( aff==SQLITE_AFF_ಪಠ್ಯ )    ಹಿಂತಿರುಗಿ 0x06;
        ಹಿಂತಿರುಗಿ 0x07;
      }
      ಸ್ಥಿತಿ TK_CASE: {
        ಪೂರ್ಣಾಂಕ res = 0;
        ಪೂರ್ಣಾಂಕ ii;
        ಅಭಿವ್ಯಕ್ತಿ_List *pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) && pList!=0 );
        ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ > 0);
        ಗಾಗಿ(ii=1; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii+=2){
          res |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಆದರೆ( pList->n_ಅಭಿವ್ಯಕ್ತಿ % 2 ){
          res |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಹಿಂತಿರುಗಿ res;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಹಿಂತಿರುಗಿ 0x01;
      }
    } /* End of switch(op) */
  } /* End of while(pExpr) */
  ಹಿಂತಿರುಗಿ 0x00;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to a new Expr node that
** implements the COLLATE operator.
**
** If a memory allocation error occurs, that fact is recorded in pParse->db
** and the pExpr parameter is returned unchanged.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_Token(
  Parse *pParse,           /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,             /* Add the "COLLATE" clause to this expression */
  ಸ್ಥಿರಾಂಕ Token *pCollName,  /* Name of collating sequence */
  ಪೂರ್ಣಾಂಕ dequote              /* True to dequote pCollName */
){
  ಆದರೆ( pCollName->n>0 && pParse->nErr==0 ){
    ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_COLLATE, pCollName, dequote);
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    ಆದರೆ( pNew ){
      pNew->p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ;
      pNew->flags |= EP_Collate|EP_Skip;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
      pNew->n_ಎತ್ತರ = p_ಅಭಿವ್ಯಕ್ತಿ->n_ಎತ್ತರ+1;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(pParse, pNew->n_ಎತ್ತರ);
      testcase( pParse->nErr>0 );
#ಆದರೆ_ಅಂತ್ಯ
      p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
    }
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* Add the "COLLATE" clause to this expression */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zC        /* The collating sequence name */
){
  Token s;
  ಪ್ರತಿಜ್ಞಾ( zC!=0 );
  sqlite3TokenInit(&s, (ಅಕ್ಷರ*)zC);
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_Token(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &s, 0);
}

/*
** Skip over any TK_COLLATE operators.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆಗ( p_ಅಭಿವ್ಯಕ್ತಿ && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip) ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE );
    p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  }  
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Skip over any TK_COLLATE operators and/or any unlikely()
** or likelihood() or likely() functions at the root of an
** expression.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆಗ( p_ಅಭಿವ್ಯಕ್ತಿ && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_Unlikely) ){
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Unlikely) ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ನಿಲ್ಲಿಸು;
    }
  }  
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
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
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಆಗ( p ){
    ಪೂರ್ಣಾಂಕ op = p->op;
    ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ) op = p->op2;
    ಆದರೆ( (op==TK_AGG_ಸ್ತಂಭ && p->y.pTab!=0)
     || op==TK_ಸ್ತಂಭ || op==TK_TRIGGER
    ){
      ಪೂರ್ಣಾಂಕ j;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
      ಪ್ರತಿಜ್ಞಾ( p->y.pTab!=0 );
      ಆದರೆ( (j = p->i_ಸ್ತಂಭ)>=0 ){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl = sqlite3_ಸ್ತಂಭ_Coll(&p->y.pTab->a_ಲಂಬಸಾಲು[j]);
        pColl = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, ENC(db), zColl, 0);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( op==TK_CAST || op==TK_UPLUS ){
      p = p->p_ಎಡ;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( op==TK_ಸದಿಶ
     || (op==TK_FUNCTION && p->aff_ಅಭಿವ್ಯಕ್ತಿ==SQLITE_AFF_DEFER)
    ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      p = p->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( op==TK_COLLATE ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) );
      pColl = sqlite3GetCollSeq(pParse, ENC(db), 0, p->u.zToken);
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( p->flags & EP_Collate ){
      ಆದರೆ( p->p_ಎಡ && (p->p_ಎಡ->flags & EP_Collate)!=0 ){
        p = p->p_ಎಡ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಮುಂದಿನ  = p->p_ಬಲ;
        /* The Expr.x union is never used at the same time as Expr.pRight */
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) || p->x.pList==0 || p->p_ಬಲ==0 );
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList!=0 && !db->mallocFailed ){
          ಪೂರ್ಣಾಂಕ i;
          ಗಾಗಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
            ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
              p_ಮುಂದಿನ = p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
              ನಿಲ್ಲಿಸು;
            }
          }
        }
        p = p_ಮುಂದಿನ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( sqlite3CheckCollSeq(pParse, pColl) ){
    pColl = 0;
  }
  ಹಿಂತಿರುಗಿ pColl;
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
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  CollSeq *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( p==0 ) p = pParse->db->pDfltColl;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಹಿಂತಿರುಗಿ p;
}

/*
** Return TRUE if the two expressions have equivalent collating sequences.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq_ಹೊಂದಿಕೆ(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pE1, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pE2){
  CollSeq *pColl1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, pE1);
  CollSeq *pColl2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, pE2);
  ಪ್ರತಿಜ್ಞಾ( (pColl1==pColl2) ==
          (sqlite3_stricmp(pColl1->zName,pColl2->zName)==0) );
  ಹಿಂತಿರುಗಿ pColl1==pColl2;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
ಅಕ್ಷರ sqlite3CompareAffinity(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಅಕ್ಷರ aff2){
  ಅಕ್ಷರ aff1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( aff1>SQLITE_AFF_NONE && aff2>SQLITE_AFF_NONE ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    ಆದರೆ( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      ಹಿಂತಿರುಗಿ SQLITE_AFF_NUMERIC;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ SQLITE_AFF_BLOB;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* One side is a column, the other is not. Use the columns affinity. */
    ಪ್ರತಿಜ್ಞಾ( aff1<=SQLITE_AFF_NONE || aff2<=SQLITE_AFF_NONE );
    ಹಿಂತಿರುಗಿ (aff1<=SQLITE_AFF_NONE ? aff2 : aff1) | SQLITE_AFF_NONE;
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
ಸ್ಥಿರ ಅಕ್ಷರ comparisonAffinity(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಕ್ಷರ aff;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಒಳ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LT ||
          p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GT || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LE ||
          p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
    aff = sqlite3CompareAffinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, aff);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( aff==0 ){
    aff = SQLITE_AFF_BLOB;
  }
  ಹಿಂತಿರುಗಿ aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Ok(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಅಕ್ಷರ idx_affinity){
  ಅಕ್ಷರ aff = comparisonAffinity(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( aff<SQLITE_AFF_ಪಠ್ಯ ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( aff==SQLITE_AFF_ಪಠ್ಯ ){
    ಹಿಂತಿರುಗಿ idx_affinity==SQLITE_AFF_ಪಠ್ಯ;
  }
  ಹಿಂತಿರುಗಿ sqlite3IsNumericAffinity(idx_affinity);
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
ಸ್ಥಿರ u8 ದ್ವಿಮಾನ_Compare_P5(
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pExpr1,   /* Left operand */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pExpr2,   /* Right operand */
  ಪೂರ್ಣಾಂಕ jumpIfNull        /* Extra flags added to P5 */
){
  u8 aff = (ಅಕ್ಷರ)sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  ಹಿಂತಿರುಗಿ aff;
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
CollSeq *sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(
  Parse *pParse,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ
){
  CollSeq *pColl;
  ಪ್ರತಿಜ್ಞಾ( p_ಎಡ );
  ಆದರೆ( p_ಎಡ->flags & EP_Collate ){
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಎಡ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಬಲ && (p_ಬಲ->flags & EP_Collate)!=0 ){
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಬಲ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಎಡ);
    ಆದರೆ( !pColl ){
      pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಬಲ);
    }
  }
  ಹಿಂತಿರುಗಿ pColl;
}

/* Expression p is a comparison operator.  Return a collation sequence
** appropriate for the comparison operator.
**
** This is normally just a wrapper around sqlite3BinaryCompareCollSeq().
** However, if the OP_Commuted flag is set, then the order of the operands
** is reversed in the sqlite3BinaryCompareCollSeq() call so that the
** correct collating sequence is found.
*/
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Commuted) ){
    ಹಿಂತಿರುಗಿ sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p->p_ಬಲ, p->p_ಎಡ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p->p_ಎಡ, p->p_ಬಲ);
  }
}

/*
** Generate code for a comparison operator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Compare(
  Parse *pParse,    /* The parsing (and code generating) context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,      /* The left operand */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,     /* The right operand */
  ಪೂರ್ಣಾಂಕ opcode,       /* The comparison opcode */
  ಪೂರ್ಣಾಂಕ in1, ಪೂರ್ಣಾಂಕ in2, /* Register holding operands */
  ಪೂರ್ಣಾಂಕ dest,         /* Jump here if true.  */
  ಪೂರ್ಣಾಂಕ jumpIfNull,   /* If true, jump if either operand is NULL */
  ಪೂರ್ಣಾಂಕ isCommuted    /* The comparison has been commuted */
){
  ಪೂರ್ಣಾಂಕ p5;
  ಪೂರ್ಣಾಂಕ addr;
  CollSeq *p4;

  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( isCommuted ){
    p4 = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಬಲ, p_ಎಡ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p4 = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p_ಎಡ, p_ಬಲ);
  }
  p5 = ದ್ವಿಮಾನ_Compare_P5(p_ಎಡ, p_ಬಲ, jumpIfNull);
  addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(pParse->pVdbe, opcode, in2, dest, in1,
                           (ಶೂನ್ಯ*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  ಹಿಂತಿರುಗಿ addr;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ)>1;
}

/*
** If the expression passed as the only argument is of type TK_VECTOR
** return the number of expressions in the vector. Or, if the expression
** is a sub-select, return the number of columns in the sub-select. For
** any other type of expression, return 1.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u8 op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ) op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
  ಆದರೆ( op==TK_ಸದಿಶ ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 1;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(ಅಭಿವ್ಯಕ್ತಿ *p_ಸದಿಶ, ಪೂರ್ಣಾಂಕ i){
  ಪ್ರತಿಜ್ಞಾ( i<sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಸದಿಶ) || p_ಸದಿಶ->op==TK_ERROR );
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಸದಿಶ) ){
    ಪ್ರತಿಜ್ಞಾ( p_ಸದಿಶ->op2==0 || p_ಸದಿಶ->op==TK_ರಿಜಿಸ್ಟರ್ );
    ಆದರೆ( p_ಸದಿಶ->op==TK_SELECT || p_ಸದಿಶ->op2==TK_SELECT ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಸದಿಶ) );
      ಹಿಂತಿರುಗಿ p_ಸದಿಶ->x.pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಸದಿಶ) );
      ಹಿಂತಿರುಗಿ p_ಸದಿಶ->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    }
  }
  ಹಿಂತಿರುಗಿ p_ಸದಿಶ;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_ಸದಿಶ_ಕ್ಷೇತ್ರ(
  Parse *pParse,       /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಸದಿಶ,       /* The vector.  List of expressions or a sub-SELECT */
  ಪೂರ್ಣಾಂಕ i_ಕ್ಷೇತ್ರ,          /* Which column of the vector to return */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ           /* Total number of columns in the vector */
){
  ಅಭಿವ್ಯಕ್ತಿ *pRet;
  ಆದರೆ( p_ಸದಿಶ->op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಸದಿಶ) );
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
    pRet = sqlite3PExpr(pParse, TK_SELECT_ಸ್ತಂಭ, 0, 0);
    ಆದರೆ( pRet ){
      ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pRet, EP_ತುಂಬಿದ_ಗಾತ್ರ);
      pRet->i_ಕೋಷ್ಟಕ = n_ಕ್ಷೇತ್ರ;
      pRet->i_ಸ್ತಂಭ = i_ಕ್ಷೇತ್ರ;
      pRet->p_ಎಡ = p_ಸದಿಶ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( p_ಸದಿಶ->op==TK_ಸದಿಶ ){
      ಅಭಿವ್ಯಕ್ತಿ **pp_ಸದಿಶ;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಸದಿಶ) );
      pp_ಸದಿಶ = &p_ಸದಿಶ->x.pList->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ;
      p_ಸದಿಶ = *pp_ಸದಿಶ;
      ಆದರೆ( ಒಳ_RENAME_OBJECT ){
        /* This must be a vector UPDATE inside a trigger */
        *pp_ಸದಿಶ = 0;
        ಹಿಂತಿರುಗಿ p_ಸದಿಶ;
      }
    }
    pRet = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, p_ಸದಿಶ, 0);
  }
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** If expression pExpr is of type TK_SELECT, generate code to evaluate
** it. Return the register in which the result is stored (or, if the
** sub-select returns more than one column, the first in an array
** of registers in which the result is stored).
**
** If pExpr is not a TK_SELECT expression, return 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Subselect(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ reg = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
    reg = sqlite3_ಸಂಕೇತ_Subselect(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ reg;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ರಿಜಿಸ್ಟರ್(
  Parse *pParse,                  /* Parse context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಸದಿಶ,                  /* Vector to extract element from */
  ಪೂರ್ಣಾಂಕ i_ಕ್ಷೇತ್ರ,                     /* Field to extract from pVector */
  ಪೂರ್ಣಾಂಕ regSelect,                  /* First in array of registers */
  ಅಭಿವ್ಯಕ್ತಿ *pTmp,                     /* Temporary space */
  ಅಭಿವ್ಯಕ್ತಿ **pp_ಅಭಿವ್ಯಕ್ತಿ,                  /* OUT: Expression element */
  ಪೂರ್ಣಾಂಕ *pRegFree                   /* OUT: Temp register to free */
){
  u8 op = p_ಸದಿಶ->op;
  ಪ್ರತಿಜ್ಞಾ( op==TK_ಸದಿಶ || op==TK_ರಿಜಿಸ್ಟರ್ || op==TK_SELECT || op==TK_ERROR );
  ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ){
    *pp_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಸದಿಶ, i_ಕ್ಷೇತ್ರ);
    ಹಿಂತಿರುಗಿ p_ಸದಿಶ->i_ಕೋಷ್ಟಕ+i_ಕ್ಷೇತ್ರ;
  }
  ಆದರೆ( op==TK_SELECT ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಸದಿಶ) );
    /* Use the temporary expression node to wrap expression iField of the
    ** sub-select in a TK_SELECT_COLUMN node. This causes the caller to
    ** use the affinity of the expression in any comparison, but not the
    ** collation sequence.  */
    memset(pTmp, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    pTmp->op = TK_SELECT_ಸ್ತಂಭ;
    pTmp->p_ಎಡ = p_ಸದಿಶ;
    pTmp->i_ಸ್ತಂಭ = i_ಕ್ಷೇತ್ರ;
    pTmp->i_ಕೋಷ್ಟಕ = p_ಸದಿಶ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    *pp_ಅಭಿವ್ಯಕ್ತಿ = pTmp;
    ಹಿಂತಿರುಗಿ regSelect+i_ಕ್ಷೇತ್ರ;
  }
  ಆದರೆ( op==TK_ಸದಿಶ ){
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಸದಿಶ) );
    *pp_ಅಭಿವ್ಯಕ್ತಿ = p_ಸದಿಶ->x.pList->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, *pp_ಅಭಿವ್ಯಕ್ತಿ, pRegFree);
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_ಸದಿಶ_Compare(
  Parse *pParse,        /* Code generator context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The comparison operation */
  ಪೂರ್ಣಾಂಕ dest,             /* Write results into this register */
  u8 op,                /* Comparison operator */
  u8 p5                 /* SQLITE_NULLEQ or zero */
){
  Vdbe *v = pParse->pVdbe;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
  ಪೂರ್ಣಾಂಕ n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಎಡ);
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ reg_ಎಡ = 0;
  ಪೂರ್ಣಾಂಕ reg_ಬಲ = 0;
  u8 opx = op;
  ಪೂರ್ಣಾಂಕ addrCmp = 0;
  ಪೂರ್ಣಾಂಕ addr_ಮುಗಿದ = sqlite3VdbeMakeLabel(pParse);
  ಪೂರ್ಣಾಂಕ isCommuted = ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted);

  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( n_ಎಡ!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಬಲ) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE
       || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT
       || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LT || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GT
       || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GE
  );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==op || (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS && op==TK_EQ)
            || (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT && op==TK_NE) );
  ಪ್ರತಿಜ್ಞಾ( p5==0 || p_ಅಭಿವ್ಯಕ್ತಿ->op!=op );
  ಪ್ರತಿಜ್ಞಾ( p5==SQLITE_NULLEQ || p_ಅಭಿವ್ಯಕ್ತಿ->op==op );

  ಆದರೆ( op==TK_LE ) opx = TK_LT;
  ಆದರೆ( op==TK_GE ) opx = TK_GT;
  ಆದರೆ( op==TK_NE ) opx = TK_EQ;

  reg_ಎಡ = ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Subselect(pParse, p_ಎಡ);
  reg_ಬಲ = ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Subselect(pParse, p_ಬಲ);

  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, dest);
  ಗಾಗಿ(i=0; 1 /*Loop exits by "break"*/; i++){
    ಪೂರ್ಣಾಂಕ regFree1 = 0, regFree2 = 0;
    ಅಭಿವ್ಯಕ್ತಿ *pL = 0, *pR = 0;
    ಅಭಿವ್ಯಕ್ತಿ tmp1, tmp2;
    ಪೂರ್ಣಾಂಕ r1, r2;
    ಪ್ರತಿಜ್ಞಾ( i>=0 && i<n_ಎಡ );
    ಆದರೆ( addrCmp ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrCmp);
    r1 = ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ರಿಜಿಸ್ಟರ್(pParse, p_ಎಡ, i, reg_ಎಡ, &tmp1, &pL, &regFree1);
    r2 = ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ರಿಜಿಸ್ಟರ್(pParse, p_ಬಲ, i, reg_ಬಲ, &tmp2, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    ಸಂಕೇತ_Compare(pParse, pL, pR, opx, r1, r2, addr_ಮುಗಿದ, p5, isCommuted);
    testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
    testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
    testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
    testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
    testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
    testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree1);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree2);
    ಆದರೆ( (opx==TK_LT || opx==TK_GT) && i<n_ಎಡ-1 ){
      addrCmp = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_ElseEq);
      testcase(opx==TK_LT); VdbeCoverageIf(v,opx==TK_LT);
      testcase(opx==TK_GT); VdbeCoverageIf(v,opx==TK_GT);
    }
    ಆದರೆ( p5==SQLITE_NULLEQ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, dest);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ZeroOrNull, r1, dest, r2);
    }
    ಆದರೆ( i==n_ಎಡ-1 ){
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( opx==TK_EQ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NotNull, dest, addr_ಮುಗಿದ); VdbeCoverage(v);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( op==TK_LT || op==TK_GT || op==TK_LE || op==TK_GE );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, addr_ಮುಗಿದ);
      ಆದರೆ( i==n_ಎಡ-2 ) opx = op;
    }
  }
  sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addr_ಮುಗಿದ);
  ಆದರೆ( op==TK_NE ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Not, dest, dest);
  }
}

#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(Parse *pParse, ಪೂರ್ಣಾಂಕ n_ಎತ್ತರ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ mx_ಎತ್ತರ = pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ];
  ಆದರೆ( n_ಎತ್ತರ>mx_ಎತ್ತರ ){
    sqlite3_Error_ಸಂದೇಶ(pParse,
       "Expression tree is too large (maximum depth %d)", mx_ಎತ್ತರ
    );
    rc = SQLITE_ERROR;
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ *pn_ಎತ್ತರ){
  ಆದರೆ( p ){
    ಆದರೆ( p->n_ಎತ್ತರ>*pn_ಎತ್ತರ ){
      *pn_ಎತ್ತರ = p->n_ಎತ್ತರ;
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ_List(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಂಕ *pn_ಎತ್ತರ){
  ಆದರೆ( p ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(p->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, pn_ಎತ್ತರ);
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ ಎತ್ತರ_ದ_Select(ಸ್ಥಿರಾಂಕ Select *pSelect, ಪೂರ್ಣಾಂಕ *pn_ಎತ್ತರ){
  ಸ್ಥಿರಾಂಕ Select *p;
  ಗಾಗಿ(p=pSelect; p; p=p->p_ಪೂರ್ವ){
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, pn_ಎತ್ತರ);
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(p->pHaving, pn_ಎತ್ತರ);
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(p->p_ಎಲ್ಲೆ, pn_ಎತ್ತರ);
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ_List(p->pEList, pn_ಎತ್ತರ);
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ_List(p->p_ಗುಂಪು_By, pn_ಎತ್ತರ);
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ_List(p->p_ಕ್ರಮ_By, pn_ಎತ್ತರ);
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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ(ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪೂರ್ಣಾಂಕ n_ಎತ್ತರ = p->p_ಎಡ ? p->p_ಎಡ->n_ಎತ್ತರ : 0;
  ಆದರೆ( NEVER(p->p_ಬಲ) && p->p_ಬಲ->n_ಎತ್ತರ>n_ಎತ್ತರ ){
    n_ಎತ್ತರ = p->p_ಬಲ->n_ಎತ್ತರ;
  }
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
    ಎತ್ತರ_ದ_Select(p->x.pSelect, &n_ಎತ್ತರ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->x.pList ){
    ಎತ್ತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ_List(p->x.pList, &n_ಎತ್ತರ);
    p->flags |= EP_Propagate & sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(p->x.pList);
  }
  p->n_ಎತ್ತರ = n_ಎತ್ತರ + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
**
** Also propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ(p);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(pParse, p->n_ಎತ್ತರ);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_ಎತ್ತರ(ಸ್ಥಿರಾಂಕ Select *p){
  ಪೂರ್ಣಾಂಕ n_ಎತ್ತರ = 0;
  ಎತ್ತರ_ದ_Select(p, &n_ಎತ್ತರ);
  ಹಿಂತಿರುಗಿ n_ಎತ್ತರ;
}
#ಇಲ್ಲದಿದ್ದರೆ /* ABOVE:  Height enforcement enabled.  BELOW: Height enforcement off */
/*
** Propagate all EP_Propagate flags from the Expr.x.pList into
** Expr.flags.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p && ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList ){
    p->flags |= EP_Propagate & sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(p->x.pList);
  }
}
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ(y)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** Set the error offset for an Expr node, if possible.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Error_ಸ್ಥಾನಾಂತರ(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ iOfst){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( NEVER(ಅಭಿವ್ಯಕ್ತಿ_Use_WJoin(p_ಅಭಿವ್ಯಕ್ತಿ)) ) ಹಿಂತಿರುಗಿ;
  p_ಅಭಿವ್ಯಕ್ತಿ->w.iOfst = iOfst;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocRawNN() */
  ಪೂರ್ಣಾಂಕ op,                 /* Expression opcode */
  ಸ್ಥಿರಾಂಕ Token *pToken,    /* Token argument.  Might be NULL */
  ಪೂರ್ಣಾಂಕ dequote             /* True to dequote */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  ಪೂರ್ಣಾಂಕ nExtra = pToken ? pToken->n+1 : 0;

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  pNew = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ)+nExtra);
  ಆದರೆ( pNew ){
    memset(pNew, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    ಆದರೆ( nExtra ){
      ಪ್ರತಿಜ್ಞಾ( pToken!=0 );
      pNew->u.zToken = (ಅಕ್ಷರ*)&pNew[1];
      ಪ್ರತಿಜ್ಞಾ( pToken->z!=0 || pToken->n==0 );
      ಆದರೆ( pToken->n ) ಸ್ಮರಣೆ_ನಕಲು(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      ಆದರೆ( dequote && sqlite3Isquote(pNew->u.zToken[0]) ){
        sqlite3_Dequote_ಅಭಿವ್ಯಕ್ತಿ(pNew);
      }
    }
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    pNew->n_ಎತ್ತರ = 1;
#ಆದರೆ_ಅಂತ್ಯ 
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  ಪೂರ್ಣಾಂಕ op,                 /* Expression opcode */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, op, &x, 0);
}

/*
** Allocate an expression for a 32-bit signed integer literal.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(sqlite3 *db, ಪೂರ್ಣಾಂಕ iVal){
  ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
  ಆದರೆ( pNew ){
    memset(pNew, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    pNew->op = TK_INTEGER;
    pNew->iAgg = -1;
    pNew->flags = EP_Int_ಮೌಲ್ಯ|EP_ಎಲೆ|(iVal?EP_IsTrue:EP_IsFalse);
    pNew->u.i_ಮೌಲ್ಯ = iVal;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    pNew->n_ಎತ್ತರ = 1;
#ಆದರೆ_ಅಂತ್ಯ 
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Attach_Subtrees(
  sqlite3 *db,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬೇರು,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ
){
  ಆದರೆ( p_ಬೇರು==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಎಡ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಬಲ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಬೇರು) );
    ಪ್ರತಿಜ್ಞಾ( p_ಬೇರು->x.pSelect==0 );
    ಆದರೆ( p_ಬಲ ){
      p_ಬೇರು->p_ಬಲ = p_ಬಲ;
      p_ಬೇರು->flags |= EP_Propagate & p_ಬಲ->flags;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
      p_ಬೇರು->n_ಎತ್ತರ = p_ಬಲ->n_ಎತ್ತರ+1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಬೇರು->n_ಎತ್ತರ = 1;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ( p_ಎಡ ){
      p_ಬೇರು->p_ಎಡ = p_ಎಡ;
      p_ಬೇರು->flags |= EP_Propagate & p_ಎಡ->flags;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
      ಆದರೆ( p_ಎಡ->n_ಎತ್ತರ>=p_ಬೇರು->n_ಎತ್ತರ ){
        p_ಬೇರು->n_ಎತ್ತರ = p_ಎಡ->n_ಎತ್ತರ+1;
      }
#ಆದರೆ_ಅಂತ್ಯ
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  ಪೂರ್ಣಾಂಕ op,                 /* Expression opcode */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ,            /* Left operand */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ            /* Right operand */
){
  ಅಭಿವ್ಯಕ್ತಿ *p;
  p = sqlite3DbMallocRawNN(pParse->db, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
  ಆದರೆ( p ){
    memset(p, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Attach_Subtrees(pParse->db, p, p_ಎಡ, p_ಬಲ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(pParse, p->n_ಎತ್ತರ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಎಡ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಬಲ);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Add pSelect to the Expr.x.pSelect field.  Or, if pExpr is NULL (due
** do a memory allocation failure) then delete the pSelect object.
*/
ಶೂನ್ಯ sqlite3_PExpr_ಕೂಡಿಸು_Select(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, Select *pSelect){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect = pSelect;
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_Subquery);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
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
Select *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಮ್ಯ_ಮೌಲ್ಯಗಳು(Parse *pParse, ಪೂರ್ಣಾಂಕ nElem, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಪೂರ್ಣಾಂಕ ii;
  Select *pRet = 0;
  ಪ್ರತಿಜ್ಞಾ( nElem>1 );
  ಗಾಗಿ(ii=0; ii<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    Select *pSel;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ_Elem;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸದಿಶ ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      n_ಅಭಿವ್ಯಕ್ತಿ_Elem = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಅಭಿವ್ಯಕ್ತಿ_Elem = 1;
    }
    ಆದರೆ( n_ಅಭಿವ್ಯಕ್ತಿ_Elem!=nElem ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "IN(...) element has %d term%s - expected %d",
          n_ಅಭಿವ್ಯಕ್ತಿ_Elem, n_ಅಭಿವ್ಯಕ್ತಿ_Elem>1?"s":"", nElem
      );
      ನಿಲ್ಲಿಸು;
    }
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pSel = sqlite3SelectNew(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0, 0, 0, 0, SF_ಮೌಲ್ಯಗಳು,0);
    p_ಅಭಿವ್ಯಕ್ತಿ->x.pList = 0;
    ಆದರೆ( pSel ){
      ಆದರೆ( pRet ){
        pSel->op = TK_ALL;
        pSel->p_ಪೂರ್ವ = pRet;
      }
      pRet = pSel;
    }
  }

  ಆದರೆ( pRet && pRet->p_ಪೂರ್ವ ){
    pRet->selFlags |= SF_Multi_ಮೌಲ್ಯ;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, pEList);
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
**
** If one side or the other of the AND is known to be false, and neither side
** is part of an ON clause, then instead of returning an AND expression,
** just return a constant expression with a value of false.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ, ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ){
  sqlite3 *db = pParse->db;
  ಆದರೆ( p_ಎಡ==0  ){
    ಹಿಂತಿರುಗಿ p_ಬಲ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಬಲ==0 ){
    ಹಿಂತಿರುಗಿ p_ಎಡ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    u32 f = p_ಎಡ->flags | p_ಬಲ->flags;
    ಆದರೆ( (f&(EP_OuterON|EP_InnerON|EP_IsFalse|EP_HasFunc))==EP_IsFalse
     && !ಒಳ_RENAME_OBJECT
    ){
      ಅಭಿವ್ಯಕ್ತಿ *p;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಎಡ);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಬಲ);
      p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 0);
      ಆದರೆ( p ) p->op2 = 1;
      ಹಿಂತಿರುಗಿ p;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ sqlite3PExpr(pParse, TK_AND, p_ಎಡ, p_ಬಲ);
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,      /* Argument list */
  ಸ್ಥಿರಾಂಕ Token *pToken,  /* Name of the function */
  ಪೂರ್ಣಾಂಕ eDistinct         /* SF_Distinct or SF_ALL or 0 */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pToken );
  pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_FUNCTION, pToken, 1);
  ಆದರೆ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList); /* Avoid memory leak when malloc fails */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pNew, EP_InnerON|EP_OuterON) );
  pNew->w.iOfst = (ಪೂರ್ಣಾಂಕ)(pToken->z - pParse->z_ಬಾಲ);
  ಆದರೆ( pList
   && pList->n_ಅಭಿವ್ಯಕ್ತಿ > pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_FUNCTION_ARG]
   && !pParse->nested
  ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pNew, EP_HasFunc);
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pNew) );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(pParse, pNew);
  ಆದರೆ( eDistinct==SF_Distinct ) ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pNew, EP_Distinct);
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Report an error when attempting to use an ORDER BY clause within
** the arguments of a non-aggregate function.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕ್ರಮ_By_Aggregate_Error(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  sqlite3_Error_ಸಂದೇಶ(pParse,
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Function_ಕ್ರಮ_By(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The function call to which ORDER BY is to be added */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By    /* The ORDER BY clause to add */
){
  ಅಭಿವ್ಯಕ್ತಿ *pOB;
  sqlite3 *db = pParse->db;
  ಆದರೆ( NEVER(p_ಕ್ರಮ_By==0) ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==0) ){
    /* Ignore ORDER BY on zero-argument aggregates */
    sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( Is_ಕಿಟಕಿ_Func(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕ್ರಮ_By_Aggregate_Error(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in ORDER BY clause");
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ;
  }

  pOB = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ಕ್ರಮ, 0, 0);
  ಆದರೆ( pOB==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಕ್ರಮ_By);
    ಹಿಂತಿರುಗಿ;
  }
  pOB->x.pList = p_ಕ್ರಮ_By;
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pOB) );
  p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = pOB;
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pOB, EP_ತುಂಬಿದ_ಗಾತ್ರ);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(
  Parse *pParse,         /* Parsing and code generating context */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* The function invocation */
  ಸ್ಥಿರಾಂಕ FuncDef *pDef    /* The function being invoked */
){
  ಪ್ರತಿಜ್ಞಾ( !ಒಳ_RENAME_OBJECT );
  ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & (SQLITE_FUNC_ನೇರ|SQLITE_FUNC_UNSAFE))!=0 );
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) 
   || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL
  ){
    ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_ನೇರ)!=0
     || (pParse->db->flags & SQLITE_TrustedSchema)==0
    ){
      /* Functions prohibited in triggers and views if:
      **     (1) tagged with SQLITE_DIRECTONLY
      **     (2) not tagged with SQLITE_INNOCUOUS (which means it
      **         is tagged with SQLITE_FUNC_UNSAFE) and
      **         SQLITE_DBCONFIG_TRUSTED_SCHEMA is off (meaning
      **         that the schema is possibly tainted).
      */
      sqlite3_Error_ಸಂದೇಶ(pParse, "unsafe use of %#T()", p_ಅಭಿವ್ಯಕ್ತಿ);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ನಿಯೋಜಿಸು_Var_ಸಂಖ್ಯೆ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u32 n){
  sqlite3 *db = pParse->db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  ynVar x;

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ|EP_Reduced|EP_TokenOnly) );
  z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
  ಪ್ರತಿಜ್ಞಾ( z[0]!=0 );
  ಪ್ರತಿಜ್ಞಾ( n==(u32)sqlite3Strlen30(z) );
  ಆದರೆ( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    ಪ್ರತಿಜ್ಞಾ( z[0]=='?' );
    x = (ynVar)(++pParse->nVar);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ do_ಕೂಡಿಸು = 0;
    ಆದರೆ( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      ಪೂರ್ಣಾಂಕ bOk;
      ಆದರೆ( n==2 ){ /*OPTIMIZATION-IF-TRUE*/
        i = z[1]-'0';  /* The common case of ?N for a single digit N */
        bOk = 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      }
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ]-1 );
      testcase( i==db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ] );
      ಆದರೆ( bOk==0 || i<1 || i>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ] ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "variable number must be between ?1 and ?%d",
            db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ]);
        sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
        ಹಿಂತಿರುಗಿ;
      }
      x = (ynVar)i;
      ಆದರೆ( x>pParse->nVar ){
        pParse->nVar = (ಪೂರ್ಣಾಂಕ)x;
        do_ಕೂಡಿಸು = 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x<SQLITE_VNBMC*64 ){
        do_ಕೂಡಿಸು = (pParse->aVnbmc[x>>6] & MASKBIT64(x&63))==0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_VList_ಅಂಕೆ_ಗಮ್ಯ_Name(pParse->pVList, x)==0 ){
        do_ಕೂಡಿಸು = 1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      x = (ynVar)sqlite3_VList_Name_ಗಮ್ಯ_ಅಂಕೆ(pParse->pVList, z, n);
      ಆದರೆ( x==0 ){
        x = (ynVar)(++pParse->nVar);
        do_ಕೂಡಿಸು = 1;
      }
    }
    ಆದರೆ( do_ಕೂಡಿಸು ){
      pParse->pVList = sqlite3_VList_ಕೂಡಿಸು(db, pParse->pVList, z, n, x);
      ಆದರೆ( pParse->pVList!=0 && x<SQLITE_VNBMC*64 ){
        pParse->aVnbmc[x>>6] |= MASKBIT64(x&63);
      }
    }
  }
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = x;
  ಆದರೆ( x>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_VARIABLE_ಸಂಖ್ಯೆ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many SQL variables");
    sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}

/*
** Recursively delete an expression tree.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
ಅಭಿವ್ಯಕ್ತಿ_Delete_Restart:
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_UValue(p) || p->u.i_ಮೌಲ್ಯ>=0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p) || !ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p) );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p) || p->y.pWin!=0 || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( p->op!=TK_FUNCTION || !ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p) );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_ಎಲೆ) && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly) ){
    ಪ್ರತಿಜ್ಞಾ( p->p_ಎಡ==0 );
    ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) || p->x.pSelect==0 );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) || p->x.pList==0 );
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, (EP_TokenOnly|EP_ಎಲೆ)) ){
    /* The Expr.x union is never used at the same time as Expr.pRight */
    ಪ್ರತಿಜ್ಞಾ( (ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) && p->x.pList==0) || p->p_ಬಲ==0 );
    ಆದರೆ( p->p_ಬಲ ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p->p_ಬಲ);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) );
      sqlite3SelectDelete(db, p->x.pSelect);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->x.pList);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) ){
        sqlite3_ಕಿಟಕಿ_Delete(db, p->y.pWin);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ( p->p_ಎಡ && p->op!=TK_SELECT_ಸ್ತಂಭ ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p->p_ಎಡ;
      ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Static)
       && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಎಡ, EP_Static)
      ){
        /* Avoid unnecessary recursion on unary operators */
        sqlite3DbNNFreeNN(db, p);
        p = p_ಎಡ;
        ಹೋಗು ಅಭಿವ್ಯಕ್ತಿ_Delete_Restart;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p_ಎಡ);
      }
    }
  }
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Static) ){
    sqlite3DbNNFreeNN(db, p);
  }
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( p ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_Generic(sqlite3 *db, ಶೂನ್ಯ *p){
  ಆದರೆ( ALWAYS(p) ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, (ಅಭಿವ್ಯಕ್ತಿ*)p);
}

/*
** Clear both elements of an OnOrUsing object
*/
ಶೂನ್ಯ sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p){
  ಆದರೆ( p==0 ){
    /* Nothing to clear */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->pOn ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(db, p->pOn);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->pUsing ){
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಹಿಂತಿರುಗಿ 0==sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_Generic, p_ಅಭಿವ್ಯಕ್ತಿ);
}

/* Invoke sqlite3RenameExprUnmap() and sqlite3ExprDelete() on the
** expression.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Unmap_And_Delete(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( p ){
    ಆದರೆ( ಒಳ_RENAME_OBJECT ){
      sqlite3_Rename_ಅಭಿವ್ಯಕ್ತಿ_Unmap(pParse, p);
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_NN(pParse->db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Struct_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly) ) ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_TOKENONLYSIZE;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Reduced) ) ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_REDUCEDSIZE;
  ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ n_ಗಾತ್ರ;
  ಪ್ರತಿಜ್ಞಾ( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE<=0xfff );
  ಪ್ರತಿಜ್ಞಾ( (0xfff & (EP_Reduced|EP_TokenOnly))==0 );
  ಆದರೆ( 0==flags || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_ತುಂಬಿದ_ಗಾತ್ರ) ){
    n_ಗಾತ್ರ = ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_OuterON) );
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p, EP_NoReduce) );
    ಆದರೆ( p->p_ಎಡ || p->x.pList ){
      n_ಗಾತ್ರ = ಅಭಿವ್ಯಕ್ತಿ_REDUCEDSIZE | EP_Reduced;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 );
      n_ಗಾತ್ರ = ಅಭಿವ್ಯಕ್ತಿ_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  ಹಿಂತಿರುಗಿ n_ಗಾತ್ರ;
}

/*
** This function returns the space in bytes required to store the copy
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ nByte = duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_ಗಾತ್ರ(p, flags) & 0xfff;
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
    nByte += sqlite3Strlen30NN(p->u.zToken)+1;
  }
  ಹಿಂತಿರುಗಿ ROUND8(nByte);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p){
  ಪೂರ್ಣಾಂಕ nByte;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  nByte = duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_ಗಾತ್ರ(p, EXPRDUP_REDUCE);
  ಆದರೆ( p->p_ಎಡ ) nByte += duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಾತ್ರ(p->p_ಎಡ);
  ಆದರೆ( p->p_ಬಲ ) nByte += duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಾತ್ರ(p->p_ಬಲ);
  ಪ್ರತಿಜ್ಞಾ( nByte==ROUND8(nByte) );
  ಹಿಂತಿರುಗಿ nByte;
}

/*
** An EdupBuf is a memory allocation used to stored multiple Expr objects
** together with their Expr.zToken content.  This is used to help implement
** compression while doing sqlite3ExprDup().  The top-level Expr does the
** allocation for itself and many of its decendents, then passes an instance
** of the structure down into exprDup() so that they decendents can have
** access to that memory.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ EdupBuf EdupBuf;
ರಚನೆ EdupBuf {
  u8 *zAlloc;          /* Memory space available for storage */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 *z_ಕೊನೆ;            /* First byte past the end of memory */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** This function is similar to sqlite3ExprDup(), except that if pEdupBuf
** is not NULL then it points to memory that can be used to store a copy
** of the input Expr p together with its p->u.zToken (if any).  pEdupBuf
** is updated with the new buffer tail prior to returning.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *ಅಭಿವ್ಯಕ್ತಿ_Dup(
  sqlite3 *db,          /* Database connection (for memory allocation) */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p,        /* Expr tree to be duplicated */
  ಪೂರ್ಣಾಂಕ dupFlags,         /* EXPRDUP_REDUCE for compression.  0 if not */
  EdupBuf *pEdupBuf     /* Preallocated storage space, or NULL */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;           /* Value to return */
  EdupBuf sEdupBuf;     /* Memory space from which to build Expr object */
  u32 staticFlag;       /* EP_Static if space not obtained from malloc */
  ಪೂರ್ಣಾಂಕ nToken = -1;       /* Space needed for p->u.zToken.  -1 means unknown */

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( dupFlags==0 || dupFlags==EXPRDUP_REDUCE );
  ಪ್ರತಿಜ್ಞಾ( pEdupBuf==0 || dupFlags==EXPRDUP_REDUCE );

  /* Figure out where to write the new Expr structure. */
  ಆದರೆ( pEdupBuf ){
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    sEdupBuf.z_ಕೊನೆ = pEdupBuf->z_ಕೊನೆ;
#ಆದರೆ_ಅಂತ್ಯ
    staticFlag = EP_Static;
    ಪ್ರತಿಜ್ಞಾ( sEdupBuf.zAlloc!=0 );
    ಪ್ರತಿಜ್ಞಾ( dupFlags==EXPRDUP_REDUCE );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ nAlloc;
    ಆದರೆ( dupFlags ){
      nAlloc = duped_ಅಭಿವ್ಯಕ್ತಿ_ಗಾತ್ರ(p);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
      nToken = sqlite3Strlen30NN(p->u.zToken)+1;
      nAlloc = ROUND8(ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE + nToken);
    }ಇಲ್ಲದಿದ್ದರೆ{
      nToken = 0;
      nAlloc = ROUND8(ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE);
    }
    ಪ್ರತಿಜ್ಞಾ( nAlloc==ROUND8(nAlloc) );
    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    sEdupBuf.z_ಕೊನೆ = sEdupBuf.zAlloc ? sEdupBuf.zAlloc+nAlloc : 0;
#ಆದರೆ_ಅಂತ್ಯ
    
    staticFlag = 0;
  }
  pNew = (ಅಭಿವ್ಯಕ್ತಿ *)sEdupBuf.zAlloc;
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pNew) );

  ಆದರೆ( pNew ){
    /* Set nNewSize to the size allocated for the structure pointed to
    ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
    ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
    ** by the copy of the p->u.zToken string (if any).
    */
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ n_Struct_ಗಾತ್ರ = duped_ಅಭಿವ್ಯಕ್ತಿ_Struct_ಗಾತ್ರ(p, dupFlags);
    ಪೂರ್ಣಾಂಕ n_New_ಗಾತ್ರ = n_Struct_ಗಾತ್ರ & 0xfff;
    ಆದರೆ( nToken<0 ){
      ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        nToken = 0;
      }
    }
    ಆದರೆ( dupFlags ){
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)(sEdupBuf.z_ಕೊನೆ - sEdupBuf.zAlloc) >= n_New_ಗಾತ್ರ+nToken );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Reduced)==0 );
      ಸ್ಮರಣೆ_ನಕಲು(sEdupBuf.zAlloc, p, n_New_ಗಾತ್ರ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      u32 n_ಗಾತ್ರ = (u32)ಅಭಿವ್ಯಕ್ತಿ_Struct_ಗಾತ್ರ(p);
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)(sEdupBuf.z_ಕೊನೆ - sEdupBuf.zAlloc) >=
                                                   (ಪೂರ್ಣಾಂಕ)ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE+nToken );
      ಸ್ಮರಣೆ_ನಕಲು(sEdupBuf.zAlloc, p, n_ಗಾತ್ರ);
      ಆದರೆ( n_ಗಾತ್ರ<ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE ){
        memset(&sEdupBuf.zAlloc[n_ಗಾತ್ರ], 0, ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE-n_ಗಾತ್ರ);
      }
      n_New_ಗಾತ್ರ = ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE;
    }

    /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
    pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
    pNew->flags |= n_Struct_ಗಾತ್ರ & (EP_Reduced|EP_TokenOnly);
    pNew->flags |= staticFlag;
    ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(pNew);
    ಆದರೆ( dupFlags ){
      ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(pNew, EP_Immutable);
    }

    /* Copy the p->u.zToken string, if any. */
    ಪ್ರತಿಜ್ಞಾ( nToken>=0 );
    ಆದರೆ( nToken>0 ){
      ಅಕ್ಷರ *zToken = pNew->u.zToken = (ಅಕ್ಷರ*)&sEdupBuf.zAlloc[n_New_ಗಾತ್ರ];
      ಸ್ಮರಣೆ_ನಕಲು(zToken, p->u.zToken, nToken);
      n_New_ಗಾತ್ರ += nToken;
    }
    sEdupBuf.zAlloc += ROUND8(n_New_ಗಾತ್ರ);

    ಆದರೆ( ((p->flags|pNew->flags)&(EP_TokenOnly|EP_ಎಲೆ))==0 ){

      /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pNew->x.pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->x.pList,
                           p->op!=TK_ಕ್ರಮ ? dupFlags : 0);
      }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_WinFunc) ){
        pNew->y.pWin = sqlite3_ಕಿಟಕಿ_Dup(db, pNew, p->y.pWin);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pNew, EP_WinFunc) );
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

      /* Fill in pNew->pLeft and pNew->pRight. */
      ಆದರೆ( dupFlags ){
        ಆದರೆ( p->op==TK_SELECT_ಸ್ತಂಭ ){
          pNew->p_ಎಡ = p->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 
               || p->p_ಬಲ==p->p_ಎಡ
               || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->p_ಎಡ, EP_Subquery) );
        }ಇಲ್ಲದಿದ್ದರೆ{
          pNew->p_ಎಡ = p->p_ಎಡ ?
                      ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಡ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
        }
        pNew->p_ಬಲ = p->p_ಬಲ ?
                       ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಬಲ, EXPRDUP_REDUCE, &sEdupBuf) : 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( p->op==TK_SELECT_ಸ್ತಂಭ ){
          pNew->p_ಎಡ = p->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( p->p_ಬಲ==0 
               || p->p_ಬಲ==p->p_ಎಡ
               || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p->p_ಎಡ, EP_Subquery) );
        }ಇಲ್ಲದಿದ್ದರೆ{
          pNew->p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಡ, 0);
        }
        pNew->p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಬಲ, 0);
      }
    }
  }
  ಆದರೆ( pEdupBuf ) ಸ್ಮರಣೆ_ನಕಲು(pEdupBuf, &sEdupBuf, ಗಾತ್ರ(sEdupBuf));
  ಪ್ರತಿಜ್ಞಾ( sEdupBuf.zAlloc <= sEdupBuf.z_ಕೊನೆ );
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Create and return a deep copy of the object passed as the second
** argument. If an OOM condition is encountered, NULL is returned
** and the db->mallocFailed flag set.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
With *sqlite3WithDup(sqlite3 *db, With *p){
  With *pRet = 0;
  ಆದರೆ( p ){
    sqlite3_int64 nByte = SZ_WITH(p->nCte);
    pRet = sqlite3DbMallocZero(db, nByte);
    ಆದರೆ( pRet ){
      ಪೂರ್ಣಾಂಕ i;
      pRet->nCte = p->nCte;
      ಗಾಗಿ(i=0; i<p->nCte; i++){
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->a[i].p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  ಹಿಂತಿರುಗಿ pRet;
}
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3WithDup(x,y) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** The gatherSelectWindows() procedure and its helper routine
** gatherSelectWindowsCallback() are used to scan all the expressions
** an a newly duplicated SELECT statement and gather all of the Window
** objects found there, assembling them onto the linked list at Select->pWin.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gatherSelectWindowsCallback(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    Select *pSelect = pWalker->u.pSelect;
    ಕಿಟಕಿ *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
    ಪ್ರತಿಜ್ಞಾ( pWin );
    ಪ್ರತಿಜ್ಞಾ( Is_ಕಿಟಕಿ_Func(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತಿಜ್ಞಾ( pWin->ppThis==0 );
    sqlite3_ಕಿಟಕಿ_Link(pSelect, pWin);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p){
  ಹಿಂತಿರುಗಿ p==pWalker->u.pSelect ? WRC_Continue : WRC_Prune;
}
ಸ್ಥಿರ ಶೂನ್ಯ gatherSelectWindows(Select *p){
  Walker w;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}
#ಆದರೆ_ಅಂತ್ಯ


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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ flags){
  ಪ್ರತಿಜ್ಞಾ( flags==0 || flags==EXPRDUP_REDUCE );
  ಹಿಂತಿರುಗಿ p ? ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p, flags, 0) : 0;
}
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಂಕ flags){
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಸ್ಥಿರಾಂಕ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_Old_ಅಂಶ;
  ಪೂರ್ಣಾಂಕ i;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_Old = 0;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_New = 0;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3_Db_Malloc_ಗಾತ್ರ(db, p));
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew->n_ಅಭಿವ್ಯಕ್ತಿ = p->n_ಅಭಿವ್ಯಕ್ತಿ;
  pNew->nAlloc = p->nAlloc;
  p_ಅಂಶ = pNew->a;
  p_Old_ಅಂಶ = p->a;
  ಗಾಗಿ(i=0; i<p->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++, p_Old_ಅಂಶ++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Old_ಅಭಿವ್ಯಕ್ತಿ = p_Old_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *p_New_ಅಭಿವ್ಯಕ್ತಿ;
    p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Old_ಅಭಿವ್ಯಕ್ತಿ, flags);
    ಆದರೆ( p_Old_ಅಭಿವ್ಯಕ್ತಿ
     && p_Old_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT_ಸ್ತಂಭ
     && (p_New_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ)!=0
    ){
      ಆದರೆ( p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
        p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_Old = p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_New = p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_Old ){
          p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_Old = p_Old_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_New = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_Old, flags);
          p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_New;
        }
        p_New_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = p_ಪೂರ್ವ_Select_ಲಂಬಸಾಲು_New;
      }
    }
    p_ಅಂಶ->zEName = sqlite3DbStrDup(db, p_Old_ಅಂಶ->zEName);
    p_ಅಂಶ->fg = p_Old_ಅಂಶ->fg;
    p_ಅಂಶ->u = p_Old_ಅಂಶ->u;
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ SrcList *p, ಪೂರ್ಣಾಂಕ flags){
  SrcList *pNew;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_SRCLIST(p->nSrc) );
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  ಗಾಗಿ(i=0; i<p->nSrc; i++){
    Src_ಅಂಶ *p_New_ಅಂಶ = &pNew->a[i];
    ಸ್ಥಿರಾಂಕ Src_ಅಂಶ *p_Old_ಅಂಶ = &p->a[i];
    ಕೋಷ್ಟಕ *pTab;
    p_New_ಅಂಶ->fg = p_Old_ಅಂಶ->fg;
    ಆದರೆ( p_Old_ಅಂಶ->fg.isSubquery ){
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, ಗಾತ್ರ(Subquery));
      ಆದರೆ( pNewSubq==0 ){
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed );
        p_New_ಅಂಶ->fg.isSubquery = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸ್ಮರಣೆ_ನಕಲು(pNewSubq, p_Old_ಅಂಶ->u4.pSubq, ಗಾತ್ರ(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        ಆದರೆ( pNewSubq->pSelect==0 ){
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          p_New_ಅಂಶ->fg.isSubquery = 0;
        }
      }
      p_New_ಅಂಶ->u4.pSubq = pNewSubq;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_Old_ಅಂಶ->fg.fixedSchema ){
      p_New_ಅಂಶ->u4.pSchema = p_Old_ಅಂಶ->u4.pSchema;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_New_ಅಂಶ->u4.zDatabase = sqlite3DbStrDup(db, p_Old_ಅಂಶ->u4.zDatabase);
    }
    p_New_ಅಂಶ->zName = sqlite3DbStrDup(db, p_Old_ಅಂಶ->zName);
    p_New_ಅಂಶ->zAlias = sqlite3DbStrDup(db, p_Old_ಅಂಶ->zAlias);
    p_New_ಅಂಶ->iCursor = p_Old_ಅಂಶ->iCursor;
    ಆದರೆ( p_New_ಅಂಶ->fg.is_ಸೂಚ್ಯಂಕಿತ_By ){
      p_New_ಅಂಶ->u1.z_ಸೂಚ್ಯಂಕಿತ_By = sqlite3DbStrDup(db, p_Old_ಅಂಶ->u1.z_ಸೂಚ್ಯಂಕಿತ_By);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_New_ಅಂಶ->fg.isTabFunc ){
      p_New_ಅಂಶ->u1.pFuncArg =
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p_Old_ಅಂಶ->u1.pFuncArg, flags);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_New_ಅಂಶ->u1.n_ಅಡ್ಡಸಾಲು = p_Old_ಅಂಶ->u1.n_ಅಡ್ಡಸಾಲು;
    }
    p_New_ಅಂಶ->u2 = p_Old_ಅಂಶ->u2;
    ಆದರೆ( p_New_ಅಂಶ->fg.isCte ){
      p_New_ಅಂಶ->u2.pCteUse->nUse++;
    }
    pTab = p_New_ಅಂಶ->pSTab = p_Old_ಅಂಶ->pSTab;
    ಆದರೆ( pTab ){
      pTab->nTabRef++;
    }
    ಆದರೆ( p_Old_ಅಂಶ->fg.isUsing ){
      ಪ್ರತಿಜ್ಞಾ( p_New_ಅಂಶ->fg.isUsing );
      p_New_ಅಂಶ->u3.pUsing = sqlite3IdListDup(db, p_Old_ಅಂಶ->u3.pUsing);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_New_ಅಂಶ->u3.pOn = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Old_ಅಂಶ->u3.pOn, flags);
    }
    p_New_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ = p_Old_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ;
  }
  ಹಿಂತಿರುಗಿ pNew;
}
IdList *sqlite3IdListDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ IdList *p){
  IdList *pNew;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew = sqlite3DbMallocRawNN(db, SZ_IDLIST(p->nId));
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ 0;
  pNew->nId = p->nId;
  ಗಾಗಿ(i=0; i<p->nId; i++){
    ರಚನೆ Id_List_ಅಂಶ *p_New_ಅಂಶ = &pNew->a[i];
    ಸ್ಥಿರಾಂಕ ರಚನೆ Id_List_ಅಂಶ *p_Old_ಅಂಶ = &p->a[i];
    p_New_ಅಂಶ->zName = sqlite3DbStrDup(db, p_Old_ಅಂಶ->zName);
  }
  ಹಿಂತಿರುಗಿ pNew;
}
Select *sqlite3SelectDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ Select *pDup, ಪೂರ್ಣಾಂಕ flags){
  Select *pRet = 0;
  Select *p_ಮುಂದಿನ = 0;
  Select **pp = &pRet;
  ಸ್ಥಿರಾಂಕ Select *p;

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಗಾಗಿ(p=pDup; p; p=p->p_ಪೂರ್ವ){
    Select *pNew = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(*p) );
    ಆದರೆ( pNew==0 ) ನಿಲ್ಲಿಸು;
    pNew->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pWhere, flags);
    pNew->p_ಗುಂಪು_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->p_ಗುಂಪು_By, flags);
    pNew->pHaving = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pHaving, flags);
    pNew->p_ಕ್ರಮ_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p->p_ಕ್ರಮ_By, flags);
    pNew->op = p->op;
    pNew->p_ಮುಂದಿನ = p_ಮುಂದಿನ;
    pNew->p_ಪೂರ್ವ = 0;
    pNew->p_ಎಲ್ಲೆ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಲ್ಲೆ, flags);
    pNew->i_ಎಲ್ಲೆ = 0;
    pNew->i_ಸ್ಥಾನಾಂತರ = 0;
    pNew->selFlags = p->selFlags;
    pNew->n_Select_ಅಡ್ಡಸಾಲು = p->n_Select_ಅಡ್ಡಸಾಲು;
    pNew->pWith = sqlite3WithDup(db, p->pWith);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3_ಕಿಟಕಿ_List_Dup(db, p->pWinDefn);
    ಆದರೆ( p->pWin && db->mallocFailed==0 ) gatherSelectWindows(pNew);
#ಆದರೆ_ಅಂತ್ಯ
    pNew->selId = p->selId;
    ಆದರೆ( db->mallocFailed ){
      /* Any prior OOM might have left the Select object incomplete.
      ** Delete the whole thing rather than allow an incomplete Select
      ** to be used by the code generator. */
      pNew->p_ಮುಂದಿನ = 0;
      sqlite3SelectDelete(db, pNew);
      ನಿಲ್ಲಿಸು;
    }
    *pp = pNew;
    pp = &pNew->p_ಪೂರ್ವ;
    p_ಮುಂದಿನ = pNew;
  }
  ಹಿಂತಿರುಗಿ pRet;
}
#ಇಲ್ಲದಿದ್ದರೆ
Select *sqlite3SelectDup(sqlite3 *db, ಸ್ಥಿರಾಂಕ Select *p, ಪೂರ್ಣಾಂಕ flags){
  ಪ್ರತಿಜ್ಞಾ( p==0 );
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ


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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ zero_ಅಂಶ = {0};
SQLITE_NOINLINE ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_New(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;

  pList = sqlite3DbMallocRawNN(db, SZ_EXPRLIST(4));
  ಆದರೆ( pList==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಹಿಂತಿರುಗಿ 0;
  }
  pList->nAlloc = 4;
  pList->n_ಅಭಿವ್ಯಕ್ತಿ = 1;
  p_ಅಂಶ = &pList->a[0];
  *p_ಅಂಶ = zero_ಅಂಶ;
  p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಹಿಂತಿರುಗಿ pList;
}
SQLITE_NOINLINE ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_Grow(
  sqlite3 *db,            /* Database handle.  Used for memory allocation */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to append. Might be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಅಭಿವ್ಯಕ್ತಿ_List *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, SZ_EXPRLIST(pList->nAlloc));
  ಆದರೆ( pNew==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pList);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pList = pNew;
  }
  p_ಅಂಶ = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ++];
  *p_ಅಂಶ = zero_ಅಂಶ;
  p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಹಿಂತಿರುಗಿ pList;
}
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to append. Might be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* Expression to be appended. Might be NULL */
){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಆದರೆ( pList==0 ){
    ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_New(pParse->db,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಆದರೆ( pList->nAlloc<pList->n_ಅಭಿವ್ಯಕ್ತಿ+1 ){
    ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_Grow(pParse->db,pList,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  p_ಅಂಶ = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ++];
  *p_ಅಂಶ = zero_ಅಂಶ;
  p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಹಿಂತಿರುಗಿ pList;
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
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_ಸದಿಶ(
  Parse *pParse,         /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,       /* List to which to append. Might be NULL */
  IdList *pColumns,      /* List of names of LHS of the assignment */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ            /* Vector expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ n;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  /* pColumns can only be NULL due to an OOM but an OOM will cause an
  ** exit prior to this routine being invoked */
  ಆದರೆ( NEVER(pColumns==0) ) ಹೋಗು ಸದಿಶ_append_error;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹೋಗು ಸದಿಶ_append_error;

  /* If the RHS is a vector, then we can immediately check to see that
  ** the size of the RHS and LHS match.  But if the RHS is a SELECT,
  ** wildcards ("*") in the result set of the SELECT must be expanded before
  ** we can do the size check, so defer the size check until code generation.
  */
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_SELECT && pColumns->nId!=(n=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ)) ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "%d columns assigned %d values",
                    pColumns->nId, n);
    ಹೋಗು ಸದಿಶ_append_error;
  }

  ಗಾಗಿ(i=0; i<pColumns->nId; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_Sub_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_ಸದಿಶ_ಕ್ಷೇತ್ರ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, i, pColumns->nId);
    ಪ್ರತಿಜ್ಞಾ( p_Sub_ಅಭಿವ್ಯಕ್ತಿ!=0 || db->mallocFailed );
    ಆದರೆ( p_Sub_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಮುಂದುವರಿ;
    pList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pList, p_Sub_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( pList ){
      ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==i_ಮೊದಲ+i+1 );
      pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  ಆದರೆ( !db->mallocFailed && p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT && ALWAYS(pList!=0) ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಮೊದಲ = pList->a[i_ಮೊದಲ].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( p_ಮೊದಲ!=0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಮೊದಲ->op==TK_SELECT_ಸ್ತಂಭ );
    
    /* Store the SELECT statement in pRight so it will be deleted when
    ** sqlite3ExprListDelete() is called */
    p_ಮೊದಲ->p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ;
    p_ಅಭಿವ್ಯಕ್ತಿ = 0;

    /* Remember the size of the LHS in iTable so that we can check that
    ** the RHS and LHS sizes match during code generation. */
    p_ಮೊದಲ->i_ಕೋಷ್ಟಕ = pColumns->nId;
  }

ಸದಿಶ_append_error:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Unmap_And_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  sqlite3IdListDelete(db, pColumns);
  ಹಿಂತಿರುಗಿ pList;
}

/*
** Set the sort order for the last element on the given ExprList.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_ವಿಂಗಡಿಸು_ಕ್ರಮ(ಅಭಿವ್ಯಕ್ತಿ_List *p, ಪೂರ್ಣಾಂಕ i_ವಿಂಗಡಿಸು_ಕ್ರಮ, ಪೂರ್ಣಾಂಕ eNulls){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( p->n_ಅಭಿವ್ಯಕ್ತಿ>0 );

  ಪ್ರತಿಜ್ಞಾ( SQLITE_SO_UNDEFINED<0 && SQLITE_SO_ASC==0 && SQLITE_SO_DESC>0 );
  ಪ್ರತಿಜ್ಞಾ( i_ವಿಂಗಡಿಸು_ಕ್ರಮ==SQLITE_SO_UNDEFINED
       || i_ವಿಂಗಡಿಸು_ಕ್ರಮ==SQLITE_SO_ASC
       || i_ವಿಂಗಡಿಸು_ಕ್ರಮ==SQLITE_SO_DESC
  );
  ಪ್ರತಿಜ್ಞಾ( eNulls==SQLITE_SO_UNDEFINED
       || eNulls==SQLITE_SO_ASC
       || eNulls==SQLITE_SO_DESC
  );

  p_ಅಂಶ = &p->a[p->n_ಅಭಿವ್ಯಕ್ತಿ-1];
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.bNulls==0 );
  ಆದರೆ( i_ವಿಂಗಡಿಸು_ಕ್ರಮ==SQLITE_SO_UNDEFINED ){
    i_ವಿಂಗಡಿಸು_ಕ್ರಮ = SQLITE_SO_ASC;
  }
  p_ಅಂಶ->fg.ವಿಂಗಡಿಸು_Flags = (u8)i_ವಿಂಗಡಿಸು_ಕ್ರಮ;

  ಆದರೆ( eNulls!=SQLITE_SO_UNDEFINED ){
    p_ಅಂಶ->fg.bNulls = 1;
    ಆದರೆ( i_ವಿಂಗಡಿಸು_ಕ್ರಮ!=eNulls ){
      p_ಅಂಶ->fg.ವಿಂಗಡಿಸು_Flags |= KEYINFO_ಕ್ರಮ_BIGNULL;
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_Name(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to add the span. */
  ಸ್ಥಿರಾಂಕ Token *pName,     /* Name to be added */
  ಪೂರ್ಣಾಂಕ dequote             /* True to cause the name to be dequoted */
){
  ಪ್ರತಿಜ್ಞಾ( pList!=0 || pParse->db->mallocFailed!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->e_Parse_ಬಹುಲಕ!=PARSE_ಬಹುಲಕ_UNMAP || dequote==0 );
  ಆದರೆ( pList ){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
    p_ಅಂಶ = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1];
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->zEName==0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.eEName==ENAME_NAME );
    p_ಅಂಶ->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    ಆದರೆ( dequote ){
      /* If dequote==0, then pName->z does not point to part of a DDL
      ** statement handled by the parser. And so no token need be added
      ** to the token-map.  */
      sqlite3Dequote(p_ಅಂಶ->zEName);
      ಆದರೆ( ಒಳ_RENAME_OBJECT ){
        sqlite3RenameTokenMap(pParse, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)p_ಅಂಶ->zEName, pName);
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_ಹರವು(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,        /* List to which to add the span. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ,     /* Start of the span */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ        /* End of the span */
){
  sqlite3 *db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 || db->mallocFailed!=0 );
  ಆದರೆ( pList ){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &pList->a[pList->n_ಅಭಿವ್ಯಕ್ತಿ-1];
    ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
    ಆದರೆ( p_ಅಂಶ->zEName==0 ){
      p_ಅಂಶ->zEName = sqlite3_Db_ಹರವು_Dup(db, z_ಆರಂಭ, z_ಕೊನೆ);
      p_ಅಂಶ->fg.eEName = ENAME_ಹರವು;
    }
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Check_ಉದ್ದಳತೆ(
  Parse *pParse,
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zObject
){
  ಪೂರ್ಣಾಂಕ mx = pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ];
  testcase( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ==mx );
  testcase( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ==mx+1 );
  ಆದರೆ( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ>mx ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಂಕ i = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ =  pList->a;
  ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಮಾಡು{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( p_ಅಂಶ->zEName ) sqlite3DbNNFreeNN(db, p_ಅಂಶ->zEName);
    p_ಅಂಶ++;
  }ಆಗ( --i>0 );
  sqlite3DbNNFreeNN(db, pList);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಆದರೆ( pList ) ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(db, pList);
}
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic(sqlite3 *db, ಶೂನ್ಯ *pList){
  ಆದರೆ( ALWAYS(pList) ) ಅಭಿವ್ಯಕ್ತಿ_List_Delete_NN(db, (ಅಭಿವ್ಯಕ್ತಿ_List*)pList);
}

/*
** Return the bitwise-OR of all Expr.flags fields in the given
** ExprList.
*/
u32 sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಂಕ i;
  u32 m = 0;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
     ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
     ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
     m |= p_ಅಭಿವ್ಯಕ್ತಿ->flags;
  }
  ಹಿಂತಿರುಗಿ m;
}

/*
** This is a SELECT-node callback for the expression walker that
** always "fails".  By "fail" in this case, we mean set
** pWalker->eCode to zero and abort.
**
** This callback is used by multiple expression walkers.
*/
ಪೂರ್ಣಾಂಕ sqlite3SelectWalkFail(Walker *pWalker, Select *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  pWalker->e_ಸಂಕೇತ = 0;
  ಹಿಂತಿರುಗಿ WRC_Abort;
}

/*
** Check the input string to see if it is "true" or "false" (in any case).
**
**       If the string is....           Return
**         "true"                         EP_IsTrue
**         "false"                        EP_IsFalse
**         anything else                  0
*/
u32 sqlite3IsTrueOrFalse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ){
  ಆದರೆ( sqlite3StrICmp(z_ಒಳ, "true")==0  ) ಹಿಂತಿರುಗಿ EP_IsTrue;
  ಆದರೆ( sqlite3StrICmp(z_ಒಳ, "false")==0 ) ಹಿಂತಿರುಗಿ EP_IsFalse;
  ಹಿಂತಿರುಗಿ 0;
}


/*
** If the input expression is an ID with the name "true" or "false"
** then convert it into an TK_TRUEFALSE term.  Return non-zero if
** the conversion happened, and zero if the expression is unaltered.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_ಗಮ್ಯ_True_False(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  u32 v;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_STRING );
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Quoted|EP_Int_ಮೌಲ್ಯ)
   && (v = sqlite3IsTrueOrFalse(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken))!=0
  ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUEFALSE;
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, v);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** The argument must be a TK_TRUEFALSE Expr node.  Return 1 if it is TRUE
** and 0 if it is FALSE.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely((ಅಭಿವ್ಯಕ್ತಿ*)p_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUEFALSE );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"true")==0
       || sqlite3StrICmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"false")==0 );
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[4]==0;
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
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಎಡ) || ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಬಲ) ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ? p_ಬಲ : p_ಎಡ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಬಲ) || ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಎಡ) ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ? p_ಎಡ : p_ಬಲ;
    }
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}

/*
** Return true if it might be advantageous to compute the right operand
** of expression pExpr first, before the left operand.
**
** Normally the left operand is computed before the right operand.  But if
** the left operand contains a subquery and the right does not, then it
** might be more efficient to compute the right operand first.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_ಮೊದಲ(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, EP_Subquery)
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery)
  ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,       /* The comparison expression */
  ಪೂರ್ಣಾಂಕ *pR1,          /* OUT: Register holding the left operand */
  ಪೂರ್ಣಾಂಕ *pR2,          /* OUT: Register holding the right operand */
  ಪೂರ್ಣಾಂಕ *pFree1,       /* OUT: Temp register to free if not zero */
  ಪೂರ್ಣಾಂಕ *pFree2        /* OUT: Another temp register to free if not zero */
){
  ಪೂರ್ಣಾಂಕ addrIsNull;
  ಪೂರ್ಣಾಂಕ r1, r2;
  Vdbe *v = pParse->pVdbe;

  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  /*
  ** If the left operand contains a (possibly expensive) subquery and the
  ** right operand does not and the right operation might be NULL,
  ** then compute the right operand first and do an IsNull jump if the
  ** right operand evalutes to NULL.
  */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_ಮೊದಲ(p_ಅಭಿವ್ಯಕ್ತಿ) && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ) ){
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pFree2);
    addrIsNull = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v);
  }ಇಲ್ಲದಿದ್ದರೆ{
    r2 = 0; /* Silence a false-positive uninit-var warning in MSVC */
    addrIsNull = 0;
  }
  r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, pFree1);
  ಆದರೆ( addrIsNull==0 ){
    /*
    ** If the right operand contains a subquery and the left operand does not
    ** and the left operand might be NULL, then do an IsNull check
    ** check on the left operand before computing the right operand.
    */
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery)
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)
    ){
      addrIsNull = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  ಹಿಂತಿರುಗಿ addrIsNull;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Function(
  Walker *pWalker,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
){
  ಪೂರ್ಣಾಂಕ n;             /* Number of arguments */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;   /* List of arguments */
  FuncDef *pDef;     /* The function */
  sqlite3 *db;       /* The database */

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly)
   || (pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList)==0
  ){;
    n = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    n = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pList);
    ಆದರೆ( pWalker->e_ಸಂಕೇತ==0 ) ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n, ENC(db), 0);
  ಆದರೆ( pDef==0
   || pDef->xFinalize!=0
   || (pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG))==0
   || NEVER(ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc))
  ){
    pWalker->e_ಸಂಕೇತ = 0;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಹಿಂತಿರುಗಿ WRC_Prune;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತಿಜ್ಞಾ( pWalker->e_ಸಂಕೇತ>0 );

  /* If pWalker->eCode is 2 then any term of the expression that comes from
  ** the ON or USING clauses of an outer join disqualifies the expression
  ** from being considered constant. */
  ಆದರೆ( pWalker->e_ಸಂಕೇತ==2 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
    pWalker->e_ಸಂಕೇತ = 0;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }

  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_FUNCTION:
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
        pWalker->e_ಸಂಕೇತ = 0;  /* Window functions are never constant */
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pWalker->e_ಸಂಕೇತ>=4 && pWalker->e_ಸಂಕೇತ<=5 ){
        /* Functions used within a CREATE TABLE statement are constant as
        ** long as all of their arguments are constant */
        ಆದರೆ( pWalker->e_ಸಂಕೇತ==5 ) ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL);
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ConstFunc) ){
        /* Functions marked with EP_ConstFunc are constant as long as 
        ** all their arugments are constant and they are not window functions */
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pWalker->pParse ){
        /* Functions are constant if they have SQLITE_FUNC_CONSTANT or
        ** SQLITE_FUNC_SLOCHNG and if all arguments are constant */
        ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Function(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pWalker->e_ಸಂಕೇತ = 0;  /* Function call is not constant */
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
    ಸ್ಥಿತಿ TK_ID:
      /* Convert "true" or "false" in a DEFAULT clause into the
      ** appropriate TK_TRUEFALSE operator */
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_ಗಮ್ಯ_True_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಹಿಂತಿರುಗಿ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ:
    ಸ್ಥಿತಿ TK_AGG_FUNCTION:
    ಸ್ಥಿತಿ TK_AGG_ಸ್ತಂಭ:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_ಸ್ತಂಭ );
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು) && pWalker->e_ಸಂಕೇತ!=2 ){
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }
      ಆದರೆ( pWalker->e_ಸಂಕೇತ==3 && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pWalker->u.iCur ){
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_IF_NULL_ಅಡ್ಡಸಾಲು:
    ಸ್ಥಿತಿ TK_ರಿಜಿಸ್ಟರ್:
    ಸ್ಥಿತಿ TK_DOT:
    ಸ್ಥಿತಿ TK_RAISE:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ರಿಜಿಸ್ಟರ್ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_DOT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_RAISE );
      pWalker->e_ಸಂಕೇತ = 0;
      ಹಿಂತಿರುಗಿ WRC_Abort;
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ISNOT:
      ಆದರೆ( pWalker->e_ಸಂಕೇತ==6 ){
        pWalker->e_ಸಂಕೇತ = 0;
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಹಿಂತಿರುಗಿ WRC_Continue;
    ಸ್ಥಿತಿ TK_VARIABLE:
      ಆದರೆ( pWalker->e_ಸಂಕೇತ==5 ){
        /* Silently convert bound parameters that appear inside of CREATE
        ** statements into a NULL when parsing the CREATE statement text out
        ** of the sqlite_schema table */
        p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pWalker->e_ಸಂಕೇತ==4 ){
        /* A bound parameter in a CREATE statement that originates from
        ** sqlite3_prepare() causes an error */
        pWalker->e_ಸಂಕೇತ = 0;
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      /* no break */ deliberate_fall_through
    ಪೂರ್ವನಿಯೋಜಿತ:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ); /* sqlite3SelectWalkFail() disallows */
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS ); /* sqlite3SelectWalkFail() disallows */
      ಹಿಂತಿರುಗಿ WRC_Continue;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ initFlag){
  Walker w;
  w.e_ಸಂಕೇತ = initFlag;
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, p, 1);
}

/*
** Return true if all expressions in pList are constant. If parameter bNoIs
** is true, do not consider expressions that contain "IS" operators to be
** constant. IS operators are not always considered constant as expressions
** like "x IS TRUE" need to be transformed to TK_TRUTH expression nodes,
** which happens at the same time as column name resolution.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *pList, ಪೂರ್ಣಾಂಕ bNoIs){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iInit = bNoIs ? 6 : 1;
  ಪೂರ್ಣಾಂಕ ii;
  ಗಾಗಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    ಆದರೆ( 0==ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ, iInit) ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p){
  ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(pParse, p, 2);
}

/*
** This routine examines sub-SELECT statements as an expression is being
** walked as part of sqlite3ExprIsTableConstant().  Sub-SELECTs are considered
** constant as long as they are uncorrelated - meaning that they do not
** contain any terms from outer contexts.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Select_Walk_ಕೋಷ್ಟಕ_Constant(Walker *pWalker, Select *pSelect){
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( pWalker->e_ಸಂಕೇತ==3 || pWalker->e_ಸಂಕೇತ==0 );
  ಆದರೆ( (pSelect->selFlags & SF_Correlated)!=0 ){
    pWalker->e_ಸಂಕೇತ = 0;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಹಿಂತಿರುಗಿ WRC_Prune;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಕೋಷ್ಟಕ_Constant(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ iCur, ಪೂರ್ಣಾಂಕ bAllowSubq){
  Walker w;
  w.e_ಸಂಕೇತ = 3;
  w.pParse = 0;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant;
  ಆದರೆ( bAllowSubq ){
    w.xSelectCallback = ಅಭಿವ್ಯಕ್ತಿ_Select_Walk_ಕೋಷ್ಟಕ_Constant;
  }ಇಲ್ಲದಿದ್ದರೆ{
    w.xSelectCallback = sqlite3SelectWalkFail;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ಆದರೆ_ಅಂತ್ಯ
  }
  w.u.iCur = iCur;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Single_ಕೋಷ್ಟಕ_Constraint(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,                 /* The constraint */
  ಸ್ಥಿರಾಂಕ SrcList *pSrcList,     /* Complete FROM clause */
  ಪೂರ್ಣಾಂಕ iSrc,                    /* Which element of pSrcList to use */
  ಪೂರ್ಣಾಂಕ bAllowSubq               /* Allow non-correlated subqueries */
){
  ಸ್ಥಿರಾಂಕ Src_ಅಂಶ *pSrc = &pSrcList->a[iSrc];
  ಆದರೆ( pSrc->fg.jointype & JT_LTORJ ){
    ಹಿಂತಿರುಗಿ 0;  /* rule (3) */
  }
  ಆದರೆ( pSrc->fg.jointype & JT_ಎಡ ){
    ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಹಿಂತಿರುಗಿ 0;   /* rule (4a) */
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin!=pSrc->iCursor ) ಹಿಂತಿರುಗಿ 0;         /* rule (4b) */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಹಿಂತಿರುಗಿ 0;    /* rule (5) */
  }
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON)  /* (6a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (6b) */
  ){
    ಪೂರ್ಣಾಂಕ jj;
    ಗಾಗಿ(jj=0; jj<iSrc; jj++){
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin==pSrcList->a[jj].iCursor ){
        ಆದರೆ( (pSrcList->a[jj].fg.jointype & JT_LTORJ)!=0 ){
          ಹಿಂತಿರುಗಿ 0;  /* restriction (6) */
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  /* Rules (1), (2a), and (2b) handled by the following: */
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಕೋಷ್ಟಕ_Constant(p_ಅಭಿವ್ಯಕ್ತಿ, pSrc->iCursor, bAllowSubq);
}


/*
** sqlite3WalkExpr() callback used by sqlite3ExprIsConstantOrGroupBy().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Or_ಗುಂಪು_By(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By = pWalker->u.p_ಗುಂಪು_By;
  ಪೂರ್ಣಾಂಕ i;

  /* Check if pExpr is identical to any GROUP BY term. If so, consider
  ** it constant.  */
  ಗಾಗಿ(i=0; i<p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = p_ಗುಂಪು_By->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, p, -1)<2 ){
      CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pWalker->pParse, p);
      ಆದರೆ( sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
        ಹಿಂತಿರುಗಿ WRC_Prune;
      }
    }
  }

  /* Check if pExpr is a sub-select. If so, consider it variable. */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    pWalker->e_ಸಂಕೇತ = 0;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }

  ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_ಗುಂಪು_By(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By){
  Walker w;
  w.e_ಸಂಕೇತ = 1;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Is_Constant_Or_ಗುಂಪು_By;
  w.xSelectCallback = 0;
  w.u.p_ಗುಂಪು_By = p_ಗುಂಪು_By;
  w.pParse = pParse;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Function(ಅಭಿವ್ಯಕ್ತಿ *p, u8 isInit){
  ಪ್ರತಿಜ್ಞಾ( isInit==0 || isInit==1 );
  ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Is_Const(0, p, 4+isInit);
}

/*
** Walk an expression tree.  Return 1 if the expression contains a
** subquery of some kind.  Return 0 if there are no subqueries.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಒಳಗೊಂಡಿದೆ_Subquery(ಅಭಿವ್ಯಕ್ತಿ *p){
  Walker w;
  w.e_ಸಂಕೇತ = 1;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
  w.xSelectCallback = sqlite3SelectWalkFail;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  w.xSelectCallback2 = sqlite3SelectWalkAssert2;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ==0;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ *p_ಮೌಲ್ಯ, Parse *pParse, ಪೂರ್ಣಾಂಕ bRSI){
  ಪೂರ್ಣಾಂಕ iSign = 1;   /* Either +1 or -1. */
  ಆಗ( 1/*exit-by-break*/ ){
    ಆದರೆ( NEVER(p==0) ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_UValue(p) ){
      ಪ್ರತಿಜ್ಞಾ( p->op==TK_INTEGER );
      *p_ಮೌಲ್ಯ = p->u.i_ಮೌಲ್ಯ*iSign;
      ಆದರೆ( bRSI && pParse==0 && p->op2 ){
        ಹಿಂತಿರುಗಿ 0;
      }
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( p->op==TK_UPLUS ){
      p = p->p_ಎಡ;
      pParse = 0;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( p->op==TK_UMINUS ){
      iSign = -iSign;
      p = p->p_ಎಡ;
      pParse = 0;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( p->op==TK_VARIABLE && pParse!=0 ){
      sqlite3_ಮೌಲ್ಯ *pVal; /* The variable */
      ಪೂರ್ಣಾಂಕ rc = 0;          /* 1 if successful, 0 if failed */
      ಪ್ರತಿಜ್ಞಾ( iSign==1 );
      ಆದರೆ( NEVER(pParse->pVdbe==0) ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ನಿಲ್ಲಿಸು;
      pVal = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pParse->pReprepare, p->i_ಸ್ತಂಭ,
                                      SQLITE_AFF_BLOB);
      ಆದರೆ( pVal ){
        i64 vv;
        ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(pVal)==SQLITE_INTEGER
         && (vv = sqlite3_ಮೌಲ್ಯ_int64(pVal))>=0
         && vv<=0x7fffffff
        ){
          *p_ಮೌಲ್ಯ = (ಪೂರ್ಣಾಂಕ)vv;
          rc = 1;       
        }
        sqlite3_ಮೌಲ್ಯ_Free(pVal);
      }
      sqlite3VdbeReprepareOnBind(pParse->pVdbe, p->i_ಸ್ತಂಭ, bRSI);
      ಹಿಂತಿರುಗಿ rc;
    }
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p){
  u8 op;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆಗ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    p = p->p_ಎಡ;
    ಪ್ರತಿಜ್ಞಾ( p!=0 );
  }
  op = p->op;
  ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ) op = p->op2;
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ TK_INTEGER:
    ಸ್ಥಿತಿ TK_STRING:
    ಸ್ಥಿತಿ TK_FLOAT:
    ಸ್ಥಿತಿ TK_BLOB:
      ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ:
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_CanBeNull)
          || NEVER(p->y.pTab==0) /* Reference to column of index on expr */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
          || (p->i_ಸ್ತಂಭ==XN_ROWID && IsView(p->y.pTab))
#ಆದರೆ_ಅಂತ್ಯ
          || (p->i_ಸ್ತಂಭ>=0
              && p->y.pTab->a_ಲಂಬಸಾಲು!=0 /* Possible due to prior error */
              && ALWAYS(p->i_ಸ್ತಂಭ<p->y.pTab->n_ಲಂಬಸಾಲು)
              && p->y.pTab->a_ಲಂಬಸಾಲು[p->i_ಸ್ತಂಭ].notNull==0);
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 1;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Needs_No_Affinity_Change(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p, ಅಕ್ಷರ aff){
  u8 op;
  ಪೂರ್ಣಾಂಕ unaryMinus = 0;
  ಆದರೆ( aff==SQLITE_AFF_BLOB ) ಹಿಂತಿರುಗಿ 1;
  ಆಗ( p->op==TK_UPLUS || p->op==TK_UMINUS ){
    ಆದರೆ( p->op==TK_UMINUS ) unaryMinus = 1;
    p = p->p_ಎಡ;
  }
  op = p->op;
  ಆದರೆ( op==TK_ರಿಜಿಸ್ಟರ್ ) op = p->op2;
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ TK_INTEGER: {
      ಹಿಂತಿರುಗಿ aff>=SQLITE_AFF_NUMERIC;
    }
    ಸ್ಥಿತಿ TK_FLOAT: {
      ಹಿಂತಿರುಗಿ aff>=SQLITE_AFF_NUMERIC;
    }
    ಸ್ಥಿತಿ TK_STRING: {
      ಹಿಂತಿರುಗಿ !unaryMinus && aff==SQLITE_AFF_ಪಠ್ಯ;
    }
    ಸ್ಥಿತಿ TK_BLOB: {
      ಹಿಂತಿರುಗಿ !unaryMinus;
    }
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      ಪ್ರತಿಜ್ಞಾ( p->i_ಕೋಷ್ಟಕ>=0 );  /* p cannot be part of a CHECK constraint */
      ಹಿಂತಿರುಗಿ aff>=SQLITE_AFF_NUMERIC && p->i_ಸ್ತಂಭ<0;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಹಿಂತಿರುಗಿ 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
ಪೂರ್ಣಾಂಕ sqlite3IsRowid(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಆದರೆ( sqlite3StrICmp(z, "_ROWID_")==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( sqlite3StrICmp(z, "ROWID")==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( sqlite3StrICmp(z, "OID")==0 ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return a pointer to a buffer containing a usable rowid alias for table
** pTab. An alias is usable if there is not an explicit user-defined column 
** of the same name.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3RowidAlias(ಕೋಷ್ಟಕ *pTab){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  ಪೂರ್ಣಾಂಕ ii;
  ಪ್ರತಿಜ್ಞಾ( VisibleRowid(pTab) );
  ಗಾಗಿ(ii=0; ii<Array_ಗಾತ್ರ(azOpt); ii++){
    ಆದರೆ( sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, azOpt[ii])<0 ) ಹಿಂತಿರುಗಿ azOpt[ii];
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** pX is the RHS of an IN operator.  If pX is a SELECT statement
** that can be simplified to a direct table access, then return
** a pointer to the SELECT statement.  If pX is not a SELECT statement,
** or if the SELECT statement needs to be materialized into a transient
** table, then return NULL.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
ಸ್ಥಿರ Select *is_Candidate_For_ಒಳ_Opt(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pX){
  Select *p;
  SrcList *pSrc;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  ಕೋಷ್ಟಕ *pTab;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX) ) ಹಿಂತಿರುಗಿ 0;                 /* Not a subquery */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pX, EP_VarSelect)  ) ಹಿಂತಿರುಗಿ 0;  /* Correlated subq */
  p = pX->x.pSelect;
  ಆದರೆ( p->p_ಪೂರ್ವ ) ಹಿಂತಿರುಗಿ 0;              /* Not a compound SELECT */
  ಆದರೆ( p->selFlags & SF_Aggregate ){
    ಹಿಂತಿರುಗಿ 0; /* No GROUP BY keyword or aggregate functions */
  }
  ಪ್ರತಿಜ್ಞಾ( p->p_ಗುಂಪು_By==0 );              /* Has no GROUP BY clause */
  ಆದರೆ( p->p_ಎಲ್ಲೆ ) ಹಿಂತಿರುಗಿ 0;              /* Has no LIMIT clause */
  ಆದರೆ( p->pWhere ) ಹಿಂತಿರುಗಿ 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
  ಆದರೆ( pSrc->nSrc!=1 ) ಹಿಂತಿರುಗಿ 0;          /* Single term in FROM clause */
  ಆದರೆ( pSrc->a[0].fg.isSubquery) ಹಿಂತಿರುಗಿ 0;/* FROM is not a subquery or view */
  pTab = pSrc->a[0].pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab)  );              /* FROM clause is not a view */
  ಆದರೆ( IsVirtual(pTab) ) ಹಿಂತಿರುಗಿ 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
  /* All SELECT results must be columns. */
  ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *pRes = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( pRes->op!=TK_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ 0;
    ಪ್ರತಿಜ್ಞಾ( pRes->i_ಕೋಷ್ಟಕ==pSrc->a[0].iCursor );  /* Not a correlated subquery */
  }
  ಹಿಂತಿರುಗಿ p;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Generate code that checks the left-most column of index table iCur to see if
** it contains any NULL entries.  Cause the register at regHasNull to be set
** to a non-NULL value if iCur contains no NULLs.  Cause register regHasNull
** to be set to NULL if iCur contains one or more NULL values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ಗಣ_Has_Null_Flag(
  Vdbe *v,           /* Write new code into this statement under construction */
  ಪೂರ್ಣಾಂಕ iCur,          /* Cursor for the index */
  ಪೂರ್ಣಾಂಕ regHasNull,    /* Register in which to store hasNull flag */
  ಪೂರ್ಣಾಂಕ e_ವಿಂಗಡಿಸು_ಕ್ರಮ     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  ಪೂರ್ಣಾಂಕ addr1;
  ಪೂರ್ಣಾಂಕ op;
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regHasNull);
  ಆದರೆ( e_ವಿಂಗಡಿಸು_ಕ್ರಮ==SQLITE_SO_ASC ){
    op = OP_Rewind;
  }ಇಲ್ಲದಿದ್ದರೆ{
    op = OP_ಕೊನೆಯ;
  }
  addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, op, iCur); VdbeCoverage(v);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
  VdbeComment((v, op==OP_ಕೊನೆಯ?"last_entry_in(%d)":"first_entry_in(%d)", iCur));
  sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
}
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** The argument is an IN operator with a list (not a subquery) on the
** right-hand side.  Return TRUE if that list is constant.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಒಳ_Rhs_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಒಳ){
  ಅಭಿವ್ಯಕ್ತಿ *pLHS;
  ಪೂರ್ಣಾಂಕ res;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಒಳ, EP_xIsSelect) );
  pLHS = p_ಒಳ->p_ಎಡ;
  p_ಒಳ->p_ಎಡ = 0;
  res = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, p_ಒಳ);
  p_ಒಳ->p_ಎಡ = pLHS;
  ಹಿಂತಿರುಗಿ res;
}
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
ಪೂರ್ಣಾಂಕ sqlite3_ಕಂಡುಹಿಡಿ_ಒಳ_ಸೂಚ್ಯಂಕ(
  Parse *pParse,             /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *pX,                  /* The IN expression */
  u32 ಒಳ_Flags,               /* IN_INDEX_LOOP, _MEMBERSHIP, and/or _NOOP_OK */
  ಪೂರ್ಣಾಂಕ *prRhsHasNull,         /* Register holding NULL status.  See notes */
  ಪೂರ್ಣಾಂಕ *aiMap,                /* Mapping from Index fields to RHS fields */
  ಪೂರ್ಣಾಂಕ *piTab                 /* OUT: index to use */
){
  Select *p;                            /* SELECT to the right of IN operator */
  ಪೂರ್ಣಾಂಕ eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  ಪೂರ್ಣಾಂಕ iTab;                             /* Cursor of the RHS table */
  ಪೂರ್ಣಾಂಕ mustBeUnique;                     /* True if RHS must be unique */
  Vdbe *v = sqlite3GetVdbe(pParse);     /* Virtual machine being coded */

  ಪ್ರತಿಜ್ಞಾ( pX->op==TK_ಒಳ );
  mustBeUnique = (ಒಳ_Flags & ಒಳ_ಸೂಚ್ಯಂಕ_LOOP)!=0;
  iTab = pParse->nTab++;

  /* If the RHS of this IN(...) operator is a SELECT, and if it matters
  ** whether or not the SELECT result contains NULL values, check whether
  ** or not NULL is actually possible (it may not be, for example, due
  ** to NOT NULL constraints in the schema). If no NULL values are possible,
  ** set prRhsHasNull to 0 before continuing.  */
  ಆದರೆ( prRhsHasNull && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX) ){
    ಪೂರ್ಣಾಂಕ i;
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList = pX->x.pSelect->pEList;
    ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ) ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( i==pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      prRhsHasNull = 0;
    }
  }

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new
  ** ephemeral table.  */
  ಆದರೆ( pParse->nErr==0 && (p = is_Candidate_For_ಒಳ_Opt(pX))!=0 ){
    sqlite3 *db = pParse->db;              /* Database connection */
    ಕೋಷ್ಟಕ *pTab;                           /* Table <table>. */
    ಪೂರ್ಣಾಂಕ iDb;                               /* Database idx for pTab */
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList = p->pEList;
    ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;

    ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );             /* Because of isCandidateForInOpt(p) */
    ಪ್ರತಿಜ್ಞಾ( p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 ); /* Because of isCandidateForInOpt(p) */
    ಪ್ರತಿಜ್ಞಾ( p->pSrc!=0 );               /* Because of isCandidateForInOpt(p) */
    pTab = p->pSrc->a[0].pSTab;

    /* Code an OP_Transaction and OP_TableLock for <table>. */
    iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
    ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<SQLITE_MAX_DB );
    sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
    sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, 0, pTab->zName);

    ಪ್ರತಿಜ್ಞಾ(v);  /* sqlite3GetVdbe() has always been previously called */
    ಆದರೆ( n_ಅಭಿವ್ಯಕ್ತಿ==1 && pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ){
      /* The "x IN (SELECT rowid FROM table)" case */
      ಪೂರ್ಣಾಂಕ iAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once);
      VdbeCoverage(v);

      sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = ಒಳ_ಸೂಚ್ಯಂಕ_ROWID;
      ExplainQueryPlan((pParse, 0,
            "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR",pTab->zName));
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, iAddr);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸೂಚ್ಯಂಕ *pIdx;                         /* Iterator variable */
      ಪೂರ್ಣಾಂಕ affinity_ok = 1;
      ಪೂರ್ಣಾಂಕ i;

      /* Check that the affinity that will be used to perform each
      ** comparison is the same as the affinity of each column in table
      ** on the RHS of the IN operator.  If it not, it is not possible to
      ** use any index of the RHS table.  */
      ಗಾಗಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ && affinity_ok; i++){
        ಅಭಿವ್ಯಕ್ತಿ *pLhs = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(pX->p_ಎಡ, i);
        ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
        ಅಕ್ಷರ idxaff = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_Affinity(pTab,i_ಲಂಬಸಾಲು); /* RHS table */
        ಅಕ್ಷರ cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        testcase( cmpaff==SQLITE_AFF_BLOB );
        testcase( cmpaff==SQLITE_AFF_ಪಠ್ಯ );
        ಆಯ್ಕೆ( cmpaff ){
          ಸ್ಥಿತಿ SQLITE_AFF_BLOB:
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ SQLITE_AFF_ಪಠ್ಯ:
            /* sqlite3CompareAffinity() only returns TEXT if one side or the
            ** other has no affinity and the other side is TEXT.  Hence,
            ** the only way for cmpaff to be TEXT is for idxaff to be TEXT
            ** and for the term on the LHS of the IN to have no affinity. */
            ಪ್ರತಿಜ್ಞಾ( idxaff==SQLITE_AFF_ಪಠ್ಯ );
            ನಿಲ್ಲಿಸು;
          ಪೂರ್ವನಿಯೋಜಿತ:
            affinity_ok = sqlite3IsNumericAffinity(idxaff);
        }
      }

      ಆದರೆ( affinity_ok ){
        /* Search for an existing index that will work for this IN operator */
        ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx && eType==0; pIdx=pIdx->p_ಮುಂದಿನ){
          Bitmask ಲಂಬಸಾಲು_ಬಳಸಿದ;      /* Columns of the index used */
          Bitmask m_ಲಂಬಸಾಲು;         /* Mask for the current column */
          ಆದರೆ( pIdx->n_ಸ್ತಂಭ<n_ಅಭಿವ್ಯಕ್ತಿ ) ಮುಂದುವರಿ;
          ಆದರೆ( pIdx->pPartIdxWhere!=0 ) ಮುಂದುವರಿ;
          /* Maximum nColumn is BMS-2, not BMS-1, so that we can compute
          ** BITMASK(nExpr) without overflowing */
          testcase( pIdx->n_ಸ್ತಂಭ==BMS-2 );
          testcase( pIdx->n_ಸ್ತಂಭ==BMS-1 );
          ಆದರೆ( pIdx->n_ಸ್ತಂಭ>=BMS-1 ) ಮುಂದುವರಿ;
          ಆದರೆ( mustBeUnique ){
            ಆದರೆ( pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು>n_ಅಭಿವ್ಯಕ್ತಿ
             ||(pIdx->n_ಸ್ತಂಭ>n_ಅಭಿವ್ಯಕ್ತಿ && !Is_Unique_ಸೂಚ್ಯಂಕ(pIdx))
            ){
              ಮುಂದುವರಿ;  /* This index is not unique over the IN RHS columns */
            }
          }
 
          ಲಂಬಸಾಲು_ಬಳಸಿದ = 0;   /* Columns of index used so far */
          ಗಾಗಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ; i++){
            ಅಭಿವ್ಯಕ್ತಿ *pLhs = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(pX->p_ಎಡ, i);
            ಅಭಿವ್ಯಕ್ತಿ *pRhs = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
            CollSeq *pReq = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, pLhs, pRhs);
            ಪೂರ್ಣಾಂಕ j;
 
            ಗಾಗಿ(j=0; j<n_ಅಭಿವ್ಯಕ್ತಿ; j++){
              ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[j]!=pRhs->i_ಸ್ತಂಭ ) ಮುಂದುವರಿ;
              ಪ್ರತಿಜ್ಞಾ( pIdx->azColl[j] );
              ಆದರೆ( pReq!=0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j])!=0 ){
                ಮುಂದುವರಿ;
              }
              ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ( j==n_ಅಭಿವ್ಯಕ್ತಿ ) ನಿಲ್ಲಿಸು;
            m_ಲಂಬಸಾಲು = MASKBIT(j);
            ಆದರೆ( m_ಲಂಬಸಾಲು & ಲಂಬಸಾಲು_ಬಳಸಿದ ) ನಿಲ್ಲಿಸು; /* Each column used only once */
            ಲಂಬಸಾಲು_ಬಳಸಿದ |= m_ಲಂಬಸಾಲು;
            ಆದರೆ( aiMap ) aiMap[i] = j;
          }
 
          ಪ್ರತಿಜ್ಞಾ( n_ಅಭಿವ್ಯಕ್ತಿ>0 && n_ಅಭಿವ್ಯಕ್ತಿ<BMS );
          ಪ್ರತಿಜ್ಞಾ( i==n_ಅಭಿವ್ಯಕ್ತಿ || ಲಂಬಸಾಲು_ಬಳಸಿದ!=(MASKBIT(n_ಅಭಿವ್ಯಕ್ತಿ)-1) );
          ಆದರೆ( ಲಂಬಸಾಲು_ಬಳಸಿದ==(MASKBIT(n_ಅಭಿವ್ಯಕ್ತಿ)-1) ){
            /* If we reach this point, that means the index pIdx is usable */
            ಪೂರ್ಣಾಂಕ iAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
            ExplainQueryPlan((pParse, 0,
                              "USING INDEX %s FOR IN-OPERATOR",pIdx->zName));
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenRead, iTab, pIdx->tnum, iDb);
            sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pIdx);
            VdbeComment((v, "%s", pIdx->zName));
            ಪ್ರತಿಜ್ಞಾ( ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC == ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_ASC+1 );
            eType = ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_ASC + pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[0];
 
            ಆದರೆ( prRhsHasNull ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_ಬಳಸಿದ_ಮುಖವಾಡ
              u64 ಮುಖವಾಡ = (1<<n_ಅಭಿವ್ಯಕ್ತಿ)-1;
              sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Columns_ಬಳಸಿದ,
                                iTab, LOWER32(ಮುಖವಾಡ), UPPER32(ಮುಖವಾಡ));
#ಆದರೆ_ಅಂತ್ಯ
              *prRhsHasNull = ++pParse->nMem;
              ಆದರೆ( n_ಅಭಿವ್ಯಕ್ತಿ==1 ){
                sqlite3_ಗಣ_Has_Null_Flag(v, iTab, *prRhsHasNull,
                                      pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[0]);
              }
            }
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, iAddr);
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
  ಆದರೆ( eType==0
   && (ಒಳ_Flags & ಒಳ_ಸೂಚ್ಯಂಕ_NOOP_OK)
   && ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pX)
   && (!sqlite3_ಒಳ_Rhs_Is_Constant(pParse,pX) || pX->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ<=2)
  ){
    pParse->nTab--;  /* Back out the allocation of the unused cursor */
    iTab = -1;       /* Cursor is not allocated */
    eType = ಒಳ_ಸೂಚ್ಯಂಕ_NOOP;
  }

  ಆದರೆ( eType==0 ){
    /* Could not find an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    u32 savedNQueryLoop = pParse->nQueryLoop;
    ಪೂರ್ಣಾಂಕ rMayHaveNull = 0;
    ಪೂರ್ಣಾಂಕ bloomOk = (ಒಳ_Flags & ಒಳ_ಸೂಚ್ಯಂಕ_MEMBERSHIP)!=0;
    eType = ಒಳ_ಸೂಚ್ಯಂಕ_EPH;
    ಆದರೆ( ಒಳ_Flags & ಒಳ_ಸೂಚ್ಯಂಕ_LOOP ){
      pParse->nQueryLoop = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( prRhsHasNull ){
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }
    ಪ್ರತಿಜ್ಞಾ( pX->op==TK_ಒಳ );
    ಆದರೆ( !bloomOk
     && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX)
     && (pX->x.pSelect->selFlags & SF_Cloned_Rhs_ಒಳ)!=0
    ){
      bloomOk = 1;
    }
    sqlite3_ಸಂಕೇತ_Rhs_ದ_ಒಳ(pParse, pX, iTab, bloomOk);
    ಆದರೆ( rMayHaveNull ){
      sqlite3_ಗಣ_Has_Null_Flag(v, iTab, rMayHaveNull, SQLITE_SO_ASC);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  ಆದರೆ( aiMap && eType!=ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_ASC && eType!=ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC ){
    ಪೂರ್ಣಾಂಕ i, n;
    n = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(pX->p_ಎಡ);
    ಗಾಗಿ(i=0; i<n; i++) aiMap[i] = i;
  }
  *piTab = iTab;
  ಹಿಂತಿರುಗಿ eType;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Argument pExpr is an (?, ?...) IN(...) expression. This
** function allocates and returns a nul-terminated string containing
** the affinities to be used for each column of the comparison.
**
** It is the responsibility of the caller to ensure that the returned
** string is eventually freed using sqlite3DbFree().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *ಅಭಿವ್ಯಕ್ತಿ_INAffinity(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಪೂರ್ಣಾಂಕ nVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಎಡ);
  Select *pSelect = ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect : 0;
  ಅಕ್ಷರ *zRet;

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಒಳ );
  zRet = sqlite3DbMallocRaw(pParse->db, 1+(i64)nVal);
  ಆದರೆ( zRet ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<nVal; i++){
      ಅಭಿವ್ಯಕ್ತಿ *pA = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಎಡ, i);
      ಅಕ್ಷರ a = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pA);
      ಆದರೆ( pSelect ){
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, a);
      }ಇಲ್ಲದಿದ್ದರೆ{
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  ಹಿಂತಿರುಗಿ zRet;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Load the Parse object passed as the first argument with an error
** message of the form:
**
**   "sub-select returns N columns - expected M"
*/  
ಶೂನ್ಯ sqlite3SubselectError(Parse *pParse, ಪೂರ್ಣಾಂಕ nActual, ಪೂರ್ಣಾಂಕ nExpect){
  ಆದರೆ( pParse->nErr==0 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3_Error_ಸಂದೇಶ(pParse, zFmt, nActual, nExpect);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಶೂನ್ಯ sqlite3_ಸದಿಶ_Error_ಸಂದೇಶ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3SubselectError(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 1);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Scan all previously generated bytecode looking for an OP_BeginSubrtn
** that is compatible with pExpr.  If found, add the y.sub values
** to pExpr and return true.  If not found, return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_Compatible_ಒಳ_Rhs_Subrtn(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* IN operator with RHS that we want to reuse */
  SubrtnSig *pNewSig      /* Signature for the IN operator */
){
  VdbeOp *pOp, *p_ಕೊನೆ;
  SubrtnSig *pSig;
  Vdbe *v;

  ಆದರೆ( pNewSig==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( (pParse->mSubrtnSig & (1<<(pNewSig->selId&7)))==0 ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಒಳ );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_All)==0 );
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  pOp = sqlite3VdbeGetOp(v, 1);
  p_ಕೊನೆ = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v);
  ಗಾಗಿ(; pOp<p_ಕೊನೆ; pOp++){
    ಆದರೆ( pOp->p4type!=P4_SUBRTNSIG ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_BeginSubrtn );
    pSig = pOp->p4.pSubrtnSig;
    ಪ್ರತಿಜ್ಞಾ( pSig!=0 );
    ಆದರೆ( !pSig->bComplete ) ಮುಂದುವರಿ;
    ಆದರೆ( pNewSig->selId!=pSig->selId ) ಮುಂದುವರಿ;
    ಆದರೆ( strcmp(pNewSig->zAff,pSig->zAff)!=0 ) ಮುಂದುವರಿ;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr = pSig->iAddr;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = pSig->regReturn;
    p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pSig->i_ಕೋಷ್ಟಕ;
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
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
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Rhs_ದ_ಒಳ(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* The IN operator */
  ಪೂರ್ಣಾಂಕ iTab,               /* Use this cursor number */
  ಪೂರ್ಣಾಂಕ allowBloom          /* True to allow the use of a Bloom filter */
){
  ಪೂರ್ಣಾಂಕ addrOnce = 0;           /* Address of the OP_Once instruction at top */
  ಪೂರ್ಣಾಂಕ addr;                   /* Address of OP_OpenEphemeral instruction */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;                /* the LHS of the IN operator */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = 0;      /* Key information */
  ಪೂರ್ಣಾಂಕ nVal;                   /* Size of vector pLeft */
  Vdbe *v;                    /* The prepared statement under construction */
  SubrtnSig *pSig = 0;        /* Signature for this subroutine */

  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );

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
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) && pParse->iSelfTab==0 ){
    /* Reuse of the RHS is allowed
    **
    ** Compute a signature for the RHS of the IN operator to facility
    ** finding and reusing prior instances of the same IN operator.
    */
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) || p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) && (p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags & SF_All)==0 ){
      pSig = sqlite3DbMallocRawNN(pParse->db, ಗಾತ್ರ(pSig[0]));
      ಆದರೆ( pSig ){
        pSig->selId = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId;
        pSig->zAff = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
    }

    /* Check to see if there is a prior materialization of the RHS of
    ** this IN operator.  If there is, then make use of that prior
    ** materialization rather than recomputing it.
    */
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) 
     || ಕಂಡುಹಿಡಿ_Compatible_ಒಳ_Rhs_Subrtn(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSig)
    ){
      addrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ExplainQueryPlan((pParse, 0, "REUSE LIST SUBQUERY %d",
              p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId));
      }
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                        p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
      ಪ್ರತಿಜ್ಞಾ( iTab!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenDup, iTab, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrOnce);
      ಆದರೆ( pSig ){
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      ಹಿಂತಿರುಗಿ;
    }

    /* Begin coding the subroutine */
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = ++pParse->nMem;
    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr =
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_BeginSubrtn, 0, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn) + 1;
    ಆದರೆ( pSig ){
      pSig->bComplete = 0;
      pSig->iAddr = p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr;
      pSig->regReturn = p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn;
      pSig->i_ಕೋಷ್ಟಕ = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId&7);
      sqlite3VdbeChangeP4(v, -1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)pSig, P4_SUBRTNSIG);
    }
    addrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
  }

  /* Check to see if this is a vector IN operator */
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  nVal = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಎಡ);

  /* Construct the ephemeral table that will contain the content of
  ** RHS of the IN operator.
  */
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = iTab;
  addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, nVal);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    VdbeComment((v, "Result of SELECT %u", p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selId));
  }ಇಲ್ಲದಿದ್ದರೆ{
    VdbeComment((v, "RHS of IN operator"));
  }
#ಆದರೆ_ಅಂತ್ಯ
  p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_Alloc(pParse->db, nVal, 1);
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 || pParse->nErr );
  ಆದರೆ( p_ಕೀಲಿ_Info==0 ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    /* Case 1:     expr IN (SELECT ...)
    **
    ** Generate code to write the results of the select into the temporary
    ** table allocated and opened above.
    */
    Select *pSelect = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect;
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList = pSelect->pEList;

    ExplainQueryPlan((pParse, 1, "%sLIST SUBQUERY %d",
        addrOnce?"":"CORRELATED ", pSelect->selId
    ));
    /* If the LHS and RHS of the IN operator do not match, that
    ** error will have been caught long before we reach this point. */
    ಆದರೆ( ALWAYS(pEList->n_ಅಭಿವ್ಯಕ್ತಿ==nVal) ){
      Select *pCopy;
      SelectDest dest;
      ಪೂರ್ಣಾಂಕ i;
      ಪೂರ್ಣಾಂಕ rc;
      ಪೂರ್ಣಾಂಕ addrBloom = 0;
      sqlite3SelectDestInit(&dest, SRT_ಗಣ, iTab);
      dest.zAffSdst = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      pSelect->i_ಎಲ್ಲೆ = 0;
      ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pEList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Info) );
      ಗಾಗಿ(i=0; i<nVal; i++){
        ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಎಡ, i);
        CollSeq *pColl;
        p_ಕೀಲಿ_Info->aColl[i] = pColl = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(
            pParse, p, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ
        );
        ಆದರೆ( !sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
          allowBloom = 0;  /* tag-202607231411 */
        }
      }
      ಆದರೆ( addrOnce
       && allowBloom
       && OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
      ){
        ಪೂರ್ಣಾಂಕ regBloom = ++pParse->nMem;
        addrBloom = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Blob, 10000, regBloom);
        VdbeComment((v, "Bloom filter"));
        dest.iSDParm2 = regBloom;
        sqlite3VdbeChangeP4(v, addr, (ಶೂನ್ಯ *)p_ಕೀಲಿ_Info, P4_KEYINFO);
        p_ಕೀಲಿ_Info = 0;
      }
      testcase( pSelect->selFlags & SF_Distinct );
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 :sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      ಆದರೆ( addrBloom ){
        /* Remember that location of the Bloom filter in the P3 operand
        ** of the OP_Once that began this subroutine. tag-202407032019 */
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        ಆದರೆ( dest.iSDParm2==0 ){
          /* If the Bloom filter won't actually be used, keep it small */
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      ಆದರೆ( rc ){
        sqlite3_ಕೀಲಿ_Info_Unref(p_ಕೀಲಿ_Info);
        ಹಿಂತಿರುಗಿ;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0) ){
    /* Case 2:     expr IN (exprlist)
    **
    ** For each expression, build an index key from the evaluation and
    ** store it in the temporary table. If <expr> is a column, then use
    ** that columns affinity when building index keys. If <expr> is not
    ** a column, use numeric affinity.
    */
    ಅಕ್ಷರ affinity;            /* Affinity of the LHS of the IN */
    ಪೂರ್ಣಾಂಕ i;
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
    ಪೂರ್ಣಾಂಕ r1, r2;
    affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ);
    ಆದರೆ( affinity<=SQLITE_AFF_NONE ){
      affinity = SQLITE_AFF_BLOB;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( affinity==SQLITE_AFF_REAL ){
      affinity = SQLITE_AFF_NUMERIC;
    }
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 );
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Info) );
    p_ಕೀಲಿ_Info->aColl[0] = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);

    /* Loop through each expression in <exprlist>. */
    r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    ಗಾಗಿ(i=pList->n_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಂಶ=pList->a; i>0; i--, p_ಅಂಶ++){
      ಅಭಿವ್ಯಕ್ತಿ *pE2 = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;

      /* If the expression is not constant then we will need to
      ** disable the test that was generated above that makes sure
      ** this code only executes once.  Because for a non-constant
      ** expression we need to rerun this code each time.
      */
      ಆದರೆ( addrOnce && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, pE2) ){
        sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addrOnce-1);
        sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addrOnce);
        ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
        addrOnce = 0;
      }

      /* Evaluate the expression and insert it into the temp table */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pE2, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, r1, 1, r2, &affinity, 1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iTab, r2, r1, 1);
    }
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r2);
  }
  ಆದರೆ( pSig ) pSig->bComplete = 1;
  ಆದರೆ( p_ಕೀಲಿ_Info ){
    sqlite3VdbeChangeP4(v, addr, (ಶೂನ್ಯ *)p_ಕೀಲಿ_Info, P4_KEYINFO);
  }
  ಆದರೆ( addrOnce ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iTab);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrOnce);
    /* Subroutine return */
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeGetOp(v,p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
            || pParse->nErr );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Return, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                      p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr, 1);
    VdbeCoverage(v);
    sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
ಪೂರ್ಣಾಂಕ sqlite3_ಸಂಕೇತ_Subselect(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ addrOnce = 0;           /* Address of OP_Once at top of subroutine */
  ಪೂರ್ಣಾಂಕ rReg = 0;               /* Register storing resulting */
  Select *pSel;               /* SELECT statement to encode */
  SelectDest dest;            /* How to deal with SELECT result */
  ಪೂರ್ಣಾಂಕ nReg;                   /* Registers to allocate */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ;               /* New limit expression */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಂಕ addrExplain;            /* Address of OP_Explain instruction */
#ಆದರೆ_ಅಂತ್ಯ

  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ 0;
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS );
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pSel = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect;

  /* If this routine has already been coded, then invoke it as a
  ** subroutine. */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
    ExplainQueryPlan((pParse, 0, "REUSE SUBQUERY %d", pSel->selId));
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                      p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
    ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
  }

  /* Begin coding the subroutine */
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) );
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn);
  p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn = ++pParse->nMem;
  p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr =
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_BeginSubrtn, 0, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn) + 1;

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
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect) ){
    addrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
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
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Counters(v, addrExplain, addrExplain, -1);
  nReg = p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ? pSel->pEList->n_ಅಭಿವ್ಯಕ್ತಿ : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem+1);
  pParse->nMem += nReg;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
    dest.eDest = SRT_Mem;
    ಆದರೆ( (pSel->selFlags&SF_Distinct) && pSel->p_ಎಲ್ಲೆ && pSel->p_ಎಲ್ಲೆ->p_ಬಲ ){
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
    }ಇಲ್ಲದಿದ್ದರೆ{
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, dest.iSDParm, pParse->nMem);
    VdbeComment((v, "Init subquery result"));
  }ಇಲ್ಲದಿದ್ದರೆ{
    dest.eDest = SRT_Exists;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, dest.iSDParm);
    VdbeComment((v, "Init EXISTS result"));
  }
  ಆದರೆ( pSel->p_ಎಲ್ಲೆ ){
    /* The subquery already has a limit.  If the pre-existing limit X is 
    ** not already integer value 1 or 0, then make the new limit X<>0 so that
    ** the new limit is either 1 or 0 */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = pSel->p_ಎಲ್ಲೆ->p_ಎಡ;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಎಡ, EP_Int_ಮೌಲ್ಯ)==0
     || (p_ಎಡ->u.i_ಮೌಲ್ಯ!=1 && p_ಎಡ->u.i_ಮೌಲ್ಯ!=0)
    ){
      sqlite3 *db = pParse->db;
      p_ಎಲ್ಲೆ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 0);
      ಆದರೆ( p_ಎಲ್ಲೆ ){
        p_ಎಲ್ಲೆ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_NUMERIC;
        p_ಎಲ್ಲೆ = sqlite3PExpr(pParse, TK_NE,
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಡ, 0), p_ಎಲ್ಲೆ);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಎಡ);
      pSel->p_ಎಲ್ಲೆ->p_ಎಡ = p_ಎಲ್ಲೆ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If there is no pre-existing limit add a limit of 1 */
    p_ಎಲ್ಲೆ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(pParse->db, 1);
    pSel->p_ಎಲ್ಲೆ = sqlite3PExpr(pParse, TK_ಎಲ್ಲೆ, p_ಎಲ್ಲೆ, 0);
  }
  pSel->i_ಎಲ್ಲೆ = 0;
  ಆದರೆ( sqlite3Select(pParse, pSel, &dest) ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op2 = p_ಅಭಿವ್ಯಕ್ತಿ->op;
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_ERROR;
    ಹಿಂತಿರುಗಿ 0;
  }
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = rReg = dest.iSDParm;
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
  ಆದರೆ( addrOnce ){
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrOnce);
  }
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, addrExplain, -1);

  ಆದರೆ( dest.eDest==SRT_Mem 
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, pSel->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ) 
  ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ClrSubtype, rReg);
  }

  /* Subroutine return */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeGetOp(v,p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr-1)->opcode==OP_BeginSubrtn
          || pParse->nErr );
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Return, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn,
                    p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr, 1);
  VdbeCoverage(v);
  sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);
  ಹಿಂತಿರುಗಿ rReg;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Expr pIn is an IN(...) expression. This function checks that the
** sub-select on the RHS of the IN() operator has the same number of
** columns as the vector on the LHS. Or, if the RHS of the IN() is not
** a sub-query, that the LHS is a vector of size 1.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಒಳ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಒಳ){
  ಪೂರ್ಣಾಂಕ n_ಸದಿಶ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಒಳ->p_ಎಡ);
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಒಳ) && !pParse->db->mallocFailed ){
    ಆದರೆ( n_ಸದಿಶ!=p_ಒಳ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3SubselectError(pParse, p_ಒಳ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, n_ಸದಿಶ);
      ಹಿಂತಿರುಗಿ 1;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n_ಸದಿಶ!=1 ){
    sqlite3_ಸದಿಶ_Error_ಸಂದೇಶ(pParse, p_ಒಳ->p_ಎಡ);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ(
  Parse *pParse,        /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,          /* The IN expression */
  ಪೂರ್ಣಾಂಕ destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  ಪೂರ್ಣಾಂಕ destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  ಪೂರ್ಣಾಂಕ rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  ಪೂರ್ಣಾಂಕ eType;            /* Type of the RHS */
  ಪೂರ್ಣಾಂಕ rLhs;             /* Register(s) holding the LHS values */
  Vdbe *v;              /* Statement under construction */
  ಪೂರ್ಣಾಂಕ *aiMap = 0;       /* Map from vector field to index column */
  ಅಕ್ಷರ *zAff = 0;       /* Affinity string for comparisons */
  ಪೂರ್ಣಾಂಕ n_ಸದಿಶ;          /* Size of vectors for this IN operator */
  ಪೂರ್ಣಾಂಕ iDummy;           /* Dummy parameter to exprCodeVector() */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;          /* The LHS of the IN operator */
  ಪೂರ್ಣಾಂಕ i;                /* loop counter */
  ಪೂರ್ಣಾಂಕ destStep2;        /* Where to jump when NULLs seen in step 2 */
  ಪೂರ್ಣಾಂಕ destStep6 = 0;    /* Start of code for Step 6 */
  ಪೂರ್ಣಾಂಕ addrTruthOp;      /* Address of opcode that determines the IN is true */
  ಪೂರ್ಣಾಂಕ destNotNull;      /* Jump here if a comparison is not true in step 6 */
  ಪೂರ್ಣಾಂಕ addr_ತುದಿ;          /* Top of the step-6 loop */
  ಪೂರ್ಣಾಂಕ iTab = 0;         /* Index to use */
  u8 ok_Const_ಗುಣಕ = pParse->ok_Const_ಗುಣಕ;

  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ) ಹಿಂತಿರುಗಿ;
  zAff = ಅಭಿವ್ಯಕ್ತಿ_INAffinity(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
  n_ಸದಿಶ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
  aiMap = (ಪೂರ್ಣಾಂಕ*)sqlite3DbMallocZero(pParse->db, n_ಸದಿಶ*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
  ಆದರೆ( pParse->db->mallocFailed ) ಹೋಗು sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_oom_error;

  /* Attempt to compute the RHS. After this step, if anything other than
  ** IN_INDEX_NOOP is returned, the table opened with cursor iTab
  ** contains the values that make up the RHS. If IN_INDEX_NOOP is returned,
  ** the RHS has not yet been coded.  */
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3_ಕಂಡುಹಿಡಿ_ಒಳ_ಸೂಚ್ಯಂಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                             ಒಳ_ಸೂಚ್ಯಂಕ_MEMBERSHIP | ಒಳ_ಸೂಚ್ಯಂಕ_NOOP_OK,
                             destIfFalse==destIfNull ? 0 : &rRhsHasNull,
                             aiMap, &iTab);

  ಪ್ರತಿಜ್ಞಾ( pParse->nErr || n_ಸದಿಶ==1 || eType==ಒಳ_ಸೂಚ್ಯಂಕ_EPH
       || eType==ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_ASC || eType==ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC
  );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Confirm that aiMap[] contains nVector integer values between 0 and
  ** nVector-1. */
  ಗಾಗಿ(i=0; i<n_ಸದಿಶ; i++){
    ಪೂರ್ಣಾಂಕ j, cnt;
    ಗಾಗಿ(cnt=j=0; j<n_ಸದಿಶ; j++) ಆದರೆ( aiMap[j]==i ) cnt++;
    ಪ್ರತಿಜ್ಞಾ( cnt==1 );
  }
#ಆದರೆ_ಅಂತ್ಯ

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
  ಪ್ರತಿಜ್ಞಾ( pParse->ok_Const_ಗುಣಕ==ok_Const_ಗುಣಕ );
  pParse->ok_Const_ಗುಣಕ = 0;
  rLhs = ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸದಿಶ(pParse, p_ಎಡ, &iDummy);
  pParse->ok_Const_ಗುಣಕ = ok_Const_ಗುಣಕ;

  /* If sqlite3FindInIndex() did not find or create an index that is
  ** suitable for evaluating the IN operator, then evaluate using a
  ** sequence of comparisons.
  **
  ** This is step (1) in the in-operator.md optimized algorithm.
  */
  ಆದರೆ( eType==ಒಳ_ಸೂಚ್ಯಂಕ_NOOP ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    CollSeq *pColl;
    ಪೂರ್ಣಾಂಕ labelOk = sqlite3VdbeMakeLabel(pParse);
    ಪೂರ್ಣಾಂಕ r2, reg_ಗಮ್ಯ_Free;
    ಪೂರ್ಣಾಂಕ regCkNull = 0;
    ಪೂರ್ಣಾಂಕ ii;
    ಪ್ರತಿಜ್ಞಾ( n_ಸದಿಶ==1 );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಆದರೆ( destIfNull!=destIfFalse ){
      regCkNull = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಬಿಟ್_And, rLhs, rLhs, regCkNull);
    }
    ಗಾಗಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
      r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ, &reg_ಗಮ್ಯ_Free);
      ಆದರೆ( regCkNull && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಬಿಟ್_And, regCkNull, r2, regCkNull);
      }
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ಗಮ್ಯ_Free);
      ಆದರೆ( ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 || destIfNull!=destIfFalse ){
        ಪೂರ್ಣಾಂಕ op = rLhs!=r2 ? OP_Eq : OP_NotNull;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, op, rLhs, labelOk, r2,
                          (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii==pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_Eq);
        VdbeCoverageIf(v, ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_NotNull);
        VdbeCoverageIf(v, ii==pList->n_ಅಭಿವ್ಯಕ್ತಿ-1 && op==OP_NotNull);
        sqlite3VdbeChangeP5(v, zAff[0]);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ op = rLhs!=r2 ? OP_Ne : OP_IsNull;
        ಪ್ರತಿಜ್ಞಾ( destIfNull==destIfFalse );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, op, rLhs, destIfFalse, r2,
                          (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
        VdbeCoverageIf(v, op==OP_Ne);
        VdbeCoverageIf(v, op==OP_IsNull);
        sqlite3VdbeChangeP5(v, zAff[0] | SQLITE_JUMPIFNULL);
      }
    }
    ಆದರೆ( regCkNull ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, regCkNull, destIfNull); VdbeCoverage(v);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regCkNull);
    ಹೋಗು sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_ಪೂರ್ಣಗೊಂಡ;
  }

  ಆದರೆ( eType!=ಒಳ_ಸೂಚ್ಯಂಕ_ROWID ){
    /* If this IN operator will use an index, then the order of columns in the
    ** vector might be different from the order in the index.  In that case,
    ** we need to reorder the LHS values to be in index order.  Run Affinity
    ** before reordering the columns, so that the affinity is correct.
    */
    ಆದರೆ( n_ಸದಿಶ==1 ){
      ಅಕ್ಷರ aff = zAff[0];
      ಆದರೆ( aff>=SQLITE_AFF_ಪಠ್ಯ && aff!=sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಎಡ) ){
        /* The OP_Affinity below may change the value. In this case, create a
        ** copy of rLhs to run OP_Affinity on, in case the original register
        ** is used again (e.g. if it is TK_AGG_COLUMN).  */
        ಪೂರ್ಣಾಂಕ rTmp = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, rLhs, rTmp, 0);
        rLhs = rTmp;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, rLhs, 1, 0, zAff, 1);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, rLhs, n_ಸದಿಶ, 0, zAff, n_ಸದಿಶ);
    }

    ಗಾಗಿ(i=0; i<n_ಸದಿಶ && aiMap[i]==i; i++){} /* Are LHS fields reordered? */
    ಆದರೆ( i!=n_ಸದಿಶ ){
      /* Need to reorder the LHS fields according to aiMap */
      ಪೂರ್ಣಾಂಕ rLhsOrig = rLhs;
      rLhs = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಸದಿಶ);
      ಗಾಗಿ(i=0; i<n_ಸದಿಶ; i++){
        testcase( aiMap[i]!=i );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, rLhsOrig+i, rLhs+aiMap[i], 0);
      }
    }
  }


  /* Step 2: Check to see if the LHS contains any NULL columns.  If the
  ** LHS does contain NULLs then the result must be either FALSE or NULL.
  ** We will then skip the binary search of the RHS.
  */
  ಆದರೆ( destIfNull==destIfFalse ){
    destStep2 = destIfFalse;
  }ಇಲ್ಲದಿದ್ದರೆ{
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  ಗಾಗಿ(i=0; i<n_ಸದಿಶ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, i);
    ಆದರೆ( pParse->nErr ) ಹೋಗು sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_oom_error;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p) ){
      testcase( aiMap[i]!=i );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, rLhs+aiMap[i], destStep2);
      VdbeCoverage(v);
    }
  }

  /* Step 3.  The LHS is now known to be non-NULL.  Do the binary search
  ** of the RHS using the LHS as a probe.  If found, the result is
  ** true.
  */
  ಆದರೆ( eType==ಒಳ_ಸೂಚ್ಯಂಕ_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree and so we also
    ** know that the RHS is non-NULL.  Hence, we combine steps 3 and 4
    ** into a single opcode. */
    ಪ್ರತಿಜ್ಞಾ( n_ಸದಿಶ==1 );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_SeekRowid, iTab, destIfFalse, rLhs);
    VdbeCoverage(v);
    addrTruthOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);  /* Return True */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( destIfFalse==destIfNull ){
      /* Combine Step 3 and Step 5 into a single opcode */
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
        ಸ್ಥಿರಾಂಕ VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
        ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_Once || pParse->nErr );
        ಆದರೆ( pOp->p3>0 ){  /* tag-202407032019 */
          ಪ್ರತಿಜ್ಞಾ( OptimizationEnabled(pParse->db, SQLITE_BloomFilter)
                 || pParse->nErr );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter, pOp->p3, destIfFalse,
                               rLhs, n_ಸದಿಶ); VdbeCoverage(v);
        }
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು, iTab, destIfFalse,
                           rLhs, n_ಸದಿಶ); VdbeCoverage(v);
      ಹೋಗು sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_ಪೂರ್ಣಗೊಂಡ;
    }
    /* Ordinary Step 3, for the case where FALSE and NULL are distinct */
    addrTruthOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iTab, 0,
                                      rLhs, n_ಸದಿಶ); VdbeCoverage(v);
  }

  /* Step 4.  If the RHS is known to be non-NULL and we did not find
  ** an match on the search above, then the result must be FALSE.
  */
  ಆದರೆ( rRhsHasNull && n_ಸದಿಶ==1 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NotNull, rRhsHasNull, destIfFalse);
    VdbeCoverage(v);
  }

  /* Step 5.  If we do not care about the difference between NULL and
  ** FALSE, then just return false.
  */
  ಆದರೆ( destIfFalse==destIfNull ) sqlite3VdbeGoto(v, destIfFalse);

  /* Step 6: Loop through rows of the RHS.  Compare each row to the LHS.
  ** If any comparison is NULL, then the result is NULL.  If all
  ** comparisons are FALSE then the final result is FALSE.
  **
  ** For a scalar LHS, it is sufficient to check just the first row
  ** of the RHS.
  */
  ಆದರೆ( destStep6 ) sqlite3VdbeResolveLabel(v, destStep6);
  ಆದರೆ( eType==ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC ){
    addr_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೊನೆಯ, iTab, destIfFalse);
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( eType==ಒಳ_ಸೂಚ್ಯಂಕ_EPH );
    testcase( eType==ಒಳ_ಸೂಚ್ಯಂಕ_ROWID );
    addr_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, iTab, destIfFalse);
  }
  VdbeCoverage(v);
  ಆದರೆ( n_ಸದಿಶ>1 ){
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For nVector==1, combine steps 6 and 7 by immediately returning
    ** FALSE if the first comparison is not NULL */
    destNotNull = destIfFalse;
  }
  ಗಾಗಿ(i=0; i<n_ಸದಿಶ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p;
    CollSeq *pColl;
    ಪೂರ್ಣಾಂಕ r3 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    p = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಎಡ, i);
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಅಭಿವ್ಯಕ್ತಿ *pRhs = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
      pColl = sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(pParse, p, pRhs);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* If the RHS of the IN(...) expression are scalar expressions, do
      ** not consider their collation sequences. The documentation says
      ** "The collating sequence used for expressions of the form "x IN (y, z,
      ** ...)" is the collating sequence of x.".  */
      pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p);
    }
    testcase( aiMap[i]!=i );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iTab, aiMap[i], r3);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Ne, rLhs+aiMap[i], destNotNull, r3,
                      (ಶೂನ್ಯ*)pColl, P4_COLLSEQ);
    VdbeCoverage(v);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r3);
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, destIfNull);
  ಆದರೆ( n_ಸದಿಶ>1 ){
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iTab, addr_ತುದಿ+1);
    VdbeCoverage(v);

    /* Step 7:  If we reach this point, we know that the result must
    ** be false. */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, destIfFalse);
  }

  /* Jumps here in order to return true. */
  sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrTruthOp);

sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_ಪೂರ್ಣಗೊಂಡ:
  VdbeComment((v, "end IN expr"));
sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Real(Vdbe *v, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ negateFlag, ಪೂರ್ಣಾಂಕ iMem){
  ಆದರೆ( ALWAYS(z!=0) ){
    ದ್ವಿಗುಣ ಮೌಲ್ಯ;
    sqlite3AtoF(z, &ಮೌಲ್ಯ);
    ಪ್ರತಿಜ್ಞಾ( !sqlite3IsNaN(ಮೌಲ್ಯ) ); /* The new AtoF never returns NaN */
    ಆದರೆ( negateFlag ) ಮೌಲ್ಯ = -ಮೌಲ್ಯ;
    sqlite3_Vdbe_ಕೂಡಿಸು_Double(v, iMem, ಮೌಲ್ಯ);
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Integer(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ negFlag, ಪೂರ್ಣಾಂಕ iMem){
  Vdbe *v = pParse->pVdbe;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Int_ಮೌಲ್ಯ ){
    ಪೂರ್ಣಾಂಕ i = p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ;
    ಪ್ರತಿಜ್ಞಾ( i>=0 );
    ಆದರೆ( negFlag ) i = -i;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, i, iMem);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ c;
    i64 ಮೌಲ್ಯ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
    ಪ್ರತಿಜ್ಞಾ( z!=0 );
    c = sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(z, &ಮೌಲ್ಯ);
    ಆದರೆ( (c==3 && !negFlag) || (c==2) || (negFlag && ಮೌಲ್ಯ==ಅತಿಚಿಕ್ಕ_INT64)){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
      sqlite3_Error_ಸಂದೇಶ(pParse, "oversized integer: %s%#T", negFlag?"-":"",p_ಅಭಿವ್ಯಕ್ತಿ);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_HEX_INTEGER
      ಆದರೆ( sqlite3_strnicmp(z,"0x",2)==0 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "hex literal too big: %s%#T",
                        negFlag?"-":"",p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
      {
        ಸಂಕೇತ_Real(v, z, negFlag, iMem);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( negFlag ){ ಮೌಲ್ಯ = c==3 ? ಅತಿಚಿಕ್ಕ_INT64 : -ಮೌಲ್ಯ; }
      sqlite3_Vdbe_ಕೂಡಿಸು_Int64(v, iMem, ಮೌಲ್ಯ);
    }
  }
}


/* Generate code that will load into register regOut a value that is
** appropriate for the iIdxCol-th column of index pIdx.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತುಂಬು_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ(
  Parse *pParse,  /* The parsing context */
  ಸೂಚ್ಯಂಕ *pIdx,    /* The index whose column is to be loaded */
  ಪೂರ್ಣಾಂಕ iTabCur,    /* Cursor pointing to a table row */
  ಪೂರ್ಣಾಂಕ i_Idx_ಲಂಬಸಾಲು,    /* The column of the index to be loaded */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು      /* Store the index column value in this register */
){
  i16 i_Tab_ಲಂಬಸಾಲು = pIdx->ai_ಸ್ತಂಭ[i_Idx_ಲಂಬಸಾಲು];
  ಆದರೆ( i_Tab_ಲಂಬಸಾಲು==XN_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ );
    ಪ್ರತಿಜ್ಞಾ( pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->n_ಅಭಿವ್ಯಕ್ತಿ>i_Idx_ಲಂಬಸಾಲು );
    pParse->iSelfTab = iTabCur + 1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(pParse, pIdx->a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->a[i_Idx_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ, reg_ಹೊರಹೋಗು);
    pParse->iSelfTab = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(pParse->pVdbe, pIdx->p_ಕೋಷ್ಟಕ, iTabCur,
                                    i_Tab_ಲಂಬಸಾಲು, reg_ಹೊರಹೋಗು);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
/*
** Generate code that will compute the value of generated column pCol
** and store the result in register regOut
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Generated_ಸ್ತಂಭ(
  Parse *pParse,     /* Parsing context */
  ಕೋಷ್ಟಕ *pTab,       /* Table containing the generated column */
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು,      /* The generated column */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು         /* Put the result in this register */
){
  ಪೂರ್ಣಾಂಕ iAddr;
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ nErr = pParse->nErr;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->iSelfTab!=0 );
  ಆದರೆ( pParse->iSelfTab>0 ){
    iAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_If_Null_ಅಡ್ಡಸಾಲು, pParse->iSelfTab-1, 0, reg_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    iAddr = 0;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(pParse, sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(pTab,p_ಲಂಬಸಾಲು), reg_ಹೊರಹೋಗು);
  ಆದರೆ( (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL)!=0
   && (pTab->tabFlags & TF_Strict)!=0
  ){
    ಪೂರ್ಣಾಂಕ p3 = 2+(ಪೂರ್ಣಾಂಕ)(p_ಲಂಬಸಾಲು - pTab->a_ಲಂಬಸಾಲು);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_TypeCheck, reg_ಹೊರಹೋಗು, 1, p3, (ಅಕ್ಷರ*)pTab, P4_ಕೋಷ್ಟಕ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು->affinity>=SQLITE_AFF_ಪಠ್ಯ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, reg_ಹೊರಹೋಗು, 1, 0, &p_ಲಂಬಸಾಲು->affinity, 1);
  }
  ಆದರೆ( iAddr ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, iAddr);
  ಆದರೆ( pParse->nErr>nErr ) pParse->db->err_Byte_ಸ್ಥಾನಾಂತರ = -1;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_GENERATED_COLUMNS */

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(
  Vdbe *v,        /* Parsing context */
  ಕೋಷ್ಟಕ *pTab,    /* The table containing the value */
  ಪೂರ್ಣಾಂಕ iTabCur,    /* The table cursor.  Or the PK cursor for WITHOUT ROWID */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,       /* Index of the column to extract */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು      /* Extract the value into this register */
){
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು!=XN_ಅಭಿವ್ಯಕ್ತಿ );
  ಆದರೆ( i_ಲಂಬಸಾಲು<0 || i_ಲಂಬಸಾಲು==pTab->iPKey ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iTabCur, reg_ಹೊರಹೋಗು);
    VdbeComment((v, "%s.rowid", pTab->zName));
 }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ x;
    ಆದರೆ( IsVirtual(pTab) ){
      x = i_ಲಂಬಸಾಲು;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_VColumn, iTabCur, x, reg_ಹೊರಹೋಗು);
      ಹಿಂತಿರುಗಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p_ಲಂಬಸಾಲು = &pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು])->ಲಂಬಸಾಲು_Flags & COLFLAG_VIRTUAL ){
      Parse *pParse = sqlite3VdbeParser(v);
      ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_BUSY ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "generated column loop on \"%s\"",
                        p_ಲಂಬಸಾಲು->zCnName);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ savedSelfTab = pParse->iSelfTab;
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_BUSY;
        pParse->iSelfTab = iTabCur+1;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Generated_ಸ್ತಂಭ(pParse, pTab, p_ಲಂಬಸಾಲು, reg_ಹೊರಹೋಗು);
        pParse->iSelfTab = savedSelfTab;
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags &= ~COLFLAG_BUSY;
      }
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !HasRowid(pTab) ){
      testcase( i_ಲಂಬಸಾಲು!=sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು) );
      x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab), i_ಲಂಬಸಾಲು);
    }ಇಲ್ಲದಿದ್ದರೆ{
      x = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab,i_ಲಂಬಸಾಲು);
      testcase( x!=i_ಲಂಬಸಾಲು );
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iTabCur, x, reg_ಹೊರಹೋಗು);
    sqlite3_ಸ್ತಂಭ_Default(v, pTab, i_ಲಂಬಸಾಲು, reg_ಹೊರಹೋಗು);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in register iReg.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ(
  Parse *pParse,   /* Parsing and code generating context */
  ಕೋಷ್ಟಕ *pTab,     /* Description of the table we are reading from */
  ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ,     /* Index of the table column */
  ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ,      /* The cursor pointing to the table */
  ಪೂರ್ಣಾಂಕ iReg,        /* Store results here */
  u8 p5            /* P5 value for OP_Column + FLAGS */
){
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );
  ಪ್ರತಿಜ್ಞಾ( (p5 & (OPFLAG_NOCHNG|OPFLAG_TYPEOFARG|OPFLAG_LENGTHARG))==p5 );
  ಪ್ರತಿಜ್ಞಾ( IsVirtual(pTab) || (p5 & OPFLAG_NOCHNG)==0 );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(pParse->pVdbe, pTab, i_ಕೋಷ್ಟಕ, i_ಸ್ತಂಭ, iReg);
  ಆದರೆ( p5 ){
    VdbeOp *pOp = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(pParse->pVdbe);
    ಆದರೆ( pOp->opcode==OP_ಸ್ತಂಭ ) pOp->p5 = p5;
    ಆದರೆ( pOp->opcode==OP_VColumn ) pOp->p5 = (p5 & OPFLAG_NOCHNG);
  }
  ಹಿಂತಿರುಗಿ iReg;
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(Parse *pParse, ಪೂರ್ಣಾಂಕ iFrom, ಪೂರ್ಣಾಂಕ i_ಗಮ್ಯ, ಪೂರ್ಣಾಂಕ nReg){
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(pParse->pVdbe, OP_Move, iFrom, i_ಗಮ್ಯ, nReg);
}

/*
** Convert a scalar expression node to a TK_REGISTER referencing
** register iReg.  The caller must ensure that iReg already contains
** the correct value for the expression.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ iReg){
  ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( NEVER(p==0) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p->op==TK_ರಿಜಿಸ್ಟರ್ ){
    ಪ್ರತಿಜ್ಞಾ( p->i_ಕೋಷ್ಟಕ==iReg );
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->op2 = p->op;
    p->op = TK_ರಿಜಿಸ್ಟರ್;
    p->i_ಕೋಷ್ಟಕ = iReg;
    ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p, EP_Skip);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸದಿಶ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ *piFreeable){
  ಪೂರ್ಣಾಂಕ i_ಫಲಿತಾಂಶ;
  ಪೂರ್ಣಾಂಕ n_ಫಲಿತಾಂಶ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p);
  ಆದರೆ( n_ಫಲಿತಾಂಶ==1 ){
    i_ಫಲಿತಾಂಶ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p, piFreeable);
  }ಇಲ್ಲದಿದ್ದರೆ{
    *piFreeable = 0;
    ಆದರೆ( p->op==TK_SELECT ){
#ಆದರೆ SQLITE_OMIT_SUBQUERY
      i_ಫಲಿತಾಂಶ = 0;
#ಇಲ್ಲದಿದ್ದರೆ
      i_ಫಲಿತಾಂಶ = sqlite3_ಸಂಕೇತ_Subselect(pParse, p);
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ i;
      i_ಫಲಿತಾಂಶ = pParse->nMem+1;
      pParse->nMem += n_ಫಲಿತಾಂಶ;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      ಗಾಗಿ(i=0; i<n_ಫಲಿತಾಂಶ; i++){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(pParse, p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, i+i_ಫಲಿತಾಂಶ);
      }
    }
  }
  ಹಿಂತಿರುಗಿ i_ಫಲಿತಾಂಶ;
}

/*
** If the last opcode is a OP_Copy, then set the do-not-merge flag (p5)
** so that a subsequent copy will not be merged into this one.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Do_Not_ವಿಲೀನ_Flag_On_Copy(Vdbe *v){
  ಆದರೆ( sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v)->opcode==OP_Copy ){
    sqlite3VdbeChangeP5(v, 1);  /* Tag trailing OP_Copy as not mergeable */
  }
}

/*
** Generate code to implement special SQL functions that are implemented
** in-line rather than by using the usual callbacks.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Inline_Function(
  Parse *pParse,        /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pFarg,      /* List of function arguments */
  ಪೂರ್ಣಾಂಕ iFuncId,          /* Function ID.  One of the INTFUNC_... values */
  ಪೂರ್ಣಾಂಕ ಗುರಿ            /* Store function result in this register */
){
  ಪೂರ್ಣಾಂಕ nFarg;
  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFarg!=0 );
  nFarg = pFarg->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( nFarg>0 );  /* All in-line functions have at least one argument */
  ಆಯ್ಕೆ( iFuncId ){
    ಸ್ಥಿತಿ INLINEFUNC_coalesce: {
      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evaluation of
      ** arguments past the first non-NULL argument.
      */
      ಪೂರ್ಣಾಂಕ ಕೊನೆ_Coalesce = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ i;
      ಪ್ರತಿಜ್ಞಾ( nFarg>=2 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
      ಗಾಗಿ(i=1; i<nFarg; i++){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NotNull, ಗುರಿ, ಕೊನೆ_Coalesce);
        VdbeCoverage(v);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
      }
      ಗಣ_Do_Not_ವಿಲೀನ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, ಕೊನೆ_Coalesce);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ INLINEFUNC_iif: {
      ಅಭಿವ್ಯಕ್ತಿ case_ಅಭಿವ್ಯಕ್ತಿ;
      memset(&case_ಅಭಿವ್ಯಕ್ತಿ, 0, ಗಾತ್ರ(case_ಅಭಿವ್ಯಕ್ತಿ));
      case_ಅಭಿವ್ಯಕ್ತಿ.op = TK_CASE;
      case_ಅಭಿವ್ಯಕ್ತಿ.x.pList = pFarg;
      ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, &case_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಥಾನಾಂತರ_SQL_FUNC
    ಸ್ಥಿತಿ INLINEFUNC_sqlite_ಸ್ಥಾನಾಂತರ: {
      ಅಭಿವ್ಯಕ್ತಿ *pArg = pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( pArg->op==TK_ಸ್ತಂಭ && pArg->i_ಕೋಷ್ಟಕ>=0 ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ಥಾನಾಂತರ, pArg->i_ಕೋಷ್ಟಕ, pArg->i_ಸ್ತಂಭ, ಗುರಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ವನಿಯೋಜಿತ: {  
      /* The UNLIKELY() function is a no-op.  The result is the value
      ** of the first argument.
      */
      ಪ್ರತಿಜ್ಞಾ( nFarg==1 || nFarg==2 );
      ಗುರಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
      ನಿಲ್ಲಿಸು;
    }

  /***********************************************************************
  ** Test-only SQL functions that are only usable if enabled
  ** via SQLITE_TESTCTRL_INTERNAL_FUNCTIONS
  */
#ಆದರೆ !defined(SQLITE_UNTESTABLE)
    ಸ್ಥಿತಿ INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_compare: {
      /* Compare two expressions using sqlite3ExprCompare() */
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer,
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ,-1),
         ಗುರಿ);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ: {
      /* Compare two expressions using sqlite3ExprImpliesExpr() */
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer,
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(pParse,pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ,-1),
         ಗುರಿ);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ INLINEFUNC_ಸೂಚಿಸುತ್ತದೆ_nonnull_ಅಡ್ಡಸಾಲು: {
      /* Result of sqlite3ExprImpliesNonNullRow() */
      ಅಭಿವ್ಯಕ್ತಿ *pA1;
      ಪ್ರತಿಜ್ಞಾ( nFarg==2 );
      pA1 = pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( pA1->op==TK_ಸ್ತಂಭ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer,
           sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Non_Null_ಅಡ್ಡಸಾಲು(pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,pA1->i_ಕೋಷ್ಟಕ,1),
           ಗುರಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
      }
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ INLINEFUNC_affinity: {
      /* The AFFINITY() function evaluates to a string that describes
      ** the type affinity of the argument.  This is used for testing of
      ** the SQLite type logic.
      */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azAff[] = { "blob", "text", "numeric", "integer",
                              "real", "flexnum" };
      ಅಕ್ಷರ aff;
      ಪ್ರತಿಜ್ಞಾ( nFarg==1 );
      aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
      ಪ್ರತಿಜ್ಞಾ( aff<=SQLITE_AFF_NONE
           || (aff>=SQLITE_AFF_BLOB && aff<=SQLITE_AFF_FLEXNUM) );
      sqlite3_Vdbe_ತುಂಬು_String(v, ಗುರಿ,
              (aff<=SQLITE_AFF_NONE) ? "none" : azAff[aff-SQLITE_AFF_BLOB]);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_UNTESTABLE) */
  }
  ಹಿಂತಿರುಗಿ ಗುರಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Can_Return_Subtype(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ n;
  FuncDef *pDef;
  sqlite3 *db;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_UPLUS 
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CAST 
  ){
    ಹಿಂತಿರುಗಿ WRC_Continue;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ){
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  db = pWalker->pParse->db;
  n = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, n, ENC(db), 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ಆದರೆ( pDef==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pDef!=0 );
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( (pDef->funcFlags & SQLITE_ಫಲಿತಾಂಶ_SUBTYPE)!=0 ){
    pWalker->e_ಸಂಕೇತ = 1;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Return TRUE if expression pExpr is able to return a subtype.
**
** A TRUE return does not guarantee that a subtype will be returned.
** It only indicates that a subtype return is possible.  False positives
** are acceptable as they only disable an optimization.  False negatives,
** on the other hand, can lead to incorrect answers.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Walker w;
  memset(&w, 0, ಗಾತ್ರ(w));
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_ಗಂಟು_Can_Return_Subtype;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ;
}


/*
** Check to see if pExpr is one of the indexed expressions on pParse->pIdxEpr.
** If it is, then resolve the expression by reading from the index and
** return the register into which the value has been read.  If pExpr is
** not an indexed expression, then return negative.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ_Lookup(
  Parse *pParse,   /* The parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* The expression to potentially bypass */
  ಪೂರ್ಣಾಂಕ ಗುರಿ       /* Where to store the result of the expression */
){
  ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *p;
  Vdbe *v;
  ಗಾಗಿ(p=pParse->pIdxEpr; p; p=p->pIENext){
    u8 ಅಭಿವ್ಯಕ್ತಿ_Aff;
    ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur = p->i_ದತ್ತಾಂಶ_Cur;
    ಆದರೆ( i_ದತ್ತಾಂಶ_Cur<0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pParse->iSelfTab ){
      ಆದರೆ( p->i_ದತ್ತಾಂಶ_Cur!=pParse->iSelfTab-1 ) ಮುಂದುವರಿ;
      i_ದತ್ತಾಂಶ_Cur = -1;
    }
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, p->p_ಅಭಿವ್ಯಕ್ತಿ, i_ದತ್ತಾಂಶ_Cur)!=0 ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( p->aff>=SQLITE_AFF_BLOB && p->aff<=SQLITE_AFF_NUMERIC );
    ಅಭಿವ್ಯಕ್ತಿ_Aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( (ಅಭಿವ್ಯಕ್ತಿ_Aff<=SQLITE_AFF_BLOB && p->aff!=SQLITE_AFF_BLOB)
     || (ಅಭಿವ್ಯಕ್ತಿ_Aff==SQLITE_AFF_ಪಠ್ಯ && p->aff!=SQLITE_AFF_ಪಠ್ಯ)
     || (ಅಭಿವ್ಯಕ್ತಿ_Aff>=SQLITE_AFF_NUMERIC && p->aff!=SQLITE_AFF_NUMERIC)
    ){
      /* Affinity mismatch on a generated column */
      ಮುಂದುವರಿ;
    }


    /* Functions that might set a subtype should not be replaced by the
    ** value taken from an expression index if they are themselves an
    ** argument to another scalar function or aggregate. 
    ** https://sqlite.org/forum/forumpost/68d284c86b082c3e */
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg)
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) 
    ){
      ಮುಂದುವರಿ;
    }

    v = pParse->pVdbe;
    ಪ್ರತಿಜ್ಞಾ( v!=0 );
    ಆದರೆ( p->b_Maybe_Null_ಅಡ್ಡಸಾಲು || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
      /* The value of pExpr should be computed from the original expression,
      ** not from the index, if either:
      **
      **    1.   The index might be on a NULL row due to an outer join.
      **
      **    2.   The expression is a constant.  This prevents an attempt to
      **         use an index value outside of the loop that positions the
      **         index.  (Bug 2026-10-11T19:26:40Z).
      */
      ಪೂರ್ಣಾಂಕ addr = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_If_Null_ಅಡ್ಡಸಾಲು, p->iIdxCur, addr+3, ಗುರಿ);
      VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, p->iIdxCur, p->i_Idx_ಲಂಬಸಾಲು, ಗುರಿ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ಲಂಬಸಾಲು));
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
      pParse->pIdxEpr = p;
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr+2);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, p->iIdxCur, p->i_Idx_ಲಂಬಸಾಲು, ಗುರಿ);
      VdbeComment((v, "%s expr-column %d", p->zIdxName, p->i_Idx_ಲಂಬಸಾಲು));
    }
    ಹಿಂತಿರುಗಿ ಗುರಿ;
  }
  ಹಿಂತಿರುಗಿ -1;  /* Not found */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Partidx_ಅಭಿವ್ಯಕ್ತಿ_Lookup(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ i_ಗುರಿ){
  ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *p;
  ಗಾಗಿ(p=pParse->p_Idx_Part_ಅಭಿವ್ಯಕ್ತಿ; p; p=p->pIENext){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ==p->i_Idx_ಲಂಬಸಾಲು && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==p->i_ದತ್ತಾಂಶ_Cur ){
      Vdbe *v = pParse->pVdbe;
      ಪೂರ್ಣಾಂಕ addr = 0;
      ಪೂರ್ಣಾಂಕ ret;

      ಆದರೆ( p->b_Maybe_Null_ಅಡ್ಡಸಾಲು ){
        addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_If_Null_ಅಡ್ಡಸಾಲು, p->iIdxCur);
      }
      ret = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p->p_ಅಭಿವ್ಯಕ್ತಿ, i_ಗುರಿ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(pParse->pVdbe, OP_Affinity, ret, 1, 0,
                        (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&p->aff, 1);
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pParse, p->p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* If the expression value may have a sub-type, clear it. Values
        ** read from columns do not have subtypes. */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(pParse->pVdbe, OP_ClrSubtype, ret);
      }
      ಆದರೆ( addr ){
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      ಹಿಂತಿರುಗಿ ret;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ_And_Or(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,       /* AND or OR expression to be coded */
  ಪೂರ್ಣಾಂಕ ಗುರಿ,        /* Put result in this register, guaranteed */
  ಪೂರ್ಣಾಂಕ *pTmpReg       /* Write a temporary register here */
){
  ಪೂರ್ಣಾಂಕ op;            /* The opcode.  TK_AND or TK_OR */
  ಪೂರ್ಣಾಂಕ skipOp;        /* Opcode for the branch that skips one operand */
  ಪೂರ್ಣಾಂಕ addrSkip;      /* Branch instruction that skips one of the operands */
  ಪೂರ್ಣಾಂಕ regSS = 0;     /* Register holding computed operand when other omitted */
  ಪೂರ್ಣಾಂಕ r1, r2;        /* Registers for left and right operands, respectively */
  ಅಭಿವ್ಯಕ್ತಿ *pAlt;        /* Alternative, simplified expression */
  Vdbe *v;           /* statement being coded */

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಪ್ರತಿಜ್ಞಾ( op==TK_AND || op==TK_OR );
  ಪ್ರತಿಜ್ಞಾ( TK_AND==OP_And );            testcase( op==TK_AND );
  ಪ್ರತಿಜ್ಞಾ( TK_OR==OP_Or );              testcase( op==TK_OR );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );
  v = pParse->pVdbe;
  pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, pAlt, ಗುರಿ);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_And, r1, r1, ಗುರಿ);
    ಹಿಂತಿರುಗಿ ಗುರಿ;
  }
  skipOp = op==TK_AND ? OP_IfNot : OP_If;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_ಮೊದಲ(p_ಅಭಿವ್ಯಕ್ತಿ) ){
    /* Compute the right operand first.  Skip the computation of the left
    ** operand if the right operand fully determines the result */
    r2 = regSS = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, ಗುರಿ);
    addrSkip = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, skipOp, r2);
    VdbeComment((v, "skip left operand"));
    VdbeCoverage(v); 
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, pTmpReg);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Compute the left operand first */
    r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, ಗುರಿ);
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, EP_Subquery) ){
      /* Skip over the computation of the right operand if the right
      ** operand is a subquery and the left operand completely determines
      ** the result */
      regSS = r1;
      addrSkip = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, skipOp, r1);
      VdbeComment((v, "skip right operand"));
      VdbeCoverage(v);
    }ಇಲ್ಲದಿದ್ದರೆ{
      addrSkip = regSS = 0;
    }
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, pTmpReg);
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, op, r2, r1, ಗುರಿ);
  testcase( (*pTmpReg)==0 );
  ಆದರೆ( addrSkip ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrSkip);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Or, regSS, regSS, ಗುರಿ);
    VdbeComment((v, "short-circut value"));
  }
  ಹಿಂತಿರುಗಿ ಗುರಿ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ ಗುರಿ){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  ಪೂರ್ಣಾಂಕ op;                   /* The opcode being coded */
  ಪೂರ್ಣಾಂಕ ಒಳ_Reg = ಗುರಿ;       /* Results stored in register inReg */
  ಪೂರ್ಣಾಂಕ regFree1 = 0;         /* If non-zero free this temporary register */
  ಪೂರ್ಣಾಂಕ regFree2 = 0;         /* If non-zero free this temporary register */
  ಪೂರ್ಣಾಂಕ r1, r2;               /* Various register numbers */
  ಅಭಿವ್ಯಕ್ತಿ ತಾತ್ಕಾಲಿಕ_X;               /* Temporary expression node */
  ಪೂರ್ಣಾಂಕ p5 = 0;

  ಪ್ರತಿಜ್ಞಾ( ಗುರಿ>0 && ಗುರಿ<=pParse->nMem );
  ಪ್ರತಿಜ್ಞಾ( v!=0 );

ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_doover:
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    op = TK_NULL;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->pIdxEpr!=0
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_ಎಲೆ)
   && (r1 = sqlite3_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ_Lookup(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ))>=0
  ){
    ಹಿಂತಿರುಗಿ r1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
    op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  }
  ಪ್ರತಿಜ್ಞಾ( op!=TK_ಕ್ರಮ );
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ TK_AGG_ಸ್ತಂಭ: {
      AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *p_ಲಂಬಸಾಲು;
      ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 );
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pAggInfo->n_ಸ್ತಂಭ ){
        /* Happens when the left table of a RIGHT JOIN is null and
        ** is using an expression index */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
        /* Verify that the OP_Null above is exercised by tests
        ** tag-20230325-2 */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_NotNull, ಗುರಿ, 1, 20230325);
        Vdbe_Coverage_Never_ತೆಗೆದುಕೊಂಡ(v);
#ಆದರೆ_ಅಂತ್ಯ
        ನಿಲ್ಲಿಸು;
      }
      p_ಲಂಬಸಾಲು = &pAggInfo->a_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg];
      ಆದರೆ( !pAggInfo->ನೇರ_ಬಹುಲಕ ){
        ಹಿಂತಿರುಗಿ Agg_Info_ಸ್ತಂಭ_Reg(pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pAggInfo->use_ವಿಂಗಡಣೆ_Idx ){
        ಕೋಷ್ಟಕ *pTab = p_ಲಂಬಸಾಲು->pTab;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, pAggInfo->ವಿಂಗಡಣೆ_Idx_PTab,
                              p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ, ಗುರಿ);
        ಆದರೆ( pTab==0 ){
          /* No comment added */
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ<0 ){
          VdbeComment((v,"%s.rowid",pTab->zName));
        }ಇಲ್ಲದಿದ್ದರೆ{
          VdbeComment((v,"%s.%s",
              pTab->zName, pTab->a_ಲಂಬಸಾಲು[p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ].zCnName));
          ಆದರೆ( pTab->a_ಲಂಬಸಾಲು[p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ].affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, ಗುರಿ);
          }
        }
        ಹಿಂತಿರುಗಿ ಗುರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab==0 ){
        /* This case happens when the argument to an aggregate function
        ** is rewritten by aggregateConvertIndexedExprRefToColumn() */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, ಗುರಿ);
        ಹಿಂತಿರುಗಿ ಗುರಿ;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
      /* no break */ deliberate_fall_through
    }
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      ಪೂರ್ಣಾಂಕ iTab = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
      ಪೂರ್ಣಾಂಕ iReg;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು) ){
        /* This COLUMN expression is really a constant due to WHERE clause
        ** constraints, and that constant is coded by the pExpr->pLeft
        ** expression.  However, make sure the constant has the correct
        ** datatype by applying the Affinity of the table column to the
        ** constant.
        */
        ಪೂರ್ಣಾಂಕ aff;
        iReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ,ಗುರಿ);
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
        aff = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
        ಆದರೆ( aff>SQLITE_AFF_BLOB ){
          ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zAff[] = "B\000C\000D\000E\000F";
          ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_BLOB=='A' );
          ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_ಪಠ್ಯ=='B' );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, iReg, 1, 0,
                            &zAff[(aff-'B')*2], P4_STATIC);
        }
        ಹಿಂತಿರುಗಿ iReg;
      }
      ಆದರೆ( iTab<0 ){
        ಆದರೆ( pParse->iSelfTab<0 ){
          /* Other columns in the same row for CHECK constraints or
          ** generated columns or for inserting into partial index.
          ** The row is unpacked into registers beginning at
          ** 0-(pParse->iSelfTab).  The rowid (if any) is in a register
          ** immediately prior to the first column.
          */
          ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
          ಕೋಷ್ಟಕ *pTab;
          ಪೂರ್ಣಾಂಕ iSrc;
          ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
          pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
          ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
          ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=XN_ROWID );
          ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು );
          ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
            ಹಿಂತಿರುಗಿ -1-pParse->iSelfTab;
          }
          p_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು + i_ಲಂಬಸಾಲು;
          testcase( i_ಲಂಬಸಾಲು!=sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab,i_ಲಂಬಸಾಲು) );
          iSrc = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು) - pParse->iSelfTab;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
          ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED ){
            ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_BUSY ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "generated column loop on \"%s\"",
                              p_ಲಂಬಸಾಲು->zCnName);
              ಹಿಂತಿರುಗಿ 0;
            }
            p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_BUSY;
            ಆದರೆ( p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_NOTAVAIL ){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Generated_ಸ್ತಂಭ(pParse, pTab, p_ಲಂಬಸಾಲು, iSrc);
            }
            p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags &= ~(COLFLAG_BUSY|COLFLAG_NOTAVAIL);
            ಹಿಂತಿರುಗಿ iSrc;
          }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_GENERATED_COLUMNS */
          ಆದರೆ( p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_REAL ){
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy, iSrc, ಗುರಿ);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, ಗುರಿ);
            ಹಿಂತಿರುಗಿ ಗುರಿ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಹಿಂತಿರುಗಿ iSrc;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Coding an expression that is part of an index where column names
          ** in the index refer to the table to which the index belongs */
          iTab = pParse->iSelfTab - 1;
        }
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pParse->p_Idx_Part_ಅಭಿವ್ಯಕ್ತಿ 
       && 0!=(r1 = ಅಭಿವ್ಯಕ್ತಿ_Partidx_ಅಭಿವ್ಯಕ್ತಿ_Lookup(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ))
      ){
        ಹಿಂತಿರುಗಿ r1;
      }
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 );
      iReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab,
                               p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, iTab, ಗುರಿ,
                               p_ಅಭಿವ್ಯಕ್ತಿ->op2);
      ಹಿಂತಿರುಗಿ iReg;
    }
    ಸ್ಥಿತಿ TK_INTEGER: {
      ಸಂಕೇತ_Integer(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, 0, ಗುರಿ);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
    ಸ್ಥಿತಿ TK_TRUEFALSE: {
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ), ಗುರಿ);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
    ಸ್ಥಿತಿ TK_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಸಂಕೇತ_Real(v, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0, ಗುರಿ);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_STRING: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_Vdbe_ತುಂಬು_String(v, ಗುರಿ, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
    ಸ್ಥಿತಿ TK_NULLS: {
      /* Set a range of registers to NULL.  pExpr->y.nReg registers starting
      ** with target */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, ಗುರಿ, ಗುರಿ + p_ಅಭಿವ್ಯಕ್ತಿ->y.nReg - 1);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      /* Make NULL the default case so that if a bug causes an illegal
      ** Expr node to be passed into this function, it will be handled
      ** sanely and not crash.  But keep the assert() to bring the problem
      ** to the attention of the developers. */
      ಪ್ರತಿಜ್ಞಾ( op==TK_NULL || op==TK_ERROR || pParse->db->mallocFailed );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_BLOB_LITERAL
    ಸ್ಥಿತಿ TK_BLOB: {
      ಪೂರ್ಣಾಂಕ n;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
      ಅಕ್ಷರ *zBlob;
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='x' || p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]=='X' );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[1]=='\'' );
      z = &p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      ಪ್ರತಿಜ್ಞಾ( z[n]=='\'' );
      zBlob = sqlite3_Hex_ಗಮ್ಯ_Blob(sqlite3VdbeDb(v), z, n);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Blob, n/2, ಗುರಿ, 0, zBlob, P4_DYNAMIC);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_VARIABLE: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken[0]!=0 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Variable, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, ಗುರಿ);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
    ಸ್ಥಿತಿ TK_ರಿಜಿಸ್ಟರ್: {
      ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
    ಸ್ಥಿತಿ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, ಗುರಿ);
      ಪ್ರತಿಜ್ಞಾ( ಒಳ_Reg==ಗುರಿ );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Cast, ಗುರಿ,
                        sqlite3AffinityType(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, 0));
      ಹಿಂತಿರುಗಿ ಒಳ_Reg;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CAST */
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ISNOT:
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      p5 = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_EQ: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪೂರ್ಣಾಂಕ addrIsNull = 0;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಎಡ) ){
        ಸಂಕೇತ_ಸದಿಶ_Compare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ, op, p5);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && p5!=SQLITE_NULLEQ ){
          addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                     &r1, &r2, &regFree1, &regFree2);
        }ಇಲ್ಲದಿದ್ದರೆ{
          r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
          r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, ಒಳ_Reg);
        ಸಂಕೇತ_Compare(pParse, p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op, r1, r2,
            sqlite3VdbeCurrentAddr(v)+2, p5,
            ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted));
        ಪ್ರತಿಜ್ಞಾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
        ಪ್ರತಿಜ್ಞಾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
        ಪ್ರತಿಜ್ಞಾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
        ಪ್ರತಿಜ್ಞಾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
        ಪ್ರತಿಜ್ಞಾ(TK_EQ==OP_Eq); testcase(op==OP_Eq); VdbeCoverageIf(v,op==OP_Eq);
        ಪ್ರತಿಜ್ಞಾ(TK_NE==OP_Ne); testcase(op==OP_Ne); VdbeCoverageIf(v,op==OP_Ne);
        ಆದರೆ( p5==SQLITE_NULLEQ ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, ಒಳ_Reg);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ZeroOrNull, r1, ಒಳ_Reg, r2);
          ಆದರೆ( addrIsNull ){
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
            sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrIsNull);
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಒಳ_Reg);
          }
        }
        testcase( regFree1==0 );
        testcase( regFree2==0 );
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_AND:
    ಸ್ಥಿತಿ TK_OR: {
      ಒಳ_Reg = ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ_And_Or(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ, &regFree1);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_PLUS:
    ಸ್ಥಿತಿ TK_ನಕ್ಷತ್ರ:
    ಸ್ಥಿತಿ TK_MINUS:
    ಸ್ಥಿತಿ TK_REM:
    ಸ್ಥಿತಿ TK_BITAND:
    ಸ್ಥಿತಿ TK_BITOR:
    ಸ್ಥಿತಿ TK_SLASH:
    ಸ್ಥಿತಿ TK_LSHIFT:
    ಸ್ಥಿತಿ TK_RSHIFT:
    ಸ್ಥಿತಿ TK_CONCAT: {
      ಪೂರ್ಣಾಂಕ addrIsNull;
      ಪ್ರತಿಜ್ಞಾ( TK_PLUS==OP_ಕೂಡಿಸು );           testcase( op==TK_PLUS );
      ಪ್ರತಿಜ್ಞಾ( TK_MINUS==OP_ಕಳೆ );     testcase( op==TK_MINUS );
      ಪ್ರತಿಜ್ಞಾ( TK_REM==OP_Remainder );      testcase( op==TK_REM );
      ಪ್ರತಿಜ್ಞಾ( TK_BITAND==OP_ಬಿಟ್_And );      testcase( op==TK_BITAND );
      ಪ್ರತಿಜ್ಞಾ( TK_BITOR==OP_ಬಿಟ್_Or );        testcase( op==TK_BITOR );
      ಪ್ರತಿಜ್ಞಾ( TK_SLASH==OP_Divide );       testcase( op==TK_SLASH );
      ಪ್ರತಿಜ್ಞಾ( TK_LSHIFT==OP_ಸರಿಸು_ಎಡ );   testcase( op==TK_LSHIFT );
      ಪ್ರತಿಜ್ಞಾ( TK_RSHIFT==OP_ಸರಿಸು_ಬಲ );  testcase( op==TK_RSHIFT );
      ಪ್ರತಿಜ್ಞಾ( TK_CONCAT==OP_Concat );      testcase( op==TK_CONCAT );
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, op, r2, r1, ಗುರಿ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ಆದರೆ( addrIsNull ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrIsNull);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
        VdbeComment((v, "short-circut value"));
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_UMINUS: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ );
      ಆದರೆ( p_ಎಡ->op==TK_INTEGER ){
        ಸಂಕೇತ_Integer(pParse, p_ಎಡ, 1, ಗುರಿ);
        ಹಿಂತಿರುಗಿ ಗುರಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಎಡ->op==TK_FLOAT ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        ಸಂಕೇತ_Real(v, p_ಎಡ->u.zToken, 1, ಗುರಿ);
        ಹಿಂತಿರುಗಿ ಗುರಿ;
#ಆದರೆ_ಅಂತ್ಯ
      }ಇಲ್ಲದಿದ್ದರೆ{
        ತಾತ್ಕಾಲಿಕ_X.op = TK_INTEGER;
        ತಾತ್ಕಾಲಿಕ_X.flags = EP_Int_ಮೌಲ್ಯ|EP_TokenOnly;
        ತಾತ್ಕಾಲಿಕ_X.u.i_ಮೌಲ್ಯ = 0;
        ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(&ತಾತ್ಕಾಲಿಕ_X);
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, &ತಾತ್ಕಾಲಿಕ_X, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree2);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಕಳೆ, r2, r1, ಗುರಿ);
        testcase( regFree2==0 );
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_BITNOT:
    ಸ್ಥಿತಿ TK_NOT: {
      ಪ್ರತಿಜ್ಞಾ( TK_BITNOT==OP_ಬಿಟ್_Not );   testcase( op==TK_BITNOT );
      ಪ್ರತಿಜ್ಞಾ( TK_NOT==OP_Not );         testcase( op==TK_NOT );
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, op, r1, ಒಳ_Reg);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRUTH: {
      ಪೂರ್ಣಾಂಕ isTrue;    /* IS TRUE or IS NOT TRUE */
      ಪೂರ್ಣಾಂಕ b_ಸಾಮಾನ್ಯ;   /* IS TRUE or IS FALSE */
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      b_ಸಾಮಾನ್ಯ = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IS;
      testcase( isTrue && b_ಸಾಮಾನ್ಯ);
      testcase( !isTrue && b_ಸಾಮಾನ್ಯ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IsTrue, r1, ಒಳ_Reg, !isTrue, isTrue ^ b_ಸಾಮಾನ್ಯ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ISNULL:
    ಸ್ಥಿತಿ TK_NOTNULL: {
      ಪೂರ್ಣಾಂಕ addr;
      ಪ್ರತಿಜ್ಞಾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, ಗುರಿ);
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, op, r1);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, ಗುರಿ);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_AGG_FUNCTION: {
      AggInfo *pInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ಆದರೆ( pInfo==0
       || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg<0)
       || NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pInfo->nFunc)
      ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aggregate: %#T()", p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ AggInfoFuncReg(pInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pFarg;       /* List of function arguments */
      ಪೂರ್ಣಾಂಕ nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zId;       /* The function name */
      u32 const_ಮುಖವಾಡ = 0;     /* Mask of function arguments that are constant */
      ಪೂರ್ಣಾಂಕ i;                 /* Loop counter */
      sqlite3 *db = pParse->db;  /* The database connection */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
        ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->reg_ಫಲಿತಾಂಶ;
      }
#ಆದರೆ_ಅಂತ್ಯ

      ಆದರೆ( Const_ಗುಣಕ_Ok(pParse)
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ)
      ){
        /* SQL functions can be expensive. So try to avoid running them
        ** multiple times if we know they always give the same result */
        ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, -1);
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      pFarg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
      nFarg = pFarg ? pFarg->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      zId = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, zId, nFarg, enc, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
      ಆದರೆ( pDef==0 && pParse->explain ){
        pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, "unknown", nFarg, enc, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( pDef==0
       || pDef->xFinalize!=0
       || ((pDef->funcFlags & SQLITE_FUNC_ಆಂತರಿಕ)!=0 &&
           !pParse->nested &&
           (db->mDbFlags & DBFLAG_ಆಂತರಿಕ_Func)==0)
      ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "unknown function: %#T()", p_ಅಭಿವ್ಯಕ್ತಿ);
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_INLINE)!=0 && ALWAYS(pFarg!=0) ){
        ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & SQLITE_FUNC_UNSAFE)==0 );
        ಪ್ರತಿಜ್ಞಾ( (pDef->funcFlags & SQLITE_FUNC_ನೇರ)==0 );
        ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Inline_Function(pParse, pFarg,
             SQLITE_PTR_ಗಮ್ಯ_INT(pDef->p_User_ದತ್ತಾಂಶ), ಗುರಿ);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pDef->funcFlags & (SQLITE_FUNC_ನೇರ|SQLITE_FUNC_UNSAFE) ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pDef);
      }

      ಗಾಗಿ(i=0; i<nFarg; i++){
        ಆದರೆ( i<32 && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ) ){
          testcase( i==31 );
          const_ಮುಖವಾಡ |= MASKBIT32(i);
        }
        ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pFarg->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
      }
      ಆದರೆ( pFarg ){
        ಆದರೆ( const_ಮುಖವಾಡ ){
          r1 = pParse->nMem+1;
          pParse->nMem += nFarg;
        }ಇಲ್ಲದಿದ್ದರೆ{
          r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nFarg);
        }

        /* For length() and typeof() and octet_length() functions,
        ** set the P5 parameter to the OP_Column opcode to OPFLAG_LENGTHARG
        ** or OPFLAG_TYPEOFARG or OPFLAG_BYTELENARG respectively, to avoid
        ** unnecessary data loading.
        */
        ಆದರೆ( (pDef->funcFlags & (SQLITE_FUNC_ಉದ್ದಳತೆ|SQLITE_FUNC_TYPEOF))!=0 ){
          u8 ಅಭಿವ್ಯಕ್ತಿ_Op;
          ಪ್ರತಿಜ್ಞಾ( nFarg==1 );
          ಪ್ರತಿಜ್ಞಾ( pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_Op = pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->op;
          ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Op==TK_ಸ್ತಂಭ || ಅಭಿವ್ಯಕ್ತಿ_Op==TK_AGG_ಸ್ತಂಭ ){
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ಉದ್ದಳತೆ==OPFLAG_LENGTHARG );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_TYPEOF==OPFLAG_TYPEOFARG );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_BYTELEN==OPFLAG_BYTELENARG );
            ಪ್ರತಿಜ್ಞಾ( (OPFLAG_LENGTHARG|OPFLAG_TYPEOFARG)==OPFLAG_BYTELENARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_LENGTHARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_TYPEOFARG );
            testcase( (pDef->funcFlags & OPFLAG_BYTELENARG)==OPFLAG_BYTELENARG);
            pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ->op2 = pDef->funcFlags & OPFLAG_BYTELENARG;
          }
        }

        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pFarg, r1, 0, SQLITE_ECEL_ಗುಣಕ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = 0;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
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
      ಆದರೆ( nFarg>=2 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( pDef->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        ಆದರೆ( !pColl ) pColl = db->pDfltColl;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_CollSeq, 0, 0, 0, (ಅಕ್ಷರ *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(pParse, const_ಮುಖವಾಡ, r1, ಗುರಿ, nFarg,
                                 pDef, p_ಅಭಿವ್ಯಕ್ತಿ->op2);
      ಆದರೆ( nFarg ){
        ಆದರೆ( const_ಮುಖವಾಡ==0 ){
          sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r1, nFarg);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3VdbeReleaseRegisters(pParse, r1, nFarg, const_ಮುಖವಾಡ, 1);
        }
      }
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_EXISTS:
    ಸ್ಥಿತಿ TK_SELECT: {
      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      ಆದರೆ( pParse->db->mallocFailed ){
        ಹಿಂತಿರುಗಿ 0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_SELECT
             && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) )
             && (n_ಲಂಬಸಾಲು = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ)!=1
      ){
        sqlite3SubselectError(pParse, n_ಲಂಬಸಾಲು, 1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ sqlite3_ಸಂಕೇತ_Subselect(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_SELECT_ಸ್ತಂಭ: {
      ಪೂರ್ಣಾಂಕ n;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಆದರೆ( p_ಎಡ->i_ಕೋಷ್ಟಕ==0 || pParse->withinRJSubrtn > p_ಎಡ->op2 ){
        p_ಎಡ->i_ಕೋಷ್ಟಕ = sqlite3_ಸಂಕೇತ_Subselect(pParse, p_ಎಡ);
        p_ಎಡ->op2 = pParse->withinRJSubrtn;
      }
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op==TK_SELECT || p_ಎಡ->op==TK_ERROR );
      n = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಎಡ);
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ!=n ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "%d columns assigned %d values",
                                p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, n);
      }
      ಹಿಂತಿರುಗಿ p_ಎಡ->i_ಕೋಷ್ಟಕ + p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
    }
    ಸ್ಥಿತಿ TK_ಒಳ: {
      ಪೂರ್ಣಾಂಕ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, destIfFalse, destIfNull);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, ಗುರಿ);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, ಗುರಿ, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */


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
    ಸ್ಥಿತಿ TK_BETWEEN: {
      ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ, 0, 0);
      ಹಿಂತಿರುಗಿ ಗುರಿ;
    }
    ಸ್ಥಿತಿ TK_COLLATE: {
      ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ){
        /* A TK_COLLATE Expr node without the EP_Collate tag is a so-called
        ** "SOFT-COLLATE" that is added to constraints that are pushed down
        ** from outer queries into sub-queries by the WHERE-clause push-down
        ** optimization. Clear subtypes as subtypes may not cross a subquery
        ** boundary.
        */
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, ಗುರಿ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ClrSubtype, ಗುರಿ);
        ಹಿಂತಿರುಗಿ ಗುರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಹೋಗು ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_doover; /* 2018-04-28: Prevent deep recursion. */
      }
    }
    ಸ್ಥಿತಿ TK_ಹರವು:
    ಸ್ಥಿತಿ TK_UPLUS: {
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಹೋಗು ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_doover; /* 2018-04-28: Prevent deep recursion. OSSFuzz. */
    }

    ಸ್ಥಿತಿ TK_TRIGGER: {
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
      ಕೋಷ್ಟಕ *pTab;
      ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
      ಪೂರ್ಣಾಂಕ p1;

      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
      i_ಲಂಬಸಾಲು = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
      p1 = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ * (pTab->n_ಲಂಬಸಾಲು+1) + 1
                     + sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು);

      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==0 || p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==1 );
      ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=-1 && i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು );
      ಪ್ರತಿಜ್ಞಾ( pTab->iPKey<0 || i_ಲಂಬಸಾಲು!=pTab->iPKey );
      ಪ್ರತಿಜ್ಞಾ( p1>=0 && p1<(pTab->n_ಲಂಬಸಾಲು*2+2) );

      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Param, p1, ಗುರಿ);
      VdbeComment((v, "r[%d]=%s.%s", ಗುರಿ,
        (p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ? "new" : "old"),
        (p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ? "rowid" : p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName)
      ));

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.
      **
      ** EVIDENCE-OF: R-60985-57662 SQLite will convert the value back to
      ** floating point when extracting it from the record.  */
      ಆದರೆ( i_ಲಂಬಸಾಲು>=0 && pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].affinity==SQLITE_AFF_REAL ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_RealAffinity, ಗುರಿ);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ TK_ಸದಿಶ: {
      sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
      ನಿಲ್ಲಿಸು;
    }

    /* TK_IF_NULL_ROW Expr nodes are inserted ahead of expressions
    ** that derive from the right-hand table of a LEFT JOIN.  The
    ** Expr.iTable value is the table number for the right-hand table.
    ** The expression is only evaluated if that table is not currently
    ** on a LEFT JOIN NULL row.
    */
    ಸ್ಥಿತಿ TK_IF_NULL_ಅಡ್ಡಸಾಲು: {
      ಪೂರ್ಣಾಂಕ addrINR;
      u8 ok_Const_ಗುಣಕ = pParse->ok_Const_ಗುಣಕ;
      AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
      ಆದರೆ( pAggInfo ){
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 && p_ಅಭಿವ್ಯಕ್ತಿ->iAgg<pAggInfo->n_ಸ್ತಂಭ );
        ಆದರೆ( !pAggInfo->ನೇರ_ಬಹುಲಕ ){
          ಒಳ_Reg = Agg_Info_ಸ್ತಂಭ_Reg(pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
          ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo->use_ವಿಂಗಡಣೆ_Idx ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, pAggInfo->ವಿಂಗಡಣೆ_Idx_PTab,
                            pAggInfo->a_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg].i_Sorter_ಸ್ತಂಭ,
                            ಗುರಿ);
          ಒಳ_Reg = ಗುರಿ;
          ನಿಲ್ಲಿಸು;
        }
      }
      addrINR = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_If_Null_ಅಡ್ಡಸಾಲು, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, 0, ಗುರಿ);
      /* The OP_IfNullRow opcode above can overwrite the result register with
      ** NULL.  So we have to ensure that the result register is not a value
      ** that is suppose to be a constant.  Two defenses are needed:
      **   (1)  Temporarily disable factoring of constant expressions
      **   (2)  Make sure the computed value really is stored in register
      **        "target" and not someplace else.
      */
      pParse->ok_Const_ಗುಣಕ = 0;   /* note (1) above */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, ಗುರಿ);
      ಪ್ರತಿಜ್ಞಾ( ಗುರಿ==ಒಳ_Reg );
      pParse->ok_Const_ಗುಣಕ = ok_Const_ಗುಣಕ;
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrINR);
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ TK_CASE: {
      ಪೂರ್ಣಾಂಕ ಕೊನೆ_Label;                     /* GOTO label for end of CASE stmt */
      ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_Case;                     /* GOTO label for next WHEN clause */
      ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ;                        /* 2x number of WHEN terms */
      ಪೂರ್ಣಾಂಕ i;                            /* Loop counter */
      ಅಭಿವ್ಯಕ್ತಿ_List *pEList;                 /* List of WHEN terms */
      ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *aListelem;  /* Array of WHEN terms */
      ಅಭಿವ್ಯಕ್ತಿ opCompare;                   /* The X==Ei expression */
      ಅಭಿವ್ಯಕ್ತಿ *pX;                         /* The X expression */
      ಅಭಿವ್ಯಕ್ತಿ *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      ಅಭಿವ್ಯಕ್ತಿ *pDel = 0;
      sqlite3 *db = pParse->db;

      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) && p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0 );
      ಪ್ರತಿಜ್ಞಾ(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ > 0);
      pEList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
      aListelem = pEList->a;
      n_ಅಭಿವ್ಯಕ್ತಿ = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಕೊನೆ_Label = sqlite3VdbeMakeLabel(pParse);
      ಆದರೆ( (pX = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)!=0 ){
        pDel = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pX, 0);
        ಆದರೆ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDel);
          ನಿಲ್ಲಿಸು;
        }
        testcase( pX->op==TK_ಸ್ತಂಭ );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್(pDel, ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸದಿಶ(pParse, pDel, &regFree1));
        testcase( regFree1==0 );
        memset(&opCompare, 0, ಗಾತ್ರ(opCompare));
        opCompare.op = TK_EQ;
        opCompare.p_ಎಡ = pDel;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      ಗಾಗಿ(i=0; i<n_ಅಭಿವ್ಯಕ್ತಿ-1; i=i+2){
        ಆದರೆ( pX ){
          ಪ್ರತಿಜ್ಞಾ( pTest!=0 );
          opCompare.p_ಬಲ = aListelem[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pTest = aListelem[i].p_ಅಭಿವ್ಯಕ್ತಿ;
        }
        ಮುಂದಿನ_Case = sqlite3VdbeMakeLabel(pParse);
        testcase( pTest->op==TK_ಸ್ತಂಭ );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pTest, ಮುಂದಿನ_Case, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ );
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, aListelem[i+1].p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
        sqlite3VdbeGoto(v, ಕೊನೆ_Label);
        sqlite3VdbeResolveLabel(v, ಮುಂದಿನ_Case);
      }
      ಆದರೆ( (n_ಅಭಿವ್ಯಕ್ತಿ&1)!=0 ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pEList->a[n_ಅಭಿವ್ಯಕ್ತಿ-1].p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, ಗುರಿ);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDel);
      ಗಣ_Do_Not_ವಿಲೀನ_Flag_On_Copy(v);
      sqlite3VdbeResolveLabel(v, ಕೊನೆ_Label);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
    ಸ್ಥಿತಿ TK_RAISE: {
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Rollback
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Abort
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Fail
           || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Ignore
      );
      ಆದರೆ( !pParse->pTriggerTab && !pParse->nested ){
        sqlite3_Error_ಸಂದೇಶ(pParse,
                       "RAISE() may only be used within a trigger-program");
        ಹಿಂತಿರುಗಿ 0;
      }
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ==OE_Ignore ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Halt, SQLITE_OK, OE_Ignore);
        VdbeCoverage(v);
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Halt, 
             pParse->pTriggerTab ? SQLITE_CONSTRAINT_TRIGGER : SQLITE_ERROR,
             p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ, r1);
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree1);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree2);
  ಹಿಂತಿರುಗಿ ಒಳ_Reg;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(
  Parse *pParse,    /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* The expression to code when the VDBE initializes */
  ಪೂರ್ಣಾಂಕ regDest       /* Store the value in this register */
){
  ಅಭಿವ್ಯಕ್ತಿ_List *p;
  ಪ್ರತಿಜ್ಞಾ( Const_ಗುಣಕ_Ok(pParse) );
  ಪ್ರತಿಜ್ಞಾ( regDest!=0 );
  p = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
  ಆದರೆ( regDest<0 && p ){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(p_ಅಂಶ=p->a, i=p->n_ಅಭಿವ್ಯಕ್ತಿ; i>0; p_ಅಂಶ++, i--){
      ಆದರೆ( p_ಅಂಶ->fg.reusable
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ,p_ಅಭಿವ್ಯಕ್ತಿ,-1)==0
      ){
        ಹಿಂತಿರುಗಿ p_ಅಂಶ->u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg;
      }
    }
  }
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_HasFunc) ){
    Vdbe *v = pParse->pVdbe;
    ಪೂರ್ಣಾಂಕ addr;
    ಪ್ರತಿಜ್ಞಾ( v );
    addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
    pParse->ok_Const_ಗುಣಕ = 0;
    ಆದರೆ( !pParse->db->mallocFailed ){
      ಆದರೆ( regDest<0 ) regDest = ++pParse->nMem;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, regDest);
    }
    pParse->ok_Const_ಗುಣಕ = 1;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, p, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( p ){
       ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &p->a[p->n_ಅಭಿವ್ಯಕ್ತಿ-1];
       p_ಅಂಶ->fg.reusable = regDest<0;
       ಆದರೆ( regDest<0 ) regDest = ++pParse->nMem;
       p_ಅಂಶ->u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg = regDest;
    }
    pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ = p;
  }
  ಹಿಂತಿರುಗಿ regDest;
}

/*
** Make arrangements to invoke OP_Null on a range of registers
** during initialization.
*/
SQLITE_NOINLINE ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Null_ರಿಜಿಸ್ಟರ್_Range(
  Parse *pParse,   /* Parsing context */
  ಪೂರ್ಣಾಂಕ iReg,        /* First register to set to NULL */
  ಪೂರ್ಣಾಂಕ nReg         /* Number of sequential registers to NULL out */
){
  u8 ok_Const_ಗುಣಕ = pParse->ok_Const_ಗುಣಕ;
  ಅಭಿವ್ಯಕ್ತಿ t;
  memset(&t, 0, ಗಾತ್ರ(t));
  t.op = TK_NULLS;
  t.y.nReg = nReg;
  pParse->ok_Const_ಗುಣಕ = 1;
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(pParse, &t, iReg);
  pParse->ok_Const_ಗುಣಕ = ok_Const_ಗುಣಕ;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ *pReg){
  ಪೂರ್ಣಾಂಕ r2;
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಆದರೆ( Const_ಗುಣಕ_Ok(pParse)
   && ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ!=0)
   && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ರಿಜಿಸ್ಟರ್
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse, p_ಅಭಿವ್ಯಕ್ತಿ)
  ){
    *pReg  = 0;
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, -1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, r1);
    ಆದರೆ( r2==r1 ){
      *pReg = r1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      *pReg = 0;
    }
  }
  ಹಿಂತಿರುಗಿ r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ ಗುರಿ){
  ಪೂರ್ಣಾಂಕ ಒಳ_Reg;

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );
  ಪ್ರತಿಜ್ಞಾ( ಗುರಿ>0 && ಗುರಿ<=pParse->nMem );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 || pParse->db->mallocFailed );
  ಆದರೆ( pParse->pVdbe==0 ) ಹಿಂತಿರುಗಿ;
  ಒಳ_Reg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
  ಆದರೆ( ಒಳ_Reg!=ಗುರಿ ){
    u8 op;
    ಅಭಿವ್ಯಕ್ತಿ *pX = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ);
    testcase( pX!=p_ಅಭಿವ್ಯಕ್ತಿ );
    ಆದರೆ( ALWAYS(pX)
     && (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pX,EP_Subquery) || pX->op==TK_ರಿಜಿಸ್ಟರ್)
    ){
      op = OP_Copy;
    }ಇಲ್ಲದಿದ್ದರೆ{
      op = OP_SCopy;
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(pParse->pVdbe, op, ಒಳ_Reg, ಗುರಿ);
  }
}

/*
** Make a transient copy of expression pExpr and then code it using
** sqlite3ExprCode().  This routine works just like sqlite3ExprCode()
** except that the input expression is guaranteed to be unchanged.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ ಗುರಿ){
  sqlite3 *db = pParse->db;
  p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಆದರೆ( !db->mallocFailed ) sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.  If the expression is constant, then this routine
** might choose to code the expression at initialization time.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ ಗುರಿ){
  ಆದರೆ( pParse->ok_Const_ಗುಣಕ && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ);
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,     /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,   /* The expression list to be coded */
  ಪೂರ್ಣಾಂಕ ಗುರಿ,        /* Where to write results */
  ಪೂರ್ಣಾಂಕ srcReg,        /* Source registers if SQLITE_ECEL_REF */
  u8 flags           /* SQLITE_ECEL_* flags */
){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಪೂರ್ಣಾಂಕ i, j, n;
  u8 copyOp = (flags & SQLITE_ECEL_DUP) ? OP_Copy : OP_SCopy;
  Vdbe *v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಪ್ರತಿಜ್ಞಾ( ಗುರಿ>0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  ಆದರೆ( !Const_ಗುಣಕ_Ok(pParse) ) flags &= ~SQLITE_ECEL_ಗುಣಕ;
  ಗಾಗಿ(p_ಅಂಶ=pList->a, i=0; i<n; i++, p_ಅಂಶ++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    ಆದರೆ( p_ಅಂಶ->fg.bSorterRef ){
      i--;
      n--;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( (flags & SQLITE_ECEL_REF)!=0 && (j = p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು)>0 ){
      ಆದರೆ( flags & SQLITE_ECEL_OMITREF ){
        i--;
        n--;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, copyOp, j+srcReg-1, ಗುರಿ+i);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & SQLITE_ECEL_ಗುಣಕ)!=0
           && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Not_Join(pParse,p_ಅಭಿವ್ಯಕ್ತಿ)
    ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ+i);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ ಒಳ_Reg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, ಗುರಿ+i);
      ಆದರೆ( ಒಳ_Reg!=ಗುರಿ+i ){
        VdbeOp *pOp;
        ಆದರೆ( copyOp==OP_Copy
         && (pOp=sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v))->opcode==OP_Copy
         && pOp->p1+pOp->p3+1==ಒಳ_Reg
         && pOp->p2+pOp->p3+1==ಗುರಿ+i
         && pOp->p5==0  /* The do-not-merge flag must be clear */
        ){
          pOp->p3++;
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, copyOp, ಒಳ_Reg, ಗುರಿ+i);
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ n;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Between(
  Parse *pParse,    /* Parsing and code generating context */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,      /* The BETWEEN expression */
  ಪೂರ್ಣಾಂಕ dest,         /* Jump destination or storage location */
  ಶೂನ್ಯ (*xJump)(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ), /* Action to take */
  ಪೂರ್ಣಾಂಕ jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  ಅಭಿವ್ಯಕ್ತಿ ಅಭಿವ್ಯಕ್ತಿ_And;     /* The AND operator in  x>=y AND x<=z  */
  ಅಭಿವ್ಯಕ್ತಿ comp_ಎಡ;    /* The  x>=y  term */
  ಅಭಿವ್ಯಕ್ತಿ comp_ಬಲ;   /* The  x<=z  term */
  ಪೂರ್ಣಾಂಕ regFree1 = 0; /* Temporary use register */
  ಅಭಿವ್ಯಕ್ತಿ *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&comp_ಎಡ, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
  memset(&comp_ಬಲ, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
  memset(&ಅಭಿವ್ಯಕ್ತಿ_And, 0, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));

  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pDel = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
  ಆದರೆ( db->mallocFailed==0 ){
    ಅಭಿವ್ಯಕ್ತಿ_And.op = TK_AND;
    ಅಭಿವ್ಯಕ್ತಿ_And.p_ಎಡ = &comp_ಎಡ;
    ಅಭಿವ್ಯಕ್ತಿ_And.p_ಬಲ = &comp_ಬಲ;
    comp_ಎಡ.op = TK_GE;
    comp_ಎಡ.p_ಎಡ = pDel;
    comp_ಎಡ.p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
    comp_ಬಲ.op = TK_LE;
    comp_ಬಲ.p_ಎಡ = pDel;
    comp_ಬಲ.p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್(pDel, ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸದಿಶ(pParse, pDel, &regFree1));
    ಆದರೆ( xJump ){
      xJump(pParse, &ಅಭಿವ್ಯಕ್ತಿ_And, dest, jumpIfNull);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Mark the expression is being from the ON or USING clause of a join
      ** so that the sqlite3ExprCodeTarget() routine will not attempt to move
      ** it into the Parse.pConstExpr list.  We should use a new bit for this,
      ** for clarity, but we are out of bits in the Expr.flags field so we
      ** have to reuse the EP_OuterON bit.  Bummer. */
      pDel->flags |= EP_OuterON;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, &ಅಭಿವ್ಯಕ್ತಿ_And, dest);
    }
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree1);
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDel);

  /* Ensure adequate test coverage */
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True  && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True  && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True  && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True  && jumpIfNull!=0 && regFree1!=0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False && jumpIfNull==0 && regFree1==0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False && jumpIfNull==0 && regFree1!=0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False && jumpIfNull!=0 && regFree1==0 );
  testcase( xJump==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False && jumpIfNull!=0 && regFree1!=0 );
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ dest, ಪೂರ್ಣಾಂಕ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ op = 0;
  ಪೂರ್ಣಾಂಕ regFree1 = 0;
  ಪೂರ್ಣಾಂಕ regFree2 = 0;
  ಪೂರ್ಣಾಂಕ r1, r2;

  ಪ್ರತಿಜ್ಞಾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ಆದರೆ( NEVER(v==0) )     ಹಿಂತಿರುಗಿ;  /* Existence of VDBE checked by caller */
  ಆದರೆ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಹಿಂತಿರುಗಿ;  /* No way this can happen */
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Immutable) );
  op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ TK_AND:
    ಸ್ಥಿತಿ TK_OR: {
      ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
      ಆದರೆ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pAlt, dest, jumpIfNull);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಮೊದಲ, *p_ಎರಡನೇ;
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_ಮೊದಲ(p_ಅಭಿವ್ಯಕ್ತಿ) ){
          p_ಮೊದಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
          p_ಎರಡನೇ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p_ಮೊದಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          p_ಎರಡನೇ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        }
        ಆದರೆ( op==TK_AND ){
          ಪೂರ್ಣಾಂಕ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಮೊದಲ, d2,
                             jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಎರಡನೇ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }ಇಲ್ಲದಿದ್ದರೆ{
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಮೊದಲ, dest, jumpIfNull);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಎರಡನೇ, dest, jumpIfNull);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest, jumpIfNull);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRUTH: {
      ಪೂರ್ಣಾಂಕ isNot;      /* IS NOT TRUE or IS NOT FALSE */
      ಪೂರ್ಣಾಂಕ isTrue;     /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT;
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ಆದರೆ( isTrue ^ isNot ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                          isNot ? SQLITE_JUMPIFNULL : 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                           isNot ? SQLITE_JUMPIFNULL : 0);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ISNOT:
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_EQ: {
      ಪೂರ್ಣಾಂಕ addrIsNull;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ) ಹೋಗು default_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      ಸಂಕೇತ_Compare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op,
                  r1, r2, dest, jumpIfNull, ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted));
      ಪ್ರತಿಜ್ಞಾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ಪ್ರತಿಜ್ಞಾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ಪ್ರತಿಜ್ಞಾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ಪ್ರತಿಜ್ಞಾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ಪ್ರತಿಜ್ಞಾ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      ಪ್ರತಿಜ್ಞಾ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ಆದರೆ( addrIsNull ){
        ಆದರೆ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrIsNull);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ISNULL:
    ಸ್ಥಿತಿ TK_NOTNULL: {
      ಪ್ರತಿಜ್ಞಾ( TK_ISNULL==OP_IsNull );   testcase( op==TK_ISNULL );
      ಪ್ರತಿಜ್ಞಾ( TK_NOTNULL==OP_NotNull ); testcase( op==TK_NOTNULL );
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      ಪ್ರತಿಜ್ಞಾ( regFree1==0 || regFree1==r1 );
      ಆದರೆ( regFree1 ) sqlite3_Vdbe_Typeof_ಸ್ತಂಭ(v, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, op, r1, dest);
      VdbeCoverageIf(v, op==TK_ISNULL);
      VdbeCoverageIf(v, op==TK_NOTNULL);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True, jumpIfNull);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_ಒಳ: {
      ಪೂರ್ಣಾಂಕ destIfFalse = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ವನಿಯೋಜಿತ: {
    default_ಅಭಿವ್ಯಕ್ತಿ:
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3VdbeGoto(v, dest);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* No-op */
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &regFree1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_If, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree1);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree2); 
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ dest, ಪೂರ್ಣಾಂಕ jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ op = 0;
  ಪೂರ್ಣಾಂಕ regFree1 = 0;
  ಪೂರ್ಣಾಂಕ regFree2 = 0;
  ಪೂರ್ಣಾಂಕ r1, r2;

  ಪ್ರತಿಜ್ಞಾ( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  ಆದರೆ( NEVER(v==0) ) ಹಿಂತಿರುಗಿ; /* Existence of VDBE checked by caller */
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 )    ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Immutable) );

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
  op = ((p_ಅಭಿವ್ಯಕ್ತಿ->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ISNULL || op==OP_NotNull );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_NOTNULL || op==OP_IsNull );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_NE || op==OP_Eq );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_EQ || op==OP_Ne );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_LT || op==OP_Ge );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_LE || op==OP_Gt );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_GT || op==OP_Le );
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_GE || op==OP_Lt );

  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_AND:
    ಸ್ಥಿತಿ TK_OR: {
      ಅಭಿವ್ಯಕ್ತಿ *pAlt = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(p_ಅಭಿವ್ಯಕ್ತಿ);
      ಆದರೆ( pAlt!=p_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pAlt, dest, jumpIfNull);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಮೊದಲ, *p_ಎರಡನೇ;
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Eval_Rhs_ಮೊದಲ(p_ಅಭಿವ್ಯಕ್ತಿ) ){
          p_ಮೊದಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
          p_ಎರಡನೇ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p_ಮೊದಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          p_ಎರಡನೇ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        }
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ){
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಮೊದಲ, dest, jumpIfNull);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಎರಡನೇ, dest, jumpIfNull);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ d2 = sqlite3VdbeMakeLabel(pParse);
          testcase( jumpIfNull==0 );
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಮೊದಲ, d2,
                            jumpIfNull^SQLITE_JUMPIFNULL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಎರಡನೇ, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest, jumpIfNull);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRUTH: {
      ಪೂರ್ಣಾಂಕ isNot;   /* IS NOT TRUE or IS NOT FALSE */
      ಪೂರ್ಣಾಂಕ isTrue;  /* IS TRUE or IS NOT TRUE */
      testcase( jumpIfNull==0 );
      isNot = p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT;
      isTrue = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      testcase( isTrue && isNot );
      testcase( !isTrue && isNot );
      ಆದರೆ( isTrue ^ isNot ){
        /* IS TRUE and IS NOT FALSE */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                           isNot ? 0 : SQLITE_JUMPIFNULL);

      }ಇಲ್ಲದಿದ್ದರೆ{
        /* IS FALSE and IS NOT TRUE */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, dest,
                          isNot ? 0 : SQLITE_JUMPIFNULL);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ISNOT:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
      op = (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS) ? TK_NE : TK_EQ;
      jumpIfNull = SQLITE_NULLEQ;
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_EQ: {
      ಪೂರ್ಣಾಂಕ addrIsNull;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ) ಹೋಗು default_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) && jumpIfNull!=SQLITE_NULLEQ ){
        addrIsNull = ಅಭಿವ್ಯಕ್ತಿ_Compute_Operands(pParse, p_ಅಭಿವ್ಯಕ್ತಿ,
                                   &r1, &r2, &regFree1, &regFree2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
        r2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, &regFree2);
        addrIsNull = 0;
      }
      ಸಂಕೇತ_Compare(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, op,
                  r1, r2, dest, jumpIfNull,ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Commuted));
      ಪ್ರತಿಜ್ಞಾ(TK_LT==OP_Lt); testcase(op==OP_Lt); VdbeCoverageIf(v,op==OP_Lt);
      ಪ್ರತಿಜ್ಞಾ(TK_LE==OP_Le); testcase(op==OP_Le); VdbeCoverageIf(v,op==OP_Le);
      ಪ್ರತಿಜ್ಞಾ(TK_GT==OP_Gt); testcase(op==OP_Gt); VdbeCoverageIf(v,op==OP_Gt);
      ಪ್ರತಿಜ್ಞಾ(TK_GE==OP_Ge); testcase(op==OP_Ge); VdbeCoverageIf(v,op==OP_Ge);
      ಪ್ರತಿಜ್ಞಾ(TK_EQ==OP_Eq); testcase(op==OP_Eq);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Eq && jumpIfNull==SQLITE_NULLEQ);
      ಪ್ರತಿಜ್ಞಾ(TK_NE==OP_Ne); testcase(op==OP_Ne);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull!=SQLITE_NULLEQ);
      VdbeCoverageIf(v, op==OP_Ne && jumpIfNull==SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      ಆದರೆ( addrIsNull ){
        ಆದರೆ( jumpIfNull ){
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrIsNull);
        }
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ISNULL:
    ಸ್ಥಿತಿ TK_NOTNULL: {
      r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, &regFree1);
      ಪ್ರತಿಜ್ಞಾ( regFree1==0 || regFree1==r1 );
      ಆದರೆ( regFree1 ) sqlite3_Vdbe_Typeof_ಸ್ತಂಭ(v, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, op, r1, dest);
      testcase( op==TK_ISNULL );   VdbeCoverageIf(v, op==TK_ISNULL);
      testcase( op==TK_NOTNULL );  VdbeCoverageIf(v, op==TK_NOTNULL);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Between(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False, jumpIfNull);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_ಒಳ: {
      ಆದರೆ( jumpIfNull ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, dest);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಒಳ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ವನಿಯೋಜಿತ: {
    default_ಅಭಿವ್ಯಕ್ತಿ:
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3VdbeGoto(v, dest);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Always_True(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        /* no-op */
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, &regFree1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
        VdbeCoverage(v);
        testcase( regFree1==0 );
        testcase( jumpIfNull==0 );
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree1);
  sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regFree2);
}

/*
** Like sqlite3ExprIfFalse() except that a copy is made of pExpr before
** code generation, and that copy is deleted after code generation. This
** ensures that the original pExpr is unchanged.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False_Dup(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ dest,ಪೂರ್ಣಾಂಕ jumpIfNull){
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pCopy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
  ಆದರೆ( db->mallocFailed==0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pCopy);
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Compare_Variable(
  ಸ್ಥಿರಾಂಕ Parse *pParse,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pVar,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
){
  ಪೂರ್ಣಾಂಕ res = 2;
  ಪೂರ್ಣಾಂಕ iVar;
  sqlite3_ಮೌಲ್ಯ *pL, *pR = 0;
 
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_VARIABLE && pVar->i_ಸ್ತಂಭ==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( (pParse->db->flags & SQLITE_EnableQPSG)!=0 ) ಹಿಂತಿರುಗಿ 2;
  sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ, SQLITE_UTF8, SQLITE_AFF_BLOB, &pR);
  ಆದರೆ( pR ){
    iVar = pVar->i_ಸ್ತಂಭ;
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iVar, 0);
    pL = sqlite3_Vdbe_Get_Bound_ಮೌಲ್ಯ(pParse->pReprepare, iVar, SQLITE_AFF_BLOB);
    ಆದರೆ( pL ){
      ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(pL)==SQLITE_ಪಠ್ಯ ){
        sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pL); /* Make sure the encoding is UTF-8 */
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3_ಮೌಲ್ಯ_Free(pR);
    sqlite3_ಮೌಲ್ಯ_Free(pL);
  }
  ಹಿಂತಿರುಗಿ res;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(
  ಸ್ಥಿರಾಂಕ Parse *pParse,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pA,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pB,
  ಪೂರ್ಣಾಂಕ iTab
){
  u32 combinedFlags;
  ಆದರೆ( pA==0 || pB==0 ){
    ಹಿಂತಿರುಗಿ pB==pA ? 0 : 2;
  }
  ಆದರೆ( pParse && pA->op==TK_VARIABLE ){
    ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_Compare_Variable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  ಆದರೆ( combinedFlags & EP_Int_ಮೌಲ್ಯ ){
    ಆದರೆ( (pA->flags&pB->flags&EP_Int_ಮೌಲ್ಯ)!=0 && pA->u.i_ಮೌಲ್ಯ==pB->u.i_ಮೌಲ್ಯ ){
      ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ 2;
  }
  ಆದರೆ( pA->op!=pB->op || pA->op==TK_RAISE ){
    ಆದರೆ( pA->op==TK_COLLATE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಎಡ,pB,iTab)<2 ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( pB->op==TK_COLLATE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA,pB->p_ಎಡ,iTab)<2 ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( pA->op==TK_AGG_ಸ್ತಂಭ && pB->op==TK_ಸ್ತಂಭ
     && pB->i_ಕೋಷ್ಟಕ<0 && pA->i_ಕೋಷ್ಟಕ==iTab
    ){
      /* fall through */
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ 2;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA, EP_Int_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB, EP_Int_ಮೌಲ್ಯ) );
  ಆದರೆ( pA->u.zToken ){
    ಆದರೆ( pA->op==TK_FUNCTION || pA->op==TK_AGG_FUNCTION ){
      ಆದರೆ( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ) ಹಿಂತಿರುಗಿ 2;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಪ್ರತಿಜ್ಞಾ( pA->op==pB->op );
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_WinFunc)!=ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pB,EP_WinFunc) ){
        ಹಿಂತಿರುಗಿ 2;
      }
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pA,EP_WinFunc) ){
        ಆದರೆ( sqlite3_ಕಿಟಕಿ_Compare(pParse, pA->y.pWin, pB->y.pWin, 1)!=0 ){
          ಹಿಂತಿರುಗಿ 2;
        }
      }
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pA->op==TK_NULL ){
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pA->op==TK_COLLATE ){
      ಆದರೆ( sqlite3_stricmp(pA->u.zToken,pB->u.zToken)!=0 ) ಹಿಂತಿರುಗಿ 2;
    }ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( pB->u.zToken!=0
     && pA->op!=TK_ಸ್ತಂಭ
     && pA->op!=TK_AGG_ಸ್ತಂಭ
     && strcmp(pA->u.zToken,pB->u.zToken)!=0
    ){
      ಹಿಂತಿರುಗಿ 2;
    }
  }
  ಆದರೆ( (pA->flags & (EP_Distinct|EP_Commuted))
     != (pB->flags & (EP_Distinct|EP_Commuted)) ) ಹಿಂತಿರುಗಿ 2;
  ಆದರೆ( ALWAYS((combinedFlags & EP_TokenOnly)==0) ){
    ಆದರೆ( combinedFlags & EP_xIsSelect ) ಹಿಂತಿರುಗಿ 2;
    ಆದರೆ( (combinedFlags & EP_Fixed_ಲಂಬಸಾಲು)==0
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಎಡ, pB->p_ಎಡ, iTab) ) ಹಿಂತಿರುಗಿ 2;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pA->p_ಬಲ, pB->p_ಬಲ, iTab) ) ಹಿಂತಿರುಗಿ 2;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pA->x.pList, pB->x.pList, iTab) ) ಹಿಂತಿರುಗಿ 2;
    ಆದರೆ( pA->op!=TK_STRING
     && pA->op!=TK_TRUEFALSE
     && ALWAYS((combinedFlags & EP_Reduced)==0)
    ){
      ಆದರೆ( pA->i_ಸ್ತಂಭ!=pB->i_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ 2;
      ಆದರೆ( pA->op2!=pB->op2 && pA->op==TK_TRUTH ) ಹಿಂತಿರುಗಿ 2;
      ಆದರೆ( pA->op!=TK_ಒಳ && pA->i_ಕೋಷ್ಟಕ!=pB->i_ಕೋಷ್ಟಕ && pA->i_ಕೋಷ್ಟಕ!=iTab ){
        ಹಿಂತಿರುಗಿ 2;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pA, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pB, ಪೂರ್ಣಾಂಕ iTab){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pA==0 && pB==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pA==0 || pB==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( pA->n_ಅಭಿವ್ಯಕ್ತಿ!=pB->n_ಅಭಿವ್ಯಕ್ತಿ ) ಹಿಂತಿರುಗಿ 1;
  ಗಾಗಿ(i=0; i<pA->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಪೂರ್ಣಾಂಕ res;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ_A = pA->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ_B = pB->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( pA->a[i].fg.ವಿಂಗಡಿಸು_Flags!=pB->a[i].fg.ವಿಂಗಡಿಸು_Flags ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( (res = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ_A, p_ಅಭಿವ್ಯಕ್ತಿ_B, iTab)) ) ಹಿಂತಿರುಗಿ res;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Like sqlite3ExprCompare() except COLLATE operators at the top-level
** are ignored.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Skip(ಅಭಿವ್ಯಕ್ತಿ *pA,ಅಭಿವ್ಯಕ್ತಿ *pB, ಪೂರ್ಣಾಂಕ iTab){
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,
             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pA),
             sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(pB),
             iTab);
}

/*
** Return non-zero if Expr p can only be true if pNN is not NULL.
**
** Or if seenNot is true, return non-zero if Expr p can only be
** non-NULL if pNN is not NULL
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(
  ಸ್ಥಿರಾಂಕ Parse *pParse,/* Parsing context */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p,      /* The expression to be checked */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pNN,    /* The expression that is NOT NULL */
  ಪೂರ್ಣಾಂಕ iTab,           /* Table being evaluated */
  ಪೂರ್ಣಾಂಕ ನೋಡಿದ_Not         /* Return true only if p can be any non-NULL value */
){
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( pNN );
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, p, pNN, iTab)==0 ){
    ಹಿಂತಿರುಗಿ pNN->op!=TK_NULL;
  }
  ಆಯ್ಕೆ( p->op ){
    ಸ್ಥಿತಿ TK_ಒಳ: {
      ಆದರೆ( ನೋಡಿದ_Not && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_xIsSelect) ) ಹಿಂತಿರುಗಿ 0;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) || (p->x.pList!=0 && p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0) );
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಸ್ಥಿತಿ TK_BETWEEN: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pList;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      pList = p->x.pList;
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      ಆದರೆ( ನೋಡಿದ_Not ) ಹಿಂತಿರುಗಿ 0;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, pNN, iTab, 1)
       || ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ, pNN, iTab, 1)
      ){
        ಹಿಂತಿರುಗಿ 1;
      }
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಸ್ಥಿತಿ TK_EQ:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE:
    ಸ್ಥಿತಿ TK_PLUS:
    ಸ್ಥಿತಿ TK_MINUS:
    ಸ್ಥಿತಿ TK_BITOR:
    ಸ್ಥಿತಿ TK_LSHIFT:
    ಸ್ಥಿತಿ TK_RSHIFT:
    ಸ್ಥಿತಿ TK_CONCAT:
      ನೋಡಿದ_Not = 1;
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ TK_ನಕ್ಷತ್ರ:
    ಸ್ಥಿತಿ TK_REM:
    ಸ್ಥಿತಿ TK_BITAND:
    ಸ್ಥಿತಿ TK_SLASH: {
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಬಲ, pNN, iTab, ನೋಡಿದ_Not) ) ಹಿಂತಿರುಗಿ 1;
      /* no break */ deliberate_fall_through
    }
    ಸ್ಥಿತಿ TK_ಹರವು:
    ಸ್ಥಿತಿ TK_COLLATE:
    ಸ್ಥಿತಿ TK_UPLUS:
    ಸ್ಥಿತಿ TK_UMINUS: {
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, ನೋಡಿದ_Not);
    }
    ಸ್ಥಿತಿ TK_TRUTH: {
      ಆದರೆ( ನೋಡಿದ_Not ) ಹಿಂತಿರುಗಿ 0;
      ಆದರೆ( p->op2!=TK_IS ) ಹಿಂತಿರುಗಿ 0;
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
    ಸ್ಥಿತಿ TK_BITNOT:
    ಸ್ಥಿತಿ TK_NOT: {
      ಹಿಂತಿರುಗಿ ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, p->p_ಎಡ, pNN, iTab, 1);
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return true if the boolean value of the expression is always either
** FALSE or NULL.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Not_True(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ v;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NULL ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUEFALSE && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ)==0 ) ಹಿಂತಿರುಗಿ 1;
  v = 1;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p_ಅಭಿವ್ಯಕ್ತಿ, &v, 0, 0) && v==0 ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_IIF(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
    FuncDef *pDef;
    ಆದರೆ( (z[0]!='i' && z[0]!='I') ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 ) ಹಿಂತಿರುಗಿ 0;
    pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(db, z, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ, ENC(db), 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    ಆದರೆ( pDef==0 ) ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( NEVER(pDef==0) ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_INLINE)==0 ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( SQLITE_PTR_ಗಮ್ಯ_INT(pDef->p_User_ದತ್ತಾಂಶ)!=INLINEFUNC_iif ) ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE ){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 ) ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
  pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
  ಪ್ರತಿಜ್ಞಾ( pList!=0 );
  ಆದರೆ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( pList->n_ಅಭಿವ್ಯಕ್ತಿ==3 && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Not_True(pList->a[2].p_ಅಭಿವ್ಯಕ್ತಿ) ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(
  ಸ್ಥಿರಾಂಕ Parse *pParse,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pE1,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pE2,
  ಪೂರ್ಣಾಂಕ iTab
){
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(pParse, pE1, pE2, iTab)==0 ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( pE2->op==TK_OR
   && (sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(pParse, pE1, pE2->p_ಎಡ, iTab)
             || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(pParse, pE1, pE2->p_ಬಲ, iTab) )
  ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( pE2->op==TK_NOTNULL
   && ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Not_Null(pParse, pE1, pE2->p_ಎಡ, iTab, 0)
  ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_IIF(pParse->db, pE1) ){
    ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(pParse,pE1->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,pE2,iTab);
  }
  ಹಿಂತಿರುಗಿ 0;
}

/* This is a helper function to impliesNotNullRow().  In this routine,
** set pWalker->eCode to one only if *both* of the input expressions
** separately have the implies-not-null-row property.
*/
ಸ್ಥಿರ ಶೂನ್ಯ both_Imply_Not_Null_ಅಡ್ಡಸಾಲು(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *pE1, ಅಭಿವ್ಯಕ್ತಿ *pE2){
  ಆದರೆ( pWalker->e_ಸಂಕೇತ==0 ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pE1);
    ಆದರೆ( pWalker->e_ಸಂಕೇತ ){
      pWalker->e_ಸಂಕೇತ = 0;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pE2);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೂಚಿಸುತ್ತದೆ_Not_Null_ಅಡ್ಡಸಾಲು(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_ಸ್ತಂಭ );
  testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಹಿಂತಿರುಗಿ WRC_Prune;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) && pWalker->mWFlags ){
    /* If iCur is used in an inner-join ON clause to the left of a
    ** RIGHT JOIN, that does *not* mean that the table must be non-null.
    ** But it is difficult to check for that condition precisely.
    ** To keep things simple, any use of iCur from any inner-join is
    ** ignored while attempting to simplify a RIGHT JOIN. */
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_ISNOT:
    ಸ್ಥಿತಿ TK_ISNULL:
    ಸ್ಥಿತಿ TK_NOTNULL:
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ಸದಿಶ:
    ಸ್ಥಿತಿ TK_FUNCTION:
    ಸ್ಥಿತಿ TK_TRUTH:
    ಸ್ಥಿತಿ TK_CASE:
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNULL );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸದಿಶ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUTH );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE );
      ಹಿಂತಿರುಗಿ WRC_Prune;

    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ:
      ಆದರೆ( pWalker->u.iCur==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ){
        pWalker->e_ಸಂಕೇತ = 1;
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಹಿಂತಿರುಗಿ WRC_Prune;

    ಸ್ಥಿತಿ TK_OR:
    ಸ್ಥಿತಿ TK_AND:
      /* Both sides of an AND or OR must separately imply non-null-row.
      ** Consider these cases:
      **    1.  NOT (x AND y)
      **    2.  x OR y
      ** If only one of x or y is non-null-row, then the overall expression
      ** can be true if the other arm is false (case 1) or true (case 2).
      */
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_OR );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND );
      both_Imply_Not_Null_ಅಡ್ಡಸಾಲು(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      ಹಿಂತಿರುಗಿ WRC_Prune;
       
    ಸ್ಥಿತಿ TK_ಒಳ:
      /* Beware of "x NOT IN ()" and "x NOT IN (SELECT 1 WHERE false)",
      ** both of which can be true.  But apart from these cases, if
      ** the left-hand side of the IN is NULL then the IN itself will be
      ** NULL. */
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) && ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ>0) ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      }
      ಹಿಂತಿರುಗಿ WRC_Prune;

    ಸ್ಥಿತಿ TK_BETWEEN:
      /* In "x NOT BETWEEN y AND z" either x must be non-null-row or else
      ** both y and z must be non-null row */
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      both_Imply_Not_Null_ಅಡ್ಡಸಾಲು(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,
                                   p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
      ಹಿಂತಿರುಗಿ WRC_Prune;

    /* Virtual tables are allowed to use constraints like x=NULL.  So
    ** a term of the form x=y does not prove that y is not null if x
    ** is the column of a virtual table */
    ಸ್ಥಿತಿ TK_EQ:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LE );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GT );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GE );
      /* The y.pTab=0 assignment in wherecode.c always happens after the
      ** impliesNotNullRow() test */
      ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op!=TK_ಸ್ತಂಭ || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op!=TK_ಸ್ತಂಭ || ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಬಲ) );
      ಆದರೆ( (p_ಎಡ->op==TK_ಸ್ತಂಭ
           && ALWAYS(p_ಎಡ->y.pTab!=0)
           && IsVirtual(p_ಎಡ->y.pTab))
       || (p_ಬಲ->op==TK_ಸ್ತಂಭ
           && ALWAYS(p_ಬಲ->y.pTab!=0)
           && IsVirtual(p_ಬಲ->y.pTab))
      ){
        ಹಿಂತಿರುಗಿ WRC_Prune;
      }
      /* no break */ deliberate_fall_through
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Non_Null_ಅಡ್ಡಸಾಲು(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ iTab, ಪೂರ್ಣಾಂಕ isRJ){
  Walker w;
  p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p);
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->op==TK_NOTNULL ){
    p = p->p_ಎಡ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( p->op==TK_AND ){
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Non_Null_ಅಡ್ಡಸಾಲು(p->p_ಎಡ, iTab, isRJ) ) ಹಿಂತಿರುಗಿ 1;
      p = p->p_ಬಲ;
    }
  }
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಸೂಚಿಸುತ್ತದೆ_Not_Null_ಅಡ್ಡಸಾಲು;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.e_ಸಂಕೇತ = 0;
  w.mWFlags = isRJ!=0;
  w.u.iCur = iTab;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p);
  ಹಿಂತಿರುಗಿ w.e_ಸಂಕೇತ;
}

/*
** An instance of the following structure is used by the tree walker
** to determine if an expression can be evaluated by reference to the
** index only, without having to do a search for the corresponding
** table entry.  The IdxCover.pIdx field is the index.  IdxCover.iCur
** is the cursor for the table.
*/
ರಚನೆ IdxCover {
  ಸೂಚ್ಯಂಕ *pIdx;     /* The index to be tested for coverage */
  ಪೂರ್ಣಾಂಕ iCur;        /* Cursor number for the table corresponding to the index */
};

/*
** Check to see if there are references to columns in table
** pWalker->u.pIdxCover->iCur can be satisfied using the index
** pWalker->u.pIdxCover->pIdx.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Idx_Cover(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pWalker->u.pIdxCover->iCur
   && sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pWalker->u.pIdxCover->pIdx, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ)<0
  ){
    pWalker->e_ಸಂಕೇತ = 1;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Covered_By_ಸೂಚ್ಯಂಕ(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,        /* The index to be tested */
  ಪೂರ್ಣಾಂಕ iCur,           /* The cursor number for the corresponding table */
  ಸೂಚ್ಯಂಕ *pIdx         /* The index that might be used for coverage */
){
  Walker w;
  ರಚನೆ IdxCover xcov;
  memset(&w, 0, ಗಾತ್ರ(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_Idx_Cover;
  w.u.pIdxCover = &xcov;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  ಹಿಂತಿರುಗಿ !w.e_ಸಂಕೇತ;
}


/* Structure used to pass information throughout the Walker in order to
** implement sqlite3ReferencesSrcList().
*/
ರಚನೆ RefSrcList {
  sqlite3 *db;         /* Database connection used for sqlite3DbRealloc() */
  SrcList *pRef;       /* Looking for references to these tables */
  i64 nExclude;        /* Number of tables to exclude from the search */
  ಪೂರ್ಣಾಂಕ *aiExclude;      /* Cursor IDs for tables to exclude from the search */
};

/*
** Walker SELECT callbacks for sqlite3ReferencesSrcList().
**
** When entering a new subquery on the pExpr argument, add all FROM clause
** entries for that subquery to the exclude list.
**
** When leaving the subquery, remove those entries from the exclude list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selectRefEnter(Walker *pWalker, Select *pSelect){
  ರಚನೆ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  ಪೂರ್ಣಾಂಕ *piNew;
  ಆದರೆ( pSrc->nSrc==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
  ಆದರೆ( piNew==0 ){
    p->nExclude = 0;
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->aiExclude = piNew;
  }
  ಗಾಗಿ(i=0; i<pSrc->nSrc; i++, j++){
     p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ selectRefLeave(Walker *pWalker, Select *pSelect){
  ರಚನೆ RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  ಆದರೆ( p->nExclude ){
    ಪ್ರತಿಜ್ಞಾ( p->nExclude>=pSrc->nSrc );
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Ref_ಗಮ್ಯ_Src_List(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_ಸ್ತಂಭ
  ){
    ಪೂರ್ಣಾಂಕ i;
    ರಚನೆ RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    ಪೂರ್ಣಾಂಕ nSrc = pSrc ? pSrc->nSrc : 0;
    ಗಾಗಿ(i=0; i<nSrc; i++){
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pSrc->a[i].iCursor ){
        pWalker->e_ಸಂಕೇತ |= 1;
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }
    }
    ಗಾಗಿ(i=0; i<p->nExclude && p->aiExclude[i]!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ; i++){}
    ಆದರೆ( i>=p->nExclude ){
      pWalker->e_ಸಂಕೇತ |= 2;
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಪೂರ್ಣಾಂಕ sqlite3ReferencesSrcList(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, SrcList *pSrcList){
  Walker w;
  ರಚನೆ RefSrcList x;
  ಪ್ರತಿಜ್ಞಾ( pParse->db!=0 );
  memset(&w, 0, ಗಾತ್ರ(w));
  memset(&x, 0, ಗಾತ್ರ(x));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಅಭಿವ್ಯಕ್ತಿ_Ref_ಗಮ್ಯ_Src_List;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಕ್ರಮ );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList!=0 );
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( x.aiExclude ) sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  ಆದರೆ( w.e_ಸಂಕೇತ & 0x01 ){
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( w.e_ಸಂಕೇತ ){
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ -1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ agginfo_Persist_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( ALWAYS(!ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced))
   && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0
  ){
    AggInfo *pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
    ಪೂರ್ಣಾಂಕ iAgg = p_ಅಭಿವ್ಯಕ್ತಿ->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;
    ಪ್ರತಿಜ್ಞಾ( iAgg>=0 );
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ){
      ಆದರೆ( iAgg<pAggInfo->n_ಸ್ತಂಭ
       && pAggInfo->a_ಲಂಬಸಾಲು[iAgg].pCExpr==p_ಅಭಿವ್ಯಕ್ತಿ
      ){
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pAggInfo->a_ಲಂಬಸಾಲು[iAgg].pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
      ಆದರೆ( ALWAYS(iAgg<pAggInfo->nFunc)
       && pAggInfo->aFunc[iAgg].pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ
      ){
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, p_ಅಭಿವ್ಯಕ್ತಿ) ){
          pAggInfo->aFunc[iAgg].pFExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
        }
      }
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Initialize a Walker object so that will persist AggInfo entries referenced
** by the tree that is walked.
*/
ಶೂನ್ಯ sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse){
  memset(pWalker, 0, ಗಾತ್ರ(*pWalker));
  pWalker->pParse = pParse;
  pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback = agginfo_Persist_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_Agg_Info_ಸ್ತಂಭ(sqlite3 *db, AggInfo *pInfo){
  ಪೂರ್ಣಾಂಕ i;
  pInfo->a_ಲಂಬಸಾಲು = sqlite3ArrayAllocate(
       db,
       pInfo->a_ಲಂಬಸಾಲು,
       ಗಾತ್ರ(pInfo->a_ಲಂಬಸಾಲು[0]),
       &pInfo->n_ಸ್ತಂಭ,
       &i
  );
  ಹಿಂತಿರುಗಿ i;
}   

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_Agg_Info_Func(sqlite3 *db, AggInfo *pInfo){
  ಪೂರ್ಣಾಂಕ i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db,
       pInfo->aFunc,
       ಗಾತ್ರ(pInfo->aFunc[0]),
       &pInfo->nFunc,
       &i
  );
  ಹಿಂತಿರುಗಿ i;
}

/*
** Search the AggInfo object for an aCol[] entry that has iTable and iColumn.
** Return the index in aCol[] of the entry that describes that column.
**
** If no prior entry is found, create a new one and return -1.  The
** new column will have an index of pAggInfo->nColumn-1.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕಂಡುಹಿಡಿ_Or_Create_Agg_Info_ಸ್ತಂಭ(
  Parse *pParse,       /* Parsing context */
  AggInfo *pAggInfo,   /* The AggInfo object to search and/or modify */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Expr describing the column to find or insert */
){
  ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *p_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ k;
  ಪೂರ್ಣಾಂಕ mx_ಪದ = pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ];

  ಪ್ರತಿಜ್ಞಾ( mx_ಪದ <= SMXV(i16) );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg==0 );
  p_ಲಂಬಸಾಲು = pAggInfo->a_ಲಂಬಸಾಲು;
  ಗಾಗಿ(k=0; k<pAggInfo->n_ಸ್ತಂಭ; k++, p_ಲಂಬಸಾಲು++){
    ಆದರೆ( p_ಲಂಬಸಾಲು->pCExpr==p_ಅಭಿವ್ಯಕ್ತಿ ) ಹಿಂತಿರುಗಿ;
    ಆದರೆ( p_ಲಂಬಸಾಲು->i_ಕೋಷ್ಟಕ==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ
     && p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ
     && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IF_NULL_ಅಡ್ಡಸಾಲು
    ){
      ಹೋಗು fix_ಮೇಲೆ_ಅಭಿವ್ಯಕ್ತಿ;
    }
  }
  k = ಕೂಡಿಸು_Agg_Info_ಸ್ತಂಭ(pParse->db, pAggInfo);
  ಆದರೆ( k<0 ){
    /* OOM on resize */
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( k>mx_ಪದ ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "more than %d aggregate terms", mx_ಪದ);
    k = mx_ಪದ;
  }
  p_ಲಂಬಸಾಲು = &pAggInfo->a_ಲಂಬಸಾಲು[k];
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
  p_ಲಂಬಸಾಲು->pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
  p_ಲಂಬಸಾಲು->i_ಕೋಷ್ಟಕ = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
  p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
  p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ = -1;
  p_ಲಂಬಸಾಲು->pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
  ಆದರೆ( pAggInfo->p_ಗುಂಪು_By && p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_IF_NULL_ಅಡ್ಡಸಾಲು ){
    ಪೂರ್ಣಾಂಕ j, n;
    ಅಭಿವ್ಯಕ್ತಿ_List *pGB = pAggInfo->p_ಗುಂಪು_By;
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಪದ = pGB->a;
    n = pGB->n_ಅಭಿವ್ಯಕ್ತಿ;
    ಗಾಗಿ(j=0; j<n; j++, p_ಪದ++){
      ಅಭಿವ್ಯಕ್ತಿ *pE = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( pE->op==TK_ಸ್ತಂಭ
       && pE->i_ಕೋಷ್ಟಕ==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ
       && pE->i_ಸ್ತಂಭ==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ
      ){
        p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ = j;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಆದರೆ( p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ<0 ){
    p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ = pAggInfo->n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ++;
  }
fix_ಮೇಲೆ_ಅಭಿವ್ಯಕ್ತಿ:
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0 || p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
  p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ ){
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_ಸ್ತಂಭ;
  }
  ಪ್ರತಿಜ್ಞಾ( k <= SMXV(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg) );
  p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = (i16)k;
}

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ analyzeAggregate(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  ಪ್ರತಿಜ್ಞಾ( pNC->ncFlags & NC_UAggInfo );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg==0 );
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *pIEpr;
      ಅಭಿವ್ಯಕ್ತಿ tmp;
      ಪ್ರತಿಜ್ಞಾ( pParse->iSelfTab==0 );
      ಆದರೆ( (pNC->ncFlags & NC_ಒಳ_Agg_Func)==0 ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( pParse->pIdxEpr==0 ) ನಿಲ್ಲಿಸು;
      ಗಾಗಿ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
        ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur = pIEpr->i_ದತ್ತಾಂಶ_Cur;
        ಆದರೆ( i_ದತ್ತಾಂಶ_Cur<0 ) ಮುಂದುವರಿ;
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, pIEpr->p_ಅಭಿವ್ಯಕ್ತಿ, i_ದತ್ತಾಂಶ_Cur)==0 ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( pIEpr==0 ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( NEVER(!ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ)) ) ನಿಲ್ಲಿಸು;
      ಗಾಗಿ(i=0; i<pSrcList->nSrc; i++){
         ಆದರೆ( pSrcList->a[i].iCursor==pIEpr->i_ದತ್ತಾಂಶ_Cur ){
           testcase( i>0 );
           ನಿಲ್ಲಿಸು;
         }
      }
      ಆದರೆ( i>=pSrcList->nSrc ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0) ) ನಿಲ್ಲಿಸು; /* Resolved by outer context */
      ಆದರೆ( pParse->nErr ){ ಹಿಂತಿರುಗಿ WRC_Abort; }

      /* If we reach this point, it means that expression pExpr can be
      ** translated into a reference to an index column as described by
      ** pIEpr.
      */
      memset(&tmp, 0, ಗಾತ್ರ(tmp));
      tmp.op = TK_AGG_ಸ್ತಂಭ;
      tmp.i_ಕೋಷ್ಟಕ = pIEpr->iIdxCur;
      tmp.i_ಸ್ತಂಭ = pIEpr->i_Idx_ಲಂಬಸಾಲು;
      ಕಂಡುಹಿಡಿ_Or_Create_Agg_Info_ಸ್ತಂಭ(pParse, pAggInfo, &tmp);
      ಆದರೆ( pParse->nErr ){ ಹಿಂತಿರುಗಿ WRC_Abort; }
      ಪ್ರತಿಜ್ಞಾ( pAggInfo->a_ಲಂಬಸಾಲು!=0 );
      ಪ್ರತಿಜ್ಞಾ( tmp.iAgg<pAggInfo->n_ಸ್ತಂಭ );
      pAggInfo->a_ಲಂಬಸಾಲು[tmp.iAgg].pCExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
      p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
      p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = tmp.iAgg;
      ಹಿಂತಿರುಗಿ WRC_Prune;
    }
    ಸ್ಥಿತಿ TK_IF_NULL_ಅಡ್ಡಸಾಲು:
    ಸ್ಥಿತಿ TK_AGG_ಸ್ತಂಭ:
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_ಸ್ತಂಭ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      ಆದರೆ( ALWAYS(pSrcList!=0) ){
        Src_ಅಂಶ *p_ಅಂಶ = pSrcList->a;
        ಗಾಗಿ(i=0; i<pSrcList->nSrc; i++, p_ಅಂಶ++){
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
          ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==p_ಅಂಶ->iCursor ){
            ಕಂಡುಹಿಡಿ_Or_Create_Agg_Info_ಸ್ತಂಭ(pParse, pAggInfo, p_ಅಭಿವ್ಯಕ್ತಿ);
            ನಿಲ್ಲಿಸು;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      ಹಿಂತಿರುಗಿ WRC_Continue;
    }
    ಸ್ಥಿತಿ TK_AGG_FUNCTION: {
      ಆದರೆ( (pNC->ncFlags & NC_ಒಳ_Agg_Func)==0
       && pWalker->walker_ಆಳ==p_ಅಭಿವ್ಯಕ್ತಿ->op2
       && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0
      ){
        /* Check to see if pExpr is a duplicate of another aggregate
        ** function that is already in the pAggInfo structure
        */
        ರಚನೆ AggInfo_func *p_ಅಂಶ = pAggInfo->aFunc;
        ಪೂರ್ಣಾಂಕ mx_ಪದ = pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ];
        ಪ್ರತಿಜ್ಞಾ( mx_ಪದ <= SMXV(i16) );
        ಗಾಗಿ(i=0; i<pAggInfo->nFunc; i++, p_ಅಂಶ++){
          ಆದರೆ( NEVER(p_ಅಂಶ->pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ) ) ನಿಲ್ಲಿಸು;
          ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಂಶ->pFExpr, p_ಅಭಿವ್ಯಕ್ತಿ, -1)==0 ){
            ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( i>mx_ಪದ ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "more than %d aggregate terms", mx_ಪದ);
          i = mx_ಪದ;
          ಪ್ರತಿಜ್ಞಾ( i<pAggInfo->nFunc );
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = ಕೂಡಿಸು_Agg_Info_Func(pParse->db, pAggInfo);
          ಆದರೆ( i>=0 ){
            ಪೂರ್ಣಾಂಕ nArg;
            ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect) );
            p_ಅಂಶ = &pAggInfo->aFunc[i];
            p_ಅಂಶ->pFExpr = p_ಅಭಿವ್ಯಕ್ತಿ;
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಅಭಿವ್ಯಕ್ತಿ) );
            nArg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ? p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
            p_ಅಂಶ->pFunc = sqlite3_ಕಂಡುಹಿಡಿ_Function(pParse->db,
                                         p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, nArg, enc, 0);
            ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->bOBUnique==0 );
            ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ
             && (p_ಅಂಶ->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)==0
            ){
              /* The NEEDCOLL test above causes any ORDER BY clause on
              ** aggregate min() or max() to be ignored. */
              ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;
              ಪ್ರತಿಜ್ಞಾ( nArg>0 );
              ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಕ್ರಮ );
              ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
              p_ಅಂಶ->iOBTab = pParse->nTab++;
              pOBList = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList;
              ಪ್ರತಿಜ್ಞಾ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
              ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->bOBUnique==0 );
              ಆದರೆ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ==1
               && nArg==1
               && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0,pOBList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,
                               p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ,-1)==0
              ){
                p_ಅಂಶ->bOBPayload = 0;
                p_ಅಂಶ->bOBUnique = ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct);
              }ಇಲ್ಲದಿದ್ದರೆ{
                p_ಅಂಶ->bOBPayload = 1;
              }
              p_ಅಂಶ->bUseSubtype =
                    (p_ಅಂಶ->pFunc->funcFlags & SQLITE_SUBTYPE)!=0;
            }ಇಲ್ಲದಿದ್ದರೆ{
              p_ಅಂಶ->iOBTab = -1;
            }
            ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct) && !p_ಅಂಶ->bOBUnique ){
              p_ಅಂಶ->iDistinct = pParse->nTab++;
            }ಇಲ್ಲದಿದ್ದರೆ{
              p_ಅಂಶ->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );
        ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);
        ಪ್ರತಿಜ್ಞಾ( i <= SMXV(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg) );
        p_ಅಭಿವ್ಯಕ್ತಿ->iAgg = (i16)i;
        p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo = pAggInfo;
        ಹಿಂತಿರುಗಿ WRC_Prune;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ WRC_Continue;
      }
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(NameContext *pNC, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Walker w;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = analyzeAggregate;
  w.xSelectCallback = sqlite3_Walker_ಆಳ_Increase;
  w.xSelectCallback2 = sqlite3_Walker_ಆಳ_Decrease;
  w.walker_ಆಳ = 0;
  w.u.pNC = pNC;
  w.pParse = 0;
  ಪ್ರತಿಜ್ಞಾ( pNC->pSrcList!=0 );
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(NameContext *pNC, ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pList ){
    ಗಾಗಿ(p_ಅಂಶ=pList->a, i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(pNC, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(Parse *pParse){
  ಆದರೆ( pParse->n_ತಾತ್ಕಾಲಿಕ_Reg==0 ){
    ಹಿಂತಿರುಗಿ ++pParse->nMem;
  }
  ಹಿಂತಿರುಗಿ pParse->a_ತಾತ್ಕಾಲಿಕ_Reg[--pParse->n_ತಾತ್ಕಾಲಿಕ_Reg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
*/
ಶೂನ್ಯ sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(Parse *pParse, ಪೂರ್ಣಾಂಕ iReg){
  ಆದರೆ( iReg ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    /* Verify that iReg is not already on the unused register list */
    ಪೂರ್ಣಾಂಕ kk;
    ಗಾಗಿ(kk=0; kk<pParse->n_ತಾತ್ಕಾಲಿಕ_Reg; kk++){
      ಪ್ರತಿಜ್ಞಾ( pParse->a_ತಾತ್ಕಾಲಿಕ_Reg[kk]!=iReg );
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3VdbeReleaseRegisters(pParse, iReg, 1, 0, 0);
    ಆದರೆ( pParse->n_ತಾತ್ಕಾಲಿಕ_Reg<Array_ಗಾತ್ರ(pParse->a_ತಾತ್ಕಾಲಿಕ_Reg) ){
      pParse->a_ತಾತ್ಕಾಲಿಕ_Reg[pParse->n_ತಾತ್ಕಾಲಿಕ_Reg++] = iReg;
    }
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(Parse *pParse, ಪೂರ್ಣಾಂಕ nReg){
  ಪೂರ್ಣಾಂಕ i, n;
  ಆದರೆ( nReg==1 ) ಹಿಂತಿರುಗಿ sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  ಆದರೆ( nReg<=n ){
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }ಇಲ್ಲದಿದ್ದರೆ{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  ಹಿಂತಿರುಗಿ i;
}
ಶೂನ್ಯ sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(Parse *pParse, ಪೂರ್ಣಾಂಕ iReg, ಪೂರ್ಣಾಂಕ nReg){
  ಆದರೆ( nReg==1 ){
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, iReg);
    ಹಿಂತಿರುಗಿ;
  }
  sqlite3VdbeReleaseRegisters(pParse, iReg, nReg, 0, 0);
  ಆದರೆ( nReg>pParse->nRangeReg ){
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
ಶೂನ್ಯ sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(Parse *pParse){
  pParse->n_ತಾತ್ಕಾಲಿಕ_Reg = 0;
  pParse->nRangeReg = 0;
}

/*
** Make sure sufficient registers have been allocated so that
** iReg is a valid register number.
*/
ಶೂನ್ಯ sqlite3_Touch_ರಿಜಿಸ್ಟರ್(Parse *pParse, ಪೂರ್ಣಾಂಕ iReg){
  ಆದರೆ( pParse->nMem<iReg ) pParse->nMem = iReg;
}

#ಆದರೆ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
/*
** Return the latest reusable register in the set of all registers.
** The value returned is no less than iMin.  If any register iMin or
** greater is in permanent use, then return one more than that last
** permanent register.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಮೊದಲ_ಲಭ್ಯ_ರಿಜಿಸ್ಟರ್(Parse *pParse, ಪೂರ್ಣಾಂಕ iMin){
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
  ಆದರೆ( pList ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಆದರೆ( pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg>=iMin ){
        iMin = pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg + 1;
      }
    }
  }
  pParse->n_ತಾತ್ಕಾಲಿಕ_Reg = 0;
  pParse->nRangeReg = 0;
  ಹಿಂತಿರುಗಿ iMin;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 || SQLITE_DEBUG */

/*
** Validate that no temporary register falls within the range of
** iFirst..iLast, inclusive.  This routine is only call from within assert()
** statements.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3_No_Temps_ಒಳ_Range(Parse *pParse, ಪೂರ್ಣಾಂಕ i_ಮೊದಲ, ಪೂರ್ಣಾಂಕ i_ಕೊನೆಯ){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pParse->nRangeReg>0
   && pParse->iRangeReg+pParse->nRangeReg > i_ಮೊದಲ
   && pParse->iRangeReg <= i_ಕೊನೆಯ
  ){
     ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ(i=0; i<pParse->n_ತಾತ್ಕಾಲಿಕ_Reg; i++){
    ಆದರೆ( pParse->a_ತಾತ್ಕಾಲಿಕ_Reg[i]>=i_ಮೊದಲ && pParse->a_ತಾತ್ಕಾಲಿಕ_Reg[i]<=i_ಕೊನೆಯ ){
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಆದರೆ( pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = pParse->p_Const_ಅಭಿವ್ಯಕ್ತಿ;
    ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪೂರ್ಣಾಂಕ iReg = pList->a[i].u.i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg;
      ಆದರೆ( iReg==0 ) ಮುಂದುವರಿ;
      ಆದರೆ( iReg>=i_ಮೊದಲ && iReg<=i_ಕೊನೆಯ ) ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
